//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2015 Cecilio Salmeron. All rights reserved.
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
    #include <wx/dcclient.h>
    #include <wx/event.h>
    #include <wx/sizer.h>
#endif

#include <iostream>

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
// Define the application
class MyApp: public wxApp
{
public:
    bool OnInit();

};

//forward declarations
class MyCanvas;

//---------------------------------------------------------------------------------------
// Define the main frame
class MyFrame: public wxFrame
{
public:
    MyFrame();
    virtual ~MyFrame();

    //commands
    void open_test_document();

    //event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

protected:
    //accessors
    MyCanvas* get_active_canvas() const { return m_canvas; }

    //event handlers
    void OnOpen(wxCommandEvent& WXUNUSED(event));
    void OnZoomIn(wxCommandEvent& WXUNUSED(event));
    void OnZoomOut(wxCommandEvent& WXUNUSED(event));

    //lomse related
    void initialize_lomse();

    void create_menu();

    LomseDoorway m_lomse;        //the Lomse library doorway
    MyCanvas* m_canvas;

    DECLARE_EVENT_TABLE()
};

//---------------------------------------------------------------------------------------
// MyCanvas is a window on which we show the scores
class MyCanvas : public wxWindow
{
public:
    MyCanvas(wxFrame *frame, LomseDoorway& lomse);
    ~MyCanvas();

    void update_view_content();

    //callback wrappers
    static void wrapper_update_window(void* pThis, SpEventInfo pEvent);

    //commands
    void open_test_document();
    void open_file(const wxString& fullname);
    void zoom_in();
    void zoom_out();

protected:
    //event handlers
    void OnPaint(wxPaintEvent& WXUNUSED(event));
    void OnSize(wxSizeEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);

    void delete_rendering_buffer();
    void create_rendering_buffer(int width, int height);
    void copy_buffer_on_dc(wxDC& dc);
    void update_rendering_buffer_if_needed();
    void force_redraw();
    void update_window();
    void on_key(int x, int y, unsigned key, unsigned flags);
    unsigned get_keyboard_flags(wxKeyEvent& event);
    unsigned get_mouse_flags(wxMouseEvent& event);

    // In this first example we are just going to display an score on the window.
    // Let's define the necessary variables:
    LomseDoorway&   m_lomse;        //the Lomse library doorway
    Presenter*      m_pPresenter;

    //the Lomse View renders its content on a bitmap. To manage it, Lomse
    //associates the bitmap to a RenderingBuffer object.
    //It is your responsibility to render the bitmap on a window.
    //Here you define the rendering buffer and its associated bitmap to be
    //used by the previously defined View.
    RenderingBuffer     m_rbuf_window;
    wxImage*            m_buffer;		//the image to serve as buffer
	unsigned char*      m_pdata;		//ptr to the bitmap
    int                 m_nBufWidth, m_nBufHeight;	//size of the bitmap


    //some additinal variables
    bool    m_view_needs_redraw;      //to control when the View must be re-drawn


    DECLARE_EVENT_TABLE()
};



//=======================================================================================
// MyApp implementation
//=======================================================================================

IMPLEMENT_APP(MyApp)

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame;
    frame->Show(true);
    SetTopWindow(frame);

    frame->open_test_document();

    return true;
}


//=======================================================================================
// MyFrame implementation
//=======================================================================================

//---------------------------------------------------------------------------------------
// constants for menu IDs
enum
{
    //new IDs
    k_menu_file_open = wxID_HIGHEST + 1,

    //using standard IDs
    //it is important for the id corresponding to the "About" command to have
    //this standard value as otherwise it won't be handled properly under Mac
    //(where it is special and put into the "Apple" menu)
    k_menu_file_quit = wxID_EXIT,
    k_menu_help_about = wxID_ABOUT,
    k_menu_zoom_in = wxID_ZOOM_IN,
    k_menu_zoom_out = wxID_ZOOM_OUT,
};

//---------------------------------------------------------------------------------------
// events table
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(k_menu_file_quit, MyFrame::OnQuit)
    EVT_MENU(k_menu_help_about, MyFrame::OnAbout)
    EVT_MENU(k_menu_file_open, MyFrame::OnOpen)
    EVT_MENU(k_menu_zoom_in, MyFrame::OnZoomIn)
    EVT_MENU(k_menu_zoom_out, MyFrame::OnZoomOut)
END_EVENT_TABLE()

//---------------------------------------------------------------------------------------
MyFrame::MyFrame()
	: wxFrame(NULL, wxID_ANY, _T("Lomse sample for wxWidgets"),
			  wxDefaultPosition, wxSize(850, 600))
{
    create_menu();
    initialize_lomse();

    // create our one and only child -- it will take our entire client area
    m_canvas = new MyCanvas(this, m_lomse);
    wxSizer *sz = new wxBoxSizer(wxVERTICAL);
    sz->Add(m_canvas, 3, wxGROW);
    SetSizer(sz);
}

//---------------------------------------------------------------------------------------
MyFrame::~MyFrame()
{
}

//---------------------------------------------------------------------------------------
void MyFrame::create_menu()
{
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(k_menu_file_open, _T("&Open..."));
    fileMenu->AppendSeparator();
    fileMenu->Append(k_menu_file_quit, _T("E&xit"));

    wxMenu* zoomMenu = new wxMenu;
    zoomMenu->Append(k_menu_zoom_in);
    zoomMenu->Append(k_menu_zoom_out);

    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(k_menu_help_about, _T("&About"));

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(zoomMenu, _T("&Zoom"));
    menuBar->Append(helpMenu, _T("&Help"));

    SetMenuBar(menuBar);
}

//---------------------------------------------------------------------------------------
void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true /*force to close*/);
}

//---------------------------------------------------------------------------------------
void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Lomse: sample 1 for wxWidgets"),
                 _T("About wxWidgets Lomse sample"),
                 wxOK | wxICON_INFORMATION, this);
}

//---------------------------------------------------------------------------------------
void MyFrame::initialize_lomse()
{
    // Lomse knows nothing about windows. It renders everything on bitmaps and the
    // user application uses them. For instance, to display it on a wxWindows.
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

        //the desired resolution. For Linux and Windows 96 pixels per inch works ok.
        int resolution = 96;    //96 ppi

        //Normal y axis direction is 0 coordinate at top and increase downwards. You
        //must specify if you would like just the opposite behaviour. For Windows and
        //Linux the default behaviour is the right behaviour.
        bool reverse_y_axis = false;

    //initialize the library with these values
    m_lomse.init_library(pixel_format,resolution, reverse_y_axis);
}

//---------------------------------------------------------------------------------------
void MyFrame::open_test_document()
{
    get_active_canvas()->open_test_document();

    //BUG_BYPASS
    // In Linux there are problems to catch Key Up/Down events. See for instance
    // http://forums.wxwidgets.org/viewtopic.php?t=33057&p=137567
    // Following line is not needed for Windows (doen't hurt) but it is
    // necessary for Linux, in order to receive Key Up/Down events
    get_active_canvas()->SetFocus();
}

//---------------------------------------------------------------------------------------
void MyFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    wxString defaultPath = wxT("../../../test-scores/");

    wxString filename = wxFileSelector(_("Open score"), defaultPath,
        wxEmptyString, wxEmptyString, wxT("LenMus files|*.lms;*.lmd"));

    if (filename.empty())
        return;

    get_active_canvas()->open_file(filename);
}

//---------------------------------------------------------------------------------------
void MyFrame::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->zoom_in();
}

//---------------------------------------------------------------------------------------
void MyFrame::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->zoom_out();
}


//=======================================================================================
// MyCanvas implementation
//=======================================================================================

BEGIN_EVENT_TABLE(MyCanvas, wxWindow)
	EVT_KEY_DOWN(MyCanvas::OnKeyDown)
    EVT_MOUSE_EVENTS(MyCanvas::OnMouseEvent)
    EVT_SIZE(MyCanvas::OnSize)
    EVT_PAINT(MyCanvas::OnPaint)
END_EVENT_TABLE()


//---------------------------------------------------------------------------------------
MyCanvas::MyCanvas(wxFrame *frame, LomseDoorway& lomse)
    : wxWindow(frame, wxID_ANY)
    , m_lomse(lomse)
	, m_pPresenter(NULL)
	, m_buffer(NULL)
	, m_view_needs_redraw(true)
{
}

//---------------------------------------------------------------------------------------
MyCanvas::~MyCanvas()
{
	delete_rendering_buffer();

    //delete the Presenter. This will also delete the Document, the Interactor,
    //the View and other related objects
    delete m_pPresenter;
}

//---------------------------------------------------------------------------------------
void MyCanvas::open_file(const wxString& fullname)
{
    //create a new View
    std::string filename( fullname.mb_str(wxConvUTF8) );
    delete m_pPresenter;
    m_pPresenter = m_lomse.open_document(ViewFactory::k_view_vertical_book,
                                         filename);

    //get the pointer to the interactor, set the rendering buffer and register for
    //receiving desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        //connect the View with the window buffer
        spInteractor->set_rendering_buffer(&m_rbuf_window);

        //ask to receive desired events
        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
    }

    //render the new score
    m_view_needs_redraw = true;
    Refresh(false /* don't erase background */);
}

//---------------------------------------------------------------------------------------
void MyCanvas::OnSize(wxSizeEvent& WXUNUSED(event))
{
    wxSize size = this->GetClientSize();
    create_rendering_buffer(size.GetWidth(), size.GetHeight());

    Refresh(false /* don't erase background */);
}

//---------------------------------------------------------------------------------------
void MyCanvas::OnPaint(wxPaintEvent& event)
{
    if (!m_pPresenter)
        event.Skip(false);
    else
    {
        update_rendering_buffer_if_needed();
        wxPaintDC dc(this);
        copy_buffer_on_dc(dc);
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::update_rendering_buffer_if_needed()
{
    if (m_view_needs_redraw)
        update_view_content();

    m_view_needs_redraw = false;
}

//---------------------------------------------------------------------------------------
void MyCanvas::delete_rendering_buffer()
{
    delete m_buffer;
}

//---------------------------------------------------------------------------------------
void MyCanvas::create_rendering_buffer(int width, int height)
{
    //creates a bitmap of specified size and associates it to the rendering
    //buffer for the view. Any existing buffer is automatically deleted

    // allocate a new rendering buffer
	delete_rendering_buffer();
    m_nBufWidth = width;
    m_nBufHeight = height;
    m_buffer = new wxImage(width, height);

    //get pointer to wxImage internal bitmap
    m_pdata = m_buffer->GetData();

    //Attach this bitmap to Lomse rendering buffer
    #define BYTES_PER_PIXEL 3   //wxImage  has RGB, 24 bits format
    int stride = m_nBufWidth * BYTES_PER_PIXEL;     //number of bytes per row
    m_rbuf_window.attach(m_pdata, m_nBufWidth, m_nBufHeight, stride);

    m_view_needs_redraw = true;
}

//-------------------------------------------------------------------------
void MyCanvas::open_test_document()
{
    //Normally you will load the content of a file. But in this
    //simple example we will create an empty document and define its content
    //from a text string

    //first, we will create a 'presenter'. It takes care of cretaing and maintaining
    //all objects and relationships between the document, its views and the interactors
    //to interct with the view
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(ViewFactory::k_view_vertical_book,
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
        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::force_redraw()
{
    update_view_content();
    update_window();
}

//---------------------------------------------------------------------------------------
void MyCanvas::wrapper_update_window(void* pThis, SpEventInfo pEvent)
{
    static_cast<MyCanvas*>(pThis)->update_window();
}

//---------------------------------------------------------------------------------------
void MyCanvas::update_window()
{
    // Invoking update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without neither calling
    // any lomse methods nor generating any events  (i.e. Refresh() window)

    wxClientDC dc(this);
    copy_buffer_on_dc(dc);
}

//---------------------------------------------------------------------------------------
void MyCanvas::copy_buffer_on_dc(wxDC& dc)
{
    if (!m_buffer || !m_buffer->IsOk())
        return;

    wxBitmap bitmap(*m_buffer);
    dc.DrawBitmap(bitmap, 0, 0, false /* don't use mask */);
}

//-------------------------------------------------------------------------
void MyCanvas::update_view_content()
{
    //request the view to re-draw the bitmap

    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        spInteractor->redraw_bitmap();
}

//---------------------------------------------------------------------------------------
void MyCanvas::OnKeyDown(wxKeyEvent& event)
{
    if (!m_pPresenter) return;

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
void MyCanvas::on_key(int x, int y, unsigned key, unsigned flags)
{
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        switch (key)
        {
            case 'D':
                spInteractor->switch_task(TaskFactory::k_task_drag_view);
                break;
            case 'S':
                spInteractor->switch_task(TaskFactory::k_task_selection);
                break;
            case '+':
                spInteractor->zoom_in(x, y);
                force_redraw();
                break;
            case '-':
                spInteractor->zoom_out(x, y);
                force_redraw();
                break;
            default:
               return;
        }
    }
}

//-------------------------------------------------------------------------
void MyCanvas::zoom_in()
{
    if (!m_pPresenter) return;

    //do zoom in centered on window center
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        wxSize size = this->GetClientSize();
        spInteractor->zoom_in(size.GetWidth()/2, size.GetHeight()/2);
        force_redraw();
    }
}

//-------------------------------------------------------------------------
void MyCanvas::zoom_out()
{
    if (!m_pPresenter) return;

    //do zoom out centered on window center
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        wxSize size = this->GetClientSize();
        spInteractor->zoom_out(size.GetWidth()/2, size.GetHeight()/2);
        force_redraw();
    }
}

//---------------------------------------------------------------------------------------
unsigned MyCanvas::get_mouse_flags(wxMouseEvent& event)
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
unsigned MyCanvas::get_keyboard_flags(wxKeyEvent& event)
{
    unsigned flags = 0;
    if (event.ShiftDown())   flags |= k_kbd_shift;
    if (event.AltDown()) flags |= k_kbd_alt;
    if (event.ControlDown()) flags |= k_kbd_ctrl;
    return flags;
}

//-------------------------------------------------------------------------
void MyCanvas::OnMouseEvent(wxMouseEvent& event)
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        wxEventType nEventType = event.GetEventType();
        wxPoint pos = event.GetPosition();
        unsigned flags = get_mouse_flags(event);

        if (nEventType==wxEVT_LEFT_DOWN)
        {
            flags |= k_mouse_left;
            spInteractor->on_mouse_button_down(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_LEFT_UP)
        {
            flags |= k_mouse_left;
            spInteractor->on_mouse_button_up(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_RIGHT_DOWN)
        {
            flags |= k_mouse_right;
            spInteractor->on_mouse_button_down(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_RIGHT_UP)
        {
            flags |= k_mouse_right;
            spInteractor->on_mouse_button_up(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_MOTION)
            spInteractor->on_mouse_move(pos.x, pos.y, flags);
    }
}
