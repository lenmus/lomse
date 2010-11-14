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
//#include "lomse_mvc_builder.h"
//#include "lomse_controller.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
//GraphicView::GraphicView(Document* pDoc)  //, Controller* pController)
GraphicView::GraphicView(LibraryScope& libraryScope, Document* pDoc,
                         PlatformSupport* pPlatform, ScreenDrawer* pDrawer)
    : View(pDoc)    //, pController)
    , m_libraryScope(libraryScope)
    , m_pGraficModel(NULL)
    , m_pDrawer(pDrawer)    // new ScreenDrawer() )
    , m_pPlatform(pPlatform)
    , m_options()
    //, m_cursor(pDoc)

    , m_expand(0.0)
    , m_gamma(1.0)
    , m_scale(1.0)
    , m_rotation(0.0)   //degrees: -180.0 to 180.0
    , m_vxOrg(0)
    , m_vyOrg(0)
    , m_dx(0)
    , m_dy(0)
    , m_drag_flag(false)
{
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
    TextMeter meter( m_libraryScope.font_storage() );
    DocLayouter layouter( m_pDoc->get_im_model(), &meter);
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
    m_pPlatform->update_window();
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
    m_pPlatform->start_timer();

    m_pDrawer->reset(m_pPlatform->get_window_buffer());
    m_pDrawer->set_viewport(m_vxOrg, m_vyOrg);
    m_pDrawer->set_scale(m_scale);

    generate_paths();
    //m_pDrawer->render(true);

    //m_pDrawer->draw_text(1000.0, 3000.0, "Rasterized text placed at (10, 300)");

    //m_pDrawer->begin_path();
    //m_pDrawer->move_to(50.0, 50.0);
    //m_pDrawer->line_to(2000.0, 2000.0);
    //m_pDrawer->line_to(2000.0, 2500.0);
    //m_pDrawer->line_to(50.0, 50.0);
    //m_pDrawer->render(true);

    //m_pDrawer->set_font("timesi.ttf", 1400.0, k_vector_font_cache);
    //m_pDrawer->text(10.0, 400.0, "Vector Text -- Slower, but can be rotated");

    //m_pRenderer->get_bounding_rect(&m_uxMin, &m_uyMin, &m_uxMax, &m_uyMax);

    //render statistics
    double tm = m_pPlatform->elapsed_time();
    char buf[256];
    sprintf(buf, "Time=%.3f ms\n\n"
                 "+/- : ZoomIn / ZoomOut\n\n"
                 "1-5 : draw boxes. 0- remove boxes\n\n"
                 "Click and drag: move score",
                 tm);
    m_pDrawer->gsv_text(10.0, 20.0, buf);
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
void GraphicView::on_mouse_button_down(int x, int y, unsigned flags)
{
    m_dx = x - m_vxOrg;
    m_dy = y - m_vyOrg;
    m_drag_flag = true;
}

//---------------------------------------------------------------------------------------
void GraphicView::on_mouse_move(int x, int y, unsigned flags)
{
    if(flags == 0)
    {
        m_drag_flag = false;
    }

    if(m_drag_flag)
    {
        m_vxOrg = x - m_dx;
        m_vyOrg = y - m_dy;
        m_pPlatform->force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void GraphicView::on_mouse_button_up(int x, int y, unsigned flags)
{
    m_drag_flag = false;
}

////---------------------------------------------------------------------------------------
//void GraphicView::on_document_reloaded()
//{
//    DocCursor cursor(m_pDoc);
//    m_cursor = cursor;
//}

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


//---------------------------------------------------------------------------------------
// SimpleView implementation
// A graphic view with one page, no margins (i.e. LenMus SscoreAuxCtrol)
//---------------------------------------------------------------------------------------
SimpleView::SimpleView(LibraryScope& libraryScope, Document* pDoc,
                       PlatformSupport* pPlatform, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDoc, pPlatform, pDrawer)
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


//---------------------------------------------------------------------------------------
// VerticalBookView implementation
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
//---------------------------------------------------------------------------------------
VerticalBookView::VerticalBookView(LibraryScope& libraryScope, Document* pDoc,
                                   PlatformSupport* pPlatform, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDoc, pPlatform, pDrawer)
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


//---------------------------------------------------------------------------------------
// HorizontalBookView implementation
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
//---------------------------------------------------------------------------------------
HorizontalBookView::HorizontalBookView(LibraryScope& libraryScope, Document* pDoc,
                                       PlatformSupport* pPlatform, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDoc, pPlatform, pDrawer)
{
}

//---------------------------------------------------------------------------------------
void HorizontalBookView::generate_paths()
{
    GraphicModel* pGModel = get_graphic_model();
    UPoint origin(0.0f, 0.0f);

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
