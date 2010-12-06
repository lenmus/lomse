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
// All platform dependent code MUST be on the user application


#include "lomse_build_options.h"
#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_trans_viewport.h"
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

//-------------------------------------------------------------input_flag_e
// Mouse and keyboard flags. They can be different on different platforms
// and the ways they are obtained are also different. But in any case
// the system dependent flags should be mapped into these ones. The meaning
// of that is as follows. For example, if kbd_ctrl is set it means that the 
// ctrl key is pressed and being held at the moment. They are also used in 
// the overridden methods such as on_mouse_move(), on_mouse_button_down(),
// on_mouse_button_dbl_click(), on_mouse_button_up(), on_key(). 
// In the method on_mouse_button_up() the mouse flags have different
// meaning. They mean that the respective button is being released, but
// the meaning of the keyboard flags remains the same.
// There's absolut minimal set of flags is used because they'll be most
// probably supported on different platforms. Even the mouse_right flag
// is restricted because Mac's mice have only one button, but AFAIK
// it can be simulated with holding a special key on the keydoard.
enum input_flag_e
{
    mouse_left  = 1,
    mouse_right = 2,
    kbd_shift   = 4,
    kbd_ctrl    = 8
};

//--------------------------------------------------------------key_code_e
// Keyboard codes. There's also a restricted set of codes that are most 
// probably supported on different platforms. Any platform dependent codes
// should be converted into these ones. There're only those codes are
// defined that cannot be represented as printable ASCII-characters. 
// All printable ASCII-set can be used in a regular C/C++ manner: 
// ' ', 'A', '0' '+' and so on.
// Since the class is used for creating very simple demo-applications
// we don't need very rich possibilities here, just basic ones. 
// Actually the numeric key codes are taken from the SDL library, so,
// the implementation of the SDL support does not require any mapping.
enum key_code_e
{
    // ASCII set. Should be supported everywhere
    key_backspace      = 8,
    key_tab            = 9,
    key_clear          = 12,
    key_return         = 13,
    key_pause          = 19,
    key_escape         = 27,

    // Keypad 
    key_delete         = 127,
    key_kp0            = 256,
    key_kp1            = 257,
    key_kp2            = 258,
    key_kp3            = 259,
    key_kp4            = 260,
    key_kp5            = 261,
    key_kp6            = 262,
    key_kp7            = 263,
    key_kp8            = 264,
    key_kp9            = 265,
    key_kp_period      = 266,
    key_kp_divide      = 267,
    key_kp_multiply    = 268,
    key_kp_minus       = 269,
    key_kp_plus        = 270,
    key_kp_enter       = 271,
    key_kp_equals      = 272,

    // Arrow-keys and stuff
    key_up             = 273,
    key_down           = 274,
    key_right          = 275,
    key_left           = 276,
    key_insert         = 277,
    key_home           = 278,
    key_end            = 279,
    key_page_up        = 280,
    key_page_down      = 281,

    // Functional keys. You'd better avoid using
    // f11...f15 in your applications if you want 
    // the applications to be portable
    key_f1             = 282,
    key_f2             = 283,
    key_f3             = 284,
    key_f4             = 285,
    key_f5             = 286,
    key_f6             = 287,
    key_f7             = 288,
    key_f8             = 289,
    key_f9             = 290,
    key_f10            = 291,
    key_f11            = 292,
    key_f12            = 293,
    key_f13            = 294,
    key_f14            = 295,
    key_f15            = 296,

    // The possibility of using these keys is 
    // very restricted. Actually it's guaranteed 
    // only in win32_api and win32_sdl implementations
    key_numlock        = 300,
    key_capslock       = 301,
    key_scrollock      = 302,

    // Phew!
    end_of_key_codes
};


//
// Default values are as follows:
//
//    Win32 - pix_format_bgra32     // B-G-R-A, native win32 BMP format
//    Mac   - pix_format_argb32,    // A-R-G-B, native MAC format
//    X11   - pix_format_rgba32,    // R-G-B-A, one byte per color
//
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

    //call backs for application provided methods
    void (*m_pFunc_update_window)();
    void (*m_pFunc_force_redraw)();
    void (*m_pFunc_start_timer)();
    double (*m_pFunc_elapsed_time)();

public:
    LomseDoorway();
    ~LomseDoorway();

    enum EPlatformType { k_platform_win32, k_platform_x11, };
    enum EViewType { k_horizontal_book_view=0, k_vertical_book_view, k_simple_view, };

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

    //providing platform services to lomse
    void update_window() { m_pFunc_update_window(); }
    void force_redraw() { m_pFunc_force_redraw(); }
    void start_timer() { m_pFunc_start_timer(); }
    //millisecods since last start_timer() invocation
    double elapsed_time() const { return m_pFunc_elapsed_time(); }

};


}   //namespace lomse

#endif      //__LOMSE_PLATFORM_H__
