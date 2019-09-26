//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_barline_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_barline.h"
#include "lomse_box_slice_instr.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// BarlineEngraver implementation
//---------------------------------------------------------------------------------------
BarlineEngraver::BarlineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                                 int iInstr)
    : Engraver(libraryScope, pScoreMeter, iInstr)
    , m_pBarlineShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
BarlineEngraver::BarlineEngraver(LibraryScope& libraryScope)
    : Engraver(libraryScope, nullptr)
    , m_pBarlineShape(nullptr)
{
    //constructor for dragged images
}

//---------------------------------------------------------------------------------------
GmoShape* BarlineEngraver::create_shape(ImoBarline* pBarline, LUnits xPos,
                                        LUnits yTop, LUnits yBottom, Color color)
{
    LUnits thinLineWidth = m_pMeter->tenths_to_logical(LOMSE_THIN_LINE_WIDTH, m_iInstr, 0);
    LUnits thickLineWidth = m_pMeter->tenths_to_logical(LOMSE_THICK_LINE_WIDTH, m_iInstr, 0);
    LUnits spacing = m_pMeter->tenths_to_logical(LOMSE_LINES_SPACING, m_iInstr, 0);
    LUnits radius = m_pMeter->tenths_to_logical(LOMSE_BARLINE_RADIUS, m_iInstr, 0);

    //force selection rectangle to have at least a width of half line (5 tenths)
    LUnits uMinWidth = 0;   //m_pMeter->tenths_to_logical(5.0f, m_iInstr, 0);

    ShapeId idx = 0;
    return LOMSE_NEW GmoShapeBarline(pBarline, idx, pBarline->get_type(),
                                     xPos, yTop, yBottom,
                                     thinLineWidth, thickLineWidth, spacing,
                                     radius, color, uMinWidth);
}

//---------------------------------------------------------------------------------------
GmoShape* BarlineEngraver::create_system_barline_shape(ImoObj* pCreatorImo, LUnits xPos,
                                                       LUnits yTop, LUnits yBottom,
                                                       Color color)
{
    LUnits uLineThickness = m_pMeter->tenths_to_logical(LOMSE_THIN_LINE_WIDTH, 0, 0);
    return LOMSE_NEW GmoShapeBarline(pCreatorImo, 0, k_barline_simple,
                                     xPos, yTop, yBottom,
                                     uLineThickness, uLineThickness,
                                     0.0f, 0.0f, color, uLineThickness);
}

//---------------------------------------------------------------------------------------
GmoShape* BarlineEngraver::create_tool_dragged_shape(int barType)
{
    //typical values, obtained by by placing a breakpoint in create_shape() method
    LUnits thinLineWidth = 27.0;
    LUnits thickLineWidth = 108.0;
    LUnits spacing = 72.0;
    LUnits radius = 36.0;
    LUnits uMinWidth = 0;
    LUnits yBottom = 720.0;

    Color color(255,0,0);       //TODO: options/configuration
    ShapeId idx = 0;

    m_pBarlineShape = LOMSE_NEW GmoShapeBarline(nullptr, idx, barType, 0.0, 0.0, yBottom,
                                                thinLineWidth, thickLineWidth, spacing,
                                                radius, color, uMinWidth);
    return m_pBarlineShape;
}

//---------------------------------------------------------------------------------------
UPoint BarlineEngraver::get_drag_offset()
{
    //return left side, vertical center
    URect bounds = m_pBarlineShape->get_bounds();
    return UPoint(0.0f, bounds.get_height() / 2.0f);
}



}  //namespace lomse
