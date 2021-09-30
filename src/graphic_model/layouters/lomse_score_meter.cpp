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

#include "lomse_score_meter.h"

#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_staffobjs_table.h"

using namespace std;


namespace lomse
{


//=======================================================================================
//ScoreMeter implementation
//=======================================================================================
ScoreMeter::ScoreMeter(ImoScore* pScore)
    : m_maxLineSpace(0.0f)
    , m_numInstruments( pScore->get_num_instruments() )
    , m_pScore(pScore)
{
    get_staff_spacing(pScore);
    get_options(pScore);

    m_fScoreIsEmpty = pScore->get_staffobjs_table()->num_entries() == 0;
}

//---------------------------------------------------------------------------------------
//constructor ONLY FOR unit tests. numStaves is for each instrument
ScoreMeter::ScoreMeter(ImoScore* pScore, int numInstruments, int numStaves,
                       LUnits lineSpacing,
                       float rSpacingFactor, ESpacingMethod nSpacingMethod,
                       Tenths rSpacingValue, bool fDrawLeftBarline)
    : m_renderSpacingOpts(k_render_opt_breaker_simple)
    , m_spacingOptForce(1.4f)
    , m_spacingAlpha(rSpacingFactor)
    , m_spacingDmin(16)
    , m_spacingSmin(0.0f)
    , m_nSpacingMethod(nSpacingMethod)
    , m_rSpacingValue(rSpacingValue)
    , m_rUpperLegerLinesDisplacement(0.0f)
    , m_fDrawLeftBarline(fDrawLeftBarline)
    , m_fFillPageWithEmptyStaves(false)
    , m_fHideStaffLines(false)
    , m_nJustifyLastSystem(0L)
    , m_nTruncateStaffLines(0L)
    , m_maxLineSpace(0.0f)
    , m_numInstruments(0)
    , m_numStaves(0)
    , m_fScoreIsEmpty(false)
    , m_pScore(pScore)
{
    m_staffIndex.resize(numInstruments);
    int staves = 0;
    for (int iInstr=0; iInstr < numInstruments; ++iInstr)
    {
        m_staffIndex[iInstr] = staves;
        staves += numStaves;
        for (int iStaff=0; iStaff < numStaves; ++iStaff)
        {
            m_maxLineSpace = max(m_maxLineSpace, lineSpacing);
            m_lineSpace.push_back(lineSpacing);
            m_lineThickness.push_back(LOMSE_STAFF_LINE_THICKNESS);
        }
    }
    m_numStaves = numStaves * numInstruments;
}

//---------------------------------------------------------------------------------------
int ScoreMeter::num_lines()
{
    return m_pScore->get_staffobjs_table()->num_lines();
}

//---------------------------------------------------------------------------------------
void ScoreMeter::get_options(ImoScore* pScore)
{
    ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
    m_spacingAlpha = pOpt->get_float_value();

    pOpt = pScore->get_option("Render.SpacingFopt");
    m_spacingOptForce = pOpt->get_float_value();

    pOpt = pScore->get_option("Render.SpacingMethod");
    m_nSpacingMethod = ESpacingMethod( pOpt->get_long_value() );

    pOpt = pScore->get_option("Render.SpacingOptions");
    m_renderSpacingOpts = pOpt->get_long_value();

    pOpt = pScore->get_option("Render.SpacingValue");
    m_rSpacingValue = Tenths( pOpt->get_long_value() );

    pOpt = pScore->get_option("Staff.DrawLeftBarline");
    m_fDrawLeftBarline = pOpt->get_bool_value();

    pOpt = pScore->get_option("Staff.UpperLegerLines.Displacement");
    m_rUpperLegerLinesDisplacement = Tenths( pOpt->get_long_value() );

	m_spacingSmin = tenths_to_logical_max(LOMSE_MIN_SPACE);

    pOpt = pScore->get_option("Render.SpacingDmin");
    m_spacingDmin = pOpt->get_float_value();

	pOpt = pScore->get_option("Score.FillPageWithEmptyStaves");
    m_fFillPageWithEmptyStaves = pOpt->get_bool_value();

	pOpt = pScore->get_option("Score.JustifyLastSystem");
    m_nJustifyLastSystem = pOpt->get_long_value();

	pOpt = pScore->get_option("StaffLines.Hide");
    m_fHideStaffLines = pOpt->get_bool_value();

	pOpt = pScore->get_option("StaffLines.Truncate");
    m_nTruncateStaffLines = pOpt->get_long_value();

    LOMSE_LOG_DEBUG(Logger::k_all, "SpacingFactor=%f, SpacingFopt=%f, SpacingMethod=%d, "
        "SpacingOptions=%d, SpacingValue=%f, DrawLeftBarline=%s, "
        "UpperLegerLines.Displacement=%f, spacingDmin=%f, spacingSmin =%f",
        m_spacingAlpha, m_spacingOptForce, m_nSpacingMethod, m_renderSpacingOpts,
        m_rSpacingValue, (m_fDrawLeftBarline ? "yes" : "no"),
        m_rUpperLegerLinesDisplacement, m_spacingDmin, m_spacingSmin);
}

//---------------------------------------------------------------------------------------
void ScoreMeter::get_staff_spacing(ImoScore* pScore)
{
    int instruments = pScore->get_num_instruments();
    m_staffIndex.resize(instruments);
    int staves = 0;
    for (int iInstr=0; iInstr < instruments; ++iInstr)
    {
        m_staffIndex[iInstr] = staves;
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        int numStaves = pInstr->get_num_staves();
        staves += numStaves;
        for (int iStaff=0; iStaff < numStaves; ++iStaff)
        {
            LUnits lineSpacing = pInstr->get_line_spacing_for_staff(iStaff);
            m_lineSpace.push_back(lineSpacing);
            m_maxLineSpace = max(m_maxLineSpace, lineSpacing);

            m_lineThickness.push_back( pInstr->get_line_thickness_for_staff(iStaff) );
        }
    }
    m_numStaves = staves;
}

//---------------------------------------------------------------------------------------
ImoStaffInfo* ScoreMeter::get_staff_info(int iInstr, int iStaff)
{
    if (!m_pScore)   //AWARE: can be nullptr in unit-tests
        return nullptr;

    ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
    return (pInstr ? pInstr->get_staff(iStaff) : nullptr);
}

//---------------------------------------------------------------------------------------
ImoStyle* ScoreMeter::get_style_info(const string& name)
{
    return m_pScore->get_style_or_default(name);
}

//---------------------------------------------------------------------------------------
LUnits ScoreMeter::tenths_to_logical(Tenths value, int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return (value * m_lineSpace[idx]) / 10.0f;
}

//---------------------------------------------------------------------------------------
Tenths ScoreMeter::logical_to_tenths(LUnits value, int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return (value * 10.0f) / m_lineSpace[idx];
}

//---------------------------------------------------------------------------------------
LUnits ScoreMeter::line_spacing_for_instr_staff(int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return m_lineSpace[idx];
}

//---------------------------------------------------------------------------------------
LUnits ScoreMeter::line_thickness_for_instr_staff(int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return m_lineThickness[idx];
}


}  //namespace lomse
