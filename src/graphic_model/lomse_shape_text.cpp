//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#include "lomse_shape_text.h"

#include "lomse_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"


namespace lomse
{

//=======================================================================================
// GmoShapeText implementation
//=======================================================================================
GmoShapeText::GmoShapeText(ImoObj* pCreatorImo, ShapeId idx, const std::string& text,
                           ImoStyle* pStyle, const string& language, LUnits xLeft,
                           LUnits yBaseline, LibraryScope& libraryScope)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_text, idx, Color(0,0,0))
    , m_text(text)
    , m_language(language)
    , m_pStyle(pStyle)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
    , m_space(0.0f)
{
    //bounds
    select_font();
    TextMeter meter(m_libraryScope);
    m_size.width = meter.measure_width(text);
    m_size.height = meter.get_ascender() - meter.get_descender();   //meter.get_font_height();

    //position
    m_space = m_size.height - meter.get_ascender() + meter.get_descender();
    m_origin.x = xLeft;
    m_origin.y = yBaseline - meter.get_ascender() + m_space;

    //color
    if (m_pStyle)
        m_color = m_pStyle->color();
}

//---------------------------------------------------------------------------------------
Color GmoShapeText::get_normal_color()
{
    if (m_pStyle)
        return m_pStyle->color();
    else
        return m_color;
}

//---------------------------------------------------------------------------------------
void GmoShapeText::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    //select_font();
    TextMeter meter(m_libraryScope);
    if (!m_pStyle)
        meter.select_font(m_language, "", "Liberation serif", 12.0);
    else
        meter.select_font(m_language,
                          m_pStyle->font_file(),
                          m_pStyle->font_name(),
                          m_pStyle->font_size(),
                          m_pStyle->is_bold(),
                          m_pStyle->is_italic() );

    pDrawer->set_text_color( determine_color_to_use(opt) );
    LUnits x = m_origin.x;
    LUnits y = m_origin.y + meter.get_ascender() - m_space;     //reference is at text baseline
    pDrawer->draw_text(x, y, m_text);

    //std::string str("¿This? is a test: Ñ € & abc. Ruso:Текст на кирилица");
    //pDrawer->draw_text(m_xPos, m_yPos, str);

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeText::select_font()
{
    TextMeter meter(m_libraryScope);
    if (!m_pStyle)
        meter.select_font(m_language, "", "Liberation serif", 12.0);
    else
        meter.select_font(m_language,
                          m_pStyle->font_file(),
                          m_pStyle->font_name(),
                          m_pStyle->font_size(),
                          m_pStyle->is_bold(),
                          m_pStyle->is_italic() );
}

//---------------------------------------------------------------------------------------
void GmoShapeText::set_text(const std::string& text)
{
    m_text = text;
    select_font();
    TextMeter meter(m_libraryScope);
    m_size.width = meter.measure_width(text);
    set_dirty(true);
}



//=======================================================================================
// GmoShapeWord implementation
//=======================================================================================
GmoShapeWord::GmoShapeWord(ImoObj* pCreatorImo, ShapeId idx, const wstring& text,
                           ImoStyle* pStyle, const string& language,
                           LUnits x, LUnits y, LUnits halfLeading,
                           LibraryScope& libraryScope)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_word, idx, Color(0,0,0))
    , m_text(text)
    , m_language(language)
    , m_pStyle(pStyle)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
    , m_halfLeading(halfLeading)
{
    //bounds
    TextMeter meter(m_libraryScope);
    select_font();
    m_size.width = meter.measure_width(text);
    m_size.height = halfLeading + meter.get_font_height() + halfLeading;

    //position
    m_origin.x = x;
    m_origin.y = y;

    //other
    m_color = m_pStyle->color();
    m_baseline = m_halfLeading + meter.get_ascender();          //relative to m_origin.y
}

//---------------------------------------------------------------------------------------
Color GmoShapeWord::get_normal_color()
{
    return m_color;
}

//---------------------------------------------------------------------------------------
void GmoShapeWord::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (!static_cast<ImoContentObj*>(m_pCreatorImo)->is_visible())
        return;

    select_font();
    Color color = determine_color_to_use(opt);
    pDrawer->set_text_color(color);
    //AWARE: FreeType reference is at baseline
    pDrawer->draw_text(m_origin.x, m_baseline + m_origin.y, m_text);
//    URect pos = determine_text_position_and_size();
//    pDrawer->draw_text(pos.x, pos.y, m_label);

    //text decoration
    if (m_pStyle->text_decoration() == ImoStyle::k_decoration_underline)
    {
        LUnits xStart = m_origin.x;
        LUnits xEnd = m_origin.x + m_size.width;
        LUnits y = m_origin.y + m_size.height * 0.80f;
        pDrawer->begin_path();
        pDrawer->fill(color);
        pDrawer->stroke(color);
        pDrawer->stroke_width( m_size.height * 0.075f );
        pDrawer->move_to(xStart, y);
        pDrawer->hline_to(xEnd);
        pDrawer->end_path();
    }

    //draw reference lines
    if (opt.must_draw_box_for(GmoObj::k_box_paragraph))
    {
        TextMeter meter(m_libraryScope);
        LUnits xStart = m_origin.x;
        LUnits xEnd = m_origin.x + m_size.width;
        pDrawer->begin_path();
        pDrawer->fill(Color(0, 0, 0, 0));     //transparent black

        //text-top (ascender: cyan)
        LUnits yTextTop = m_origin.y + m_halfLeading;
        pDrawer->begin_path();
        pDrawer->stroke(Color(0, 255, 255));
        pDrawer->stroke_width(10.0);
        pDrawer->move_to(xStart, yTextTop);
        pDrawer->hline_to(xEnd);
        pDrawer->end_path();

        //baseline (blue)
        LUnits yBase = yTextTop + meter.get_ascender();
        pDrawer->begin_path();
        pDrawer->stroke(Color(0, 0, 255));
        pDrawer->stroke_width(10.0);
        pDrawer->move_to(xStart, yBase);
        pDrawer->hline_to(xEnd);
        pDrawer->end_path();

        //text-bottom (descender: green)
        LUnits yBottom = m_origin.y + m_size.height - m_halfLeading;
        pDrawer->begin_path();
        pDrawer->stroke(Color(0, 255, 0));
        pDrawer->stroke_width(10.0);
        pDrawer->move_to(xStart, yBottom);
        pDrawer->hline_to(xEnd);
        pDrawer->end_path();

        //middle line (magenta)
        URect rect = meter.bounding_rectangle('x');
        LUnits yMiddle = yBase - rect.height / 2.0f;
        pDrawer->begin_path();
        pDrawer->stroke(Color(255, 0, 255));
        pDrawer->stroke_width(10.0);
        pDrawer->fill(Color(0, 0, 0, 0));     //transparent black
        pDrawer->move_to(xStart, yMiddle);
        pDrawer->hline_to(xEnd);
        pDrawer->end_path();

        //shape bounding box (red)
        pDrawer->begin_path();
        pDrawer->stroke(Color(255, 0, 0));
        pDrawer->stroke_width(10.0);
        pDrawer->fill(Color(0, 0, 0, 0));     //transparent black
        pDrawer->move_to(m_origin.x, m_origin.y);
        pDrawer->hline_to(m_origin.x + m_size.width);
        pDrawer->vline_to(m_origin.y + m_size.height);
        pDrawer->hline_to(m_origin.x);
        pDrawer->vline_to(m_origin.y);
        pDrawer->end_path();
    }

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeWord::select_font()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_language,
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size(),
                      m_pStyle->is_bold(),
                      m_pStyle->is_italic() );
}



////---------------------------------------------------------------------------------------
////helper class to contain a line of text (to distribute text inside the box)
//class TextLine
//{
//public:
//    TextLine(std::string text, LUnits width, LUnits height)
//        : sText(text), uWidth(width), uHeight(height) {}
//    ~TextLine() {}
//
//	std::string		sText;
//    LUnits        uWidth;
//    LUnits        uHeight;
//    UPoint        uPos;     // text position (relative to top-left of rectangle)
//};



//=======================================================================================
// GmoShapeTextBox object implementation
//=======================================================================================
GmoShapeTextBox::GmoShapeTextBox(ImoObj* pCreatorImo, ShapeId idx, const string& text,
                                 const string& language, ImoStyle* pStyle,
                                 LibraryScope& libraryScope, const UPoint& pos,
                                 const USize& size, LUnits radius)
    : GmoShapeRectangle(pCreatorImo, GmoObj::k_shape_text_box, idx, pos, size, radius,
                        pStyle)
    , m_text(text)
    , m_language(language)
    , m_pStyle(pStyle)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
{
//    SetBorderStyle(nBorderStyle);
//
//    //measure text
//    pPaper->SetFont(*m_pFont);
//    pPaper->GetTextExtent(m_text, &m_uTextWidth, &m_uTextHeight);
//
//    //Adjust text within the box
//    ComputeTextPosition(pPaper);
//}
    //bounds
    select_font();
    TextMeter meter(m_libraryScope);
    m_size.width = meter.measure_width(text);
    m_size.height = meter.get_font_height();

    //position
    m_origin.x = pos.x;
    m_origin.y = pos.y - m_size.height;     //reference is at text bottom

    //color
    if (m_pStyle)
        m_color = m_pStyle->color();
}

//---------------------------------------------------------------------------------------
GmoShapeTextBox::~GmoShapeTextBox()
{
//    DeleteTextLines();      //delete text lines
}

////---------------------------------------------------------------------------------------
//void GmoShapeTextBox::DeleteTextLines()
//{
//    //delete text lines
//    std::list<TextLine*>::iterator it;
//    for (it = m_TextLines.begin(); it != m_TextLines.end(); ++it)
//        delete *it;
//    m_TextLines.clear();
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTextBox::ComputeTextPosition(lmPaper* pPaper)
//{
//    //Position the text within the box, splitting lines if necessary
//
//    //delete obsolete text arrangement
//    DeleteTextLines();
//
//    //compute box margin: the width of the 'l' letter
//    LUnits uxMargin, uyMargin;
//	pPaper->SetFont(*m_pFont);
//	pPaper->GetTextExtent(_T("l"), &uxMargin, &uyMargin);
//    uyMargin = uxMargin;
//
//    LUnits uBoxAreaWidth = m_uBoundsBottom.x - m_uBoundsTop.x - uxMargin - uxMargin;
//
//	//split text in lines
//    TextLine* pCurLine;
//    LUnits uxLine = uxMargin;
//    LUnits uyLine = uyMargin;
//	if (uBoxAreaWidth >= m_uTextWidth)
//    {
//        //the text fits in one line
//        pCurLine = LOMSE_NEW TextLine(m_text, m_uTextWidth, m_uTextHeight);
//    }
//    else
//	{
//		//we have to split the text. Loop to add chars until line full
//		LUnits uWidth, uHeight;
//        pCurLine = LOMSE_NEW TextLine(_T(""), 0.0f, m_uTextHeight);
//		int iC = 0;
//		LUnits uAvailable = uBoxAreaWidth;
//		while(iC < (int)m_text.Length())
//		{
//			const std::string ch = m_text.Mid(iC, 1);
//			pPaper->GetTextExtent(ch, &uWidth, &uHeight);
//			if (uAvailable < uWidth)
//            {
//                //line full. Save it and start a new line
//                m_TextLines.push_back(pCurLine);
//                uxLine += ApplyHAlign(uBoxAreaWidth, pCurLine->uWidth, m_nHAlign);
//                pCurLine->uPos = UPoint(uxLine, uyLine);
//                uyLine += pCurLine->uHeight;
//
//                pCurLine = LOMSE_NEW TextLine(_T(""), 0.0f, m_uTextHeight);
//                uAvailable = uBoxAreaWidth;
//            }
//
//			//add char to clipped text
//			uAvailable -= uWidth;
//            pCurLine->sText += ch;
//			iC++;
//		}
//        pPaper->GetTextExtent(pCurLine->sText, &(pCurLine->uWidth), &(pCurLine->uHeight));
//	}
//    m_TextLines.push_back(pCurLine);
//    uxLine += ApplyHAlign(uBoxAreaWidth, pCurLine->uWidth, m_nHAlign);
//    pCurLine->uPos = UPoint(uxLine, uyLine);
//}

////---------------------------------------------------------------------------------------
//LUnits GmoShapeTextBox::ApplyHAlign(LUnits uAvailableWidth, LUnits uLineWidth,
//                                     lmEHAlign nHAlign)
//{
//    //Returns x shift to apply for desired aligment
//
//    switch (nHAlign)
//    {
//        case lmHALIGN_DEFAULT:
//        case lmHALIGN_LEFT:
//            return 0.0f;
//
//        case lmHALIGN_RIGHT:
//            return uAvailableWidth - uLineWidth;
//
//        case lmHALIGN_JUSTIFY:
//            //TODO
//            return 0.0f;
//
//        case lmHALIGN_CENTER:
//            return (uAvailableWidth - uLineWidth) / 2.0f;
//
//        default:
//            wxASSERT(false);
//            return 0.0f;    //compiler happy
//    }
//}

//---------------------------------------------------------------------------------------
void GmoShapeTextBox::select_font()
{
    TextMeter meter(m_libraryScope);
    if (!m_pStyle)
        meter.select_font(m_language, "", "Liberation serif", 12.0);
    else
        meter.select_font(m_language,
                          m_pStyle->font_file(),
                          m_pStyle->font_name(),
                          m_pStyle->font_size(),
                          m_pStyle->is_bold(),
                          m_pStyle->is_italic() );
}

//---------------------------------------------------------------------------------------
void GmoShapeTextBox::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    GmoShapeRectangle::on_draw(pDrawer, opt);
    draw_text(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeTextBox::draw_text(Drawer* pDrawer, RenderOptions& opt)
{
//    //ensure text is properly split for current box size
//    ComputeTextPosition(pPaper);
//
//    pPaper->SetFont(*m_pFont);
//    pPaper->SetTextForeground(m_nTxtColor);
//
//    LUnits uySpace = m_uBoundsBottom.y - m_uBoundsTop.y;
//    std::list<TextLine*>::iterator it;
//    for (it = m_TextLines.begin(); it != m_TextLines.end(); ++it)
//    {
//        LUnits uySpaceLeft = uySpace - (*it)->uPos.y - (*it)->uHeight;
//        if (uySpaceLeft < 0.0f) break;
//        pPaper->DrawText((*it)->sText, (*it)->uPos.x + m_uBoundsTop.x,
//                         (*it)->uPos.y + m_uBoundsTop.y);
//    }
    select_font();
    pDrawer->set_text_color( determine_color_to_use(opt) );
    LUnits x = m_origin.x;
    LUnits y = m_origin.y + m_size.height;     //reference is at text bottom
    pDrawer->draw_text(x, y, m_text);

    //std::string str("¿This? is a test: Ñ € & abc. Ruso:Текст на кирилица");
    //pDrawer->draw_text(m_xPos, m_yPos, str);

    GmoSimpleShape::on_draw(pDrawer, opt);
}

////---------------------------------------------------------------------------------------
//void GmoShapeTextBox::SetFont(wxFont *pFont)
//{
//    m_pFont = pFont;
//}
//
////---------------------------------------------------------------------------------------
//wxBitmap* GmoShapeTextBox::OnBeginDrag(double rScale, wxDC* pDC)
//{
//	// A dragging operation is started. The view invokes this method to request the
//	// bitmap to be used as drag image. No other action is required.
//	// If no bitmap is returned drag is cancelled.
//	//
//	// So this method returns the bitmap to use with the drag image.
//
//
//    //as this is a shape defined by points: save all points position
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//        m_uSavePoint[i] = m_uPoint[i];
//
//    // allocate a bitmap whose size is that of the box area
//    // convert size to pixels
//    LUnits uySpace = m_uBoundsBottom.y - m_uBoundsTop.y;
//    int wD = (int)pDC->LogicalToDeviceXRel( m_uBoundsBottom.x - m_uBoundsTop.x );
//    int hD = (int)pDC->LogicalToDeviceYRel( uySpace );
//    wxBitmap bitmap(wD+2, hD+2);
//
//    // allocate a memory DC for drawing into a bitmap
//    wxMemoryDC dc2;
//    dc2.SelectObject(bitmap);
//    dc2.SetMapMode(lmDC_MODE);
//    dc2.SetUserScale(rScale, rScale);
//    dc2.SetFont(*m_pFont);
//
//    // draw onto the bitmap
//    dc2.SetBackground(* wxWHITE_BRUSH);
//    dc2.Clear();
//    dc2.SetBackgroundMode(wxTRANSPARENT);
//    dc2.SetPen(*wxBLACK_PEN);
//    dc2.DrawRectangle(0, 0, GetBounds().GetWidth(), GetBounds().GetHeight() );
//    dc2.SetTextForeground(g_pColors->ScoreSelected());
//
//    std::list<TextLine*>::iterator it;
//    for (it = m_TextLines.begin(); it != m_TextLines.end(); ++it)
//    {
//        LUnits uySpaceLeft = uySpace - (*it)->uPos.y - (*it)->uHeight;
//        if (uySpaceLeft < 0.0f) break;
//        dc2.DrawText((*it)->sText, (*it)->uPos.x, (*it)->uPos.y);
//    }
//    dc2.SelectObject(wxNullBitmap);
//
//    // Make the bitmap masked
//    wxImage image = bitmap.ConvertToImage();
//    image.SetMaskColour(255, 255, 255);
//    wxBitmap* pBitmap = LOMSE_NEW wxBitmap(image);
//
//    ////DBG -----------
//    //std::string sFileName = _T("GmoShapeTextBox.bmp");
//    //pBitmap->SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    return pBitmap;
//
//}
//



////========================================================================================
//// GmoShapeTitle object implementation
////========================================================================================
//////Simple text constructor
////GmoShapeTitle::GmoShapeTitle(lmScoreObj* pOwner, const std::string& sText, wxFont* pFont,
////                         lmPaper* pPaper, UPoint offset, std::string sName,
////				         bool fDraggable, wxColour color)
////    : GmoShapeRectangle(pOwner, offset.x, offset.y, 0.0f, 0.0f, 0.0f, color,
////                       sName, fDraggable, true)
////{
////    //measure text size
////    LUnits uWidth, uHeight;
////    pPaper->SetFont(*pFont);
////    pPaper->GetTextExtent(sText, &uWidth, &uHeight);
////
////    Create(sText, pFont, pPaper, lmBLOCK_ALIGN_LEFT, lmHALIGN_LEFT, lmVALIGN_MIDDLE,
////           offset.x, offset.y, offset.x + uWidth, offset.y + uHeight);
////}
//
////---------------------------------------------------------------------------------------
////block text constructor
//GmoShapeTitle::GmoShapeTitle(lmScoreObj* pOwner, const std::string& sText, wxFont* pFont,
//                         lmPaper* pPaper, lmEBlockAlign nBlockAlign,
//                         lmEHAlign nHAlign, lmEVAlign nVAlign,
//                         LUnits xLeft, LUnits yTop,
//                         LUnits xRight, LUnits yBottom,
//                         wxColour nColor, std::string sName,
//						 bool fDraggable)
//    : GmoShapeRectangle(pOwner, xLeft, yTop, xRight, yBottom, 0.0f, nColor,
//                       sName, fDraggable, true)
//{
//    Create(sText, pFont, pPaper, nBlockAlign, nHAlign, nVAlign, xLeft, yTop, xRight,
//           yBottom);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTitle::Create(const std::string& sText, wxFont* pFont, lmPaper* pPaper,
//                         lmEBlockAlign nBlockAlign, lmEHAlign nHAlign, lmEVAlign nVAlign,
//                         LUnits xLeft, LUnits yTop, LUnits xRight, LUnits yBottom)
//{
//    //The simplest constructor: just a text using a single font. No break lines
//
//	m_nType = eGMO_ShapeTextBlock;
//
//    m_text = sText;
//    m_pFont = pFont;
//    m_nBlockAlign = nBlockAlign;
//    m_nHAlign = nHAlign;
//    m_nVAlign = nVAlign;
//
//    //For now, behaviour wiil be that of a SimpleText object. Rectangle will be
//    //adjusted to text
//
//    // store boundling rectangle position and size
//    pPaper->SetFont(*m_pFont);
//    pPaper->GetTextExtent(m_text, &m_uTextWidth, &m_uTextHeight);
//
//    //compute rectangle bounds
//    ComputeBlockBounds(xLeft, yTop, xRight, yBottom);
//
//    //Position the text within the box
//    ComputeTextPosition(pPaper);
//
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTitle::ComputeBlockBounds(LUnits xLeft, LUnits yTop, LUnits xRight, LUnits yBottom)
//{
//    //compute rectangle bounds
//    LUnits uxLeft, uxRight;
//    ImoScore* pScore = m_pOwner->GetScore();
//    switch (m_nBlockAlign)
//    {
//        case lmBLOCK_ALIGN_DEFAULT:
//        case lmBLOCK_ALIGN_BOTH:
//            {
//                //xLeft and xRight on respective page margins
//                uxLeft = pScore->GetLeftMarginXPos();
//                uxRight = pScore->GetRightMarginXPos();
//            }
//            break;
//
//        case lmBLOCK_ALIGN_LEFT:
//            {
//                //xLeft on left page margin
//                uxLeft = pScore->GetLeftMarginXPos();
//                uxRight = xRight;
//            }
//            break;
//
//        case lmBLOCK_ALIGN_RIGHT:
//            {
//                //xRight on right page margin
//                uxLeft = xLeft;
//                uxRight = pScore->GetRightMarginXPos();
//            }
//            break;
//
//        case lmBLOCK_ALIGN_NONE:
//            {
//                //Floating block: xLeft and xRight set by user
//                uxLeft = xLeft;
//                uxRight = xRight;
//            }
//            break;
//
//        default:
//            wxASSERT(false);
//    }
//
//    m_uBoundsTop.x = uxLeft;
//    m_uBoundsTop.y = yTop;
//    m_uBoundsBottom.x = uxRight;
//    m_uBoundsBottom.y = m_uBoundsTop.y + m_uTextHeight;
//
//    // store selection rectangle position and size
//	m_uSelRect = GetBounds();
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTitle::ComputeTextPosition(lmPaper* pPaper)
//{
//    //Position the text within the box
//    LUnits uxLeft, uyTop;
//    LUnits uBoxAreaWidth = m_uBoundsBottom.x - m_uBoundsTop.x;
//
//	//clip text if longer than available space
//	m_sClippedText = m_text;
//    m_uClippedTextWidth = m_uTextWidth;
//    m_uClippedTextHeight = m_uTextHeight;
//	if (uBoxAreaWidth < m_uTextWidth)
//	{
//		//we have to cut the text. Loop to add chars until line full
//		pPaper->SetFont(*m_pFont);
//		LUnits uWidth, uHeight;
//		m_sClippedText = _T("");
//		int iC = 0;
//		LUnits uAvailable = uBoxAreaWidth;
//		while(iC < (int)m_text.Length())
//		{
//			const std::string ch = m_text.Mid(iC, 1);
//			pPaper->GetTextExtent(ch, &uWidth, &uHeight);
//			if (uAvailable < uWidth)
//				break;
//
//			//add char to clipped text
//			uAvailable -= uWidth;
//			m_sClippedText += ch;
//			iC++;
//		}
//		pPaper->GetTextExtent(m_sClippedText, &m_uClippedTextWidth, &m_uClippedTextHeight);
//	}
//
//
//    switch (m_nHAlign)
//    {
//        case lmHALIGN_DEFAULT:
//        case lmHALIGN_LEFT:
//            {
//                uxLeft = m_uBoundsTop.x;
//                uyTop = m_uBoundsTop.y;
//            }
//            break;
//
//        case lmHALIGN_RIGHT:
//            {
//                uxLeft = m_uBoundsBottom.x - m_uClippedTextWidth;
//                uyTop = m_uBoundsTop.y;
//            }
//            break;
//
//        case lmHALIGN_JUSTIFY:
//            {
//                //TODO
//                uxLeft = m_uBoundsTop.x;
//                uyTop = m_uBoundsTop.y;
//            }
//            break;
//
//        case lmHALIGN_CENTER:
//            {
//                uxLeft = m_uBoundsTop.x + (uBoxAreaWidth - m_uClippedTextWidth) / 2.0f;
//                uyTop = m_uBoundsTop.y;
//            }
//            break;
//
//        default:
//            wxASSERT(false);
//    }
//    m_uTextPos.x = uxLeft;
//    m_uTextPos.y = uyTop;
//
//    // store selection rectangle position and size
//	m_uSelRect.SetLeftTop(m_uTextPos);
//    m_uSelRect.SetWidth(m_uClippedTextWidth);
//    m_uSelRect.SetHeight(m_uClippedTextHeight);
//
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTitle::Render(lmPaper* pPaper, wxColour color)
//{
//    //ensure measures are ok
//    ComputeTextPosition(pPaper);
//
//    pPaper->SetFont(*m_pFont);
//    pPaper->SetTextForeground(color);
//    pPaper->DrawText(m_sClippedText, m_uTextPos.x, m_uTextPos.y);
//
//    GmoShapeRectangle::Render(pPaper, color);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTitle::SetFont(wxFont *pFont)
//{
//    m_pFont = pFont;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTitle::Shift(LUnits xIncr, LUnits yIncr)
//{
//    m_uTextPos.x += xIncr;
//    m_uTextPos.y += yIncr;
//
//    ShiftBoundsAndSelRec(xIncr, yIncr);
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((lmCompositeShape*)GetParentShape())->RecomputeBounds();
//}
//
////---------------------------------------------------------------------------------------
//wxBitmap* GmoShapeTitle::OnBeginDrag(double rScale, wxDC* pDC)
//{
//	// A dragging operation is started. The view invokes this method to request the
//	// bitmap to be used as drag image. No other action is required.
//	// If no bitmap is returned drag is cancelled.
//	//
//	// So this method returns the bitmap to use with the drag image.
//
//
//    // allocate a bitmap whose size is that of the box area
//    // convert size to pixels
//    int wD = (int)pDC->LogicalToDeviceXRel( m_uBoundsBottom.x - m_uBoundsTop.x );
//    int hD = (int)pDC->LogicalToDeviceYRel( m_uBoundsBottom.y - m_uBoundsTop.y );
//    wxBitmap bitmap(wD+2, hD+2);
//
//    // allocate a memory DC for drawing into a bitmap
//    wxMemoryDC dc2;
//    dc2.SelectObject(bitmap);
//    dc2.SetMapMode(lmDC_MODE);
//    dc2.SetUserScale(rScale, rScale);
//    dc2.SetFont(*m_pFont);
//
//    // draw onto the bitmap
//    dc2.SetBackground(* wxWHITE_BRUSH);
//    dc2.Clear();
//    dc2.SetBackgroundMode(wxTRANSPARENT);
//    dc2.SetTextForeground(g_pColors->ScoreSelected());
//    dc2.DrawText(m_sClippedText, m_uTextPos.x - m_uBoundsTop.x, m_uTextPos.y - m_uBoundsTop.y);
//    dc2.SelectObject(wxNullBitmap);
//
//    // Make the bitmap masked
//    wxImage image = bitmap.ConvertToImage();
//    image.SetMaskColour(255, 255, 255);
//    wxBitmap* pBitmap = LOMSE_NEW wxBitmap(image);
//
//    ////DBG -----------
//    //std::string sFileName = _T("GmoShapeTitle.bmp");
//    //pBitmap->SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    return pBitmap;
//
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeTitle::OnDrag(lmPaper* pPaper, const UPoint& uPos)
//{
//	// The view informs that the user continues dragging. We receive the new desired
//	// shape position and we must return the new allowed shape position.
//	//
//	// The default behaviour is to return the received position, so the view redraws
//	// the drag image at that position. No action must be performed by the shape on
//	// the score and score objects.
//	//
//	// The received new desired shape position is in logical units and referred to page
//	// origin. The returned new allowed shape position must also be in in logical units
//	// and referred to page origin.
//
//
//    // A TextBlock can only be vertically moved, unless it is a floating block
//    if (m_nBlockAlign == lmBLOCK_ALIGN_NONE)
//    {
//        //floating block
//        //TODO
//        return UPoint(uPos.x, uPos.y);
//    }
//    else
//    {
//        //move only in vertical
//        return UPoint(m_uBoundsTop.x, uPos.y);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTitle::OnEndDrag(lmPaper* pPaper, lmInteractor* pCanvas, const UPoint& uPos)
//{
//	// End drag. Receives the command processor associated to the view and the
//	// final position of the object (logical units referred to page origin).
//	// This method must validate/adjust final position and, if ok, it must
//	// send a move object command to the Interactor.
//
//	UPoint uFinalPos(uPos.x, uPos.y);
//    if (m_nBlockAlign == lmBLOCK_ALIGN_NONE)
//    {
//        //floating block
//        //TODO
//    }
//    else
//    {
//        //move only in vertical
//        uFinalPos.x = m_uBoundsTop.x;
//    }
//
//	//send a move object command to the Interactor
//	pCanvas->MoveObject(this, uFinalPos);
//}

}  //namespace lomse
