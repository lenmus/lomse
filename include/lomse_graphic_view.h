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

#ifndef __LOMSE_GRAPHIC_VIEW_H__
#define __LOMSE_GRAPHIC_VIEW_H__

#include "lomse_injectors.h"
#include "lomse_view.h"
#include "lomse_drawer.h"
#include "lomse_doorway.h"
#include "lomse_agg_types.h"

#include <vector>
#include <list>


namespace lomse
{

//forward declarations
class ScreenDrawer;
class Drawer;
class Interactor;
class GraphicModel;
class Document;
class ImoStaffObj;


////---------------------------------------------------------------------------------------
//enum ERepaintOptions
//{
//    k_repaint_full = 0,     //repaint all
//    k_repaint_only_dirty,   //only GmoObj objects marked as dirty
//};


//---------------------------------------------------------------------------------------
// factory class to create views
class ViewFactory
{
public:
    ViewFactory();
    virtual ~ViewFactory();

    enum { k_view_simple=0, k_view_vertical_book, k_view_horizontal_book, };

    static View* create_view(LibraryScope& libraryScope, int viewType,
                             ScreenDrawer* pDrawer);

};

//---------------------------------------------------------------------------------------
// helper struct to contain data about a rectangle on a page
struct PageRectangle
{
    int iPage;
    URect rect;

    PageRectangle(int page, LUnits left, LUnits top, LUnits right, LUnits bottom)
        : iPage(page)
        , rect(left, top, right-left, bottom-top)
    {
    }
};

//---------------------------------------------------------------------------------------
// A view to edit the score in full page
class GraphicView : public View
{
protected:
    LibraryScope& m_libraryScope;
    ScreenDrawer* m_pDrawer;
    std::vector<RenderingBuffer*> m_pPages;
    RenderOptions m_options;
    RenderingBuffer* m_pRenderBuf;
    //DocCursor       m_cursor;

    //renderization parameters
    double m_expand;
    double m_gamma;
    double m_rotation;
    TransAffine m_transform;

    //current viewport origin
    Pixels m_vxOrg, m_vyOrg;

    //selection rectangle
    bool                m_fSelRectVisible;
    Rectangle<Pixels>   m_selRect;

    //line to highlight tempo when playing back a score
    bool                m_fTempoLineVisible;
    Rectangle<Pixels>   m_tempoLine;

    //objects currently highlighted
    std::list<ImoStaffObj*> m_highlighted;

    //bounds for each displayed page
    std::list<URect> m_pageBounds;

    //for printing
    RenderingBuffer* m_pPrintBuf;

public:
    virtual ~GraphicView();

    //view settings
    void new_viewport(Pixels x, Pixels y);
    void set_rendering_buffer(RenderingBuffer* rbuf) { m_pRenderBuf = rbuf; }
    void get_viewport(Pixels* x, Pixels* y) { *x = m_vxOrg; *y = m_vyOrg; }
    void set_viewport_at_page_center(Pixels screenWidth);
    virtual void set_viewport_for_page_fit_full(Pixels screenWidth) = 0;

    //scrolling support
    virtual void get_view_size(Pixels* xWidth, Pixels* yHeight) = 0;

    //selection rectangle
    void show_selection_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    void hide_selection_rectangle();
    void update_selection_rectangle(Pixels x2, Pixels y2);

    //tempo line
    void show_tempo_line(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    void hide_tempo_line();
    void update_tempo_line(Pixels x2, Pixels y2);

    //highlighting staffobjs
    void highlight_object(ImoStaffObj* pSO);
    void remove_highlight_from_object(ImoStaffObj* pSO);
    void remove_all_highlight();

    // The View is requested to re-paint itself onto the window
    virtual void redraw_bitmap();

    //inline DocCursor& get_cursor() { return m_cursor; }

    ////caret movement
    //void caret_right();
    //void caret_left();
    //void caret_to_object(long nId);

    //graphic model
    GraphicModel* get_graphic_model();

    //coordinates conversions
    void screen_point_to_page_point(double* x, double* y);
    void model_point_to_screen(double* x, double* y, int iPage);
    virtual int page_at_screen_point(double x, double y);
    virtual bool trim_rectangle_to_be_on_pages(double* xLeft, double* yTop,
                                               double* xRight, double* yBottom);
    virtual void screen_rectangle_to_page_rectangles(Pixels x1, Pixels y1,
                                                     Pixels x2, Pixels y2,
                                                     list<PageRectangle*>* rectangles);

    //scale
    void zoom_in(Pixels x=0, Pixels y=0);
    void zoom_out(Pixels x=0, Pixels y=0);
    void zoom_fit_full(Pixels width, Pixels height);
    void zoom_fit_width(Pixels width);
    void set_scale(double scale, Pixels x=0, Pixels y=0);
    double get_scale();

    //rendering options
    void set_rendering_option(int option, bool value);
    void reset_boxes_to_draw();
    void set_box_to_draw(int boxType);

    //support for printing
    void set_printing_buffer(RenderingBuffer* rbuf) { m_pPrintBuf = rbuf; }
    virtual void on_print_page(int page, double scale, VPoint viewport);
    VSize get_page_size_in_pixels(int nPage);

protected:
    GraphicView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);

    void draw_graphic_model();
    void draw_sel_rectangle();
    void draw_tempo_line();
    void add_controls();
    virtual void generate_paths() = 0;
    URect get_page_bounds(int iPage);
    int find_page_at_point(LUnits x, LUnits y);
    bool shift_right_x_to_be_on_page(double* xLeft);
    bool shift_left_x_to_be_on_page(double* xRight);
    bool shift_down_y_to_be_on_page(double* yTop);
    bool shift_up_y_to_be_on_page(double* yBottom);
    void normalize_rectangle(double* xLeft, double* yTop,
                             double* xRight, double* yBottom);
    void trimmed_rectangle_to_page_rectangles(list<PageRectangle*>* rectangles,
                                              double xLeft, double yTop,
                                              double xRight, double yBottom);

};


//---------------------------------------------------------------------------------------
// A graphic view with one page, no margins (i.e. LenMus ScoreAuxCtrol)
class LOMSE_EXPORT SimpleView : public GraphicView
{
public:
    SimpleView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~SimpleView() {}

    virtual int page_at_screen_point(double x, double y);
    void set_viewport_for_page_fit_full(Pixels screenWidth);
    void get_view_size(Pixels* xWidth, Pixels* yHeight);

protected:
    void generate_paths();

};


//---------------------------------------------------------------------------------------
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
class LOMSE_EXPORT VerticalBookView : public GraphicView
{
public:
    VerticalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~VerticalBookView() {}

    void set_viewport_for_page_fit_full(Pixels screenWidth);
    void get_view_size(Pixels* xWidth, Pixels* yHeight);

protected:
    void generate_paths();

};


//---------------------------------------------------------------------------------------
// A graphic view with pages in horizontall (i.e. Finale, Sibelius)
class LOMSE_EXPORT HorizontalBookView : public GraphicView
{
protected:

public:
    HorizontalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~HorizontalBookView() {}

    void set_viewport_for_page_fit_full(Pixels screenWidth);
    void get_view_size(Pixels* xWidth, Pixels* yHeight);

protected:
    void generate_paths();

};



}   //namespace lomse

#endif      //__LOMSE_GRAPHIC_VIEW_H__
