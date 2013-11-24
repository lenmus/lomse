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
#include "lomse_im_note.h"
#include "lomse_im_attributes.h"
#include "lomse_selections.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
// helper macros
// CHECK_ENTRY0: does not check/displays ids
#define CHECK_ENTRY0(it, _instr, _staff, _measure, _time, _line, _object) \
            CHECK( (*it)->num_instrument() == _instr );         \
            CHECK( (*it)->staff() == _staff );                  \
            CHECK( (*it)->measure() == _measure );              \
            CHECK( is_equal_time((*it)->time(), _time) );       \
            CHECK( (*it)->line() == _line );                    \
            CHECK( (*it)->to_string() == _object );             \
            ++it;


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
        //"(lenmusdoc#0 (vers 0.0) (content#3 "
        //    "(score#15 (vers 1.6) "
        //        "(instrument#19 (musicData#20 (clef#21 G)(key#22 C)"
        //        "(time#23 2 4)(n#24 c4 q)(r#25 q) )))"
        //    "(para#26 (txt#27 \"Hello world!\"))"
        //"))"
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
        m_pDoc->clear_dirty();
    }

    void create_document_2()
    {
        //(score#15 (vers 2.0)(instrument#19 (musicData#20
        //(clef#21 G)(key#22 C)(time#23 2 4)(n#24 c4 q)(r#25 q)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0) "
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

    // CmdAddNoteRest -------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0101)
    {
        //replace edition mode.
        //101. Notes inserted at empty places
        //  (clef G)
        //          |
        //          +-- add t=0 (n a4 e v1)
        //  (clef G)(n a4 e v1)

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to end of score
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        executer.execute(&cursor, pCmd, NULL);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( *cursor == NULL );
        CHECK( pSC->is_at_end_of_staff() == true );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0101_ur)
    {
        //101. undo/redo

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to end of score
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);

        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);
        executer.redo(&cursor);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( *cursor == NULL );
        CHECK( pSC->is_at_end_of_staff() == true );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0201)
    {
        //replace edition mode.
        //201. new note starts at same timepos than old one. Both notes equal duration:
        //     -> replace.
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //          |
        //          +-- add t=0 (n a4 e v1)
        //  (clef G)(n a4 e v1)(n f4 e v1)(n g4 e v1)
        //                     |
        //                     +-- t=32, v=1, obj=f4 e

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        executer.execute(&cursor, pCmd, NULL);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 e v1  p1 )" );
        CHECK( is_equal_time(pSC->time(), 32.0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0201_ur)
    {
        //201. undo/redo

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);

        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);
        executer.redo(&cursor);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 e v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1  p1 )" );
    }

//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0202)
//    {
//        //replace edition mode.
//        //202. new note starts in the middle of existing note. New note end at same
//        //     timepos than old note: -> reduce & insert
//        //  (clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)
//        //               |
//        //               +-- add t=48 (n a4 s v1)
//        //  (clef G)(n e4 e. v1)(n a4 s v1)(n f4 q v1)(n g4 q v1)
//        //                                 |
//        //                                 +-- t=64, v=1, obj=f4 q
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        pSC->to_time(0, 0, 48.0);   //instr=0, staff=0, time=48
//        //cout << pSC->dump_cursor() << endl;
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 s v1)", k_edit_mode_replace);
//        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n f4 q v1  p1 )" );
//        CHECK( is_equal_time(pSC->time(), 64.0) );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 s v1  p1 )" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 e. v1  p1 )" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0202_ur)
//    {
//        //202. undo/redo
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        pSC->to_time(0, 0, 48.0);   //instr=0, staff=0, time=48
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 s v1)", k_edit_mode_replace);
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        executer.undo(&cursor);
//        executer.redo(&cursor);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n f4 q v1  p1 )" );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 s v1  p1 )" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 e. v1  p1 )" );
//    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0203)
    {
        //replace edition mode.
        //203. new note starts at same timepos than existing one. New note is shorter than old note:
        //     -> insert & reduce
        //  (clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)
        //          |
        //          +-- add t=0 (n a4 s v1)
        //  (clef G)(n a4 s v1)(n e4 e. v1)(n f4 q v1)(n g4 q v1)
        //                     |
        //                     +-- t=16, v=1, obj=e4 e.

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 q

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 s v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        executer.execute(&cursor, pCmd, NULL);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e. v1  p1 )" );
        CHECK( is_equal_time(pSC->time(), 16.0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 s v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0203_ur)
    {
        //203. undo/redo.

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 q

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 s v1)", k_edit_mode_replace);

        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);
        executer.redo(&cursor);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e. v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 s v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0204)
    {
        //replace edition mode.
        //204. new note starts at same timepos than existing one. New note is longer than
        //     old note: -> replace many & reduce last
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //          |
        //          +-- add t=0 (n a4 e. v1)
        //  (clef G)(n a4 e. v1)(n f4 s v1)(n g4 e v1)
        //                      |
        //                      +-- t=48, v=1, obj=f4 s

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e. v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        executer.execute(&cursor, pCmd, NULL);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 s v1  p1 )" );
        CHECK( is_equal_time(pSC->time(), 48.0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e. v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0204_ur)
    {
        //204. undo/redo

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e. v1)", k_edit_mode_replace);

        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);
        executer.redo(&cursor);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 s v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e. v1  p1 )" );
    }

//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0205)
//    {
//        //replace edition mode.
//        //205. new note starts in the middle of existing note. New note is longer
//        //     than old note: -> reduce first, replace many & reduce last
//        //  (clef G)(n e4 q v1)(n f4 e v1)(n g4 e v1)(n b4 q v1)
//        //               |
//        //               +-- add t=16 (n a4 h v1)
//        //  (clef G)(n e4 s v1)(n a4 h v1)(n b4 e. v1)
//        //                                |
//        //                                +-- t=144, v=1, obj=b4 e.
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 q v1)(n f4 e v1)(n g4 e v1)(n b4 q v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        pSC->to_time(0, 0, 16.0);   //instr=0, staff=0, time=16
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 h v1)", k_edit_mode_replace);
//        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n b4 e. v1  p1 )" );
//        CHECK( is_equal_time(pSC->time(), 144.0) );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 h v1  p1 )" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 s v1  p1 )" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0205_ur)
//    {
//        //205. undo/redo.
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 q v1)(n f4 e v1)(n g4 e v1)(n b4 q v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        pSC->to_time(0, 0, 16.0);   //instr=0, staff=0, time=16
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 h v1)", k_edit_mode_replace);
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        executer.undo(&cursor);
//        executer.redo(&cursor);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n b4 e. v1  p1 )" );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 h v1  p1 )" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 s v1  p1 )" );
//    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0301)
    {
        //replace edition mode.
        //301. added v2 at empty position for v2, no previous notes v2 in score:
        //     -> a gap from start of measure inserted before the new note.
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //                     |
        //                     +-- add t=32 (n a4 e v2)
        //  (clef G)(n e4 e v1)(goFwd e v2)(n a4 e v2)(n f4 e v1)(n g4 e v1)
        //                                                       |
        //                                 t=64, v=1, obj=g4 e --+

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        cursor.move_next();         //points to f4 e

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v2)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        executer.execute(&cursor, pCmd, NULL);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(goFwd e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(n a4 e v2  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1  p1 )" );
        //cout << pTable->dump() << endl;

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( pSC->is_at_empty_place() == false );
        CHECK( pSC->staffobj_internal()->to_string() == "(n g4 e v1  p1 )" );
        CHECK( is_equal_time(pSC->time(), 64.0) );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0301_ur)
    {
        //301. undo/redo
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        cursor.move_next();         //points to f4 e

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v2)", k_edit_mode_replace);

        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);
        executer.redo(&cursor);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;

        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(goFwd e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(n a4 e v2  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1  p1 )" );
        //cout << pTable->dump() << endl;

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( pSC->is_at_empty_place() == false );
        CHECK( pSC->staffobj_internal()->to_string() == "(n g4 e v1  p1 )" );
        CHECK( is_equal_time(pSC->time(), 64.0) );
    }

//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0302)
//    {
//        //replace edition mode.
//        //302. added v2 at empty position for v2: a previous v2 note exists in that measure:
//        //     -> a gap from end or previous v2 note inserted.
//        //  (clef G)(n e4 e v1)(n c5 e v2)(n f4 e v1)(n g4 e v1)
//        //                                           |
//        //                                           +-- add t=64 (n a4 e v2)
//        //  (clef G)(n e4 e v1)(n c5 e v2)(n f4 e v1)(goFwd q v2)(n a4 e v2)(n g4 e v1)
//        //                                                                             |
//        //                                               t=96, v=1, obj=end-of-score --+
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 e v1)(n c5 e v2)(n f4 e v1)(n g4 e v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        cursor.move_next();         //points to e4 e v1
//        cursor.move_next();         //points to c5 e v2
//        cursor.move_next();         //points to f4 e v1
//        cursor.move_next();         //points to g4 e v1
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v2)", k_edit_mode_replace);
//        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        CHECK( pTable->num_lines() == 2 );
//        CHECK( pTable->num_entries() == 7 );
//        CHECK( pTable->is_anacrusis_start() == false );
//
//        ColStaffObjsIterator it = pTable->begin();
//        //              instr, staff, meas. time, line, scr
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1  p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 e v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1  p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(goFwd q v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1  p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     1, "(n g4 e v1  p1 )" );
//        cout << pTable->dump() << endl;
//
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points at end of score
//        CHECK( pSC->is_at_end_of_score() == true );
//        CHECK( is_equal_time(pSC->time(), 128.0) );
//
//        cursor.move_prev();
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n g4 e v1  p1 )" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0302_ur)
//    {
//        //302. undo/redo
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 e v1)(n c5 e v2)(n f4 e v1)(n g4 e v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        cursor.move_next();         //points to e4 e v1
//        cursor.move_next();         //points to c5 e v2
//        cursor.move_next();         //points to f4 e v1
//        cursor.move_next();         //points to g4 e v1
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v2)", k_edit_mode_replace);
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        executer.undo(&cursor);
//        executer.redo(&cursor);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        CHECK( pTable->num_lines() == 2 );
//        CHECK( pTable->num_entries() == 7 );
//        CHECK( pTable->is_anacrusis_start() == false );
//
//        ColStaffObjsIterator it = pTable->begin();
//        //              instr, staff, meas. time, line, scr
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1  p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 e v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1  p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(goFwd e v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     1, "(n a4 e v2  p1 )" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1  p1 )" );
//        cout << pTable->dump() << endl;
//
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points at end of score
//
//        cursor.move_prev();
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n g4 e v1  p1 )" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0303)
//    {
//        //replace edition mode.
//        //303. added v2 at empty position for v2, no previous notes v2 in that measure:
//        //     -> a gap from end or previous v2 note inserted.
//        //  (clef G)(n e4 e v1)(barline)(n f4 e v1)(n g4 e v1)
//        //                                         |
//        //                                         +-- add t=64 (n a4 e v2)
//        //  (clef G)(n e4 e v1)(barline)(n f4 e v1)(goFwd e v2)(n a4 e v2)(n g4 e v1)
//        //                                                                |
//        //                          t=128, v=2, obj=empty, ref_obj=g4 e --+
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 e v1)(barline)(n f4 e v1)(n g4 e v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        cursor.move_next();         //points to e4 e v1
//        cursor.move_next();         //points to barline
//        cursor.move_next();         //points to f4 e v1
//        cursor.move_next();         //points to g4 e v1
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v2)", k_edit_mode_replace);
//        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
////        CHECK( (*cursor)->is_note() == true );
////        CHECK( (*cursor)->to_string() == "(n g4 e v1  p1 )" );
//        CHECK( pSC->is_at_empty_place() == true );
//        CHECK( pSC->staffobj_internal()->to_string() == "(n g4 e v1  p1 )" );
//        CHECK( is_equal_time(pSC->time(), 128.0) );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 7 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 e v2  p1 )" );
//
//        cursor.move_prev();         //prev note is the goFwd
//        CHECK( (*cursor)->is_gap() == true );
//        CHECK( (*cursor)->to_string() == "(goFwd e v2 p1)" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0303_ur)
//    {
//        //303. undo/redo
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(n e4 e v1)(barline)(n f4 e v1)(n g4 e v1)"
//            ")))");
//        doc.clear_dirty();
//        DocCursor cursor(&doc);
//        DocCommandExecuter executer(&doc);
//        cursor.enter_element();     //points to clef
//        cursor.move_next();         //points to e4 e v1
//        cursor.move_next();         //points to barline
//        cursor.move_next();         //points to f4 e v1
//        cursor.move_next();         //points to g4 e v1
//
//        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v2)", k_edit_mode_replace);
//
//        executer.execute(&cursor, pCmd, NULL);
//
//        executer.undo(&cursor);
//        executer.redo(&cursor);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
////        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n g4 e v1  p1 )" );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 7 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 e v2  p1 )" );
//
//        cursor.move_prev();         //prev note is the goFwd
//        CHECK( (*cursor)->is_gap() == true );
//        CHECK( (*cursor)->to_string() == "(goFwd e v2 p1)" );
//    }


    // CmdAddTie ------------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, add_tie_1001)
    {
        //add tie
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to e4 q
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdAddTie();

        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Add tie" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote2 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pNote2->is_tied_next() == false );
        cursor.move_prev();
        pNote1 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pNote1->is_tied_next() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdAddTuplet ---------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, add_tuplet_1101)
    {
        //add tuplet
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e)(n g4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to n f4 e
        cursor.move_next();         //points to n g4 e
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdAddTuplet("(t + 2 3)");

        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Add tuplet" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->is_in_tuplet() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, add_tuplet_1102)
    {
        //add tuplet error
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e)(n g4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to n f4 e
        cursor.move_next();         //points to n g4 e
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdAddTuplet("(t = 2 3)");  // "(t % 2 3)");

        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK( result == k_failure );
        CHECK( executer.get_error() == "Line 0. Missing or invalid tuplet type. Tuplet ignored.\n" );
//        cout << "msg=[" << executer.get_error() << "]" << endl;

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, add_tuplet_1103)
    {
        //undo add tuplet
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e)(n g4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to n f4 e
        cursor.move_next();         //points to n g4 e
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdAddTuplet("(t + 2 3)");

        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->is_in_tuplet() == false );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        executer.redo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->is_in_tuplet() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdBreakBeam ---------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1201)
    {
        //break beam. note + note = beam removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n a4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4
        cursor.move_next();         //points to n a4
        DocCommand* pCmd = LOMSE_NEW CmdBreakBeam();

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->is_beamed() == false );
        cursor.move_prev();         //points to n e4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1202)
    {
        //break beam: note + beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 s)(n a4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4
        cursor.move_next();         //points to n f4
        DocCommand* pCmd = LOMSE_NEW CmdBreakBeam();

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        cursor.move_prev();         //points to n e4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == false );
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );
        cursor.move_next();         //points to a4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1203)
    {
        //break beam: beam + note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 s g+)(n f4 s)(n a4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4
        cursor.move_next();         //points to n f4
        cursor.move_next();         //points to n a4
        DocCommand* pCmd = LOMSE_NEW CmdBreakBeam();

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_A );
        cursor.move_prev();         //points to n f4
        cursor.move_prev();         //points to n e4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );
        cursor.move_next();         //points to a4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->is_beamed() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1204)
    {
        //break beam: two beams
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 s g+)(n f4 s)(n g4 s)(n a4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 s
        cursor.move_next();         //points to n f4
        cursor.move_next();         //points to n g4
        DocCommand* pCmd = LOMSE_NEW CmdBreakBeam();

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        cursor.move_prev();         //points to n f4
        cursor.move_prev();         //points to n e4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );
        cursor.move_next();         //points to g4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(2) == ImoBeam::k_none );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdChangeAccidentals --------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, change_accidentals_1301)
    {
        //change accidentals
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdChangeAccidentals(k_sharp);

        SelectionSet sel;
        sel.debug_add(pNote);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Change accidentals" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote->get_notated_accidentals() == k_sharp );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_accidentals_1302)
    {
        //undo change accidentals
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to n g4 s
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdChangeAccidentals(k_flat_flat);
        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_notated_accidentals() == 0 );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_notated_accidentals() == 0 );

        executer.redo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_notated_accidentals() == k_flat_flat );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_notated_accidentals() == k_flat_flat );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdChangeAttribute ---------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1401)
    {
        //change barline type
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(barline simple)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to barline
        ImoObj* pImo = *cursor;
        DocCommand* pCmd = LOMSE_NEW CmdChangeAttribute(k_attr_barline,
                                                        k_barline_double,
                                                        "Change barline type");

        executer.execute(&cursor, pCmd, NULL);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Change barline type" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( pImo == *cursor );
        ImoBarline* pBar = static_cast<ImoBarline*>(pImo);
        CHECK( pBar->get_type() == k_barline_double );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1402)
    {
        //undo/redo change barline type
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(barline simple)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to barline
        ImoObj* pImo = *cursor;
        DocCommand* pCmd = LOMSE_NEW CmdChangeAttribute(k_attr_barline,
                                                        k_barline_double,
                                                        "Change barline type");

        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( pImo == *cursor );
        ImoBarline* pBar = static_cast<ImoBarline*>(pImo);
        CHECK( pBar->get_type() == k_barline_simple );

        executer.redo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( pImo == *cursor );
        pBar = static_cast<ImoBarline*>(pImo);
        CHECK( pBar->get_type() == k_barline_double );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1403)
    {
        //toggle note stem
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q (stem up))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to barline
        ImoObj* pImo = *cursor;
        DocCommand* pCmd = LOMSE_NEW CmdChangeAttribute(k_attr_stem_type,
                                                        k_stem_down,
                                                        "Toggle note stem");

        executer.execute(&cursor, pCmd, NULL);

        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Toggle note stem" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pImo == *cursor );
        ImoNote* pNote = static_cast<ImoNote*>(pImo);
        CHECK( pNote->get_stem_direction() == k_stem_down );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1404)
    {
        //undo/redo toggle note stem
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 q (stem up))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to barline
        ImoObj* pImo = *cursor;
        DocCommand* pCmd = LOMSE_NEW CmdChangeAttribute(k_attr_stem_type,
                                                        k_stem_down,
                                                        "Toggle note stem");

        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pImo == *cursor );
        ImoNote* pNote = static_cast<ImoNote*>(pImo);
        CHECK( pNote->get_stem_direction() == k_stem_up );

        executer.redo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pImo == *cursor );
        pNote = static_cast<ImoNote*>(pImo);
        CHECK( pNote->get_stem_direction() == k_stem_down );
    }

    // CmdChangeDots --------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, change_dots_1501)
    {
        //change dots
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdChangeDots(1);

        SelectionSet sel;
        sel.debug_add(pNote);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Change dots" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote->get_dots() == 1 );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_dots_1502)
    {
        //undo change dots
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to n g4 s
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdChangeDots(2);

        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);      //cursor is pointing to note2

        executer.undo(&cursor);     //cursor restored to note2
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_dots() == 0 );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_dots() == 0 );

        executer.redo(&cursor);     //cursor pointing to note1 but redo restores original pos
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_dots() == 2 );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_dots() == 2 );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdCursor ------------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, cursor_1601)
    {
        //1601. point to an object
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        DocCommand* pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_point_to, 25L);    //first note

        executer.execute(&cursor, pCmd, NULL);

//        cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( pCmd->get_name() == "Cursor: point to" );
        CHECK( cursor.get_pointee_id() == 25L );
        CHECK( m_pDoc->is_dirty() == false );
    }

    // CmdDeleteBlockLevelObj -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_blocks_container_1701)
    {
        //delete blocks container
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        DocCommand* pCmd = LOMSE_NEW CmdDeleteBlockLevelObj();

        //cout << m_pDoc->to_string(true) << endl;
        cursor.point_to(15L);   //score
        executer.execute(&cursor, pCmd, NULL);
//        cout << m_pDoc->to_string(true) << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_full_checkpoint );
        CHECK( pCmd->get_name() == "Delete score" );
        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 1 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_blocks_container_1702)
    {
        //undo/redo delete blocks container
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteBlockLevelObj();

        cursor.point_to(15L);   //score
//        cout << "cursor: top-level=" << cursor.get_top_id()
//             << ", pointee=" << cursor.get_pointee_id()
//             << ", delegating?=" << (cursor.is_inside_terminal_node() ? "Yes" : "No")
//             << endl;
        executer.execute(&cursor, pCmd, NULL);
        executer.undo(&cursor);
        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 2 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_score() == true );

        executer.redo(&cursor);

        CHECK( m_pDoc->get_imodoc()->get_num_content_items() == 1 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    // CmdDeleteRelation ----------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1801)
    {
        //delete relation (tuplet)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+ (t + 2 3))(n f4 e)(n g4 e g- (t -))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        ImoTuplet* pTuplet = pNote1->get_tuplet();
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation();

        SelectionSet sel;
        sel.debug_add(pTuplet);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Delete tuplet" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->is_in_tuplet() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1802)
    {
        //delete relation (beam)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e)(n g4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        ImoBeam* pBeam = pNote1->get_beam();
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation();

        SelectionSet sel;
        sel.debug_add(pBeam);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Delete beam" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1803)
    {
        //delete relation (beam)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e)(n g4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        ImoBeam* pBeam = pNote->get_beam();
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation();
        SelectionSet sel;
        sel.debug_add(pBeam);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_beamed() == true );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == true );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == true );

        executer.redo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1804)
    {
        //delete relation (old tie)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n f4 e l)(n f4 q)(n e4 e l)(n e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n f4 e
        cursor.move_next();         //points to n f4 q
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        ImoTie* pTie = pNote->get_tie_next();
//        cout << "Before delete tie: id=" << pTie->get_id() << endl;
//        cout << doc.to_string(true) << endl;
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation();

        SelectionSet sel;
        sel.debug_add(pTie);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );
        cursor.move_next();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

        executer.undo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == true );
        CHECK( pNote->is_tied_prev() == false );
        cursor.move_next();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == true );
        pTie = pNote->get_tie_prev();
//        cout << "After undo: id=" << pTie->get_id() << endl;
//        cout << doc.to_string(true) << endl;

        executer.redo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );
        cursor.move_next();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1805)
    {
        //delete relation (new tie, notes appart)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n f4 e v1 (tie 1 start))(n a4 q v2)(n f4 e v1 (tie 1 stop))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n f4 e
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        ImoTie* pTie = pNote->get_tie_next();
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation();

//        cout << "Before delete tie: id=" << pTie->get_id() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;

        SelectionSet sel;
        sel.debug_add(pTie);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "After delete tie:" << endl;
//        pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );      // f4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );
        cursor.move_next();                         // f4, skips a4 in v2
//        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

//        cout << "After delete tie:" << endl;
//        pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        executer.undo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );      // f4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == true );
        CHECK( pNote->is_tied_prev() == false );
        cursor.move_next();                         // f4, skips a4 in v2
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == true );

//        pTie = pNote->get_tie_prev();
//        cout << "After undo: id=" << pTie->get_id() << endl;
//        pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        executer.redo(&cursor);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );      //f4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );
        cursor.move_next();                         // f4, skips a4 in v2
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        cout << "After redo:" << endl;
//        pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1806)
    {
        //delete relation (alternative constructor)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+ (t + 2 3))(n f4 e)(n g4 e g- (t -))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        ImoTuplet* pTuplet = pNote1->get_tuplet();
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation(k_imo_tuplet);

        SelectionSet sel;
        sel.debug_add(pTuplet);
        sel.debug_add(pNote1);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Delete tuplet" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->is_in_tuplet() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdDeleteSelection ---------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1901)
    {
        //delete staffobjs
        //cursor positioned at first object after first deleted one
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e g-)(n g4 q)"
            ")))");

        SelectionSet sel;
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        ++cursor;       //n e4
        sel.debug_add(*cursor);
        ++cursor;       //n f4
        sel.debug_add(*cursor);

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteSelection("Delete my selection");

        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_full_checkpoint );
        CHECK( pCmd->get_name() == "Delete my selection" );
        CHECK( doc.is_dirty() == true );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );
        //cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 q v1  p1 )" );

        //cursor pointing to right place
        CHECK( (*cursor)->to_string() == "(n g4 q v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1902)
    {
        //1902. delete staffobjs. First staffobj deleted
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e g-)(n g4 q)"
            ")))");

        SelectionSet sel;
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        sel.debug_add(*cursor);
        ++cursor;       //n e4
        sel.debug_add(*cursor);

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteSelection();

        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_full_checkpoint );
        CHECK( pCmd->get_name() == "Delete selection" );
        CHECK( doc.is_dirty() == true );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );
        //cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n f4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   32,     0, "(n g4 q v1  p1 )" );

        //cursor pointing to right place
        CHECK( (*cursor)->to_string() == "(n f4 e v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1903)
    {
        //delete relation
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n f4 e g-)(n g4 q)"
            ")))");

        SelectionSet sel;
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        ++cursor;       //n e4
        ImoNote* pNote = static_cast<ImoNote*>(*cursor);
        sel.debug_add(pNote->get_beam());

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteSelection();

        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacrusis_start() == false );
//        cout << doc.to_string(true) << endl;
//        cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        ImoStaffObj* pSO = (*it)->imo_object();     //it points to next: n e4
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 q v1  p1 )" );

        CHECK( static_cast<ImoStaffObj*>(*cursor) == pSO );    //cursor points to n g4

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1904)
    {
        //delete auxobj
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e (text \"This is a note\"))(n f4 e)(n g4 q)"
            ")))");

        SelectionSet sel;
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        ++cursor;       //n e4
        ImoNote* pNote = static_cast<ImoNote*>(*cursor);
        sel.debug_add(pNote->get_attachment(0));

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteSelection();

        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacrusis_start() == false );
//        cout << doc.to_string(true) << endl;
//        cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        ImoStaffObj* pSO = (*it)->imo_object();     //it points to next: n e4
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 q v1  p1 )" );

        CHECK( static_cast<ImoStaffObj*>(*cursor) == pSO );    //cursor points to n e4

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
    }

    // CmdDeleteStaffObj ----------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2001)
    {
        //delete object. cursor points to next one
        create_document_1();
        DocCursor cursor(m_pDoc);
        cursor.point_to(24L);
        DocCommandExecuter executer(m_pDoc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);
        //cout << m_pDoc->to_string(true) << endl;
        //ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        //cout << pScore->get_staffobjs_table()->dump() << endl;
        //cout << "cmd name = " << pCmd->get_name() << endl;

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK( pCmd->get_name() == "Delete note" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_rest() == true );
        CHECK( m_pDoc->is_dirty() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2002)
    {
        //2002. undo delete staffobj
        create_document_2();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        cursor.point_to(24L);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);
        executer.undo(&cursor);

//        cout << m_pDoc->to_string(true) << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//        cout << pScore->get_staffobjs_table()->dump() << endl;
        //cout << cursor.dump_cursor();

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == 24L );
        CHECK( m_pDoc->is_dirty() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2003)
    {
        //delete beamed note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 e g+)(n e4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e v1  p1 )" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2004)
    {
        //undo delete beamed note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 e g+)(n e4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);
        executer.undo(&cursor);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 e v1  p1 (beam 24 +))" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 e v1  p1 (beam 24 +))" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n e4 e v1  p1 (beam 24 -))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2005)
    {
        //delete beamed note. Still two notes in beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n g5 s g+)(n f5 s)(n g5 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n g5 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f5 s v1  p1 (beam 25 +f))" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n f5 s v1  p1 (beam 25 +f))" );
        CHECK_ENTRY0(it, 0,    0,      0,  16,     0, "(n g5 e v1  p1 (beam 25 -))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2006)
    {
        //undo delete beamed note when still two notes in beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n g5 s g+)(n f5 s)(n g5 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n g5 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);
        executer.undo(&cursor);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n g5 s v1  p1 (beam 25 ++))" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g5 s v1  p1 (beam 25 ++))" );
        CHECK_ENTRY0(it, 0,    0,      0,  16,     0, "(n f5 s v1  p1 (beam 25 =-))" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n g5 e v1  p1 (beam 25 -))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2007)
    {
        //delete note in chord. Still two notes in chord
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(chord (n c4 s)(n e4 s)(n g4 s))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(chord (n e4 s v1  p1 )" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(chord (n e4 s v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 s v1  p1 ))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2008)
    {
        //delete note in chord. One note left: chord removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(chord (n e4 s)(n g4 s))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n g4 s v1  p1 )" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_chord() == false );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 s v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2009)
    {
        //delete tied note. Tie removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n g4 s l+)(n g4 s l-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n g4 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n g4 s v1  p1 )" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 s v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2010)
    {
        //delete start note of tuplet. Tuplet removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n g4 s (t + 2 3))(n e4 s (t -))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n g4 s
        CHECK( (*cursor)->is_note() == true );
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_tuplet() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 s v1  p1 )" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_tuplet() == false );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 s v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2011)
    {
        //delete end note of tuplet. Tuplet removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 s (t + 2 3))(n g4 s (t -))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 s
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_tuplet() == true );
        cursor.move_next();         //points to n g4 s
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_tuplet() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->is_at_end_of_staff() == true );
        cursor.move_prev();         //points to n e4 s
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 s v1  p1 )" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_tuplet() == false );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 s v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2012)
    {
        //2012. delete first staffobj in score
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n f4 e)(n g4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );
        //cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n f4 e v1  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   32,     0, "(n g4 q v1  p1 )" );

        //cursor pointing to right place
        CHECK( (*cursor)->to_string() == "(n f4 e v1  p1 )" );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2013)
    {
        //2013. delete the only staffobj in score
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        executer.execute(&cursor, pCmd, NULL);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_entries() == 0 );
        CHECK( pTable->is_anacrusis_start() == false );

        //cursor pointing to right place
        //cout << "id=" << cursor.get_pointee_id() << endl;
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_score() );
    }

    // CmdInsertBlockLevelObj -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, insert_block_2101)
    {
        //push_back_blocks_container
        Document doc(m_libraryScope);
        doc.create_empty();
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj(k_imo_para);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == false );

        executer.execute(&cursor, pCmd, NULL);
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

    TEST_FIXTURE(DocCommandTestFixture, insert_block_2102)
    {
        //insert_blocks_container
        Document doc(m_libraryScope);
        doc.create_empty();
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj(k_imo_para);
        executer.execute(&cursor, pCmd, NULL);

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

    TEST_FIXTURE(DocCommandTestFixture, insert_block_2103)
    {
        //insert_block_from_source
        Document doc(m_libraryScope);
        doc.create_empty();
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj("<para>Hello world!</para>");
        //DocCommand* pCmd = LOMSE_NEW CmdInsertBlockLevelObj("(para (txt \"Hello world!\"))");
        CHECK( doc.is_dirty() == false );

        executer.execute(&cursor, pCmd, NULL);
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

    // CmdInsertManyStaffObjs -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2201)
    {
        //objects inserted and cursor updated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        DocCursor cursor(&doc);
        cursor.enter_element();
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );

        executer.execute(&cursor, pCmd, NULL);

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

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2202)
    {
        //undo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        executer.execute(&cursor, pCmd, NULL);
        cout << doc.to_string() << endl;

        executer.undo(&cursor);

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );
        CHECK( pSC->time() == 0 );
//        cout << pSC->dump_cursor() << endl;

        CHECK( doc.is_dirty() == true );
        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content  (score (vers 2.0)(instrument (staves 1)(musicData )))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 0 );
        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2203)
    {
        //redo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        executer.execute(&cursor, pCmd, NULL);

        executer.undo(&cursor);

        executer.redo(&cursor);

        CHECK( *cursor == NULL );
        CHECK( doc.is_dirty() == true );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->is_at_end_of_empty_score() == false );
        CHECK( pSC->is_at_end_of_staff() == true );

        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content  (score (vers 2.0)(instrument (staves 1)(musicData (clef G p1 )(n e4 e v1  p1 (beam 34 +))(n c4 e v1  p1 (beam 34 -)))))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2204)
    {
        //undo/redo when cursor repositioned
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();

        executer.execute(&cursor, pCmd, NULL);

        pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_point_to, 23);     //point to first note
        executer.execute(&cursor, pCmd, NULL);
        ImoObj* pNoteE4 = *cursor;

        pCmd = LOMSE_NEW CmdInsertStaffObj("(n d4 q)");
        executer.execute(&cursor, pCmd, NULL);

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

    // CmdInsertStaffObj ----------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2301)
    {
        //clef inserted and cursor updated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );

        DocCursor cursor(&doc);
        cursor.enter_element();
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );

        executer.execute(&cursor, pCmd, NULL);

        CHECK( pCmd->get_name() == "Insert clef" );
        CHECK( *cursor == NULL );
        CHECK( doc.is_dirty() == true );

//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << pSC->dump_cursor() << endl;

        CHECK( pSC->is_at_end_of_empty_score() == false );
        CHECK( pSC->is_at_end_of_staff() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2302)
    {
        //undo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        executer.execute(&cursor, pCmd, NULL);

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

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2303)
    {
        //source code validated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clof G)");
        DocCursor cursor(&doc);
        cursor.enter_element();

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK( result == k_failure );
        string expected = "Line 0. Unknown tag 'clof'.\n"
            "Missing analyser for element 'undefined'. Node ignored.\n";
        CHECK( executer.get_error() == expected );
        //cout << executer.get_error() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2304)
    {
        //undo/redo with cursor changes
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();

        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        executer.execute(&cursor, pCmd, NULL);
        pCmd = LOMSE_NEW CmdInsertStaffObj("(n c4 q)");
        executer.execute(&cursor, pCmd, NULL);
        pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_move_prev);     //to inserted note
        executer.execute(&cursor, pCmd, NULL);
        ImoObj* pNoteC4 = *cursor;
        pCmd = LOMSE_NEW CmdInsertStaffObj("(n d4 q)");
        executer.execute(&cursor, pCmd, NULL);

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

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2305)
    {
        //validate source code: start/end parenthesis
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("clef G)");

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "Missing start or end parenthesis" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2306)
    {
        //validate source code: more than one LDP elements
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)(n e4 e g+)(n c4 e g-)");

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "More than one LDP elements" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2307)
    {
        //validate source code: parenthesis missmatch
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e (stem up)");

        int result = executer.execute(&cursor, pCmd, NULL);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "Parenthesis missmatch" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2308)
    {
        //add note in second voice
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");

        executer.execute(&cursor, pCmd, NULL);
        //cout << cursor.dump_cursor();

        CHECK( pCmd->get_name() == "Insert note" );
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
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v2  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c4 q v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2309)
    {
        //undo insertion in second voice
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");
        executer.execute(&cursor, pCmd, NULL);

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
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2310)
    {
        //redo insertion in second voice
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");
        executer.execute(&cursor, pCmd, NULL);

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
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v2  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c4 q v1  p1 )" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2311)
    {
        //undo/redo when cursor repositioned (two consecutive commands but
        //with a cursor reposition before second command)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");
        executer.execute(&cursor, pCmd, NULL);
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );

        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v2  p1 )" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c4 q v1  p1 )" );
        //Fcout << pTable->dump();

        executer.undo(&cursor);    //remove note e4. Cursor points to note c4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );

        executer.redo(&cursor);    //add note e4. Cursor points to note c4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1  p1 )" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
        //cout << "cursor: " << (*cursor)->to_string() << endl;
    }

    // CmdJoinBeam ----------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, join_beam_2401)
    {
        //join beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        SelectionSet sel;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to g4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        DocCommand* pCmd = LOMSE_NEW CmdJoinBeam();

        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_full_checkpoint );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Join beam" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->is_beamed() == true );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->is_beamed() == true );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // DocCmdComposite ------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2501)
    {
        //Composite cmd: checkpoint undo
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        SelectionSet sel;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to g4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        DocCommand* pCmd1 = LOMSE_NEW CmdJoinBeam();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Join beam and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_full_checkpoint );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Join beam and change dots" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_dots() == 1 );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_dots() == 1 );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_dots() == 1 );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2502)
    {
        //Composite cmd: checkpoint undo. Undo works
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        SelectionSet sel;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to g4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        DocCommand* pCmd1 = LOMSE_NEW CmdJoinBeam();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Join beam and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor);

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->is_beamed() == false );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->is_beamed() == false );
        CHECK( pNote->get_dots() == 0 );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == false );
        CHECK( pNote->get_dots() == 0 );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2503)
    {
        //Composite cmd: checkpoint undo. Redo works
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        SelectionSet sel;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to f4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        cursor.move_next();         //points to g4
        pNote = static_cast<ImoNote*>( *cursor );
        sel.debug_add(pNote);
        DocCommand* pCmd1 = LOMSE_NEW CmdJoinBeam();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Join beam and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor);
        executer.redo(&cursor);

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_dots() == 1 );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_F );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_dots() == 1 );
        cursor.move_prev();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_dots() == 1 );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2504)
    {
        //Composite cmd: specific undo
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to e4 q
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);

        DocCommand* pCmd1 = LOMSE_NEW CmdAddTie();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Tie notes and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Tie notes and change dots" );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote2 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->get_dots() == 1 );
        cursor.move_prev();
        pNote1 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->get_dots() == 1 );
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2505)
    {
        //Composite cmd: specific undo. Undo works
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to e4 q
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);

        DocCommand* pCmd1 = LOMSE_NEW CmdAddTie();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Tie notes and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor);

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote2 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote2->is_tied_prev() == false );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->get_dots() == 0 );
        cursor.move_prev();
        pNote1 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pNote1->is_tied_next() == false );
        CHECK( pNote1->get_dots() == 0 );
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2506)
    {
        //Composite cmd: specific undo. Redo works
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e)(n e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        cursor.move_next();         //points to e4 q
        ImoNote* pNote2 = static_cast<ImoNote*>( *cursor );
        SelectionSet sel;
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);

        DocCommand* pCmd1 = LOMSE_NEW CmdAddTie();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Tie notes and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor);
        executer.redo(&cursor);

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        pNote2 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->get_dots() == 1 );
        cursor.move_prev();
        pNote1 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->get_dots() == 1 );
    }

}
