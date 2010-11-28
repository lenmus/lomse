//---------------------------------------------------------------------------------------
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
using namespace std;

typedef agg::rendering_buffer  RenderingBuffer;

namespace lomse
{

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
class PlatformSupport
{
public:
    // format - see enum EPixelFormat
    // flip_y - true if you want to have the Y-axis flipped vertically.
    PlatformSupport(EPixelFormat format, bool flip_y) {}
    PlatformSupport(bool flip_y) {}
    virtual ~PlatformSupport() {}

    //--------------------------------------------------------------------
    // These two functions control updating the window.
    // force_redraw() is an analog of the Win32 InvalidateRect() function.
    // When invoked, it sets a flag (or sends a message) which results
    // in calling View::on_paint() and updating the content of
    // the window when the next event cycle comes.
    // update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without calling
    // any View methods (i.e. on_paint)
    virtual void force_redraw() = 0;
    virtual void update_window() = 0;

    //--------------------------------------------------------------------
    //change window title
    virtual void set_window_title(const std::string& title) = 0;

    //--------------------------------------------------------------------
    //access to renderization bitmap
    virtual RenderingBuffer& get_window_buffer() = 0;

    //--------------------------------------------------------------------
    // Stopwatch functions. Function elapsed_time() returns time elapsed
    // since the latest start_timer() invocation in millisecods.
    // The resolutoin depends on the implementation.
    virtual void start_timer() = 0;
    virtual double elapsed_time() const = 0;

};



}   //namespace lomse

#endif      //__LOMSE_PLATFORM_H__
