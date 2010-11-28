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

#ifndef __LOMSE_INJECTORS_H__
#define __LOMSE_INJECTORS_H__

#include <iostream>
#include "lomse_ldp_factory.h"
#include "lomse_id_assigner.h"

using namespace std;

namespace lomse
{

//forward declarations
class LdpParser;
class Analyser;
class ModelBuilder;
class LdpCompiler;
class Document;
class LdpFactory;
class FontStorage;
class MusicGlyphs;
//class UserCommandExecuter;
//class GraphicView;
//class Controller;
//class MvcElement;


//---------------------------------------------------------------------------------------
class LibraryScope
{
protected:
    ostream& m_reporter;
    LdpFactory* m_pLdpFactory;
    FontStorage* m_pFontStorage;
    double m_ppi;
    //MusicGlyphs* m_pMusicGlyphs;

public:
    LibraryScope(ostream& reporter=cout, double ppi=96.0)
        : m_reporter(reporter)
        , m_pLdpFactory(NULL)       //lazzy instantiation. Singleton scope.
        , m_pFontStorage(NULL)      //lazzy instantiation. Singleton scope.
        , m_ppi(ppi)
    {
    }

    ~LibraryScope();

    inline ostream& default_reporter() { return m_reporter; }
    LdpFactory* ldp_factory();
    FontStorage* font_storage();
    inline double pixels_per_inch() { return m_ppi; }
    //MusicGlyphs* music_glyphs();

};

//---------------------------------------------------------------------------------------
class DocumentScope
{
public:
    DocumentScope(ostream& reporter=cout) : m_reporter(reporter) {}
    ~DocumentScope() {}

    ostream& default_reporter() { return m_reporter; }
    IdAssigner* id_assigner() { return &m_idAssigner; }

protected:
    ostream& m_reporter;
    IdAssigner m_idAssigner;

};

//---------------------------------------------------------------------------------------
class Injector
{
public:
    Injector() {}
    ~Injector() {}

    static LdpParser* inject_LdpParser(LibraryScope& libraryScope,
                                       DocumentScope& documentScope);
    static Analyser* inject_Analyser(LibraryScope& libraryScope,
                                     DocumentScope& documentScope);
    static ModelBuilder* inject_ModelBuilder(DocumentScope& documentScope);
    static LdpCompiler* inject_LdpCompiler(LibraryScope& libraryScope,
                                           DocumentScope& documentScope);
    static Document* inject_Document(LibraryScope& libraryScope);
//    static UserCommandExecuter* inject_UserCommandExecuter(Document* pDoc);
//    static GraphicView* inject_EditView(LibraryScope& libraryScope, Document* pDoc,
//                                     UserCommandExecuter* pExec);
//    static Controller* inject_Controller(LibraryScope& libraryScope,
//                                         Document* pDoc, UserCommandExecuter* pExec);
//    static MvcElement* inject_MvcElement(LibraryScope& libraryScope,
//                                         int viewType, Document* pDoc);

};



}   //namespace lomse

#endif      //__LOMSE_INJECTORS_H__
