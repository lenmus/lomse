//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2017. All rights reserved.
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
                                           ScoreMeter* pScoreMeter, LUnits uStaffLeft)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(NULL)
    , m_numShapes(0)
    , m_uPrologWidth(0.0f)
    , m_uStaffLeft(uStaffLeft)
{
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int iSystem, int iCol, LUnits UNUSED(xRight),
                                      LUnits UNUSED(xLeft), LUnits yTop)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pVolta = dynamic_cast<ImoVoltaBracket*>(pRO);

    m_pStartBarline = dynamic_cast<ImoBarline*>(pSO);
    m_pStartBarlineShape = dynamic_cast<GmoShapeBarline*>(pStaffObjShape);

    m_shapesInfo[0].iCol = iCol;
    m_shapesInfo[0].iInstr = iInstr;
    m_shapesInfo[0].iSystem = iSystem;

    m_uStaffTopStart = yTop - m_pStartBarlineShape->get_top();     //relative to note top
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int iInstr,
                                    int UNUSED(iStaff), int iSystem, int iCol,
                                    LUnits UNUSED(xRight), LUnits UNUSED(xLeft),
                                    LUnits yTop)
{
    m_pStopBarline = dynamic_cast<ImoBarline*>(pSO);
    m_pStopBarlineShape = dynamic_cast<GmoShapeBarline*>(pStaffObjShape);

    m_shapesInfo[1].iCol = iCol;
    m_shapesInfo[1].iInstr = iInstr;
    m_shapesInfo[1].iSystem = iSystem;

    m_uStaffTopEnd = yTop - m_pStopBarlineShape->get_top();     //relative to note top;
}

//---------------------------------------------------------------------------------------
int VoltaBracketEngraver::create_shapes(Color color)
{
    m_color = color;
    m_pStyle = m_pMeter->get_tuplets_style_info();      //TODO

    //decide_if_one_or_two_brackets();
    //if (two_brackets_needed())
    //    create_two_shapes();
    //else
        create_one_shape();

    return m_numShapes;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::create_one_shape()
{
    m_numShapes = 1;

    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, 0, m_color);
    m_shapesInfo[0].pShape = pShape;

    //add text
    string text = m_pVolta->get_volta_text();
    if (text.empty())
        text = m_pVolta->get_volta_number();
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", m_pStyle);
    GmoShapeText* pTextShape = engr.create_shape(m_pVolta, 0.0f, 0.0f);
    pShape->add_label(pTextShape);

    set_shape_details(pShape);


    m_shapesInfo[1].pShape = NULL;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::create_two_shapes()
{
    m_numShapes = 2;

    //create first shape
    GmoShapeVoltaBracket* pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, 0, m_color);
    set_shape_details(pShape);
    m_shapesInfo[0].pShape = pShape;

    //create second shape
    pShape = LOMSE_NEW GmoShapeVoltaBracket(m_pVolta, 0, m_color);
    set_shape_details(pShape);
    m_shapesInfo[1].pShape = pShape;
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::set_shape_details(GmoShapeVoltaBracket* pShape)
{
    LUnits uLineThick = tenths_to_logical(LOMSE_TUPLET_BRACKET_THICKNESS);
    LUnits uSpaceToText = tenths_to_logical(LOMSE_TUPLET_SPACE_TO_NUMBER);

    LUnits xStart = m_pStartBarlineShape->get_x_right_line();
    LUnits xEnd = m_pStopBarlineShape->get_x_left_line();
    LUnits yPos;
    if (xEnd < xStart)
    {
        xStart = m_uStaffLeft;
        yPos = m_pStopBarlineShape->get_top() +
                        tenths_to_logical(LOMSE_TUPLET_SPACE_TO_NUMBER);
    }
    else
        yPos = m_pStartBarlineShape->get_top() +
                        tenths_to_logical(LOMSE_TUPLET_SPACE_TO_NUMBER);

    LUnits yJog = 4 * tenths_to_logical(LOMSE_TUPLET_SPACE_TO_NUMBER);

    pShape->set_layout_data(xStart, xEnd, yPos, yJog, uLineThick, uSpaceToText,
                            m_uStaffLeft, m_pStartBarlineShape, m_pStopBarlineShape);
}

//---------------------------------------------------------------------------------------
void VoltaBracketEngraver::decide_if_one_or_two_brackets()
{
//    m_fTwoBrackets = (m_shapesInfo[0].iSystem != m_shapesInfo[1].iSystem);
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
