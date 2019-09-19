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

#include "lomse_gm_measures_table.h"

#include "lomse_internal_model.h"
#include "lomse_im_measures_table.h"
#include "lomse_box_system.h"
#include "lomse_shape_barline.h"

//std
#include <sstream>
#include <iomanip>
using namespace std;

namespace lomse
{

//=======================================================================================
//GmMeasuresTable implementation
//=======================================================================================
GmMeasuresTable::GmMeasuresTable(ImoScore* pScore)
{
    initialize_vectors(pScore);
}

//---------------------------------------------------------------------------------------
GmMeasuresTable::~GmMeasuresTable()
{
    for (size_t iInstr=0; iInstr < m_instrument.size(); ++iInstr)
    {
        if (m_instrument[iInstr])
        {
            m_instrument[iInstr]->clear();
            delete m_instrument[iInstr];
        }
    }
    m_instrument.clear();
}

//---------------------------------------------------------------------------------------
void GmMeasuresTable::initialize_vectors(ImoScore* pScore)
{
    int numInstruments = pScore->get_num_instruments();
    m_instrument.resize(numInstruments);
    m_numBarlines.assign(numInstruments, 0);

    for (int iInstr=0; iInstr < numInstruments; ++iInstr)
    {
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        if (pTable)
        {
            int numMeasures = pTable->num_entries();
            BarlinesVector* pBarlines = LOMSE_NEW BarlinesVector;
            pBarlines->assign(numMeasures, nullptr);
            m_instrument[iInstr] = pBarlines;
        }
        else
        {
            m_instrument[iInstr] = nullptr;     //empty instrument
        }
    }
}

//---------------------------------------------------------------------------------------
int GmMeasuresTable::get_num_measures(int iInstr)
{
    BarlinesVector* pBarlines = m_instrument[iInstr];
    if (pBarlines)
        return int(pBarlines->size());

    return 0;
}

//---------------------------------------------------------------------------------------
void GmMeasuresTable::finish_measure(int iInstr, GmoShapeBarline* pBarlineShape)
{
    //invoked when a non-middle barline is found

    BarlinesVector* pBarlines = m_instrument[iInstr];
    pBarlines->at(m_numBarlines[iInstr]) = pBarlineShape;
    m_numBarlines[iInstr]++;
}

//---------------------------------------------------------------------------------------
LUnits GmMeasuresTable::get_end_barline_left(int iInstr, int iMeasure,
                                             GmoBoxSystem* pBox)
{
    LUnits xLeft = pBox->get_right();
    BarlinesVector* pBarlines = m_instrument.at(iInstr);
    GmoShapeBarline* pEndShape = pBarlines->at(iMeasure);
    if (pEndShape)
        xLeft = pEndShape->get_left();

    return xLeft;
}

//---------------------------------------------------------------------------------------
LUnits GmMeasuresTable::get_start_barline_right(int iInstr, int iMeasure,
                                                GmoBoxSystem* pBox)
{
    BarlinesVector* pBarlines = m_instrument.at(iInstr);
    LUnits xRight = pBox->get_start_measure_xpos();
    if (iMeasure > 0)
    {
        int iFirstMeasure = pBox->get_first_measure(iInstr);
        if (iFirstMeasure >=0 && iMeasure > iFirstMeasure)
        {
            GmoShapeBarline* pStartShape = pBarlines->at(iMeasure-1);
            if (pStartShape)
                xRight = pStartShape->get_right();
        }
    }

    return xRight;
}

//---------------------------------------------------------------------------------------
void GmMeasuresTable::dump_gm_measures()
{
    stringstream s;
    s << endl << "GmMeasuresTable:" << endl;
    vector<BarlinesVector*>::iterator it;
    int iInstr = 0;
    for (it = m_instrument.begin(); it != m_instrument.end(); ++it, ++iInstr)
    {
        s << fixed << setprecision(2) << iInstr << ": ";
        BarlinesVector* pBarlines = *it;
        for (size_t i=0; i < pBarlines->size(); ++i)
        {
            s << (pBarlines->at(i) ? "1," : "0,");
        }
        s << endl;
    }
    dbgLogger << s.str();
}


}  //namespace lomse

