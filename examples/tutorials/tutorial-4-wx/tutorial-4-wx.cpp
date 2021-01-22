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
    #include <wx/choicdlg.h>
    #include <wx/arrstr.h>
#endif

#include <iostream>

//lomse headers
#include <lomse_doorway.h>
#include <lomse_document.h>
#include <lomse_graphic_view.h>
#include <lomse_interactor.h>
#include <lomse_presenter.h>
#include <lomse_events.h>
#include <lomse_score_player.h>
#include <lomse_player_gui.h>
#include <lomse_tasks.h>


using namespace lomse;

//wxMidi headers
#include <wxMidi.h>


//---------------------------------------------------------------------------------------
// Define the application
class MyApp: public wxApp
{
public:
    bool OnInit();

};

//forward declarations
class MyCanvas;
class MidiServer;


//---------------------------------------------------------------------------------------
// MyVisualTrackingEvent
//      An event to signal different actions related to
//      score visual tracking while it is being played back.
//---------------------------------------------------------------------------------------

DECLARE_EVENT_TYPE( MY_EVT_VISUAL_TRACKING_TYPE, -1 )

class MyVisualTrackingEvent : public wxEvent
{
private:
    SpEventVisualTracking m_pEvent;   //lomse event

public:
    MyVisualTrackingEvent(SpEventVisualTracking pEvent, int id = 0)
        : wxEvent(id, MY_EVT_VISUAL_TRACKING_TYPE)
        , m_pEvent(pEvent)
    {
    }

    // copy constructor
    MyVisualTrackingEvent(const MyVisualTrackingEvent& event)
        : wxEvent(event)
        , m_pEvent( event.m_pEvent )
    {
    }

    // clone constructor. Required for sending with wxPostEvent()
    virtual wxEvent *Clone() const { return new MyVisualTrackingEvent(*this); }

    // accessors
    SpEventVisualTracking get_lomse_event() { return m_pEvent; }
};

typedef void (wxEvtHandler::*VisualTrackingEventFunction)(MyVisualTrackingEvent&);

#define MY_EVT_VISUAL_TRACKING(fn) \
    DECLARE_EVENT_TABLE_ENTRY( MY_EVT_VISUAL_TRACKING_TYPE, wxID_ANY, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( VisualTrackingEventFunction, & fn ), (wxObject *) nullptr ),




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

    //callback wrappers
    static void wrapper_lomse_event(void* pThis, SpEventInfo pEvent);

protected:
    //accessors
    MyCanvas* get_active_canvas() const { return m_canvas; }

    //event handlers
    void OnOpen(wxCommandEvent& WXUNUSED(event));
    void OnZoomIn(wxCommandEvent& WXUNUSED(event));
    void OnZoomOut(wxCommandEvent& WXUNUSED(event));
    void on_midi_settings(wxCommandEvent& WXUNUSED(event));
    void on_sound_test(wxCommandEvent& WXUNUSED(event));
    void on_play_start(wxCommandEvent& WXUNUSED(event));
    void on_play_stop(wxCommandEvent& WXUNUSED(event));
    void on_play_pause(wxCommandEvent& WXUNUSED(event));
    void on_lomse_event(SpEventInfo pEvent);

    //lomse related
    void initialize_lomse();

    void create_menu();
    void show_midi_settings_dlg();

    LomseDoorway m_lomse;        //the Lomse library doorway
    MyCanvas* m_canvas;

    //sound related
    MidiServer* get_midi_server();
    ScorePlayer* get_score_player();

    MidiServer* m_pMidi;
    ScorePlayer* m_pPlayer;

    DECLARE_EVENT_TABLE()
};

//---------------------------------------------------------------------------------------
// MyCanvas is a window on which we show the scores
class MyCanvas : public wxWindow, public PlayerNoGui
{
public:
    MyCanvas(wxFrame *frame, LomseDoorway& lomse, ScorePlayer* pPlayer);
    ~MyCanvas();

    void update_view_content();

    //callback wrappers
    static void wrapper_update_window(void* pThis, SpEventInfo pEvent);

    //commands
    void open_test_document();
    void open_file(const wxString& fullname);
    void zoom_in();
    void zoom_out();
    void play_start();
    void play_stop();
    void play_pause();

protected:
    //event handlers
    void OnPaint(wxPaintEvent& WXUNUSED(event));
    void OnSize(wxSizeEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void on_visual_tracking(MyVisualTrackingEvent& event);

    void delete_rendering_buffer();
    void create_rendering_buffer(int width, int height);
    void copy_buffer_on_dc(wxDC& dc);
    void update_rendering_buffer_if_needed();
    void force_redraw();
    void update_window();
    void on_key(int x, int y, unsigned key, unsigned flags);
    unsigned get_keyboard_flags(wxKeyEvent& event);
    unsigned get_mouse_flags(wxMouseEvent& event);

    // In this first tutorial we are just going to display an score on the window.
    // Let's define the necessary variables:
    LomseDoorway&   m_lomse;        //the Lomse library doorway
    Presenter*      m_pPresenter;

    //the Lomse View renders its content on a bitmap. To manage it, Lomse
    wxImage*            m_buffer;		//the image to serve as buffer

    // for score playback
    ScorePlayer* m_pPlayer;

    //some additinal variables
    bool    m_view_needs_redraw;      //to control when the View must be re-drawn

    DECLARE_EVENT_TABLE()
};

//---------------------------------------------------------------------------------------
//stores current MIDI configuration and interfaces with the MIDI API
class MidiServer : public MidiServerBase
{
protected:
    wxMidiSystem*  m_pMidiSystem;       //MIDI system
    wxMidiOutDevice*  m_pMidiOut;       //out device object

    //MIDI configuration information
    int		m_nOutDevId;
    int		m_nVoiceChannel;

public:
    MidiServer();
    ~MidiServer();

    //get number of available Midi devices
    int count_devices();

    //set up configuration
    void set_out_device(int nOutDevId);

    //create some sounds to test Midi
    void test_midi_out();

    //mandatory overrides from MidiServerBase
    void program_change(int channel, int instr);
    void voice_change(int channel, int instr);
    void note_on(int channel, int pitch, int volume);
    void note_off(int channel, int pitch, int volume);
    void all_sounds_off();
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
// MyEvents implementation
//=======================================================================================
DEFINE_EVENT_TYPE( MY_EVT_VISUAL_TRACKING_TYPE )



//=======================================================================================
// MyFrame implementation
//=======================================================================================

//---------------------------------------------------------------------------------------
// constants for menu IDs
enum
{
    //menu File
    k_menu_file_open = wxID_HIGHEST + 1,

    //menu Sound
    k_menu_play_start,
    k_menu_play_stop,
    k_menu_play_pause,
    k_menu_midi_settings,
    k_menu_midi_test,

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

    EVT_MENU(k_menu_midi_settings, MyFrame::on_midi_settings)
    EVT_MENU(k_menu_midi_test, MyFrame::on_sound_test)
    EVT_MENU(k_menu_play_start, MyFrame::on_play_start)
    EVT_MENU(k_menu_play_stop, MyFrame::on_play_stop)
    EVT_MENU(k_menu_play_pause, MyFrame::on_play_pause)
END_EVENT_TABLE()

//---------------------------------------------------------------------------------------
MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, _T("Lomse sample for wxWidgets"),
			  wxDefaultPosition, wxSize(850, 600))
    , m_pMidi(nullptr)
    , m_pPlayer(nullptr)
{
    create_menu();
    initialize_lomse();
    show_midi_settings_dlg();

    // create our one and only child -- it will take our entire client area
    m_canvas = new MyCanvas(this, m_lomse, get_score_player());
    wxSizer *sz = new wxBoxSizer(wxVERTICAL);
    sz->Add(m_canvas, 3, wxGROW);
    SetSizer(sz);
}

//---------------------------------------------------------------------------------------
MyFrame::~MyFrame()
{
    delete m_pMidi;
    delete m_pPlayer;
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

    wxMenu *soundMenu = new wxMenu;
    soundMenu->Append(k_menu_play_start, _T("&Play"));
    soundMenu->Append(k_menu_play_stop, _T("&Stop"));
    soundMenu->Append(k_menu_play_pause, _T("Pause/&Resume"));
    soundMenu->Append(k_menu_midi_settings, _T("&Midi settings"));
    soundMenu->Append(k_menu_midi_test, _T("Midi &test"));

    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(k_menu_help_about, _T("&About"));

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(zoomMenu, _T("&Zoom"));
    menuBar->Append(soundMenu, _T("&Sound"));
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
    wxMessageBox(_T("Lomse: sample 4 for wxWidgets"),
                 _T("About wxWidgets Lomse sample"),
                 wxOK | wxICON_INFORMATION, this);
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
    // Let's define the requiered information:

        //the pixel format
        int pixel_format = k_pix_format_rgb24;  //RGB 24bits

        //the desired resolution: 96 pixels per inch
        int resolution = 96;

    //initialize the library with these values
    m_lomse.init_library(pixel_format, resolution);

    //set required callbacks
    m_lomse.set_notify_callback(this, wrapper_lomse_event);
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
void MyFrame::wrapper_lomse_event(void* pThis, SpEventInfo pEvent)
{
    static_cast<MyFrame*>(pThis)->on_lomse_event(pEvent);
}

//---------------------------------------------------------------------------------------
void MyFrame::on_lomse_event(SpEventInfo pEvent)
{
    MyCanvas* pCanvas = get_active_canvas();

    switch (pEvent->get_event_type())
    {
        case k_tracking_event:
        {
            if (pCanvas)
            {
                SpEventVisualTracking pEv(
                    static_pointer_cast<EventVisualTracking>(pEvent) );
                MyVisualTrackingEvent event(pEv);
                ::wxPostEvent(pCanvas, event);
            }
            break;
        }

        default:
            ;
    }
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

//---------------------------------------------------------------------------------------
void MyFrame::on_play_start(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->play_start();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_play_stop(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->play_stop();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_play_pause(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->play_pause();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_midi_settings(wxCommandEvent& WXUNUSED(event))
{
    show_midi_settings_dlg();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_sound_test(wxCommandEvent& WXUNUSED(event))
{
    MidiServer* pMidi = get_midi_server();
    if (!pMidi) return;
    pMidi->test_midi_out();
}

//---------------------------------------------------------------------------------------
void MyFrame::show_midi_settings_dlg()
{
    wxArrayString outDevices;
    vector<int> deviceIndex;

    //get available Midi out devices
    MidiServer* pMidi = get_midi_server();
    int nNumDevices = pMidi->count_devices();
    for (int i = 0; i < nNumDevices; i++)
    {
        wxMidiOutDevice device(i);
        if (device.IsOutputPort())
        {
            outDevices.Add( device.DeviceName() );
            deviceIndex.push_back(i);
        }
    }

    int iSel = ::wxGetSingleChoiceIndex(
                            _T("Select Midi output device to use:"),    //message
                            _T("Midi settings dlg"),                    //window title
                            outDevices,
                            this                                        //parent window
                       );
    if (iSel == -1)
    {
        //the user pressed cancel
        //
    }
    else
    {
        //set current selection
        MidiServer* pMidi = get_midi_server();
        int deviceID = deviceIndex[iSel];   //output device
        pMidi->set_out_device(deviceID);
    }
}

//---------------------------------------------------------------------------------------
MidiServer* MyFrame::get_midi_server()
{
    if (!m_pMidi)
        m_pMidi = new MidiServer();
    return m_pMidi;
}

//---------------------------------------------------------------------------------------
ScorePlayer* MyFrame::get_score_player()
{
    if (!m_pPlayer)
    {
        MidiServer* pMidi = get_midi_server();
        m_pPlayer = m_lomse.create_score_player(pMidi);
    }
    return m_pPlayer;
}



//=======================================================================================
// MyCanvas implementation
//=======================================================================================

BEGIN_EVENT_TABLE(MyCanvas, wxWindow)
	EVT_KEY_DOWN(MyCanvas::OnKeyDown)
    EVT_MOUSE_EVENTS(MyCanvas::OnMouseEvent)
    EVT_SIZE(MyCanvas::OnSize)
    EVT_PAINT(MyCanvas::OnPaint)
    MY_EVT_VISUAL_TRACKING(MyCanvas::on_visual_tracking)
END_EVENT_TABLE()


//---------------------------------------------------------------------------------------
MyCanvas::MyCanvas(wxFrame *frame, LomseDoorway& lomse, ScorePlayer* pPlayer)
    : wxWindow(frame, wxID_ANY)
    , PlayerNoGui(60L /*tempo 60 MM*/, false /*no count off*/, false /*no metronome clicks*/)
    , m_lomse(lomse)
	, m_pPresenter(nullptr)
	, m_buffer(nullptr)
	, m_pPlayer(pPlayer)
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
    m_pPresenter = m_lomse.open_document(k_view_vertical_book,
                                         filename);

    //get the pointer to the interactor and register for receiving desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
    }

    //render the new score
    wxSize size = this->GetClientSize();
    create_rendering_buffer(size.GetWidth(), size.GetHeight());
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
    //creates a bitmap of specified size and defines it s the rendering
    //buffer for the view.

    delete_rendering_buffer();

    // allocate a new rendering buffer
    m_buffer = new wxImage(width, height);

    //get pointer to wxImage internal bitmap
    unsigned char* pdata = m_buffer->GetData();

    //use this bitmap as Lomse rendering buffer
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        spInteractor->set_rendering_buffer(pdata, width, height);
    }

    m_view_needs_redraw = true;
}

//-------------------------------------------------------------------------
void MyCanvas::open_test_document()
{
    //Normally you will load the content of a file. But in this
    //simple example we will create an empty document and define its content
    //from a text string

    //first, we will create a 'presenter'. It takes care of creating and maintaining
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

    //get the pointer to the interactor and register for receiving desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
    }

    //render the new score
    wxSize size = this->GetClientSize();
    create_rendering_buffer(size.GetWidth(), size.GetHeight());
    Refresh(false /* don't erase background */);
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


//-------------------------------------------------------------------------
void MyCanvas::play_start()
{
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        //AWARE: It is assumed that the score to play is the first score in
        //the document. For a real application, as the document could contain
        //texts, images and many scores, you shoud get the right score to play.
        ADocument doc = m_pPresenter->get_document();
        AScore score = doc.first_score();
        if (score.is_valid())
        {
            m_pPlayer->load_score(score, this);
            m_pPlayer->play(k_do_visual_tracking, 0, spInteractor.get());
        }
    }
}

//-------------------------------------------------------------------------
void MyCanvas::play_stop()
{
    m_pPlayer->stop();
}

//-------------------------------------------------------------------------
void MyCanvas::play_pause()
{
    m_pPlayer->pause();
}

//---------------------------------------------------------------------------------------
void MyCanvas::on_visual_tracking(MyVisualTrackingEvent& event)
{
    SpEventVisualTracking pEv = event.get_lomse_event();
    WpInteractor wpInteractor = pEv->get_interactor();
    if (SpInteractor sp = wpInteractor.lock())
        sp->on_visual_tracking(pEv);
}


//=======================================================================================
// MidiServer implementation
//=======================================================================================
MidiServer::MidiServer()
    : m_pMidiSystem( wxMidiSystem::GetInstance() )
    , m_pMidiOut(nullptr)
    , m_nOutDevId(-1)
    , m_nVoiceChannel(0)    // 0 based. So this is channel 1
{
}

//---------------------------------------------------------------------------------------
MidiServer::~MidiServer()
{
    if (m_pMidiOut)
        m_pMidiOut->Close();

    delete m_pMidiOut;
    delete m_pMidiSystem;
}

//---------------------------------------------------------------------------------------
void MidiServer::set_out_device(int nOutDevId)
{
    wxMidiError nErr;

    //if out device Id has changed close current device and open the new one
    if (!m_pMidiOut || (m_nOutDevId != nOutDevId))
    {
        //close current device
         if (m_pMidiOut)
         {
            nErr = m_pMidiOut->Close();
            delete m_pMidiOut;
            m_pMidiOut = nullptr;
            if (nErr)
            {
                wxMessageBox( wxString::Format(
                    _T("Error %d while closing Midi device: %s \n")
                    , nErr, m_pMidiSystem->GetErrorText(nErr).c_str() ));
                return;
            }
        }

        //open new one
        m_nOutDevId = nOutDevId;
        if (m_nOutDevId != -1)
        {
            try
            {
                m_pMidiOut = new wxMidiOutDevice(m_nOutDevId);
                nErr = m_pMidiOut->Open(0, nullptr);        // 0 latency, no driver user info
            }
            catch(...)      //handle all exceptions
            {
				wxLogMessage(_T("[MidiServer::set_out_device] Crash opening Midi device"));
				return;
            }

            if (nErr)
				wxMessageBox( wxString::Format(
                    _T("Error %d opening Midi device: %s \n")
                    , nErr, m_pMidiSystem->GetErrorText(nErr).c_str() ));
            else
				wxMessageBox(_T("Midi out device correctly set."));
        }
    }
}

//---------------------------------------------------------------------------------------
void MidiServer::program_change(int channel, int instr)
{
    m_pMidiOut->ProgramChange(channel, instr);
}

//---------------------------------------------------------------------------------------
void MidiServer::voice_change(int channel, int instrument)
{
    m_nVoiceChannel = channel;
    if (m_pMidiOut)
    {
        wxMidiError nErr = m_pMidiOut->ProgramChange(channel, instrument);
        if (nErr)
        {
            wxMessageBox( wxString::Format(
				_T("Error %d in ProgramChange:\n%s")
                , nErr, m_pMidiSystem->GetErrorText(nErr).c_str() ));
        }
    }
}

//---------------------------------------------------------------------------------------
void MidiServer::note_on(int channel, int pitch, int volume)
{
    m_pMidiOut->NoteOn(channel, pitch, volume);
}

//---------------------------------------------------------------------------------------
void MidiServer::note_off(int channel, int pitch, int volume)
{
    m_pMidiOut->NoteOff(channel, pitch, volume);
}

//---------------------------------------------------------------------------------------
void MidiServer::all_sounds_off()
{
    m_pMidiOut->AllSoundsOff();
}

//---------------------------------------------------------------------------------------
void MidiServer::test_midi_out()
{
    if (!m_pMidiOut) return;

    //Play a scale
    int scale[] = { 60, 62, 64, 65, 67, 69, 71, 72 };
    #define SCALE_SIZE 8

    for (int i = 0; i < SCALE_SIZE; i++)
    {
        m_pMidiOut->NoteOn(m_nVoiceChannel, scale[i], 100);
        ::wxMilliSleep(200);    // wait 200ms
        m_pMidiOut->NoteOff(m_nVoiceChannel, scale[i], 100);
    }
}

//---------------------------------------------------------------------------------------
int MidiServer::count_devices()
{
    return m_pMidiSystem->CountDevices();
}
