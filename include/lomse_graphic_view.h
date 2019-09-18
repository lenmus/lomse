//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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
#include <mutex>
using namespace std;


///@cond INTERNAL
namespace lomse
{
///@endcond

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
class TempoLine;
class Handler;
class SelectionHighlight;
class SelectionSet;
class AreaInfo;
class FragmentMark;

typedef std::shared_ptr<GmoShape>  SpGmoShape;


////---------------------------------------------------------------------------------------
//enum ERepaintOptions
//{
//    k_repaint_full = 0,     //repaint all
//    k_repaint_only_dirty,   //only GmoObj objects marked as dirty
//};



//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes the available view types for displaying a document.
    - @b k_view_simple means that the document will be displayed not paginated,
        in a single page. It was developed to create small images for
        controls (i.e. a combobox) by rendering small scores (just one measure)
        without margins and grey areas (gaps between pages). **[DEPRECATED]**
    - @b k_view_vertical_book means that the document will be displayed as
        book pages, one page after the other in a vertical layout. The user will
        have to scroll down for advancing.
    - @b k_view_horizontal_book means that the document will be displayed as
        book pages, one page after the other in a horizontal layout. The user will
        have to scroll right for advancing.
    - @b k_view_single_system is for rendering documents that only contain one
        score (e.g. LDP files, LMD files with just one score, and score files imported
        from other formats such as MusicXML). It will display the score in a single
        system, as if the paper had infinite width. And for viewing the end of the score
        the user will have to scroll to the right. See SingleSystemView.

    @#include <lomse_graphic_view.h>
*/
enum EViewType {
    k_view_simple=0,
    k_view_vertical_book,
    k_view_horizontal_book,
    k_view_single_system,
};

///@cond INTERNAL

//---------------------------------------------------------------------------------------
// factory class to create views
class ViewFactory
{
public:
    ViewFactory();
    virtual ~ViewFactory();

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

///@endcond

//---------------------------------------------------------------------------------------
/** %GraphicView is an abstract base class for Views rendering the document as a
    graphic.
*/
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
    std::mutex m_viewportMutex;

    //caret and other visual effects
    Caret*              m_pCaret;
    DocCursor*          m_pCursor;
    DraggedImage*       m_pDragImg;
    SelectionRectangle* m_pSelRect;
    PlaybackHighlight*  m_pHighlighted;
    TimeGrid*           m_pTimeGrid;
    list<Handler*>      m_handlers;
    SelectionHighlight* m_pSelObjects;
    TempoLine*          m_pTempoLine;
    int                 m_trackingEffect;

    //bounds for each displayed page
    std::list<URect> m_pageBounds;

    //for printing
    RenderingBuffer* m_pPrintBuf;
    double           m_print_ppi;     //printer resolution in pixels per inch

    //options
    Color       m_backgroundColor;

public:
///@cond INTERNALS
//excluded from public API because the View methods are managed from Interactor

    virtual ~GraphicView();

    /// @name View settings
    ///@{

    void new_viewport(Pixels x, Pixels y);
    void set_rendering_buffer(RenderingBuffer* rbuf);
    void get_viewport(Pixels* x, Pixels* y) { *x = m_vxOrg; *y = m_vyOrg; }
    void set_viewport_at_page_center(Pixels screenWidth);
    virtual void set_viewport_for_page_fit_full(Pixels screenWidth) = 0;
    void use_cursor(DocCursor* pCursor);
    void use_selection_set(SelectionSet* pSelectionSet);
    void add_visual_effect(VisualEffect* pEffect);
    void set_visual_effects_for_mode(int mode);

    ///@}    //View settings


    /// @name Renderization related
    ///@{
    VRect get_damaged_rectangle();
    UPoint get_page_origin_for(GmoObj* pGmo);
    UPoint get_page_origin_for(int iPage);
    void draw_all_visual_effects();
    void draw_selection_rectangle();
    void draw_visual_tracking();
    void draw_caret();
    void draw_dragged_image();
    void draw_selected_objects();
    void draw_handler(Handler* pHandler);
    void set_background(Color color) { m_backgroundColor = color; }

    ///@}    //Renderization related


    /// @name Scrolling support
    ///@{
    virtual void get_view_size(Pixels* xWidth, Pixels* yHeight) = 0;

    /** For auto-scroll during playback. Change the viewport to ensure that the note/rest
        whose ID is @c ImoId is visible on the screen.
    */
    virtual void change_viewport_if_necessary(ImoId id);
    virtual void change_viewport_if_necessary(ImoId scoreId, TimeUnits timepos);

    /** Force to scroll to specified timepos.
    */
    virtual void change_viewport_to(ImoId scoreId, TimeUnits timepos);

    ///@}    //Scrolling support


    /// @name Selection rectangle
    ///@{
    void start_selection_rectangle(LUnits x1, LUnits y1);
    void hide_selection_rectangle();
    void update_selection_rectangle(LUnits x2, LUnits y2);

    ///@}    //Selection rectangle


    /// @name Visual effects for tracking during playback
    ///@{

    /** Move the tempo line to the given position.
        @param scoreId  Id. of the score to which the operation refers.
        @param timepos The time position to move the tempo line to.
    */
    virtual void move_tempo_line(ImoId scoreId, TimeUnits timepos);

    /** For performance and for sharing common code, this method combines the operation
        of moving the tempo line to the given time position and the operation of
        changing the viewport, if necessary, to ensure that the tempo line is visible.
        @param scoreId  Id. of the score to which the operation refers.
        @param timepos The time position to move the tempo line to.
    */
    virtual void move_tempo_line_and_change_viewport(ImoId scoreId, TimeUnits timepos);

    /** @param scoreId  Id. of the score to which the operation refers.
        @param timepos The time position to move the tempo line to.
        @todo Document Interactor::highlight_object    */
    virtual void highlight_object(ImoStaffObj* pSO);

    /** @param pSO Highlight will be removed from this note or rest.
        @todo Document Interactor::remove_highlight_from_object    */
    virtual void remove_highlight_from_object(ImoStaffObj* pSO);

    /// Remove all visual tracking visual effects.
    virtual void remove_all_visual_tracking();

    /** Select the visual effect to use for visual tracking during playback.
        By default, if this method is not invoked, k_tracking_highlight_notes is used.
        @param mode It is a value from enum EVisualTrackingMode. Several visual effects
        can be en effect simultaneously by combining values
        with the OR ('|') operator. Example:

        @code
        set_visual_tracking_mode(k_tracking_tempo_line | k_tracking_highlight_notes);
        @endcode
    */
	inline void set_visual_tracking_mode(int mode) { m_trackingEffect = mode; }

    /** Returns the specified visual tracking effect.
        @param effect It is a value from enum EVisualTrackingMode. If `k_tracking_none`
			is specified it will return @nullptr.
    */
	VisualEffect* get_tracking_effect(int effect);

    ///@}    //Visual effects for tracking during playback


    /// @name Application markings on the score
    ///@{

    /** Create a new FragmentMark on the score at the given time position for a note/rest
        or a barline.
        @param scoreId  Id. of the score on which the mark will be added.
        @param timepos The position for the mark, in Time Units from the start
               of the score.
        @param fBarline Barlines have the same timepos than the first note/rest after
               the barline. This flag is for selecting the position for the mark, either
               on the barline (if exists) or in the note/rest.
    */
    virtual FragmentMark* add_fragment_mark_at(ImoId scoreId, TimeUnits timepos,
                                               bool fBarline);

    /** Create a new FragmentMark on the score at the given staff object position.
        @param pSO Pointer to the staff object defining the position for the mark.
    */
    virtual FragmentMark* add_fragment_mark_at_staffobj(ImoStaffObj* pSO);

    /** Hide the mark and delete it.
        @param mark  Pointer to the mark to remove. After executing this method the
            pointer will no longer be valid.
    */
    virtual void remove_mark(VisualEffect* mark);

    ///@}    //Application markings on the score


    /** The View is requested to re-paint itself onto the window */
    virtual void redraw_bitmap();

    //graphical model
    GraphicModel* get_graphic_model();

    //handlers
    Handler* handlers_hit_test(LUnits x, LUnits y);


    /// @name Caret
    ///@{
    void show_caret();
    void hide_caret();
    void toggle_caret();
    string get_caret_timecode();
    DocCursorState click_event_to_cursor_state(int iPage, LUnits x, LUnits y,
                                               ImoObj* pImo, GmoObj* pGmo);
    bool is_caret_visible();
    bool is_caret_blink_enabled();
    void change_cursor_voice(int voice);

    ///@}    //Caret


    /// @name Dragged image associated to mouse cursor
    ///@{
    void move_drag_image(LUnits x, LUnits y);
    void set_drag_image(GmoShape* pShape, bool fGetOwnership, UPoint offset);
    void show_drag_image(bool value);
    void enable_drag_image(bool fEnabled);

    //@}    //Dragged image associated to mouse cursor


    /// @name Coordinates conversion
    ///@{
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

    ///@}    //Coordinates conversion


    /// @name Scale
    ///@{
    void zoom_in(Pixels x=0, Pixels y=0);
    void zoom_out(Pixels x=0, Pixels y=0);
    void zoom_fit_full(Pixels width, Pixels height);
    void zoom_fit_width(Pixels width);
    void set_scale(double scale, Pixels x=0, Pixels y=0);
    double get_scale();
    double get_resolution();

    ///@}    //Scale


    /// @name Rendering options
    ///@{
    void set_rendering_option(int option, bool value);
    void reset_boxes_to_draw();
    void set_box_to_draw(int boxType);
    void highlight_voice(int voice);

    ///@}    //Rendering options


    /// @name Layout constrains
    ///@{
    virtual int get_layout_constrains() = 0;
    virtual bool is_valid_for_this_view(Document* pDoc) = 0;

    ///@}    //Layout constrains


    /// @name Support for printing
    ///@{
    void set_print_buffer(RenderingBuffer* rbuf) { m_pPrintBuf = rbuf; }
    void set_print_ppi(double ppi) { m_print_ppi = ppi; }
    virtual void print_page(int page, VPoint viewport);

    ///@}    //Support for printing


    //info
    AreaInfo* get_info_for_point(Pixels x, Pixels y);




///@endcond

protected:
    GraphicView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);

    void draw_all();
    void draw_graphic_model();
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
    void do_change_viewport(Pixels x, Pixels y);

    //scrolling and tempo line

    void determine_scroll_position_for(ImoId scoreId, TimeUnits timepos);

    //results computed by methods:
    //  determine_page_system_and_position_for(ImoId scoreId, TimeUnits timepos)
    //  change_viewport_if_necessary(ImoId id)
    //and used by do_xxx methods:
    //  do_change_viewport_if_necessary()
    //  do_determine_if_scroll_needed()
    GmoBoxSystem* m_pScrollSystem;
    LUnits m_xScrollLeft, m_xScrollRight;

    bool determine_page_system_and_position_for(ImoId scoreId, TimeUnits timepos);
    virtual void do_change_viewport_if_necessary();
    virtual bool do_determine_if_scroll_needed();

    //results computed by method:
    //  do_determine_new_scroll_position()
    //and used by do_xxx methods:
    //  do_change_viewport();
    //  do_determine_if_scroll_needed();
    Pixels k_scrollLeftMargin;
    Pixels m_vxLast, m_vyLast;
    Pixels m_vxNew, m_vyNew;
    int m_iScrollPage;
    //to determine if scroll needed.
    //All these values are relative to current viewport origin
    Pixels m_vySysTop, m_vySysBottom;               //system top, bottom
    Pixels m_vxSysLeft, m_vxSysRight;               //system left, right
    Pixels m_vx_RequiredLeft, m_vx_RequiredRight;   //required visible zone (e.g. measure)

    void do_determine_new_scroll_position();
    void do_change_viewport();


    void do_move_tempo_line_and_change_viewport(ImoId scoreId, TimeUnits timepos,
                                                bool fTempoLine, bool fViewport);

};


///@cond INTERNALS

//---------------------------------------------------------------------------------------
/** %SimpleView is a GraphicView for displaying the document in a single page, without
    borders and margins.
*/
class LOMSE_EXPORT SimpleView : public GraphicView
{
public:

    SimpleView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~SimpleView() {}

    virtual int page_at_screen_point(double x, double y);
    void set_viewport_for_page_fit_full(Pixels screenWidth);
    void get_view_size(Pixels* xWidth, Pixels* yHeight);
    virtual int get_layout_constrains() { return k_use_paper_width | k_use_paper_height; }
    bool is_valid_for_this_view(Document* UNUSED(pDoc)) { return true; }

protected:
    void collect_page_bounds();

};
///@endcond


//---------------------------------------------------------------------------------------
/** %VerticalBookView is a GraphicView for rendering documents in pages, with the
    pages spread in vertical (i.e. Adobe PDF Reader, MS Word)
*/
class LOMSE_EXPORT VerticalBookView : public GraphicView
{
public:
///@cond INTERNALS
//excluded from public API because the View methods are managed from Interactor

    VerticalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~VerticalBookView() {}

    void set_viewport_for_page_fit_full(Pixels screenWidth);
    void get_view_size(Pixels* xWidth, Pixels* yHeight);
    virtual int get_layout_constrains() { return k_use_paper_width | k_use_paper_height; }
    bool is_valid_for_this_view(Document* UNUSED(pDoc)) { return true; }

///@endcond

protected:
    void collect_page_bounds();

};


//---------------------------------------------------------------------------------------
/** %HorizontalBookView is a GraphicView for rendering documents in pages, with the
    pages spread in horizontal (i.e. Finale, Sibelius)
*/
class LOMSE_EXPORT HorizontalBookView : public GraphicView
{
protected:

public:
///@cond INTERNALS
//excluded from public API because the View methods are managed from Interactor

    HorizontalBookView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~HorizontalBookView() {}

    void set_viewport_for_page_fit_full(Pixels screenWidth);
    void get_view_size(Pixels* xWidth, Pixels* yHeight);
    virtual int get_layout_constrains() { return k_use_paper_width | k_use_paper_height; }
    bool is_valid_for_this_view(Document* UNUSED(pDoc)) { return true; }

///@endcond

protected:
    void collect_page_bounds();

};


//---------------------------------------------------------------------------------------
/** %SingleSystemView is a GraphicView for rendering documents that only contain one
    score (e.g. LDP files, LMD files with just one score, and score files imported
    from other formats such as MusicXML). If the document does not contains scores
    or contains more than one score, this view will display an empty view.

    This view will display the score in a single system, as if the paper had infinite
    width. And for viewing the end of the score the user will have to scroll to the
    right.

    When the displayed score does not end in barline but the staff lines continue
    running until the end of the page, the staff lines will be finished after running
    empty for the length of last occupied measure.


    <b>Margins</b>

    The score is displayed on a white paper and the margins around the score are
    as follows:
    - Top margin = document top margin + score top margin
    - Left margin = document left margin + score left margin
    - Bottom margin = Top margin
    - Right margin = Left margin


    <b>Background color</b>

    The white paper is surrounded by the background (gray color). As with all Views,
    the background color can be changed by invoking Interactor::set_view_background().
    E.g. for suppressing the background:
    @code
        m_pPresenter = lomse.open_document(k_view_single_system, filename);
        if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        {
            spInteractor->set_rendering_buffer(&m_rbuf_window);
            spInteractor->set_view_background( Color(255,255,255) );  //white
            ...
    @endcode


    <b>AutoScroll</b>

    As playback advances the View generates EventUpdateViewport events so that measure
    being played is always totally visible.

*/
class LOMSE_EXPORT SingleSystemView : public GraphicView
{
public:
///@cond INTERNALS
//excluded from public API because the View methods are managed from Interactor

    SingleSystemView(LibraryScope& libraryScope, ScreenDrawer* pDrawer);
    virtual ~SingleSystemView() {}

    virtual int page_at_screen_point(double x, double y);

    void set_viewport_for_page_fit_full(Pixels screenWidth);
    void get_view_size(Pixels* xWidth, Pixels* yHeight);
    virtual int get_layout_constrains() { return k_infinite_width | k_use_paper_height; }
    bool is_valid_for_this_view(Document* pDoc);

///@endcond

protected:
    void collect_page_bounds();

};



}   //namespace lomse

#endif      //__LOMSE_GRAPHIC_VIEW_H__
