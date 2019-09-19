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

#ifndef __LOMSE_INTERACTOR_H__
#define __LOMSE_INTERACTOR_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_agg_types.h"
#include "lomse_selections.h"
#include "lomse_events.h"
#include "lomse_document_cursor.h"
#include "lomse_pitch.h"

#include <iostream>
#include <chrono>
using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class CaretPositioner;
class DocCommandExecuter;
class DocCommand;
class DocCursor;
class GmoObj;
class GmoBox;
class GraphicModel;
class Handler;
class ImoScore;
class ImoStaffObj;
class PlayerGui;
class Task;
class VisualEffect;
class FragmentMark;
class ApplicationMark;

class Document;
typedef std::shared_ptr<Document>     SpDocument;
typedef std::weak_ptr<Document>       WpDocument;

class GmoShape;
typedef std::shared_ptr<GmoShape>  SpGmoShape;

//some constants for improving code legibillity
#define k_no_redraw     false   //do not force view redraw
#define k_force_redraw  true    //force a view redraw

#define k_get_ownership         true
#define k_do_not_get_ownership  false

//---------------------------------------------------------------------------------------
// Event flags for mouse and keyboard pressed keys
/** @ingroup enumerations

	This enum describes the flags for mouse events, indicating which mouse buttons
	and keyboard keys are pressed when the mouse event takes place.

	@#include <lomse_interactor.h>
*/
enum EEventFlag
{
    k_mouse_left  = 1,      ///< 0x01. Mouse left button pressed or released
    k_mouse_right = 2,      ///< 0x02. Mouse right button pressed or released
    k_mouse_middle = 4,     ///< 0x04. Mouse middle button pressed or released
    k_kbd_shift   = 8,      ///< 0x08. Keyboard Shift key pressed while mouse event
    k_kbd_ctrl    = 16,     ///< 0x10. Keyboard Ctrol key pressed while mouse event
    k_kbd_alt     = 32,     ///< 0x20. Keyboard Alt key pressed while mouse event
};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes the valid modes for visual tracking during playback.

	@#include <lomse_interactor.h>
*/
enum EVisualTrackingMode
{
    k_tracking_none =               0x0000, ///< Do not add any visual tracking effect
    k_tracking_highlight_notes =    0x0001, ///< Highlight the notes and rest being played back
    k_tracking_tempo_line =	        0x0002, ///< Display a vertical line at beat start
//    k_tracking_tempo_block =	    0x0004, ///< Draw a rectangle surrounding all notes/rests in current beat
};


///@cond INTERNALS
//For performance measurements
struct ptime
{
    ptime(bool init_with_now = false) { if (init_with_now) init_now(); }
    void init_now() { timepoint = chrono::high_resolution_clock::now(); }
    chrono::time_point<chrono::high_resolution_clock> timepoint;
    typedef double duration;
    duration operator-(const ptime rhs);
};
///@endcond


//---------------------------------------------------------------------------------------
// Interactor
/**
	The %Interactor is the key object to interact with the document (here the name
    '%Interactor'). It is the interface between your application, the associated View
    and the Document (see @ref mvc-overview). It is responsible for translating your
    application requests into commands that manipulate the associated View and/or
    the Document, coordinating all the necessary actions. It also provides support for
    managing the user interaction with your application GUI (see @ref page-tasks).

	The %Interactor plays the role of the Controller in the MVC model. Each View has an
	associated %Interactor (in fact the View is owned by the %Interactor). The
	%Interactor is owned by the Presenter.

	The %Interactor for a %View is provided by the Presenter. It is best practice not
	to save pointers to the %Interactor because when processing a Lomse event the
	Document (and thus, the Interactor) could have been deleted (i.e. because your
    application has closed the window displaying the document).

	Lomse provides type @b SpInteractor, an smart pointer to the %Interactor. The
	recommendation is to use always smart pointers when provided by Lomse instead of
	using raw pointers. The use of threads, document edition commands, and events
	processing can invalidate stored pointers in your application. So it is always
	recommended to get the pointer to the %Interactor when needed, by accessing the
	smart pointer and checking that it is still valid:

	@code
		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    //safely use the Interactor
		    spInteractor->some_method();
		    ...
		}
	@endcode


	See:
	- @ref mvc-overview.
	- @ref page-tasks.

*/
class Interactor : public EventHandler
                 , public EventNotifier
                 , public Observable
                 , public std::enable_shared_from_this<Interactor>
{
protected:
    LibraryScope&   m_libScope;
    WpDocument      m_wpDoc;
    View*           m_pView;
    GraphicModel*   m_pGraphicModel;
    Task*           m_pTask;
    DocCursor*      m_pCursor;
    SelectionSet*   m_pSelections;
    DocCommandExecuter* m_pExec;
    GmoRef          m_grefLastMouseOver;
    int             m_operatingMode;
    bool            m_fEditionEnabled;

    //for controlling repaints
    bool        m_fViewParamsChanged;       //viewport, scale, ... have been modified

    //to avoid problems during playback
    bool        m_fViewUpdatesEnabled;

    Handler*    m_pCurHandler;  //current handler being dragged, if any
    ImoId       m_idControlledImo;

public:

    //enums
    /** Valid operating modes for the %Interactor:
        - @b k_mode_read_only  - Read only mode: no changes allowed in the document.
        - @b k_mode_edition  - Edition mode: changes allowed in the document.
        - @b k_mode_playback  - Playback mode: disabled any action that could affect playback in any way. Edition is disabled in this mode.
    */
    enum EInteractorOpMode
    {
        k_mode_read_only=0,     ///< Read only mode: no changes allowed in the document.
        k_mode_edition,         ///< Edition mode: changes allowed in the document.
        k_mode_playback,        ///< Playback mode: disabled any action that could affect
                                ///< playback in any way. Edition is disabled in this mode.
    };


    /** Method Interactor#get_elapsed_times() returns a vector of elapsed times. This enum is used as index on that vector
        for identifying the operation to which each vector element refers. So, for instance, times[0] is the time for
        building the graphic model (0 = @a k_timing_gmodel_build_time), times[1] is the time for
        rendering the graphic model (1 = @a k_timing_gmodel_draw_time), etc, according to this:
        - <b>k_timing_gmodel_build_time = 0</b> - elapsed time for building the graphic model.
        - <b>k_timing_gmodel_draw_time = 1</b> - elapsed time for rendering the graphic model
        - <b>k_timing_visual_effects_draw_time = 2</b> - elapsed time for rendering the visual effects
        - <b>k_timing_total_render_time = 3</b> - total elapsed time for renderization
        - <b>k_timing_repaint_time = 4</b> - elapsed time for repainting the view
        - <b>k_timing_max_value</b> - Not used as index. This value is for knowing how many items you should expect in the
            returned vector, for allocating space.
    */
    enum ETimingTarget { k_timing_gmodel_build_time=0, k_timing_gmodel_draw_time,
       k_timing_visual_effects_draw_time, k_timing_total_render_time,
       k_timing_repaint_time, k_timing_max_value, };


        //operating modes and related
        /** @name Modes of operation and related    */
        //@{

    /** Set current operation mode. An operation mode defines valid actions
        and commands on the Document. For instance, in 'read only' mode all actions
        that could alter Document content will be ignored. Valid operation modes
        are defined by enum #EInteractorOpMode. Look there for an explanation of
        the different operating modes.

        Example:

        @code
        void DocumentWindow::play_score(SpEventInfo pEvent)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                spInteractor->set_operating_mode(Interactor::k_mode_playback);

                SpEventPlayCtrl pEv = static_pointer_cast<EventPlayCtrl>(pEvent);
                ImoScore* pScore = pEv->get_score();
                ScorePlayer* pPlayer  = m_appScope.get_score_player();
                PlayerGui* pPlayerGui = pEv->get_player();

                pPlayer->load_score(pScore, pEv->get_player());

                //initialize with default options
                bool fVisualTracking = true;
                long nMM = pPlayerGui->get_metronome_mm();

                pPlayer->play(fVisualTracking, nMM, spInteractor.get());
            }
        @endcode
    */
    void set_operating_mode(int mode);


    /** Returns current operation mode for this %Interactor.
        Returned value is one of the values from enum EInteractorOpMode.

        Example:
        @code
        bool DocumentWindow::is_edition_enabled()
        {
            if (!m_pPresenter)
                return false;

            if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
                return spIntor->get_operating_mode() == Interactor::k_mode_edition;
            else
                return false;
        }
        @endcode
    */
    inline int get_operating_mode() { return m_operatingMode; }


    /** When in edition mode, invoking this method disables edition in all the document
        with the exception of the object whose ID is passed. This method allows your
        application to restrict edition to a certain area of the document, such as an
        score, a paragraph, etc.
        @param id   The ID of the only object that will be editable after invoking this method.
    */
    void enable_edition_restricted_to(ImoId id);


    /** Returns @TRUE if the document is editable.

        @note Currently, LenMus documents are always editable. Therefore this method will
            always return @TRUE. But by invoking this method your application can be
            ready for disabling edition for future read-only documents.
    */
    bool is_document_editable();    //not yet implemented. Made private


    /** Switch the %Task used by the %Interactor for interpreting mouse events.

        @remarks When the %Interactor is created it is initialized with an instace of
            TaskSelection class.

        See @ref page-tasks.

        Example:

        @code
        void DocumentWindow::switch_mode_for_current_tool(int toolType)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                switch(toolType)
                {
                    case k_tool_data_entry:
                        spInteractor->switch_task(TaskFactory::k_task_data_entry);
                        break;

                    case k_tool_selection:
                    default:
                        spInteractor->switch_task(TaskFactory::k_task_selection);
                }
            }
        }
        @endcode
    */
    void switch_task(int taskType);

    /** Define the duration for one beat, for metronome and for methods that use
        measure/beat parameters to define a location. This value is shared by all
        scores contained in the document and can be changed at any time.
        Changes while the score is being played back are ignored until playback finishes.
        @param beatType A value from enum #EBeatDuration.
        @param duration The duration (in Lomse Time Units) for one beat. You can use
            a value from enum ENoteDuration casted to double. This parameter is
            required only when value for parameter `beatType` is `k_beat_specified`.
            For all other values, if a non-zero value is specified, the value
            will be used for the beat duration in scores without time signature.
    */
    void define_beat(int beatType, TimeUnits duration=0.0);

    //@}    //operating modes


    //access to collaborators
    /** @name Access to collaborators in MVC model    */
    //@{

    /** Returns the graphic model object associated to the View of this %Interactor.   */
    GraphicModel* get_graphic_model();


    /** Returns the View associated to this %Interactor.    */
    inline View* get_view() { return m_pView; }


    /** Returns the selection set associated to the View of this %Interactor.

        See @ref page-edit-overview
    */
    inline SelectionSet* get_selection_set() { return m_pSelections; }

        //@}    //access to collaborators



    //interface to View
    /// @name Interface to View
    //@{

    /** Invoking this method forces Lomse to render again the View and, therefore,
        the rendering buffer gets updated. After doing it, Lomse <b>does not</b>
        generate a EventPaint event, as invoking this method implies that your
        application is aware of the rendering buffer change.

        @see force_redraw().
    */
    virtual void redraw_bitmap();


    /** Invoking this method forces Lomse to render again the View and, therefore,
        the rendering buffer gets updated. After doing it, Lomse <b>will</b>
        generate a EventPaint event.

        @note Lomse will ignore the request if forced updates are
        disabled. See enable_forced_view_updates().

        @todo Clarify the need of methods force_redraw(), redraw_bitmap() and
        enable_forced_view_updates(). Probably This code has to be re-factored.

        @see redraw_bitmap().
    */
    virtual void force_redraw();


    /** Returns @TRUE if the document, the graphic model, or any View option have
        been changed since last view rendering.

        Your application rarely will need to
        invoke this method as Lomse automatically triggers updates when anything is
        changed.
    */
    bool view_needs_repaint();


    //creating events
    /**
        @todo This method should be for internal use. Analyse and refactor or document.

        This method should be invoked when processing a EventPlayCtrl of type
        <tt>k_end_of_playback_event</tt>. But this method generates a
        k_end_of_playback_event, this will create a loop!!!!   ?????????
    */
    virtual void on_end_of_play_event(ImoScore* pScore, PlayerGui* pPlayCtrl);


    /** Invoking this method controls the behavior of method force_redraw(). If disabled,
        Lomse will ignore any invocation to force_redraw().

        @param value @TRUE for enabling method force_redraw(). @FALSE for ignoring
        invocations to this method.

        By default, force_redraw() method is enabled.
    */
    inline void enable_forced_view_updates(bool value) { m_fViewUpdatesEnabled = value; }

        //@}    //interface to View



        //interface to GraphicView. Renderization
        /// @name Interface to GraphicView. Rendering
        //@{


    /** Associate a rendering buffer to the View related to this %Interactor.
        @param rbuf A ptr to the memory to be used as rendering buffer.

        Invoking this method is mandatory before doing any operation that would require
        to render the view. Normally this method is invoked when creating a new view.
        Example:

        @code
        void DocumentWindow::display_document(const string& filename, int viewType)
        {
            //stop playback (just in case an score is being played in another window)
            ScorePlayer* pPlayer  = m_appScope.get_score_player();
            pPlayer->stop();

            //get lomse reporter
            ostringstream& reporter = m_appScope.get_lomse_reporter();
            reporter.str(std::string());      //remove any previous content

            //delete current document, view, etc. associated to this window
            delete m_pPresenter;
            //and load file
            m_pPresenter = m_lomse.new_document(viewType, filename, reporter);

            set_zoom_mode(k_zoom_fit_width);
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                //connect the View with the window buffer
                spInteractor->set_rendering_buffer(&m_rbuf_window);

                //register to receive the desired events
                spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
                spInteractor->add_event_handler(k_do_play_score_event, this, wrapper_play_score);
                spInteractor->add_event_handler(k_pause_score_event, this, wrapper_play_score);
                spInteractor->add_event_handler(k_stop_playback_event, this, wrapper_play_score);
                spInteractor->add_event_handler(k_control_point_moved_event, this, wrapper_on_command_event);
                Document* pDoc = m_pPresenter->get_document_raw_ptr();
                pDoc->add_event_handler(k_on_click_event, this, wrapper_on_click_event);

                // display any errors
                if (!reporter.str().empty())
                {
                    string errorMsg = reporter.str();
                    ...
                    ErrorDlg dlg(this, errorMsg, ...);
                    dlg.ShowModal();
                }
                reporter.str(std::string());      //remove any previous content

            }
        }
        @endcode
    */
    virtual void set_rendering_buffer(RenderingBuffer* rbuf);


    /** Set/reset a rendering option for the view associated to this %Interactor.

        @param option The option to set or reset. Valid values for this param are
            defined by enum #ERenderOptions
        @param value @TRUE for enabling the option or @FALSE for disabling it.

        Example:

        @code
        void DocumentWindow::display_voices_in_colours(bool value)
        {
            if (!m_pPresenter)
                return;

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                spInteractor->set_rendering_option(k_option_display_voices_in_colours, value);
                Refresh();    //force to repaint this window
            }
        }
        @endcode
    */
    virtual void set_rendering_option(int option, bool value);


    /** Changes the background color for the View. By default all Views have a gray
        background and the paper is white. Example, for suppressing the background:
        @code
            m_pPresenter = lomse.open_document(k_view_single_system, filename);
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                spInteractor->set_rendering_buffer(&m_rbuf_window);
                spInteractor->set_view_background( Color(255,255,255) );  //white
                ...
        @endcode
    */
    void set_view_background(Color color);

        //@}    //interface to GraphicView. Rendering



    //interface to GraphicView. Units conversion and related
    /// @name Interface to GraphicView. Coordinates conversion
    //@{

    /** Converts device coordinates (pixels) to coordinates in logical
        units. The returned values are not absolute but relative to the start of
        the page at which the device point is pointing.

        Once invoked, variables @a x and @a y should be statically
        cast to logical units (@a LUnits type).

        Example:
        @code
            double x = 4500.0;  //pixels relative to view origin
            double y = 2700.0;  //pixels
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                int iPage = spInteractor->page_at_screen_point(x, y);
                spInteractor->screen_point_to_page_point(&x, &y);

                //Here @a x and @a y contains logical units relative to
                //page iPage origin

            }
        @endcode
    */
    virtual void screen_point_to_page_point(double* x, double* y);


    /** Converts logical coordinates relative to the start of a page to absolute
        coordinates in device units (pixels, relative to view origin).

        Example:
        @code
            int iPage = 4;      //in fact page 5. Remember iPage = 0..num_pages-1
            double x = 4500.0;  //LUnits relative to page 5 origin
            double y = 2700.0;
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                spInteractor->model_point_to_screen(&x, &y, iPage);

                //Here @a x and @a y contains pixels relative to
                //view origin
            }
        @endcode
    */
    virtual void model_point_to_screen(double* x, double* y, int iPage);


    /** Returns the page number (0 .. num_pages - 1) that contains the
        point <i>(x, y)</i>  or -1 if point is out of page. Variables
        @a x and @a y are in absolute device units
        (pixels, relative to view origin).

        Example:
        @code
            double x = 4500.0;  //pixels relative to view origin
            double y = 2700.0;  //pixels
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                int iPage = spInteractor->page_at_screen_point(x, y);

                //Here @a iPage is the page number (0 .. num_pages - 1)
            }
        @endcode
    */
    virtual int page_at_screen_point(double x, double y);


    /** Converts absolute device coordinates (pixels, relative to view origin)
        to absolute logical units.

        Example:
        @code
        void DocumentWindow::on_mouse_move_event(MouseEvent& event)
        {
            if (!m_pPresenter)
                return;

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                Interactor* pInteractor = spInteractor.get();
                if (!pInteractor) return;

                Point pos = event.GetPosition();
                unsigned flags = event.GetFlags(event);

                UPoint uPos = pInteractor->screen_point_to_model_point(pos.x, pos.y);

                //inform Lomse
                pInteractor->on_mouse_move(pos.x, pos.y, flags);
            }
        }
        @endcode
    */
    virtual UPoint screen_point_to_model_point(Pixels x, Pixels y);


    /** Returns the pitch (@a DiatonicPitch type) of the staff point pointed by
        coordinates @a x, @a y or value @a k_no_pitch if not pointing to a staff.

        This helper method is useful in some scenarios. For instance, if in your
        application the user is allowed to insert notes on a staff by clicking with the
        mouse on the insertion point, you will need to determine the nearest staff
        line/space, and the applicable clef, in order to determine the pitch for the
        note to insert. This method performs all these operations and returns the
        diatonic pitch for the staff point.

        @code
        DiatonicPitch DocumentWindow::get_pitch_at(Pixels x, Pixels y)
        {
            if (m_pPresenter)
            {
                if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
                {
                    return spInteractor->get_pitch_at(x, y);
                }
            }
            return DiatonicPitch(k_no_pitch);
        }
        @endcode
    */
    DiatonicPitch get_pitch_at(Pixels x, Pixels y);

    //@}    //interface to GraphicView. Units conversion and related



    //interface to GraphicView. Viewport / scroll
    /// @name Interface to GraphicView. Viewport (for scrolling)
    //@{

    /** Sets a new origin for the viewport.
        @param x,y new coordinates (in pixels) for the viewport.
        @param fForceRedraw If @false prevents redrawing the new viewport
            into the rendering buffer; this is useful for saving time when several
            consecutive changes are going to be done. If not specified @true is assumed.

        @remarks The size of viewport (height, width) is always determined by rendering
            buffer size.

        See @ref viewport-concept

        @see set_viewport_at_page_center(), get_viewport(), get_view_size()
    */
    virtual void new_viewport(Pixels x, Pixels y, bool fForceRedraw=true);

    /** Sets a new @a x origin for the viewport so that the viewport is centered on
        document page.
        @param screenWidth is the desired viewport width (in pixels) for computing the
            the new @a x origin.

        @remarks This method computes the new origin as (pageWidth - screenWidth) / 2;

        See @ref viewport-concept

        @see new_viewport(), get_viewport(), get_view_size()
    */
    virtual void set_viewport_at_page_center(Pixels screenWidth);

    /** Returns the current coordinates (pixels) of viewport origin.
        @param x,y The variables in which the viewport origin will be returned.

        See @ref viewport-concept

        @see new_viewport(), set_viewport_at_page_center(), get_view_size()
    */
    virtual void get_viewport(Pixels* x, Pixels* y);

    /** Returns the total size (pixels) of the whole rendered document (the whole visual
        space, all pages).
        @param xWidth
            Receives the width of the whole rendered document, in pixels.
        @param yHeight
            Receives the height of the whole rendered document, in pixels.

        @see new_viewport(), set_viewport_at_page_center(), get_viewport()
    */
    virtual void get_view_size(Pixels* xWidth, Pixels* yHeight);

    /** This method invokes Lomse auto-scrolling algorithm to determine if scroll is
        necessary, and if that is the case, it will set a new origin for the
        viewport so that requested score location is visible in the viewport.
        @param scoreId ID of the score to which all other parameters refer to.
        @param iMeasure The index to the desired measure. First measure in the
            instrument, including a possible anacrusis start measure, is always measure 0.
        @param iBeat The index to the desired beat in the measure. First beat is 0.
        @param iInstr The index to the instrument to which the measure refers. If not
            specified, iInstr 0 is assumed. Normally,
            all instruments in the score use the same time signature. In these cases all
            score parts have the same number of measures and iInstr is not needed.
            But in polymetric music not all instruments use the same time signature and
            this implies that the instruments have different number of measures; in these
            cases the measure number alone is not enough for determining
            the location.

        See @ref viewport-concept
    */
    virtual void scroll_to_measure_if_necessary(ImoId scoreId, int iMeasure, int iBeat=0, int iInstr=0);

    /** This method invokes Lomse auto-scrolling algorithm to determine if scroll is
        necessary, and if that is the case, it will set a new origin for the
        viewport so that requested score location is visible in the viewport.
        @param scoreId ID of the score to which all other parameters refer to.
        @param iMeasure The index to the desired measure. First measure in the
            instrument, including a possible anacrusis start measure, is always measure 0.
        @param location Time units from the start of the measure.
        @param iInstr The index to the instrument to which the measure refers. If not
            specified, iInstr 0 is assumed. Normally,
            all instruments in the score use the same time signature. In these cases all
            score parts have the same number of measures and iInstr is not needed.
            But in polymetric music not all instruments use the same time signature and
            this implies that the instruments have different number of measures; in these
            cases the measure number alone is not enough for determining
            the location.

        See @ref viewport-concept
    */
    virtual void scroll_to_measure_if_necessary(ImoId scoreId, int iMeasure,
                                                TimeUnits location=0.0, int iInstr=0);

    /** This method forces to set a new origin for the viewport so that requested score
        location is visible in the viewport.
        @param scoreId ID of the score to which all other parameters refer to.
        @param iMeasure The index to the desired measure. First measure in the
            instrument, including a possible anacrusis start measure, is always measure 0.
        @param iBeat The index to the desired beat in the measure. First beat is 0.
        @param iInstr The index to the instrument to which the measure refers. If not
            specified, iInstr 0 is assumed. Normally,
            all instruments in the score use the same time signature. In these cases all
            score parts have the same number of measures and iInstr is not needed.
            But in polymetric music not all instruments use the same time signature and
            this implies that the instruments have different number of measures; in these
            cases the measure number alone is not enough for determining
            the location.

        See @ref viewport-concept
    */
    virtual void scroll_to_measure(ImoId scoreId, int iMeasure, int iBeat=0, int iInstr=0);

    /** This method forces to set a new origin for the viewport so that requested score
        location is visible in the viewport.
        @param scoreId ID of the score to which all other parameters refer to.
        @param iMeasure The index to the desired measure. First measure in the
            instrument, including a possible anacrusis start measure, is always measure 0.
        @param location Time units from the start of the measure.
        @param iInstr The index to the instrument to which the measure refers. If not
            specified, iInstr 0 is assumed. Normally,
            all instruments in the score use the same time signature. In these cases all
            score parts have the same number of measures and iInstr is not needed.
            But in polymetric music not all instruments use the same time signature and
            this implies that the instruments have different number of measures; in these
            cases the measure number alone is not enough for determining
            the location.

        See @ref viewport-concept
    */
    virtual void scroll_to_measure(ImoId scoreId, int iMeasure, TimeUnits location=0.0,
                                   int iInstr=0);

    //@}    //interface to GraphicView. Viewport / scroll



    //interface to GraphicView. Scale
    /// @name Interface to GraphicView. Scale
    //@{

    /** Returns the current scale factor.

        See @ref scale-factor
    */
    virtual double get_scale();


    /** Sets the scaling factor, useful for applications which require 'zooming'.
        @param scale Is the new scale factor, e.g. 2.0
        @param x,y Are the coordinates for the point that will remain fixed (unmoved)
            when applying the new scale (the center point for the zooming operation).
        @param fForceRedraw If @FALSE prevents Lomse from sending a paint event. This is
            useful to avoid repaints when some consecutive operations will affect
            the View.

        See @ref scale-factor

        @see zoom_in(), zoom_out(), zoom_fit_full(), zoom_fit_width(), get_scale()
    */
    virtual void set_scale(double scale, Pixels x=0, Pixels y=0, bool fForceRedraw=true);


    /** Increments the scaling factor by 5%.
        @param x,y Are the coordinates for the point that will remain fixed (unmoved)
            when applying the new scale (the center point for the zooming operation).
        @param fForceRedraw If @FALSE prevents Lomse from sending a paint event. This is
            useful to avoid repaints when some consecutive operations will affect
            the View.

        See @ref scale-factor

        @see set_scale(), zoom_out(), zoom_fit_full(), zoom_fit_width(), get_scale()
    */
    virtual void zoom_in(Pixels x=0, Pixels y=0, bool fForceRedraw=true);


    /** Decrements the scaling factor (near 5%) so that
        a subsequent zoom in operation will cancel the zoom out.
        @param x,y Are the coordinates for the point that will remain fixed (unmoved)
            when applying the new scale (the center point for the zooming operation).
        @param fForceRedraw If @FALSE prevents Lomse from sending a paint event. This is
            useful to avoid repaints when some consecutive operations will affect
            the View.

        See @ref scale-factor

        @see set_scale(), zoom_in(), zoom_fit_full(), zoom_fit_width(), get_scale()
    */
    virtual void zoom_out(Pixels x=0, Pixels y=0, bool fForceRedraw=true);


    /** Adjusts the scaling factor so that current document page will fit on
        the specified rectangle.
        @param width, height Are the dimensions (in @a Pixels) of the rectangle in which
             the page should fit (usually the window size).
        @param fForceRedraw If @FALSE prevents Lomse from sending a paint event. This is
            useful to avoid repaints when some consecutive operations will affect
            the View.

        See @ref scale-factor

        @see set_scale(), zoom_in(), zoom_out(), zoom_fit_width(), get_scale()
    */
    virtual void zoom_fit_full(Pixels width, Pixels height, bool fForceRedraw=true);


    /** Adjusts the scaling factor so that current document page width will fit on
        the specified screen dimension.
        @param width The dimension (in @a Pixels) in which the page width should
            fit (usually the window width).
        @param fForceRedraw If @FALSE prevents Lomse from sending a paint event. This is
            useful to avoid repaints when some consecutive operations will affect
            the View.

        See @ref scale-factor

        @see set_scale(), zoom_in(), zoom_out(), zoom_fit_full(), get_scale()
    */
    virtual void zoom_fit_width(Pixels width, bool fForceRedraw=true);

    //@}    //scale



    //interface to GraphicView. Selection rectangle
    /** @name Interface to GraphicView. Selection rectangle

        @todo Selection rectangle methods and explanation
    */
    ///@{
    virtual void start_selection_rectangle(Pixels x1, Pixels y1);
    virtual void hide_selection_rectangle();
    ///@}    //interface to GraphicView. Selection rectangle



    // Visual effects during playback
    /** @name Interface to GraphicView. Visual tracking effects during playback
    */
    //@{

    /** Select the visual effect to use for visual tracking during playback.
        By default, if this method is not invoked, k_tracking_highlight_notes is used.

        @param mode It is a value from enum EVisualTrackingMode. Several visual effects
        can be en effect simultaneously by combining values
        with the OR ('|') operator. Example:

        @code
        spInteractor->set_visual_tracking_mode(k_tracking_tempo_line | k_tracking_highlight_notes);
        @endcode
    */
	virtual void set_visual_tracking_mode(int mode);

    /** Returns the specified visual tracking effect (derived from VisualEffect).
        @param effect It is a value from enum EVisualTrackingMode. If `k_tracking_none`
			is specified it will return @nullptr.

		Example:
        @code
        VisualEffect* pVE = spInteractor->get_tracking_effect(k_tracking_tempo_line);
		if (pVE)
		{
			TempoLine* pTL = static_cast<TempoLine*>(pVE);
			pTL->set_color(Color(255,0,0,128));     //transparent red
			pTL->set_width(200);		            //logical units: 2 mm
		}
        @endcode
    */
	virtual VisualEffect* get_tracking_effect(int effect);

    /** Move the tempo line to the given time position.
        @param scoreId  Id. of the score to which all other parameters refer.
        @param timepos Time units from the start of the score.
    */
    virtual void move_tempo_line(ImoId scoreId, TimeUnits timepos);

    /** Move the tempo line to the given measure and beat.
        @param scoreId  Id. of the score to which all other parameters refer.
        @param iMeasure Measure number (0..n) in instrument iInstr.
        @param iBeat Beat number (0..m) relative to the measure.
        @param iInstr Number of the instrument (0..m) to which the measures refer to.
            Take into account that for polymetric music (music in which not all
            instruments have the same time signature), the measure number is not an
            absolute value, common to all the score instruments (score parts), but it
            is relative to an instrument. For normal scores, just providing measure
            number and location will do the job.
    */
    virtual void move_tempo_line(ImoId scoreId, int iMeasure, int iBeat, int iInstr=0);

    /** Move the tempo line to the given measure and relative location inside the measure.
        @param scoreId  Id. of the score to which all other parameters refer.
        @param iMeasure Measure number (0..n) in instrument iInstr.
        @param location Time units from the start of the measure.
        @param iInstr Number of the instrument (0..m) to which the measures refer to.
            Take into account that for polymetric music (music in which not all
            instruments have the same time signature), the measure number is not an
            absolute value, common to all the score instruments (score parts), but it
            is relative to an instrument. For normal scores, just providing measure
            number and location will do the job.
    */
    virtual void move_tempo_line(ImoId scoreId, int iMeasure, TimeUnits location,
                                 int iInstr=0);

    /** Move the tempo line to the given measure and beat and change the viewport, if
        necessary, for ensuring that the requested measure/beat is visible.
        @param scoreId  Id. of the score to which all other parameters refer.
        @param iMeasure Measure number (0..n) in instrument iInstr.
        @param iBeat Beat number (0..m) relative to the measure.
        @param iInstr Number of the instrument (0..m) to which the measures refer to.
            Take into account that for polymetric music (music in which not all
            instruments have the same time signature), the measure number is not an
            absolute value, common to all the score instruments (score parts), but it
            is relative to an instrument. For normal scores, just providing measure
            number and location will do the job.
    */
    virtual void move_tempo_line_and_scroll_if_necessary(ImoId scoreId, int iMeasure,
                                                         int iBeat, int iInstr=0);

    /** Move the tempo line to the given measure and relative location inside the measure,
        and change the viewport, if
        necessary, for ensuring that the requested measure/beat is visible.
        @param scoreId  Id. of the score to which all other parameters refer.
        @param iMeasure Measure number (0..n) in instrument iInstr.
        @param location Time units from the start of the measure.
        @param iInstr Number of the instrument (0..m) to which the measures refer to.
            Take into account that for polymetric music (music in which not all
            instruments have the same time signature), the measure number is not an
            absolute value, common to all the score instruments (score parts), but it
            is relative to an instrument. For normal scores, just providing measure
            number and location will do the job.
    */
    virtual void move_tempo_line_and_scroll_if_necessary(ImoId scoreId, int iMeasure,
                                                         TimeUnits location, int iInstr=0);

    /** @param pSO This note or rest will be highlighted
        @todo Document Interactor::highlight_object    */
    virtual void highlight_object(ImoStaffObj* pSO);

    /** @param pSO Highlight will be removed from this note or rest.
        @todo Document Interactor::remove_highlight_from_object    */
    virtual void remove_highlight_from_object(ImoStaffObj* pSO);

    /// Remove all visual tracking visual effects.
    virtual void remove_all_visual_tracking();

    /** @param pEvent The Highlight event to be processed.
        @todo Document Interactor::on_visual_tracking    */
    virtual void on_visual_tracking(SpEventVisualTracking pEvent);

    //@}    //Visual effects during playback



    //interface to GraphicView. Application markings on the score
    /// @name Interface to GraphicView. Application markings on the score
    //@{

    /** Create a new FragmentMark on the score at the given time position for notes and
        rest. If no note/rest exists in the given timepos, the mark will be placed at
        the estimated position at which the note would be placed.
        @param scoreId  Id. of the score on which the mark will be added.
        @param timepos The position for the mark, in Time Units from the start
               of the score.

        The mark will cover all staves of the system and its height will be that of
        the system box. After creation you can use methods FragmentMark::top() and
        FragmentMark::bottom() to define
        the instruments and staves range to cover, as well as to change the extra height
        with method FragmentMark::extra_height().

        By default, the properties of the created mark are as follows:
        - Marker type is <tt>k_mark_line</tt>, that is, a vertical line.
        - Line color is transparent red (Color(255,0,0,128)).
        - Line thickness is six tenths, referred to the first staff of the system at
            which the mark is placed.
        - Line style solid.

        The mark properties (type, color, position, length, etc.) can later be
        changed. See methods: FragmentMark::type(), FragmentMark::color(),
        FragmentMark::top(), FragmentMark::bottom(), FragmentMark::x_shift(),
        FragmentMark:: line_style() and FragmentMark::extra_height().

        <b>Example of use:</b>

        @code
        ImoId scoreId = ...
        TimeUnits timepos = ...
        FragmentMark* mark = pInteractor->add_fragment_mark(scoreId, timepos);

        //customize the mark: magenta solid color, covering second and third instruments
        mark->color(Color(255,0,255))->thickness(5)->top(1)->bottom(2);
            ...

        //change its appearance: rounded open bracket, cyan transparent color
        mark->type(k_mark_open_rounded)->color(Color(0,255,255,128));
            ...

        //when no longer needed remove it
        pInteractor->remove_mark(mark);

        Marks cannot be repositioned. If this is needed, just delete current mark and
        create a new one at the desired new position.

        @endcode
    */
    FragmentMark* add_fragment_mark_at_note_rest(ImoId scoreId, TimeUnits timepos);

    /** Create a new FragmentMark on the score at the barline at the given time position.
        Take into account that barlines have the same timepos than the first
        note/rest after the barline. If there is no a barline at the given timepos, this
        method will place the mark on the note/rest position for the passed timepos.
        @param scoreId  Id. of the score on which the mark will be added.
        @param timepos The position for the mark, in Time Units from the start
               of the score.

        See add_fragment_mark_at_note_rest() for more details.
    */
    FragmentMark* add_fragment_mark_at_barline(ImoId scoreId, TimeUnits timepos);

    /** Create a new FragmentMark on the score at the given staff object position.
        @param pSO Pointer to the staff object defining the position for the mark.

        See add_fragment_mark_at_note_rest() for more details.
    */
    FragmentMark* add_fragment_mark_at_staffobj(ImoStaffObj* pSO);

    /** Hide the mark and delete it.
        @param mark  Pointer to the mark to remove. After executing this method the
            pointer will no longer be valid.
    */
    void remove_mark(ApplicationMark* mark);

    //@}    //Application markings on the score


    //interface to GraphicView. Printing
    /// @name Interface to GraphicView. Print related
    //@{

    /** Sets the memory area to be used as buffer for printing operations.

        In order to not interfere with screen display, a different
        rendering buffer is used for printing.

        See @subpage page-printing
    */
    virtual void set_print_buffer(RenderingBuffer* rbuf);


    /** Sets the resolution (in dots per inch, dpi) to use for printing.

        See @subpage page-printing
    */
    virtual void set_print_ppi(double ppi);


    /** Request Lomse to render a page on current print buffer.
        @param page The page to print (0..num_pages - 1)
        @param viewport The desired viewport. By changing the viewport in a loop of calls
            to this method your application can split the page in tiles, so that printing
            in large paper formats will not require huge buffer sizes.

        See @subpage page-printing
    */
    virtual void print_page(int page, VPoint viewport=VPoint(0, 0));


    /** Returns the number of pages in current document.

        See @subpage page-printing
    */
    virtual int get_num_pages();

    //@}    //interface to GraphicView. Printing



    //cursor / caret
    /// @name Cursor and caret related methods
    //@{

    /** Returns the cursor associated to the View of this %Interactor.

        See @ref page-edit-overview
    */
    inline DocCursor* get_cursor() { return m_pCursor; }

    /** Switch the state of the caret: if it is visible, hide it; if it is hidden, show
        it. Lomse caret does not blink and this method is oriented to implement a
        blinking caret in your application.

        See @ref page-edit-overview

        @remarks
            - Caret is always shown when document edition is enabled.
              and it is hidden when edition is disabled.
            - This method has no effect when document edition is disabled. That is, if
              edition is not enabled, you cannot use this method for forcing to
              display the caret.
            - See @ref set_operating_mode().

        If your application would like a blinking caret, you will have to use a timer
        for the caret and switch the caret state on each timer event. This is an
        example:

        @code
        void MainFrame::on_caret_timer_event()
        {
            Interactor* pInteractor = get_active_canvas_interactor();
            if (pInteractor)
                pInteractor->blink_caret();
        }
        @endcode
    */
    void blink_caret();
//    void show_caret(bool fShow=true);
//    void hide_caret();

    /** Returns a string with the timecode (measure, beat, part) for current position,
        when caret on a music score. If caret is not on a music score, the returned
        string is empty.

        See @ref page-edit-overview

        This method can be useful for displaying the timecode of the caret. For example:

        @code
        void DocumentWindow::update_status_bar_caret_timepos()
        {
            if (is_edition_enabled())
            {
                if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
                {
                    StatusReporter* pStatus = m_appScope.get_status_reporter();
                    pStatus->report_caret_time( spInteractor->get_caret_timecode() );
                }
            }
        }
        @endcode

        @see blink_caret()
    */
    string get_caret_timecode();

    /** Returns a DocCursorState object pointing to the nearest valid position to the
        mouse click point.

        This is a support method for applications wishing to get cursor related
        information from mouse clicks. For instance, to move the caret by pointing with
        the mouse and clicking:

        @code
        void DocumentWindow::move_cursor_to_click_point(SpEventMouse event)
        {
            if (!m_pPresenter)
                return;

            if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
            {
                if (spIntor->get_operating_mode() == Interactor::k_mode_edition)
                {
                    DocCursorState state = spIntor->click_event_to_cursor_state(event);
                    spIntor->exec_command(new CmdCursor(state));
                }
            }
        }
        @endcode
    */
    DocCursorState click_event_to_cursor_state(SpEventMouse event);

    /** Restrict document cursor so that it only moves to positions occupied
        by the selected voice.
        @param voice The new voice to be tracked (0 ... num_voices - 1)

        @remarks
        - As a consequence of this operation the selected voice will be
          rendered highlighted.
        - This method only has effects is the object being edited is a music score. For
          other top level objects this method does nothing.

    */
    void select_voice(int voice);

    //@}    //Cursor and caret



    //dragged image associated to mouse cursor
    /** @name Drag image associated to mouse cursor

        @todo Drag images explanation and methods for dragging images
    */
    //@{
    void show_drag_image(bool value);
    void set_drag_image(GmoShape* pShape, bool fGetOwnership, UPoint offset);
    void enable_drag_image(bool fEnabled);

    //@}    //dragged image associated to mouse cursor



    //edition
    /// @name Document edition
    //@{

    /** Execute an edition command for modifying the document content, the current set
        of selected objects or the cursor position.
        @param pCmd The command to execute.

        @remarks This method has no effect when document edition is disabled.
        See @ref set_operating_mode().

        Example:

        @code
        void CommandGenerator::insert_rest(ENoteType noteType, int dots, int voice, int staff)
        {
	        //insert a rest at current cursor position

            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                stringstream src;
                src << "(r "
                    " " << LdpExporter::notetype_to_string(noteType, dots)
                    << " v" << voice
                    << " p" << staff+1
                    << ")";
                string name = "Insert rest";
                spInteractor->exec_command( new CmdInsertStaffObj(src.str(), name) );
            }
        }
        @endcode

        See @ref edit-overview

        @see exec_undo(), exec_redo()
    */
    void exec_command(DocCommand* pCmd);


    /** Undo the last edition command executed via exec_command().

        @remarks This method has no effect when document edition is disabled.
        See @ref set_operating_mode().

        Example:

        @code
        void DocumentWindow::on_edit_undo()
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
                spInteractor->exec_undo();
        }
        @endcode

        See @ref edit-overview

        @see exec_command(), exec_redo(), should_enable_edit_undo()
    */
    void exec_undo();


    /** Redo the last edition command undone via exec_undo().

        @remarks This method has no effect when document edition is disabled.
        See @ref set_operating_mode().

        See @ref edit-overview

        @see exec_command(), exec_undo(), should_enable_edit_redo(),
    */
    void exec_redo();

    //edition related info
    /** Returns @true if there are commands in the undo queue.

        @remarks This method is oriented for applications wishing to validate
        that exec_undo() can be invoked.

        See @ref edit-overview

        @see exec_command(), exec_undo(), exec_redo(), should_enable_edit_redo(),
    */
    bool should_enable_edit_undo();

    /** Returns @true if there are commands in the redo queue.

        @remarks This method is oriented for applications wishing to validate
        that exec_redo() can be invoked.

        See @ref edit-overview

        @see exec_command(), exec_undo(), exec_redo(), should_enable_edit_undo(),
    */
    bool should_enable_edit_redo();
//    void enable_edition(bool value);
//    inline bool is_edition_enabled() { return m_fEditionEnabled; }

    //@}    //edition

    //event handlers for user actions. Library API
    /// @name User application: to inform Lomse about certain events
    //@{

    /** Inform Lomse that the %Document associated to this %Interactor has been
        modified. This forces Lomse to rebuild all the internal data associated
        to the document (e.g. the graphic model) and to refresh all the views.

        @remarks Normally you don't have to use this method. But in some applications that
        don't use edition commands but modify the %Document by direct manipulation of its
        internal data structures, this method it is needed for informing Lomse and forcing
        to render again all the views associated to the document.
    */
    virtual void on_document_updated();

    /** Inform Lomse that a mouse move event received by your application has to be handled
        by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by
            your application.
        @param flags Flags for keys pressed in the keyboard while moving the mouse,
            as reported by the mouse event received by your application. Values for
            these flags are described by enum #EEventFlag.

        @see
        - @ref page-tasks.
        - switch_task().
    */
    virtual void on_mouse_move(Pixels x, Pixels y, unsigned flags);

    /** Inform Lomse that a mouse button down event received by your application has to
        be handled by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by
            your application.
        @param flags Flags for keys pressed in the keyboard while the mouse button
            was pressed, as reported by the mouse event received by your application.
            Values for these flas are described by enum #EEventFlag.

        @see
        - @ref page-tasks.
        - switch_task().
    */
    virtual void on_mouse_button_down(Pixels x, Pixels y, unsigned flags);

    /** Inform Lomse that a mouse button up event received by your application has to
        be handled by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by
            your application.
        @param flags Flags for keys pressed in the keyboard while the mouse button was
            released, as reported by the mouse event received by your application.
            Values for these flags are described by enum #EEventFlag.

        @see
        - @ref page-tasks.
        - switch_task().
    */
    virtual void on_mouse_button_up(Pixels x, Pixels y, unsigned flags);

    /** Inform Lomse that a mouse enter window event received by your application has to
        be handled by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by
            your application.
        @param flags Flags for keys pressed in the keyboard while the mouse entered the
            window, as reported by the mouse event received by your application. Values
            for these flags are described by enum #EEventFlag.

        @see
        - @ref page-tasks.
        - switch_task().
    */
    virtual void on_mouse_enter_window(Pixels x, Pixels y, unsigned flags);

    /** Inform Lomse that a mouse leave window event received by your application has to
        be handled by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by
            your application.
        @param flags Flags for keys pressed in the keyboard while the mouse moved out of
            the window, as reported by the mouse event received by your application.
            Values for these flags are described by enum #EEventFlag.

        @see
        - @ref page-tasks.
        - switch_task().
    */
    virtual void on_mouse_leave_window(Pixels x, Pixels y, unsigned flags);
    //virtual void on_init();
//    virtual void on_resize(Pixels x, Pixels y);
    //virtual void on_idle();
    //virtual void on_key(Pixels x, Pixels y, unsigned key, unsigned flags);
    //virtual void on_ctrl_change();

    //@}    //event handlers for user actions



    //for performance measurements
    /// @name For performance measurements
    //@{

    /** Invoke this method for informing Lomse when the
        rendering buffer has been copied onto the application window.
        See get_elapsed_times() for more information.

        This method is oriented to performance measurements.
    */
    void timing_repaint_done();

    /** Returns a vector of times, with the elapsed times for the different steps related
        to rendering a document. When a new rendering is necessary, Lomse resets all time
        counters and starts timing the different steps. Your application only has to
        inform Lomse when the rendering buffer has been copied onto the application
        window (do this by invoking method timing_repaint_done()). Then all times are
        available to be accessed by using this method.

        Enum Interactor::ETimingTarget is used as index on the returned vector
        for identifying the operation to which each vector element refers.

        Example:

        @code
        void DocumentWindow::copy_buffer_on_dc(DC& dc)
        {
            if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            {
                // draw lomse buffer onto DC
                ...

                //inform lomse that the buffer has been copied
                spInteractor->timing_repaint_done();

                //Display ellapsed times for the different renderization steps
                double* pTimes = spInteractor->get_elapsed_times();
                std::ostringstream msg;
                msg << std::fixed << std::setprecision(2)
                    << "Build graphic model = "
                    << *(pTimes + Interactor::k_timing_gmodel_build_time) << "msecs." << endl
                    << "Render gaphic model = "
                    << *(pTimes + Interactor::k_timing_gmodel_draw_time) << "msecs." << endl
                    << "Render visual effects = "
                    << *(pTimes + Interactor::k_timing_visual_effects_draw_time) << "msecs." << endl
                    << "Total Lomse render time = "
                    << *(pTimes + Interactor::k_timing_total_render_time) << "msecs." << endl
                    << "Paint buffer on DC = "
                    << *(pTimes + Interactor::k_timing_repaint_time) << "msecs." << endl;

                ...
            }
        }

        @endcode
    */
    inline double* get_elapsed_times() { return &m_elapsedTimes[0]; }

    //@}    //for performance measurements



    //Debugging
    /** @name Debugging
        These methods are oriented to debug Lomse and should not be in the public API.
        If you use them be aware that they might be removed from public API in future.
    */
    //@{

    /** Instructs Lomse renderer to draw a rectangle around the area occupied by elements
        of type @a boxType.

        @param boxType The elements for which it is requested to draw a surrounding
            rectangle. Valid values are given by enum constants
            <i>GmoObj::k_box_xxxxxxx</i> defined in GmoObj class.

        See: reset_boxes_to_draw().

        @attention This method is oriented to debug Lomse. It might be removed from
            public API in future.
    */
    virtual void set_box_to_draw(int boxType);

    /** Instructs Lomse renderer to not render surrounding rectangles around any element.

        See: set_box_to_draw().

        @attention This method is oriented to debug Lomse. It might be removed from
            public API in future.
    */

    virtual void reset_boxes_to_draw();

    /** Returns an string with the content of current <i>Cursor</i>.

        @attention This method is oriented to debug Lomse. It might be removed from
            public API in future.
    */
    string dump_cursor();

    /** Returns an string with the content of current <i>Selection Set</i>.

        @attention This method is oriented to debug Lomse. It might be removed from
            public API in future.
    */
    string dump_selection();

    //@}    //Debugging


	///@cond INTERNALS
	//excluded from public API. Only for internal use.
    Interactor(LibraryScope& libraryScope, WpDocument wpDoc, View* pView,
               DocCommandExecuter* pExec);
    virtual ~Interactor();

    inline std::shared_ptr<Interactor> get_shared_ptr_from_this() { return shared_from_this(); }

    //mandatory override required by EventHandler
	void handle_event(SpEventInfo pEvent);

    //Deprecated ?
    virtual void highlight_voice(int voice);

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return this; }

    //for auto-scroll during playback
    virtual void change_viewport_if_necessary(ImoId id);
    virtual void request_viewport_change(Pixels x, Pixels y);

    //for performance measurements
    void timing_start_measurements();
    void timing_graphic_model_build_end();
    void timing_graphic_model_render_end();
    void timing_visual_effects_start();
    void timing_renderization_end();

    //interface to SelectionSet
    virtual void select_object(GmoObj* pGmo, bool fClearSelection=true);
    virtual void select_object(ImoId id, bool fClearSelection=true);
    virtual bool is_in_selection(GmoObj* pGmo);

    //actions requested by Task objects
    virtual void task_action_click_at_screen_point(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_select_objects_in_screen_rectangle(Pixels x1, Pixels y1,
                                                                Pixels x2, Pixels y2,
                                                                unsigned flags);
    virtual void task_action_select_object_and_show_contextual_menu(Pixels x, Pixels y,
                                                                    unsigned flags);
    virtual void task_action_mouse_in_out(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_insert_object_at_point(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_drag_the_view(Pixels x, Pixels y);
    virtual void task_action_decide_on_switching_task(Pixels x, Pixels y, unsigned flags);
    virtual void task_action_switch_to_default_task();
    virtual void task_action_move_drag_image(Pixels x, Pixels y);
    virtual void task_action_move_object(Pixels x, Pixels y);
    virtual void task_action_move_handler(Pixels x, Pixels y);
    virtual void task_action_move_handler_end_point(Pixels xFinal, Pixels yFinal,
                                                    Pixels xTotalShift, Pixels yTotalShift);
    virtual void task_action_update_selection_rectangle(Pixels x2, Pixels y2);

//    virtual void task_action_single_click_at(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_double_click_at(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_start_move_drag(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_continue_move_drag(Pixels x, Pixels y, bool fLeftButton);
//    virtual void task_action_end_move_drag(Pixels x, Pixels y, bool fLeftButton,
//                                           Pixels xTotalShift, Pixels yTotalShift);

	///@endcond


protected:

    //for performance measurements
    double m_elapsedTimes[k_timing_max_value];
    ptime m_renderStartTime;
    ptime m_visualEffectsStartTime;
    ptime m_repaintStartTime;
    ptime m_gmodelBuildStartTime;

    void create_graphic_model();
    void delete_graphic_model();
    void request_window_update();
    VRect get_damaged_rectangle();
    GmoObj* find_object_at(Pixels x, Pixels y);
    GmoBox* find_box_at(Pixels x, Pixels y);
    void send_mouse_out_event(GmoRef gref, Pixels x, Pixels y);
    void send_mouse_in_event(GmoRef gref, Pixels x, Pixels y);
    void send_click_event(GmoObj* pGmo, Pixels x, Pixels y, unsigned flags);
    void notify_event(SpEventInfo pEvent, GmoObj* pGmo);
    void update_view_if_gmodel_modified();
    void update_view_if_needed();
    void find_parent_link_box_and_notify_event(SpEventInfo pEvent, GmoObj* pGmo);
    void do_force_redraw();
    ImoObj* find_event_originator_imo(GmoObj* pGmo);
    GmoRef find_event_originator_gref(GmoObj* pGmo);
    bool discard_visual_tracking_event_if_not_valid(ImoId scoreId);
    bool is_valid_play_score_event(SpEventPlayCtrl pEvent);
    void update_caret_and_view();
    void redraw_caret();
    void send_update_UI_event(EEventType type);
    double get_elapsed_time_since(ptime startTime) const;
    Handler* handlers_hit_test(Pixels x, Pixels y);
    void restore_selection();
    bool is_operating_mode_allowed(int mode);

    //To select mode for do_move_tempo_line_and_scroll()
    enum { k_only_tempo_line=0, k_scroll_and_tempo_line, k_only_scroll };
    void do_move_tempo_line_and_scroll(ImoId scoreId, const MeasureLocator& ml,int mode);
    void do_move_tempo_line_and_scroll(ImoId scoreId, int iMeasure, int iBeat,
                                       int iInstr, int mode);

};

typedef std::shared_ptr<Interactor>   SpInteractor;
typedef std::weak_ptr<Interactor>     WpInteractor;

////---------------------------------------------------------------------------------------
////A view to edit the document in full page
//class EditInteractor : public Interactor
//{
//protected:
//
//public:
//
//    EditInteractor(LibraryScope& libraryScope, Document* pDoc, View* pView,
//                   DocCommandExecuter* pExec);
//    virtual ~EditInteractor();
//
//};



}   //namespace lomse

#endif      //__LOMSE_INTERACTOR_H__
