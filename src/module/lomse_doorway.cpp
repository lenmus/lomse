//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#define LOMSE_INTERNAL_API
#include "lomse_doorway.h"

#include "lomse_injectors.h"
#include "lomse_presenter.h"
#include "lomse_import_options.h"
#include "lomse_graphic_view.h"

#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_trans_viewport.h"
//#include "lomse_tasks.h"
#include "lomse_agg_types.h"
using namespace agg;

#include <sstream>
using namespace std;


namespace lomse
{

//=======================================================================================
//platform independent methods
//=======================================================================================
LomseDoorway::LomseDoorway()
    : m_pLibraryScope(nullptr)
    , m_platform{ EPixelFormat::k_pix_format_undefined, false, 72.0 }
    , m_pFunc_notify(null_notify_function)
    , m_pFunc_request(null_request_function)
    , m_pObj_notify(nullptr)
    , m_pObj_request(nullptr)
{
    clear_forensic_log();
}

//---------------------------------------------------------------------------------------
LomseDoorway::~LomseDoorway()
{
    delete m_pLibraryScope;
}

//---------------------------------------------------------------------------------------
void LomseDoorway::clear_forensic_log()
{
    ofstream logger;
    logger.open("forensic_log.txt");
    logger.close();
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::new_document(int viewType)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.new_document(viewType);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::new_document(int viewType, const string& source, int format,
                                      ostream& reporter)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.new_document(viewType, source, reporter, format);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, const string& filename,
                                       ostream& reporter)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, filename, reporter);
}

//---------------------------------------------------------------------------------------
Presenter* LomseDoorway::open_document(int viewType, LdpReader& reader,
                                       ostream& reporter)
{
    PresenterBuilder builder(*m_pLibraryScope);
    return builder.open_document(viewType, reader, reporter);
}

//---------------------------------------------------------------------------------------
void LomseDoorway::init_library(int pixel_format, int ppi, bool reverse_y_axis,
                               ostream& reporter)
{
    m_platform.pixel_format = EPixelFormat(pixel_format);
    m_platform.flip_y = reverse_y_axis;
    m_platform.screen_ppi = float(ppi);

    m_pLibraryScope = LOMSE_NEW LibraryScope(reporter, this);
//    m_pLibraryScope->get_threads_poll();        //force to create the threads
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

//---------------------------------------------------------------------------------------
ScorePlayer* LomseDoorway::create_score_player(MidiServerBase* pSoundServer)
{
    return Injector::inject_ScorePlayer(*m_pLibraryScope, pSoundServer);
}

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


}   //namespace lomse
