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
#include <sstream>
using namespace std;
//#define TRT(a) (a)

namespace lomse
{

//=======================================================================================
// Interactor implementation
//=======================================================================================
Interactor::Interactor(LibraryScope& libraryScope, Document* pDoc, View* pView)      //UserCommandExecuter* pExec)
    : Observer()
    , m_libScope(libraryScope)
    , m_pDoc(pDoc)
    , m_pView(pView)
    , m_pGraphicModel(NULL)
    , m_pTask( Injector::inject_Task(TaskFactory::k_task_null, NULL) )
    , m_selections()
    //, m_pExec(pExec)
    //m_pCompiler( Injector::inject_LdpCompiler(m_libScope, *pDocScope) )
{
    switch_task(TaskFactory::k_task_drag_view);
}

//---------------------------------------------------------------------------------------
Interactor::~Interactor()
{
    delete m_pTask;
    delete m_pView;
    delete m_pGraphicModel;
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
        m_pGraphicModel = create_graphic_model();
    return m_pGraphicModel;
}

//---------------------------------------------------------------------------------------
GraphicModel* Interactor::create_graphic_model()
{
    DocLayouter layouter( m_pDoc->get_im_model(), m_libScope);
    layouter.layout_document();
    return layouter.get_gm_model();
}

//---------------------------------------------------------------------------------------
void Interactor::on_document_reloaded()
{
    //TODO
    //DocCursor cursor(m_pDoc);
    //m_cursor = cursor;
}

//---------------------------------------------------------------------------------------
void Interactor::handle_event(Observable* ref)
{
    //TODO: This method is required by base class Observer
    //if (m_pOwner)
    //{
    //    Notification event(m_pOwner, m_pOwner->get_document(), this);
    //    m_pOwner->notify_user_application(&event);
    //}
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
//    m_pDoc->notify_that_document_has_been_modified();
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
    double xPos = double(x);
    double yPos = double(y);
    int iPage = page_at_screen_point(xPos, yPos);
    screen_point_to_model(&xPos, &yPos);
    GraphicModel* pGM = get_graphic_model();
    GmoObj* pGmo = pGM->hit_test(iPage, LUnits(xPos), LUnits(yPos));
    if (pGmo)
        select_object(pGmo, flags);
}

//---------------------------------------------------------------------------------------
void Interactor::select_objects_in_screen_rectangle(Pixels x1, Pixels y1,
                                                    Pixels x2, Pixels y2,
                                                    unsigned flags)
{
    double xLeft = double(x1);
    double yTop = double(y1);
    double xRight = double(x2);
    double yBottom = double(y2);
    int iPage = page_at_screen_point(xLeft, yTop);
    screen_point_to_model(&xLeft, &yTop);
    screen_point_to_model(&xRight, &yBottom);

    GraphicModel* pGM = get_graphic_model();
    URect selRect(xLeft, yTop, xRight-xLeft, yBottom-yTop);
    pGM->select_objects_in_rectangle(iPage, m_selections, selRect, flags);
}

//---------------------------------------------------------------------------------------
void Interactor::on_paint()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->on_paint();
}

//---------------------------------------------------------------------------------------
void Interactor::update_window()
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->update_window();
}

//---------------------------------------------------------------------------------------
void Interactor::new_viewport(Pixels x, Pixels y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->new_viewport(x, y);
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
void Interactor::screen_point_to_model(double* x, double* y)
{
    GraphicView* pGView = dynamic_cast<GraphicView*>(m_pView);
    if (pGView)
        pGView->screen_point_to_model(x, y);
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
