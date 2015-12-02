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
class IdAssignerTestFixture
{
public:

    IdAssignerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
    }

    ~IdAssignerTestFixture()    //TearDown fixture
    {
    }

    LibraryScope m_libraryScope;

};

//---------------------------------------------------------------------------------------
SUITE(IdAssignerTest)
{

    TEST_FIXTURE(IdAssignerTestFixture, assigns_id)
    {
        Document doc(m_libraryScope);
        ImoObj* pImo1 = ImFactory::inject(k_imo_clef, &doc);
        CHECK( pImo1->get_id() == 0L );

        ImoObj* pImo2 = ImFactory::inject(k_imo_key_signature, &doc);
        CHECK( pImo2->get_id() == 1L );

        delete pImo1;
        delete pImo2;
    }

    TEST_FIXTURE(IdAssignerTestFixture, stores_id)
    {
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_clef, &doc);

        CHECK( pImo->get_id() == 0L );
        CHECK( doc.get_pointer_to_imo(0L) == pImo );

        delete pImo;
    }

    TEST_FIXTURE(IdAssignerTestFixture, id_not_found)
    {
        Document doc(m_libraryScope);
        CHECK( doc.get_pointer_to_imo(7L) == NULL );
    }

    TEST_FIXTURE(IdAssignerTestFixture, removes_id)
    {
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_clef, &doc);
        CHECK( doc.get_pointer_to_imo(0L) == pImo );

        doc.removed_from_model(pImo);
        CHECK( doc.get_pointer_to_imo(0L) == NULL );
        delete pImo;
    }
};



//=======================================================================================
// Document tests
//=======================================================================================

class DocumentTestFixture
{
public:

    DocumentTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(NULL)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~DocumentTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
        m_pDoc = NULL;
    }

    void create_document_1()
    {
        //"(lenmusdoc#0 (vers 0.0) (content#3 "
        //    "(score#4 (vers 2.0) "
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
    }

    LibraryScope m_libraryScope;
    string m_scores_path;
    Document* m_pDoc;
};

//---------------------------------------------------------------------------------------
SUITE(DocumentTest)
{

    TEST_FIXTURE(DocumentTestFixture, creation_000)
    {
        //000. create_empty does create a valid empty document
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
        CHECK( pImoDoc->get_content_item(0) == NULL );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_001)
    {
        //001. from_file. valid file, ldp format
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData)))))" );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_002)
    {
        //002. from_file. valid file, lmd format
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "08011-paragraph.lmd", Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_003)
    {
        //003. from string, ldp format
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

    TEST_FIXTURE(DocumentTestFixture, creation_004)
    {
        //004. from string, lmd format
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

    TEST_FIXTURE(DocumentTestFixture, creation_005)
    {
        //005. from_input, ldp format
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

    TEST_FIXTURE(DocumentTestFixture, creation_006)
    {
        //006. error when trying to create already created document
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

    TEST_FIXTURE(DocumentTestFixture, creation_007)
    {
        //007. error when trying to load from string already created document
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

    TEST_FIXTURE(DocumentTestFixture, creation_008)
    {
        //008. error when trying to load from ldp file already created document
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

    TEST_FIXTURE(DocumentTestFixture, creation_009)
    {
        //009. error when trying to create with empty score already created document
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

    TEST_FIXTURE(DocumentTestFixture, get_score_100)
    {
        //100. in empty doc returns NULL
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore == NULL );
    }

    TEST_FIXTURE(DocumentTestFixture, get_score_101)
    {
        //101. return the score if not empty. Valid score from file
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore != NULL );
//        CHECK( pScore.to_string() == "(score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData )))" );
    }

    TEST_FIXTURE(DocumentTestFixture, get_score_102)
    {
        //102. return the score if not empty. Empty score
        Document doc(m_libraryScope);
        doc.create_with_empty_score();
        ImoDocument* pImoDoc = doc.get_imodoc();
        CHECK( pImoDoc != NULL );
        CHECK( pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore != NULL );
    }

    TEST_FIXTURE(DocumentTestFixture, id_assigner_110)
    {
        //110. get_pointer_to_imo() locates the imo
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q))))))");
//        cout << doc.to_string(k_save_ids) << endl;
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( doc.get_pointer_to_imo(4L) == pScore );
    }

//    TEST_FIXTURE(DocumentTestFixture, id_assigner_111)
//    {
//        //111. deleting imo removes its id from table
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (n c4 q))))))");
//        //cout << doc.to_string(k_save_ids) << endl;
//        imoDocument* pImoDoc = doc.get_imodoc();
//        pImoDoc->add_
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        CHECK( pScore != NULL );
//        CHECK( doc.get_pointer_to_imo(4L) == pScore );
//    }

    TEST_FIXTURE(DocumentTestFixture, dirty_bit_120)
    {
        //120. dirty bit can be cleared
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q))))))");
        CHECK( doc.is_dirty() == true );

        doc.clear_dirty();

        CHECK( doc.is_dirty() == false );
    }


    TEST_FIXTURE(DocumentTestFixture, other_130)
    {
        //130. access to weak pointer
        SpDocument spDoc( LOMSE_NEW Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q))))))");

        CHECK( spDoc.use_count() == 1 );

        WpDocument wpDoc(spDoc);
        CHECK( wpDoc.expired() == false );
    }

//    TEST_FIXTURE(DocumentTestFixture, checkpoints_210)
//    {
//        //210. get checkpoint data for score
//        SpDocument spDoc( LOMSE_NEW Document(m_libraryScope) );
//        spDoc->from_file(m_scores_path + "09007-score-in-exercise.lmd",
//                         Document::k_format_lmd );
//
//        cout << "DocumentTest 210: " << spDoc->get_checkpoint_data_for(64L);     //score
//    }

    TEST_FIXTURE(DocumentTestFixture, checkpoints_211)
    {
        //211. replace object from checkpoint data
        create_document_1();
        ImoObj* pImo = m_pDoc->get_pointer_to_imo(22L);
        CHECK( pImo->is_clef() );
        string data =
            "<ldpmusic>"
            "(score#4 (vers 2.0)(instrument#20 (musicData#21 (n#42 c4 q) )))"
            "</ldpmusic>";

        m_pDoc->replace_object_from_checkpoint_data(4L, data);

        //"(lenmusdoc#0 (vers 0.0) (content#3 "
        //    "(score#4 (vers 2.0) "
        //        "(instrument#20 (musicData#21 (clef#22 G)(key#23 C)"
        //        "(time#24 2 4)(n#25 c4 q)(r#26 q) )))"
        //    "(para#27 (txt#28 \"Hello world!\"))"
        //"))"

        //cout << m_pDoc->get_checkpoint_data();
        pImo = m_pDoc->get_pointer_to_imo(22L);
        CHECK( pImo == NULL );
        pImo = m_pDoc->get_pointer_to_imo(42L);
        CHECK( pImo->is_note() );
    }

};

