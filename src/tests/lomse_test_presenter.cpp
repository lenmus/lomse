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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_presenter.h"
#include "lomse_document.h"
#include "lomse_compiler.h"
//#include "lomse_user_command.h"
#include "lomse_view.h"
#include "lomse_graphic_view.h"
#include "lomse_document_cursor.h"
#include "lomse_interactor.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class PresenterBuilderTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    PresenterBuilderTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~PresenterBuilderTestFixture()    //TearDown fixture
    {
    }
};

SUITE(PresenterTest)
{

//    //TEST_FIXTURE(PresenterBuilderTestFixture, PresenterBuilderCreatesPresenter)
//    //{
//    //    Document* pDoc = new Document(m_libraryScope);
//    //    pDoc->create_empty();
//    //    UserCommandExecuter* pExec = new UserCommandExecuter(pDoc);
//    //    Interactor* pInteractor = new EditInteractor(m_libraryScope, pDoc, pExec);
//    //    delete pDoc;
//    //    delete pExec;
//    //    delete pInteractor;
//    //}

    TEST_FIXTURE(PresenterBuilderTestFixture, PresenterBuilder_NewDocument)
    {
        PresenterBuilder builder(m_libraryScope);

        Presenter* pPresenter = builder.new_document(ViewFactory::k_view_simple);
        CHECK( pPresenter != NULL );
        Document* pDoc = pPresenter->get_document();
        CHECK( pDoc->to_string() == "(lenmusdoc (vers 0.0) (content))" );
        CHECK( pPresenter->get_num_interactors() == 1 );
        Interactor* pIntor = pPresenter->get_interactor(0);
        CHECK( pIntor != NULL );
        CHECK( pIntor->get_view() != NULL );
        SimpleView* pView = dynamic_cast<SimpleView*>( pIntor->get_view() );
        CHECK( pView != NULL );

        delete pPresenter;
    }

    //TEST_FIXTURE(PresenterBuilderTestFixture, PresenterBuilderCreatesViewCursorAtEnd)
    //{
    //    PresenterBuilder builder(m_libraryScope);
    //    Presenter* pPresenter = builder.new_document(ViewFactory::k_view_simple);
    //    SimpleView* pView = dynamic_cast<SimpleView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( *cursor == NULL );
    //    delete pPresenter;
    //}

    //TEST_FIXTURE(PresenterBuilderTestFixture, PresenterBuilderCreatesViewCursorAtStart)
    //{
    //    PresenterBuilder builder(m_libraryScope);
    //    Presenter* pPresenter = builder.new_document(ViewFactory::k_view_simple,
    //              "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))");
    //    SimpleView* pView = dynamic_cast<SimpleView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( *cursor != NULL );
    //    //CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
    //    delete pPresenter;
    //}

    //TEST_FIXTURE(PresenterBuilderTestFixture, PresenterBuilder_OpenDocument)
    //{
    //    PresenterBuilder builder(m_libraryScope);

    //    Presenter* pPresenter = builder.open_document(ViewFactory::k_view_simple,
    //                            m_scores_path + "00011-empty-fill-page.lms");
    //    CHECK( pPresenter != NULL );
    //    Document* pDoc = pPresenter->get_document();
    //    //cout << pDoc->to_string() << endl;
    //    CHECK( pDoc->to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData )))))" );
    //    CHECK( pPresenter->get_command_executer() != NULL );

    //    delete pPresenter;
    //}



// PresentersCollection tests --------------------------------------------------


    TEST_FIXTURE(PresenterBuilderTestFixture, PresentersCollection_CloseDocumentByIndex)
    {
        PresenterBuilder builder(m_libraryScope);
        Presenter* pPresenter = builder.new_document(ViewFactory::k_view_simple);
        PresentersCollection elements;
        elements.add(pPresenter);
        CHECK( elements.get_num_documents() == 1 );

        elements.close_document(0);
        CHECK( elements.get_num_documents() == 0 );
    }

    TEST_FIXTURE(PresenterBuilderTestFixture, PresentersCollection_CloseDocumentByPointer)
    {
        PresenterBuilder builder(m_libraryScope);
        Presenter* pPresenter = builder.new_document(ViewFactory::k_view_simple);
        PresentersCollection elements;
        elements.add(pPresenter);
        CHECK( elements.get_num_documents() == 1 );
        Document* pDoc = pPresenter->get_document();

        elements.close_document(pDoc);
        CHECK( elements.get_num_documents() == 0 );
    }

//    TEST_FIXTURE(PresenterBuilderTestFixture, PresentersCollection_AddView)
//    {
//        PresentersCollection elements;
//        PresenterBuilder builder(m_libraryScope);
//        Document* pDoc = builder.new_document(ViewFactory::k_view_simple);
//        View* pView = new SimpleView(pDoc);
//        CHECK( elements.get_num_views(pDoc) == 0 );
//        elements.add_view(pDoc, pView);
//        CHECK( elements.get_num_views(pDoc) == 1 );
//        elements.close_document(pDoc);
//        delete pView;
//    }
//



// MvcModel tests --------------------------------------------------


    //TEST_FIXTURE(PresenterBuilderTestFixture, MvcModel_ViewIsNotifiedWhenModifications)
    //{
    //    PresenterBuilder builder(m_libraryScope);
    //    Presenter* pPresenter = builder.new_document(ViewFactory::k_view_simple,
    //              "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))");
    //    SimpleView* pView = dynamic_cast<SimpleView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( *cursor != NULL );
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
    //    delete pPresenter;
    //}

}

