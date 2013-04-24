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

#include "lomse_interactor.h"

#include "lomse_ldp_compiler.h"
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_basic.h"
#include "lomse_tasks.h"
#include "lomse_gm_basic.h"
#include "lomse_document_layouter.h"
#include "lomse_view.h"
#include "lomse_graphic_view.h"
#include "lomse_events.h"
#include "lomse_player_gui.h"
#include "lomse_document_cursor.h"
#include "lomse_command.h"
#include "lomse_logger.h"

#include <sstream>
using namespace std;

//other
#include <boost/format.hpp>

namespace lomse
{

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
    , m_pGraphicModel(NULL)
    , m_pTask( Injector::inject_Task(TaskFactory::k_task_null, NULL) )
    , m_pCursor(NULL)
    , m_pExec(pExec)
    , m_selections()
    , m_grefLastMouseOver(k_no_gmo_ref)
    , m_renderTime(0.0)
    , m_gmodelBuildTime(0.0)
    , m_fViewParamsChanged(false)
    , m_fViewUpdatesEnabled(true)
{
    switch_task(TaskFactory::k_task_selection);     //k_task_drag_view);

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        LOMSE_LOG_DEBUG(Logger::k_mvc, "[Interactor::Interactor] Creating Interactor. Document is valid");
        Document* pDoc = spDoc.get();
        m_pCursor = Injector::inject_DocCursor(pDoc);

        GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
        if (pGView)
            pGView->use_cursor(m_pCursor);
    }
    LOMSE_LOG_DEBUG(Logger::k_mvc, "[Interactor::Interactor] Interactor created.");
}

//---------------------------------------------------------------------------------------
Interactor::~Interactor()
{
    delete m_pTask;
    delete m_pView;
    delete_graphic_model();
    delete m_pCursor;
    LOMSE_LOG_DEBUG(Logger::k_mvc, "[Interactor::~Interactor] Interactor is deleted");
}

//---------------------------------------------------------------------------------------
void Interactor::switch_task(int taskType)
{
    delete m_pTask;
    m_pTask = Injector::inject_Task(taskType, this);
    m_pTask->init_task();
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
    LOMSE_LOG_DEBUG(Logger::k_render, "");

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        start_timer();

        InternalModel* pIModel = spDoc->get_im_model();
        if (pIModel)
        {
            LOMSE_LOG_DEBUG(Logger::k_render, "[Interactor::create_graphic_model]");
            DocLayouter layouter(pIModel, m_libScope);
            layouter.layout_document();
            m_pGraphicModel = layouter.get_gm_model();
            m_pGraphicModel->build_main_boxes_table();
        }
        spDoc->clear_dirty();

        m_gmodelBuildTime = get_elapsed_time();
    }
//    m_idLastMouseOver = k_no_imoid;
}

//---------------------------------------------------------------------------------------
void Interactor::on_document_reloaded()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, "[Interactor::on_document_reloaded]");
    delete_graphic_model();
    create_graphic_model();
    //TODO: Interactor::on_document_reloaded. Update cursor
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
            force_redraw();
            break;

        case k_highlight_event:
        {
            //AWARE: It sould never arrive here as on_visual_highlight() must
            //be invoked directly by user application to save time
            LOMSE_LOG_DEBUG(Logger::k_events, "Interactor::handle_even] Higlight event received");
            SpEventScoreHighlight pEv(
                boost::static_pointer_cast<EventScoreHighlight>(pEvent) );
            on_visual_highlight(pEv);
            break;
        }

        case k_end_of_playback_event:
        {
            //AWARE: It could never arrive here as send_end_of_play_event() could
            //be invoked directly by user application
            LOMSE_LOG_DEBUG(Logger::k_events, "Interactor::handle_even] End of playback event received");
            SpEventPlayScore pEv( boost::static_pointer_cast<EventPlayScore>(pEvent) );
            if (is_valid_play_score_event(pEv))
                send_end_of_play_event(pEv->get_score(), pEv->get_player());
            break;
        }

        default:
            break;
    }
}

//---------------------------------------------------------------------------------------
void Interactor::delete_graphic_model()
{
    m_selections.clear();
    delete m_pGraphicModel;
    m_pGraphicModel = NULL;
//    m_idLastMouseOver = k_no_imoid;
    LOMSE_LOG_DEBUG(Logger::k_render, "[Interactor::delete_graphic_model] deleted.");
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
    LOMSE_LOG_DEBUG(Logger::k_events, "");
    m_pTask->process_event( Event((flags & k_mouse_left ? Event::k_mouse_left_down
                                                        : Event::k_mouse_right_down),
                                  x, y, flags) );
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_move(Pixels x, Pixels y, unsigned flags)
{
    LOMSE_LOG_TRACE(Logger::k_events, "[Interactor::on_mouse_move] mouse move detected");
    m_pTask->process_event( Event(Event::k_mouse_move, x, y, flags) );
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_button_up(Pixels x, Pixels y, unsigned flags)
{
    m_pTask->process_event( Event((flags & k_mouse_left ? Event::k_mouse_left_up
                                                        : Event::k_mouse_right_up),
                                  x, y, flags) );
}

//---------------------------------------------------------------------------------------
void Interactor::select_object(GmoObj* pGmo, unsigned flags)
{
    m_selections.add(pGmo, flags);
}

//---------------------------------------------------------------------------------------
bool Interactor::is_in_selection(GmoObj* pGmo)
{
    return m_selections.contains(pGmo);
}

//---------------------------------------------------------------------------------------
void Interactor::select_object_at_screen_point(Pixels x, Pixels y, unsigned flags)
{
    GmoObj* pGmo = find_object_at(x, y);
    if (pGmo)
        select_object(pGmo, flags);
}

//---------------------------------------------------------------------------------------
void Interactor::click_at_screen_point(Pixels x, Pixels y, unsigned flags)

{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        GmoObj* pGmo = find_object_at(x, y);
        if (pGmo)
        {
            ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( pGmo->get_creator_imo() );
            if (pGmo->is_box_control() || (pImo && pImo->is_visible()) )
            {
                ImoObj* pImo = find_event_originator_imo(pGmo);
                ImoId id = pImo ? pImo->get_id() : k_no_imoid;
                SpInteractor sp = get_shared_ptr_from_this();
                WpInteractor wp(sp);
                SpEventMouse pEvent( LOMSE_NEW EventMouse(k_on_click_event, wp, id, m_wpDoc) );
                notify_event(pEvent, pGmo);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::mouse_in_out(Pixels x, Pixels y)
{
    GmoObj* pGmo = find_object_at(x, y);
    if (pGmo == NULL)
        return;

    GmoRef gref = find_event_originator_gref(pGmo);

    LOMSE_LOG_DEBUG(Logger::k_events, str(boost::format(
        "[Interactor::mouse_in_out] Gmo %d, %s / gref(%d, %d) -------------------")
         % pGmo->get_gmobj_type() % pGmo->get_name()
         % gref.first % gref.second ));

    if (m_grefLastMouseOver != k_no_gmo_ref && m_grefLastMouseOver != gref)
    {
        LOMSE_LOG_DEBUG(Logger::k_events, str(boost::format(
            "[Interactor::mouse_in_out] Mouse out. gref(%d %d)")
            % m_grefLastMouseOver.first % m_grefLastMouseOver.second ));
        send_mouse_out_event(m_grefLastMouseOver);
        m_grefLastMouseOver = k_no_gmo_ref;
    }

    if (m_grefLastMouseOver == k_no_gmo_ref && gref != k_no_gmo_ref)
    {
        LOMSE_LOG_DEBUG(Logger::k_events, str(boost::format(
            "[Interactor::mouse_in_out] Mouse in. gref(%d %d)")
            % gref.first % gref.second ));
        send_mouse_in_event(gref);
        m_grefLastMouseOver = gref;
    }

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
        return dynamic_cast<ImoContentObj*>( pGmo->get_creator_imo() );
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
//    return NULL;
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
bool Interactor::view_needs_repaint()
{
    //if (!m_fViewUpdatesEnabled)
    //    return false;

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
        return NULL;

    screen_point_to_page_point(&xPos, &yPos);
    GraphicModel* pGM = get_graphic_model();
    return pGM->hit_test(iPage, LUnits(xPos), LUnits(yPos));
}

//---------------------------------------------------------------------------------------
void Interactor::select_objects_in_screen_rectangle(Pixels x1, Pixels y1,
                                                    Pixels x2, Pixels y2,
                                                    unsigned flags)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (!pGView)
        return;

    list<PageRectangle*> rectangles;
    pGView->screen_rectangle_to_page_rectangles(x1, y1, x2, y2, &rectangles);
    if (rectangles.size() == 0)
        return;

    GraphicModel* pGM = get_graphic_model();
    list<PageRectangle*>::iterator it;
    for (it = rectangles.begin(); it != rectangles.end(); ++it)
    {
        pGM->select_objects_in_rectangle((*it)->iPage, m_selections, (*it)->rect, flags);
        delete *it;
    }
}

//---------------------------------------------------------------------------------------
void Interactor::redraw_bitmap()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->redraw_bitmap();
    m_fViewParamsChanged = false;
}

//---------------------------------------------------------------------------------------
void Interactor::request_window_update()
{
    LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::request_window_update]");

    SpInteractor sp = get_shared_ptr_from_this();
    WpInteractor wpIntor(sp);
    SpEventView pEvent( LOMSE_NEW EventView(k_update_window_event, wpIntor) );
    notify_observers(pEvent, this);
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
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->redraw_bitmap();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::new_viewport(Pixels x, Pixels y)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->new_viewport(x, y);
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
void Interactor::show_selection_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->show_selection_rectangle(x1, y1, x2, y2);
}

//---------------------------------------------------------------------------------------
void Interactor::hide_selection_rectangle()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->hide_selection_rectangle();
}

//---------------------------------------------------------------------------------------
void Interactor::update_selection_rectangle(Pixels x2, Pixels y2)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->update_selection_rectangle(x2, y2);
}

//---------------------------------------------------------------------------------------
void Interactor::show_tempo_line(Pixels x1, Pixels y1, Pixels x2, Pixels y2)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->show_tempo_line(x1, y1, x2, y2);
}

//---------------------------------------------------------------------------------------
void Interactor::hide_tempo_line()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->hide_tempo_line();
}

//---------------------------------------------------------------------------------------
void Interactor::remove_all_highlight()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->remove_all_highlight();
}

//---------------------------------------------------------------------------------------
void Interactor::update_tempo_line(Pixels x2, Pixels y2)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->update_tempo_line(x2, y2);
}

//---------------------------------------------------------------------------------------
void Interactor::highlight_object(ImoStaffObj* pSO)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->highlight_object(pSO);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::remove_highlight_from_object(ImoStaffObj* pSO)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->remove_highlight_from_object(pSO);
}

//---------------------------------------------------------------------------------------
void Interactor::discard_all_highlight()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->discard_all_highlight();
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
int Interactor::page_at_screen_point(double x, double y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->page_at_screen_point(x, y);
    return 0;
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_in(Pixels x, Pixels y)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->zoom_in(x, y);
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_out(Pixels x, Pixels y)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->zoom_out(x, y);
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_fit_full(Pixels width, Pixels height)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->zoom_fit_full(width, height);
}

//---------------------------------------------------------------------------------------
void Interactor::zoom_fit_width(Pixels width)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->zoom_fit_width(width);
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
void Interactor::set_scale(double scale, Pixels x, Pixels y)
{
    m_fViewParamsChanged = true;
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_scale(scale, x, y);
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
void Interactor::set_printing_buffer(RenderingBuffer* rbuf)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->set_printing_buffer(rbuf);
}

//---------------------------------------------------------------------------------------
void Interactor::on_print_page(int page, double scale, VPoint viewport)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->on_print_page(page, scale, viewport);
}

//---------------------------------------------------------------------------------------
VSize Interactor::get_page_size_in_pixels(int nPage)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->get_page_size_in_pixels(nPage);
    else
        return VSize(0, 0);
}

////---------------------------------------------------------------------------------------
//void Interactor::on_resize(Pixels x, Pixels y)
//{
//    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
//    if (pGView)
//        pGView->on_resize(x, y);
//}

//---------------------------------------------------------------------------------------
bool Interactor::discard_score_highlight_event_if_not_valid(SpEventScoreHighlight pEvent)
{
    //returns true if event discarded

    ImoObj* pScore = NULL;
    if (SpDocument spDoc = m_wpDoc.lock())
        pScore = spDoc->get_pointer_to_imo( pEvent->get_score_id() );

    if (!pScore || !pScore->is_score())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::discard_score_highlight_event_if_not_valid] Score deleted. All highlight discarded");
        discard_all_highlight();
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool Interactor::is_valid_play_score_event(SpEventPlayScore pEvent)
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
void Interactor::on_visual_highlight(SpEventScoreHighlight pEvent)
{
    static Pixels xPos = 100;

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        if (discard_score_highlight_event_if_not_valid(pEvent))
            return;

        LOMSE_LOG_DEBUG(Logger::k_events, "Interactor::on_visual_highlight] Processing higlight event");
        std::list< pair<int, ImoId> >& items = pEvent->get_items();
        std::list< pair<int, ImoId> >::iterator it;
        for (it = items.begin(); it != items.end(); ++it)
        {
            switch ((*it).first)
            {
                case k_end_of_higlight_event:
                    hide_tempo_line();
                    remove_all_highlight();
                    break;

                case k_highlight_off_event:
                    remove_highlight_from_object( static_cast<ImoStaffObj*>(
                                                spDoc->get_pointer_to_imo((*it).second) ));
                    break;

                case k_highlight_on_event:
                    highlight_object( static_cast<ImoStaffObj*>(
                                            spDoc->get_pointer_to_imo((*it).second) ));
                    break;

                case k_advance_tempo_line_event:
                    xPos += 20;
                    show_tempo_line(xPos, 150, xPos+1, 200);
                    break;

                default:
                    string msg = str( boost::format(
                                    "[Interactor::on_visual_highlight] Unknown event type %d.")
                                    % (*it).first );
                    throw std::runtime_error(msg);
            }
        }
        do_force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void Interactor::send_end_of_play_event(ImoScore* pScore, PlayerGui* pPlayCtrl)
{
    LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::send_end_of_play_event]");

    if (SpDocument spDoc = m_wpDoc.lock())
    {
        Document* pDoc = spDoc.get();
        SpInteractor sp = get_shared_ptr_from_this();
        WpInteractor wpIntor(sp);
        SpEventView pEvent( LOMSE_NEW EventView(k_end_of_playback_event, wpIntor) );
        if (pPlayCtrl)
            pPlayCtrl->on_end_of_playback();

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
void Interactor::send_mouse_out_event(GmoRef gref)
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::send_mouse_out_event]");
        SpInteractor spIntor = get_shared_ptr_from_this();
        WpInteractor wpIntor(spIntor);
        ImoId id = gref.first;
        SpEventMouse pEvent( LOMSE_NEW EventMouse(k_mouse_out_event, wpIntor, id, m_wpDoc));
        GraphicModel* pGM = get_graphic_model();
        GmoObj* pGmo = pGM->get_box_for_control(gref);
        if (pGmo)
            notify_event(pEvent, pGmo);
    }
}

//---------------------------------------------------------------------------------------
void Interactor::send_mouse_in_event(GmoRef gref)
{
    if (SpDocument spDoc = m_wpDoc.lock())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::send_mouse_in_event]");
        SpInteractor sp = get_shared_ptr_from_this();
        WpInteractor wp(sp);
        ImoId id = gref.first;
        SpEventMouse pEvent( LOMSE_NEW EventMouse(k_mouse_in_event, wp, id, m_wpDoc) );
        GraphicModel* pGM = get_graphic_model();
        GmoObj* pGmo = pGM->get_box_for_control(gref);
        if (pGmo)
            notify_event(pEvent, pGmo);
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
            LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::notify_event] notify to GmoBoxControl");
            (static_cast<GmoBoxControl*>(pGmo))->notify_event(pEvent);
            update_view_if_gmodel_modified();
        }
        else if (pGmo->is_box_link() || pGmo->is_in_link())
        {
            LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::notify_event] notify to link");
            find_parent_link_box_and_notify_event(pEvent, pGmo);
        }
        else
        {
            LOMSE_LOG_DEBUG(Logger::k_events, "[Interactor::notify_event] notify to Document");
            spDoc->notify_observers(pEvent, pEvent->get_source() );
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::find_parent_link_box_and_notify_event(SpEventInfo pEvent, GmoObj* pGmo)
{
    while(pGmo && !pGmo->is_box_link())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, str( boost::format("Gmo type: %d, %s")
                    % pGmo->get_gmobj_type() % pGmo->get_name() ) );
        pGmo = pGmo->get_owner_box();
    }

    if (pGmo)
    {
        (static_cast<GmoBoxLink*>(pGmo))->notify_event(pEvent);

        if (pEvent->is_on_click_event())
        {
            LOMSE_LOG_DEBUG(Logger::k_events, " post_event to user app.");
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
void Interactor::start_timer()
{
    m_startTime = clock();
    m_start = microsec_clock::universal_time();
}

//---------------------------------------------------------------------------------------
double Interactor::get_elapsed_time() const
{
    //millisecods since last start_timer() invocation

    ptime now = microsec_clock::universal_time();
    time_duration diff = now - m_start;
    return double( diff.total_milliseconds() );
    //return double( diff.total_microseconds() );
    //return double(clock() - m_startTime) * 1000.0 / CLOCKS_PER_SEC;
}

//---------------------------------------------------------------------------------------
bool Interactor::blink_caret()
{
    // returns true if caret has been repainted
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->toggle_caret();
    else
        return false;
}

//---------------------------------------------------------------------------------------
void Interactor::show_caret()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->show_caret();
}

//---------------------------------------------------------------------------------------
void Interactor::hide_caret()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->hide_caret();
}

//---------------------------------------------------------------------------------------
void Interactor::update_caret()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
    {
        pGView->update_caret();
        request_window_update();
    }
}

//---------------------------------------------------------------------------------------
string Interactor::get_caret_timecode()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        return pGView->get_caret_timecode();

    return "";
}

//---------------------------------------------------------------------------------------
void Interactor::exec_command(DocCommand* pCmd)
{
    m_pExec->execute(m_pCursor, pCmd);
}


////=======================================================================================
//// EditInteractor implementation
////=======================================================================================
//EditInteractor::EditInteractor(LibraryScope& libraryScope, Document* pDoc, View* pView,
//                               DocCommandExecuter* pExec)
//    : Interactor(libraryScope, pDoc, pView, pExec)
//{
//}
//
////---------------------------------------------------------------------------------------
//EditInteractor::~EditInteractor()
//{
//}



}  //namespace lomse
