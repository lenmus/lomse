//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_STAFFOBJS_TABLE_H__
#define __LOMSE_STAFFOBJS_TABLE_H__

#include "private/lomse_document_p.h"
#include "lomse_time.h"

//std
#include <vector>
#include <ostream>
#include <map>

namespace lomse
{

#define LOMSE_NO_NOTE_DURATION  100000000.0f    //any too high value for note/rest

//forward declarations
class DivisionsComputer;
class ImoAuxObj;
class ImoDirection;
class ImoGoBackFwd;
class ImoGraceNote;
class ImoGraceRelObj;
class ImoMusicData;
class ImoObj;
class ImoScore;
class ImoStaffObj;
class ImoTimeSignature;


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
    ColStaffObjsEntry(int measure, int instr, int line, int staff, ImoStaffObj* pImo)
        : m_measure(measure)
        , m_instr(instr)
        , m_line(line)
        , m_staff(staff)
        , m_pImo(pImo)
        , m_pNext(nullptr)
        , m_pPrev(nullptr)
    {
        m_pImo->set_colstaffobjs_entry(this);
    }

    //getters
    inline int measure() const { return m_measure; }
    inline TimeUnits time() const { return m_pImo->get_time(); }
    inline int num_instrument() const { return m_instr; }
    inline int line() const { return m_line; }
    inline int staff() const { return m_staff; }
    inline ImoStaffObj* imo_object() const { return m_pImo; }
    inline long element_id() { return m_pImo->get_id(); }
    inline TimeUnits duration() const { return m_pImo->get_duration(); }

    //debug
    std::string dump(bool fWithIds=true);
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
class ColStaffObjs
{
protected:
    int m_numLines;
    int m_numEntries;
    TimeUnits m_rMissingTime;
    TimeUnits m_rAnacrusisExtraTime;    //extra anacrusis time introduced by grace notes
    TimeUnits m_minNoteDuration;
    int m_numHalf;
    int m_numQuarter;
    int m_numEighth;
    int m_num16th;
    int m_divisions = 480;

    ColStaffObjsEntry* m_pFirst;
    ColStaffObjsEntry* m_pLast;

public:
    ColStaffObjs();
    ~ColStaffObjs();

    //table info
    inline int num_entries() const { return m_numEntries; }
    inline int num_lines() const { return m_numLines; }
    inline bool is_anacrusis_start() const { return is_greater_time(m_rMissingTime, 0.0); }
    inline TimeUnits anacrusis_missing_time() const { return m_rMissingTime; }
    inline TimeUnits anacrusis_extra_time() const { return m_rAnacrusisExtraTime; }
    inline TimeUnits min_note_duration() const { return m_minNoteDuration; }
    inline int num_half_noterests() const { return m_numHalf; }
    inline int num_quarter_noterests() const { return m_numQuarter; }
    inline int num_eighth_noterests() const { return m_numEighth; }
    inline int num_16th_noterests() const { return m_num16th; }
    inline int get_divisions() const { return m_divisions; }

    //table management
    ColStaffObjsEntry* add_entry(int measure, int instr, int voice, int staff,
                                 ImoStaffObj* pImo);
    void delete_entry_for(ImoStaffObj* pSO);

    //iterator related
    class iterator
    {
        protected:
            friend class ColStaffObjs;
            ColStaffObjsEntry* m_pCurrent;
            ColStaffObjsEntry* m_pPrev;
            ColStaffObjsEntry* m_pNext;

        public:
            iterator() : m_pCurrent(nullptr), m_pPrev(nullptr), m_pNext(nullptr) {}

            ///AWARE: This constructor requires pEntry != nullptr.
            iterator(ColStaffObjsEntry* pEntry)
            {
                m_pCurrent = pEntry;
                if (m_pCurrent)
                {
                    m_pPrev = m_pCurrent->get_prev();
                    m_pNext = m_pCurrent->get_next();
                }
                else
                {
                    //is at end. But it is impossible to access any element!!!
                    m_pPrev = nullptr;
                    m_pNext = nullptr;
                }
            }

	        ColStaffObjsEntry* operator *() const { return m_pCurrent; }

            iterator& operator ++() {
                move_next();
                return *this;
            }
            iterator& operator --() {
                move_prev();
                return *this;
            }
		    bool operator ==(const iterator& it) const { return m_pCurrent == it.m_pCurrent; }
		    bool operator !=(const iterator& it) const { return m_pCurrent != it.m_pCurrent; }

		    //access to prev/next element without changing iterator position
		    inline ColStaffObjsEntry* next() { return m_pNext; }
		    inline ColStaffObjsEntry* prev() { return m_pPrev; }

        protected:
            void move_next()
            {
                m_pPrev = m_pCurrent;
                m_pCurrent = m_pNext;
                if (m_pCurrent)
                    m_pNext = m_pCurrent->get_next();
            }
            void move_prev()
            {
                m_pNext = m_pCurrent;
                m_pCurrent = m_pPrev;
                if (m_pCurrent)
                    m_pPrev = m_pCurrent->get_prev();
            }
    };

	inline iterator begin() { return iterator(m_pFirst); }
	inline iterator end() { return iterator(nullptr); }
    inline ColStaffObjsEntry* back() { return m_pLast; }
    inline ColStaffObjsEntry* front() { return m_pFirst; }
    inline iterator find(ImoStaffObj* pSO) { return iterator(find_entry_for(pSO)); }

    //debug
    std::string dump(bool fWithIds=true);

protected:

    friend class ColStaffObjsBuilder;
    friend class ColStaffObjsBuilderEngine;
    friend class ColStaffObjsBuilderEngine1x;
    friend class ColStaffObjsBuilderEngine2x;

    inline void set_total_lines(int number) { m_numLines = number; }
    inline void set_anacrusis_missing_time(TimeUnits rTime) { m_rMissingTime = rTime; }
    inline void set_anacrusis_extra_time(TimeUnits rTime) { m_rAnacrusisExtraTime = rTime; }
    void sort_table();
    static bool is_lower_entry(ColStaffObjsEntry* b, ColStaffObjsEntry* a);
    inline void set_min_note(TimeUnits duration) { m_minNoteDuration = duration; }
    void count_noterest(ImoNoteRest* pNR);
    inline void set_divisions(int div) { m_divisions = div; }

    void add_entry_to_list(ColStaffObjsEntry* pEntry);
    ColStaffObjsEntry* find_entry_for(ImoStaffObj* pSO);

};

typedef  ColStaffObjs::iterator      ColStaffObjsIterator;


//---------------------------------------------------------------------------------------
/** StaffVoiceLineTable: algorithm assign line number to voices/staves
    The algorithm is very simple:
    - It assigns a default voice to each staff, to be used when no voice (voice == 0)
      and for first voice defined in that staff.
    - All other voices receive a different line number
*/
class StaffVoiceLineTable
{
protected:
    int                 m_lastDefinedLine;      //number of the last defined line, initially -1
    std::vector<int>    m_firstVoiceForStaff;   //default line assigned to each staff
    std::map<int, int>  m_lineForStaffVoice;    //voice assigned to each (staff, voice) combination.
                                                //The key for the map is key = 100*staff + voice

public:
    StaffVoiceLineTable();

    int get_line_assigned_to(int nVoice, int nStaff);
    void new_instrument();
    inline int get_number_of_lines() { return m_lastDefinedLine; }

private:
    int assign_line_to(int nVoice, int nStaff);
    inline int form_key(int nVoice, int nStaff) { return 100 * nStaff + nVoice; }

};


//---------------------------------------------------------------------------------------
// ColStaffObjsBuilder: generic algorithm to create a ColStaffObjs table
//---------------------------------------------------------------------------------------
class ColStaffObjsBuilderEngine;


class ColStaffObjsBuilder
{
public:
    ColStaffObjsBuilder() {}
    virtual ~ColStaffObjsBuilder() {}

    ColStaffObjs* build(ImoScore* pScore);

protected:
    ColStaffObjsBuilderEngine* create_builder_engine(ImoScore* pScore);
};

//---------------------------------------------------------------------------------------
// ColStaffObjsBuilderEngine: specific algorithm to create a ColStaffObjs table
// Abstract class, because different LDP versions requires different algorithms
//---------------------------------------------------------------------------------------
class ColStaffObjsBuilderEngine
{
protected:
    ColStaffObjs* m_pColStaffObjs = nullptr;
    ImoScore* m_pImScore = nullptr;
    DivisionsComputer* m_pDivComputer = nullptr;    //for computing MusicXML divisions

    int         m_nCurMeasure = 0;
    TimeUnits   m_rMaxSegmentTime = 0.0;
    TimeUnits   m_rStartSegmentTime = 0.0;
    TimeUnits   m_minNoteDuration = LOMSE_NO_NOTE_DURATION;
    TimeUnits   m_gracesAnacrusisTime = 0.0;
    StaffVoiceLineTable  m_lines;
    std::vector<ColStaffObjsEntry*> m_graces;   //entries for grace notes
    std::vector<ImoNote*> m_arpeggios;          //chord base notes for arpeggiated chords


    ColStaffObjsBuilderEngine(ImoScore* pScore);

public:
    virtual ~ColStaffObjsBuilderEngine();
    ColStaffObjsBuilderEngine(const ColStaffObjsBuilderEngine&) = delete;
    ColStaffObjsBuilderEngine& operator= (const ColStaffObjsBuilderEngine&) = delete;
    ColStaffObjsBuilderEngine(ColStaffObjsBuilderEngine&&) = delete;
    ColStaffObjsBuilderEngine& operator= (ColStaffObjsBuilderEngine&&) = delete;

    ColStaffObjs* do_build();

    //debug
    std::string dump_divisions_data() const;

protected:
    virtual void initializations()=0;
    virtual void determine_timepos(ImoStaffObj* pSO)=0;
    virtual void create_entries_for_instrument(int nInstr)=0;
    virtual void prepare_for_next_instrument()=0;

    void create_table();
    void collect_anacrusis_info();
    void collect_note_rest_info(ImoNoteRest* pNR);
    int get_line_for(int nVoice, int nStaff);
    void set_num_lines();
    void add_entries_for_key_or_time_signature(ImoObj* pImo, int nInstr);
    void set_min_note_duration();
    void compute_grace_notes_playback_time();
    void process_grace_relobj(ImoGraceNote* pGrace, ImoGraceRelObj* pGRO,
                              ColStaffObjsEntry* pEntry);
    ImoNote* locate_grace_principal_note(ColStaffObjsEntry* pEntry);
    ImoNote* locate_grace_previous_note(ColStaffObjsEntry* pEntry);
    void fix_negative_playback_times();
    void compute_arpeggiated_chords_playback_time();
    void compute_divisions();

    static void save_arpeggiated_note(ImoNote* pNote, bool fBottomUp,
                                      list<ImoNote*>& chordNotes);


};


//---------------------------------------------------------------------------------------
// ColStaffObjsBuilderEngine1x: algorithm to create a ColStaffObjs table for LDP 1.x
// Notes/rests for the different voices go intermixed. goBack & goFwd elements exist.
// No longer needed for LDP but this is the model followed by MusicXML, so must be
// maintained for importing MusicXML files.
//---------------------------------------------------------------------------------------
class ColStaffObjsBuilderEngine1x : public ColStaffObjsBuilderEngine
{
protected:

public:
    ColStaffObjsBuilderEngine1x(ImoScore* pScore)
        : ColStaffObjsBuilderEngine(pScore)
        , m_rCurTime(0.0)
        , m_rCurAlignTime(0.0)
        , m_pLastBarline(nullptr)
    {
    }
    ~ColStaffObjsBuilderEngine1x() override {}

private:
    TimeUnits   m_rCurTime;
    TimeUnits   m_rCurAlignTime;
    ImoBarline* m_pLastBarline;

    //overrides for base class ColStaffObjsBuilderEngine
    void initializations() override;
    void determine_timepos(ImoStaffObj* pSO) override;
    void create_entries_for_instrument(int nInstr) override;
    void prepare_for_next_instrument() override;

    //specific
    void reset_counters();
    void update_measure(ImoStaffObj* pSO);
    void update_time_counter(ImoGoBackFwd* pGBF);
    void add_entry_for_staffobj(ImoObj* pImo, int nInstr);
    ImoDirection* anchor_object(ImoAuxObj* pImo);
    void delete_node(ImoGoBackFwd* pGBF, ImoMusicData* pMusicData);

};


//---------------------------------------------------------------------------------------
// ColStaffObjsBuilderEngine2x: algorithm to create a ColStaffObjs table for LDP 2.x
// Notes/rests for the different voices DO NOT go intermixed. Instead, voices are
// defined in sequence. goBack elements do not exist. goFwd elements exist.
// This is the model for LDP >= 2.0.
//---------------------------------------------------------------------------------------
class ColStaffObjsBuilderEngine2x : public ColStaffObjsBuilderEngine
{
protected:
    std::vector<TimeUnits> m_rCurTime;      //time for each voice
    std::vector<TimeUnits> m_rStaffTime;    //time for each staff
    std::list< std::pair<ImoStaffObj*, int> > m_pendingObjs;
    int         m_curVoice = 0;
    int         m_prevVoice = 0;
    int         m_numStaves = 1;            //in current instrument
    TimeUnits   m_rCurAlignTime = 0.0;
    TimeUnits   m_instrTime = 0.0;        //current timepos for this instrument
    ImoBarline* m_pLastBarline = nullptr;

public:
    ColStaffObjsBuilderEngine2x(ImoScore* pScore) : ColStaffObjsBuilderEngine(pScore) {}
    ~ColStaffObjsBuilderEngine2x() override {}

private:

    //overrides for base class ColStaffObjsBuilderEngine
    void initializations() override;
    void determine_timepos(ImoStaffObj* pSO) override;
    void create_entries_for_instrument(int nInstr) override;
    void prepare_for_next_instrument() override;

    //specific
    void reset_counters();
    void update_measure();
    void add_entry_for_staffobj(ImoObj* pImo, int nInstr);

};


}   //namespace lomse

#endif      //__LOMSE_STAFFOBJS_TABLE_H__
