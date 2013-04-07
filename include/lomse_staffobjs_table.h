//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_STAFFOBJS_TABLE_H__
#define __LOMSE_STAFFOBJS_TABLE_H__

#include <vector>
#include <ostream>
#include <map>
#include "lomse_document.h"
#include "lomse_time.h"

using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class ImoStaffObj;
class ImoGoBackFwd;
class ImoAuxObj;
class ImoSpacer;
class ImoScore;
class ImoTimeSignature;
class ImoGoBackFwd;
class ImoMusicData;


//---------------------------------------------------------------------------------------
// ColStaffObjsEntry: an entry in the ColStaffObjs table
//---------------------------------------------------------------------------------------
class ColStaffObjsEntry
{
protected:
    int                 m_measure;
    int                 m_instr;
    int                 m_line;
    int                 m_staff;
    ImoStaffObj*        m_pImo;

    ColStaffObjsEntry*  m_pNext;    //next entry in the collection
    ColStaffObjsEntry*  m_pPrev;    //prev. entry in the collection

public:
    ColStaffObjsEntry(int measure, int instr, int line, int staff,
                      ImoStaffObj* pImo)
            : m_measure(measure), m_instr(instr), m_line(line)
            , m_staff(staff), m_pImo(pImo), m_pNext(NULL), m_pPrev(NULL) {}

    //getters
    inline int measure() const { return m_measure; }
    inline TimeUnits time() const { return m_pImo->get_time(); }
    inline int num_instrument() const { return m_instr; }
    inline int line() const { return m_line; }
    inline int staff() const { return m_staff; }
    inline ImoStaffObj* imo_object() const { return m_pImo; }
    inline long element_id() { return m_pImo->get_id(); }

    //setters
    inline void decrement_time(TimeUnits timeShift) {
        m_pImo->set_time( m_pImo->get_time() - timeShift );
    }

    //debug
    string dump();
    std::string to_string();
    std::string to_string_with_ids();

    //list structure
    inline ColStaffObjsEntry* get_next() { return m_pNext; }
    inline ColStaffObjsEntry* get_prev() { return m_pPrev; }

protected:
    friend class ColStaffObjs;
    inline void set_next(ColStaffObjsEntry* pEntry) { m_pNext = pEntry; }
    inline void set_prev(ColStaffObjsEntry* pEntry) { m_pPrev = pEntry; }


};


//---------------------------------------------------------------------------------------
// ColStaffObjs: encapsulates the staff objects collection for a score
//---------------------------------------------------------------------------------------

//typedef  vector<ColStaffObjsEntry*>::iterator      ColStaffObjsIterator;

class ColStaffObjs
{
protected:
    int m_numLines;
    int m_numEntries;
    TimeUnits m_rMissingTime;

    ColStaffObjsEntry* m_pFirst;
    ColStaffObjsEntry* m_pLast;

public:
    ColStaffObjs();
    ~ColStaffObjs();

    //table info
    inline int num_entries() { return m_numEntries; }
    inline int num_lines() { return m_numLines; }
    inline bool is_anacrusis_start() { return is_greater_time(m_rMissingTime, 0.0); }
    inline TimeUnits anacrusis_missing_time() { return m_rMissingTime; }

    //table management
    void add_entry(int measure, int instr, int voice, int staff, ImoStaffObj* pImo);
    inline void set_total_lines(int number) { m_numLines = number; }
    inline void set_anacrusis_missing_time(TimeUnits rTime) { m_rMissingTime = rTime; }
    void delete_entry_for(ImoStaffObj* pSO);
    void sort_table();

    //iterator related
    class iterator
    {
        protected:
            friend class ColStaffObjs;
            ColStaffObjsEntry* m_pCurrent;
            ColStaffObjsEntry* m_pPrev;
            ColStaffObjsEntry* m_pNext;

        public:
            iterator() : m_pCurrent(NULL), m_pPrev(NULL), m_pNext(NULL) {}
            iterator(ColStaffObjsEntry* pEntry) { init(pEntry); }
            virtual ~iterator() {}

            iterator& operator =(const iterator& it) {
                init(it.m_pCurrent); 
                return *this;
            }

	        ColStaffObjsEntry* operator *() const { return m_pCurrent; }

            iterator& operator ++() {
                init(m_pNext);
                return *this;
            }
            iterator& operator --() {
                init(m_pPrev);
                return *this;
            }
		    bool operator ==(const iterator& it) const { return m_pCurrent == it.m_pCurrent; }
		    bool operator !=(const iterator& it) const { return m_pCurrent != it.m_pCurrent; }

        protected:
            void init(ColStaffObjsEntry* pEntry)
            {
                m_pCurrent = pEntry;
                if (m_pCurrent)
                {
                    m_pPrev = m_pCurrent->get_prev();
                    m_pNext = m_pCurrent->get_next();
                }
                else
                {
                    m_pPrev = NULL;
                    m_pNext = NULL;
                }
            }
    };

	inline iterator begin() { return iterator(m_pFirst); }
	inline iterator end() { return iterator(NULL); }
    inline ColStaffObjsEntry* back() { return m_pLast; }
    inline ColStaffObjsEntry* front() { return m_pFirst; }
    inline iterator find(ImoStaffObj* pSO) { return iterator(find_entry_for(pSO)); }

    //debug
    string dump();

protected:
    void add_entry_to_list(ColStaffObjsEntry* pEntry);
    ColStaffObjsEntry* find_entry_for(ImoStaffObj* pSO);

};

typedef  ColStaffObjs::iterator      ColStaffObjsIterator;


//---------------------------------------------------------------------------------------
// StaffVoiceLineTable: algorithm assign and manage line number to voices/staves
//---------------------------------------------------------------------------------------
class StaffVoiceLineTable
{
protected:
    int                 m_lastAssignedLine;
    std::map<int, int>  m_lineForStaffVoice;    //key = 100*staff + voice
    std::vector<int>    m_firstVoiceForStaff;   //key = staff

public:
    StaffVoiceLineTable();

    int get_line_assigned_to(int nVoice, int nStaff);
    void new_instrument();
    inline int get_number_of_lines() { return m_lastAssignedLine; }

private:
    int assign_line_to(int nVoice, int nStaff);
    inline int form_key(int nVoice, int nStaff) { return 100 * nStaff + nVoice; }

};


//---------------------------------------------------------------------------------------
// ColStaffObjsBuilder: algorithm to create a ColStaffObjs table
//---------------------------------------------------------------------------------------
class ColStaffObjsBuilder
{
protected:
    ColStaffObjs*   m_pColStaffObjs;
    ImoScore*       m_pImScore;

public:
    ColStaffObjsBuilder();
    ~ColStaffObjsBuilder() {}

    ColStaffObjs* build(ImoScore* pScore);

private:
    //global counters to assign measure, timepos and staff
    int     m_nCurMeasure;
    TimeUnits   m_rCurTime;
    TimeUnits   m_rMaxSegmentTime;
    TimeUnits   m_rStartSegmentTime;
    StaffVoiceLineTable m_lines;

    void create_table();
    void set_num_lines();
    void find_voices_per_staff(int nInstr);
    void create_entries(int nInstr);
    void reset_counters();
    void prepare_for_next_instrument();
    int get_line_for(int nVoice, int nStaff);
    void determine_timepos(ImoStaffObj* pSO);
    void update_measure(ImoStaffObj* pSO);
    void update_time_counter(ImoGoBackFwd* pGBF);
    void add_entry_for_staffobj(ImoObj* pImo, int nInstr);
    void add_entries_for_key_or_time_signature(ImoObj* pImo, int nInstr);
    ImoSpacer* anchor_object(ImoAuxObj* pImo);
    void collect_anacrusis_info();
    void delete_node(ImoGoBackFwd* pGBF, ImoMusicData* pMusicData);

};


}   //namespace lomse

#endif      //__LOMSE_STAFFOBJS_TABLE_H__
