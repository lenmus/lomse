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
#include "lomse_document_cursor.h"
#include "lomse_pitch.h"

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
class DocCursor;
class GmoObj;
class GmoBox;
class GraphicModel;
class ImoScore;
class ImoStaffObj;
class PlayerGui;
class Task;
class Handler;

class Document;
typedef SharedPtr<Document>     SpDocument;
typedef WeakPtr<Document>       WpDocument;

class GmoShape;
typedef SharedPtr<GmoShape>  SpGmoShape;

//some constants for improving code legibillity
#define k_no_redraw     false   //do not force view redraw
#define k_force_redraw  true    //force a view redraw

#define k_get_ownership         true
#define k_do_not_get_ownership  false

//---------------------------------------------------------------------------------------
// Event flags for mouse and keyboard pressed keys
enum EEventFlag
{
    k_mouse_left  = 1,
    k_mouse_right = 2,
    k_mouse_middle = 4,
    k_kbd_shift   = 8,
    k_kbd_ctrl    = 16,
    k_kbd_alt     = 32,
};

//---------------------------------------------------------------------------------------
//Abstract class from which all Interactors must derive
class Interactor : public EventHandler
                 , public EventNotifier
                 , public Observable
                 , public EnableSharedFromThis<Interactor>
{
protected:
    LibraryScope&   m_libScope;
    WpDocument      m_wpDoc;
    View*           m_pView;
    GraphicModel*   m_pGraphicModel;
    Task*           m_pTask;
    DocCursor*      m_pCursor;
    SelectionSet*   m_pSelections;
    DocCommandExecuter* m_pExec;
    GmoRef          m_grefLastMouseOver;
    int             m_operatingMode;
    bool            m_fEditionEnabled;

    //for controlling repaints
    bool        m_fViewParamsChanged;       //viewport, scale, ... have been modified

    //to avoid problems during playback
    bool        m_fViewUpdatesEnabled;

    Handler*    m_pCurHandler;  //current handler being dragged, if any
    ImoId       m_idControlledImo;

public:

    //enums
    enum EInteractorOpMode { k_mode_read_only=0, k_mode_edition, k_mode_playback, };
    enum ETimingTarget { k_timing_gmodel_build_time=0, k_timing_gmodel_draw_time,
       k_timing_visual_effects_draw_time, k_timing_total_render_time,
       k_timing_repaint_time, k_timing_max_value, };


    //operating modes and related
    void set_operating_mode(int mode);
    inline int get_operating_mode() { return m_operatingMode; }
    void enable_edition_restricted_to(ImoId id);
    bool is_document_editable();    //not yet implemented. Made private
    void switch_task(int taskType);

    //access to collaborators in MVC model
    GraphicModel* get_graphic_model();
    inline View* get_view() { return m_pView; }
    inline DocCursor* get_cursor() { return m_pCursor; }
    inline SelectionSet* get_selection_set() { return m_pSelections; }

    //interface to View
    virtual void redraw_bitmap();
    virtual void force_redraw();
    bool view_needs_repaint();

    //creating events
    virtual void send_end_of_play_event(ImoScore* pScore, PlayerGui* pPlayCtrl);

    //play back
    inline void enable_forced_view_updates(bool value) { m_fViewUpdatesEnabled = value; }


    //interface to GraphicView
        //renderization
    virtual void set_rendering_buffer(RenderingBuffer* rbuf);
    virtual void set_rendering_option(int option, bool value);
        //units conversion and related
    virtual void screen_point_to_page_point(double* x, double* y);
    virtual void model_point_to_screen(double* x, double* y, int iPage);
    virtual int page_at_screen_point(double x, double y);
    virtual UPoint screen_point_to_model_point(Pixels x, Pixels y);
    DiatonicPitch get_pitch_at(Pixels x, Pixels y);
        //wiewport / scroll
    virtual void new_viewport(Pixels x, Pixels y, bool fForceRedraw=true);
    virtual void set_viewport_at_page_center(Pixels screenWidth);
    virtual void get_viewport(Pixels* x, Pixels* y);
    virtual void get_view_size(Pixels* xWidth, Pixels* yHeight);
        //scale
    virtual double get_scale();
    virtual void set_scale(double scale, Pixels x=0, Pixels y=0, bool fForceRedraw=true);
    virtual void zoom_in(Pixels x=0, Pixels y=0, bool fForceRedraw=true);
    virtual void zoom_out(Pixels x=0, Pixels y=0, bool fForceRedraw=true);
    virtual void zoom_fit_full(Pixels width, Pixels height, bool fForceRedraw=true);
    virtual void zoom_fit_width(Pixels width, bool fForceRedraw=true);
        //selection rectangle
    virtual void start_selection_rectangle(Pixels x1, Pixels y1);
    virtual void hide_selection_rectangle();
        //tempo line
    virtual void show_tempo_line(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    virtual void hide_tempo_line();
    virtual void update_tempo_line(Pixels x2, Pixels y2);
        //highlight
    virtual void highlight_object(ImoStaffObj* pSO);
    virtual void remove_highlight_from_object(ImoStaffObj* pSO);
    virtual void remove_all_highlight();
    virtual void discard_all_highlight();
    virtual void on_visual_highlight(SpEventScoreHighlight pEvent);
        //printing
    virtual void set_printing_buffer(RenderingBuffer* rbuf);
    virtual void on_print_page(int page, double scale, VPoint viewport);
    virtual VSize get_page_size_in_pixels(int nPage);
    virtual int get_num_pages();

    //interface to SelectionSet
    virtual void select_object(GmoObj* pGmo, bool fClearSelection=true);
    virtual void select_object(ImoId id, bool fClearSelection=true);
//    void select_object(ImoObj* pImo, bool fClearSelection=true);
    virtual bool is_in_selection(GmoObj* pGmo);

    //caret
    void blink_caret();
//    void show_caret(bool fShow=true);
//    void hide_caret();
    string get_caret_timecode();
    DocCursorState click_event_to_cursor_state(SpEventMouse event);
    void select_voice(int voice);

    //dragged image associated to mouse cursor
    void show_drag_image(bool value);
    void set_drag_image(GmoShape* pShape, bool fGetOwnership, UPoint offset);
    void enable_drag_image(bool fEnabled);

    //edition commands
    void exec_command(DocCommand* pCmd);
    void exec_undo();
    void exec_redo();

    //edition related info
    bool should_enable_edit_undo();
    bool should_enable_edit_redo();
//    void enable_edition(bool value);
//    inline bool is_edition_enabled() { return m_fEditionEnabled; }

    // event handlers for user actions and related
    virtual void on_document_updated();
    virtual void on_mouse_move(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_down(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_button_up(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_enter_window(Pixels x, Pixels y, unsigned flags);
    virtual void on_mouse_leave_window(Pixels x, Pixels y, unsigned flags);
    //virtual void on_init();
//    virtual void on_resize(Pixels x, Pixels y);
    //virtual void on_idle();
    //virtual void on_key(Pixels x, Pixels y, unsigned key, unsigned flags);
    //virtual void on_ctrl_change();


    //actions requested by Task objects
    virtual void task_action_click_at_screen_point(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_select_objects_in_screen_rectangle(Pixels x1, Pixels y1,
                                                                Pixels x2, Pixels y2,
                                                                unsigned flags);
    virtual void task_action_select_object_and_show_contextual_menu(Pixels x, Pixels y,
                                                                    unsigned flags);
    virtual void task_action_mouse_in_out(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_insert_object_at_point(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_drag_the_view(Pixels x, Pixels y);
    virtual void task_action_decide_on_switching_task(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_switch_to_default_task();
    virtual void task_action_move_drag_image(Pixels x, Pixels y);
    virtual void task_action_move_object(Pixels x, Pixels y);
    virtual void task_action_move_handler(Pixels x, Pixels y);
    virtual void task_action_move_handler_end_point(Pixels xFinal, Pixels yFinal,
                                                    Pixels xTotalShift, Pixels yTotalShift);
    virtual void task_action_update_selection_rectangle(Pixels x2, Pixels y2);

//    virtual void task_action_single_click_at(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_double_click_at(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_start_move_drag(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_continue_move_drag(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_end_move_drag(Pixels x, Pixels y, bool fLeftButton,
//                                           Pixels xTotalShift, Pixels yTotalShift);

    //for performance measurements
    void timing_repaint_done();
    inline double* get_ellapsed_times() { return &m_ellapsedTimes[0]; }

    //Debugging
    virtual void set_box_to_draw(int boxType);
    virtual void reset_boxes_to_draw();
    string dump_cursor();
    string dump_selection();


//excluded from public API. Only for internal use.
#ifdef LOMSE_INTERNAL_API
public:
    Interactor(LibraryScope& libraryScope, WpDocument wpDoc, View* pView,
               DocCommandExecuter* pExec);
    virtual ~Interactor();

    inline SharedPtr<Interactor> get_shared_ptr_from_this() { return shared_from_this(); }

    //mandatory override required by EventHandler
	void handle_event(SpEventInfo pEvent);

    //Deprecated ?
    virtual void highlight_voice(int voice);

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return this; }

    //for performance measurements
    void timing_start_measurements();
    void timing_graphic_model_build_end();
    void timing_graphic_model_render_end();
    void timing_visual_effects_start();
    void timing_renderization_end();

#else
protected:
    Interactor(LibraryScope& libraryScope, WpDocument wpDoc, View* pView,
               DocCommandExecuter* pExec);

#endif  //excluded from public API

protected:
    //for performance measurements
    double m_ellapsedTimes[k_timing_max_value];
    ptime m_renderStartTime;
    ptime m_visualEffectsStartTime;
    ptime m_repaintStartTime;
    ptime m_gmodelBuildStartTime;

    void create_graphic_model();
    void delete_graphic_model();
    void request_window_update();
    VRect get_damaged_rectangle();
    GmoObj* find_object_at(Pixels x, Pixels y);
    GmoBox* find_box_at(Pixels x, Pixels y);
    void send_mouse_out_event(GmoRef gref, Pixels x, Pixels y);
    void send_mouse_in_event(GmoRef gref, Pixels x, Pixels y);
    void send_click_event(GmoObj* pGmo, Pixels x, Pixels y, unsigned flags);
    void notify_event(SpEventInfo pEvent, GmoObj* pGmo);
    void update_view_if_gmodel_modified();
    void update_view_if_needed();
    void find_parent_link_box_and_notify_event(SpEventInfo pEvent, GmoObj* pGmo);
    void do_force_redraw();
    ImoObj* find_event_originator_imo(GmoObj* pGmo);
    GmoRef find_event_originator_gref(GmoObj* pGmo);
    bool discard_score_highlight_event_if_not_valid(SpEventScoreHighlight pEvent);
    bool is_valid_play_score_event(SpEventPlayScore pEvent);
    void update_caret_and_view();
    void redraw_caret();
    void send_update_UI_event(EEventType type);
    ptime get_current_time() const;
    double get_ellapsed_time_since(ptime startTime) const;
    Handler* handlers_hit_test(Pixels x, Pixels y);
    void restore_selection();
    bool is_operating_mode_allowed(int mode);

};

typedef SharedPtr<Interactor>   SpInteractor;
typedef WeakPtr<Interactor>     WpInteractor;

////---------------------------------------------------------------------------------------
////A view to edit the document in full page
//class EditInteractor : public Interactor
//{
//protected:
//
//public:
//
//    EditInteractor(LibraryScope& libraryScope, Document* pDoc, View* pView,
//                   DocCommandExecuter* pExec);
//    virtual ~EditInteractor();
//
//};



}   //namespace lomse

#endif      //__LOMSE_INTERACTOR_H__
