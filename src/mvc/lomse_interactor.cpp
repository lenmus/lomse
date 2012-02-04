//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

//#include "lomse_user_command.h"
//#include "lomse_command.h"
#include "lomse_compiler.h"
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_basic.h"
#include "lomse_tasks.h"
#include "lomse_gm_basic.h"
#include "lomse_document_layouter.h"
#include "lomse_view.h"
#include "lomse_graphic_view.h"
#include "lomse_events.h"
#include "lomse_player_ctrl.h"
#include <sstream>
using namespace std;

//other
#include <boost/format.hpp>

namespace lomse
{

//=======================================================================================
// Interactor implementation
//=======================================================================================
Interactor::Interactor(LibraryScope& libraryScope, Document* pDoc, View* pView)      //UserCommandExecuter* pExec)
    : EventHandler()
    , EventNotifier()
    , Observable()
    , m_libScope(libraryScope)
    , m_pDoc(pDoc)
    , m_pView(pView)
    , m_pGraphicModel(NULL)
    , m_pTask( Injector::inject_Task(TaskFactory::k_task_null, NULL) )
    , m_selections()
    , m_pLastMouseOverGmo(NULL)
    , m_renderTime(0.0)
    , m_gmodelBuildTime(0.0)
    , m_fViewParamsChanged(false)
    //, m_pExec(pExec)
    //m_pCompiler( Injector::inject_LdpCompiler(m_libScope, *pDocScope) )
{
    switch_task(TaskFactory::k_task_selection);     //k_task_drag_view);
}

//---------------------------------------------------------------------------------------
Interactor::~Interactor()
{
    delete m_pTask;
    delete m_pView;
    delete_graphic_model();
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
    start_timer();

    DocLayouter layouter( m_pDoc->get_im_model(), m_libScope);
    layouter.layout_document();
    m_pGraphicModel = layouter.get_gm_model();

    m_gmodelBuildTime = get_elapsed_time();
}

//---------------------------------------------------------------------------------------
void Interactor::on_document_reloaded()
{
    delete_graphic_model();
    create_graphic_model();
    //TODO
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
            SpEventScoreHighlight pEv(
                boost::static_pointer_cast<EventScoreHighlight>(pEvent) );
            on_visual_highlight(pEv);
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
    m_pLastMouseOverGmo = NULL;
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
    m_pTask->process_event( Event((flags & k_mouse_left ? Event::k_mouse_left_down
                                                        : Event::k_mouse_right_down),
                                  x, y, flags) );
}

//---------------------------------------------------------------------------------------
void Interactor::on_mouse_move(Pixels x, Pixels y, unsigned flags)
{
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
    GmoObj* pGmo = find_object_at(x, y);
    if (pGmo)
    {
        ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( pGmo->get_creator_imo() );
        if (pGmo->is_box_control() || (pImo && pImo->is_visible()) )
        {
            SpEventMouse pEvent( LOMSE_NEW EventMouse(k_on_click_event, this, pGmo,
                                                get_graphic_model()) );
            notify_event(pEvent, pGmo);
        }
    }
}

//---------------------------------------------------------------------------------------
void Interactor::mouse_in_out(Pixels x, Pixels y)
{
    GmoObj* pGmo = find_object_at(x, y);

    if (m_pLastMouseOverGmo && m_pLastMouseOverGmo != pGmo)
    {
        send_mouse_out_event(m_pLastMouseOverGmo);
        m_pLastMouseOverGmo = NULL;
    }

    if (pGmo && m_pLastMouseOverGmo != pGmo)
    {
        ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( pGmo->get_creator_imo() );
        if (pGmo->is_box_control() || (pImo && pImo->is_visible()) )
        {
            send_mouse_in_event(pGmo);
            m_pLastMouseOverGmo = pGmo;
        }
    }

    update_view_if_needed();
}

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
    if (m_pDoc->is_dirty() || m_fViewParamsChanged)
        return true;
    else
    {
        GraphicModel* pGM = get_graphic_model();
        return pGM->is_modified();
    }
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
    SpEventView pEvent( LOMSE_NEW EventView(k_update_window_event, this) );
    notify_observers(pEvent, this);
}

//---------------------------------------------------------------------------------------
void Interactor::force_redraw()
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
        pGView->highlight_object(pSO);
}

//---------------------------------------------------------------------------------------
void Interactor::remove_highlight_from_object(ImoStaffObj* pSO)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->remove_highlight_from_object(pSO);
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
void Interactor::on_visual_highlight(SpEventScoreHighlight pEvent)
{
    static Pixels xPos = 100;

    std::list< pair<int, ImoStaffObj*> >& items = pEvent->get_items();
    std::list< pair<int, ImoStaffObj*> >::iterator it;
    for (it = items.begin(); it != items.end(); ++it)
    {
        ImoStaffObj* pSO = (*it).second;
        switch ((*it).first)
        {
            case k_end_of_higlight_event:
                hide_tempo_line();
                //pScore->RemoveAllHighlight((wxWindow*)m_pPanel);
                break;

            case k_highlight_off_event:
                remove_highlight_from_object(pSO);
                break;

            case k_highlight_on_event:
                highlight_object(pSO);
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
    force_redraw();
}

//---------------------------------------------------------------------------------------
void Interactor::send_end_of_play_event(ImoScore* pScore, PlayerCtrl* pPlayCtrl)
{
    remove_all_highlight();
    SpEventView pEvent( LOMSE_NEW EventView(k_end_of_playback_event, this) );
    if (pPlayCtrl)
        pPlayCtrl->on_end_of_playback();

    m_pDoc->notify_observers(pEvent, m_pDoc);

    update_view_if_needed();
}

//---------------------------------------------------------------------------------------
void Interactor::update_view_if_needed()
{
    if (m_pDoc->is_dirty())
        m_pDoc->notify_if_document_modified();
    else
        update_view_if_gmodel_modified();
}

//---------------------------------------------------------------------------------------
void Interactor::send_mouse_out_event(GmoObj* pGmo)
{
    SpEventMouse pEvent( LOMSE_NEW EventMouse(k_mouse_out_event, this, pGmo,
                                        get_graphic_model()) );
    notify_event(pEvent, pGmo);
}

//---------------------------------------------------------------------------------------
void Interactor::send_mouse_in_event(GmoObj* pGmo)
{
    SpEventMouse pEvent( LOMSE_NEW EventMouse(k_mouse_in_event, this, pGmo,
                                        get_graphic_model()) );
    notify_event(pEvent, pGmo);
}

//---------------------------------------------------------------------------------------
void Interactor::notify_event(SpEventInfo pEvent, GmoObj* pGmo)
{
    if (pGmo->is_box_control())
        (static_cast<GmoBoxControl*>(pGmo))->notify_event(pEvent);
    else if (pGmo->is_in_link())
        find_parent_link_box_and_notify_event(pEvent, pGmo);
    else
        m_pDoc->notify_observers(pEvent, pEvent->get_source() );
}

//---------------------------------------------------------------------------------------
void Interactor::find_parent_link_box_and_notify_event(SpEventInfo pEvent, GmoObj* pGmo)
{
    while(pGmo && !pGmo->is_box_link())
        pGmo = pGmo->get_owner_box();

    if (pGmo)
    {
        (static_cast<GmoBoxLink*>(pGmo))->notify_event(pEvent);
        if (pEvent->is_on_click_event())
            m_libScope.post_event(pEvent);
            //AWARE: current document will be destroyed when the event is processed
        else
            update_view_if_needed();
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


//=======================================================================================
// EditInteractor implementation
//=======================================================================================
EditInteractor::EditInteractor(LibraryScope& libraryScope, Document* pDoc, View* pView)
        //UserCommandExecuter* pExec)
    : Interactor(libraryScope, pDoc, pView)    //, pExec)
{
}

//---------------------------------------------------------------------------------------
EditInteractor::~EditInteractor()
{
}



}  //namespace lomse
