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

#ifndef __LOMSE_INSTRUMENT_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_INSTRUMENT_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

#include <vector>
#include <list>
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ImoInstrument;
class ImoInstrGroup;
class ImoInstrGroups;
class ImoScore;
class FontStorage;
class GmoBox;
class GmoBoxSystem;
class GmoShapeStaff;
class InstrumentEngraver;
class GroupEngraver;
class ScoreLayouter;
class RightAligner;

//---------------------------------------------------------------------------------------
// PartsEngraver: Responsible for laying out names and brackets/braces for
// parts (instruments) and groups of instruments.
class PartsEngraver : public Engraver
{
protected:
    ImoInstrGroups* m_pGroups;
    ImoScore* m_pScore;
    FontStorage* m_pFontStorage;
    ScoreLayouter* m_pScoreLyt;

    std::vector<GroupEngraver*> m_groupEngravers;
    std::vector<InstrumentEngraver*> m_instrEngravers;

    //spacing to use
    LUnits  m_uFirstSystemIndent;
    LUnits  m_uOtherSystemIndent;

    //helper objects
    RightAligner* m_pRightAlignerFirst;     //for first system
    RightAligner* m_pRightAlignerOther;     //for other systems

    //indexes to boxes
    vector<int> m_iInstrBracketFirst;
    vector<int> m_iInstrName;
    vector<int> m_iInstrBracketOther;
    vector<int> m_iInstrAbbrev;
    vector<int> m_iGrpBracketFirst;
    vector<int> m_iGrpName;
    vector<int> m_iGrpBracketOther;
    vector<int> m_iGrpAbbrev;

public:
    PartsEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                  ImoInstrGroups* pGroups, ImoScore* pScore, ScoreLayouter* pScoreLyt);
    ~PartsEngraver();

    inline InstrumentEngraver* get_engraver_for(int iInstr)
    {
        return m_instrEngravers[iInstr];
    }

    void decide_systems_indentation();
    inline LUnits get_first_system_indent() { return m_uFirstSystemIndent; }
    inline LUnits get_other_system_indent() { return m_uOtherSystemIndent; }

    //staves position
    void set_staves_horizontal_position(int iInstr, LUnits x, LUnits width, LUnits indent);
    void set_position_and_width_for_staves(LUnits indent, UPoint org, GmoBoxSystem* pBox);
    void set_staves_width(LUnits width);
    void reposition_staves_in_engravers(const vector<LUnits>& yShifts);

    //info about instruments
    LUnits get_staff_top_position_for(ImoInstrument* pInstr);
    LUnits get_staff_bottom_position_for(ImoInstrument* pInstr);

    //engraving
    void engrave_names_and_brackets(bool fDrawStafflines, GmoBoxSystem* pBox,
                                    int iSystem);

    //Unit Test
    inline std::vector<InstrumentEngraver*>& dbg_get_instrument_engravers() {
        return m_instrEngravers;
    }


protected:
    void create_group_engravers();
    void create_instrument_engravers();
    void delete_group_engravers();
    void delete_instrument_engravers();
    void determine_staves_vertical_position();
    void measure_groups_name_and_bracket();
    void measure_instruments_name_and_bracket();
    void save_names_and_brackets_positions();

};

//---------------------------------------------------------------------------------------
class InstrumentEngraver : public Engraver
{
protected:
    ImoInstrument* m_pInstr;
    ImoScore* m_pScore;
    FontStorage* m_pFontStorage;
    UPoint m_org;
    LUnits m_uBracketGap;

    //vertical positions are relative to SystemBox origin
    //horizontal positions are relative to 0.0
    URect m_bracketFirstBox;
    URect m_bracketOtherBox;
    URect m_nameBox;
    URect m_abbrevBox;
    LUnits m_stavesLeft;
    LUnits m_stavesWidth;

    //vertical positions are relative to SystemBox origin
    std::vector<LUnits> m_staffTop;
    std::vector<LUnits> m_staffBottom;
    std::vector<LUnits> m_lineThickness;
    std::vector<LUnits> m_yShifts;

    //next instrument engraver
    InstrumentEngraver* m_pNextInstrEngr;

public:
    InstrumentEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                       ImoInstrument* pInstr, ImoScore* pScore);
    ~InstrumentEngraver();

    void set_staves_horizontal_position(LUnits x, LUnits width, LUnits indent);
    LUnits set_staves_vertical_position(LUnits y);
    inline void set_slice_instr_origin(UPoint org) { m_org = org; }
    inline void set_staves_width(LUnits width) { m_stavesWidth = width; }
    void reposition_staff(int iStaff, LUnits yShift);

    //indents
    void measure_name_and_bracket();
    URect get_box_for_bracket() { return m_bracketFirstBox; }

    URect get_box_for_name() { return m_nameBox; }
    void set_name_final_pos(URect rect) { m_nameBox = rect; }
    void set_bracket_first_final_pos(URect rect) { m_bracketFirstBox = rect; }

    URect get_box_for_abbrev() { return m_abbrevBox; }
    void set_abbrev_final_pos(URect rect) { m_abbrevBox = rect; }
    void set_bracket_other_final_pos(URect rect) { m_bracketOtherBox = rect; }

    //staves position
    LUnits get_staff_top_position() { return m_staffTop.front() + m_yShifts.front(); }
    LUnits get_staff_bottom_position() { return m_staffBottom.back()  + m_yShifts.back(); }
    LUnits get_top_line_of_staff(int iStaff);
    LUnits get_bottom_line_of_staff(int iStaff);

    //shapes
    void add_staff_lines(GmoBoxSystem* pBox);
    void add_name_abbrev(GmoBoxSystem* pBox, int iSystem);
    void add_brace_bracket(GmoBoxSystem* pBox, int iSystem);
    inline LUnits get_staves_bottom() { return get_staff_bottom_position() + m_org.y; }
    LUnits get_staves_top_line();
    LUnits get_staves_bottom_line();
    inline LUnits get_staves_width() { return m_stavesWidth; }
    inline LUnits get_staves_left() { return m_stavesLeft; }
    inline LUnits get_staves_right() { return m_stavesLeft + m_stavesWidth; }

    //barlines' segments
    LUnits get_barline_top();
    LUnits get_barline_bottom();

    //helper
    LUnits tenths_to_logical(Tenths value, int iStaff=0);
    inline ImoInstrument* get_instrument() { return m_pInstr; }
    int get_num_staves();


protected:
    void measure_name_abbrev();
    void measure_brace_or_bracket();
    void measure_brace_or_bracket_height();
    bool has_brace_or_bracket();

    friend class PartsEngraver;
    void set_next_instrument_engraver(InstrumentEngraver* pNextEngrv) {
        m_pNextInstrEngr = pNextEngrv;
    }

};

//---------------------------------------------------------------------------------------
class GroupEngraver : public Engraver
{
protected:
    ImoInstrGroup* m_pGroup;
    ImoScore* m_pScore;
    PartsEngraver* m_pParts;
    FontStorage* m_pFontStorage;
    UPoint m_org;

    //vertical positions are relative to SystemBox origin
    LUnits m_stavesTop;
    LUnits m_stavesBottom;

    //measurements
    LUnits m_uBracketGap;

    //vertical positions are relative to SystemBox origin
    //horizontal positions are relative to 0.0
    URect m_bracketFirstBox;
    URect m_bracketOtherBox;
    URect m_nameBox;
    URect m_abbrevBox;

public:
    GroupEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                  ImoInstrGroup* pGroup, ImoScore* pScore, PartsEngraver* pParts);
    ~GroupEngraver();

    void measure_name_and_bracket();

    //boxes
    URect get_box_for_bracket() { return m_bracketFirstBox; }

    URect get_box_for_name() { return m_nameBox; }
    void set_name_final_pos(URect rect) { m_nameBox = rect; }
    void set_bracket_first_final_pos(URect rect) { m_bracketFirstBox = rect; }

    URect get_box_for_abbrev() { return m_abbrevBox; }
    void set_abbrev_final_pos(URect rect) { m_abbrevBox = rect; }
    void set_bracket_other_final_pos(URect rect) { m_bracketOtherBox = rect; }

    //shapes
    void add_name_abbrev(GmoBoxSystem* pBox, int iSystem);
    void add_brace_bracket(GmoBoxSystem* pBox, int iSystem);

    //position
    inline void set_slice_instr_origin(UPoint org) { m_org = org; }
    inline void reposition_staves() { determine_staves_position(); }

protected:
    void determine_staves_position();
    void measure_name_abbrev();
    void measure_brace_or_bracket();
    bool has_brace_or_bracket();

};

}   //namespace lomse

#endif    // __LOMSE_INSTRUMENT_ENGRAVER_H__

