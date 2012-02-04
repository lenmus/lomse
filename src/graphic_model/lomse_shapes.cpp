//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include "lomse_shapes.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"
#include "lomse_glyphs.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"

namespace lomse
{

//=======================================================================================
// GmoShapeGlyph object implementation
//=======================================================================================
GmoShapeGlyph::GmoShapeGlyph(ImoObj* pCreatorImo, int type, int idx, unsigned int nGlyph,
                             UPoint pos, Color color, LibraryScope& libraryScope,
                             double fontHeight)
    : GmoSimpleShape(pCreatorImo, type, idx, color)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
{
    m_glyph = glyphs_lmbasic2[nGlyph].GlyphChar;
    compute_size_origin(fontHeight, pos);
}

//---------------------------------------------------------------------------------------
void GmoShapeGlyph::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    pDrawer->select_font("LenMus basic", m_fontHeight);
    pDrawer->set_text_color( determine_color_to_use(opt) );
    LUnits x = m_shiftToDraw.width + m_origin.x;
    LUnits y = m_shiftToDraw.height + m_origin.y;
    pDrawer->draw_glyph(x, y, m_glyph);

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeGlyph::compute_size_origin(double fontHeight, UPoint pos)
{
    m_fontHeight = fontHeight;

    TextMeter meter(m_libraryScope);
    meter.select_font("LenMus basic", m_fontHeight);
    URect bbox = meter.bounding_rectangle(m_glyph);

    m_origin.x = pos.x + bbox.x;
    m_origin.y = pos.y + bbox.y;
    m_size.width = bbox.width;
    m_size.height = bbox.height;

    m_shiftToDraw.width = -bbox.x;
    m_shiftToDraw.height = -bbox.y;
}



//=======================================================================================
// GmoShapeClef
//=======================================================================================
GmoShapeClef::GmoShapeClef(ImoObj* pCreatorImo, int idx, int nGlyph, UPoint pos,
                           Color color, LibraryScope& libraryScope, double fontSize)
    : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_clef, idx, nGlyph, pos, color,
                    libraryScope, fontSize )
{
}



//=======================================================================================
// GmoShapeButton implementation: a clickable button
//=======================================================================================
GmoShapeButton::GmoShapeButton(ImoObj* pCreatorImo, UPoint pos, USize size,
                               LibraryScope& libraryScope)
	: GmoSimpleShape(pCreatorImo, GmoObj::k_shape_button, 0, Color(0,0,0))
	, m_libraryScope(libraryScope)
{
    m_origin = pos;
    m_size = size;
    m_pButton = dynamic_cast<ImoButton*>( pCreatorImo );
    m_bgColor = m_pButton->get_bg_color();

    center_text();
}

//---------------------------------------------------------------------------------------
void GmoShapeButton::center_text()
{
    select_font();
    TextMeter meter(m_libraryScope);
    LUnits height = meter.get_font_height();
    LUnits width = meter.measure_width(m_pButton->get_label());

    m_xLabel = (m_size.width - width) / 2.0f;
    m_yLabel = (m_size.height + height) / 2.0f;     //reference is at text bottom
}

//---------------------------------------------------------------------------------------
void GmoShapeButton::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (!m_pButton->is_visible())
        return;

    Color textColor = m_pButton->is_enabled() ? Color(0, 0, 0) : Color(128, 128, 128);
    Color strokeColor = Color(128, 128, 128);

    //draw button gradient and border
    pDrawer->begin_path();
    pDrawer->fill( m_bgColor );
    pDrawer->stroke( strokeColor );
    pDrawer->stroke_width(15.0);
    Color white(255, 255, 255);
    Color dark(m_bgColor);    //dark(210, 210, 210);      //0.82
    //dark.a = 45;
    Color light(dark);      //light(230, 230, 230);     //0.90
    light = light.gradient(white, 0.2);
    pDrawer->gradient_color(white, 0.0, 0.1);
    pDrawer->gradient_color(white, dark, 0.1, 0.7);
    pDrawer->gradient_color(dark, light, 0.7, 1.0);
    pDrawer->fill_linear_gradient(m_origin.x, m_origin.y,
                                  m_origin.x, m_origin.y + m_size.height);
    pDrawer->rect(m_origin, m_size, 100.0f);
    pDrawer->end_path();

    //draw text
    select_font();
    pDrawer->set_text_color( textColor );   //determine_color_to_use(opt) );
    LUnits x = m_origin.x + m_xLabel;
    LUnits y = m_origin.y + m_yLabel;
    pDrawer->draw_text(x, y, m_pButton->get_label());

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeButton::select_font()
{
    ImoStyle* pStyle = m_pButton->get_style();
    TextMeter meter(m_libraryScope);
    meter.select_font(pStyle->get_string_property(ImoStyle::k_font_name),
                      pStyle->get_float_property(ImoStyle::k_font_size),
                      pStyle->is_bold(),
                      pStyle->is_italic() );
}

//---------------------------------------------------------------------------------------
void GmoShapeButton::change_color(Color color)
{
    m_bgColor = color;
    set_dirty(true);
}


//=======================================================================================
// GmoShapeImage implementation: a bitmap image
//=======================================================================================
GmoShapeImage::GmoShapeImage(ImoObj* pCreatorImo, SpImage image, UPoint pos, USize size)
	: GmoSimpleShape(pCreatorImo, GmoObj::k_shape_image, 0, Color(0,0,0))
	, m_image(image)
{
    m_origin = pos;
    m_size = size;
}

//---------------------------------------------------------------------------------------
void GmoShapeImage::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
//    if (!m_pImage->is_visible())
//        return;

    RenderingBuffer rbuf;
    rbuf.attach(m_image->get_buffer(), m_image->get_bitmap_width(),
                m_image->get_bitmap_height(), m_image->get_stride());
    //pDrawer->copy_bitmap(rbuf, m_origin);
    pDrawer->draw_bitmap(rbuf, m_image->has_alpha(), 0, 0, m_image->get_bitmap_width(),
                          m_image->get_bitmap_height(), m_origin.x, m_origin.y,
                          m_origin.x + m_image->get_image_width(),
                          m_origin.y + m_image->get_image_height(),
                          k_quality_low);

    GmoSimpleShape::on_draw(pDrawer, opt);
}



//=======================================================================================
// GmoShapeFermata
//=======================================================================================
GmoShapeFermata::GmoShapeFermata(ImoObj* pCreatorImo, int idx, int nGlyph, UPoint pos,
                           Color color, LibraryScope& libraryScope, double fontSize)
    : GmoShapeGlyph(pCreatorImo, GmoObj::k_shape_fermata, idx, nGlyph, pos, color,
                    libraryScope, fontSize )
{
}




//=======================================================================================
// GmoShapeSimpleLine implementation
//=======================================================================================
GmoShapeSimpleLine::GmoShapeSimpleLine(ImoObj* pCreatorImo, int type,
                                       LUnits xStart, LUnits yStart, LUnits xEnd,
                                       LUnits yEnd, LUnits uWidth,
                                       LUnits uBoundsExtraWidth, Color color,
                                       ELineEdge nEdge)
    : GmoSimpleShape(pCreatorImo, type, 0, color)
{
    set_new_values(xStart, yStart, xEnd, yEnd, uWidth, uBoundsExtraWidth, color, nEdge);
}

//---------------------------------------------------------------------------------------
void GmoShapeSimpleLine::set_new_values(LUnits xStart, LUnits yStart,
                                        LUnits xEnd, LUnits yEnd,
                                        LUnits uWidth, LUnits uBoundsExtraWidth,
                                        Color color, ELineEdge nEdge)
{
    m_uWidth = uWidth;
	m_uBoundsExtraWidth = uBoundsExtraWidth;
	m_nEdge = nEdge;

///*
//	//TODO
//    // if line is neither vertical nor horizontal, should we use a strait rectangle or a
//    // leaned rectangle sorrounding the line?
//
//    //width of rectangle = width of line + 2 pixels
//    uWidth += 2.0 / g_r;
//
//    //line angle
//    double alpha = atan((yEnd - yStart) / (xEnd - xStart));
//
//    //boundling rectangle
//    {
//    LUnits uIncrX = (LUnits)( (uWidth * sin(alpha)) / 2.0 );
//    LUnits uIncrY = (LUnits)( (uWidth * cos(alpha)) / 2.0 );
//    UPoint uPoints[] = {
//        UPoint(xStart+uIncrX, yStart-uIncrY),
//        UPoint(xStart-uIncrX, yStart+uIncrY),
//        UPoint(xEnd-uIncrX, yEnd+uIncrY),
//        UPoint(xEnd+uIncrX, yEnd-uIncrY)
//    };
//    SolidPolygon(4, uPoints, color);
//*/

	//TODO: For now it is assumed that the line is either vertical or horizontal
	if (xStart == xEnd)
	{
		//vertical line
		m_origin.x = xStart;    //- uWidth / 2.0f;
		m_origin.y = yStart;
        m_size.width = uWidth + uBoundsExtraWidth;
        m_size.height = yEnd - yStart + uBoundsExtraWidth;
	}
	else
	{
		//Horizontal line
		m_origin.x = xStart;
		m_origin.y = yStart;    // - uWidth / 2.0f;
		m_size.width = xEnd - xStart + uBoundsExtraWidth;
        m_size.height = uWidth + uBoundsExtraWidth;
	}

//	NormaliceBoundsRectangle();
//
//    // store selection rectangle position and size
//	m_uSelRect = GetBounds();

}

//---------------------------------------------------------------------------------------
void GmoShapeSimpleLine::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_uWidth);
    pDrawer->move_to(m_origin.x + m_uWidth / 2.0f, m_origin.y);
    pDrawer->line_to(m_origin.x + m_uWidth / 2.0f, m_origin.y + m_size.height);
    pDrawer->end_path();
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);
}



//=======================================================================================
// GmoShapeInvisible
//=======================================================================================
GmoShapeInvisible::GmoShapeInvisible(ImoObj* pCreatorImo, int idx, UPoint uPos,
                                     USize uSize)
	: GmoSimpleShape(pCreatorImo, GmoObj::k_shape_invisible, idx, Color(0,0,0))
{
    m_origin = uPos;
    m_size = uSize;
}

//---------------------------------------------------------------------------------------
void GmoShapeInvisible::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (opt.draw_anchors)
    {
        pDrawer->begin_path();
        pDrawer->fill(Color(255, 0, 0, 32));    //light red, transparent
        pDrawer->stroke(Color(255, 0, 0));
        pDrawer->stroke_width(15.0);
        pDrawer->move_to(m_origin.x, m_origin.y);
        pDrawer->hline_to(m_origin.x + m_size.width + 10.0f);
        pDrawer->vline_to(m_origin.y + m_size.height);
        pDrawer->hline_to(m_origin.x);
        pDrawer->vline_to(m_origin.y);
        pDrawer->end_path();
    }
    GmoShape::on_draw(pDrawer, opt);
}




//=======================================================================================
// GmoShapeRectangle: a rectangle with optional rounded corners
//=======================================================================================
GmoShapeRectangle::GmoShapeRectangle(
                        ImoObj* pCreatorImo, int type, int idx,
                        const UPoint& pos, const USize& size,     //position and size
                        LUnits radius,              //for rounded corners
                        ImoStyle* pStyle            //for line style & background color
                     )
	: GmoSimpleShape(pCreatorImo, type, idx, Color(0,0,0))
	, m_radius(radius)
{
    m_origin = pos;
    m_size = size;
}

//---------------------------------------------------------------------------------------
void GmoShapeRectangle::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    pDrawer->begin_path();
    pDrawer->fill(Color(0, 0, 0, 0));
    pDrawer->stroke(Color(0, 0, 0));
    pDrawer->stroke_width(15.0);
    pDrawer->rect(m_origin, m_size, m_radius);
    pDrawer->end_path();

    GmoSimpleShape::on_draw(pDrawer, opt);
}



//=======================================================================================
// GmoShapeStem implementation: a vertical line
//=======================================================================================
GmoShapeStem::GmoShapeStem(ImoObj* pCreatorImo, LUnits xPos, LUnits yStart,
                           LUnits uExtraLength, LUnits yEnd, bool fStemDown,
                           LUnits uWidth, Color color)
	: GmoShapeSimpleLine(pCreatorImo, GmoObj::k_shape_stem, xPos, yStart, xPos, yEnd,
                         uWidth, 0.0f, color, k_edge_horizontal)
	, m_fStemDown(fStemDown)
    , m_uExtraLength(uExtraLength)
{
}

//---------------------------------------------------------------------------------------
void GmoShapeStem::change_length(LUnits length)
{
    LUnits increment = length - m_size.height;
    if (increment != 0.0f)
    {
        if (m_fStemDown)
            adjust(m_origin.x, m_origin.y, length, m_fStemDown);
        else
            adjust(m_origin.x, m_origin.y - increment, length, m_fStemDown);
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeStem::adjust(LUnits xLeft, LUnits yTop, LUnits height, bool fStemDown)
{
	m_fStemDown = fStemDown;
	set_new_values(xLeft, yTop, xLeft, yTop+height, m_uWidth, m_uBoundsExtraWidth,
                   m_color, m_nEdge);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeStem::get_y_note()
{
    if (is_stem_down())
        return get_top();
    else
        return get_bottom();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeStem::get_y_flag()
{
    if (is_stem_down())
        return get_bottom();
    else
        return get_top();
}

//---------------------------------------------------------------------------------------
void GmoShapeStem::set_stem_up(LUnits xRight, LUnits yNote)
{
	m_fStemDown = false;
    m_origin.x = xRight - m_uWidth;
    m_origin.y = yNote - m_size.height;
}

//---------------------------------------------------------------------------------------
void GmoShapeStem::set_stem_down(LUnits xLeft, LUnits yNote)
{
	m_fStemDown = true;
    m_origin.x = xLeft;
    m_origin.y = yNote;
}



////---------------------------------------------------------------------------------------
//// GmoShapeFiguredBass object implementation: a composite shape that can have
//// attached shapes.
////---------------------------------------------------------------------------------------
//void GmoShapeFiguredBass::Shift(LUnits uxIncr, LUnits uyIncr)
//{
//	lmCompositeShape::Shift(uxIncr, uyIncr);
//	InformAttachedShapes(uxIncr, uyIncr, lmSHIFT_EVENT);
//}
//
//
//
////---------------------------------------------------------------------------------------
//// GmoShapeWindow object implementation: an auxiliary shape to embbed any wxWindow
////  (Button, TextCtrol, etc.) on the score
////---------------------------------------------------------------------------------------
//GmoShapeWindow::GmoShapeWindow(GmoBox* owner, int nShapeIdx,
//                  //position and size
//                  LUnits uxLeft, LUnits uyTop, LUnits uxRight, LUnits uyBottom,
//                  //border
//                  LUnits uBorderWidth, Color nBorderColor,
//                  //content
//                  Color nBgColor,
//                  //other
//                  wxString sName,
//				  bool fDraggable, bool fSelectable, bool fVisible)
//    : GmoShapeRectangle(pOwner, uxLeft, uyTop, uxRight, uyBottom, uBorderWidth,
//                       nBorderColor, nBgColor, nShapeIdx, sName,
//                       fDraggable, fSelectable, fVisible)
//    , m_pControl((wxWindow*)NULL)
//{
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeWindow::on_draw(Drawer* pDrawer, RenderOptions& opt)
//{
//    lmBoxPage* pBPage = this->GetOwnerBoxPage();
//    wxWindow* pWindow = pBPage->GetRenderWindow();
//    wxPoint& vOffset = pBPage->GetRenderWindowOffset();
//
//    wxPoint pos(pPaper->LogicalToDeviceX(m_uBoundsTop.x) + vOffset.x,
//                pPaper->LogicalToDeviceY(m_uBoundsTop.y) + vOffset.y );
//    wxSize size(pPaper->LogicalToDeviceX(GetBounds().GetWidth()),
//                pPaper->LogicalToDeviceX(GetBounds().GetHeight()) );
//
//    m_pControl =
//        new wxTextCtrl(pWindow, wxID_ANY,
//                       _T("This is a text using a wxTextCtrl window!"),
//                       pos, size );
//}


}  //namespace lomse
