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

#include "lomse_tasks.h"

#include "lomse_interactor.h"

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
            return new TaskDragView(pIntor);

        case k_task_null:
            return new TaskNull(pIntor);

        case k_task_selection:
            return new TaskSelection(dynamic_cast<Interactor*>(pIntor));

        default:
            throw "TaskFactory::create_task: invalid task type";
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
                    start_drag(event);
                    m_state = k_waiting_for_second_point;
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
                    end_drag(event);
                    m_state = k_waiting_for_first_point;
                    break;

                case Event::k_mouse_move:
                    do_drag(event);
                    m_state = k_waiting_for_second_point;
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
    m_pIntor->get_viewport(&m_vxOrg, &m_vyOrg);
    m_dx = 0;
    m_dy = 0;
    m_drag_flag = false;
    m_state = k_waiting_for_first_point;
}

//---------------------------------------------------------------------------------------
void TaskDragView::start_drag(Event& event)
{
    m_dx = event.x() - m_vxOrg;
    m_dy = event.y() - m_vyOrg;
    m_drag_flag = true;
}

//---------------------------------------------------------------------------------------
void TaskDragView::do_drag(Event& event)
{
    m_vxOrg = event.x() - m_dx;
    m_vyOrg = event.y() - m_dy;

    repaint_view();
}

//---------------------------------------------------------------------------------------
void TaskDragView::end_drag(Event& event)
{
    m_drag_flag = false;
}

//---------------------------------------------------------------------------------------
void TaskDragView::repaint_view()
{
    m_pIntor->new_viewport(m_vxOrg, m_vyOrg);
}


//=======================================================================================
// TaskSelection implementation
//=======================================================================================
TaskSelection::TaskSelection(Interactor* pIntor)
    : Task(TaskFactory::k_task_selection, pIntor)
    , m_state(k_start)
    , m_pGView(pIntor)
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
                    record_first_point(event);
                    m_state = k_waiting_for_point_2_left;
                    break;
                case Event::k_mouse_right_down:
                    record_first_point(event);
                    m_state = k_waiting_for_point_2_right;
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
                    select_objects(event);
                    m_state = k_waiting_for_first_point;
                    break;

                case Event::k_mouse_move:
                    track_sel_rectangle(event);
                    m_state = k_waiting_for_point_2_left;
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
                    select_object_at_first_point(event);
                    show_contextual_menu();
                    m_state = k_waiting_for_first_point;
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
    m_pIntor->show_selection_rectangle(m_xStart, m_yStart, m_xStart, m_yStart);
}

//---------------------------------------------------------------------------------------
void TaskSelection::select_objects(Event& event)
{
    m_pIntor->select_objects_in_screen_rectangle(m_xStart, m_yStart, event.x(), event.y());
    m_pIntor->hide_selection_rectangle();
    repaint_view();
}

//---------------------------------------------------------------------------------------
void TaskSelection::track_sel_rectangle(Event& event)
{
    m_pIntor->update_selection_rectangle(event.x(), event.y());
    repaint_view();
}

//---------------------------------------------------------------------------------------
void TaskSelection::select_object_at_first_point(Event& event)
{
    m_pIntor->select_object_at_screen_point(m_xStart, m_yStart);
    repaint_view();
}

//---------------------------------------------------------------------------------------
void TaskSelection::show_contextual_menu()
{
}

//---------------------------------------------------------------------------------------
void TaskSelection::repaint_view()
{
    m_pIntor->on_paint();
    m_pIntor->update_window();
}



}  //namespace lomse
