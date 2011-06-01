//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/app.h>
    #include <wx/frame.h>
    #include <wx/menu.h>
    #include <wx/msgdlg.h>
    #include <wx/filedlg.h>
    #include <wx/image.h>
    #include <wx/dc.h>
    #include <wx/dcmemory.h>
    #include <wx/event.h>
#endif

#include <iostream>
#include <UnitTest++.h>

//lomse headers
#include "lomse_doorway.h"
#include "lomse_document.h"
#include "lomse_graphic_view.h"
#include "lomse_interactor.h"
#include "lomse_presenter.h"

#include "lomse_test_runner_class.h"

using namespace lomse;


//---------------------------------------------------------------------------------------
// Define the application
class MyApp: public wxApp
{
public:
    bool OnInit();

};

//---------------------------------------------------------------------------------------
// Define the main frame
class MyFrame: public wxFrame
{
public:
    MyFrame();
    virtual ~MyFrame();

    void open_document();
    void update_view_content();

    //callback wrappers
    static void wrapper_start_timer(void* pThis);
    static double wrapper_elapsed_time(void* pThis);
    static void wrapper_force_redraw(void* pThis);
    static void wrapper_update_window(void* pThis);
    static string get_font_filename(const string& fontname, bool bold, bool italic);

protected:
    void OnQuit(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& WXUNUSED(event));
    void OnAbout(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent &WXUNUSED(event));
    void OnZoomIn(wxCommandEvent& WXUNUSED(event));
    void OnZoomOut(wxCommandEvent& WXUNUSED(event));
    void OnMouseEvent(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnDoTests(wxCommandEvent& WXUNUSED(event));

    void start_timer();
    double elapsed_time();
    void force_redraw();
    void update_window();

    void create_menu();
    void initialize_lomse();

    void delete_rendering_buffer();
    void create_rendering_buffer(int width, int height);
    void do_update_window(wxDC& dc);
    void update_rendering_buffer();

    void on_key(int x, int y, unsigned key, unsigned flags);
    unsigned get_keyboard_flags(wxKeyEvent& event);
    unsigned get_mouse_flags(wxMouseEvent& event);
    void reset_boxes_to_draw();

    // In this first example we are just going to display an score on the main frame.
    // Let's define the necessary variables:
    LomseDoorway    m_lomse;        //the Lomse library doorway
    Presenter*      m_pPresenter;
    Interactor*     m_pInteractor;  //to interact with the View
    Document*       m_pDoc;         //the score to display

    //the Lomse View renders its content on a bitmap. To manage it, Lomse
    //associates the bitmap to a RenderingBuffer object.
    //It is your responsibility to render the bitmap on a window.
    //Here you define the rendering buffer and its associated bitmap to be
    //used by the previously defined View.
    RenderingBuffer     m_rbuf_window;
    wxImage*            m_buffer;               //the image to serve as buffer
    unsigned char*      m_pdata;                //ptr to the real bytes buffer
//    agg::rendering_buffer       m_oRenderingBuffer;     //the agg rendering buffer
    int                 m_nBufWidth, m_nBufHeight;      //size of the bitmap


    //some additinal variables
    bool    m_view_needs_redraw;      //to control when the View must be re-drawed

//    //to measure ellapsed time (for performance measurements)
//    LARGE_INTEGER m_sw_freq;
//    LARGE_INTEGER m_sw_start;
//
//    //for keyboard support
//    unsigned      m_last_translated_key;    //last pressed key once translated
//    unsigned      m_keymap[256];            //Win32 keys <-> Lomse keys translation map

    //for mouse click & move position
    int           m_xMouse;
    int           m_yMouse;
    unsigned      m_input_flags;


    DECLARE_EVENT_TABLE()
};

//---------------------------------------------------------------------------------------
// constants for meni IDs
enum
{
    Menu_File_Quit = wxID_EXIT,
    Menu_Help_About = wxID_ABOUT,

    Menu_File_Open = 300,
    Menu_Do_Tests,

    Menu_Max
};


//=======================================================================================
// MyApp implementation
//=======================================================================================

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame;
    frame->Show(true);
    SetTopWindow(frame);

    return true;
}


//=======================================================================================
// MyFrame implementation
//=======================================================================================

// event tables
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_KEY_DOWN(MyFrame::OnKeyDown)
    EVT_MOUSE_EVENTS(MyFrame::OnMouseEvent)
    EVT_MENU(Menu_File_Quit, MyFrame::OnQuit)
    EVT_MENU(Menu_Help_About, MyFrame::OnAbout)
    EVT_MENU(Menu_File_Open, MyFrame::OnOpen)
    EVT_MENU(wxID_ZOOM_IN, MyFrame::OnZoomIn)
    EVT_MENU(wxID_ZOOM_OUT, MyFrame::OnZoomOut)
    EVT_MENU(Menu_Do_Tests, MyFrame::OnDoTests)
    EVT_SIZE(MyFrame::OnSize)
    EVT_PAINT(MyFrame::OnPaint)
END_EVENT_TABLE()

//---------------------------------------------------------------------------------------
MyFrame::MyFrame()
       : wxFrame(NULL, wxID_ANY, _T("Lomse sample for wxWidgets"),
                 wxDefaultPosition, wxSize(850, 600))
       , m_pPresenter(NULL)
       , m_pInteractor(NULL)
       , m_pDoc(NULL)
       , m_buffer(NULL)
       , m_view_needs_redraw(true)
{
    create_menu();
    initialize_lomse();

    //create a music score and a View. The view will display the score
    //when the paint event is sent to lomse, once the main windows is
    //shown and the event handling loop is started
    open_document();
}

//---------------------------------------------------------------------------------------
MyFrame::~MyFrame()
{
    //delete the Interactor. This will also delete the Document
    delete m_pInteractor;
}

//---------------------------------------------------------------------------------------
void MyFrame::create_menu()
{
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(Menu_File_Open, _T("&Open..."));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, _T("E&xit"));

    wxMenu* zoomMenu = new wxMenu;
    zoomMenu->Append(wxID_ZOOM_100);
    zoomMenu->Append(wxID_ZOOM_FIT);
    zoomMenu->Append(wxID_ZOOM_IN);
    zoomMenu->Append(wxID_ZOOM_OUT);

    wxMenu* debugMenu = new wxMenu;
    debugMenu->Append(Menu_Do_Tests, _T("Run unit tests"));

    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Menu_Help_About, _T("&About"));

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(zoomMenu, _T("&Zoom"));
    menuBar->Append(debugMenu, _T("&Debug"));
    menuBar->Append(helpMenu, _T("&Help"));

    SetMenuBar(menuBar);
}

//---------------------------------------------------------------------------------------
void MyFrame::initialize_lomse()
{
    // Lomse knows nothing about windows. It renders everything on bitmaps and the
    // user application uses them. For instance, to display it on a wxWindos.
    // Lomse supports a lot of bitmap formats and pixel formats. Therefore, before
    // using the Lomse library you MUST specify which bitmap formap to use.
    //
    // For wxWidgets, I would suggets using a platform independent format. So
    // I will use a wxImage as the rendering  buffer. wxImage is platform independent
    // and its buffer is an array of characters in RGBRGBRGB... format,  in the
    // top-to-bottom, left-to-right order. That is, the first RGB triplet corresponds
    // to the first pixel of the first row; the second RGB triplet, to the second
    // pixel of the first row, and so on until the end of the first row,
    // with second row following after it and so on.
    // Therefore, the pixel format is RGB 24 bits.
    //
    // Let's define the requiered information:

        //the pixel format
        int pixel_format = k_pix_format_rgb24;  //RGB 24bits

        //the desired resolution. For Linux and Windows use 96 pixels per inch
        int resolution = 96;    //96 ppi

        //Normal y axis direction is 0 coordinate at top and increase downwards. You
        //must specify if you would like just the opposite behaviour. For Windows and
        //Linux the default behaviour is the right behaviour.
        bool reverse_y_axis = false;

    //Now, initialize the library with these values
    m_lomse.init_library(pixel_format,resolution, reverse_y_axis);

    //set required callbacks
    m_lomse.set_get_font_callback(get_font_filename);

    //create a bitmap for the View
    wxSize size = this->GetClientSize();
    create_rendering_buffer(size.GetWidth(), size.GetHeight());
}

//---------------------------------------------------------------------------------------
void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

//---------------------------------------------------------------------------------------
void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    (void)wxMessageBox(_T("Lomse sample for wxWidgets"),
                       _T("About wxWidgets Lomse sample"),
                       wxICON_INFORMATION);
}

//---------------------------------------------------------------------------------------
void MyFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    wxString defaultPath = wxT("../../../test-scores/");

    wxString p = wxFileSelector(_("Open score"), defaultPath,
        wxEmptyString, wxEmptyString, wxT("LenMus files|*.lms;*.lmd"));

    if (p.empty())
        return;

    //create a new View
    std::string filename( p.mb_str(wxConvUTF8) );
    delete m_pPresenter;
    m_pPresenter = m_lomse.open_document(ViewFactory::k_view_horizontal_book,
                                         filename);

    //get the pointers to the relevant components
    m_pDoc = m_pPresenter->get_document();
    m_pInteractor = m_pPresenter->get_interactor(0);

    //connect the View with the window buffer and set required callbacks
    m_pInteractor->set_rendering_buffer(&m_rbuf_window);
    m_pInteractor->set_force_redraw_callbak(this, wrapper_force_redraw);
    m_pInteractor->set_update_window_callbak(this, wrapper_update_window);
    m_pInteractor->set_start_timer_callbak(this, wrapper_start_timer);
    m_pInteractor->set_elapsed_time_callbak(this, wrapper_elapsed_time);

    //render the new score
    force_redraw();
}

//---------------------------------------------------------------------------------------
void MyFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    wxSize size = this->GetClientSize();
    create_rendering_buffer(size.GetWidth(), size.GetHeight());
    force_redraw();
}

//---------------------------------------------------------------------------------------
void MyFrame::OnPaint(wxPaintEvent &WXUNUSED(event))
{
    update_rendering_buffer();
    wxPaintDC dc(this);
    do_update_window(dc);
}

//---------------------------------------------------------------------------------------
void MyFrame::update_rendering_buffer()
{
    //update buffer if necessary
    if (m_view_needs_redraw)
        update_view_content();

    m_view_needs_redraw = false;
}

//---------------------------------------------------------------------------------------
void MyFrame::delete_rendering_buffer()
{
    delete m_buffer;
}

//---------------------------------------------------------------------------------------
void MyFrame::create_rendering_buffer(int width, int height)
{
    //creates a bitmap of specified size and associates it to the rendering
    //buffer for the view. Any existing buffer is automatically deleted

    //I will use a wxImage as the rendering  buffer. wxImage is platform independent
    //and its buffer is an array of characters in RGBRGBRGB... format,  in the
    //top-to-bottom, left-to-right order. That is, the first RGB triplet corresponds
    //to the first pixel of the first row; the second RGB triplet, to the second
    //pixel of the first row, and so on until the end of the first row,
    //with second row following after it and so on.

    #define BYTES_PP 3      // Bytes per pixel

    // allocate a new rendering buffer
    delete m_buffer;            //delete any previous buffer
    m_nBufWidth = width;
    m_nBufHeight = height;
    m_buffer = new wxImage(width, height);

    int stride = m_nBufWidth * BYTES_PP;        //number of bytes per row

    m_pdata = m_buffer->GetData();
    m_rbuf_window.attach(m_pdata, m_nBufWidth, m_nBufHeight, stride);

    m_view_needs_redraw = true;
}

//-------------------------------------------------------------------------
void MyFrame::open_document()
{
    //Normally you will load the content of a file. But in this
    //simple example we wiil crete an empty document and define its content
    //from a text string

    //first, we will create a 'presenter'. It takes care of cretaing and maintaining
    //all objects and relationships between the document, its views and the interactors
    //to interct with the view
    m_pPresenter = m_lomse.new_document(ViewFactory::k_view_horizontal_book);

    //now, get the pointers to the relevant components
    m_pDoc = m_pPresenter->get_document();
    m_pInteractor = m_pPresenter->get_interactor(0);

    //connect the View with the window buffer and set required callbacks
    m_pInteractor->set_rendering_buffer(&m_rbuf_window);
    m_pInteractor->set_force_redraw_callbak(this, wrapper_force_redraw);
    m_pInteractor->set_update_window_callbak(this, wrapper_update_window);
    m_pInteractor->set_start_timer_callbak(this, wrapper_start_timer);
    m_pInteractor->set_elapsed_time_callbak(this, wrapper_elapsed_time);

    //Now let's place content on the created document
    m_pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
        //"(instrument (musicData (clef G)(clef F3)(clef C1)(clef F4) )) )))" );

//        //instrument name
//        "(instrument (name \"Violin\")(musicData (clef G)(clef F4)(clef C1) )) )))" );

        //"(instrument (musicData )) )))" );

        //"(instrument (staves 2) (musicData )) )))" );
        //"(instrument (musicData )) (instrument (musicData )) )))" );

//    //Staves of different sizes
//    "(instrument (name \"Violin\")(abbrev \"Vln.\")(staff 1 (staffSpacing 400))(musicData (clef G)(n c4 e.))) "
//    "(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData (clef G p1)(clef F4 p2))) )))" );

//        //beamed chord. Simplest case
//        "(instrument (musicData "
//        "(clef F)(key C)(time 4 4)"
//        "(chord (n a3 e (beam 1 begin)) (n d3 e))"
//        "(chord (n g3 e (beam 1 end)) (n e3 e))"
//        "))"
//        ")))" );

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
//        "(n f3 q)(n f4 q)(barline -)"
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

//        //beamed chord. Beam determines stem direction
//        "(instrument (musicData "
//        "(clef G)(key C)(time 2 4)"
//        "(chord (n c5 s (beam 2 begin begin))(n e5 s)(n g5 s))"
//        "(chord (n c5 s (beam 2 continue continue))(n f5 s)(n a5 s))"
//        "(chord (n d5 s (beam 2 continue continue))(n g5 s)(n b5 s))"
//        "(chord (n g4 s (beam 2 end end))(n e5 s)(n g5 s))"
//        "))"
//        ")))" );

//        //tuplet
//        "(instrument (musicData "
//        "(clef G)(key A)(time 2 4)"
//        "(n c4 e g+ t3/2)(n e4 e)(n d4 e g- t-)"
//        "(n e5 e g+ t3/2)(n c5 e)(n d5 e g- t-)"
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

        //tuplets-engraving-rule-d-1
//        "(instrument (musicData "
//        "(time 4 4)"
//        "(n e4 h t3)(n e4 h)(n e4 h t-)"
//        "(barline)"
//        "(n e5 h t3)(n e5 h)(n e5 h t-)"
//        "(barline)"
//        "(time 2 4)"
//        "(n e4 q t3)(n e4 e t-)"
//        "(barline)"
//        "(n e5 q t3)(n e5 e t-)"
//        "(barline)"
//        "(time 6 8)"
//        "(n e4 e g+ t4)(n e4 e g-)"
//        "(n e4 e g+)(n e4 e g-)"
//        "(n e4 e g+)(n e4 e g-)"
//        "(n e4 e g+)(n e4 e g- t-)"
//        "(barline)"
//        "(n e5 e g+ t4)(n e5 e g-)"
//        "(n e5 e g+)(n e5 e g-)"
//        "(n e5 e g+)(n e5 e g-)"
//        "(n e5 e g+)(n e5 e g- t-)"
//        "(barline)"
//        "))"
//        ")))" );


//        //tuplets-engraving-rule-b-1
//        "(instrument (musicData "
//        "(time 4 4)"
//        "(n e4 e g+ t3)(n e4 e g-)(r e t-)"
//        "(r e t3)(n e5 e)(r e t-)"
//        "(n e5 e t3)(r e)(r e t-)"
//        "(r e t3)(r e)(n e5 e t-)"
//        "))"
//        ")))" );

        //tie
        "(instrument (musicData "
        "(clef G)(key C)(time 4 4)"
        "(n e4 q l)(n e4 q)"
        "))"
        ")))" );


}

//---------------------------------------------------------------------------------------
void MyFrame::wrapper_force_redraw(void* pThis)
{
    static_cast<MyFrame*>(pThis)->force_redraw();
}

//---------------------------------------------------------------------------------------
void MyFrame::force_redraw()
{
    // Callback method for Lomse. It can be used also by your application.
    // force_redraw() is an analog of the Win32 InvalidateRect() function.
    // When invoked by Lomse it must it set a flag (or send a message) which
    // results in invoking View->on_paint() and then updating the content of
    // the window when the next event cycle comes.

    m_view_needs_redraw = true;             //force to invoke View->on_paint()
    update_rendering_buffer();
    update_window();
}

//---------------------------------------------------------------------------------------
void MyFrame::wrapper_update_window(void* pThis)
{
    static_cast<MyFrame*>(pThis)->update_window();
}

//---------------------------------------------------------------------------------------
void MyFrame::update_window()
{
    // Callback method for Lomse. It can be used also by your application.
    // Invoking update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without calling
    // any View methods (i.e. on_paint)

    m_view_needs_redraw = false;

    wxClientDC dc(this);
    do_update_window(dc);
}

//---------------------------------------------------------------------------------------
void MyFrame::do_update_window(wxDC& dc)
{
    if (!m_buffer || !m_buffer->IsOk())
        return;

    // allocate a DC in memory for using the offscreen bitmaps
    wxMemoryDC memoryDC;
    wxBitmap bitmap(*m_buffer);
    memoryDC.SelectObject(bitmap);

    //copy bitmap onto screen
    dc.Blit(0, 0, m_nBufWidth, m_nBufHeight, &memoryDC, 0, 0);

    // deselect the bitmap
    memoryDC.SelectObject(wxNullBitmap);
}

//---------------------------------------------------------------------------------------
void MyFrame::start_timer()
{
    //::QueryPerformanceCounter(&(m_sw_start));
}

//---------------------------------------------------------------------------------------
void MyFrame::wrapper_start_timer(void* pThis)
{
    static_cast<MyFrame*>(pThis)->start_timer();
}


//---------------------------------------------------------------------------------------
double MyFrame::elapsed_time()
{
//    LARGE_INTEGER stop;
//    ::QueryPerformanceCounter(&stop);
//    return double(stop.QuadPart -
//                    m_sw_start.QuadPart) * 1000.0 /
//                    double(m_sw_freq.QuadPart);
    return 0.0;
}

//---------------------------------------------------------------------------------------
double MyFrame::wrapper_elapsed_time(void* pThis)
{
    return static_cast<MyFrame*>(pThis)->elapsed_time();
}

//---------------------------------------------------------------------------------------
string MyFrame::get_font_filename(const string& fontname, bool bold, bool italic)
{
    //This is just a trivial example. In real applications you should
    //use operating system services to find a suitable font

    //notes on parameters received:
    // - fontname can be either the face name (i.e. "Book Antiqua") or
    //   the familly name (i.e. "sans-serif")

#if defined(__WXGTK__)

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
#elif defined(__WXMSW__)

    return "";

#else

    return "";

#endif
}

//-------------------------------------------------------------------------
void MyFrame::update_view_content()
{
    //request the view to re-draw the bitmap

    if (!m_pInteractor) return;
    m_pInteractor->on_paint();
}

//---------------------------------------------------------------------------------------
void MyFrame::OnKeyDown(wxKeyEvent& event)
{
    if (!m_pInteractor) return;

    int nKeyCode = event.GetKeyCode();
    unsigned flags = get_keyboard_flags(event);

    //fix ctrol+key codes
    if (nKeyCode > 0 && nKeyCode < 27)
    {
        nKeyCode += int('A') - 1;
        flags |= k_kbd_ctrl;
    }

    //process key
    switch (nKeyCode)
    {
        case WXK_SHIFT:
        case WXK_ALT:
        case WXK_CONTROL:
            return;      //do nothing

		default:
			on_key(event.GetX(), event.GetY(), nKeyCode, flags);;
	}
}

//-------------------------------------------------------------------------
void MyFrame::reset_boxes_to_draw()
{
    m_pInteractor->set_rendering_option(k_option_draw_box_doc_page_content, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_score_page, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_system, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_slice, false);
    m_pInteractor->set_rendering_option(k_option_draw_box_slice_instr, false);
}

//-------------------------------------------------------------------------
void MyFrame::on_key(int x, int y, unsigned key, unsigned flags)
{
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
            return;
    }

    force_redraw();
}

//-------------------------------------------------------------------------
void MyFrame::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
    if (!m_pInteractor) return;

    //do zoom in centered on window center
    wxSize size = this->GetClientSize();
    m_pInteractor->zoom_in(size.GetWidth()/2, size.GetHeight()/2);
    m_pInteractor->set_rendering_option(k_option_draw_box_slice, true);
    force_redraw();
}

//-------------------------------------------------------------------------
void MyFrame::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
    if (!m_pInteractor) return;

    //do zoom out centered on window center
    wxSize size = this->GetClientSize();
    m_pInteractor->zoom_out(size.GetWidth()/2, size.GetHeight()/2);
    force_redraw();
}

//-------------------------------------------------------------------------
void MyFrame::OnDoTests(wxCommandEvent& WXUNUSED(event))
{
//    cout << "Lomse library tests runner" << endl << endl;
//    UnitTest::RunAllTests();
    MyTestRunner oTR(this);
    oTR.RunTests();
}

//---------------------------------------------------------------------------------------
unsigned MyFrame::get_mouse_flags(wxMouseEvent& event)
{
    unsigned flags = 0;
    if (event.LeftIsDown())     flags |= k_mouse_left;
    if (event.RightIsDown())    flags |= k_mouse_right;
    if (event.MiddleDown())     flags |= k_mouse_middle;
    if (event.ShiftDown())      flags |= k_kbd_shift;
    if (event.AltDown())        flags |= k_kbd_alt;
    if (event.ControlDown())    flags |= k_kbd_ctrl;
    return flags;
}

//---------------------------------------------------------------------------------------
unsigned MyFrame::get_keyboard_flags(wxKeyEvent& event)
{
    unsigned flags = 0;
    if (event.ShiftDown())   flags |= k_kbd_shift;
    if (event.AltDown()) flags |= k_kbd_alt;
    if (event.ControlDown()) flags |= k_kbd_ctrl;
    return flags;
}

//-------------------------------------------------------------------------
void MyFrame::OnMouseEvent(wxMouseEvent& event)
{
    if (!m_pInteractor) return;

    wxEventType nEventType = event.GetEventType();
    wxPoint pos = event.GetPosition();
    unsigned flags = get_mouse_flags(event);

    if (nEventType==wxEVT_LEFT_DOWN)
    {
        flags |= k_mouse_left;
        m_pInteractor->on_mouse_button_down(pos.x, pos.y, flags);
    }
    else if (nEventType==wxEVT_LEFT_UP)
    {
        flags |= k_mouse_left;
        m_pInteractor->on_mouse_button_up(pos.x, pos.y, flags);
    }
    else if (nEventType==wxEVT_RIGHT_DOWN)
    {
        flags |= k_mouse_right;
        m_pInteractor->on_mouse_button_down(pos.x, pos.y, flags);
    }
    else if (nEventType==wxEVT_RIGHT_UP)
    {
        flags |= k_mouse_right;
        m_pInteractor->on_mouse_button_up(pos.x, pos.y, flags);
    }
    else if (nEventType==wxEVT_MOTION)
        m_pInteractor->on_mouse_move(pos.x, pos.y, flags);
}
