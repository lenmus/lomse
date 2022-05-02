//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_im_measures_table.h"

#include "lomse_build_options.h"
#include "lomse_staffobjs_table.h"


#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// ImMeasuresTableEntry implementation
//=======================================================================================
ImMeasuresTableEntry::ImMeasuresTableEntry(ColStaffObjsEntry* pEntry)
    : m_index(-1)
    , m_timepos(LOMSE_NO_TIME)
    , m_firstId(-1)
    , m_bottomBeat(LOMSE_NO_DURATION)
    , m_impliedBeat(LOMSE_NO_DURATION)
    , m_pCsoEntry(pEntry)
{
    if (pEntry != nullptr)
        m_timepos = pEntry->time();
}

//---------------------------------------------------------------------------------------
ImMeasuresTableEntry::ImMeasuresTableEntry()
    : m_index(-1)
    , m_timepos(LOMSE_NO_TIME)
    , m_firstId(-1)
    , m_bottomBeat(LOMSE_NO_DURATION)
    , m_impliedBeat(LOMSE_NO_DURATION)
    , m_pCsoEntry(nullptr)
{
}

//---------------------------------------------------------------------------------------
string ImMeasuresTableEntry::dump()
{
    stringstream s;
    s << m_index << "\t" << m_timepos << "\t" << m_bottomBeat << "\t"
      << m_impliedBeat << "\t";

    if (m_pCsoEntry != nullptr)
        s << m_pCsoEntry->to_string_with_ids();
    s << endl;
    return s.str();
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
ImMeasuresTableEntry* ImMeasuresTable::add_entry(ColStaffObjsEntry* pCsoEntry)
{
    ImMeasuresTableEntry* pEntry =
        LOMSE_NEW ImMeasuresTableEntry(pCsoEntry);
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
    s << "ImMeasuresTable. Num.entries = " << num_entries() << endl;
    //    +.......+.......+.......+.......+.......+.......+
    s << "meas.   time    beat    object" << endl;
    s << "--------------------------------------------" << endl;
    for (it=m_theTable.begin(); it != m_theTable.end(); ++it)
    {
        s << (*it)->dump();
    }
    return s.str();
}

//---------------------------------------------------------------------------------------
ImMeasuresTableEntry* ImMeasuresTable::get_measure_at(TimeUnits timepos)
{
    //Binary search in table for measure containing the requested timepos.
    //Returns nullptr if no measures or negative timepos.
    //If timepos > last measure time, always returns last measure.

    if (m_theTable.size() == 0 || timepos < 0.0)
        return nullptr;

    int first = 0;
    int last = int(m_theTable.size() - 1);
    int const max = last;
    //cout << "looking for=" << timepos << "--------------------------------------" << endl;
    while (first <= last)
    {
        int guess = (first + last) / 2;
        //cout << "first=" << first << ", last=" << last << ", guess=" << guess << endl;
        ImMeasuresTableEntry* pEntry = m_theTable[guess];
        if (timepos >= pEntry->get_timepos())
        {
            if (guess < max)
            {
                ImMeasuresTableEntry* pNext = m_theTable[guess+1];
                if (timepos < pNext->get_timepos())
                {
                    //cout << "Found: in measure " << guess << endl;
                    return pEntry;
                }
            }
            else
            {
                //cout << "Found: in last measure " << guess << " or above" << endl;
                return pEntry;
            }
        }
        if (timepos < pEntry->get_timepos())
            last = guess - 1;
        else
            first = guess + 1;
    }

    return nullptr;
}


}  //namespace lomse
