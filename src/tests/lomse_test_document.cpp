//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
#include "lomse_ldp_parser.h"
#include "lomse_ldp_compiler.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_events.h"
#include "lomse_document_iterator.h"

#include <exception>
using namespace UnitTest;
using namespace std;
using namespace lomse;


////helper, to get last id
//class TestDocument : public Document
//{
//public:
//    TestDocument(LibraryScope& libraryScope, ostream& reporter=cout)
//        : Document(libraryScope, reporter) {}
//
//        long test_last_id() { return m_pIdAssigner->get_last_id(); }
//};



class DocumentTestFixture
{
public:

    LibraryScope m_libraryScope;
    LdpFactory* m_pLdpFactory;
    std::string m_scores_path;

    DocumentTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_pLdpFactory = m_libraryScope.ldp_factory();
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~DocumentTestFixture()    //TearDown fixture
    {
    }
};

SUITE(DocumentTest)
{

    TEST_FIXTURE(DocumentTestFixture, DocumentEmpty)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
        CHECK( pImoDoc->get_content_item(0) == NULL );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentFromFile)
    {
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData)))))" );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentFromFile_lmd)
    {
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "30001-paragraph.lmd", Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentFromString)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q))))))");
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( pImoDoc->get_language() == "en" );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (n c4 q))))))" );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentFromString_lmd)
    {
        Document doc(m_libraryScope);
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Credits</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content><para style='Credits'>Hello world!</para></content>"
            "</lenmusdoc>";
        doc.from_string(src, Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );

        ImoDocument* pDoc = doc.get_imodoc();
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        ImoStyle* pStyle = pPara->get_style();
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Credits" );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentFromInput)
    {
        Document doc(m_libraryScope);
        LdpFileReader reader(m_scores_path + "00011-empty-fill-page.lms");
        doc.from_input(reader);
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData)))))" );
    }

    TEST_FIXTURE(DocumentTestFixture, DoubleCreationThrows_Empty)
    {
        bool fOk = false;
        Document doc(m_libraryScope);
        doc.create_empty();
        try
        {
            doc.create_empty();
        }
        catch(std::exception& e)
        {
            //cout << e.what() << endl;
            e.what();
            fOk = true;
        }
        CHECK( fOk );
    }

    TEST_FIXTURE(DocumentTestFixture, DoubleCreationThrows_FromString)
    {
        bool fOk = false;
        Document doc(m_libraryScope);
        doc.create_empty();
        try
        {
            doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
                "(instrument (musicData (n c4 q))))))");
        }
        catch(std::exception& e)
        {
            //cout << e.what() << endl;
            e.what();
            fOk = true;
        }
        CHECK( fOk );
    }

    TEST_FIXTURE(DocumentTestFixture, DoubleCreationThrows_FromFile)
    {
        bool fOk = false;
        Document doc(m_libraryScope);
        doc.create_empty();
        try
        {
            doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        }
        catch(std::exception& e)
        {
            //cout << e.what() << endl;
            e.what();
            fOk = true;
        }
        CHECK( fOk );
    }

    TEST_FIXTURE(DocumentTestFixture, DoubleCreationThrows_WithEmptyScore)
    {
        bool fOk = false;
        Document doc(m_libraryScope);
        doc.create_empty();
        try
        {
            doc.create_with_empty_score();
        }
        catch(std::exception& e)
        {
            //cout << e.what() << endl;
            e.what();
            fOk = true;
        }
        CHECK( fOk );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentGetScoreInEmptyDoc)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoScore* pScore = doc.get_score(0);
        CHECK( pScore == NULL );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentGetScoreFromFile)
    {
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        ImoScore* pScore = doc.get_score(0);
        CHECK( pScore != NULL );
//        CHECK( pScore.to_string() == "(score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData )))" );
    }

    TEST_FIXTURE(DocumentTestFixture, Document_CreateWithEmptyScore)
    {
        Document doc(m_libraryScope);
        doc.create_with_empty_score();
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
        ImoScore* pScore = doc.get_score(0);
        CHECK( pScore != NULL );
    }

    // Document::iterator ---------------------------------------------------------------------------

//    TEST_FIXTURE(DocumentTestFixture, DocumentIteratorAdvance)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        ++it;
//        CHECK( doc.to_string(it) == "(vers 0.0)" );
//        ++it;
//        CHECK( doc.to_string(it) == "0.0" );
//        ++it;
//        CHECK( doc.to_string(it) == "(content )" );
//        ++it;
//        CHECK( it == doc.end() );
//    }
//
//    TEST_FIXTURE(DocumentTestFixture, DocumentIteratorGoBack)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.begin();
//        ++it;
//        ++it;
//        ++it;
//        CHECK( doc.to_string(it) == "(content )" );
//        --it;
//        CHECK( doc.to_string(it) == "(vers 0.0)" );
//        --it;
//        CHECK( doc.to_string(it) == "(lenmusdoc (vers 0.0) (content ))" );
//    }
//
//    TEST_FIXTURE(DocumentTestFixture, DocumentGetContent)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        Document::iterator it = doc.content();
//        CHECK( doc.to_string(it) == "(content )" );
//    }
//
//      // commands ---------------------------------------------------------------------------------
//
//    TEST_FIXTURE(DocumentTestFixture, DocumentPushBack)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentRemoveParam)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentInsertParam)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentHasImObjs)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q)(n b3 e.)(n c4 s)))) ))" );
//        Document::iterator it = doc.get_score(0);
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentPushBackCommandIsStored)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoPushBackCommand)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRedoPushBackCommand)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRedoUndoPushBackCommand)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentRemoveCommandIsStored)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRemoveCommand)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentRedoRemoveCommand)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRedoUndoRemoveCommand)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentInsertCommandIsStored)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoInsertCommandIsStored)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRedoInsertCommandIsStored)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRedoUndoInsertCommandIsStored)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentRemoveNotLast)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRemoveNotLast)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRedoRemoveNotLast)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoRedoUndoRemoveNotLast)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentRemoveFirst)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentNodesMarkedAsModified)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentUndoNodesMarkedAsModified)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentKnowsLastId_FromString)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 9L );
//    }
//
//    TEST_FIXTURE(DocumentTestFixture, DocumentKnowsLastId_Empty)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.create_empty();
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 2L );
//    }
//
//    TEST_FIXTURE(DocumentTestFixture, DocumentKnowsLastId_FromFile)
//    {
//        TestDocument doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms");
//        //cout << "Last Id = " << doc.test_last_id() << endl;
//        CHECK( doc.test_last_id() == 48L );
//    }
//
//    TEST_FIXTURE(DocumentTestFixture, DocumentInsertCommandUpdatesId)
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
//    TEST_FIXTURE(DocumentTestFixture, DocumentPushBackCommandUpdatesId)
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

    // dirty bit ------------------------------------------------------------------------

    TEST_FIXTURE(DocumentTestFixture, clear_dirty)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q))))))");
        CHECK( doc.is_dirty() == true );

        doc.clear_dirty();

        CHECK( doc.is_dirty() == false );
    }

//    TEST_FIXTURE(DocumentTestFixture, initially_is_modified)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (n c4 q))))))");
//
//        ImoDocument* pImoDoc = doc.get_imodoc();
//        CHECK( pImoDoc != NULL );
//        CHECK( pImoDoc->get_owner() == &doc );
//        CHECK( doc.is_dirty() == false );
////        cout << doc.to_string() << endl;
////        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
////            "(instrument (musicData (n c4 q))))))" );
//    }

};

