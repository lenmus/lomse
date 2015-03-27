//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
//#include "lomse_tasks.h"
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
class ScorePlayer;
class MidiServerBase;
class Metronome;


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

    //library initialization and configuration
    void init_library(int pixel_format, int ppi, bool reverse_y_axis,
                      ostream& reporter=cout);
    void set_notify_callback(void* pThis, void (*pt2Func)(void*, SpEventInfo));
    void set_request_callback(void* pThis, void (*pt2Func)(void*, Request*));
    void set_default_fonts_path(const string& fontsPath);
        //configuration of ScorePlayerCtrol
    void set_global_metronome_and_replace_local(Metronome* pMtr);
    //void set_global_metronome_and_duplicate_local(Metronome* pMtr);

    //access to global objects
    inline LibraryScope* get_library_scope() { return m_pLibraryScope; }

    //communication with user application
    inline void post_event(SpEventInfo pEvent) { m_pFunc_notify(m_pObj_notify, pEvent); }
    inline void post_request(Request* pRequest) { m_pFunc_request(m_pObj_request, pRequest); }

    //access to platform info
    inline double get_screen_ppi() { return m_platform.screen_ppi; }
    inline int get_pixel_format() { return m_platform.pixel_format; }

    //common operations on documents
    Presenter* new_document(int viewType);
    Presenter* new_document(int viewType, const string& source, int format,
                            ostream& reporter = cout);
    Presenter* open_document(int viewType, const string& filename,
                             ostream& reporter = cout);
    Presenter* open_document(int viewType, LdpReader& reader,
                             ostream& reporter = cout);
    ScorePlayer* create_score_player(MidiServerBase* pSoundServer);


    static void null_notify_function(void* pObj, SpEventInfo event);
    static void null_request_function(void* pObj, Request* event);

    //library info
    string get_version_string();
    int get_version_major();
    int get_version_minor();
    int get_version_patch();
    char get_version_type();
    long get_revision() { return LOMSE_REVISION; }

protected:
    void clear_forensic_log();

};

}   //namespace lomse

#endif      //__LOMSE_DOORWAY_H__
