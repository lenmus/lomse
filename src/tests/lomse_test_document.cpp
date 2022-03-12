//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_id_assigner.h"
#include "private/lomse_document_p.h"
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
        CHECK( doc.get_pointer_to_imo(7L) == nullptr );
    }

    TEST_FIXTURE(IdAssignerTestFixture, removes_id)
    {
        Document doc(m_libraryScope);
        ImoObj* pImo = ImFactory::inject(k_imo_clef, &doc);
        CHECK( doc.get_pointer_to_imo(0L) == pImo );

        doc.on_removed_from_model(pImo);
        CHECK( doc.get_pointer_to_imo(0L) == nullptr );
        delete pImo;
    }
};



//=======================================================================================
// Document tests
//=======================================================================================

class DocumentTestFixture
{
public:
    LibraryScope m_libraryScope;
    string m_scores_path;
    Document* m_pDoc;

    DocumentTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(nullptr)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~DocumentTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
        m_pDoc = nullptr;
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    void create_document_1()
    {
        //"(lenmusdoc#0 (vers 0.0) (content#3 "
        //    "(score#94 (vers 2.0) "
        //        "(instrument#119 (musicData#120 (clef#121 G)(key#122 C)"
        //        "(time#123 2 4)(n#124 c4 q)(r#125 q) )))"
        //    "(para#126 (txt#127 \"Hello world!\"))"
        //"))"

        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score#94 (vers 2.0) "
                "(instrument#119 (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
    }

};

//---------------------------------------------------------------------------------------
SUITE(DocumentTest)
{

    //@ document creation ---------------------------------------------------------------

    TEST_FIXTURE(DocumentTestFixture, creation_000)
    {
        //@000. create_empty does create a valid empty document
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoDocument* pImoDoc = doc.get_im_root();
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
        CHECK( pImoDoc && pImoDoc->get_content_item(0) == nullptr );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_001)
    {
        //@001. from_file. valid file, ldp format
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        ImoDocument* pImoDoc = doc.get_im_root();
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.Truncate 1) (instrument (musicData)))))" );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_002)
    {
        //@002. from_file. valid file, lmd format
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "08011-paragraph.lmd", Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_im_root();
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_003)
    {
        //@003. from string, ldp format
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (n c4 q))))))");
        ImoDocument* pImoDoc = doc.get_im_root();
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( pImoDoc && pImoDoc->get_language() == "en" );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (n c4 q))))))" );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_004)
    {
        //@004. from string, lmd format
        Document doc(m_libraryScope);
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Credits</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content><para style='Credits'>Hello world!</para></content>"
            "</lenmusdoc>";
        doc.from_string(src, Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_im_root();
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );

        ImoDocument* pDoc = doc.get_im_root();
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != nullptr );
        if (pPara)
        {
            ImoStyle* pStyle = pPara->get_style();
            CHECK( pStyle != nullptr );
            CHECK( pStyle && pStyle->get_name() == "Credits" );
            CHECK( pPara && pPara->get_num_items() == 1 );
            ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
            CHECK( pItem && pItem->get_text() == "Hello world!" );
        }
    }

    TEST_FIXTURE(DocumentTestFixture, creation_005)
    {
        //@005. from_input, ldp format
        Document doc(m_libraryScope);
        LdpFileReader reader(m_scores_path + "00011-empty-fill-page.lms");
        doc.from_input(reader);
        ImoDocument* pImoDoc = doc.get_im_root();
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
//        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0) (content (score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.Truncate 1) (instrument (musicData)))))" );
    }

    TEST_FIXTURE(DocumentTestFixture, creation_006)
    {
        //@006. error when trying to create already created document
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
        //@007. error when trying to load from string already created document
        bool fOk = false;
        Document doc(m_libraryScope);
        doc.create_empty();
        try
        {
            doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
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
        //@008. error when trying to load from ldp file already created document
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
        //@009. error when trying to create with empty score already created document
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

    TEST_FIXTURE(DocumentTestFixture, creation_010)
    {
        //@010. MusicXML without clef is fixed
        Document doc(m_libraryScope);
        doc.from_string("<?xml version='1.0' encoding='utf-8'?>"
            "<!DOCTYPE score-partwise PUBLIC '-//Recordare//DTD MusicXML 3.0 Partwise//EN' "
                "'http://www.musicxml.org/dtds/partwise.dtd'>"
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<divisions>1</divisions><key><fifths>0</fifths></key>"
                "<time><beats>4</beats><beat-type>4</beat-type></time>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><type>whole</type></note>"
            "</measure>"
            "</part></score-partwise>"
            , Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );
        ImoObj::children_iterator it = pMD->begin();

        ImoClef* pClef = dynamic_cast<ImoClef*>(*it);
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_G2 );
        CHECK( pClef->get_staff() == 0 );

        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content (score (vers 2.0)"
              "(opt Render.SpacingOptions 2)(opt StaffLines.Truncate 3)"
              "(instrument P1 (name \"Music\")"
              "(staves 1)(musicData (clef G p1)(key C)(time 4 4)(n c4 w v1 p1)"
              "(barline simple))))))" );
//        cout << test_name() << endl;
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(DocumentTestFixture, creation_011)
    {
        //011. Opening compressed file (LMB)

        stringstream errormsg;
        Document doc(m_libraryScope, errormsg);
        doc.from_file(m_scores_path + "10014-compressed-flat-lmd.zip#zip:lenmusdoc-example.lmd",
                      Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_im_root();
#if (LOMSE_ENABLE_COMPRESSION == 1)
        //@011. Compression enabled. Compressed LMB file read ok
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
        //cout << doc.to_string() << endl;
        CHECK( doc.to_string().compare(0, 36, "(lenmusdoc (vers 0.0)(content (TODO:") == 0 );
#else
        //@011. Compression disabled. Error when opening LMB compressed file
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
//        cout << doc.to_string() << endl;
        CHECK( doc.to_string() == "(lenmusdoc (vers 0.0)(content))" );
#endif
    }


    //@ properties and getters ----------------------------------------------------------

    TEST_FIXTURE(DocumentTestFixture, get_score_100)
    {
        //@100. get_score(). in empty doc returns nullptr
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore == nullptr );
    }

    TEST_FIXTURE(DocumentTestFixture, get_score_101)
    {
        //@101. return the score if not empty. Valid score from file
        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "00011-empty-fill-page.lms");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore != nullptr );
//        CHECK( pScore.to_string() == "(score (vers 1.6) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.Truncate 1) (instrument (musicData)))" );
    }

    TEST_FIXTURE(DocumentTestFixture, get_score_102)
    {
        //@102. return the score if not empty. Empty score
        Document doc(m_libraryScope);
        doc.create_with_empty_score();
        ImoDocument* pImoDoc = doc.get_im_root();
        CHECK( pImoDoc != nullptr );
        CHECK( pImoDoc && pImoDoc->get_owner() == &doc );
        CHECK( doc.is_dirty() == true );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore != nullptr );
    }

    TEST_FIXTURE(DocumentTestFixture, id_assigner_110)
    {
        //@110. get_pointer_to_imo() locates the imo
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content#100 (score (vers 2.0) "
            "(instrument (musicData (n c4 q))))))");
//        cout << doc.to_string(k_save_ids) << endl;
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( doc.get_pointer_to_imo(101L) == pScore );
    }

//    TEST_FIXTURE(DocumentTestFixture, id_assigner_111)
//    {
//        //111. deleting imo removes its id from table
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (n c4 q))))))");
//        //cout << doc.to_string(k_save_ids) << endl;
//        ImoDocument* pImoDoc = doc.get_im_root();
//        pImoDoc->add_
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        CHECK( pScore != nullptr );
//        CHECK( doc.get_pointer_to_imo(4L) == pScore );
//    }

    TEST_FIXTURE(DocumentTestFixture, id_assigner_112)
    {
        //112. set and get @xml:id string

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0)"
            "(instrument (musicData (n c4 q))))))");
        //cout << doc.to_string(k_save_ids) << endl;
        ImoDocument* pImoDoc = doc.get_im_root();
        ImoScore* pScore = static_cast<ImoScore*>( pImoDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        pScore->set_xml_id("score1");

        CHECK( pScore->get_xml_id() == "score1" );
        CHECK( pImoDoc->get_xml_id() == "" );
    }

    TEST_FIXTURE(DocumentTestFixture, dirty_bit_120)
    {
        //120. dirty bit can be cleared
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0)"
            "(instrument (musicData (n c4 q))))))");
        CHECK( doc.is_dirty() == true );

        doc.clear_dirty();

        CHECK( doc.is_dirty() == false );
    }

    TEST_FIXTURE(DocumentTestFixture, other_130)
    {
        //@130. access to weak pointer
        SpDocument spDoc( LOMSE_NEW Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0)"
            "(instrument (musicData (n c4 q))))))");

        CHECK( spDoc.use_count() == 1 );

        WpDocument wpDoc(spDoc);
        CHECK( wpDoc.expired() == false );
    }


    //@ checkpoints ---------------------------------------------------------------------

//    TEST_FIXTURE(DocumentTestFixture, checkpoints_210)
//    {
//        //@210. get checkpoint data for score
//        SpDocument spDoc( LOMSE_NEW Document(m_libraryScope) );
//        spDoc->from_file(m_scores_path + "09007-score-in-exercise.lmd",
//                         Document::k_format_lmd );
//
//        cout << "DocumentTest 210: " << spDoc->get_checkpoint_data_for(64L);     //score
//    }

    TEST_FIXTURE(DocumentTestFixture, checkpoints_211)
    {
        //@211. replace object from checkpoint data
        create_document_1();
        ImoObj* pImo = m_pDoc->get_pointer_to_imo(121L);
        CHECK( pImo->is_clef() );
        string data =
            "<ldpmusic>"
            "(score#180 (vers 2.0)(instrument#221 (musicData#222 (n#243 c4 q) )))"
            "</ldpmusic>";

        m_pDoc->replace_object_from_checkpoint_data(94L, data);

        //"(lenmusdoc#0 (vers 0.0) (content "
        //    "(score#94 (vers 2.0) "
        //        "(instrument#120 (musicData#121 (clef#122 G)(key#123 C)"
        //        "(time#124 2 4)(n#125 c4 q)(r#126 q) )))"
        //    "(para#127 (txt#128 \"Hello world!\"))"
        //"))"

        //cout << m_pDoc->get_checkpoint_data();
        pImo = m_pDoc->get_pointer_to_imo(121L);
        CHECK( pImo == nullptr );
        pImo = m_pDoc->get_pointer_to_imo(243L);
        CHECK( pImo->is_note() );
    }

};

