/**
@page render-overview Rendering documents overview

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

- The View object takes care of rendering the document so that it can be displayed by your application. The %View is responsible for providing the rendered document, usually by providing a bitmap, and it is your application responsibility to present this bitmap to the user (i.e. render it on a window, save it in a file, print it, or produce any other desired output). The rendering type depends on the specific %View class used. For instance, class GraphicView renders the document on a bitmap. Other classes would be possible (i.e. a view class to render the document as an SVG stream, a view for rendering as Braille code, a view for rendering as source code, etc.) but currently Lomse only has implemented several variations of GraphicView (VerticalBookView, HorizontalBookView, and SimpleView).

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

Once the View is created (remember that it is created automatically when your application invokes any of the methods in LomseDoorway for opening/creating documents), the next step is to associate a RenderingBuffer object to the View. This object is just a wrapper for the real memory buffer:

@code
//some MyWindow class member variables:
LomseDoorway&       m_lomse;            //the Lomse library doorway
Presenter*          m_pPresenter;       //the Presenter for this window
RenderingBuffer     m_rbuf;             //the RenderingBuffer for this window
unsigned char*      m_pBuffer;          //ptr to the real memory for the bitmap

MyWindow::MyWindow(LomseDoorway& lomse)
    : m_lomse(lomse)
    , m_pPresenter(NULL)
    , m_pBuffer(NULL)
{
}

void MyWindow::on_new_document()

{
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(....);

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        //connect the View with the rendering buffer
        spInteractor->set_rendering_buffer(&m_rbuf);
    }

}
@endcode

Notice that the real memory for the buffer is not yet allocated. The memory must be allocated when necessary, normally when the window is created and each time it is resized. Therefore, the event handler for <b>window resize</b> events is, usually, the best place for allocating memory for the buffer:

@code
void MyWindow::handle_resize_event(unsigned new_width, unsigned new_height)
{
    //create a new bitmap for the rendering buffer. The old buffer is
    //automatically deleted by Lomse
    m_pBuffer = ...   //many possibilities for allocating memory or reusing memory
    int stride = width * bytes_per_pixel;   //bitmap: number of bytes per row
    m_rbuf_window.attach(m_pBuffer, width, height, stride);

    m_view_needs_redraw = true;
}
@endcode

Probably you have noticed that the format of the bitmap (i.e., bytes per pixel, byte ordering, etc.) has not yet been specified. The place for doing it is at Lomse library initialization (see next section).

Deciding the how to allocate memory for the rendering buffer and the bitmap format to use are the most critical decisions when using Lomse. Depending on your application operating system and on the application framework used for coding it, the solution is different.




@section init-lomse Initializing the Lomse library

At Lomse library initialization it is neccesary to specify three mandatory values:

-# the bitmap format to use,
-# the resolution to use (pixels per inch), and
-# the y-axis orientation.

Lets start with the y-axis orientation. Lomse needs to know if your presentation device follows the standard convention used in screen displays in which the y coordinates increases downwards, that is, y-axis coordinate 0 is at top of screen and increases downwards to bottom of screen. This convention is just the opposite of the normal convention for geometry, in which 0 coordinate is at bottom of paper and increases upwards. Lomse follows the standard convention used in displays (y-axis 0 coordinate at top and increases downwards). If your application is going to display the documents on screen then the standard convention is follwed and you don't have to reverse the y-axis. Otherwise, you will have to set this parameter as necessary.

The next decision is how your application will allocate memory for the rendering buffer and the bitmap format to use. Depending on your application operating system and on the application framework used for coding it, the solution is different. You should take these decissions by analyzing the most convenient and fast method for rendering the bitmaps. Sometimes the options are very limited.

Here are some tested configurations that work (see <a href="examples.html">Examples</a> for full application samples):

| Application framework | Operating system | Memory for the bitmap | Bitmap format           | Lomse bitmap format |
|-----------------------|------------------|-----------------------|-------------------------|---------------------|
| Qt                    | any              | a QImage object       | QImage::Format_RGBA8888 | k_pix_format_rgba32 |
| wxWidgets             | any              | a wxImage object      | RGB 24 bits per pixel   | k_pix_format_rgb24  |
| X11                   | Linux            | a XImage object       | many suitable           | many suitable       |
| native Win32 API      | MS Windows       | a bitmap              | BGRA, 32 bits per pixel | k_pix_format_bgra32 |

Please inform about any other configuration used by your application, for improving this documentation. Thank you.


For instance, for a Qt application you can use a QImage object as buffer, and use k_pix_format_rgba32:

@code
void MyApp::initialize_lomse()
{
    //the pixel format to use
    int pixel_format = k_pix_format_rgba32;  //QImage::Format_RGBA8888

    //the desired resolution, typically 96 pixels per inch for screen
    int resolution = 96;

    //Lomse default y axis direction is 0 coordinate at top and increases
    //downwards. You must specify if you would like just the opposite behaviour.
    //For MS Windows the Lomse default behaviour is the right behaviour.
    bool reverse_y_axis = false;

    //initialize the Lomse library with these values
    m_lomse.init_library(pixel_format, resolution, reverse_y_axis);
}
@endcode





@section rendering-display Displaying the document

Once a document is open and the rendering buffer for the View is created, all your application has to do is to:
-# ask Lomse to render the desired portion of the document on the rendering buffer, and
-# copy the rendered bitmap onto the window.

These operations are usually triggered in the handler for <b>window paint</b> events. 
Notice that there is no need to ask Lomse to paint the bitmap whenever a paint event arrives. These events are generated due to different reasons. The most frequent is when the window image is damaged (i.e. another window that was covering our window has moved). But in these cases the image is preserved in the bitmap so it is enough to re-display the bitmap. Other cases for receiving paint events are because the window has changed: when the window is created or when it is resized or when your application changes its content (i.e. because the user has open a different document). Flag <i>m_view_needs_redraw</i> is defined in your application for controlling the need to repaint the buffer: do it only when the repaint event is caused by a window resize or because the application has changed the content of the document; otherwise the Lomse buffer is still valid and you can save time by skipping to ask Lomse for a repaint:


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

    if (!m_pPresenter) return;

    if (m_view_needs_redraw)
    {
        if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            spInteractor->force_redraw();
        m_view_needs_redraw = false;
    }
}
@endcode


*/

