//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

    const int xDiff = bbox.x - get_left();
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
}

}  //namespace lomse
