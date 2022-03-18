//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_volta_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_barline.h"
#include "lomse_shape_volta_bracket.h"
#include "lomse_score_meter.h"
#include "lomse_instrument_engraver.h"
#include "lomse_shape_text.h"
#include "lomse_text_engraver.h"
#include "lomse_vertical_profile.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// VoltaBracketEngraver implementation
//---------------------------------------------------------------------------------------
VoltaBracketEngraver::VoltaBracketEngraver(LibraryScope& libraryScope,
                                           ScoreMeter* pScoreMeter)
    : RelObjEngraver(libraryScope, pScoreMeter)
{
    m_pStyle = m_pMeter->get_style_info("Volta brackets");
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc)
{
    m_iInstr = aoc.iInstr;
    m_iStaff = aoc.iStaff;
    m_idxStaff = aoc.idxStaff;

    m_pVolta = dynamic_cast<ImoVoltaBracket*>(pRO);

    m_pStartBarline = dynamic_cast<ImoBarline*>(aoc.pSO);
    m_pStartBarlineShape = dynamic_cast<GmoShapeBarline*>(aoc.pStaffObjShape);
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), const AuxObjContext& aoc)
{
    m_pStopBarline = dynamic_cast<ImoBarline*>(aoc.pSO);
    m_pStopBarlineShape = dynamic_cast<GmoShapeBarline*>(aoc.pStaffObjShape);
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_first_or_intermediate_shape(const RelObjEngravingContext& ctx)
{
    save_context_parameters(ctx);

    if (m_numShapes == 0)
        return create_first_shape();
    else
        return create_intermediate_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_last_shape(const RelObjEngravingContext& ctx)
{
    save_context_parameters(ctx);

    if (m_numShapes == 0)
        return create_single_shape();
    return create_final_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_intermediate_shape()
{
    //intermediate shape spanning the whole system

    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, m_numShapes, m_color);
    pShape->enable_final_jog(false);

    set_shape_details(pShape, k_intermediate_shape);
    pShape->set_two_brackets();

    add_to_aux_shapes_aligner(pShape, true);
    ++m_numShapes;
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_single_shape()
{
    //the only shape when start and end points are in the same system

    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, m_numShapes, m_color);
    pShape->enable_final_jog( m_pVolta->has_final_jog() );

    //add text
    string text = m_pVolta->get_volta_text();
    if (text.empty())
        text = m_pVolta->get_volta_number();
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", m_pStyle,
                      TextEngraver::k_class_volta_text);
    GmoShapeText* pTextShape = engr.create_shape(m_pVolta, 0, 0.0f, 0.0f);
    pShape->add_label(pTextShape);

    set_shape_details(pShape, k_single_shape);

    add_to_aux_shapes_aligner(pShape, true);
    m_numShapes++;
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_first_shape()
{
    //first shape when there are more than one

    LUnits minLength = tenths_to_logical(10.0f);
    if (!m_fFirstShapeAtSystemStart
        && m_uStaffRight - m_pStartBarlineShape->get_x_right_line() < minLength)
    {
        //start barline is at the end of a system, create first shape at next system start instead
        m_fFirstShapeAtSystemStart = true;
        return nullptr;
    }

    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, m_numShapes, m_color);
    pShape->enable_final_jog(false);

    //add text
    string text = m_pVolta->get_volta_text();
    if (text.empty())
        text = m_pVolta->get_volta_number();
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", m_pStyle,
                      TextEngraver::k_class_volta_text);
    ShapeId idx = ShapeId(m_numShapes);
    GmoShapeText* pTextShape = engr.create_shape(m_pVolta, idx, 0.0f, 0.0f);
    pShape->add_label(pTextShape);

    //terminate first shape
    set_shape_details(pShape, k_first_shape);
    pShape->set_two_brackets();

    add_to_aux_shapes_aligner(pShape, true);
    m_numShapes++;
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_final_shape()
{
    //last shape when there are more than one

    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, m_numShapes, m_color);
    pShape->enable_final_jog( m_pVolta->has_final_jog() );

    set_shape_details(pShape, k_final_shape);
    pShape->set_two_brackets();

    add_to_aux_shapes_aligner(pShape, true);
    m_numShapes++;
    return pShape;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_shape_details(GmoShapeVoltaBracket* pShape,
                                             EShapeType shapeType)
{
    LUnits uLineThick = tenths_to_logical(LOMSE_VOLTA_BRACKET_THICKNESS);
    LUnits uLeftSpaceToText = tenths_to_logical(LOMSE_VOLTA_LEFT_SPACE_TO_TEXT);
    LUnits uTopSpaceToText = tenths_to_logical(LOMSE_VOLTA_TOP_SPACE_TO_TEXT);
    LUnits uJogLength = tenths_to_logical(LOMSE_VOLTA_JOG_LENGHT);

    //determine xStart and xEnd
    LUnits xStart = m_uStaffLeft + m_uPrologWidth;
    LUnits xEnd = m_uStaffRight;

    if (!m_fFirstShapeAtSystemStart && (shapeType == k_single_shape || shapeType == k_first_shape))
    {
        xStart = m_pStartBarlineShape->get_x_right_line();
        if (m_pStartBarlineShape->get_width() < 40.0f)
            xStart += 30.0f;
    }

    if (shapeType == k_single_shape || shapeType == k_final_shape)
    {
        xEnd = m_pStopBarlineShape->get_x_left_line();
        if (m_pStopBarlineShape->get_width() < 40.0f)
            xEnd -= 30.0f;
    }

//    if (shapeType == k_intermediate_shape
//        || (m_fFirstShapeAtSystemStart && (shapeType == k_first_shape )) )
//    {
//        xStart += m_uPrologWidth;
//    }

    //determine yPos
    LUnits uDistance = tenths_to_logical(LOMSE_VOLTA_BRACKET_DISTANCE);
    LUnits yPos = m_uStaffTop - uDistance;
    LUnits yMin = m_pVProfile->get_min_for(xStart, xEnd, m_idxStaff).first - uDistance;
    yPos = min(yPos, yMin);

    //transfer data to shape
    pShape->set_layout_data(xStart, xEnd, yPos, uDistance, uJogLength, uLineThick,
                            uLeftSpaceToText, uTopSpaceToText,
                            m_uStaffLeft + m_uPrologWidth, m_uStaffRight);
}


}  //namespace lomse
