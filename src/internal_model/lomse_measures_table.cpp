//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2018. All rights reserved.
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

#include "lomse_measures_table.h"

#include "lomse_build_options.h"


#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// ImMeasuresTableEntry implementation
//=======================================================================================
string ImMeasuresTableEntry::dump()
{
//    stringstream s;
//    s << (fWithIds ? to_string_with_ids() : to_string()) << endl;
    return string("");  //s.str();
}



//=======================================================================================
// ImMeasuresTable implementation
//=======================================================================================
ImMeasuresTable::ImMeasuresTable()
{
}

//---------------------------------------------------------------------------------------
ImMeasuresTable::~ImMeasuresTable()
{
    vector<ImMeasuresTableEntry*>::iterator it;
    for (it=m_theTable.begin(); it != m_theTable.end(); ++it)
        delete *it;
    m_theTable.clear();
}

//---------------------------------------------------------------------------------------
ImMeasuresTableEntry* ImMeasuresTable::add_entry(int mnxIndex, ColStaffObjsEntry* pCsoEntry)
{
    ImMeasuresTableEntry* pEntry =
        LOMSE_NEW ImMeasuresTableEntry(mnxIndex, pCsoEntry);
    m_theTable.push_back(pEntry);
    pEntry->set_index( int(m_theTable.size()) - 1 );
    return pEntry;
}

//---------------------------------------------------------------------------------------
ImMeasuresTableEntry* ImMeasuresTable::get_measure(int iMeasure)
{
    if (iMeasure >=0 && iMeasure < num_entries())
        return m_theTable[iMeasure];

    return nullptr;
}

//---------------------------------------------------------------------------------------
string ImMeasuresTable::dump()
{
    stringstream s;
    vector<ImMeasuresTableEntry*>::iterator it;
    s << "Num.entries = " << num_entries() << endl;
    //    +.......+.......+.......+.......+.......+.......+
    s << "instr   staff   meas.   time    line    object" << endl;
    s << "----------------------------------------------------------------" << endl;
    for (it=m_theTable.begin(); it != m_theTable.end(); ++it)
    {
        s << (*it)->dump();
    }
    return s.str();
}


}  //namespace lomse
