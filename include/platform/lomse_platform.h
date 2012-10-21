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

#ifndef __LOMSE_PLATFORM_H__
#define __LOMSE_PLATFORM_H__

//---------------------------------------------------------------------------------------
// This file MUST NOT include any system dependent .h files such as "windows.h" or
// "X11.h", so the library do not depend on the platform.
// The only file that can #include system dependend headers is the implementation
// of this header. Different implementations are placed in different directories,
// such as
//      /lomse/src/platform/win32
//      /lomse/src/platform/X11
// and so on.
//
#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "ctrl/agg_slider_ctrl.h"
using namespace agg;

#include <string>
#include <iostream>
using namespace std;


namespace lomse
{

//---------------------------------------------------------------------------------------
//forward declarations
class LibraryScope;
class View;
class VerticalBookView;
class HorizontalBookView;
class SimpleView;
class Document;

typedef agg::rendering_buffer  RenderingBuffer;


enum pix_format_e
{
    pix_format_undefined = 0,  // By default. No conversions are applied 
    pix_format_bw,             // 1 bit per color B/W
    pix_format_gray8,          // Simple 256 level grayscale
    pix_format_gray16,         // Simple 65535 level grayscale
    pix_format_rgb555,         // 15 bit rgb. Depends on the byte ordering!
    pix_format_rgb565,         // 16 bit rgb. Depends on the byte ordering!
    pix_format_rgbAAA,         // 30 bit rgb. Depends on the byte ordering!
    pix_format_rgbBBA,         // 32 bit rgb. Depends on the byte ordering!
    pix_format_bgrAAA,         // 30 bit bgr. Depends on the byte ordering!
    pix_format_bgrABB,         // 32 bit bgr. Depends on the byte ordering!
    pix_format_rgb24,          // R-G-B, one byte per color component
    pix_format_bgr24,          // B-G-R, native win32 BMP format.
    pix_format_rgba32,         // R-G-B-A, one byte per color component
    pix_format_argb32,         // A-R-G-B, native MAC format
    pix_format_abgr32,         // A-B-G-R, one byte per color component
    pix_format_bgra32,         // B-G-R-A, native win32 BMP format
    pix_format_rgb48,          // R-G-B, 16 bits per color component
    pix_format_bgr48,          // B-G-R, native win32 BMP format.
    pix_format_rgba64,         // R-G-B-A, 16 bits byte per color component
    pix_format_argb64,         // A-R-G-B, native MAC format
    pix_format_abgr64,         // A-B-G-R, one byte per color component
    pix_format_bgra64,         // B-G-R-A, native win32 BMP format

    end_of_pix_formats
};


//---------------------------------------------------------------------------------------
// Possible formats for the rendering buffer
// IMPORTANT: This enum MUST duplicate agg::pix_format_e defined in header file
//            agg/include/platform/platform_support.h
enum EPixelFormat
{
    k_pix_format_undefined = 0,  // By default. No conversions are applied
    k_pix_format_bw,             // 1 bit per color B/W
    k_pix_format_gray8,          // Simple 256 level grayscale
    k_pix_format_gray16,         // Simple 65535 level grayscale
    k_pix_format_rgb555,         // 15 bit rgb. Depends on the byte ordering!
    k_pix_format_rgb565,         // 16 bit rgb. Depends on the byte ordering!
    k_pix_format_rgbAAA,         // 30 bit rgb. Depends on the byte ordering!
    k_pix_format_rgbBBA,         // 32 bit rgb. Depends on the byte ordering!
    k_pix_format_bgrAAA,         // 30 bit bgr. Depends on the byte ordering!
    k_pix_format_bgrABB,         // 32 bit bgr. Depends on the byte ordering!
    k_pix_format_rgb24,          // R-G-B, one byte per color component
    k_pix_format_bgr24,          // B-G-R, native win32 BMP format.
    k_pix_format_rgba32,         // R-G-B-A, one byte per color component
    k_pix_format_argb32,         // A-R-G-B, native MAC format
    k_pix_format_abgr32,         // A-B-G-R, one byte per color component
    k_pix_format_bgra32,         // B-G-R-A, native win32 BMP format
    k_pix_format_rgb48,          // R-G-B, 16 bits per color component
    k_pix_format_bgr48,          // B-G-R, native win32 BMP format.
    k_pix_format_rgba64,         // R-G-B-A, 16 bits byte per color component
    k_pix_format_argb64,         // A-R-G-B, native MAC format
    k_pix_format_abgr64,         // A-B-G-R, one byte per color component
    k_pix_format_bgra64,         // B-G-R-A, native win32 BMP format

    k_pix_formats_end
};



//---------------------------------------------------------------------------------------
struct PlatformInfo
{
    pix_format_e    format;     //see enum EPixelFormat
    bool            flip_y;     //true if you want to have the Y-axis flipped vertically
    double          screen_ppi; //screen resolution in pixels per inch
};

//    //--------------------------------------------------------------------
//    // These two functions control updating the window.
//    // force_redraw() is an analog of the Win32 InvalidateRect() function.
//    // When invoked, it sets a flag (or sends a message) which results
//    // in calling View::on_paint() and updating the content of
//    // the window when the next event cycle comes.
//    // update_window() results in just putting immediately the content
//    // of the currently rendered buffer to the window without calling
//    // any View methods (i.e. on_paint)
//    virtual void force_redraw() = 0;
//    virtual void update_window() = 0;
//
//    //--------------------------------------------------------------------
//    //change window title
//    virtual void set_window_title(const std::string& title) = 0;
//
//    //--------------------------------------------------------------------
//    //access to renderization bitmap
//    virtual RenderingBuffer& get_window_buffer() = 0;
//
//    //--------------------------------------------------------------------
//    //access to display resolution
//    virtual double get_screen_ppi() const = 0;
//
//    //--------------------------------------------------------------------
//    // Stopwatch functions. Function elapsed_time() returns time elapsed
//    // since the latest start_timer() invocation in millisecods.
//    // The resolutoin depends on the implementation.
//    virtual void start_timer() = 0;
//    virtual double elapsed_time() const = 0;
//
//};


//---------------------------------------------------------------------------------------
// This class represent the gate for lomse <-> platform/user app. communication
// For the lomse library, it is the way to access platform dependent methods
// For the user application it is the way to set up lomse and access basic methods
//
class LomseDoorway
{
protected:
    LibraryScope*   m_pLibraryScope;
    PlatformInfo    m_platform;

    //call backs for application/platform provided methods
    void (*m_pFunc_update_window)();
    void (*m_pFunc_force_redraw)();
    void (*m_pFunc_start_timer)();
    double (*m_pFunc_elapsed_time)();

public:
    LomseDoorway();
    ~LomseDoorway();

    enum EPlatformType { k_platform_win32, k_platform_x11, };

    int init_library(EPlatformType platform, ostream& reporter=cout);
    inline LibraryScope* get_library_scope() { return m_pLibraryScope; }

    //setting callbacks
    void set_update_window_callbak(void (*pt2Func)()) { m_pFunc_update_window = pt2Func; }
    void set_force_redraw_callbak(void (*pt2Func)()) { m_pFunc_force_redraw = pt2Func; }
    void set_start_timer_callbak(void (*pt2Func)()) { m_pFunc_start_timer = pt2Func; }
    void set_elapsed_time_callbak(double (*pt2Func)()) { m_pFunc_elapsed_time = pt2Func; }

    //access to platform info
    inline double get_screen_ppi() { return m_platform.screen_ppi; }

    //common operations
    View* create_view(int viewType, Document* pDoc);
    HorizontalBookView* create_horizontal_book_view(Document* pDoc);

    //access to renderization bitmap
    RenderingBuffer* get_window_buffer() { return NULL; }


    //providing platform services to lomse
    void update_window();
    void force_redraw();
    void start_timer();
    double elapsed_time();

    //win32 specific
    void create_pmap(unsigned width, unsigned height, rendering_buffer* wnd);

protected:
};


}   //namespace lomse

#endif      //__LOMSE_PLATFORM_H__
