//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SCORE_METER_H__        //to avoid nested includes
#define __LOMSE_SCORE_METER_H__

#include "lomse_basic.h"
#include "lomse_score_enums.h"

#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class ImoScore;
class ImoStyle;


//---------------------------------------------------------------------------------------
//ScoreMeter: encapsulates the methods and values for options that are needed in many
//places for score measurements during layouting and engraving
//---------------------------------------------------------------------------------------
class ScoreMeter
{
protected:
    //layout options
    float m_rSpacingFactor;             //for proportional spacing of notes
    ESpacingMethod m_nSpacingMethod;    //fixed, proportional, etc.
    Tenths m_rSpacingValue;             //space for 'fixed' method
    bool m_fDrawLeftBarline;            //draw left barline joining all system staves

	std::vector<LUnits> m_lineSpace;    //spacing for each staff
    std::vector<int> m_staffIndex;

    //info about the score
    int m_numInstruments;
    int m_numStaves;
    bool m_fScoreIsEmpty;

    //info about text styles
    ImoStyle* m_tupletsStyle;

public:
    ScoreMeter(ImoScore* pScore);
    //constructor for unit testing
    ScoreMeter (int numInstruments, int numStaves, LUnits lineSpacing,
                float rSpacingFactor=0.547f,
                ESpacingMethod nSpacingMethod=k_spacing_proportional,
                Tenths rSpacingValue=35.0f,
                bool fDrawLeftBarline=true);

    //options
    inline float get_spacing_factor() { return m_rSpacingFactor; }
    inline ESpacingMethod get_spacing_method() { return m_nSpacingMethod; }
    inline Tenths get_spacing_value() { return m_rSpacingValue; }
    inline bool is_proportional_spacing() {
        return m_nSpacingMethod == k_spacing_proportional;
    }
    inline bool must_draw_left_barline() { return m_fDrawLeftBarline; }

    //spacing
    LUnits tenths_to_logical(Tenths value, int iInstr, int iStaff);
    LUnits line_spacing_for_instr_staff(int iInstr, int iStaff);

    //info about the score
    inline int num_instruments() { return m_numInstruments; }
    inline int num_staves() { return m_numStaves; }
    inline int staff_index(int iInstr, int iStaff) {
        return m_staffIndex[iInstr] + iStaff;
    }
    inline bool is_empty_score() { return m_fScoreIsEmpty; }

    //info about text styles
    inline ImoStyle* get_tuplets_style_info() { return m_tupletsStyle; }


protected:
    void get_options(ImoScore* pScore);
    void get_staff_spacing(ImoScore* pScore);
    void get_styles(ImoScore* pScore);

};



}   //namespace lomse

#endif    // __LOMSE_SCORE_METER_H__

