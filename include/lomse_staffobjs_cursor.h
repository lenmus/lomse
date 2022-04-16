//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SYSTEM_CURSOR_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_CURSOR_H__

#include "lomse_basic.h"
#include "lomse_staffobjs_table.h"

namespace lomse
{

//forward declarations
class ImoScore;
class ImoBarline;
class ImoClef;
class ImoKeySignature;
class ImoTimeSignature;
class ColStaffObjsEntry;

//-----------------------------------------------------------------------------------------
// StaffObjsCursor
// A cursor for traversing staff objects by timepos, all instruments in parallel. As the
// score is traversed, StaffObjsCursor provides information about context (current clef,
// key and time signature)
//-----------------------------------------------------------------------------------------

//#define _LOMSE_NO_BREAK_TIME  100000000000000.0f         //any too big value

class StaffObjsCursor
{
private:
    ColStaffObjs* m_pColStaffObjs;
    ColStaffObjsIterator m_it;
    ColStaffObjsIterator m_savedPos;
    int m_numInstruments;
    int m_numLines;
    int m_numStaves;
    bool m_fScoreIsEmpty;
    ImoBarline* m_pLastBarline;
    std::vector<int> m_staffIndex;
    std::vector<ColStaffObjsEntry*> m_clefs;
    std::vector<ColStaffObjsEntry*> m_keys;
    std::vector<ColStaffObjsEntry*> m_times;
    std::vector<int> m_octave_shifts;

public:
    StaffObjsCursor(ImoScore* pScore);

    //positioning
    void move_next();

    //position info
    inline bool is_end() { return m_it == m_pColStaffObjs->end(); }
    //bool change_of_measure();

    //access to info
    inline bool is_empty_score() { return m_fScoreIsEmpty; }
    inline TimeUnits anacrusis_missing_time() {
        return m_pColStaffObjs->anacrusis_missing_time();
    }
    inline TimeUnits anacrusis_extra_time() {
        return m_pColStaffObjs->anacrusis_extra_time();
    }
    int num_measures();
    TimeUnits score_total_duration();

    //access to current pointed object
    inline int num_instrument() { return (*m_it)->num_instrument(); }
    inline int staff() { return (*m_it)->staff(); }
    inline int line() { return (*m_it)->line(); }
    inline int measure() { return (*m_it)->measure(); }
    inline TimeUnits time() { return (*m_it)->time(); }
    inline ImoObj* imo_object() { return (*m_it)->imo_object(); }
    ImoStaffObj* get_staffobj();
    inline ColStaffObjsEntry* cur_entry() { return *m_it; }
    int staff_index() { return staff_index_for(num_instrument(), staff()); }

    //access next/prev object without moving cursor position
    inline ColStaffObjsEntry* next_entry() { return m_it.next(); }
    inline ColStaffObjsEntry* prev_entry() { return m_it.prev(); }
    TimeUnits next_staffobj_timepos();

    //context
    inline int get_num_instruments() { return m_numInstruments; }
    inline int get_num_lines() { return m_numLines; }
    inline int get_num_staves() { return m_numStaves; }
    int num_staves_for_instrument(int iInstr);
    ImoClef* get_clef_for_instr_staff(int iInstr, int iStaff);
    ImoClef* get_applicable_clef();
    ImoKeySignature* get_key_for_instr_staff(int iInstr, int iStaff);
    ImoKeySignature* get_applicable_key();
    ImoTimeSignature* get_applicable_time_signature();
    ImoTimeSignature* get_time_signature_for_instrument(int iInstr);
    int get_applicable_clef_type();
    int get_applicable_octave_shift();
    int get_clef_type_for_instr_staff(int iInstr, int iStaff);
    int get_applicable_key_type();
    int get_key_type_for_instr_staff(int iInstr, int iStaff);
    ColStaffObjsEntry* get_clef_entry_for_instr_staff(int iInstr, int iStaff);
    ColStaffObjsEntry* get_key_entry_for_instr_staff(int iInstr, int iStaff);
    ColStaffObjsEntry* get_time_entry_for_instrument(int iInstr);
    ColStaffObjsEntry* get_prolog_time_entry_for_instrument(int iInstr);
    std::vector<int> get_applicable_clefs_for_instrument(int iInstr);
    inline ImoBarline* get_previous_barline() { return m_pLastBarline; }

    //helper
    void staff_index_to_instr_staff(int idx, int* iInstr, int* iStaff);
    inline int staff_index_for(int iInstr, int iStaff) {
        return m_staffIndex[iInstr] + iStaff;
    }

    //iterators management
    void go_back_to_saved_position();
    void save_position();

protected:
    void initialize_clefs_keys_times(ImoScore* pScore);
    void save_clef();
    void save_key_signature();
    void save_time_signature();
    void save_barline();
    void save_octave_shift_at_start(ImoStaffObj* pSO);
    void save_octave_shift_at_end(ImoStaffObj* pSO);

};



}   //namespace lomse

#endif    // __LOMSE_SYSTEM_CURSOR_H__

