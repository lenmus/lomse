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
#include "lomse_presenter.h"
#include "lomse_document.h"
#include "lomse_view.h"
#include "lomse_interactor.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
static bool fNotified = false;
static void my_callback_function(Notification* event)
{
    fNotified = true;
}

//---------------------------------------------------------------------------------------
class InteractorTestFixture
{
public:
    LibraryScope m_libraryScope;

    InteractorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
    }

    ~InteractorTestFixture()    //TearDown fixture
    {
    }
};


SUITE(InteractorTest)
{

    TEST_FIXTURE(InteractorTestFixture, Interactor_Create)
    {
        Document* pDoc = Injector::inject_Document(m_libraryScope);
        pDoc->create_empty();
        EditInteractor interactor(m_libraryScope, pDoc);  //, pExec);

    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope, 
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    View* pView = pPresenter->get_view(0);
    //    Interactor* pInteractor = pView->get_interactor();
    //    CHECK( pInteractor );
    //    EditInteractor* pEC = dynamic_cast<EditInteractor*>( pInteractor );
    //    CHECK( pEC );

        delete pDoc;
    }

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(n d4 e)(barline simple))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q)
    //    ++cursor;   //(n d4 e)
    //    CHECK( is_equal_time( cursor.time(), 64.0f ));

    //    pPresenter->insert_rest(pView, "(r q)");

    //    CHECK( fNotified == true );
    //    CHECK( (*cursor)->to_string() == "(n d4 e)" );
    //    CHECK( cursor.is_pointing_object() );
    //    CHECK( is_equal_time( cursor.time(), 128.0f ));
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string_with_ids() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q) (r q) (n d4 e) (barline simple))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_AtEnd)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q)
    //    ++cursor;   //at end

    //    pPresenter->insert_rest(pView, "(r q)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == NULL );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q) (r q))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_DifferentVoice_TimeZero)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n a3 q v1))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n a3 q v1)

    //    pPresenter->insert_rest(pView, "(r q v2)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == NULL );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n a3 q v1) (goBack start) (r q v2))))" );

    //    delete pPresenter;
    //}

    //TEST_FIXTURE(InteractorTestFixture, Presenter_InsertRest_DifferentVoice_OtherTime)
    //{
    //    fNotified = false;
    //    Document* pDoc = Injector::inject_Document(m_libraryScope);
    //    pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q v1)(n a3 q v1))))))" );
    //    Presenter* pPresenter = Injector::inject_Presenter(m_libraryScope,
    //                                                   PresenterBuilder::k_edit_view, pDoc);
    //    pPresenter->set_callback( &my_callback_function );
    //    EditView* pView = dynamic_cast<EditView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    cursor.enter_element();     //(clef G)
    //    ++cursor;   //(key e)
    //    ++cursor;   //(n c4 q v1)
    //    ++cursor;   //(n a3 q v1)

    //    pPresenter->insert_rest(pView, "(r q v2)");

    //    CHECK( fNotified == true );
    //    CHECK( *cursor == NULL );
    //    cursor.start_of_content();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q v1) (n a3 q v1) (goBack start) (goFwd 64) (r q v2))))" );

    //    delete pPresenter;
    //}

}

