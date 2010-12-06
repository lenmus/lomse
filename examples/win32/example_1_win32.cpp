//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include "stdafx.h"
#include "resource.h"
#include "lomse_doorway.h"
#include "lomse_document.h"
#include "lomse_graphic_view.h"
#include "platform/win32/agg_win32_bmp.h"
using namespace lomse;

#define MAX_LOADSTRING 100

//---------------------------------------------------------------------------------------
// In this first example we are just going to display an score on the main window.
// Let's define the necessary variables:
//
LomseDoorway    m_lomse;        //the Lomse library doorway
GraphicView*    m_pView;        //the View for displaying a score
Document*       m_pDoc;         //the score to display

//the Lomse View renders its content on a bitmap. To manage it, Lomse
//associates the bitmap to a RenderingBuffer object.
//It is your responsibility to render the bitmap on a window.
//Here you define the rendering buffer and its associated bitmap to be
//used by the previously defined View.
pixel_map           m_pmap_window;
RenderingBuffer     m_rbuf_window;


//Lomse can manage a lot of bitmap formats and pixel formats. You must
//define the format that you are going to use 
pix_format_e     m_format;      //bitmap format
unsigned         m_bpp;         //bits per pixel
bool             m_flip_y;      //true if y axis is reversed

//some additinal variables
bool    m_view_needs_redraw;      //to control when the View must be re-drawed

//to measure ellapsed time (for performance measurements)
LARGE_INTEGER m_sw_freq;
LARGE_INTEGER m_sw_start;

//for keyboard support
unsigned      m_last_translated_key;    //last pressed key once translated
unsigned      m_keymap[256];            //Win32 keys <-> Lomse keys translation map


//for mouse click & move position
int           m_xMouse;
int           m_yMouse;
unsigned      m_input_flags;


// All typical Windows stuff needed to run the program and the main events handler loop
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
void init_keyboard_map()
{
    //creates and initilizes the Win32 keys <-> Lomse keys translation map
    //It contains lomse keycodes to assign to other keys different from
    //letters, numbers and usual punctuation marks

    memset(m_keymap, 0, sizeof(m_keymap));

    m_keymap[VK_PAUSE]      = key_pause;
    m_keymap[VK_CLEAR]      = key_clear;

    m_keymap[VK_NUMPAD0]    = key_kp0;
    m_keymap[VK_NUMPAD1]    = key_kp1;
    m_keymap[VK_NUMPAD2]    = key_kp2;
    m_keymap[VK_NUMPAD3]    = key_kp3;
    m_keymap[VK_NUMPAD4]    = key_kp4;
    m_keymap[VK_NUMPAD5]    = key_kp5;
    m_keymap[VK_NUMPAD6]    = key_kp6;
    m_keymap[VK_NUMPAD7]    = key_kp7;
    m_keymap[VK_NUMPAD8]    = key_kp8;
    m_keymap[VK_NUMPAD9]    = key_kp9;
    m_keymap[VK_DECIMAL]    = key_kp_period;
    m_keymap[VK_DIVIDE]     = key_kp_divide;
    m_keymap[VK_MULTIPLY]   = key_kp_multiply;
    m_keymap[VK_SUBTRACT]   = key_kp_minus;
    m_keymap[VK_ADD]        = key_kp_plus;

    m_keymap[VK_UP]         = key_up;
    m_keymap[VK_DOWN]       = key_down;
    m_keymap[VK_RIGHT]      = key_right;
    m_keymap[VK_LEFT]       = key_left;
    m_keymap[VK_INSERT]     = key_insert;
    m_keymap[VK_DELETE]     = key_delete;
    m_keymap[VK_HOME]       = key_home;
    m_keymap[VK_END]        = key_end;
    m_keymap[VK_PRIOR]      = key_page_up;
    m_keymap[VK_NEXT]       = key_page_down;

    m_keymap[VK_F1]         = key_f1;
    m_keymap[VK_F2]         = key_f2;
    m_keymap[VK_F3]         = key_f3;
    m_keymap[VK_F4]         = key_f4;
    m_keymap[VK_F5]         = key_f5;
    m_keymap[VK_F6]         = key_f6;
    m_keymap[VK_F7]         = key_f7;
    m_keymap[VK_F8]         = key_f8;
    m_keymap[VK_F9]         = key_f9;
    m_keymap[VK_F10]        = key_f10;
    m_keymap[VK_F11]        = key_f11;
    m_keymap[VK_F12]        = key_f12;
    m_keymap[VK_F13]        = key_f13;
    m_keymap[VK_F14]        = key_f14;
    m_keymap[VK_F15]        = key_f15;

    m_keymap[VK_NUMLOCK]    = key_numlock;
    m_keymap[VK_CAPITAL]    = key_capslock;
    m_keymap[VK_SCROLL]     = key_scrollock;
}

//---------------------------------------------------------------------------------------
void create_pmap(unsigned width, unsigned height)
{
    //creates a bitmap of specified size and associates it to the rendering
    //buffer for the view. Any existing buffer is automatically deleted

    m_pmap_window.create(width, height, org_e(m_bpp));
    m_rbuf_window.attach(m_pmap_window.buf(), 
                         m_pmap_window.width(),
                         m_pmap_window.height(),
                         m_flip_y ? m_pmap_window.stride() :
                                   -m_pmap_window.stride()
                        );
}

//---------------------------------------------------------------------------------------
void display_view_content(HDC dc)
{
    //renders the view buffer into the main window

    m_pmap_window.draw(dc);
}

//---------------------------------------------------------------------------------------
void force_redraw()
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
void update_window()
{
    // Callback method for Lomse. It can be used also by your application.
    // Invoking update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without calling
    // any View methods (i.e. on_paint)

    HDC dc = ::GetDC(m_hWnd);
    display_view_content(dc);       //render view buffer into the main window
    ::ReleaseDC(m_hWnd, dc);
}

//-------------------------------------------------------------------------
void open_document()
{
    //Normally you will create by loading the content of a file. But in this 
    //simple //example the score is defined in a text string

    m_pDoc = new Document( *(m_lomse.get_library_scope()) );
    m_pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
        //"(instrument (musicData (clef G)(clef F3)(clef C1)(clef F4)) )))" );

        //"(instrument (name \"Violin\")(musicData (clef G)(clef F4)(clef C1)) )))" );

        //"(instrument (musicData )) )))" );

        //"(instrument (staves 2) (musicData )) )))" );
        //"(instrument (musicData )) (instrument (musicData )) )))" );

        "(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData (clef G))) "
        "(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData (clef G p1)(clef F4 p2)) )))" );

    //create the View for this document
    m_pView = m_lomse.create_horizontal_book_view(m_pDoc);
    m_pView->set_rendering_buffer(&m_rbuf_window);
}

//-------------------------------------------------------------------------
void update_view_content()
{
    //request the view to re-draw the bitmap

    if (!m_pView) return;
    m_pView->on_paint();
}

//-------------------------------------------------------------------------
void on_mouse_button_down(int x, int y, unsigned flags)
{
    if (!m_pView) return;
    m_pView->on_mouse_button_down(x, y, flags);
}

//-------------------------------------------------------------------------
void on_mouse_move(int x, int y, unsigned flags)
{
    if (!m_pView) return;
    m_pView->on_mouse_move(x, y, flags);
}

//-------------------------------------------------------------------------
void on_mouse_button_up(int x, int y, unsigned flags)
{
    if (!m_pView) return;
    m_pView->on_mouse_button_up(x, y, flags);
}

//-------------------------------------------------------------------------
void on_key(int x, int y, unsigned key, unsigned flags)
{
    if (!m_pView) return;
    //m_pView->on_key(x, y, key, flags);

    switch (key)
    {
        case '1':
            m_pView->set_option_draw_box_doc_page_content(true);
            break;
        case '2':
            m_pView->set_option_draw_box_score_page(true);
            break;
        case '3':
            m_pView->set_option_draw_box_system(true);
            break;
        case '4':
            m_pView->set_option_draw_box_slice(true);
            break;
        case '5':
            m_pView->set_option_draw_box_slice_instr(true);
            break;
        case '0':
            m_pView->set_option_draw_box_doc_page_content(false);
            m_pView->set_option_draw_box_score_page(false);
            m_pView->set_option_draw_box_system(false);
            m_pView->set_option_draw_box_slice(false);
            m_pView->set_option_draw_box_slice_instr(false);
            break;
        case '+':
            m_pView->zoom_in(x, y);
            break;
        case '-':
            m_pView->zoom_out(x, y);
            break;
        default:
            ;
    }

    force_redraw();
}

//-------------------------------------------------------------------------
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
void start_timer()
{
    ::QueryPerformanceCounter(&(m_sw_start));
}

//---------------------------------------------------------------------------------------
double elapsed_time()
{
    LARGE_INTEGER stop;
    ::QueryPerformanceCounter(&stop);
    return double(stop.QuadPart - 
                    m_sw_start.QuadPart) * 1000.0 / 
                    double(m_sw_freq.QuadPart);
}

//---------------------------------------------------------------------------------------
void get_mouse_position(WPARAM wParam, LPARAM lParam)
{
    m_xMouse = int16(LOWORD(lParam));

    if(m_flip_y)
        m_yMouse = m_rbuf_window.height() - int16(HIWORD(lParam));
    else
        m_yMouse = int16(HIWORD(lParam));
}

//---------------------------------------------------------------------------------------
unsigned get_keyboard_flags(int wflags)
{
    unsigned flags = 0;
    if(wflags & MK_LBUTTON) flags |= mouse_left;
    if(wflags & MK_RBUTTON) flags |= mouse_right;
    if(wflags & MK_SHIFT)   flags |= kbd_shift;
    if(wflags & MK_CONTROL) flags |= kbd_ctrl;
    return flags;
}

//---------------------------------------------------------------------------------------
unsigned translate_keycode(unsigned keycode)
{
    return m_last_translated_key = (keycode > 255) ? 0 : m_keymap[keycode];
}

//---------------------------------------------------------------------------------------
void key_down_handler(WPARAM wParam, LPARAM lParam)
{
    m_last_translated_key = 0;
    switch(wParam) 
    {
        case VK_CONTROL:
            m_input_flags |= kbd_ctrl;
            break;

        case VK_SHIFT:
            m_input_flags |= kbd_shift;
            break;

        default:
            translate_keycode(unsigned int(wParam));
            break;
    }

    if(m_last_translated_key)
        on_key(m_xMouse, m_yMouse, m_last_translated_key, m_input_flags);
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

    //define rendering buffer bitmap format
    m_format = pix_format_bgra32;   //windows bitmap format
    m_bpp = 32;                     //32 bits per pixel
    m_flip_y = false;               //y axis is not reversed

    //initialize other lomse related variables
    m_pView = NULL;
    m_pDoc = NULL;
    m_view_needs_redraw = true;

    //initialize the keyborad translation map
    init_keyboard_map();

    //initialize the library and set the required callbacks
    m_lomse.init_library(LomseDoorway::k_platform_win32);
    m_lomse.set_start_timer_callbak(start_timer);
    m_lomse.set_elapsed_time_callbak(elapsed_time);
    m_lomse.set_force_redraw_callbak(force_redraw);
    m_lomse.set_update_window_callbak(update_window);

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

    //delete the View. This will also delete the Document
    delete m_pView;

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
                         , CW_USEDEFAULT          //width
                         , 0                      //height
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

            PAINTSTRUCT ps;
            HDC paintDC = ::BeginPaint(m_hWnd, &ps);
            m_view_needs_redraw = false;
            display_view_content(paintDC);
            //::TextOut(paintDC, 10, 10, "Windows demo program: example_1", 31 );
            ::EndPaint(m_hWnd, &ps);
		    break;
        }
    
        //--------------------------------------------------------------------
        case WM_SIZE:
            create_pmap(LOWORD(lParam), HIWORD(lParam));
            //m_lomse.trans_affine_resizing(LOWORD(lParam), HIWORD(lParam));
            force_redraw();
            break;
        
        //--------------------------------------------------------------------
        case WM_ERASEBKGND:
            break;
        
        //--------------------------------------------------------------------
        case WM_LBUTTONDOWN:
            ::SetCapture(hWnd);
            get_mouse_position(wParam, lParam);
            m_input_flags = mouse_left | get_keyboard_flags(int(wParam));
            on_mouse_button_down(m_xMouse, m_yMouse, m_input_flags);
            break;

        //--------------------------------------------------------------------
        case WM_LBUTTONUP:
            ::ReleaseCapture();
            get_mouse_position(wParam, lParam);
            m_input_flags = mouse_left | get_keyboard_flags(int(wParam));
            on_mouse_button_up(m_xMouse, m_yMouse, m_input_flags);
            break;

        //--------------------------------------------------------------------
        case WM_RBUTTONDOWN:
            ::SetCapture(hWnd);
            get_mouse_position(wParam, lParam);
            m_input_flags = mouse_right | get_keyboard_flags(int(wParam));
            on_mouse_button_down(m_xMouse, m_yMouse, m_input_flags);
            break;

        //--------------------------------------------------------------------
        case WM_RBUTTONUP:
            ::ReleaseCapture();
            get_mouse_position(wParam, lParam);
            m_input_flags = mouse_right | get_keyboard_flags(int(wParam));
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
        //    m_last_translated_key = 0;
        //    switch(wParam) 
        //    {
        //        case VK_CONTROL:
        //            m_input_flags &= ~kbd_ctrl;
        //            break;

        //        case VK_SHIFT:
        //            m_input_flags &= ~kbd_shift;
        //            break;
        //    }
        //    break;

        //--------------------------------------------------------------------
        case WM_CHAR:
        case WM_SYSCHAR:
            if(m_last_translated_key == 0)
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


