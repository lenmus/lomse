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
                             LUnits thickness, Color color, int niente, LUnits radius)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_wedge, idx, color)
    , m_thickness(thickness)
    , m_niente(niente)
    , m_radiusNiente(radius)
{
    save_points_and_compute_bounds(points);
}

//---------------------------------------------------------------------------------------
GmoShapeWedge::~GmoShapeWedge()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeWedge::save_points_and_compute_bounds(UPoint* points)
{
    m_origin.x = points[0].x;
    m_origin.y = min(points[0].y, points[1].y);
    m_size.width = abs(points[1].x - points[0].x);
    m_size.height = max( abs(points[2].y - points[0].y),
                         abs(points[1].y - points[3].y) );

    m_xTopStart = points[0].x - m_origin.x;
    m_yTopStart = points[0].y - m_origin.y;
    m_xTopEnd = points[1].x - m_origin.x;
    m_yTopEnd = points[1].y - m_origin.y;
    m_xBottomStart = points[2].x - m_origin.x;
    m_yBottomStart = points[2].y - m_origin.y;
    m_xBottomEnd = points[3].x - m_origin.x;
    m_yBottomEnd = points[3].y - m_origin.y;
}

//---------------------------------------------------------------------------------------
void GmoShapeWedge::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);

    pDrawer->begin_path();
    pDrawer->fill_none();
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_thickness);

    //draw lines
    pDrawer->move_to(m_xTopStart + m_origin.x, m_yTopStart + m_origin.y);
    pDrawer->line_to(m_xTopEnd + m_origin.x, m_yTopEnd + m_origin.y);
    pDrawer->move_to(m_xBottomStart + m_origin.x, m_yBottomStart + m_origin.y);
    pDrawer->line_to(m_xBottomEnd + m_origin.x, m_yBottomEnd + m_origin.y);

    //draw niente circle
    if (m_niente == k_niente_at_start)
        pDrawer->circle(m_xTopStart + m_origin.x - m_radiusNiente, m_yTopStart + m_origin.y, m_radiusNiente);
    else if (m_niente == k_niente_at_end)
        pDrawer->circle(m_xTopEnd + m_origin.x + m_radiusNiente, m_yTopEnd + m_origin.y, m_radiusNiente);

    pDrawer->end_path();
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);
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
