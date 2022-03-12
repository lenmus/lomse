//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class ImoStaffInfo;
class ImoStyle;


//---------------------------------------------------------------------------------------
//ScoreMeter: encapsulates the methods and values for options that are needed in many
//places for score measurements during laying out and engraving
//---------------------------------------------------------------------------------------
class ScoreMeter
{
protected:
    //options for spacing and lines breaker algorithm
    int m_renderSpacingOpts;

    //spacing algorithm params.
    float m_spacingOptForce;            //Fopt
    float m_spacingAlpha;               //alpha
    float m_spacingDmin;                //Dmin
    LUnits m_spacingSmin;               //Smin

    ESpacingMethod m_nSpacingMethod;    //fixed, proportional, etc.
    Tenths m_rSpacingValue;             //space for 'fixed' method

    //layout options
    Tenths m_rUpperLegerLinesDisplacement;
    bool m_fDrawLeftBarline;            //draw left barline joining all system staves
    bool m_fFillPageWithEmptyStaves;
    bool m_fHideStaffLines;
    long m_nJustifyLastSystem;
    long m_nTruncateStaffLines;

	std::vector<LUnits> m_lineSpace;        //spacing for each staff
	std::vector<LUnits> m_lineThickness;    //line thickness for each staff
    std::vector<int> m_staffIndex;
    LUnits m_maxLineSpace;              //spacing for greatest staff

    //info about the score
    int m_numInstruments;
    int m_numStaves;
    bool m_fScoreIsEmpty;
    ImoScore* m_pScore;


public:
    ScoreMeter(ImoScore* pScore);
    //constructor for unit testing
    ScoreMeter (ImoScore* pScore, int numInstruments, int numStaves, LUnits lineSpacing,
                float rSpacingFactor=0.547f,
                ESpacingMethod nSpacingMethod=k_spacing_proportional,
                Tenths rSpacingValue=35.0f,
                bool fDrawLeftBarline=true);

    //options
    inline float get_spacing_Fopt() { return m_spacingOptForce; }
    inline float get_spacing_alpha() { return m_spacingAlpha; }
    inline float get_spacing_dmin() { return m_spacingDmin; }
    inline float get_spacing_factor() { return m_spacingAlpha; }
	inline LUnits get_spacing_smin() { return m_spacingSmin; }
    inline int get_render_spacing_opts() { return m_renderSpacingOpts; }
    inline ESpacingMethod get_spacing_method() { return m_nSpacingMethod; }
    inline Tenths get_spacing_value() { return m_rSpacingValue; }
    inline bool is_proportional_spacing() {
        return m_nSpacingMethod == k_spacing_proportional;
    }
    inline bool must_draw_left_barline() { return m_fDrawLeftBarline; }
    inline Tenths get_upper_ledger_lines_displacement() { return m_rUpperLegerLinesDisplacement; }
    inline bool must_fill_page_with_empty_systems() { return m_fFillPageWithEmptyStaves; }
    inline bool must_hide_stafflines() { return m_fHideStaffLines; }
    inline long justify_last_system_opt() { return m_nJustifyLastSystem; }
    inline long truncate_staff_lines_opt() { return m_nTruncateStaffLines; }

    //spacing
    LUnits tenths_to_logical(Tenths value, int iInstr=0, int iStaff=0);
    LUnits tenths_to_logical_for_staff(Tenths value, int idxStaff);
    Tenths logical_to_tenths(LUnits value, int iInstr, int iStaff);
    LUnits line_spacing_for_instr_staff(int iInstr, int iStaff);
    LUnits line_thickness_for_instr_staff(int iInstr, int iStaff);
    inline LUnits tenths_to_logical_max(Tenths value)     //using biggest staff
    {
        return (value * m_maxLineSpace) / 10.0f;
    }

    //info about the score
    inline int num_instruments() { return m_numInstruments; }
    inline int num_staves() { return m_numStaves; }
    int num_lines();
    inline int staff_index(int iInstr, int iStaff) {
        return m_staffIndex[iInstr] + iStaff;
    }
    inline bool is_empty_score() { return m_fScoreIsEmpty; }

    //info about text styles
    ImoStyle* get_style_info(const string& name);

    //info about staves
    ImoStaffInfo* get_staff_info(int iInstr, int iStaff);

protected:
    void get_options(ImoScore* pScore);
    void get_staff_spacing(ImoScore* pScore);

};



}   //namespace lomse

#endif    // __LOMSE_SCORE_METER_H__

