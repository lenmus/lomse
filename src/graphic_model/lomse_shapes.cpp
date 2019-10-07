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

#include "lomse_shapes.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"
#include "lomse_glyphs.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"
#include "agg_trans_affine.h"


namespace lomse
{

//=======================================================================================
// GmoShapeGlyph object implementation
//=======================================================================================
GmoShapeGlyph::GmoShapeGlyph(ImoObj* pCreatorImo, int type, ShapeId idx,
                             unsigned int nGlyph, UPoint pos, Color color,
                             LibraryScope& libraryScope, double fontHeight)
    : GmoSimpleShape(pCreatorImo, type, idx, color)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
{
    m_glyph = m_libraryScope.get_glyphs_table()->glyph_code(nGlyph);
    compute_size_origin(fontHeight, pos);
}

//---------------------------------------------------------------------------------------
void GmoShapeGlyph::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    pDrawer->select_font("any",
                         m_libraryScope.get_music_font_file(),
                         m_libraryScope.get_music_font_name(),
                         m_fontHeight);
    pDrawer->set_text_color( determine_color_to_use(opt) );
    pDrawer->move_to(m_origin.x, m_origin.y);       //this line fixes issue #73 !!!
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
    meter.select_font("any",
                      m_libraryScope.get_music_font_file(),
                      m_libraryScope.get_music_font_name(),
                      m_fontHeight);
    URect bbox = meter.bounding_rectangle(m_glyph);

    m_origin.x = pos.x + bbox.x;
    m_origin.y = pos.y + bbox.y;
    m_size.width = bbox.width;
    m_size.height = bbox.height;

    m_shiftToDraw.width = -bbox.x;
    m_shiftToDraw.height = -bbox.y;
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
void GmoShapeImage::set_image(SpImage image)
{
    m_image = image;
}

//---------------------------------------------------------------------------------------
void GmoShapeImage::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    RenderingBuffer rbuf;
    rbuf.attach(m_image->get_buffer(), m_image->get_bitmap_width(),
                m_image->get_bitmap_height(), m_image->get_stride());
    pDrawer->draw_bitmap(rbuf, m_image->has_alpha(), 0, 0, m_image->get_bitmap_width(),
                          m_image->get_bitmap_height(), m_origin.x, m_origin.y,
                          m_origin.x + m_image->get_image_width(),
                          m_origin.y + m_image->get_image_height(),
                          k_quality_low);

    GmoSimpleShape::on_draw(pDrawer, opt);
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
                                        Color UNUSED(color), ELineEdge nEdge)
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
GmoShapeInvisible::GmoShapeInvisible(ImoObj* pCreatorImo, ShapeId idx, UPoint uPos,
                                     USize uSize)
	: GmoSimpleShape(pCreatorImo, GmoObj::k_shape_invisible, idx, Color(0,0,0))
{
    m_origin = uPos;
    m_size = uSize;
}

//---------------------------------------------------------------------------------------
void GmoShapeInvisible::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (opt.draw_anchor_objects)
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
                        ImoObj* pCreatorImo, int type, ShapeId idx,
                        const UPoint& pos, const USize& size,     //position and size
                        LUnits radius,              //for rounded corners
                        ImoStyle* UNUSED(pStyle)    //for line style & background color
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

//From ButtonCtrl::on_draw(), in lomse_button_ctrl.cpp
//
//    Color strokeColor = Color(128, 128, 128);
//    Color bgColor = m_normalColor;
//
//    //draw background and border
//    pDrawer->begin_path();
//    pDrawer->fill( m_normalColor );
//    pDrawer->stroke( strokeColor );
//    pDrawer->stroke_width(15.0);
//    Color white(255, 255, 255);
//    Color dark(bgColor);
//    dark.a = 45;
//    Color light(dark);
//    light = light.gradient(white, 0.2);
//    pDrawer->gradient_color(white, 0.0, 0.1);
//    pDrawer->gradient_color(white, dark, 0.1, 0.7);
//    pDrawer->gradient_color(dark, light, 0.7, 1.0);
//    pDrawer->fill_linear_gradient(m_pos.x, m_pos.y,
//                                  m_pos.x, m_pos.y + m_height);
//    pDrawer->rect(m_pos, USize(m_width, m_height), 100.0f);
//    pDrawer->end_path();
//
//    //draw text
//    select_font();
//    center_text();
//    pDrawer->set_text_color( textColor );
//    LUnits x = m_pos.x + m_xLabel;
//    LUnits y = m_pos.y + m_yLabel;
//    pDrawer->draw_text(x, y, m_label);
}



//=======================================================================================
// GmoShapeStem implementation: a vertical line
//=======================================================================================
GmoShapeStem::GmoShapeStem(ImoObj* pCreatorImo, LUnits xPos, LUnits yStart,
                           LUnits uExtraLength, LUnits yEnd, bool fStemDown,
                           LUnits uWidth, Color color)
	: GmoShapeSimpleLine(pCreatorImo, GmoObj::k_shape_stem, xPos, yStart, xPos, yEnd,
                         uWidth, 0.0f, color, k_edge_horizontal)
    , VoiceRelatedShape()
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



//=======================================================================================
// GmoShapeDebug
//=======================================================================================
GmoShapeDebug::GmoShapeDebug(Color color, UPoint uPos, USize uSize)
	: GmoSimpleShape(nullptr, GmoObj::k_shape_debug, 0, color)
    , m_nContour(0)
{
    rewind();

    m_origin = uPos;
    m_size = uSize;
}

//---------------------------------------------------------------------------------------
void GmoShapeDebug::add_vertex(Vertex& vertex)
{
    m_vertices.push_back(vertex);
}

//---------------------------------------------------------------------------------------
void GmoShapeDebug::add_vertex(char cmd, LUnits x, LUnits y)
{
    unsigned aggCmd = 0;
    switch(cmd)
    {
        case 'M':   aggCmd = agg::path_cmd_move_to;     break;
        case 'L':   aggCmd = agg::path_cmd_line_to;     break;
        case 'Z':   aggCmd = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;     break; //close polygon

        default:
        {
            LOMSE_LOG_ERROR("Invalid Command %c. Ignored.", cmd);
            return;
        }
    }
//    dbgLogger << "GmoShapeDebug::add_vertex(). cmd=" << cmd << ", x=" << x << ", y=" << y << endl;
    Vertex v = { x, y, aggCmd};
    m_vertices.push_back(v);
}

//---------------------------------------------------------------------------------------
void GmoShapeDebug::close_vertex_list()
{
    Vertex v = { 0.0f, 0.0f, agg::path_cmd_stop};
    m_vertices.push_back(v);
    rewind();
}

//---------------------------------------------------------------------------------------
void GmoShapeDebug::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    //set_affine_transform();

    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeDebug::rewind(int UNUSED(pathId))
{
    m_it=m_vertices.begin();
    m_nContour = 0;
}

//---------------------------------------------------------------------------------------
unsigned GmoShapeDebug::vertex(double* px, double* py)
{
	if (m_it == m_vertices.end())
		return agg::path_cmd_stop;

	*px = (*m_it).ux_coord;
	*py = (*m_it).uy_coord;
//	m_trans.transform(px, py);

	unsigned cmd = (*m_it).cmd;
	++m_it;
	return cmd;
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
//    , m_pControl((wxWindow*)nullptr)
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
