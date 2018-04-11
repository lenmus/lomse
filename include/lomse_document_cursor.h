//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#ifndef __LOMSE_DOCUMENT_CURSOR_H__
#define __LOMSE_DOCUMENT_CURSOR_H__

#include <stack>
#include "lomse_document_iterator.h"
#include "lomse_staffobjs_table.h"
#include "lomse_time.h"

using namespace std;

namespace lomse
{

//forward declarations
class Document;
class ImoObj;
class StaffObjsIterator;
class ImoScore;

//---------------------------------------------------------------------------------------
// some constants for the ID of pointed object
const ImoId k_cursor_pos_undefined =         k_no_imoid;  //-1
const ImoId k_cursor_before_start =          -2;
const ImoId k_cursor_at_end =                -3;
const ImoId k_cursor_at_end_of_child =       -4;
const ImoId k_cursor_at_empty_place =        -5;
const ImoId k_cursor_before_start_of_child = -6;
const ImoId k_cursor_at_end_of_staff =       -7;


//=======================================================================================
// Helper classes for converting TimeUnits to time code or real time
//=======================================================================================

class Timecode
{
public:
    int bar;
    int beat;
    int n16th;
    int ticks;

    Timecode(int br, int bt, int n16, int tck)
        : bar(br), beat(bt), n16th(n16), ticks(tck)
    {
    }

    Timecode() : bar(1), beat(0), n16th(0), ticks(0)
    {
    }

};

//---------------------------------------------------------------------------------------
// Helper class for computing time and time code
class TimeInfo
{
private:
    TimeUnits m_totalDuration;      //score total duration
    TimeUnits m_beatDuration;       //duration of a beat, for current time signature
                                    //  Will be 0 if not time signature yet found
    TimeUnits m_startOfBarTimepos;  //timepos at start of current measure
    TimeUnits m_curTimepos;         //current timepos
    int m_bar;                      //current bar number (1..n)

public:
    TimeInfo(TimeUnits curTimepos, TimeUnits totalDuration, TimeUnits curBeatDuration,
             TimeUnits startOfBarTimepos, int iMeasure);

    //accessors
    inline TimeUnits get_timepos() { return m_curTimepos; }
    inline TimeUnits get_current_beat_duration() { return m_beatDuration; }
    inline TimeUnits get_current_measure_start_timepos() { return m_startOfBarTimepos; }
    inline TimeUnits get_score_total_duration() { return m_totalDuration; }

    //! convert current timepos to millisecons for a given metronome speed
    long to_millisecs(int mm);

    //! determine metronome speed to force a given total duration
    float get_metronome_mm_for_lasting(long millisecs);

    //! Returns current position expressed as percentage of total score duration
    //! Returned number: 0.00 - 100.00
    float played_percentage();
    float remaining_percentage();

    //! timecode for current position
    Timecode get_timecode();

};


//=======================================================================================
// Helper classes to save cursor state
//=======================================================================================

//---------------------------------------------------------------------------------------
//base class for any cursor state class
class ElementCursorState
{
protected:
    ElementCursorState() {}

public:
    virtual ~ElementCursorState() {}

    virtual ImoId pointee_id()=0;

};

typedef std::shared_ptr<ElementCursorState>  SpElementCursorState;


//---------------------------------------------------------------------------------------
class ScoreCursorState : public ElementCursorState
{
protected:
    int     m_instr;    //instrument (0..n-1)
    int     m_staff;    //staff (0..n-1)
    int     m_measure;  //measure number (0..n-1)
	TimeUnits m_time;   //timepos
	TimeUnits m_refTime;//timepos of ref.object or 0.0f if no ref obj
    ImoId   m_id;       //id of pointed object or k_cursor_at_empty_place if none
    ImoId   m_refId;    //id of ref.object or k_cursor_at_end_of_child if at end.
    int     m_refStaff; //staff (0..n-1) of ref.object or 0 if no ref obj

    //values representing "end of score" position
    #define k_at_end_of_score 1000000
    #define k_time_at_end_of_score 100000000.0

    //values representing "before start" position
    #define k_before_start_of_score -1
    #define k_time_before_start_of_score -1.0

public:
    ScoreCursorState(int instr, int staff, int measure, TimeUnits time, ImoId id,
                     ImoId refId, TimeUnits refTime, int refStaff)
        : ElementCursorState(), m_instr(instr), m_staff(staff), m_measure(measure)
        , m_time(time), m_refTime(refTime), m_id(id), m_refId(refId)
        , m_refStaff(refStaff)
    {
    }
    ScoreCursorState()
        : ElementCursorState()
    {
        set_at_end_of_score();
    }
    ~ScoreCursorState() {}

    //mandatory overrides
    ImoId pointee_id() { return id(); }

    //getters
    inline int instrument() { return m_instr; }
    inline int staff() { return m_staff; }
    inline int measure() { return m_measure; }
    inline TimeUnits time() { return m_time; }
    inline ImoId id() { return m_id; }
    inline ImoId ref_obj_id() { return m_refId; }
    inline TimeUnits ref_obj_time() { return m_refTime; }
    inline int ref_obj_staff() { return m_refStaff; }

    //setters
    inline ScoreCursorState& instrument(int instr) { m_instr = instr; return *this; }
    inline ScoreCursorState& staff(int staff) { m_staff = staff; return *this; }
    inline ScoreCursorState& measure(int measure) { m_measure = measure; return *this; }
    inline ScoreCursorState& time(TimeUnits time) { m_time = time; return *this; }
    inline ScoreCursorState& id(ImoId id) { m_id = id; return *this; }
    inline ScoreCursorState& ref_obj_id(ImoId id) { m_refId = id; return *this; }
    inline ScoreCursorState& ref_obj_time(TimeUnits time) { m_refTime = time; return *this; }
    inline ScoreCursorState& ref_obj_staff(int iStaff) { m_refStaff = iStaff; return *this; }

    inline void set_at_end_of_score() {
        m_instr = k_at_end_of_score;
        m_staff = k_at_end_of_score;
        m_measure = k_at_end_of_score;
        m_time = k_time_at_end_of_score;
        m_id = k_cursor_at_end_of_child;
        m_refId = k_no_imoid;
        m_refTime = 0.0;
        m_refStaff = 0;
    }
    inline void set_before_start_of_score()
    {
        m_instr = k_before_start_of_score;
        m_staff = k_before_start_of_score;
        m_measure = k_before_start_of_score;
        m_time = k_time_before_start_of_score;
        m_id = k_cursor_before_start_of_child;
        m_refId = k_before_start_of_score;
        m_refTime = 0.0;
        m_refStaff = 0;
    }

    //checking position
    inline bool is_before_start_of_score() {
                        return m_id == k_cursor_before_start_of_child;
    }
    inline bool is_at_end_of_staff() {
                        return m_id == k_cursor_at_end_of_staff
                               || m_id == k_cursor_at_end_of_child;
    }
    inline bool is_at_end_of_score() {
                        return m_id == k_cursor_at_end_of_child;
    }
};

typedef std::shared_ptr<ScoreCursorState>  SpScoreCursorState;


//---------------------------------------------------------------------------------------
class DocCursorState
{
protected:
    ImoId m_id;                         //id of top level pointed object or -1 if none
    SpElementCursorState m_spState;     //delegated class state

public:
    DocCursorState(ImoId nTopLevelId, SpElementCursorState spState)
        : m_id(nTopLevelId)
        , m_spState(spState)
    {
    }

    DocCursorState() : m_id(k_no_imoid) {}

    ~DocCursorState() {}

    inline bool is_inside_terminal_node() { return m_spState.get() != nullptr; }
    inline ImoId get_parent_level_id() { return m_id; }
    inline SpElementCursorState get_delegate_state() { return m_spState; }
    ImoId pointee_id()
    {
        if (is_inside_terminal_node())
            return m_spState->pointee_id();
        else
            return m_id;
    }

};


//=======================================================================================
// Cursor clases
//=======================================================================================

//---------------------------------------------------------------------------------------
// ElementCursor: base class for any specific element cursor
class ElementCursor
{
protected:
    Document*   m_pDoc;

    ElementCursor(Document* pDoc) : m_pDoc(pDoc) {}

public:
    virtual ~ElementCursor() {}

    //interface: all operations/info refers to logical traversing, that is, traversing
    //following the logical,visual path that user would expect.

    //positioning
    inline void operator ++() { move_next(); }
    inline void operator --() { move_prev(); }
    virtual void point_to(ImoObj* pImo)=0;
    virtual void point_to(ImoId nId)=0;
//    virtual ElementCursor* enter_element() { return this; }
    virtual void move_next()=0;
    virtual void move_prev()=0;
    virtual void move_up()=0;
    virtual void move_down()=0;

    //saving/restoring state
    virtual SpElementCursorState get_state()=0;
    virtual void restore_state(SpElementCursorState spState)=0;

    //info
    virtual ImoObj* get_pointee()=0;
    virtual ImoId get_pointee_id()=0;
    //virtual bool is_at_start()=0;
    inline ImoObj* operator *() { return get_pointee(); }
    inline Document* get_document() { return m_pDoc; }
    virtual SpElementCursorState find_previous_pos_state()=0;

    //special operations for restoring cursor validity after Document modification
    virtual void reset_and_point_to(ImoId nId)=0;
    virtual void reset_and_point_after(ImoId id)=0;

    //debug
    virtual string dump_cursor()=0;
};


//---------------------------------------------------------------------------------------
// DocContentCursor
// A cursor to traverse the non-terminal nodes of a document
class DocContentCursor
{
protected:
    Document* m_pDoc;
    ImoObj* m_pCurItem;
    ImoObj* m_parent;

public:
    DocContentCursor(Document* pDoc);
    virtual ~DocContentCursor() {}

    //positioning:
    inline void operator ++() { move_next(); }
    inline void operator --() { move_prev(); }
    void point_to(ImoObj* pImo);
    void point_to(ImoId nId);
    void move_next();
    void move_prev();
    void move_up() { move_prev(); }
    void move_down() { move_next(); }

    //info
    inline ImoObj* operator *() { return get_pointee(); }
    inline Document* get_document() { return m_pDoc; }
    bool parent_is_root_node();

    //specific
    void start_of_content();
    void last_of_content();
    void to_end();
    ImoId get_prev_id();
    inline ImoObj* get_parent_object() { return m_parent; }

    //saving/restoring state: mandatory overrides
    SpElementCursorState get_state();
    void restore_state(SpElementCursorState spState);

    //access to current position: mandatory overrides
    ImoObj* get_pointee()  { return m_pCurItem; }
    ImoId get_pointee_id() { return m_pCurItem != nullptr ? m_pCurItem->get_id()
                                                       : k_cursor_at_end;
    }

    //previous state info
    SpElementCursorState find_previous_pos_state();

protected:
    void point_to_current();
    void find_parent();
    void do_move_next();
    void down_to_last();
    void do_move_prev();
    bool is_at_start();

};


//---------------------------------------------------------------------------------------
// ScoreCursor: A cursor for traversing a score
class ScoreCursor : public ElementCursor
{
protected:
    ImoScore*       m_pScore;
    ImoId           m_scoreId;
    ColStaffObjs*   m_pColStaffObjs;
    TimeUnits       m_timeStep;
    int             m_curVoice;

    //state variables
    ScoreCursorState    m_currentState;
    ColStaffObjsIterator  m_it;       //iterator pointing to ref.object

    //variables for providing time information
    TimeUnits m_totalDuration;      //score total duration
    TimeUnits m_curBeatDuration;    //duration of a beat, for current time signature
    TimeUnits m_startOfBarTimepos;  //timepos at start of current measure

public:
    ScoreCursor(Document* pDoc, ImoScore* pScore);
    virtual ~ScoreCursor();

    //mandatory overrides from ElementCursor
        //positioning
    void point_to(ImoObj* pImo);
    void point_to(ImoId nId);
    void move_next() { to_next_staffobj(); }
    void move_prev() { to_prev_staffobj(); }
    void move_up();
    void move_down();
        //saving/restoring state
    SpElementCursorState get_state();
    void restore_state(SpElementCursorState spState);
        //info
    inline ImoObj* operator *() { return staffobj(); }
    inline ImoObj* get_pointee() { return staffobj(); }
    inline ImoId get_pointee_id() { return staffobj_id(); }
    SpElementCursorState find_previous_pos_state();
        //special operations for restoring cursor validity after Document modification
    void reset_and_point_to(ImoId nId);
    void reset_and_point_after(ImoId id);

    //specific methods

    //positioning:
    void to_time(int instr, int staff, TimeUnits timepos);
    void to_state(int instr, int staff, int measure, TimeUnits time, ImoId id=k_no_imoid);
    void point_to_barline(ImoId id, int staff);
    void to_next_staffobj(bool fSkipInChord=true);
    void to_prev_staffobj(bool fSkipInChord=true);
//    //void to_start_of_instrument(int nInstr);
//    //void to_start_of_measure(int nMeasure, int nStaff);
//    void skip_clef_key_time();
    void to_measure(int measure, int instr, int staff);
    void point_to_end();

    //info for curent position
    inline int instrument() { return m_currentState.instrument(); }
    inline int measure() { return m_currentState.measure(); }
    inline int staff() { return m_currentState.staff(); }
    inline TimeUnits time() { return m_currentState.time(); }
    inline ImoId id() { return m_currentState.id(); }
    ImoStaffObj* staffobj();
    inline ImoId staffobj_id() { return m_currentState.id(); }
    inline ImoId staffobj_id_internal() { return m_currentState.ref_obj_id(); }
    inline TimeUnits ref_obj_time() { return m_currentState.ref_obj_time(); }
    inline int ref_obj_staff() { return m_currentState.ref_obj_staff(); }
    ImoObj* staffobj_internal();
    TimeInfo get_time_info();

    //boolean info. about current position
        //Score is not empty and cursor is pointing an staffobj
    inline bool is_pointing_object() { return m_currentState.id() >= 0L; }
        //Cursor is between two staffobjs, on a free time position
    inline bool is_at_empty_place() { return m_currentState.id() == k_cursor_at_empty_place; }
        //Cursor is at end of score but score is empty
    inline bool is_at_end_of_empty_score() {
            return is_at_end_of_score() && m_pColStaffObjs->num_entries() == 0;
    }
        //Cursor is at end of last staff in score. Score could be empty or not.
    inline bool is_at_end_of_score() {
        return m_currentState.id() == k_cursor_at_end_of_child;
    }
        //Cursor is at end of a staff. There could be more staves or not (end of score).
        //Score could be empty or not.
    inline bool is_at_end_of_staff() {
        return m_currentState.id() == k_cursor_at_end_of_staff
               || m_currentState.id() == k_cursor_at_end_of_child;
    }
        //at start of score (score is not empty, otherwise cursor will be at end)
    inline bool is_at_start_of_score() {
        return m_currentState.ref_obj_id() >= 0L
               && m_it == m_pColStaffObjs->begin();
    }

    //other
    inline void set_time_step(TimeUnits step) { m_timeStep = step; }
    inline void set_current_voice(int voice) { m_curVoice = voice; }
    void change_voice_to(int voice);

    //debug & unit tests
    string dump_cursor();

protected:
    //support: related to time info
    void p_determine_total_duration();
    void p_find_start_of_measure_and_time_signature();

    //support: point_to
    void p_move_iterator_to(ImoId id);

    //support: to_next_staffobj
    void p_to_next_staff();
    void p_to_next_position(bool fSkipInChord);
    void p_move_iterator_to_next();
    void p_if_chord_move_to_last_note();
    void p_skip_fwd_implicit_key_time_signatures();
    void p_to_next_in_this_staff();
    void p_advance_if_not_right_staff();
    void p_to_start_of_next_staff();
    void p_to_start_of_next_instrument();
    void p_to_start_of_staff(int instr, int staff);
    void p_forward_to_instr_with_time_not_lower_than(TimeUnits rTargetTime);
    void p_forward_to_instr_staff_with_time_not_lower_than(TimeUnits rTargetTime);
    void p_advance_to_current_voice();

    //support: to_prev_staffobj
    void p_to_prev_staff();
    void p_to_prev_position(bool fSkipInChord);
    void p_to_prev_in_this_staff(bool fSkipInChord);
    void p_move_iterator_to_prev();
    void p_go_back_if_not_right_staff();
    void p_skip_back_current_chord_if_any();
    void p_skip_back_implicit_key_time_signatures();
    void p_to_end_of_prev_staff();
    void p_to_end_of_prev_instrument();
    void p_to_end_of_staff();
    void p_to_last_object_in_staff(bool fSkipInChord);
    void p_back_to_current_voice();

    //support: other
    void p_update_state_from_iterator();
    void p_set_state_from_iterator();
    void p_set_state_as_end_of_score();
    void p_update_as_end_of_staff();
    void p_update_pointed_object();
    bool p_more_staves_in_instrument();
    bool p_more_instruments();
    void p_to_nearest_position_in_current_voice();

    //helper: checks
    inline bool p_is_iterator_at_start_of_score() { return m_it == m_pColStaffObjs->begin(); }
    bool p_is_at_start_of_staff();
    inline bool p_is_first_staff_of_current_instrument() { return m_currentState.staff() == 0; }
    inline bool p_is_first_instrument() { return m_currentState.instrument() == 0; }

    //helper: dealing with ref.object
    inline ImoStaffObj* p_iter_object() { return (*m_it)->imo_object(); }
    inline int p_iter_object_id() { return (*m_it)->element_id(); }
    inline TimeUnits p_iter_object_time() { return (*m_it)->time(); }
    inline int p_iter_object_measure() { return (*m_it)->measure(); }
    inline int p_iter_object_staff() { return (*m_it)->staff(); }
    inline int p_iter_object_instrument() { return (*m_it)->num_instrument(); }
    inline bool p_there_is_iter_object() { return m_it != m_pColStaffObjs->end() && (*m_it != nullptr); }
    inline bool p_there_is_not_iter_object() { return m_it != m_pColStaffObjs->end() || (*m_it == nullptr); }
    inline bool p_iter_is_at_end() { return m_it == m_pColStaffObjs->end(); }
    inline bool p_iter_object_is_on_measure(int measure) {
        return p_iter_object_measure() == measure;
    }
    inline bool p_iter_object_is_on_staff(int staff) {
        return p_iter_object_staff() == staff
               || p_iter_object_is_barline();
    }
    inline bool p_iter_object_is_on_instrument(int instr) {
        return p_iter_object_instrument() == instr;
    }
    inline bool p_iter_object_is_on_time(TimeUnits rTime) {
        return is_equal_time(rTime, p_iter_object_time());
    }
    TimeUnits p_iter_object_duration();
    bool p_iter_object_is_barline();
    bool p_iter_object_is_clef();
    bool p_iter_object_is_key();
    bool p_iter_object_is_time();

};


//---------------------------------------------------------------------------------------
// DocCursor
// facade object to enclose all specific cursors for traversing a document
//---------------------------------------------------------------------------------------

class DocCursor
{
protected:
    Document* m_pDoc;
    ElementCursor* m_pInnerCursor;
    DocContentCursor m_outerCursor;
    ImoId m_idJailer;

public:
    DocCursor(Document* pDoc);
    virtual ~DocCursor();

    DocCursor(DocCursor* cursor);
    DocCursor(DocCursor& cursor);

    //info
    inline ImoObj* operator *() { return get_pointee(); }
    ImoObj* get_pointee();
    ImoObj* get_parent_object();
    ImoId get_pointee_id();
    ImoId get_parent_id();
    inline bool is_inside_terminal_node() { return m_pInnerCursor != nullptr; }
//    inline bool is_at_top_level() { return m_pInnerCursor == nullptr; }
    inline Document* get_document() { return m_pDoc; }
    inline ElementCursor* get_inner_cursor() { return m_pInnerCursor; }
    DocCursorState find_previous_pos_state();

    //positioning
    inline void operator ++() { move_next(); }
    inline void operator --() { move_prev(); }
    void move_next();
    void move_prev();
    void move_up();
    void move_down();
    void enter_element();
    void exit_element();
    void point_to(ImoId nId);
    void point_to(ImoObj* pImo);
    void to_start();
    void to_end();
    void to_last_top_level();
    bool jailed_mode_in(ImoId id);
    inline void terminate_jailed_mode() { m_idJailer = k_no_imoid; }

    //special operations for restoring cursor validity after Document modification
    void reset_and_point_to(ImoId id);
    void reset_and_point_after(ImoId id);

    //saving/restoring state
    DocCursorState get_state();
    void restore_state(DocCursorState& state);

    //debug
    string dump_cursor();
    static string id_to_string(ImoId id);

protected:
    void start_delegation();
    void stop_delegation();
    bool is_out_of_jailer(ImoObj* pImo);
    inline bool is_jailed() { return m_idJailer != k_no_imoid; }
    void do_point_to(ImoObj* pImo);

};


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_CURSOR_H__
