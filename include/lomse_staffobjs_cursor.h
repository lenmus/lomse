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

#ifndef __LOMSE_SYSTEM_CURSOR_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_CURSOR_H__

#include "lomse_basic.h"
#include "lomse_score_iterator.h"

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
    StaffObjsIterator m_scoreIt;
    StaffObjsIterator m_savedPos;
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
    ~StaffObjsCursor();

    //positioning
    void move_next();

    //position info
    inline bool is_end() { return m_scoreIt.is_end(); }
    //bool change_of_measure();

    //access to info
    inline bool is_empty_score() { return m_fScoreIsEmpty; }
    inline TimeUnits anacrusis_missing_time() {
        return m_pColStaffObjs->anacrusis_missing_time();
    }
    int num_measures();
    TimeUnits score_total_duration();

    //access to current pointed object
    inline int num_instrument() { return (*m_scoreIt)->num_instrument(); }
    inline int staff() { return (*m_scoreIt)->staff(); }
    inline int line() { return (*m_scoreIt)->line(); }
    inline int measure() { return (*m_scoreIt)->measure(); }
    inline TimeUnits time() { return (*m_scoreIt)->time(); }
    inline ImoObj* imo_object() { return (*m_scoreIt)->imo_object(); }
    ImoStaffObj* get_staffobj();
    inline ColStaffObjsEntry* cur_entry() { return *m_scoreIt; }
    int staff_index() { return staff_index_for(num_instrument(), staff()); }

    //access next/prev object without moving cursor position
    inline ColStaffObjsEntry* next_entry() { return m_scoreIt.next(); }
    inline ColStaffObjsEntry* prev_entry() { return m_scoreIt.prev(); }
    TimeUnits next_staffobj_timepos();

    //context
    inline int get_num_instruments() { return m_numInstruments; }
    inline int get_num_lines() { return m_numLines; }
    inline int get_num_staves() { return m_numStaves; }
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

