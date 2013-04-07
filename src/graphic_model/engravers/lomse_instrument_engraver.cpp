//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#include "lomse_instrument_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_brace_bracket.h"
#include "lomse_shape_staff.h"
#include "lomse_box_system.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// InstrumentEngraver implementation
//---------------------------------------------------------------------------------------

InstrumentEngraver::InstrumentEngraver(LibraryScope& libraryScope,
                                       ScoreMeter* pScoreMeter,
                                       ImoInstrument* pInstr, ImoScore* pScore)
    : Engraver(libraryScope, pScoreMeter)
    , m_pInstr(pInstr)
    , m_pScore(pScore)
    , m_pFontStorage( libraryScope.font_storage() )
{
    int numStaves = m_pInstr->get_num_staves();
    m_staffTop.reserve(numStaves);
    m_staffTopLine.reserve(numStaves);
    m_lineThickness.reserve(numStaves);
}

//---------------------------------------------------------------------------------------
InstrumentEngraver::~InstrumentEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::tenths_to_logical(Tenths value, int iStaff)
{
    return (value * m_pInstr->get_staff(iStaff)->get_line_spacing()) / 10.0f;
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_indents()
{
    m_uIndentFirst = 0.0f;
    m_uIndentOther = 0.0f;
    measure_name_abbrev();
    measure_brace_or_bracket();
}

////---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_name_abbrev()
{
//    //if this instrument is in a group get group names width
//    if (m_pGroup)
//    {
//        if (m_pGroup->GetFirstInstrument() == this)
//        {
//            m_pGroup->MeasureNames(pPaper);
//        }
//        m_uIndentFirst += m_pGroup->GetIndentFirst();
//        m_uIndentOther += m_pGroup->GetIndentOther();
//    }

    LUnits uSpaceAfterName = tenths_to_logical(LOMSE_INSTR_SPACE_AFTER_NAME);
    if (m_pInstr->has_name())
    {
        ImoScoreText& text = m_pInstr->get_name();
        ImoStyle* pStyle = text.get_style();
        if (!pStyle)
            pStyle = m_pScore->get_default_style();
        TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                          text.get_language(), pStyle);
        m_uIndentFirst += engr.measure_width() + uSpaceAfterName;
    }
    if (m_pInstr->has_abbrev())
    {
        ImoScoreText& text = m_pInstr->get_abbrev();
        ImoStyle* pStyle = text.get_style();
        if (!pStyle)
            pStyle = m_pScore->get_default_style();
        TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                          text.get_language(), pStyle);
        m_uIndentOther += engr.measure_width() + uSpaceAfterName;
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_brace_or_bracket()
{
    if (has_brace_or_bracket())
    {
        m_uBracketWidth = tenths_to_logical(LOMSE_GRP_BRACKET_WIDTH);
        m_uBracketGap = tenths_to_logical(LOMSE_GRP_BRACKET_GAP);

        m_uIndentOther += m_uBracketWidth + m_uBracketGap;
        m_uIndentFirst += m_uBracketWidth + m_uBracketGap;
    }
}

//---------------------------------------------------------------------------------------
bool InstrumentEngraver::has_brace_or_bracket()
{
    return (m_pInstr->get_num_staves() > 1);

    //return (m_pInstr->get_num_staves() > 1 && m_bracketSymbol == ImoInstrGroup::k_default ||
    //        m_bracketSymbol == ImoInstrGroup::k_bracket ||
    //        m_bracketSymbol == ImoInstrGroup::k_brace );
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_name_abbrev(GmoBoxSystem* pBox, int iSystem)
{
    LUnits xLeft = pBox->get_left();
    LUnits yTop = m_stavesTop + m_org.y + (m_stavesBottom - m_stavesTop) / 2.0f;

    if (iSystem == 0)
    {
        if (m_pInstr->has_name())
        {
            ImoScoreText& text = m_pInstr->get_name();
            ImoStyle* pStyle = text.get_style();
            if (!pStyle)
                pStyle = m_pScore->get_default_style();
            TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                              text.get_language(), pStyle);
            GmoShape* pShape = engr.create_shape(m_pInstr, xLeft, yTop);
            pBox->add_shape(pShape, GmoShape::k_layer_staff);
        }
    }
    else
    {
        if (m_pInstr->has_abbrev())
        {
            ImoScoreText& text = m_pInstr->get_abbrev();
            ImoStyle* pStyle = text.get_style();
            if (!pStyle)
                pStyle = m_pScore->get_default_style();
            TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                              text.get_language(), pStyle);
            GmoShape* pShape = engr.create_shape(m_pInstr, xLeft, yTop);
            pBox->add_shape(pShape, GmoShape::k_layer_staff);
        }
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_brace_bracket(GmoBoxSystem* pBox)
{
    if (has_brace_or_bracket())
    {
        LUnits xLeft = m_stavesLeft - m_uBracketWidth - m_uBracketGap + m_org.x;
        LUnits xRight = m_stavesLeft - m_uBracketGap + m_org.x;
        LUnits yTop = m_stavesTop + m_org.y;
        LUnits yBottom = m_stavesBottom + m_org.y;
        //int nSymbol = ImoInstrGroup::k_bracket;
        //int nSymbol = (m_bracketSymbol ==
        //    (ImoInstrGroup::k_default ? ImoInstrGroup::k_bracket : m_bracketSymbol));

        GmoShape* pShape;
//        if (nSymbol == lm_eBracket)
            pShape = LOMSE_NEW GmoShapeBracket(m_pInstr, 0, xLeft, yTop, xRight, yBottom,
                                         Color(0,0,0));
//        else
//        {
//            LUnits dyHook = tenths_to_logical(6.0f);
//            pShape = LOMSE_NEW GmoShapeBrace(this, xLeft, yTop, xRight, yBottom,
//                                      dyHook, *wxBLACK);
//        }
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_staff_lines(GmoBoxSystem* pBox)
{
    for (int iStaff=0; iStaff < m_pInstr->get_num_staves(); iStaff++)
	{
        ImoStaffInfo* pStaff = m_pInstr->get_staff(iStaff);
        GmoShapeStaff* pShape
            = LOMSE_NEW GmoShapeStaff(m_pInstr, iStaff, pStaff, iStaff, m_stavesWidth,
                                Color(0,0,0));
        pShape->set_origin(m_stavesLeft + m_org.x, m_staffTop[iStaff] + m_org.y);
        pBox->add_staff_shape(pShape);
 //       pShape->SetVisible(fVisible);
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::set_staves_horizontal_position(LUnits x, LUnits width,
                                                        LUnits indent)
{
    m_stavesLeft = x + indent;
    m_stavesWidth = width - indent;
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::set_staves_vertical_position(LUnits y)
{
    m_org.x = m_org.y = 0.0f;

    m_stavesTop = y;
    for (int iStaff=0; iStaff < m_pInstr->get_num_staves(); iStaff++)
	{
        ImoStaffInfo* pStaff = m_pInstr->get_staff(iStaff);
        if (iStaff > 0)
            y += pStaff->get_staff_margin();
        m_staffTop[iStaff] = y;
        m_staffTopLine[iStaff] = y;
        y += pStaff->get_height();
        m_lineThickness[iStaff] = pStaff->get_line_thickness();
    }
    m_stavesBottom = y;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_top_line_of_staff(int iStaff)
{
    return m_org.y + m_staffTop[iStaff] + m_lineThickness[iStaff] / 2.0f;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_staves_top_line()
{
    return m_org.y + m_staffTop[0] + m_lineThickness[0] / 2.0f;;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_staves_bottom_line()
{
    int iStaff = m_pInstr->get_num_staves() - 1;
    return  m_org.y + m_stavesBottom - m_lineThickness[iStaff] / 2.0f;
}

////---------------------------------------------------------------------------------------
//void InstrumentEngraver::AddNameAndBracket(GmoBox* pBSystem, GmoBox* pBSliceInstr, lmPaper* pPaper,
//                                     int nSystem)
//{
//    //Layout.
//    // invoked when layouting first measure in system, to add instrument name and bracket/brace.
//    // This method is also responsible for managing group name and bracket/brace layout
//    // When reaching this point, BoxSystem and BoxSliceInstr have their bounds correctly
//    // set (except xRight)
//
//	if (nSystem == 1)
//        add_name_abbrev_shape(pBSliceInstr, pPaper, m_pName);
//	else
//        add_name_abbrev_shape(pBSliceInstr, pPaper, m_pAbbreviation);
//
//    // if first instrument in group, save yTop position for group
//    static LUnits yTopGroup;
//	if (IsFirstOfGroup())
//		yTopGroup = pBSliceInstr->GetYTop();
//
//    // if last instrument of a group, add group name and bracket/brace
//	if (IsLastOfGroup())
//        m_pGroup->AddNameAndBracket(pBSystem, pPaper, nSystem, pBSliceInstr->GetXLeft(),
//                                    yTopGroup, pBSliceInstr->GetYBottom() );
//}



}  //namespace lomse
