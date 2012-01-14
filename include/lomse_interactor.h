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
#include "lomse_injectors.h"
#include "lomse_agg_types.h"
#include "lomse_selections.h"
#include "lomse_events.h"

#include <iostream>
#include <ctime>   //clock
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace lomse
{

//forward declarations
class Document;
class DocCursor;
class Task;
class GraphicModel;
class GmoObj;
class ImoStaffObj;
class ImoScore;
//class UserCommandExecuter;
//class LdpCompiler;


//---------------------------------------------------------------------------------------
//Abstract class from which all Interactors must derive
class Interactor : public EventHandler
                 , public EventNotifier
                 , public Observable
{
protected:
    LibraryScope&   m_libScope;
    Document*       m_pDoc;
    View*           m_pView;
    GraphicModel*   m_pGraphicModel;
    Task*           m_pTask;
    SelectionSet    m_selections;
    GmoObj*         m_pLastMouseOverGmo;

    //for performance measurements
    clock_t     m_startTime;
    ptime       m_start;

    //measured times (milliseconds)
    double      m_renderTime;
    double      m_gmodelBuildTime;

    //to know that view parameters (viewport, scale, ..) has been changed
    bool        m_fViewParamsChanged;

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

    //mandatory override required by EventHandler
	void handle_event(SpEventInfo pEvent);

    //interface to View
    virtual void redraw_bitmap();
    virtual void force_redraw();
    bool view_needs_repaint();

    //creating events
    virtual void request_window_update();
    virtual void send_end_of_play_event(ImoScore* pScore);

    //interface to GraphicView
        //renderization
    virtual void set_rendering_buffer(RenderingBuffer* rbuf);
    virtual void set_rendering_option(int option, bool value);
    virtual void set_box_to_draw(int boxType);
    virtual void reset_boxes_to_draw();
        //units conversion and related
    virtual void screen_point_to_page_point(double* x, double* y);
    virtual void model_point_to_screen(double* x, double* y, int iPage);
    virtual int page_at_screen_point(double x, double y);
        //wiewport / scroll
    virtual void new_viewport(Pixels x, Pixels y);
    virtual void set_viewport_at_page_center(Pixels screenWidth);
    virtual void get_viewport(Pixels* x, Pixels* y);
    virtual void get_view_size(Pixels* xWidth, Pixels* yHeight);
        //scale
    virtual double get_scale();
    virtual void set_scale(double scale, Pixels x=0, Pixels y=0);
    virtual void zoom_in(Pixels x=0, Pixels y=0);
    virtual void zoom_out(Pixels x=0, Pixels y=0);
    virtual void zoom_fit_full(Pixels width, Pixels height);
    virtual void zoom_fit_width(Pixels width);
        //selection rectangle
    virtual void show_selection_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    virtual void hide_selection_rectangle();
    virtual void update_selection_rectangle(Pixels x2, Pixels y2);
        //tempo line
    virtual void show_tempo_line(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    virtual void hide_tempo_line();
    virtual void update_tempo_line(Pixels x2, Pixels y2);
        //highlight
    virtual void highlight_object(ImoStaffObj* pSO);
    virtual void remove_highlight_from_object(ImoStaffObj* pSO);
    virtual void remove_all_highlight();
    void on_visual_highlight(SpEventScoreHighlight pEvent);
        //printing
    virtual void set_printing_buffer(RenderingBuffer* rbuf);
    virtual void on_print_page(int page, double scale, VPoint viewport);
    virtual VSize get_page_size_in_pixels(int nPage);

    //interface to SelectionSet
    virtual void select_object(GmoObj* pGmo, unsigned flags=0);
    virtual bool is_in_selection(GmoObj* pGmo);

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return this; }


    ////abstract class implements all possible commands. Derived classes override
    ////them as needed, either programming a diferent behaviour or as empty methods
    ////for those commands not allowed
    //virtual void insert_rest(DocCursor& cursor, const std::string& source);

    // event handlers for user actions. Library API
    virtual void on_mouse_move(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_down(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_up(Pixels x, Pixels y, unsigned flags);
    //virtual void on_init();
//    virtual void on_resize(Pixels x, Pixels y);
    //virtual void on_idle();
    //virtual void on_key(Pixels x, Pixels y, unsigned key, unsigned flags);
    //virtual void on_ctrl_change();

    //-----------------------------------------------------------------------------------
    //commands

    //selection
    virtual void click_at_screen_point(Pixels x, Pixels y, unsigned flags=0);
    virtual void select_object_at_screen_point(Pixels x, Pixels y, unsigned flags=0);
    virtual void select_objects_in_screen_rectangle(Pixels x1, Pixels y1,
                                                    Pixels x2, Pixels y2,
                                                    unsigned flags=0);

    virtual void mouse_in_out(Pixels x, Pixels y);

    //performance measurements
    void start_timer();
    double get_elapsed_time() const;
    inline void gmodel_rendering_time(double milliseconds) { m_renderTime = milliseconds; }
    inline double gmodel_rendering_time() { return m_renderTime; }
    inline double gmodel_build_time() { return m_gmodelBuildTime; }


protected:
    void create_graphic_model();
    void delete_graphic_model();
    GmoObj* find_object_at(Pixels x, Pixels y);
    void send_mouse_out_event(GmoObj* pGmo);
    void send_mouse_in_event(GmoObj* pGmo);
    void notify_event(SpEventInfo pEvent, GmoObj* pGmo);
    void update_view_if_gmodel_modified();
    void update_view_if_needed();
    void find_parent_link_box_and_notify_event(SpEventInfo pEvent, GmoObj* pGmo);

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
