//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_shape_octave_shift.h"

#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"

#include <cmath>   //abs

namespace lomse
{


//=======================================================================================
// GmoShapeOctaveShift implementation
//=======================================================================================
GmoShapeOctaveShift::GmoShapeOctaveShift(ImoObj* pCreatorImo, ShapeId idx, Color color)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_octave_shift, idx, color)
    , m_fTwoLines(false)
    , m_fEndCorner(true)
    , m_xLineStart(0.0f)
    , m_yLineStart(0.0f)
    , m_yLineEnd(0.0f)
    , m_uLineThick(0.0f)
{
}

//---------------------------------------------------------------------------------------
GmoShapeOctaveShift::~GmoShapeOctaveShift()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeOctaveShift::set_layout_data(LUnits xStart, LUnits xEnd, LUnits yStart,
                                          LUnits yEnd, LUnits uLineThick, bool fEndCorner)
{
    //save data
    m_xLineStart = xStart - m_origin.x;
    m_yLineStart = yStart - m_origin.y;
    m_yLineEnd = yEnd - m_origin.y;
    m_uLineThick = uLineThick;
    m_fEndCorner = fEndCorner;

    //fix bounds
    m_size.width = abs(xEnd - m_origin.x);
}

//---------------------------------------------------------------------------------------
void GmoShapeOctaveShift::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);      //Color(255,0,0);

    LUnits dxLine = 100.0f;
    LUnits dxSpace = 100.0f;

    LUnits xStart = m_xLineStart + m_origin.x;
    LUnits xEnd = m_origin.x + m_size.width;
    LUnits yStart = m_yLineStart + m_origin.y;
    LUnits yEnd = m_yLineEnd + m_origin.y;

    pDrawer->start_composite_notation(get_notation_id(), get_notation_class());
    pDrawer->begin_path();
    pDrawer->fill_none();
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_uLineThick);

    //horizontal line
    pDrawer->move_to(xStart, yStart);
    LUnits xPos = xStart + dxLine;
    while (xPos < xEnd)
    {
        pDrawer->hline_to(xPos);
        xPos += dxSpace;
        pDrawer->move_to(xPos, yStart);
        xPos += dxLine;
    }
    xPos -= (dxLine + dxSpace);
    pDrawer->move_to(xPos, yStart);       //to end of last stroke
    pDrawer->hline_to(xEnd);              //continue it to end point

    //end corner
    if (m_fEndCorner)
        pDrawer->vline_to(yEnd);      //add vertical stroke

    pDrawer->end_path();
    pDrawer->render();

    GmoCompositeShape::on_draw(pDrawer, opt);
    pDrawer->end_composite_notation();
}

//---------------------------------------------------------------------------------------
int GmoShapeOctaveShift::get_num_handlers()
{
    return 4;
}

//---------------------------------------------------------------------------------------
UPoint GmoShapeOctaveShift::get_handler_point(int UNUSED(i))
{
    //TODO
    return UPoint(0.0f, 0.0f);
}

//---------------------------------------------------------------------------------------
void GmoShapeOctaveShift::on_handler_dragged(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}

//---------------------------------------------------------------------------------------
void GmoShapeOctaveShift::on_end_of_handler_drag(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}


}  //namespace lomse
