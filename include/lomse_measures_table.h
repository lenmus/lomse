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

#ifndef __LOMSE_MEASURES_TABLE_H__
#define __LOMSE_MEASURES_TABLE_H__

//#include "lomse_document.h"
//#include "lomse_time.h"

#include <vector>
#include <ostream>
//#include <map>

using namespace std;

namespace lomse
{

class ColStaffObjsEntry;

//---------------------------------------------------------------------------------------
// ImMeasuresTableEntry: an entry in the ImMeasuresTable table
//---------------------------------------------------------------------------------------
class ImMeasuresTableEntry
{
protected:
    int m_index;        //index of this element in ImMeasuresTable
    int m_mnxIndex;     //optional index for the measure, as defined in MNX. The first
                        //measure has an index of 1. index == 0 when not defined.
    string  m_number;   //an optional textual number to be displayed for the measure.

	ColStaffObjsEntry* m_pCsoEntry;    //ptr to barline (end of this measure) or nullptr
	                                //when no end barline

public:
    ImMeasuresTableEntry(int mnxIndex, ColStaffObjsEntry* pEntry)
        : m_index(-1)
        , m_mnxIndex(mnxIndex)
        , m_pCsoEntry(pEntry)
    {
    }
    ImMeasuresTableEntry()
        : m_index(-1)
        , m_mnxIndex(0)
        , m_pCsoEntry(nullptr)
    {
    }

    //getters
    inline int get_table_index() const { return m_index; }
    inline int get_mnx_index() const { return m_mnxIndex; }
    inline const string& get_number() const { return m_number; }
    inline ColStaffObjsEntry* get_entry() const { return m_pCsoEntry; }

    //setters
    inline void set_mnx_index(int mnxIndex) { m_mnxIndex = mnxIndex; }
    inline void set_number(const string& str) { m_number = str; }
	inline void set_entry(ColStaffObjsEntry* entry) { m_pCsoEntry = entry; }

    //debug
    string dump();

protected:
    friend class ImMeasuresTable;
    inline void set_index(int index) { m_index = index; }


};


//---------------------------------------------------------------------------------------
// ImMeasuresTable: encapsulates the measures table for an instrument
// The table is implemented as a vector with its items linked in a double-linked list
//---------------------------------------------------------------------------------------

class ImMeasuresTable
{
protected:
    vector<ImMeasuresTableEntry*>  m_theTable;

public:
    ImMeasuresTable();
    ~ImMeasuresTable();

    //table info
    inline int num_entries() { return int(m_theTable.size()); }

    //table management
    ImMeasuresTableEntry* add_entry(int mnxIndex, ColStaffObjsEntry* pCsoEntry);

    //access to entries
    ImMeasuresTableEntry* get_measure(int iMeasure);

    //debug
    string dump();

protected:

};


}   //namespace lomse

#endif      //__LOMSE_MEASURES_TABLE_H__
