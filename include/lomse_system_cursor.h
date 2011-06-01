//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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
// SystemCursor
// A cursor for traversing the score by tiempos, all instruments in parallel. As the score
// is traversed, SystemCursor provides information about context (current clef, key and
// time signature)
//-----------------------------------------------------------------------------------------

//#define _LOMSE_NO_BREAK_TIME  100000000000000.0f         //any too big value

class SystemCursor
{
private:
    ScoreIterator m_scoreIt;
    ScoreIterator m_savedPos;
    int m_numInstruments;
    int m_numLines;
    bool m_fScoreIsEmpty;
    ImoBarline* m_pLastBarline;
    std::vector<int> m_staffIndex;
    std::vector<ColStaffObjsEntry*> m_clefs;
    std::vector<ColStaffObjsEntry*> m_keys;
    std::vector<ColStaffObjsEntry*> m_times;

    //std::vector<ScoreIterator*> m_iterators;
    //std::vector<ScoreIterator*> m_savedIterators;
    //float m_rBreakTime;       //last time to include in current column

public:
    SystemCursor(ImoScore* pScore);
    ~SystemCursor();

//    bool ThereAreObjects();
//
    //positioning
    void move_next();

    //position info
    inline bool is_end() { return m_scoreIt.is_end(); }
    //bool change_of_measure();

    //access to info
    inline int num_instrument() { return (*m_scoreIt)->num_instrument(); }
    inline int staff() { return (*m_scoreIt)->staff(); }
    inline int line() { return (*m_scoreIt)->line(); }
    inline float time() { return (*m_scoreIt)->time(); }
    inline ImoObj* imo_object() { return (*m_scoreIt)->imo_object(); }
    ImoStaffObj* get_staffobj();
    inline bool is_empty_score() { return m_fScoreIsEmpty; }

    //context
    inline int get_num_instruments() { return m_numInstruments; }
    inline int get_num_lines() { return m_numLines; }
    ImoClef* get_clef_for_instr_staff(int iInstr, int iStaff);
    ImoClef* get_applicable_clef();
    ImoKeySignature* get_key_for_instr_staff(int iInstr, int iStaff);
    ImoKeySignature* get_applicable_key();
    ImoTimeSignature* get_applicable_time_signature();
    ImoTimeSignature* get_time_signature_for_instrument(int iInstr);
    int get_applicable_clef_type();
    int get_clef_type_for_instr_staff(int iInstr, int iStaff);
    int get_applicable_key_type();
    int get_key_type_for_instr_staff(int iInstr, int iStaff);
    ColStaffObjsEntry* get_clef_entry_for_instr_staff(int iInstr, int iStaff);
    ColStaffObjsEntry* get_key_entry_for_instr_staff(int iInstr, int iStaff);
    ColStaffObjsEntry* get_time_entry_for_instrument(int iInstr);

    inline ImoBarline* get_previous_barline() { return m_pLastBarline; }

//    //returns current absolute measure number (1..n) for VStaff
//    int GetNumMeasure(int iInstr);
//
//    //break time
//    inline float GetBreakTime() { return m_rBreakTime; }
//    inline void SetBreakTime(float rBreakTime) { m_rBreakTime = rBreakTime; }
//
    //iterators management
//    inline ScoreIterator* get_iterator(int iInstr) { return m_iterators[iInstr]; }
    void go_back_to_saved_position();
    void save_position();
//    void AdvanceAfterTimepos(float rTimepos);

protected:
    void initialize_clefs_keys_times(ImoScore* pScore);
    void save_clef();
    void save_key_signature();
    void save_time_signature();
    void save_barline();

};



}   //namespace lomse

#endif    // __LOMSE_SYSTEM_CURSOR_H__

