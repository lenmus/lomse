//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#include "lomse_shape_wedge.h"

#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"

#include <cmath>   //abs

namespace lomse
{


//=======================================================================================
// GmoShapeWedge implementation
//=======================================================================================
GmoShapeWedge::GmoShapeWedge(ImoObj* pCreatorImo, ShapeId idx, UPoint points[],
                             LUnits thickness, Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_wedge, idx, color)
    , m_thickness(thickness)
    , m_fNiente(false)
{
    save_points(points);
    compute_bounds();
}

//---------------------------------------------------------------------------------------
GmoShapeWedge::~GmoShapeWedge()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeWedge::save_points(UPoint* points)
{
    for (int i=0; i < 4; i++)
        m_points[i] = *(points+i);
}

////---------------------------------------------------------------------------------------
//void GmoShapeWedge::set_layout_data(LUnits xStart, LUnits xEnd, LUnits yPos,
//                                           LUnits uBracketDistance, LUnits uJogLength,
//                                           LUnits uLineThick, LUnits uLeftSpaceToText,
//                                           LUnits xStaffLeft, LUnits xStaffRight,
//                                           GmoShapeBarline* pStart,
//                                           GmoShapeBarline* pEnd)
//{
//    //save data
//    m_pStartBarlineShape = pStart;
//    m_pStopBarlineShape = pEnd;
//    m_uBracketDistance = uBracketDistance;
//	m_uJogLength = uJogLength;
//    m_uLineThick = uLineThick;
//    m_uStaffLeft = xStaffLeft;
//    m_uStaffRight = xStaffRight;
//
//	//move text shape to its position
//    if (m_pShapeText)
//        m_pShapeText->set_origin(xStart + uLeftSpaceToText, yPos);
//
//    compute_bounds(xStart, xEnd, yPos);
//}

//---------------------------------------------------------------------------------------
void GmoShapeWedge::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);      //Color(255,0,0);

    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_thickness);
    pDrawer->move_to(m_points[0].x, m_points[0].y);
    pDrawer->line_to(m_points[1].x, m_points[1].y);
    pDrawer->move_to(m_points[2].x, m_points[2].y);
    pDrawer->line_to(m_points[3].x, m_points[3].y);

    pDrawer->end_path();
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeWedge::compute_bounds()
{
    m_origin.x = m_points[0].x;
    m_origin.y = min(m_points[0].y, m_points[1].y);
    m_size.width = abs(m_points[1].x - m_points[0].x);
    m_size.height = max( abs(m_points[1].y - m_points[0].y),
                         abs(m_points[3].y - m_points[2].y) );
}

//---------------------------------------------------------------------------------------
int GmoShapeWedge::get_num_handlers()
{
    return 4;
}

//---------------------------------------------------------------------------------------
UPoint GmoShapeWedge::get_handler_point(int UNUSED(i))
{
    //TODO
    return UPoint(0.0f, 0.0f);
}

//---------------------------------------------------------------------------------------
void GmoShapeWedge::on_handler_dragged(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}

//---------------------------------------------------------------------------------------
void GmoShapeWedge::on_end_of_handler_drag(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}


}  //namespace lomse
