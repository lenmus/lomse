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

        executer.undo(&cursor);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 0 );
        CHECK( doc.is_dirty() == true );

        executer.redo(&cursor);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor);
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

        executer.undo(&cursor);
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.redo(&cursor);
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 2 );
        pScore = pContent->get_first_child();
        CHECK( pScore->get_obj_type() == k_imo_score );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor);
        executer.undo(&cursor);
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 0 );
        CHECK( doc.is_dirty() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_block_from_source_1)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj("<para>Hello world!</para>");
        //DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj("(para (txt \"Hello world!\"))");
        CHECK( doc.is_dirty() == false );

        executer.execute(&cursor, pCmd);
//        cout << doc.to_string() << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_name() == "Insert block" );
        ImoDocument* pImoDoc = doc.get_imodoc();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 0 );
        CHECK( doc.is_dirty() == true );

        executer.redo(&cursor);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_imodoc()->get_num_content_items() == 0 );
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

        //cout << m_pDoc->to_string(true) << endl;
        cursor.point_to(15L);   //score
        executer.execute(&cursor, pCmd);
//        cout << m_pDoc->to_string(true) << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;

        CHECK( pCmd->get_name() == "Delete score" );
        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 1 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_blocks_container_undo)
    {
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteBlockLevelObj();

        cursor.point_to(15L);   //score
        executer.execute(&cursor, pCmd);
        executer.undo(&cursor);

        cursor.update_after_deletion();
        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 2 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_score() == true );

        executer.redo(&cursor);

        cursor.update_after_deletion();
        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 1 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    // CmdInsertStaffObj ----------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_1)
    {
        //clef inserted and cursor updated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");

        DocCursor cursor(&doc);
        cursor.enter_element();
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );

        executer.execute(&cursor, pCmd);

        CHECK( pCmd->get_name() == "Insert clef" );
        CHECK( *cursor == NULL );
        CHECK( doc.is_dirty() == true );

        CHECK( pSC->is_at_end_of_empty_score() == false );
        CHECK( pSC->is_at_end_of_staff() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2)
    {
        //undo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        executer.execute(&cursor, pCmd);

        executer.undo(&cursor);

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );
        CHECK( doc.is_dirty() == true );
        string expected = "(lenmusdoc (vers 0.0)(content  (score (vers 2.0)"
            "(instrument (staves 1)(musicData )))))";
        CHECK( doc.to_string() == expected );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 0 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_3)
    {
        //source code validated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clof G)");
        DocCursor cursor(&doc);
        cursor.enter_element();

        int result = executer.execute(&cursor, pCmd);

        CHECK( result == k_failure );
        string expected = "Line 0. Unknown tag 'clof'.\n"
            "Missing analyser for element 'undefined'. Node ignored.\n";
        CHECK( executer.get_error() == expected );
        //cout << executer.get_error() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_4)
    {
        //undo/redo with cursor changes
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();

        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        executer.execute(&cursor, pCmd);
        pCmd = LOMSE_NEW CmdInsertStaffObj("(n c4 q)");
        executer.execute(&cursor, pCmd);
        pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_move_prev);     //to inserted note
        executer.execute(&cursor, pCmd);
        ImoObj* pNoteC4 = *cursor;
        pCmd = LOMSE_NEW CmdInsertStaffObj("(n d4 q)");
        executer.execute(&cursor, pCmd);

        executer.undo(&cursor);    //remove note d4. Cursor points to note c4
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == pNoteC4->get_id() );

        executer.undo(&cursor);    //remove note c4. Cursor points to end of score
        CHECK( *cursor == NULL );

        executer.redo(&cursor);    //insert note c4. Cursor points to end of score
        CHECK( *cursor == NULL );

        executer.redo(&cursor);    //insert note d4. Cursor points to note c4
        pNoteC4 = *cursor;
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == pNoteC4->get_id() );

        //cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_5)
    {
        //validate source code: start/end parenthesis
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("clef G)");

        int result = executer.execute(&cursor, pCmd);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "Missing start or end parenthesis" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_6)
    {
        //validate source code: more than one LDP elements
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)(n e4 e g+)(n c4 e g-)");

        int result = executer.execute(&cursor, pCmd);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "More than one LDP elements" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_7)
    {
        //validate source code: parenthesis missmatch
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e (stem up)");

        int result = executer.execute(&cursor, pCmd);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "Parenthesis missmatch" );
    }

    // CmdInsertManyStaffObjs -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_1)
    {
        //objects inserted and cursor updated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");

        DocCursor cursor(&doc);
        cursor.enter_element();
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );

        executer.execute(&cursor, pCmd);

        CHECK( pCmd->get_name() == "Insert staff objects" );
        CHECK( *cursor == NULL );
        CHECK( doc.is_dirty() == true );

        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->is_at_end_of_empty_score() == false );
        CHECK( pSC->is_at_end_of_staff() == true );
        CHECK( pSC->time() == 64 );

        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content  (score (vers 2.0)(instrument (staves 1)(musicData (clef G p1 )(n e4 e v1  p1 (beam 25 +))(n c4 e v1  p1 (beam 25 -)))))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2)
    {
        //undo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        executer.execute(&cursor, pCmd);

        executer.undo(&cursor);

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );
        CHECK( pSC->time() == 0 );
//        cout << pSC->dump_cursor() << endl;

        CHECK( doc.is_dirty() == true );
        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content  (score (vers 2.0)(instrument (staves 1)(musicData )))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 0 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_3)
    {
        //redo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        executer.execute(&cursor, pCmd);

        executer.undo(&cursor);

        executer.redo(&cursor);

        CHECK( *cursor == NULL );
        CHECK( doc.is_dirty() == true );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->is_at_end_of_empty_score() == false );
        CHECK( pSC->is_at_end_of_staff() == true );

        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content  (score (vers 2.0)(instrument (staves 1)(musicData (clef G p1 )(n e4 e v1  p1 (beam 25 +))(n c4 e v1  p1 (beam 25 -)))))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_4)
    {
        //undo/redo when cursor repositioned
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();

        executer.execute(&cursor, pCmd);

        pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_point_to, 23);     //point to first note
        executer.execute(&cursor, pCmd);
        ImoObj* pNoteE4 = *cursor;

        pCmd = LOMSE_NEW CmdInsertStaffObj("(n d4 q)");
        executer.execute(&cursor, pCmd);

        executer.undo(&cursor);    //remove note d4. Cursor points to note e4
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == pNoteE4->get_id() );

        executer.undo(&cursor);    //remove initial insertions
        CHECK( *cursor == NULL );

        executer.redo(&cursor);    //insert again all staffobjs. Cursor points to end of score
        CHECK( *cursor == NULL );

        executer.redo(&cursor);    //insert note d4. Cursor points to note e4
        pNoteE4 = *cursor;
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == pNoteE4->get_id() );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    // CmdAddStaffObj ----------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, add_staffobj_1)
    {
        //object added and cursor updated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdAddStaffObj("(n e4 e v2)");

        executer.execute(&cursor, pCmd);

        CHECK( pCmd->get_name() == "Add note" );
        //cout << "name: '" << pCmd->get_name() << "'" << endl;
        CHECK( doc.is_dirty() == true );

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, add_staffobj_2)
    {
        //undo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdAddStaffObj("(n e4 e v2)");
        executer.execute(&cursor, pCmd);

        executer.undo(&cursor);

        CHECK( doc.is_dirty() == true );

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, add_staffobj_3)
    {
        //redo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdAddStaffObj("(n e4 e v2)");
        executer.execute(&cursor, pCmd);

        executer.undo(&cursor);

        executer.redo(&cursor);

        CHECK( doc.is_dirty() == true );

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, add_staffobj_4)
    {
        //undo/redo when cursor repositioned (two consecutive commands but
        //with a cursor reposition before second command)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdAddStaffObj("(n e4 e v2)");
        executer.execute(&cursor, pCmd);
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );

        cursor.move_prev();         //points to inserted note e4 e
        pCmd = LOMSE_NEW CmdAddStaffObj("(n g4 e v3)");
        executer.execute(&cursor, pCmd);
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e v2  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        executer.undo(&cursor);    //remove note g4. Cursor points to note e4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e v2  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        executer.undo(&cursor);    //remove note e4. Cursor points to note c4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );

        executer.redo(&cursor);    //add note e4. Cursor points to note c4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );

        executer.redo(&cursor);    //add note g4. Cursor points to note e4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e v2  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        //cout << doc.to_string() << endl;
        //cout << pScore->get_staffobjs_table()->dump() << endl;
    }


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
//        //cout << m_pDoc->to_string(true) << endl;
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
//        executer.undo(&cursor);
//
////        cout << m_pDoc->to_string(true) << endl;
////        pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
////        cout << pScore->get_staffobjs_table()->dump() << endl;
//
//        cursor.update_after_deletion();
//        CHECK( *cursor != NULL );
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->get_id() == 24L );
//        CHECK( m_pDoc->is_dirty() == true );
//    }

}
