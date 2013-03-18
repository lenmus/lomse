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

#ifndef __LOMSE_INTERACTOR_H__
#define __LOMSE_INTERACTOR_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_agg_types.h"
#include "lomse_selections.h"
#include "lomse_events.h"

#include <iostream>
#include <ctime>   //clock
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using namespace boost::posix_time;

namespace lomse
{

//forward declarations
class CaretPositioner;
class DocCommandExecuter;
class DocCommand;
class Document;
class DocCursor;
class GmoObj;
class GraphicModel;
class ImoScore;
class ImoStaffObj;
class PlayerGui;
class Task;


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
    DocCursor*      m_pCursor;
    DocCommandExecuter* m_pExec;
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

    //to avoid problems during playback
    bool        m_fViewUpdatesEnabled;

public:
    Interactor(LibraryScope& libraryScope, Document* pDoc, View* pView,
               DocCommandExecuter* pExec);
    virtual ~Interactor();

    virtual void on_document_reloaded();
    void switch_task(int taskType);

    //access to collaborators
    GraphicModel* get_graphic_model();
    inline View* get_view() { return m_pView; }
    inline DocCursor* get_cursor() { return m_pCursor; }

    //mandatory override required by EventHandler
	void handle_event(SpEventInfo pEvent);

    //interface to View
    virtual void redraw_bitmap();
    virtual void force_redraw();
    bool view_needs_repaint();

    //creating events
    virtual void request_window_update();
    virtual void send_end_of_play_event(ImoScore* pScore, PlayerGui* pPlayCtrl);

    //play back
    inline void enable_view_updates(bool value) { m_fViewUpdatesEnabled = value; }


    //interface to GraphicView
        //renderization
    virtual void set_rendering_buffer(RenderingBuffer* rbuf);
    virtual void set_rendering_option(int option, bool value);
    virtual void set_box_to_draw(int boxType);
    virtual void reset_boxes_to_draw();
    virtual void update_caret();
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

    //caret
    bool blink_caret();
    void show_caret();
    void hide_caret();
    string get_caret_timecode();

    ////abstract class implements all possible commands. Derived classes override
    ////them as needed, either programming a diferent behaviour or as empty methods
    ////for those commands not allowed
    //virtual void insert_rest(DocCursor& cursor, const std::string& source);
    virtual void exec_command(DocCommand* pCmd);

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
    void do_force_redraw();
    long find_event_originator_imo(GmoObj* pGmo);

};


//---------------------------------------------------------------------------------------
//A view to edit the score in full page
class EditInteractor : public Interactor
{
protected:

public:

    EditInteractor(LibraryScope& libraryScope, Document* pDoc, View* pView,
                   DocCommandExecuter* pExec);
    virtual ~EditInteractor();

};



}   //namespace lomse

#endif      //__LOMSE_INTERACTOR_H__
