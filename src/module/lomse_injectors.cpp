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

#include "lomse_injectors.h"
#include "lomse_version.h"

#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_ldp_compiler.h"
#include "lomse_lmd_parser.h"
#include "lomse_lmd_analyser.h"
#include "lomse_lmd_compiler.h"
#include "lomse_model_builder.h"
#include "lomse_document.h"
#include "lomse_font_storage.h"
//#include "lomse_glyphs.h"
//#include "lomse_user_command.h"
#include "lomse_graphic_view.h"
#include "lomse_interactor.h"
#include "lomse_presenter.h"
#include "lomse_doorway.h"
#include "lomse_screen_drawer.h"
#include "lomse_tasks.h"
#include "lomse_events.h"
#include "lomse_score_player.h"
#include "lomse_metronome.h"
#include "lomse_events_dispatcher.h"

#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// LibraryScope implementation
//=======================================================================================
LibraryScope::LibraryScope(ostream& reporter, LomseDoorway* pDoorway)
    : m_reporter(reporter)
    , m_pDoorway(pDoorway)
    , m_pNullDoorway(NULL)
    , m_pLdpFactory(NULL)       //lazzy instantiation. Singleton scope.
    , m_pFontStorage(NULL)      //lazzy instantiation. Singleton scope.
    , m_sFontsPath(LOMSE_FONTS_PATH)
    , m_pGlobalMetronome(NULL)
    , m_pDispatcher(NULL)
    , m_fJustifySystems(true)
    , m_fDumpColumnTables(false)
    , m_fDrawAnchors(false)
    , m_fReplaceLocalMetronome(false)
{
    if (!m_pDoorway)
    {
        m_pNullDoorway = LOMSE_NEW LomseDoorway();
        m_pNullDoorway->init_library(k_pix_format_rgba32, 96, false);
        m_pDoorway = m_pNullDoorway;
    }
}

//---------------------------------------------------------------------------------------
LibraryScope::~LibraryScope()
{
    delete m_pLdpFactory;
    delete m_pFontStorage;
    delete m_pNullDoorway;
    if (m_pDispatcher)
    {
        m_pDispatcher->stop_events_loop();
        delete m_pDispatcher;
    }
//    delete m_pMusicGlyphs;
}

//---------------------------------------------------------------------------------------
LdpFactory* LibraryScope::ldp_factory()
{
    if (!m_pLdpFactory)
        m_pLdpFactory = LOMSE_NEW LdpFactory();
    return m_pLdpFactory;
}

//---------------------------------------------------------------------------------------
FontStorage* LibraryScope::font_storage()
{
    if (!m_pFontStorage)
        m_pFontStorage = LOMSE_NEW FontStorage(this);
    return m_pFontStorage;
}

//---------------------------------------------------------------------------------------
EventsDispatcher* LibraryScope::get_events_dispatcher()
{
    if (!m_pDispatcher)
    {
        m_pDispatcher = LOMSE_NEW EventsDispatcher();
        m_pDispatcher->start_events_loop();
    }
    return m_pDispatcher;
}

//---------------------------------------------------------------------------------------
double LibraryScope::get_screen_ppi() const
{
    return m_pDoorway->get_screen_ppi();
}

//---------------------------------------------------------------------------------------
int LibraryScope::get_pixel_format() const
{
    return m_pDoorway->get_pixel_format();
}

//---------------------------------------------------------------------------------------
void LibraryScope::post_event(SpEventInfo pEvent)
{
    m_pDoorway->post_event(pEvent);
}

//---------------------------------------------------------------------------------------
void LibraryScope::post_request(Request* pRequest)
{
    m_pDoorway->post_request(pRequest);
}

//---------------------------------------------------------------------------------------
std::string LibraryScope::get_font(const string& name, bool fBold, bool fItalic)
{
    RequestFont request(name, fBold, fItalic);
    post_request(&request);
    return request.get_font_fullname();
}

//---------------------------------------------------------------------------------------
int LibraryScope::get_version_major() { return LOMSE_VERSION_MAJOR; }

//---------------------------------------------------------------------------------------
int LibraryScope::get_version_minor() { return LOMSE_VERSION_MINOR; }

//---------------------------------------------------------------------------------------
int LibraryScope::get_version_patch() { return LOMSE_VERSION_PATCH; }

//---------------------------------------------------------------------------------------
char LibraryScope::get_version_type() { return LOMSE_VERSION_TYPE; }

//---------------------------------------------------------------------------------------
long LibraryScope::get_revision() { return LOMSE_REVISION; }

//---------------------------------------------------------------------------------------
string LibraryScope::get_version_string()
{
    //i.e. "0.7 (rev.48)", "0.7 beta 48 (rev.56)", "1.0 (rev.75)", "1.0.2 (rev.77)"

    stringstream s;
    s << get_version_major() << "." << get_version_minor();
    if (get_version_type() != ' ')
    {
        if (get_version_type() == 'a')
            s << " alpha ";
        else
            s << " beta ";
        s << get_version_patch();
    }
    else
    {
        if (get_version_patch() > 0)
            s << "." << get_version_patch();
    }

    s << " (rev." << get_revision() << ")";
    return s.str();
}



//=======================================================================================
// Injector implementation
//=======================================================================================
LdpParser* Injector::inject_LdpParser(LibraryScope& libraryScope,
                                      DocumentScope& documentScope)
{
    return LOMSE_NEW LdpParser(documentScope.default_reporter(),
                         libraryScope.ldp_factory());
}

//---------------------------------------------------------------------------------------
LdpAnalyser* Injector::inject_LdpAnalyser(LibraryScope& libraryScope, Document* pDoc)
{
    return LOMSE_NEW LdpAnalyser(pDoc->get_scope().default_reporter(), libraryScope, pDoc);
}

//---------------------------------------------------------------------------------------
LdpCompiler* Injector::inject_LdpCompiler(LibraryScope& libraryScope,
                                          Document* pDoc)
{
    return LOMSE_NEW LdpCompiler(inject_LdpParser(libraryScope, pDoc->get_scope()),
                           inject_LdpAnalyser(libraryScope, pDoc),
                           inject_ModelBuilder(pDoc->get_scope()),
                           pDoc->get_scope().id_assigner(),
                           pDoc );
}

//---------------------------------------------------------------------------------------
LmdParser* Injector::inject_LmdParser(LibraryScope& libraryScope,
                                      DocumentScope& documentScope)
{
    return LOMSE_NEW LmdParser(documentScope.default_reporter());
}

//---------------------------------------------------------------------------------------
LmdAnalyser* Injector::inject_LmdAnalyser(LibraryScope& libraryScope, Document* pDoc,
                                          LmdParser* pParser)
{
    return LOMSE_NEW LmdAnalyser(pDoc->get_scope().default_reporter(),
                                 libraryScope, pDoc, pParser);
}

//---------------------------------------------------------------------------------------
LmdCompiler* Injector::inject_LmdCompiler(LibraryScope& libraryScope,
                                          Document* pDoc)
{
    LmdParser* pParser = Injector::inject_LmdParser(libraryScope, pDoc->get_scope());
    return LOMSE_NEW LmdCompiler(pParser,
                                 inject_LmdAnalyser(libraryScope, pDoc, pParser),
                                 inject_ModelBuilder(pDoc->get_scope()),
                                 pDoc->get_scope().id_assigner(),
                                 pDoc );
}

//---------------------------------------------------------------------------------------
ModelBuilder* Injector::inject_ModelBuilder(DocumentScope& documentScope)
{
    return LOMSE_NEW ModelBuilder();
}

//---------------------------------------------------------------------------------------
Document* Injector::inject_Document(LibraryScope& libraryScope, ostream& reporter)
{
    return LOMSE_NEW Document(libraryScope, reporter);
}

//---------------------------------------------------------------------------------------
ScreenDrawer* Injector::inject_ScreenDrawer(LibraryScope& libraryScope)
{
    return LOMSE_NEW ScreenDrawer(libraryScope);
}

////---------------------------------------------------------------------------------------
//UserCommandExecuter* Injector::inject_UserCommandExecuter(Document* pDoc)
//{
//    return LOMSE_NEW UserCommandExecuter(pDoc);
//}

//---------------------------------------------------------------------------------------
SimpleView* Injector::inject_SimpleView(LibraryScope& libraryScope, Document* pDoc)  //UserCommandExecuter* pExec)
{
    return static_cast<SimpleView*>(
                        inject_View(libraryScope,
                                    ViewFactory::k_view_simple,
                                    pDoc)
                       );
}

//---------------------------------------------------------------------------------------
VerticalBookView* Injector::inject_VerticalBookView(LibraryScope& libraryScope,
                                                    Document* pDoc)  //UserCommandExecuter* pExec)
{
    return static_cast<VerticalBookView*>(
                        inject_View(libraryScope,
                                    ViewFactory::k_view_vertical_book,
                                    pDoc)
                       );
}

//---------------------------------------------------------------------------------------
HorizontalBookView* Injector::inject_HorizontalBookView(LibraryScope& libraryScope,
                                                        Document* pDoc)  //UserCommandExecuter* pExec)
{
    return static_cast<HorizontalBookView*>(
                        inject_View(libraryScope,
                                    ViewFactory::k_view_horizontal_book,
                                    pDoc)
                       );
}

//---------------------------------------------------------------------------------------
View* Injector::inject_View(LibraryScope& libraryScope, int viewType, Document* pDoc)
                            //UserCommandExecuter* pExec)
{
    ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
    return ViewFactory::create_view(libraryScope, viewType, pDrawer);
}

//---------------------------------------------------------------------------------------
Interactor* Injector::inject_Interactor(LibraryScope& libraryScope,
                                        Document* pDoc, View* pView) //, UserCommandExecuter* pExec)
{
    //factory method

    return LOMSE_NEW EditInteractor(libraryScope, pDoc, pView);  //, pExec);
}

//---------------------------------------------------------------------------------------
Presenter* Injector::inject_Presenter(LibraryScope& libraryScope,
                                      int viewType, Document* pDoc)
{
    //UserCommandExecuter* pExec = Injector::inject_UserCommandExecuter(pDoc);
    View* pView = Injector::inject_View(libraryScope, viewType, pDoc); //, pExec);
    Interactor* pInteractor = Injector::inject_Interactor(libraryScope, pDoc, pView);
    pView->set_interactor(pInteractor);
    return LOMSE_NEW Presenter(pDoc, pInteractor);  //, pExec);
}

//---------------------------------------------------------------------------------------
Task* Injector::inject_Task(int taskType, Interactor* pIntor)
{
    return TaskFactory::create_task(taskType, pIntor);
}

//---------------------------------------------------------------------------------------
ScorePlayer* Injector::inject_ScorePlayer(LibraryScope& libraryScope,
                                          MidiServerBase* pSoundServer)
{
    return LOMSE_NEW ScorePlayer(libraryScope, pSoundServer);
}



}  //namespace lomse
