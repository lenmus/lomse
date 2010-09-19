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
#include "lomse_view.h"
#include "lomse_controller.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

static bool fNotified = false;
static void my_callback_function(Notification* event)
{
    fNotified = true;
}

class ControllerTestFixture
{
public:

    ControllerTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
    }

    ~ControllerTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
};


SUITE(ControllerTest)
{

    TEST_FIXTURE(ControllerTestFixture, EditController_Create)
    {
        Document* pDoc = Injector::inject_Document(*m_pLibraryScope);
        pDoc->create_empty();
        MvcElement* pMvc = Injector::inject_MvcElement(*m_pLibraryScope, 
                                                       MvcBuilder::k_edit_view, pDoc);
        View* pView = pMvc->get_view(0);
        Controller* pController = pView->get_controller();
        CHECK( pController );
        EditController* pEC = dynamic_cast<EditController*>( pController );
        CHECK( pEC );

        delete pMvc;
    }

    TEST_FIXTURE(ControllerTestFixture, MvcElement_InsertRest)
    {
        fNotified = false;
        Document* pDoc = Injector::inject_Document(*m_pLibraryScope);
        pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(n d4 e)(barline simple))))))" );
        MvcElement* pMvc = Injector::inject_MvcElement(*m_pLibraryScope,
                                                       MvcBuilder::k_edit_view, pDoc);
        pMvc->set_callback( &my_callback_function );
        EditView* pView = dynamic_cast<EditView*>( pMvc->get_view(0) );
        DocCursor& cursor = pView->get_cursor();
        cursor.enter_element();     //(clef G)
        ++cursor;   //(key e)
        ++cursor;   //(n c4 q)
        ++cursor;   //(n d4 e)
        CHECK( is_equal_time( cursor.time(), 64.0f ));

        pMvc->insert_rest(pView, "(r q)");

        CHECK( fNotified == true );
        CHECK( (*cursor)->to_string() == "(n d4 e)" );
        CHECK( cursor.is_pointing_object() );
        CHECK( is_equal_time( cursor.time(), 128.0f ));
        cursor.start_of_content();
        //cout << (*cursor)->to_string_with_ids() << endl;
        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q) (r q) (n d4 e) (barline simple))))" );

        delete pMvc;
    }

    TEST_FIXTURE(ControllerTestFixture, MvcElement_InsertRest_AtEnd)
    {
        fNotified = false;
        Document* pDoc = Injector::inject_Document(*m_pLibraryScope);
        pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q))))))" );
        MvcElement* pMvc = Injector::inject_MvcElement(*m_pLibraryScope,
                                                       MvcBuilder::k_edit_view, pDoc);
        pMvc->set_callback( &my_callback_function );
        EditView* pView = dynamic_cast<EditView*>( pMvc->get_view(0) );
        DocCursor& cursor = pView->get_cursor();
        cursor.enter_element();     //(clef G)
        ++cursor;   //(key e)
        ++cursor;   //(n c4 q)
        ++cursor;   //at end

        pMvc->insert_rest(pView, "(r q)");

        CHECK( fNotified == true );
        CHECK( *cursor == NULL );
        cursor.start_of_content();
        //cout << (*cursor)->to_string() << endl;
        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q) (r q))))" );

        delete pMvc;
    }

    TEST_FIXTURE(ControllerTestFixture, MvcElement_InsertRest_DifferentVoice_TimeZero)
    {
        fNotified = false;
        Document* pDoc = Injector::inject_Document(*m_pLibraryScope);
        pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n a3 q v1))))))" );
        MvcElement* pMvc = Injector::inject_MvcElement(*m_pLibraryScope,
                                                       MvcBuilder::k_edit_view, pDoc);
        pMvc->set_callback( &my_callback_function );
        EditView* pView = dynamic_cast<EditView*>( pMvc->get_view(0) );
        DocCursor& cursor = pView->get_cursor();
        cursor.enter_element();     //(clef G)
        ++cursor;   //(key e)
        ++cursor;   //(n a3 q v1)

        pMvc->insert_rest(pView, "(r q v2)");

        CHECK( fNotified == true );
        CHECK( *cursor == NULL );
        cursor.start_of_content();
        //cout << (*cursor)->to_string() << endl;
        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n a3 q v1) (goBack start) (r q v2))))" );

        delete pMvc;
    }

    TEST_FIXTURE(ControllerTestFixture, MvcElement_InsertRest_DifferentVoice_OtherTime)
    {
        fNotified = false;
        Document* pDoc = Injector::inject_Document(*m_pLibraryScope);
        pDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q v1)(n a3 q v1))))))" );
        MvcElement* pMvc = Injector::inject_MvcElement(*m_pLibraryScope,
                                                       MvcBuilder::k_edit_view, pDoc);
        pMvc->set_callback( &my_callback_function );
        EditView* pView = dynamic_cast<EditView*>( pMvc->get_view(0) );
        DocCursor& cursor = pView->get_cursor();
        cursor.enter_element();     //(clef G)
        ++cursor;   //(key e)
        ++cursor;   //(n c4 q v1)
        ++cursor;   //(n a3 q v1)

        pMvc->insert_rest(pView, "(r q v2)");

        CHECK( fNotified == true );
        CHECK( *cursor == NULL );
        cursor.start_of_content();
        //cout << (*cursor)->to_string() << endl;
        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (clef G) (key e) (n c4 q v1) (n a3 q v1) (goBack start) (goFwd 64) (r q v2))))" );

        delete pMvc;
    }

}

#endif  // _LM_DEBUG_

