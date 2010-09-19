//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifdef _LM_DEBUG_

#include <UnitTest++.h>
#include <sstream>

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_mvc_builder.h"
#include "lomse_document.h"
#include "lomse_compiler.h"
#include "lomse_user_command.h"
#include "lomse_view.h"
#include "lomse_document_cursor.h"
#include "lomse_controller.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class MvcBuilderTestFixture
{
public:

    MvcBuilderTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
    }

    ~MvcBuilderTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
};

SUITE(MvcModelTest)
{

    //TEST_FIXTURE(MvcBuilderTestFixture, MvcBuilderCreatesMvcElement)
    //{
    //    Document* pDoc = new Document(*m_pLibraryScope);
    //    pDoc->create_empty();
    //    UserCommandExecuter* pExec = new UserCommandExecuter(pDoc);
    //    Controller* pController = new EditController(*m_pLibraryScope, pDoc, pExec);
    //    delete pDoc;
    //    delete pExec;
    //    delete pController;
    //}

    TEST_FIXTURE(MvcBuilderTestFixture, MvcBuilderCreatesMvcElement)
    {
        MvcBuilder builder(*m_pLibraryScope);
        MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view);
        CHECK( pMvc != NULL );
        Document* pDoc = pMvc->get_document();
        CHECK( pDoc->to_string() == "(lenmusdoc (vers 0.0) (content ))" );
        delete pMvc;
    }

    TEST_FIXTURE(MvcBuilderTestFixture, MvcBuilderCreatesViewController)
    {
        MvcBuilder builder(*m_pLibraryScope);
        MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view);
        CHECK( pMvc->get_num_views() == 1 );
        View* pView = pMvc->get_view(0);
        CHECK( pView != NULL );
        CHECK( pView->get_controller() != NULL );
        delete pMvc;
    }

    TEST_FIXTURE(MvcBuilderTestFixture, MvcBuilderCreatesViewCursorAtEnd)
    {
        MvcBuilder builder(*m_pLibraryScope);
        MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view);
        EditView* pView = dynamic_cast<EditView*>( pMvc->get_view(0) );
        DocCursor& cursor = pView->get_cursor();
        //cout << (*cursor)->to_string() << endl;
        CHECK( *cursor == NULL );
        delete pMvc;
    }

    TEST_FIXTURE(MvcBuilderTestFixture, MvcBuilderCreatesViewCursorAtStart)
    {
        MvcBuilder builder(*m_pLibraryScope);
        MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view,
                  "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))");
        EditView* pView = dynamic_cast<EditView*>( pMvc->get_view(0) );
        DocCursor& cursor = pView->get_cursor();
        //cout << (*cursor)->to_string() << endl;
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
        delete pMvc;
    }

    TEST_FIXTURE(MvcBuilderTestFixture, MvcBuilderOpenDocument)
    {
        MvcBuilder builder(*m_pLibraryScope);
        MvcElement* pMvc = builder.open_document(MvcBuilder::k_edit_view,
                                "../../test-scores/00011-empty-fill-page.lms");
        CHECK( pMvc != NULL );
        Document* pDoc = pMvc->get_document();
        //cout << pDoc->to_string() << endl;
        CHECK( pDoc->to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData )))))" );
        CHECK( pMvc->get_command_executer() != NULL );
        delete pMvc;
    }



// MvcCollection tests --------------------------------------------------


    TEST_FIXTURE(MvcBuilderTestFixture, MvcCollectionCloseDocumentByIndex)
    {
        MvcBuilder builder(*m_pLibraryScope);
        MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view);
        MvcCollection elements;
        elements.add(pMvc);
        CHECK( elements.get_num_documents() == 1 );
        elements.close_document(0);
        CHECK( elements.get_num_documents() == 0 );
    }

    TEST_FIXTURE(MvcBuilderTestFixture, MvcCollectionCloseDocumentByPointer)
    {
        MvcBuilder builder(*m_pLibraryScope);
        MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view);
        MvcCollection elements;
        elements.add(pMvc);
        CHECK( elements.get_num_documents() == 1 );
        Document* pDoc = pMvc->get_document();
        elements.close_document(pDoc);
        CHECK( elements.get_num_documents() == 0 );
    }

//    TEST_FIXTURE(MvcBuilderTestFixture, MvcCollectionAddView)
//    {
//        MvcCollection elements;
//        MvcBuilder builder(*m_pLibraryScope);
//        Document* pDoc = builder.new_document(MvcBuilder::k_edit_view);
//        View* pView = new EditView(pDoc);
//        CHECK( elements.get_num_views(pDoc) == 0 );
//        elements.add_view(pDoc, pView);
//        CHECK( elements.get_num_views(pDoc) == 1 );
//        elements.close_document(pDoc);
//        delete pView;
//    }
//



// MvcModel tests --------------------------------------------------


    //TEST_FIXTURE(MvcBuilderTestFixture, MvcModel_ViewIsNotifiedWhenModifications)
    //{
    //    MvcBuilder builder(*m_pLibraryScope);
    //    MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view,
    //              "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))");
    //    EditView* pView = dynamic_cast<EditView*>( pMvc->get_view(0) );
    //    DocCursor& cursor = pView->get_cursor();
    //    //cout << (*cursor)->to_string() << endl;
    //    CHECK( *cursor != NULL );
    //    CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
    //    delete pMvc;
    //}

}

#endif  // _LM_DEBUG_

