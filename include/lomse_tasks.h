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
// Event flags for mouse and keyboard pressed keys
enum EInputFlag
{
    k_mouse_left  = 1,
    k_mouse_right = 2,
    k_mouse_middle = 4,
    k_kbd_shift   = 8,
    k_kbd_ctrl    = 16,
    k_kbd_alt     = 32,
};


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

    enum { k_task_drag_view=0, k_task_null, k_task_selection, };

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

    //helpers
    void repaint_view();

};


//---------------------------------------------------------------------------------------
//TaskNull: A task doing nothing
class TaskNull : public Task
{
public:
    TaskNull(Interactor* pIntor) : Task(TaskFactory::k_task_null, pIntor) {}
    ~TaskNull() {}

    void process_event(Event event) {}

};


//---------------------------------------------------------------------------------------
//TaskSelection: A task to select objects.
class TaskSelection : public Task
{
protected:
    //states
    enum { k_start=0, k_waiting_for_first_point, k_waiting_for_point_2_left,
           k_waiting_for_point_2_right };
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
    void select_objects_or_click(Event& event);
    void track_sel_rectangle(Event& event);
    void select_object_at_first_point(Event& event);
    void show_contextual_menu();
    void mouse_in_out(Event& event);

    //helpers
    void repaint_view();
};



}   //namespace lomse

#endif      //__LOMSE_TASKS_H__
