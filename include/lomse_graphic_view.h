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

#ifndef __LOMSE_GRAPHIC_VIEW_H__
#define __LOMSE_GRAPHIC_VIEW_H__

#include "lomse_injectors.h"
#include "lomse_view.h"
#include "lomse_drawer.h"
#include "lomse_doorway.h"
#include "lomse_agg_types.h"
#include "lomse_document_cursor.h"

//other
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
class ImoStaffObj;
class Caret;
class DocCursor;
class OverlaysGenerator;
class VisualEffect;
class DraggedImage;
class SelectionRectangle;
class PlaybackHighlight;
class TimeGrid;
class Handler;
class SelectionHighlight;
class SelectionSet;
class AreaInfo;

typedef SharedPtr<GmoShape>  SpGmoShape;


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
    OverlaysGenerator* m_pOverlaysGenerator;

    //renderization parameters
    double m_expand;
    double m_gamma;
    double m_rotation;
    TransAffine m_transform;

    //current viewport origin and size
    Pixels m_vxOrg, m_vyOrg;
    VSize  m_viewportSize;

    //caret and other visual effects
    Caret*              m_pCaret;
    DocCursor*          m_pCursor;
    DraggedImage*       m_pDragImg;
    SelectionRectangle* m_pSelRect;
    PlaybackHighlight*  m_pHighlighted;
    TimeGrid*           m_pTimeGrid;
    list<Handler*>      m_handlers;
    SelectionHighlight* m_pSelObjects;
//    TempoLine* m_pTempoLine;
    //line to highlight tempo when playing back a score
    bool                m_fTempoLineVisible;
    Rectangle<Pixels>   m_tempoLine;

    //bounds for each displayed page
    std::list<URect> m_pageBounds;

    //for printing
    RenderingBuffer* m_pPrintBuf;

public:
    virtual ~GraphicView();

    //view settings
    void new_viewport(Pixels x, Pixels y);
    void set_rendering_buffer(RenderingBuffer* rbuf);
    void get_viewport(Pixels* x, Pixels* y) { *x = m_vxOrg; *y = m_vyOrg; }
    void set_viewport_at_page_center(Pixels screenWidth);
    virtual void set_viewport_for_page_fit_full(Pixels screenWidth) = 0;
    void use_cursor(DocCursor* pCursor);
    void use_selection_set(SelectionSet* pSelectionSet);
    void add_visual_effect(VisualEffect* pEffect);
    void set_visual_effects_for_mode(int mode);

    //renderization related
    VRect get_damaged_rectangle();
    UPoint get_page_origin_for(GmoObj* pGmo);
    void draw_all_visual_effects();
    void draw_selection_rectangle();
    void draw_playback_highlight();
    void draw_caret();
    void draw_dragged_image();
    void draw_selected_objects();
    void draw_handler(Handler* pHandler);

    //scrolling support
    virtual void get_view_size(Pixels* xWidth, Pixels* yHeight) = 0;

    //selection rectangle
    void start_selection_rectangle(LUnits x1, LUnits y1);
    void hide_selection_rectangle();
    void update_selection_rectangle(LUnits x2, LUnits y2);

    //tempo line
    void show_tempo_line(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    void hide_tempo_line();
    void update_tempo_line(Pixels x2, Pixels y2);

    //highlighting notes and rests
    void highlight_object(ImoStaffObj* pSO);
    void remove_highlight_from_object(ImoStaffObj* pSO);
    void remove_all_highlight();

    // The View is requested to re-paint itself onto the window
    virtual void redraw_bitmap();

    //inline DocCursor& get_cursor() { return m_cursor; }

    //caret
    void show_caret();
    void hide_caret();
    void toggle_caret();
    string get_caret_timecode();
    DocCursorState click_event_to_cursor_state(int iPage, LUnits x, LUnits y,
                                               ImoObj* pImo, GmoObj* pGmo);
    bool is_caret_visible();
    bool is_caret_blink_enabled();

    //dragged image associated to mouse cursor
    void move_drag_image(LUnits x, LUnits y);
    void set_drag_image(GmoShape* pShape, bool fGetOwnership, UPoint offset);
    void show_drag_image(bool value);
    void enable_drag_image(bool fEnabled);

    //handlers
    Handler* handlers_hit_test(LUnits x, LUnits y);

    //graphical model
    GraphicModel* get_graphic_model();

    //coordinates conversions
    void screen_point_to_page_point(double* x, double* y);
    void model_point_to_screen(double* x, double* y, int iPage);
    UPoint screen_point_to_model_point(Pixels x, Pixels y);
    virtual int page_at_screen_point(double x, double y);
    virtual bool trim_rectangle_to_be_on_pages(double* xLeft, double* yTop,
                                               double* xRight, double* yBottom);
    virtual void screen_rectangle_to_page_rectangles(Pixels x1, Pixels y1,
                                                     Pixels x2, Pixels y2,
                                                     list<PageRectangle*>* rectangles);
    LUnits pixels_to_lunits(Pixels pixels);

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

    //info
    AreaInfo* get_info_for_point(Pixels x, Pixels y);

protected:
    GraphicView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);

    void draw_all();
    void draw_graphic_model();
    void draw_tempo_line();
    void draw_time_grid();
    void generate_paths();
    virtual void collect_page_bounds() = 0;
    void draw_visible_pages(int minPage, int maxPage);
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
    void determine_visible_pages(int* minPage, int* maxPage);
    bool is_valid_viewport();
    void delete_rectangles(list<PageRectangle*>& rectangles);
    void layout_caret();
    void layout_time_grid();
    void layout_selection_highlight();
    void delete_all_handlers();
    void add_handler(int iHandler, GmoObj* pOwnerGmo);

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
    void collect_page_bounds();

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
    void collect_page_bounds();

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
    void collect_page_bounds();

};



}   //namespace lomse

#endif      //__LOMSE_GRAPHIC_VIEW_H__
