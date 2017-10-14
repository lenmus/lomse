//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2017. All rights reserved.
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

#include "lomse_shape_volta_bracket.h"

#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"
#include "lomse_shape_barline.h"


namespace lomse
{


//=======================================================================================
// GmoShapeVoltaBracket implementation
//=======================================================================================
GmoShapeVoltaBracket::GmoShapeVoltaBracket(ImoObj* pCreatorImo, ShapeId idx, Color color)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_volta_bracket, idx, color)
    , m_pStartBarlineShape(NULL)
    , m_pStopBarlineShape(NULL)
    , m_uLineThick(0.0f)
    , m_uStaffLeft(0.0f)
    , m_pShapeText(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShapeVoltaBracket::~GmoShapeVoltaBracket()
{
}

//void GmoShapeTuplet::compute_horizontal_position()
//{
//    //determine x start/end coordinates
//
//    //AWARE: It is simpler to do this here than in the tuplets engraver because
//    //when the tuplet is engraved the system is not justified and, therefore,
//    //the notes will be moved.
//
//    if (m_pStartNR->is_shape_rest())
//        m_uxStart = m_pStartNR->get_left();
//    else
//    {
//        GmoShapeNote* pStartNote = static_cast<GmoShapeNote*>(m_pStartNR);
//        m_uxStart = pStartNote->get_notehead_left();
//    }
//
//    if (m_pEndNR->is_shape_rest())
//        m_uxEnd = m_pEndNR->get_right();
//    else
//    {
//        GmoShapeNote* pEndNote = static_cast<GmoShapeNote*>(m_pEndNR);
//        m_uxEnd = pEndNote->get_notehead_right();
//    }
//}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::add_label(GmoShapeText* pShape)
{
    m_pShapeText = pShape;
    add(pShape);
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::set_layout_data(LUnits xStart, LUnits xEnd, LUnits yPos,
                                           LUnits yJog, LUnits uLineThick,
                                           LUnits uSpaceToText, LUnits xStaffLeft,
                                           GmoShapeBarline* pStart,
                                           GmoShapeBarline* pEnd)
{
    m_pStartBarlineShape = pStart;
    m_pStopBarlineShape = pEnd;

    m_uxStart = xStart;
    m_uxEnd = xEnd;
    m_yPos = yPos;
	m_yJog = yPos + yJog;
    m_xText = xStart + uSpaceToText;
    m_yText = yPos + uSpaceToText;
    m_uLineThick = uLineThick;
    m_uStaffLeft = xStaffLeft;
    //m_uTextWidth;

	//text position
	LUnits xText = 0.0f;
	LUnits yText = 0.0f;
	LUnits uTextWidth = 0.0f;
	LUnits uTextHeight = 0.0f;
    if (m_pShapeText)
    {
        uTextWidth = m_pShapeText->get_width();
        uTextHeight = 1.33f * m_pShapeText->get_height();
            //1.33 accounts for the fact that there is some space on top of the
            //text glyph

        //determine text x position
        xText = m_uxStart + 80; //+ m_uSpaceToText;

        //determine text y position
        yText = m_yPos - 700;

        //move text shape to its position
        m_pShapeText->set_origin(xText, yText);
    }

    compute_bounds();
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);      //Color(255,0,0);

    LUnits xStart = m_pStartBarlineShape->get_x_right_line();
    LUnits xEnd = m_pStopBarlineShape->get_x_left_line();
    LUnits yTop;
    if (xEnd < xStart)
    {
        xStart = m_uStaffLeft;
        yTop = m_pStopBarlineShape->get_top() - 600;
    }
    else
        yTop = m_pStartBarlineShape->get_top() - 600;

    LUnits yBottom = yTop + 400;

    //start jog
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->line(xStart, yBottom, xStart, yTop, m_uLineThick, k_edge_normal);
    pDrawer->end_path();

    //horizontal line
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->line(xStart, yTop, xEnd, yTop, m_uLineThick, k_edge_normal);
    pDrawer->end_path();

    //end jog
    //if (m_fStopJog)
        pDrawer->begin_path();
        pDrawer->fill(color);
        pDrawer->line(xEnd, yTop, xEnd, yBottom, m_uLineThick, k_edge_normal);

        pDrawer->end_path();

    pDrawer->render();

    GmoCompositeShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::compute_bounds()
{
    m_origin.x = m_uxStart;
    m_size.width = abs(m_uxEnd - m_uxStart);

//    if (m_pShapeText)
//    {
//        m_size.height = m_pShapeText->get_height();
//        if (m_fAbove)
//            m_origin.y = min(m_uyStart, min(m_uyEnd, m_yNumber));
//        else
//            m_origin.y = min(m_yEndBorder, min(m_yStartBorder, m_yNumber));
//    }
//    else
    {
        m_size.height = 400;
        m_origin.y = m_yPos - 650;
    }
}

//---------------------------------------------------------------------------------------
int GmoShapeVoltaBracket::get_num_handlers()
{
    return 4;
}

//---------------------------------------------------------------------------------------
UPoint GmoShapeVoltaBracket::get_handler_point(int i)
{
//    return m_points[i] + m_origin;
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::on_handler_dragged(int iHandler, UPoint newPos)
{
//    m_points[iHandler] = newPos;
//    compute_vertices();
//    compute_bounds();
//    make_points_and_vertices_relative_to_origin();
}

//---------------------------------------------------------------------------------------
void GmoShapeVoltaBracket::on_end_of_handler_drag(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}


}  //namespace lomse
