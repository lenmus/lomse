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

#include "lomse_score_meter.h"

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
    : m_numInstruments( pScore->get_num_instruments() )
    , m_tupletsStyle(NULL)
{
    get_options(pScore);
    get_staff_spacing(pScore);
    get_styles(pScore);

    m_fScoreIsEmpty = pScore->get_staffobjs_table()->num_entries() == 0;
}

//---------------------------------------------------------------------------------------
ScoreMeter::ScoreMeter(int numInstruments, int numStaves, LUnits lineSpacing,
                       float rSpacingFactor, ESpacingMethod nSpacingMethod,
                       Tenths rSpacingValue, bool fDrawLeftBarline)
    : m_rSpacingFactor(rSpacingFactor)
    , m_nSpacingMethod(nSpacingMethod)
    , m_rSpacingValue(rSpacingValue)
    , m_fDrawLeftBarline(fDrawLeftBarline)
    , m_tupletsStyle(NULL)
{
    //constructor for using in unit tests. numStaves is for each instrument
    m_staffIndex.reserve(numInstruments);
    int staves = 0;
    for (int iInstr=0; iInstr < numInstruments; ++iInstr)
    {
        m_staffIndex[iInstr] = staves;
        staves += numStaves;
        for (int iStaff=0; iStaff < numStaves; ++iStaff)
            m_lineSpace.push_back(lineSpacing);
    }
    m_numStaves = numStaves * numInstruments;
}

//---------------------------------------------------------------------------------------
void ScoreMeter::get_options(ImoScore* pScore)
{
    ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
    m_rSpacingFactor = pOpt->get_float_value();

    pOpt = pScore->get_option("Render.SpacingMethod");
    m_nSpacingMethod = static_cast<ESpacingMethod>( pOpt->get_long_value() );

    pOpt = pScore->get_option("Render.SpacingValue");
    m_rSpacingValue = static_cast<Tenths>( pOpt->get_long_value() );

    pOpt = pScore->get_option("Staff.DrawLeftBarline");
    m_fDrawLeftBarline = pOpt->get_bool_value();
}

//---------------------------------------------------------------------------------------
void ScoreMeter::get_staff_spacing(ImoScore* pScore)
{
    int instruments = pScore->get_num_instruments();
    m_staffIndex.reserve(instruments);
    int staves = 0;
    for (int iInstr=0; iInstr < instruments; ++iInstr)
    {
        m_staffIndex[iInstr] = staves;
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        int numStaves = pInstr->get_num_staves();
        staves += numStaves;
        for (int iStaff=0; iStaff < numStaves; ++iStaff)
            m_lineSpace.push_back( pInstr->get_line_spacing_for_staff(iStaff) );
    }
    m_numStaves = staves;
}

//---------------------------------------------------------------------------------------
void ScoreMeter::get_styles(ImoScore* pScore)
{
    m_tupletsStyle = pScore->get_style_or_default("Tuplet numbers");
}

//---------------------------------------------------------------------------------------
LUnits ScoreMeter::tenths_to_logical(Tenths value, int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return (value * m_lineSpace[idx]) / 10.0f;
}

//---------------------------------------------------------------------------------------
LUnits ScoreMeter::line_spacing_for_instr_staff(int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return m_lineSpace[idx];
}


}  //namespace lomse
