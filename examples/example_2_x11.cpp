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

// header files required for X11. The order is important:
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

//some additional needed stuff
#include <stdio.h>
#include <stdlib.h>

//lomse headers
#include <lomse_doorway.h>
#include <lomse_document.h>
#include <lomse_graphic_view.h>
#include <lomse_interactor.h>
#include <lomse_presenter.h>
#include <lomse_events.h>
#include <lomse_tasks.h>

using namespace lomse;


//---------------------------------------------------------------------------------------
// In this first example we are just going to display an score on the main window.
// Let's define the necessary variables:
//
LomseDoorway    m_lomse;        //the Lomse library doorway
Presenter*      m_pPresenter;	//relates the View, the Document and the Interactor

//the Lomse View renders its content on a bitmap. To manage it, Lomse
//associates the bitmap to a RenderingBuffer object.
//It is your responsibility to render the bitmap on a window.
//Here you define the rendering buffer and its associated bitmap to be
//used by the previously defined View.
RenderingBuffer     m_rbuf_window;      //Lomse struct to contain the bitmap
unsigned char*      m_buf_window;       //memory for the bitmap
//some X11 related variables
XImage*             m_ximg_window;      //the image to display
int                 m_depth;            //color depth
Visual*             m_visual;           //X11 Visual to use

//Lomse can manage a lot of bitmap formats and pixel formats. You must
//define the format that you are going to use
int              m_byte_order;
EPixelFormat     m_format;      //bitmap format
unsigned         m_bpp;         //bits per pixel
bool             m_flip_y;      //true if y axis is reversed

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

//for mouse click & move position
int           m_xMouse;
int           m_yMouse;




//---------------------------------------------------------------------------------------
void display_view_content(const rendering_buffer* rbuf)
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
              rbuf->width(),
              rbuf->height()
             );
}

//---------------------------------------------------------------------------------------
void do_update_window()
{
    // Invoking do_update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without neither calling
    // any lomse methods nor generating platform related events (i.e. window on_paint)

    display_view_content(&m_rbuf_window);
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
    //to interct with the view
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(k_view_vertical_book,
        "(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
        "(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData "
        "(clef F4)(key E)(time 2 4)(n +c3 e.)(barline)"
        "(n e2 q)(n e3 q)(barline)"
        "(n f2 e (beam 1 +))(n g2 e (beam 1 -))"
            "(n f3 e (beam 3 +))(n g3 e (beam 3 -))(barline)"
        "(n f2 e. (beam 4 +))(n g2 s (beam 4 -b))"
            "(n f3 s (beam 5 +f))(n g3 e. (beam 5 -))(barline)"
        "(n g2 e. (beam 2 +))(n e3 s (beam 2 -b))(n g3 q)(barline)"
        "(n a2 e (beam 6 +))(n g2 e (beam 6 -))(n a3 q)(barline)"
        "(n -b2 q)(n =b3 q)(barline)"
        "(n xc3 q)(n ++c4 q)(barline)"
        "(n d3 q)(n --d4 q)(barline)"
        "(chord (n e3 q)(n c3 q)(n g3 q))"
        "(n e4 q)(barline)"
        "(n f3 q)(n f4 q)(barline end)"
        "))"
        "(instrument (name \"piano\")(abbrev \"P.\")(staves 2)(musicData "
        "(clef G p1)(clef F4 p2)(key F)(time 12 8)"
        "(n c5 e. p1)(barline)"
        "(n e4 e p1 (beam 10 +))(n g3 e p2 (beam 10 -))"
        "(n e4 e p1 (stem up)(beam 11 +))(n e5 e p1 (stem down)(beam 11 -))(barline)"
        "(n e4 s p1 (beam 12 ++))(n f4 s p1 (beam 12 ==))"
            "(n g4 s p1 (beam 12 ==))(n a4 s p1 (beam 12 --))"
        "(n c5 q p1)(barline)"
        "(n c4 q (slur 1 start))(n e4 q)"
        "(barline)"
        "(n g4 q )(n c5 q (slur 1 stop))"
        "(barline)"
        "(n e4 q (tie 1 start))(n e4 q (tie 1 stop))"
        "(barline)"
        "(n c4 e g+ t3/2)(n e4 e)(n d4 e g- t-)(n g4 q)"
        "(barline)"
        "(n c4 e t3/2)(n e4 e)(n d4 e t-)(n g4 q)"
        "(barline)"
        "))"
        ")))",
        Document::k_format_ldp);

    //get the pointer to the interactor, set the rendering buffer and register for
    //receiving desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        //connect the View with the window buffer
        spInteractor->set_rendering_buffer(&m_rbuf_window);

        //ask to receive desired events
        spInteractor->add_event_handler(k_update_window_event, update_window);
    }
}

//---------------------------------------------------------------------------------------
void update_view_content()
{
    //request the view to re-draw the bitmap

    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        spInteractor->redraw_bitmap();
}

//---------------------------------------------------------------------------------------
void on_mouse_button_down(int x, int y, unsigned flags)
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        spInteractor->on_mouse_button_down(x, y, flags);
}

//---------------------------------------------------------------------------------------
void on_mouse_move(int x, int y, unsigned flags)
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        spInteractor->on_mouse_move(x, y, flags);
}

//---------------------------------------------------------------------------------------
void on_mouse_button_up(int x, int y, unsigned flags)
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        spInteractor->on_mouse_button_up(x, y, flags);
}

//---------------------------------------------------------------------------------------
void on_key(int x, int y, unsigned key, unsigned flags)
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        switch (key)
        {
            case 'd':
                spInteractor->switch_task(TaskFactory::k_task_drag_view);
                break;
            case 's':
                spInteractor->switch_task(TaskFactory::k_task_selection);
                break;
            case '+':
                spInteractor->zoom_in(x, y);
                break;
            case '-':
                spInteractor->zoom_out(x, y);
                break;
            default:
                ;
        }

        spInteractor->force_redraw();
    }
}

//---------------------------------------------------------------------------------------
void get_mouse_position(XEvent& event)
{
    m_xMouse = event.xbutton.x;
    m_yMouse = m_flip_y ? m_rbuf_window.height() - event.xbutton.y
                        : event.xbutton.y;
}

//---------------------------------------------------------------------------------------
unsigned get_mouse_flags(XEvent& event)
{
    unsigned flags = 0;
    if(event.xbutton.state & ShiftMask)   flags |= k_kbd_shift;
    if(event.xbutton.state & ControlMask) flags |= k_kbd_ctrl;
    if(event.xbutton.state & Button1Mask) flags |= k_mouse_left;
    if(event.xbutton.state & Button3Mask) flags |= k_mouse_right;
    if(event.xbutton.button == Button1)   flags |= k_mouse_left;
    if(event.xbutton.button == Button3)   flags |= k_mouse_right;
    return flags;
}

//---------------------------------------------------------------------------------------
bool determine_suitable_bitmap_format()
{
    //Returns false if Lomse can not find a suitable bitmap format
    //for this /operating system / platform

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
    m_buf_window = new unsigned char[width * height * (m_bpp / 8)];
    memset(m_buf_window, 255, width * height * (m_bpp / 8));

    //attach this memory to the rendering buffer
    m_rbuf_window.attach(m_buf_window, width, height,
                         (m_flip_y ? -width * (m_bpp / 8) : width * (m_bpp / 8)) );

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
//    unsigned flags;
//    int cur_x;
//    int cur_y;

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

        //discard all intermediate MotionNotify events
        if(event.type == MotionNotify)
        {
            XEvent te = event;
            for(;;)
            {
                if(XPending(m_pDisplay) == 0) break;
                XNextEvent(m_pDisplay, &te);
                if(te.type != MotionNotify) break;
            }
            event = te;
        }

        switch(event.type)
        {
            //--------------------------------------------------------------------
            case ConfigureNotify:
            {
                if(event.xconfigure.width  != int(m_rbuf_window.width()) ||
                   event.xconfigure.height != int(m_rbuf_window.height()))
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
                    display_view_content(&m_rbuf_window);
                    XFlush(m_pDisplay);
                    XSync(m_pDisplay, false);
                }
                break;

            //--------------------------------------------------------------------
            case KeyPress:
            {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                on_key(event.xkey.x,
                       m_flip_y ?
                           m_rbuf_window.height() - event.xkey.y :
                           event.xkey.y,
                       key,
                       0);
                break;
            }

            //--------------------------------------------------------------------
            case ButtonPress:
            {
                unsigned flags = get_mouse_flags(event);
                get_mouse_position(event);

                if(flags & (k_mouse_left | k_mouse_right))
                    on_mouse_button_down(m_xMouse, m_yMouse, flags);

                break;
            }

            //--------------------------------------------------------------------
            case ButtonRelease:
            {
                unsigned flags = get_mouse_flags(event);
                get_mouse_position(event);

                if(flags & (k_mouse_left | k_mouse_right))
                    on_mouse_button_up(m_xMouse, m_yMouse, flags);

                break;
            }

            //--------------------------------------------------------------------
            case MotionNotify:
            {
                unsigned flags = get_mouse_flags(event);
                get_mouse_position(event);
                on_mouse_move(m_xMouse, m_yMouse, flags);
                break;
            }

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
    XSelectInput(m_pDisplay, m_window, PointerMotionMask
                                       | ButtonPressMask
                                       | ButtonReleaseMask
                                       | ExposureMask
                                       | KeyPressMask
                                       | StructureNotifyMask );

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
                           "Lomse examples. Example_1",     //window title
                           "Lomse_1",                       //name in tasks bar
                           None,
                           NULL,
                           0,
                           NULL
                          );


    // create the Graphics Context
    m_gc = XCreateGC(m_pDisplay, m_window, 0, 0);

}

//---------------------------------------------------------------------------------------
bool init_x()
{
    //returns false if an error occurs

    //create X connection
    m_pDisplay = XOpenDisplay(NULL);
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
    m_flip_y = false;               //y axis is not reversed
    m_lomse.init_library(m_format, 96, m_flip_y);   //resolution=96 ppi

    //initialize lomse related variables
    m_pPresenter = NULL;
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
