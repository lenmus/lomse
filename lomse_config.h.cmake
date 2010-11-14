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

#ifndef __LOMSE_CONFIG_H__
#define __LOMSE_CONFIG_H__

//==================================================================
// Template configuration file.
// Variables are replaced by CMake settings
//==================================================================


//---------------------------------------------------------------------------------------
// path for test scores. 
//---------------------------------------------------------------------------------------
#define LOMSE_TEST_SCORES_PATH      @LOMSE_TEST_SCORES_PATH@


//---------------------------------------------------------------------------------------
// library version, i.e.: 2.3
//---------------------------------------------------------------------------------------
#define LOMSE_VERSION_MAJOR     @LOMSE_VERSION_MAJOR@
#define LOMSE_VERSION_MINOR     @LOMSE_VERSION_MINOR@


//---------------------------------------------------------------------------------------
// Specifies the pixel format to use for the rendering buffer. 
//
// Some OSs can render several formats and it doesn't obligatory mean the necessity of 
// software conversion. For example, win32 API can natively display  gray8, 15-bit RGB, 
// 24-bit BGR, and 32-bit BGRA formats. Value for LOMSE_BITMAP_FORMAT must be one of 
// the following:
//
//    k_pix_format_bw,        // 1 bit per color B/W
//    k_pix_format_gray8,     // Simple 256 level grayscale
//    k_pix_format_gray16,    // Simple 65535 level grayscale
//    k_pix_format_rgb555,    // 15 bit rgb. Depends on the byte ordering!
//    k_pix_format_rgb565,    // 16 bit rgb. Depends on the byte ordering!
//    k_pix_format_rgbAAA,    // 30 bit rgb. Depends on the byte ordering!
//    k_pix_format_rgbBBA,    // 32 bit rgb. Depends on the byte ordering!
//    k_pix_format_bgrAAA,    // 30 bit bgr. Depends on the byte ordering!
//    k_pix_format_bgrABB,    // 32 bit bgr. Depends on the byte ordering!
//    k_pix_format_rgb24,     // R-G-B, one byte per color component
//    k_pix_format_bgr24,     // B-G-R, native win32 BMP format.
//    k_pix_format_rgba32,    // R-G-B-A, one byte per color component
//    k_pix_format_argb32,    // A-R-G-B, native MAC format
//    k_pix_format_abgr32,    // A-B-G-R, one byte per color component
//    k_pix_format_bgra32,    // B-G-R-A, native win32 BMP format
//    k_pix_format_rgb48,     // R-G-B, 16 bits per color component
//    k_pix_format_bgr48,     // B-G-R, native win32 BMP format.
//    k_pix_format_rgba64,    // R-G-B-A, 16 bits byte per color component
//    k_pix_format_argb64,    // A-R-G-B, native MAC format
//    k_pix_format_abgr64,    // A-B-G-R, one byte per color component
//    k_pix_format_bgra64,    // B-G-R-A, native win32 BMP format
//
// Default values are as follows:
//
//    Win32 - k_pix_format_bgra32     // B-G-R-A, native win32 BMP format
//    Mac   - k_pix_format_argb32,    // A-R-G-B, native MAC format
//    X11   - k_pix_format_rgba32,    // R-G-B-A, one byte per color
//
//---------------------------------------------------------------------------------------
#define LOMSE_BITMAP_FORMAT     @LOMSE_BITMAP_FORMAT@
    

        

#endif  // __LOMSE_CONFIG_H__

