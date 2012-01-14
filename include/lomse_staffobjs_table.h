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


//---------------------------------------------------------------------------------------
// ColStaffObjsEntry: an entry in the ColStaffObjs table
//---------------------------------------------------------------------------------------
class ColStaffObjsEntry
{
protected:
    int                 m_measure;
    float               m_time;
    int                 m_instr;
    int                 m_line;
    int                 m_staff;
    ImoObj*             m_pImo;
    LdpElement*         m_pElm;

public:
    ColStaffObjsEntry(int measure, float time, int instr, int line, int staff,
                      ImoObj* pImo, LdpElement* pElm)
            : m_measure(measure), m_time(time), m_instr(instr), m_line(line)
            , m_staff(staff), m_pImo(pImo), m_pElm(pElm) {}

    inline int measure() const { return m_measure; }
    inline float time() const { return m_time; }
    inline int num_instrument() const { return m_instr; }
    inline int line() const { return m_line; }
    inline int staff() const { return m_staff; }
    inline LdpElement* element() const { return m_pElm; }
    inline long element_id() const { return m_pElm->get_id(); }
    inline ImoObj* imo_object() const { return m_pImo; }

    //debug
    void dump();
    std::string to_string();
    std::string to_string_with_ids();


protected:

};


//---------------------------------------------------------------------------------------
// ColStaffObjs: encapsulates the staff objects collection for a score
//---------------------------------------------------------------------------------------
class ColStaffObjs
{
protected:
    std::vector<ColStaffObjsEntry*> m_table;
    int m_numLines;
    float m_rMissingTime;

public:
    ColStaffObjs();
    ~ColStaffObjs();

    //table info
    inline int num_entries() { return static_cast<int>(m_table.size()); }
    inline int num_lines() { return m_numLines; }
    inline bool is_anacrusis_start() { return is_greater_time(m_rMissingTime, 0.0f); }
    inline float anacrusis_missing_time() { return m_rMissingTime; }

    //table management
    void sort();
    void AddEntry(int measure, float time, int instr, int voice, int staff, ImoObj* pImo);
    inline void set_total_lines(int number) { m_numLines = number; }
    inline void set_anacrusis_missing_time(float rTime) { m_rMissingTime = rTime; }

    class iterator
    {
        protected:
            friend class ColStaffObjs;
            std::vector<ColStaffObjsEntry*>::iterator m_it;

        public:
            iterator() {}
			iterator(std::vector<ColStaffObjsEntry*>::iterator& it) { m_it = it; }
            virtual ~iterator() {}

	        ColStaffObjsEntry* operator *() const { return *m_it; }
            iterator& operator ++() { ++m_it; return *this; }
            iterator& operator --() { --m_it; return *this; }
		    bool operator ==(const iterator& it) const { return m_it == it.m_it; }
		    bool operator !=(const iterator& it) const { return m_it != it.m_it; }
    };

	iterator begin() { std::vector<ColStaffObjsEntry*>::iterator it = m_table.begin(); return iterator(it); }
	iterator end() { std::vector<ColStaffObjsEntry*>::iterator it = m_table.end(); return iterator(it); }
    inline ColStaffObjsEntry* back() { return m_table.back(); }
    inline ColStaffObjsEntry* front() { return m_table.front(); }

    //debug
    void dump();

protected:

};


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
    LdpElement*     m_pScore;
    ImoScore*       m_pImScore;

public:
    ColStaffObjsBuilder();
    ~ColStaffObjsBuilder() {}

//    ColStaffObjs* build(LdpElement* pScore, bool fSort=true);
    ColStaffObjs* build(ImoScore* pScore, bool fSort=true);
    void update(LdpElement* pScore);
    void update(ImoScore* pScore);

private:
    //global counters to assign measure, timepos and staff
    int     m_nCurSegment;
    float   m_rCurTime;
    float   m_rMaxSegmentTime;
    float   m_rStartSegmentTime;
    int     m_nCurStaff;
    StaffVoiceLineTable m_lines;

    void create_table();
    void set_num_lines();
    void find_voices_per_staff(int nInstr);
    void create_entries(int nInstr);
    void sort_table(bool fSort);
    void reset_counters();
    void prepare_for_next_instrument();
    int get_line_for(int nVoice, int nStaff);
    float determine_timepos(ImoStaffObj* pSO);
    void update_measure(ImoStaffObj* pSO);
    void update_time_counter(ImoGoBackFwd* pGBF);
    void add_entry_for_staffobj(ImoObj* pImo, int nInstr);
    void add_entries_for_key_or_time_signature(ImoObj* pImo, int nInstr);
    ImoSpacer* anchor_object(ImoAuxObj* pImo);
    void collect_anacrusis_info();

};


}   //namespace lomse

#endif      //__LOMSE_STAFFOBJS_TABLE_H__
