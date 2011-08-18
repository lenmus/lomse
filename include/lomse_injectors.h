//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
#include "lomse_build_options.h"

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
class Task;
class EventInfo;
class Request;

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

    //options
    bool m_fJustifySystems;
    bool m_fDumpColumnTables;


public:
    LibraryScope(ostream& reporter=cout, LomseDoorway* pDoorway=NULL);
    ~LibraryScope();

    inline ostream& default_reporter() { return m_reporter; }
    inline LomseDoorway* platform_interface() { return m_pDoorway; }
    LdpFactory* ldp_factory();
    FontStorage* font_storage();

    //callbacks
    void post_event(EventInfo* pEvent);
    void post_request(Request* pRequest);
    std::string get_font(const string& name, bool fBold, bool fItalic);

    double get_screen_ppi() const;
    int get_pixel_format() const;
    //MusicGlyphs* music_glyphs();

    //global options, mainly for debug
    inline void set_justify_systems(bool value) { m_fJustifySystems = value; }
    inline bool justify_systems() { return m_fJustifySystems; }
    inline void set_dump_column_tables(bool value) { m_fDumpColumnTables = value; }
    inline bool dump_column_tables() { return m_fDumpColumnTables; }
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
    static Analyser* inject_Analyser(LibraryScope& libraryScope, Document* pDoc);
    static ModelBuilder* inject_ModelBuilder(DocumentScope& documentScope);
    static LdpCompiler* inject_LdpCompiler(LibraryScope& libraryScope,
                                           Document* pDoc);
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
                                         Document* pDoc, View* pView);   //, UserCommandExecuter* pExec);
    static Presenter* inject_Presenter(LibraryScope& libraryScope,
                                       int viewType, Document* pDoc);
    static Task* inject_Task(int taskType, Interactor* pIntor);

};



}   //namespace lomse

#endif      //__LOMSE_INJECTORS_H__
