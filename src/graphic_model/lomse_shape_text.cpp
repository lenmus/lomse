//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    if (!m_pStyle)
        pDrawer->select_font(m_language, "", "Liberation serif", 12.0);
    else
        pDrawer->select_font(m_language,
                          m_pStyle->font_file(),
                          m_pStyle->font_name(),
                          m_pStyle->font_size(),
                          m_pStyle->is_bold(),
                          m_pStyle->is_italic() );

    //AWARE: the selected font is stored in library scope. Thus the next TextMeter
    //       will use the font selected in the drawer
    TextMeter meter(m_libraryScope);

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
    select_font();
    TextMeter meter(m_libraryScope);
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

    //select_font
    if (!m_pStyle)
        pDrawer->select_font(m_language, "", "Liberation serif", 12.0);
    else
        pDrawer->select_font(m_language,
                          m_pStyle->font_file(),
                          m_pStyle->font_name(),
                          m_pStyle->font_size(),
                          m_pStyle->is_bold(),
                          m_pStyle->is_italic() );

    //AWARE: the selected font is stored in library scope. Thus any TextMeter
    //       created after this point will use the font selected in the drawer

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


////=======================================================================================
//// GmoShapeLyricText object implementation
////=======================================================================================
//void GmoShapeLyricText::on_draw(Drawer* pDrawer, RenderOptions& opt)
//{
//}


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
    pDrawer->start_composite_notation(get_notation_id(), get_notation_class());
    GmoShapeRectangle::on_draw(pDrawer, opt);
    draw_text(pDrawer, opt);
    pDrawer->end_composite_notation();
}

//---------------------------------------------------------------------------------------
void GmoShapeTextBox::draw_text(Drawer* pDrawer, RenderOptions& opt)
{
    select_font();
    pDrawer->set_text_color( determine_color_to_use(opt) );
    LUnits x = m_origin.x;
    LUnits y = m_origin.y + m_size.height;     //reference is at text bottom
    pDrawer->draw_text(x, y, m_text);

    //std::string str("¿This? is a test: Ñ € & abc. Ruso:Текст на кирилица");
    //pDrawer->draw_text(m_xPos, m_yPos, str);

    GmoSimpleShape::on_draw(pDrawer, opt);
}


}  //namespace lomse
