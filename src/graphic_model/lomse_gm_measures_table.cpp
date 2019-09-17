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

#include "lomse_time.h"

//std
#include <sstream>
#include <iomanip>
using namespace std;

namespace lomse
{

//=======================================================================================
//GmMeasuresTable implementation
//=======================================================================================
GmMeasuresTable::GmMeasuresTable()
{
}

//---------------------------------------------------------------------------------------
GmMeasuresTable::~GmMeasuresTable()
{
    m_measures.clear();
}

////---------------------------------------------------------------------------------------
//void GmMeasuresTable::add_entries(vector<GmMeasuresTableEntry>& entries)
//{
//    int iMax = int(entries.size());
//    for (int i=0; i < iMax; ++i)
//    {
//        add_entry(entries[i]);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmMeasuresTable::add_entry(GmMeasuresTableEntry& entry)
//{
//    GmMeasuresTableEntry tPosTime = {entry.rTimepos, entry.rDuration, entry.uxPos};
//    m_measures.push_back(tPosTime);
//}
//
////---------------------------------------------------------------------------------------
//string GmMeasuresTable::dump()
//{
//    stringstream s;
//                //...........+..........+.............
//    s << endl << "     timepos        Dur          Pos" << endl;
//    vector<GmMeasuresTableEntry>::iterator it;
//    for (it = m_measures.begin(); it != m_measures.end(); ++it)
//    {
//        s << fixed << setprecision(2) << setfill(' ')
//                   << setw(11) << (*it).rTimepos
//                   << setw(11) << (*it).rDuration
//                   << setw(14) << setprecision(5) << (*it).uxPos
//                   << endl;
//    }
//    return s.str();
//}


}  //namespace lomse

