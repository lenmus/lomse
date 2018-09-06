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
#include "lomse_injectors.h"

#include "lomse_version.h"
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_ldp_compiler.h"
#include "lomse_xml_parser.h"
#include "lomse_lmd_analyser.h"
#include "lomse_lmd_compiler.h"
#include "lomse_mxl_analyser.h"
#include "lomse_mxl_compiler.h"
#include "lomse_mnx_analyser.h"
#include "lomse_mnx_compiler.h"
#include "lomse_model_builder.h"
#include "lomse_document.h"
#include "lomse_font_storage.h"
#include "lomse_graphic_view.h"
#include "lomse_interactor.h"
#include "lomse_presenter.h"
#include "lomse_doorway.h"
#include "lomse_screen_drawer.h"
#include "lomse_tasks.h"
#include "lomse_events.h"
#include "lomse_score_player.h"
#include "lomse_metronome.h"
#include "lomse_id_assigner.h"
#include "lomse_document_cursor.h"
#include "lomse_command.h"
#include "lomse_caret_positioner.h"
#include "lomse_glyphs.h"
#include "lomse_engraving_options.h"

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
    , m_pNullDoorway(nullptr)
    , m_pLdpFactory(nullptr)       //lazzy instantiation. Singleton scope.
    , m_pFontStorage(nullptr)      //lazzy instantiation. Singleton scope.
    , m_pGlobalMetronome(nullptr)
    , m_pDispatcher(nullptr)
    , m_sMusicFontFile("Bravura.otf")
    , m_sMusicFontName("Bravura")
    , m_sMusicFontPath(LOMSE_FONTS_PATH)
    , m_sFontsPath(LOMSE_FONTS_PATH)
    , m_pMusicGlyphs(nullptr)      //lazzy instantiation. Singleton scope.
    , m_fReplaceLocalMetronome(false)
    , m_importOptions()
    , m_fJustifySystems(true)
    , m_fDumpColumnTables(false)
    , m_fDrawAnchorObjects(false)
    , m_fDrawAnchorLines(false)
    , m_fShowShapeBounds(false)
    , m_fUnitTests(false)
    , m_traceLinesBreaker(k_trace_breaks_off)
    , m_fUseDbgValues(false)
    , m_spacingOptForce(1.0f)
    , m_spacingAlpha(0.666666667f)
    , m_spacingDmin(16.0f)
    , m_spacingSmin(LOMSE_MIN_SPACE)
    , m_renderSpacingOpts(k_render_opt_breaker_optimal)
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
    delete m_pMusicGlyphs;
    if (m_pDispatcher)
    {
        m_pDispatcher->stop_events_loop();
        delete m_pDispatcher;
    }
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
MusicGlyphs* LibraryScope::get_glyphs_table()
{
    if (!m_pMusicGlyphs)
        m_pMusicGlyphs = LOMSE_NEW MusicGlyphs(this);
    return m_pMusicGlyphs;
}

//---------------------------------------------------------------------------------------
void LibraryScope::set_music_font(const string& fontFile, const string& fontName,
                                  const string& path)
{
    m_sMusicFontName = fontName;
    m_sMusicFontFile = fontFile;
    m_sMusicFontPath = path;
    //TODO: ensure that font path ends in path separator ("\" or "/" depending on platform)

    get_glyphs_table()->update();
}

//---------------------------------------------------------------------------------------
const string& LibraryScope::get_music_font_path()
{
    if (!m_sMusicFontPath.empty())
        return m_sMusicFontPath;
    else
        return m_sFontsPath;
}

//---------------------------------------------------------------------------------------
bool LibraryScope::is_music_font_smufl_compliant()
{
    return m_sMusicFontName != "lmbasic2.ttf";
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
    LOMSE_LOG_DEBUG(Logger::k_events, string(""));
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
string LibraryScope::get_version_string()
{
    return LOMSE_VERSION;
}

//---------------------------------------------------------------------------------------
string LibraryScope::get_version_long_string()
{
    return LOMSE_VERSION_LONG;
}

//---------------------------------------------------------------------------------------
string LibraryScope::get_build_date()
{
    //__DATE__ string: contains eleven characters and looks like "Feb 12 1996".
    // If the day of the month is less than 10, it is padded with a space on the
    // left, i.e. "Oct  8 2013"
    //__TIME__ : the time at which the preprocessor is being run. The string
    // contains eight characters and looks like "23:59:01"

    string date(__DATE__);
    return date.substr(4,2) + "-" + date.substr(0,3) + "-" + date.substr(7,4)
           + " " + string(__TIME__);
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
                                 pDoc );
}

//---------------------------------------------------------------------------------------
XmlParser* Injector::inject_XmlParser(LibraryScope& UNUSED(libraryScope),
                                      DocumentScope& documentScope)
{
    return LOMSE_NEW XmlParser(documentScope.default_reporter());
}

//---------------------------------------------------------------------------------------
LmdAnalyser* Injector::inject_LmdAnalyser(LibraryScope& libraryScope, Document* pDoc,
                                          XmlParser* pParser)
{
    return LOMSE_NEW LmdAnalyser(pDoc->get_scope().default_reporter(),
                                 libraryScope, pDoc, pParser);
}

//---------------------------------------------------------------------------------------
LmdCompiler* Injector::inject_LmdCompiler(LibraryScope& libraryScope,
                                          Document* pDoc)
{
    XmlParser* pParser = Injector::inject_XmlParser(libraryScope, pDoc->get_scope());
    return LOMSE_NEW LmdCompiler(pParser,
                                 inject_LmdAnalyser(libraryScope, pDoc, pParser),
                                 inject_ModelBuilder(pDoc->get_scope()),
                                 pDoc );
}

//---------------------------------------------------------------------------------------
MxlAnalyser* Injector::inject_MxlAnalyser(LibraryScope& libraryScope, Document* pDoc,
                                          XmlParser* pParser)
{
    return LOMSE_NEW MxlAnalyser(pDoc->get_scope().default_reporter(),
                                 libraryScope, pDoc, pParser);
}

//---------------------------------------------------------------------------------------
MxlCompiler* Injector::inject_MxlCompiler(LibraryScope& libraryScope,
                                          Document* pDoc)
{
    XmlParser* pParser = Injector::inject_XmlParser(libraryScope, pDoc->get_scope());
    return LOMSE_NEW MxlCompiler(pParser,
                                 inject_MxlAnalyser(libraryScope, pDoc, pParser),
                                 inject_ModelBuilder(pDoc->get_scope()),
                                 pDoc );
}

//---------------------------------------------------------------------------------------
MnxAnalyser* Injector::inject_MnxAnalyser(LibraryScope& libraryScope, Document* pDoc,
                                          XmlParser* pParser)
{
    return LOMSE_NEW MnxAnalyser(pDoc->get_scope().default_reporter(),
                                 libraryScope, pDoc, pParser);
}

//---------------------------------------------------------------------------------------
MnxCompiler* Injector::inject_MnxCompiler(LibraryScope& libraryScope,
                                          Document* pDoc)
{
    XmlParser* pParser = Injector::inject_XmlParser(libraryScope, pDoc->get_scope());
    return LOMSE_NEW MnxCompiler(pParser,
                                 inject_MnxAnalyser(libraryScope, pDoc, pParser),
                                 inject_ModelBuilder(pDoc->get_scope()),
                                 pDoc );
}

//---------------------------------------------------------------------------------------
ModelBuilder* Injector::inject_ModelBuilder(DocumentScope& UNUSED(documentScope))
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
SimpleView* Injector::inject_SimpleView(LibraryScope& libraryScope, Document* pDoc)
{
    return static_cast<SimpleView*>(
                        inject_View(libraryScope, k_view_simple, pDoc) );
}

//---------------------------------------------------------------------------------------
VerticalBookView* Injector::inject_VerticalBookView(LibraryScope& libraryScope,
                                                    Document* pDoc)
{
    return static_cast<VerticalBookView*>(
                        inject_View(libraryScope, k_view_vertical_book, pDoc) );
}

//---------------------------------------------------------------------------------------
HorizontalBookView* Injector::inject_HorizontalBookView(LibraryScope& libraryScope,
                                                        Document* pDoc)
{
    return static_cast<HorizontalBookView*>(
                        inject_View(libraryScope, k_view_horizontal_book, pDoc) );
}

//---------------------------------------------------------------------------------------
SingleSystemView* Injector::inject_SingleSystemView(LibraryScope& libraryScope,
                                                    Document* pDoc)
{
    return static_cast<SingleSystemView*>(
                        inject_View(libraryScope, k_view_single_system, pDoc) );
}

//---------------------------------------------------------------------------------------
View* Injector::inject_View(LibraryScope& libraryScope, int viewType,
                            Document* UNUSED(pDoc))
{
    ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
    return ViewFactory::create_view(libraryScope, viewType, pDrawer);
}

//---------------------------------------------------------------------------------------
Interactor* Injector::inject_Interactor(LibraryScope& libraryScope,
                                        WpDocument wpDoc, View* pView,
                                        DocCommandExecuter* pExec)
{
    //factory method

    return LOMSE_NEW Interactor(libraryScope, wpDoc, pView, pExec);
}

//---------------------------------------------------------------------------------------
Presenter* Injector::inject_Presenter(LibraryScope& libraryScope,
                                      int viewType, Document* pDoc)
{
    View* pView = Injector::inject_View(libraryScope, viewType, pDoc);
    DocCommandExecuter* pExec = Injector::inject_DocCommandExecuter(pDoc);
    SpDocument spDoc(pDoc);
    WpDocument wpDoc(spDoc);
    Interactor* pInteractor = Injector::inject_Interactor(libraryScope, wpDoc, pView, pExec);
    pView->set_interactor(pInteractor);
    return LOMSE_NEW Presenter(spDoc, pInteractor, pExec);
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

//---------------------------------------------------------------------------------------
DocCursor* Injector::inject_DocCursor(Document* pDoc)
{
    return LOMSE_NEW DocCursor(pDoc);
}

//---------------------------------------------------------------------------------------
SelectionSet* Injector::inject_SelectionSet(Document* pDoc)
{
    return LOMSE_NEW SelectionSet(pDoc);
}

//---------------------------------------------------------------------------------------
DocCommandExecuter* Injector::inject_DocCommandExecuter(Document* pDoc)
{
    return LOMSE_NEW DocCommandExecuter(pDoc);
}


//=======================================================================================
// DocumentScope implementation
//=======================================================================================
DocumentScope::DocumentScope(ostream& reporter)
    : m_reporter(reporter)
{
    m_idAssigner = LOMSE_NEW IdAssigner();
}

//---------------------------------------------------------------------------------------
DocumentScope::~DocumentScope()
{
    delete m_idAssigner;
}


}  //namespace lomse
