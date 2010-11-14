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
//#include "lomse_user_command.h"
//#include "lomse_view.h"
//#include "lomse_controller.h"
//#include "lomse_mvc_builder.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// LibraryScope implementation
//---------------------------------------------------------------------------------------
LibraryScope::~LibraryScope()
{
    if (m_pLdpFactory)
        delete m_pLdpFactory;
    if (m_pFontStorage)
        delete m_pFontStorage;
}

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
// Injector implementation
//---------------------------------------------------------------------------------------
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

////---------------------------------------------------------------------------------------
//UserCommandExecuter* Injector::inject_UserCommandExecuter(Document* pDoc)
//{
//    return new UserCommandExecuter(pDoc);
//}
//
////---------------------------------------------------------------------------------------
//GraphicView* Injector::inject_EditView(LibraryScope& libraryScope, Document* pDoc,
//                                    UserCommandExecuter* pExec)
//{
//    Controller* pController = Injector::inject_Controller(libraryScope, pDoc, pExec);
//    return new GraphicView(pDoc, pController);
//}

////---------------------------------------------------------------------------------------
//Controller* Injector::inject_Controller(LibraryScope& libraryScope,
//                                        Document* pDoc, UserCommandExecuter* pExec)
//{
//    return new EditController(libraryScope, pDoc, pExec);
//}
//
////---------------------------------------------------------------------------------------
//MvcElement* Injector::inject_MvcElement(LibraryScope& libraryScope,
//                                        int viewType, Document* pDoc)
//{
//    UserCommandExecuter* pExec = Injector::inject_UserCommandExecuter(pDoc);
//    GraphicView* pView = Injector::inject_EditView(libraryScope, pDoc, pExec);
//    return new MvcElement(pDoc, pExec, pView);
//}


}  //namespace lomse
