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

#ifndef __LOMSE_PIXEL_FORMATS_H__
#define __LOMSE_PIXEL_FORMATS_H__


#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
using namespace agg;

namespace lomse
{

//---------------------------------------------------------------------------------------
// Possible formats for the rendering buffer
//
// Native bitmap formats for the different platforms are:
//
//    Win32 - pix_format_bgra32     // B-G-R-A, native win32 BMP format
//    X11   - pix_format_rgba32,    // R-G-B-A, one byte per color
//    Mac   - pix_format_argb32,    // A-R-G-B, native MAC format
//
// IMPORTANT: This enum MUST duplicate agg::pix_format_e defined in header file
//            agg/include/platform/platform_support.h
//
enum EPixelFormat
{
    k_pix_format_undefined = 0,  // By default. No conversions are applied
    //k_pix_format_bw = 1,         // 1 bit per color B/W
    k_pix_format_gray8 = 2,      // Simple 256 level grayscale
    k_pix_format_gray16 = 3,     // Simple 65535 level grayscale
    k_pix_format_rgb555 = 4,     // 15 bit rgb. Depends on the byte ordering!
    k_pix_format_rgb565 = 5,     // 16 bit rgb. Depends on the byte ordering!
    k_pix_format_rgbAAA = 6,     // 30 bit rgb. Depends on the byte ordering!
    k_pix_format_rgbBBA = 7,     // 32 bit rgb. Depends on the byte ordering!
    k_pix_format_bgrAAA = 8,     // 30 bit bgr. Depends on the byte ordering!
    k_pix_format_bgrABB = 9,     // 32 bit bgr. Depends on the byte ordering!
    k_pix_format_rgb24 = 10,     // R-G-B, one byte per color component
    k_pix_format_bgr24 = 11,     // B-G-R, native win32 BMP format.
    k_pix_format_rgba32 = 12,    // R-G-B-A, one byte per color component
    k_pix_format_argb32 = 13,    // A-R-G-B, native MAC format
    k_pix_format_abgr32 = 14,    // A-B-G-R, one byte per color component
    k_pix_format_bgra32 = 15,    // B-G-R-A, native win32 BMP format
    k_pix_format_rgb48 = 16,     // R-G-B, 16 bits per color component
    k_pix_format_bgr48 = 17,     // B-G-R, native win32 BMP format.
    k_pix_format_rgba64 = 18,    // R-G-B-A, 16 bits byte per color component
    k_pix_format_argb64 = 19,    // A-R-G-B, native MAC format
    k_pix_format_abgr64 = 20,    // A-B-G-R, one byte per color component
    k_pix_format_bgra64 = 21,    // B-G-R-A, native win32 BMP format
};



}   //namespace lomse

#endif      //__LOMSE_PIXEL_FORMATS_H__
