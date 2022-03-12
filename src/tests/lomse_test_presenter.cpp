//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#define LOMSE_INTERNAL_API
#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_presenter.h"
#include "private/lomse_document_p.h"
#include "lomse_ldp_compiler.h"
//#include "lomse_user_command.h"
#include "lomse_view.h"
#include "lomse_graphic_view.h"
#include "lomse_document_cursor.h"
#include "lomse_interactor.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class PresenterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    PresenterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~PresenterTestFixture()    //TearDown fixture
    {
    }
};

SUITE(PresenterTest)
{

    TEST_FIXTURE(PresenterTestFixture, PresenterBuilder_NewDocument)
    {
        PresenterBuilder builder(m_libraryScope);

        Presenter* pPresenter = builder.new_document(k_view_simple);
        CHECK( pPresenter != nullptr );
        Document* pDoc = pPresenter->get_document_raw_ptr();
        ImoDocument* pImoDoc = pDoc->get_im_root();
        CHECK( pImoDoc->get_content_item(0) == nullptr );
        CHECK( pPresenter->get_num_interactors() == 1 );
        Interactor* pIntor = pPresenter->get_interactor_raw_ptr(0);
        CHECK( pIntor != nullptr );
        CHECK( pIntor->get_view() != nullptr );
        SimpleView* pView = dynamic_cast<SimpleView*>( pIntor->get_view() );
        CHECK( pView != nullptr );

        delete pPresenter;
    }

    TEST_FIXTURE(PresenterTestFixture, get_weak_ptr_to_document)
    {
        PresenterBuilder builder(m_libraryScope);
        Presenter* pPresenter = builder.new_document(k_view_simple);

        WpDocument wpDoc = pPresenter->get_document_weak_ptr();
        CHECK( wpDoc.expired() == false );

        delete pPresenter;
        CHECK( wpDoc.expired() == true );
    }

    TEST_FIXTURE(PresenterTestFixture, get_weak_ptr_to_interactor)
    {
        PresenterBuilder builder(m_libraryScope);
        Presenter* pPresenter = builder.new_document(k_view_simple);

        WpInteractor wpIntor = pPresenter->get_interactor(0);
        CHECK( wpIntor.expired() == false );

        delete pPresenter;
        CHECK( wpIntor.expired() == true );
    }

    //TEST_FIXTURE(PresenterTestFixture, PresenterBuilderCreatesViewCursorAtEnd)
    //{
    //    PresenterBuilder builder(m_libraryScope);
    //    Presenter* pPresenter = builder.new_document(k_view_simple);
    //    SimpleView* pView = dynamic_cast<SimpleView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( *cursor == nullptr );
    //    delete pPresenter;
    //}

//    TEST_FIXTURE(PresenterTestFixture, PresenterBuilderCreatesViewCursorAtStart)
//    {
//        PresenterBuilder builder(m_libraryScope);
//        Presenter* pPresenter = builder.new_document(k_view_simple,
//                  "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))");
//        WpInteractor wpIntor = pPresenter->get_interactor(0);
//        if (SpInteractor sp = wpIntor.lock())
//        {
//            SimpleView* pView = dynamic_cast<SimpleView*>( sp->get_view() );
//            DocCursor& cursor = pView->get_cursor();
//            //cout << (*cursor)->to_string() << endl;
//            CHECK( *cursor != nullptr );
//            //CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//        }
//        else
//            CHECK( false );
//        delete pPresenter;
//    }

    //TEST_FIXTURE(PresenterTestFixture, PresenterBuilder_OpenDocument)
    //{
    //    PresenterBuilder builder(m_libraryScope);

    //    Presenter* pPresenter = builder.open_document(k_view_simple,
    //                            m_scores_path + "00011-empty-fill-page.lms");
    //    CHECK( pPresenter != nullptr );
    //    Document* pDoc = pPresenter->get_document();
    //    //cout << pDoc->to_string() << endl;
    //    CHECK( pDoc->to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.Truncate 1) (instrument (musicData)))))" );
    //    CHECK( pPresenter->get_command_executer() != nullptr );

    //    delete pPresenter;
    //}



//// PresentersCollection tests --------------------------------------------------
//
//
//    TEST_FIXTURE(PresenterTestFixture, PresentersCollection_CloseDocumentByIndex)
//    {
//        PresenterBuilder builder(m_libraryScope);
//        Presenter* pPresenter = builder.new_document(k_view_simple);
//        PresentersCollection elements;
//        elements.add(pPresenter);
//        CHECK( elements.get_num_documents() == 1 );
//
//        elements.close_document(0);
//        CHECK( elements.get_num_documents() == 0 );
//    }
//
//    TEST_FIXTURE(PresenterTestFixture, PresentersCollection_CloseDocumentByPointer)
//    {
//        PresenterBuilder builder(m_libraryScope);
//        Presenter* pPresenter = builder.new_document(k_view_simple);
//        PresentersCollection elements;
//        elements.add(pPresenter);
//        CHECK( elements.get_num_documents() == 1 );
//        Document* pDoc = pPresenter->get_document_raw_ptr();
//
//        elements.close_document(pDoc);
//        CHECK( elements.get_num_documents() == 0 );
//    }
//
////    TEST_FIXTURE(PresenterTestFixture, PresentersCollection_AddView)
////    {
////        PresentersCollection elements;
////        PresenterBuilder builder(m_libraryScope);
////        Document* pDoc = builder.new_document(k_view_simple);
////        View* pView = LOMSE_NEW SimpleView(pDoc);
////        CHECK( elements.get_num_views(pDoc) == 0 );
////        elements.add_view(pDoc, pView);
////        CHECK( elements.get_num_views(pDoc) == 1 );
////        elements.close_document(pDoc);
////        delete pView;
////    }
////



// MvcModel tests --------------------------------------------------


    //TEST_FIXTURE(PresenterTestFixture, MvcModel_ViewIsNotifiedWhenModifications)
    //{
    //    PresenterBuilder builder(m_libraryScope);
    //    Presenter* pPresenter = builder.new_document(k_view_simple,
    //              "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))");
    //    SimpleView* pView = dynamic_cast<SimpleView*>( pPresenter->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( *cursor != nullptr );
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
    //    delete pPresenter;
    //}

}

