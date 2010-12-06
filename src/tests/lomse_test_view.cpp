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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_graphic_view.h"
#include "lomse_doorway.h"
#include "lomse_screen_drawer.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class MyDoorway : public LomseDoorway
{
protected:
    bool m_fUpdateWindowInvoked;
    bool m_fSetWindowTitleInvoked;
    std::string m_title;
    RenderingBuffer m_buffer;

public:
    MyDoorway() 
        : LomseDoorway()
    { 
        init_library(k_platform_win32);
    }
    virtual ~MyDoorway() {}

    void update_window() { m_fUpdateWindowInvoked = true; }
    void set_window_title(const std::string& title) {
        m_fSetWindowTitleInvoked = true; 
        m_title = title;
    }
    void force_redraw() {}
    RenderingBuffer& get_window_buffer() { return m_buffer; }

    bool set_window_title_invoked() { return m_fSetWindowTitleInvoked; }
    bool update_window_invoked() { return m_fUpdateWindowInvoked; }
    const std::string& get_title() { return m_title; }
    double get_screen_ppi() const { return 0.0; }
    void start_timer() {}
    double elapsed_time() const { return 0.0; }

};


//---------------------------------------------------------------------------------------
class GraphicViewTestFixture
{
public:
    std::string m_scores_path;

    GraphicViewTestFixture()     //SetUp fixture
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~GraphicViewTestFixture()    //TearDown fixture
    {
    }
};


//---------------------------------------------------------------------------------------
SUITE(GraphicViewTest)
{

    TEST_FIXTURE(GraphicViewTestFixture, EditView_CreatesGraphicModel)
    {
        MyDoorway platform;
        LibraryScope libraryScope(cout, &platform);
        Document doc(libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, Injector::k_vertical_book_view, &doc) );
        CHECK( pView->get_graphic_model() != NULL );
        delete pView;
    }

    //TEST_FIXTURE(GraphicViewTestFixture, EditView_UpdateWindow)
    //{
    //    MyDoorway platform;
    //    LibraryScope libraryScope(cout, &platform);
    //    Document doc(libraryScope);
    //    doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
    //        "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
    //    VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
    //        Injector::inject_View(libraryScope, Injector::k_vertical_book_view, &doc) );
    //    CHECK( m_platform.update_window_invoked() == false );
    //    pView->update_window();
    //    CHECK( m_platform.update_window_invoked() == true );
    //    delete pView;
    //}

    //TEST_FIXTURE(GraphicViewTestFixture, EditView_CreatesBitmap)
    //{
    //    MyDoorway platform;
    //    LibraryScope libraryScope(cout, &platform);
    //    Document doc(libraryScope);
    //    doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
    //        "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
    //    VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
    //        Injector::inject_View(libraryScope, Injector::k_vertical_book_view, &doc) );
    //    pView->on_draw();
    //    CHECK( ????????????????????????? );
    //    delete pView;
    //}

}

