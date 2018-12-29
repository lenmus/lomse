//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#define LOMSE_INTERNAL_API
#include "lomse_tasks.h"

#include "lomse_interactor.h"
#include "lomse_logger.h"

namespace lomse
{


//=======================================================================================
// TaskFactory implementation
//=======================================================================================
TaskFactory::TaskFactory()
{
}

//---------------------------------------------------------------------------------------
TaskFactory::~TaskFactory()
{
}

//---------------------------------------------------------------------------------------
Task* TaskFactory::create_task(int taskType, Interactor* pIntor)
{
    switch(taskType)
    {
        case k_task_drag_view:
            return LOMSE_NEW TaskDragView(pIntor);

        case k_task_null:
            return LOMSE_NEW TaskNull(pIntor);

        case k_task_only_clicks:
            return LOMSE_NEW TaskOnlyClicks(pIntor);

        case k_task_selection:
            return LOMSE_NEW TaskSelection(pIntor);

        case k_task_selection_rectangle:
            return LOMSE_NEW TaskSelectionRectangle(pIntor);

        case k_task_move_object:
            return LOMSE_NEW TaskMoveObject(pIntor);

        case k_task_data_entry:
            return LOMSE_NEW TaskDataEntry(pIntor);

        case k_task_move_handler:
            return LOMSE_NEW TaskMoveHandler(pIntor);

        default:
        {
            LOMSE_LOG_ERROR("invalid task type");
            throw runtime_error("[TaskFactory::create_task] invalid task type");
        }
    }
}


//=======================================================================================
// TaskDragView implementation
//=======================================================================================
void TaskDragView::process_event(Event event)
{
    switch (m_state)
    {
        //--------------------------------------------------------------------------
        case k_waiting_for_first_point:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_down:
                    m_state = k_waiting_for_second_point;
                    start_drag(event);
                    break;
            }
            break;
        }

        //--------------------------------------------------------------------------
        case k_waiting_for_second_point:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_up:
                    m_state = k_waiting_for_first_point;
                    end_drag(event);
                    break;

                case Event::k_mouse_move:
                    m_state = k_waiting_for_second_point;
                    do_drag(event);
                    break;
            }
            break;
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void TaskDragView::init_task()
{
    m_drag_flag = false;
    m_state = k_waiting_for_first_point;
}

//---------------------------------------------------------------------------------------
void TaskDragView::start_drag(Event& event)
{
    m_pIntor->get_viewport(&m_vxOrg, &m_vyOrg);
    m_dx = event.x() + m_vxOrg;
    m_dy = event.y() + m_vyOrg;
    m_drag_flag = true;
}

//---------------------------------------------------------------------------------------
void TaskDragView::do_drag(Event& event)
{
    m_vxOrg = m_dx - event.x();
    m_vyOrg = m_dy - event.y();

    m_pIntor->task_action_drag_the_view(m_vxOrg, m_vyOrg);
}

//---------------------------------------------------------------------------------------
void TaskDragView::end_drag(Event& UNUSED(event))
{
    m_drag_flag = false;
}


//=======================================================================================
// TaskOnlyClicks implementation
//=======================================================================================
TaskOnlyClicks::TaskOnlyClicks(Interactor* pIntor)
    : Task(TaskFactory::k_task_only_clicks, pIntor)
    , m_state(k_start)
    , m_pGView(pIntor)
    , m_xStart(0)
    , m_yStart(0)
{
}

//---------------------------------------------------------------------------------------
void TaskOnlyClicks::process_event(Event event)
{
    switch (m_state)
    {
        //--------------------------------------------------------------------------
        case k_waiting_for_first_point:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_down:
                    m_state = k_waiting_for_point_2;
                    record_first_point(event);
                    break;
                case Event::k_mouse_move:
                    mouse_in_out(event);
                    break;
            }
            break;
       }

        //--------------------------------------------------------------------------
        case k_waiting_for_point_2:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_up:
                    m_state = k_waiting_for_first_point;
                    click_at_point(event);
                    break;
            }
            break;
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void TaskOnlyClicks::init_task()
{
    m_state = k_waiting_for_first_point;
    m_xStart = 0;
    m_yStart = 0;
}

//---------------------------------------------------------------------------------------
void TaskOnlyClicks::record_first_point(Event& event)
{
    m_xStart = event.x();
    m_yStart = event.y();
}

//---------------------------------------------------------------------------------------
void TaskOnlyClicks::click_at_point(Event& event)
{
    m_pIntor->task_action_click_at_screen_point(m_xStart, m_yStart, event.flags());
}

//---------------------------------------------------------------------------------------
void TaskOnlyClicks::mouse_in_out(Event& event)
{
    m_pIntor->task_action_mouse_in_out(event.x(), event.y(), event.flags());
}



//=======================================================================================
// TaskSelection implementation
//=======================================================================================
TaskSelection::TaskSelection(Interactor* pIntor)
    : Task(TaskFactory::k_task_selection, pIntor)
    , m_state(k_start)
    , m_pGView(pIntor)
    , m_xStart(0)
    , m_yStart(0)
{
}

//---------------------------------------------------------------------------------------
void TaskSelection::process_event(Event event)
{
    switch (m_state)
    {
        //--------------------------------------------------------------------------
        case k_waiting_for_first_point:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_down:
                    m_state = k_waiting_for_point_2;
                    record_first_point(event);
                    decide_on_switching_task(event);
                    break;
                case Event::k_mouse_right_down:
                    m_state = k_waiting_for_point_2;
                    record_first_point(event);
                    break;
                case Event::k_mouse_move:
                    mouse_in_out(event);
                    break;
            }
            break;
       }

        //--------------------------------------------------------------------------
        case k_waiting_for_point_2:
        {
            switch (event.type())
            {
                case Event::k_mouse_move:
                    m_state = k_waiting_for_point_2;
                    break;
                case Event::k_mouse_left_up:
                    m_state = k_waiting_for_first_point;
                    break;
                case Event::k_mouse_right_up:
                    m_state = k_waiting_for_first_point;
                    select_object_and_show_contextual_menu(event);
                    break;
            }
            break;
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void TaskSelection::init_task()
{
    m_state = k_waiting_for_first_point;
    m_xStart = 0;
    m_yStart = 0;
}

//---------------------------------------------------------------------------------------
void TaskSelection::record_first_point(Event& event)
{
    m_xStart = event.x();
    m_yStart = event.y();
}

//---------------------------------------------------------------------------------------
void TaskSelection::decide_on_switching_task(Event& event)
{
    m_pIntor->task_action_decide_on_switching_task(event.x(), event.y(), event.flags());
}

//---------------------------------------------------------------------------------------
void TaskSelection::select_object_and_show_contextual_menu(Event& event)
{
    m_pIntor->task_action_select_object_and_show_contextual_menu(m_xStart, m_yStart,
                                                                 event.flags());
}

//---------------------------------------------------------------------------------------
void TaskSelection::mouse_in_out(Event& event)
{
    m_pIntor->task_action_mouse_in_out(event.x(), event.y(), event.flags());
}



//=======================================================================================
// TaskSelectionRectangle implementation
//=======================================================================================
TaskSelectionRectangle::TaskSelectionRectangle(Interactor* pIntor)
    : Task(TaskFactory::k_task_selection_rectangle, pIntor)
    , m_state(k_start)
    , m_pGView(pIntor)
    , m_xStart(0)
    , m_yStart(0)
{
}

//---------------------------------------------------------------------------------------
void TaskSelectionRectangle::process_event(Event event)
{
    switch (m_state)
    {
        //--------------------------------------------------------------------------
        case k_waiting_for_point_2:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_up:
                    m_state = k_request_task_switch;
                    select_objects_or_click(event);
                    switch_to_default_task();
                    break;
                case Event::k_mouse_move:
                    track_sel_rectangle(event);
                    break;
            }
            break;
       }

        //--------------------------------------------------------------------------
        case k_request_task_switch:
        {
            m_state = k_request_task_switch;
            switch_to_default_task();
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void TaskSelectionRectangle::init_task()
{
    m_state = k_waiting_for_point_2;
    m_xStart = 0;
    m_yStart = 0;
}

//---------------------------------------------------------------------------------------
void TaskSelectionRectangle::set_first_point(Pixels xStart, Pixels yStart)
{
    m_xStart = xStart;
    m_yStart = yStart;
}

//---------------------------------------------------------------------------------------
void TaskSelectionRectangle::select_objects_or_click(Event& event)
{
    m_pIntor->hide_selection_rectangle();

    //at least 5 pixels width, height to consider it a selection rectangle
    if (abs(m_xStart - event.x()) < 5 || abs(m_yStart - event.y()) < 5)
        m_pIntor->task_action_click_at_screen_point(m_xStart, m_yStart, event.flags());
    else
        m_pIntor->task_action_select_objects_in_screen_rectangle(m_xStart, m_yStart,
                                                                 event.x(), event.y(),
                                                                 event.flags());
}

//---------------------------------------------------------------------------------------
void TaskSelectionRectangle::track_sel_rectangle(Event& event)
{
    m_pIntor->task_action_update_selection_rectangle(event.x(), event.y());
}

//---------------------------------------------------------------------------------------
void TaskSelectionRectangle::switch_to_default_task()
{
    m_pIntor->task_action_switch_to_default_task();
}


//=======================================================================================
// TaskMoveObject implementation
//=======================================================================================
TaskMoveObject::TaskMoveObject(Interactor* pIntor)
    : Task(TaskFactory::k_task_move_object, pIntor)
    , m_state(k_start)
    , m_pGView(pIntor)
    , m_xStart(0)
    , m_yStart(0)
{
}

//---------------------------------------------------------------------------------------
void TaskMoveObject::process_event(Event event)
{
    switch (m_state)
    {
        //--------------------------------------------------------------------------
        case k_waiting_for_point_2:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_up:
                    m_state = k_request_task_switch;
                    move_object_or_click(event);
                    switch_to_default_task();
                    break;
                case Event::k_mouse_move:
                    move_drag_image(event);
                    break;
            }
            break;
       }

        //--------------------------------------------------------------------------
        case k_request_task_switch:
        {
            m_state = k_request_task_switch;
            switch_to_default_task();
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void TaskMoveObject::init_task()
{
    m_state = k_waiting_for_point_2;
    m_xStart = 0;
    m_yStart = 0;
}

//---------------------------------------------------------------------------------------
void TaskMoveObject::set_first_point(Pixels xStart, Pixels yStart)
{
    m_xStart = xStart;
    m_yStart = yStart;
}

//---------------------------------------------------------------------------------------
void TaskMoveObject::move_object_or_click(Event& event)
{
    //at least 5 pixels width, height to consider it a move object action
    if (abs(m_xStart - event.x()) < 5 || abs(m_yStart - event.y()) < 5)
        m_pIntor->task_action_click_at_screen_point(m_xStart, m_yStart, event.flags());
    else
        m_pIntor->task_action_move_object(event.x(), event.y());
}

//---------------------------------------------------------------------------------------
void TaskMoveObject::move_drag_image(Event& event)
{
    m_pIntor->task_action_move_drag_image(event.x(), event.y());
}

//---------------------------------------------------------------------------------------
void TaskMoveObject::switch_to_default_task()
{
    m_pIntor->task_action_switch_to_default_task();
}


//=======================================================================================
// TaskDataEntry implementation
//=======================================================================================
TaskDataEntry::TaskDataEntry(Interactor* pIntor)
    : Task(TaskFactory::k_task_selection, pIntor)
    , m_state(k_start)
    , m_pGView(pIntor)
    , m_xStart(0)
    , m_yStart(0)
{
}

//---------------------------------------------------------------------------------------
void TaskDataEntry::process_event(Event event)
{
    switch (m_state)
    {
        //--------------------------------------------------------------------------
        case k_waiting_for_first_point:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_down:
                    m_state = k_waiting_for_point_2_left;
                    record_first_point(event);
                    break;
                case Event::k_mouse_right_down:
                    m_state = k_waiting_for_point_2_right;
                    record_first_point(event);
                    break;
                case Event::k_mouse_move:
                    move_drag_image(event);
                    break;
            }
            break;
       }

        //--------------------------------------------------------------------------
        case k_waiting_for_point_2_left:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_up:
                    m_state = k_waiting_for_first_point;
                    insert_object(event);
                    break;
            }
            break;
        }

        //--------------------------------------------------------------------------
        case k_waiting_for_point_2_right:
        {
            switch (event.type())
            {
                case Event::k_mouse_right_up:
                    m_state = k_waiting_for_first_point;
                    show_contextual_menu(event);
                    break;
            }
            break;
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void TaskDataEntry::init_task()
{
    m_state = k_waiting_for_first_point;
    m_xStart = 0;
    m_yStart = 0;
}

//---------------------------------------------------------------------------------------
void TaskDataEntry::record_first_point(Event& event)
{
    m_xStart = event.x();
    m_yStart = event.y();
}

//---------------------------------------------------------------------------------------
void TaskDataEntry::insert_object(Event& event)
{
    m_pIntor->task_action_insert_object_at_point(m_xStart, m_yStart, event.flags());
}

//---------------------------------------------------------------------------------------
void TaskDataEntry::show_contextual_menu(Event& event)
{
    m_pIntor->task_action_select_object_and_show_contextual_menu(m_xStart, m_yStart,
                                                                 event.flags());
}

//---------------------------------------------------------------------------------------
void TaskDataEntry::move_drag_image(Event& event)
{
    m_pIntor->task_action_move_drag_image(event.x(), event.y());
}


//=======================================================================================
// TaskMoveHandler implementation
//=======================================================================================
TaskMoveHandler::TaskMoveHandler(Interactor* pIntor)
    : Task(TaskFactory::k_task_move_object, pIntor)
    , m_state(k_start)
    , m_pGView(pIntor)
    , m_xStart(0)
    , m_yStart(0)
{
}

//---------------------------------------------------------------------------------------
void TaskMoveHandler::process_event(Event event)
{
    switch (m_state)
    {
        //--------------------------------------------------------------------------
        case k_waiting_for_point_2:
        {
            switch (event.type())
            {
                case Event::k_mouse_left_up:
                    m_state = k_request_task_switch;
                    move_handler_end_point(event);
                    switch_to_default_task();
                    break;
                case Event::k_mouse_move:
                    move_handler(event);
                    break;
            }
            break;
       }

        //--------------------------------------------------------------------------
        case k_request_task_switch:
        {
            m_state = k_request_task_switch;
            switch_to_default_task();
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void TaskMoveHandler::init_task()
{
    m_state = k_waiting_for_point_2;
    m_xStart = 0;
    m_yStart = 0;
}

//---------------------------------------------------------------------------------------
void TaskMoveHandler::set_first_point(Pixels xStart, Pixels yStart)
{
    m_xStart = xStart;
    m_yStart = yStart;
}

//---------------------------------------------------------------------------------------
void TaskMoveHandler::move_handler(Event& event)
{
    m_pIntor->task_action_move_handler(event.x(), event.y());
}

//---------------------------------------------------------------------------------------
void TaskMoveHandler::move_handler_end_point(Event& event)
{
    Pixels xTotalShift = event.x() - m_xStart;
    Pixels yTotalShift = event.y() - m_yStart;
    m_pIntor->task_action_move_handler_end_point(event.x(), event.y(),
                                                 xTotalShift, yTotalShift);
}

//---------------------------------------------------------------------------------------
void TaskMoveHandler::switch_to_default_task()
{
    m_pIntor->task_action_switch_to_default_task();
}



}  //namespace lomse
