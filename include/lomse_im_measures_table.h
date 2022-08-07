//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class ImoBarline;

//---------------------------------------------------------------------------------------
// ImMeasuresTableEntry: an entry in the ImMeasuresTable table
//---------------------------------------------------------------------------------------
class ImMeasuresTableEntry
{
protected:
    int         m_index = -1;                       //index of this element in ImMeasuresTable
	TimeUnits   m_timepos = LOMSE_NO_TIME;          //measure starts at this timepos
	ImoId       m_firstId = -1;                     //id of first note/rest in measure
    TimeUnits   m_bottomBeat = LOMSE_NO_DURATION;   //applicable TS bottom number (as note duration)
    TimeUnits   m_impliedBeat = LOMSE_NO_DURATION;  //implied beat duration for applicable TS

	ColStaffObjsEntry* m_pStartEntry = nullptr; //entry for first staffobj in this measure
	ColStaffObjsEntry* m_pEndEntry = nullptr;   //entry for barline (end of this measure) or
	                                            //nullptr when no end barline

public:
    ImMeasuresTableEntry(ColStaffObjsEntry* pEntry);
    ImMeasuresTableEntry() {};

    //getters
    inline int get_table_index() const { return m_index; }
	inline TimeUnits get_timepos() const { return m_timepos; }
	inline ImoId get_first_id() const { return m_firstId; }
	inline TimeUnits get_implied_beat_duration() const { return m_bottomBeat; }
	inline TimeUnits get_bottom_ts_beat_duration() const { return m_impliedBeat; }
    inline ColStaffObjsEntry* get_start_entry() const { return m_pStartEntry; }
    inline ColStaffObjsEntry* get_end_entry() const { return m_pEndEntry; }

    /** ptr to barline (end of this measure) or nullptr when no end barline */
    ImoBarline* get_barline();

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
	inline void set_start_entry(ColStaffObjsEntry* entry) { m_pStartEntry = entry; }
    inline void set_end_entry(ColStaffObjsEntry* pEntry) { m_pEndEntry = pEntry; }

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

    //access to barlines
    ImoBarline* get_barline(int iMeasure);

    //debug
    string dump();

protected:

};


}   //namespace lomse

#endif      //__LOMSE_MEASURES_TABLE_H__
