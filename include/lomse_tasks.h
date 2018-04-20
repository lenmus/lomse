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

#ifndef __LOMSE_TASKS_H__
#define __LOMSE_TASKS_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"

#include <iostream>
using namespace std;

namespace lomse
{

//forward declarations
class Interactor;


//---------------------------------------------------------------------------------------
//Event: encapsulates info about an event

class Event
{
protected:
    int         m_type;
    Pixels      m_x;
    Pixels      m_y;
    unsigned    m_flags;

public:
    enum { k_run=0, k_mouse_move,
           k_mouse_left_down, k_mouse_left_up,
           k_mouse_right_down, k_mouse_right_up,};

    Event(int type, Pixels x=0, Pixels y=0, unsigned flags=0)
        : m_type(type)
        , m_x(x)
        , m_y(y)
        , m_flags(flags)
    {
    }

    //getters
    inline int type() { return m_type; }
    inline Pixels x() { return m_x; }
    inline Pixels y() { return m_y; }
    inline unsigned flags() { return m_flags; }

};


//---------------------------------------------------------------------------------------
//Abstract base class for all tasks that interact with the user
class Task
{
protected:
    int         m_taskType;
    Interactor* m_pIntor;

public:
    Task(int type, Interactor* pIntor) : m_taskType(type), m_pIntor(pIntor) {}
    virtual ~Task() {}

    virtual void init_task() {}
    virtual void process_event(Event event) = 0;

};


//---------------------------------------------------------------------------------------
// factory class to create tasks
class TaskFactory
{
public:
    TaskFactory();
    virtual ~TaskFactory();

    enum { k_task_drag_view=0, k_task_null, k_task_only_clicks, k_task_selection,
           k_task_data_entry, k_task_selection_rectangle, k_task_move_object,
           k_task_move_handler, };

    static Task* create_task(int taskType, Interactor* pIntor);

};


//---------------------------------------------------------------------------------------
//TaskDragView: A task to move (scroll, drag) the view
class TaskDragView : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_first_point, k_waiting_for_second_point };
    int m_state;

    Pixels m_vxOrg, m_vyOrg;    //current viewport origin
    Pixels m_dx, m_dy;          //mouse point while dragging
    bool m_drag_flag;

public:
    TaskDragView(Interactor* pIntor)
        : Task(TaskFactory::k_task_drag_view, pIntor)
        , m_state(k_start) {}
    ~TaskDragView() {}

    void init_task();
    void process_event(Event event);

protected:
    //actions
    void start_drag(Event& event);
    void do_drag(Event& event);
    void end_drag(Event& event);
};


//---------------------------------------------------------------------------------------
//TaskNull: A task doing nothing
class TaskNull : public Task
{
public:
    TaskNull(Interactor* pIntor) : Task(TaskFactory::k_task_null, pIntor) {}
    ~TaskNull() {}

    void process_event(Event UNUSED(event)) {}

};


//---------------------------------------------------------------------------------------
//TaskOnlyClicks: Default task when edition mode disabled. User can only move mouse,
//and click on objects (links, buttons, etc.)
class TaskOnlyClicks : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_first_point, k_waiting_for_point_2, };
    int m_state;
    Interactor* m_pGView;
    Pixels m_xStart, m_yStart;      //first point

public:
    TaskOnlyClicks(Interactor* pIntor);
    ~TaskOnlyClicks() {}

    void init_task();
    void process_event(Event event);

protected:
    //actions
    void record_first_point(Event& event);
    void click_at_point(Event& event);
    void mouse_in_out(Event& event);
};


//---------------------------------------------------------------------------------------
//TaskSelection: A task to select objects.
class TaskSelection : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_first_point, k_waiting_for_point_2, };
    int m_state;
    Interactor* m_pGView;
    Pixels m_xStart, m_yStart;      //first point

public:
    TaskSelection(Interactor* pIntor);
    ~TaskSelection() {}

    void init_task();
    void process_event(Event event);

protected:
    //actions
    void record_first_point(Event& event);
//    void click_at_point();
    void decide_on_switching_task(Event& event);
    void select_object_and_show_contextual_menu(Event& event);
    void mouse_in_out(Event& event);
};


////---------------------------------------------------------------------------------------
////TaskSelection: A task to select objects.
//class TaskSelection : public Task
//{
//protected:
//    //states
//    enum { k_start=0, k_waiting_for_first_point, k_waiting_for_button_up,
//           k_waiting_for_dclick, k_waiting_for_double_up, k_waiting_for_end_point, };
//
//    int m_state;
//    Interactor* m_pGView;
//    Pixels m_xStart, m_yStart;      //first point
//    bool m_fLeftButton;
//    ptime m_downTime;
//
//public:
//    TaskSelection(Interactor* pIntor);
//    ~TaskSelection() {}
//
//    void init_task();
//    void process_event(Event event);
//
//protected:
//    //actions
//    void record_time_point_and_button(Event& event);
//    void mouse_in_out(Event& event);
//    void single_click();
//    void double_click();
//    void start_move_drag();
//    void continue_move_drag(Event& event);
//    void end_move_drag(Event& event);
//
//    double get_elapsed_time();
//
//};


//---------------------------------------------------------------------------------------
//TaskSelectionRectangle: A task to display a rubber band rectangle
class TaskSelectionRectangle : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_point_2, k_request_task_switch };
    int m_state;
    Interactor* m_pGView;
    Pixels m_xStart, m_yStart;      //first point

public:
    TaskSelectionRectangle(Interactor* pIntor);
    ~TaskSelectionRectangle() {}

    void init_task();
    void process_event(Event event);
    void set_first_point(Pixels xStart, Pixels yStart);

protected:
    //actions
    void select_objects_or_click(Event& event);
    void track_sel_rectangle(Event& event);
    void switch_to_default_task();
};


//---------------------------------------------------------------------------------------
//TaskMoveObject: A task to drag an image and, finally, move the object to end point
class TaskMoveObject : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_point_2, k_request_task_switch };
    int m_state;
    Interactor* m_pGView;
    Pixels m_xStart, m_yStart;      //first point

public:
    TaskMoveObject(Interactor* pIntor);
    ~TaskMoveObject() {}

    void init_task();
    void process_event(Event event);
    void set_first_point(Pixels xStart, Pixels yStart);

protected:
    //actions
    void move_drag_image(Event& event);
    void move_object_or_click(Event& event);
    void switch_to_default_task();
};


//---------------------------------------------------------------------------------------
//TaskDataEntry: A task to insert new objects by clicking with the mouse
class TaskDataEntry : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_first_point, k_waiting_for_point_2_left,
           k_waiting_for_point_2_right };
    int m_state;
    Interactor* m_pGView;
    Pixels m_xStart, m_yStart;      //first point

public:
    TaskDataEntry(Interactor* pIntor);
    ~TaskDataEntry() {}

    void init_task();
    void process_event(Event event);

protected:
    //actions
    void record_first_point(Event& event);
    void insert_object(Event& event);
    void show_contextual_menu(Event& event);
    void move_drag_image(Event& event);
};


//---------------------------------------------------------------------------------------
//TaskMoveHandler: A task to move a handler
class TaskMoveHandler : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_point_2, k_request_task_switch };
    int m_state;
    Interactor* m_pGView;
    Pixels m_xStart, m_yStart;      //first point

public:
    TaskMoveHandler(Interactor* pIntor);
    ~TaskMoveHandler() {}

    void init_task();
    void process_event(Event event);
    void set_first_point(Pixels xStart, Pixels yStart);

protected:
    //actions
    void move_handler(Event& event);
    void move_handler_end_point(Event& event);
    void switch_to_default_task();
};



}   //namespace lomse

#endif      //__LOMSE_TASKS_H__
