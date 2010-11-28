//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

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
class ScoreIterator;
class ImoScore;


//-------------------------------------------------------------------------------------
// Helper classes to save cursor state
//-------------------------------------------------------------------------------------

//base class for any cursor state class
class ElementCursorState
{
protected:

public:
    ElementCursorState() {}
    virtual ~ElementCursorState() {}
};


class ScoreCursorState : public ElementCursorState
{
protected:
    int     m_instr;    //instrument (0..n-1)
    int     m_staff;    //staff (0..n-1)
	float   m_time;     //timepos
    int     m_segment;  //segment number (0..n-1)
    long    m_id;       //id of pointed object or -1 if none

public:
    ScoreCursorState(int instr, int segment, int staff, float time, long id)
        : ElementCursorState(), m_instr(instr), m_staff(staff), m_time(time)
        , m_segment(segment), m_id(id)
    {
    }
    ~ScoreCursorState() {}

    //getters
    inline int instrument() { return m_instr; }
    inline int segment() { return m_segment; }
    inline int staff() { return m_staff; }
    inline float time() { return m_time; }
    inline long get_id() { return m_id; }

    //setters
    inline void set_time(float time) { m_time = time; }

};


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

    inline int segment() {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        return (pState ? pState->segment() : 0);
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
        return (pState ? pState->get_id() : m_id);
    }

    inline void set_time(float time) {
        ScoreCursorState* pState = dynamic_cast<ScoreCursorState*>(m_pState);
        if (pState)
            pState->set_time(time);
    }

};


//-------------------------------------------------------------------------------------
// interfaces for traversing specific elements
//-------------------------------------------------------------------------------------

class ScoreCursorInterface
{
public:
    //// move to next object but with dfferent time than current one
    //virtual void move_next_new_time()=0;
    //// move to prev object but with dfferent time than current one
    //virtual void move_prev_new_time()=0;
    ////to first obj in instr nInstr
    //virtual void to_start_of_instrument(int nInstr)=0;
    ////to first obj in specified segment and staff
    //virtual void to_start_of_segment(int nSegment, int nStaff)=0;
    //while pointing clef, key or time, move next
    virtual void skip_clef_key_time()=0;

    //access to pointed position/object
    virtual int instrument()=0;
    virtual int segment()=0;
    virtual int staff()=0;
    virtual float time()=0;
    virtual bool is_pointing_object()=0;

    //direct positioning
    virtual void point_to_barline(long nId, int nStaff)=0;
    virtual void to_state(int nInstr, int nSegment, int nStaff, float rTime)=0;

    //ScoreCursorInterface: info
    virtual bool is_at_end_of_staff()=0;
    virtual ImoObj* get_musicData_for_current_instrument()=0;

};


//-------------------------------------------------------------------------------------
// ElementCursor: base class for any specific element cursor
//-------------------------------------------------------------------------------------

class ElementCursor
{
protected:
    Document*   m_pDoc;

public:
    ElementCursor(Document* pDoc) : m_pDoc(pDoc) {}
    virtual ~ElementCursor() {}

    //positioning
    inline void operator ++() { move_next(); }
    inline void operator --() { move_prev(); }
    virtual void point_to(ImoObj* pImo)=0;
    virtual void point_to(long nId)=0;
    virtual ElementCursor* enter_element() { return this; }
    virtual void move_next()=0;
    virtual void move_prev()=0;
    virtual void reset_and_point_to(long nId)=0;

    //saving/restoring state
    virtual ElementCursorState* get_state()=0;
    virtual void restore(ElementCursorState* pState)=0;

    //info
    virtual ImoObj* get_pointee()=0;
    virtual bool is_at_start()=0;
};


//-------------------------------------------------------------------------------------
// ScoreCursor
//-------------------------------------------------------------------------------------

class ScoreCursor : public ElementCursor, public ScoreCursorInterface
{
protected:
    ImoScore*            m_pScore;
    ColStaffObjs*        m_pColStaffObjs;

    //state variables
    int                     m_nInstr;       //instrument (0..n-1)
    int				        m_nStaff;       //staff (0..n-1)
	float			        m_rTime;        //timepos
    int                     m_nSegment;     //segment number (0..n-1)
    ColStaffObjs::iterator  m_it;           //iterator pointing to ref.object

public:
    ScoreCursor(Document* pDoc, ImoScore* pScore);
    virtual ~ScoreCursor();


        // ElementCursor interface ---

    //ElementCursor interface: positioning
    void point_to(ImoObj* pImo);
    void point_to(long nId);
    void move_next();       //move to next object. time doesn't matter
    void move_prev();       //move to prev object. time doesn't matter
    void reset_and_point_to(long nId);

    //ElementCursor interface: saving/restoring state
    ElementCursorState* get_state();
    void restore(ElementCursorState* pState);

    //ElementCursor interface: info
    ImoObj* get_pointee();
    inline bool is_at_start() { return is_at_start_of_score(); }


        //ScoreCursorInterface ---

    //ScoreCursorInterface: access to pointed position/object
    inline int instrument() { return m_nInstr; }
    inline int segment() { return m_nSegment; }
    inline int staff() { return m_nStaff; }
    inline float time() { return m_rTime; }
    inline bool is_pointing_object() {
        return there_is_ref_object()
               && ref_object_is_on_instrument(m_nInstr)
               && ref_object_is_on_segment(m_nSegment)
               && ref_object_is_on_time(m_rTime)
               && ref_object_is_on_staff(m_nStaff);
    }

    //ScoreCursorInterface: move cursor
    //void move_next_new_time();
    //void move_prev_new_time();
    //void to_start_of_instrument(int nInstr);
    //void to_start_of_segment(int nSegment, int nStaff);
    void skip_clef_key_time();

    //ScoreCursorInterface: direct positioning
    void point_to_barline(long nId, int nStaff);
    void to_state(int nInstr, int nSegment, int nStaff, float rTime);

    //ScoreCursorInterface: info
    bool is_at_end_of_staff() { return m_it == m_pColStaffObjs->end(); }
    ImoObj* get_musicData_for_current_instrument();


protected:
    void start();
    void next();
    void prev();
    inline bool is_at_start_of_score() { return m_it == m_pColStaffObjs->begin(); }
    void update_state();
    bool more_staves_in_instrument();
    void to_start_of_next_staff();
    bool more_instruments();
    void to_start_of_next_instrument();

    //helper: dealing with ref.object
    inline int ref_object_id() { return (*m_it)->element_id(); }
    inline float ref_object_time() { return (*m_it)->time(); }
    inline int ref_object_segment() { return (*m_it)->segment(); }
    inline int ref_object_staff() { return (*m_it)->staff(); }
    inline int ref_object_instrument() { return (*m_it)->num_instrument(); }
    inline bool there_is_ref_object() { return m_it != m_pColStaffObjs->end() && (*m_it != NULL); }
    inline bool there_is_not_ref_object() { return m_it != m_pColStaffObjs->end() || (*m_it == NULL); }
    inline bool ref_object_is_on_segment(int segment) {
        return ref_object_segment() == segment;
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
    int determine_next_target_segment();
    void forward_to_instr_segment_with_time_not_lower_than(float rTargetTime);
    void forward_to_current_staff();
    bool find_current_staff_at_current_ref_object_time();
    void forward_to_state(int nInstr, int nSegment, int nStaff, float rTime);
    bool try_next_at_same_time();

    //helper: for move_prev
    inline bool is_first_staff_of_current_instrument() { return (m_nStaff == 0); }
    void backward_to_prev_time();
    bool try_prev_at_same_time();
    bool is_at_start_of_staff();
    void to_end_of_prev_staff();
    inline bool is_first_instrument() { return m_nInstr == 0; }
    void to_end_of_prev_instrument();
    void to_end_of_staff();

};


//-------------------------------------------------------------------------------------
// DocCursor
//-------------------------------------------------------------------------------------

class DocCursor : public ScoreCursorInterface
{
protected:
    Document*       m_pDoc;
    ElementCursor*  m_pCursor;
    DocIterator     m_it;
    ImoObj*         m_pFirst;

public:
    DocCursor(Document* pDoc);
    virtual ~DocCursor();

    DocCursor(const DocCursor& cursor);
    DocCursor& operator= (DocCursor const& cursor);

    inline ImoObj* operator *() { return get_pointee(); }
    ImoObj* get_pointee();

//    //basic positioning
//    void reset_and_point_to(long nId);
    void start_of_content();
//    void enter_element();
//    inline void operator ++() { next(); }
//    inline void operator --() { prev(); }
    void point_to(long nId);
//
//	//info
//	inline bool is_at_end_of_child() { return is_delegating() && get_pointee() == NULL; }
//    inline ImoObj* get_top_level_element() { return *m_it; }
    inline DocIterator get_iterator() { return m_it; }
//
//    //saving/restoring state
//    DocCursorState* get_state();
//    void restore(DocCursorState* pState);

    //ScoreCursorInterface
    int instrument() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        return (pCursor ? pCursor->instrument() : 0);
    }
    int segment() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        return (pCursor ? pCursor->segment() : 0);
    }
    int staff() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        return (pCursor ? pCursor->staff() : 0);
    }
    float time() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        return (pCursor ? pCursor->time() : 0.0f);
    }
    bool is_pointing_object() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        return (pCursor ? pCursor->is_pointing_object() : true);
    }
    inline void move_next() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        if (pCursor)
            pCursor->move_next();
    }
    void move_prev() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        if (pCursor)
            pCursor->move_prev();
    }
    //void move_next_new_time() {
    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
    //    if (pCursor)
    //        pCursor->move_next_new_time();
    //}
    //void move_prev_new_time() {
    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
    //    if (pCursor)
    //        pCursor->move_prev_new_time();
    //}
    //void to_start_of_instrument(int nInstr) {
    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
    //    if (pCursor)
    //        pCursor->to_start_of_instrument(nInstr);
    //}
    //void to_start_of_segment(int nSegment, int nStaff) {
    //    ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
    //    if (pCursor)
    //        pCursor->to_start_of_segment(nSegment, nStaff);
    //}
    void skip_clef_key_time() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        if (pCursor)
            pCursor->skip_clef_key_time();
    }

    //ScoreCursorInterface: direct positioning
    void point_to_barline(long nId, int nStaff) {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        if (pCursor)
            pCursor->point_to_barline(nId, nStaff);
    }

    void to_state(int nInstr, int nSegment, int nStaff, float rTime) {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        if (pCursor)
            pCursor->to_state(nInstr, nSegment, nStaff, rTime);
    }

    //ScoreCursorInterface: info
    bool is_at_end_of_staff() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        return (pCursor ? pCursor->is_at_end_of_staff() : false);
    }

    ImoObj* get_musicData_for_current_instrument() {
        ScoreCursor* pCursor = dynamic_cast<ScoreCursor*>(m_pCursor);
        return (pCursor ? pCursor->get_musicData_for_current_instrument() : NULL);
    }

protected:
    void next();
    void prev();
    void start_delegation();
    void stop_delegation();
    inline bool is_delegating() { return m_pCursor != NULL; }

};


}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_CURSOR_H__
