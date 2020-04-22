//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#ifndef __LOMSE_EVENTS_H__
#define __LOMSE_EVENTS_H__

#include "lomse_build_options.h"
#include "lomse_basic.h"

#include <list>
#include <string>
using namespace std;

///@cond INTERNAL
namespace lomse
{
///@endcond

//forward declarations
class DocCursor;
class ImoObj;
class ImoContentObj;
class ImoDynamic;
class ImoDocument;
class ImoScore;
class ImoStaffObj;
class PlayerGui;
class SelectionSet;
class GmoObj;

class Interactor;
typedef std::weak_ptr<Interactor>     WpInteractor;

class Document;
typedef std::weak_ptr<Document>       WpDocument;


//observer pattern
class EventNotifier;
class Observer;
class Observable;
class EventHandler;
class EventCallback;
class EventsDispatcher;


//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum assigns an event type for Lomse events.

    @#include <lomse_events.h>
*/
enum EEventType
{
    k_null_event = -1,

    //EventDoc
        k_doc_modified_event,       ///< Informs that the Document has been modified

    //EventPaint
        k_update_window_event,      ///< Request to update window with current bitmap

    //EventAction
        //EventMouse
        k_mouse_in_event,               ///< mouse enters in an object
        k_mouse_out_event,              ///< mouse goes out from an object
        k_on_click_event,               ///< click on object
        k_link_clicked_event,           ///< left mouse click on a link (ImoLink object)
        k_show_contextual_menu_event,   ///< right click on object: contextual menu request
        //EventControlPointMoved
        k_control_point_moved_event,    ///< user moves a handler: handler released event
        //EventUpdateUI             ///< possible need for UI updates
        k_selection_set_change,         ///< selected objects changed
        k_pointed_object_change,        ///< cursor pointing to a different object
        //EventPlayCtrl
        k_do_play_score_event,          ///< Request to start/resume playback
        k_pause_score_event,            ///< Request to pause playback
        k_stop_playback_event,          ///< Request to stop playback

    //EventPlayback
        //EventVisualTracking
        k_tracking_event,              ///< Wrapper event containing a list of real events
        //EventUpdateViewport
        k_update_viewport_event,        ///< Suggest to change viewport.
        //EventEndOfPlayback
        k_end_of_playback_event,        ///< Playback ended.


};

//---------------------------------------------------------------------------------------
/** Abstract class holding information about an event. All events derive from it.
*/
class EventInfo
{
protected:
    EEventType m_type;

    EventInfo(EEventType type) : m_type(type) {}

public:
    /// Destructor
    virtual ~EventInfo() {}

    /** Returns a pointer to the Observable object (an ImoContentObj) related to the
        event. It is either the mouse pointed object or the first ancestor of type
        ImoContentObj in the internal model hierarchy. If the event is not related
        to an Observable object it returns @nullptr.
    */
    virtual Observable* get_source() { return nullptr; }

    /// Returns the event type. It is a value from enmun #EEventType.
    inline EEventType get_event_type() { return m_type; }

    /// @name Helpers for checking event type
    //@{
    inline bool is_doc_modified_event() { return m_type == k_doc_modified_event; }
    inline bool is_update_window_event() { return m_type == k_update_window_event; }
    inline bool is_mouse_in_event() { return m_type == k_mouse_in_event; }
    inline bool is_mouse_out_event() { return m_type == k_mouse_out_event; }
    inline bool is_on_click_event() { return m_type == k_on_click_event; }
    inline bool is_link_clicked_event() { return m_type == k_link_clicked_event; }
    inline bool is_show_contextual_menu_event() { return m_type == k_show_contextual_menu_event; }
    inline bool is_control_point_moved_event() { return m_type == k_control_point_moved_event; }
    inline bool is_selection_set_change() { return m_type == k_selection_set_change; }
    inline bool is_pointed_object_change() { return m_type == k_pointed_object_change; }
    inline bool is_do_play_score_event() { return m_type == k_do_play_score_event; }
    inline bool is_pause_score_event() { return m_type == k_pause_score_event; }
    inline bool is_stop_playback_event() { return m_type == k_stop_playback_event; }
    inline bool is_tracking_event() { return m_type == k_tracking_event; }
    inline bool is_update_viewport_event() { return m_type == k_update_viewport_event; }
    inline bool is_end_of_playback_event() { return m_type == k_end_of_playback_event; }
    //@}

protected:
    friend class Interactor;
    inline void set_type(EEventType type) { m_type = type; }
};

/** A shared pointer for an EventInfo.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventInfo>  SpEventInfo;


//---------------------------------------------------------------------------------------
/** An event generated by the Document, not related to any specific View.

    The only type for this event is <b>k_doc_modified_event</b> that is generated when
    the Document is modified. This event is observed by the Interactor in order to
    generate EvenPaint events when necessary. Therefore, your application normally
    will not have to handle this event.

    For handling it in your application, you will have to register a handler function
    at the Document:
    @code
    pDoc->add_event_handler(k_doc_modified_event, MyHandler);
    @endcode

    You can always check if a Document has been modified by invoking method
    Document::is_dirty().

    See @ref handling-events
*/
class EventDoc : public EventInfo
{
protected:
   Document* m_pDoc;

public:
    /// Constructor
    EventDoc(EEventType type, Document* pDoc)
        : EventInfo(type)
        , m_pDoc(pDoc)
    {
    }
    /// Destructor
    virtual ~EventDoc() {}

    ///Returns a ptr. to the Document object in which the event is generated.
    inline Document* get_document() { return m_pDoc; }
};

/** A shared pointer for an EventDoc.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventDoc>  SpEventDoc;


//---------------------------------------------------------------------------------------
// EventPaint
/** An event to inform user application about the need to repaint the screen.

	The View content (the buffer bitmap) has been updated by Lomse and the user
	application is not aware of this update. Therefore, Lomse sends this event to
	inform the user application that it must immediately update the content of the
	window associated to the lomse View, by displaying the current bitmap. User
	application must put immediately the content of the currently rendered buffer
	into the window without calling any Lomse methods (i.e. force_redraw) or generating
	application events. .

	For receiving these events you will have to register a callback when
	the View is created. For processing these events take into account that this event
	is decoupled by design: user must do repaint immediately, without more delays; your
	application should just blit the new bitmap onto the window.

    <b>Example</b>

	For instance, in an application written using the wxWidgets framework you
	could do something as:

	@code
	class DocumentWindow : public wxWindow
	{
	public:
		...
		//callback wrappers
		static void wrapper_update_window(void* pThis, SpEventInfo pEvent);
		...
	};

	void DocumentWindow::display_document(const string& filename, ostringstream& reporter)
	{
		...
		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    //connect the View with the window buffer
		    spInteractor->set_rendering_buffer(&m_rbuf_window);

		    //register to receive desired events
		    spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
		...
	}

	void DocumentWindow::wrapper_update_window(void* pThis, SpEventInfo pEvent)
	{
		if (pEvent->get_event_type() == k_update_window_event)
		{
		    SpEventPaint pEv( static_pointer_cast<EventPaint>(pEvent) );
		    static_cast<DocumentWindow*>(pThis)->update_window(pEv->get_damaged_rectangle());
		}
	}

	void DocumentWindow::update_window(VRect damagedRect)
	{
		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    if (!m_buffer || !m_buffer->IsOk() || m_buffer->GetSize() != GetClientSize() )
		        return;

			wxClientDC dc(this);
		    wxBitmap bitmap(*m_buffer);
		    dc.DrawBitmap(bitmap, 0, 0, false);
		    SetFocus();
		}
	}
	@endcode
*/
class EventPaint : public EventInfo
{
protected:
    WpInteractor m_wpInteractor;
    VRect m_damagedRectangle;

    EventPaint(EEventType type, WpInteractor wpInteractor, VRect damagedRectangle)
        : EventInfo(type)
        , m_wpInteractor(wpInteractor)
        , m_damagedRectangle(damagedRectangle)
    {
    }

public:
    /// Constructor
    EventPaint(WpInteractor wpInteractor, VRect damagedRectangle)
        : EventInfo(k_update_window_event)
        , m_wpInteractor(wpInteractor)
        , m_damagedRectangle(damagedRectangle)
    {
    }
    /// Destructor
    virtual ~EventPaint() {}

    /** Returns a weak pointer to the Interactor object managing the
        View in which the event is generated. */
    inline WpInteractor get_interactor() { return m_wpInteractor; }

    /** Returns the damaged rectangle, that is, the rectangle that needs repaint. */
    inline VRect get_damaged_rectangle() { return m_damagedRectangle; }
};

/** A shared pointer for an EventPaint.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventPaint>  SpEventPaint;


//---------------------------------------------------------------------------------------
/** Base class for all events representing a user mouse action that
    has been interpreted by Lomse as a command to change the Document or the GUI.
*/
class EventAction : public EventInfo
{
protected:
    WpInteractor m_wpInteractor;
    WpDocument m_wpDoc;

    /// Constructor
    EventAction(EEventType type, WpInteractor wpInteractor, WpDocument wpDoc)
        : EventInfo(type)
        , m_wpInteractor(wpInteractor)
        , m_wpDoc(wpDoc)
    {
    }

public:
    /// Destructor
    virtual ~EventAction() {}

    /** Returns a weak pointer to the Interactor object managing the
        View in which the event is generated. */
    inline WpInteractor get_interactor() { return m_wpInteractor; }

    /** Returns a weak pointer to the Document object related to this event. */
    inline WpDocument get_document() { return m_wpDoc; }

    ///Returns @true if the event is still valid and can be processed.
    bool is_still_valid() {
        return !m_wpDoc.expired() && !m_wpInteractor.expired();
    }
};

/** A shared pointer for an EventAction.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventAction>  SpEventAction;


//---------------------------------------------------------------------------------------
/** Base class for all events generated events generated by SoundPlayer in the sound
    thread, during playback.
*/
class EventPlayback : public EventInfo
{
protected:
    WpInteractor m_wpInteractor;

    /// Constructor
    EventPlayback(EEventType type, WpInteractor wpInteractor)   //, WpDocument wpDoc)
        : EventInfo(type)
        , m_wpInteractor(wpInteractor)
    {
    }

public:
    /// Destructor
    virtual ~EventPlayback() {}

    /** Returns a weak pointer to the Interactor object managing the
        View in which the event is generated. */
    inline WpInteractor get_interactor() { return m_wpInteractor; }
};

/** A shared pointer for an EventPlayback.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventPlayback>  SpEventPlayback;


//---------------------------------------------------------------------------------------
// EventUpdateViewport
/**	An event to inform user application about the need to change the viewport
	during score playback, so that current played notes are visible.

	@warning This event is sent to your application from the Lomse sound thread.
			 For processing it, do not retain control: generate an application
			 event, place it on the application events loop, and return control to Lomse.

	During playback Lomse could generate this event when auto-scroll mode is on. The event
	is generated when Lomse detects that the viewport has to be changed for ensuring that the
	notes being played are visible in the viewport. The event provides information about
	the new proposed viewport origin.

	For processing this event the user application should create an application "viewport
	update"
	event, place it on the application events loop, and return control to Lomse. Later,
	when the
	application event is processed, the user application should invoke method
	<tt>Interactor.new_viewport(xPos, yPos)</tt> and, if necessary, update any GUI
	objects, such as scrollbars or other.

    <b>Example</b>

	For instance, in an application written using the wxWidgets framework you
	could have a global method for receiving all Lomse events, convert them in
	application events, and return control to Lomse:

	@code
	void MainFrame::on_lomse_event(SpEventInfo pEvent)
	{
		DocumentWindow* pCanvas = get_active_document_window();

		switch (pEvent->get_event_type())
		{
            ...
			case k_update_viewport_event:
			{
			    if (pCanvas)
			    {
					//generate update viewport event
			        SpEventUpdateViewport pEv(
			            static_pointer_cast<EventUpdateViewport>(pEvent) );
			        MyUpdateViewportEvent event(pEv);
			        ::wxPostEvent(pCanvas, event);
			    }
			    break;
			}
		...
	@endcode

	Later, your application should just process the application event as convenient.
	The recommended action is to use the Interactor for requesting to
	change the viewport to the new origin provided by the Lomse event. This will
	generate later an EventPaint for updating the View.
	In addition your application should to update
	scrollbars and other information or widgets used by the application. For
	instance:

	@code
	void DocumentWindow::on_update_viewport(MyUpdateViewportEvent& event)
	{
		SpEventUpdateViewport pEv = event.get_lomse_event();
		WpInteractor wpInteractor = pEv->get_interactor();
		int xPos = pEv->get_new_viewport_x();
		int yPos = pEv->get_new_viewport_y();
		WpInteractor wpInteractor = pEv->get_interactor();
		if (SpInteractor sp = wpInteractor.lock())
		{
			sp->new_viewport(xPos, yPos);

			//reposition scroll thumb
			SetScrollPos(wxVERTICAL, yPos);
		}
	}
	@endcode
*/
class EventUpdateViewport : public EventPlayback
{
protected:
    VRect m_damagedRectangle;
    Pixels m_x;
    Pixels m_y;

public:
    /// Constructor
    EventUpdateViewport(WpInteractor wpInteractor, Pixels x, Pixels y)
        : EventPlayback(k_update_viewport_event, wpInteractor)
        , m_x(x)
        , m_y(y)
    {
    }
    /// Destructor
    virtual ~EventUpdateViewport() {}

    /** Returns the x coordinate (pixels) for the new viewport origin. */
    inline Pixels get_new_viewport_x() { return m_x; }
    /** Returns the y coordinate (pixels) for the new viewport origin. */
    inline Pixels get_new_viewport_y() { return m_y; }
    /** Updates parameters x, y with the values for the new viewport origin. */
    void get_new_viewport(Pixels* x, Pixels* y)  { *x = m_x; *y = m_x; }
};

/** A shared pointer for an EventUpdateViewport.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventUpdateViewport>  SpEventUpdateViewport;


//---------------------------------------------------------------------------------------
// EventUpdateUI
/** %EventUpdateUI events are generated to inform about actions related to the Document
    that could require changes on the user application GUI, such as enabling or
    disabling tools, menu items and buttons.

    %EventUpdateUI events of type **k_selection_set_change** are generated when
    Document objects are selected or deselected. The rationale is that some application
    tools should be enabled or disabled depending on the kind of selected objects.

    %EventUpdateUI events of type **k_pointed_object_change** are generated when the
    document cursor is updated, as the object pointed by the cursor has changed. Again,
    the rationale is that some application tools should be enabled or disabled depending
    on the nature of the pointed object.

    %EventUpdateUI events are sent directly to the application global handler (the one
    set by invoking LomseDoorway::set_notify_callback() ). For handling these events
    you should generate application events, place them in the application events loop
    and return control to Lomse.

    <b>Example</b>

	For instance, in an application written using the wxWidgets framework you
	could handle the %EventUpdateUI as follows:

	@code
	void MainFrame::on_lomse_event(SpEventInfo pEvent)
	{
		DocumentWindow* pCanvas = get_active_document_window();

		switch (pEvent->get_event_type())
		{
            ...
            case k_selection_set_change:
            case k_pointed_object_change:
            {
                if (is_toolbox_visible())
                {
                    SpEventUpdateUI pEv( static_pointer_cast<EventUpdateUI>(pEvent) );
                    MyUpdateUIEvent event(pEv);
                    ::wxPostEvent(m_pToolBox, event);
                }
                break;
            }
		...
	@endcode

	Later, your application event will be processed by the appropriate objects
	(i.e., a ToolBox object) for synchronizing the state of tools, menu items
	and buttons:
	@code
    void ToolBox::on_update_UI(MyUpdateUIEvent& event)
    {
        SpEventUpdateUI pEv = event.get_lomse_event();
        WpInteractor wpInteractor = pEv->get_interactor();
        if (SpInteractor sp = wpInteractor.lock())
        {
            SelectionSet* pSelection = pEv->get_selection();
            DocCursor* pCursor = pEv->get_cursor();
            synchronize_tools(pSelection, pCursor);
        }
    }

    void ToolBox::synchronize_tools(SelectionSet* pSelection, DocCursor* pCursor)
    {
        //synchronize toolbox selected options with current selection and cursor object

        if (!pSelection->empty())
        {
            //there is a selection. Disable options related to cursor
            synchronize_with_cursor(false);
            synchronize_with_selection(true, pSelection);
        }
        else
        {
            //No selection. Disable options related to selections
            synchronize_with_cursor(true, pCursor);
            synchronize_with_selection(false);
        }
    }
    @endcode
*/
class EventUpdateUI : public EventAction
{
protected:
    SelectionSet* m_pSelection;
    DocCursor* m_pCursor;

    //for unit tests
    EventUpdateUI(EEventType type)
        : EventAction(type, WpInteractor(), WpDocument())
        , m_pSelection(nullptr)
        , m_pCursor(nullptr)
    {
    }

public:
    /// Constructor
    EventUpdateUI(EEventType type, WpInteractor wpInteractor, WpDocument wpDoc,
                  SelectionSet* pSelection, DocCursor* pCursor)
        : EventAction(type, wpInteractor, wpDoc)
        , m_pSelection(pSelection)
        , m_pCursor(pCursor)
    {
    }

    /** Returns a ptr to current SelectionSet object, containg the information for
        currently selected objects.    */
    inline SelectionSet* get_selection() { return m_pSelection; }

    /** Returns a ptr to current DocCursor object, so that user application can
        determine where is the cursor pointing.    */
    inline DocCursor* get_cursor() { return m_pCursor; }
};

/** A shared pointer for an EventUpdateUI.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventUpdateUI>  SpEventUpdateUI;


//---------------------------------------------------------------------------------------
// EventMouse
/** EventMouse is an object holding information about a mouse interaction. The event
    type informs about the specific interaction.

    Operating system raw mouse events should be passed to Lomse to be processed by
    the currently selected Task object (see @ref tasks-overview). Once done, Lomse will
    generate relevant events of different type (normally EventMouse,
    EventControlPointMoved and EventUpdateUI events) with relevant information about
    affected Document objects or other. For instance, a mouse click interpreted as a
    '<i>select object</i>' action will generate a EventUpdateUI event; or a mouse move
    could be interpreted as a View drag operation and will produce a change in the
    viewport origin and a EventPaint event for repainting the View.

    In particular, EventMouse events generation and suggested processing is as follows:

    <b>Types k_mouse_in_event and k_mouse_out_event</b>

    They are generated in the following situations:
    - TaskOnlyClicks selected: Move mouse generates mouse-in-out events as mouse flies
        over the different box areas. Probably your application should ignore these
        events.
    - TaskSelection selected: As mouse moves without clicking EventMouse of types
        k_mouse_in_event & k_mouse_out_event are generated as mouse flies over the
        different box areas. Probably your application should ignore these events.
    - TaskDataEntry selected: As mouse moves, mouse in and out events are generated,
        as mouse flies over the different box areas. Your application can handle these
        events and react to them as convenient (i.e. highlighting the implied
        insertion point/area).

    <b>Type k_on_click_event</b>

    They are generated in the following situations:
    - TaskOnlyClicks selected: When mouse left click at any point, except on Document
        controls (buttons, link, etc.). Probably your application should ignore these
        events.
    - TaskSelection selected: Left click down generates on-click event, to initiate
        selection process and ask your application to take the relevant decisions about
        how to continue the selection operation. When processing this event, your
        application should:
        - switch to TaskSelectionRectangle, or
        - switch to task TaskSelectText, or
        - select the clicked object and switch to TaskMoveObject.
        Remember that to switch Task you have to invoke method Interactor::switch_task().
    - TaskDataEntry selected: When mouse left click. This tasks is oriented to inserting
        new objects in the Document by clicking with the mouse. User moves the mouse
        and clicks at insertion point. Your application should determine the type of
        object to insert based on the application active tool and issue an edition
        command for inserting the object at the position implied by the event click
        point.

    <b>Type k_link_clicked_event</b>

    This event type is generated to inform about a left mouse click on a link (ImoLink)
    object.

    This event is generated when an %EventMouse of type k_on_click_event is being
    processing by Lomse to determine possible actions to do before passing the event
    to the user application. When Lomse determines that it is a left click on an ImoLink
    object, the event is passed to the user as an event of type k_link_clicked_event
    instead of a generic k_on_click_event.

    @note
	- Notice that an ImoLink object is different from an hyperlink control (an
    ImoControl containing an HyperlinkCtrl object). Hyperlink controls are only used in
    user applications that generate dynamic content (user defined GUI control objects
    embedded in the %Document) and can not be created from files.
	- A link (ImoLink) is only generated by a LMD @<link@> tag. Therefore, your application
	will never receive events of this type when processing MusicXML or LDP files.

    <b>Type k_show_contextual_menu_event</b>

    An %EventMouse of type k_show_contextual_menu_event informs about a mouse right
    button click on an ImoObj that has been interpreted by Lomse as a request for
    displaying its <i>contextual menu</i> (it is up to your application the decide what
    is this menu, if any, and what to do whit this event).

    They are generated in the following situations:
    - TaskSelection selected: Right click down, selects object and sends EventUpdateUI
        event (in case your application would like to enable/disable tools), then sends
        a EventMouse (k_show_contextual_menu_event) to request to show the contextual
        menu associated to the selected object, if any.
    - TaskDataEntry selected: Right click sends a EventMouse
        (k_show_contextual_menu_event) to request to show the contextual menu associated
        to the pointed object, if any.
*/
class EventMouse : public EventAction
{
protected:
    ImoId m_imoId;
    Pixels m_x;
    Pixels m_y;
    unsigned m_flags;

    //for unit tests
    EventMouse(EEventType type)
        : EventAction(type, WpInteractor(), WpDocument())
        , m_imoId(k_no_imoid)
        , m_x(0)
        , m_y(0)
        , m_flags(0)
    {
    }

public:
    /// Constructor
    EventMouse(EEventType type, WpInteractor wpInteractor, ImoId id,
               Pixels x, Pixels y, unsigned flags, WpDocument wpDoc)
        : EventAction(type, wpInteractor, wpDoc)
        , m_imoId(id)
        , m_x(x)
        , m_y(y)
        , m_flags(flags)
    {
    }

    // accessors

    /** Returns a pointer to the Document object (ImoObj) affected by the event. For
        instance, for a mouse click event this object will be the ImoObj pointed by the
        mouse.   */
    ImoObj* get_imo_object();

    /** Returns a pointer to the Observable object (an ImoContentObj) related to the
        event. It is either the mouse pointed object or the first ancestor of type
        ImoContentObj in the internal model hierarchy. */
    Observable* get_source();

    /** Returns the ID of the Document object (ImoObj) affected by the event. For
        instance, for a mouse click event this object will be the ID of the ImoObj
        pointed by the mouse.   */
    inline ImoId get_imo_id() { return m_imoId; }

    /** Returns the x coordinate (pixels relative to viewport origin) at which the event
        took place.    */
    inline Pixels get_x() { return m_x; }

    /** Returns the y coordinate (pixels relative to viewport origin) at which the event
        took place.    */
    inline Pixels get_y() { return m_y; }

    /** Returns a set of flags signaling the conditions at which the event
        took place. The different flags are described by enum #EEventFlag    */
    inline unsigned get_flags() { return m_flags; }

};

/** A shared pointer for an EventMouse.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventMouse>  SpEventMouse;


//---------------------------------------------------------------------------------------
// EventPlayCtrl
/** An object holding information about an score player control event. The event type informs
	the user application about the specific action.

	This event is generated only from LMD documents with embedded controls (buttons,
    links, etc.). In particular, these events are generated by the ScorePlayerCtrl
    object associated to an ImoScorePlayer object embedded in the Document. These events
    inform about a user action on any of the buttons of the ScorePlayerCtrl, so that the
    user application can perform the expected actions associated to these buttons. If
    your application is oriented to process scores in MusicXML format or regular LDP
    files without embedded controls you will never receive these event types.

    <b>Example</b>

	For instance, in an application oriented to process LMD documents with embedded
	controls, written using the wxWidgets framework, you
	could handle these events as follows:

	@code
	void DocumentWindow::display_document(const string& filename, ostringstream& reporter)
	{
		...
		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    //connect the View with the window buffer
		    spInteractor->set_rendering_buffer(&m_rbuf_window);

		    //register to receive desired events
		    spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
		    spInteractor->add_event_handler(k_do_play_score_event, this, wrapper_play_score);
		    spInteractor->add_event_handler(k_pause_score_event, this, wrapper_play_score);
		    spInteractor->add_event_handler(k_stop_playback_event, this, wrapper_play_score);
		...
	}

	void DocumentWindow::wrapper_play_score(void* pThis, SpEventInfo pEvent)
	{
		static_cast<DocumentWindow*>(pThis)->on_play_score(pEvent);
	}

	void DocumentWindow::on_play_score(SpEventInfo pEvent)
	{
		switch (pEvent->get_event_type())
		{
		    case k_do_play_score_event:
		        play_score(pEvent);
		        return;

		    case k_pause_score_event:
		        play_pause();
		        return;

		    case k_stop_playback_event:
		    default:
		        play_stop();
		        return;
		}
	}

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
	}

	void DocumentWindow::play_stop()
	{
		ScorePlayer* pPlayer  = m_appScope.get_score_player();
		pPlayer->stop();

		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    spInteractor->set_operating_mode(is_edition_enabled() ? Interactor::k_mode_edition
		                                                          : Interactor::k_mode_read_only);
		}
	}

	void DocumentWindow::play_pause()
	{
		ScorePlayer* pPlayer  = m_appScope.get_score_player();
		pPlayer->pause();
	}
	@endcode
*/
class EventPlayCtrl : public EventAction
{
protected:
    ImoScore* m_pScore;
    PlayerGui* m_pPlayer;

    //for unit tests
    EventPlayCtrl(EEventType evType)
        : EventAction(evType, WpInteractor(), WpDocument())
        , m_pScore(nullptr)
        , m_pPlayer(nullptr)
    {
    }

public:
    /// Constructor
    EventPlayCtrl(EEventType evType, WpInteractor wpInteractor, WpDocument wpDoc,
                  ImoScore* pScore, PlayerGui* pPlayer)
        : EventAction(evType, wpInteractor, wpDoc)
        , m_pScore(pScore)
        , m_pPlayer(pPlayer)
    {
    }

    // accessors
    /// Returns a ptr. to the score affected by the event
    inline ImoScore* get_score() const { return m_pScore; }
	/// Returns a ptr. to the PlayerGui object associated with this score playback
    inline PlayerGui* get_player() const { return m_pPlayer; }
};

/** A shared pointer for an EventPlayCtrl.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventPlayCtrl>  SpEventPlayCtrl;


//---------------------------------------------------------------------------------------
// EventEndOfPlayback
/** An event generated by the ScorePlayer object to inform about the end of playback,
    just in case your application would like to do any action, such as enabling or
    disabling GUI buttons related to playback or to enable document edition.

	Events of type <tt>k_end_of_playback_event</tt> must be handled by
	by converting them into application events, placing them in the application events
	loop and returning control to Lomse:

	@code
	void MainFrame::on_lomse_event(SpEventInfo pEvent)
	{
		DocumentWindow* pCanvas = get_active_document_window();

		switch (pEvent->get_event_type())
		{
			...
			case k_end_of_playback_event:
			{
			    if (pCanvas)
			    {
					//generate end of playback event
			        SpEventEndOfPlayback pEv( static_pointer_cast<EventEndOfPlayback>(pEvent) );
			        MyEndOfPlaybackEvent event(pEv);
			        ::wxPostEvent(pCanvas, event);
			    }
			    break;
			}
		...
	@endcode

	Later, your application will process the application event. When doing it, it must
	inform the Interactor, and then your application should do whatever it needs to
	do, i.e.: enabling document edition:

	@code
	void DocumentWindow::on_end_of_playback(MyEndOfPlaybackEvent& event)
	{
		SpEventEndOfPlayback pEv = event.get_lomse_event();
		WpInteractor wpInteractor = pEv->get_interactor();
		if (SpInteractor spInteractor = wpInteractor.lock())
		{
		    spInteractor->on_end_of_play_event(pEv->get_score(), pEv->get_player());
		    spInteractor->set_operating_mode(is_edition_enabled() ? Interactor::k_mode_edition
		                                                          : Interactor::k_mode_read_only);
		}
	}
	@endcode
*/
class EventEndOfPlayback : public EventPlayback
{
protected:
    ImoScore* m_pScore;
    PlayerGui* m_pPlayer;

    EventEndOfPlayback(EEventType evType)    //for unit tests
        : EventPlayback(evType, WpInteractor())
        , m_pScore(nullptr)
        , m_pPlayer(nullptr)
    {
    }

public:
    /// Constructor
    EventEndOfPlayback(EEventType evType, WpInteractor wpInteractor, ImoScore* pScore,
                   PlayerGui* pPlayer)
        : EventPlayback(evType, wpInteractor)
        , m_pScore(pScore)
        , m_pPlayer(pPlayer)
    {
    }

    // accessors
    /// Returns a ptr. to the score affected by the event
    inline ImoScore* get_score() const { return m_pScore; }
	/// Returns a ptr. to the PlayerGui object associated with this score playback
    inline PlayerGui* get_player() const { return m_pPlayer; }
};

/** A shared pointer for an EventEndOfPlayback.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventEndOfPlayback>  SpEventEndOfPlayback;



//---------------------------------------------------------------------------------------
// EventVisualTracking
/**
    An event to signal different actions related to
    visual tracking effects during score playback.

	@warning This event is sent to your application from the Lomse sound thread.
			 For processing it, do not retain control: generate an application
			 event, place it on the application events loop, and return control to Lomse.

	The %EventVisualTracking event, derived from EventPlayback, signals the need to do
	several actions related to visual tracking effects during score playback. It contains
	a list of sub-events and affected objects:
	- <b>k_highlight_on</b>. Add highlight to a note/rest.
	- <b>k_highlight_off</b>. Remove highlight from a note/rest.
	- <b>k_end_of_visual_tracking</b>. End of score playback. Remove all visual tracking effects.
	- <b>k_move_tempo_line</b>. Advance visual tempo line to next beat.

	For handling all events related to score playback it is **very important** to
	return control to Lomse as soon
	as possible. This is because, currently, Lomse does not implement an event
	subsystem with its own thread. Instead Lomse sends events to your application
	by invoking a callback. This implies that your application code for handling
	the event is processed by the Lomse sound thread. As this thread is dealing
	with sound generation, your application **must** return control to Lomse as
	soon as possible, so that the sound thread can continue processing sound events.
	Otherwise, sound can be stalled! The suggested way for handling
	EventVisualTracking events is to generate an application
	event and to enqueue it in the application events system.

	For instance, in an application written using the wxWidgets framework you
	could have a global method for receiving all Lomse events, convert them in
	application events, and return control to Lomse:

	@code
	void MainFrame::on_lomse_event(SpEventInfo pEvent)
	{
		DocumentWindow* pCanvas = get_active_document_window();

		switch (pEvent->get_event_type())
		{
			case k_tracking_event:
			{
			    if (pCanvas)
			    {
					//generate visual tracking event
			        SpEventVisualTracking pEv(
			            static_pointer_cast<EventVisualTracking>(pEvent) );
			        MyVisualTrackingEvent event(pEv);
			        ::wxPostEvent(pCanvas, event);
			    }
			    break;
			}
		...
	@endcode

	Later, your application will just process the application event as convenient.
	The recommended action is to use the Interactor for requesting to
	process the event and update the rendering bitmap. For instance:

	@code
	void DocumentWindow::on_visual_tracking(MyVisualTrackingEvent& event)
	{
		SpEventVisualTracking pEv = event.get_lomse_event();
		WpInteractor wpInteractor = pEv->get_interactor();
		if (SpInteractor sp = wpInteractor.lock())
		    sp->on_visual_tracking(pEv);
	}
	@endcode
*/
class EventVisualTracking : public EventPlayback
{
protected:
    ImoId m_nID;            //ID of the target score for the event
    std::list< pair<int, ImoId> > m_items;
    TimeUnits m_timepos;    //for move tempo line sub-event

public:
    /// Constructor
    EventVisualTracking(WpInteractor wpInteractor, ImoId nScoreID)
        : EventPlayback(k_tracking_event, wpInteractor)
        , m_nID(nScoreID)
        , m_timepos(0.0)
    {
    }

    /// Copy constructor
    EventVisualTracking(const EventVisualTracking& event)
        : EventPlayback(event.m_type, event.m_wpInteractor)
        , m_nID(event.m_nID)
        , m_items(event.m_items)
        , m_timepos(event.m_timepos)
    {
    }

    ///Values for sub-events
    enum ETrackingEvent
    {
        k_highlight_on,           ///< add highlight to a note/rest
        k_highlight_off,          ///< remove highlight from a note/rest
        k_end_of_visual_tracking, ///< end of score playback. Remove all visual tracking effects.       ///< end of score play back. Remove all highlight.
        k_move_tempo_line,        ///< advance visual tempo line to next beat
    };

    /// Returns ID of the score affected by the event
    inline ImoId get_score_id() { return m_nID; }

	/// Returns the number of sub-events
    inline int get_num_items() { return int( m_items.size() ); }

	/** Returns the list of notes & rests affected by the event. Each list item is a pair
		of two values: the sub-event type (from enum #ETrackingEvent), and the ID of
		the object (note / rest) to highlight or un-highlight. */
    inline std::list< pair<int, ImoId> >&  get_items() { return m_items; }

	/// Returns the time position for the tempo line sub-event
    inline TimeUnits get_timepos() { return m_timepos; }

///@cond INTERNAL
	//construction
    void add_item(int type, ImoId id)
    {
        m_items.push_back( make_pair(type, id) );
    }
    void add_move_tempo_line_event(TimeUnits timepos)
    {
        m_items.push_back( make_pair(k_move_tempo_line, -1) );
        m_timepos = timepos;
    }
///@endcond
};

/** A shared pointer for an EventVisualTracking.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventVisualTracking>  SpEventVisualTracking;


//---------------------------------------------------------------------------------------
// EventControlPointMoved
/** %EventControlPointMoved event is generated when a mouse action has been
	interpreted by Lomse as a command for moving an object control point (handler).

    Operating system raw mouse events can be passed to Lomse to be processed by
    the currently selected Task object (see @ref tasks-overview). Once done, Lomse will
    generate relevant events of different types (normally EventMouse,
    EventControlPointMoved and EventUpdateUI events) with relevant information about
    affected Document objects or other.

	In particular, an %EventControlPointMoved event is generated when TaskMoveHandler
	is selected and the user releases the mouse button. For having a full view of the
	event generation process, imagine that your application has selected TaskSelection
	behavior; this is the default task when no specific task is assigned. This task is
	oriented to initiate actions related to selecting objects: drawing a section
	rectangle, displaying a contextual menu, generating on-click events, and
	initiating a '<i>move object</i>' action. In this situation a mouse left click down
	generates an EventMouse of type <tt>k_on_click_event</tt>. Your application
	receives this event and having determined that the pointed object is a handle,
	decides to select the clicked object and switch to TaskMoveObject. Now, Lomse
	will take care of moving the handle while the mouse moves, and when the mouse button
	is released, Lomse will generate the %EventControlPointMoved to inform your
	application about the event and the final destination point of the handle.

	For processing this event, probably your application will issue an edition command
	for changing the object owning the moved handle. All relevant information for
    this is provided by the event. For example:

	@code
	class DocumentWindow : public wxWindow
	{
	public:
		...
		//callback wrappers
		static void wrapper_on_command_event(void* pThis, SpEventInfo pEvent);
		...
	};

	void DocumentWindow::display_document(const string& filename, ostringstream& reporter)
	{
		...
		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    //connect the View with the window buffer
		    spInteractor->set_rendering_buffer(&m_rbuf_window);

		    //register to receive desired events
		    spInteractor->add_event_handler(k_control_point_moved_event, this, wrapper_on_action_event);
		...
	}

	void DocumentWindow::wrapper_on_action_event(void* pThis, SpEventInfo pEvent)
	{
		static_cast<DocumentWindow*>(pThis)->on_action_event(pEvent);
	}

	void DocumentWindow::on_action_event(SpEventInfo pEvent)
	{
		SpEventAction pEv( static_pointer_cast<EventAction>(pEvent) );
		if (!pEv->is_still_valid())
		    return;

		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    SelectionSet* selection = spInteractor->get_selection_set();
		    DocCursor* cursor = spInteractor->get_cursor();
		    CommandEventHandler handler(m_appScope, this, m_toolsInfo, selection, cursor);
		    handler.process_action_event(pEv);
		}
	}

    void CommandEventHandler::process_action_event(SpEventAction event)
	{
		m_fEventProcessed = false;
		if (m_pController->is_edition_enabled())
		{
		    if (event->is_control_point_moved_event())
		    {
		        SpEventControlPointMoved pEv(
		                static_pointer_cast<EventControlPointMoved>(event) );
		        UPoint shift = pEv->get_shift();
		        int iPoint = pEv->get_handler_index();
		        m_executer.move_object_point(iPoint, shift);
		        m_fEventProcessed = true;
		    }
		}
	}

	void CommandGenerator::move_object_point(int iPoint, UPoint shift)
	{
		string name = to_std_string(_("Move control point"));
		m_pController->exec_lomse_command( new CmdMoveObjectPoint(iPoint, shift, name),
		                                   k_show_busy );
	}

	void DocumentWindow::exec_lomse_command(DocCommand* pCmd, bool fShowBusy)
	{
		if (!m_pPresenter)
		    return;

		if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
		{
		    if (fShowBusy)
		        ::wxBeginBusyCursor();
		    spInteractor->exec_command(pCmd);
		    update_status_bar_caret_timepos();
		    if (fShowBusy)
		        ::wxEndBusyCursor();
		}
	}
	@endcode
*/
class EventControlPointMoved : public EventAction
{
protected:
    ImoId m_imoId;
    int m_iHandler;     //modified point index
    UPoint m_uShift;    //shift to apply to object
    int m_gmoType;
    ShapeId m_idx;      //affected shape or -1 if box

public:
    /// Constructor
    EventControlPointMoved(EEventType type, WpInteractor wpInteractor, GmoObj* pGmo,
                           int iHandler, UPoint uShift, WpDocument wpDoc);

    // accessors
	///Returns the handler number (0...n-1) for the moved handler
    inline int get_handler_index() { return m_iHandler; }
	///Returns the shape ID of the object owning the handler  or -1 if it is a box
    inline ShapeId get_shape_index() { return m_idx; }
	///Returns the shift (in logical units) to apply to the object
    inline UPoint get_shift() { return m_uShift; }
	///Returns the type of graphic object owning the handler
    inline int get_gmo_type() { return m_gmoType; }

};

/** A shared pointer for an Interactor.
    @ingroup typedefs
    @#include <lomse_events.h>
*/
typedef std::shared_ptr<EventControlPointMoved>  SpEventControlPointMoved;


//=======================================================================================
// Requests
//=======================================================================================


//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum assigns an request type for Lomse requests.

    @#include <lomse_events.h>
*/
enum ERequestType
{
    k_null_request = -1,
    k_dynamic_content_request,      ///> request for dynamic content creation
    k_get_font_filename,            ///> request the file path for the given font
};

//---------------------------------------------------------------------------------------
/** An abstract class holding information about a Lomse Request

    When Lomse needs some information or some platform dependent service it sends
    a Request to the user application asking for the required information or service.
    Lomse process is paused until the user application provides the requested data.

    See details on each derived class.
*/
class Request
{
protected:
    int m_type;

    Request(int type) : m_type(type) {}

public:
    /// Destructor
    virtual ~Request() {}

    /// Returns the request type. It is a value from enmun #ERequestType.
    inline int get_request_type() { return m_type; }

    //classification
    /// @name For checking the request type
    ///@{
    inline bool is_dynamic_content_request() { return m_type == k_dynamic_content_request; }
    inline bool is_get_font_filename() { return m_type == k_get_font_filename; }
    ///@}
};

//---------------------------------------------------------------------------------------
// RequestDynamic
/** While parsing an LDP document, a <tt>(dynamic)</tt> tag has been found. Lomse
    sends this request to the user application, asking for the content that must be
    inserted in the Document to replace the <tt>(dynamic)</tt> tag.

    The user application has to answer this request by invoking method
    RequestDynamic::set_object() passing as argument a pointer to the ImoObj
    that must be inserted in the Document.

    See @ref page-dynamic-content.
*/
class RequestDynamic : public Request
{
protected:
    Document* m_pDoc;
    ImoObj* m_pObj;

public:
    /// Constructor
    RequestDynamic(Document* pDoc, ImoObj* pObj)
        : Request(k_dynamic_content_request)
        , m_pDoc(pDoc)
        , m_pObj(pObj)
    {
    }
    ~RequestDynamic() {}

    /** If the user application has not yet handled this request, this method returns
        the ImoDynamic object that should be replaced. Otherwise, returns the replacement
        ImoObj set by the user application.    */
    inline ImoObj* get_object() { return m_pObj; }

    /// Returns a pointer to the Document to which this request refers to.    */
    inline Document* get_document() { return m_pDoc; }

    ///for setting requested data
    inline void set_object(ImoObj* pObj) { m_pObj = pObj; }

};

//---------------------------------------------------------------------------------------
// RequestFont
/** When a Document uses a font that Lomse can not find it sends this request to the
    user application, asking for the file path for the font to use. The user application
    has to answer this request by invoking method
    RequestFont::set_font_fullname() passing as argument the path to the font to use.

    Lomse manages fonts in different ways depending on the platform:
      - For Linux (and macOS) Lomse uses system installed fonts and makes substitutions
        when requested font is not available. The RequestFont callback is only used
        when Lomse cannot find the "Bravura.otf" music font needed for rendering musical
        symbols, when it is not installed in the system (e.g. missing dependency in
        installation page, local installation from sources, etc.).
      - For Windows, Lomse also uses system installed fonts and makes substitutions
        when requested font is not available, but the substitutions table is small and
        thus, the RequestFont callback is used when Lomse cannot find the the requested
        font or an appropriate substitution.
      - For other platforms Lomse only uses the fonts distributed with Lomse package
        and the RequestFont callback is used when requesting any other font not present
        in Lomse package.

    <b>Example:</b>

    @code
    class MyApp
    {
    public:
        ...

        //callbacks
        static void lomse_request_handler_method(void* pThis, Request* pRequest);
    }

    void MyApp::initialize_lomse()
    {
        ...

        //initialize the library with these values
        m_lomse.init_library(pixel_format, resolution, reverse_y_axis, m_lomseReporter);

        //set callback for requests
        m_lomse.set_request_callback(this, lomse_request_handler_method);
    }

    void MyApp::lomse_request_handler_method(void* pThis, Request* pRequest)
    {
        static_cast<MyApp*>(pThis)->on_lomse_request(pRequest);
    }

    void MyApp::on_lomse_request(Request* pRequest)
    {
        int type = pRequest->get_request_type();
        switch (type)
        {
            case k_dynamic_content_request:
                generate_dynamic_content( dynamic_cast<RequestDynamic*>(pRequest) );
                break;

            case k_get_font_filename:
                get_font_filename( dynamic_cast<RequestFont*>(pRequest) );
                break;

            default:
                LogError("Unknown request %d", type);
        }
    }

    void MyApp::get_font_filename(RequestFont* pRequest)
    {
        //This is just a trivial example. In real applications you should
        //use operating system services to find a suitable font

        //notes on parameters received:
        // - fontname can be either the face name (i.e. "Book Antiqua") or
        //   the familly name (i.e. "Liberation sans")

        const string& fontname = pRequest->get_fontname();
        bool bold = pRequest->get_bold();
        bool italic = pRequest->get_italic();

        string path = "/usr/share/fonts/truetype/";

        //if family name, choose a font name
        string name = fontname;
        if (name == "serif")
            name = "Times New Roman";
        else if (name == "sans-serif")
            name = "Tahoma";
        else if (name == "handwritten" || name == "cursive")
            name = "Monotype Corsiva";
        else if (name == "monospaced")
            name = "Courier New";

        //choose a suitable font file
        string fontfile;
        if (name == "Times New Roman")
        {
            if (italic && bold)
                fontfile = "freefont/FreeSerifBoldItalic.ttf";
            else if (italic)
                fontfile = "freefont/FreeSerifItalic.ttf";
            else if (bold)
                fontfile = "freefont/FreeSerifBold.ttf";
            else
                fontfile = "freefont/FreeSerif.ttf";
        }

        else if (name == "Tahoma")
        {
            if (bold)
                fontfile = "freefont/FreeSansOblique.ttf";
            else
                fontfile = "freefont/FreeSans.ttf";
        }

        else if (name == "Monotype Corsiva")
        {
            fontfile = "ttf-dejavu/DejaVuSans-Oblique.ttf";
        }

        else if (name == "Courier New")
        {
            if (italic && bold)
                fontfile = "freefont/FreeMonoBoldOblique.ttf";
            else if (italic)
                fontfile = "freefont/FreeMonoOblique.ttf";
            else if (bold)
                fontfile = "freefont/FreeMonoBold.ttf";
            else
                fontfile = "freefont/FreeMono.ttf";
        }

        else
            fontfile = "freefont/FreeSerif.ttf";


        pRequest->set_font_fullname( path + fontfile );
    }
    @endcode
*/
class RequestFont : public Request
{
protected:
    const string& m_fontname;
    bool m_bold;
    bool m_italic;
    string m_fullName;

public:
    /// Constructor
    RequestFont(const string& fontname, bool bold, bool italic)
        : Request(k_get_font_filename)
        , m_fontname(fontname)
        , m_bold(bold)
        , m_italic(italic)
    {
        set_default_fullname();
    }
    ~RequestFont() {}

    //getters
    /** Returns the <i>fontname</i>. It can be either the face name (i.e.
        "Book Antiqua") or the familly name (i.e. "Liberation sans"). */
    inline const string& get_fontname() { return m_fontname; }
    ///Returns @true is the font is requested in bold face. */
    inline bool get_bold() { return m_bold; }
    ///Returns @true is the font is requested in italic style. */
    inline bool get_italic() { return m_italic; }

    /** User application should use this method for setting the requested data
	*/
    inline void set_font_fullname(const string& name) { m_fullName = name; }

///@cond INTERNAL
    inline string get_font_fullname() { return m_fullName; }
///@endcond

protected:

    void set_default_fullname()
    {
        //This is just a mock method to avoid crashes when using the library without
        //initializing it

        m_fullName = LOMSE_FONTS_PATH;

        if (m_fontname == "Bravura")
            m_fullName += "Bravura.otf";
        else
            m_fullName += "LiberationSerif-Regular.ttf";
    }

};


// ======================================================================================
/** Abstract base class. Any class that wants to receive events must derive from
	EventHandler and must implement the handle_event() method.
*/
class EventHandler
{
public:
    /// Destructor
	virtual ~EventHandler() {}

    /// Request for handling the passed event.
	virtual void handle_event(SpEventInfo pEvent) = 0;
};


// ======================================================================================
/** Auxiliary object responsible for keeping information about an Observable object
	being observed, the events to listen to and the methods that must invoke to
	dispatch each event. It is also responsible for doing this dispatch when
	requested.
*/
class Observer
{
protected:
    Observable* m_target;
    int m_type;
    ImoId m_id;
    std::list<EventCallback*> m_handlers;

    friend class EventNotifier;
    Observer(Observable* target);
    Observer(Observable* root, int childType, ImoId childId);

public:
    /// Destructor
    virtual ~Observer();

    /** Returns the event generator object that is being
        observed by this %Observer.    */
    Observable* target();

    /** Notify the event pEvent to all registered objects observing events of the same
        type than pEvent type.    */
    void notify(SpEventInfo pEvent);

    ///@{
    /// Register to receive notifications from this %Observer.
    void add_handler(int eventType, void (*pt2Func)(SpEventInfo event) );
    void add_handler(int eventType, void* pThis,
                     void (*pt2Func)(void* pObj, SpEventInfo event) );
    void add_handler(int eventType, EventHandler* pHandler);
    void add_handler(int eventType, int childType, ImoId childId, EventHandler* pHandler);
    ///@}

    /** Remove a registered handler so that in future it will not be notified of
        new events.    */
    void remove_handler(int evtType);

    /** Returns the type of the observed event generator object.    */
    inline int get_observable_type() { return m_type; }

protected:
    std::list<EventCallback*>::iterator find_handler(int eventType);
    void remove_old_handler(int eventType);

};

// ======================================================================================
/** Any object generating events must derive from Observable and must implement
	method "get_event_notifier()".

	In order to allow for different event management models, responsibilities for
	event generation and event dispatching are decoupled. For this, the Observable
	pattern is split into two objects: the Observable itself is just a facade
	object providing the interface for adding/removing observers, and delegates in
	the EventNotifier object for doing the real work and event dispatching.
*/
class Observable
{
public:
    /// Destructor
	virtual ~Observable() {}

    /// Returns the EventNotifier object associated to this %Observable object.
	virtual EventNotifier* get_event_notifier() = 0;

    //register to observe the whole object
    /** Register a notification handler (EventHandler object) for all events generated by
        this %Observable object.
        @param eventType    The type of event to be notified. It must be a value from
            enmu #EEventType.
        @param pHandler     The object, derived from EventHandler, that will receive
            the notifications (by invoking its EventHandler::handle_event() method.    */
    virtual void add_event_handler(int eventType, EventHandler* pHandler);

    /** Register a notification handler (c++ method) for all events generated by this
        %Observable object.
        @param eventType    The type of event to be notified. It must be a value from
            enmu #EEventType.
        @param pThis    Pointer to the object that will receive the notifications.
        @param pt2Func  The method in previous object that will be invoked to
            notify events.

        See @ref handling-events
    */
    virtual void add_event_handler(int eventType, void* pThis,
                                   void (*pt2Func)(void* pObj, SpEventInfo event) );

    /** Register a notification handler (c function) for all events generated by this
        %Observable object.
        @param eventType    The type of event to be notified. It must be a value from
            enmu #EEventType.
        @param pt2Func  The function that will be invoked to notify events.

        See @ref handling-events
    */
    virtual void add_event_handler(int eventType, void (*pt2Func)(SpEventInfo event) );

    /// This enum describes the valid observable targets.
    enum EObservedChild
    {   k_root=0,       ///< The whole Document
        k_control,      ///< A control (an ImoControl) object
        k_imo,          ///< A child of the Document (an ImoObj) except ImoControl objs.
    };

    //register to observe just a child
    /** Register an event handler (EventHandler object) for some events generated by
        children of this %Observable object. Parameters <i>childType</i> and
        <i>childId</i> allows to filter the desired events.
        @param childType    It must be value <i>k_control</i> or <i>k_imo</i> from
            enum #EObservedChild, and selects the source of the events to be notified.
        @param childId      It is the ID of the ImoObj to observe.
        @param eventType    The type of event to be notified. It must be a value from
            enmu #EEventType.
        @param pHandler     The object, derived from EventHandler, that will receive
            the notifications (by invoking its EventHandler::handle_event() method.
    */
    void add_event_handler(int childType, ImoId childId, int eventType,
                           EventHandler* pHandler);

    /** Register an event handler (c++ method) for some events generated by children of
        this %Observable object. Parameters <i>childType</i> and <i>childId</i> allows
        to filter the desired events.
        @param childType    It must be value <i>k_control</i> or <i>k_imo</i> from
            enum #EObservedChild, and selects the source of the events to be notified.
        @param childId      It is the ID of the ImoObj to observe.
        @param eventType    The type of event to be notified. It must be a value from
            enmu #EEventType.
        @param pThis    Pointer to the object that will be notified.
        @param pt2Func  The method in previous object that will be invoked to
            notify events.

        See @ref handling-events
    */
    void add_event_handler(int childType, ImoId childId, int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) );

    /** Register an event handler (c function) for some events generated by children
        of this %Observable object. Parameters <i>childType</i> and <i>childId</i> allows
        to filter the desired events.
        @param childType    It must be value <i>k_control</i> or <i>k_imo</i> from
            enum #EObservedChild, and selects the source of the events to be notified.
        @param childId      It is the ID of the ImoObj to observe.
        @param eventType    The type of event to be notified. It must be a value from
            enmu #EEventType.
        @param pt2Func  The method in previous object that will be invoked to
            notify events.
    */
    void add_event_handler(int childType, ImoId childId, int eventType,
                           void (*pt2Func)(SpEventInfo event) );

    /// Returns a pointer to the observable child.    */
    virtual Observable* get_observable_child(int UNUSED(childType),
                                             ImoId UNUSED(childId))
    {
        return nullptr;
    }

};


// ======================================================================================
/** Any object that wants to dispatch events must derive from EventNotifier class
*/
class EventNotifier
{
protected:
    EventsDispatcher* m_pDispatcher;
    std::list<Observer*> m_observers;

public:
    /// Constructor
    EventNotifier(EventsDispatcher* dispatcher) : m_pDispatcher(dispatcher) {}
    /// Destructor
    virtual ~EventNotifier();

    //Event notification
    /// Sends the event to all objects observing the event generator object.
    bool notify_observers(SpEventInfo pEvent, Observable* target);
    /// Remove an Observer
    void remove_observer(Observer* observer);
    /// Add Observer for the given event generator target.
    Observer* add_observer_for(Observable* target);
    /** Add Observer for events generated by the object whose ID and type are passed
        as parameters. The type must be a value from enum Observable::EObservedChild. */
    Observer* add_observer_for_child(Observable* parent, int childType, ImoId childId);

protected:
    friend class Observable;
    void add_handler(Observable* target, int eventType, EventHandler* pHandler);
    void add_handler(Observable* target, int eventType, void* pThis,
                     void (*pt2Func)(void* pObj, SpEventInfo event) );
    void add_handler(Observable* target, int eventType,
                     void (*pt2Func)(SpEventInfo event) );

    void add_handler_for_child(Observable* parent, int childType, ImoId childId,
                               int eventType, EventHandler* pHandler);
    void add_handler_for_child(Observable* parent, int childType, ImoId childId,
                               int eventType, void (*pt2Func)(SpEventInfo event) );
    void add_handler_for_child(Observable* parent, int childType, ImoId childId,
                               int eventType, void* pThis,
                               void (*pt2Func)(void* pObj, SpEventInfo event) );

protected:
    void delete_observers();

};


}   //namespace lomse

#endif      //__LOMSE_EVENTS_H__
