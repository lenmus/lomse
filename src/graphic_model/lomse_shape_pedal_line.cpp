//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_shape_pedal_line.h"

#include "lomse_internal_model.h"
#include "lomse_gm_basic.h"

#include <cmath>

namespace lomse
{


//=======================================================================================
// GmoShapePedalLine implementation
//=======================================================================================
GmoShapePedalLine::GmoShapePedalLine(ImoObj* pCreatorImo, ShapeId idx, Color color)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_pedal_line, idx, color)
    , m_xLineStart(0.0f)
    , m_yLineStart(0.0f)
    , m_yLineEnd(0.0f)
    , m_uLineThick(0.0f)
    , m_fStartCorner(false)
    , m_fEndCorner(false)
{
}

//---------------------------------------------------------------------------------------
void GmoShapePedalLine::set_layout_data(LUnits xStart, LUnits xEnd, LUnits yStart,
                                    LUnits yEnd, LUnits uLineThick,
                                    bool fStartCorner, bool fEndCorner)
{
    URect bbox(xStart, min(yStart, yEnd), xEnd - xStart, abs(yStart - yEnd));

    if (!m_components.empty())
        bbox.Union(get_bounds());

    const LUnits xDiff = bbox.x - get_left();
    m_origin = bbox.get_top_left();
    m_size.width = bbox.get_width();
    m_size.height = bbox.get_height();

    //save points in relative coordinates
    m_xLineStart = xStart - m_origin.x;
    m_yLineStart = yStart - m_origin.y;
    m_yLineEnd = yEnd - m_origin.y;
    m_uLineThick = uLineThick;
    m_fStartCorner = fStartCorner;
    m_fEndCorner = fEndCorner;

    for (LineGap& gap : m_lineGaps)
    {
        gap.xStart -= xDiff;
        gap.xEnd -= xDiff;
    }
}

//---------------------------------------------------------------------------------------
void GmoShapePedalLine::add_line_gap(LUnits xStart, LUnits xEnd)
{
    //make the coordinates relative to the shape's origin
    xStart -= m_origin.x;
    xEnd -= m_origin.x;

    m_lineGaps.push_back({ xStart, xEnd });
}

//---------------------------------------------------------------------------------------
void GmoShapePedalLine::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);      //Color(255,0,0);

    LUnits xStart = m_xLineStart + m_origin.x;
    LUnits xEnd = m_origin.x + m_size.width;
    LUnits yStart = m_yLineStart + m_origin.y;
    LUnits yEnd = m_yLineEnd + m_origin.y;

    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation(get_notation_id(), get_notation_class());

    pDrawer->begin_path();
    pDrawer->fill_none();
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_uLineThick);

    //start corner, if needed
    if (m_fStartCorner) {
        pDrawer->move_to(xStart, yEnd);
        pDrawer->vline_to(yStart);
    } else {
        pDrawer->move_to(xStart, yStart);
    }

    //horizontal line
    for (const LineGap& gap : m_lineGaps)
    {
        pDrawer->hline_to(gap.xStart + m_origin.x);
        pDrawer->move_to(gap.xEnd + m_origin.x, yStart);
    }
    pDrawer->hline_to(xEnd);

    //end corner
    if (m_fEndCorner)
        pDrawer->vline_to(yEnd);      //add vertical stroke

    pDrawer->end_path();
    pDrawer->render();

    GmoCompositeShape::on_draw(pDrawer, opt);

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}

}  //namespace lomse
