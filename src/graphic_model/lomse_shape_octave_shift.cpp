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
    , m_xLineStart(0.0f)
    , m_xLineEnd(0.0f)
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
                                          LUnits yEnd, LUnits uLineThick)
{
    //save data
    m_xLineStart = xStart;
    m_xLineEnd = xEnd;
    m_yLineStart = yStart;
    m_yLineEnd = yEnd;
    m_uLineThick = uLineThick;

//    compute_bounds(xStart, xEnd, yPos);       //TODO
}

//---------------------------------------------------------------------------------------
void GmoShapeOctaveShift::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);      //Color(255,0,0);

    LUnits dxLine = 100.0f;
    LUnits dxSpace = 100.0f;

    pDrawer->begin_path();
    pDrawer->fill_none();
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_uLineThick);

    //horizontal line
    pDrawer->move_to(m_xLineStart, m_yLineStart);
    LUnits xPos = m_xLineStart + dxLine;
    while (xPos < m_xLineEnd)
    {
        pDrawer->hline_to(xPos);
        xPos += dxSpace;
        pDrawer->move_to(xPos, m_yLineStart);
        xPos += dxLine;
    }

    //vertical line
    xPos -= (dxLine + dxSpace);
    pDrawer->move_to(xPos, m_yLineStart);       //to end of last stroke
    pDrawer->hline_to(m_xLineEnd);              //continue it to end point
    pDrawer->vline_to(m_yLineEnd);              //add vertical stroke

    pDrawer->end_path();
    pDrawer->render();

    GmoCompositeShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeOctaveShift::compute_bounds(LUnits xStart, LUnits xEnd, LUnits yPos)
{
//TODO
//    m_origin.x = xStart;
//    m_size.width = abs(xEnd - xStart);
//    m_origin.y = yPos;
//
//    if (m_pShapeText)
//        m_size.height = max(m_uJogLength, m_pShapeText->get_height());
//    else
//        m_size.height = m_uJogLength;
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
