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

#include "lomse_graphic_view.h"

#include <cstdio>       //for sprintf
#include "lomse_gm_basic.h"
#include "lomse_screen_drawer.h"
#include "lomse_interactor.h"

using namespace std;

namespace lomse
{


//=======================================================================================
// ViewFactory implementation
//=======================================================================================
ViewFactory::ViewFactory()
{
}

//---------------------------------------------------------------------------------------
ViewFactory::~ViewFactory()
{
}

//---------------------------------------------------------------------------------------
View* ViewFactory::create_view(LibraryScope& libraryScope, int viewType,
                               ScreenDrawer* pDrawer)
{
    switch(viewType)
    {
        case k_view_simple:
            return new SimpleView(libraryScope, pDrawer);

        case k_view_vertical_book:
            return new VerticalBookView(libraryScope, pDrawer);

        case k_view_horizontal_book:
            return new HorizontalBookView(libraryScope, pDrawer);

        default:
            throw "ViewFactory::create_view: invalid view type";
    }
    return NULL;
}


//=======================================================================================
// GraphicView implementation
//=======================================================================================
GraphicView::GraphicView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
    : View()
    , m_libraryScope(libraryScope)
    , m_pDrawer(pDrawer)
    , m_options()
    , m_pRenderBuf(NULL)
    //, m_cursor(pDoc)

    , m_expand(0.0)
    , m_gamma(1.0)
    , m_rotation(0.0)   //degrees: -180.0 to 180.0
    , m_transform()
    , m_vxOrg(0)
    , m_vyOrg(0)
    , m_fSelRectVisible(false)
{
    m_pDoorway = libraryScope.platform_interface();
}

//---------------------------------------------------------------------------------------
GraphicView::~GraphicView()
{
    delete m_pDrawer;
}

//---------------------------------------------------------------------------------------
void GraphicView::new_viewport(Pixels x, Pixels y)
{
    m_vxOrg = x;
    m_vyOrg = y;
    m_transform.tx = double(x);
    m_transform.ty = double(y);

    m_pDoorway->force_redraw();
}

//---------------------------------------------------------------------------------------
GraphicModel* GraphicView::get_graphic_model()
{
    return m_pInteractor->get_graphic_model();
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
    if (m_pRenderBuf)
    {
        draw_graphic_model();
        draw_sel_rectangle();
        add_controls();
    }
}

//---------------------------------------------------------------------------------------
void GraphicView::draw_graphic_model()
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

//---------------------------------------------------------------------------------------
void GraphicView::draw_sel_rectangle()
{
    if (m_fSelRectVisible)
    {
        double x1 = double( m_selRect.left() );
        double y1 = double( m_selRect.top() );
        double x2 = double( m_selRect.right() );
        double y2 = double( m_selRect.bottom() );

        m_pDrawer->screen_point_to_model(&x1, &y1);
        m_pDrawer->screen_point_to_model(&x2, &y2);

        double line_width = double( m_pDrawer->PixelsToLUnits(1) );

        m_pDrawer->begin_path();
        m_pDrawer->fill( Color(0, 0, 255, 16) );        //background blue transparent
        m_pDrawer->stroke( Color(0, 0, 255, 255) );     //solid blue
        m_pDrawer->stroke_width(line_width);
        m_pDrawer->move_to(x1, y1);
        m_pDrawer->hline_to(x2);
        m_pDrawer->vline_to(y2);
        m_pDrawer->hline_to(x1);
        m_pDrawer->vline_to(y1);
        m_pDrawer->end_path();

        m_pDrawer->render(true);
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
void GraphicView::show_selection_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2)
{
    m_fSelRectVisible = true;
    m_selRect.left(x1);
    m_selRect.top(y1);
    m_selRect.right(x2);
    m_selRect.bottom(y2);
}

//---------------------------------------------------------------------------------------
void GraphicView::hide_selection_rectangle()
{
    m_fSelRectVisible = false;
}

//---------------------------------------------------------------------------------------
void GraphicView::update_selection_rectangle(Pixels x2, Pixels y2)
{
    m_selRect.right(x2);
    m_selRect.bottom(y2);
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

    //move origin (left-top window corner) to rx, ry
    m_transform *= agg::trans_affine_translation(-rx, -ry);

    //apply scaling
    m_transform *= agg::trans_affine_scaling(1.0/1.05);

    //move origin back to (rx, ry) so this point remains un-moved
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
void GraphicView::screen_point_to_model(double* x, double* y)
{
    m_pDrawer->screen_point_to_model(x, y);
    int iPage = find_page_at_point(LUnits(*x), LUnits(*y));
    if (iPage != -1)
    {
        URect pageBounds = get_page_bounds(iPage);
        *x -= pageBounds.left();
        *y -= pageBounds.top();
    }
    else
    {
        *x = LOMSE_OUT_OF_MODEL;
        *y = LOMSE_OUT_OF_MODEL;
    }
}

//---------------------------------------------------------------------------------------
void GraphicView::model_point_to_screen(double* x, double* y, int iPage)
{
    URect pageBounds = get_page_bounds(iPage);
    *x += pageBounds.left();
    *y += pageBounds.top();
    m_pDrawer->model_point_to_screen(x, y);
}

//---------------------------------------------------------------------------------------
URect GraphicView::get_page_bounds(int iPage)
{
    int i = 0;
    std::list< Rectangle<LUnits> >::iterator it;
    for (it = m_pageBounds.begin(); it != m_pageBounds.end() && i != iPage; ++it, ++i);
    return URect(*it);
}

//---------------------------------------------------------------------------------------
int GraphicView::page_at_screen_point(double x, double y)
{
    //returns -1 if point is out of any page

    double ux = x;
    double uy = y;
    screen_point_to_model(&ux, &uy);

    return find_page_at_point(LUnits(ux), LUnits(uy));
}

//---------------------------------------------------------------------------------------
int GraphicView::find_page_at_point(LUnits x, LUnits y)
{
    std::list< Rectangle<LUnits> >::iterator it;
    int iPage = 0;
    for (it = m_pageBounds.begin(); it != m_pageBounds.end(); ++it, ++iPage)
    {
        if ((*it).contains(x, y))
            return iPage;
    }
    return -1;
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


//=======================================================================================
// SimpleView implementation
// A graphic view with one page, no margins (i.e. LenMus SscoreAuxCtrol)
//=======================================================================================
SimpleView::SimpleView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDrawer)
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
int SimpleView::page_at_screen_point(double x, double y)
{
    return 0;       //simple view is only one page
}


//=======================================================================================
// VerticalBookView implementation
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
//=======================================================================================
VerticalBookView::VerticalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDrawer)
{
}

//---------------------------------------------------------------------------------------
void VerticalBookView::generate_paths()
{
    GraphicModel* pGModel = get_graphic_model();
    UPoint origin(500.0f, 500.0f);

    m_options.background_color = Color(127,127,127);
    m_options.page_border_flag = true;
    m_options.cast_shadow_flag = true;

    m_pageBounds.clear();

    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        UPoint bottomRight(origin.x+21000, origin.y+29700);
        m_pageBounds.push_back( URect(origin, bottomRight) );
        pGModel->draw_page(i, origin, m_pDrawer, m_options);
        origin.y += 29700 + 1500;
    }
        //pGModel->draw_page(0, origin, m_pDrawer, m_options);
}


//=======================================================================================
// HorizontalBookView implementation
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
//=======================================================================================
HorizontalBookView::HorizontalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDrawer)
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

    m_pageBounds.clear();

    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        UPoint bottomRight(origin.x+21000, origin.y+29700);
        m_pageBounds.push_back( URect(origin, bottomRight) );
        pGModel->draw_page(i, origin, m_pDrawer, m_options);
        origin.x += 21000 + 1500;
    }
        pGModel->draw_page(0, origin, m_pDrawer, m_options);
        UPoint bottomRight(origin.x+21000, origin.y+29700);
        m_pageBounds.push_back( URect(origin, bottomRight) );
}


}  //namespace lomse
