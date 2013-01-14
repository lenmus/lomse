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
#include "lomse_id_assigner.h"
#include "lomse_document.h"
#include "lomse_ldp_parser.h"
#include "lomse_ldp_compiler.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_events.h"
#include "lomse_document_iterator.h"
#include "lomse_im_factory.h"

#include <exception>
using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// IdAssigner tests
//=======================================================================================

//helper, to access protected members
class MyIdAssigner : public IdAssigner
{
public:
    MyIdAssigner() : IdAssigner() {}

        long my_get_last_id() { return m_idCounter; }
};

//---------------------------------------------------------------------------------------
class IdAssignerTestFixture
{
public:

    IdAssignerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pImo(NULL)
    {
    }

    ~IdAssignerTestFixture()    //TearDown fixture
    {
        delete m_pImo;
    }

    void create_imo()
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        m_pImo = ImFactory::inject(k_imo_clef, &doc);
        m_pImo->set_id(-1L);
    }

    LibraryScope m_libraryScope;
    std::string m_scores_path;
    ImoObj* m_pImo;

};

//---------------------------------------------------------------------------------------
SUITE(IdAssignerTest)
{

    TEST_FIXTURE(IdAssignerTestFixture, initial_value)
    {
        MyIdAssigner ida;
        CHECK( ida.my_get_last_id() == -1L );
    }

    TEST_FIXTURE(IdAssignerTestFixture, assigns_id)
    {
        MyIdAssigner ida;
        create_imo();
        CHECK( m_pImo->get_id() == -1L );

        ida.assign_id(m_pImo);
        CHECK( m_pImo->get_id() == 0L );
        delete m_pImo;

        create_imo();
        ida.assign_id(m_pImo);
        CHECK( m_pImo->get_id() == 1L );
    }

    TEST_FIXTURE(IdAssignerTestFixture, stores_id)
    {
        MyIdAssigner ida;
        create_imo();
        ida.assign_id(m_pImo);

        CHECK( ida.get_pointer_to_imo(0L) == m_pImo );
    }

    TEST_FIXTURE(IdAssignerTestFixture, id_not_found)
    {
        MyIdAssigner ida;
        CHECK( ida.get_pointer_to_imo(7L) == NULL );
    }

    TEST_FIXTURE(IdAssignerTestFixture, removes_id)
    {
        MyIdAssigner ida;
        create_imo();
        ida.assign_id(m_pImo);
        CHECK( ida.get_pointer_to_imo(0L) == m_pImo );

        ida.remove(m_pImo);
        CHECK( ida.get_pointer_to_imo(0L) == NULL );
    }
};



//=======================================================================================
// Document tests
//=======================================================================================

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
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~DocumentTestFixture()    //TearDown fixture
    {
    }
};

//---------------------------------------------------------------------------------------
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
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore == NULL );
    }

    TEST_FIXTURE(DocumentTestFixture, DocumentGetScoreFromFile)
    {
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
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
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore != NULL );
    }

    // Document: id_assigner ------------------------------------------------------------

    TEST_FIXTURE(DocumentTestFixture, locate_imo_by_id)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q))))))");
//        cout << doc.to_string(k_save_ids) << endl;
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( doc.get_pointer_to_imo(15L) == pScore );
    }

//    TEST_FIXTURE(DocumentTestFixture, deleting_imo_removes_it_from_table)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (n c4 q))))))");
//        //cout << doc.to_string(k_save_ids) << endl;
//        imoDocument* pImoDoc = doc.get_imodoc();
//        pImoDoc->add_
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        CHECK( pScore != NULL );
//        CHECK( doc.get_pointer_to_imo(15L) == pScore );
//    }

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

