//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
                             LUnits thickness, Color color, int niente, LUnits radius,
                             LUnits yBaseline)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_wedge, idx, color)
    , m_thickness(thickness)
    , m_niente(niente)
    , m_radiusNiente(radius)
{
    save_points_and_compute_bounds(points);
    m_yBaseline = yBaseline - m_origin.y;
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

    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation(get_notation_id(), get_notation_class());

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

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeWedge::get_baseline_y() const
{
    return m_origin.y + m_yBaseline;
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
