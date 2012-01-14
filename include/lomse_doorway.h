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

#ifndef __LOMSE_DOORWAY_H__
#define __LOMSE_DOORWAY_H__

//---------------------------------------------------------------------------------------
// This file MUST NOT include any system dependent .h files such as "windows.h" or
// "X11.h", so the library do not depend on the platform.
// All platform dependent code MUST be on the user application


#include "lomse_build_options.h"
#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_trans_viewport.h"
#include "lomse_tasks.h"
#include "lomse_agg_types.h"
#include "lomse_reader.h"
#include "lomse_pixel_formats.h"
#include "lomse_events.h"
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
class Presenter;


//---------------------------------------------------------------------------------------
struct PlatformInfo
{
    EPixelFormat    pixel_format;     //see enum EPixelFormat
    bool            flip_y;     //true if you want to have the Y-axis flipped vertically
    double          screen_ppi; //screen resolution in pixels per inch
};


//---------------------------------------------------------------------------------------
// This class represent the gate for lomse <-> platform/user app. communication
// For the lomse library, it is the way to access platform dependent methods
// For the user application it is the way to set up lomse and access basic methods
//
typedef void (*pt2NotifyFunction)(void*, SpEventInfo);
typedef void (*pt2RequestFunction)(void*, Request*);

class LomseDoorway
{
protected:
    LibraryScope*       m_pLibraryScope;
    PlatformInfo        m_platform;
    pt2NotifyFunction   m_pFunc_notify;
    pt2RequestFunction  m_pFunc_request;
    void*               m_pObj_notify;
    void*               m_pObj_request;

public:
    LomseDoorway();
    virtual ~LomseDoorway();

    enum EViewType { k_view_simple=0, k_view_vertical_book, k_view_horizontal_book, };

    void init_library(int pixel_format, int ppi, bool reverse_y_axis,
                      ostream& reporter=cout);
    void set_notify_callback(void* pThis, void (*pt2Func)(void*, SpEventInfo));
    void set_request_callback(void* pThis, void (*pt2Func)(void*, Request*));

    inline LibraryScope* get_library_scope() { return m_pLibraryScope; }
    inline void post_event(SpEventInfo pEvent) { m_pFunc_notify(m_pObj_notify, pEvent); }
    inline void post_request(Request* pRequest) { m_pFunc_request(m_pObj_request, pRequest); }

    //access to platform info
    inline double get_screen_ppi() { return m_platform.screen_ppi; }
    inline int get_pixel_format() { return m_platform.pixel_format; }

    //common operations
    Presenter* new_document(int viewType);
    Presenter* new_document(int viewType, const string& ldpSource);
    Presenter* open_document(int viewType, const string& filename);
    Presenter* open_document(int viewType, LdpReader& reader);

    static void null_notify_function(void* pObj, SpEventInfo event);
    static void null_request_function(void* pObj, Request* event);
};


}   //namespace lomse

#endif      //__LOMSE_DOORWAY_H__
