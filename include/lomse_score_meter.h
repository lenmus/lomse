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
    Tenths m_rUpperLegerLinesDisplacement;
    bool m_fDrawLeftBarline;            //draw left barline joining all system staves

	std::vector<LUnits> m_lineSpace;    //spacing for each staff
    std::vector<int> m_staffIndex;

    //info about the score
    int m_numInstruments;
    int m_numStaves;
    bool m_fScoreIsEmpty;

    //info about text styles
    ImoStyle* m_tupletsStyle;
    ImoStyle* m_metronomeStyle;
    ImoStyle* m_lyricsStyle;

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
    inline Tenths get_upper_ledger_lines_displacement() { return m_rUpperLegerLinesDisplacement; }

    //spacing
    LUnits tenths_to_logical(Tenths value, int iInstr, int iStaff);
    Tenths logical_to_tenths(LUnits value, int iInstr, int iStaff);
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
    inline ImoStyle* get_metronome_style_info() { return m_metronomeStyle; }
    inline ImoStyle* get_lyrics_style_info() { return m_lyricsStyle; }


protected:
    void get_options(ImoScore* pScore);
    void get_staff_spacing(ImoScore* pScore);
    void get_styles(ImoScore* pScore);

};



}   //namespace lomse

#endif    // __LOMSE_SCORE_METER_H__

