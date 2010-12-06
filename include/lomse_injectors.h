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

#include "lomse_ldp_factory.h"
#include "lomse_id_assigner.h"
#include <iostream>
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
class View;
class SimpleView;
class VerticalBookView;
class HorizontalBookView;
class Interactor;
class Presenter;
class LomseDoorway;
class Drawer;
class ScreenDrawer;


//---------------------------------------------------------------------------------------
class LOMSE_EXPORT LibraryScope
{
protected:
    ostream& m_reporter;
    LomseDoorway* m_pDoorway;
    LomseDoorway* m_pNullDoorway;
    LdpFactory* m_pLdpFactory;
    FontStorage* m_pFontStorage;
    //MusicGlyphs* m_pMusicGlyphs;

public:
    LibraryScope(ostream& reporter=cout, LomseDoorway* pDoorway=NULL);
    ~LibraryScope();

    inline ostream& default_reporter() { return m_reporter; }
    inline LomseDoorway* platform_interface() { return m_pDoorway; }
    LdpFactory* ldp_factory();
    FontStorage* font_storage();

    double get_screen_ppi() const;
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

    enum { k_simple_view=0, k_vertical_book_view, k_horizontal_book_view, };

    static LdpParser* inject_LdpParser(LibraryScope& libraryScope,
                                       DocumentScope& documentScope);
    static Analyser* inject_Analyser(LibraryScope& libraryScope,
                                     DocumentScope& documentScope);
    static ModelBuilder* inject_ModelBuilder(DocumentScope& documentScope);
    static LdpCompiler* inject_LdpCompiler(LibraryScope& libraryScope,
                                           DocumentScope& documentScope);
    static Document* inject_Document(LibraryScope& libraryScope);
    static ScreenDrawer* inject_ScreenDrawer(LibraryScope& libraryScope);
//    static UserCommandExecuter* inject_UserCommandExecuter(Document* pDoc);
    static View* inject_View(LibraryScope& libraryScope, int viewType, Document* pDoc);  //UserCommandExecuter* pExec)
    static SimpleView* inject_SimpleView(LibraryScope& libraryScope, Document* pDoc);  //UserCommandExecuter* pExec)
    static VerticalBookView* inject_VerticalBookView(LibraryScope& libraryScope,
                                                     Document* pDoc);  //UserCommandExecuter* pExec)
    static HorizontalBookView* inject_HorizontalBookView(LibraryScope& libraryScope,
                                                         Document* pDoc);  //UserCommandExecuter* pExec)
    static Interactor* inject_Interactor(LibraryScope& libraryScope,
                                         Document* pDoc);   //, UserCommandExecuter* pExec);
    static Presenter* inject_Presenter(LibraryScope& libraryScope,
                                       int viewType, Document* pDoc);

};



}   //namespace lomse

#endif      //__LOMSE_INJECTORS_H__
