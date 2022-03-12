//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_barline_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_instrument_engraver.h"
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
                                 int iInstr, InstrumentEngraver* pInstrEngrv)
    : StaffObjEngraver(libraryScope, pScoreMeter, iInstr, 0)
    , m_pBarlineShape(nullptr)
    , m_pInstrEngrv(pInstrEngrv)
{
}

//---------------------------------------------------------------------------------------
BarlineEngraver::BarlineEngraver(LibraryScope& libraryScope)
    : StaffObjEngraver(libraryScope, nullptr, 0, 0)
    , m_pBarlineShape(nullptr)
    , m_pInstrEngrv(nullptr)
{
    //constructor for dragged images
}

//---------------------------------------------------------------------------------------
static bool is_repeat_barline_type(EBarline type)
{
    switch (type)
    {
        case k_barline_unknown:
        case k_barline_none:
        case k_barline_simple:
        case k_barline_double:
        case k_barline_start:
        case k_barline_end:
            return false;
        case k_barline_start_repetition:
        case k_barline_end_repetition:
        case k_barline_double_repetition:
        case k_barline_double_repetition_alt:
            return true;
        case k_max_barline:
            break;
    }

    return false;
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

    const EBarline barline_type = static_cast<EBarline>(pBarline->get_type());

    ShapeId idx = 0;
    GmoShapeBarline* pBarlineShape = LOMSE_NEW GmoShapeBarline(pBarline, idx, barline_type,
                                                               xPos, yTop, yBottom,
                                                               thinLineWidth, thickLineWidth, spacing,
                                                               radius, color, uMinWidth);

    if (m_pInstrEngrv && is_repeat_barline_type(barline_type))
    {
        const int numStaves = m_pInstrEngrv->get_num_staves();

        std::vector<LUnits> relStaffTopPositions;
        relStaffTopPositions.reserve(numStaves);

        for (int i = 0; i < numStaves; ++i)
        {
            const LUnits yStaffTop = m_pInstrEngrv->get_top_line_of_staff(i);
            relStaffTopPositions.push_back(yStaffTop - yTop);
        }

        pBarlineShape->set_relative_staff_top_positions(std::move(relStaffTopPositions));
    }

    return pBarlineShape;
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
