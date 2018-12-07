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
    #include <wx/button.h>
    #include <wx/checkbox.h>
    #include <wx/statbox.h>
    #include <wx/radiobox.h>
    #include <wx/combobox.h>
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
#include <lomse_command.h>
#include <lomse_interval.h>

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
    void on_quit(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);

protected:
    //accessors
    MyCanvas* get_active_canvas() const { return m_canvas; }

    //event handlers
    void on_open_file(wxCommandEvent& WXUNUSED(event));
    void on_zoom_in(wxCommandEvent& WXUNUSED(event));
    void on_zoom_out(wxCommandEvent& WXUNUSED(event));
    void on_transpose_notes(wxCommandEvent& WXUNUSED(event));
    void on_transpose_key(wxCommandEvent& WXUNUSED(event));
    void on_edit_undo(wxCommandEvent& WXUNUSED(event));
    void on_edit_redo(wxCommandEvent& WXUNUSED(event));

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
    void transpose_notes();
    void transpose_key();
    void undo();
    void redo();

protected:
    //event handlers
    void on_paint(wxPaintEvent& WXUNUSED(event));
    void on_size(wxSizeEvent& event);
    void on_key_down(wxKeyEvent& event);
    void on_mouse_event(wxMouseEvent& event);

    void delete_rendering_buffer();
    void create_rendering_buffer(int width, int height);
    void copy_buffer_on_dc(wxDC& dc);
    void update_rendering_buffer_if_needed();
    void force_redraw();
    void update_window();
    void on_key(int x, int y, unsigned key, unsigned flags);
    unsigned get_keyboard_flags(wxKeyEvent& event);
    unsigned get_mouse_flags(wxMouseEvent& event);

    //edition related
    void transpose_notes_chromatically(FIntval interval, bool fUp);
    void transpose_notes_diatonically(int steps, bool fUp);
    void transpose_key(int iInterval, bool fUp, bool fDiatonically);


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

//---------------------------------------------------------------------------------------
//A dialog for displaying transposition options for transposing notes
class DlgTransposeNotes : public wxDialog
{
protected:
    FIntval* m_interval;
    int* m_steps;
    bool* m_fUp;
    bool* m_fDiatonically;

    wxRadioBox* m_radMethod;
    wxRadioBox* m_radUpDown;
    wxComboBox* m_cboInterval;
    wxButton* m_btnOk;
    wxButton* m_btnCancel;


public:
    DlgTransposeNotes(wxWindow* parent, FIntval* interval, int* steps, bool* fUp,
                      bool *fDiatonically);
    virtual ~DlgTransposeNotes();

protected:

    //event handlers
    virtual void on_method_changed(wxCommandEvent& event);
    virtual void on_ok(wxMouseEvent& event);
    virtual void on_cancel(wxMouseEvent& event);

    void create_controls();
    void load_options();
    void load_combo_values();
    void load_intervals();
    void load_steps();
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
    k_menu_edit_transpose_notes,
    k_menu_edit_transpose_key,
    k_menu_edit_undo,
    k_menu_edit_redo,

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
    EVT_MENU(k_menu_file_quit, MyFrame::on_quit)
    EVT_MENU(k_menu_help_about, MyFrame::on_about)
    EVT_MENU(k_menu_file_open, MyFrame::on_open_file)
    EVT_MENU(k_menu_zoom_in, MyFrame::on_zoom_in)
    EVT_MENU(k_menu_zoom_out, MyFrame::on_zoom_out)
    EVT_MENU(k_menu_edit_transpose_notes, MyFrame::on_transpose_notes)
    EVT_MENU(k_menu_edit_transpose_key, MyFrame::on_transpose_key)
    EVT_MENU      (k_menu_edit_undo, MyFrame::on_edit_undo)
//    EVT_UPDATE_UI (k_menu_edit_undo, MyFrame::on_update_UI_edit)
    EVT_MENU      (k_menu_edit_redo, MyFrame::on_edit_redo)
//    EVT_UPDATE_UI (k_menu_edit_redo, MyFrame::on_update_UI_edit)
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

    wxMenu* editMenu = new wxMenu;
    editMenu->Append(k_menu_edit_transpose_notes, _T("Transpose &notes"));
    editMenu->Append(k_menu_edit_transpose_key, _T("Transpose &key"));
    editMenu->AppendSeparator();
    editMenu->Append(k_menu_edit_undo, _T("&Undo"));
    editMenu->Append(k_menu_edit_redo, _T("&Redo"));

    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(k_menu_help_about, _T("&About"));

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(zoomMenu, _T("&Zoom"));
    menuBar->Append(editMenu, _T("&Edit"));
    menuBar->Append(helpMenu, _T("&Help"));

    SetMenuBar(menuBar);
}

//---------------------------------------------------------------------------------------
void MyFrame::on_quit(wxCommandEvent& WXUNUSED(event))
{
    Close(true /*force to close*/);
}

//---------------------------------------------------------------------------------------
void MyFrame::on_about(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Lomse: wxWidgets sample to test transposition"),
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
void MyFrame::on_open_file(wxCommandEvent& WXUNUSED(event))
{
    wxString defaultPath = wxT("../../../test-scores/");

    wxString filename = wxFileSelector(_("Open score"), defaultPath,
        wxEmptyString, wxEmptyString, wxT("LenMus files|*.lms;*.lmd"));

    if (filename.empty())
        return;

    get_active_canvas()->open_file(filename);
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
void MyFrame::on_edit_undo(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->undo();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_edit_redo(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->redo();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_transpose_notes(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->transpose_notes();
}

//---------------------------------------------------------------------------------------
void MyFrame::on_transpose_key(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->transpose_key();
}


//=======================================================================================
// MyCanvas implementation
//=======================================================================================

BEGIN_EVENT_TABLE(MyCanvas, wxWindow)
	EVT_KEY_DOWN(MyCanvas::on_key_down)
    EVT_MOUSE_EVENTS(MyCanvas::on_mouse_event)
    EVT_SIZE(MyCanvas::on_size)
    EVT_PAINT(MyCanvas::on_paint)
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
    m_pPresenter = m_lomse.open_document(k_view_vertical_book,
                                         filename);

    //get the pointer to the interactor, set the rendering buffer, register for
    //receiving desired events and enable edition
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        //connect the View with the window buffer
        spIntor->set_rendering_buffer(&m_rbuf_window);

        //ask to receive desired events
        spIntor->add_event_handler(k_update_window_event, this, wrapper_update_window);

        //set in edition mode
        spIntor->set_operating_mode(Interactor::k_mode_edition);
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
        "(score (vers 2.0) "
        "(instrument (musicData "
        "(clef G)(key G)(time 4 4)"
        "(n g4 q)(n a4 q)(n b4 q)(n c5 q)"
        "(barline)"
        "(n d5 q)(n e5 q)(n f5 q)(n g5 q)"
        "(barline)"
        ")))",
        Document::k_format_ldp);

    //get the pointer to the interactor, set the rendering buffer, register for
    //receiving desired events and enable edition
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        //connect the View with the window buffer
        spIntor->set_rendering_buffer(&m_rbuf_window);

        //ask to receive desired events
        spIntor->add_event_handler(k_update_window_event, this, wrapper_update_window);

        //set in edition mode
        spIntor->set_operating_mode(Interactor::k_mode_edition);
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
void MyCanvas::wrapper_update_window(void* pThis, SpEventInfo UNUSED(pEvent))
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

    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
        spIntor->redraw_bitmap();
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
void MyCanvas::on_key(int x, int y, unsigned key, unsigned UNUSED(flags))
{
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        switch (key)
        {
            case 'D':
                spIntor->switch_task(TaskFactory::k_task_drag_view);
                break;
            case 'S':
                spIntor->switch_task(TaskFactory::k_task_selection);
                break;
            case '+':
                spIntor->zoom_in(x, y);
                force_redraw();
                break;
            case '-':
                spIntor->zoom_out(x, y);
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
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        wxSize size = this->GetClientSize();
        spIntor->zoom_in(size.GetWidth()/2, size.GetHeight()/2);
        force_redraw();
    }
}

//-------------------------------------------------------------------------
void MyCanvas::zoom_out()
{
    if (!m_pPresenter) return;

    //do zoom out centered on window center
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        wxSize size = this->GetClientSize();
        spIntor->zoom_out(size.GetWidth()/2, size.GetHeight()/2);
        force_redraw();
    }
}

//-------------------------------------------------------------------------
void MyCanvas::transpose_notes()
{
    if (!m_pPresenter) return;

    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        FIntval interval("M2");
        int steps = 0;
        bool fUp = true;
        bool fDiatonically = false;
        DlgTransposeNotes dlg(this, &interval, &steps, &fUp, &fDiatonically);
        if (dlg.ShowModal() == wxID_OK)
        {
            if (fDiatonically)
                transpose_notes_diatonically(steps, fUp);
            else
                transpose_notes_chromatically(interval, fUp);
        }
    }
}

//-------------------------------------------------------------------------
void MyCanvas::transpose_key()
{
    if (!m_pPresenter) return;

    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
    //TODO
//        FIntval interval("M2");
//        bool fUp = true;
//        bool fDiatonically = false;
//        DlgTransposeNotes dlg(this, &interval, &fUp, &fDiatonically);
//        if (dlg.ShowModal() == wxID_OK)
//        {
//            transpose(interval, fUp, fDiatonically);
//        }
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::undo()
{
    if (!m_pPresenter) return;

    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        ::wxBeginBusyCursor();
        spIntor->exec_undo();
        ::wxEndBusyCursor();
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::redo()
{
    if (!m_pPresenter) return;

    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        ::wxBeginBusyCursor();
        spIntor->exec_redo();
        ::wxEndBusyCursor();
    }
}

//-------------------------------------------------------------------------
void MyCanvas::transpose_notes_chromatically(FIntval interval, bool fUp)
{
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        ::wxBeginBusyCursor();
        string name = gettext("Chromatic transposition");
        spIntor->exec_command(
            new CmdTransposeByInterval(interval, fUp) );
        ::wxEndBusyCursor();
    }
}

//-------------------------------------------------------------------------
void MyCanvas::transpose_notes_diatonically(int steps, bool fUp)
{
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        ::wxBeginBusyCursor();
        string name = gettext("Diatonic transposition");
        spIntor->exec_command(
            new CmdTransposeDiatonically(steps, fUp) );
        ::wxEndBusyCursor();
    }
}

//-------------------------------------------------------------------------
void MyCanvas::transpose_key(int iInterval, bool fUp, bool fDiatonically)
{
    //TODO
    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        ::wxBeginBusyCursor();
        string name = gettext("Chromatic transposition");
//        spIntor->exec_command(
//            new CmdTransposeByInterval(FIntval(iInterval), fUp) );
        ::wxEndBusyCursor();
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

    if (SpInteractor spIntor = m_pPresenter->get_interactor(0).lock())
    {
        wxEventType nEventType = event.GetEventType();
        wxPoint pos = event.GetPosition();
        unsigned flags = get_mouse_flags(event);

        if (nEventType==wxEVT_LEFT_DOWN)
        {
            flags |= k_mouse_left;
            spIntor->on_mouse_button_down(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_LEFT_UP)
        {
            flags |= k_mouse_left;
            spIntor->on_mouse_button_up(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_RIGHT_DOWN)
        {
            flags |= k_mouse_right;
            spIntor->on_mouse_button_down(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_RIGHT_UP)
        {
            flags |= k_mouse_right;
            spIntor->on_mouse_button_up(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_MOTION)
            spIntor->on_mouse_move(pos.x, pos.y, flags);
    }
}


//=======================================================================================
// DlgTransposeNotes implementation
//=======================================================================================
DlgTransposeNotes::DlgTransposeNotes(wxWindow* parent, FIntval* interval, int* steps,
                                     bool* fUp, bool* fDiatonically)
    : wxDialog(parent, wxID_ANY, _("Transpose notes"), wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE)
    , m_interval(interval)
    , m_steps(steps)
    , m_fUp(fUp)
    , m_fDiatonically(fDiatonically)
{
    create_controls();
    load_options();
}

//---------------------------------------------------------------------------------------
DlgTransposeNotes::~DlgTransposeNotes()
{
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::create_controls()
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* szrMain;
	szrMain = new wxBoxSizer( wxVERTICAL );

	wxString m_radMethodChoices[] = { _("Chromatically"), _("Diatonically") };
	int m_radMethodNChoices = sizeof( m_radMethodChoices ) / sizeof( wxString );
	m_radMethod = new wxRadioBox( this, wxID_ANY, _("Tranposition method "), wxDefaultPosition, wxDefaultSize, m_radMethodNChoices, m_radMethodChoices, 1, wxRA_SPECIFY_COLS );
	m_radMethod->SetSelection( 1 );
	szrMain->Add( m_radMethod, 1, wxALL|wxEXPAND, 5 );

	wxString m_radUpDownChoices[] = { _("Up"), _("Down") };
	int m_radUpDownNChoices = sizeof( m_radUpDownChoices ) / sizeof( wxString );
	m_radUpDown = new wxRadioBox( this, wxID_ANY, _("Direction "), wxDefaultPosition, wxDefaultSize, m_radUpDownNChoices, m_radUpDownChoices, 1, wxRA_SPECIFY_COLS );
	m_radUpDown->SetSelection( 0 );
	szrMain->Add( m_radUpDown, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );

	wxStaticBoxSizer* szrInterval;
	szrInterval = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Interval ") ), wxVERTICAL );

	m_cboInterval = new wxComboBox( szrInterval->GetStaticBox(), wxID_ANY, _("Unison"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	szrInterval->Add( m_cboInterval, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );


	szrMain->Add( szrInterval, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer* szrButtons;
	szrButtons = new wxBoxSizer( wxHORIZONTAL );


	szrButtons->Add( 0, 0, 1, wxEXPAND, 5 );

	m_btnOk = new wxButton( this, wxID_ANY, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	szrButtons->Add( m_btnOk, 0, wxALL, 5 );

	m_btnCancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	szrButtons->Add( m_btnCancel, 0, wxALL, 5 );


	szrButtons->Add( 0, 0, 1, wxEXPAND, 5 );


	szrMain->Add( szrButtons, 0, wxTOP|wxBOTTOM|wxEXPAND, 5 );


	this->SetSizer( szrMain );
	this->Layout();
	szrMain->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	m_radMethod->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DlgTransposeNotes::on_method_changed ), NULL, this );
	m_btnOk->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DlgTransposeNotes::on_ok ), NULL, this );
	m_btnCancel->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DlgTransposeNotes::on_cancel ), NULL, this );
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::load_options()
{
    m_radMethod->Select(*m_fDiatonically ? 1 : 0);
    m_radUpDown->Select(*m_fUp ? 0 : 1);
    load_combo_values();
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::on_method_changed(wxCommandEvent& event)
{
    load_combo_values();
    event.Skip();
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::load_combo_values()
{
    if (m_radMethod->GetSelection() == 1)
        load_steps();
    else
        load_intervals();
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::load_intervals()
{
    //Combo box for intervals
    m_cboInterval->Clear();
    m_cboInterval->Append(_("Perfect unison"));
    m_cboInterval->Append(_("Augmented unison"));
    m_cboInterval->Append(_("Diminished second"));
    m_cboInterval->Append(_("Minor second"));
    m_cboInterval->Append(_("Major second"));
    m_cboInterval->Append(_("Augmented second"));
    m_cboInterval->Append(_("Diminished third"));
    m_cboInterval->Append(_("Minor third"));
    m_cboInterval->Append(_("Major third"));
    m_cboInterval->Append(_("Augmented third"));
    m_cboInterval->Append(_("Diminished fourth"));
    m_cboInterval->Append(_("Perfect fourth"));
    m_cboInterval->Append(_("Augmented fourth"));
    m_cboInterval->Append(_("Diminished fifth"));
    m_cboInterval->Append(_("Perfect fifth"));
    m_cboInterval->Append(_("Augmented fifth"));
    m_cboInterval->Append(_("Diminished sixth"));
    m_cboInterval->Append(_("Minor sixth"));
    m_cboInterval->Append(_("Major sixth"));
    m_cboInterval->Append(_("Augmented sixth"));
    m_cboInterval->Append(_("Diminished seventh"));
    m_cboInterval->Append(_("Minor seventh"));
    m_cboInterval->Append(_("Major seventh"));
    m_cboInterval->Append(_("Augmented seventh"));
    m_cboInterval->Append(_("Diminished octave"));
    m_cboInterval->Append(_("Perfect octave"));

    m_cboInterval->SetSelection(0);
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::load_steps()
{
    //Combo box for intervals
    m_cboInterval->Clear();
    m_cboInterval->Append(_("Second (1 step)"));
    m_cboInterval->Append(_("Third (2 steps)"));
    m_cboInterval->Append(_("Fourth (3 steps)"));
    m_cboInterval->Append(_("Fifth (4 steps)"));
    m_cboInterval->Append(_("Sixth (5 steps)"));
    m_cboInterval->Append(_("Seventh (6 steps)"));
    m_cboInterval->Append(_("Octave (7 steps)"));

    m_cboInterval->SetSelection(0);
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::on_ok(wxMouseEvent& event)
{
    static string intervals[] = {
        "p1",  "a1",  "d2", "m2",  "M2", "a2", "d3",  "m3",  "M3", "a3",
        "d4",  "p4",  "a4", "d5",  "p5", "a5",  "d6",  "m6", "M6",  "a6",
        "d7",  "m7",  "M7",  "a7", "d8", "p8"
    };

    *m_fUp = m_radUpDown->GetSelection() == 0;
    *m_fDiatonically = m_radMethod->GetSelection() == 1;
    if (*m_fDiatonically)
        *m_steps = m_cboInterval->GetSelection() + 1;
    else
        *m_interval = FIntval( intervals[m_cboInterval->GetSelection()] );

    event.Skip();
    EndModal(wxID_OK);
}

//---------------------------------------------------------------------------------------
void DlgTransposeNotes::on_cancel(wxMouseEvent& event)
{
    event.Skip();
    EndModal(wxID_CANCEL);
}
