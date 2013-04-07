//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_command.h"
#include "lomse_document_cursor.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

////---------------------------------------------------------------------------------------
//class TestUserCommand : public UserCommand
//{
//public:
//    TestUserCommand(DocCursor& cursor1, DocCursor& cursor2)
//        : UserCommand("test command")
//        , m_it1(*cursor1)
//        , m_it2(*cursor2)
//    {
//    }
//    ~TestUserCommand() {};
//
//protected:
//    bool do_actions(DocCommandExecuter* dce)
//    {
//        dce->execute( new DocCommandRemove(m_it1) );
//        dce->execute( new DocCommandRemove(m_it2) );
//        return true;
//    }
//
//    Document::iterator m_it1;
//    Document::iterator m_it2;
//
//};

//---------------------------------------------------------------------------------------
class DocCommandTestFixture
{
public:

    DocCommandTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(NULL)
    {
    }

    ~DocCommandTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
    }

    void create_document_1()
    {
//        "(lenmusdoc#0 (vers 0.0) (content#3 "
//            "(score#15 (vers 1.6) "
//                "(instrument#19 (musicData#20 (clef#21 G)(key#22 C)"
//                "(time#23 2 4)(n#24 c4 q)(r#25 q) )))"
//            "(para#26 (txt#27 \"Hello world!\"))"
//        "))"

        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
        m_pDoc->clear_dirty();
    }

    void create_document_2()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "))" );
        m_pDoc->clear_dirty();
    }

    LibraryScope m_libraryScope;
    std::string m_scores_path;
    Document* m_pDoc;
};

SUITE(DocCommandTest)
{
    // CmdCursor ------------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, cursor_point_to_1)
    {
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        DocCommand* pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_point_to, 25L);    //first note

        executer.execute(&cursor, pCmd);

//        cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_name() == "Cursor: point to" );
        CHECK( cursor.get_pointee_id() == 25L );
        CHECK( m_pDoc->is_dirty() == false );
    }

    TEST_FIXTURE(DocCommandTestFixture, cursor_point_to_undo_1)
    {
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        DocCommand* pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_point_to, 25L);    //first note
        executer.execute(&cursor, pCmd);

        executer.undo();

        CHECK( cursor.get_pointee_id() == 15L );    //start of document: score
        CHECK( m_pDoc->is_dirty() == false );
    }

    // CmdInsertBlockLevelObj -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, push_back_blocks_container_1)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj(k_imo_para);
        CHECK( doc.is_dirty() == false );

        executer.execute(&cursor, pCmd);
//        cout << doc.to_string() << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_name() == "Insert paragraph" );
        ImoDocument* pImoDoc = doc.get_imodoc();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo();
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 0 );
        CHECK( doc.is_dirty() == true );

        executer.redo();
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo();
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 0 );
        CHECK( doc.is_dirty() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_blocks_container_1)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj(k_imo_para);
        executer.execute(&cursor, pCmd);

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CmdInsertBlockLevelObj* pCmd2 =
            LOMSE_NEW CmdInsertBlockLevelObj(k_imo_score);

        executer.execute(&cursor, pCmd2);
        CHECK( pCmd2->get_name() == "Insert score" );
//        cout << doc.to_string() << endl;
        ImoDocument* pImoDoc = doc.get_imodoc();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 2 );
        ImoObj* pScore = pContent->get_first_child();
        CHECK( pScore->get_obj_type() == k_imo_score );
        CHECK( doc.is_dirty() == true );

        executer.undo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.redo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 2 );
        pScore = pContent->get_first_child();
        CHECK( pScore->get_obj_type() == k_imo_score );
        CHECK( doc.is_dirty() == true );

        executer.undo();
        executer.undo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 0 );
        CHECK( doc.is_dirty() == true );
    }

    // CmdDeleteBlockLevelObj -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_blocks_container_1)
    {
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        DocCommand* pCmd = LOMSE_NEW CmdDeleteBlockLevelObj();

        //cout << m_pDoc->to_string(k_save_ids) << endl;
        cursor.point_to(15L);   //score
        executer.execute(&cursor, pCmd);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;

        cursor.update_after_deletion();
        CHECK( pCmd->get_name() == "Delete score" );
        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 1 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

//    TEST_FIXTURE(DocCommandTestFixture, delete_blocks_container_undo)
//    {
//        create_document_1();
//        DocCursor cursor(m_pDoc);
//        DocCommandExecuter executer(m_pDoc);
//        DocCommand* pCmd = LOMSE_NEW CmdDeleteBlockLevelObj();
//
//        cursor.point_to(15L);   //score
//        executer.execute(&cursor, pCmd);
//        executer.undo();
//
//        cursor.update_after_deletion();
//        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 2 );
//        CHECK( m_pDoc->is_dirty() == true );
//        CHECK( *cursor != NULL );
//        CHECK( (*cursor)->is_score() == true );
//
//        executer.redo();
//
//        cursor.update_after_deletion();
//        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 1 );
//        CHECK( m_pDoc->is_dirty() == true );
//        CHECK( *cursor != NULL );
//        CHECK( (*cursor)->is_paragraph() == true );
//    }

//    // CmdDeleteStaffObj ----------------------------------------------------------------
//
//    TEST_FIXTURE(DocCommandTestFixture, delete_note_1)
//    {
//        create_document_1();
//        DocCursor cursor(m_pDoc);
//        DocCommandExecuter executer(m_pDoc);
//        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();
//
//        cursor.point_to(24L);
//        executer.execute(&cursor, pCmd);
//        //cout << m_pDoc->to_string(k_save_ids) << endl;
//        //ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//        //cout << pScore->get_staffobjs_table()->dump() << endl;
//        //cout << "cmd name = " << pCmd->get_name() << endl;
//
//        CHECK( pCmd->get_name() == "Delete note" );
//        cursor.update_after_deletion();
//        CHECK( *cursor != NULL );
//        CHECK( (*cursor)->is_rest() == true );
//        CHECK( m_pDoc->is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, delete_note_1_undo)
//    {
//        create_document_2();
//        DocCursor cursor(m_pDoc);
//        DocCommandExecuter executer(m_pDoc);
//        CHECK( m_pDoc->is_dirty() == false );
//        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();
//
//        cursor.point_to(24L);
//        executer.execute(&cursor, pCmd);
//        executer.undo();
//
////        cout << m_pDoc->to_string(k_save_ids) << endl;
////        pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
////        cout << pScore->get_staffobjs_table()->dump() << endl;
//
//        cursor.update_after_deletion();
//        CHECK( *cursor != NULL );
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->get_id() == 24L );
//        CHECK( m_pDoc->is_dirty() == true );
//    }

    // CmdInsertStaffObj ----------------------------------------------------------------

//    TEST_FIXTURE(DocCommandTestFixture, insert_note_1)
//    {
//        create_document_1();
//        DocCursor cursor(m_pDoc);
//        DocCommandExecuter executer(m_pDoc);
//
//        cursor.point_to(24L);
//        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj(cursor, k_imo_note);
//        cout << "cmd name = " << pCmd->get_name() << endl;
//        CHECK( pCmd->get_name() == "Insert note" );
//
//        executer.execute(&cursor, pCmd);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//
////        cursor.update_after_deletion();
//        CHECK( *cursor != NULL );
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( cursor.get_pointee_id() == 24L );
//        CHECK( m_pDoc->is_dirty() == true );
//    }

}
