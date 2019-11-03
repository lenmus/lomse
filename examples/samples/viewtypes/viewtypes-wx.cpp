//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

//TESTS
#include <lomse_internal_model.h>       //to use ImoScore
#include <lomse_graphical_model.h>      //to dump GM


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
//global helper methods
wxString to_wx_string(const std::string& string)
{
    return wxString( string.c_str(), wxConvUTF8 );
}

std::string to_std_string(const wxString& wxstring)
{
    return std::string( wxstring.mb_str(wxConvUTF8) );
}

//---------------------------------------------------------------------------------------
// MyVisualTrackingEvent
//      An event to signal different actions related to
//      visual tracking effects for score playback.
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
    wxStaticCastEvent( VisualTrackingEventFunction, & fn ), (wxObject *) NULL ),


//---------------------------------------------------------------------------------------
// MyUpdateViewportEvent
//      An event to signal the need to repaint the window and to update scrollbars
//      due to an auto-scroll while the score is being played back.
//---------------------------------------------------------------------------------------

DECLARE_EVENT_TYPE( MY_EVT_UPDATE_VIEWPORT_TYPE, -1 )

class MyUpdateViewportEvent : public wxEvent
{
private:
    SpEventUpdateViewport m_pEvent;   //lomse event

public:
    MyUpdateViewportEvent(SpEventUpdateViewport pEvent, int id = 0)
        : wxEvent(id, MY_EVT_UPDATE_VIEWPORT_TYPE)
        , m_pEvent(pEvent)
    {
    }

    // copy constructor
    MyUpdateViewportEvent(const MyUpdateViewportEvent& event)
        : wxEvent(event)
        , m_pEvent( event.m_pEvent )
    {
    }

    // clone constructor. Required for sending with wxPostEvent()
    virtual wxEvent *Clone() const { return new MyUpdateViewportEvent(*this); }

    // accessors
    SpEventUpdateViewport get_lomse_event() { return m_pEvent; }
};

typedef void (wxEvtHandler::*UpdateViewportEventFunction)(MyUpdateViewportEvent&);

#define MY_EVT_UPDATE_VIEWPORT(fn) \
    DECLARE_EVENT_TABLE_ENTRY( MY_EVT_UPDATE_VIEWPORT_TYPE, wxID_ANY, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( UpdateViewportEventFunction, & fn ), (wxObject *) NULL ),




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
    void on_quit(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);

    //callback wrappers
    static void wrapper_lomse_event(void* pThis, SpEventInfo pEvent);

protected:
    //menus
    wxMenu* m_viewMenu;
	wxMenu* m_effectMenu;

    //accessors
    MyCanvas* get_active_canvas() const { return m_canvas; }

    //event handlers
    void on_open_file(wxCommandEvent& WXUNUSED(event));
    void on_open_test(wxCommandEvent& WXUNUSED(event));
    void on_zoom_in(wxCommandEvent& WXUNUSED(event));
    void on_zoom_out(wxCommandEvent& WXUNUSED(event));
    void on_midi_settings(wxCommandEvent& WXUNUSED(event));
    void on_sound_test(wxCommandEvent& WXUNUSED(event));
    void on_play_start(wxCommandEvent& WXUNUSED(event));
    void on_play_stop(wxCommandEvent& WXUNUSED(event));
    void on_play_pause(wxCommandEvent& WXUNUSED(event));
    void on_lomse_event(SpEventInfo pEvent);
    void on_debug_dump_gmodel(wxCommandEvent& WXUNUSED(event));

    //lomse related
    void initialize_lomse();

    void create_menu();
    void show_midi_settings_dlg();
    int select_view_type();

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
    void open_test_document(int viewType);
    void open_file(const wxString& fullname, int viewType);
    void zoom_in();
    void zoom_out();
    void play_start(int effects);
    void play_stop();
    void play_pause();

    //debug
    void on_debug_dump_gmodel();

protected:
    //event handlers
    void on_paint(wxPaintEvent& WXUNUSED(event));
    void on_size(wxSizeEvent& event);
    void on_key_down(wxKeyEvent& event);
    void on_mouse_event(wxMouseEvent& event);
    void on_visual_tracking(MyVisualTrackingEvent& event);
    void on_update_viewport(MyUpdateViewportEvent& event);

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

//---------------------------------------------------------------------------------------
//A general dialog for displaying texts
class DlgDebug : public wxDialog
{
   wxDECLARE_DYNAMIC_CLASS(DlgDebug);

public:
    DlgDebug(wxWindow* parent, wxString sTitle, wxString sData, bool fSave = true);
    virtual ~DlgDebug();

    void OnOK(wxCommandEvent& WXUNUSED(event));
    void OnSave(wxCommandEvent& WXUNUSED(event));

    void AppendText(wxString sText);

private:
    wxTextCtrl*     m_pTxtData;
    bool            m_fSave;        //true to include 'Save' button

    wxDECLARE_EVENT_TABLE();
};


//=======================================================================================
// MyApp implementation
//=======================================================================================

IMPLEMENT_APP(MyApp)

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    //DEBUG -----------------------------------------------------------------------------
    logger.set_logging_mode(Logger::k_debug_mode);
    logger.set_logging_areas(Logger::k_events | Logger::k_score_player);

	// For debugging: send wxWidgets log messages to a file
    wxString sUserId = ::wxGetUserId();
    wxString sLogFile = "Debug_log.txt";
	wxLog *logger = new wxLogStderr( wxFopen(sLogFile.wx_str(), "w") );
	wxLog::SetActiveTarget(logger);
	wxLogMessage("[ApplicationScope::create_logger] Log messages derived to file.");
	//END DEBUG -------------------------------------------------------------------------

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
DEFINE_EVENT_TYPE( MY_EVT_UPDATE_VIEWPORT_TYPE )



//=======================================================================================
// MyFrame implementation
//=======================================================================================

//---------------------------------------------------------------------------------------
// constants for menu IDs
enum
{
    //menu File
    k_menu_file_open = wxID_HIGHEST + 1,
    k_menu_file_open_test,

    //menu Sound
    k_menu_play_start,
    k_menu_play_stop,
    k_menu_play_pause,
    k_menu_midi_settings,
    k_menu_midi_test,

    //menu View type
    k_menu_view_vertical_book,
    k_menu_view_horizontal_book,
    k_menu_view_single_system,

    //menu Effects
    k_menu_effect_highlight,
    k_menu_effect_tempo_line,

    //menu Debug
	k_menu_debug_dump_gmodel,

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

    //File menu
    EVT_MENU(k_menu_file_quit, MyFrame::on_quit)
    EVT_MENU(k_menu_file_open, MyFrame::on_open_file)
    EVT_MENU(k_menu_file_open_test, MyFrame::on_open_test)

    //Zoom menu
    EVT_MENU(k_menu_zoom_in, MyFrame::on_zoom_in)
    EVT_MENU(k_menu_zoom_out, MyFrame::on_zoom_out)

    //Sound menu
    EVT_MENU(k_menu_midi_settings, MyFrame::on_midi_settings)
    EVT_MENU(k_menu_midi_test, MyFrame::on_sound_test)
    EVT_MENU(k_menu_play_start, MyFrame::on_play_start)
    EVT_MENU(k_menu_play_stop, MyFrame::on_play_stop)
    EVT_MENU(k_menu_play_pause, MyFrame::on_play_pause)

    //Help menu
    EVT_MENU(k_menu_help_about, MyFrame::on_about)

    //Debug menu
    EVT_MENU(k_menu_debug_dump_gmodel, MyFrame::on_debug_dump_gmodel)

END_EVENT_TABLE()

//---------------------------------------------------------------------------------------
MyFrame::MyFrame()
	: wxFrame(NULL, wxID_ANY, _T("Lomse sample for wxWidgets"),
			  wxDefaultPosition, wxSize(850, 400))
    , m_pMidi(NULL)
    , m_pPlayer(NULL)
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
    fileMenu->Append(k_menu_file_open_test, _T("Open test document"));
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

    m_viewMenu = new wxMenu;
    m_viewMenu->Append(k_menu_view_single_system, _T("k_view_single_system"),
                       _T("Use Single system View"), wxITEM_RADIO);
    m_viewMenu->Append(k_menu_view_vertical_book, _T("k_view_vertical_book"),
                       _T("Use Vertical book View"), wxITEM_RADIO);
    m_viewMenu->Append(k_menu_view_horizontal_book, _T("k_view_horizontal_book"),
                       _T("Use Horizontal book View"), wxITEM_RADIO);

    m_effectMenu = new wxMenu;
    m_effectMenu->Append(k_menu_effect_highlight, _T("k_tracking_highlight_notes"),
                         _T("Highlight the notes and rest being played back"), wxITEM_CHECK);
    m_effectMenu->Append(k_menu_effect_tempo_line, _T("k_tracking_tempo_line"),
                         _T("Display a vertical line at beat start"), wxITEM_CHECK);

    wxMenu *debugMenu = new wxMenu;
    debugMenu->Append(k_menu_debug_dump_gmodel, _T("See graphic model"));


    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(zoomMenu, _T("&Zoom"));
    menuBar->Append(soundMenu, _T("&Sound"));
    menuBar->Append(m_viewMenu, _T("&View type"));
    menuBar->Append(m_effectMenu, _T("&Effect"));
    menuBar->Append(debugMenu, _T("&Debug"));
    menuBar->Append(helpMenu, _T("&Help"));

    SetMenuBar(menuBar);

    m_effectMenu->Check(k_menu_effect_tempo_line, true);
    m_viewMenu->Check(k_menu_view_single_system, true);
}

//---------------------------------------------------------------------------------------
void MyFrame::on_quit(wxCommandEvent& WXUNUSED(event))
{
    Close(true /*force to close*/);
}

//---------------------------------------------------------------------------------------
void MyFrame::on_about(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Lomse: Test single system view, for wxWidgets"),
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
    // Let's define the required information:

        //the pixel format
        int pixel_format = k_pix_format_rgb24;  //RGB 24bits

        //the desired resolution. For Linux and Windows 96 pixels per inch works ok.
        int resolution = 96;    //96 ppi

        //Normal y axis direction is 0 coordinate at top and increase downwards. You
        //must specify if you would like just the opposite behavior. For Windows and
        //Linux the default behavior is the right behavior.
        bool reverse_y_axis = false;

    //Now, initialize the library with these values
    m_lomse.init_library(pixel_format,resolution, reverse_y_axis);

    //set required callbacks
    m_lomse.set_notify_callback(this, wrapper_lomse_event);
}

//---------------------------------------------------------------------------------------
void MyFrame::open_test_document()
{
    get_active_canvas()->open_test_document( select_view_type() );

    //BUG_BYPASS
    // In Linux there are problems to catch Key Up/Down events. See for instance
    // http://forums.wxwidgets.org/viewtopic.php?t=33057&p=137567
    // Following line is not needed for Windows (doen't hurt) but it is
    // necessary for Linux, in order to receive Key Up/Down events
    get_active_canvas()->SetFocus();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_open_test(wxCommandEvent& WXUNUSED(event))
{
    open_test_document();
}

//---------------------------------------------------------------------------------------
int MyFrame::select_view_type()
{
    int viewType = k_view_single_system;
    if (m_viewMenu->IsChecked(k_menu_view_vertical_book))
        viewType = k_view_vertical_book;
    else if (m_viewMenu->IsChecked(k_menu_view_horizontal_book))
        viewType = k_view_horizontal_book;

    return viewType;
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

        case k_update_viewport_event:
        {
            if (pCanvas)
            {
                SpEventUpdateViewport pEv(
                    static_pointer_cast<EventUpdateViewport>(pEvent) );
                MyUpdateViewportEvent event(pEv);
                ::wxPostEvent(pCanvas, event);
            }
            break;
        }

        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void MyFrame::on_open_file(wxCommandEvent& WXUNUSED(event))
{
    wxString defaultPath = wxT("../../../test-scores/");

    //prepare a filter for the supported files
    wxString sFilter = "All supported files";
    sFilter += "|*.lms;*.lmb;*.lmd;*.xml;*.musicxml;*.mnx|";
    sFilter += _("LenMus files");
    sFilter += "|*.lms;*.lmb;*.lmd|";
    sFilter += _("MusicXML files");
    sFilter += "|*.xml;*.musicxml|";
    sFilter += _("MNX files");
    sFilter += "|*.mnx";

    //ask for the file to open/import
    wxString filename = wxFileSelector(_("Choose the file to open"),
                                       defaultPath,     //default path
                                       "",              //default filename
                                       "",              //default_extension
                                       sFilter);

    if (filename.empty())
        return;

    get_active_canvas()->open_file(filename, select_view_type());
}

//---------------------------------------------------------------------------------------
void MyFrame::on_zoom_in(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->zoom_in();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_zoom_out(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->zoom_out();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_play_start(wxCommandEvent& WXUNUSED(event))
{
    int effects = k_tracking_none;
    if (m_effectMenu->IsChecked(k_menu_effect_highlight))
        effects |= k_tracking_highlight_notes;
    if (m_effectMenu->IsChecked(k_menu_effect_tempo_line))
        effects |= k_tracking_tempo_line;

    get_active_canvas()->play_start(effects);
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

//---------------------------------------------------------------------------------------
void MyFrame::on_debug_dump_gmodel(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->on_debug_dump_gmodel();
}



//=======================================================================================
// MyCanvas implementation
//=======================================================================================

BEGIN_EVENT_TABLE(MyCanvas, wxWindow)
	EVT_KEY_DOWN(MyCanvas::on_key_down)
    EVT_MOUSE_EVENTS(MyCanvas::on_mouse_event)
    EVT_SIZE(MyCanvas::on_size)
    EVT_PAINT(MyCanvas::on_paint)
    MY_EVT_VISUAL_TRACKING(MyCanvas::on_visual_tracking)
    MY_EVT_UPDATE_VIEWPORT(MyCanvas::on_update_viewport)
END_EVENT_TABLE()


//---------------------------------------------------------------------------------------
MyCanvas::MyCanvas(wxFrame *frame, LomseDoorway& lomse, ScorePlayer* pPlayer)
    : wxWindow(frame, wxID_ANY)
    , PlayerNoGui(60L /*tempo 60 MM*/, false /*no count off*/, false /*no metronome clicks*/)
    , m_lomse(lomse)
	, m_pPresenter(NULL)
	, m_buffer(NULL)
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
void MyCanvas::open_file(const wxString& fullname, int viewType)
{
    //create a new View
    std::string filename( fullname.mb_str(wxConvUTF8) );
    delete m_pPresenter;
    m_pPresenter = m_lomse.open_document(viewType, filename);

    //get the pointer to the interactor, set the rendering buffer and register for
    //receiving desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        //connect the View with the window buffer
        spInteractor->set_rendering_buffer(&m_rbuf_window);

        //ask to receive desired events
        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);

//        //TEST: set infinite page width
//        Document* pDoc = m_pPresenter->get_document_raw_ptr();
//        ImoDocument* pImoDoc = pDoc->get_im_root();
//        ImoPageInfo* pPageInfo = pImoDoc->get_page_info();
//        pPageInfo->set_page_width(2000000.0f);
//        //TEST: and do not justify systems
//        ImoScore* pScore = dynamic_cast<ImoScore*>(pDoc->get_content_item(0));
//        pScore->set_long_option("Score.JustifyLastSystem", 0L);
    }

    //render the new score
    m_view_needs_redraw = true;
    Refresh(false /* don't erase background */);
}

//---------------------------------------------------------------------------------------
void MyCanvas::on_size(wxSizeEvent& WXUNUSED(event))
{
    wxSize size = this->GetClientSize();
    create_rendering_buffer(size.GetWidth(), size.GetHeight());

    Refresh(false /* don't erase background */);
}

//---------------------------------------------------------------------------------------
void MyCanvas::on_paint(wxPaintEvent& event)
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
void MyCanvas::open_test_document(int viewType)
{
    //Normally you will load the content of a file. But in this
    //simple example we will create an empty document and define its content
    //from a text string

    //first, we will create a 'presenter'. It takes care of creating and maintaining
    //all objects and relationships between the document, its views and the interactors
    //to interct with the view
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(viewType,
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
    }

    m_view_needs_redraw = true;
    Refresh(false);
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
void MyCanvas::on_key_down(wxKeyEvent& event)
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
void MyCanvas::on_key(int x, int y, unsigned key, unsigned WXUNUSED(flags))
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
void MyCanvas::on_mouse_event(wxMouseEvent& event)
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
void MyCanvas::play_start(int effects)
{
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        Document* pDoc = m_pPresenter->get_document_raw_ptr();

        //AWARE: Then next line of code is just an example, in which it is assumed that
        //the score to play is the first element in the document.
        //In a real application, as the document could contain texts, images and many
        //scores, you shoud get the pointer to the score to play in a suitable way.
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );

        if (pScore)
        {
            spInteractor->set_visual_tracking_mode(effects);
            m_pPlayer->load_score(pScore, this);
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

//---------------------------------------------------------------------------------------
void MyCanvas::on_update_viewport(MyUpdateViewportEvent& event)
{
    SpEventUpdateViewport pEv = event.get_lomse_event();
    WpInteractor wpInteractor = pEv->get_interactor();
    if (SpInteractor sp = wpInteractor.lock())
    {
        int xPos = pEv->get_new_viewport_x();
        int yPos = pEv->get_new_viewport_y();

        //change viewport
        sp->new_viewport(xPos, yPos);
    }
    event.Skip(false);      //do not propagate event
}

//---------------------------------------------------------------------------------------
void MyCanvas::on_debug_dump_gmodel()
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        GraphicModel* pGM = spInteractor->get_graphic_model();
        stringstream out;
        int pages = pGM->get_num_pages();
        for (int i=0; i < pages; ++i)
        {
            out << "Page " << i
                << " ==================================================================="
                << endl;
            pGM->dump_page(i, out);
        }
        DlgDebug dlg(this, "graphical model dump", to_wx_string(out.str()) );
        dlg.ShowModal();
    }
}


//=======================================================================================
// MidiServer implementation
//=======================================================================================
MidiServer::MidiServer()
    : m_pMidiSystem( wxMidiSystem::GetInstance() )
    , m_pMidiOut(NULL)
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
            m_pMidiOut = NULL;
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
                nErr = m_pMidiOut->Open(0, NULL);        // 0 latency, no driver user info
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

// IDs for controls
enum
{
	lmID_HTML_WND = 3010,
	lmID_ACCEPT,
	lmID_SAVE,
};


//=======================================================================================
// DlgDebug implementation
//=======================================================================================


wxBEGIN_EVENT_TABLE(DlgDebug, wxDialog)
   EVT_BUTTON(wxID_OK, DlgDebug::OnOK)
   EVT_BUTTON(lmID_SAVE, DlgDebug::OnSave)
wxEND_EVENT_TABLE()

IMPLEMENT_CLASS(DlgDebug, wxDialog)

//---------------------------------------------------------------------------------------
DlgDebug::DlgDebug(wxWindow * parent, wxString sTitle, wxString sData, bool fSave)
    : wxDialog(parent, -1, sTitle, wxDefaultPosition, wxSize(800, 430),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
    , m_fSave(fSave)
{
    Centre();

    wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);

    // use wxTE_RICH2 style to avoid 64kB limit under MSW and display big files
    // faster than with wxTE_RICH
    m_pTxtData = new wxTextCtrl(this, wxID_ANY, sData,
                                wxPoint(0, 0), wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_NOHIDESEL
                                | wxTE_RICH2);

    // use fixed-width font
    m_pTxtData->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE,
                               wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    pMainSizer->Add(m_pTxtData,
                    1,            //vertically stretchable
                    wxEXPAND |    //horizontally stretchable
                    wxALL,        //some space border all around
                    5 );          //set border width to 5 px

    wxBoxSizer* pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton *cmdOK = new wxButton(this, wxID_OK, _("OK"));
    pButtonsSizer->Add(cmdOK, 0, 0, 1);
    cmdOK->SetDefault();
    cmdOK->SetFocus();

    if (m_fSave)
    {
	    wxButton *cmdSave = new wxButton(this, lmID_SAVE, _("Save"));

	    pButtonsSizer->Add(cmdSave, 0, 0, 1);
    }

    pMainSizer->Add(pButtonsSizer, 0, wxALIGN_CENTER | wxALL, 5);

    // set autolayout based on sizers
    SetAutoLayout(true);
    SetSizer(pMainSizer);
}

//---------------------------------------------------------------------------------------
DlgDebug::~DlgDebug()
{
}

//---------------------------------------------------------------------------------------
void DlgDebug::OnOK(wxCommandEvent& WXUNUSED(event))
{
   EndModal(wxID_OK);
}

//---------------------------------------------------------------------------------------
void DlgDebug::AppendText(wxString sText)
{
    m_pTxtData->AppendText(sText);
}

//---------------------------------------------------------------------------------------
void DlgDebug::OnSave(wxCommandEvent& WXUNUSED(event))
{
    wxString sFilename = wxFileSelector("File to save", "", "debug", "txt",
                                        "*.*",  wxFD_SAVE);
	if ( !sFilename.empty() )
	{
		// save the file
		m_pTxtData->SaveFile(sFilename);
	}
	//else: cancelled by user

}

