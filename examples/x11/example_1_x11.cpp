//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

// header files required for X11. The order is important:
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

//some additional needed stuff
#include <stdio.h>
#include <stdlib.h>

//lomse headers
#include "lomse_doorway.h"
#include "lomse_document.h"
#include "lomse_graphic_view.h"
#include "lomse_interactor.h"
#include "lomse_presenter.h"

using namespace lomse;


//---------------------------------------------------------------------------------------
// In this first example we are just going to display an score on the main window.
// Let's define the necessary variables:
//
LomseDoorway    m_lomse;        //the Lomse library doorway
Presenter*      m_pPresenter;
Interactor*     m_pInteractor;  //to interact with the View
Document*       m_pDoc;         //the score to display

//the Lomse View renders its content on a bitmap. To manage it, Lomse
//associates the bitmap to a RenderingBuffer object.
//It is your responsibility to render the bitmap on a window.
//Here you define the rendering buffer and its associated bitmap to be
//used by the previously defined View.
unsigned char*      m_buf_window;
RenderingBuffer     m_rbuf_window;
XImage*             m_ximg_window;
int                 m_depth;
Visual*             m_visual;

//Lomse can manage a lot of bitmap formats and pixel formats. You must
//define the format that you are going to use
int              m_byte_order;
EPixelFormat     m_format;      //bitmap format
unsigned         m_bpp;         //bits per pixel
bool             m_flip_y;      //true if y axis is reversed

//some additinal variables
bool    m_view_needs_redraw;      //to control when the View must be re-drawed

//to measure ellapsed time (for performance measurements)
struct timeval m_start_tv;


//for keyboard support
unsigned      m_last_translated_key;    //last pressed key once translated
unsigned      m_keymap[256];            //Win32 keys <-> Lomse keys translation map


//for mouse click & move position
int           m_xMouse;
int           m_yMouse;


//To be moved to platform support
XSetWindowAttributes m_window_attributes;
Atom                 m_close_atom;


// All typical X stuff needed to run the program and the main events handler loop.
// X has a number of important variables for handling windows:

Display *m_pDisplay;    //points to the X Server.
int m_screen;           //refers to which screen of the display to use.
Window m_window;        //the actual window itself
GC m_gc;                //And the GC is the graphics context.



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
void force_redraw(void* pThis)
{
    // Callback method for Lomse. It can be used also by your application.
    // force_redraw() is an analog of the Win32 InvalidateRect() function.
    // When invoked by Lomse it must it set a flag (or send a message) which
    // results in invoking View->on_paint() and then updating the content of
    // the window when the next event cycle comes.

    m_view_needs_redraw = true;             //force to invoke View->on_paint()
}

//---------------------------------------------------------------------------------------
void update_window(void* pThis)
{
    // Callback method for Lomse. It can be used also by your application.
    // Invoking update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without calling
    // any View methods (i.e. on_paint)

    display_view_content(&m_rbuf_window);
    XSync(m_pDisplay, false);
}

//---------------------------------------------------------------------------------------
void start_timer(void* pThis)
{
    gettimeofday(&m_start_tv, NULL);
}

//---------------------------------------------------------------------------------------
double elapsed_time(void* pThis)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    double ms = (tv.tv_usec - m_start_tv.tv_usec) / 1000.0;
    if (tv.tv_sec > m_start_tv.tv_sec)
    {
         double seconds = double(tv.tv_sec - m_start_tv.tv_sec);
         ms += seconds * 1000000.0;
   }
   m_start_tv = tv;
   return ms;
}

//---------------------------------------------------------------------------------------
string get_font_filename(const string& fontname, bool bold, bool italic)
{
    //This is just a trivial example. In real applications you should
    //use operating system services to find a suitable font

    //notes on parameters received:
    // - fontname can be either the face name (i.e. "Book Antiqua") or
    //   the familly name (i.e. "sans-serif")


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


   return path + fontfile;
}

//-------------------------------------------------------------------------
void open_document()
{
    //Normally you will load the content of a file. But in this
    //simple example we wiil crete an empty document and define its content
    //from a text string

    //first, we will create a 'presenter'. It takes care of creating and maintaining
    //all objects and relationships between the document, its views and the interactors
    //to interact with the view
    m_pPresenter = m_lomse.new_document(ViewFactory::k_view_horizontal_book);

    //next, get the pointers to the relevant components
    m_pDoc = m_pPresenter->get_document();
    m_pInteractor = m_pPresenter->get_interactor(0);

    //connect the View with the window buffer and set required callbacks
    m_pInteractor->set_rendering_buffer(&m_rbuf_window);
    m_pInteractor->set_force_redraw_callbak(NULL, force_redraw);
    m_pInteractor->set_update_window_callbak(NULL, update_window);
    m_pInteractor->set_start_timer_callbak(NULL, start_timer);
    m_pInteractor->set_elapsed_time_callbak(NULL, elapsed_time);

    //Now let's place content on the created document
    //TODO: Next instruction creates a new document without deleting the previous content
    //thus creating memory leaks.
    m_pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
        //"(instrument (musicData (clef G)(clef F3)(clef C1)(clef F4) )) )))" );

//        "(instrument (name \"Violin\")(musicData (clef G)(clef F4)(clef C1) )) )))" );

//        //empty score
//        "(instrument (musicData )) )))" );

//        //two instruments
//        "(instrument (musicData )) (instrument (musicData )) )))" );

//        //piano: stems and flags
//        "(instrument (staves 2) (musicData (clef G p1)(clef C1 p2)"
//        "(n d5 q p1)(n e5 e p1)(goBack start)"
//        "(n c4 q p2)(n b3 e p2)(barline)"
//        ")) )))" );

//        //note with two accidentals
//        "(instrument (musicData (clef G)"
//        "(n --g4 q)"
//        ")) )))" );

//        //beams
//        "(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData "
//        "(clef F4)(key E)(time 2 4)(n +c3 e.)(barline)"
//        "(n e2 q)(n e3 q)(barline)"
//        "(n f2 e (beam 1 +))(n g2 e (beam 1 -))"
//            "(n f3 e (beam 3 +))(n g3 e (beam 3 -))(barline)"
//        "(n f2 e. (beam 4 +))(n g2 s (beam 4 -b))"
//            "(n f3 s (beam 5 +f))(n g3 e. (beam 5 -))(barline)"
//        "(n g2 e. (beam 2 +))(n e3 s (beam 2 -b))(n g3 q)(barline)"
//        "(n a2 e (beam 6 +))(n g2 e (beam 6 -))(n a3 q)(barline)"
//        "(n -b2 q)(n =b3 q)(barline)"
//        "(n xc3 q)(n ++c4 q)(barline)"
//        "(n d3 q)(n --d4 q)(barline)"
//        "(n e3 q)(n e4 q)(barline)"
//        "(n f3 q)(n f4 q)(barline end)"
//        "))"
//        "(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData "
//        "(clef G p1)(clef F4 p2)(key F)(time 12 8)"
//        "(n c5 e. p1)(barline)"
//        "(n e4 e p1 (beam 10 +))(n g3 e p2 (beam 10 -))"
//        "(n e4 e p1 (stem up)(beam 11 +))(n e5 e p1 (stem down)(beam 11 -))(barline)"
//        "(n e4 s p1 (beam 12 ++))(n f4 s p1 (beam 12 ==))"
//            "(n g4 s p1 (beam 12 ==))(n a4 s p1 (beam 12 --))"
//        "(n c5 q p1)(barline)"
////        "(chord (n c4 q p1)(n e4 q p1)(n g4 q p1))"
////        "(chord (n c4 q p1)(n d4 q p1)(n g4 q p1))"
//        "))"
//        ")))" );

//        //anchor / chord
//        "(instrument (staves 2)(musicData "
//        "(clef G p1)(clef F4 p2)(key F)(time 2 4)"
//        "(chord (n c4 h p1)(n d4 h p1)(n g4 h p1))"
//        "(goBack start)(n c3 h p2)(barline)"
//        "(chord (n c4 q p1)(n d4 q p1)(n g4 q p1))"
//        "(chord (n a4 e p1)(n b4 e p1)(n d5 e p1))"
//        "(chord (n a4 e p1)(n b4 e p1)(n c5 e p1))"
//        "(goBack start)(n c3 q p2)(n c3 e p2)(n c3 e p2)(barline)"
//        "(chord (n f4 e p1)(n g4 e p1)(n b4 e p1))"
//        "(chord (n f4 e p1)(n g4 e p1)(n a4 e p1))"
//        "(goBack start)(n c3 e p2)(n c3 e p2)(barline)"
//
//        "(chord (n c4 h p1)(n -d4 h p1)(n +g4 h p1))"
//        "(goBack start)(n c3 h p2)(barline)"
//        "(chord (n +c4 q p1)(n +d4 q p1)(n g4 q p1))"
//        "(chord (n +a4 e p1)(n b4 e p1)(n d5 e p1))"
//        "(chord (n a4 e p1)(n b4 e p1)(n +c5 e p1))"
//        "(goBack start)(n c3 q p2)(n c3 e p2)(n c3 e p2)(barline)"
//        "(chord (n f4 e p1)(n g4 e p1)(n b4 e p1))"
//        "(chord (n f4 e p1)(n g4 e p1)(n a4 e p1))"
//        "(goBack start)(n c3 e p2)(n c3 e p2)"
//        "))"
//        ")))" );

//        //chord with accidentals
//        "(instrument (staves 2)(musicData "
//        "(clef G p1)(clef 8_F4 p2)(key F)(time 2 4)"
//            //no displaced notes, no accidentals
//        "(chord (n c4 q p1)(n e4 q p1)(n g4 q p1))"
//        "(chord (n g5 q p1)(n e5 q p1)(n g4 q p1))"
//        "(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
//            //no displaced notes, accidentals
//        "(chord (n c4 q p1)(n -e4 q p1)(n +g4 q p1))"
//        "(chord (n +g5 q p1)(n -e5 q p1)(n g4 q p1))"
//        "(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
//            //displaced notes, no accidentals
//        "(chord (n c4 q p1)(n d4 q p1)(n g4 q p1))"
//        "(chord (n g5 q p1)(n f5 q p1)(n g4 q p1))"
//        "(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
//            //displaced notes, accidentals
//        "(chord (n c4 q p1)(n -d4 q p1)(n +g4 q p1))"
//        "(chord (n +g5 q p1)(n -f5 q p1)(n =g4 q p1))"
//        "(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
//            //chords from ref.paper
//        "(chord (n -e5 q p1)(n c5 q p1)(n =a4 q p1))"
//        "(chord (n +a5 q p1)(n +e5 q p1)(n +c5 q p1)(n +a4 q p1)(n +f4 q p1))"
//        "(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
//        "))"
//        ")))" );

//        //note with accidentals. Anchor alignment
//        "(instrument (staves 2)(musicData "
//        "(clef G p1)(clef F4 p2)(key F)(time 2 4)"
//        "(n c4 q p1)(barline)"
//        "(n +c4 q p1)"
//        "(n +g5 q p1)"
//        "(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
//        ")) )))" );

//        //all clefs
//        "(instrument (musicData "
//        "(clef G)(clef G1)(clef F3)(clef F4)(clef F5)(clef C1)(clef C2)"
//        "(clef C3)(clef C4)(clef C5)(clef percussion)(clef 8_G)(clef G_8)"
//        "(clef 15_G)(clef G_15)(clef 8_F4)(clef F4_8)(clef 15_F4)(clef F4_15)"
//        "))"
//        ")))" );

//        //all notes
//        "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 40)"
//        "(instrument (musicData "
//        "(clef G)"
//        "(n f4 l)(n f4 b)(n f4 w)(n f4 h)(n f4 q)(n f4 e)(n f4 s)(n f4 t)"
//        "(n f4 i)(n f4 o)(n f4 f)(barline)"
//        "(n c5 l)(n c5 b)(n c5 w)(n c5 h)(n c5 q)(n c5 e)(n c5 s)(n c5 t)"
//        "(n c5 i)(n c5 o)(n c5 f)(barline -)"
//        "))"
//        ")))" );

//        //beamed chord
//        "(instrument (musicData "
//        "(clef G)(time 2 4)"
//        "(chord (n a4 e (beam 1 +))(n b4 e)(n d5 e))"
//        "(chord (n a4 e (beam 1 -))(n b4 e)(n c5 e))"
//        "))"
//        ")))" );

//        //tuplet
//        "(instrument (musicData "
//        "(clef G)(key A)(time 2 4)"
//        "(n c4 e g+ t3/2)(n e4 e)(n d4 e g- t-)"
//        "(n c4 e t3/2)(n e4 e)(n d4 e t-)"
//        "))"
//        ")))" );

//        //tuplets-engraving-rule-a-1
//        "(instrument (musicData "
//        "(time 2 4)"
//        "(n a4 e g+ t3)(n a4 e)(n a4 e g- t-)"
//        "(n a4 e g+)(n a4 e g-)"
//        "(barline)"
//        "(time 3 8)"
//        "(n a4 e g+ t4)(n a4 e)(n a4 e)(n a4 e g- t-)"
//        "(barline)"
//        "))"
//        ")))" );

//        //tie. old syntax
//        "(instrument (musicData "
//        "(clef G)(key C)(time 4 4)"
//        "(n e4 q l)(n e4 q)"
//        "))"
//        ")))" );

//        //tie
//        "(instrument (musicData "
//        "(clef G)(key C)(time 4 4)"
//        "(n e4 q (tie 1 start))(n e4 q (tie 1 stop))"
//        "))"
//        ")))" );

//        //tie bezier
//        "(instrument (musicData "
//        "(clef G)(key C)(time 4 4)"
//        "(n e4 q (tie 1 start (bezier (start-x 30)(start-y 40))))(n e4 q (tie 1 stop))"
//        "))"
//        ")))" );

//        //system break
//        "(instrument (musicData "
//        "(clef G)(key C)(time 2 4)"
//        "(n c4 q l)(n c4 q)"
//        "(barline)"
//        "(n e4 q v1 (tie 3 start) )"
//        "(newSystem)"
//        "(barline)"
//        "(n e4 q v1 (tie 3 stop) )"
//        "(barline end)"
//        "))"
//        ")))" );

//        //context change
//        "(instrument (musicData "
//        "(clef G)(key A)(time 2 4)"
//        "(n c4 q l)(n c4 q)(barline)"
//        "(clef F4)(n c3 h)(barline)"
//        "(clef G)(n g4 h)(barline)"
//        "(newSystem)(n c5 h)(barline)"
//        "(clef F4)(n c3 h)(barline end)"
//        "))"
//        ")))" );

//        //one slur
//        "(instrument (musicData "
//        "(clef G)(key C)(time 2 4)"
//        "(n c4 q (slur 1 start))(n e4 q)"
//        "(barline)"
//        "(n g4 q )(n c5 q (slur 1 stop))"
//        "(barline end)"
//        "))"
//        ")))" );

//        //slur placement
//        "(instrument (musicData "
//        "(clef G)(key C)(time 2 4)"
//        "(n c4 q (slur 1 start))(n e4 q)"
//        "(barline)"
//        "(n g4 q )(n c5 q (slur 1 stop))"
//        "(barline)"
//        "(n c5 q (slur 2 start))(n b4 q)"
//        "(barline)"
//        "(n e5 q )(n a5 q (slur 2 stop))"
//        "(barline)"
//        "(n c4 q (slur 3 start))(n b3 q)"
//        "(barline)"
//        "(n e4 q )(n f4 q (slur 3 stop))"
//        "(barline end)"
//        "))"
//        ")))" );

        //full
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
        "(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData "
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
        ")))" );

}

//-------------------------------------------------------------------------
void update_view_content()
{
    //request the view to re-draw the bitmap

    if (!m_pInteractor) return;
    m_pInteractor->on_paint();
}

//-------------------------------------------------------------------------
void on_mouse_button_down(int x, int y, unsigned flags)
{
    if (!m_pInteractor) return;
    m_pInteractor->on_mouse_button_down(x, y, flags);
}

//-------------------------------------------------------------------------
void on_mouse_move(int x, int y, unsigned flags)
{
    if (!m_pInteractor) return;
    m_pInteractor->on_mouse_move(x, y, flags);
}

//-------------------------------------------------------------------------
void on_mouse_button_up(int x, int y, unsigned flags)
{
    if (!m_pInteractor) return;
    m_pInteractor->on_mouse_button_up(x, y, flags);
}

//-------------------------------------------------------------------------
void reset_boxes_to_draw()
{
    m_pInteractor->set_rendering_option(k_option_draw_box_doc_page_content, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_score_page, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_system, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_slice, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_slice_instr, false);
}

//-------------------------------------------------------------------------
void on_key(int x, int y, unsigned key, unsigned flags)
{
    if (!m_pInteractor) return;

    switch (key)
    {
        case '1':
            reset_boxes_to_draw();
            m_pInteractor->set_rendering_option(k_option_draw_box_doc_page_content, true);
            break;
        case '2':
            reset_boxes_to_draw();
            m_pInteractor->set_rendering_option(k_option_draw_box_score_page, true);
            break;
        case '3':
            reset_boxes_to_draw();
            m_pInteractor->set_rendering_option(k_option_draw_box_system, true);
            break;
        case '4':
            reset_boxes_to_draw();
            m_pInteractor->set_rendering_option(k_option_draw_box_slice, true);
            break;
        case '5':
            reset_boxes_to_draw();
            m_pInteractor->set_rendering_option(k_option_draw_box_slice_instr, true);
            break;
        case '8':
            m_pInteractor->switch_task(TaskFactory::k_task_drag_view);
            break;
        case '9':
            m_pInteractor->switch_task(TaskFactory::k_task_selection);
            break;
        case '0':
            reset_boxes_to_draw();
            break;
        case '+':
            m_pInteractor->zoom_in(x, y);
            break;
        case '-':
            m_pInteractor->zoom_out(x, y);
            break;
        default:
            ;
    }

    force_redraw(NULL);
}

//---------------------------------------------------------------------------------------
void get_mouse_position(XEvent& event)
{
    m_xMouse = event.xbutton.x;
    m_yMouse = m_flip_y ? m_rbuf_window.height() - event.xbutton.y
                        : event.xbutton.y;
}

//---------------------------------------------------------------------------------------
unsigned get_keyboard_flags(XEvent& event)
{
    unsigned flags = 0;
    if(event.xkey.state & Button1Mask) flags |= k_mouse_left;
    if(event.xkey.state & Button3Mask) flags |= k_mouse_right;
    if(event.xkey.state & ShiftMask)   flags |= k_kbd_shift;
    if(event.xkey.state & ControlMask) flags |= k_kbd_ctrl;
    return flags;
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
bool determine_bitmap_format()
{
    //TODO: This code must be moved to Lomse platform support library
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
    m_buf_window = new unsigned char[width * height * (m_bpp / 8)];
    memset(m_buf_window, 255, width * height * (m_bpp / 8));

    m_rbuf_window.attach(m_buf_window, width, height,
                         (m_flip_y ? -width * (m_bpp / 8) : width * (m_bpp / 8)) );

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
            update_window(NULL);
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
                    update_window(NULL);
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
    if (!determine_bitmap_format())
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
// application entry point
int main ()
{
    if (!init_x())
        exit(1);

    //initialize lomse related variables
    m_flip_y = false;               //y axis is not reversed
    m_pInteractor = NULL;
    m_pDoc = NULL;
    m_view_needs_redraw = true;

    //initialize the Lomse library
    m_lomse.init_library(k_pix_format_rgba32, 96, false);

    //set required callbacks
    m_lomse.set_get_font_callback(get_font_filename);

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
