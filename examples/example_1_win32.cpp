//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

// header files required by Microsoft stuff
#include <windows.h>

//lomse headers
#include <lomse_doorway.h>
#include <lomse_document.h>
#include <lomse_graphic_view.h>
#include <lomse_interactor.h>
#include <lomse_presenter.h>
#include <lomse_events.h>

using namespace lomse;


//---------------------------------------------------------------------------------------
// Helper class to create and manage bitmaps
// This class is copied (and simplified) from AGG project.
//---------------------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.

class Bitmap
{
private:
    BITMAPINFO*    m_bmp;
    unsigned char* m_buf;
    unsigned       m_bpp;
    bool           m_is_internal;
    unsigned       m_img_size;
    unsigned       m_full_size;

public:
    Bitmap();
    ~Bitmap();

    void destroy();
    void create(unsigned int width, unsigned int height, unsigned int bpp,
                unsigned int clear_val=256);

    void draw(HDC h_dc, const RECT* device_rect=0, const RECT* bmp_rect=0) const;

    inline unsigned char* buf() {return m_buf; };
    inline unsigned width() const { return m_bmp->bmiHeader.biWidth; }
    inline unsigned height() const { return m_bmp->bmiHeader.biHeight; }
    int stride() const;

private:
    //Auxiliary static functions
    static unsigned calc_full_size(BITMAPINFO *bmp);
    static unsigned calc_header_size(BITMAPINFO *bmp);
    static unsigned calc_palette_size(unsigned clr_used, 
                                        unsigned bits_per_pixel);
    static unsigned calc_palette_size(BITMAPINFO *bmp);
    static unsigned char* calc_img_ptr(BITMAPINFO *bmp);
    static BITMAPINFO* create_bitmap_info(unsigned width, 
                                            unsigned height, 
                                            unsigned bits_per_pixel);
    static void     create_gray_scale_palette(BITMAPINFO *bmp);
    static unsigned calc_row_len(unsigned width, unsigned bits_per_pixel);
    
    void create_from_bmp(BITMAPINFO *bmp);

};

//---------------------------------------------------------------------------------------
// Global variables
// In this first example we are just going to display an score on the main window.
// Let's define the necessary variables:
//
LomseDoorway    m_lomse;        //the Lomse library doorway
Presenter*      m_pPresenter;
Interactor*     m_pInteractor;  //to interact with the View

//the Lomse View renders its content on a bitmap. To manage it, Lomse
//associates the bitmap to a RenderingBuffer object.
//It is your responsibility to render the bitmap on a window.
//Here you define the rendering buffer and its associated bitmap
RenderingBuffer     m_rbuf_window;
Bitmap              m_bitmap;

//Lomse can manage a lot of bitmap formats and pixel formats. You must
//define the format that you are going to use 
unsigned int        m_bpp;         //bits per pixel

//some additinal variables
bool    m_view_needs_redraw;      //to control when the View must be re-drawed


// All typical MS Windows stuff needed to run the program and the main events handler loop
HINSTANCE m_hInst;      //current instance
HWND m_hWnd;            //the main window

//forward declarations
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


//---------------------------------------------------------------------------------------
void create_bitmap_for_the_rendering_buffer(unsigned width, unsigned height)
{
    //creates a bitmap of specified size and associates it to the rendering
    //buffer for the view. Any existing buffer is automatically deleted

    m_bitmap.create(width, height, m_bpp);
    m_rbuf_window.attach(m_bitmap.buf(), 
                         m_bitmap.width(),
                         m_bitmap.height(),
                         -m_bitmap.stride()
                        );
    m_view_needs_redraw = true;
}

//---------------------------------------------------------------------------------------
void copy_buffer_on_dc(HDC dc)
{
    m_bitmap.draw(dc);
}

//---------------------------------------------------------------------------------------
string get_font_filename(RequestFont* pRequest)
{
    //This is just a trivial example. In real applications you should 
    //use operating system services to find a suitable font
    
    //notes on parameters received:
    // - fontname can be either the face name (i.e. "Book Antiqua") or
    //   the familly name (i.e. "sans-serif")

    string fontname = pRequest->get_fontname();
    bool bold = pRequest->get_bold();
    bool italic = pRequest->get_italic();

    string path = "C:\\WINNT\\Fonts\\";

    //if family name, choose a font name
    if (fontname == "serif")
        fontname = "Times New Roman";
    else if (fontname == "sans-serif")
        fontname = "Tahoma";
    else if (fontname == "handwritten")
        fontname = "Lucida Handwriting";
    else if (fontname == "cursive")
        fontname = "Monotype Corsiva";
    else if (fontname == "monospaced")
        fontname = "Courier New";

    //choose a suitable font file
    string fontfile;
    if (fontname == "Times New Roman")
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

    else if (fontname == "Tahoma")
    {
        if (bold)
            fontfile = "tahomabd.ttf";
        else
            fontfile = "tahoma.ttf";
    }

    else if (fontname == "Lucida Handwriting")
    {
        fontfile = "lhandw.ttf";
    }

    else if (fontname == "Monotype Corsiva")
    {
        fontfile = "mtcorsva.ttf";
    }

    else if (fontname == "Courier New")
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
void on_lomse_request(void* pThis, Request* pRequest)
{
    int type = pRequest->get_request_type();
    switch (type)
    {
        case k_get_font_filename:
            get_font_filename( dynamic_cast<RequestFont*>(pRequest) );
            break;

        default:
            fprintf(stderr, "on_lomse_request] Unknown request\n");
    }
}

//---------------------------------------------------------------------------------------
void open_test_document()
{
    //Normally you will load the content of a file. But in this simple example we
    //will create an empty document and define its content from a text string

    //first, we will create a 'presenter'. It takes care of creating and maintaining
    //all objects and relationships between the document, its views and the interactors
    //to interact with the view
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(ViewFactory::k_view_vertical_book,
        "(lenmusdoc (vers 0.0)"
            "(content "
                "(para (txt \"Hello world!\"))"
                "(score (vers 1.6) "
                    "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q) )))"
            ")"
        ")" );

    //now, get the pointers to the relevant components
    m_pInteractor = m_pPresenter->get_interactor(0);

    //connect the View with the window buffer
    m_pInteractor->set_rendering_buffer(&m_rbuf_window);
}

//---------------------------------------------------------------------------------------
void update_rendering_buffer_if_needed()
{
    //request the view to re-draw the bitmap

    if (!m_pInteractor) return;

    if (m_view_needs_redraw)
        m_pInteractor->force_redraw();
    m_view_needs_redraw = false;
}

//---------------------------------------------------------------------------------------
void initialize_lomse()
{
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
    m_lomse.set_request_callback(NULL, on_lomse_request);
}

//---------------------------------------------------------------------------------------
int handle_events()
{
    //the main event handling loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, NULL, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

//---------------------------------------------------------------------------------------
void free_resources()
{
    //delete the Presenter. 
    //This will also delete the Interactor, the Document and the View
    delete m_pPresenter;
}

//---------------------------------------------------------------------------------------
// register this window class
//
ATOM register_window_class(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

    m_hInst = hInstance;
	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInst;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION); 
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW); 
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= "Lomse_Example1";
	wcex.hIconSm		= 0;

	return RegisterClassEx(&wcex);
}

//---------------------------------------------------------------------------------------
// save instance identifier and create main window
//
BOOL create_main_window(int nCmdShow)
{
    m_hWnd = CreateWindow( "Lomse_Example1"                 //class name
                         , "Lomse tutorial 1 for win32"     //window caption
                         , WS_OVERLAPPEDWINDOW              //wflags
                         , CW_USEDEFAULT                    //pos-x
                         , 0                                //pos-y
                         , 840                              //width
                         , 600                              //height
                         , NULL                             //parent Window
                         , NULL                             //menu, or windows id if child
                         , m_hInst
                         , NULL                             //ptr to window specific data
             );

    if (!m_hWnd)
        return FALSE;

    //display the window
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return TRUE;
}

//---------------------------------------------------------------------------------------
// messages dispatcher for main window
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
	{
        //-------------------------------------------------
	    case WM_PAINT:
        {
            if (m_view_needs_redraw)
                update_rendering_buffer_if_needed();
            m_view_needs_redraw = false;

            PAINTSTRUCT ps;
            HDC paintDC = ::BeginPaint(m_hWnd, &ps);
            copy_buffer_on_dc(paintDC);
            ::EndPaint(m_hWnd, &ps);
		    break;
        }
    
        //--------------------------------------------------------------------
        case WM_SIZE:
            create_bitmap_for_the_rendering_buffer(LOWORD(lParam), HIWORD(lParam));
            break;
        
        //--------------------------------------------------------------------
        case WM_ERASEBKGND:
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
// application main entry point
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
	register_window_class(hInstance);
    initialize_lomse();

    //create a music score and a View. The view will display the score 
    //when a paint event is received, once the main windows is
    //shown and the event handling loop is started
    open_test_document();

	//initialize and show main window
	if (!create_main_window(nCmdShow)) 
		return FALSE;
	
    //enter the main event handling loop
    int retcode = handle_events(); 

    //terminate the application
    free_resources();
	return retcode;
}



//---------------------------------------------------------------------------------------
// Implementation of helper class Bitmap
//---------------------------------------------------------------------------------------
Bitmap::Bitmap()
    : m_bmp(0)
    , m_buf(0)
    , m_bpp(0)
    , m_is_internal(false)
    , m_img_size(0)
    , m_full_size(0)
{
}

//---------------------------------------------------------------------------------------
Bitmap::~Bitmap()
{
    destroy();
}

//---------------------------------------------------------------------------------------
void Bitmap::destroy()
{
    if(m_bmp && m_is_internal)
        delete [] (unsigned char*)m_bmp;
    m_bmp  = 0;
    m_is_internal = false;
    m_buf = 0;
}

//---------------------------------------------------------------------------------------
void Bitmap::create(unsigned int width, unsigned int height, unsigned int bpp,
                        unsigned int clear_val)
{
    destroy();
    if(width == 0)  width = 1;
    if(height == 0) height = 1;
    m_bpp = bpp;
    create_from_bmp(create_bitmap_info(width, height, m_bpp));
    create_gray_scale_palette(m_bmp);
    m_is_internal = true;
    if(clear_val <= 255)
    {
        memset(m_buf, clear_val, m_img_size);
    }
}

//---------------------------------------------------------------------------------------
unsigned Bitmap::calc_full_size(BITMAPINFO *bmp)
{
    if(bmp == 0) return 0;

    return sizeof(BITMAPINFOHEADER) +
            sizeof(RGBQUAD) * calc_palette_size(bmp) +
            bmp->bmiHeader.biSizeImage;
}

//---------------------------------------------------------------------------------------
unsigned Bitmap::calc_header_size(BITMAPINFO *bmp)
{
    if(bmp == 0) return 0;
    return sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * calc_palette_size(bmp);
}

//---------------------------------------------------------------------------------------
unsigned  Bitmap::calc_palette_size(unsigned  clr_used, unsigned bits_per_pixel)
{
    int palette_size = 0;

    if(bits_per_pixel <= 8)
    {
        palette_size = clr_used;
        if(palette_size == 0)
        {
            palette_size = 1 << bits_per_pixel;
        }
    }
    return palette_size;
}

//---------------------------------------------------------------------------------------
unsigned Bitmap::calc_palette_size(BITMAPINFO *bmp)
{
    if(bmp == 0) return 0;
    return calc_palette_size(bmp->bmiHeader.biClrUsed, bmp->bmiHeader.biBitCount);
}

//---------------------------------------------------------------------------------------
unsigned char * Bitmap::calc_img_ptr(BITMAPINFO *bmp)
{
    if(bmp == 0) return 0;
    return ((unsigned char*)bmp) + calc_header_size(bmp);
}

//---------------------------------------------------------------------------------------
BITMAPINFO* Bitmap::create_bitmap_info(unsigned width, 
                                            unsigned height, 
                                            unsigned bits_per_pixel)
{
    unsigned line_len = calc_row_len(width, bits_per_pixel);
    unsigned img_size = line_len * height;
    unsigned rgb_size = calc_palette_size(0, bits_per_pixel) * sizeof(RGBQUAD);
    unsigned full_size = sizeof(BITMAPINFOHEADER) + rgb_size + img_size;

    BITMAPINFO *bmp = (BITMAPINFO *) new unsigned char[full_size];

    bmp->bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
    bmp->bmiHeader.biWidth  = width;
    bmp->bmiHeader.biHeight = height;
    bmp->bmiHeader.biPlanes = 1;
    bmp->bmiHeader.biBitCount = (unsigned short)bits_per_pixel;
    bmp->bmiHeader.biCompression = 0;
    bmp->bmiHeader.biSizeImage = img_size;
    bmp->bmiHeader.biXPelsPerMeter = 0;
    bmp->bmiHeader.biYPelsPerMeter = 0;
    bmp->bmiHeader.biClrUsed = 0;
    bmp->bmiHeader.biClrImportant = 0;

    return bmp;
}

//---------------------------------------------------------------------------------------
void Bitmap::create_gray_scale_palette(BITMAPINFO *bmp)
{
    if(bmp == 0) return;

    unsigned rgb_size = calc_palette_size(bmp);
    RGBQUAD *rgb = (RGBQUAD*)(((unsigned char*)bmp) + sizeof(BITMAPINFOHEADER));
    unsigned brightness;
    unsigned i;

    for(i = 0; i < rgb_size; i++)
    {
        brightness = (255 * i) / (rgb_size - 1);
        rgb->rgbBlue =
        rgb->rgbGreen =  
        rgb->rgbRed = (unsigned char)brightness; 
        rgb->rgbReserved = 0;
        rgb++;
    }
}

//---------------------------------------------------------------------------------------
unsigned Bitmap::calc_row_len(unsigned width, unsigned bits_per_pixel)
{
    unsigned n = width;
    unsigned k;

    switch(bits_per_pixel)
    {
        case  1: k = n;
                    n = n >> 3;
                    if(k & 7) n++; 
                    break;

        case  4: k = n;
                    n = n >> 1;
                    if(k & 3) n++; 
                    break;

        case  8:
                    break;

        case 16: n *= 2;
                    break;

        case 24: n *= 3; 
                    break;

        case 32: n *= 4;
                    break;

        case 48: n *= 6; 
                    break;

        case 64: n *= 8; 
                    break;

        default: n = 0;
                    break;
    }
    return ((n + 3) >> 2) << 2;
}

//---------------------------------------------------------------------------------------
void Bitmap::draw(HDC h_dc, const RECT *device_rect, const RECT *bmp_rect) const
{
    if(m_bmp == 0 || m_buf == 0) return;

    unsigned bmp_x = 0;
    unsigned bmp_y = 0;
    unsigned bmp_width  = m_bmp->bmiHeader.biWidth;
    unsigned bmp_height = m_bmp->bmiHeader.biHeight;
    unsigned dvc_x = 0;
    unsigned dvc_y = 0; 
    unsigned dvc_width  = m_bmp->bmiHeader.biWidth;
    unsigned dvc_height = m_bmp->bmiHeader.biHeight;
    
    if(bmp_rect) 
    {
        bmp_x      = bmp_rect->left;
        bmp_y      = bmp_rect->top;
        bmp_width  = bmp_rect->right  - bmp_rect->left;
        bmp_height = bmp_rect->bottom - bmp_rect->top;
    } 

    dvc_x      = bmp_x;
    dvc_y      = bmp_y;
    dvc_width  = bmp_width;
    dvc_height = bmp_height;

    if(device_rect) 
    {
        dvc_x      = device_rect->left;
        dvc_y      = device_rect->top;
        dvc_width  = device_rect->right  - device_rect->left;
        dvc_height = device_rect->bottom - device_rect->top;
    }

    if(dvc_width != bmp_width || dvc_height != bmp_height)
    {
        ::SetStretchBltMode(h_dc, COLORONCOLOR);
        ::StretchDIBits(
            h_dc,            // handle of device context 
            dvc_x,           // x-coordinate of upper-left corner of source rect. 
            dvc_y,           // y-coordinate of upper-left corner of source rect. 
            dvc_width,       // width of source rectangle 
            dvc_height,      // height of source rectangle 
            bmp_x,
            bmp_y,           // x, y -coordinates of upper-left corner of dest. rect. 
            bmp_width,       // width of destination rectangle 
            bmp_height,      // height of destination rectangle 
            m_buf,           // address of bitmap bits 
            m_bmp,           // address of bitmap data 
            DIB_RGB_COLORS,  // usage 
            SRCCOPY          // raster operation code 
        );
    }
    else
    {
        ::SetDIBitsToDevice(
            h_dc,            // handle to device context
            dvc_x,           // x-coordinate of upper-left corner of 
            dvc_y,           // y-coordinate of upper-left corner of 
            dvc_width,       // source rectangle width
            dvc_height,      // source rectangle height
            bmp_x,           // x-coordinate of lower-left corner of 
            bmp_y,           // y-coordinate of lower-left corner of 
            0,               // first scan line in array
            bmp_height,      // number of scan lines
            m_buf,           // address of array with DIB bits
            m_bmp,           // address of structure with bitmap info.
            DIB_RGB_COLORS   // RGB or palette indexes
        );
    }
}

//---------------------------------------------------------------------------------------
int Bitmap::stride() const
{
    return calc_row_len(m_bmp->bmiHeader.biWidth, 
                        m_bmp->bmiHeader.biBitCount);
}

//---------------------------------------------------------------------------------------
void Bitmap::create_from_bmp(BITMAPINFO *bmp)
{
    if(bmp)
    {
        m_img_size  = calc_row_len(bmp->bmiHeader.biWidth, 
                                    bmp->bmiHeader.biBitCount) * 
                        bmp->bmiHeader.biHeight;

        m_full_size = calc_full_size(bmp);
        m_bmp       = bmp;
        m_buf       = calc_img_ptr(bmp);
    }
}
