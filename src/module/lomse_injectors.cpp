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

#include "lomse_injectors.h"

#include <sstream>
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_compiler.h"
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
{
    if (!m_pDoorway)
    {
        m_pNullDoorway = new LomseDoorway();
        m_pNullDoorway->init_library(LomseDoorway::k_platform_win32);
        m_pDoorway = m_pNullDoorway;
    }
}

//---------------------------------------------------------------------------------------
LibraryScope::~LibraryScope()
{
    delete m_pLdpFactory;
    delete m_pFontStorage;
    delete m_pNullDoorway;
//    delete m_pMusicGlyphs;
}

//---------------------------------------------------------------------------------------
LdpFactory* LibraryScope::ldp_factory()
{
    if (!m_pLdpFactory)
        m_pLdpFactory = new LdpFactory();
    return m_pLdpFactory;
}

//---------------------------------------------------------------------------------------
FontStorage* LibraryScope::font_storage()
{
    if (!m_pFontStorage)
        m_pFontStorage = new FontStorage();
    return m_pFontStorage;
}

//---------------------------------------------------------------------------------------
double LibraryScope::get_screen_ppi() const
{
    return m_pDoorway->get_screen_ppi();
}

////---------------------------------------------------------------------------------------
//MusicGlyphs* LibraryScope::music_glyphs()
//{
//    if (!m_pMusicGlyphs)
//        m_pMusicGlyphs = new MusicGlyphs();
//    return m_pMusicGlyphs;
//}



//=======================================================================================
// Injector implementation
//=======================================================================================
LdpParser* Injector::inject_LdpParser(LibraryScope& libraryScope,
                                      DocumentScope& documentScope)
{
    return new LdpParser(documentScope.default_reporter(),
                         libraryScope.ldp_factory());
}

//---------------------------------------------------------------------------------------
Analyser* Injector::inject_Analyser(LibraryScope& libraryScope,
                                    DocumentScope& documentScope)
{
    return new Analyser(documentScope.default_reporter(), libraryScope.ldp_factory());
}

//---------------------------------------------------------------------------------------
ModelBuilder* Injector::inject_ModelBuilder(DocumentScope& documentScope)
{
    return new ModelBuilder(documentScope.default_reporter());
}

//---------------------------------------------------------------------------------------
LdpCompiler* Injector::inject_LdpCompiler(LibraryScope& libraryScope,
                                          DocumentScope& documentScope)
{
    return new LdpCompiler(inject_LdpParser(libraryScope, documentScope),
                           inject_Analyser(libraryScope, documentScope),
                           inject_ModelBuilder(documentScope),
                           documentScope.id_assigner() );
}

//---------------------------------------------------------------------------------------
Document* Injector::inject_Document(LibraryScope& libraryScope)
{
    return new Document(libraryScope);
}

//---------------------------------------------------------------------------------------
ScreenDrawer* Injector::inject_ScreenDrawer(LibraryScope& libraryScope)
{
    return new ScreenDrawer(libraryScope);
}

////---------------------------------------------------------------------------------------
//UserCommandExecuter* Injector::inject_UserCommandExecuter(Document* pDoc)
//{
//    return new UserCommandExecuter(pDoc);
//}

//---------------------------------------------------------------------------------------
SimpleView* Injector::inject_SimpleView(LibraryScope& libraryScope, Document* pDoc)  //UserCommandExecuter* pExec)
{
    Interactor* pInteractor = Injector::inject_Interactor(libraryScope, pDoc);  //, pExec);
    ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
    return new SimpleView(libraryScope, pDoc, pInteractor, pDrawer);
}

//---------------------------------------------------------------------------------------
VerticalBookView* Injector::inject_VerticalBookView(LibraryScope& libraryScope,
                                                    Document* pDoc)  //UserCommandExecuter* pExec)
{
    Interactor* pInteractor = Injector::inject_Interactor(libraryScope, pDoc);  //, pExec);
    ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
    return new VerticalBookView(libraryScope, pDoc, pInteractor, pDrawer);
}

//---------------------------------------------------------------------------------------
HorizontalBookView* Injector::inject_HorizontalBookView(LibraryScope& libraryScope,
                                                        Document* pDoc)  //UserCommandExecuter* pExec)
{
    Interactor* pInteractor = Injector::inject_Interactor(libraryScope, pDoc);  //, pExec);
    ScreenDrawer* pDrawer = Injector::inject_ScreenDrawer(libraryScope);
    return new HorizontalBookView(libraryScope, pDoc, pInteractor, pDrawer);
}

//---------------------------------------------------------------------------------------
View* Injector::inject_View(LibraryScope& libraryScope, int viewType, Document* pDoc)
                            //UserCommandExecuter* pExec)
{
    //factory method

    switch(viewType)
    {
        case Injector::k_simple_view: 
            return Injector::inject_SimpleView(libraryScope, pDoc);
        
        case Injector::k_vertical_book_view: 
            return Injector::inject_VerticalBookView(libraryScope, pDoc);

        case Injector::k_horizontal_book_view: 
            return Injector::inject_HorizontalBookView(libraryScope, pDoc);

        default:
            throw "Injector::inject_View: invalid view type";
    }
}

//---------------------------------------------------------------------------------------
Interactor* Injector::inject_Interactor(LibraryScope& libraryScope,
                                        Document* pDoc) //, UserCommandExecuter* pExec)
{
    //factory method

    return new EditInteractor(libraryScope, pDoc);  //, pExec);
}

//---------------------------------------------------------------------------------------
Presenter* Injector::inject_Presenter(LibraryScope& libraryScope,
                                      int viewType, Document* pDoc)
{
    //UserCommandExecuter* pExec = Injector::inject_UserCommandExecuter(pDoc);
    View* pView = Injector::inject_View(libraryScope, viewType, pDoc); //, pExec);
    return new Presenter(pDoc, pView);  //, pExec);
}


}  //namespace lomse
