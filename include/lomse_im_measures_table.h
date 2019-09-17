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

#include "lomse_time.h"

#include <vector>
#include <ostream>
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
    int         m_index;            //index of this element in ImMeasuresTable
	TimeUnits   m_timepos;          //measure starts at this timepos
	ImoId       m_firstId;          //id of first note/rest in measure
    TimeUnits   m_bottomBeat;       //applicable TS bottom number (as note duration)
    TimeUnits   m_impliedBeat;      //implied beat duration for applicable TS

	ColStaffObjsEntry* m_pCsoEntry; //ptr to barline (end of this measure) or nullptr
	                                //when no end barline

public:
    ImMeasuresTableEntry(ColStaffObjsEntry* pEntry);
    ImMeasuresTableEntry();

    //getters
    inline int get_table_index() const { return m_index; }
	inline TimeUnits get_timepos() const { return m_timepos; }
	inline ImoId get_first_id() const { return m_firstId; }
	inline TimeUnits get_implied_beat_duration() const { return m_bottomBeat; }
	inline TimeUnits get_bottom_ts_beat_duration() const { return m_impliedBeat; }
    inline ColStaffObjsEntry* get_entry() const { return m_pCsoEntry; }

    //debug
    string dump();

protected:
    friend class ImMeasuresTable;
    inline void set_index(int index) { m_index = index; }

    //setters
    friend class MeasuresTableBuilder;
	inline void set_timepos(TimeUnits timepos) { m_timepos = timepos; }
	inline void set_first_id(ImoId id) { m_firstId = id; }
	inline void set_implied_beat_duration(TimeUnits duration) { m_bottomBeat = duration; }
	inline void set_bottom_ts_beat_duration(TimeUnits duration) { m_impliedBeat = duration; }
	inline void set_entry(ColStaffObjsEntry* entry) { m_pCsoEntry = entry; }

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
    ImMeasuresTableEntry* add_entry(ColStaffObjsEntry* pCsoEntry);

    //access to entries
    ImMeasuresTableEntry* get_measure(int iMeasure);
    ImMeasuresTableEntry* back() { return m_theTable.back(); }
    ImMeasuresTableEntry* front() { return m_theTable.front(); }

    //search
    ImMeasuresTableEntry* get_measure_at(TimeUnits timepos);

    //debug
    string dump();

protected:

};


}   //namespace lomse

#endif      //__LOMSE_MEASURES_TABLE_H__
