//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "private/lomse_document_p.h"
#include "lomse_document_cursor.h"
#include "lomse_ldp_elements.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_time.h"
#include "lomse_logger.h"

using namespace std;

namespace lomse
{

//=======================================================================================
// DocContentCursor implementation
//=======================================================================================
DocContentCursor::DocContentCursor(Document* pDoc)
    : m_pDoc(pDoc)
    , m_pCurItem(nullptr)
    , m_parent(nullptr)
{
    start_of_content();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::move_next()
{
    if (m_pCurItem)
    {
        do_move_next();
        while (m_pCurItem && !m_pCurItem->is_edit_terminal())
        {
            do_move_next();
        }
        find_parent();
    }
}

//---------------------------------------------------------------------------------------
void DocContentCursor::do_move_next()
{
    //Depth first forward traversal: non-recursive algorithm
    //-------------------------------------------------------
    //curNode = root;
    //while (true)
    //{
    //    //do something with curNode
    //    if (curNode.has_children())
    //        curNode = curNode.first_child();
    //    else
    //    {
    //        while (!curNode.exists_next_sibbling())
    //        {
    //            if (curNote == root)
    //                return;   //end of traversal
    //            curNode = curNode.get_parent();     //walk up ...
    //        }
    //        curNode = curNode.get_next_sibbling();  //... and right
    //    }
    //}


    if (m_pCurItem->has_children())
        m_pCurItem = m_pCurItem->get_first_child();
    else
    {
        ImoObj* nextSibbling = m_pCurItem->get_next_sibling();
        while (!nextSibbling)
        {
            find_parent();
            if (parent_is_root_node())
            {
                m_pCurItem = nullptr;   //at end
                return;
            }
            m_pCurItem = m_parent;                          //walk up ...
            nextSibbling = m_pCurItem->get_next_sibling();  //... and right
        }
        m_pCurItem = nextSibbling;
    }
}

//---------------------------------------------------------------------------------------
void DocContentCursor::move_prev()
{
    if (m_pCurItem)
    {
        do_move_prev();
        while (m_pCurItem && !m_pCurItem->is_edit_terminal())
        {
            do_move_prev();
        }
    }
    else
    {
        //case: k_cursor_at_end. m_pCurItem is nullptr
        last_of_content();
    }
}

//---------------------------------------------------------------------------------------
void DocContentCursor::do_move_prev()
{
    //Depth first backward traversal: non-recursive algorithm
    //-------------------------------------------------------
    //if (curNode.exists_prev_sibbling())
    //{
    //    curNode = curNode.get_prev_sibbling();
    //    curNode = curNode.down_to_last();
    //}
    //else
    //{
    //    if (curNote == root)
    //        return;   //end of traversal
    //    curNode = curNode.get_parent();
    //}

    if (m_pCurItem)
    {
        ImoObj* prev = m_pCurItem->get_prev_sibling();
        if (prev != nullptr)
        {
            m_pCurItem = prev;
            down_to_last();
        }
        else
        {
            find_parent();
            if (parent_is_root_node())
            {
                start_of_content();
                return;
            }
            m_pCurItem = m_parent;
        }
        find_parent();
    }
}

//---------------------------------------------------------------------------------------
void DocContentCursor::down_to_last()
{
    while(m_pCurItem && !m_pCurItem->is_edit_terminal())
    {
        m_pCurItem = m_pCurItem->get_last_child();
    }
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to_current()
{
    //TODO: implement or remove method
//    if (m_curItemIndex >=0 && m_curItemIndex < m_numContentItems)
//        m_pCurItem = m_pDoc->get_content_item(m_curItemIndex);
//    else
//        m_pCurItem = nullptr;
}

//---------------------------------------------------------------------------------------
bool DocContentCursor::parent_is_root_node()
{
    ImoDocument* pImoDoc = m_pDoc->get_im_root();
    return m_parent == pImoDoc->get_content();
}

//---------------------------------------------------------------------------------------
bool DocContentCursor::is_at_start()
{
    ImoDocument* pImoDoc = m_pDoc->get_im_root();
    ImoContent* pContent = pImoDoc->get_content();
    return m_pCurItem == pContent->get_first_child();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::start_of_content()
{
    ImoDocument* pImoDoc = m_pDoc->get_im_root();
    ImoContent* pContent = pImoDoc->get_content();
    m_pCurItem = pContent->get_first_child();
    m_parent = pContent;
}

//---------------------------------------------------------------------------------------
void DocContentCursor::last_of_content()
{
    ImoDocument* pImoDoc = m_pDoc->get_im_root();
    ImoContent* pContent = pImoDoc->get_content();
    m_pCurItem = pContent->get_last_child();
    down_to_last();
    find_parent();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to(ImoId id)
{
    point_to( m_pDoc->get_pointer_to_imo(id) );
}

//---------------------------------------------------------------------------------------
void DocContentCursor::point_to(ImoObj* pImo)
{
    if (pImo && pImo->is_block_level_obj())
    {
        m_pCurItem = pImo;
        find_parent();
    }
    else
        to_end();
}

//---------------------------------------------------------------------------------------
ImoId DocContentCursor::get_prev_id()
{
    if (is_at_start())
        return k_cursor_before_start;
    else
    {
        ImoObj* pSaveCurItem = m_pCurItem;
        ImoObj* m_pSaveParent = m_parent;
        move_prev();
        ImoId id = m_pCurItem->get_id();
        m_pCurItem = pSaveCurItem;
        m_parent = m_pSaveParent;
        return id;
    }
}

//---------------------------------------------------------------------------------------
void DocContentCursor::find_parent()
{
    if (m_pCurItem)
        m_parent = m_pCurItem->get_block_level_parent();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::to_end()
{
    ImoDocument* pImoDoc = m_pDoc->get_im_root();
    ImoContent* pContent = pImoDoc->get_content();
    m_pCurItem = pContent->get_last_child();
    m_pCurItem = nullptr;
}

//---------------------------------------------------------------------------------------
SpElementCursorState DocContentCursor::get_state()
{
    //TODO: Not needed. Perhaps DocContentCursor should not derive from ElementCursor?
    return std::shared_ptr<ElementCursorState>();
}

//---------------------------------------------------------------------------------------
void DocContentCursor::restore_state(SpElementCursorState UNUSED(spState))
{
    //TODO
}

//---------------------------------------------------------------------------------------
SpElementCursorState DocContentCursor::find_previous_pos_state()
{
    //TODO: Not needed. Perhaps DocContentCursor should not derive from ElementCursor?
    return std::shared_ptr<ElementCursorState>();
}



//=======================================================================================
// DocCursor implementation
//=======================================================================================
DocCursor::DocCursor(Document* pDoc)
    : m_pDoc(pDoc)
    , m_pInnerCursor(nullptr)
    , m_outerCursor(pDoc)
    , m_idJailer(k_no_imoid)
{
}

//---------------------------------------------------------------------------------------
DocCursor::DocCursor(DocCursor* cursor)
    : m_pDoc(cursor->m_pDoc)
    , m_pInnerCursor(nullptr)
    , m_outerCursor(m_pDoc)
    , m_idJailer(cursor->m_idJailer)
{
    point_to( cursor->get_pointee_id() );
}

//---------------------------------------------------------------------------------------
DocCursor::DocCursor(DocCursor& cursor)
    : m_pDoc(cursor.m_pDoc)
    , m_pInnerCursor(nullptr)
    , m_outerCursor(m_pDoc)
    , m_idJailer(cursor.m_idJailer)
{
    point_to( cursor.get_pointee_id() );
}

//---------------------------------------------------------------------------------------
DocCursor::~DocCursor()
{
    delete m_pInnerCursor;
}

//---------------------------------------------------------------------------------------
void DocCursor::to_start()
{
    if (is_inside_terminal_node())
        stop_delegation();
    m_outerCursor.start_of_content();
}

//---------------------------------------------------------------------------------------
ImoObj* DocCursor::get_pointee()
{
	return is_inside_terminal_node() ? m_pInnerCursor->get_pointee()
                                     : m_outerCursor.get_pointee();
}

//---------------------------------------------------------------------------------------
ImoObj* DocCursor::get_parent_object()
{
    if (is_inside_terminal_node())
        return m_outerCursor.get_pointee();
    else if (m_outerCursor.parent_is_root_node())
        return get_pointee();
    else
        return m_outerCursor.get_parent_object();
}

//---------------------------------------------------------------------------------------
ImoId DocCursor::get_pointee_id()
{
    if (is_inside_terminal_node())
        return m_pInnerCursor->get_pointee_id();
    else
    {
        ImoObj* pImo = get_pointee();
        return (pImo == nullptr ? k_cursor_at_end : pImo->get_id() );
    }
}

//---------------------------------------------------------------------------------------
ImoId DocCursor::get_parent_id()
{
    ImoObj* pImo = get_parent_object();
    return (pImo == nullptr ? k_cursor_at_end : pImo->get_id() );
}

//---------------------------------------------------------------------------------------
void DocCursor::enter_element()
{
    if (!is_inside_terminal_node())
    {
        ImoObj* pImo = get_pointee();
        if (pImo && pImo->is_edit_terminal())
            start_delegation();
    }
}

//---------------------------------------------------------------------------------------
void DocCursor::exit_element()
{
    if (!is_jailed())
        stop_delegation();
}

//---------------------------------------------------------------------------------------
void DocCursor::start_delegation()
{
    //Factory method to create delegate cursors
    int type = m_outerCursor.get_pointee()->get_obj_type();

    switch(type)
    {
        case k_imo_score:
        {
            ImoScore* pScore = static_cast<ImoScore*>(m_outerCursor.get_pointee());
            m_pInnerCursor = LOMSE_NEW ScoreCursor(m_pDoc, pScore);
            break;
        }
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
    m_pInnerCursor = nullptr;
}

//---------------------------------------------------------------------------------------
void DocCursor::move_next()
{
//    if (is_at_end_of_child())
//    {
//        stop_delegation();
//        ++m_outerCursor;
//    }
//    else
    if (is_inside_terminal_node())
		m_pInnerCursor->move_next();
	else
		++m_outerCursor;
}

//---------------------------------------------------------------------------------------
void DocCursor::move_prev()
{
    if (is_inside_terminal_node())
        m_pInnerCursor->move_prev();
    else
        --m_outerCursor;
}

//---------------------------------------------------------------------------------------
void DocCursor::move_up()
{
    if (is_inside_terminal_node())
        m_pInnerCursor->move_up();
    else
        m_outerCursor.move_up();
}

//---------------------------------------------------------------------------------------
void DocCursor::move_down()
{
    if (is_inside_terminal_node())
        m_pInnerCursor->move_down();
    else
        m_outerCursor.move_down();
}

//---------------------------------------------------------------------------------------
void DocCursor::to_end()
{
    stop_delegation();
    m_outerCursor.to_end();
}

//---------------------------------------------------------------------------------------
void DocCursor::to_last_top_level()
{
    stop_delegation();
    m_outerCursor.last_of_content();
}

//---------------------------------------------------------------------------------------
void DocCursor::point_to(ImoId id)
{
    if (is_jailed() && id < 0)
        return;

    if (id == k_cursor_before_start)
    {
        to_start();
        move_prev();
    }
    else if (id == k_cursor_at_end)
        to_end();
    else
        point_to( m_pDoc->get_pointer_to_imo(id) );
}

//---------------------------------------------------------------------------------------
void DocCursor::point_to(ImoObj* pImo)
{
    if (is_jailed() && is_out_of_jailer(pImo))
        return;

    do_point_to(pImo);
}

//---------------------------------------------------------------------------------------
void DocCursor::do_point_to(ImoObj* pImo)
{
    if (pImo)
    {
        if (pImo->is_block_level_obj())
        {
            stop_delegation();
            m_outerCursor.point_to( static_cast<ImoBlockLevelObj*>(pImo) );
        }
        else
        {
            ImoBlockLevelObj* pParent = pImo->find_block_level_parent();
            if (m_outerCursor.get_pointee() != pParent)
            {
                stop_delegation();
                m_outerCursor.point_to( pParent );
            }
            if (!is_inside_terminal_node())
                enter_element();
            m_pInnerCursor->point_to(pImo);
        }
    }
    else
        to_end();
}

//---------------------------------------------------------------------------------------
bool DocCursor::is_out_of_jailer(ImoObj* pImo)
{
    if (pImo)
    {
        ImoObj* pParent = pImo->get_block_level_parent();
        while (pParent && pParent->get_id() != m_idJailer)
            pParent = pParent->get_block_level_parent();
        return pParent == nullptr;
    }
    else
        return true;
}

//---------------------------------------------------------------------------------------
DocCursorState DocCursor::find_previous_pos_state()
{
    ImoId id = get_parent_id();
    if (is_inside_terminal_node())
		return DocCursorState(id, m_pInnerCursor->find_previous_pos_state());
	else
		return DocCursorState(m_outerCursor.get_prev_id(),
                              std::shared_ptr<ElementCursorState>());
}

//---------------------------------------------------------------------------------------
string DocCursor::dump_cursor()
{
    if (is_inside_terminal_node())
		return m_pInnerCursor->dump_cursor();
	else
	{
        stringstream sout;
        ImoId id = get_pointee_id();
        sout << "Cursor at top level. id=" << id;
        if (id >= 0L)
            sout << ", type=" << get_pointee()->get_name();
        else
            sout << ", " << DocCursor::id_to_string(id);
        sout << endl;
        return sout.str();
	}
}

//---------------------------------------------------------------------------------------
string DocCursor::id_to_string(ImoId id)
{
    if (id < 0L)
    {
        switch (id)
        {
            case k_cursor_pos_undefined:    return "Pos.undefined";
            case k_cursor_before_start:     return "Before start of doc.";
            case k_cursor_at_end:           return "At end of doc.";
            case k_cursor_at_end_of_child:  return "At end of score";
            case k_cursor_at_empty_place:   return "At empty place";
            case k_cursor_before_start_of_child: return "Before start";
            case k_cursor_at_end_of_staff:  return "At end of staff";
            default:                        return "Invalid value !!!";
        }
    }
    else
        return "Valid object";
}

//---------------------------------------------------------------------------------------
void DocCursor::reset_and_point_to(ImoId id)
{
    if (is_inside_terminal_node())
		m_pInnerCursor->reset_and_point_to(id);
	else
        point_to(id);
}

//---------------------------------------------------------------------------------------
void DocCursor::reset_and_point_after(ImoId id)
{
    if (is_inside_terminal_node())
		m_pInnerCursor->reset_and_point_after(id);
	else if (id == k_cursor_before_start)
        to_start();
	else if (id == k_cursor_at_end)
        to_end();
    else
	{
        point_to(id);
        move_next();
	}
}

//---------------------------------------------------------------------------------------
DocCursorState DocCursor::get_state()
{
    if (is_inside_terminal_node())
		return DocCursorState(get_parent_id(), m_pInnerCursor->get_state());
	else
        return DocCursorState(get_pointee_id(), std::shared_ptr<ElementCursorState>());
}

//---------------------------------------------------------------------------------------
void DocCursor::restore_state(DocCursorState& state)
{
        if (!state.is_inside_terminal_node())
            point_to( state.get_parent_level_id() );
	    else
        {
            point_to( state.get_parent_level_id() );
            enter_element();
            m_pInnerCursor->restore_state( state.get_delegate_state() );
        }
}

//---------------------------------------------------------------------------------------
bool DocCursor::jailed_mode_in(ImoId id)
{
    terminate_jailed_mode();
    if (id >= 0)
    {
        ImoObj* pImo = m_pDoc->get_pointer_to_imo(id);
        if (pImo && pImo->is_edit_terminal())
        {
            m_idJailer = id;
            do_point_to(pImo);
            enter_element();
        }
    }
    return is_jailed();
}


//=======================================================================================
// TimeInfo implementation
//=======================================================================================
TimeInfo::TimeInfo(TimeUnits curTimepos, TimeUnits totalDuration,
                   TimeUnits curBeatDuration, TimeUnits startOfBarTimepos, int iMeasure)
    : m_totalDuration(totalDuration)
    , m_beatDuration(curBeatDuration)
    , m_startOfBarTimepos(startOfBarTimepos)
    , m_curTimepos(curTimepos)
    , m_bar(iMeasure + 1)
{
}

//---------------------------------------------------------------------------------------
long TimeInfo::to_millisecs(int UNUSED(mm))
{
    //TODO
    return 0L;
}

//---------------------------------------------------------------------------------------
float TimeInfo::get_metronome_mm_for_lasting(long UNUSED(millisecs))
{
    //TODO
    return 0.0f;
}

//---------------------------------------------------------------------------------------
float TimeInfo::played_percentage()
{
    if ( is_equal_time(m_totalDuration, 0.0) )
        return 100.0;
    else
        return float( (m_curTimepos * 100.0) / m_totalDuration);
}

//---------------------------------------------------------------------------------------
float TimeInfo::remaining_percentage()
{
    if ( is_equal_time(m_totalDuration, 0.0) )
        return 0.0;
    else
        return float( ((m_totalDuration - m_curTimepos) * 100.0) / m_totalDuration);
}

//---------------------------------------------------------------------------------------
Timecode TimeInfo::get_timecode()
{
    TimeUnits remain = m_curTimepos - m_startOfBarTimepos;
    int beat = int( remain / m_beatDuration );
    remain -= TimeUnits(beat++ * m_beatDuration);
    int n16th = int(remain / k_duration_16th);
    remain -= TimeUnits(n16th * k_duration_16th);
    int ticks = int( 7.5 * remain);

    return Timecode(m_bar, beat, n16th, ticks);
}



//=======================================================================================
// ScoreCursor implementation
//=======================================================================================
ScoreCursor::ScoreCursor(Document* pDoc, ImoScore* pScore)
    : ElementCursor(pDoc)
    , m_pScore(pScore)
    , m_scoreId(pScore->get_id())
    , m_pColStaffObjs( pScore->get_staffobjs_table() )
    , m_timeStep( TimeUnits(k_duration_eighth) )
    , m_curVoice(1)
    , m_totalDuration(0.0)
    , m_curBeatDuration(k_duration_quarter)
    , m_startOfBarTimepos(0.0)
{
    p_determine_total_duration();
    p_to_start_of_staff(0, 0);
}

//---------------------------------------------------------------------------------------
ScoreCursor::~ScoreCursor()
{
}

//---------------------------------------------------------------------------------------
void ScoreCursor::reset_and_point_to(ImoId id)
{
    m_pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(m_scoreId) );
    m_pColStaffObjs = m_pScore->get_staffobjs_table();

    point_to(id);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::reset_and_point_after(ImoId id)
{
    m_pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(m_scoreId) );
    m_pColStaffObjs = m_pScore->get_staffobjs_table();

    if (id == k_cursor_before_start_of_child)
        p_to_start_of_staff(0, 0);
	else
	{
        point_to(id);
        move_next();
	}
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to_end()
{
    p_set_state_as_end_of_score();
    p_find_start_of_measure_and_time_signature();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to(ImoId id)
{
    p_move_iterator_to(id);
    p_set_state_from_iterator();
    p_find_start_of_measure_and_time_signature();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to(ImoObj* pImo)
{
    point_to( pImo->get_id() );
}

//---------------------------------------------------------------------------------------
void ScoreCursor::point_to_barline(ImoId id, int staff)
{
    point_to(id);
    m_currentState.staff(staff);
    p_find_start_of_measure_and_time_signature();
}

////---------------------------------------------------------------------------------------
//void ScoreCursor::to_state(int iInstr, int iStaff, int iMeasure, TimeUnits rTime, ImoId id)
//{
//    p_to_state(iInstr, iStaff, iMeasure, rTime, id);
//    p_find_start_of_measure_and_time_signature();
//}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_up()
{
    //TODO
}

//---------------------------------------------------------------------------------------
void ScoreCursor::move_down()
{
    //TODO
}

//---------------------------------------------------------------------------------------
void ScoreCursor::to_next_staffobj(bool fSkipInChord)
{
    if (is_at_end_of_staff())
        p_to_next_staff();

    else if (p_there_is_iter_object())
    {
        p_to_next_position(fSkipInChord);
        p_update_pointed_object();
    }

    else
        p_update_as_end_of_staff();

    //TODO: Possible optimization: as departure point for move_next is a
    //      situation in which cursor time information is valid, updating this
    //      time info is just controlling current object before movig and saving it.
    //      The, after moving, if cursor has not changed instrument/staff the info
    //      is still valid and it is not necessary to perform next sentence.
    p_find_start_of_measure_and_time_signature();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_next_staff()
{
    if (p_more_staves_in_instrument())
        p_to_start_of_next_staff();
    else if (p_more_instruments())
        p_to_start_of_next_instrument();
    else
    {
        //we are at end of score: remain there
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_next_position(bool fSkipInChord)
{
    //increment measure number if we are in a barline
    if (!is_at_empty_place() && p_iter_object_is_barline())
        m_currentState.measure( p_iter_object_measure() + 1 );

    //determine expected time for next position
    TimeUnits expected = p_iter_object_time() + p_iter_object_duration();

    //skip notes in chord if required
    if (fSkipInChord)
        p_if_chord_move_to_last_note();

    //advance to next object in this staff
    ColStaffObjsIterator itSave = m_it;
    if (!is_at_empty_place())
    {
        p_to_next_in_this_staff();
        p_advance_to_current_voice();
        m_currentState.time(expected);
        if (p_there_is_iter_object() && !is_equal_time(p_iter_object_time(), expected) )
        {
            //object found but not in right time: move to empty position
            m_it = itSave;
            p_move_iterator_to_next();
            p_forward_to_instr_with_time_not_lower_than(expected);
        }
        else
        {
            //no object or object found and in right time: try to find right voice
            itSave = m_it;
            p_advance_to_current_voice();
            if (p_there_is_iter_object() && !is_equal_time(p_iter_object_time(), expected) )
                m_it = itSave;
        }
    }
    else
    {
        p_advance_to_current_voice();
        if (p_there_is_iter_object())
            m_currentState.time(p_iter_object_time());
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_next_in_this_staff()
{
    p_move_iterator_to_next();
    p_advance_if_not_right_staff();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_advance_if_not_right_staff()
{
    while ( p_there_is_iter_object()
            && (!p_iter_object_is_on_instrument( m_currentState.instrument() )
                || !p_iter_object_is_on_staff( m_currentState.staff() )
          ))
    {
        p_move_iterator_to_next();
    }
    p_skip_fwd_implicit_key_time_signatures();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_skip_fwd_implicit_key_time_signatures()
{
    if ( p_there_is_iter_object()
         && p_iter_object_is_on_instrument( m_currentState.instrument() )
         && p_iter_object_is_on_staff( m_currentState.staff() )
         && is_equal_time(p_iter_object_time(), m_currentState.time())
       )
    {
        ImoStaffObj* pImo = p_iter_object();
        if (p_iter_object_staff() > 0
            && (pImo->is_key_signature() || pImo->is_time_signature()) )
        {
            while (p_there_is_iter_object()
                   && p_iter_object_staff() > 0
                   && (pImo->is_key_signature() || pImo->is_time_signature()) )
            {
                p_move_iterator_to_next();
                if (p_there_is_iter_object())
                    pImo = p_iter_object();
            }
            if (p_there_is_iter_object())
                p_advance_if_not_right_staff();
        }
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_advance_to_current_voice()
{
    while (p_there_is_iter_object() && p_iter_object()->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(p_iter_object());
        if (pNR->get_voice() != m_curVoice)
            p_move_iterator_to_next();
        else
            return;
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_if_chord_move_to_last_note()
{
    ImoStaffObj* pImo = p_iter_object();
    if (!pImo->is_note())
        return;

    ImoNote* pNote = static_cast<ImoNote*>(pImo);
    if (!pNote->is_in_chord())
        return;

    ImoChord* pChord = pNote->get_chord();
    ColStaffObjsIterator itSave = m_it;
    p_move_iterator_to_next();
    while (p_there_is_iter_object())
    {
        pImo = p_iter_object();
        if (!pImo->is_note())
            break;
        pNote = static_cast<ImoNote*>(pImo);
        if (!pNote->is_in_chord())
            break;
        if (pNote->get_chord() != pChord)
            break;
        itSave = m_it;
        p_move_iterator_to_next();
    }
    m_it = itSave;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::to_prev_staffobj(bool fSkipInChord)
{
    if (p_is_at_start_of_staff())
        p_to_prev_staff();

    else if (is_at_end_of_staff())
    {
        p_to_last_object_in_staff(fSkipInChord);
        p_skip_back_implicit_key_time_signatures();
        p_back_to_current_voice();
        p_update_state_from_iterator();
    }

    else if (p_there_is_iter_object())
        p_to_prev_position(fSkipInChord);

    else
    {
        //Impossible: were are we?
    }

    p_find_start_of_measure_and_time_signature();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_prev_staff()
{
    if (!p_is_first_staff_of_current_instrument())
        p_to_end_of_prev_staff();
    else if (!p_is_first_instrument())
        p_to_end_of_prev_instrument();
    else
    {
        //we are are start of score: remain there
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
void ScoreCursor::p_to_prev_position(bool fSkipInChord)
{
    //save current time. It should be the end time of previous object, unless
    //there are empty positions
    TimeUnits expected = m_currentState.time();

    //move back to prev object in this staff
    p_skip_back_current_chord_if_any();
    p_to_prev_in_this_staff(fSkipInChord);
    p_back_to_current_voice();

    //determine time at end of this object
    TimeUnits endTime = 0.0;    //start of score
    if (p_there_is_iter_object())
        endTime = p_iter_object_time() + p_iter_object_duration();

    //if object found but not in right time, move to empty position after this object
    m_currentState.time(p_iter_object_time());
    if (p_there_is_iter_object())
    {
        if (!is_equal_time(endTime, expected) )
        {
            m_currentState.time(endTime);
            p_move_iterator_to_next();
            p_forward_to_instr_with_time_not_lower_than(endTime);
        }
        else
        {
            //object found and in right time: try to find right voice
            ColStaffObjsIterator itSave = m_it;
            p_back_to_current_voice();
            if (p_there_is_iter_object() && !is_equal_time(p_iter_object_time(), expected) )
                m_it = itSave;
        }
    }

    p_update_pointed_object();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_prev_in_this_staff(bool fSkipInChord)
{
    p_move_iterator_to_prev();
    p_go_back_if_not_right_staff();
    if (p_there_is_iter_object() && fSkipInChord)
        p_skip_back_current_chord_if_any();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_move_iterator_to_prev()
{
    if (m_it != m_pColStaffObjs->end())
        --m_it;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_back_to_current_voice()
{
    while (p_there_is_iter_object() && p_iter_object()->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(p_iter_object());
        if (pNR->get_voice() != m_curVoice)
            p_move_iterator_to_prev();
        else
            return;
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_last_object_in_staff(bool fSkipInChord)
{
    m_it = ColStaffObjsIterator( m_pColStaffObjs->back() );
    while (p_there_is_iter_object()
           && !(p_iter_object_is_on_instrument(m_currentState.instrument())
                && p_iter_object_is_on_staff(m_currentState.staff())) )
    {
        p_move_iterator_to_prev();
    }

    if (p_there_is_iter_object() && fSkipInChord)
        p_skip_back_current_chord_if_any();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_go_back_if_not_right_staff()
{
    while ( p_there_is_iter_object()
            && (!p_iter_object_is_on_instrument( m_currentState.instrument() )
                || !p_iter_object_is_on_staff( m_currentState.staff() )
          ))
    {
        p_move_iterator_to_prev();
    }
    p_skip_back_implicit_key_time_signatures();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_skip_back_implicit_key_time_signatures()
{
    if ( p_there_is_iter_object()
         && p_iter_object_is_on_instrument( m_currentState.instrument() )
         && p_iter_object_is_on_staff( m_currentState.staff() )
         && is_equal_time(p_iter_object_time(), m_currentState.time())
       )
    {
        ImoStaffObj* pImo = p_iter_object();
        if (p_iter_object_staff() > 0
            && (pImo->is_key_signature() || pImo->is_time_signature()) )
        {
            while (p_iter_object_staff() > 0
                   && (pImo->is_key_signature() || pImo->is_time_signature()) )
            {
                p_move_iterator_to_prev();
                if (p_there_is_iter_object())
                    pImo = p_iter_object();
            }
            p_go_back_if_not_right_staff();
        }
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_skip_back_current_chord_if_any()
{
    ImoStaffObj* pImo = p_iter_object();
    ImoNote* pNote = nullptr;
    if (!pImo || !pImo->is_note())
        return;

    pNote = static_cast<ImoNote*>(pImo);
    ColStaffObjsIterator itSave = m_it;
    if (pNote->is_in_chord())
    {
        ImoChord* pChord = pNote->get_chord();
        while (p_there_is_iter_object())
        {
            pImo = p_iter_object();
            if (!pImo->is_note())
                break;
            pNote = static_cast<ImoNote*>(pImo);
            if (!pNote->is_in_chord())
                break;
            if (pNote->get_chord() != pChord)
                break;
            itSave = m_it;
            p_move_iterator_to_prev();
        }
        m_it = itSave;
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::to_time(int iInstr, int iStaff, TimeUnits rTargetTime)
{
    m_currentState.instrument(iInstr);
    m_currentState.staff(iStaff);

    //optimization: avoid search from start if current time lower than target time
    if (!p_there_is_iter_object() || is_greater_time(p_iter_object_time(), rTargetTime))
        m_it = m_pColStaffObjs->begin();

    p_forward_to_instr_with_time_not_lower_than(rTargetTime);

    //here time is greater or equal. Instr is ok or not found
    if (p_there_is_iter_object())
    {
        //save this point as possible point for empty position
        ColStaffObjsIterator itSave = m_it;
        TimeUnits timeFound = p_iter_object_time();

        //find staff
        p_forward_to_instr_staff_with_time_not_lower_than(rTargetTime);

        //here time is greater or equal. Instr and staff are ok or not found
        if (p_there_is_iter_object() && p_iter_object_is_on_time(timeFound))
        {
            //existing object. This is the point
            m_currentState.time(rTargetTime);
            p_update_pointed_object();
        }
        else
        {
            //empty position, but ref.obj exists
            m_it = itSave;
            m_currentState.time(rTargetTime);
            p_update_pointed_object();
        }
    }
    else
    {
        //end of collection reached. Time not found
        p_to_end_of_staff();
    }

    p_find_start_of_measure_and_time_signature();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::to_measure(int measure, int instr, int staff)
{
    //AWARE: instr==-1 or staff==-1  keep current instrument/staff
    if (instr >= 0)
        m_currentState.instrument(instr);
    if (staff >= 0)
        m_currentState.staff(staff);

    if (measure == 0)
    {
        p_to_start_of_staff(m_currentState.instrument(), m_currentState.staff());
        return;
    }

    m_startOfBarTimepos = 0.0;
    m_curBeatDuration = k_duration_quarter;

    m_it = m_pColStaffObjs->begin();
    while (p_there_is_iter_object())
    {
        if (p_iter_object_instrument() == m_currentState.instrument())
        {
            if (p_iter_object()->is_barline())
            {
                m_startOfBarTimepos = p_iter_object_time();
                if (p_iter_object_measure() == measure - 1)
                {
                    m_currentState.time( p_iter_object_time() );
                    p_update_pointed_object();
                    to_next_staffobj(true);
                    return;
                }
            }
            if (p_iter_object()->is_time_signature())
            {
                ImoTimeSignature* pTS =
                        static_cast<ImoTimeSignature*>( p_iter_object() );
                m_curBeatDuration = pTS->get_beat_duration();
            }
        }
        p_move_iterator_to_next();
    }

    if (p_there_is_iter_object())
    {
        m_currentState.time( p_iter_object_time() );
        p_update_pointed_object();
    }
    else
    {
        //end of collection reached. Measure not found
        p_to_end_of_staff();
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::restore_state(SpElementCursorState spState)
{
    ScoreCursorState* pSCS = dynamic_cast<ScoreCursorState*>(spState.get());
    if (pSCS == nullptr)
        return;

    m_pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(m_scoreId) );
    m_pColStaffObjs = m_pScore->get_staffobjs_table();

    if (pSCS->id() == k_no_imoid)
        to_time(pSCS->instrument(), pSCS->staff(), pSCS->time());
    else
    {
        point_to(pSCS->ref_obj_id());

        m_currentState.instrument( pSCS->instrument() );
        m_currentState.staff( pSCS->staff() );
        m_currentState.measure( pSCS->measure() );
        m_currentState.time( pSCS->time() );
        m_currentState.id( pSCS->id() );
        m_currentState.ref_obj_id( pSCS->ref_obj_id() );
        m_currentState.ref_obj_time( pSCS->ref_obj_time() );
        m_currentState.ref_obj_staff( pSCS->ref_obj_staff() );
    }
}

//---------------------------------------------------------------------------------------
SpElementCursorState ScoreCursor::get_state()
{
    return SpElementCursorState(
                LOMSE_NEW ScoreCursorState(instrument(), staff(), measure(),
                                          time(), id(), staffobj_id_internal(),
                                          ref_obj_time(), ref_obj_staff())
                                );
}

//---------------------------------------------------------------------------------------
SpElementCursorState ScoreCursor::find_previous_pos_state()
{
    if (p_is_iterator_at_start_of_score())
    {
        ScoreCursorState* pPrevState = LOMSE_NEW ScoreCursorState();
        pPrevState->set_before_start_of_score();
        return SpElementCursorState(pPrevState);
    }
    else
    {
        SpElementCursorState spState = get_state();
        move_prev();
        SpElementCursorState spPrevState = get_state();
        restore_state(spState);
        return spPrevState;
    }
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ScoreCursor::staffobj()
{
    if (m_currentState.id() >= 0L)
        return (*m_it)->imo_object();
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoObj* ScoreCursor::staffobj_internal()
{
    if (m_it != m_pColStaffObjs->end())
        return (*m_it)->imo_object();
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
TimeInfo ScoreCursor::get_time_info()
{
    return TimeInfo(m_currentState.time(), m_totalDuration, m_curBeatDuration,
                    m_startOfBarTimepos, m_currentState.measure());
}

//---------------------------------------------------------------------------------------
void ScoreCursor::change_voice_to(int voice)
{
    set_current_voice(voice);
    ColStaffObjsIterator itCur = m_it;
    p_to_nearest_position_in_current_voice();

    if (p_there_is_iter_object())
    {
        //AWARE: by design, here p_iter_object_time() >= m_currentState.time()

        if (is_equal_time(m_currentState.time(), p_iter_object_time()))
        {
            //object in new voice at same timepos than current object in old voice
            p_update_pointed_object();
        }
        else
        {
            //there is an object in new voice occupying current time. If it
            //is a gap, allow current time as empty position. Otherwise, advance
            //to found object.
            ColStaffObjsIterator itSave = m_it;
            p_move_iterator_to_prev();
            p_back_to_current_voice();
            if (p_iter_object()->is_gap())
            {
                //allow empty position at current time
                m_it = itCur;
                m_currentState.id(k_cursor_at_empty_place);
            }
            else
            {
                //advance to new time & object
                m_it = itSave;
                m_currentState.time( p_iter_object_time() );
                p_update_pointed_object();
            }
        }
    }
    else
    {
        //allow empty pos at current object & time
        if (!is_at_end_of_staff())
            m_currentState.id(k_cursor_at_empty_place);
        else
            p_update_as_end_of_staff();
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_nearest_position_in_current_voice()
{
    ColStaffObjsIterator itSave;
    while (p_there_is_iter_object()
           && is_equal_time(m_currentState.time(), p_iter_object_time())
           && p_iter_object()->is_note_rest())
    {
        itSave = m_it;
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(p_iter_object());
        if (pNR->get_voice() != m_curVoice)
            p_move_iterator_to_prev();
        else
            return;
    }

    if (p_there_is_iter_object()
        && !is_equal_time(m_currentState.time(), p_iter_object_time()))
    {
        m_it = itSave;
        p_advance_to_current_voice();
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_move_iterator_to_next()
{
    ++m_it;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_move_iterator_to(ImoId id)
{
    if (id <= k_no_imoid)
        m_it = m_pColStaffObjs->end();
    else
    {
        m_it = m_pColStaffObjs->begin();
        for (; m_it != m_pColStaffObjs->end() && (*m_it)->element_id() != id; ++m_it);
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_start_of_staff(int instr, int staff)
{
    m_currentState.instrument(instr).staff(staff).measure(0).time(0.0);
    m_it = m_pColStaffObjs->begin();
    if (p_there_is_iter_object())
    {
        p_advance_if_not_right_staff();
        p_update_pointed_object();
    }
    else
        //empty score
        p_set_state_as_end_of_score();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_start_of_next_staff()
{
    int staff = m_currentState.staff() + 1;
    p_to_start_of_staff(m_currentState.instrument(), staff);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_update_as_end_of_staff()
{
    m_currentState.id( k_cursor_at_end_of_staff );
    m_currentState.ref_obj_id(k_no_imoid);
    m_currentState.ref_obj_time(0.0);
    m_currentState.ref_obj_staff(0);

    //check if end of score
    int instr = m_pScore->get_num_instruments() - 1;
    if (m_currentState.instrument() == instr)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(instr);
        int staff = pInstr->get_num_staves() - 1;
        if (m_currentState.staff() == staff)
            m_currentState.id( k_cursor_at_end_of_child );

        //fix end time for current voice
        p_to_last_object_in_staff(false /*do not skip in chord*/);
        p_back_to_current_voice();
        if (p_there_is_iter_object())
        {
            m_currentState.time( p_iter_object_time() + p_iter_object_duration() );
            m_it = m_pColStaffObjs->end();
        }
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_set_state_as_end_of_score()
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
        TimeUnits time = pEntry->time();
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
        m_currentState.time(0.0);
    }


    m_currentState.id( k_cursor_at_end_of_child );  //k_cursor_at_end_of_staff );
    m_currentState.ref_obj_id( k_no_imoid );
    m_currentState.ref_obj_time(0.0);
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
    p_to_start_of_staff(m_currentState.instrument()+1, 0);
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_update_state_from_iterator()
{
    //AWARE: This method assumes that instr & staff are valid, and updates all
    // other informatin based on iterator position

    if (p_there_is_iter_object())
    {
        //pointing to that object
        m_currentState.measure( p_iter_object_measure() );
        m_currentState.time( p_iter_object_time() );
        m_currentState.id( p_iter_object_id() );
        m_currentState.ref_obj_id( p_iter_object_id() );
        m_currentState.ref_obj_time( p_iter_object_time() );
        m_currentState.ref_obj_staff( p_iter_object_staff() );
    }
    else
    {
        //end of staff or end of score
        ImoInstrument* pInstr = m_pScore->get_instrument( m_currentState.instrument() );
        int staff = pInstr->get_num_staves() - 1;
        if (m_currentState.staff() == staff)
        {
            if (p_more_instruments())
                p_update_as_end_of_staff();
            else
                p_set_state_as_end_of_score();
        }
        else
            p_update_as_end_of_staff();
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_set_state_from_iterator()
{
    //AWARE: This method assumes that current position (m_currentState) has nothing
    // to do with the real position. Therefore, all information is set based on
    // iterator position.

    if (p_there_is_iter_object())
    {
        m_currentState.instrument( p_iter_object_instrument() );
        m_currentState.staff( p_iter_object_staff() );
        m_currentState.measure( p_iter_object_measure() );
        m_currentState.time( p_iter_object_time() );
        m_currentState.id( p_iter_object_id() );
        m_currentState.ref_obj_id( p_iter_object_id() );
        m_currentState.ref_obj_time( p_iter_object_time() );
        m_currentState.ref_obj_staff( p_iter_object_staff() );
    }
    else
    {
        //As there is not enough information, move to end of score
        p_set_state_as_end_of_score();
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_update_pointed_object()
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
            m_currentState.ref_obj_staff( p_iter_object_staff() );
        }
        else
        {
            m_currentState.id( k_cursor_at_empty_place );
            m_currentState.ref_obj_id( p_iter_object_id() );
            m_currentState.ref_obj_time( p_iter_object_time() );
            m_currentState.ref_obj_staff( p_iter_object_staff() );
        }
    }
    else
        p_update_as_end_of_staff();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_forward_to_instr_staff_with_time_not_lower_than(TimeUnits rTargetTime)
{
    while (p_there_is_iter_object()
           && ( !p_iter_object_is_on_instrument( m_currentState.instrument() )
                || !p_iter_object_is_on_staff( m_currentState.staff() )
                || is_greater_time(rTargetTime, p_iter_object_time()) ))
    {
        p_move_iterator_to_next();
    }
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_forward_to_instr_with_time_not_lower_than(TimeUnits rTargetTime)
{
    while (p_there_is_iter_object()
           && ( !p_iter_object_is_on_instrument( m_currentState.instrument() )
                || is_greater_time(rTargetTime, p_iter_object_time()) )
          )
    {
        p_move_iterator_to_next();
    }
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_is_at_start_of_staff()
{
    if (m_currentState.measure() != 0 || !is_equal_time(m_currentState.time(), 0.0))
        return false;

    ColStaffObjsIterator itSave = m_it;
    if (!p_there_is_iter_object())
        m_it = ColStaffObjsIterator( m_pColStaffObjs->back() );
    else
        p_move_iterator_to_prev();

    while (p_there_is_iter_object() && is_equal_time(p_iter_object_time(), 0.0))
    {
        if (p_iter_object_staff() == m_currentState.staff()
            && p_iter_object_instrument() == m_currentState.instrument() )
        {
            m_it = itSave;
            return false;
        }
        p_move_iterator_to_prev();
    }
    m_it = itSave;
    return true;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_end_of_prev_staff()
{
    m_currentState.time(0.0);
    m_currentState.staff( m_currentState.staff() - 1 );

    p_to_end_of_staff();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_to_end_of_prev_instrument()
{
    m_currentState.time(0.0);
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
    m_it = ColStaffObjsIterator( m_pColStaffObjs->back() );
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
            TimeUnits time = p_iter_object_time();
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
            m_currentState.time(0.0);
        }
    }
    else
    {
        m_currentState.measure(0);
        m_currentState.time(0.0);
    }

    //move to end of staff
    m_it = m_pColStaffObjs->end();

    p_update_as_end_of_staff();
}

//---------------------------------------------------------------------------------------
TimeUnits ScoreCursor::p_iter_object_duration()
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
    return (pImo != nullptr);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_iter_object_is_key()
{
    ImoKeySignature* pImo = dynamic_cast<ImoKeySignature*>((*m_it)->imo_object());
    return (pImo != nullptr);
}

//---------------------------------------------------------------------------------------
bool ScoreCursor::p_iter_object_is_time()
{
    ImoTimeSignature* pImo = dynamic_cast<ImoTimeSignature*>((*m_it)->imo_object());
    return (pImo != nullptr);
}

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
         << ", ref_time=" << m_currentState.ref_obj_time()
         << ", ref_staff=" << m_currentState.ref_obj_staff();


    if (m_currentState.id() < 0L)
        sout << " (" << DocCursor::id_to_string(m_currentState.id()) << ")";
    sout << endl;

    return sout.str();
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_determine_total_duration()
{
    ColStaffObjsEntry* pEntry = nullptr;
    if (m_pColStaffObjs->num_entries() > 0)
        pEntry = m_pColStaffObjs->back();

    if (pEntry)
        m_totalDuration = pEntry->time() + pEntry->duration();
    else
        m_totalDuration = 0.0;
}

//---------------------------------------------------------------------------------------
void ScoreCursor::p_find_start_of_measure_and_time_signature()
{
    bool fBarlineFound = false;
    bool fTimeFound = false;
    m_startOfBarTimepos = 0.0;
    m_curBeatDuration = k_duration_quarter;

    ColStaffObjsIterator itSave = m_it;
    p_move_iterator_to_prev();
    while (p_there_is_iter_object())
    {
        if (p_iter_object_instrument() == m_currentState.instrument())
        {
            if (!fBarlineFound && p_iter_object()->is_barline())
            {
                m_startOfBarTimepos = p_iter_object_time();
                fBarlineFound = true;
            }
            if (!fTimeFound && p_iter_object()->is_time_signature())
            {
                ImoTimeSignature* pTS =
                        static_cast<ImoTimeSignature*>( p_iter_object() );
                m_curBeatDuration = pTS->get_beat_duration();
                fTimeFound = true;
            }
        }
        if (fBarlineFound && fTimeFound)
            break;
        p_move_iterator_to_prev();
    }
    m_it = itSave;
}


}  //namespace lomse
