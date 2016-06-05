//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
#include "lomse_score_layouter.h"
#include "lomse_right_aligner.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// PartsEngraver implementation
//---------------------------------------------------------------------------------------

PartsEngraver::PartsEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                             ImoInstrGroups* pGroups, ImoScore* pScore,
                             ScoreLayouter* pScoreLyt)
    : Engraver(libraryScope, pScoreMeter)
    , m_pGroups(pGroups)
    , m_pScore(pScore)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_pScoreLyt(pScoreLyt)
    , m_uFirstSystemIndent(0.0f)
    , m_uOtherSystemIndent(0.0f)
    , m_pRightAlignerFirst(NULL)
{
    create_group_engravers();
    create_instrument_engravers();
}

//---------------------------------------------------------------------------------------
PartsEngraver::~PartsEngraver()
{
    delete_group_engravers();
    delete_instrument_engravers();
    delete m_pRightAlignerFirst;
}

//---------------------------------------------------------------------------------------
LUnits PartsEngraver::tenths_to_logical(Tenths value, int iStaff)
{
    //return (value * m_pInstr->get_staff(iStaff)->get_line_spacing()) / 10.0f;
    return 0.0;
}

//---------------------------------------------------------------------------------------
void PartsEngraver::create_group_engravers()
{
    if (m_pGroups)
    {
        ImoObj::children_iterator it;
        for (it= m_pGroups->begin(); it != m_pGroups->end(); ++it)
        {
            ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(*it);
            m_groupEngravers.push_back(
                LOMSE_NEW GroupEngraver(m_libraryScope, m_pMeter, pGroup, m_pScore) );
        }
    }
}

//---------------------------------------------------------------------------------------
void PartsEngraver::delete_group_engravers()
{
    std::vector<GroupEngraver*>::iterator it;
    for (it = m_groupEngravers.begin(); it != m_groupEngravers.end(); ++it)
        delete *it;
    m_groupEngravers.clear();
}

//---------------------------------------------------------------------------------------
void PartsEngraver::create_instrument_engravers()
{
    int numInstr = m_pScore->get_num_instruments();

    for (int iInstr = 0; iInstr < numInstr; iInstr++)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        m_instrEngravers.push_back(
                    LOMSE_NEW InstrumentEngraver(m_libraryScope, m_pMeter,
                                                 pInstr, m_pScore) );
    }

    m_iInstrName.reserve(numInstr);
    m_iInstrBracketFirst.reserve(numInstr);
    m_iInstrAbbrev.reserve(numInstr);
    m_iInstrBracketOther.reserve(numInstr);
}

//---------------------------------------------------------------------------------------
void PartsEngraver::delete_instrument_engravers()
{
    std::vector<InstrumentEngraver*>::iterator it;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it)
        delete *it;
    m_instrEngravers.clear();
}

//---------------------------------------------------------------------------------------
void PartsEngraver::decide_systems_indentation()
{
    //Algorithm for laying out parts names and brackets/braces
    //----------------------------------------------------------------

    //Traverse all instruments and determine yTop for staves
    determine_staves_vertical_position();

    //Create a RightAligner to contain and assign final positions to all
    //names, brackets and braces. Initially empty.
    m_pRightAlignerFirst = LOMSE_NEW RightAligner();
    m_pRightAlignerOther = LOMSE_NEW RightAligner();

    //Traverse all groups from inner to outer (backwards, from last defined one to
    //first one). For each group:
    // - measure bracket/brace. Bracket/brace coords: x=0, y = determined by first
    //   instrument yTop and last instrument yBottom.
    // - add bracket/brace to the RightAligner
    measure_groups_name_and_bracket();

    //Traverse all instruments. For each instrument:
    // - engrave name/abbr at x=0, y= instr.center (middle of instrument yTop and
    //   instrument yBottom).
    // - add name/abbr to the RightAligner
    measure_instruments_name_and_bracket();

    //As each box is added to the RightAligner it is repositioned to
    //the correct position. Therefore, at this point, all the boxes for names
    //and brackets/braces are correctly positioned. Save the information.
    save_names_and_brackets_positions();
}

//---------------------------------------------------------------------------------------
void PartsEngraver::set_staves_horizontal_position(int iInstr, LUnits x, LUnits width,
                                                   LUnits indent)
{
    InstrumentEngraver* engrv = m_instrEngravers[iInstr];
    engrv->set_staves_horizontal_position(x, width, indent);
}

//---------------------------------------------------------------------------------------
void PartsEngraver::determine_staves_vertical_position()
{
    //computed vertical positions are relative to 0.0f, that is, first stave will
    //be positioned at 0.0f

    LUnits yPos = 0.0f;

    int numInstrs = m_pScore->get_num_instruments();
    for (int iInstr = 0; iInstr < numInstrs; iInstr++)
    {
        //if not first instrument add top margin
        if (iInstr > 0)
            yPos += m_pScoreLyt->determine_top_space(iInstr);

        yPos = m_instrEngravers[iInstr]->set_staves_vertical_position(yPos);

        //if more instruments add bottom margin
        if (iInstr != numInstrs-1)
            yPos += m_pScoreLyt->determine_top_space(iInstr+1);
    }
}

//---------------------------------------------------------------------------------------
void PartsEngraver::measure_groups_name_and_bracket()
{
    //Traverse all groups from inner to outer (backwards, from last defined one to
    //first one). For each group:
    // - measure bracket/brace. Bracket/brace coords: x=0, y = determined by first
    //   instrument yTop and last instrument yBottom.
    // - add bracket/brace to the RightAligner

    std::vector<GroupEngraver*>::iterator it;
    for (it = m_groupEngravers.begin(); it != m_groupEngravers.end(); ++it)
    {
        (*it)->measure_name_and_bracket();
        //m_pRightAlignerFirst->add_box( (*it)->get_box_for_bracket() );
        //m_pRightAlignerFirst->add_box( (*it)->get_box_for_name() );
    }
}

//---------------------------------------------------------------------------------------
void PartsEngraver::measure_instruments_name_and_bracket()
{
    int i = 0;
    std::vector<InstrumentEngraver*>::iterator it;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it, ++i)
    {
        (*it)->measure_name_and_bracket();
        m_iInstrBracketFirst[i] = m_pRightAlignerFirst->add_box( (*it)->get_box_for_bracket() );
        m_iInstrName[i] = m_pRightAlignerFirst->add_box( (*it)->get_box_for_name() );
        m_iInstrBracketOther[i] = m_pRightAlignerOther->add_box( (*it)->get_box_for_bracket() );
        m_iInstrAbbrev[i] = m_pRightAlignerOther->add_box( (*it)->get_box_for_abbrev() );
    }
}

//---------------------------------------------------------------------------------------
void PartsEngraver::save_names_and_brackets_positions()
{
    std::vector<int>::iterator itN = m_iInstrName.begin();
    std::vector<int>::iterator itBF = m_iInstrBracketFirst.begin();
    std::vector<int>::iterator itA = m_iInstrAbbrev.begin();
    std::vector<int>::iterator itBO = m_iInstrBracketOther.begin();
    std::vector<InstrumentEngraver*>::iterator it = m_instrEngravers.begin();

    for (; it != m_instrEngravers.end(); ++it, ++itN, ++itBF, ++itA, ++itBO)
    {
        (*it)->set_name_final_pos( m_pRightAlignerFirst->get_box(*itN) );
        (*it)->set_bracket_first_final_pos( m_pRightAlignerFirst->get_box(*itBF) );
        (*it)->set_abbrev_final_pos( m_pRightAlignerOther->get_box(*itA) );
        (*it)->set_bracket_other_final_pos( m_pRightAlignerOther->get_box(*itBO) );
    }

    m_uFirstSystemIndent = m_pRightAlignerFirst->get_total_width();
    m_uOtherSystemIndent = m_pRightAlignerOther->get_total_width();
}


//---------------------------------------------------------------------------------------
// GroupEngraver implementation
//---------------------------------------------------------------------------------------

GroupEngraver::GroupEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                             ImoInstrGroup* pGroup, ImoScore* pScore)
    : Engraver(libraryScope, pScoreMeter)
    , m_pGroup(pGroup)
    , m_pScore(pScore)
    , m_pFontStorage( libraryScope.font_storage() )
{
}

//---------------------------------------------------------------------------------------
GroupEngraver::~GroupEngraver()
{
}

//---------------------------------------------------------------------------------------
void GroupEngraver::measure_name_and_bracket()
{

}

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
void InstrumentEngraver::measure_name_and_bracket()
{
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

        m_nameBox.width = engr.measure_width() + uSpaceAfterName;
        m_nameBox.height = engr.measure_height();
        m_nameBox.x = 0.0f;
        //m_nameBox.y = m_stavesTop + ((m_stavesBottom - m_stavesTop) - m_nameBox.height) / 2.0f;
        m_nameBox.y = m_stavesTop + (m_stavesBottom - m_stavesTop) / 2.0f;
    }
    if (m_pInstr->has_abbrev())
    {
        ImoScoreText& text = m_pInstr->get_abbrev();
        ImoStyle* pStyle = text.get_style();
        if (!pStyle)
            pStyle = m_pScore->get_default_style();
        TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                          text.get_language(), pStyle);

        m_abbrevBox.width = engr.measure_width() + uSpaceAfterName;
        m_abbrevBox.height = engr.measure_height();
        m_abbrevBox.x = 0.0f;
        //m_abbrevBox.y = m_stavesTop + ((m_stavesBottom - m_stavesTop) - m_abbrevBox.height) / 2.0f;
        m_abbrevBox.y = m_stavesTop + (m_stavesBottom - m_stavesTop) / 2.0f;
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_brace_or_bracket()
{
    if (has_brace_or_bracket())
    {
        m_uBracketWidth = tenths_to_logical(LOMSE_GRP_BRACKET_WIDTH);
        m_uBracketGap = tenths_to_logical(LOMSE_GRP_BRACKET_GAP);

        m_bracketFirstBox.x = 0.0f;
        m_bracketFirstBox.y = m_stavesTop;
        m_bracketFirstBox.width = m_uBracketWidth + m_uBracketGap;
        m_bracketFirstBox.height = m_stavesBottom - m_stavesTop;
    }
}

//---------------------------------------------------------------------------------------
bool InstrumentEngraver::has_brace_or_bracket()
{
    return (m_pInstr->get_num_staves() > 1);
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::add_name_abbrev(GmoBoxSystem* pBox, int iSystem)
{
    if (iSystem == 0)
    {
        if (m_pInstr->has_name())
        {
            LUnits xLeft = m_nameBox.x + pBox->get_left();
            LUnits yTop = m_nameBox.y + m_org.y;

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
            LUnits xLeft = m_abbrevBox.x + pBox->get_left();
            LUnits yTop = m_abbrevBox.y + m_org.y;

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
void InstrumentEngraver::add_brace_bracket(GmoBoxSystem* pBox, int iSystem)
{
    if (has_brace_or_bracket())
    {
        LUnits xLeft, xRight, yTop, yBottom;
        if (iSystem == 0)
        {
            xLeft = m_bracketFirstBox.x + m_org.x + pBox->get_left();
            xRight = xLeft + m_bracketFirstBox.width - m_uBracketGap;
            yTop = m_bracketFirstBox.y + m_org.y;
            yBottom = yTop + m_bracketFirstBox.height;
        }
        else
        {
            xLeft = m_bracketOtherBox.x + m_org.x + pBox->get_left();
            xRight = xLeft + m_bracketOtherBox.width - m_uBracketGap;
            yTop = m_bracketOtherBox.y + m_org.y;
            yBottom = yTop + m_bracketOtherBox.height;
        }
        //@int nSymbol = ImoInstrGroup::k_bracket;
        //@int nSymbol = (m_bracketSymbol ==
        //@    (ImoInstrGroup::k_default ? ImoInstrGroup::k_bracket : m_bracketSymbol));

        GmoShape* pShape;
//@        if (nSymbol == lm_eBracket)
            pShape = LOMSE_NEW GmoShapeBracket(m_pInstr, 0, xLeft, yTop, xRight, yBottom,
                                         Color(0,0,0));
//@        else
//@        {
//@            LUnits dyHook = tenths_to_logical(6.0f);
//@            pShape = LOMSE_NEW GmoShapeBrace(this, xLeft, yTop, xRight, yBottom,
//@                                      dyHook, *wxBLACK);
//@        }
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
LUnits InstrumentEngraver::set_staves_vertical_position(LUnits y)
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
    return m_stavesBottom + m_org.y;
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
//    // invoked when laying out first measure in system, to add instrument name and bracket/brace.
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
