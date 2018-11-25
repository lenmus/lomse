//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
#include "lomse_ldp_exporter.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// helper class
class MySelectionSet : public SelectionSet
{
public:
    MySelectionSet(Document* pDoc) : SelectionSet(pDoc) {}
    ~MySelectionSet() {}

    void debug_add(ImoObj* pImo)
    {
        if (pImo)
        {
            m_imos.push_back(pImo);
            m_ids.push_back( pImo->get_id() );

            if (pImo->is_staffobj())
                add_staffobj_to_collection( static_cast<ImoStaffObj*>(pImo) );
        }
    }

};

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
        , m_pDoc(nullptr)
    {
    }

    ~DocCommandTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
    }

    void create_document_1()
    {
        //"(lenmusdoc#0 (vers 0.0) (content#3 "
        //    "(score#94 (vers 1.6) "
        //        "(instrument#20 (musicData#21 (clef#21 G)(key#23 C)"
        //        "(time#24 2 4)(n#25 c4 q)(r#25 q) )))"
        //    "(para#27 (txt#28 \"Hello world!\"))"
        //"))"
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score#94 (vers 2.0) "
                "(instrument#121 (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
        m_pDoc->clear_dirty();
    }

    void create_document_2()
    {
        //(score (vers 2.0)(instrument#20 (musicData#21
        //(clef#22 G)(key#23 C)(time#24 2 4)(n#25 c4 q)(r#26 q)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score#94 (vers 2.0) "
                "(instrument#121 (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "))" );
        m_pDoc->clear_dirty();
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
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
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to end of score
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK( pCmd->get_cursor_update_policy() == DocCommand::k_refresh );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( *cursor == nullptr );
        CHECK( pSC->is_at_end_of_staff() == true );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0101_ur)
    {
        //101. undo/redo

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to end of score
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        executer.redo(&cursor, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( *cursor == nullptr );
        CHECK( pSC->is_at_end_of_staff() == true );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0201)
    {
        //replace edition mode.
        //@201. new note starts at same timepos than old one. Both notes equal duration:
        //     -> replace.
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //          |
        //          +-- add t=0 (n a4 e v1)
        //  (clef G)(n a4 e v1)(n f4 e v1)(n g4 e v1)
        //                     |
        //                     +-- t=32, v=1, obj=f4 e

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e v1 p1)" );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 e v1 p1)" );
        CHECK( is_equal_time(pSC->time(), 32.0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0201_ur)
    {
        //@201. undo/redo

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v1)", k_edit_mode_replace);

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        executer.redo(&cursor, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 e v1 p1)" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e v1 p1)" );
    }

//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0202)
//    {
//        //replace edition mode.
//        //@202. new note starts in the middle of existing note. New note end at same
//        //     timepos than old note: -> reduce & insert
//        //  (clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)
//        //               |
//        //               +-- add t=48 (n a4 s v1)
//        //  (clef G)(n e4 e. v1)(n a4 s v1)(n f4 q v1)(n g4 q v1)
//        //                                 |
//        //                                 +-- t=64, v=1, obj=f4 q
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n f4 q v1 p1)" );
//        CHECK( is_equal_time(pSC->time(), 64.0) );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 s v1 p1)" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 e. v1 p1)" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0202_ur)
//    {
//        //@202. undo/redo
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        executer.undo(&cursor, &sel);
//        executer.redo(&cursor, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n f4 q v1 p1)" );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 s v1 p1)" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 e. v1 p1)" );
//    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0203)
    {
        //replace edition mode.
        //@203. new note starts at same timepos than existing one. New note is shorter than old note:
        //     -> insert & reduce
        //  (clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)
        //          |
        //          +-- add t=0 (n a4 s v1)
        //  (clef G)(n a4 s v1)(n e4 e. v1)(n f4 q v1)(n g4 q v1)
        //                     |
        //                     +-- t=16, v=1, obj=e4 e.

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 q

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 s v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e. v1 p1)" );
        CHECK( is_equal_time(pSC->time(), 16.0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 s v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0203_ur)
    {
        //@203. undo/redo.

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 q v1)(n f4 q v1)(n g4 q v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 q

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 s v1)", k_edit_mode_replace);

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        executer.redo(&cursor, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e. v1 p1)" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 5 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 s v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0204)
    {
        //replace edition mode.
        //@204. new note starts at same timepos than existing one. New note is longer than
        //     old note: -> replace many & reduce last
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //          |
        //          +-- add t=0 (n a4 e. v1)
        //  (clef G)(n a4 e. v1)(n f4 s v1)(n g4 e v1)
        //                      |
        //                      +-- t=48, v=1, obj=f4 s

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e. v1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 s v1 p1)" );
        CHECK( is_equal_time(pSC->time(), 48.0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e. v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0204_ur)
    {
        //@204. undo/redo

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e. v1)", k_edit_mode_replace);

        MySelectionSet sel(m_pDoc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        executer.redo(&cursor, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f4 s v1 p1)" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );

        cursor.move_prev();         //prev note is the inserted one
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n a4 e. v1 p1)" );
    }

//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0205)
//    {
//        //replace edition mode.
//        //@205. new note starts in the middle of existing note. New note is longer
//        //     than old note: -> reduce first, replace many & reduce last
//        //  (clef G)(n e4 q v1)(n f4 e v1)(n g4 e v1)(n b4 q v1)
//        //               |
//        //               +-- add t=16 (n a4 h v1)
//        //  (clef G)(n e4 s v1)(n a4 h v1)(n b4 e. v1)
//        //                                |
//        //                                +-- t=144, v=1, obj=b4 e.
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n b4 e. v1 p1)" );
//        CHECK( is_equal_time(pSC->time(), 144.0) );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 h v1 p1)" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 s v1 p1)" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0205_ur)
//    {
//        //@205. undo/redo.
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        executer.undo(&cursor, &sel);
//        executer.redo(&cursor, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n b4 e. v1 p1)" );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 h v1 p1)" );
//
//        cursor.move_prev();         //prev note is the original one, shortened
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n e4 s v1 p1)" );
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
        //                                            |
        //                      t=32, v=1, obj=f4 e --+

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(goFwd e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(n a4 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1 p1)" );
        //cout << pTable->dump() << endl;

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( pSC->is_at_empty_place() == false );
        CHECK( pSC->staffobj_internal()->to_string() == "(n f4 e v1 p1)" );
        CHECK( is_equal_time(pSC->time(), 32.0) );
//        cout << "cursor points to " << pSC->staffobj_internal()->to_string()
//             << " at timepos " << pSC->time() << endl;

    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0301_ur)
    {
        //301. undo/redo
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        cursor.move_next();         //points to f4 e

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n a4 e v2)", k_edit_mode_replace);

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        executer.redo(&cursor, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;

        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 6 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(goFwd e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(n a4 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1 p1)" );
        //cout << pTable->dump() << endl;

        CHECK( pCmd->get_name() == "Add note" );
        CHECK( doc.is_dirty() == true );
        //cursor points after inserted note
        CHECK( pSC->is_at_empty_place() == false );
        CHECK( pSC->staffobj_internal()->to_string() == "(n f4 e v1 p1)" );
        CHECK( is_equal_time(pSC->time(), 32.0) );
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
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        CHECK( pTable->num_lines() == 2 );
//        CHECK( pTable->num_entries() == 7 );
//        CHECK( pTable->is_anacrusis_start() == false );
//
//        ColStaffObjsIterator it = pTable->begin();
//        //              instr, staff, meas. time, line, scr
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 e v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(goFwd q v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     1, "(n g4 e v1 p1)" );
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
//        CHECK( (*cursor)->to_string() == "(n g4 e v1 p1)" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0302_ur)
//    {
//        //302. undo/redo
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        executer.undo(&cursor, &sel);
//        executer.redo(&cursor, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        CHECK( pTable->num_lines() == 2 );
//        CHECK( pTable->num_entries() == 7 );
//        CHECK( pTable->is_anacrusis_start() == false );
//
//        ColStaffObjsIterator it = pTable->begin();
//        //              instr, staff, meas. time, line, scr
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c5 e v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  32,     1, "(goFwd e v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     1, "(n a4 e v2 p1)" );
//        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 e v1 p1)" );
//        cout << pTable->dump() << endl;
//
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points at end of score
//
//        cursor.move_prev();
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n g4 e v1 p1)" );
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
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
////        CHECK( (*cursor)->is_note() == true );
////        CHECK( (*cursor)->to_string() == "(n g4 e v1 p1)" );
//        CHECK( pSC->is_at_empty_place() == true );
//        CHECK( pSC->staffobj_internal()->to_string() == "(n g4 e v1 p1)" );
//        CHECK( is_equal_time(pSC->time(), 128.0) );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 7 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 e v2 p1)" );
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
//        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        executer.execute(&cursor, pCmd, &sel);
//
//        executer.undo(&cursor, &sel);
//        executer.redo(&cursor, &sel);
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
////        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
////        cout << pSC->dump_cursor() << endl;
////        ColStaffObjs* pTable = pScore->get_staffobjs_table();
////        cout << pTable->dump();
//
//        CHECK( pCmd->get_name() == "Add note" );
//        CHECK( doc.is_dirty() == true );
//        //cursor points after inserted note
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n g4 e v1 p1)" );
//        CHECK( pScore->get_staffobjs_table()->num_entries() == 7 );
//
//        cursor.move_prev();         //prev note is the inserted one
//        CHECK( (*cursor)->is_note() == true );
//        CHECK( (*cursor)->to_string() == "(n a4 e v2 p1)" );
//
//        cursor.move_prev();         //prev note is the goFwd
//        CHECK( (*cursor)->is_gap() == true );
//        CHECK( (*cursor)->to_string() == "(goFwd e v2 p1)" );
//    }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0401)
    {
		//0401. if replaced note is beamed new note (< quarter) should continue beamed

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(key C)(time 6 8)"
            "(n e4 e v1 p1 (beam 129 +))(n g4 e v1 p1 (beam 129 =))(n c5 e v1 p1 (beam 129 -))"
            "(n c4 e v1 p1 (beam 139 +))(n e4 e v1 p1 (beam 139 =))(n g4 e v1 p1 (beam 139 -))"
            "(barline simple)"
            ")))");
        doc.clear_dirty();
        //cout << "993 test_command:   " << doc.to_string(true) << endl;
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.point_to(124L);       //points to first note

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n c4 e. v1 p1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 10 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key C)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 6 8)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 e. v1 p1 (beam 153 +))" );
        CHECK_ENTRY0(it, 0,    0,      0,  48,     0, "(n g4 s v1 p1 (beam 153 =b))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n c5 e v1 p1 (beam 153 -))" );
        CHECK_ENTRY0(it, 0,    0,      0,  96,     0, "(n c4 e v1 p1 (beam 139 +))" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(n e4 e v1 p1 (beam 139 =))" );
        CHECK_ENTRY0(it, 0,    0,      0, 160,     0, "(n g4 e v1 p1 (beam 139 -))" );
        CHECK_ENTRY0(it, 0,    0,      0, 192,     0, "(barline simple)" );
//        cout << pTable->dump() << endl;
//        cout << doc.dump_ids() << endl;
     }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0402)
    {
		//0402. if replaced note is beamed new note (> eighth) should not be beamed. Beam rearranged

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(key C)(time 2 4)"
            "(n e4 e v1 p1 (beam 129 +))(n g4 e v1 p1 (beam 129 =))"
            "(n c5 e v1 p1 (beam 129 =))(n e5 e v1 p1 (beam 129 -))"
            "(barline simple)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.point_to(125L);       //points to first note

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n c4 q v1 p1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 7 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key C)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 2 4)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n c5 e v1 p1 (beam 130 +))" );
        CHECK_ENTRY0(it, 0,    0,      0,  96,     0, "(n e5 e v1 p1 (beam 130 -))" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(barline simple)" );
        //cout << pTable->dump() << endl;
     }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0403)
    {
		//0403. if replaced note is beamed new note (> eighth) should not be beamed. Beam removed

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(key C)(time 6 8)"
            "(n e4 e v1 p1 (beam 129 +))(n g4 e v1 p1 (beam 129 =))(n c5 e v1 p1 (beam 129 -))"
            "(n c4 e v1 p1 (beam 139 +))(n e4 e v1 p1 (beam 139 =))(n g4 e v1 p1 (beam 139 -))"
            "(barline simple)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.point_to(125L);       //points to first note

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n c4 q v1 p1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 9 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key C)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 6 8)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n c5 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  96,     0, "(n c4 e v1 p1 (beam 139 +))" );
        CHECK_ENTRY0(it, 0,    0,      0, 128,     0, "(n e4 e v1 p1 (beam 139 =))" );
        CHECK_ENTRY0(it, 0,    0,      0, 160,     0, "(n g4 e v1 p1 (beam 139 -))" );
        CHECK_ENTRY0(it, 0,    0,      0, 192,     0, "(barline simple)" );
        //cout << pTable->dump() << endl;
     }

    TEST_FIXTURE(DocCommandTestFixture, add_noterest_0900)
    {
		//0900. bug. replace triplet eighth note by eight dotted

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(key C)(time 6 8)"
            "(n e4 e v1 p1 (beam 129 +))(n g4 e v1 p1 (beam 129 =))(n c5 e v1 p1 (beam 129 -))"
            "(n c4 e v1 p1 (beam 139 +))(n e4 e v1 p1 (beam 139 =))(n g4 e v1 p1 (beam 139 -))"
            "(barline simple)"
            ")))");
//        cout << test_name() << endl;
//        cout << doc.to_string(true) << endl;
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.point_to(136L);       //points to first note of second beamed group

        DocCommand* pCmd = LOMSE_NEW CmdAddNoteRest("(n d4 e. v1 p1)", k_edit_mode_replace);
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 10 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(key C)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(time 6 8)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1 (beam 129 +))" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n g4 e v1 p1 (beam 129 =))" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n c5 e v1 p1 (beam 129 -))" );
        CHECK_ENTRY0(it, 0,    0,      0,  96,     0, "(n d4 e. v1 p1 (beam 153 +))" );
        CHECK_ENTRY0(it, 0,    0,      0, 144,     0, "(n e4 s v1 p1 (beam 153 =b))" );
        CHECK_ENTRY0(it, 0,    0,      0, 160,     0, "(n g4 e v1 p1 (beam 153 -))" );
        CHECK_ENTRY0(it, 0,    0,      0, 192,     0, "(barline simple)" );
//        cout << pTable->dump() << endl;
//        cout << doc.dump_ids() << endl;
     }


    // CmdAddTie ------------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, add_tie_1001)
    {
        //add tie
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Add tie" );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdAddTuplet ---------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, add_tuplet_1101)
    {
        //add tuplet
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Add tuplet" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->is_in_tuplet() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, add_tuplet_1102)
    {
        //add tuplet error
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK( result == k_failure );
        CHECK( executer.get_error() == "Line 0. Missing or invalid tuplet type. Tuplet ignored.\n" );
//        cout << "msg=[" << executer.get_error() << "]" << endl;

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, add_tuplet_1103)
    {
        //undo add tuplet
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->is_in_tuplet() == false );
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        executer.redo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->is_in_tuplet() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdBreakBeam ---------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1201)
    {
        //1201. break beam. note + note = beam removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+)(n a4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4
        cursor.move_next();         //points to n a4
        DocCommand* pCmd = LOMSE_NEW CmdBreakBeam();

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1202)
    {
        //1202. break beam: note + beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+)(n f4 s)(n a4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4
        cursor.move_next();         //points to n f4
        DocCommand* pCmd = LOMSE_NEW CmdBreakBeam();

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1203)
    {
        //1203. break beam: beam + note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1204)
    {
        //1204. break beam: two beams
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK ( result == k_success );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Break beam" );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, break_beam_1205)
    {
        //1205. bug: undo break beam
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)"
			"(instrument#121 (musicData "
			"(clef G)(n e4 e g+)(n g4 e)(n c5 e g-)"
			"))) ))" );
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4
        cursor.move_next();         //points to n g4
        DocCommand* pCmd = LOMSE_NEW CmdBreakBeam();

//        cout << "DocCommandTest 1205. IdAssigner. Initial: " << doc.dump_ids() << endl;
//        cout << doc.id_assigner_size() << endl;
        size_t num_ids = doc.id_assigner_size();

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);
        CHECK ( result == k_success );

//        cout << "DocCommandTest 1205. IdAssigner. command: " << doc.dump_ids() << endl;
//        cout << doc.id_assigner_size() << endl;

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        cursor.move_prev();         //points to e4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_none );
        cursor.move_next();         //points to g4
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_continue );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_none );
        cursor.move_next();         //points to c5
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->is_beamed() == true );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote->get_beam_type(1) == ImoBeam::k_none );

//        cout << "DocCommandTest 1205. IdAssigner. undo  :  " << doc.dump_ids() << endl;
//        cout << doc.id_assigner_size() << endl;
        ImoObj* pImo = doc.get_pointer_to_imo(0);
        CHECK (pImo->is_document() == true );
        CHECK (num_ids == doc.id_assigner_size() );
    }

    // CmdChangeAccidentals --------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, change_accidentals_1301)
    {
        //change accidentals
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdChangeAccidentals(k_sharp);

        MySelectionSet sel(&doc);
        sel.debug_add(pNote);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Change accidentals" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote->get_notated_accidentals() == k_sharp );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_accidentals_1302)
    {
        //undo change accidentals
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_notated_accidentals() == 0 );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_notated_accidentals() == 0 );

        executer.redo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_notated_accidentals() == k_flat_flat );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_notated_accidentals() == k_flat_flat );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdChangeAttribute ---------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1401)
    {
        //change barline type
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Change barline type" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( pImo == *cursor );
        ImoBarline* pBar = static_cast<ImoBarline*>(pImo);
        CHECK( pBar->get_type() == k_barline_double );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1402)
    {
        //undo/redo change barline type
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( pImo == *cursor );
        ImoBarline* pBar = static_cast<ImoBarline*>(pImo);
        CHECK( pBar->get_type() == k_barline_simple );

        executer.redo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( pImo == *cursor );
        pBar = static_cast<ImoBarline*>(pImo);
        CHECK( pBar->get_type() == k_barline_double );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1403)
    {
        //toggle note stem
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Toggle note stem" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pImo == *cursor );
        ImoNote* pNote = static_cast<ImoNote*>(pImo);
        CHECK( pNote->get_stem_direction() == k_stem_down );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_attribute_int_1404)
    {
        //undo/redo toggle note stem
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pImo == *cursor );
        ImoNote* pNote = static_cast<ImoNote*>(pImo);
        CHECK( pNote->get_stem_direction() == k_stem_up );

        executer.redo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdChangeDots(1);

        MySelectionSet sel(&doc);
        sel.debug_add(pNote);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Change dots" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote->get_dots() == 1 );
    }

    TEST_FIXTURE(DocCommandTestFixture, change_dots_1502)
    {
        //undo change dots
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);
        executer.execute(&cursor, pCmd, &sel);      //cursor is pointing to note2

        executer.undo(&cursor, &sel);     //cursor restored to note2
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_dots() == 0 );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_dots() == 0 );

        executer.redo(&cursor, &sel);     //cursor pointing to note1 but redo restores original pos
        CHECK( doc.is_dirty() == true );
        CHECK( pNote2 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote2->get_dots() == 2 );
        cursor.move_prev();
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->get_dots() == 2 );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
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
        DocCommand* pCmd = LOMSE_NEW CmdCursor(126L);    //first note

        MySelectionSet sel(m_pDoc);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( pCmd->get_name() == "Cursor: point to" );
        CHECK( cursor.get_pointee_id() == 126L );
        CHECK( m_pDoc->is_dirty() == false );
    }

    // CmdDeleteBlockLevelObj -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_blocks_container_1701)
    {
        //1701. delete blocks container
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        DocCommand* pCmd = LOMSE_NEW CmdDeleteBlockLevelObj();

        //cout << m_pDoc->to_string(true) << endl;
        cursor.point_to(94L);   //score
        MySelectionSet sel(m_pDoc);
        executer.execute(&cursor, pCmd, &sel);
        //cout << m_pDoc->to_string(true) << endl;
        //cout << "cmd name = " << pCmd->get_name() << endl;

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_full_checkpoint );
        CHECK( pCmd->get_name() == "Delete score" );
        CHECK( m_pDoc->get_im_root()->get_num_content_items() == 1 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_blocks_container_1702)
    {
        //undo/redo delete blocks container
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteBlockLevelObj();

        cursor.point_to(94L);   //score
//        cout << "cursor: top-level=" << cursor.get_top_id()
//             << ", pointee=" << cursor.get_pointee_id()
//             << ", delegating?=" << (cursor.is_inside_terminal_node() ? "Yes" : "No")
//             << endl;
        MySelectionSet sel(m_pDoc);
        executer.execute(&cursor, pCmd, &sel);
        executer.undo(&cursor, &sel);
        CHECK( m_pDoc->get_im_root()->get_num_content_items() == 2 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_score() == true );

        executer.redo(&cursor, &sel);

        CHECK( m_pDoc->get_im_root()->get_num_content_items() == 1 );
        CHECK( m_pDoc->is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    // CmdDeleteRelation ----------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1801)
    {
        //delete relation (tuplet)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+ (t + 2 3))(n f4 e)(n g4 e g- (t -))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        ImoTuplet* pTuplet = pNote1->get_first_tuplet();
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation();

        MySelectionSet sel(&doc);
        sel.debug_add(pTuplet);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Delete tuplet" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->is_in_tuplet() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1802)
    {
        //delete relation (beam)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        sel.debug_add(pBeam);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Delete beam" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1803)
    {
        //delete relation (beam)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
        MySelectionSet sel(&doc);
        sel.debug_add(pBeam);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_beamed() == true );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == true );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == true );

        executer.redo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );
        cursor.move_next();
        CHECK( static_cast<ImoNote*>( *cursor )->is_beamed() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1804)
    {
        //delete relation (old tie)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        sel.debug_add(pTie);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );
        cursor.move_next();
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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

        executer.redo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1805)
    {
        //delete relation (new tie, notes appart)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;

        MySelectionSet sel(&doc);
        sel.debug_add(pTie);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "After delete tie:" << endl;
//        pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
//        pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        executer.undo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
//        pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        executer.redo(&cursor, &sel);
        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
//        pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_relation_1806)
    {
        //delete relation (alternative constructor)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+ (t + 2 3))(n f4 e)(n g4 e g- (t -))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 e
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        ImoTuplet* pTuplet = pNote1->get_first_tuplet();
        DocCommand* pCmd = LOMSE_NEW CmdDeleteRelation(k_imo_tuplet);

        MySelectionSet sel(&doc);
        sel.debug_add(pTuplet);
        sel.debug_add(pNote1);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Delete tuplet" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( pNote1->is_in_tuplet() == false );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // CmdDeleteSelection ---------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1901)
    {
        //delete staffobjs
        //cursor positioned at first object after first deleted one
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+)(n f4 e g-)(n g4 q)"
            ")))");

        MySelectionSet sel(&doc);
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

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );
        //cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 q v1 p1)" );

        //cursor pointing to right place
        CHECK( (*cursor)->to_string() == "(n g4 q v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1902)
    {
        //1902. delete staffobjs. First staffobj deleted
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+)(n f4 e g-)(n g4 q)"
            ")))");

        MySelectionSet sel(&doc);
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

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );
        //cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n f4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   32,     0, "(n g4 q v1 p1)" );

        //cursor pointing to right place
        CHECK( (*cursor)->to_string() == "(n f4 e v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1903)
    {
        //delete relation
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e g+)(n f4 e g-)(n g4 q)"
            ")))");

        MySelectionSet sel(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        ++cursor;       //n e4
        ImoNote* pNote = static_cast<ImoNote*>(*cursor);
        sel.debug_add(pNote->get_beam());

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteSelection();

        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacrusis_start() == false );
//        cout << doc.to_string(true) << endl;
//        cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        ImoStaffObj* pSO = (*it)->imo_object();     //it points to next: n e4
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 q v1 p1)" );

        CHECK( static_cast<ImoStaffObj*>(*cursor) == pSO );    //cursor points to n g4

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_selection_1904)
    {
        //delete auxobj
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e (text \"This is a note\"))(n f4 e)(n g4 q)"
            ")))");

        MySelectionSet sel(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        ++cursor;       //n e4
        ImoNote* pNote = static_cast<ImoNote*>(*cursor);
        sel.debug_add(pNote->get_attachment(0));

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteSelection();

        executer.execute(&cursor, pCmd, &sel);

        CHECK( doc.is_dirty() == true );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacrusis_start() == false );
//        cout << doc.to_string(true) << endl;
//        cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        ImoStaffObj* pSO = (*it)->imo_object();     //it points to next: n e4
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n f4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,  64,     0, "(n g4 q v1 p1)" );

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
        cursor.point_to(126L);
        DocCommandExecuter executer(m_pDoc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(m_pDoc);
        executer.execute(&cursor, pCmd, &sel);
        //cout << m_pDoc->to_string(true) << endl;
        //ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_im_root()->get_content_item(0) );
        //cout << pScore->get_staffobjs_table()->dump() << endl;
        //cout << "cmd name = " << pCmd->get_name() << endl;

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );
        CHECK( pCmd->get_name() == "Delete note" );
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_rest() == true );
        CHECK( m_pDoc->is_dirty() == true );

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2002)
    {
        //@2002. undo delete staffobj
        create_document_2();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        cursor.point_to(126L);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(m_pDoc);
        executer.execute(&cursor, pCmd, &sel);
        executer.undo(&cursor, &sel);

//        cout << m_pDoc->to_string(true) << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_im_root()->get_content_item(0) );
//        cout << pScore->get_staffobjs_table()->dump() << endl;
        //cout << cursor.dump_cursor();

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == 126L );
        CHECK( m_pDoc->is_dirty() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2003)
    {
        //delete beamed note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n c4 e g+)(n e4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 e v1 p1)" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2004)
    {
        //undo delete beamed note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n c4 e g+)(n e4 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
        executer.undo(&cursor, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 e v1 p1 (beam 126 +))" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 e v1 p1 (beam 126 +))" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n e4 e v1 p1 (beam 126 -))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2005)
    {
        //delete beamed note. Still two notes in beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n g5 s g+)(n f5 s)(n g5 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n g5 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n f5 s v1 p1 (beam 127 +f))" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n f5 s v1 p1 (beam 127 +f))" );
        CHECK_ENTRY0(it, 0,    0,      0,  16,     0, "(n g5 e v1 p1 (beam 127 -))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2006)
    {
        //undo delete beamed note when still two notes in beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n g5 s g+)(n f5 s)(n g5 e g-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n g5 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
        executer.undo(&cursor, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n g5 s v1 p1 (beam 127 ++))" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 4 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g5 s v1 p1 (beam 127 ++))" );
        CHECK_ENTRY0(it, 0,    0,      0,  16,     0, "(n f5 s v1 p1 (beam 127 =-))" );
        CHECK_ENTRY0(it, 0,    0,      0,  32,     0, "(n g5 e v1 p1 (beam 127 -))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2007)
    {
        //delete note in chord. Still two notes in chord
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(chord (n c4 s)(n e4 s)(n g4 s))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(chord (n e4 s v1 p1)" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(chord (n e4 s v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 s v1 p1))" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2008)
    {
        //delete note in chord. One note left: chord removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(chord (n e4 s)(n g4 s))"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n e4 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n g4 s v1 p1)" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_chord() == false );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 s v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2009)
    {
        //delete tied note. Tie removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n g4 s l+)(n g4 s l-)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n g4 s
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n g4 s v1 p1)" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        ImoNote* pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n g4 s v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2010)
    {
        //delete start note of tuplet. Tuplet removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 s v1 p1)" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_tuplet() == false );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 s v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2011)
    {
        //delete end note of tuplet. Tuplet removed
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
        CHECK( pSC->is_at_end_of_staff() == true );
        cursor.move_prev();         //points to n e4 s
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n e4 s v1 p1)" );
//        cout << "cursor: " << (*cursor)->to_string() << endl;
        pNote = static_cast<ImoNote*>( *cursor );
        CHECK( pNote->is_in_tuplet() == false );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 s v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2012)
    {
        //@2012. delete first staffobj in score
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n f4 e)(n g4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );
        //cout << pTable->dump() << endl;

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n f4 e v1 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   32,     0, "(n g4 q v1 p1)" );

        //cursor pointing to right place
        CHECK( (*cursor)->to_string() == "(n f4 e v1 p1)" );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_staffobj_2013)
    {
        //@2013. delete the only staffobj in score
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdDeleteStaffObj();

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
//        cout << doc.to_string() << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_name() == "Insert paragraph" );
        ImoDocument* pImoDoc = doc.get_im_root();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_im_root()->get_num_content_items() == 0 );
        CHECK( doc.is_dirty() == true );

        executer.redo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_im_root()->get_num_content_items() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_im_root()->get_num_content_items() == 0 );
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
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CmdInsertBlockLevelObj* pCmd2 =
            LOMSE_NEW CmdInsertBlockLevelObj(k_imo_score);

        executer.execute(&cursor, pCmd2, &sel);
        CHECK( pCmd2->get_name() == "Insert score" );
//        cout << doc.to_string() << endl;
        ImoDocument* pImoDoc = doc.get_im_root();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 2 );
        ImoObj* pScore = pContent->get_first_child();
        CHECK( pScore->get_obj_type() == k_imo_score );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_im_root();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.redo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_im_root();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 2 );
        pScore = pContent->get_first_child();
        CHECK( pScore->get_obj_type() == k_imo_score );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor, &sel);
        executer.undo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_im_root();
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

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
//        cout << doc.to_string() << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_name() == "Insert block" );
        ImoDocument* pImoDoc = doc.get_im_root();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_im_root()->get_num_content_items() == 0 );
        CHECK( doc.is_dirty() == true );

        executer.redo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_im_root()->get_num_content_items() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );
        CHECK( doc.is_dirty() == true );

        executer.undo(&cursor, &sel);
//        cout << doc.to_string() << endl;
        CHECK( doc.get_im_root()->get_num_content_items() == 0 );
        CHECK( doc.is_dirty() == true );
    }

    // CmdInsertManyStaffObjs -----------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2201)
    {
        //objects inserted and cursor updated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        DocCursor cursor(&doc);
        cursor.enter_element();
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_name() == "Insert staff objects" );
        CHECK( *cursor == nullptr );
        CHECK( doc.is_dirty() == true );

        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->is_at_end_of_empty_score() == false );
        CHECK( pSC->is_at_end_of_staff() == true );
        CHECK( pSC->time() == 64 );

        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content (score (vers 2.0)"
              "(instrument (staves 1)(musicData (clef G p1)(n e4 e v1 p1 (beam 130 +))"
              "(n c4 e v1 p1 (beam 130 -)))))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2202)
    {
        //undo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
//        cout << doc.to_string() << endl;

        executer.undo(&cursor, &sel);

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );
        CHECK( pSC->time() == 0 );
//        cout << pSC->dump_cursor() << endl;

        CHECK( doc.is_dirty() == true );
        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content (score (vers 2.0)(instrument (staves 1)(musicData)))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 0 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2203)
    {
        //redo insertion
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);

        executer.redo(&cursor, &sel);

        CHECK( *cursor == nullptr );
        CHECK( doc.is_dirty() == true );

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->is_at_end_of_empty_score() == false );
        CHECK( pSC->is_at_end_of_staff() == true );

        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content (score (vers 2.0)"
              "(instrument (staves 1)(musicData (clef G p1)(n e4 e v1 p1 (beam 139 +))"
              "(n c4 e v1 p1 (beam 139 -)))))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
//        cout << doc.to_string() << endl;
//        cout << doc.dump_ids() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_many_staffobjs_2204)
    {
        //undo/redo when cursor repositioned
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertManyStaffObjs("(clef G)(n e4 e g+)(n c4 e g-)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        pCmd = LOMSE_NEW CmdCursor(128L);     //point to first note
        executer.execute(&cursor, pCmd, &sel);
        ImoObj* pNoteE4 = *cursor;

        pCmd = LOMSE_NEW CmdInsertStaffObj("(n d4 q)");
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);    //remove note d4. Cursor points to note e4
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == pNoteE4->get_id() );

        pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_move_next);     //point note c4
        executer.execute(&cursor, pCmd, &sel);
        CHECK ( (*cursor)->is_note() );
        ImoNote* pNoteC4 = static_cast<ImoNote*>(*cursor);
        CHECK ( pNoteC4->get_fpitch() == C4_FPITCH );

        executer.undo(&cursor, &sel);    //remove initial insertions
        CHECK( *cursor == nullptr );

        executer.redo(&cursor, &sel);    //insert again all staffobjs. Cursor points to end of score
        CHECK( *cursor == nullptr );

        executer.redo(&cursor, &sel);    //insert note d4. Cursor points to end of score
        CHECK( *cursor == nullptr );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 4 );
    }

    // CmdInsertStaffObj ----------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2301)
    {
        //clef inserted and cursor updated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );

        DocCursor cursor(&doc);
        cursor.enter_element();
        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( pCmd->get_name() == "Insert clef" );
        CHECK( *cursor == nullptr );
        CHECK( doc.is_dirty() == true );

//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
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
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        DocCursor cursor(&doc);
        cursor.enter_element();
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        CHECK( pSC->is_at_end_of_empty_score() == true );
        CHECK( doc.is_dirty() == true );
        string expected = "(lenmusdoc (vers 0.0)(content (score (vers 2.0)"
            "(instrument (staves 1)(musicData)))))";
        CHECK( doc.to_string() == expected );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 0 );
//        cout << doc.to_string() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2303)
    {
        //source code validated
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clof G)");
        DocCursor cursor(&doc);
        cursor.enter_element();

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

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
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();

        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)");
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
        pCmd = LOMSE_NEW CmdInsertStaffObj("(n c4 q)");
        executer.execute(&cursor, pCmd, &sel);
        pCmd = LOMSE_NEW CmdCursor(CmdCursor::k_move_prev);     //to inserted note
        executer.execute(&cursor, pCmd, &sel);
        ImoObj* pNoteC4 = *cursor;
        pCmd = LOMSE_NEW CmdInsertStaffObj("(n d4 q)");
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);    //remove note d4. Cursor points to note c4
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == pNoteC4->get_id() );

        executer.undo(&cursor, &sel);    //remove note c4. Cursor points to end of score
        CHECK( *cursor == nullptr );

        executer.redo(&cursor, &sel);    //insert note c4. Cursor points to end of score
        CHECK( *cursor == nullptr );

        executer.redo(&cursor, &sel);    //insert note d4. Cursor points to note c4
        pNoteC4 = *cursor;
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == pNoteC4->get_id() );

        //cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2305)
    {
        //validate source code: start/end parenthesis
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("clef G)");

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "Missing start or end parenthesis" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2306)
    {
        //validate source code: more than one LDP elements
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(clef G)(n e4 e g+)(n c4 e g-)");

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "More than one LDP elements" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2307)
    {
        //validate source code: parenthesis mismatch
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#121 (musicData)))");
        doc.clear_dirty();
        DocCommandExecuter executer(&doc);
        DocCursor cursor(&doc);
        cursor.enter_element();
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e (stem up)");

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK( result == k_failure );
        //cout << "Error: '" << executer.get_error() << "'" << endl;
        CHECK( executer.get_error() == "Parenthesis mismatch" );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2308)
    {
        //add note in second voice
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        CHECK( (*cursor)->is_note() == true );

        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");

        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
        //cout << cursor.dump_cursor();

        CHECK( pCmd->get_name() == "Insert note" );
        //cout << "name: '" << pCmd->get_name() << "'" << endl;
        CHECK( doc.is_dirty() == true );

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1 p1)" );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c4 q v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2309)
    {
        //undo insertion in second voice
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);

        CHECK( doc.is_dirty() == true );

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1 p1)" );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 1 );
        CHECK( pTable->num_entries() == 2 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n c4 q v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2310)
    {
        //redo insertion in second voice
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);

        executer.redo(&cursor, &sel);

        CHECK( doc.is_dirty() == true );

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1 p1)" );
        //cout << "cursor: " << (*cursor)->to_string() << endl;

        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
        //cout << pSC->dump_cursor() << endl;
        CHECK( pSC->time() == 0 );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c4 q v1 p1)" );
//        cout << pTable->dump();
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_staffobj_2311)
    {
        //undo/redo when cursor repositioned (two consecutive commands but
        //with a cursor reposition before second command)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData (clef G)(n c4 q))))");
        doc.clear_dirty();
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        DocCursor cursor(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to n c4 q
        DocCommandExecuter executer(&doc);
        DocCommand* pCmd = LOMSE_NEW CmdInsertStaffObj("(n e4 e v2)");
        MySelectionSet sel(&doc);
        executer.execute(&cursor, pCmd, &sel);
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1 p1)" );

        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 3 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        //              instr, staff, meas. time, line, scr
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(clef G p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     0, "(n e4 e v2 p1)" );
        CHECK_ENTRY0(it, 0,    0,      0,   0,     1, "(n c4 q v1 p1)" );
        //Fcout << pTable->dump();

        executer.undo(&cursor, &sel);    //remove note e4. Cursor points to note c4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1 p1)" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 2 );

        executer.redo(&cursor, &sel);    //add note e4. Cursor points to note c4
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->to_string() == "(n c4 q v1 p1)" );
        CHECK( pScore->get_staffobjs_table()->num_entries() == 3 );
        //cout << "cursor: " << (*cursor)->to_string() << endl;
    }

    // CmdJoinBeam ----------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, join_beam_2401)
    {
        //join beam
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        MySelectionSet sel(&doc);
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
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    // DocCmdComposite ------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2501)
    {
        //Composite cmd: checkpoint undo
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        MySelectionSet sel(&doc);
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
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2502)
    {
        //Composite cmd: checkpoint undo. Undo works
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        MySelectionSet sel(&doc);
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

        executer.undo(&cursor, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2503)
    {
        //Composite cmd: checkpoint undo. Redo works
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e)(n f4 e)(n g4 e)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4
        MySelectionSet sel(&doc);
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

        executer.undo(&cursor, &sel);
        executer.redo(&cursor, &sel);

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
    }

    TEST_FIXTURE(DocCommandTestFixture, composite_cmd_2504)
    {
        //Composite cmd: specific undo
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
        MySelectionSet sel(&doc);
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
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( doc.is_dirty() == true );
        CHECK( pCmd->get_name() == "Tie notes and change dots" );
        CHECK( *cursor != nullptr );
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
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);

        DocCommand* pCmd1 = LOMSE_NEW CmdAddTie();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Tie notes and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
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
        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        sel.debug_add(pNote2);

        DocCommand* pCmd1 = LOMSE_NEW CmdAddTie();
        DocCommand* pCmd2 = LOMSE_NEW CmdChangeDots(1);
        DocCmdComposite* pCmd = LOMSE_NEW DocCmdComposite("Tie notes and change dots");
        pCmd->add_child_command(pCmd1);
        pCmd->add_child_command(pCmd2);

        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);
        executer.redo(&cursor, &sel);

//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

        CHECK( doc.is_dirty() == true );
        CHECK( *cursor != nullptr );
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

    // CmdSelection ---------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, selection_2601)
    {
        //@2601. create selection with given objects
        create_document_1();
        //        "(instrument#120 (musicData#121 (clef#122 G)(key#123 C)"
        //        "(time#124 2 4)(n#125 c4 q)(r#126 q) )))"
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        DocCommand* pCmd = LOMSE_NEW CmdSelection(CmdSelection::k_set, 126L);    //rest

        MySelectionSet sel(m_pDoc);
        executer.execute(&cursor, pCmd, &sel);

        //cout << "cmd name = " << pCmd->get_name() << endl;
        //cout << sel.dump_selection() << endl;
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( pCmd->get_name() == "Selection: set selection" );
        CHECK( sel.num_selected() == 1 );
        ImoObj* pObj = m_pDoc->get_pointer_to_imo(126L);
        CHECK( sel.contains(pObj) == true );
        CHECK( m_pDoc->is_dirty() == false );
    }

    TEST_FIXTURE(DocCommandTestFixture, selection_2602)
    {
        //@2602. add object
        create_document_1();
        //        "(instrument#20 (musicData#21 (clef#22 G)(key#23 C)"
        //        "(time#24 2 4)(n#25 c4 q)(r#26 q) )))"
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        MySelectionSet sel(m_pDoc);
        ImoObj* pObj = m_pDoc->get_pointer_to_imo(126L);     //rest
        sel.debug_add(pObj);

        DocCommand* pCmd = LOMSE_NEW CmdSelection(CmdSelection::k_add, 125L);    //first note
        executer.execute(&cursor, pCmd, &sel);

//        cout << "cmd name = " << pCmd->get_name() << endl;
//        cout << sel.dump_selection() << endl;
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( pCmd->get_name() == "Selection: add obj. to selection" );
        CHECK( sel.num_selected() == 2 );
        pObj = m_pDoc->get_pointer_to_imo(125L);
        CHECK( sel.contains(pObj) == true );
        pObj = m_pDoc->get_pointer_to_imo(126L);
        CHECK( sel.contains(pObj) == true );
        CHECK( m_pDoc->is_dirty() == false );
    }

    TEST_FIXTURE(DocCommandTestFixture, selection_2603)
    {
        //@2603. remove object
        create_document_1();
        //        "(instrument#20 (musicData#21 (clef#22 G)(key#23 C)"
        //        "(time#24 2 4)(n#25 c4 q)(r#26 q) )))"
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        MySelectionSet sel(m_pDoc);
        ImoObj* pObj = m_pDoc->get_pointer_to_imo(126L);     //rest
        sel.debug_add(pObj);
        pObj = m_pDoc->get_pointer_to_imo(125L);             //note
        sel.debug_add(pObj);

        DocCommand* pCmd = LOMSE_NEW CmdSelection(CmdSelection::k_remove, 126L);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "cmd name = " << pCmd->get_name() << endl;
//        cout << sel.dump_selection() << endl;
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( pCmd->get_name() == "Selection: remove obj. from selection" );
        CHECK( sel.num_selected() == 1 );
        pObj = m_pDoc->get_pointer_to_imo(125L);
        CHECK( sel.contains(pObj) == true );
        CHECK( m_pDoc->is_dirty() == false );
    }

    TEST_FIXTURE(DocCommandTestFixture, selection_2604)
    {
        //@2604. clear selection
        create_document_1();
        //        "(instrument#20 (musicData#21 (clef#22 G)(key#23 C)"
        //        "(time#24 2 4)(n#25 c4 q)(r#26 q) )))"
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        MySelectionSet sel(m_pDoc);
        ImoObj* pObj = m_pDoc->get_pointer_to_imo(126L);     //rest
        sel.debug_add(pObj);
        pObj = m_pDoc->get_pointer_to_imo(125L);             //note
        sel.debug_add(pObj);

        DocCommand* pCmd = LOMSE_NEW CmdSelection(CmdSelection::k_clear);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "cmd name = " << pCmd->get_name() << endl;
//        cout << sel.dump_selection() << endl;
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( pCmd->get_name() == "Selection: clear selection" );
        CHECK( sel.num_selected() == 0 );
        CHECK( m_pDoc->is_dirty() == false );
    }

    TEST_FIXTURE(DocCommandTestFixture, selection_2605)
    {
        //@2605. add object: not added if duplicate
        create_document_1();
        //        "(instrument#20 (musicData#21 (clef#22 G)(key#23 C)"
        //        "(time#24 2 4)(n#25 c4 q)(r#26 q) )))"
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        MySelectionSet sel(m_pDoc);
        ImoObj* pObj = m_pDoc->get_pointer_to_imo(126L);     //rest
        sel.debug_add(pObj);
        pObj = m_pDoc->get_pointer_to_imo(125L);             //note
        sel.debug_add(pObj);

        DocCommand* pCmd = LOMSE_NEW CmdSelection(CmdSelection::k_add, 126L);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "cmd name = " << pCmd->get_name() << endl;
//        cout << sel.dump_selection() << endl;
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );
        CHECK( pCmd->get_name() == "Selection: add obj. to selection" );
        CHECK( sel.num_selected() == 2 );
        pObj = m_pDoc->get_pointer_to_imo(125L);
        CHECK( sel.contains(pObj) == true );
        pObj = m_pDoc->get_pointer_to_imo(126L);
        CHECK( sel.contains(pObj) == true );
        CHECK( m_pDoc->is_dirty() == false );
    }


    // CmdAddChordNote ------------------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, add_chord_note_2701)
    {
        //@2701. To note in middle. Success. Added note selected. Cursor doesn't move.
        create_document_2();
        //(score#5 (vers 2.0)(instrument#21 (musicData#22
        //(clef#23 G)(key#24 C)(time#25 2 4)(n#26 c4 q)(r#27 q)
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        CHECK( m_pDoc->is_dirty() == false );
        cursor.point_to(126L);
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdAddChordNote("e4");
        CHECK( pCmd->get_cursor_update_policy() == DocCommand::k_refresh );
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_partial_checkpoint );

        MySelectionSet sel(m_pDoc);
        sel.debug_add(pNote1);
        executer.execute(&cursor, pCmd, &sel);

//        ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_im_root()->get_content_item(0) );
//        cout << m_pDoc->to_string(true) << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << cursor.dump_cursor();

        //cursor has not moved
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( pNote1 == static_cast<ImoNote*>( *cursor ) );
        CHECK( cursor.get_pointee_id() == 126L );

        //selection only contains added note
        CHECK( sel.num_selected() == 1 );
        ImoNote* pNote2 = static_cast<ImoNote*>( sel.front() );
        CHECK( pNote2->get_fpitch() == FPitch("e4") );
        CHECK( pNote2->is_in_chord() == true );

        //chord formed, document dirty
        CHECK( pNote1->is_in_chord() == true );
        CHECK( m_pDoc->is_dirty() == true );
        ImoChord* pChord = pNote2->get_chord();
        CHECK( pChord->get_num_objects() == 2 );
        CHECK( pChord->get_start_object() == pNote1 );
        CHECK( pChord->get_end_object() == pNote2 );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_chord_note_2701u)
    {
        //@2701. undo/redo
        create_document_2();
        //(score#5 (vers 2.0)(instrument#21 (musicData#22
        //(clef#23 G)(key#24 C)(time#25 2 4)(n#26 c4 q)(r#27 q)
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);
        cursor.point_to(126L);
        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        DocCommand* pCmd = LOMSE_NEW CmdAddChordNote("e4");

        MySelectionSet sel(m_pDoc);
        sel.debug_add(pNote1);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "After excute command:" << endl;
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;

        executer.undo(&cursor, &sel);

        CHECK( m_pDoc->is_dirty() == true );

//        cout << "After undo:" << endl;
//        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        pScore = static_cast<ImoScore*>( m_pDoc->get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;

        //cursor has not moved
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == 126L );

        //selection only contains base note
        CHECK( sel.num_selected() == 1 );
        ImoNote* pNote2 = static_cast<ImoNote*>( sel.front() );
        CHECK( pNote2->get_fpitch() == FPitch("c4") );
        CHECK( pNote2->is_in_chord() == false );
        CHECK( pNote2->get_id() == 126L );

        executer.redo(&cursor, &sel);

//        cout << "After redo:" << endl;
//        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        pScore = static_cast<ImoScore*>( m_pDoc->get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
        //cout << sel.dump_selection() << endl;

        //cursor has not moved
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == 126L );

        //selection only contains added note
        CHECK( sel.num_selected() == 1 );
        pNote2 = static_cast<ImoNote*>( sel.front() );
        CHECK( pNote2->get_fpitch() == FPitch("e4") );
        CHECK( pNote2->is_in_chord() == true );

        //chord formed, document dirty
        pNote1 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote1->is_in_chord() == true );
        CHECK( m_pDoc->is_dirty() == true );
        ImoChord* pChord = pNote2->get_chord();
        CHECK( pChord->get_num_objects() == 2 );
        CHECK( pChord->get_start_object() == pNote1 );
        CHECK( pChord->get_end_object() == pNote2 );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_chord_note_2702)
    {
        //@2702. To note at end. Success. Added note selected. Cursor doesn't move.
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        cursor.move_next();         //points to f4 e
        cursor.move_next();         //points to g4 e

        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        ImoId idNote1 = cursor.get_pointee_id();
        DocCommand* pCmd = LOMSE_NEW CmdAddChordNote("c5");

        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        executer.execute(&cursor, pCmd, &sel);

//        cout << "After excute command:" << endl;
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;

        executer.undo(&cursor, &sel);

        CHECK( doc.is_dirty() == true );

//        cout << "After undo:" << endl;
//        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;

        //cursor has not moved
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == idNote1 );

        //selection only contains base note
        CHECK( sel.num_selected() == 1 );
        ImoNote* pNote2 = static_cast<ImoNote*>( sel.front() );
        CHECK( pNote2->get_fpitch() == FPitch("g4") );
        CHECK( pNote2->is_in_chord() == false );
        CHECK( pNote2->get_id() == idNote1 );

        executer.redo(&cursor, &sel);

//        cout << "After redo:" << endl;
//        pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;

        //cursor has not moved
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == idNote1 );

        //selection only contains added note
        CHECK( sel.num_selected() == 1 );
        pNote2 = static_cast<ImoNote*>( sel.front() );
        CHECK( pNote2->get_fpitch() == FPitch("c5") );
        CHECK( pNote2->is_in_chord() == true );

        //chord formed, document dirty
        pNote1 = static_cast<ImoNote*>( *cursor );
        CHECK( pNote1->is_in_chord() == true );
        CHECK( doc.is_dirty() == true );
        ImoChord* pChord = pNote2->get_chord();
        CHECK( pChord->get_num_objects() == 2 );
        CHECK( pChord->get_start_object() == pNote1 );
        CHECK( pChord->get_end_object() == pNote2 );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_chord_note_2703)
    {
        //@2703. Add chord note fails. No note selected
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        cursor.move_next();         //points to f4 e
        cursor.move_next();         //points to g4 e

        ImoId idNote1 = cursor.get_pointee_id();
        DocCommand* pCmd = LOMSE_NEW CmdAddChordNote("c5");

        MySelectionSet sel(&doc);
        int result = executer.execute(&cursor, pCmd, &sel);

        CHECK( result == k_failure );
        CHECK( executer.get_error() == "Command ignored. No note selected or more than one." );

//        cout << "msg=[" << executer.get_error() << "]" << endl;
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;

        //cursor has not moved
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == idNote1 );

        //selection is empty
        CHECK( sel.num_selected() == 0 );
    }

    TEST_FIXTURE(DocCommandTestFixture, add_chord_note_2704)
    {
        //@2704. Add two notes to chord
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n c4 e v1)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to e4 e
        cursor.move_next();         //points to f4 e
        cursor.move_next();         //points to c4 e

        ImoNote* pNote1 = static_cast<ImoNote*>( *cursor );
        ImoId idNote1 = cursor.get_pointee_id();

        cursor.move_next();         //now at end of score
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );

        DocCommand* pCmd = LOMSE_NEW CmdAddChordNote("g4");

        MySelectionSet sel(&doc);
        sel.debug_add(pNote1);
        executer.execute(&cursor, pCmd, &sel);

        CHECK( sel.num_selected() == 1 );
        ImoNote* pNote2 = static_cast<ImoNote*>( sel.front() );
        CHECK( pNote2->get_fpitch() == FPitch("g4") );
        CHECK( pNote2->is_in_chord() == true );

//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;
//        LdpExporter exporter(&m_libraryScope);
//        //exporter.set_remove_newlines(true);
//        exporter.set_add_id(true);
//        cout << exporter.get_source(pScore) << endl;

        pCmd = LOMSE_NEW CmdAddChordNote("e4");
        executer.execute(&cursor, pCmd, &sel);

//        cout << "msg=[" << executer.get_error() << "]" << endl;
//        ScoreCursor* pSC = static_cast<ScoreCursor*>( cursor.get_inner_cursor() );
//        cout << pSC->dump_cursor() << endl;
//        pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << exporter.get_source(pScore) << endl;
//        cout << sel.dump_selection() << endl;

        //cursor has not moved
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );

        //selection only contains added note
        CHECK( sel.num_selected() == 1 );
        pNote2 = static_cast<ImoNote*>( sel.front() );
        CHECK( pNote2->get_fpitch() == FPitch("e4") );
        CHECK( pNote2->is_in_chord() == true );

        //chord formed, document dirty
        CHECK( doc.is_dirty() == true );
        ImoChord* pChord = pNote2->get_chord();
        CHECK( pChord->get_num_objects() == 3 );
        CHECK( pChord->get_start_object()->get_id() == idNote1 );
        CHECK( pChord->get_end_object() == pNote2 );
    }

    //@ CmdChromaticTransposition -------------------------------------------------------

    TEST_FIXTURE(DocCommandTestFixture, chromatic_transposition_2801)
    {
        //@2801. Do. Whole score, no key transposition. Success. Cursor doesn't move.
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(key B-)(n c4 q)(n =e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        MySelectionSet sel(&doc);

        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to key
        ImoId idKey = cursor.get_pointee_id();
        cursor.move_next();         //points to c4 q
        sel.debug_add( *cursor );
        cursor.move_next();         //points to =e4 q
        sel.debug_add( *cursor );
        ImoId idCur = cursor.get_pointee_id();

        DocCommand* pCmd = LOMSE_NEW
            CmdChromaticTransposition(2, k_do_not_change_key);
        CHECK( pCmd->get_name() == "Chromatic transposition" );
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );

        executer.execute(&cursor, pCmd, &sel);

//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;
//        LdpExporter exporter(&m_libraryScope);
//        //exporter.set_remove_newlines(true);
//        exporter.set_add_id(true);
//        cout << exporter.get_source(pScore) << endl;

        //selection unchanged
        CHECK( sel.num_selected() == 2 );

        //cursor has not moved
        CHECK( cursor.get_pointee_id() == idCur );

        //key signature is not changed
        cursor.point_to(idKey);   //key

        //the score is transposed
        CHECK( doc.is_dirty() == true );
        DocCursor c(&doc);
        c.enter_element();     //points to clef
        c.move_next();         //points to key
        c.move_next();         //points to c4 (now transposed to d4)
        ImoNote* pNote = dynamic_cast<ImoNote*>( c.get_pointee() );
        CHECK( pNote && pNote->get_step() == k_step_D );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_actual_accidentals() == 0.0f );
        c.move_next();         //points to =e4 (now transposed to +f4)
        pNote = dynamic_cast<ImoNote*>( c.get_pointee() );
        CHECK( pNote && pNote->get_step() == k_step_F );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_actual_accidentals() == 1.0f );
    }

    TEST_FIXTURE(DocCommandTestFixture, chromatic_transposition_2802)
    {
        //@2802. Undo. Whole score, no key transposition. Cursor doesn't move.
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(key B-)(n c4 q)(n =e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        MySelectionSet sel(&doc);

        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to key
        ImoId idKey = cursor.get_pointee_id();
        cursor.move_next();         //points to c4 q
        sel.debug_add( *cursor );
        cursor.move_next();         //points to =e4 q
        sel.debug_add( *cursor );
        ImoId idCur = cursor.get_pointee_id();

        DocCommand* pCmd = LOMSE_NEW
            CmdChromaticTransposition(2, k_do_not_change_key);
        CHECK( pCmd->get_name() == "Chromatic transposition" );
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );

        executer.execute(&cursor, pCmd, &sel);

        executer.undo(&cursor, &sel);

//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;
//        LdpExporter exporter(&m_libraryScope);
//        //exporter.set_remove_newlines(true);
//        exporter.set_add_id(true);
//        cout << exporter.get_source(pScore) << endl;

        //selection unchanged
        CHECK( sel.num_selected() == 2 );

        //cursor has not moved
        CHECK( cursor.get_pointee_id() == idCur );

        //key signature is not changed
        cursor.point_to(idKey);   //key

        //the score is transposed
        CHECK( doc.is_dirty() == true );
        DocCursor c(&doc);
        c.enter_element();     //points to clef
        c.move_next();         //points to key
        c.move_next();         //points to d4 (now transposed back to c4)
        ImoNote* pNote = dynamic_cast<ImoNote*>( c.get_pointee() );
        CHECK( pNote && pNote->get_step() == k_step_C );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_actual_accidentals() == 0.0f );
        c.move_next();         //points to +f4 (now transposed back to e4)
        pNote = dynamic_cast<ImoNote*>( c.get_pointee() );
        CHECK( pNote && pNote->get_step() == k_step_E );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_actual_accidentals() == 0.0f );
    }

    TEST_FIXTURE(DocCommandTestFixture, chromatic_transposition_2803)
    {
        //@2803. Do. Whole score, key transposition. Cursor doesn't move.
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument#121 (musicData "
            "(clef G)(key B-)(n c4 q)(n =e4 q)"
            ")))");
        doc.clear_dirty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        MySelectionSet sel(&doc);

        cursor.enter_element();     //points to clef
        cursor.move_next();         //points to key
        ImoId idKey = cursor.get_pointee_id();
        cursor.move_next();         //points to c4 q
        sel.debug_add( *cursor );
        cursor.move_next();         //points to =e4 q
        sel.debug_add( *cursor );
        ImoId idCur = cursor.get_pointee_id();

        DocCommand* pCmd = LOMSE_NEW
            CmdChromaticTransposition(2, k_change_key);
        CHECK( pCmd->get_name() == "Chromatic transposition" );
        CHECK( pCmd->get_undo_policy() == DocCommand::k_undo_policy_specific );

        executer.execute(&cursor, pCmd, &sel);

//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();
//        cout << sel.dump_selection() << endl;
//        LdpExporter exporter(&m_libraryScope);
//        //exporter.set_remove_newlines(true);
//        exporter.set_add_id(true);
//        cout << exporter.get_source(pScore) << endl;

        //selection unchanged
        CHECK( sel.num_selected() == 2 );

        //cursor has not moved
        CHECK( cursor.get_pointee_id() == idCur );

        //key signature is not changed
        cursor.point_to(idKey);   //key

        //the score is transposed
        CHECK( doc.is_dirty() == true );
        DocCursor c(&doc);
        c.enter_element();     //points to clef
        c.move_next();         //points to key B- (now transposed to C)
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( c.get_pointee() );
        CHECK( pKey && pKey->get_key_type() == k_key_C );
        c.move_next();         //points to c4 (now transposed to d4)
        ImoNote* pNote = dynamic_cast<ImoNote*>( c.get_pointee() );
        CHECK( pNote && pNote->get_step() == k_step_D );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_actual_accidentals() == 0.0f );
        c.move_next();         //points to =e4 (now transposed to +f4)
        pNote = dynamic_cast<ImoNote*>( c.get_pointee() );
        CHECK( pNote && pNote->get_step() == k_step_F );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_actual_accidentals() == 1.0f );
    }

}
