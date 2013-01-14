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
    : m_pDoc(pDoc)
    , m_pCurItem(NULL)
{
    start_of_content();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::next()
{
    if (m_pCurItem)
        m_pCurItem = m_pCurItem->get_next_sibling();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::prev()
{
    if (m_pCurItem)
        m_pCurItem = m_pCurItem->get_prev_sibling();
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
}

//---------------------------------------------------------------------------------------
void DocContentCursor::last_of_content()
{
    ImoDocument* pImoDoc = m_pDoc->get_imodoc();
    ImoContent* pContent = pImoDoc->get_content();
    m_pCurItem = pContent->get_last_child();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to(long id)
{
    ImoObj* pImo = m_pDoc->get_pointer_to_imo(id);
    if (pImo && pImo->is_block_level_obj())
        m_pCurItem = pImo;
    else
        to_end();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to(ImoBlockLevelObj* pImo)
{
    m_pCurItem = pImo;
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
DocCursor::DocCursor(const DocCursor& cursor)
    : m_pDoc(cursor.m_pDoc)
    , m_topLevelCursor(m_pDoc)
{
//    m_topLevelCursor = cursor.m_topLevelCursor;
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

////---------------------------------------------------------------------------------------
//void DocCursor::start_of_content()
//{
//    if (is_delegating())
//        stop_delegation();
//    m_topLevelCursor.start_of_content();
//}

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
void DocCursor::next()
{
//    if (is_at_end_of_child())
//    {
//        stop_delegation();
//        ++m_topLevelCursor;
//    }
//    else if (is_delegating())
//	{
//		m_pInnerCursor->move_next();
//	}
//	else
		++m_topLevelCursor;
}

//---------------------------------------------------------------------------------------
void DocCursor::prev()
{
//    if (is_delegating())
//    {
//        if (m_pInnerCursor->is_at_start())
//            stop_delegation();
//        else
//        {
//            m_pInnerCursor->move_prev();
//            if (m_pInnerCursor->get_pointee() == NULL)
//                stop_delegation();
//            else
//                --m_topLevelCursor;
//        }
//    }
//    else
//    {
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
//    }
}

//---------------------------------------------------------------------------------------
void DocCursor::to_end()
{
    stop_delegation();
    m_topLevelCursor.to_end();
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
        next();
}

////---------------------------------------------------------------------------------------
//void DocCursor::reset_and_point_to(long nId)
//{
//    if (is_delegating())
//		m_pInnerCursor->reset_and_point_to(nId);
//	else
//        point_to(nId);
//}
//
////---------------------------------------------------------------------------------------
//DocCursorState* DocCursor::get_state()
//{
//    int id =  (*m_topLevelCursor != NULL ? (*m_topLevelCursor)->get_id() : -1);
//    if (is_delegating())
//		return new DocCursorState(id, m_pInnerCursor->get_state());
//	else
//        return new DocCursorState(id, NULL);
//}
//
////---------------------------------------------------------------------------------------
//void DocCursor::restore(DocCursorState* pState)
//{
//    DocCursorState* pDCS = dynamic_cast<DocCursorState*>(pState);
//    if (pDCS)
//    {
//        if (!pDCS->is_delegating())
//            point_to(pDCS->get_id());
//	    else
//        {
//            point_to(pDCS->get_top_level_id());
//            enter_element();
//            m_pInnerCursor->restore( pDCS->get_delegate_state() );
//        }
//    }
//}


//=======================================================================================
// ScoreCursor implementation
//=======================================================================================
ScoreCursor::ScoreCursor(Document* pDoc, ImoScore* pScore)
    : ElementCursor(pDoc), ScoreCursorInterface()
    , m_scoreId(pScore->get_id())
    , m_pColStaffObjs( pScore->get_staffobjs_table() )
{
    start();
}

//---------------------------------------------------------------------------------------
ScoreCursor::~ScoreCursor()
{
}

//---------------------------------------------------------------------------------------
void ScoreCursor::start()
{
    m_it = m_pColStaffObjs->begin();
    update_state_from_iterator();

    m_prevState.set_before_start_of_score();
    m_itPrev = m_pColStaffObjs->end();
}

//---------------------------------------------------------------------------------------
ImoObj* ScoreCursor::staffobj()
{
    if (is_pointing_object())
        return (*m_it)->imo_object();
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
long ScoreCursor::staffobj_id()
{
    if (is_pointing_object())
        return (*m_it)->imo_object()->get_id();
    else
        return -1L;
}

////---------------------------------------------------------------------------------------
//ImoObj* ScoreCursor::get_pointee()
//{
//    if (m_it != m_pColStaffObjs->end())
//        return (*m_it)->imo_object();
//    else
//        return NULL;
//}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_iterator_to_next()
{
//    if (m_it == m_pColStaffObjs->end())
//        m_it = m_pColStaffObjs->begin();
//    else
        ++m_it;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to(long nId)
{
//    if (nId != -1)
//    {
        move_iterator_to(nId);
        update_state_from_iterator();
        find_previous_state();
//    }
//    else
//        m_it = m_pColStaffObjs->end();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_iterator_to(long id)
{
    if (id == -1)
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
    point_to( pImo->get_id() );
}

////---------------------------------------------------------------------------------------
//void ScoreCursor::reset_and_point_to(long nId)
//{
//    m_pColStaffObjs = m_pScore->get_staffobjs_table();
//    point_to(nId);
//}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::point_to_barline(long nId, int nStaff)
//{
//    point_to(nId);
//    m_nStaff = nStaff;
//}

//---------------------------------------------------------------------------------------
void ScoreCursor::to_state(int nInstr, int nStaff, int nMeasure, float rTime, long id)
{
    //TODO: This method will fail when several objects at same timepos (i.e. notes
    //in chord, notes in different voices, prolog -- clef, key, time, note --)
    //because it will always move to first object, not to desired one.
    //It is necessary to modify parameters list to pass Object ID

    m_it = m_pColStaffObjs->begin();
    forward_to_state(nInstr, nStaff, nMeasure, rTime);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::forward_to_state(int instr, int staff, int measure, float time)
{
    m_currentState.instrument(instr);
    m_currentState.staff(staff);
    m_currentState.measure(measure);

    forward_to_instr_measure_with_time_not_lower_than(time);
    m_currentState.time(time);

    forward_to_current_staff();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::save_current_state_as_previous_state()
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

    if (m_currentState.is_at_end_of_score())
        return;

    save_current_state_as_previous_state();

    if (is_at_end())
    {
//        if (more_staves_in_instrument())
//            to_start_of_next_staff();
//        else if (more_instruments())
//            to_start_of_next_instrument();
//        else     //we are at end of score
            return;     //remain there
    }
    else if (try_next_at_same_time())
    {
        update_state_from_iterator();
        return;
    }
    else
    {
        forward_to_next_time();
        update_state_from_iterator();
    }
}

////---------------------------------------------------------------------------------------
//bool ScoreCursor::more_staves_in_instrument()
//{
//	ImoInstrument* pInstr = m_pScore->get_instrument(m_nInstr);
//    int numStaves = pInstr->get_num_staves();
//	return (m_nStaff < numStaves - 1);
//}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::to_start_of_next_staff()
//{
//    m_nMeasure = 0;
//    m_rTime = 0.0f;
//    m_nStaff++;
//
//    to_state(m_nInstr, m_nStaff, m_nMeasure, m_rTime);
//}
//
////---------------------------------------------------------------------------------------
//bool ScoreCursor::more_instruments()
//{
//	int numInstruments = m_pScore->get_num_instruments();
//    return m_nInstr < numInstruments - 1;
//}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::to_start_of_next_instrument()
//{
//    m_nInstr++;
//    m_nMeasure = 0;
//    m_nStaff = 0;
//    m_rTime = 0.0f;
//
//    to_state(m_nInstr, m_nStaff, m_nMeasure, m_rTime);
//}

//---------------------------------------------------------------------------------------
void ScoreCursor::forward_to_next_time()
{
    float rTargetTime = determine_next_target_time();
    int nTargetMeasure = determine_next_target_measure();

    forward_to_state(m_currentState.instrument(), m_currentState.staff(), nTargetMeasure,
                     rTargetTime);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::forward_to_current_staff()
{
//    if (!is_at_end() && !ref_object_is_on_staff(m_nStaff))
//    {
//        ColStaffObjs::iterator  itLast = m_it;          //save current ref.object
//        if ( !find_current_staff_at_current_ref_object_time() )
//            m_it = itLast;      //not found. Go back to previous ref.object
//    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::update_after_deletion()
{
    m_currentState = m_prevState;
    //AWARE: As the staff objs collection might have been rebuilt:
    //  * need to update staffobjs table pointer
    //  * cannot restore saved iterator (m_itPrev)
    // score pointer could have been invalidated
    ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(m_scoreId) );
    m_pColStaffObjs = pScore->get_staffobjs_table();
    move_iterator_to( m_currentState.id() );

    move_next();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::update_state_from_iterator()
{
    if (there_is_ref_object())
    {
        m_currentState.instrument( (*m_it)->num_instrument() );
        m_currentState.staff( (*m_it)->staff() );
        m_currentState.measure( (*m_it)->measure() );
        m_currentState.time( (*m_it)->time() );
        m_currentState.id( (*m_it)->imo_object()->get_id() );
    }
    else
        m_currentState.set_at_end_of_score();
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::try_next_at_same_time()
{
    //try to move to a possible next object at same time

    move_iterator_to_next();
    if ( there_is_ref_object()
         && ref_object_is_on_instrument( m_currentState.instrument() )
         && ref_object_is_on_staff( m_currentState.staff() )
         && is_equal_time(ref_object_time(), m_currentState.time()) )
    {
        return true;     //next object found. done
    }
    move_iterator_to_prev();
    return false;
}

//---------------------------------------------------------------------------------------
float ScoreCursor::determine_next_target_time()
{
//    if (is_pointing_object())
//    {
//        if (ref_object_is_barline())
//            return 0.0f;
//        else
            return ref_object_time() + ref_object_duration();
//    }
//    else
//    {
//        if ( is_equal_time(m_rTime, ref_object_time()) )
//            return ref_object_time() + ref_object_duration();
//        else
//            return ref_object_time();
//    }
}

//---------------------------------------------------------------------------------------
int ScoreCursor::determine_next_target_measure()
{
    if (is_pointing_barline())
        return ref_object_measure() + 1;
    else
        return m_currentState.measure();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::forward_to_instr_measure_with_time_not_lower_than(float rTargetTime)
{
    while (there_is_ref_object()
           && ( !ref_object_is_on_instrument( m_currentState.instrument() )
                || !ref_object_is_on_measure( m_currentState.measure() )
                || is_greater_time(rTargetTime, ref_object_time()) ))
    {
        move_iterator_to_next();
    }
}

////---------------------------------------------------------------------------------------
//bool ScoreCursor::find_current_staff_at_current_ref_object_time()
//{
//    float rCurTime = ref_object_time();
//    while (there_is_ref_object()
//            && is_equal_time(rCurTime, ref_object_time())
//            && ( !ref_object_is_on_instrument(m_nInstr)
//                || !ref_object_is_on_staff(m_nStaff) ))
//    {
//        move_iterator_to_next();
//    }
//
//    return there_is_ref_object()
//           && ref_object_is_on_instrument(m_nInstr)
//           && is_equal_time(rCurTime, ref_object_time())
//           && ref_object_is_on_staff(m_nStaff);
//}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_prev()
{
    m_currentState = m_prevState;
    m_it = m_itPrev;

    find_previous_state();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::find_previous_state()
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
    ColStaffObjs::iterator itCurr = m_it;

    if (is_iterator_at_start_of_score())
    {
        m_currentState.set_before_start_of_score();
        m_it = m_pColStaffObjs->end();
    }
//    else if (try_prev_at_same_time())
//        return;         //prev object found. done
//    else if (is_at_start_of_staff())
//    {
//        if (!is_first_staff_of_current_instrument())
//            to_end_of_prev_staff();
//        else if (!is_first_instrument())
//            to_end_of_prev_instrument();
//    }
    else
        backward_to_prev_time();


    ScoreCursorState prev = m_currentState;
    ColStaffObjs::iterator itPrev = m_it;

    set_current_state(current);
    m_it = itCurr;
    set_previous_state(prev);
    m_itPrev = itPrev;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_iterator_to_prev()
{
//    if (m_it == m_pColStaffObjs->begin())
//        m_it = m_pColStaffObjs->end();
//    else
        --m_it;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::backward_to_prev_time()
{
    move_iterator_to_prev();
    update_state_from_iterator();

//    //back_to_different_time
//    move_iterator_to_prev();
//    while ( there_is_ref_object() && is_equal_time(ref_object_time(), m_rTime) )
//        move_iterator_to_prev();
//
//    //change measure if we are going to cross a barline
//    if (is_equal_time(m_rTime, 0.0f))
//        m_nMeasure--;
//
//    //back to current instrument
//    while ( there_is_ref_object()
//            && !ref_object_is_on_instrument(m_nInstr) )
//        move_iterator_to_prev();
//
//    //set new current time
//    m_rTime = ref_object_time();
//
//    //back to start of current time
//    while ( there_is_ref_object()
//            && ref_object_is_on_instrument(m_nInstr)
//            && is_equal_time(ref_object_time(), m_rTime) )
//    {
//        move_iterator_to_prev();
//    }
//    move_iterator_to_next();
//
//    //forward to right staff
//    forward_to_current_staff();
//
//    //forward to last object in target staff & time
//    if (there_is_ref_object())
//    {
//        move_iterator_to_next();
//        while ( there_is_ref_object()
//                && ref_object_is_on_instrument(m_nInstr)
//                && ref_object_is_on_staff(m_nStaff)
//                && is_equal_time(ref_object_time(), m_rTime) )
//        {
//            move_iterator_to_next();
//        }
//        move_iterator_to_prev();
//    }
}

////---------------------------------------------------------------------------------------
//bool ScoreCursor::try_prev_at_same_time()
//{
//    //try to move to a possible prev object at same time
//
//    move_iterator_to_prev();
//    if ( there_is_ref_object()
//         && ref_object_is_on_instrument(m_nInstr)
//         && ref_object_is_on_staff(m_nStaff)
//         && is_equal_time(ref_object_time(), m_rTime) )
//    {
//        return true;     //prev object found. done
//    }
//    move_iterator_to_next();
//    return false;
//}
//
////---------------------------------------------------------------------------------------
//bool ScoreCursor::is_at_start_of_staff()
//{
//    return m_nMeasure == 0 && is_equal_time(m_rTime, 0.0f);
//}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::to_end_of_prev_staff()
//{
//    m_rTime = 0.0f;
//    m_nStaff--;
//
//    to_end_of_staff();
//}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::to_end_of_prev_instrument()
//{
//    m_rTime = 0.0f;
//    m_nInstr--;
//
//    //determine last staff
//	ImoInstrument* pInstr = m_pScore->get_instrument(m_nInstr);
//    m_nStaff = pInstr->get_num_staves() - 1;
//
//    to_end_of_staff();
//}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::to_end_of_staff()
//{
//    //determine last measure
//    m_it = m_pColStaffObjs->end();
//    move_iterator_to_prev();
//    if (there_is_ref_object())
//    {
//        while ( there_is_ref_object()
//                && !ref_object_is_on_instrument(m_nInstr) )
//        {
//            move_iterator_to_prev();
//        }
//        if (there_is_ref_object())
//            m_nMeasure = ref_object_measure() + 1;
//        else
//            m_nMeasure = 0;
//    }
//    else
//        m_nMeasure = 0;
//
//    //move to end of staff
//    m_it = m_pColStaffObjs->end();
//}
//
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
//    while (is_pointing_object()
//           && (ref_object_is_clef() || ref_object_is_key() || ref_object_is_time()) )
//    {
//        move_next();
//    }
//}

//---------------------------------------------------------------------------------------
float ScoreCursor::ref_object_duration()
{
    ImoStaffObj* pISO = dynamic_cast<ImoStaffObj*>((*m_it)->imo_object());
    return pISO->get_duration();
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::ref_object_is_barline()
{
    ImoBarline* pImo = dynamic_cast<ImoBarline*>((*m_it)->imo_object());
    return (pImo != NULL);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::ref_object_is_clef()
{
    ImoClef* pImo = dynamic_cast<ImoClef*>((*m_it)->imo_object());
    return (pImo != NULL);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::ref_object_is_key()
{
    ImoKeySignature* pImo = dynamic_cast<ImoKeySignature*>((*m_it)->imo_object());
    return (pImo != NULL);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::ref_object_is_time()
{
    ImoTimeSignature* pImo = dynamic_cast<ImoTimeSignature*>((*m_it)->imo_object());
    return (pImo != NULL);
}

////---------------------------------------------------------------------------------------
//ElementCursorState* ScoreCursor::get_state()
//{
//    //// for debugging
//    //bool f1 = there_is_ref_object();
//    //bool f2 = f1 && ref_object_is_on_instrument(m_nInstr);
//    //bool f3 = f2 && ref_object_is_on_measure(m_nMeasure);
//    //bool f4 = f3;     //&& ref_object_is_on_time(m_rTime);
//    //if (f4)
//    //{
//    //    float rRefObjTime = ref_object_time();
//    //    f4 = is_equal_time(m_rTime, rRefObjTime);
//    //}
//    //bool f5 = f4 && ref_object_is_on_staff(m_nStaff);
//    //long id = (f5 ? ref_object_id() : -1L);
//
//    long id = (is_pointing_object() ? ref_object_id() : -1L);
//    return new ScoreCursorState(instrument(), measure(), staff(), time(), id);
//}
//
////---------------------------------------------------------------------------------------
//void ScoreCursor::restore(ElementCursorState* pState)
//{
//    ScoreCursorState* pSCS = dynamic_cast<ScoreCursorState*>(pState);
//    if (pSCS)
//    {
//        m_pColStaffObjs = m_pScore->get_staffobjs_table();
//        point_to(pSCS->get_id());
//        m_nInstr = pSCS->instrument();
//        m_nStaff = pSCS->staff();
//        m_rTime = pSCS->time();
//        m_nMeasure = pSCS->measure();
//    }
//}
//
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


}  //namespace lomse
