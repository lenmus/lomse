//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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
#include "lomse_interactor.h"

#include "lomse_ldp_compiler.h"
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_basic.h"
#include "lomse_tasks.h"
#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_note.h"
#include "lomse_document_layouter.h"
#include "lomse_view.h"
#include "lomse_graphic_view.h"
#include "lomse_events.h"
#include "lomse_player_gui.h"
#include "lomse_document_cursor.h"
#include "lomse_command.h"
#include "lomse_logger.h"
#include "lomse_handler.h"
#include "lomse_visual_effect.h"
#include "lomse_fragment_mark.h"
#include "lomse_score_utilities.h"
#include "lomse_shape_staff.h"
#include "lomse_score_algorithms.h"

#include <sstream>
#include <chrono>
using namespace std;

namespace lomse
{

ptime::duration ptime::operator-(const ptime rhs)
{
    long long millis = chrono::duration_cast<chrono::milliseconds>(timepoint - rhs.timepoint).count();
    return ptime::duration(millis);
}

//=======================================================================================
// Interactor implementation
//=======================================================================================
Interactor::Interactor(LibraryScope& libraryScope, WpDocument wpDoc, View* pView,
                       DocCommandExecuter* pExec)
    : EventHandler()
    , EventNotifier(libraryScope.get_events_dispatcher())
    , Observable()
    , m_libScope(libraryScope)
    , m_wpDoc(wpDoc)
    , m_pView(pView)
    , m_pGraphicModel(nullptr)
    , m_pTask(nullptr)
    , m_pCursor(nullptr)
    , m_pSelections(nullptr)
    , m_pExec(pExec)
    , m_grefLastMouseOver(k_no_gmo_ref)
    , m_operatingMode(k_mode_read_only)
    , m_fEditionEnabled(false)
    , m_fViewParamsChanged(false)
    , m_fViewUpdatesEnabled(true)
    , m_idControlledImo(k_no_imoid)
{
    switch_task(TaskFactory::k_task_only_clicks);

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        LOMSE_LOG_DEBUG(Logger::k_mvc, "Creating Interactor. Document is valid");
        Document* pDoc = spDoc.get();
        m_pCursor = Injector::inject_DocCursor(pDoc);
        m_pSelections = Injector::inject_SelectionSet(pDoc);

        GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
        if (pGView)
        {
            pGView->use_cursor(m_pCursor);
            pGView->use_selection_set(m_pSelections);
        }
    }
    LOMSE_LOG_DEBUG(Logger::k_mvc, "Interactor created.");
    highlight_voice(2);
}

//---------------------------------------------------------------------------------------
Interactor::~Interactor()
{
    delete_graphic_model();
    delete m_pTask;
    delete m_pView;
    delete m_pCursor;
    delete m_pSelections;
    LOMSE_LOG_DEBUG(Logger::k_mvc, "Interactor is deleted");
}

//---------------------------------------------------------------------------------------
void Interactor::switch_task(int taskType)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, "new task type=%d", taskType);

    delete m_pTask;
    m_pTask = Injector::inject_Task(taskType, this);
    m_pTask->init_task();
}

//---------------------------------------------------------------------------------------
void Interactor::define_beat(int beatType, TimeUnits duration)
{
    if (SpDocument spDoc = m_wpDoc.lock())
        spDoc->define_beat(beatType, duration);
}

//---------------------------------------------------------------------------------------
GraphicModel* Interactor::get_graphic_model()
{
    if (!m_pGraphicModel)
        create_graphic_model();
    return m_pGraphicModel;
}

//---------------------------------------------------------------------------------------
void Interactor::create_graphic_model()
{
    //This method creates the @GM. The process is just to create a document layouter
    //and to ask it to layout the document.
    //As the @GM must suit the View needs, the document layouter must be informed
    //of the requirements. The Interactor is the owner of the View.


    LOMSE_LOG_DEBUG(Logger::k_render, string(""));

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        m_gmodelBuildStartTime.init_now();

        GraphicView* pView = dynamic_cast<GraphicView*>(m_pView);
        Document* pDoc = spDoc.get();
        if (pView && pDoc)
        {
            LOMSE_LOG_DEBUG(Logger::k_render, "[Interactor::create_graphic_model]");
            int constrains = pView->get_layout_constrains();
            DocLayouter layouter(pDoc, m_libScope, constrains);

            if (pView->is_valid_for_this_view(pDoc))
                layouter.layout_document();
            else
                layouter.layout_empty_document();

            m_pGraphicModel = layouter.get_graphic_model();
            m_pGraphicModel->build_main_boxes_table();
            m_pSelections->graphic_model_changed(m_pGraphicModel);
        }
        spDoc->clear_dirty();

        timing_graphic_model_build_end();

        double buildTime = get_elapsed_time_since(m_gmodelBuildStartTime);
        LOMSE_LOG_INFO("gmodel build time = %d ms.", (int)buildTime);
    }
//    m_idLastMouseOver = k_no_imoid;
}

//---------------------------------------------------------------------------------------
void Interactor::on_document_updated()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, "[Interactor::on_document_updated]");
    delete_graphic_model();
    create_graphic_model();
    //TODO: Interactor::on_document_updated. Update cursor
    //DocCursor cursor(m_pDoc);
    //m_cursor = cursor;
}

//---------------------------------------------------------------------------------------
void Interactor::handle_event(SpEventInfo pEvent)
{
    switch(pEvent->get_event_type())
    {
        case k_doc_modified_event:
            delete_graphic_model();
            restore_selection();
            force_redraw();
            break;

        case k_tracking_event:
        {
            //AWARE: It sould never arrive here as on_visual_tracking() must
            //be invoked directly by user application to save time
            LOMSE_LOG_DEBUG(Logger::k_events, "Interactor::handle_even] Tracking event received");
            SpEventVisualTracking pEv(
                static_pointer_cast<EventVisualTracking>(pEvent) );
            on_visual_tracking(pEv);
            break;
        }

        case k_end_of_playback_event:
        {
            //AWARE: It could never arrive here as on_end_of_play_event() could
            //be invoked directly by user application
            LOMSE_LOG_DEBUG(Logger::k_events, "Interactor::handle_even] End of playback event received");
            SpEventPlayCtrl pEv( static_pointer_cast<EventPlayCtrl>(pEvent) );
            if (is_valid_play_score_event(pEv))
                on_end_of_play_event(pEv->get_score(), pEv->get_player());
            break;
        }

        default:
            break;
    }
}

//---------------------------------------------------------------------------------------
void Interactor::delete_graphic_model()
{
    delete m_pGraphicModel;
    m_pGraphicModel = nullptr;
    m_pSelections->graphic_model_changed(nullptr);

    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->remove_all_visual_tracking();

//    m_idLastMouseOver = k_no_imoid;
    set_drag_image(nullptr, k_do_not_get_ownership, UPoint(0.0, 0.0));
    LOMSE_LOG_DEBUG(Logger::k_render, "GModel deleted.");
}

////---------------------------------------------------------------------------------------
//void Interactor::insert_rest(DocCursor& cursor, const std::string& source)
//{
//    LdpElement* pElm = m_pCompiler->create_element(source);
//    CmdInsertElement cmd(TRT("Insert rest"), cursor, pElm, m_pCompiler);
//    m_pExec->execute(cmd);
//
//    //place cursor after inserted object
//    cursor.reset_and_point_to( pElm->get_id() );
//    cursor.move_next();
//
//    m_pDoc->notify_if_document_modified();
//}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_button_down(Pixels x, Pixels y, unsigned flags)
{
    LOMSE_LOG_DEBUG(Logger::k_events, string(""));
    m_pTask->process_event( Event((flags & k_mouse_left ? Event::k_mouse_left_down
                                                        : Event::k_mouse_right_down),
                                  x, y, flags) );
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_move(Pixels x, Pixels y, unsigned flags)
{
    LOMSE_LOG_TRACE(Logger::k_events, string(""));
    m_pTask->process_event( Event(Event::k_mouse_move, x, y, flags) );
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_enter_window(Pixels UNUSED(x), Pixels UNUSED(y),
                                       unsigned UNUSED(flags))
{
    enable_drag_image(true);
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_leave_window(Pixels UNUSED(x), Pixels UNUSED(y),
                                       unsigned UNUSED(flags))
{
    enable_drag_image(false);
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_button_up(Pixels x, Pixels y, unsigned flags)
{
    m_pTask->process_event( Event((flags & k_mouse_left ? Event::k_mouse_left_up
                                                        : Event::k_mouse_right_up),
                                  x, y, flags) );
}

//---------------------------------------------------------------------------------------
void Interactor::select_object(GmoObj* pGmo, bool fClearSelection)
{
    if (fClearSelection)
        m_pSelections->clear();
    m_pSelections->add(pGmo);
    send_update_UI_event(k_selection_set_change);
}

//---------------------------------------------------------------------------------------
void Interactor::select_object(ImoId id, bool fClearSelection)
{
    if (fClearSelection)
        m_pSelections->clear();

    m_pSelections->add(id, get_graphic_model());
    send_update_UI_event(k_selection_set_change);
}

//---------------------------------------------------------------------------------------
bool Interactor::is_in_selection(GmoObj* pGmo)
{
    return m_pSelections->contains( pGmo->get_creator_imo() );
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_select_object_and_show_contextual_menu(
                                                    Pixels x, Pixels y, unsigned flags)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        GmoObj* pGmo = find_object_at(x, y);
        if (pGmo)
        {
            //TODO: Review this code. flag for select_object() is just fClearSelection,
            //      not event flags. It is necessary to decide if selection must be
            //      clared based on event flags:
            //        k_kbd_shift   = 8,      ///< 0x08. Keyboard Shift key pressed while mouse event
            //        k_kbd_ctrl    = 16,     ///< 0x10. Keyboard Ctrol key pressed while mouse event
            //        k_kbd_alt     = 32,     ///< 0x20. Keyboard Alt key pressed while mouse event
            select_object(pGmo, false); //flags);
            force_redraw();     //to draw it as selected and remove previous selection
            ImoObj* pImo = pGmo->get_creator_imo();
            if (pImo)
            {
                ImoId id = pImo->get_id();
                SpInteractor sp = get_shared_ptr_from_this();
                WpInteractor wp(sp);
                SpEventInfo pEvent(
                    LOMSE_NEW EventMouse(k_show_contextual_menu_event,
                                                  wp, id, x, y, flags, m_wpDoc) );
                m_libScope.post_event(pEvent);  //--> to global handler
                //notify_event(pEvent, pGmo);     //--> to Document observers
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_click_at_screen_point(Pixels x, Pixels y, unsigned flags)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    //mouse left click when in selection mode

//    m_pSelections->clear();
    GmoObj* pGmo = find_object_at(x, y);

//    stringstream msg;
//    msg << "Click: Gmo=" << (pGmo ? pGmo->get_name() : "nullptr") << ", point("
//        << x << ", " << y << ")";
//    LOMSE_LOG_INFO(msg.str());

    if (pGmo)
        send_click_event(pGmo, x, y, flags);
}

//---------------------------------------------------------------------------------------
void Interactor::send_click_event(GmoObj* pGmo, Pixels x, Pixels y, unsigned flags)

{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (pGmo)
        {
            ImoObj* pImo = find_event_originator_imo(pGmo);
            ImoId id = pImo ? pImo->get_id() : k_no_imoid;
            SpInteractor sp = get_shared_ptr_from_this();
            WpInteractor wp(sp);
            SpEventMouse pEvent(
                LOMSE_NEW EventMouse(k_on_click_event, wp, id, x, y, flags, m_wpDoc) );
            notify_event(pEvent, pGmo);
        }
    }
}

//---------------------------------------------------------------------------------------
DocCursorState Interactor::click_event_to_cursor_state(SpEventMouse event)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView == nullptr)
    {
        string msg = "Invoking Interactor::click_event_to_cursor_state() but no graphic view!";
        LOMSE_LOG_ERROR(msg);
        throw runtime_error(msg);
    }

    ImoObj* pImo = event->get_imo_object();
    double x = double(event->get_x());
    double y = double(event->get_y());
    int iPage = pGView->page_at_screen_point(x, y);
    GmoObj* pGmo = find_object_at(Pixels(x), Pixels(y));
    screen_point_to_page_point(&x, &y);
    return pGView->click_event_to_cursor_state(iPage, LUnits(x), LUnits(y), pImo, pGmo);
}

//---------------------------------------------------------------------------------------
DiatonicPitch Interactor::get_pitch_at(Pixels x, Pixels y)
{
    //What would be the pitch if a note is inserted at received point?

    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView == nullptr)
    {
        string msg = "Invoking Interactor::get_pitch_at() but no graphic view!";
        LOMSE_LOG_ERROR(msg);
        throw runtime_error(msg);
    }

    double xPos = double(x);
    double yPos = double(y);
    int iPage = page_at_screen_point(xPos, yPos);
    if (iPage == -1)
        return DiatonicPitch(k_no_pitch);

    screen_point_to_page_point(&xPos, &yPos);
    GraphicModel* pGM = get_graphic_model();
    AreaInfo* pInfo = pGM->get_info_for_point(iPage, LUnits(xPos), LUnits(yPos));
    if (pInfo->pShapeStaff)
    {
        //determine position on staff
        int lineSpace = pInfo->pShapeStaff->line_space_at(LUnits(yPos));     //0=first ledger line below staff

        //determine instrument, staff and timepos
        GmoObj* pGmo = pInfo->pGmo;
        ImoObj* pImo = pGmo->get_creator_imo();
        DocCursorState state = pGView->click_event_to_cursor_state(iPage, LUnits(xPos),
                                                                LUnits(yPos), pImo, pGmo);
        if (state.get_parent_level_id() != k_no_imoid)
        {
            SpScoreCursorState pState(
                static_pointer_cast<ScoreCursorState>(state.get_delegate_state()) );
            int staff = pState->staff();
            int instr = pState->instrument();
            TimeUnits time = pState->time();

            //determine clef
            ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                        pInfo->pShapeStaff->get_creator_imo() );
            ImoScore* pScore = pInstr->get_score();
            EClef clef = EClef(
                ScoreAlgorithms::get_applicable_clef_for(pScore, instr, staff, time) );
            if (clef == k_clef_undefined || clef == k_clef_percussion)
                return DiatonicPitch(k_no_pitch);

            //determine pitch
            DiatonicPitch dp = get_diatonic_pitch_for_first_line(clef);
            dp += (lineSpace - 2);

            return dp;
        }
    }
    return DiatonicPitch(k_no_pitch);
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_mouse_in_out(Pixels x, Pixels y,
                                          unsigned UNUSED(flags))
{
    GmoObj* pGmo = find_object_at(x, y);
    if (pGmo == nullptr)
        return;

    GmoRef gref = find_event_originator_gref(pGmo);

//    LOMSE_LOG_DEBUG(Logger::k_events,
//        "Gmo %d, %s / gref(%d, %d) -------------------",
//         pGmo->get_gmobj_type(), pGmo->get_name().c_str(),
//         gref.first, gref.second );

    if (m_grefLastMouseOver != k_no_gmo_ref && m_grefLastMouseOver != gref)
    {
//        LOMSE_LOG_DEBUG(Logger::k_events,
//            "Mouse out. gref(%d %d)",
//            m_grefLastMouseOver.first, m_grefLastMouseOver.second );
        send_mouse_out_event(m_grefLastMouseOver, x, y);
        m_grefLastMouseOver = k_no_gmo_ref;
    }

    if (m_grefLastMouseOver == k_no_gmo_ref && gref != k_no_gmo_ref)
    {
//        LOMSE_LOG_DEBUG(Logger::k_events,
//            "Mouse in. gref(%d %d)",
//            gref.first, gref.second );
        send_mouse_in_event(gref, x, y);
        m_grefLastMouseOver = gref;
    }

}

////---------------------------------------------------------------------------------------
//void Interactor::task_action_single_click_at(Pixels x, Pixels y, bool fLeftButton)
//{
//    //TODO
//    task_action_click_at_screen_point(x, y);
//}
//
////---------------------------------------------------------------------------------------
//void Interactor::task_action_double_click_at(Pixels x, Pixels y, bool fLeftButton)
//{
//    //TODO
//}
//
////---------------------------------------------------------------------------------------
//void Interactor::task_action_start_move_drag(Pixels x, Pixels y, bool fLeftButton)
//{
//    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));
//
//    // Edition mode: mouse click down at point (x, y). Decide what to do.
//
//    if (!fLeftButton)
//        return;
//
////    m_pCurHandler = handlers_hit_test(x, y);
////    if (m_pCurHandler)
////    {
////        //click on handler: move it
////        switch_task(TaskFactory::k_task_move_handler);
////        static_cast<TaskMoveHandler*>(m_pTask)->set_first_point(x, y);
////        return;
////    }
////
////    GmoObj* pGmo = find_object_at(x, y);
////    ImoObj* pImo = (pGmo != nullptr ? pGmo->get_creator_imo() : nullptr);
////    if (pImo && pImo->is_staffobj())
////    {
////        //click on staffobj: drag it
////        GmoShape* pShape = nullptr;
////        if (pImo->is_note())
////        {
////            GmoShapeNote* pNote = static_cast<GmoShapeNote*>(pGmo);
////            pShape = pNote->get_notehead_shape();
////        }
////        else
////            pShape = static_cast<GmoShape*>(pGmo);
////
////        //compute offset so that hotspot is at clicked point
////        double xPos = double(x);
////        double yPos = double(y);
////        screen_point_to_page_point(&xPos, &yPos);
////        UPoint org = pShape->get_origin();
////        UPoint offset(LUnits(xPos)-org.x, LUnits(yPos)-org.y);
////
////        set_drag_image(pShape, k_do_not_get_ownership, offset);
////        show_drag_image(true);
////
////        m_pSelections->clear();
////        switch_task(TaskFactory::k_task_move_object);
////        static_cast<TaskMoveObject*>(m_pTask)->set_first_point(x, y);
////    }
////    else
//    {
//        //click at other areas: start a selection rectangle
//        m_pSelections->clear();
//        start_selection_rectangle(x, y);
//
////        switch_task(TaskFactory::k_task_selection_rectangle);
////        static_cast<TaskSelectionRectangle*>(m_pTask)->set_first_point(x, y);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void Interactor::task_action_continue_move_drag(Pixels x, Pixels y, bool fLeftButton)
//{
//    //TODO
//    task_action_update_selection_rectangle(x, y);
//}
//
////---------------------------------------------------------------------------------------
//void Interactor::task_action_end_move_drag(Pixels x, Pixels y, bool fLeftButton,
//                                           Pixels xTotalShift, Pixels yTotalShift)
//{
//    //TODO
//    task_action_select_objects_in_screen_rectangle(x, y, x+xTotalShift, y+yTotalShift);
//}

//---------------------------------------------------------------------------------------
void Interactor::task_action_move_drag_image(Pixels x, Pixels y)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        UPoint pos = screen_point_to_model_point(x, y);
        pGView->move_drag_image(pos.x, pos.y);
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_insert_object_at_point(Pixels x, Pixels y, unsigned flags)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    //invoked only from TaskDataEntry: mouse left click when in data entry mode

    //TODO: decide if special treatment. For now, same than selection mode
    task_action_click_at_screen_point(x, y, flags);
    force_redraw();
}

//---------------------------------------------------------------------------------------
ImoObj* Interactor::find_event_originator_imo(GmoObj* pGmo)
{
    ImoContentObj* pParent = dynamic_cast<ImoContentObj*>( pGmo->get_creator_imo() );
    while (pParent && !pParent->is_link())
        pParent = dynamic_cast<ImoContentObj*>( pParent->get_parent() );

    if (pParent && pParent->is_link())
        return pParent;
    else
        return pGmo->get_creator_imo();
}

//---------------------------------------------------------------------------------------
GmoRef Interactor::find_event_originator_gref(GmoObj* pGmo)
{
    if (pGmo->is_box_control())
        return pGmo->get_ref();

    else if (pGmo->is_shape_word())
    {
        ImoObj* pImo = find_event_originator_imo(pGmo);
        if (pImo && pImo->is_mouse_over_generator())
            return make_pair(pImo->get_id(), 0);
    }

    return k_no_gmo_ref;
}

////---------------------------------------------------------------------------------------
//Observable* Interactor::find_event_source(GmoObj* pGmo)
//{
//    ImoContentObj* pParent = dynamic_cast<ImoContentObj*>( pGmo->get_creator_imo() );
//
//    //try link
//    while (pParent && !pParent->is_link())
//        pParent = dynamic_cast<ImoContentObj*>( pParent->get_parent() );
//
//    if (pParent && pParent->is_link())
//        return pParent;
//
//    //try other Imo
//    pParent = dynamic_cast<ImoContentObj*>( pGmo->get_creator_imo() );
//    if (pParent)
//        return pParent;
//
//    //try Control
//    if (pGmo->is_box_control())
//    {
//        GmoBoxControl* pGBC = static_cast<GmoBoxControl*>( pGmo );
//        return pGBC->get_creator_control();
//    }
//
//    //?????????? throw
//    return nullptr;
//}

//---------------------------------------------------------------------------------------
void Interactor::update_view_if_gmodel_modified()
{
    GraphicModel* pGM = get_graphic_model();
    if (pGM->is_modified())
    {
        force_redraw();
        pGM->set_modified(false);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::redraw_caret()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->draw_caret();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
bool Interactor::view_needs_repaint()
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (spDoc->is_dirty() || m_fViewParamsChanged)
            return true;
        else
        {
            GraphicModel* pGM = get_graphic_model();
            return pGM->is_modified();
        }
    }
    else
        return true;
}

//---------------------------------------------------------------------------------------
GmoObj* Interactor::find_object_at(Pixels x, Pixels y)
{
    double xPos = double(x);
    double yPos = double(y);
    int iPage = page_at_screen_point(xPos, yPos);
    if (iPage == -1)
        return nullptr;

    screen_point_to_page_point(&xPos, &yPos);
    GraphicModel* pGM = get_graphic_model();
    return pGM->hit_test(iPage, LUnits(xPos), LUnits(yPos));
}

//---------------------------------------------------------------------------------------
GmoBox* Interactor::find_box_at(Pixels x, Pixels y)
{
    double xPos = double(x);
    double yPos = double(y);
    int iPage = page_at_screen_point(xPos, yPos);
    if (iPage == -1)
        return nullptr;

    screen_point_to_page_point(&xPos, &yPos);
    GraphicModel* pGM = get_graphic_model();
    return pGM->find_inner_box_at(iPage, LUnits(xPos), LUnits(yPos));
}

//---------------------------------------------------------------------------------------
Handler* Interactor::handlers_hit_test(Pixels x, Pixels y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        UPoint pos = screen_point_to_model_point(x, y);
        return pGView->handlers_hit_test(pos.x, pos.y);
    }
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_select_objects_in_screen_rectangle(Pixels x1, Pixels y1,
                                                                Pixels x2, Pixels y2,
                                                                unsigned flags)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    //invoked only from TaskSelection

    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (!pGView)
        return;

    list<PageRectangle*> rectangles;
    pGView->screen_rectangle_to_page_rectangles(x1, y1, x2, y2, &rectangles);
    if (rectangles.size() == 0)
        return;

    GraphicModel* pGM = get_graphic_model();
    list<PageRectangle*>::iterator it;
    m_pSelections->clear();
    for (it = rectangles.begin(); it != rectangles.end(); ++it)
    {
        pGM->select_objects_in_rectangle((*it)->iPage, m_pSelections, (*it)->rect, flags);
        delete *it;
    }

    pGView->draw_selected_objects();
    request_window_update();

    send_update_UI_event(k_selection_set_change);
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_decide_on_switching_task(Pixels x, Pixels y,
                                                      unsigned UNUSED(flags))
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    // Edition mode: mouse left click at point (x, y). Decide what to do.

    m_pCurHandler = handlers_hit_test(x, y);
    if (m_pCurHandler)
    {
        //click on handler: move it
        switch_task(TaskFactory::k_task_move_handler);
        static_cast<TaskMoveHandler*>(m_pTask)->set_first_point(x, y);
        return;
    }

    GmoObj* pGmo = find_object_at(x, y);
    ImoObj* pImo = (pGmo != nullptr ? pGmo->get_creator_imo() : nullptr);
    if (pImo && pImo->is_staffobj())
    {
        //click on staffobj: drag it
        GmoShape* pShape = nullptr;
        if (pImo->is_note())
        {
            GmoShapeNote* pNote = static_cast<GmoShapeNote*>(pGmo);
            pShape = pNote->get_notehead_shape();
        }
        else
            pShape = static_cast<GmoShape*>(pGmo);

        //compute offset so that hotspot is at clicked point
        double xPos = double(x);
        double yPos = double(y);
        screen_point_to_page_point(&xPos, &yPos);
        UPoint org = pShape->get_origin();
        UPoint offset(LUnits(xPos)-org.x, LUnits(yPos)-org.y);

        set_drag_image(pShape, k_do_not_get_ownership, offset);
        show_drag_image(true);

//        m_pSelections->clear();
        switch_task(TaskFactory::k_task_move_object);
        static_cast<TaskMoveObject*>(m_pTask)->set_first_point(x, y);
    }
    else
    {
        //click at other areas: start a selection rectangle
        m_pSelections->clear();
        start_selection_rectangle(x, y);

        switch_task(TaskFactory::k_task_selection_rectangle);
        static_cast<TaskSelectionRectangle*>(m_pTask)->set_first_point(x, y);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_switch_to_default_task()
{
    switch_task(TaskFactory::k_task_selection);
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_move_object(Pixels UNUSED(x), Pixels UNUSED(y))
{
    //TODO
    show_drag_image(false);
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_move_handler(Pixels x, Pixels y)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    UPoint pos = screen_point_to_model_point(x, y);
    m_pCurHandler->move_to(pos);

    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->draw_handler(m_pCurHandler);
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_move_handler_end_point(Pixels xFinal, Pixels yFinal,
                                                    Pixels xTotalShift, Pixels yTotalShift)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    task_action_move_handler(xFinal, yFinal);

    //generate event for updating the internal model
    GmoObj* pGmo = m_pCurHandler->get_controlled_gmo();
    int iHandler = m_pCurHandler->get_handler_index();
    //UPoint shift = screen_point_to_model_point(xTotalShift, yTotalShift);
    GraphicView* pGView = static_cast<GraphicView*>(m_pView);
    UPoint shift(pGView->pixels_to_lunits(xTotalShift),
                 pGView->pixels_to_lunits(yTotalShift) );

    //save reference for re-selecting the object
    m_idControlledImo = m_pCurHandler->get_controlled_gmo()->get_creator_imo()->get_id();
    m_pCurHandler = nullptr;

    SpInteractor sp = get_shared_ptr_from_this();
    WpInteractor wpIntor(sp);
    SpEventControlPointMoved pEvent(
        LOMSE_NEW EventControlPointMoved(k_control_point_moved_event, wpIntor,
                                         pGmo, iHandler, shift, m_wpDoc) );
    notify_observers(pEvent, this);

//    //when redrawing the graphic model the selection set is cleared.
//    //So tie must be re-selected here
//    GraphicModel* pGModel = get_graphic_model();
//    pGmo = pGModel->get_main_shape_for_imo(id);
//    select_object(pGmo);

}

//---------------------------------------------------------------------------------------
void Interactor::task_action_drag_the_view(Pixels x, Pixels y)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    new_viewport(x, y);
}

//---------------------------------------------------------------------------------------
void Interactor::restore_selection()
{
    //when redrawing the graphic model the selection set is cleared.
    //This method restores the selection if only one Imo with handlers was selected

    if (m_idControlledImo != k_no_imoid)
    {
        GraphicModel* pGModel = get_graphic_model();
        GmoObj* pGmo = pGModel->get_main_shape_for_imo(m_idControlledImo);
        select_object(pGmo);
        m_idControlledImo = k_no_imoid;
    }
}

//---------------------------------------------------------------------------------------
void Interactor::redraw_bitmap()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (spDoc->is_dirty())
            delete_graphic_model();

        GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
        if (pGView)
            pGView->redraw_bitmap();
    }
    m_fViewParamsChanged = false;
}

//---------------------------------------------------------------------------------------
void Interactor::request_window_update()
{
    LOMSE_LOG_DEBUG(Logger::k_events | Logger::k_mvc, string(""));

    SpInteractor sp = get_shared_ptr_from_this();
    WpInteractor wpIntor(sp);
    SpEventPaint pEvent( LOMSE_NEW EventPaint(wpIntor, get_damaged_rectangle()) );
    notify_observers(pEvent, this);
}

//---------------------------------------------------------------------------------------
VRect Interactor::get_damaged_rectangle()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->get_damaged_rectangle();
    else
        return VRect(0, 0, 0, 0);
}
//---------------------------------------------------------------------------------------
void Interactor::force_redraw()
{
    if (m_fViewUpdatesEnabled)
        do_force_redraw();
}

//---------------------------------------------------------------------------------------
void Interactor::do_force_redraw()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    redraw_bitmap();
    request_window_update();
}

//---------------------------------------------------------------------------------------
void Interactor::new_viewport(Pixels x, Pixels y, bool fForceRedraw)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->new_viewport(x, y);
        if (fForceRedraw)
            force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::get_view_size(Pixels* xWidth, Pixels* yHeight)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->get_view_size(xWidth, yHeight);
}

//---------------------------------------------------------------------------------------
void Interactor::set_viewport_at_page_center(Pixels screenWidth)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_viewport_at_page_center(screenWidth);
}

//---------------------------------------------------------------------------------------
void Interactor::set_rendering_buffer(RenderingBuffer* rbuf)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_rendering_buffer(rbuf);
}

//---------------------------------------------------------------------------------------
void Interactor::get_viewport(Pixels* x, Pixels* y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->get_viewport(x, y);
}

//---------------------------------------------------------------------------------------
void Interactor::request_viewport_change(Pixels x, Pixels y)
{
    //AWARE: This code is executed in the sound thread

    LOMSE_LOG_DEBUG(lomse::Logger::k_events | lomse::Logger::k_score_player, string(""));

    SpInteractor sp = get_shared_ptr_from_this();
    WpInteractor wpIntor(sp);
    SpEventInfo pEvent( LOMSE_NEW EventUpdateViewport(wpIntor, x, y) );  //m_wpDoc

    m_libScope.post_event(pEvent);
}

//---------------------------------------------------------------------------------------
void Interactor::start_selection_rectangle(Pixels x1, Pixels y1)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        UPoint start = screen_point_to_model_point(x1, y1);
        pGView->start_selection_rectangle(start.x, start.y);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::hide_selection_rectangle()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->hide_selection_rectangle();
}

//---------------------------------------------------------------------------------------
void Interactor::task_action_update_selection_rectangle(Pixels x2, Pixels y2)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        UPoint end = screen_point_to_model_point(x2, y2);
        pGView->update_selection_rectangle(end.x, end.y);
        pGView->draw_selection_rectangle();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::set_visual_tracking_mode(int mode)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_visual_tracking_mode(mode);
}

//---------------------------------------------------------------------------------------
VisualEffect* Interactor::get_tracking_effect(int effect)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->get_tracking_effect(effect);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void Interactor::remove_all_visual_tracking()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->remove_all_visual_tracking();
        pGView->draw_visual_tracking();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::move_tempo_line(ImoId scoreId, TimeUnits timepos)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->move_tempo_line_and_change_viewport(scoreId, timepos);
        pGView->draw_visual_tracking();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::scroll_to_measure_if_necessary(ImoId scoreId, int iMeasure,
                                                TimeUnits location, int iInstr)
{
    MeasureLocator ml(iInstr, iMeasure, location);
    do_move_tempo_line_and_scroll(scoreId, ml, k_only_scroll);
}

//---------------------------------------------------------------------------------------
void Interactor::move_tempo_line(ImoId scoreId, int iMeasure, TimeUnits location,
                                 int iInstr)
{
    MeasureLocator ml(iInstr, iMeasure, location);
    do_move_tempo_line_and_scroll(scoreId, ml, k_only_tempo_line);
}

//---------------------------------------------------------------------------------------
void Interactor::move_tempo_line_and_scroll_if_necessary(ImoId scoreId, int iMeasure,
                                                         TimeUnits location, int iInstr)
{
    MeasureLocator ml(iInstr, iMeasure, location);
    do_move_tempo_line_and_scroll(scoreId, ml, k_scroll_and_tempo_line);
}

//---------------------------------------------------------------------------------------
void Interactor::do_move_tempo_line_and_scroll(ImoId scoreId, const MeasureLocator& ml,
                                               int mode)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        ImoObj* pScore = nullptr;
        if (SpDocument spDoc = m_wpDoc.lock())
            pScore = spDoc->get_pointer_to_imo(scoreId);

        if (pScore && pScore->is_score())
        {
            ImoScore* score = static_cast<ImoScore*>(pScore);
            TimeUnits timepos = ScoreAlgorithms::get_timepos_for(score, ml);
            if (mode == k_only_tempo_line)
                pGView->move_tempo_line(scoreId, timepos);
            else if (mode == k_only_scroll)
                pGView->change_viewport_if_necessary(scoreId, timepos);
            else
                pGView->move_tempo_line_and_change_viewport(scoreId, timepos);

            pGView->draw_visual_tracking();
            request_window_update();
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::move_tempo_line(ImoId scoreId, int iMeasure, int iBeat, int iInstr)
{
    do_move_tempo_line_and_scroll(scoreId, iMeasure, iBeat, iInstr, k_only_tempo_line);
}

//---------------------------------------------------------------------------------------
void Interactor::move_tempo_line_and_scroll_if_necessary(ImoId scoreId, int iMeasure,
                                                         int iBeat, int iInstr)
{
    do_move_tempo_line_and_scroll(scoreId, iMeasure, iBeat, iInstr, k_scroll_and_tempo_line);
}

//---------------------------------------------------------------------------------------
void Interactor::scroll_to_measure_if_necessary(ImoId scoreId, int iMeasure, int iBeat, int iInstr)
{
    do_move_tempo_line_and_scroll(scoreId, iMeasure, iBeat, iInstr, k_only_scroll);
}

//---------------------------------------------------------------------------------------
void Interactor::do_move_tempo_line_and_scroll(ImoId scoreId, int iMeasure, int iBeat,
                                               int iInstr, int mode)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        ImoObj* pScore = nullptr;
        if (SpDocument spDoc = m_wpDoc.lock())
            pScore = spDoc->get_pointer_to_imo(scoreId);

        if (pScore && pScore->is_score())
        {
            ImoScore* score = static_cast<ImoScore*>(pScore);
            TimeUnits timepos = ScoreAlgorithms::get_timepos_for(score, iMeasure,
                                                                 iBeat, iInstr);
            if (mode == k_only_tempo_line)
                pGView->move_tempo_line(scoreId, timepos);
            else if (mode == k_only_scroll)
                pGView->change_viewport_if_necessary(scoreId, timepos);
            else
                pGView->move_tempo_line_and_change_viewport(scoreId, timepos);

            pGView->draw_visual_tracking();
            request_window_update();
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::scroll_to_measure(ImoId scoreId, int iMeasure, int iBeat, int iInstr)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        ImoObj* pScore = nullptr;
        if (SpDocument spDoc = m_wpDoc.lock())
            pScore = spDoc->get_pointer_to_imo(scoreId);

        if (pScore && pScore->is_score())
        {
            ImoScore* score = static_cast<ImoScore*>(pScore);
            TimeUnits timepos = ScoreAlgorithms::get_timepos_for(score, iMeasure,
                                                                 iBeat, iInstr);
            pGView->change_viewport_to(scoreId, timepos);
            pGView->draw_visual_tracking();
            request_window_update();
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::scroll_to_measure(ImoId scoreId, int iMeasure, TimeUnits location,
                                   int iInstr)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        ImoObj* pScore = nullptr;
        if (SpDocument spDoc = m_wpDoc.lock())
            pScore = spDoc->get_pointer_to_imo(scoreId);

        if (pScore && pScore->is_score())
        {
            ImoScore* score = static_cast<ImoScore*>(pScore);
            MeasureLocator ml(iInstr, iMeasure, location);
            TimeUnits timepos = ScoreAlgorithms::get_timepos_for(score, ml);
            pGView->change_viewport_to(scoreId, timepos);
            pGView->draw_visual_tracking();
            request_window_update();
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::highlight_object(ImoStaffObj* pSO)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->highlight_object(pSO);
        pGView->draw_visual_tracking();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::remove_highlight_from_object(ImoStaffObj* pSO)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->remove_highlight_from_object(pSO);
        pGView->draw_visual_tracking();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::change_viewport_if_necessary(ImoId id)
{
    LOMSE_LOG_DEBUG(lomse::Logger::k_events | lomse::Logger::k_score_player, string(""));
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->change_viewport_if_necessary(id);
}

//---------------------------------------------------------------------------------------
void Interactor::screen_point_to_page_point(double* x, double* y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->screen_point_to_page_point(x, y);
}

//---------------------------------------------------------------------------------------
void Interactor::model_point_to_screen(double* x, double* y, int iPage)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->model_point_to_screen(x, y, iPage);
}

//---------------------------------------------------------------------------------------
UPoint Interactor::screen_point_to_model_point(Pixels x, Pixels y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->screen_point_to_model_point(x, y);
    else
        return UPoint(0.0, 0.0);
}

//---------------------------------------------------------------------------------------
int Interactor::page_at_screen_point(double x, double y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->page_at_screen_point(x, y);
    return 0;
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_in(Pixels x, Pixels y, bool fForceRedraw)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->zoom_in(x, y);
        if (fForceRedraw)
            force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_out(Pixels x, Pixels y, bool fForceRedraw)
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, string(""));

    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->zoom_out(x, y);
        if (fForceRedraw)
            force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_fit_full(Pixels width, Pixels height, bool fForceRedraw)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->zoom_fit_full(width, height);
        if (fForceRedraw)
            force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_fit_width(Pixels width, bool fForceRedraw)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->zoom_fit_width(width);
        if (fForceRedraw)
            force_redraw();
    }
}

//---------------------------------------------------------------------------------------
double Interactor::get_scale()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->get_scale();
    else
        return 1.0;
}

//---------------------------------------------------------------------------------------
void Interactor::set_scale(double scale, Pixels x, Pixels y, bool fForceRedraw)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->set_scale(scale, x, y);
        if (fForceRedraw)
            force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::set_rendering_option(int option, bool value)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_rendering_option(option, value);
}

//---------------------------------------------------------------------------------------
void Interactor::set_view_background(Color color)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_background(color);
}

//---------------------------------------------------------------------------------------
void Interactor::set_box_to_draw(int boxType)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_box_to_draw(boxType);
}

//---------------------------------------------------------------------------------------
void Interactor::reset_boxes_to_draw()
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->reset_boxes_to_draw();
}

//---------------------------------------------------------------------------------------
void Interactor::highlight_voice(int voice)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->highlight_voice(voice);
}

//---------------------------------------------------------------------------------------
void Interactor::select_voice(int voice)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->change_cursor_voice(voice);
}

//---------------------------------------------------------------------------------------
void Interactor::set_print_buffer(RenderingBuffer* rbuf)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_print_buffer(rbuf);
}

//---------------------------------------------------------------------------------------
void Interactor::set_print_ppi(double ppi)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_print_ppi(ppi);
}

//---------------------------------------------------------------------------------------
void Interactor::print_page(int page, VPoint viewport)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->print_page(page, viewport);
}

//---------------------------------------------------------------------------------------
int Interactor::get_num_pages()
{
    GraphicModel* pGModel = get_graphic_model();
    if (pGModel)
        return pGModel->get_num_pages();
    else
        return 0;
}


////---------------------------------------------------------------------------------------
//void Interactor::on_resize(Pixels x, Pixels y)
//{
//    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
//    if (pGView)
//        pGView->on_resize(x, y);
//}

//---------------------------------------------------------------------------------------
bool Interactor::discard_visual_tracking_event_if_not_valid(ImoId scoreId)
{
    //returns true if event discarded

    ImoObj* pScore = nullptr;
    if (SpDocument spDoc = m_wpDoc.lock())
        pScore = spDoc->get_pointer_to_imo(scoreId);

    if (!pScore || !pScore->is_score())
    {
        LOMSE_LOG_DEBUG(Logger::k_events,
            "Visual tracking event discarded: score id: %d, pScore? %s",
            scoreId,
            (pScore ? "not null" : "null") );

        remove_all_visual_tracking();
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool Interactor::is_valid_play_score_event(SpEventPlayCtrl UNUSED(pEvent))
{
    LOMSE_LOG_ERROR("TODO: Method not implemented");
    //TODO
//    ImoObj* pScore = m_pDoc->get_pointer_to_imo( pEvent->get_score_id() );
//    if (!pScore || !pScore->is_score())
//    {
//        logger.log_message("[Interactor::is_valid_play_score_event] Score not valid. Event discarded", __FILE__, __LINE__);
//        return false;
//    }
    return true;
}

//---------------------------------------------------------------------------------------
void Interactor::on_visual_tracking(SpEventVisualTracking pEvent)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (!pGView)
        return;

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (discard_visual_tracking_event_if_not_valid( pEvent->get_score_id() ))
            return;

        LOMSE_LOG_DEBUG(Logger::k_events, "Processing visual tracking event");
        std::list< pair<int, ImoId> >& items = pEvent->get_items();
        std::list< pair<int, ImoId> >::iterator it;
        for (it = items.begin(); it != items.end(); ++it)
        {
            switch ((*it).first)
            {
                case EventVisualTracking::k_end_of_visual_tracking:
                    //LOMSE_LOG_DEBUG(Logger::k_events, "Processing k_end_of_visual_tracking");
                    remove_all_visual_tracking();
                    break;

                case EventVisualTracking::k_highlight_off:
                    //LOMSE_LOG_DEBUG(Logger::k_events, "Processing k_highlight_off");
                    remove_highlight_from_object( static_cast<ImoStaffObj*>(
                                                spDoc->get_pointer_to_imo((*it).second) ));
                    break;

                case EventVisualTracking::k_highlight_on:
                    //LOMSE_LOG_DEBUG(Logger::k_events, "Processing k_highlight_on");
                    highlight_object( static_cast<ImoStaffObj*>(
                                            spDoc->get_pointer_to_imo((*it).second) ));
                    break;

                case EventVisualTracking::k_move_tempo_line:
                    //LOMSE_LOG_DEBUG(Logger::k_events, "Processing k_move_tempo_line");
                    move_tempo_line(pEvent->get_score_id(), pEvent->get_timepos());
                    break;

                default:
                {
                    stringstream msg;
                    msg << "Unknown event type " <<
                           (*it).first << "." ;
                    LOMSE_LOG_ERROR(msg.str());
                    throw runtime_error(msg.str());
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::on_end_of_play_event(ImoScore* pScore, PlayerGui* pPlayCtrl)
{
    //AWARE: This method initially named "send_end_of_play_event" was designed to be
    //directly invoked from the SoundPlayer for generating the EndOfPlay event.
    //Unfortunately, it is necessary to decouple from the sound thread and currently this
    //is done by generating the EndOfPlay event in the sound thread, posting it to the
    //user application global handler, and requesting the user to invoke this method.
    //Therefore, the name of this method was changed to "on_end_of_play_event" to
    //avoid raising questions on users about the contradiction of having to invoke
    // "send_end_of_play_event" when an end of play event is received!!

    LOMSE_LOG_DEBUG(Logger::k_events, string(""));

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (pPlayCtrl)
            pPlayCtrl->on_end_of_playback();

        //AWARE: now generate the end of play event for observers (i.e. an play ctrl) and
        //for the user application. Due to current event handling path, it is non-sense
        //to send again a new end-of-play event to the user application; worse: this
        //could create an infinite processing loop. Therefore, user application must
        //never register at the Document an observer for this event.
        Document* pDoc = spDoc.get();
        SpInteractor sp = get_shared_ptr_from_this();
        WpInteractor wpIntor(sp);
        SpEventInfo pEvent( LOMSE_NEW EventEndOfPlayback(k_end_of_playback_event,
                                                     wpIntor, pScore, pPlayCtrl) );
        pDoc->notify_observers(pEvent, pDoc);

        update_view_if_needed();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::update_view_if_needed()
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (spDoc->is_dirty())
        {
            LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::update_view_if_needed] ask Document to update if dirty.");
            spDoc->notify_if_document_modified();
        }
        else
        {
            LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::update_view_if_needed] update view if GModel dirty");
            update_view_if_gmodel_modified();
        }
    }
}

//---------------------------------------------------------------------------------------
bool Interactor::is_document_editable()
{
    if (SpDocument spDoc = m_wpDoc.lock())
        return spDoc->is_editable();
    else
        return false;
}

//---------------------------------------------------------------------------------------
void Interactor::send_mouse_out_event(GmoRef gref, Pixels x, Pixels y)
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::send_mouse_out_event]");
        SpInteractor spIntor = get_shared_ptr_from_this();
        WpInteractor wpIntor(spIntor);
        ImoId id = gref.first;
        unsigned flags = 0;
        SpEventMouse pEvent( LOMSE_NEW EventMouse(k_mouse_out_event, wpIntor,
                                                  id, x, y, flags, m_wpDoc) );
        GraphicModel* pGM = get_graphic_model();
        GmoObj* pGmo = pGM->get_box_for_control(gref);
        if (pGmo)
            notify_event(pEvent, pGmo);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::send_mouse_in_event(GmoRef gref, Pixels x, Pixels y)
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::send_mouse_in_event]");
        SpInteractor sp = get_shared_ptr_from_this();
        WpInteractor wp(sp);
        ImoId id = gref.first;
        unsigned flags = 0;
        SpEventMouse pEvent( LOMSE_NEW EventMouse(k_mouse_in_event,
                                                  wp, id, x, y, flags, m_wpDoc) );
        GraphicModel* pGM = get_graphic_model();
        GmoObj* pGmo = pGM->get_box_for_control(gref);
        if (pGmo)
            notify_event(pEvent, pGmo);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::send_update_UI_event(EEventType type)
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::send_update_UI_event]");
        SpInteractor sp = get_shared_ptr_from_this();
        WpInteractor wp(sp);
        SpEventUpdateUI pEvent(
            LOMSE_NEW EventUpdateUI(type, wp, m_wpDoc, m_pSelections, m_pCursor) );

        //AWARE: update UI events are sent directly to the application global handler.
        //It is assumed that this event is of interest only for application main frame.
        //TODO: lomse global option at initialization?
        m_libScope.post_event(pEvent);
//        notify_observers(pEvent, this);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::notify_event(SpEventInfo pEvent, GmoObj* pGmo)
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (spDoc->is_dirty())
        {
            LOMSE_LOG_TRACE(Logger::k_events, "Document is already dirty!");
        }

        if (pGmo->is_box_control())
        {
            //AWARE: HyperlinkCtrl and all other dynamic controls arrive here
            LOMSE_LOG_DEBUG(Logger::k_events, "Notify to GmoBoxControl -> Document");
            (static_cast<GmoBoxControl*>(pGmo))->notify_event(pEvent);
            update_view_if_gmodel_modified();
        }
        else if (pGmo->is_box_link() || pGmo->is_in_link())
        {
            //AWARE: Only ImoLink (LMD tag <link>) arrives here.
            LOMSE_LOG_DEBUG(Logger::k_events, "Notify to link -> Global handler");
            find_parent_link_box_and_notify_event(pEvent, pGmo);
        }
        else
        {
            //All other cases arrive here
            LOMSE_LOG_DEBUG(Logger::k_events, "Notify to Document");
            if (!spDoc->notify_observers(pEvent, pEvent->get_source() ))
            {
                LOMSE_LOG_DEBUG(Logger::k_events, "Event discarded");
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::find_parent_link_box_and_notify_event(SpEventInfo pEvent, GmoObj* pGmo)
{
    while(pGmo && !pGmo->is_box_link())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "Gmo type: %d, %s",
                    pGmo->get_gmobj_type(), pGmo->get_name().c_str() );
        pGmo = pGmo->get_owner_box();
    }

    if (pGmo)
    {
        (static_cast<GmoBoxLink*>(pGmo))->notify_event(pEvent);

        if (pEvent->is_on_click_event())
        {
            LOMSE_LOG_DEBUG(Logger::k_events, " post_event to user app.");
            //change event type to be more specific
            pEvent->set_type(k_link_clicked_event);
            m_libScope.post_event(pEvent);
            //AWARE: current document will be destroyed when the event is processed
        }
        else
        {
            LOMSE_LOG_DEBUG(Logger::k_events, " update_view_if_needed");
            update_view_if_needed();
        }
    }
    else
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "No  parent box link found");
    }
}

//---------------------------------------------------------------------------------------
double Interactor::get_elapsed_time_since(ptime startTime) const
{
    //millisecods

    ptime now(true);
    ptime::duration diff = now - startTime;
    return double( diff );
}

//---------------------------------------------------------------------------------------
void Interactor::timing_start_measurements()
{
    //timing starts. render_start = now, gmodel_build=0, gmodel_draw=0, visual_start = now

    for (int i=0; i < k_timing_max_value; ++i)
        m_elapsedTimes[i] = 0.0;

    m_renderStartTime.init_now();
    m_visualEffectsStartTime = m_renderStartTime;
}

//---------------------------------------------------------------------------------------
void Interactor::timing_graphic_model_build_end()
{
    //gmodel_build ends. gmodel_build = now - render_star

    ptime now(true);
    ptime::duration diff = now - m_renderStartTime;
    m_elapsedTimes[k_timing_gmodel_build_time] = double( diff );
}

//---------------------------------------------------------------------------------------
void Interactor::timing_graphic_model_render_end()
{
    //draw gmodel ends. gmodel_draw = (now - render_start) - gmodel_build

    ptime now(true);
    ptime::duration diff = now - m_renderStartTime;
    m_elapsedTimes[k_timing_gmodel_draw_time] =
        double( diff ) - m_elapsedTimes[k_timing_gmodel_build_time];
}

//---------------------------------------------------------------------------------------
void Interactor::timing_visual_effects_start()
{
    //draw visual effects start. visual_start = now

    m_visualEffectsStartTime.init_now();
}

//---------------------------------------------------------------------------------------
void Interactor::timing_renderization_end()
{
    //draw end. visual_draw = now - visual_start; total_render = now - render_start;
    //repaint_start=now

    ptime now(true);
    ptime::duration diff = now - m_visualEffectsStartTime;
    m_elapsedTimes[k_timing_visual_effects_draw_time] =
        double( diff );

    diff = now - m_renderStartTime;
    m_elapsedTimes[k_timing_total_render_time] =
        double( diff );

    m_repaintStartTime = now;
}

//---------------------------------------------------------------------------------------
void Interactor::timing_repaint_done()
{
    //repaint_time = (now - repaint_start)

    ptime now(true);
    ptime::duration diff = now - m_repaintStartTime;
    m_elapsedTimes[k_timing_repaint_time] =  double( diff );
}

//---------------------------------------------------------------------------------------
void Interactor::blink_caret()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView && pGView->is_caret_visible() && pGView->is_caret_blink_enabled())
    {
        pGView->toggle_caret();
        redraw_caret();
    }
}

////---------------------------------------------------------------------------------------
//void Interactor::show_caret(bool fShow)
//{
//    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
//    if (pGView)
//    {
//        fShow ? pGView->show_caret(): pGView->hide_caret();
//        redraw_caret();
//    }
//}
//
////---------------------------------------------------------------------------------------
//void Interactor::hide_caret()
//{
//    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
//    if (pGView)
//    {
//        pGView->hide_caret();
//        redraw_caret();
//    }
//}

//---------------------------------------------------------------------------------------
string Interactor::get_caret_timecode()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->get_caret_timecode();

    return "";
}

//---------------------------------------------------------------------------------------
void Interactor::set_operating_mode(int mode)
{
    if (mode != m_operatingMode && is_operating_mode_allowed(mode))
    {
        m_operatingMode = mode;
        switch_task(mode == k_mode_edition ? TaskFactory::k_task_selection
                                           : TaskFactory::k_task_only_clicks);

        GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
        if (pGView)
        {
            pGView->set_visual_effects_for_mode(mode);
            pGView->draw_all_visual_effects();
            request_window_update();
        }
    }
}

//---------------------------------------------------------------------------------------
bool Interactor::is_operating_mode_allowed(int mode)
{
    if (mode == Interactor::k_mode_edition)
    {
        if (SpDocument spDoc = m_wpDoc.lock())
        {
            ImoDocument* pImoDoc = spDoc->get_im_root();
            int iMax = pImoDoc->get_num_content_items();
            for (int i=0; i < iMax; ++i)
            {
                ImoObj* pImo = pImoDoc->get_content_item(i);
                if (pImo->is_score())
                {
                    ImoScore* pScore = static_cast<ImoScore*>(pImo);
                    if (pScore->get_version_number() < 200)
                        return false;
                }
            }
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------
void Interactor::enable_edition_restricted_to(ImoId id)
{
    //AWARE: This is not implemented as a command, as it is considered an structural
    //operation, not a user command.
    m_operatingMode = k_mode_edition;
    switch_task(TaskFactory::k_task_selection);
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    m_pCursor->jailed_mode_in(id);
    if (pGView)
        pGView->set_visual_effects_for_mode(k_mode_edition);
}

//---------------------------------------------------------------------------------------
void Interactor::show_drag_image(bool fShow)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->show_drag_image(fShow);

        //if hidding, remove image from screen
        if (!fShow)
        {
            pGView->draw_dragged_image();
            request_window_update();
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::enable_drag_image(bool fEnabled)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->enable_drag_image(fEnabled);
        if (fEnabled == false)
        {
            pGView->draw_dragged_image();
            request_window_update();
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::set_drag_image(GmoShape* pShape, bool fGetOwnership, UPoint offset)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_drag_image(pShape, fGetOwnership, offset);
}

//---------------------------------------------------------------------------------------
void Interactor::exec_command(DocCommand* pCmd)
{
    m_pExec->execute(m_pCursor, pCmd, m_pSelections);
    update_caret_and_view();
    send_update_UI_event(k_pointed_object_change);
}

//---------------------------------------------------------------------------------------
void Interactor::exec_undo()
{
    m_pExec->undo(m_pCursor, m_pSelections);
    update_caret_and_view();
    send_update_UI_event(k_pointed_object_change);
}

//---------------------------------------------------------------------------------------
void Interactor::exec_redo()
{
    m_pExec->redo(m_pCursor, m_pSelections);
    update_caret_and_view();
    send_update_UI_event(k_pointed_object_change);
}

//---------------------------------------------------------------------------------------
void Interactor::update_caret_and_view()
{
    update_view_if_needed();
    redraw_caret();
}

//---------------------------------------------------------------------------------------
bool Interactor::should_enable_edit_undo()
{
    return m_pExec->is_undo_possible();
}

//---------------------------------------------------------------------------------------
bool Interactor::should_enable_edit_redo()
{
    return m_pExec->is_redo_possible();
}

////---------------------------------------------------------------------------------------
//void Interactor::enable_edition(bool value)
//{
//    m_fEditionEnabled = value;
//    switch_task(m_fEditionEnabled ? TaskFactory::k_task_selection
//                                  : TaskFactory::k_task_only_clicks);
//}

//---------------------------------------------------------------------------------------
string Interactor::dump_cursor()
{
    return m_pCursor->dump_cursor();
}

//---------------------------------------------------------------------------------------
string Interactor::dump_selection()
{
    return m_pSelections->dump_selection();
}

//---------------------------------------------------------------------------------------
FragmentMark* Interactor::add_fragment_mark_at_note_rest(ImoId scoreId,
                                                               TimeUnits timepos)
{
    FragmentMark* pMark = nullptr;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pMark = pGView->add_fragment_mark_at(scoreId, timepos, false);
        request_window_update();
    }
    return pMark;
}

//---------------------------------------------------------------------------------------
FragmentMark* Interactor::add_fragment_mark_at_barline(ImoId scoreId,
                                                               TimeUnits timepos)
{
    FragmentMark* pMark = nullptr;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pMark = pGView->add_fragment_mark_at(scoreId, timepos, true);
        request_window_update();
    }
    return pMark;
}

//---------------------------------------------------------------------------------------
FragmentMark* Interactor::add_fragment_mark_at_staffobj(ImoStaffObj* pSO)
{
    FragmentMark* pMark = nullptr;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (!pGView)
        return nullptr;

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (!pSO)
            return nullptr;

        pMark = pGView->add_fragment_mark_at_staffobj(pSO);
        request_window_update();
    }

    return pMark;
}

//---------------------------------------------------------------------------------------
void Interactor::remove_mark(ApplicationMark* mark)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    pGView->remove_mark(mark);
}



}  //namespace lomse
