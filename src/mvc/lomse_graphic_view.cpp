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
            throw std::runtime_error("[ViewFactory::create_view] invalid view type");
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
    , m_fTempoLineVisible(false)

    , m_pFunc_update_window(NULL)
    , m_pFunc_force_redraw(NULL)
    , m_pFunc_notify(NULL)
    , m_pObj_update_window(NULL)
    , m_pObj_force_redraw(NULL)
    , m_pObj_notify(NULL)
{
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

    do_force_redraw();
}

//---------------------------------------------------------------------------------------
GraphicModel* GraphicView::get_graphic_model()
{
    return m_pInteractor->get_graphic_model();
}

////---------------------------------------------------------------------------------------
//void GraphicView::on_resize(Pixels vx, Pixels vy)
//{
//    set_viewport_at_page_center(vx);
//}

//---------------------------------------------------------------------------------------
void GraphicView::update_window()
{
    do_update_window();
}

//---------------------------------------------------------------------------------------
void GraphicView::on_paint() //, RepaintOptions& opt)
{
    if (m_pRenderBuf)
    {
        draw_graphic_model();
        draw_sel_rectangle();
        //draw_tempo_line();
        add_controls();
    }
}

//---------------------------------------------------------------------------------------
void GraphicView::on_print_page(int page, double scale, VPoint viewport)
{
    //save and replace scale
    double screenScale = get_scale();
    set_scale(scale);

    //draw page
    m_pDrawer->reset(*m_pPrintBuf, m_options.background_color);
    m_pDrawer->set_viewport(viewport.x, viewport.y);
    m_pDrawer->set_transform(m_transform);

    UPoint origin(0.0f, 0.0f);
    GraphicModel* pGModel = get_graphic_model();
    pGModel->draw_page(page, origin, m_pDrawer, m_options);
    m_pDrawer->render(true);

    //restore scale
    set_scale(screenScale);
}

//---------------------------------------------------------------------------------------
void GraphicView::draw_graphic_model()
{
    start_timer();

    m_options.background_color = Color(35, 52, 91); //127,127,127);
    m_options.page_border_flag = true;
    m_options.cast_shadow_flag = true;

    m_pDrawer->reset(*m_pRenderBuf, m_options.background_color);
    m_pDrawer->set_viewport(m_vxOrg, m_vyOrg);
    m_pDrawer->set_transform(m_transform);

    generate_paths();
    m_pDrawer->render(true);

    ////render statistics
    //double tm = get_elapsed_time();
    //char buf[256];
    //sprintf(buf, "Time=%.3f ms, scale=%.3f\n\n"
    //            "+/- : ZoomIn / ZoomOut (zoom center at mouse point)\n\n"
    //            "1-5 : draw boxes  |  0- remove boxes  |  Click and drag: move score\n\n"
    //            "8-drag mode, 9-selection mode",
    //            tm, m_transform.scale() );
    //m_pDrawer->gsv_text(10.0, 20.0, buf);
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

        double line_width = double( m_pDrawer->Pixels_to_LUnits(1) );

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
void GraphicView::draw_tempo_line()
{
    if (m_fTempoLineVisible)
    {
        double x1 = double( m_tempoLine.left() );
        double y1 = double( m_tempoLine.top() );
        double x2 = double( m_tempoLine.right() );
        double y2 = double( m_tempoLine.bottom() );

        m_pDrawer->screen_point_to_model(&x1, &y1);
        m_pDrawer->screen_point_to_model(&x2, &y2);

        double line_width = double( m_pDrawer->Pixels_to_LUnits(1) );

        m_pDrawer->begin_path();
        m_pDrawer->fill( Color(0, 255, 0) );        //solid green
        m_pDrawer->stroke( Color(0, 255, 0) );      //solid green
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
void GraphicView::highlight_object(ImoStaffObj* pSO)
{

    GraphicModel* pGModel = get_graphic_model();
    pGModel->highlight_object(pSO, true);
    m_highlighted.push_back(pSO);
}

//---------------------------------------------------------------------------------------
void GraphicView::remove_highlight_from_object(ImoStaffObj* pSO)
{
    GraphicModel* pGModel = get_graphic_model();
    pGModel->highlight_object(pSO, false);
    m_highlighted.remove(pSO);
}

//---------------------------------------------------------------------------------------
void GraphicView::remove_all_highlight()
{
    GraphicModel* pGModel = get_graphic_model();
    std::list<ImoStaffObj*>::iterator it;

    for (it = m_highlighted.begin(); it != m_highlighted.end(); ++it)
        pGModel->highlight_object(*it, false);

    m_highlighted.clear();
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
void GraphicView::show_tempo_line(Pixels x1, Pixels y1, Pixels x2, Pixels y2)
{
    m_fTempoLineVisible = true;
    m_tempoLine.left(x1);
    m_tempoLine.top(y1);
    m_tempoLine.right(x2);
    m_tempoLine.bottom(y2);
}

//---------------------------------------------------------------------------------------
void GraphicView::hide_tempo_line()
{
    m_fTempoLineVisible = false;
}

//---------------------------------------------------------------------------------------
void GraphicView::update_tempo_line(Pixels x2, Pixels y2)
{
    m_tempoLine.right(x2);
    m_tempoLine.bottom(y2);
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

    //ensure drawer has the new transform, for pixels <-> LUnits conversions
    m_pDrawer->set_transform(m_transform);
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
void GraphicView::zoom_fit_full(Pixels screenWidth, Pixels screenHeight)
{
//    VPoint invariant = get_invariant_point_for_fit_full(screenWidth, screenHeight);
    //move viewport origin (left-top window corner) to top screen, center
    double rx(screenWidth/2);
    double ry(0);
    m_transform *= agg::trans_affine_translation(-rx, -ry);

    //compute required scale
    GraphicModel* pGModel = get_graphic_model();

    //TODO: Dimensions are based on first page. Change this to take measurements
    //from currently displayed page
    GmoBoxDocPage* pPage = pGModel->get_page(0);
    URect rect = pPage->get_bounds();
    double xScale = m_pDrawer->Pixels_to_LUnits(screenWidth) / rect.width;
    double yScale = m_pDrawer->Pixels_to_LUnits(screenHeight) / rect.height;
    double scale = 0.95 * min (xScale, yScale);

    //apply new user scaling factor
    m_transform.scale(scale);
    m_pDrawer->set_transform(m_transform);

    //move origin back to (rx, ry) so this point remains un-moved
    m_transform *= agg::trans_affine_translation(rx, ry);
    m_vxOrg = Pixels(m_transform.tx);
    m_vyOrg = Pixels(m_transform.ty);

    set_viewport_for_page_fit_full(screenWidth);
}

//---------------------------------------------------------------------------------------
void GraphicView::zoom_fit_width(Pixels screenWidth)
{
    //move viewport origin (left-top window corner) to top screen, center
    double rx(screenWidth/2);
    double ry(0);
    m_transform *= agg::trans_affine_translation(-rx, -ry);

    //compute required scale
    GraphicModel* pGModel = get_graphic_model();
    GmoBoxDocPage* pPage = pGModel->get_page(0);
    URect rect = pPage->get_bounds();
    double scale = 0.95 * ( m_pDrawer->Pixels_to_LUnits(screenWidth) / rect.width );

    //apply new user scaling factor
    m_transform.scale(scale);
    m_pDrawer->set_transform(m_transform);

    //move origin back to (rx, ry) so this point remains un-moved
    m_transform *= agg::trans_affine_translation(rx, ry);
    m_vxOrg = Pixels(m_transform.tx);
    m_vyOrg = Pixels(m_transform.ty);

    set_viewport_at_page_center(screenWidth);
}

//---------------------------------------------------------------------------------------
void GraphicView::set_viewport_at_page_center(Pixels screenWidth)
{
    //get page width
    //TODO: Width taken for first page. Change this to use currently displayed page
    GraphicModel* pGModel = get_graphic_model();
    GmoBoxDocPage* pPage = pGModel->get_page(0);
    URect rect = pPage->get_bounds();

    //determine new viewport origin to center page on screen
    Pixels pageWidth = m_pDrawer->LUnits_to_Pixels(rect.width);
    Pixels left = (screenWidth - pageWidth) / 2;

    //force new viewport
    m_vxOrg = left;
    m_transform.tx = double(left);
}

//---------------------------------------------------------------------------------------
void GraphicView::set_scale(double scale, Pixels x, Pixels y)
{
    double rx(x);
    double ry(y);

    //move origin (left-top window corner) to rx, ry
    m_transform *= agg::trans_affine_translation(-rx, -ry);

    //determine and apply scaling
    double factor = scale / m_transform.scale();
    m_transform *= agg::trans_affine_scaling(factor);

    //move origin back to (rx, ry) so this point remains un-moved
    m_transform *= agg::trans_affine_translation(rx, ry);
    m_vxOrg = Pixels(m_transform.tx);
    m_vyOrg = Pixels(m_transform.ty);
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

//---------------------------------------------------------------------------------------
void GraphicView::set_rendering_option(int option, bool value)
{
    switch(option)
    {
        case k_option_draw_box_doc_page_content:
            m_options.draw_box_doc_page_content_flag = value;
            break;

        case k_option_draw_box_container:
            m_options.draw_box_container_flag = value;
            break;

        case k_option_draw_box_system:
            m_options.draw_box_system_flag = value;
            break;

        case k_option_draw_box_slice:
            m_options.draw_box_slice_flag = value;
            break;

        case k_option_draw_box_slice_instr:
            m_options.draw_box_slice_instr_flag = value;
            break;

        case k_option_draw_box_inline_flag:
            m_options.draw_box_inline_flag = value;
            break;
    }
}

//---------------------------------------------------------------------------------------
void GraphicView::set_update_window_callbak(void* pThis, void (*pt2Func)(void* pObj))
{
    m_pFunc_update_window = pt2Func;
    m_pObj_update_window = pThis;
}

//---------------------------------------------------------------------------------------
void GraphicView::set_force_redraw_callbak(void* pThis, void (*pt2Func)(void* pObj))
{
    m_pFunc_force_redraw = pt2Func;
    m_pObj_force_redraw = pThis;
}

//---------------------------------------------------------------------------------------
void GraphicView::set_notify_callback(void* pThis,
                                      void (*pt2Func)(void* pObj, EventInfo* pEvent))
{
    m_pFunc_notify = pt2Func;
    m_pObj_notify = pThis;
}

//---------------------------------------------------------------------------------------
void GraphicView::do_update_window()
{
    if (m_pFunc_update_window)
        m_pFunc_update_window(m_pObj_update_window);
}

//---------------------------------------------------------------------------------------
void GraphicView::do_force_redraw()
{
    if (m_pFunc_force_redraw)
        m_pFunc_force_redraw(m_pObj_force_redraw);
}

//---------------------------------------------------------------------------------------
void GraphicView::start_timer()
{
    m_startTime = clock();
    m_start = microsec_clock::universal_time();
}

//---------------------------------------------------------------------------------------
double GraphicView::get_elapsed_time() const
{
    //millisecods since last start_timer() invocation

    ptime now = microsec_clock::universal_time();
    time_duration diff = now - m_start;
    return double( diff.total_milliseconds() );
    //return double( diff.total_microseconds() );
    //return double(clock() - m_startTime) * 1000.0 / CLOCKS_PER_SEC;
}

//---------------------------------------------------------------------------------------
void GraphicView::notify_user(EventInfo* pEvent)
{
    if (m_pFunc_notify)
        m_pFunc_notify(m_pObj_notify, pEvent);
}

//---------------------------------------------------------------------------------------
VSize GraphicView::get_page_size_in_pixels(int nPage)
{
    GraphicModel* pGModel = get_graphic_model();
    GmoBoxDocPage* pPage = pGModel->get_page(nPage-1);
    URect rect = pPage->get_bounds();
    VSize size;
    size.width = m_pDrawer->LUnits_to_Pixels(rect.width);
    size.height = m_pDrawer->LUnits_to_Pixels(rect.height);
    return size;
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

    pGModel->draw_page(0, origin, m_pDrawer, m_options);
}

//---------------------------------------------------------------------------------------
int SimpleView::page_at_screen_point(double x, double y)
{
    return 0;       //simple view is only one page
}


//---------------------------------------------------------------------------------------
void SimpleView::set_viewport_for_page_fit_full(Pixels screenWidth)
{
    set_viewport_at_page_center(screenWidth);
}

//---------------------------------------------------------------------------------------
void SimpleView::get_view_size(Pixels* xWidth, Pixels* yHeight)
{
    //TODO
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
    UPoint origin(0.0f, 500.0f);

    m_pageBounds.clear();

    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        GmoBoxDocPage* pPage = pGModel->get_page(i);
        URect rect = pPage->get_bounds();
        UPoint bottomRight(origin.x+rect.width, origin.y+rect.height);
        m_pageBounds.push_back( URect(origin, bottomRight) );
        pGModel->draw_page(i, origin, m_pDrawer, m_options);
        origin.y += rect.height + 1500;
    }
}

//---------------------------------------------------------------------------------------
void VerticalBookView::set_viewport_for_page_fit_full(Pixels screenWidth)
{
    set_viewport_at_page_center(screenWidth);
}

//---------------------------------------------------------------------------------------
void VerticalBookView::get_view_size(Pixels* xWidth, Pixels* yHeight)
{
    LUnits width = 0.0f;
    LUnits height = 500.0f;

    GraphicModel* pGModel = get_graphic_model();
    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        GmoBoxDocPage* pPage = pGModel->get_page(i);
        URect rect = pPage->get_bounds();
        width = max(width, rect.width);
        height += rect.height + 1500;
    }

    *xWidth = m_pDrawer->LUnits_to_Pixels(width);
    *yHeight = m_pDrawer->LUnits_to_Pixels(height);
}


//=======================================================================================
// HorizontalBookView implementation
// A graphic view with pages in horizontal (i.e. Sibelius)
//=======================================================================================
HorizontalBookView::HorizontalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer)
    : GraphicView(libraryScope, pDrawer)
{
}

//---------------------------------------------------------------------------------------
void HorizontalBookView::generate_paths()
{
    GraphicModel* pGModel = get_graphic_model();
    UPoint origin(0.0f, 500.0f);

    m_pageBounds.clear();

    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        GmoBoxDocPage* pPage = pGModel->get_page(i);
        URect rect = pPage->get_bounds();
        UPoint bottomRight(origin.x+rect.width, origin.y+rect.height);
        m_pageBounds.push_back( URect(origin, bottomRight) );
        pGModel->draw_page(i, origin, m_pDrawer, m_options);
        origin.x += rect.width + 1500;
    }
}

//---------------------------------------------------------------------------------------
void HorizontalBookView::set_viewport_for_page_fit_full(Pixels screenWidth)
{
    //Do nothing, so that vertical position of first displayed page is maintained
//    //TODO: sample image to determine first visible page
//
//    //for page 0 start at 500
//    //for page i start at 500 + (page.width + 1500)*i //each page could have a different width
//    int iCurPage = 1;
//    LUnits left = 500.0f;
//    GraphicModel* pGModel = get_graphic_model();
//    for (int i=0; i < iCurPage; i++)
//    {
//        GmoBoxDocPage* pPage = pGModel->get_page(i);
//        URect rect = pPage->get_bounds();
//        left += rect.width;
//        left += 1500;   //TODO named constant
//    }
//
//    m_vxOrg = - m_pDrawer->LUnits_to_Pixels(left);
//    m_transform.tx = double(m_vxOrg);
}

//---------------------------------------------------------------------------------------
void HorizontalBookView::get_view_size(Pixels* xWidth, Pixels* yHeight)
{
    LUnits width = 500.0f;
    LUnits height = 0.0f;

    GraphicModel* pGModel = get_graphic_model();
    for (int i=0; i < pGModel->get_num_pages(); i++)
    {
        GmoBoxDocPage* pPage = pGModel->get_page(i);
        URect rect = pPage->get_bounds();
        height = max(height, rect.height);
        width += rect.width + 1500;
    }

    *xWidth = m_pDrawer->LUnits_to_Pixels(width);
    *yHeight = m_pDrawer->LUnits_to_Pixels(height);
}


}  //namespace lomse
