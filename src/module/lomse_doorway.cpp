//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#define LOMSE_INTERNAL_API
#include "lomse_doorway.h"

#include "lomse_injectors.h"
#include "lomse_presenter.h"
#include "lomse_import_options.h"
#include "lomse_graphic_view.h"
#include "lomse_bitmap_drawer.h"

#include "lomse_reader.h"
#include "lomse_events.h"

#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_trans_viewport.h"
#include "lomse_agg_types.h"
using namespace agg;

#include <sstream>
using namespace std;


namespace lomse
{

//=======================================================================================
//platform independent methods
//=======================================================================================
LomseDoorway::LomseDoorway(std::ostream* logStream, std::ostream* forensicLogStream)
    : m_pLibraryScope(nullptr)
    , m_platform{ EPixelFormat::k_pix_format_undefined, false, 72.0 }
    , m_pFunc_notify(null_notify_function)
    , m_pFunc_request(null_request_function)
    , m_pObj_notify(nullptr)
    , m_pObj_request(nullptr)
{
    glogger.init(logStream, forensicLogStream);

    //initialize lomse library with some default values, to facilitate using SVG
    //as these values doesn't matter in SVG
    init_library(k_pix_format_rgba32, 96);
}

//---------------------------------------------------------------------------------------
LomseDoorway::~LomseDoorway()
{
    delete m_pLibraryScope;
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::new_document(int viewType, Drawer* screenDrawer,
                                      Drawer* printDrawer)
{
    if (screenDrawer == nullptr)
        screenDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    if (printDrawer == nullptr)
        printDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    PresenterBuilder builder(*m_pLibraryScope);
    return builder.new_document(viewType, screenDrawer, printDrawer);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::new_document(int viewType, const string& source, int format,
                                      ostream& reporter, Drawer* screenDrawer,
                                      Drawer* printDrawer)

{
    if (screenDrawer == nullptr)
        screenDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    if (printDrawer == nullptr)
        printDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    PresenterBuilder builder(*m_pLibraryScope);
    return builder.new_document(viewType, screenDrawer, printDrawer, source,
                                reporter, format);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, const string& filename,
                                       ostream& reporter, Drawer* screenDrawer,
                                       Drawer* printDrawer)
{
    if (screenDrawer == nullptr)
        screenDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    if (printDrawer == nullptr)
        printDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, filename, screenDrawer, printDrawer, reporter);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, LdpReader& reader,
                                       ostream& reporter, Drawer* screenDrawer,
                                       Drawer* printDrawer)
{
    if (screenDrawer == nullptr)
        screenDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    if (printDrawer == nullptr)
        printDrawer = Injector::inject_BitmapDrawer(*m_pLibraryScope);

    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, reader, screenDrawer, printDrawer, reporter);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::init_library(int pixel_format, int ppi, bool reverse_y_axis,
                               ostream& reporter)
{
    //DEPRECATED method Jan/2021

    m_platform.pixel_format = EPixelFormat(pixel_format);
    m_platform.flip_y = reverse_y_axis;
    m_platform.screen_ppi = float(ppi);

    delete m_pLibraryScope;
    m_pLibraryScope = LOMSE_NEW LibraryScope(reporter, this);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::init_library(int pixel_format, int ppi, ostream& reporter)
{
    m_platform.pixel_format = EPixelFormat(pixel_format);
    m_platform.screen_ppi = float(ppi);

    delete m_pLibraryScope;
    m_pLibraryScope = LOMSE_NEW LibraryScope(reporter, this);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::set_notify_callback(void* pThis,
                                       void (*pt2Func)(void* pObj, SpEventInfo event))
{
    m_pFunc_notify = pt2Func;
    m_pObj_notify = pThis;
}

//---------------------------------------------------------------------------------------
void LomseDoorway::set_request_callback(void* pThis,
                                        void (*pt2Func)(void* pObj, Request* event))
{
    m_pFunc_request = pt2Func;
    m_pObj_request = pThis;
}

//---------------------------------------------------------------------------------------
void LomseDoorway::set_default_fonts_path(const string& fontsPath)
{
    m_pLibraryScope->set_default_fonts_path(fontsPath);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::null_notify_function(void* UNUSED(pObj), SpEventInfo UNUSED(event))
{
    //This is just a mock method to avoid crashes when using the libary without
    //initializing it
}

//---------------------------------------------------------------------------------------
void LomseDoorway::null_request_function(void* UNUSED(pObj), Request* UNUSED(pRequest))
{
    //This is just a mock method to avoid crashes when using the libary without
    //initializing it
}

//---------------------------------------------------------------------------------------
int LomseDoorway::get_version_major()
{
    return m_pLibraryScope->get_version_major();
}

//---------------------------------------------------------------------------------------
int LomseDoorway::get_version_minor()
{
    return m_pLibraryScope->get_version_minor();
}

//---------------------------------------------------------------------------------------
int LomseDoorway::get_version_patch()
{
    return m_pLibraryScope->get_version_patch();
}

//---------------------------------------------------------------------------------------
string LomseDoorway::get_version_string()
{
    return m_pLibraryScope->get_version_string();
}

//---------------------------------------------------------------------------------------
string LomseDoorway::get_version_long_string()
{
    return m_pLibraryScope->get_version_long_string();
}

//---------------------------------------------------------------------------------------
string LomseDoorway::get_build_date()
{
    return m_pLibraryScope->get_build_date();
}

#if (LOMSE_ENABLE_THREADS == 1)
//---------------------------------------------------------------------------------------
ScorePlayer* LomseDoorway::create_score_player(MidiServerBase* pSoundServer)
{
    return Injector::inject_ScorePlayer(*m_pLibraryScope, pSoundServer);
}
#endif

//---------------------------------------------------------------------------------------
void LomseDoorway::set_global_metronome_and_replace_local(Metronome* pMtr)
{
    m_pLibraryScope->set_global_metronome_and_replace_local(pMtr);
}

//---------------------------------------------------------------------------------------
MusicXmlOptions* LomseDoorway::get_musicxml_options()
{
    return m_pLibraryScope->get_musicxml_options();
}

//---------------------------------------------------------------------------------------
void LomseDoorway::post_event(SpEventInfo pEvent)
{
    m_pFunc_notify(m_pObj_notify, pEvent);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::post_request(Request* pRequest)
{
    m_pFunc_request(m_pObj_request, pRequest);
}


}   //namespace lomse
