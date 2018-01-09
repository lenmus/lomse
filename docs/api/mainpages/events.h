/**
@page page-events Events and Requests


@tableofcontents


@section notifications Notifications: events and requests

Lomse is platform independent code, and knows nothing about your platform mechanisms for creating events. For this reason, all Lomse communication with the user application takes place through <i>notifications</i>, implemented by invoking a callback function in your application.

Lomse <i>notifications</i> can be divided into two categories based on how they are created and how they are processed:
    - <b>Requests</b>. When Lomse needs some information or some platform dependent service it sends a <i>Request</i>. Lomse process is paused until the user application provides the requested data so that Lomse can continue.
	- <b>Events</b>. They are informative notifications about something, for instance, about a mouse click. Events can refer:
		- To a View, i.e. a <i>playback highlight</i> event, or a <i>window update</i> event.
		- To a Document or its content, i.e. a <i>mouse click</i> event; and
		- To a Control object, i.e. a click on a link control.


@section handling-events Events and how to handle them

For managing events, Lomse architecture uses the observer pattern: any other object wanting to know about events must register as an observer by providing an event handler object or a callback method/function for handling the events. In theory, your application could register a handler for each event type wanting to receive.

But in practice, you must be aware that <b>the name event could be misleading.</b> Lomse is not an event driven library, but a collection of services that run in the user application thread from which the service is requested. While processing a service request Lomse could find important things to communicate to your application. For this Lomse will invoke a callback so that you can take note, but Lomse <b>needs</b>to continue processing the service request. Therefore:

- You should be aware that <b>Lomse events are not 'true' events</b>, for which the event generation code and the event processing code is decoupled. Lomse event generation is just invoking the callback handler function in your application and so, it is not decoupled from processing the event in your application.

- <b>It is your application responsibility to decouple event processing</b>, that is, your application shouldn’t retain the control more time than the strictly necessary for taking note of the event. The strongly recommended way for processing Lomse events is to create an OS/application event, put it in the application events loop and return control to Lomse, so that Lomse can terminate processing the service request that originated the event.

Due to this, it was decided to send all events to a single event handler, instead of using the subscription mechanism. The rationale was that a single handler would simplify the user application task of creating an OS/application event, and putting it in the application events loop. This handler, named the <i>global handler</i> for events, is set up at Lomse initialization:

@code
LomseDoorway& lib = ...
lib.set_notify_callback(this, lomse_event_handler);
@endcode

Unfortunately, currently not all events are sent to this handler. As I was developing the library I forgot the reasoning for having created the global handler and thus, for some events created later I maintained the subscription mechanism of the observer pattern. During the library tests, with the LenMus Phonascus application, there were no problems with these events by not having decoupled its processing in the user application. And so, the subscription mechanism remained active for these events.

As a consequence, the situation is now a little bit confusing: some events are always sent to the global handler but other events will not be received unless your application register an specific event handler for each one of these event types. Of course you can always register the global handler as the handler for these events, but you will have to code it in your application.

With the exception of the <tt>k_update_window_event</tt> it is not mandatory to handle all other Lomse events.




@section events-list Events and how are they notified

Here is a list of all events, grouped by handling mechanism. For more information on each event read the class documentation for the event.

a) Events always sent to the global handler:

- All events created during playback in the Lomse sound thread:

	- EventUpdateViewport (type <tt>k_update_viewport_event</tt>) - Ask user app to update viewport origin using provided coordinates.
	- EventEndOfPlayback (type <tt>k_end_of_playback_event</tt>) - End of playback.
	- EventScoreHighlight (type <tt>k_highlight_event</tt>) - Event containing mainly a list of notes/rests to highlight or unhighlight

- Mouse clicks on links (ImoLink objects):

	- EventMouse (type <tt>k_on_link_clicked_event</tt>) - left click on link (ImoLink object).

- Some events created only when document edition is enabled:

	- EventMouse (type <tt>k_show_contextual_menu_event</tt>) - right click on object: contextual menu request
	- EventUpdateUI (type <tt>k_selection_set_change</tt>) - Selected objects changed
	- EventUpdateUI (type <tt>k_pointed_object_change</tt>) - Cursor pointing to a different object

b) Events for which you will have to register a handler at the event creator object (see @ref event-handlers):

- Register at the Interactor:
	- EventPaint (type <tt>k_update_window_event</tt>) - Ask user app to update window with current bitmap.
		@code
	    spInteractor->add_event_handler(k_update_window_event, this, on_update_window);
		@endcode
		This event is decoupled by design: user must do repaint immediately, without more delays.

	- EventPlayCtrl (type <tt>k_do_play_score_event</tt>) - Start/resume playback
	- EventPlayCtrl (type <tt>k_pause_score_event</tt>) - Pause playback
	- EventPlayCtrl (type <tt>k_stop_playback_event</tt>) - Stop playback
		@code
	    spInteractor->add_event_handler(k_do_play_score_event, this, wrapper_play_score);
	    spInteractor->add_event_handler(k_pause_score_event, this, wrapper_play_score);
	    spInteractor->add_event_handler(k_stop_playback_event, this, wrapper_play_score);
		@endcode
		These events are created by ScorePlayerGui objects. These objects are only created when processing
		LMD files with @<scorePlayer@> tags.

	- EventControlPointMoved (event type <tt>k_control_point_moved_event</tt>) - User moves a handler: handler released event
		@code
		spInteractor->add_event_handler(k_control_point_moved_event, this, wrapper_on_command_event);
		@endcode
		These events are created only when document edition is enabled.

- Register at the Document or at specific objects in the Document:

	- EventMouse (type <tt>k_mouse_in_event</tt>) - Mouse goes over an object
	- EventMouse (type <tt>k_mouse_out_event</tt>) - Mouse goes out from an object
	- EventMouse (type <tt>k_on_click_event</tt>) - Document, ImoContentObj: click on object
		@code
		//example 1: register at the document
	    pDoc->add_event_handler(k_on_click_event, this, wrapper_on_click_event);
		//example 2: register for handling events related to an specific object
        ButtonCtrl* pButton = ...
        pButton->add_event_handler(k_on_click_event, this);
        pButton->add_event_handler(k_mouse_in_event, this);
        pButton->add_event_handler(k_mouse_out_event, this);
		@endcode




@section event-handlers How to register an event handler

To capture and handle an event you must register an <tt>event handler</tt> on the object generating the events by invoking its <tt>add_event_handler()</tt> method. Your handler can be:
- a C function,
- a C++ method, or
- a C++ object, derived form lomse::EventHandler.

All Lomse objects that generate events derive from lomse::Observable class. Currently, events can be generated by the following objects:
	- A View. For handling the View events you must register an <tt>event handler</tt> on the Interactor.
	- A Document or a document object (ImoContentObj); and
	- A Control object.

The parameters for the <tt>add_event_handler()</tt> method depends on the type of handler method. There are three possibilities:

a) The handler is a C function:
@code
void add_handler(int eventType, void (*pt2Func)(SpEventInfo event) );
@endcode

b) The handler is a C++ method:
@code
void add_handler(int eventType, void* pThis,
	             void (*pt2Func)(void* pObj, SpEventInfo event) );
@endcode

c) The handler is a C++ object derived form lomse::EventHandler:
@code
void add_handler(int eventType, EventHandler* pHandler);
@endcode

The parameters for these methods are:
- <tt>eventType</tt> is the event type you wish to handle, such as a mouse click.

- <tt>SpEventInfo</tt> is an shared pointer to the event object. All events derive from <tt>lomse::EventInfo</tt> class.


For the C function case:
- <tt>pt2Func</tt> is a pointer to the C function that will handle the event. It expects only one parameter: a shared pointer to the Event object.


For C++ method case:
- <tt>pThis</tt> is a pointer to the object that will handle the event.

- <tt>pt2Func</tt> is a pointer to the member method that will handle the event. It must be an <b>static method</b>. It will receive as first parameter the pointer to the object, so that you can invoke non-static methods if necessary. 

And for the C++ object case:
- <tt>pHandler</tt> is a pointer to an object derived from EventHandler that will handle the event. It must implement method <tt>handle_event()</tt>.


Example:

@code
if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
{
    //connect the View with the window buffer
    spInteractor->set_rendering_buffer(&m_rbuf_window);

    //register to receive some events
    spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
    spInteractor->add_event_handler(k_do_play_score_event, this, wrapper_play_score);
    spInteractor->add_event_handler(k_pause_score_event, this, wrapper_play_score);
    spInteractor->add_event_handler(k_stop_playback_event, this, wrapper_play_score);
	...
@endcode


@section handling-requests Handling requests

It is mandatory to handle Lomse requests. For this, you have to set up a callback method (at library initialization). For instance:

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
@endcode

Lomse will make any necessary request by invoking this callback method. Therefore, requests are always handled in a single point in your application. 

When your callback method is invoked, it will receive as parameter an object derived from class Request, containing the information about the required information or platform dependent service. And Lomse process is paused until the user application provides the requested data.

The list of possible Requests is:
- RequestDynamic. While parsing an LDP document, a <tt>(dynamic)</tt> element has been found. Lomse is requesting user application for the dynamic content that must be inserted.
- RequestFont. The font used for an element is not available in Lomse package. Lomse requests the file path for the font.




*/

