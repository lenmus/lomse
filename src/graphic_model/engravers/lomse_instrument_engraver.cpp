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
    , m_pRightAlignerFirst(nullptr)
    , m_pRightAlignerOther(nullptr)
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
void PartsEngraver::create_group_engravers()
{
    if (m_pGroups)
    {
        int numGrps = m_pGroups->get_num_items();

        ImoObj::children_iterator it;
        for (it= m_pGroups->begin(); it != m_pGroups->end(); ++it)
        {
            ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(*it);
            m_groupEngravers.push_back(
                LOMSE_NEW GroupEngraver(m_libraryScope, m_pMeter, pGroup, m_pScore, this) );
        }

        m_iGrpName.resize(numGrps);
        m_iGrpBracketFirst.resize(numGrps);
        m_iGrpAbbrev.resize(numGrps);
        m_iGrpBracketOther.resize(numGrps);
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

    InstrumentEngraver* pPrevEngrv = nullptr;
    for (int iInstr = 0; iInstr < numInstr; iInstr++)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        InstrumentEngraver* pEngrv = LOMSE_NEW InstrumentEngraver(m_libraryScope, m_pMeter,
                                                                  pInstr, m_pScore);
        m_instrEngravers.push_back(pEngrv);
        if (pPrevEngrv)
            pPrevEngrv->set_next_instrument_engraver(pEngrv);

        pPrevEngrv = pEngrv;
    }

    m_iInstrName.resize(numInstr);
    m_iInstrBracketFirst.resize(numInstr);
    m_iInstrAbbrev.resize(numInstr);
    m_iInstrBracketOther.resize(numInstr);
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
LUnits PartsEngraver::get_staff_top_position_for(ImoInstrument* pInstr)
{
    int iInstr = m_pScore->get_instr_number_for(pInstr);
    return m_instrEngravers[iInstr]->get_staff_top_position();
}

//---------------------------------------------------------------------------------------
LUnits PartsEngraver::get_staff_bottom_position_for(ImoInstrument* pInstr)
{
    int iInstr = m_pScore->get_instr_number_for(pInstr);
    return m_instrEngravers[iInstr]->get_staff_bottom_position();
}

//---------------------------------------------------------------------------------------
void PartsEngraver::measure_groups_name_and_bracket()
{
    //Traverse all groups from inner to outer (backwards, from last defined one to
    //first one). For each group:
    // - measure bracket/brace. Bracket/brace coords: x=0, y = determined by first
    //   instrument yTop and last instrument yBottom.
    // - add bracket/brace to the RightAligner

    int i = 0;
    std::vector<GroupEngraver*>::iterator it;
    for (it = m_groupEngravers.begin(); it != m_groupEngravers.end(); ++it, ++i)
    {
        (*it)->measure_name_and_bracket();
        m_iGrpBracketFirst[i] = m_pRightAlignerFirst->add_box( (*it)->get_box_for_bracket() );
        m_iGrpName[i] = m_pRightAlignerFirst->add_box( (*it)->get_box_for_name() );
        m_iGrpBracketOther[i] = m_pRightAlignerOther->add_box( (*it)->get_box_for_bracket() );
        m_iGrpAbbrev[i] = m_pRightAlignerOther->add_box( (*it)->get_box_for_abbrev() );
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

    itN = m_iGrpName.begin();
    itBF = m_iGrpBracketFirst.begin();
    itA = m_iGrpAbbrev.begin();
    itBO = m_iGrpBracketOther.begin();
    std::vector<GroupEngraver*>::iterator itG = m_groupEngravers.begin();

    for (; itG != m_groupEngravers.end(); ++itG, ++itN, ++itBF, ++itA, ++itBO)
    {
        (*itG)->set_name_final_pos( m_pRightAlignerFirst->get_box(*itN) );
        (*itG)->set_bracket_first_final_pos( m_pRightAlignerFirst->get_box(*itBF) );
        (*itG)->set_abbrev_final_pos( m_pRightAlignerOther->get_box(*itA) );
        (*itG)->set_bracket_other_final_pos( m_pRightAlignerOther->get_box(*itBO) );
    }

    m_uFirstSystemIndent = m_pRightAlignerFirst->get_total_width();
    m_uOtherSystemIndent = m_pRightAlignerOther->get_total_width();
}

//---------------------------------------------------------------------------------------
void PartsEngraver::engrave_names_and_brackets(bool fDrawStafflines, GmoBoxSystem* pBox,
                                               int iSystem)
{
    std::vector<GroupEngraver*>::iterator itG;
    for (itG = m_groupEngravers.begin(); itG != m_groupEngravers.end(); ++itG)
    {
        (*itG)->add_name_abbrev(pBox, iSystem);
        (*itG)->add_brace_bracket(pBox, iSystem);
    }

    std::vector<InstrumentEngraver*>::iterator it;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it)
    {
        if (fDrawStafflines)
            (*it)->add_staff_lines(pBox);
        (*it)->add_name_abbrev(pBox, iSystem);
        (*it)->add_brace_bracket(pBox, iSystem);
    }
}

//---------------------------------------------------------------------------------------
void PartsEngraver::set_position_and_width_for_staves(LUnits indent, UPoint org,
                                                      GmoBoxSystem* pBoxSystem)
{
    //For engraving staffobjs it is necessary to know the staves position.
    //Now, once the system box is created, instrument engravers will compute
    //staves position, width and vertical distance between staves. The
    //vertical distance is standard, based only on staves margins.

    LUnits width = pBoxSystem->get_usable_width();
    LUnits left = pBoxSystem->get_left();

    std::vector<GroupEngraver*>::iterator itG;
    for (itG = m_groupEngravers.begin(); itG != m_groupEngravers.end(); ++itG)
    {
        (*itG)->set_slice_instr_origin(org);
    }

    std::vector<InstrumentEngraver*>::iterator it;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it)
    {
        (*it)->set_staves_horizontal_position(left, width, indent);
        (*it)->set_slice_instr_origin(org);
    }
}

//---------------------------------------------------------------------------------------
void PartsEngraver::set_staves_width(LUnits width)
{
    std::vector<InstrumentEngraver*>::iterator it;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it)
    {
        (*it)->set_staves_width(width);
    }
}

//---------------------------------------------------------------------------------------
void PartsEngraver::reposition_staves_in_engravers(const vector<LUnits>& yShifts)
{
    std::vector<InstrumentEngraver*>::iterator it;
    int idxStaff = 0;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it)
    {
        int numStaves = (*it)->get_num_staves();
        for (int iStaff=0; iStaff < numStaves; ++iStaff)
        {
            (*it)->reposition_staff(iStaff, yShifts[idxStaff]);
            ++idxStaff;
        }
    }

    std::vector<GroupEngraver*>::iterator itG;
    for (itG = m_groupEngravers.begin(); itG != m_groupEngravers.end(); ++itG)
    {
        (*itG)->reposition_staves();
    }
}


//---------------------------------------------------------------------------------------
// GroupEngraver implementation
//---------------------------------------------------------------------------------------

GroupEngraver::GroupEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                             ImoInstrGroup* pGroup, ImoScore* pScore,
                             PartsEngraver* pParts)
    : Engraver(libraryScope, pScoreMeter)
    , m_pGroup(pGroup)
    , m_pScore(pScore)
    , m_pParts(pParts)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_stavesTop(0.0f)
    , m_stavesBottom(0.0f)
    , m_uBracketGap(0.0f)
{
}

//---------------------------------------------------------------------------------------
GroupEngraver::~GroupEngraver()
{
}

//---------------------------------------------------------------------------------------
void GroupEngraver::measure_name_and_bracket()
{
    determine_staves_position();
    measure_name_abbrev();
    measure_brace_or_bracket();
}

//---------------------------------------------------------------------------------------
void GroupEngraver::determine_staves_position()
{
    ImoInstrument* pFirstInstr = m_pGroup->get_first_instrument();
    ImoInstrument* pLastInstr = m_pGroup->get_last_instrument();
    m_stavesTop = m_pParts->get_staff_top_position_for(pFirstInstr);
    m_stavesBottom = m_pParts->get_staff_bottom_position_for(pLastInstr);
}

////---------------------------------------------------------------------------------------
void GroupEngraver::measure_name_abbrev()
{
    LUnits uSpaceAfterName = tenths_to_logical(LOMSE_INSTR_SPACE_AFTER_NAME);
    if (m_pGroup->has_name())
    {
        ImoScoreText& text = m_pGroup->get_name();
        ImoStyle* pStyle = text.get_style();
        if (!pStyle)
            pStyle = m_pScore->get_default_style();
        TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                          text.get_language(), pStyle);

        m_nameBox.width = engr.measure_width() + uSpaceAfterName;
        m_nameBox.height = engr.measure_height();
        m_nameBox.x = 0.0f;
        m_nameBox.y = m_stavesTop + (m_stavesBottom - m_stavesTop) / 2.0f;
    }
    if (m_pGroup->has_abbrev())
    {
        ImoScoreText& text = m_pGroup->get_abbrev();
        ImoStyle* pStyle = text.get_style();
        if (!pStyle)
            pStyle = m_pScore->get_default_style();
        TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                          text.get_language(), pStyle);

        m_abbrevBox.width = engr.measure_width() + uSpaceAfterName;
        m_abbrevBox.height = engr.measure_height();
        m_abbrevBox.x = 0.0f;
        m_abbrevBox.y = m_stavesTop + (m_stavesBottom - m_stavesTop) / 2.0f;
    }
}

//---------------------------------------------------------------------------------------
void GroupEngraver::measure_brace_or_bracket()
{
    if (has_brace_or_bracket())
    {
        int symbol = m_pGroup->get_symbol();

        LUnits uBracketWidth;
        if (symbol == ImoInstrGroup::k_brace)
        {
            uBracketWidth = tenths_to_logical(LOMSE_GRP_BRACE_WIDTH);
            m_uBracketGap = tenths_to_logical(LOMSE_GRP_BRACKET_GAP);
        }
        else if  (symbol == ImoInstrGroup::k_bracket)
        {
            uBracketWidth = tenths_to_logical(LOMSE_GRP_BRACKET_WIDTH);
            m_uBracketGap = tenths_to_logical(LOMSE_GRP_BRACKET_GAP);
        }
        else
        {
            uBracketWidth = tenths_to_logical(LOMSE_GRP_SQBRACKET_WIDTH);
            m_uBracketGap = tenths_to_logical(1.0f);   //0.0f;
        }

        m_bracketFirstBox.x = 0.0f;
        m_bracketFirstBox.y = m_stavesTop;
        m_bracketFirstBox.width = uBracketWidth + m_uBracketGap;
        m_bracketFirstBox.height = m_stavesBottom - m_stavesTop;
    }
}

//---------------------------------------------------------------------------------------
bool GroupEngraver::has_brace_or_bracket()
{
    int symbol = m_pGroup->get_symbol();
    return (symbol != ImoInstrGroup::k_none);
}

//---------------------------------------------------------------------------------------
void GroupEngraver::add_name_abbrev(GmoBoxSystem* pBox, int iSystem)
{
    determine_staves_position();
    LUnits yTop = m_org.y + (m_stavesBottom + m_stavesTop) / 2.0f;

    if (iSystem == 0)
    {
        if (m_pGroup->has_name())
        {
            LUnits xLeft = m_nameBox.x + pBox->get_left();

            ImoScoreText& text = m_pGroup->get_name();
            ImoStyle* pStyle = text.get_style();
            if (!pStyle)
                pStyle = m_pScore->get_default_style();
            TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                              text.get_language(), pStyle);
            GmoShape* pShape = engr.create_shape(m_pGroup, xLeft, yTop);
            pBox->add_shape(pShape, GmoShape::k_layer_staff);
        }
    }
    else
    {
        if (m_pGroup->has_abbrev())
        {
            LUnits xLeft = m_abbrevBox.x + pBox->get_left();

            ImoScoreText& text = m_pGroup->get_abbrev();
            ImoStyle* pStyle = text.get_style();
            if (!pStyle)
                pStyle = m_pScore->get_default_style();
            TextEngraver engr(m_libraryScope, m_pMeter, text.get_text(),
                              text.get_language(), pStyle);
            GmoShape* pShape = engr.create_shape(m_pGroup, xLeft, yTop);
            pBox->add_shape(pShape, GmoShape::k_layer_staff);
        }
    }
}

//---------------------------------------------------------------------------------------
void GroupEngraver::add_brace_bracket(GmoBoxSystem* pBox, int iSystem)
{
    if (has_brace_or_bracket())
    {
        determine_staves_position();
        LUnits yTop = m_stavesTop + m_org.y;
        LUnits yBottom = m_stavesBottom + m_org.y;

        LUnits xLeft, xRight;
        if (iSystem == 0)
        {
            xLeft = m_bracketFirstBox.x + m_org.x + pBox->get_left();
            xRight = xLeft + m_bracketFirstBox.width - m_uBracketGap;
        }
        else
        {
            xLeft = m_bracketOtherBox.x + m_org.x + pBox->get_left();
            xRight = xLeft + m_bracketOtherBox.width - m_uBracketGap;
        }

        GmoShape* pShape;
        ShapeId idx = 0;
        int symbol = m_pGroup->get_symbol();
        if (symbol == ImoInstrGroup::k_brace)
            pShape = LOMSE_NEW GmoShapeBrace(m_pGroup, idx, xLeft, yTop,
                                             xRight, yBottom, Color(0,0,0));
        else if (symbol == ImoInstrGroup::k_bracket)
        {
            LUnits dyHook = tenths_to_logical(LOMSE_GRP_BRACKET_HOOK);
            pShape = LOMSE_NEW GmoShapeBracket(m_pGroup, idx, xLeft, yTop, xRight,
                                               yBottom, dyHook, Color(0,0,0));
        }
        else
        {
            LUnits lineThickness = tenths_to_logical(LOMSE_GRP_SQBRACKET_LINE);
            pShape = LOMSE_NEW GmoShapeSquaredBracket(m_pGroup, idx, xLeft, yTop, xRight,
                                               yBottom, lineThickness, Color(0,0,0));
        }
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
    }
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
    , m_uBracketGap(0.0f)
    , m_stavesLeft(0.0f)
    , m_stavesWidth(0.0f)
    , m_pNextInstrEngr(nullptr)
{
    int numStaves = m_pInstr->get_num_staves();
    m_staffTop.resize(numStaves);
    m_staffBottom.resize(numStaves);
    m_lineThickness.resize(numStaves);
    m_yShifts.resize(numStaves, 0.0f);
}

//---------------------------------------------------------------------------------------
InstrumentEngraver::~InstrumentEngraver()
{
}

//---------------------------------------------------------------------------------------
int InstrumentEngraver::get_num_staves()
{
    return m_pInstr->get_num_staves();
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
        m_nameBox.y = (get_staff_top_position() + get_staff_bottom_position()) / 2.0f;
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
        m_abbrevBox.y = (get_staff_top_position() + get_staff_bottom_position()) / 2.0f;
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_brace_or_bracket()
{
    if (has_brace_or_bracket())
    {
        LUnits uBracketWidth = tenths_to_logical(LOMSE_GRP_BRACE_WIDTH);
        m_uBracketGap = tenths_to_logical(LOMSE_GRP_BRACE_GAP);

        m_bracketFirstBox.x = 0.0f;
        m_bracketFirstBox.y = get_staff_top_position();
        m_bracketFirstBox.width = uBracketWidth + m_uBracketGap;
        m_bracketFirstBox.height = get_staff_bottom_position() - get_staff_top_position();
    }
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::measure_brace_or_bracket_height()
{
    //brace/bracket shape is going to be engraved, once the system si fully engraved
    //and placed in its final position. As staves could have been moved from initial
    //computed positions, it is necessary to re-compute brace/bracket vertical position
    //and height

    m_bracketFirstBox.y = get_staff_top_position();
    m_bracketFirstBox.height = get_staff_bottom_position() - get_staff_top_position();
    m_bracketOtherBox.y = m_bracketFirstBox.y;
    m_bracketOtherBox.height = m_bracketFirstBox.height;
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
            m_nameBox.y = (get_staff_top_position() + get_staff_bottom_position()) / 2.0f;
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
            m_abbrevBox.y = (get_staff_top_position() + get_staff_bottom_position()) / 2.0f;
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
        measure_brace_or_bracket_height();

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

        GmoShape* pShape = LOMSE_NEW GmoShapeBrace(m_pInstr, 0, xLeft, yTop, xRight,
                                                   yBottom, Color(0,0,0));
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
        pShape->set_origin(m_stavesLeft + m_org.x,
                           m_org.y + m_staffTop[iStaff] + m_yShifts[iStaff]);
        pBox->add_staff_shape(pShape);
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
    int iStaff = 0;
    for (; iStaff < m_pInstr->get_num_staves(); iStaff++)
	{
        ImoStaffInfo* pStaff = m_pInstr->get_staff(iStaff);
        if (iStaff > 0)
            y += pStaff->get_staff_margin();
        m_staffTop[iStaff] = y;
        y += pStaff->get_height();
        m_staffBottom[iStaff] = y;
        m_lineThickness[iStaff] = pStaff->get_line_thickness();
    }
    return m_staffBottom[iStaff-1];
}

//---------------------------------------------------------------------------------------
void InstrumentEngraver::reposition_staff(int iStaff, LUnits yShift)
{
    m_yShifts[iStaff] = yShift;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_top_line_of_staff(int iStaff)
{
    return m_org.y + m_staffTop[iStaff] + m_lineThickness[iStaff] / 2.0f;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_bottom_line_of_staff(int iStaff)
{
    return m_org.y + m_staffBottom[iStaff] + m_yShifts[iStaff]
           + m_lineThickness[iStaff] / 2.0f;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_staves_top_line()
{
    return m_org.y + m_staffTop[0] + m_yShifts[0] + m_lineThickness[0] / 2.0f;;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_staves_bottom_line()
{
    int iStaff = m_pInstr->get_num_staves() - 1;
    return  m_org.y + m_staffBottom.back() + m_yShifts.back() - m_lineThickness[iStaff] / 2.0f;
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_barline_top()
{
    int layout = m_pInstr->get_barline_layout();
    if (layout == ImoInstrument::k_isolated || layout == ImoInstrument::k_joined)
        return get_staves_top_line();
    else if (layout == ImoInstrument::k_mensurstrich)
        return get_staves_bottom_line();
    else
        return 0.0f;    //k_nothing
}

//---------------------------------------------------------------------------------------
LUnits InstrumentEngraver::get_barline_bottom()
{
    int layout = m_pInstr->get_barline_layout();
    if (layout == ImoInstrument::k_isolated)
        return get_staves_bottom_line();
    else if (layout == ImoInstrument::k_joined || layout == ImoInstrument::k_mensurstrich)
    {
        if (m_pNextInstrEngr)
            return m_pNextInstrEngr->get_staves_top_line();
        else
            return get_staves_bottom_line();
    }
    else
        return 0.0f;    //k_nothing
}


}  //namespace lomse
