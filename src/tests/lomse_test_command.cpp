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
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
    }

    void create_document_2()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "))" );
    }

//    void point_cursors_1(Document* pDoc, DocCursor* pCursor1, DocCursor* pCursor2)
//    {
//        DocCursor cursor(pDoc);     //score
//        cursor.enter_element();     //(clef G)
//        ++cursor;       //(key e)
//        *pCursor1 = cursor;
//        ++cursor;       //(n c4 q)
//        ++cursor;       //(r q)
//        *pCursor2 = cursor;
//    }
//
//    void point_cursors_2(Document* pDoc, DocCursor* pCursor1, DocCursor* pCursor2)
//    {
//        DocCursor cursor(pDoc);     //score
//        *pCursor1 = cursor;
//        ++cursor;       //(text "this is a text")
//        ++cursor;       //(text "to be removed")
//        *pCursor2 = cursor;
//    }
//
//    static void my_callback_method(Notification* event)
//    {
//        m_fNotified = true;
//    }

    LibraryScope m_libraryScope;
    std::string m_scores_path;
    Document* m_pDoc;
//    static bool m_fNotified;
};

SUITE(DocCommandTest)
{

    TEST_FIXTURE(DocCommandTestFixture, push_back_blocks_container_1)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        CmdInsertBlockLevelObj* pCmd =
            LOMSE_NEW CmdInsertBlockLevelObj(cursor, k_imo_para);
        CHECK( pCmd->get_name() == "Insert paragraph" );

        executer.execute(pCmd);
//        cout << doc.to_string() << endl;
//        cout << "cmd name = " << pCmd->get_name() << endl;
        ImoDocument* pImoDoc = doc.get_imodoc();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );

        executer.undo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 0 );

        executer.redo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );

        executer.undo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 0 );
    }

    TEST_FIXTURE(DocCommandTestFixture, insert_blocks_container_1)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        DocCursor cursor(&doc);
        DocCommandExecuter executer(&doc);
        CmdInsertBlockLevelObj* pCmd =
            LOMSE_NEW CmdInsertBlockLevelObj(cursor, k_imo_para);
        executer.execute(pCmd);

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CmdInsertBlockLevelObj* pCmd2 =
            LOMSE_NEW CmdInsertBlockLevelObj(cursor, k_imo_score);
        CHECK( pCmd2->get_name() == "Insert score" );

        executer.execute(pCmd2);
//        cout << doc.to_string() << endl;
        ImoDocument* pImoDoc = doc.get_imodoc();
        ImoObj* pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 2 );
        ImoObj* pScore = pContent->get_first_child();
        CHECK( pScore->get_obj_type() == k_imo_score );

        executer.undo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 1 );
        CHECK( pContent->get_first_child()->get_obj_type() == k_imo_para );

        executer.redo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 2 );
        pScore = pContent->get_first_child();
        CHECK( pScore->get_obj_type() == k_imo_score );

        executer.undo();
        executer.undo();
//        cout << doc.to_string() << endl;
        pImoDoc = doc.get_imodoc();
        pContent = pImoDoc->get_child_of_type(k_imo_content);
        CHECK( pContent->get_num_children() == 0 );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_note_1)
    {
        create_document_1();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);

        cursor.point_to(24L);
        CmdDeleteStaffObj* pCmd =
            LOMSE_NEW CmdDeleteStaffObj(cursor);
        //cout << "cmd name = " << pCmd->get_name() << endl;
        CHECK( pCmd->get_name() == "Delete note" );

        executer.execute(pCmd);
        //cout << m_pDoc->to_string(k_save_ids) << endl;
        //ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        //cout << pScore->get_staffobjs_table()->dump() << endl;

        cursor.update_after_deletion();
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_rest() == true );
    }

    TEST_FIXTURE(DocCommandTestFixture, delete_note_1_undo)
    {
        create_document_2();
        DocCursor cursor(m_pDoc);
        DocCommandExecuter executer(m_pDoc);

        cursor.point_to(24L);
        CmdDeleteStaffObj* pCmd =
            LOMSE_NEW CmdDeleteStaffObj(cursor);

//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        ImoScore* pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//        cout << pScore->get_staffobjs_table()->dump() << endl;

        executer.execute(pCmd);
        executer.undo();

//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//        cout << pScore->get_staffobjs_table()->dump() << endl;

        cursor.update_after_deletion();
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( (*cursor)->get_id() == 24L );
    }

//    TEST_FIXTURE(DocCommandTestFixture, DocumentPushBack)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.content();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(text ''Title of this book'')");
//        LdpElement* elm = tree->get_root();
//        doc.add_param(it, elm);
//        //cout << doc.to_string(it) << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (text \"Title of this book\")))" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentRemoveParam)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        LdpElement* elm = doc.remove(it);
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (content ))" );
//        CHECK( doc.is_dirty() == false );
//        delete elm;
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentInsertParam)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        doc.insert(it, elm);
//        //cout << doc.to_string() << endl;
//        CHECK( doc.is_dirty() == false );
//        CHECK( doc.to_string() == "(lenmusdoc (dx 20) (vers 0.0) (content ))" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentHasImObjs)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q)(n b3 e.)(n c4 s)))) ))" );
//        Document::iterator it = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (n b3 e.) (n c4 s))))))" );
//        ImScore* pScore = dynamic_cast<ImScore*>( (*it)->get_imobj() );
//        CHECK( pScore != NULL );
//        ++it;
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(vers 1.6)" );
//        ++it;
//        ++it;
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (n b3 e.) (n c4 s)))" );
//        ImInstrument* pInstr = dynamic_cast<ImInstrument*>( (*it)->get_imobj() );
//        CHECK( pInstr != NULL );
//        ++it;
//        CHECK( (*it)->to_string() == "(musicData (n c4 q) (n b3 e.) (n c4 s))" );
//        ++it;
//        CHECK( (*it)->to_string() == "(n c4 q)" );
//        ImNote* pNote = dynamic_cast<ImNote*>( (*it)->get_imobj() );
//        CHECK( pNote != NULL );
//        ++it;
//        ++it;
//        ++it;
//        CHECK( (*it)->to_string() == "(n b3 e.)" );
//        pNote = dynamic_cast<ImNote*>( (*it)->get_imobj() );
//        CHECK( pNote != NULL );
//        ++it;
//        ++it;
//        ++it;
//        CHECK( (*it)->to_string() == "(n c4 s)" );
//        pNote = dynamic_cast<ImNote*>( (*it)->get_imobj() );
//        CHECK( pNote != NULL );
//    }
//
//    // undo/redo --------------------------------------------------------
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentPushBackCommandIsStored)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(text ''Title of this book'')");
//        LdpElement* elm = tree->get_root();
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandPushBack(it, elm) );
//        CHECK( ce.undo_stack_size() == 1 );
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ) (text \"Title of this book\"))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoPushBackCommand)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(text ''Title of this book'')");
//        LdpElement* elm = tree->get_root();
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandPushBack(it, elm) );
//        ce.undo();
//        CHECK( ce.undo_stack_size() == 0 );
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == false );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRedoPushBackCommand)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(text ''Title of this book'')");
//        LdpElement* elm = tree->get_root();
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandPushBack(it, elm) );
//        ce.undo();
//        ce.redo();
//        CHECK( ce.undo_stack_size() == 1 );
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ) (text \"Title of this book\"))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRedoUndoPushBackCommand)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(text ''Title of this book'')");
//        LdpElement* elm = tree->get_root();
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandPushBack(it, elm) );
//        ce.undo();
//        ce.redo();
//        ce.undo();
//        //cout << doc.to_string() << endl;
//        CHECK( ce.undo_stack_size() == 0 );
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == false );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentRemoveCommandIsStored)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(it) );
//        CHECK( ce.undo_stack_size() == 1 );
//        CHECK( doc.to_string() == "(lenmusdoc (content ))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRemoveCommand)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(it) );
//        ce.undo();
//        CHECK( ce.undo_stack_size() == 0 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == false );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentRedoRemoveCommand)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(it) );
//        ce.undo();
//        ce.redo();
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (content ))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRedoUndoRemoveCommand)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(it) );
//        ce.undo();
//        ce.redo();
//        ce.undo();
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == false );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentInsertCommandIsStored)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandInsert(it, elm) );
//        CHECK( ce.undo_stack_size() == 1 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (dx 20) (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoInsertCommandIsStored)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandInsert(it, elm) );
//        ce.undo();
//        CHECK( ce.undo_stack_size() == 0 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == false );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRedoInsertCommandIsStored)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandInsert(it, elm) );
//        ce.undo();
//        ce.redo();
//        CHECK( ce.undo_stack_size() == 1 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (dx 20) (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRedoUndoInsertCommandIsStored)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandInsert(it, elm) );
//        ce.undo();
//        ce.redo();
//        ce.undo();
//        CHECK( ce.undo_stack_size() == 0 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == false );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentRemoveNotLast)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(musicData (n c4 q)(r e)(n b3 e)(dx 20))");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        Document::iterator itNew = doc.insert(it, elm);
//        CHECK( doc.to_string() == "(lenmusdoc (musicData (n c4 q) (r e) (n b3 e) (dx 20)) (vers 0.0) (content ))" );
//        CHECK( doc.to_string( itNew ) == "(musicData (n c4 q) (r e) (n b3 e) (dx 20))" );
//        ++itNew;    //n c4
//        ++itNew;    //c4
//        ++itNew;    //q
//        ++itNew;    //r
//        CHECK( doc.to_string( itNew ) == "(r e)" );
//
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(itNew) );
//        CHECK( ce.undo_stack_size() == 1 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (musicData (n c4 q) (n b3 e) (dx 20)) (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRemoveNotLast)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        Document::iterator itNew = doc.insert(it, elm);
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(itNew) );
//        ce.undo();
//        CHECK( ce.undo_stack_size() == 0 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (dx 20) (vers 0.0) (content ))" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRedoRemoveNotLast)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        Document::iterator itNew = doc.insert(it, elm);
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(itNew) );
//        ce.undo();
//        ce.redo();
//        CHECK( ce.undo_stack_size() == 1 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content ))" );
//        CHECK( doc.is_dirty() == true );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoRedoUndoRemoveNotLast)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        Document::iterator itNew = doc.insert(it, elm);
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(itNew) );
//        ce.undo();
//        ce.redo();
//        ce.undo();
//        CHECK( ce.undo_stack_size() == 0 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (dx 20) (vers 0.0) (content ))" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentRemoveFirst)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(musicData (n c4 q)(r e)(n b3 e)(dx 20))");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        Document::iterator itNew = doc.insert(it, elm);
//        CHECK( doc.to_string() == "(lenmusdoc (musicData (n c4 q) (r e) (n b3 e) (dx 20)) (vers 0.0) (content ))" );
//        CHECK( doc.to_string( itNew ) == "(musicData (n c4 q) (r e) (n b3 e) (dx 20))" );
//        ++itNew;    //n c4
//        CHECK( doc.to_string( itNew ) == "(n c4 q)" );
//
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(itNew) );
//        CHECK( ce.undo_stack_size() == 1 );
//        //cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (musicData (r e) (n b3 e) (dx 20)) (vers 0.0) (content ))" );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentNodesMarkedAsModified)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))");
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        ++it;   //0.0
//        ++it;   //content
//        ++it;   //score
//        ++it;   //vers
//        ++it;   //1.6
//        ++it;   //instrument
//        ++it;   //musicData
//        ++it;   //clef
//        ++it;   //G
//        ++it;   //key
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(it) );
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G) (n c4 q) (r q) (barline simple))))))" );
//        it = doc.begin();   //lenmusdoc
//        CHECK( (*it)->is_dirty() );
//        ++it;   //vers
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //0.0
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //content
//        CHECK( (*it)->is_dirty() );
//        ++it;   //score
//        CHECK( (*it)->is_dirty() );
//        ++it;   //vers
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //1.6
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //instrument
//        CHECK( (*it)->is_dirty() );
//        ++it;   //musicData
//        CHECK( (*it)->is_dirty() );
//        ++it;   //clef
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //G
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //n
//        CHECK( !(*it)->is_dirty() );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentUndoNodesMarkedAsModified)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))");
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        ++it;   //0.0
//        ++it;   //content
//        ++it;   //score
//        ++it;   //vers
//        ++it;   //1.6
//        ++it;   //instrument
//        ++it;   //musicData
//        ++it;   //clef
//        ++it;   //G
//        ++it;   //key
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandRemove(it) );
//        ce.undo();
//        it = doc.begin();   //lenmusdoc
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //vers
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //0.0
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //content
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //score
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //vers
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //1.6
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //instrument
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //musicData
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //clef
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //G
//        CHECK( !(*it)->is_dirty() );
//        ++it;   //key
//        CHECK( !(*it)->is_dirty() );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentKnowsLastId_FromString)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 9L );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentKnowsLastId_Empty)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.create_empty();
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 2L );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentKnowsLastId_FromFile)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms");
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 48L );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentInsertCommandUpdatesId)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.create_empty();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(dx 20)");
//        LdpElement* elm = tree->get_root();
//        Document::iterator it = doc.begin();
//        ++it;   //vers
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandInsert(it, elm) );
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 3L );
//    }
//
//    TEST_FIXTURE(DocCommandTestFixture, DocumentPushBackCommandUpdatesId)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        LdpParser parser(cout, m_pLdpFactory);
//        SpLdpTree tree = parser.parse_text("(text ''Title of this book'')");
//        LdpElement* elm = tree->get_root();
//        DocCommandExecuter ce(&doc);
//        ce.execute( new DocCommandPushBack(it, elm) );
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 3L );
//    }

}
