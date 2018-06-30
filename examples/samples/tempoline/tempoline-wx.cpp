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
    #include <wx/timer.h>
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

protected:
    //accessors
    MyCanvas* get_active_canvas() const { return m_canvas; }

    //event handlers
    void on_test_start(wxCommandEvent& WXUNUSED(event));
    void on_test_stop(wxCommandEvent& WXUNUSED(event));

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
    void start_test();
    void stop_test();

protected:
    //event handlers
    void OnPaint(wxPaintEvent& WXUNUSED(event));
    void OnSize(wxSizeEvent& event);
    void on_tempo_line_timer(wxTimerEvent& event);

    void delete_rendering_buffer();
    void create_rendering_buffer(int width, int height);
    void copy_buffer_on_dc(wxDC& dc);
    void update_rendering_buffer_if_needed();
    void force_redraw();
    void update_window();

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

    //external playback simulation
    wxTimer m_playbackTimer;        //Timer for controlling tempo line movement
    int     m_nBeatTime;            //Interval for moving the tempo line
    int     m_beat;                 //current beat
    int     m_measure;              //current measure
    ImoId   m_scoreId;              //Id of the score to playback


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
    k_menu_test_start = wxID_HIGHEST + 1,
    k_menu_test_stop,
    k_id_tempo_line_timer,

    //using standard IDs
    //it is important for the id corresponding to the "About" command to have
    //this standard value as otherwise it won't be handled properly under Mac
    //(where it is special and put into the "Apple" menu)
    k_menu_file_quit = wxID_EXIT,
};

//---------------------------------------------------------------------------------------
// events table
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(k_menu_file_quit, MyFrame::OnQuit)
    EVT_MENU(k_menu_test_start, MyFrame::on_test_start)
    EVT_MENU(k_menu_test_stop, MyFrame::on_test_stop)
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
    fileMenu->Append(k_menu_file_quit, _T("E&xit"));

    wxMenu* testMenu = new wxMenu;
    testMenu->Append(k_menu_test_start, _T("&Start..."));
    testMenu->Append(k_menu_test_stop, _T("S&top..."));

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(testMenu, _T("&Test"));

    SetMenuBar(menuBar);
}

//---------------------------------------------------------------------------------------
void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true /*force to close*/);
}

//---------------------------------------------------------------------------------------
void MyFrame::initialize_lomse()
{
    // Lomse knows nothing about windows. It renders everything on bitmaps and the
    // user application uses them. For instance, to display it on a wxWindows.
    // Lomse supports a lot of bitmap formats and pixel formats. Therefore, before
    // using the Lomse library you MUST specify which bitmap format to use.
    //
    // For wxWidgets, I would suggest using a platform independent format. So
    // I will use a wxImage as the rendering  buffer. wxImage is platform independent
    // and its buffer is an array of characters in RGBRGBRGB... format,  in the
    // top-to-bottom, left-to-right order. That is, the first RGB triplet corresponds
    // to the first pixel of the first row; the second RGB triplet, to the second
    // pixel of the first row, and so on until the end of the first row,
    // with second row following after it and so on.
    // Therefore, the pixel format is RGB 24 bits.
    //
    // Let's define the required information:

        //the pixel format
        int pixel_format = k_pix_format_rgb24;  //RGB 24bits

        //the desired resolution. For Linux and Windows 96 pixels per inch works ok.
        int resolution = 96;    //96 ppi

        //Normal y axis direction is 0 coordinate at top and increase downwards. You
        //must specify if you would like just the opposite behavior. For Windows and
        //Linux the default behavior is the right behavior.
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
void MyFrame::on_test_start(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->start_test();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_test_stop(wxCommandEvent& WXUNUSED(event))
{
     get_active_canvas()->stop_test();
}


//=======================================================================================
// MyCanvas implementation
//=======================================================================================

BEGIN_EVENT_TABLE(MyCanvas, wxWindow)
    EVT_SIZE(MyCanvas::OnSize)
    EVT_PAINT(MyCanvas::OnPaint)
    EVT_TIMER(k_id_tempo_line_timer, MyCanvas::on_tempo_line_timer)
END_EVENT_TABLE()


//---------------------------------------------------------------------------------------
MyCanvas::MyCanvas(wxFrame *frame, LomseDoorway& lomse)
    : wxWindow(frame, wxID_ANY)
    , m_lomse(lomse)
	, m_pPresenter(NULL)
	, m_buffer(NULL)
	, m_view_needs_redraw(true)
    , m_playbackTimer(this, k_id_tempo_line_timer)
    , m_nBeatTime(1000)     //1000 milliseconds = 1 sec.
    , m_beat(0)
    , m_measure(0)
{
}

//---------------------------------------------------------------------------------------
MyCanvas::~MyCanvas()
{
    m_playbackTimer.Stop();
	delete_rendering_buffer();

    //delete the Presenter. This will also delete the Document, the Interactor,
    //the View and other related objects
    delete m_pPresenter;
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
    m_pPresenter = m_lomse.new_document(k_view_vertical_book,
        "(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
        "(instrument (staves 2) (musicData "
        "(clef G p1)(clef F4 p2)(key C)(time 4 4)"
        "(n c4 s g+ p1)(n d4 s)(n c4 s)(n d4 s g-)"
        "(n e4 s g+ p1)(n f4 s)(n e4 s)(n f4 s g-)"
        "(n f4 s g+ p1)(n g4 s)(n f4 s)(n g4 s g-)"
        "(n g4 s g+ p1)(n a4 s)(n g4 s)(n a4 s g-)"
        "(goBack start)"
        "(chord (n c3 q p2)(n e3 q)(n g3 q))"
        "(r q)"
        "(chord (n a2 q p2)(n c3 q)(n f3 q))"
        "(r q)"
        "(barline)"
        "(chord (n g3 q p1)(n d4 q))"
        "(r e)(n g5 e)"
        "(n g5 s g+)(n f5 s)(n g5 e g-)"
        "(n c4 q)"
        "(goBack start)"
        "(n g2 q p2)"
        "(n d3 e g+)(n d3 e g-)"
        "(n b3 e g+)(n a3 s)(n g3 s g-)"
        "(chord (n g3 q)(n e3 q)(n c3 q))"
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

        //get the score and enable visual tracking
        //AWARE: Then next line of code is just an example, in which it is assumed that
        //the score to play is the first element in the document.
        //In a real application, as the document could contain texts, images and many
        //scores, you shoud get the pointer to the score to play in a suitable way.
        Document* pDoc = m_pPresenter->get_document_raw_ptr();
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_im_root()->get_content_item(0) );
        if (pScore)
        {
            m_scoreId = pScore->get_id();
            spInteractor->set_visual_tracking_mode(k_tracking_tempo_line);
        }
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::force_redraw()
{
    update_view_content();
    update_window();
}

//---------------------------------------------------------------------------------------
void MyCanvas::wrapper_update_window(void* pThis, SpEventInfo WXUNUSED(pEvent))
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
void MyCanvas::start_test()
{
    m_beat = -1;
    m_measure = 0;
    m_playbackTimer.Start(m_nBeatTime, wxTIMER_CONTINUOUS);
}

//---------------------------------------------------------------------------------------
void MyCanvas::stop_test()
{
    m_playbackTimer.Stop();

    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        spInteractor->remove_all_visual_tracking();
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::on_tempo_line_timer(wxTimerEvent& WXUNUSED(event))
{
    //It is time to advance the tempo line

    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        ++m_beat;
        if (m_beat == 4)
        {
            m_beat = 0;
            ++m_measure;
        }
        spInteractor->move_tempo_line(m_scoreId, m_measure, m_beat);
    }
}
