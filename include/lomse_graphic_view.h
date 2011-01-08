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
using namespace std;


namespace lomse
{

//forward declarations
class ScreenDrawer;
class Drawer;
class Interactor;
class GraphicModel;
class Document;


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
// A view to edit the score in full page
class GraphicView : public View
{
protected:
    LibraryScope& m_libraryScope;
    ScreenDrawer* m_pDrawer;
    std::vector<RenderingBuffer*> m_pPages;
    LomseDoorway* m_pDoorway;
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

    //bounds for each displayed page
    std::list< Rectangle<LUnits> > m_pageBounds;


public:
    GraphicView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~GraphicView();

    //view settings
    void new_viewport(Pixels x, Pixels y);
    void set_rendering_buffer(RenderingBuffer* rbuf) { m_pRenderBuf = rbuf; }
    void get_viewport(Pixels* x, Pixels* y) { *x = m_vxOrg; *y = m_vyOrg; }

//    //--------------------------------------------------------------------
//    // Methods to serve platform dependent event handlers.
   virtual void on_resize(Pixels x, Pixels y);

    //selection rectangle
    void show_selection_rectangle(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    void hide_selection_rectangle();
    void update_selection_rectangle(Pixels x2, Pixels y2);

    //--------------------------------------------------------------------
    // The View is requested to re-paint itself onto the window
    virtual void on_paint();
    //virtual void on_post_draw(void* raw_handler);

    //window related commands
    void update_window();

    //inline DocCursor& get_cursor() { return m_cursor; }

    ////caret movement
    //void caret_right();
    //void caret_left();
    //void caret_to_object(long nId);

    //graphic model
    GraphicModel* get_graphic_model();

    //coordinates conversions
    void screen_point_to_model(double* x, double* y);
    void model_point_to_screen(double* x, double* y, int iPage);
    virtual int page_at_screen_point(double x, double y);

    //scale
    void zoom_in(Pixels x=0, Pixels y=0);
    void zoom_out(Pixels x=0, Pixels y=0);
    void set_scale(double scale, Pixels x=0, Pixels y=0);
    double get_scale();

    //rendering options
    inline void set_option_draw_box_doc_page_content(bool value) {
        m_options.draw_box_doc_page_content_flag = value;
    }
    inline void set_option_draw_box_score_page(bool value) {
        m_options.draw_box_score_page_flag = value;
    }
    inline void set_option_draw_box_system(bool value) {
        m_options.draw_box_system_flag = value;
    }
    inline void set_option_draw_box_slice(bool value) {
        m_options.draw_box_slice_flag = value;
    }
    inline void set_option_draw_box_slice_instr(bool value) {
        m_options.draw_box_slice_instr_flag = value;
    }

protected:
    //GraphicModel* create_graphic_model();
    void draw_graphic_model();
    void draw_sel_rectangle();
    void add_controls();
    virtual void generate_paths() = 0;
    URect get_page_bounds(int iPage);
    int find_page_at_point(LUnits x, LUnits y);



};


//---------------------------------------------------------------------------------------
// A graphic view with one page, no margins (i.e. LenMus ScoreAuxCtrol)
class LOMSE_EXPORT SimpleView : public GraphicView
{
protected:

public:
    SimpleView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~SimpleView() {}

    virtual int page_at_screen_point(double x, double y);

protected:
    void generate_paths();

};


//---------------------------------------------------------------------------------------
// A graphic view with pages in vertical (i.e. Adobe PDF Reader, MS Word)
class LOMSE_EXPORT VerticalBookView : public GraphicView
{
protected:

public:
    VerticalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~VerticalBookView() {}

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

protected:
    void generate_paths();

};



}   //namespace lomse

#endif      //__LOMSE_GRAPHIC_VIEW_H__
