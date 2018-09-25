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


namespace lomse
{

//---------------------------------------------------------------------------------------
// VoltaBracketEngraver implementation
//---------------------------------------------------------------------------------------
VoltaBracketEngraver::VoltaBracketEngraver(LibraryScope& libraryScope,
                                           ScoreMeter* pScoreMeter,
                                           LUnits uStaffLeft, LUnits uStaffRight)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_numShapes(0)
    , m_pVolta(nullptr)
    , m_uStaffLeft(uStaffLeft)
    , m_uStaffRight(uStaffRight)
    , m_pStyle(nullptr)
    , m_pStartBarline(nullptr)
    , m_pStopBarline(nullptr)
    , m_pStartBarlineShape(nullptr)
    , m_pStopBarlineShape(nullptr)
    , m_fTwoBrackets(false)
{
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int iSystem, int iCol, LUnits UNUSED(xRight),
                                      LUnits UNUSED(xLeft), LUnits UNUSED(yTop))
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pVolta = dynamic_cast<ImoVoltaBracket*>(pRO);

    m_pStartBarline = dynamic_cast<ImoBarline*>(pSO);
    m_pStartBarlineShape = dynamic_cast<GmoShapeBarline*>(pStaffObjShape);

    m_shapesInfo[0].iCol = iCol;
    m_shapesInfo[0].iInstr = iInstr;
    m_shapesInfo[0].iSystem = iSystem;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int iInstr,
                                    int UNUSED(iStaff), int iSystem, int iCol,
                                    LUnits UNUSED(xRight), LUnits UNUSED(xLeft),
                                    LUnits UNUSED(yTop))
{
    m_pStopBarline = dynamic_cast<ImoBarline*>(pSO);
    m_pStopBarlineShape = dynamic_cast<GmoShapeBarline*>(pStaffObjShape);

    m_shapesInfo[1].iCol = iCol;
    m_shapesInfo[1].iInstr = iInstr;
    m_shapesInfo[1].iSystem = iSystem;
}

//---------------------------------------------------------------------------------------
int VoltaBracketEngraver::create_shapes(Color color)
{
    m_color = color;
    m_pStyle = m_pMeter->get_style_info("Volta brackets");

    decide_if_one_or_two_brackets();
    if (two_brackets_needed())
        create_two_shapes();
    else
        create_one_shape();

    return m_numShapes;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::create_one_shape()
{
    m_numShapes = 1;

    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, 0, m_color);
    m_shapesInfo[0].pShape = pShape;
    pShape->enable_final_jog( m_pVolta->has_final_jog() );

    //add text
    string text = m_pVolta->get_volta_text();
    if (text.empty())
        text = m_pVolta->get_volta_number();
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", m_pStyle);
    GmoShapeText* pTextShape = engr.create_shape(m_pVolta, 0.0f, 0.0f);
    pShape->add_label(pTextShape);

    set_shape_details(pShape, true);


    m_shapesInfo[1].pShape = nullptr;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::create_two_shapes()
{
    m_numShapes = 2;

    //create first shape
    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, 0, m_color);
    m_shapesInfo[0].pShape = pShape;
    pShape->enable_final_jog(false);
        //add text
    string text = m_pVolta->get_volta_text();
    if (text.empty())
        text = m_pVolta->get_volta_number();
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", m_pStyle);
    GmoShapeText* pTextShape = engr.create_shape(m_pVolta, 0.0f, 0.0f);
    pShape->add_label(pTextShape);
        //terminate first shape
    set_shape_details(pShape, true);
    pShape->set_two_brackets();

    //create second shape
    pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, 1, m_color);
    m_shapesInfo[1].pShape = pShape;
    pShape->enable_final_jog( m_pVolta->has_final_jog() );
        //terminate second shape
    set_shape_details(pShape, false);
    pShape->set_two_brackets();
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_shape_details(GmoShapeVoltaBracket* pShape,
                                             bool fFirstShape)
{
    //data that will not change at system justification time is set here

    LUnits uDistance = tenths_to_logical(LOMSE_VOLTA_BRACKET_DISTANCE);
    LUnits uLineThick = tenths_to_logical(LOMSE_VOLTA_BRACKET_THICKNESS);
    LUnits uLeftSpaceToText = tenths_to_logical(LOMSE_VOLTA_LEFT_SPACE_TO_TEXT);
    LUnits uJogLength = tenths_to_logical(LOMSE_VOLTA_JOG_LENGHT);

    LUnits xStart = m_pStartBarlineShape->get_x_right_line();
    LUnits xEnd = m_pStopBarlineShape->get_x_left_line();
    LUnits yPos = m_pStopBarlineShape->get_top() - uDistance;
    if (m_fTwoBrackets)
    {
        if (fFirstShape )
        {
            yPos = m_pStartBarlineShape->get_top() - uDistance;
            xEnd = m_uStaffRight;
        }
        else
        {
            xStart = m_uStaffLeft;
        }
    }
    else if (xEnd < xStart)
    {
        xStart = m_uStaffLeft;
    }



    pShape->set_layout_data(xStart, xEnd, yPos, uDistance, uJogLength, uLineThick,
                            uLeftSpaceToText, m_uStaffLeft, m_uStaffRight,
                            m_pStartBarlineShape, m_pStopBarlineShape);
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::decide_if_one_or_two_brackets()
{
    m_fTwoBrackets = (m_shapesInfo[0].iSystem != m_shapesInfo[1].iSystem);
}

//---------------------------------------------------------------------------------------
int VoltaBracketEngraver::get_num_shapes()
{
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
ShapeBoxInfo* VoltaBracketEngraver::get_shape_box_info(int i)
{
    return &m_shapesInfo[i];
}


}  //namespace lomse
