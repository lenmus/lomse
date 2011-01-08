//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifdef _LOMSE_DEBUG

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
#include "lomse_model_builder.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

class TestUserCommand : public UserCommand
{
public:
    TestUserCommand(DocCursor& cursor1, DocCursor& cursor2)
        : UserCommand("test command")
        , m_it1(*cursor1)
        , m_it2(*cursor2)
    {
    }
    ~TestUserCommand() {};

protected:
    bool do_actions(DocCommandExecuter* dce)
    {
        dce->execute( new DocCommandRemove(m_it1) );
        dce->execute( new DocCommandRemove(m_it2) );
        return true;
    }

    Document::iterator m_it1;
    Document::iterator m_it2;

};

static bool fNotified = false;
static void my_callback_function(Notification* event)
{
    fNotified = true;
}

class MvcModelTestFixture
{
public:

    MvcModelTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
    }

    ~MvcModelTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    void point_cursors_1(Document* pDoc, DocCursor* pCursor1, DocCursor* pCursor2)
    {
        DocCursor cursor(pDoc);     //score
        cursor.enter_element();     //(clef G)
        ++cursor;       //(key e)
        *pCursor1 = cursor;
        ++cursor;       //(n c4 q)
        ++cursor;       //(r q)
        *pCursor2 = cursor;
    }

    void point_cursors_2(Document* pDoc, DocCursor* pCursor1, DocCursor* pCursor2)
    {
        DocCursor cursor(pDoc);     //score
        *pCursor1 = cursor;
        ++cursor;       //(text "this is a text")
        ++cursor;       //(text "to be removed")
        *pCursor2 = cursor;
    }

    static void my_callback_method(Notification* event)
    {
        m_fNotified = true;
    }

    static bool m_fNotified;
    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
};

bool MvcModelTestFixture::m_fNotified;


SUITE(MvcModelTest)
{

    //TEST_FIXTURE(MvcModelTestFixture, MvcModel_ViewIsNotifiedWhenModifications)
    //{
    //    MvcBuilder builder(*m_pLibraryScope);
    //    MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view,
    //              "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
    //    pMvc->set_callback( &my_callback_method );
    //    Document* pDoc = pMvc->get_document();
    //    DocCursor cursor1(pDoc);
    //    DocCursor cursor2(pDoc);
    //    point_cursors_1(pDoc, &cursor1, &cursor2);
    //    TestUserCommand cmd(cursor1, cursor2);

    //    m_fNotified = false;
    //    UserCommandExecuter* pExec = pMvc->get_command_executer();
    //    pExec->execute(cmd);
    //    CHECK( m_fNotified == true );

    //    delete pMvc;
    //}

    //TEST_FIXTURE(MvcModelTestFixture, MvcModel_ViewIsNotifiedWhenModifications2)
    //{
    //    MvcBuilder builder(*m_pLibraryScope);
    //    MvcElement* pMvc = builder.new_document(MvcBuilder::k_edit_view,
    //              "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
    //    pMvc->set_callback( &my_callback_function );
    //    Document* pDoc = pMvc->get_document();
    //    DocCursor cursor1(pDoc);
    //    DocCursor cursor2(pDoc);
    //    point_cursors_1(pDoc, &cursor1, &cursor2);
    //    TestUserCommand cmd(cursor1, cursor2);

    //    fNotified = false;
    //    UserCommandExecuter* pExec = pMvc->get_command_executer();
    //    pExec->execute(cmd);
    //    CHECK( fNotified == true );

    //    delete pMvc;
    //}

}

#endif  // _LM_DEBUG_

