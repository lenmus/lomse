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
    , m_numShapes(0)
    , m_pVolta(nullptr)
    , m_uStaffTop(0.0f)
    , m_uStaffLeft(0.0f)
    , m_uStaffRight(0.0f)
    , m_pStyle(nullptr)
    , m_pStartBarline(nullptr)
    , m_pStopBarline(nullptr)
    , m_pStartBarlineShape(nullptr)
    , m_pStopBarlineShape(nullptr)
{
    m_pStyle = m_pMeter->get_style_info("Volta brackets");
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int UNUSED(iSystem), int UNUSED(iCol),
                                      LUnits xStaffLeft, LUnits xStaffRight, LUnits yStaffTop,
                                      int idxStaff, VerticalProfile* pVProfile)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pVolta = dynamic_cast<ImoVoltaBracket*>(pRO);

    m_pStartBarline = dynamic_cast<ImoBarline*>(pSO);
    m_pStartBarlineShape = dynamic_cast<GmoShapeBarline*>(pStaffObjShape);

    m_uStaffLeft = xStaffLeft;
    m_uStaffRight = xStaffRight;
    m_uStaffTop = yStaffTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                    int UNUSED(iStaff), int UNUSED(iSystem),
                                    int UNUSED(iCol), LUnits xStaffLeft,
                                    LUnits xStaffRight, LUnits yStaffTop,
                                    int idxStaff, VerticalProfile* pVProfile)
{
    m_pStopBarline = dynamic_cast<ImoBarline*>(pSO);
    m_pStopBarlineShape = dynamic_cast<GmoShapeBarline*>(pStaffObjShape);

    m_uStaffLeft = xStaffLeft;
    m_uStaffRight = xStaffRight;
    m_uStaffTop = yStaffTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_first_or_intermediate_shape(Color color)
{
    m_color = color;
    if (m_numShapes == 0)
        return create_first_shape();
    else
        return create_intermediate_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_last_shape(Color color)
{
    m_color = color;
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
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", m_pStyle);
    GmoShapeText* pTextShape = engr.create_shape(m_pVolta, 0.0f, 0.0f);
    pShape->add_label(pTextShape);

    set_shape_details(pShape, k_single_shape);

    m_numShapes++;
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* VoltaBracketEngraver::create_first_shape()
{
    //first shape when there are more than one

    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, m_numShapes, m_color);
    pShape->enable_final_jog(false);

    //add text
    string text = m_pVolta->get_volta_text();
    if (text.empty())
        text = m_pVolta->get_volta_number();
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", m_pStyle);
    GmoShapeText* pTextShape = engr.create_shape(m_pVolta, 0.0f, 0.0f);
    pShape->add_label(pTextShape);

    //terminate first shape
    set_shape_details(pShape, k_first_shape);
    pShape->set_two_brackets();

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
    LUnits xStart = m_uStaffLeft;
    LUnits xEnd = m_uStaffRight;

    if (shapeType == k_single_shape)
    {
        xStart = m_pStartBarlineShape->get_x_right_line();
        if (m_pStartBarlineShape->get_width() < 40.0f)
            xStart += 30.0f;

        xEnd = m_pStopBarlineShape->get_x_left_line();
        if (m_pStopBarlineShape->get_width() < 40.0f)
            xEnd -= 30.0f;
    }
    else if (shapeType == k_first_shape)
    {
        xStart = m_pStartBarlineShape->get_x_right_line();
        if (m_pStartBarlineShape->get_width() < 40.0f)
            xStart += 30.0f;
    }
    else if (shapeType == k_final_shape)
    {
        xEnd = m_pStopBarlineShape->get_x_left_line();
        if (m_pStopBarlineShape->get_width() < 40.0f)
            xEnd -= 30.0f;
    }

    //determine yPos
    LUnits uDistance = tenths_to_logical(LOMSE_VOLTA_BRACKET_DISTANCE);
    LUnits yPos = m_uStaffTop - uDistance;
    LUnits yMin = m_pVProfile->get_min_for(xStart, xEnd, m_idxStaff).first - uDistance;
    yPos = min(yPos, yMin);

    //transfer data to shape
    pShape->set_layout_data(xStart, xEnd, yPos, uDistance, uJogLength, uLineThick,
                            uLeftSpaceToText, uTopSpaceToText, m_uStaffLeft, m_uStaffRight);
}


}  //namespace lomse
