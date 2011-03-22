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

// header files required by Microsoft stuff
#include "stdafx.h"
#include "resource.h"
#include "Commdlg.h"

//lomse headers
#include "lomse_doorway.h"
#include "lomse_document.h"
#include "lomse_graphic_view.h"
#include "lomse_interactor.h"
#include "lomse_presenter.h"
#include "platform/win32/agg_win32_bmp.h"

using namespace lomse;


#define MAX_LOADSTRING 100

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
//Here you define the rendering buffer and its associated bitmap
pixel_map           m_pmap_window;
RenderingBuffer     m_rbuf_window;


//Lomse can manage a lot of bitmap formats and pixel formats. You must
//define the format that you are going to use 
unsigned         m_bpp;         //bits per pixel

//some additinal variables
bool    m_view_needs_redraw;      //to control when the View must be re-drawed

//to measure ellapsed time (for performance measurements)
LARGE_INTEGER m_sw_freq;
LARGE_INTEGER m_sw_start;

//for keyboard support
unsigned      m_last_key;    //last pressed key


//for mouse click & move position
int           m_xMouse;
int           m_yMouse;
unsigned      m_input_flags;


// All typical MS Windows stuff needed to run the program and the main events handler loop
HINSTANCE m_hInst;								//current instance
HWND m_hWnd;                                    //the main window
TCHAR szTitle[MAX_LOADSTRING];					//main window title
TCHAR szWindowClass[MAX_LOADSTRING];			//main window class name

//forward declarations
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


//---------------------------------------------------------------------------------------
void create_pmap(unsigned width, unsigned height)
{
    //creates a bitmap of specified size and associates it to the rendering
    //buffer for the view. Any existing buffer is automatically deleted

    m_pmap_window.create(width, height, org_e(m_bpp));
    m_rbuf_window.attach(m_pmap_window.buf(), 
                         m_pmap_window.width(),
                         m_pmap_window.height(),
                         -m_pmap_window.stride()
                        );
}

//---------------------------------------------------------------------------------------
void create_rendering_buffer_bitmap(HWND hWnd)
{
    RECT rct;
    ::GetClientRect(hWnd, &rct);
    int width = rct.right - rct.left;
    int height = rct.bottom - rct.top;

    create_pmap(width, height);
    m_view_needs_redraw = true;
}

//---------------------------------------------------------------------------------------
void display_view_content(HDC dc)
{
    //renders the view buffer into the main window

    m_pmap_window.draw(dc);
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
    ::InvalidateRect(m_hWnd, 0, FALSE);     //generate WM_PAINT event
}

//---------------------------------------------------------------------------------------
void update_window(void* pThis)
{
    // Callback method for Lomse. It can be used also by your application.
    // Invoking update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without calling
    // any View methods (i.e. on_paint)

    HDC dc = ::GetDC(m_hWnd);
    display_view_content(dc);       //render view buffer into the main window
    ::ReleaseDC(m_hWnd, dc);
}

//---------------------------------------------------------------------------------------
void start_timer(void* pThis)
{
    ::QueryPerformanceCounter(&(m_sw_start));
}

//---------------------------------------------------------------------------------------
double elapsed_time(void* pThis)
{
    LARGE_INTEGER stop;
    ::QueryPerformanceCounter(&stop);
    return double(stop.QuadPart - 
                    m_sw_start.QuadPart) * 1000.0 / 
                    double(m_sw_freq.QuadPart);
}

//---------------------------------------------------------------------------------------
string get_font_filename(const string& fontname, bool bold, bool italic)
{
    //This is just a trivial example. In real applications you should 
    //use operating system services to find a suitable font
    
    //notes on parameters received:
    // - fontname can be either the face name (i.e. "Book Antiqua") or
    //   the familly name (i.e. "sans-serif")


//#if WINDOWS
    string path = "C:\\WINNT\\Fonts\\";
//#else if LINUX
//    string path = "\usr\shared\fonts\";
//#endif

    //if family name, choose a font name
    string name = fontname;
    if (name == "serif")
        name = "Times New Roman";
    else if (name == "sans-serif")
        name = "Tahoma";
    else if (name == "handwritten")
        name = "Lucida Handwriting";
    else if (name == "cursive")
        name = "Monotype Corsiva";
    else if (name == "monospaced")
        name = "Courier New";

    //choose a suitable font file
    string fontfile;
    if (name == "Times New Roman")
    {
        if (italic && bold)
            fontfile = "timesbi.ttf";
        else if (italic)
            fontfile = "timesi.ttf";
        else if (bold)
            fontfile = "timesbd.ttf";
        else
            fontfile = "times.ttf";
    }

    else if (name == "Tahoma")
    {
        if (bold)
            fontfile = "tahomabd.ttf";
        else
            fontfile = "tahoma.ttf";
    }

    else if (name == "Lucida Handwriting")
    {
        fontfile = "lhandw.ttf";
    }

    else if (name == "Monotype Corsiva")
    {
        fontfile = "mtcorsva.ttf";
    }

    else if (name == "Courier New")
    {
        if (italic && bold)
            fontfile = "courbi.ttf";
        else if (italic)
            fontfile = "couri.ttf";
        else if (bold)
            fontfile = "courbd.ttf";
        else
            fontfile = "cour.ttf";
    }

    else
        fontfile = "times.ttf";

    
   return path + fontfile;
}

//---------------------------------------------------------------------------------------
void on_open_file()
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[MAX_PATH];  // buffer for file name

    //initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;

    //set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	//use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';

	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "LenMus scores (*.lms)\0*.lms\0All Files (*.*)\0*.*\0";
    ofn.lpstrDefExt = "lms";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	//display the Open dialog box. 
    if (!GetOpenFileName( &ofn ))
    {
        MessageBoxW(
            NULL, 
            (LPCWSTR)L"Error: file could not be open",
            (LPCWSTR)L"File open error",
            MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
        );
        return;
    }

    //create a new View
    std::string filename(ofn.lpstrFile);
    delete m_pPresenter;
    m_pPresenter = m_lomse.open_document(ViewFactory::k_view_horizontal_book,
                                         filename);

    //now, get the pointers to the relevant components
    m_pDoc = m_pPresenter->get_document();
    m_pInteractor = m_pPresenter->get_interactor(0);

    //connect the View with the window buffer and set required callbacks
    m_pInteractor->set_rendering_buffer(&m_rbuf_window);
    m_pInteractor->set_force_redraw_callbak(NULL, force_redraw);
    m_pInteractor->set_update_window_callbak(NULL, update_window);
    m_pInteractor->set_start_timer_callbak(NULL, start_timer);
    m_pInteractor->set_elapsed_time_callbak(NULL, elapsed_time);

    force_redraw(NULL);
}

//---------------------------------------------------------------------------------------
void open_document()
{
    //Normally you will load the content of a file. But in this
    //simple example we wiil crete an empty document and define its content
    //from a text string

    //first, we will create a 'presenter'. It takes care of cretaing and maintaining
    //all objects and relationships between the document, its views and the interactors
    //to interct with the view
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(ViewFactory::k_view_horizontal_book);

    //now, get the pointers to the relevant components
    m_pDoc = m_pPresenter->get_document();
    m_pInteractor = m_pPresenter->get_interactor(0);

    //connect the View with the window buffer and set required callbacks
    m_pInteractor->set_rendering_buffer(&m_rbuf_window);
    m_pInteractor->set_force_redraw_callbak(NULL, force_redraw);
    m_pInteractor->set_update_window_callbak(NULL, update_window);
    m_pInteractor->set_start_timer_callbak(NULL, start_timer);
    m_pInteractor->set_elapsed_time_callbak(NULL, elapsed_time);

    //Now let's place content on the created document
    m_pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "

        ////instrument name
        //"(instrument (name \"Violin\")(musicData (clef G)(clef F4)(clef C1)) )))" );

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

        //beams
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
        "(n e3 q)(n e4 q)(barline)"
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
//        "(chord (n c4 q p1)(n e4 q p1)(n g4 q p1))"
//        "(chord (n c4 q p1)(n d4 q p1)(n g4 q p1))"
        "))"
        ")))" );

        ////anchor / chord
        //"(instrument (staves 2)(musicData "
        //"(clef G p1)(clef F4 p2)(key F)(time 2 4)"
        //"(chord (n c4 h p1)(n d4 h p1)(n g4 h p1))"
        //"(goBack start)(n c3 h p2)(barline)"
        //"(chord (n c4 q p1)(n d4 q p1)(n g4 q p1))"
        //"(chord (n a4 e p1)(n b4 e p1)(n d5 e p1))"
        //"(chord (n a4 e p1)(n b4 e p1)(n c5 e p1))"
        //"(goBack start)(n c3 q p2)(n c3 e p2)(n c3 e p2)(barline)"
        //"(chord (n f4 e p1)(n g4 e p1)(n b4 e p1))"
        //"(chord (n f4 e p1)(n g4 e p1)(n a4 e p1))"
        //"(goBack start)(n c3 e p2)(n c3 e p2)(barline)"

        //"(chord (n c4 h p1)(n -d4 h p1)(n +g4 h p1))"
        //"(goBack start)(n c3 h p2)(barline)"
        //"(chord (n +c4 q p1)(n +d4 q p1)(n g4 q p1))"
        //"(chord (n +a4 e p1)(n b4 e p1)(n d5 e p1))"
        //"(chord (n a4 e p1)(n b4 e p1)(n +c5 e p1))"
        //"(goBack start)(n c3 q p2)(n c3 e p2)(n c3 e p2)(barline)"
        //"(chord (n f4 e p1)(n g4 e p1)(n b4 e p1))"
        //"(chord (n f4 e p1)(n g4 e p1)(n a4 e p1))"
        //"(goBack start)(n c3 e p2)(n c3 e p2)"
        //"))"
        //")))" );

        ////chord with accidentals
        //"(instrument (staves 2)(musicData "
        //"(clef G p1)(clef 8_F4 p2)(key F)(time 2 4)"
        //    //no displaced notes, no accidentals
        //"(chord (n c4 q p1)(n e4 q p1)(n g4 q p1))"
        //"(chord (n g5 q p1)(n e5 q p1)(n g4 q p1))"
        //"(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
        //    //no displaced notes, accidentals
        //"(chord (n c4 q p1)(n -e4 q p1)(n +g4 q p1))"
        //"(chord (n +g5 q p1)(n -e5 q p1)(n g4 q p1))"
        //"(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
        //    //displaced notes, no accidentals
        //"(chord (n c4 q p1)(n d4 q p1)(n g4 q p1))"
        //"(chord (n g5 q p1)(n f5 q p1)(n g4 q p1))"
        //"(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
        //    //displaced notes, accidentals
        //"(chord (n c4 q p1)(n -d4 q p1)(n +g4 q p1))"
        //"(chord (n +g5 q p1)(n -f5 q p1)(n =g4 q p1))"
        //"(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
        //    //chords from ref.paper
        //"(chord (n -e5 q p1)(n c5 q p1)(n =a4 q p1))"
        //"(chord (n +a5 q p1)(n +e5 q p1)(n +c5 q p1)(n +a4 q p1)(n +f4 q p1))"
        //"(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
        //"))"
        //")))" );

//        //note with accidentals. Anchor alignment
//        "(instrument (staves 2)(musicData "
//        "(clef G p1)(clef F4 p2)(key F)(time 2 4)"
//        "(n c4 q p1)(barline)"
//        "(n +c4 q p1)"
//        "(n +g5 q p1)"
//        "(goBack start)(n g3 q p2)(n g3 q p2)(barline)"
//        ")) )))" );

//        //70010-all-clefs
//        "(instrument (musicData "
//        "(clef G)(clef G1)(clef F3)(clef F4)(clef F5)(clef C1)(clef C2)"
//        "(clef C3)(clef C4)(clef C5)(clef percussion)(clef 8_G)(clef G_8)"
//        "(clef 15_G)(clef G_15)(clef 8_F4)(clef F4_8)(clef 15_F4)(clef F4_15)"
//        "))"
//        ")))" );

        ////70020-all-notes
        //"(opt Render.SpacingMethod 1)(opt Render.SpacingValue 40)"
        //"(instrument (musicData "
        //"(clef G)"
        //"(n f4 l)(n f4 b)(n f4 w)(n f4 h)(n f4 q)(n f4 e)(n f4 s)(n f4 t)"
        //"(n f4 i)(n f4 o)(n f4 f)(barline)"
        //"(n c5 l)(n c5 b)(n c5 w)(n c5 h)(n c5 q)(n c5 e)(n c5 s)(n c5 t)"
        //"(n c5 i)(n c5 o)(n c5 f)(barline end)"
        //"))"
        //")))" );

        ////00034-chord-with-accidentals-aligned
        //"(instrument (musicData "
        //"(clef F4)(key c)"
        //"(chord (n =e3 q)(n g3 q)(n -b3 q)(n c4 q)(n =e4 q))"
        //"))"
        //")))" );

        ////Two instruments
        //"(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData (clef G)(n c4 e.) )) "
        //"(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData (clef G p1)(clef F4 p2))) "
        //")))" );

        ////70030-all-rests
        //"(opt Render.SpacingMethod 1)(opt Render.SpacingValue 20)"
        //"(instrument (musicData "
        //"(clef G)(key C)(r l)(r b)(r w)(r h)(r q)(r e)(r s)(r t)(r i)(r o)(r f)"
        //"))"
        //")))" );

        ////80091-rests-in-beam
        //"(instrument (musicData "
        //"(clef G)(n g3 e g+)(r e)(n b3 e g-)(n c6 e g+)(r e)(n e5 e g-)"
        //"(barline end)"
        //"))"
        //")))" );

        ////beams
        //"(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData "
        //"(clef F4)(key E)(time 2 4)"
        //"(n c2 h)(barline)"
        //"(n d2 h)(barline)"
        //"(n e2 h)(barline)"
        //"(n f2 h)(barline)"
        //"(n g2 h)(barline)"
        //"(n a2 h)(barline)"
        //"(n b2 h)(barline)"
        //"(n c3 h)(barline)"
        //"(n d3 h)(barline)"
        //"(n b2 e (beam 6 begin))(n a2 e (beam 6 end))(n b3 q)(barline)"
        //"))"
        //")))" );

        ////nfoMIDI crash
        //"(instrument (staves 1)(infoMIDI 10 12)(musicData "
        //"(clef G)(n g4 q)"
        //"))"
        //")))" );

        ////Two instruments. Different staff size
        //"(instrument (staff 1 (staffSpacing 300))(musicData (clef G)(n c4 q)(n g4 q) )) "
        //"(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
        //"(n e5 q p1)(n f5 q p1)(goBack start)(n f3 q p2)(n c3 q p2))) "
        //")))" );

        ////beamed chords
        //"(instrument (musicData "
        //"(clef F)(key C)(time 4 4)"
        //"(chord (n a3 e) (n d3 e (beam 1 begin)))"
        //"(chord (n g3 e)(n e3 e (beam 1 end)))"
        //"))"
        //")))" );

        ////tuplet
        //"(instrument (musicData "
        //"(clef G)(key A)(time 2 4)"
        //"(n c4 e g+ t3/2)(n e4 e)(n d4 e g- t-)"
        //"(n e5 e g+ t3/2)(n c5 e)(n d5 e g- t-)"
        //"))"
        //")))" );

        //error in beam: stem position
        //"(instrument (musicData "
        //"(clef G)(key A)(time 2 4)"
        //"(n c4 e (t 1 + 3 2)(beam 1 begin))(n e4 e (beam 1 continue))(n d4 e (t 1 -)(beam 1 end))"
        //"(n c4 e (beam 1 begin))(n d4 s t3/2 (beam 1 continue begin))(n c4 s (beam 1 continue continue))(n b3 s t- (beam 1 end end))"
        //"(n c4 s (t 1 + 3 2)(g 1 ++))(n d4 s (g 1 ==))(n e3 s (t 1 -)(g 1 ==))"
        //"(n f4 s (t 1 + 3 2)(g 1 ==))(n g4 s (g 1 ==))(n a4 s (t 2 -)(g 1 --))"
        //"))"
        //")))" );

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


        ////tuplets-engraving-rule-b-1
        //"(instrument (name \"Violin\")(musicData "
        //"(time 4 4)"
        //"(n e4 e g+ t3)(n e4 e g-)(r e t-)"
        //"(r e t3)(n e4 e)(r e t-)"
        //"(n e5 e t3)(r e)(r e t-)"
        //"(r e t3)(r e)(n e5 e t-)"
        //"))"
        //")))" );

        ////fermata
        //"(instrument (musicData "
        //"(clef G)(key C)(time 4 4)"
        //"(n e4 q (fermata below))"
        //"(n c5 q (fermata above))"
        //"(r q (fermata))"
        //"(n g4 q (fermata))"
        //"(n a5 q (fermata above))"
        //"(n a3 q (fermata below))"
        //"(n e5 q (stem up)(fermata above))"
        //"))"
        //")))" );

        ////tie
        //"(instrument (musicData "
        //"(clef G)(key C)(time 4 4)"
        //"(n e4 q l)(n e4 q)"
        //"))"
        //")))" );

        ////system break
        //"(instrument (musicData "
        //"(clef G)(key C)(time 2 4)"
        //"(n c4 q l)(n c4 q)"
        //"(barline)"
        //"(n e4 q v1 (tie 3 start (bezier (start-x 30))) )"
        //"(newSystem)"
        //"(barline)"
        //"(n e4 q v1 (tie 3 stop (bezier (start-y 30))) )"
        //"(barline end)"
        //"))"
        //")))" );

        ////beam error
        //"(instrument (musicData (clef G)"
        //"(n d4 e v1 (beam 28 +))"
        //"(n c4 e v1 (beam 28 =))"
        //"(n d4 e v1 (beam 28 -))"
        //"(barline simple)"
        //")) )))" );


    // move to unit tests ----

        ////reposition. one staff, one measure
        //"(instrument (musicData (clef G)"
        //"(n e4 q)"
        //")) )))" );

        ////reposition. two staves, one measure
        //"(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
        //"(n e4 q p1)"
        //"(goBack start)(n e3 q p2)"
        //")) )))" );

        ////reposition. one staff, two measures
        //"(instrument (musicData (clef G)"
        //"(n e4 q)(barline)"
        //"(n f4 q)(barline)"
        //")) )))" );

        ////reposition. one staff, two systems
        //"(instrument (musicData (clef G)"
        //"(n e4 q)(barline)"
        //"(n f4 q)(barline)"
        //"(n g4 q)(barline)"
        //"(n a4 q)(barline)"
        //"(n b4 q)(barline)"
        //"(n c5 q)(barline)"
        //"(n b4 q)(barline)"
        //"(n a4 q)(barline)"
        //"(n g4 q)(barline)"
        //"(n f4 q)(barline)"
        //"(n e4 q)(barline)"
        //"(n d4 q)(barline)"
        //"(n c4 q)(barline)"
        //"(n d4 q)(barline)"
        //"(n e4 q)(barline)"
        //"(n f4 q)(barline)"
        //"(n g4 q)(barline)"
        //")) )))" );


}

//---------------------------------------------------------------------------------------
void update_view_content()
{
    //request the view to re-draw the bitmap

    if (!m_pInteractor) return;
    m_pInteractor->on_paint();
}

//---------------------------------------------------------------------------------------
void on_mouse_button_down(int x, int y, unsigned flags)
{
    if (!m_pInteractor) return;
    m_pInteractor->on_mouse_button_down(x, y, flags);
}

//---------------------------------------------------------------------------------------
void on_mouse_move(int x, int y, unsigned flags)
{
    if (!m_pInteractor) return;
    m_pInteractor->on_mouse_move(x, y, flags);
}

//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
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
void get_mouse_position(WPARAM wParam, LPARAM lParam)
{
    m_xMouse = int16(LOWORD(lParam));
    m_yMouse = int16(HIWORD(lParam));
}

//---------------------------------------------------------------------------------------
unsigned get_keyboard_flags(int wflags)
{
    unsigned flags = 0;
    if(wflags & MK_LBUTTON) flags |= k_mouse_left;
    if(wflags & MK_RBUTTON) flags |= k_mouse_right;
    if(wflags & MK_SHIFT)   flags |= k_kbd_shift;
    if(wflags & MK_CONTROL) flags |= k_kbd_ctrl;
    return flags;
}

//---------------------------------------------------------------------------------------
unsigned get_keycode(unsigned keycode)
{
    return m_last_key = (keycode > 255) ? 0 : keycode;
}

//---------------------------------------------------------------------------------------
void key_down_handler(WPARAM wParam, LPARAM lParam)
{
    m_last_key = 0;
    switch(wParam) 
    {
        case VK_CONTROL:
            m_input_flags |= k_kbd_ctrl;
            break;

        case VK_SHIFT:
            m_input_flags |= k_kbd_shift;
            break;

        default:
            get_keycode(unsigned int(wParam));
            break;
    }

    if(m_last_key)
        on_key(m_xMouse, m_yMouse, m_last_key, m_input_flags);
}

//---------------------------------------------------------------------------------------
// application main entry point
//
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	//initialize global strings
    m_hInst = hInstance;
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_EXAMPLE_1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

    //define a stopwatch for performance meassurements
    ::QueryPerformanceFrequency(&m_sw_freq);
    ::QueryPerformanceCounter(&m_sw_start);

    // Lomse knows nothing about windows. It renders everything on a bitmap and the
    // user application uses this bitmap. For instance, to display it on a window.
    // Lomse supports a lot of bitmap formats and pixel formats. Therefore, before
    // using the Lomse library you MUST specify which bitmap formap to use.
    //
    // For native MS Windows applications you can use, for instance, pixel format
    // BGRA, 32 bits. Screen resolution, in MS Windows, is 96 pixels per inch.
    // Let's define the requiered information:

        //the pixel format
        int pixel_format = k_pix_format_bgra32;  //BGRA, 32 bits
        m_bpp = 32;                              //32 bits per pixel

        //the desired resolution. For MS Windows use 96 pixels per inch
        int resolution = 96;    //96 ppi

        //Lomse default y axis direction is 0 coordinate at top and increases
        //downwards. You must specify if you would like just the opposite behaviour.
        //For MS Windows the Lomse default behaviour is the right behaviour.
        bool reverse_y_axis = false;

    //Now, initialize the Lomse library with these values
    m_lomse.init_library(pixel_format, resolution, reverse_y_axis);

    //set required callbacks
    m_lomse.set_get_font_callback(get_font_filename);

    //create a music score and a View. The view will display the score 
    //when the paint event is sent to lomse, once the main windows is
    //shown and the event handling loop is started
    open_document();

	//initialize and show main window
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	//the main event handling loop
	HACCEL hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_EXAMPLE_1);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    //delete the Presenter. This will also delete the Document and the View
    delete m_pPresenter;

	return (int) msg.wParam;
}

//---------------------------------------------------------------------------------------
// register this window class
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_EXAMPLE_1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_EXAMPLE_1;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//---------------------------------------------------------------------------------------
// save instance identifier and create main window
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    m_hWnd = CreateWindow(szWindowClass           //class name
                         , szTitle                //window caption
                         , WS_OVERLAPPEDWINDOW    //wflags
                         , CW_USEDEFAULT          //pos-x
                         , 0                      //pos-y
                         , 840  //CW_USEDEFAULT          //width
                         , 600  //0                      //height
                         , NULL
                         , NULL
                         , hInstance
                         , NULL
             );

    if (!m_hWnd)
        return FALSE;

    //create a bitmap for the View
    create_rendering_buffer_bitmap(m_hWnd);

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);

    return TRUE;
}

//---------------------------------------------------------------------------------------
// messages dispatcher for main window
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

    switch (message) 
	{
        //-------------------------------------------------
	    case WM_COMMAND:
		    wmId    = LOWORD(wParam); 
		    wmEvent = HIWORD(wParam); 
		    switch (wmId)
		    {
		        case IDM_ABOUT:
			        DialogBox(m_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			        break;
		        case IDM_EXIT:
			        DestroyWindow(hWnd);
			        break;
                case ID_FILE_OPEN:
                    on_open_file();
                    break;
		        default:
			        return DefWindowProc(hWnd, message, wParam, lParam);
		    }
		    break;

        //-------------------------------------------------
	    case WM_PAINT:
        {
            if (m_view_needs_redraw)
                update_view_content();
            m_view_needs_redraw = false;

            PAINTSTRUCT ps;
            HDC paintDC = ::BeginPaint(m_hWnd, &ps);
            display_view_content(paintDC);
            //::TextOut(paintDC, 10, 10, "Windows demo program: example_1", 31 );
            ::EndPaint(m_hWnd, &ps);
		    break;
        }
    
        //--------------------------------------------------------------------
        case WM_SIZE:
            create_pmap(LOWORD(lParam), HIWORD(lParam));
            //m_lomse.trans_affine_resizing(LOWORD(lParam), HIWORD(lParam));
            force_redraw(NULL);
            break;
        
        //--------------------------------------------------------------------
        case WM_ERASEBKGND:
            break;
        
        //--------------------------------------------------------------------
        case WM_LBUTTONDOWN:
            ::SetCapture(hWnd);
            get_mouse_position(wParam, lParam);
            m_input_flags = k_mouse_left | get_keyboard_flags(int(wParam));
            on_mouse_button_down(m_xMouse, m_yMouse, m_input_flags);
            break;

        //--------------------------------------------------------------------
        case WM_LBUTTONUP:
            ::ReleaseCapture();
            get_mouse_position(wParam, lParam);
            m_input_flags = k_mouse_left | get_keyboard_flags(int(wParam));
            on_mouse_button_up(m_xMouse, m_yMouse, m_input_flags);
            break;

        //--------------------------------------------------------------------
        case WM_RBUTTONDOWN:
            ::SetCapture(hWnd);
            get_mouse_position(wParam, lParam);
            m_input_flags = k_mouse_right | get_keyboard_flags(int(wParam));
            on_mouse_button_down(m_xMouse, m_yMouse, m_input_flags);
            break;

        //--------------------------------------------------------------------
        case WM_RBUTTONUP:
            ::ReleaseCapture();
            get_mouse_position(wParam, lParam);
            m_input_flags = k_mouse_right | get_keyboard_flags(int(wParam));
            on_mouse_button_up(m_xMouse, m_yMouse, m_input_flags);
            break;

        //--------------------------------------------------------------------
        case WM_MOUSEMOVE:
            get_mouse_position(wParam, lParam);
            m_input_flags = get_keyboard_flags(int(wParam));
            on_mouse_move(m_xMouse, m_yMouse, m_input_flags);
            break;

        ////--------------------------------------------------------------------
        //case WM_SYSKEYDOWN:
        //    // http://msdn.microsoft.com/en-us/library/ff468861%28v=VS.85%29.aspx
        //    //WM_SYSKEYDOWN : posted when the user presses the F10 key (which 
        //    //activates the menu bar) or holds down the ALT key and then presses 
        //    //another key. It also occurs when no window currently has the keyboard
        //    //focus; in this case, the WM_SYSKEYDOWN message is sent to the active
        //    //window. The window that receives the message can distinguish between
        //    //these two contexts by checking the context code in lParam parameter. 
        //case WM_KEYDOWN:
        //    key_down_handler(wParam, lParam);
        //    break;

        ////--------------------------------------------------------------------
        //case WM_SYSKEYUP:
        //case WM_KEYUP:
        //    m_last_key = 0;
        //    switch(wParam) 
        //    {
        //        case VK_CONTROL:
        //            m_input_flags &= ~k_kbd_ctrl;
        //            break;

        //        case VK_SHIFT:
        //            m_input_flags &= ~k_kbd_shift;
        //            break;
        //    }
        //    break;

        //--------------------------------------------------------------------
        case WM_CHAR:
        case WM_SYSCHAR:
            if(m_last_key == 0)
                on_key(m_xMouse, m_yMouse, unsigned int(wParam), m_input_flags);
            break;
        
        //--------------------------------------------------------------------
	    case WM_DESTROY:
		    PostQuitMessage(0);
		    break;

        //--------------------------------------------------------------------
	    default:
		    return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//---------------------------------------------------------------------------------------
// messages dispatcher for about window
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}


