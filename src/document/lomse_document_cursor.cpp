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

#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_ldp_elements.h"
#include "lomse_score_iterator.h"
#include "lomse_internal_model.h"
#include "lomse_time.h"

using namespace std;

namespace lomse
{

//=======================================================================================
// DocContentCursor implementation
//=======================================================================================
DocContentCursor::DocContentCursor(Document* pDoc)
    : ElementCursor(pDoc)
    , m_pCurItem(NULL)
    , m_idPrev(k_cursor_pos_undefined)
{
    start_of_content();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::move_next()
{
    if (m_pCurItem)
    {
        m_idPrev = m_pCurItem->get_id();
        m_pCurItem = m_pCurItem->get_next_sibling();
    }
}

//---------------------------------------------------------------------------------------
void DocContentCursor::move_prev()
{
    if (m_pCurItem)
    {
        ImoObj* prev = m_pCurItem->get_prev_sibling();
        if (prev != NULL)
        {
            m_pCurItem = prev;
            find_previous();
        }
    }
    else
        last_of_content();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to_current()
{
//    if (m_curItemIndex >=0 && m_curItemIndex < m_numContentItems)
//        m_pCurItem = m_pDoc->get_content_item(m_curItemIndex);
//    else
//        m_pCurItem = NULL;
}

//---------------------------------------------------------------------------------------
void DocContentCursor::start_of_content()
{
    ImoDocument* pImoDoc = m_pDoc->get_imodoc();
    ImoContent* pContent = pImoDoc->get_content();
    m_pCurItem = pContent->get_first_child();
    m_idPrev = k_cursor_before_start;
}

//---------------------------------------------------------------------------------------
void DocContentCursor::last_of_content()
{
    ImoDocument* pImoDoc = m_pDoc->get_imodoc();
    ImoContent* pContent = pImoDoc->get_content();
    m_pCurItem = pContent->get_last_child();
    find_previous();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to(long id)
{
    point_to( m_pDoc->get_pointer_to_imo(id) );
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to(ImoObj* pImo)
{
    if (pImo && pImo->is_block_level_obj())
    {
        m_pCurItem = pImo;
        find_previous();
    }
    else
        to_end();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::update_after_deletion()
{
    if (m_idPrev >= 0L)
    {
        m_pCurItem = m_pDoc->get_pointer_to_imo(m_idPrev);
        move_next();
    }
    else if (m_idPrev == k_cursor_before_start)
        start_of_content();
    else
        to_end();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::find_previous()
{
     if (m_pCurItem)
     {
        ImoObj* pPrevItem = m_pCurItem->get_prev_sibling();
        m_idPrev = (pPrevItem != NULL ? pPrevItem->get_id() : k_cursor_before_start);
     }
     else
        m_idPrev = k_cursor_before_start;
}

//---------------------------------------------------------------------------------------
void DocContentCursor::to_end()
{
    ImoDocument* pImoDoc = m_pDoc->get_imodoc();
    ImoContent* pContent = pImoDoc->get_content();
    m_pCurItem = pContent->get_last_child();
    m_idPrev = (m_pCurItem != NULL ? m_pCurItem->get_id() : k_cursor_before_start);
    m_pCurItem = NULL;
}

//---------------------------------------------------------------------------------------
ElementCursorState* DocContentCursor::get_state()
{
    //TODO
    return NULL;
}

//---------------------------------------------------------------------------------------
void DocContentCursor::restore_state(ElementCursorState* pState)
{
    //TODO
}



//=======================================================================================
// DocCursor implementation
//=======================================================================================
DocCursor::DocCursor(Document* pDoc)
    : m_pDoc(pDoc)
    , m_pInnerCursor(NULL)
    , m_topLevelCursor(pDoc)
{
}

//---------------------------------------------------------------------------------------
DocCursor::DocCursor(const DocCursor* cursor)
    : m_pDoc(cursor->m_pDoc)
    , m_pInnerCursor(NULL)
    , m_topLevelCursor(m_pDoc)
{
//    m_pFirst = cursor.m_pFirst;
//    if (cursor.m_pInnerCursor == NULL)
//        m_pInnerCursor = NULL;
//    else
//    {
//        start_delegation();
//        DocCursor &cRef = const_cast<DocCursor&>(cursor);
//        m_pInnerCursor->point_to( cRef.get_pointee() );
//    }
}

//---------------------------------------------------------------------------------------
DocCursor::~DocCursor()
{
    if (m_pInnerCursor)
        delete m_pInnerCursor;
}

//---------------------------------------------------------------------------------------
void DocCursor::to_start()
{
    if (is_delegating())
        stop_delegation();
    m_topLevelCursor.start_of_content();
}

////---------------------------------------------------------------------------------------
//DocCursor& DocCursor::operator= (DocCursor const& cursor)
//{
//    if (this != &cursor)
//    {
//        assert(m_pDoc == cursor.m_pDoc);
//        m_topLevelCursor = cursor.m_topLevelCursor;
//        m_pFirst = cursor.m_pFirst;
//        if (cursor.m_pInnerCursor == NULL)
//            m_pInnerCursor = NULL;
//        else
//        {
//            start_delegation();
//            DocCursor &cRef = const_cast<DocCursor&>(cursor);
//            m_pInnerCursor->point_to( cRef.get_pointee() );
//        }
//    }
//    return *this;
//}

//---------------------------------------------------------------------------------------
ImoObj* DocCursor::get_pointee()
{
	return (is_delegating() ? m_pInnerCursor->get_pointee() : *m_topLevelCursor);
}

//---------------------------------------------------------------------------------------
ImoObj* DocCursor::get_top_object()
{
    return *m_topLevelCursor;
}

//---------------------------------------------------------------------------------------
long DocCursor::get_pointee_id()
{
    ImoObj* pImo = get_pointee();
    return (pImo == NULL ? k_cursor_at_end : pImo->get_id() );
}

//---------------------------------------------------------------------------------------
long DocCursor::get_top_id()
{
    ImoObj* pImo = get_top_object();
    return (pImo == NULL ? k_cursor_at_end : pImo->get_id() );
}

//---------------------------------------------------------------------------------------
void DocCursor::enter_element()
{
    if (!is_delegating())
        start_delegation();
}

//---------------------------------------------------------------------------------------
void DocCursor::start_delegation()
{
    //Factory method to create delegate cursors
    int type = (*m_topLevelCursor)->get_obj_type();
    //const string& name = ImoObj::get_name(type);

    switch(type)
    {
        case k_imo_score:
            m_pInnerCursor = LOMSE_NEW ScoreCursor(m_pDoc, static_cast<ImoScore*>(*m_topLevelCursor) );
            break;
        default:
        {
//            stop_delegation();
        }
    }
}

//---------------------------------------------------------------------------------------
void DocCursor::stop_delegation()
{
    delete m_pInnerCursor;
    m_pInnerCursor = NULL;
}

//---------------------------------------------------------------------------------------
void DocCursor::move_next()
{
//    if (is_at_end_of_child())
//    {
//        stop_delegation();
//        ++m_topLevelCursor;
//    }
//    else
    if (is_delegating())
		m_pInnerCursor->move_next();
	else
		++m_topLevelCursor;
}

//---------------------------------------------------------------------------------------
void DocCursor::move_prev()
{
    if (is_delegating())
    {
//        if (m_pInnerCursor->is_at_start())
//            stop_delegation();
//        else
//        {
            m_pInnerCursor->move_prev();
//            if (m_pInnerCursor->get_pointee() == NULL)
//                stop_delegation();
//            else
//                --m_topLevelCursor;
//        }
    }
    else
    {
//        if (m_pFirst == NULL)
//        {
//            //update ptr to first just in case document modified
//            DocContentIterator it(m_pDoc);
//            it.start_of_content();
//            m_pFirst = *it;
//        }
//
//        if (*m_topLevelCursor != m_pFirst)
//        {
//            if (*m_topLevelCursor != NULL)
                --m_topLevelCursor;
//            else
//                m_topLevelCursor.last_of_content();
//        }
    }
}

//---------------------------------------------------------------------------------------
void DocCursor::to_end()
{
    stop_delegation();
    m_topLevelCursor.to_end();
}

//---------------------------------------------------------------------------------------
void DocCursor::to_last_top_level()
{
    stop_delegation();
    m_topLevelCursor.last_of_content();
}

//---------------------------------------------------------------------------------------
void DocCursor::point_to(long id)
{
    point_to( m_pDoc->get_pointer_to_imo(id) );
}

//---------------------------------------------------------------------------------------
void DocCursor::point_to(ImoObj* pImo)
{
    if (pImo)
    {
        if (pImo->is_block_level_obj())
        {
            stop_delegation();
            m_topLevelCursor.point_to( static_cast<ImoBlockLevelObj*>(pImo) );
        }
        else
        {
            ImoBlockLevelObj* pParent = find_block_level_parent(pImo);
            if (*m_topLevelCursor != pParent)
            {
                stop_delegation();
                m_topLevelCursor.point_to( pParent );
            }
            if (!is_delegating())
                enter_element();
            m_pInnerCursor->point_to(pImo);
        }
    }
    else
        to_end();
}

//---------------------------------------------------------------------------------------
ImoBlockLevelObj* DocCursor::find_block_level_parent(ImoObj* pImo)
{
    ImoObj* pParent = pImo->get_parent_imo();
    while (pParent && !pParent->is_block_level_obj())
    {
        pParent = pParent->get_parent_imo();
    }

    return static_cast<ImoBlockLevelObj*>( pParent );
}

//---------------------------------------------------------------------------------------
void DocCursor::update_after_deletion()
{
    if (is_delegating())
		m_pInnerCursor->update_after_deletion();
	else
		m_topLevelCursor.update_after_deletion();
}

////---------------------------------------------------------------------------------------
//void DocCursor::reset_and_point_to(long nId)
//{
//    if (is_delegating())
//		m_pInnerCursor->reset_and_point_to(nId);
//	else
//        point_to(nId);
//}

//---------------------------------------------------------------------------------------
DocCursorState DocCursor::get_state()
{
    long id = get_top_id();
    if (is_delegating())
		return DocCursorState(id, m_pInnerCursor->get_state());
	else
        return DocCursorState(id, NULL);
}

//---------------------------------------------------------------------------------------
void DocCursor::restore_state(DocCursorState& state)
{
        if (!state.is_delegating())
            point_to( state.get_top_level_id() );
	    else
        {
            point_to( state.get_top_level_id() );
            enter_element();
            m_pInnerCursor->restore_state( state.get_delegate_state() );
        }
}


//=======================================================================================
// ScoreCursor implementation
//=======================================================================================
ScoreCursor::ScoreCursor(Document* pDoc, ImoScore* pScore)
    : ElementCursor(pDoc)
    , m_scoreId(pScore->get_id())
    , m_pColStaffObjs( pScore->get_staffobjs_table() )
    , m_pScore(pScore)
    , m_fAutoRefresh(false)
    , m_timeStep( float(k_duration_eighth) )
{
    p_start_cursor();
}

//---------------------------------------------------------------------------------------
ScoreCursor::~ScoreCursor()
{
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_start_cursor()
{
    m_it = m_pColStaffObjs->begin();
    while ( p_there_is_iter_object()
            && !(p_iter_object_is_on_instrument(0) && p_iter_object_is_on_staff(0)) )
    {
        p_move_iterator_to_next();
    }
    p_update_state_from_iterator();

    m_prevState.set_before_start_of_score();
    m_itPrev = m_pColStaffObjs->end();
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ScoreCursor::staffobj()
{
    auto_refresh();

    if (m_currentState.id() >= 0L)
        return (*m_it)->imo_object();
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoObj* ScoreCursor::staffobj_internal()
{
    auto_refresh();

    if (m_it != m_pColStaffObjs->end())
        return (*m_it)->imo_object();
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* ScoreCursor::find_previous_imo()
{
    auto_refresh();

    ColStaffObjsIterator itSave = m_it;

    float curTime = m_currentState.time();

    //move to prev with lower time
    p_move_iterator_to_prev();
    while ( p_there_is_iter_object()
            && !is_lower_time(p_iter_object_time(), curTime)
          )
    {
        p_move_iterator_to_prev();
    }

    //based on found object, determine position
    if ( !p_there_is_iter_object()
         || !is_lower_time(p_iter_object_time(), curTime)
       )
    {
        //no previous staffobj!
        throw runtime_error("Must exist");
    }

    ColStaffObjsEntry* pEntry = *m_it;
    m_it = itSave;
    return pEntry;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_move_iterator_to_next()
{
    if (m_it == m_pColStaffObjs->end())
        m_it = m_pColStaffObjs->begin();
    else
        ++m_it;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to(long nId)
{
    auto_refresh();

    p_point_to(nId);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_point_to(long nId)
{
    p_move_iterator_to(nId);
    p_update_state_from_iterator();
    p_find_previous_state();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_move_iterator_to(long id)
{
    if (id <= -1L)
        m_it = m_pColStaffObjs->end();
    else
    {
        m_it = m_pColStaffObjs->begin();
        for (; m_it != m_pColStaffObjs->end() && (*m_it)->element_id() != id; ++m_it);
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to(ImoObj* pImo)
{
    auto_refresh();

    p_point_to( pImo->get_id() );
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to_barline(long id, int staff)
{
    auto_refresh();

    p_point_to(id);
    m_currentState.staff(staff);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::to_state(int nInstr, int nStaff, int nMeasure, float rTime, long id)
{
    auto_refresh();

    p_to_state(nInstr, nStaff, nMeasure, rTime, id);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_state(int nInstr, int nStaff, int nMeasure, float rTime, long id)
{
    //if id==-1L move to first object satisfying all other conditions

    //TODO: This method will fail when several objects at same timepos (i.e. notes
    //in chord, notes in different voices, prolog -- clef, key, time, note --)
    //because it will always move to first object, not to desired one.
    //It is necessary to modify parameters list to pass object id and ref.obj id

    m_it = m_pColStaffObjs->begin();
    p_forward_to_state(nInstr, nStaff, nMeasure, rTime);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_forward_to_state(int instr, int staff, int measure, float time)
{
    m_currentState.instrument(instr);
    m_currentState.staff(staff);
    m_currentState.measure(measure);

    p_forward_to_instr_measure_with_time_not_lower_than(time);
    m_currentState.time(time);

    p_forward_to_current_staff();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_save_current_state_as_previous_state()
{
    m_prevState = m_currentState;
    m_itPrev = m_it;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_next()
{
    //Implements user expectations when pressing 'cursor right' key: move cursor to
    //next timepos in current instrument. Cursor remains in current staff even if
    //the timepos is not occupied in that staff. When in last timepos, moves to next
    //logical timepos (current timepos + object duration). When end of staff is
    //reached:
    // - if current instrument has more staves,
    //   advance to next staff, to first object in first measure.
    // - else to first staff of next instrument.
    // - If no more instruments, remains at end of score

    auto_refresh();
    p_move_next();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_move_next()
{
    if (p_is_at_end_of_score())
        return;

    p_save_current_state_as_previous_state();

    if (p_is_at_end_of_staff())
    {
        if (p_more_staves_in_instrument())
            p_to_start_of_next_staff();
        else if (p_more_instruments())
            p_to_start_of_next_instrument();
        else     //we are at end of score
            return;     //remain there. should'not arrive here!
    }
    else if (p_try_next_at_same_time())
    {
        p_update_state_from_iterator();
        return;
    }
    else
    {
        p_find_next_time_in_this_staff();
    }
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_more_staves_in_instrument()
{
    int instr = m_currentState.instrument();
	ImoInstrument* pInstr = m_pScore->get_instrument(instr);
    int numStaves = pInstr->get_num_staves();
	return (m_currentState.staff() < numStaves - 1);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_start_of_next_staff()
{
    int staff = m_currentState.staff() + 1;
    p_to_state(m_currentState.instrument(), staff, 0, 0.0f);
    p_update_pointed_objects();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_update_as_end_of_staff()
{
    m_currentState.id( k_cursor_at_end_of_staff );
    m_currentState.ref_obj_id(-1L);
    m_currentState.ref_obj_time(0.0f);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_update_as_end_of_score()
{
    int instr = m_pScore->get_num_instruments() - 1;
    m_currentState.instrument(instr);

	ImoInstrument* pInstr = m_pScore->get_instrument(instr);
    int staff = pInstr->get_num_staves() - 1;
    m_currentState.staff(staff);

    if (m_pColStaffObjs->num_entries() > 0)
    {
        ColStaffObjsEntry* pEntry = m_pColStaffObjs->back();
        ImoStaffObj* pImo = dynamic_cast<ImoStaffObj*>( pEntry->imo_object() );
        int measure = pEntry->measure();
        float time = pEntry->time();
        if (pImo)
        {
            time += pImo->get_duration();
            if (pImo->is_barline())
                ++measure;
        }
        m_currentState.measure(measure);
        m_currentState.time(time);
    }
    else
    {
        m_currentState.measure(0);
        m_currentState.time(0.0f);
    }


    m_currentState.id( k_cursor_at_end_of_staff );
    m_currentState.ref_obj_id( -1L );
    m_currentState.ref_obj_time(0.0f);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::is_at_end_of_score()
{
    auto_refresh();
    return p_is_at_end_of_score();
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_is_at_end_of_score()
{
    if (m_currentState.id() != k_cursor_at_end_of_staff)
        return false;

    int instr = m_pScore->get_num_instruments() - 1;
    if (m_currentState.instrument() != instr)
        return false;

	ImoInstrument* pInstr = m_pScore->get_instrument(instr);
    int staff = pInstr->get_num_staves() - 1;
    if (m_currentState.staff() != staff)
        return false;

    return true;
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::is_at_end_of_empty_score()
{
    //intrinsically safe

    return m_prevState.is_before_start_of_score();
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_more_instruments()
{
	int numInstruments = m_pScore->get_num_instruments() - 1;
    return m_currentState.instrument() < numInstruments;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_start_of_next_instrument()
{
    p_to_state(m_currentState.instrument()+1, 0, 0, 0.0f);
    p_update_pointed_objects();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_forward_to_current_staff()
{
    if (!p_iter_is_at_end() && !p_iter_object_is_on_staff( m_currentState.staff() ))
    {
        ColStaffObjsIterator  itLast = m_it;          //save current ref.object
        if ( !p_find_current_staff_at_current_iter_object_time() )
            m_it = itLast;      //not found. Go back to previous ref.object
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_update_state_from_iterator()
{
    if (p_there_is_iter_object())
    {
        m_currentState.instrument( p_iter_object_instrument() );
        m_currentState.staff( p_iter_object_staff() );
        m_currentState.measure( p_iter_object_measure() );
        m_currentState.time( p_iter_object_time() );
        m_currentState.id( p_iter_object_id() );
        m_currentState.ref_obj_id( p_iter_object_id() );
        m_currentState.ref_obj_time( p_iter_object_time() );
    }
    else
        p_update_as_end_of_score();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_update_pointed_objects()
{
    //instrument, staff and time are already set.
    //determine if iter is pointing an object and update state in accordance

    if (p_there_is_iter_object())
    {
        m_currentState.measure( p_iter_object_measure() );

        if (p_iter_object_is_on_instrument( m_currentState.instrument() )
            && p_iter_object_is_on_staff( m_currentState.staff() )
            && p_iter_object_is_on_time( m_currentState.time() ))
        {
            m_currentState.id( p_iter_object_id() );
            m_currentState.ref_obj_id( p_iter_object_id() );
            m_currentState.ref_obj_time( p_iter_object_time() );
        }
        else
        {
            m_currentState.id( k_cursor_at_empty_place );
            m_currentState.ref_obj_id( p_iter_object_id() );
            m_currentState.ref_obj_time( p_iter_object_time() );
        }
    }
    else
        p_update_as_end_of_staff();
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_try_next_at_same_time()
{
    //try to move to a possible next object at same time

    ColStaffObjsIterator itSave = m_it;
    while ( p_there_is_iter_object()
            && is_equal_time(p_iter_object_time(), m_currentState.time()) )
    {
        p_move_iterator_to_next();
        if ( p_there_is_iter_object()
             && p_iter_object_is_on_instrument( m_currentState.instrument() )
             && p_iter_object_is_on_staff( m_currentState.staff() )
             && is_equal_time(p_iter_object_time(), m_currentState.time()) )
        {
            return true;     //prev object found. done
        }
    }
    m_it = itSave;
    return false;
}

//---------------------------------------------------------------------------------------
int ScoreCursor::p_determine_next_target_measure()
{
    if (p_iter_object_is_barline())
        return p_iter_object_measure() + 1;
    else
        return m_currentState.measure();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_forward_to_instr_measure_with_time_not_lower_than(float rTargetTime)
{
    while (p_there_is_iter_object()
           && ( !p_iter_object_is_on_instrument( m_currentState.instrument() )
                || !p_iter_object_is_on_measure( m_currentState.measure() )
                || is_greater_time(rTargetTime, p_iter_object_time()) ))
    {
        p_move_iterator_to_next();
    }
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_find_current_staff_at_current_iter_object_time()
{
    float rCurTime = p_iter_object_time();
    while (p_there_is_iter_object()
            && is_equal_time(rCurTime, p_iter_object_time())
            && ( !p_iter_object_is_on_instrument( m_currentState.instrument() )
                || !p_iter_object_is_on_staff( m_currentState.staff() ) ))
    {
        p_move_iterator_to_next();
    }

    return p_there_is_iter_object()
           && p_iter_object_is_on_instrument( m_currentState.instrument() )
           && is_equal_time(rCurTime, p_iter_object_time())
           && p_iter_object_is_on_staff( m_currentState.staff() );
}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_prev()
{
    auto_refresh();

    //AWARE: is_at_end_of_empty_score() is safe and will not invoke auto-refresh
    if (is_at_end_of_empty_score())
        return;

    m_currentState = m_prevState;
    m_it = m_itPrev;

    p_find_previous_state();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_find_previous_state()
{
    //Implements user expectations when pressing 'cursor left' key: move cursor to
    //previous time in current instrument.
    //Cursor will always stop in each measure at timepos 0 (even if no objects
    //there) and then move to prev measure and stop before barline.
    //If cursor is at start of score will remain there.
    //When cursor is at start of staff:
    // - if current instrument has more staves,
    //   goes back to end of previous staff.
    // - else to end of last staff of previous instrument.

    ScoreCursorState current = m_currentState;
    ColStaffObjsIterator itCurr = m_it;

    if (p_is_iterator_at_start_of_score())
    {
        m_currentState.set_before_start_of_score();
        m_it = m_pColStaffObjs->end();
    }
    else if (p_is_at_end_of_staff())
    {
        p_iter_to_last_object_in_current_time();
        p_find_position_at_current_time();
    }
    else if (p_try_prev_at_same_time())
        p_update_pointed_objects();         //prev object found. done
    else if (p_is_at_start_of_staff())
    {
        if (!p_is_first_staff_of_current_instrument())
            p_to_end_of_prev_staff();
        else if (!p_is_first_instrument())
            p_to_end_of_prev_instrument();
    }
    else
    {
        p_find_prev_time_in_this_staff();
    }


    ScoreCursorState prev = m_currentState;
    ColStaffObjsIterator itPrev = m_it;

    p_set_current_state(current);
    m_it = itCurr;
    p_set_previous_state(prev);
    m_itPrev = itPrev;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_move_iterator_to_prev()
{
    if (m_it == m_pColStaffObjs->begin())
        m_it = m_pColStaffObjs->end();
    else
    {
        if (m_it != m_pColStaffObjs->end())
            --m_it;
        else
            m_it = ColStaffObjsIterator( m_pColStaffObjs->back() );
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_find_prev_time_in_this_staff()
{
    ColStaffObjsIterator itSave = m_it;
    float targetTime = m_currentState.time() - m_timeStep;
    int staff = m_currentState.staff();
    int instr = m_currentState.instrument();

    //move to prev in this instr & staff
    p_move_iterator_to_prev();
    while ( p_there_is_iter_object()
            && !(p_iter_object_is_on_staff(staff)
                 && instr == p_iter_object_instrument() ))
    {
        p_move_iterator_to_prev();
    }

    //based on found object, determine position
    if ( !p_there_is_iter_object()
         || !p_iter_object_is_on_staff(staff)
         || instr != p_iter_object_instrument()
       )
    {
        //prev position is empty
        m_currentState.time( targetTime );
        m_currentState.id( k_cursor_at_empty_place );
        m_it = itSave;
    }
    else if (!is_lower_time(p_iter_object_time(), targetTime))
    {
        p_update_state_from_iterator();
        m_currentState.staff(staff);
    }
    else if (!is_greater_time(p_iter_object_time() + p_iter_object_duration(), targetTime))
    {
        //prev position is empty
        m_currentState.time( targetTime );
        m_currentState.id( k_cursor_at_empty_place );
        m_it = itSave;
    }
    else
    {
        p_update_state_from_iterator();
        m_currentState.staff(staff);
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_find_next_time_in_this_staff()
{
    float targetTime = m_currentState.time() + m_timeStep;
    int measure = m_currentState.measure();
    ImoStaffObj* pSO = staffobj();
    if (pSO)
    {
        if (pSO->is_barline())
        {
            targetTime = m_currentState.time();
            ++measure;
        }
        else
            targetTime = max(targetTime, m_currentState.time() + pSO->get_duration());
    }

    int staff = m_currentState.staff();
    int instr = m_currentState.instrument();

    //move to next in this instr & staff
    if (m_currentState.id() >= 0L)
        p_move_iterator_to_next();
    while ( p_there_is_iter_object()
//            && is_lower_time(p_iter_object_time(), targetTime)
            && !(p_iter_object_is_on_staff(staff)
                 && instr == p_iter_object_instrument() ))
    {
        p_move_iterator_to_next();
    }

    //based on found object, determine position
    if ( !p_there_is_iter_object() )
    {
        //next position is end of staff/score
        m_currentState.time( targetTime );
        m_currentState.measure(measure);
        p_update_as_end_of_staff();
    }
    else if (!p_iter_object_is_on_staff(staff) || instr != p_iter_object_instrument())
    {
        //next position is empty
        p_update_state_from_iterator();
        m_currentState.time( targetTime );
        m_currentState.id( k_cursor_at_empty_place );
        m_currentState.staff(staff);
        m_currentState.measure(measure);
    }
    else if (!is_greater_time(p_iter_object_time(), targetTime))
    {
        p_update_state_from_iterator();
        m_currentState.staff(staff);
        m_currentState.measure(measure);
    }
    else
    {
        //next position is empty
        p_update_state_from_iterator();
        m_currentState.time( targetTime );
        m_currentState.id( k_cursor_at_empty_place );
        m_currentState.staff(staff);
        m_currentState.measure(measure);
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_iter_to_last_object_in_current_time()
{
    float curTime = m_currentState.time();
    m_it = m_pColStaffObjs->end();
    p_move_iterator_to_prev();
    while ( p_there_is_iter_object() && is_greater_time(p_iter_object_time(), curTime) )
        p_move_iterator_to_prev();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_find_position_at_current_time()
{
    //iterator is set pointing to last object at current time. Move back
    //to find the right position

    //set new current time
    float curTime = p_iter_object_time();
    m_currentState.time(curTime);

    //an object in current time found.
    //try to find object in current time, instrument & staff.
    //if not found, it is an empty place at current time
    int instr = m_currentState.instrument();
    int staff = m_currentState.staff();
    ColStaffObjsIterator itCurr = m_it;
    while (p_there_is_iter_object()
           && is_equal_time(p_iter_object_time(), curTime)
           && !(p_iter_object_is_on_instrument(instr)
                && p_iter_object_is_on_staff(staff)) )
    {
        p_move_iterator_to_prev();
    }
    if (p_there_is_iter_object()
        && is_equal_time(p_iter_object_time(), curTime)
        && p_iter_object_is_on_instrument(instr)
        && p_iter_object_is_on_staff(staff) )
    {
        //right object found
        p_update_pointed_objects();
        return;
    }
    else
    {
        //not found. empty place
        m_it = itCurr;
        p_update_pointed_objects();
        return;
    }
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_try_prev_at_same_time()
{
    //try to move to a possible prev object at same time

    ColStaffObjsIterator itSave = m_it;
    while ( p_there_is_iter_object()
            && is_equal_time(p_iter_object_time(), m_currentState.time()) )
    {
        p_move_iterator_to_prev();
        if ( p_there_is_iter_object()
             && p_iter_object_is_on_instrument( m_currentState.instrument() )
             && p_iter_object_is_on_staff( m_currentState.staff() )
             && is_equal_time(p_iter_object_time(), m_currentState.time()) )
        {
            return true;     //prev object found. done
        }
    }
    m_it = itSave;
    return false;
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_is_at_start_of_staff()
{
    return m_currentState.measure() == 0
           && is_equal_time(m_currentState.time(), 0.0f);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_end_of_prev_staff()
{
    m_currentState.time(0.0f);
    m_currentState.staff( m_currentState.staff() - 1 );

    p_to_end_of_staff();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_end_of_prev_instrument()
{
    m_currentState.time(0.0f);
    int instr = m_currentState.instrument() - 1;
    m_currentState.instrument(instr);

    //determine last staff
	ImoInstrument* pInstr = m_pScore->get_instrument(instr);
    m_currentState.staff( pInstr->get_num_staves() - 1 );

    p_to_end_of_staff();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_end_of_staff()
{
    //determine last measure
    int instr = m_currentState.instrument();
    m_it = m_pColStaffObjs->end();
    p_move_iterator_to_prev();
    if (p_there_is_iter_object())
    {
        while ( p_there_is_iter_object()
                && !p_iter_object_is_on_instrument(instr) )
        {
            p_move_iterator_to_prev();
        }
        if (p_there_is_iter_object())
        {
            int measure = p_iter_object_measure();
            float time = p_iter_object_time();
            if (p_iter_object_is_barline())
                ++ measure;
            else
                time += p_iter_object_duration();
            m_currentState.measure( measure );
            m_currentState.time( time );
        }
        else
        {
            m_currentState.measure(0);
            m_currentState.time(0.0f);
        }
    }
    else
    {
        m_currentState.measure(0);
        m_currentState.time(0.0f);
    }

    //move to end of staff
    m_it = m_pColStaffObjs->end();

    p_update_as_end_of_staff();
}

//////---------------------------------------------------------------------------------------
////void ScoreCursor::move_next_new_time()
////{
////    // move to next object but with different time than current one
////    //Behaviour is as move_next but repeats while new time is equal than current time.
////}
////
//////---------------------------------------------------------------------------------------
////void ScoreCursor::move_prev_new_time()
////{
////    // move to prev object but with dfferent time than current one.
////    //Behaviour is as move_prev but repeats while new time is equal than current time.
////}
////
//////---------------------------------------------------------------------------------------
////void ScoreCursor::to_start_of_instrument(int nInstr)
////{
////    //to first obj in instr nInstr
////    //Moves cursor to instrument nInstr (1..n), at first object.
////    //[at timepos 0 after prolog] ?
////}
////
//////---------------------------------------------------------------------------------------
////void ScoreCursor::to_start_of_measure(int nMeasure, int nStaff)
////{
////    //to first obj in specified measure and staff
////    //Limited to current instrument. Move cursor to start of measure,
////    //that is to first SO and timepos 0. Set staff. Then, if fSkipClef,
////    //advances after last clef in this measure, if any. And then, if
////    //fSkipKey, advances after last key, if any.
////}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::skip_clef_key_time()
//{
//    //while pointing clef, key or time, move next
//    while (p_there_is_iter_object()
//           && (p_iter_object_is_clef() || p_iter_object_is_key() || p_iter_object_is_time()) )
//    {
//        move_next();
//    }
//}

//---------------------------------------------------------------------------------------
float ScoreCursor::p_iter_object_duration()
{
    ImoStaffObj* pISO = dynamic_cast<ImoStaffObj*>((*m_it)->imo_object());
    return pISO->get_duration();
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_iter_object_is_barline()
{
    if (p_there_is_iter_object())
        return (*m_it)->imo_object()->is_barline();
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_iter_object_is_clef()
{
    ImoClef* pImo = dynamic_cast<ImoClef*>((*m_it)->imo_object());
    return (pImo != NULL);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_iter_object_is_key()
{
    ImoKeySignature* pImo = dynamic_cast<ImoKeySignature*>((*m_it)->imo_object());
    return (pImo != NULL);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_iter_object_is_time()
{
    ImoTimeSignature* pImo = dynamic_cast<ImoTimeSignature*>((*m_it)->imo_object());
    return (pImo != NULL);
}

//---------------------------------------------------------------------------------------
ElementCursorState* ScoreCursor::get_state()
{
    auto_refresh();

    return LOMSE_NEW ScoreCursorState(instrument(), staff(), measure(),
                                      time(), id(), staffobj_id_internal(),
                                      ref_obj_time());
}

//---------------------------------------------------------------------------------------
void ScoreCursor::restore_state(ElementCursorState* pState)
{
    //intrinsically safe

    ScoreCursorState* pSCS = dynamic_cast<ScoreCursorState*>(pState);
    if (pSCS == NULL)
        return;

    m_pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(m_scoreId) );
    m_pColStaffObjs = m_pScore->get_staffobjs_table();
    p_point_to(pSCS->ref_obj_id());
    m_currentState.staff( pSCS->staff() );  //fix staff when pointin to a barline
    m_currentState.id( pSCS->id() );
    m_currentState.time( pSCS->time() );
    m_currentState.ref_obj_time( pSCS->ref_obj_time() );

    p_find_previous_state();
}

////---------------------------------------------------------------------------------------
//ImoObj* ScoreCursor::get_musicData_for_current_instrument()
//{
//    int nInstr = instrument();
//    DocCursor cursor(m_pDoc);
//    cursor.point_to( m_pScore->get_id() );
//    DocIterator it = cursor.get_iterator();
//    it.enter_element();
//    it.point_to(ImoObj::k_instrument);
//    for (; nInstr > 0; nInstr--)
//    {
//        ++it;
//        it.point_to(k_instrument);
//    }
//    it.enter_element();
//    it.point_to(ImoObj::k_music_data);
//    return *it;
//}

//---------------------------------------------------------------------------------------
string ScoreCursor::dump_cursor()
{
    stringstream sout;

    sout << "Curr.: instr=" << m_currentState.instrument()
         << ", staff=" << m_currentState.staff()
         << ", measure=" << m_currentState.measure()
         << ", time=" << m_currentState.time()
         << ", id=" << m_currentState.id()
         << ", ref_id=" << m_currentState.ref_obj_id()
         << ", ref_time=" << m_currentState.ref_obj_time();

    if (m_currentState.id() < 0L)
    {
        switch (m_currentState.id())
        {
            case k_cursor_pos_undefined:    sout << " (Pos.undefined.)";        break;
            case k_cursor_before_start:     sout << " (Before start of doc) !!";  break;
            case k_cursor_at_end:           sout << " (At end of doc) !!";      break;
            case k_cursor_at_end_of_child:  sout << " (At end of score)";       break;
            case k_cursor_at_empty_place:   sout << " (At empty place)";        break;
            case k_cursor_before_start_of_child: sout << " (Before start)";     break;
            case k_cursor_at_end_of_staff:  sout << " (At end of staff)";       break;
            default:
                sout << " (Invalid value)";
        }
    }
    sout << endl;

    sout << "Prev.: instr=" << m_prevState.instrument()
         << ", staff=" << m_prevState.staff()
         << ", measure=" << m_prevState.measure()
         << ", time=" << m_prevState.time()
         << ", id=" << m_prevState.id()
         << ", ref_id=" << m_prevState.ref_obj_id()
         << ", ref_time=" << m_prevState.ref_obj_time();

    if (m_prevState.id() < 0L)
    {
        switch (m_prevState.id())
        {
            case k_cursor_pos_undefined:    sout << " (Pos.undefined.)";        break;
            case k_cursor_before_start:     sout << " (Before start of doc) !!";  break;
            case k_cursor_at_end:           sout << " (At end of doc) !!";      break;
            case k_cursor_at_end_of_child:  sout << " (At end of score)";       break;
            case k_cursor_at_empty_place:   sout << " (At empty place)";        break;
            case k_cursor_before_start_of_child: sout << " (Before start)";     break;
            case k_cursor_at_end_of_staff:  sout << " (At end of staff)";       break;
            default:
                sout << " (Invalid value)";
        }
    }
    sout << endl;

    return sout.str();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::refresh()
{
    m_pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(m_scoreId) );
    m_pColStaffObjs = m_pScore->get_staffobjs_table();

    //TODO: changes in ColStaffObjs for optimizations.
//    bool fThereAreChanges = (m_pColStaffObjs->get_signature() != m_signature);
//    bool fPointersStillValid = (m_pColStaffObjs->get_timestamp() == m_timestamp);
//
//    if (!fThereAreChanges)
//    {
//        if (fPointersStillValid)
//            return;
//        else
//        {
//            //no content changes but new pointers. Update iterators
//            m_timestamp = m_pColStaffObjs->get_timestamp();
//            p_move_iterator_to( m_prevState.ref_obj_id() );
//            m_itPrev = m_it;
//            p_move_iterator_to( m_currentState.ref_obj_id() );
//        }
//    }
//    else
    {
        //Score modified

//        m_signature = m_pColStaffObjs->get_signature();
//        m_timestamp = m_pColStaffObjs->get_timestamp();

        if (p_success_refreshing_current())
            return;
        else if (p_success_refreshing_prev())
            return;
        else
            ;   //p_start_cursor();   //TODO: another behaviour?
    }
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_success_refreshing_prev()
{
    //if prev pos valid, find current

    if (m_prevState.ref_obj_id() >= 0L)
    {
        ImoObj* pImo = m_pDoc->get_pointer_to_imo( m_prevState.ref_obj_id() );
        if (pImo == NULL)
            return false;
    }

    if (m_prevState.is_before_start_of_score())
    {
        p_start_cursor();
    }
    else if (m_prevState.is_at_end_of_staff())
    {
        m_currentState = m_prevState;
        if (p_more_staves_in_instrument())
            p_to_start_of_next_staff();
        else if (p_more_instruments())
            p_to_start_of_next_instrument();
        else     //we are at end of score? impossible; prev state should be before start!
            return true;     //remain there. should not arrive here!
    }
    else
    {
        m_currentState = m_prevState;
        p_move_iterator_to( m_currentState.ref_obj_id() );
        p_move_next();
    }
    return true;
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_success_refreshing_current()
{
    //if current pos valid refresh current pos and find prev

    ImoObj* pImo = m_pDoc->get_pointer_to_imo( m_currentState.ref_obj_id() );
    if (pImo == NULL)
        return false;

    p_point_to( m_currentState.ref_obj_id() );
    p_find_previous_state();
    return true;
}


}  //namespace lomse
