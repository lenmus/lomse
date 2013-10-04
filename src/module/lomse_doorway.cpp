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

#include "lomse_doorway.h"

#include "lomse_injectors.h"
#include "lomse_presenter.h"

#include <sstream>
using namespace std;


namespace lomse
{

//=======================================================================================
//platform independent methods
//=======================================================================================
LomseDoorway::LomseDoorway()
    : m_pLibraryScope(NULL)
    , m_pFunc_notify(null_notify_function)
    , m_pFunc_request(null_request_function)
{
}

//---------------------------------------------------------------------------------------
LomseDoorway::~LomseDoorway()
{
    delete m_pLibraryScope;
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
void LomseDoorway::null_notify_function(void* pObj, SpEventInfo event)
{
    //This is just a mock method to avoid crashes when using the libary without
    //initializing it
}

//---------------------------------------------------------------------------------------
void LomseDoorway::null_request_function(void* pObj, Request* pRequest)
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
char LomseDoorway::get_version_type()
{
    return m_pLibraryScope->get_version_type();
}

//---------------------------------------------------------------------------------------
string LomseDoorway::get_version_string()
{
    return m_pLibraryScope->get_version_string();
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


}   //namespace lomse
