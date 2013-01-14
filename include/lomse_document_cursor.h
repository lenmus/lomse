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


//=======================================================================================
// Helper classes to save cursor state
//=======================================================================================

//---------------------------------------------------------------------------------------
//base class for any cursor state class
class ElementCursorState
{
protected:

public:
    ElementCursorState() {}
    virtual ~ElementCursorState() {}
};

//---------------------------------------------------------------------------------------
class ScoreCursorState : public ElementCursorState
{
protected:
    int     m_instr;    //instrument (0..n-1)
    int     m_staff;    //staff (0..n-1)
    int     m_measure;  //measure number (0..n-1)
	float   m_time;     //timepos
    long    m_id;       //id of pointed object or -1 if none

    //values representing "end of score" position
    #define k_at_end_of_score 1000000
    #define k_time_at_end_of_score 100000000.0f

    //values representing "before start" position
    #define k_before_start_of_score -1
    #define k_time_before_start_of_score -1.0f

public:
    ScoreCursorState(int instr, int staff, int measure, float time, long id)
        : ElementCursorState(), m_instr(instr), m_staff(staff), m_measure(measure)
        , m_time(time), m_id(id)
    {
    }
    ScoreCursorState()
        : ElementCursorState()
    {
        set_at_end_of_score();
    }
    ~ScoreCursorState() {}

    //getters
    inline int instrument() { return m_instr; }
    inline int staff() { return m_staff; }
    inline int measure() { return m_measure; }
    inline float time() { return m_time; }
    inline long id() { return m_id; }

    //setters
    inline void instrument(int instr) { m_instr = instr; }
    inline void staff(int staff) { m_staff = staff; }
    inline void measure(int measure) { m_measure = measure; }
    inline void time(float time) { m_time = time; }
    inline void id(long id) { m_id = id; }
    inline void set_at_end_of_score() {
        m_instr = k_at_end_of_score;
        m_staff = k_at_end_of_score;
        m_measure = k_at_end_of_score;
        m_time = k_time_at_end_of_score;
        m_id = -1L;
    }
    inline void set_before_start_of_score()
    {
        m_instr = k_before_start_of_score;
        m_staff = k_before_start_of_score;
        m_measure = k_before_start_of_score;
        m_time = k_time_before_start_of_score;
        m_id = -1L;
    }

    //checking position
    inline bool is_at_end_of_score() { return m_instr == k_at_end_of_score; }
    inline bool is_before_start_of_score() { return m_instr == k_before_start_of_score; }
};

//---------------------------------------------------------------------------------------
class DocCursorState
{
protected:
    long                m_id;       //id of top level pointed object or -1 if none
    ElementCursorState* m_pState;   //delegated class state

public:
    DocCursorState(long nTopLevelId, ElementCursorState* pState)
        : m_id(nTopLevelId)
        , m_pState(pState)
    {
    }

    ~DocCursorState() {
        if (m_pState)
            delete m_pState;
    }

    inline bool is_delegating() { return m_pState != NULL; }
    inline long get_top_level_id() { return m_id; }
    inline ElementCursorState* get_delegate_state() { return m_pState; }

    //ScoreCursorState interface
    inline int instrument() {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        return (pState ? pState->instrument() : 0);
    }

    inline int measure() {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        return (pState ? pState->measure() : 0);
    }

    inline int staff() {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        return (pState ? pState->staff() : 0);
    }

    inline float time() {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        return (pState ? pState->time() : 0.0f);
    }

    inline long get_id() {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        return (pState ? pState->id() : m_id);
    }

    inline void set_time(float time) {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        if (pState)
            pState->time(time);
    }

};


//=======================================================================================
// interfaces for traversing specific elements
//=======================================================================================

class ScoreCursorInterface
{
public:
//    //// move to next object but with dfferent time than current one
//    //virtual void move_next_new_time()=0;
//    //// move to prev object but with dfferent time than current one
//    //virtual void move_prev_new_time()=0;
//    ////to first obj in instr nInstr
//    //virtual void to_start_of_instrument(int nInstr)=0;
//    ////to first obj in specified measure and staff
//    //virtual void to_start_of_measure(int nMeasure, int nStaff)=0;
//    //while pointing clef, key or time, move next
//    virtual void skip_clef_key_time()=0;

    //access to pointed position/object
    virtual int instrument()=0;
    virtual int measure()=0;
    virtual int staff()=0;
    virtual float time()=0;
//    virtual bool is_pointing_object()=0;

//    //direct positioning
//    virtual void point_to_barline(long nId, int nStaff)=0;
//    virtual void to_state(int nInstr, int nMeasure, int nStaff, float rTime)=0;
//
//    //ScoreCursorInterface: info
//    virtual bool is_at_end()=0;
//    virtual ImoObj* get_musicData_for_current_instrument()=0;

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

public:
    ElementCursor(Document* pDoc) : m_pDoc(pDoc) {}
    virtual ~ElementCursor() {}

//    //positioning
//    inline void operator ++() { move_next(); }
//    inline void operator --() { move_prev(); }
    virtual void point_to(ImoObj* pImo)=0;
    virtual void point_to(long nId)=0;
//    virtual ElementCursor* enter_element() { return this; }
    virtual void move_next()=0;
//    virtual void move_prev()=0;
//    virtual void reset_and_point_to(long nId)=0;
//
//    //saving/restoring state
//    virtual ElementCursorState* get_state()=0;
//    virtual void restore(ElementCursorState* pState)=0;
//
    //info
    virtual ImoObj* get_pointee()=0;
//    virtual bool is_at_start()=0;

    //special operations
    virtual void update_after_deletion()=0;
};


//---------------------------------------------------------------------------------------
// DocContentCursor
// A cursor to traverse the content (top level elements) of a document
class DocContentCursor
{
protected:
    Document* m_pDoc;
    ImoObj* m_pCurItem;

public:
    DocContentCursor(Document* pDoc);
    virtual ~DocContentCursor() {}

    //positioning
    void start_of_content();
    void last_of_content();
    inline void operator ++() { next(); }
    inline void operator --() { prev(); }
    void point_to(ImoBlockLevelObj* pImo);
    void point_to(long id);
    void point_to(ImoObj* pImo);
    inline void to_end() { m_pCurItem = NULL; }


    //access to info
    inline ImoObj* operator *() { return m_pCurItem; }

protected:
    void next();
    void prev();
    void point_to_current();

};


//---------------------------------------------------------------------------------------
class ScoreCursor : public ElementCursor, public ScoreCursorInterface
{
protected:
    long            m_scoreId;
    ColStaffObjs*   m_pColStaffObjs;

    //state variables
    ScoreCursorState    m_currentState;
    ScoreCursorState    m_prevState;
    ColStaffObjs::iterator  m_it;       //iterator pointing to ref.object
    ColStaffObjs::iterator  m_itPrev;   //iterator for previous state


public:
    ScoreCursor(Document* pDoc, ImoScore* pScore);
    virtual ~ScoreCursor();


        // ElementCursor interface ---

    //ElementCursor interface: positioning
    void point_to(long nId);
    void point_to(ImoObj* pImo);
    void move_next();       //move to next object. time doesn't matter
    void move_prev();       //move to prev object. time doesn't matter
//    void reset_and_point_to(long nId);
//
//    //ElementCursor interface: saving/restoring state
//    ElementCursorState* get_state();
//    void restore(ElementCursorState* pState);

    //ElementCursor interface: info
//    inline ImoObj* operator *() { return get_pointee(); }
//    ImoObj* get_pointee();
    inline ImoObj* operator *() { return staffobj(); }
    inline ImoObj* get_pointee() { return staffobj(); }
//    inline bool is_at_start() { return is_at_start_of_score(); }
    void update_after_deletion();


        //ScoreCursorInterface ---

    //ScoreCursorInterface: access to pointed position/object
    inline int instrument() { return is_at_end() ? -1 : m_currentState.instrument(); }
    inline int measure() { return is_at_end() ? -1 : m_currentState.measure(); }
    inline int staff() { return is_at_end() ? -1 : m_currentState.staff(); }
    inline float time() { return is_at_end() ? -1.0f : m_currentState.time(); }
    inline long id() { return is_at_end() ? -1.0f : m_currentState.id(); }
    ImoObj* staffobj();
    long staffobj_id();
    inline bool is_empty_place() { return !is_pointing_object(); }
    inline bool is_pointing_object() {
        return there_is_ref_object()
               && ref_object_is_on_instrument( m_currentState.instrument() )
               && ref_object_is_on_measure( m_currentState.measure() )
               && ref_object_is_on_time( m_currentState.time() )
               && ref_object_is_on_staff( m_currentState.staff() );
    }

//    //ScoreCursorInterface: move cursor
//    //void move_next_new_time();
//    //void move_prev_new_time();
//    //void to_start_of_instrument(int nInstr);
//    //void to_start_of_measure(int nMeasure, int nStaff);
//    void skip_clef_key_time();
//
//    //ScoreCursorInterface: direct positioning
//    void point_to_barline(long nId, int nStaff);
    void to_state(int nInstr, int nMeasure, int nStaff, float rTime, long id);

    //ScoreCursorInterface: info
    inline bool is_at_end_of_score() { return m_currentState.is_at_end_of_score(); }
    bool is_at_end() { return m_it == m_pColStaffObjs->end(); }
//    ImoObj* get_musicData_for_current_instrument();


protected:
    void start();
    void move_iterator_to_next();
    void move_iterator_to_prev();
    void move_iterator_to(long id);
    inline bool is_iterator_at_start_of_score() { return m_it == m_pColStaffObjs->begin(); }
    void update_state_from_iterator();
//    bool more_staves_in_instrument();
//    void to_start_of_next_staff();
//    bool more_instruments();
//    void to_start_of_next_instrument();
    void find_previous_state();
    inline ScoreCursorState get_current_state() { return m_currentState; }
    inline void set_previous_state(int instr, int staff, int measure, float time, long id)
    {
        m_prevState.instrument(instr);
        m_prevState.staff(staff);
        m_prevState.measure(measure);
        m_prevState.time(time);
        m_prevState.id(id);
    }
    inline void set_previous_state(ScoreCursorState& state) { m_prevState = state; }
    inline void set_current_state(int instr, int staff, int measure, float time, long id)
    {
        m_currentState.instrument(instr);
        m_currentState.staff(staff);
        m_currentState.measure(measure);
        m_currentState.time(time);
        m_currentState.id(id);
    }
    inline void set_current_state(ScoreCursorState& state) { m_currentState = state; }
    void save_current_state_as_previous_state();

    //helper: dealing with ref.object
    inline int ref_object_id() { return (*m_it)->element_id(); }
    inline float ref_object_time() { return (*m_it)->time(); }
    inline int ref_object_measure() { return (*m_it)->measure(); }
    inline int ref_object_staff() { return (*m_it)->staff(); }
    inline int ref_object_instrument() { return (*m_it)->num_instrument(); }
    inline bool there_is_ref_object() { return m_it != m_pColStaffObjs->end() && (*m_it != NULL); }
    inline bool there_is_not_ref_object() { return m_it != m_pColStaffObjs->end() || (*m_it == NULL); }
    inline bool ref_object_is_on_measure(int measure) {
        return ref_object_measure() == measure;
    }
    inline bool ref_object_is_on_staff(int staff) {
        return ref_object_staff() == staff
               || ref_object_is_barline();
    }
    inline bool ref_object_is_on_instrument(int instr) {
        return ref_object_instrument() == instr;
    }
    inline bool ref_object_is_on_time(float rTime) {
        return is_equal_time(rTime, ref_object_time());
    }
    float ref_object_duration();
    bool ref_object_is_barline();
    bool ref_object_is_clef();
    bool ref_object_is_key();
    bool ref_object_is_time();
    bool is_pointing_barline() {
        return is_pointing_object() && ref_object_is_barline();
    }

    //helper: for move_next
    void forward_to_next_time();
    float determine_next_target_time();
    int determine_next_target_measure();
    void forward_to_instr_measure_with_time_not_lower_than(float rTargetTime);
    void forward_to_current_staff();
//    bool find_current_staff_at_current_ref_object_time();
    void forward_to_state(int instr, int staff, int measure, float time);
    bool try_next_at_same_time();

    //helper: for move_prev
//    inline bool is_first_staff_of_current_instrument() { return (m_nStaff == 0); }
    void backward_to_prev_time();
//    bool try_prev_at_same_time();
//    bool is_at_start_of_staff();
//    void to_end_of_prev_staff();
//    inline bool is_first_instrument() { return m_nInstr == 0; }
//    void to_end_of_prev_instrument();
//    void to_end_of_staff();

};


//---------------------------------------------------------------------------------------
// DocCursor
//---------------------------------------------------------------------------------------

class DocCursor //: public ScoreCursorInterface
{
protected:
    Document*       m_pDoc;
    ElementCursor*  m_pInnerCursor;
    DocContentCursor m_topLevelCursor;
    ImoObj*         m_pFirst;

public:
    DocCursor(Document* pDoc);
    virtual ~DocCursor();

    DocCursor(const DocCursor& cursor);
    DocCursor& operator= (DocCursor const& cursor);

    inline ImoObj* operator *() { return get_pointee(); }
    ImoObj* get_pointee();

    //basic positioning
    inline void operator ++() { next(); }
    inline void operator --() { prev(); }
    void enter_element();
////    void reset_and_point_to(long nId);
//    void start_of_content();
    void point_to(long nId);
    void point_to(ImoObj* pImo);
    void to_end();

    //special operations
    void update_after_deletion();

////	//info
////	inline bool is_at_end_of_child() { return is_delegating() && get_pointee() == NULL; }
////    inline ImoObj* get_top_level_element() { return *m_topLevelCursor; }
//    inline DocContentIterator get_iterator() { return m_topLevelCursor; }
    inline long get_top_id() { return (*m_topLevelCursor)->get_id(); }
    inline long get_inner_id() { return m_pInnerCursor->get_pointee()->get_id(); }
////
////    //saving/restoring state
////    DocCursorState* get_state();
////    void restore(DocCursorState* pState);
//
//    //ScoreCursorInterface
//    int instrument() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        return (pCursor ? pCursor->instrument() : 0);
//    }
//    int measure() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        return (pCursor ? pCursor->measure() : 0);
//    }
//    int staff() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        return (pCursor ? pCursor->staff() : 0);
//    }
//    float time() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        return (pCursor ? pCursor->time() : 0.0f);
//    }
//    bool is_pointing_object() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        return (pCursor ? pCursor->is_pointing_object() : true);
//    }
//    inline void move_next() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        if (pCursor)
//            pCursor->move_next();
//    }
//    void move_prev() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        if (pCursor)
//            pCursor->move_prev();
//    }
//    //void move_next_new_time() {
//    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//    //    if (pCursor)
//    //        pCursor->move_next_new_time();
//    //}
//    //void move_prev_new_time() {
//    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//    //    if (pCursor)
//    //        pCursor->move_prev_new_time();
//    //}
//    //void to_start_of_instrument(int nInstr) {
//    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//    //    if (pCursor)
//    //        pCursor->to_start_of_instrument(nInstr);
//    //}
//    //void to_start_of_measure(int nMeasure, int nStaff) {
//    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//    //    if (pCursor)
//    //        pCursor->to_start_of_measure(nMeasure, nStaff);
//    //}
//    void skip_clef_key_time() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        if (pCursor)
//            pCursor->skip_clef_key_time();
//    }
//
//    //ScoreCursorInterface: direct positioning
//    void point_to_barline(long nId, int nStaff) {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        if (pCursor)
//            pCursor->point_to_barline(nId, nStaff);
//    }
//
//    void to_state(int nInstr, int nMeasure, int nStaff, float rTime) {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        if (pCursor)
//            pCursor->to_state(nInstr, nMeasure, nStaff, rTime);
//    }
//
//    //ScoreCursorInterface: info
//    bool is_at_end() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        return (pCursor ? pCursor->is_at_end() : false);
//    }
//
//    ImoObj* get_musicData_for_current_instrument() {
//        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pInnerCursor);
//        return (pCursor ? pCursor->get_musicData_for_current_instrument() : NULL);
//    }

protected:
    void next();
    void prev();
    void start_delegation();
    void stop_delegation();
    inline bool is_delegating() { return m_pInnerCursor != NULL; }
    ImoBlockLevelObj* find_block_level_parent(ImoObj* pImo);

};


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_CURSOR_H__
