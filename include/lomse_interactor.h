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

#ifndef __LOMSE_INTERACTOR_H__
#define __LOMSE_INTERACTOR_H__

#include "lomse_basic.h"
#include "lomse_observable.h"
#include "lomse_injectors.h"
#include "lomse_agg_types.h"
#include "lomse_selections.h"
#include <iostream>
using namespace std;

namespace lomse
{

//forward declarations
class Document;
class DocCursor;
class Task;
class GraphicModel;
class GmoObj;
//class UserCommandExecuter;
//class LdpCompiler;


//---------------------------------------------------------------------------------------
//Abstract class from which all Interactors must derive
class Interactor : public Observer
{
protected:
    LibraryScope&   m_libScope;
    Document*       m_pDoc;
    View*           m_pView;
    GraphicModel*   m_pGraphicModel;
    Task*           m_pTask;
    SelectionSet    m_selections;
    //UserCommandExecuter*    m_pExec;
    //LdpCompiler*            m_pCompiler;

public:
    Interactor(LibraryScope& libraryScope, Document* pDoc, View* pView); //, UserCommandExecuter* pExec);
    virtual ~Interactor();

    virtual void on_document_reloaded();
    void switch_task(int taskType);

    //access to collaborators
    GraphicModel* get_graphic_model();
    inline View* get_view() { return m_pView; }

    //observed object notificationson_paint
	void handle_event(Observable* ref);

    //interface to View
    virtual void on_paint();
    virtual void update_window();

    //interface to GraphicView
    virtual void new_viewport(Pixels x, Pixels y);
    virtual void set_rendering_buffer(RenderingBuffer* rbuf);
    virtual void get_viewport(Pixels* x, Pixels* y);
    virtual void show_selection_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    virtual void hide_selection_rectangle();
    virtual void update_selection_rectangle(Pixels x2, Pixels y2);
    virtual void screen_point_to_model(double* x, double* y);
    virtual void model_point_to_screen(double* x, double* y, int iPage);
    virtual int page_at_screen_point(double x, double y);
    virtual void zoom_in(Pixels x=0, Pixels y=0);
    virtual void zoom_out(Pixels x=0, Pixels y=0);
    virtual void set_rendering_option(int option, bool value);

    //setting callbacks
    void set_update_window_callbak(void* pThis, void (*pt2Func)(void* pObj));
    void set_force_redraw_callbak(void* pThis, void (*pt2Func)(void* pObj));
    void set_start_timer_callbak(void* pThis, void (*pt2Func)(void* pObj));
    void set_elapsed_time_callbak(void* pThis, double (*pt2Func)(void* pObj));
    void set_notify_callback(void* pThis, void (*pt2Func)(void* pObj, EventInfo& event));

    //interface to SelectionSet
    virtual void select_object(GmoObj* pGmo, unsigned flags=0);
    virtual bool is_in_selection(GmoObj* pGmo);


    ////abstract class implements all possible commands. Derived classes override
    ////them as needed, either programming a diferent behaviour or as empty methods
    ////for those commands not allowed
    //virtual void insert_rest(DocCursor& cursor, const std::string& source);

    // event handlers for user actions. Library API
    virtual void on_mouse_move(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_down(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_up(Pixels x, Pixels y, unsigned flags);
    //virtual void on_init();
    //virtual void on_resize(Pixels x, Pixels y);
    //virtual void on_idle();
    //virtual void on_key(Pixels x, Pixels y, unsigned key, unsigned flags);
    //virtual void on_ctrl_change();

    //-----------------------------------------------------------------------------------
    //commands

    //selection
    virtual void select_object_at_screen_point(Pixels x, Pixels y, unsigned flags=0);
    virtual void select_objects_in_screen_rectangle(Pixels x1, Pixels y1,
                                                    Pixels x2, Pixels y2,
                                                    unsigned flags=0);

protected:
    void create_graphic_model();

};


//---------------------------------------------------------------------------------------
//A view to edit the score in full page
class EditInteractor : public Interactor
{
protected:

public:

    EditInteractor(LibraryScope& libraryScope, Document* pDoc, View* pView); //, UserCommandExecuter* pExec);
    virtual ~EditInteractor();

};



}   //namespace lomse

#endif      //__LOMSE_INTERACTOR_H__
