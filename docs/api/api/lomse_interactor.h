typedef SharedPtr<Document>     SpDocument;
typedef WeakPtr<Document>       WpDocument;
typedef SharedPtr<GmoShape>  SpGmoShape;

//some constants for improving code legibillity
#define k_no_redraw     false   //do not force view redraw
#define k_force_redraw  true    //force a view redraw

#define k_get_ownership         true
#define k_do_not_get_ownership  false

//---------------------------------------------------------------------------------------
// Event flags for mouse and keyboard pressed keys
/**
@ingroup enumerations

This enum describes the flags for mouse events, indicating which mouse buttons
and keyboard keys are pressed when the mouse event takes place.

\#include <lomse_interactor.h>

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
//Abstract class from which all Interactors must derive
/**
The %Interactor is the key object to interact with the document (here the name '%Interactor'). It
is the interface between your application, the associated View and the Document (see @ref mvc-overview). It is responsible for translating your application requests into commands that manipulate the associated View and/or the Document, coordinating all the necessary actions. It also provides support for managing the user interaction with your application GUI (see @ref tasks-page).

The %Interactor plays the role of the Controller in the MVC model. Each View has an associated %Interactor (in fact the View is owned by the %Interactor). The %Interactor is owned by the Presenter.

The %Interactor for a %View is provided by the Presenter. It is best practise not to save pointers to the %Interactor because when processing a Lomse event the Document (and thus, the Interactor) could have been deleted (i.e. because your application has closed the window displaying the document).

Lomse provides type @b SpInteractor, an smart pointer to the %Interactor. The recomendation is to use always smart pointers when provided by Lomse instea of using raw pointers. The use of threads, document edition commands, and events processing can invalidate stored pointers in your application. So it is always recommended to get the pointer to the %Interactor when needed, by accessing the smart pointer and checking that it is still valid:

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
- @ref tasks-page.

*/
class Interactor : public EventHandler
                 , public EventNotifier
                 , public Observable
                 , public EnableSharedFromThis<Interactor>
{
public:

    //enums
    /**
        Valid operating modes for the %Interactor:
        - @b k_mode_read_only  - Read only mode: no changes allowed in the document.
        - @b k_mode_edition  - Edition mode: changes allowed in the document.
        - @b k_mode_playback  - Playback mode: disabled any action that could affect playback in any way. Edition is disabled in this mode.
    */
    enum EInteractorOpMode { k_mode_read_only=0, k_mode_edition, k_mode_playback, };


    /**
        Method Interactor#get_elapsed_times() returns a vector of elapsed times. This enum is used as index on that vector
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


    
        //operating modes
        /**
            @name Modes of operation and related
        */
        //@{

    /**
        Set current operation mode. An operation mode defines valid actions
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

                SpEventPlayScore pEv = boost::static_pointer_cast<EventPlayScore>(pEvent);
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


    /**
        Returns current operation mode for this %Interactor.
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


    /**
        When in edition mode, invoking this method disables edition in all the document with the exception of the
        object whose ID is passed. This method allows your application to restrict edition to a certain area
        of the document, such as an score, a paragraph, etc.
        @param id The ID of the only object that will be editable after invoking this method.
    */
    void enable_edition_restricted_to(ImoId id);


    /**
        Returns @TRUE if the document is editable. 

        @note Currently, LenMus documents are always editable. Therefore this method will always return @TRUE.
        But by invoking this method your application can be ready for disabling edition for future 
        read-only documents.
    */
    bool is_document_editable();


    /**
        Switch the %Task used by the %Interactor for interpreting mouse events.

        @remarks When the %Interactor is created it is initialized with an instace of TaskSelection class.
        
        See @ref tasks-page.

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

        //@}    //operating modes


        //access to collaborators
        /**
            @name Access to collaborators in MVC model
        */
        //@{

    /**
        Returns the graphic model object associated to the View of this %Interactor.
    */
    GraphicModel* get_graphic_model();


    /**
        Returns the View associated to this %Interactor.
    */
    inline View* get_view() { return m_pView; }


    /**
        Returns the selection set associated to the View of this %Interactor.

        See @ref document-edition-overview
    */
    inline SelectionSet* get_selection_set() { return m_pSelections; }

        //@}    //access to collaborators



        //interface to View
        /**
            @name Interface to View
        */
        //@{

    /**
        Invoking this method forces Lomse to render again the View and, therefore, 
        the rendering buffer gets updated. After doing it, Lomse <b>does not</b>
        generate a EventPaint event, as invoking this method implies that your
        application is aware of the rendering buffer change.

        @see force_redraw().
    */
    virtual void redraw_bitmap();


    /**
        Invoking this method forces Lomse to render again the View and, therefore, 
        the rendering buffer gets updated. After doing it, Lomse <b>will</b>
        generate a EventPaint event.

        @note Lomse will ignore the request if forced updates are 
        disabled. See enable_forced_view_updates().

        @todo Clarify the need of methods force_redraw(), redraw_bitmap() and 
        enable_forced_view_updates(). Probably This code has to be refactored.

        @see redraw_bitmap().
    */
    virtual void force_redraw();


    /**
        Returns @TRUE if the document, the graphic model, or any View option have 
        been changed since last view renderization.

        Your application rarely will need to
        invoke this method as Lomse automatically triggers updates when anything is
        changed.
    */
    bool view_needs_repaint();


    //creating events
    /**
        @todo This method should be for internal use. Analyse and refactor or document.
    */
    virtual void send_end_of_play_event(ImoScore* pScore, PlayerGui* pPlayCtrl);


    /**
        Invoking this method controls the behavior of method force_redraw(). If disabled, Lomse
        will ignore any invocation to force_redraw().

        @param value @TRUE for enabling method force_redraw(). @FALSE for ignoring invocations
        to this method.

        By default, force_redraw() method is enabled.
    */
    inline void enable_forced_view_updates(bool value) { m_fViewUpdatesEnabled = value; }

        //@}    //interface to View



        //interface to GraphicView. Renderization
        /**
            @name Interface to GraphicView. Rederization
        */
        //@{


    /**
        Associate a rendering buffer to the View related to this %Interactor.
        @param rbuf A ptr to the memory to be used as rendering buffer.

        Invoking this method is mandatory before doing any operation that would require to render
        the view. Normally this method is invoked when creating a new view. Example:

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


    /**
        Set/reset a rendering option for the view associated to this %Interactor.

        @param option The option to set or reset. Valid values for this param are
            defined by enum #ERenderOptions
        @param value @TRUE for enabling the option or @FALSE for disabling it.

        @todo Document enum ERenderOptions

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

        //@}    //interface to GraphicView. Rederization



        //interface to GraphicView. Units conversion and related
        /**
            @name Interface to GraphicView. Coordinates conversion
        */
        //@{

    /**
        Converts device coordinates (pixels) to coordinates in logical
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


    /**
        Converts logical coordinates relative to the start of a page to absolute
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


    /**
        Returns the page number (0 .. num_pages - 1) that contains the
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


    /**
        Converts absolute device coordinates (pixels, relative to view origin)
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


    /**
        Returns the pitch (@a DiatonicPitch type) of the staff point pointed by coordinates
        @a x, @a y or value @a k_no_pitch if not pointing to a staff.

        This helper method is useful in some scenarios. For instance, if in your application
        the user is allowed to insert notes on a staff by clicking with the mouse on the 
        insertion point, you will need to determine the nearest staff line/space, and the applicable
        clef, in order to determine the pitch for the note to insert. This method performs all these
        operations and returns the diatonic pitch for the staff point. 

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
        /**
            @name Interface to GraphicView. Viewport (for scrolling)
        */
        //@{

    /**
        Sets a new origin for the viewport.
        @param x,y new coordinates (in pixels) for the viewport.
        @param fForceRedraw If @false prevents redrawing the new viewport
            into the rendering buffer; this is useful for saving time when several consecutive
            changes are going to be done. If not specified @true is assumed.

        @remarks The size of viewport (height, width) is always determined by rendering buffer size.

        See @ref viewport-concept

        @see set_viewport_at_page_center(), get_viewport(), get_view_size()
    */
    virtual void new_viewport(Pixels x, Pixels y, bool fForceRedraw=true);
    /**
        Sets a new @a x origin for the viewport so that the viewport is centered on document page.
        @param screenWidth is the desired viewport width (in pixels) for computing the the new @a x origin.

        @remarks This method computes the new origin as (pageWidth - screenWidth) / 2;

        See @ref viewport-concept

        @see new_viewport(), get_viewport(), get_view_size()
    */
    virtual void set_viewport_at_page_center(Pixels screenWidth);
    /**
        Returns the current coordinates (pixels) of viewport origin.
        @param x,y The variables in which the viewport origin will be returned.

        See @ref viewport-concept

        @see new_viewport(), set_viewport_at_page_center(), get_view_size()
    */
    virtual void get_viewport(Pixels* x, Pixels* y);
    /**
        Returns the total size (pixels) of the whole rendered document (the whole visual space, all pages).
        @param xWidth
            Receives the width of the whole rendered document, in pixels.
        @param yHeight
            Receives the height of the whole rendered document, in pixels.

        @see new_viewport(), set_viewport_at_page_center(), get_viewport()
    */
    virtual void get_view_size(Pixels* xWidth, Pixels* yHeight);

        //@}    //interface to GraphicView. Viewport / scroll



        //interface to GraphicView. Scale
        /**
            @name Interface to GraphicView. Scale
        */
        //@{

    /**
        Returns the current scale factor.

        See @ref scale-factor
    */
    virtual double get_scale();


    /**
        Sets the scaling factor, useful for applications which require 'zooming'.
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


    /**
        Increments the scaling factor by 5%.
        @param x,y Are the coordinates for the point that will remain fixed (unmoved)
            when applying the new scale (the center point for the zooming operation).
        @param fForceRedraw If @FALSE prevents Lomse from sending a paint event. This is
            useful to avoid repaints when some consecutive operations will affect
            the View. 

        See @ref scale-factor

        @see set_scale(), zoom_out(), zoom_fit_full(), zoom_fit_width(), get_scale()
    */
    virtual void zoom_in(Pixels x=0, Pixels y=0, bool fForceRedraw=true);


    /**
        Decrements the scaling factor (near 5%) so that 
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


    /**
        Adjusts the scaling factor so that current document page will fit on  
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


    /**
        Adjusts the scaling factor so that current document page width will fit on  
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
        /**
            @name Interface to GraphicView. Selection rectangle

            @todo Selection rectangle methods and explanation
        */
        //@{
    virtual void start_selection_rectangle(Pixels x1, Pixels y1);
    virtual void hide_selection_rectangle();

        //@}    //interface to GraphicView. Selection rectangle



        //tempo line
        /**
            @name Interface to GraphicView. Playback effects: tempo line

            @todo Tempo line methods and Playback effects explanation
        */
        //@{
    virtual void show_tempo_line(Pixels x1, Pixels y1, Pixels x2, Pixels y2);
    virtual void hide_tempo_line();
    virtual void update_tempo_line(Pixels x2, Pixels y2);

        //@}    //tempo line



        //highlight
        /**
            @name Interface to GraphicView. Playback effects: highlight played notes/rests

            @todo Highlight methods and Playback effects explanation
        */
        //@{
    virtual void highlight_object(ImoStaffObj* pSO);
    virtual void remove_highlight_from_object(ImoStaffObj* pSO);
    virtual void remove_all_highlight();
    virtual void discard_all_highlight();
    virtual void on_visual_highlight(SpEventScoreHighlight pEvent);

        //@}    //highlight



        //interface to GraphicView. Printing
        /**
            @name Interface to GraphicView. Print related
        */
        //@{

    /**
        Sets the memory area to be used as buffer for printing operations.

        In order to not interfere with screen display, a different
        rendering buffer is used for printing.

        See @subpage print-api
    */
    virtual void set_print_buffer(RenderingBuffer* rbuf);


    /**
        Sets the resolution (in dots per inch, dpi) to use for printing.

        See @subpage print-api
    */
    virtual void set_print_ppi(double ppi);


    /**
        Request Lomse to render a page on current print buffer.
        @param page The page to print (0..num_pages - 1)
        @param viewport The desired viewport. By changing the viewport in a loop of calls to this method your application
            can split the page in tiles, so that printing in large paper formats will not require huge buffer sizes.

        See @subpage print-api
    */
    virtual void print_page(int page, VPoint viewport=VPoint(0, 0));


    /**
        Returns the number of pages in current document.

        See @subpage print-api
    */
    virtual int get_num_pages();

        //@}    //interface to GraphicView. Printing



        //cursor / caret
        /**
            @name Cursor and caret related methods
        */
        //@{

    /**
        Returns the cursor associated to the View of this %Interactor.

        See @ref document-edition-overview
    */
    inline DocCursor* get_cursor() { return m_pCursor; }

    /**
        Switch the state of the caret: if it is visible, hide it; if it is hidden, show it.
        Lomse caret does not blink and this method is oriented to implement a blinking
        caret in your application.

        See @ref document-edition-overview

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

    /**
        Returns a string with the timecode (measure, beat, part) for current position, when caret on
        a music score. If caret is not on a music score, the returned string is empty.

        See @ref document-edition-overview

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

    /**
        Returns a DocCursorState object pointing to the nearest valid position to the mouse click point.

        This is a support method for applications wishing to get cursor related information
        from mouse clicks. For instance, to move the caret by pointing with the mouse and clicking:

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

    /**
        Restrict document cursor so that it only moves to positions occupied
        by the selected voice.
        @param voice The new voice to be tracked (0 ... num_voices - 1)

        @remarks
        - As a consequence of this operation the selected voice will be
          rendered highlighted.
        - This method only has effects is the object beign edited is a music score. For
          other top level objects this method does nothing.

    */
    void select_voice(int voice);

        //@}    //Cursor and caret



        //dragged image associated to mouse cursor
        /**
            @name Drag image associated to mouse cursor

            @todo Drag images explanation and methods for dragging images
        */
        //@{
    void show_drag_image(bool value);
    void set_drag_image(GmoShape* pShape, bool fGetOwnership, UPoint offset);
    void enable_drag_image(bool fEnabled);

        //@}    //dragged image associated to mouse cursor



        //edition
        /**
            @name Document edition
        */
        //@{
    /**
        Execute an edition command for modifying the document content, the current set 
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

    /**
        Undo the last edition command executed via exec_command().

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

    /**
        Redo the last edition command undone via exec_undo().

        @remarks This method has no effect when document edition is disabled.
        See @ref set_operating_mode().

        See @ref edit-overview

        @see exec_command(), exec_undo(), should_enable_edit_redo(),
    */
    void exec_redo();

    //edition related info
    /**
        Returns @true if there are commands in the undo queue.

        @remarks This method is oriented for applications wishing to validate
        that exec_undo() can be invoked.

        See @ref edit-overview

        @see exec_command(), exec_undo(), exec_redo(), should_enable_edit_redo(),
    */
    bool should_enable_edit_undo();

    /**
        Returns @true if there are commands in the redo queue.

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
        /**
            @name User application: to inform Lomse about certain events
        */
        //@{

    /**
        Inform Lomse that the %Document associated to this %Interactor has been
        modified. This forces Lomse to rebuild all the internal data associated 
        to the document (e.g. the graphic model) and to refresh all the views. 

        @remarks Normally you don't have to use this method. But in some applications that
        don't use edition commands but modify the %Document by direct manipulation of its 
        internal data structures, this method it is needed for informing Lomse and forcing
        to render again all the views associated to the document.
    */
    virtual void on_document_updated();

    /**
        Inform Lomse that a mouse move event received by your application has to be handled
        by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by your application.
        @param flags Flags for keys pressed in the keyboard while moving the mouse, as reported
        by the mouse event received by your application. Values for these flas are described by
        enum #EEventFlag.

        @see
        - @ref tasks-page.
        - switch_task().
    */
    virtual void on_mouse_move(Pixels x, Pixels y, unsigned flags);

    /**
        Inform Lomse that a mouse button down event received by your application has to be handled
        by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by your application.
        @param flags Flags for keys pressed in the keyboard while the mouse button was pressed, as reported
        by the mouse event received by your application. Values for these flas are described by
        enum #EEventFlag.

        @see
        - @ref tasks-page.
        - switch_task().
    */
    virtual void on_mouse_button_down(Pixels x, Pixels y, unsigned flags);

    /**
        Inform Lomse that a mouse button up event received by your application has to be handled
        by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by your application.
        @param flags Flags for keys pressed in the keyboard while the mouse button was released, as reported
        by the mouse event received by your application. Values for these flas are described by
        enum #EEventFlag.

        @see
        - @ref tasks-page.
        - switch_task().
    */
    virtual void on_mouse_button_up(Pixels x, Pixels y, unsigned flags);

    /**
        Inform Lomse that a mouse enter window event received by your application has to be handled
        by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by your application.
        @param flags Flags for keys pressed in the keyboard while the mouse entered the window, as reported
        by the mouse event received by your application. Values for these flas are described by
        enum #EEventFlag.

        @see
        - @ref tasks-page.
        - switch_task().
    */
    virtual void on_mouse_enter_window(Pixels x, Pixels y, unsigned flags);

    /**
        Inform Lomse that a mouse leave window event received by your application has to be handled
        by Lomse in accordance to current selected Task.
        @param x,y Current mouse position, as reported by the mouse event received by your application.
        @param flags Flags for keys pressed in the keyboard while the mouse moved out of the window, as reported
        by the mouse event received by your application. Values for these flas are described by
        enum #EEventFlag.

        @see
        - @ref tasks-page.
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
        /**
            @name For performance measurements
        */
        //@{
    /**
        Invoke this method for informing Lomse when the 
        rendering buffer has been copied onto the application window.
        See get_elapsed_times() for more information.

        This method is oriented to performance measurements.
    */
    void timing_repaint_done();
    /**
        Rerturns a vector of times, with the elapsed times for the different steps related to
        rendering a document. When a new renderization is necessary, Lomse resets all time counters
        and starts timing the different steps. Your application only has to inform lomse when the 
        rendering buffer has been copied onto the application window (do this by invioking
        method timing_repaint_done()). Then all times are available to be accessed by using
        this method.

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
        /**
            @name Debugging
            These methods are oriented to debug Lomse and should not be in the public API.
            If you use them be aware that they might be removed from public API in future.
        */
        //@{

    /**
        Instructs Lomse renderer to draw a rectangle around the area occupied by elements
        of type @a boxType.

        @param boxType The elements for which it is requested to draw a surrounding rectangle. Valid
            values are given by enum constants <i>GmoObj::k_box_xxxxxxx</i> defined in GmoObj class.

        See: reset_boxes_to_draw().

        @attention This method is oriented to debug Lomse. It might be removed from public API in future.
    */
    virtual void set_box_to_draw(int boxType);


    /**
        Instructs Lomse renderer to not render surrounding rectangles around any element.

        See: set_box_to_draw().

        @attention This method is oriented to debug Lomse. It might be removed from public API in future.
    */
    virtual void reset_boxes_to_draw();

    /**
        Returns an string with the content of current <i>Cursor</i>.

        @attention This method is oriented to debug Lomse. It might be removed from public API in future.
    */
    string dump_cursor();
    /**
        Returns an string with the content of current <i>Selection Set</i>.

        @attention This method is oriented to debug Lomse. It might be removed from public API in future.
    */
    string dump_selection();

        //@}    //Debugging


//excluded from public API. Only for internal use.
#ifdef LOMSE_INTERNAL_API
    Interactor(LibraryScope& libraryScope, WpDocument wpDoc, View* pView,
               DocCommandExecuter* pExec);
    virtual ~Interactor();

    inline SharedPtr<Interactor> get_shared_ptr_from_this() { return shared_from_this(); }

    //mandatory override required by EventHandler
	void handle_event(SpEventInfo pEvent);

    //Deprecated ?
    virtual void highlight_voice(int voice);

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return this; }

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


#endif  //excluded from public API
};

