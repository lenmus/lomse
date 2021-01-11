/**
@page page-render-overview Rendering documents overview

@tableofcontents


@section mvc-rendering How to render a document

The first and most important thing to learn about Lomse is that is platform independent code, with no knowledge about things such as how to display a document on a window on the screen or how to handle mouse events. Lomse provides the necessary services and interfaces for displaying documents and interacting with them but it is your application responsibility to code the presentation layer, that is the methods and functions for asking for services to the operating system (e.g. creating windows, receiving mouse events, etc.) and for requesting Lomse the appropiate services, such as rendering the document in the windows buffer or handling the passed events.

Lomse works by rendering the documents on a bitmap buffer, that is, on an array of consecutive memory bytes. This buffer can be any type of memory, such as a real bitmap, a window's buffer, etc. The simplest and usual way of rendering documents on a window is:
    -# Create a new empty bitmap when necessary (i.e when the window is created or resized),
    -# Ask Lomse to render the desired portion of the document on this bitmap, and
    -# Copy the bitmap onto the window.

These operations are usually triggered by your application when handling some operating system events, such as <b>window paint</b> events. Before entering into details it is necessary to understand some important concepts. If you are new to Lomse, @b please read in sequence this document.



@section mvc-overview The Lomse Model-View-Controller 

The Model-View-Controller (MVC) is an architecture for providing isolation between the various functions of the GUI: maintaining the document (the Model), displaying all or a portion of the document (the View), and handling events that affect the model or the view(s) (the Controller).

Lomse MVC model has four components: the ``Model`` (the document), the ``View`` (its visual representation), the ``Interactor`` (a kind of Controller) and the ``Presenter`` (the 'glue' to join all the pieces):

- The Document object (the Model) stores the document content (music scores, paragraphs, images, etc.) and notifies other objects when changes occur to the document. 

- The View object takes care of rendering the document so that it can be displayed by your application. The %View is responsible for providing the rendered document, usually by providing a bitmap, and it is your application responsibility to present this bitmap to the user (i.e. render it on a window, save it in a file, print it, or produce any other desired output). The rendering type depends on the specific %View class used. For instance, class GraphicView renders the document on a bitmap. Other classes would be possible (i.e. a view class to render the document as an SVG stream, a view for rendering as Braille code, a view for rendering as source code, etc.) but currently Lomse only has implemented several variations of GraphicView (VerticalBookView, HorizontalBookView, and SingleSystemView).

- A Document can have many View objects. For instance, your application can display two windows, one for presenting a music score as a music sheet, and another window for displaying the same music score but as MusicXML source code. This behaviour can be achived by associating two simultaneous views to the document.

- The Interactor object plays the role of the Controller in the MVC model. Each %View has an associated %Interactor (in fact the %View is owned by the %Interactor). The %Interactor is the interface between your application, the associated %View and the %Document. It is responsible for translating your application requests into commands that manipulate the associated %View and/or the %Document, coordinating all the necessary actions.

- Finally, the Presenter object (short for <i>document presenter</i>) is the glue that links all objects in the MVC model. It maintains the life cycle and relationships between %Views, %Interactors, %Commands, %Selections, and the %Document.

The %Presenter and most associated objects are created when your application invokes any of the methods in LomseDoorway for opening/creating documents:

@code
    LomseDoorway    m_lomse;
    Presenter*      m_pPresenter = m_lomse.new_document(...);
@endcode

Your application will take ownership of the %Presenter and will have to delete it when no longer needed. Deleting the %Presenter will automaticall cause deletion of all MVC involved objets: the %Document, all existing %Views and their %Interactors, selection sets, undo/redo stacks, etc.

By default, when the presented is created a View and its Interactor are created. Method Presenter::get_interactor() provides a [smart pointer](https://en.wikipedia.org/wiki/Smart_pointer) to the desired %Interactor:

@code
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        //use the Interactor
        spInteractor->some_method();
        ...
    }
@endcode


Normally, the %Interactor is the only object you would need to use for interacting with the associated %View and with the %Document.




@section rendering-buffer  The rendering buffer for the View

The View renders the document on a memory buffer that your application must provide. Once Lomse has rendered the document on this bitmap buffer, it is your application responsibility to do whatever is needed with the bitmap: rendering it on a window, exporting it as a file, printing it, etc.

Once the View is created (remember that it is created automatically when your application invokes any of the methods in LomseDoorway for opening/creating documents), and before processing any paint event, it is necessary to inform Lomse about the rendering buffer to use. The memory for this buffer must be allocated when necessary, normally when the window is created and each time it is resized. Therefore, the event handler for <b>window resize</b> events is, usually, the best place for allocating memory for the buffer and informing Lomse about the new buffer:

@code
//some MyWindow class member variables:
LomseDoorway&       m_lomse;            //the Lomse library doorway
Presenter*          m_pPresenter;       //the Presenter for this window
unsigned char*      m_pBuffer;          //ptr to the bitmap memory
bool                m_renderView;       //the score needs to be rendered on the bitmap

MyWindow::MyWindow(LomseDoorway& lomse)
    : m_lomse(lomse)
    , m_pPresenter(nullptr)
    , m_pBuffer(nullptr)
    , m_renderView(false)
{
}

void MyWindow::on_new_document()

{
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(....);
    if (!m_pBuffer)
        create_rendering_buffer();
}

void MyWindow::handle_resize_event()
{
    create_rendering_buffer();
}

void MyWindow::create_rendering_buffer()

{
    //determine current window size
    int width = ...
    int height = ...

    //create a new bitmap for the rendering buffer.
    delete m_pBuffer;
    m_pBuffer = ...   //many possibilities for allocating memory or reusing memory

    //use this bitmap as Lomse rendering buffer
    if (m_pPresenter)
    {
        if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            spInteractor->set_rendering_buffer(m_pBuffer, width, height);
    
        m_renderView = true;
    }
}

@endcode

Probably you have noticed that the format of the bitmap (i.e., bytes per pixel, byte ordering, etc.) has not been specified in previous code. The place for doing it is at Lomse library initialization (see next section).

Deciding the how to allocate memory for the rendering buffer and the bitmap format to use are the most critical decisions when using Lomse. Depending on your application operating system and on the application framework used for coding it, the solution is different.




@section page-render-overview-init-lomse Lomse library initialization

The first step is to include the needed headers. At the time of writing this the Lomse API is not yet fixed; therefore there is not a single header file (or set of headers) to include. Instead, the headers to include will depend on the classes and functions you would like to use. Anyway, with current API you will always include:

@code
#include <lomse_doorway.h>
using namespace Lomse;
@endcode

The LomseDoorway object is the access point to the Lomse library and the main interface with the library. It must be used by your application at two points:

  -# Before using the Lomse library it is necessary to initialize it. This means setting up certain global options about rendering and events handling.

  -# Later, your application has to use it for:
    - Opening and creating Document objects.
    - Accessing global objects and variables in Lomse library.

You need to define and create an instance of LomseDoorway, usually with global scope:

@code
LomseDoorway&   m_lomse;        //the Lomse library doorway
@endcode

The most important aspect to consider to initialize Lomse is the format of the images to be generated. As Lomse renders music scores on a bitmap it is necessary to inform Lomse about:

-# the bitmap format to use,
-# the resolution to use (pixels per inch), and
-# the y-axis orientation.

Lets start with the y-axis orientation. Lomse needs to know if your presentation device follows the standard convention used in screen displays in which the y coordinates increases downwards, that is, y-axis coordinate 0 is at top of screen and increases downwards to bottom of screen. This convention is just the opposite of the normal convention for geometry, in which 0 coordinate is at bottom of paper and increases upwards. Lomse follows the standard convention used in displays (y-axis 0 coordinate at top and increases downwards). If your application is going to display the documents on screen then the standard convention is follwed and you don't have to reverse the y-axis. Otherwise, you will have to set this parameter as necessary.

Also Lomse needs to know the resolution to use. If you the scores are going to be displayed on screen, you should use the appropriate screen resolution for the intended device. A value of 96ppi is typical for Linux and Windows systems. But, probably you should get this value by invoking some operating system related methods (i.e. wxDC::GetPPI() method, in wxWidgets framework).

The next and <b>most important</b> decision is how your application will allocate memory for the rendering buffer and the bitmap format to use, and how this bitmap will be rendered (or converted to a image format to be exported to a file or embedded in a document). Depending on your application operating system and on the application framework used for coding it, the solution is different. You should take these decissions by analyzing the most convenient and fast method for rendering the bitmaps and to avoid format conversions. Sometimes the options are very limited.

One you have decided on the values to use, the code to write is simple:

With this, we have finished Lomse initialization. Here is the full code:

@code
void MyApp::initialize_lomse()
{
    //the pixel format, e.g.: ARGB 32bits
    int pixel_format = k_pix_format_argb32;

    //the desired resolution, e.g.: 96 pixels per inch
    int resolution = 96;

    //For most systems y axis direction is 0 coordinate at top and increases
    //downwards. This is the this the assumed behaviour unless you 
    // specify 'reverse_y_axis = true'
    bool reverse_y_axis = false;    //y increases downwards

    //initialize the library with these values
    m_lomse.init_library(pixel_format, resolution, reverse_y_axis);
}
@endcode

@attention Two important points:
-# The library must be always initialized, even if your application will not use Lomse to render scores, e.g.: uses it only for playback or other. In these cases any values for pixel format, resolution and reverse_y_axis will be valid. But your application will have to invoke the init_library() method. 
-# The library can be safely re-initialized if you would like to change currently defined values.

At end of this chapter there are summary cards with information about using Lomse in different frameworks and operating systems. See page @ref page-examples for full application code samples.



@section rendering-display Displaying the document

Once a document is open and the rendering buffer for the View is created, all your application has to do is to:
-# ask Lomse to render the desired portion of the document on the rendering buffer, and
-# copy the rendered bitmap onto the window.

These operations are usually triggered in the handler for <b>window paint</b> events. 
Notice that there is no need to ask Lomse to paint the bitmap whenever a paint event arrives. These events are generated due to different reasons. The most frequent is when the window image is damaged (i.e. another window that was covering our window has moved). But in these cases the image is preserved in the bitmap so it is enough to re-display the bitmap. Other cases for receiving paint events are because the window has changed: when the window is created or when it is resized or when your application changes its content (i.e. because the user has open a different document). Flag <i>m_renderView</i> is defined in your application for controlling the need to repaint the buffer: do it only when the repaint event is caused by a window resize or because the application has changed the content of the document; otherwise the Lomse buffer is still valid and you can save time by skipping to ask Lomse for a repaint:


@code
void MyWindow::handle_paint_event(DeviceContext* paintDC)
{
    //ensure that the bitmap has the right content or repaint it
    update_rendering_buffer_if_needed();

    //copy the bitmap to the window.
    //AWARE: Platform dependent code. This is just an example 
    //       using generic methods. Adapt this code before using
    //       it in a real application.
    paintDC->BeginPaint();
    paintDC->DrawBitmap(m_pBuffer, ...);
    paintDC->EndPaint();
}

void MyWindow::update_rendering_buffer_if_needed()
{
    //request Lomse to re-draw the bitmap

    if (m_pPresenter != nullptr && m_renderView)
    {
        if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            spInteractor->force_redraw();
        m_renderView = false;
    }
}
@endcode




@section page-render-overview-viewtypes View types

As said, Lomse uses a Model-View-Controller architecture. This means that, internally, Lomse maintains a graphic representation of the document defined by the chosen view type. Lomse has several view types:

<b>Vertical Book View</b>. View type <i>k_view_vertical_book</i> is a view oriented to display the document in pages, with the
pages spread in vertical, one page after the other in a vertical layout. The user will have to scroll down for advancing.
This is the typical view used in e.g. Adobe PDF Reader and MS Word.

@image html view-vertical-book.png "Image: The 'real world' when using a 'k_view_vertical_book' View"


<b>Horizontal Book View</b>. View type <i>k_view_horizontal_book</i> means that the document will be displayed in pages, with the
pages spread in horizontal, one page after the other in a horizontal layout. The user will have to scroll right for advancing.
This is the typical view used to display scores in e.g. Finale or Sibelius.

@image html view-horizontal-book.png "Image: The 'real world' when using a 'k_view_horizontal_book' View"


<b>Single System View</b>. View type <i>k_view_single_system</i> is for rendering documents that only contain one score (e.g. LDP files, LMD files with just one score, and score files imported from other formats such as MusicXML). It will display the score in a single system, as if the paper had infinite width. And for viewing the end of the score the user will have to scroll to the right.

@image html view-single-system.png "Image: The 'real world' when using a 'k_view_single_system' View"


<b>Single Page View</b>. View type <i>k_view_single_page</i> is similar to an HTML page having a body of fixed width. All the
document is rendered in a single page having the required height to contain the full document. Is a kind of %k_view_vertical_book
but without gaps in the content for separting pages. As with %k_view_vertical_book the user will have to scroll down for advancing.

@image html view-single-page.png "Image: The 'real world' when using a 'k_view_single_page' View"


<b>Free Flow View</b>. View type <i>k_view_free_flow</i> is for rendering documents in a single page as high
as necessary. It is similar to a VerticalBookView but using a paper size of infinite
height, so that only paper width is meaningful and the document has just one page
(e.g., an HTML page with unconstrained body width)

@image html view-free-flow.png "Image: The 'real world' when using a 'k_view_free_flow' View"


<b>Half Page View</b>. View type <i>k_view_half_page</i> has a double behaviour. In normal mode (no playback) it behaves as
SinglePageView, that is the score is rendered on a single page as high as necessary
to contain all the score (e.g., an HTML page having a body of fixed size).
But when in playback mode, the bitmap to be rendered in the application window is
split horizontally in two halves, originating two virtual vertical windows, one at
top and the other at bottom. This view allows to solve the problem page turning and the problem of repetition
marks and jumps during playback. See HalfPageView for more details.






@section page-render-overview-control Controlling what is displayed

When Lomse renders the document, it uses a graphic representation of the document defined by the chosen view type. When requesting Lomse to render the document onto your application window it is not expected that Lomse will squeeze all the document pages into that window, but just the specific part of the document that the user wants to visualize, as it is expected that the users can pan and zoom to see different areas of the document.

The portion of the document that is rendered on the bitmap is controlled by the viewport and the scale that your application defines (see @ref page-coordinates-viewport). Initialy, the viewport is set at the top left corner of the view, and it width and height is defined by the bitmap size, scaled by the current scaling factor (initially 1.0).

@image html viewport.png "Image: The 'viewport' and the 'device window'"


Your application can use %Interactor specific methods for changing the viewport so that the user can choose what to display and to implement scrolling. 
See: Interactor::new_viewport(), Interactor::set_viewport_at_page_center(), Interactor::get_viewport(), Interactor::get_view_size().

Also there are methods for adjusting the scaling factor, so that the user can zoom in and out, or adjust the scale so that a whloe page fits in the display. See: Interactor::get_scale(), Interactor::set_scale(), Interactor::zoom_in(), Interactor::zoom_out(), Interactor::zoom_fit_full(), Interactor::zoom_fit_width().

As these concepts are common and widely used in computer graphics theory, I will not enter here into more details, but your application have full control of what to render on the bitmap passed to Lomse as rendering buffer.

You have also the possibility of defining a view clip area so that only part of the passed rendering buffer will be used by Lomse. See Interactor::set_view_area().




@section page-render-overview-tips Tips for several OSs/frameworks

Here you can find summary cards with information about using Lomse in different frameworks and operating systems. See page @ref page-examples for full application code samples. I would appreciate if you could help me to improve this documentation. Open an issue in lomse repo or send a PR (the source of this document is at 'lomse/docs/api/mainpages/render-overview.h'). Thank you.


@subsection page-render-overview-qt Using Lomse in Qt

Qt is a free and open-source widget toolkit for creating graphical user interfaces as well as cross-platform applications that run on various software and hardware platforms such as Linux, Windows, macOS, Android or embedded systems. Looking at Qt documentation, it seems that to render an image on screen, a good approach is to use a QImage object as it can be created from a raw bitmap without the need of image format conversions if we choose the right format. To render on screen I have tested bitmaps in RGBA, 32 bits format (Lomse format k_pix_format_rgba32). This is a summary:

@code
Application framework:  Qt
Operating system:       any

Lomse library initialization
---------------------------------------
    //Resolution: 96 pixels per inch
	int resolution = 96;
	
	//pixel format RGBA, 32 bits 
	int pixel_format = k_pix_format_rgba32;

	//Lomse default y axis direction is 0 coordinate at top and increases
	//downwards. For Qt the Lomse default behaviour is the right behaviour.
	bool reverse_y_axis = false;

	//initialize the Lomse library with these values
	m_lomse.init_library(pixel_format, resolution, reverse_y_axis);


Creating the rendering buffer
------------------------------
    //Memory for the bitmap:    just raw memory
    //Object to manage it:      QImage,  Format_RGBA8888

    #define BYTES_PER_PIXEL 4       //using RGBA, 32 bits 
    m_pdata = (unsigned char*)malloc(width * height * BYTES_PER_PIXEL);


Paint event
-----------------
void MyWindow::paintEvent(QPaintEvent* event)
{
	if (m_presenter && m_pdata)
	{
        QPainter painter(this);
        QImage image(m_pdata, m_bufWidth, m_bufHeight, QImage::Format_RGBA8888);
        QRect dirtyRect = event->rect();
        painter.drawImage(dirtyRect, image, dirtyRect);
	}
}
@endcode



@subsection page-render-overview-wxwidgets Using Lomse in wxWidgets

wxWidgets is a free and open-source widget toolkit for creating graphical user interfaces as well as cross-platform applications that run on Linux, Windows, and macOS. For wxWidgets applications I've found that using a wxImage as bitmap buffer is a good strategy, as <tt>wxImage</tt> is a platform independent class and contains a buffer for a bitmap in RGB, 24 bits format. This is a summary of how to use:

@code
Application framework:  wxWidgets
Operating system:       any

Lomse library initialization
---------------------------------------
    //Resolution: 96 pixels per inch, or use wxDC::GetPPI() to get it
	int resolution = 96;
	
	//pixel format RGB 24bits 
	int pixel_format = k_pix_format_rgb24;

	//Lomse default y axis direction is 0 coordinate at top and increases
	//downwards. For wxWidgets the Lomse default behaviour is the right behaviour.
	bool reverse_y_axis = false;

	//initialize the Lomse library with these values
	m_lomse.init_library(pixel_format, resolution, reverse_y_axis);


Creating the rendering buffer
------------------------------
    //Memory for the bitmap:    the internal bitmap of a wxImage
    //Object to manage it:      wxImage

    // allocate a new rendering buffer
    delete m_buffer;
    m_buffer = new wxImage(width, height);


Paint event
-----------------
void MyWindow::paintEvent(wxPaintEvent& event)
{
	if (m_presenter && m_buffer)
	{
        wxPaintDC dc(this);
        wxBitmap bitmap(*m_buffer);
        dc.DrawBitmap(bitmap, 0, 0, false); //false = don't use mask
	}
}
@endcode



@subsection page-render-overview-juce Using Lomse in JUCE

JUCE is a partially open-source cross-platform C++ application framework, used for the development of desktop and mobile applications that run on various software and hardware platforms such as Linux, Windows, macOS and Android. I have not tested it but Lomse users report that the following configuration works well:

@code
Application framework:  JUCE
Operating system:       any

Lomse library initialization
---------------------------------------
    //Resolution: 96 * UI scale
	m_scale = m_settings.zoomUi * Desktop::getInstance().getDisplays().getMainDisplay().scale;
	int resolution = int(96 * m_scale);
	
	//pixel format
	int pixel_format = k_pix_format_rgba32;

	//Lomse default y axis direction is 0 coordinate at top and increases
	//downwards. For JUCE the Lomse default behaviour is the right behaviour.
	bool reverse_y_axis = false;

	//initialize the Lomse library with these values
	m_lomse.init_library(pixel_format, resolution, reverse_y_axis);


Creating the rendering buffer
------------------------------
    //Memory for the bitmap:    juce::Image internal BitmapData
    //Object to manage it:      juce::Image,  PixelFormat::ARGB

	//create image
	m_image.reset(new juce::Image(juce::Image::PixelFormat::ARGB, width, height,
                                  false, SoftwareImageType()));
		
	//creates a bitmap of specified size
	juce::Image::BitmapData bitmap(*m_image, juce::Image::BitmapData::readWrite);


Paint event
-----------------
void MyWindow::paint(Graphics& g)
{
	if (m_presenter && m_image)
	{
		g.drawImage(*m_image, 0, 0, getWidth(), getHeight(), 0, 0, m_image->getWidth(), m_image->getHeight());
	}
	else
	{
		String text = "Some error message";
		juce::Rectangle<int> rec(20, 80, getWidth() - 40, getHeight() - 100);
		g.drawFittedText(text, rec, Justification::centredTop, 100, 1);
	}
}
@endcode



@subsection page-render-overview-x11 Using Lomse in X11

The X Window System (X11, or simply X) is a windowing system for bitmap displays, common on Unix-like operating systems. To decide which bitmap format we are going to use it is possible to choose a common, widely supported format, such as bitmaps in RGBA format, 8 bits per pixel. But for your convenience, the code in tutorial-1-x11 includes a function "determine_suitable_bitmap_format()" for selecting a suitable bitmap format by determining the available X11 Visuals. This function has been borrowed from examples in AGG project and sets global variables <i>m_depth</i> (the color depth to use), <i>m_visual</i> (the X11 Visual to use), <i>m_format</i> (a Lomse enum describing the bitmap format) and <i>m_byte_order</i> (the endian or byte ordering for this platform).

@code
Framework:          The X Window System (X11)
Operating system:   Unix-like operating systems

Lomse library initialization
---------------------------------------
    //Resolution: 96 pixels per inch, or use wxDC::GetPPI() to get it
	int resolution = 96;
	
	//pixel format: choose one from the available X11 Visuals
    determine_suitable_bitmap_format();
	int pixel_format = m_format;

	//Lomse default y axis direction is 0 coordinate at top and increases
	//downwards. For X11 the Lomse default behaviour is the right behaviour.
	bool reverse_y_axis = false;

	//initialize the Lomse library with these values
	m_lomse.init_library(pixel_format, resolution, reverse_y_axis);


Creating the rendering buffer
------------------------------
    //Memory for the bitmap:    just raw memory
    //Object to manage it:      XImage

    //allocate memory for the bitmap, fill it with 1's
    m_buffer = new unsigned char[width * height * (m_bpp / 8)];
    memset(m_buffer, 255, width * height * (m_bpp / 8));

    //create an X11 image using the allocated memory as buffer
    m_ximg = XCreateImage(m_pDisplay,
                          m_visual,
                          m_depth,
                          ZPixmap,
                          0,
                          (char*)m_buffer,
                          width,
                          height,
                          m_bpp,
                          width * (m_bpp / 8)
                        );
    m_ximg->byte_order = m_byte_order;


Paint event
-----------------
    //copy the view bitmap onto the image
    m_ximg->data = (char*)m_buffer;

    //display the image
    XPutImage(m_pDisplay,
              m_window,
              m_gc,
              m_ximg,
              0, 0, 0, 0,
              rbuf->width(),
              rbuf->height()
             );
    XSync(m_pDisplay, false);
@endcode



@subsection page-render-overview-windows Using Lomse in MS Windows

My knowledge of using the Microsoft Windows API is nullptr. In other platforms normally your application uses one of the available image objects. Although the Windows API provides many functions for creating and managing bitmaps, as my lack of knowledge about Windows, I opted to to some tests by borrwing code from the AGG project, instead of finding documentation and studying how to use the Windows API functions. So, I choose to create a <tt>Bitmap</tt> class, enclosing the necessary methods and knowledge in it. See tutorial 1 for Windows. If you have good knowledge of the Windows API probably you would prefer a different solution for managing bitmaps. In that case, I would appreciate if you could help me to improve this documentation. Open an issue in lomse repo or send a PR (the source of this document is at 'lomse/docs/api/mainpages/render-overview.h'). Thank you. This is a summary of how I used it in the tutorials and samples:

@code
Operating system:       Microsoft Windows
Memory for the bitmap:  ad-hoc bitmap class (see code in tutorial-1-win)


Lomse library initialization
---------------------------------------
    //Resolution: 96 pixels per inch
	int resolution = 96;
	
	//pixel format BGRA, 32 bits
	int pixel_format = k_pix_format_bgra32;

	//Lomse default y axis direction is 0 coordinate at top and increases
	//downwards. For MS Windows the Lomse default behaviour is the right behaviour.
	bool reverse_y_axis = false;

	//initialize the Lomse library with these values
	m_lomse.init_library(pixel_format, resolution, reverse_y_axis);


Creating the rendering buffer
------------------------------
    //Memory for the bitmap:    just raw memory. Allocated in ad-hoc Bitmap class
    //Object to manage it:      ad-hoc Bitmap class

    m_bitmap.create(width, height, m_bpp);


Paint event
-----------------
    case WM_PAINT:
    {
        update_rendering_buffer_if_needed();

        PAINTSTRUCT ps;
        HDC paintDC = ::BeginPaint(m_hWnd, &ps);
        m_bitmap.draw(paintDC);
        ::EndPaint(m_hWnd, &ps);
        break;
    }
@endcode


Please help me to improve this documentation and to add information for other platforms and operating systems. Open an issue in lomse repo or send a PR (the source of this document is at 'lomse/docs/api/mainpages/render-overview.h'. Thank you.

*/

