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
{
    m_pStartEntry = pEntry;
    if (pEntry != nullptr)
        m_timepos = pEntry->time();
}

//---------------------------------------------------------------------------------------
string ImMeasuresTableEntry::dump()
{
    stringstream s;
    s << m_index << "\t" << m_timepos << "\t" << m_bottomBeat << "\t"
      << m_impliedBeat << "\t";

    s << "start=" << (m_pStartEntry == nullptr ? "nullptr"
                                               : m_pStartEntry->to_string_with_ids());
    s << ", end=" << (m_pEndEntry == nullptr ? "nullptr"
                                             : m_pEndEntry->to_string_with_ids());
    s << endl;
    return s.str();
}

//---------------------------------------------------------------------------------------
ImoBarline* ImMeasuresTableEntry::get_barline()
{
    if (m_pEndEntry && m_pEndEntry->imo_object()->is_barline())
    {
        return static_cast<ImoBarline*>(m_pEndEntry->imo_object());
    }
    return nullptr;
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
    s << "meas.   time    beat    implied  objects" << endl;
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
    while (first <= last)
    {
        int guess = (first + last) / 2;
        ImMeasuresTableEntry* pEntry = m_theTable[guess];
        if (timepos >= pEntry->get_timepos())
        {
            if (guess < max)
            {
                ImMeasuresTableEntry* pNext = m_theTable[guess+1];
                if (timepos < pNext->get_timepos())
                    return pEntry;
            }
            else
                return pEntry;
        }
        if (timepos < pEntry->get_timepos())
            last = guess - 1;
        else
            first = guess + 1;
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoBarline* ImMeasuresTable::get_barline(int iMeasure)
{
    if (get_measure(iMeasure))
        return get_measure(iMeasure)->get_barline();
    return nullptr;
}


}  //namespace lomse
