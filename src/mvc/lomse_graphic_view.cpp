//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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

#include "lomse_graphic_view.h"

#include <cstdio>       //for sprintf
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_screen_drawer.h"
#include "lomse_document_layouter.h"
#include "lomse_presenter.h"
#include "lomse_interactor.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
GraphicView::GraphicView(LibraryScope& libraryScope, Document* pDoc,
                         Interactor* pInteractor, ScreenDrawer* pDrawer)
    : View(pDoc, pInteractor)
    , m_libraryScope(libraryScope)
    , m_pGraficModel(NULL)
    , m_pDrawer(pDrawer)    // new ScreenDrawer() )
    , m_options()
    , m_pRenderBuf(NULL)
    //, m_cursor(pDoc)

    , m_expand(0.0)
    , m_gamma(1.0)
    , m_rotation(0.0)   //degrees: -180.0 to 180.0
    , m_transform()
    , m_vxOrg(0)
    , m_vyOrg(0)
    , m_dx(0)
    , m_dy(0)
    , m_drag_flag(false)
{
    m_pDoorway = libraryScope.platform_interface();
}

//---------------------------------------------------------------------------------------
GraphicView::~GraphicView()
{
    if (m_pGraficModel)
        delete m_pGraficModel;
}

//---------------------------------------------------------------------------------------
GraphicModel* GraphicView::get_graphic_model()
{
    if (!m_pGraficModel)
        m_pGraficModel = create_graphic_model();
    return m_pGraficModel;
}

//---------------------------------------------------------------------------------------
GraphicModel* GraphicView::create_graphic_model()
{
    DocLayouter layouter( m_pDoc->get_im_model(), m_libraryScope);
    layouter.layout_document();
    return layouter.get_gm_model();
}

//---------------------------------------------------------------------------------------
void GraphicView::on_resize(Pixels vx, Pixels vy)
{

}

//---------------------------------------------------------------------------------------
void GraphicView::update_window()
{
    m_pDoorway->update_window();
}

//---------------------------------------------------------------------------------------
void GraphicView::on_paint() //, RepaintOptions& opt)
{
    draw_graphic_model();
    add_controls();
}

//---------------------------------------------------------------------------------------
void GraphicView::draw_graphic_model()
{
    if (m_pRenderBuf)
    {
        m_pDoorway->start_timer();

        m_pDrawer->reset(*m_pRenderBuf);
        m_pDrawer->set_viewport(m_vxOrg, m_vyOrg);
        m_pDrawer->set_transform(m_transform);

        generate_paths();
        m_pDrawer->render(true);

        //render statistics
        double tm = m_pDoorway->elapsed_time();
        char buf[256];
        sprintf(buf, "Time=%.3f ms, scale=%.3f\n\n"
                    "+/- : ZoomIn / ZoomOut (zoom center at mouse point)\n\n"
                    "1-5 : draw boxes  |  0- remove boxes  |  "
                    "Click and drag: move score",
                    tm, m_transform.scale() );
        m_pDrawer->gsv_text(10.0, 20.0, buf);
    }
}

//---------------------------------------------------------------------------------------
void GraphicView::add_controls()
{

    //////render controls
    ////ras.gamma(agg::gamma_none());
    ////agg::render_ctrl(ras, sl, rb, m_expand);
    ////agg::render_ctrl(ras, sl, rb, m_gamma);
    ////agg::render_ctrl(ras, sl, rb, m_scale);
    ////agg::render_ctrl(ras, sl, rb, m_rotate);

    ////render statistics
    //char buf[128];
    //agg::gsv_text t;
    //t.size(10.0);
    //t.flip(true);

    //agg::conv_stroke<agg::gsv_text> pt(t);
    //pt.width(1.5);

    //sprintf(buf, "Vertices=%d Time=%.3f ms", vertex_count, tm);

    //t.start_point(10.0, 40.0);
    //t.text(buf);

    //ras.add_path(pt);
    //ren.color(agg::rgba(0,0,0));
    //agg::render_scanlines(ras, sl, ren);
}

//---------------------------------------------------------------------------------------
void GraphicView::on_mouse_button_down(Pixels x, Pixels y, unsigned flags)
{
    m_dx = x - m_vxOrg;
    m_dy = y - m_vyOrg;
    m_drag_flag = true;
}

//---------------------------------------------------------------------------------------
void GraphicView::on_mouse_move(Pixels x, Pixels y, unsigned flags)
{
    if(flags == 0)
    {
        m_drag_flag = false;
    }

    if(m_drag_flag)
    {
        m_vxOrg = x - m_dx;
        m_vyOrg = y - m_dy;
        
        m_transform.tx = double(m_vxOrg);
        m_transform.ty = double(m_vyOrg);

        m_pDoorway->force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void GraphicView::on_mouse_button_up(Pixels x, Pixels y, unsigned flags)
{
    m_drag_flag = false;
}

//---------------------------------------------------------------------------------------
void GraphicView::zoom_in(Pixels x, Pixels y)
{ 
    double rx(x);
    double ry(y);

    //move origin (left-top window corner) to rx, ry
    m_transform *= agg::trans_affine_translation(-rx, -ry);
    //apply scaling
    m_transform *= agg::trans_affine_scaling(1.05);
    //move origin back to (rx, ry) so this point remains un-moved
    m_transform *= agg::trans_affine_translation(rx, ry);

    m_vxOrg = Pixels(m_transform.tx);
    m_vyOrg = Pixels(m_transform.ty);
}

//---------------------------------------------------------------------------------------
void GraphicView::zoom_out(Pixels x, Pixels y)
{ 
    double rx(x);
    double ry(y);

    m_transform *= agg::trans_affine_translation(-rx, -ry);
    m_transform *= agg::trans_affine_scaling(1.0/1.05);
    m_transform *= agg::trans_affine_translation(rx, ry);

    m_vxOrg = Pixels(m_transform.tx);
    m_vyOrg = Pixels(m_transform.ty);
}

//---------------------------------------------------------------------------------------
void GraphicView::set_scale(double scale, Pixels x, Pixels y) 
{ 
    m_transform.scale(scale);
}

//---------------------------------------------------------------------------------------
double GraphicView::get_scale() 
{ 
    return m_transform.scale();
}

//---------------------------------------------------------------------------------------
void GraphicView::on_document_reloaded()
{
    //TODO
    //DocCursor cursor(m_pDoc);
    //m_cursor = cursor;
}

////---------------------------------------------------------------------------------------
//void GraphicView::caret_right()
//{
//    m_cursor.move_next();
//}
//
////---------------------------------------------------------------------------------------
//void GraphicView::caret_left()
//{
//    m_cursor.move_prev();
//}
//
////---------------------------------------------------------------------------------------
//void GraphicView::caret_to_object(long nId)
//{
//    m_cursor.reset_and_point_to(nId);
//}
//

//---------------------------------------------------------------------------------------
void GraphicView::handle_event(Observable* ref)
{
    //TODO: This method is required by base class Observer
    //if (m_pOwner)
    //{
    //    Notification event(m_pOwner, m_pOwner->get_document(), this);
    //    m_pOwner->notify_user_application(&event);
    //}
}


//=======================================================================================
// SimpleView implementation
// A graphic view with one page, no margins (i.e. LenMus SscoreAuxCtrol)
//=======================================================================================
SimpleView::SimpleView(LibraryScope& libraryScope, Document* pDoc,
                       Interactor* pInteractor, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDoc, pInteractor, pDrawer)
{
}

//---------------------------------------------------------------------------------------
void SimpleView::generate_paths()
{
    GraphicModel* pGModel = get_graphic_model();
    UPoint origin(0.0f, 0.0f);

    m_options.background_color = Color(1,1,1);
    m_options.page_border_flag = false;
    m_options.cast_shadow_flag = false;

    pGModel->draw_page(0, origin, m_pDrawer, m_options);
}


//=======================================================================================
// VerticalBookView implementation
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
//=======================================================================================
VerticalBookView::VerticalBookView(LibraryScope& libraryScope, Document* pDoc,
                                   Interactor* pInteractor, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDoc, pInteractor, pDrawer)
{
}

//---------------------------------------------------------------------------------------
void VerticalBookView::generate_paths()
{
    GraphicModel* pGModel = get_graphic_model();
    UPoint origin(0.0f, 0.0f);

    m_options.background_color = Color(127,127,127);
    m_options.page_border_flag = true;
    m_options.cast_shadow_flag = true;

    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        pGModel->draw_page(i, origin, m_pDrawer, m_options);
        origin.y += 29700 + 1500;
    }
        //pGModel->draw_page(0, origin, m_pDrawer, m_options);
}


//=======================================================================================
// HorizontalBookView implementation
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
//=======================================================================================
HorizontalBookView::HorizontalBookView(LibraryScope& libraryScope, Document* pDoc,
                                       Interactor* pInteractor, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDoc, pInteractor, pDrawer)
{
}

//---------------------------------------------------------------------------------------
void HorizontalBookView::generate_paths()
{
    GraphicModel* pGModel = get_graphic_model();
    UPoint origin(500.0f, 500.0f);

    m_options.background_color = Color(127,127,127);
    m_options.page_border_flag = true;
    m_options.cast_shadow_flag = true;

    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        pGModel->draw_page(i, origin, m_pDrawer, m_options);
        origin.x += 21000 + 1500;
    }
        pGModel->draw_page(0, origin, m_pDrawer, m_options);
}


}  //namespace lomse
