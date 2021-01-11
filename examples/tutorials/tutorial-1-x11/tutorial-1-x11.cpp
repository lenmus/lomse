//---------------------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http:unlicense.org>
//---------------------------------------------------------------------------------------

//lomse headers
#include <lomse_doorway.h>
#include <lomse_document.h>
#include <lomse_graphic_view.h>
#include <lomse_interactor.h>
#include <lomse_presenter.h>
#include <lomse_events.h>

// header files required for X11. The order is important:
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

//some additional needed stuff
#include <stdio.h>
#include <stdlib.h>

using namespace lomse;


//---------------------------------------------------------------------------------------
// In this first tutorial we are just going to display an score on the main window.
// Let's define the necessary variables:
//
LomseDoorway    m_lomse;        //the Lomse library doorway
Presenter*      m_pPresenter;	//relates the View, the Document and the Interactor

//the Lomse View renders its content on a bitmap. Here we define some variables
//to manage the bitmap
unsigned char*      m_buf_window = nullptr;     //memory for the bitmap
int                 m_bufWidth, m_bufHeight;    //bitmap size
//some related variables for X11
XImage*             m_ximg_window;      //the image to display
int                 m_depth;            //color depth
Visual*             m_visual;           //X11 Visual to use

//Lomse can manage a lot of bitmap formats and pixel formats. You must
//define the format that you are going to use
int              m_byte_order;	//endian (platform byte ordering)
EPixelFormat     m_format;      //bitmap format
unsigned         m_bpp;         //bits per pixel

//All typical X stuff needed to run the program and the main events handler loop,
//as well as for handling windows:
Display*    m_pDisplay;     //points to the X Server.
int         m_screen;       //refers to which screen of the display to use.
Window      m_window;       //the actual window itself
GC          m_gc;           //And the GC is the graphics context.
Atom        m_close_atom;
XSetWindowAttributes m_window_attributes;

//some additinal variables
bool    m_view_needs_redraw;      //to control when the View must be re-drawn


//---------------------------------------------------------------------------------------
void display_view_content()
{
    if(m_ximg_window == 0) return;

    //copy the view bitmap onto the image
    m_ximg_window->data = (char*)m_buf_window;

    //display the image
    XPutImage(m_pDisplay,
              m_window,
              m_gc,
              m_ximg_window,
              0, 0, 0, 0,
              m_bufWidth,
              m_bufHeight
             );
}

//---------------------------------------------------------------------------------------
void do_update_window()
{
    // Invoking do_update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without neither calling
    // any lomse methods nor generating platform related events (i.e. window on_paint)

    display_view_content();
    XSync(m_pDisplay, false);
}

//---------------------------------------------------------------------------------------
void update_window(SpEventInfo pEvent)
{
    // Callback method for Lomse

    do_update_window();
}

//---------------------------------------------------------------------------------------
void open_document()
{
    //Normally you will load the content of a file. But in this
    //simple example we will create an empty document and define its content
    //from a text string

    //create a document and get the 'presenter'.
    //The Presenter takes care of creating and maintaining all objects
    //and relationships between the document, its views and the interactors
    //to interact with the view
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(k_view_vertical_book,
        "(lenmusdoc (vers 0.0)"
            "(content "
                "(para (txt \"Hello world!\"))"
                "(score (vers 1.6) "
                    "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q) )))"
            ")"
        ")",
        Document::k_format_ldp);

    //get the pointer to the interactor and register for receiving desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        spInteractor->add_event_handler(k_update_window_event, update_window);
    }
}

//---------------------------------------------------------------------------------------
void update_view_content()
{
    //request the view to re-draw the bitmap

    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        spInteractor->set_rendering_buffer(m_buf_window, m_bufWidth, m_bufHeight);
        spInteractor->redraw_bitmap();
    }
}

//---------------------------------------------------------------------------------------
bool determine_suitable_bitmap_format()
{
    //Returns false if we can not find a suitable bitmap format
    //for this platform

    //determine color depth
    m_depth  = XDefaultDepth(m_pDisplay, m_screen);
    m_visual = XDefaultVisual(m_pDisplay, m_screen);
    unsigned long r_mask = m_visual->red_mask;
    unsigned long g_mask = m_visual->green_mask;
    unsigned long b_mask = m_visual->blue_mask;

    if(m_depth < 15 || r_mask == 0 || g_mask == 0 || b_mask == 0)
    {
        fprintf(stderr,
               "There's no Visual compatible with minimal Lomse requirements:\n"
               "At least 15-bit color depth and TrueColor or DirectColor class.\n\n");
        XCloseDisplay(m_pDisplay);
        return false;
    }

    //determine byte ordering
    int t = 1;
    int hw_byte_order = LSBFirst;
    if(*(char*)&t == 0)
        hw_byte_order = MSBFirst;

    //use mask to determine the bitmap format to use
    switch(m_depth)
    {
        case 15:
            m_bpp = 16;
            if(r_mask == 0x7C00 && g_mask == 0x3E0 && b_mask == 0x1F)
            {
                m_format = k_pix_format_rgb555;
                m_byte_order = hw_byte_order;
            }
            break;

        case 16:
            m_bpp = 16;
            if(r_mask == 0xF800 && g_mask == 0x7E0 && b_mask == 0x1F)
            {
                m_format = k_pix_format_rgb565;
                m_byte_order = hw_byte_order;
            }
            break;

        case 24:
        case 32:
            m_bpp = 32;
            if(g_mask == 0xFF00)
            {
                if(r_mask == 0xFF && b_mask == 0xFF0000)
                {
                    switch(m_format)
                    {
                        case k_pix_format_rgba32:
                            m_format = k_pix_format_rgba32;
                            m_byte_order = LSBFirst;
                            break;

                        case k_pix_format_abgr32:
                            m_format = k_pix_format_abgr32;
                            m_byte_order = MSBFirst;
                            break;

                        default:
                            m_byte_order = hw_byte_order;
                            m_format =
                                (hw_byte_order == LSBFirst) ?
                                k_pix_format_rgba32 :
                                k_pix_format_abgr32;
                            break;
                    }
                }

                if(r_mask == 0xFF0000 && b_mask == 0xFF)
                {
                    switch(m_format)
                    {
                        case k_pix_format_argb32:
                            m_format = k_pix_format_argb32;
                            m_byte_order = MSBFirst;
                            break;

                        case k_pix_format_bgra32:
                            m_format = k_pix_format_bgra32;
                            m_byte_order = LSBFirst;
                            break;

                        default:
                            m_byte_order = hw_byte_order;
                            m_format =
                                (hw_byte_order == MSBFirst) ?
                                k_pix_format_argb32 :
                                k_pix_format_bgra32;
                            break;
                    }
                }
            }
            break;
    }

    //if no suitable format found, terminate
    if(m_format == k_pix_format_undefined)
    {
        fprintf(stderr,
               "RGB masks are not compatible with Lomse pixel formats:\n"
               "R=%08lx, R=%08lx, B=%08lx\n", r_mask, g_mask, b_mask);
        XCloseDisplay(m_pDisplay);
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------
bool create_rendering_buffer(unsigned width, unsigned height, unsigned flags)
{
    //allocate memory for the bitmap, fill it with 1's
    m_bufWidth = width;
    m_bufHeight = height;
    m_buf_window = new unsigned char[width * height * (m_bpp / 8)];
    memset(m_buf_window, 255, width * height * (m_bpp / 8));

    //create an X11 image using the allocated memory as buffer
    m_ximg_window = XCreateImage(m_pDisplay,
                                 m_visual,
                                 m_depth,
                                 ZPixmap,
                                 0,
                                 (char*)m_buf_window,
                                 width,
                                 height,
                                 m_bpp,
                                 width * (m_bpp / 8)
                                );
    m_ximg_window->byte_order = m_byte_order;

    //raise flag to redraw window when requested
    m_view_needs_redraw = true;

    return true;
}

//---------------------------------------------------------------------------------------
void delete_rendering_buffer()
{
    delete [] m_buf_window;
    if (m_ximg_window)
    {
        m_ximg_window->data = 0;
        XDestroyImage(m_ximg_window);
    }
}

//---------------------------------------------------------------------------------------
// application main events handler loop
int handle_events()
{
    XFlush(m_pDisplay);

    bool quit = false;
    while(!quit)
    {
        if(m_view_needs_redraw)
        {
            update_view_content();
            do_update_window();
            m_view_needs_redraw = false;
        }

        XEvent event;
        XNextEvent(m_pDisplay, &event);

        switch(event.type)
        {
            //--------------------------------------------------------------------
            case ConfigureNotify:
            {
                if(event.xconfigure.width  != m_bufWidth ||
                   event.xconfigure.height != m_bufHeight)
                {
                    int width  = event.xconfigure.width;
                    int height = event.xconfigure.height;
                    delete_rendering_buffer();
                    create_rendering_buffer(width, height, 0);
                    do_update_window();
                }
                break;
            }

            //--------------------------------------------------------------------
            case Expose:
                if (event.xexpose.count == 0)
                {
                    display_view_content();
                    XFlush(m_pDisplay);
                    XSync(m_pDisplay, false);
                }
                break;

            //--------------------------------------------------------------------
            case ClientMessage:
                if(event.xclient.format == 32
                   && event.xclient.data.l[0] == int(m_close_atom) )
                {
                    quit = true;
                }
                break;
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------
void define_events_to_process()
{
    XSelectInput(m_pDisplay, m_window, ExposureMask | StructureNotifyMask );
}

//---------------------------------------------------------------------------------------
void create_main_window(unsigned width, unsigned height)
{
    memset(&m_window_attributes, 0, sizeof(m_window_attributes));
    m_window_attributes.border_pixel = XBlackPixel(m_pDisplay, m_screen);
    m_window_attributes.background_pixel = XWhitePixel(m_pDisplay, m_screen);
    m_window_attributes.override_redirect = 0;

    unsigned long window_mask = CWBackPixel | CWBorderPixel;

    m_window = XCreateWindow(m_pDisplay,
                             XDefaultRootWindow(m_pDisplay),
                             0, 0,
                             width, height,
                             0,
                             m_depth,
                             InputOutput,
                             CopyFromParent,
                             window_mask,
                             &m_window_attributes
                            );

    //set window title
    XSetStandardProperties(m_pDisplay,
                           m_window,
                           "Lomse tutorials. Tutorial 1",     //window title
                           "Lomse_1",                       //name in tasks bar
                           None,
                           nullptr,
                           0,
                           nullptr
                          );


    // create the Graphics Context
    m_gc = XCreateGC(m_pDisplay, m_window, 0, 0);

}

//---------------------------------------------------------------------------------------
bool init_x()
{
    //returns false if an error occurs

    //create X connection
    m_pDisplay = XOpenDisplay(nullptr);
    if(m_pDisplay == 0)
    {
        fprintf(stderr, "Unable to open DISPLAY!\n");
        return false;
    }

    m_screen = XDefaultScreen(m_pDisplay);

    //As lomse renders on a bitmap it is necessary to determine the best
    //bitmap format suited for your specific OS and platform
    if (!determine_suitable_bitmap_format())
        return false;

    create_main_window(850, 600);       //850 x 600 pixels
    create_rendering_buffer(850, 600, 0);

    XMapWindow(m_pDisplay, m_window);

    m_close_atom = XInternAtom(m_pDisplay, "WM_DELETE_WINDOW", false);

    XSetWMProtocols(m_pDisplay, m_window, &m_close_atom, 1);
    return true;        //no error
};

//---------------------------------------------------------------------------------------
void close_x()
{
    XFreeGC(m_pDisplay, m_gc);
    XDestroyWindow(m_pDisplay,m_window);
    XCloseDisplay(m_pDisplay);
};

//---------------------------------------------------------------------------------------
void initialize_lomse()
{
    //initialize the Lomse library
    bool flip_y = false;               //y axis is not reversed
    m_lomse.init_library(m_format, 96, flip_y);   //resolution=96 ppi

    //initialize lomse related variables
    m_pPresenter = nullptr;
}

//---------------------------------------------------------------------------------------
// application entry point
int main ()
{
    if (!init_x())
        exit(1);

    initialize_lomse();

    //create a music score and a View. The view will display the score
    //when the paint event is sent to lomse, once the main windows is
    //shown and the event handling loop is started
    open_document();

	//run the main events handling loop
	define_events_to_process();
    handle_events();

    //delete the view and the rendering buffer
    delete_rendering_buffer();
    delete m_pPresenter;    //this will also delete the Doc, the Views and all other stuff

    //close X connection
    close_x();

    return 0;
}
