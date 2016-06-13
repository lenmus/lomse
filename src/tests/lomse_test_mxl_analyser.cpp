//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#define LOMSE_INTERNAL_API
#include <UnitTest++.h>
#include <iostream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_xml_parser.h"
#include "lomse_mxl_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_events.h"
#include "lomse_doorway.h"
#include "lomse_im_factory.h"
#include "lomse_time.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// access to protected members
class MyMxlAnalyser : public MxlAnalyser
{
public:
    MyMxlAnalyser(ostream& reporter, LibraryScope& libScope, Document* pDoc,
                  XmlParser* parser)
        : MxlAnalyser(reporter, libScope, pDoc, parser)
    {
    }

    void do_not_delete_instruments_in_destructor()
    {
        m_partList.do_not_delete_instruments_in_destructor();
    }

};

//---------------------------------------------------------------------------------------
class MxlAnalyserTestFixture
{
public:
    LibraryScope m_libraryScope;
    int m_requestType;
    bool m_fRequestReceived;
    ImoDocument* m_pDoc;


    MxlAnalyserTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_requestType(k_null_request)
        , m_fRequestReceived(false)
        , m_pDoc(NULL)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~MxlAnalyserTestFixture()    //TearDown fixture
    {
    }

    static void wrapper_lomse_request(void* pThis, Request* pRequest)
    {
        static_cast<MxlAnalyserTestFixture*>(pThis)->on_lomse_request(pRequest);
    }

    void on_lomse_request(Request* pRequest)
    {
        m_fRequestReceived = true;
        m_requestType = pRequest->get_request_type();
        if (m_requestType == k_dynamic_content_request)
        {
            RequestDynamic* pRq = dynamic_cast<RequestDynamic*>(pRequest);
            ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pRq->get_object() );
            m_pDoc = pDyn->get_document();
        }
    }

};


SUITE(MxlAnalyserTest)
{

    //@ score_partwise ------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_1)
    {
        //@00001 missing mandatory <part-list>. Returns empty document
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <score-partwise>: missing mandatory element <part-list>." << endl;
        parser.parse_text("<score-partwise version='3.0'></score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( a.get_musicxml_version() == 300 );
        CHECK( pIModel->get_root() != NULL );
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc->get_num_content_items() == 0 );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_2)
    {
        //@00002 invalid score version, 1.0 assumed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <score-partwise>: missing mandatory element <part-list>." << endl;
        parser.parse_text("<score-partwise version='3.a'></score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( a.get_musicxml_version() == 100 );
        CHECK( pIModel->get_root() != NULL );
        CHECK( pIModel->get_root()->is_document() == true );
        CHECK( errormsg.str() == expected.str() );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_3)
    {
        //@00003 missing <score-part>. Returns empty document
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part-list>: missing mandatory element <score-part>.\n"
                 << "Line 0. errors in <part-list>. Analysis stopped." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list/></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 0 );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_4)
    {
        //@00004 missing <part>. Returns empty score v1.6
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;

        stringstream expected;
        expected << "Error: missing <part> for <score-part id='P1'>." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
                          "<score-part id='P1'><part-name>Music</part-name></score-part>"
                          "</part-list></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_version_number() == 160 );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_num_items() == 0 );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_5)
    {
        //@00005 minimum score ok. Returns empty score with one instrument
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <score-partwise>: missing mandatory element <part>." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
                          "<score-part id='P1'><part-name>Music</part-name></score-part>"
                          "</part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_num_items() == 0 );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_6)
    {
        //@00006 ImoScore created for <score-partwise>
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <score-partwise>: missing mandatory element <part>." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
                          "<score-part id='P1'><part-name>Music</part-name></score-part>"
                          "</part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_7)
    {
        //@00007 ImoInstrument created for <part-list>
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part>: missing mandatory 'id' attribute. <part> content will be ignored"
                 << endl << "Error: missing <part> for <score-part id='P1'>." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
                          "<score-part id='P1'><part-name>Music</part-name></score-part>"
                          "</part-list><part></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_8)
    {
        //@00008 ImoMusicData created for <part>
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <score-partwise>: missing mandatory element <part>." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
                          "<score-part id='P1'><part-name>Music</part-name></score-part>"
                          "</part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_num_items() == 0 );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_9)
    {
        //@00009 <part-list> with several <score-part>. Missing part
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Error: missing <part> for <score-part id='P2'>." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "</part-list><part id='P1'></part></score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );

        CHECK( pMD->get_num_items() == 0 );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != NULL );
        pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_num_items() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ score-part -------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_part_101)
    {
        //@00101 <part-name> sets instrument name
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<score-part id='P1'><part-name>Guitar</part-name></score-part>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_instrument() == true );
        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "Guitar" );
//        cout << "Name: '" << pInstr->get_name().get_text()
//             << "', Abbrev: '" << pInstr->get_abbrev().get_text() << "'" << endl;
        CHECK( pInstr->get_abbrev().get_text() == "" );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_num_items() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ part-group -------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_110)
    {
        //@00110 <part-group> type=start matched with type=stop
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'></part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_none );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_111)
    {
        //@00111 <part-group> type=start present but missing type=stop
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Error: missing <part-group type='stop'> for <part-group> number='1'."
                 << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'></part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups == NULL );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_112)
    {
        //@00112 <part-group> missing number
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part-group>: invalid or missing mandatory 'number' "
                    "attribute. Tag ignored." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group type='start'></part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups == NULL );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_113)
    {
        //@00113 <part-group> missing type
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part-group>: missing mandatory 'type' attribute. Tag ignored."
                 << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1'></part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups == NULL );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_114)
    {
        //@00114 <part-group> missing type start for this number
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part-group> type='stop': missing <part-group> with the "
                    "same number and type='start'." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups == NULL );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_115)
    {
        //@00115 <part-group> type is not either 'start' or 'stop'
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part-group>: invalid mandatory 'type' attribute. Must be "
                    "'start' or 'stop'." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='begin'></part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups == NULL );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_116)
    {
        //@00116 <part-group> type=start for number already started and not stopped
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part-group> type=start for number already started and not stopped"
                 << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'></part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='start'></part-group>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_none );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_117)
    {
        //@00117 <part-group> group name and group abbrev ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'>"
                "<group-name>Group</group-name>"
                "<group-abbreviation>Grp</group-abbreviation>"
            "</part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "Grp" );
        CHECK( pGroup->get_name_string() == "Group" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_none );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_118)
    {
        //@00118 <part-group> group symbol ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'>"
                "<group-name>Group</group-name>"
                "<group-symbol>brace</group-symbol>"
            "</part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "Group" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_brace );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_119)
    {
        //@00119 <part-group> group symbol: invalid value
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Invalid value for <group-symbol>. Must be "
                    "'none', 'brace', 'line' or 'bracket'. 'none' assumed." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'>"
                "<group-name>Group</group-name>"
                "<group-symbol>dots</group-symbol>"
            "</part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "Group" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_none );
        CHECK( pGroup->join_barlines() == ImoInstrGroup::k_standard );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_120)
    {
        //@00120 <part-group> group-barline ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'>"
                "<group-name>Group</group-name>"
                "<group-symbol>bracket</group-symbol>"
                "<group-barline>no</group-barline>"
            "</part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "Group" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );
        CHECK( pGroup->join_barlines() == ImoInstrGroup::k_no );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_121)
    {
        //@00121 <part-group> group-barline mensurstrich ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list>"
            "<part-group number='1' type='start'>"
                "<group-name>Group</group-name>"
                "<group-symbol>bracket</group-symbol>"
                "<group-barline>Mensurstrich</group-barline>"
            "</part-group>"
            "<score-part id='P1'><part-name>Voice</part-name></score-part>"
            "<score-part id='P2'><part-name>Piano</part-name></score-part>"
            "<part-group number='1' type='stop'></part-group>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "Group" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );
        CHECK( pGroup->join_barlines() == ImoInstrGroup::k_mensurstrich );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ clef -------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_clef_130)
    {
        //@00130 minimum content parsed ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<clef><sign>G</sign><line>2</line></clef>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );
        CHECK( pClef->get_staff() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_clef_131)
    {
        //@00131 error in clef sign
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Unknown clef 'H'. Assumed 'G' in line 2." << endl;
        parser.parse_text("<clef><sign>H</sign><line>2</line></clef>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_clef_132)
    {
        //@00132 staff num parsed ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<clef number='2'><sign>F</sign><line>4</line></clef>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_F4 );
        CHECK( pClef->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    //@ time ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_time_140)
    {
        //@00140 minimum content parsed ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<time><beats>6</beats><beat-type>8</beat-type></time>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature->get_bottom_number() == 8 );
//        cout << UnitTest::CurrentTest::Details()->testName
//             << ": top number=" << pTimeSignature->get_top_number()
//             << ", bottom: " << pTimeSignature->get_bottom_number() << endl;

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_time_141)
    {
        //@00141 error in time signature
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <time>: missing mandatory element <beat-type>." << endl;
        parser.parse_text("<time><beats>6</beats></time>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ key ------------------------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_160)
    {
        //@00160 minimum content parsed ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<key><fifths>2</fifths></key>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_key_signature() == true );
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pIModel->get_root() );
        CHECK( pKeySignature != NULL );
        CHECK( pKeySignature->get_key_type() == k_key_D );
        CHECK( pKeySignature->get_staff() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_161)
    {
        //@00161 key in minor mode
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<key><fifths>5</fifths><mode>minor</mode></key>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_key_signature() == true );
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pIModel->get_root() );
        CHECK( pKeySignature != NULL );
        CHECK( pKeySignature->get_key_type() == k_key_gs );
        CHECK( pKeySignature->get_staff() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ attributes -------------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_180)
    {
        //@00180 attributes are added to MusicData in required order
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<key><fifths>2</fifths></key>"
                "<time><beats>4</beats><beat-type>4</beat-type></time>"
                "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_num_items() == 4 );
        ImoObj::children_iterator it = pMD->begin();
        CHECK( (*it)->is_clef() == true );
        ++it;
        CHECK( (*it)->is_key_signature() == true );
        ++it;
        CHECK( (*it)->is_time_signature() == true );
        ++it;
        CHECK( (*it)->is_barline() == true );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_181)
    {
        //@00181 divisions
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<attributes>"
                "<divisions>7</divisions>"
            "</attributes>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL);
        CHECK( a.current_divisions() == 7.0f );

        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_182)
    {
        //@00182 if no divisions assume 1
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes></attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "divisons:" << a.current_divisions() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( a.current_divisions() == 1.0f );

        delete pIModel;
    }


    //@ note -----------------------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_300)
    {
        //@00300 minimum content parsed ok. Note saved
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>E</step><octave>3</octave></pitch>"
            "<duration>4</duration><type>whole</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_whole );
        CHECK( pNote->get_octave() == 3 );
        CHECK( pNote->get_step() == k_step_E );
        CHECK( pNote->get_duration() == k_duration_whole );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );
        CHECK( a.get_last_note() == pNote );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_301)
    {
        //@00301 invalid step returns C
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Unknown note step 'e'. Replaced by 'C'." << endl;
        parser.parse_text("<note><pitch><step>e</step><octave>4</octave></pitch>"
            "<duration>4</duration><type>whole</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_whole );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->get_duration() == k_duration_whole );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_302)
    {
        //@00302 invalid octave returns 4
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Unknown octave 'e'. Replaced by '4'." << endl;
        parser.parse_text("<note><pitch><step>D</step><octave>e</octave></pitch>"
            "<duration>1</duration><type>quarter</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_quarter );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_D );
        CHECK( pNote->get_duration() == k_duration_quarter );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_303)
    {
        //@00303 alter. Duration different from type
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>G</step><alter>-1</alter>"
            "<octave>5</octave></pitch>"
            "<duration>4</duration><type>eighth</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_actual_accidentals() == k_flat );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 5 );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->get_duration() == k_duration_whole );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_304)
    {
        //@00304 accidental
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>B</step>"
            "<octave>1</octave></pitch>"
            "<duration>1</duration><type>half</type>"
            "<accidental>sharp</accidental></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_actual_accidentals() == k_no_accidentals );
        CHECK( pNote->get_notated_accidentals() == k_sharp );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_half );
        CHECK( pNote->get_octave() == 1 );
        CHECK( pNote->get_step() == k_step_B );
        CHECK( pNote->get_duration() == k_duration_quarter );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_305)
    {
        //@00305 staff
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>A</step>"
            "<octave>3</octave></pitch>"
            "<duration>4</duration><type>whole</type>"
            "<staff>2</staff></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_actual_accidentals() == k_no_accidentals );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_whole );
        CHECK( pNote->get_octave() == 3 );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->get_duration() == k_duration_whole );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );
        CHECK( pNote->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_306)
    {
        //@00306 stem
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>A</step>"
            "<octave>3</octave></pitch>"
            "<duration>1</duration><type>quarter</type>"
            "<stem>down</stem></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_actual_accidentals() == k_no_accidentals );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_quarter );
        CHECK( pNote->get_octave() == 3 );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->get_duration() == k_duration_quarter );
        CHECK( pNote->get_stem_direction() == k_stem_down );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_307)
    {
        //@00307 chord ok. start and end notes
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );

        ImoObj::children_iterator it = pMD->begin();
        CHECK( pMD->get_num_children() == 3 );

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == true );
        CHECK( pNote->is_end_of_chord() == false );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == true );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_308)
    {
        //@00308 chord ok. intermediate notes
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );

        ImoObj::children_iterator it = pMD->begin();
        CHECK( pMD->get_num_children() == 4 );

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == true );
        CHECK( pNote->is_end_of_chord() == false );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == true );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_309)
    {
        //@00309. Type implied by duration
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>G</step><alter>-1</alter>"
            "<octave>5</octave></pitch>"
            "<duration>2</duration></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_actual_accidentals() == k_flat );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_half );
        CHECK( pNote->get_octave() == 5 );
        CHECK( pNote->get_step() == k_step_G );
        CHECK( pNote->get_duration() == k_duration_half );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

//    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_309)
//    {
//        //@00309 timepos is computed right
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        parser.parse_text(
//            "<score-partwise version='3.0'><part-list>"
//            "<score-part id='P1'><part-name>Music</part-name></score-part>"
//            "</part-list><part id='P1'>"
//            "<measure number='1'>"
//            "<note><pitch><step>A</step><octave>3</octave></pitch>"
//                "<duration>4</duration><type>16th</type></note>"
//            "<note><pitch><step>C</step><octave>4</octave></pitch>"
//                "<duration>4</duration><type>16th</type></note>"
//            "<note><pitch><step>E</step><octave>4</octave></pitch>"
//                "<duration>4</duration><type>16th</type></note>"
//            "</measure>"
//            "</part></score-partwise>"
//        );
//        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
////        cout << UnitTest::CurrentTest::Details()->testName << endl;
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root() != NULL);
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//        ColStaffObjs* pCol = pScore->get_staffobjs_table();
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pIModel;
//    }



////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Chord_Beamed)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(musicData "
////            "(chord (n a3 e (beam 1 +)) (n d3 e))"
////            "(chord (n a3 e (beam 1 -))(n d3 e)) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////
////        ImoObj::children_iterator it = pMusic->begin();
////        CHECK( pMusic->get_num_children() == 4 );
////
////        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_in_chord() == true );
////        CHECK( pNote->is_start_of_chord() == true );
////        CHECK( pNote->is_end_of_chord() == false );
////        CHECK( pNote->is_beamed() == true );
////
////        ++it;
////        pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_in_chord() == true );
////        CHECK( pNote->is_start_of_chord() == false );
////        CHECK( pNote->is_end_of_chord() == true );
////        CHECK( pNote->is_beamed() == false );
////
////        ++it;
////        pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_in_chord() == true );
////        CHECK( pNote->is_start_of_chord() == true );
////        CHECK( pNote->is_end_of_chord() == false );
////        CHECK( pNote->is_beamed() == true );
////
////        ++it;
////        pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_in_chord() == true );
////        CHECK( pNote->is_start_of_chord() == false );
////        CHECK( pNote->is_end_of_chord() == true );
////        CHECK( pNote->is_beamed() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////

    //@ rest --------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_rest_350)
    {
        //@00350 staff
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><rest/>"
            "<duration>1</duration><type>quarter</type>"
            "<staff>2</staff></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_rest() == true );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
        CHECK( pRest != NULL );
        CHECK( pRest->get_dots() == 0 );
        CHECK( pRest->get_note_type() == k_quarter );
        CHECK( pRest->get_duration() == k_duration_quarter );
        CHECK( pRest->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ barline --------------------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_barline_400)
    {
        //@00400 barline minimal content
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<barline><bar-style>light-light</bar-style></barline>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ backup --------------------------------------------------------------------------
    //@ forward -------------------------------------------------------------------------


//    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_backup_420)
//    {
//        //@00420 backup
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        parser.parse_text("<backup><duration>18</duration></backup>");
//        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root() == NULL);
//        //AWARE: initialy <divisions>==1 ==> duration is expressed in quarter notes
//        CHECK( is_equal_time(a.get_current_time(), -18.0f*k_duration_64th) );
//        cout << "420: timepos= " << a.get_current_time() << endl;
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pIModel;
//    }

    //@ Hello World -------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_hello_world_99999)
    {
        //@99999
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<divisions>1</divisions><key><fifths>0</fifths></key>"
                "<time><beats>4</beats><beat-type>4</beat-type></time>"
                "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><type>whole</type></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_num_items() == 5 );
        ImoObj* pImo = pMD->get_first_child();
        CHECK( pImo->is_clef() == true );

        delete pIModel;
    }

////    // stem
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_StemUp)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. ?" << endl;
////        parser.parse_text("(n c4 e (stem up))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_stem_up() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_StemDown)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. ?" << endl;
////        parser.parse_text("(n c4 e (stem down))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_stem_down() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_StemError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid value 'no' for stem type. Default stem asigned." << endl;
////        parser.parse_text("(n c4 e (stem no))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_stem_default() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pNote->get_note_type() == k_eighth );
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == k_step_C );
////        CHECK( pNote->get_stem_direction() == k_stem_default );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_StemTie)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(musicData (n c4 q (tie 12 start)(stem down)) (n c4 e (stem up)(tie 12 stop)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1->is_tied_next() == true );
////        CHECK( pNote1->is_tied_prev() == false );
////        CHECK( pNote1->is_stem_down() == true );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_tied_next() == false );
////        CHECK( pNote2->is_tied_prev() == true );
////        CHECK( pNote2->is_stem_up() == true );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_SeveralOldParams)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(n +d3 e. g+ p2)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root()->is_note() == true );
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_notated_accidentals() == k_sharp );
////        CHECK( pNote->get_dots() == 1 );
////        CHECK( pNote->get_note_type() == k_eighth );
////        CHECK( pNote->get_octave() == 3 );
////        CHECK( pNote->get_step() == k_step_D );
////        CHECK( pNote->get_duration() == 48.0f );
////        CHECK( pNote->get_staff() == 1 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // tie (old syntax) -----------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TieOld)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(musicData (n c4 e l)(n c4 q))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        ImoTie* pTie = pNote1->get_tie_next();
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1->is_tied_next() == true );
////        CHECK( pNote1->is_tied_prev() == false );
////        CHECK( pTie->get_start_note() == pNote1 );
////        ImoRelations* pRelObjs = pNote1->get_relations();
////        CHECK( pRelObjs != NULL );
////        CHECK( pRelObjs->get_item(0) == pTie );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_tied_next() == false );
////        CHECK( pNote2->is_tied_prev() == true );
////        CHECK( pTie->get_end_note() == pNote2 );
////        pRelObjs = pNote2->get_relations();
////        CHECK( pRelObjs != NULL );
////        CHECK( pRelObjs->get_item(0) == pTie );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TieOld_Error1)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. No note found to match old syntax tie. Tie ignored." << endl;
////        parser.parse_text("(musicData (n c4 e l)(n d4 q))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1->is_tied_next() == false );
////        CHECK( pNote1->is_tied_prev() == false );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_tied_next() == false );
////        CHECK( pNote2->is_tied_prev() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TieOld_IntermediateNote)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. No 'start' element for tie number 12. Tie ignored." << endl;
////        parser.parse_text("(musicData (n c4 q v1 l)(n e4 q v2)(n c4 e v1))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        ImoTie* pTie = pNote1->get_tie_next();
////        CHECK( pNote1->is_tied_next() == true );
////        CHECK( pNote1->is_tied_prev() == false );
////        CHECK( pTie->get_start_note() == pNote1 );
////
////        ++it;
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_tied_next() == false );
////        CHECK( pNote2->is_tied_prev() == true );
////        CHECK( pTie->get_end_note() == pNote2 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TieOld_IntermediateBarline)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. No 'start' element for tie number 12. Tie ignored." << endl;
////        parser.parse_text("(musicData (n c4 q v1 l)(barline simple)(n c4 e v1))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        ImoTie* pTie = pNote1->get_tie_next();
////        CHECK( pNote1->is_tied_next() == true );
////        CHECK( pNote1->is_tied_prev() == false );
////        CHECK( pTie->get_start_note() == pNote1 );
////        CHECK( pTie->is_tie() == true );
////
////        ++it;
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_tied_next() == false );
////        CHECK( pNote2->is_tied_prev() == true );
////        CHECK( pTie->get_end_note() == pNote2 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TieOld_Several)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(musicData (n c4 q l)(n c4 e l)(n c4 e))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1->is_tied_next() == true );
////        CHECK( pNote1->is_tied_prev() == false );
////        ImoTie* pTie1 = pNote1->get_tie_next();
////        CHECK( pTie1->get_start_note() == pNote1 );
////        ImoRelations* pRelObjs = pNote1->get_relations();
////        CHECK( pRelObjs != NULL );
////        CHECK( pRelObjs->get_item(0) == pTie1 );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_tied_next() == true );
////        CHECK( pNote2->is_tied_prev() == true );
////        ImoTie* pTie2 = pNote2->get_tie_next();
////        CHECK( pTie1->get_end_note() == pNote2 );
////        CHECK( pTie2->get_start_note() == pNote2 );
////        pRelObjs = pNote2->get_relations();
////        CHECK( pRelObjs != NULL );
////        CHECK( pRelObjs->get_item(0) == pTie1 );
////        CHECK( pRelObjs->get_item(1) == pTie2 );
////
////        ++it;
////        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote3 != NULL );
////        CHECK( pNote3->is_tied_next() == false );
////        CHECK( pNote3->is_tied_prev() == true );
////        CHECK( pTie2->get_end_note() == pNote3 );
////        pRelObjs = pNote3->get_relations();
////        CHECK( pRelObjs != NULL );
////        CHECK( pRelObjs->get_item(0) == pTie2 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // tie ------------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tie_ParsedStop)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
////        parser.parse_text("(tie 12 stop)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root()->is_tie_dto() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start() == false );
////        CHECK( pInfo->get_tie_number() == 12 );
////        CHECK( pInfo->get_note() == NULL );
////        CHECK( pInfo->get_bezier() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tie_ParsedStart)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
////        parser.parse_text("(tie 15 start)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start() == true );
////        CHECK( pInfo->get_tie_number() == 15 );
////        CHECK( pInfo->get_note() == NULL );
////        CHECK( pInfo->get_bezier() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tie_Bezier)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
////        parser.parse_text("(tie 15 start (bezier (ctrol2-x -25)(start-y 36.765)) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start() == true );
////        CHECK( pInfo->get_tie_number() == 15 );
////        CHECK( pInfo->get_note() == NULL );
////        ImoBezierInfo* pBezier = pInfo->get_bezier();
////        CHECK( pBezier != NULL );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 36.765f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == -25.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tie_ParsedError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid tie type. Tie ignored." << endl;
////        parser.parse_text("(tie 15 end)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tie_Color)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(tie 12 stop <color>#00ff00))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root()->is_tie_dto() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start() == false );
////        CHECK( pInfo->get_tie_number() == 12 );
////        CHECK( pInfo->get_note() == NULL );
////        CHECK( pInfo->get_bezier() == NULL );
////        CHECK( is_equal(pInfo->get_color(), Color(0,255,0,255)) );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // bezier ---------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Bezier_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
////        parser.parse_text("(bezier ctrol1-x:-25 (start-x 36.765) ctrol1-y:55)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_bezier_info() == true );
////        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pIModel->get_root() );
////        CHECK( pBezier != NULL );
////        //cout << "start.x = " << pBezier->get_point(ImoBezierInfo::k_start).x << endl;
////        //cout << "start.y = " << pBezier->get_point(ImoBezierInfo::k_start).y << endl;
////        //cout << "end.x = " << pBezier->get_point(ImoBezierInfo::k_end).x << endl;
////        //cout << "end.y = " << pBezier->get_point(ImoBezierInfo::k_end).y << endl;
////        //cout << "ctrol1.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).x << endl;
////        //cout << "ctrol1.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).y << endl;
////        //cout << "ctrol2.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).x << endl;
////        //cout << "ctrol2.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).y << endl;
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 36.765f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == -25.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 55.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Bezier_Error)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Unknown tag 'startx'." << endl <<
////            "Line 0. Element 'undefined' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(bezier (startx 36.765) ctrol1-x:-25 ctrol1-y:55)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pIModel->get_root() );
////        CHECK( pBezier != NULL );
////        //cout << "start.x = " << pBezier->get_point(ImoBezierInfo::k_start).x << endl;
////        //cout << "start.y = " << pBezier->get_point(ImoBezierInfo::k_start).y << endl;
////        //cout << "end.x = " << pBezier->get_point(ImoBezierInfo::k_end).x << endl;
////        //cout << "end.y = " << pBezier->get_point(ImoBezierInfo::k_end).y << endl;
////        //cout << "ctrol1.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).x << endl;
////        //cout << "ctrol1.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).y << endl;
////        //cout << "ctrol2.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).x << endl;
////        //cout << "ctrol2.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).y << endl;
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == -25.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 55.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Bezier_MissingValues)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
////        parser.parse_text("(bezier (start-x 36.765))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_bezier_info() == true );
////        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pIModel->get_root() );
////        CHECK( pBezier != NULL );
////        //cout << "start.x = " << pBezier->get_point(ImoBezierInfo::k_start).x << endl;
////        //cout << "start.y = " << pBezier->get_point(ImoBezierInfo::k_start).y << endl;
////        //cout << "end.x = " << pBezier->get_point(ImoBezierInfo::k_end).x << endl;
////        //cout << "end.y = " << pBezier->get_point(ImoBezierInfo::k_end).y << endl;
////        //cout << "ctrol1.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).x << endl;
////        //cout << "ctrol1.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).y << endl;
////        //cout << "ctrol2.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).x << endl;
////        //cout << "ctrol2.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).y << endl;
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 36.765f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // slur -----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Slur_ParsedStop)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(slur 12 stop)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root()->is_slur_dto() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_stop() == true );
////        CHECK( pInfo->get_slur_number() == 12 );
////        CHECK( pInfo->get_note() == NULL );
////        CHECK( pInfo->get_bezier() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Slur_ParsedStart)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(slur 15 start)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start() == true );
////        CHECK( pInfo->get_slur_number() == 15 );
////        CHECK( pInfo->get_note() == NULL );
////        CHECK( pInfo->get_bezier() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Slur_ParsedContinue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(slur 15 continue)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_continue() == true );
////        CHECK( pInfo->get_slur_number() == 15 );
////        CHECK( pInfo->get_note() == NULL );
////        CHECK( pInfo->get_bezier() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Slur_Bezier)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
////        parser.parse_text("(slur 27 start (bezier (ctrol2-x -25)(start-y 36.765)) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start() == true );
////        CHECK( pInfo->get_slur_number() == 27 );
////        CHECK( pInfo->get_note() == NULL );
////        ImoBezierInfo* pBezier = pInfo->get_bezier();
////        CHECK( pBezier != NULL );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 36.765f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == -25.0f );
////        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Slur_Color)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(slur 12 start <color>#00ff00))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root()->is_slur_dto() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start() == true );
////        CHECK( pInfo->get_slur_number() == 12 );
////        CHECK( pInfo->get_note() == NULL );
////        CHECK( pInfo->get_bezier() == NULL );
////        CHECK( is_equal(pInfo->get_color(), Color(0,255,0,255)) );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Slur_ParsedError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid slur type. Slur ignored." << endl;
////        parser.parse_text("(slur 15 end)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_NotesSlurred)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(musicData (n c4 q (slur 12 start)) (n c4 e (slur 12 stop)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
////        ImoSlur* pSlur = static_cast<ImoSlur*>( pNote->find_relation(k_imo_slur) );
////        CHECK( pSlur->get_slur_number() == 12 );
////        CHECK( pSlur->get_num_objects() == 2 );
////        ImoNote* pSlurNote1 = pSlur->get_start_note();
////        ImoNote* pSlurNote2 = pSlur->get_end_note();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 == pSlurNote1 );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 == pSlurNote2 );
////
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // rest -----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Rest)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(r e.)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_rest() == true );
////        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
////        CHECK( pRest != NULL );
////        CHECK( pRest->get_dots() == 1 );
////        CHECK( pRest->get_note_type() == k_eighth );
////        CHECK( pRest->has_attachments() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Rest_StaffNum)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(r e. p2)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
////        CHECK( pRest != NULL );
////        CHECK( pRest->get_dots() == 1 );
////        CHECK( pRest->get_note_type() == k_eighth );
////        CHECK( pRest->get_staff() == 1 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Rest_DefaultStaffNum)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(r e.)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
////        CHECK( pRest != NULL );
////        CHECK( pRest->get_dots() == 1 );
////        CHECK( pRest->get_note_type() == k_eighth );
////        CHECK( pRest->get_staff() == 0 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Rest_StaffNumInherited)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(musicData (r e. p2)(n c4 q)(n d4 e p3)(r q))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        LdpTree::iterator it = tree->begin();
////        ++it;
////        ImoRest* pRest = dynamic_cast<ImoRest*>( (*it)->get_imo() );
////        CHECK( pRest != NULL );
////        CHECK( pRest->get_staff() == 1 );
////        ++it;
////        ++it;
////        ++it;
////        ImoNote* pNote = dynamic_cast<ImoNote*>( (*it)->get_imo() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_staff() == 1 );
////        ++it;
////        ++it;
////        ++it;
////        pNote = dynamic_cast<ImoNote*>( (*it)->get_imo() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_staff() == 2 );
////        ++it;
////        ++it;
////        ++it;
////        ++it;
////        pRest = dynamic_cast<ImoRest*>( (*it)->get_imo() );
////        CHECK( pRest != NULL );
////        CHECK( pRest->get_staff() == 2 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Rest_Attachment)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(r e. (text \"andante\"))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
////        CHECK( pRest != NULL );
////        CHECK( pRest->get_dots() == 1 );
////        CHECK( pRest->get_note_type() == k_eighth );
////        CHECK( pRest->has_attachments() == true );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // fermata --------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Fermata)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(fermata below)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_fermata() == true );
////        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
////        CHECK( pFerm != NULL );
////        CHECK( pFerm->get_placement() == k_placement_below );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Fermata_ErrorPlacement)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Unknown fermata placement 'under'. Replaced by 'above'." << endl;
////        parser.parse_text("(fermata under)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
////        CHECK( pFerm != NULL );
////        CHECK( pFerm->get_placement() == k_placement_above );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Fermata_Location)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
////        parser.parse_text("(fermata above (dx 70))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
////        CHECK( pFerm != NULL );
////        CHECK( pFerm->get_placement() == k_placement_above );
////        CHECK( pFerm->get_user_location_x() == 70.0f );
////        CHECK( pFerm->get_user_location_y() == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Fermata_ErrorMore)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'fermata': too many parameters. Extra parameters from 'fermata' have been ignored." << endl;
////        parser.parse_text("(fermata above (dx 70)(fermata below))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
////        CHECK( pFerm != NULL );
////        CHECK( pFerm->get_placement() == k_placement_above );
////        CHECK( pFerm->get_user_location_x() == 70.0f );
////        CHECK( pFerm->get_user_location_y() == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_Fermata)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. ?" << endl;
////        parser.parse_text("(n c4 e (stem up)(fermata above (dx 70)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_stem_up() == true );
////        CHECK( pNote->get_num_attachments() == 1 );
////        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pNote->get_attachment(0) );
////        CHECK( pFerm != NULL );
////        CHECK( pFerm->get_placement() == k_placement_above );
////        CHECK( pFerm->get_user_location_x() == 70.0f );
////        CHECK( pFerm->get_user_location_y() == 0.0f );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // goFwd ----------------------------------------------------------------------------
////    // goBack ---------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoBackStart)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
////        parser.parse_text("(goBack start)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_go_back_fwd() == true );
////        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
////        CHECK( pGBF != NULL );
////        CHECK( pGBF->is_to_start() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoBackEnd)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
////        parser.parse_text("(goBack end)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoBackQ)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
////        parser.parse_text("(goBack q)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
////        CHECK( pGBF != NULL );
////        CHECK( !pGBF->is_to_start() );
////        CHECK( !pGBF->is_to_end() );
////        CHECK( pGBF->get_time_shift() == -k_duration_quarter );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoFwdEnd)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
////        parser.parse_text("(goFwd end)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
////        CHECK( pGBF != NULL );
////        CHECK( pGBF->is_to_end() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoFwdStart)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'goFwd' has an incoherent value: go forward to start?. Element ignored." << endl;
////        parser.parse_text("(goFwd start)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoFwdH)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
////        parser.parse_text("(goFwd h)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
////        CHECK( pGBF != NULL );
////        CHECK( !pGBF->is_to_start() );
////        CHECK( !pGBF->is_to_end() );
////        CHECK( pGBF->get_time_shift() == k_duration_16th );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoFwdNum)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
////        parser.parse_text("(goFwd 128)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
////        CHECK( pGBF != NULL );
////        CHECK( !pGBF->is_to_start() );
////        CHECK( !pGBF->is_to_end() );
////        CHECK( pGBF->get_time_shift() == k_duration_16th );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoFwdBadNumber)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Negative value for element 'goFwd/goBack'. Element ignored." << endl;
////        parser.parse_text("(goFwd -128.3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserGoBackNum)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
////        parser.parse_text("(goBack 21.3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
////        CHECK( pGBF != NULL );
////        CHECK( !pGBF->is_to_start() );
////        CHECK( !pGBF->is_to_end() );
////        CHECK( pGBF->get_time_shift() == -21.3f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // instrument -----------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Instrument_Staves)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(instrument (staves 2)(musicData))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root()->is_instrument() == true );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoInstrument* pInstrument = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
////        CHECK( pInstrument != NULL );
////        //cout << "num.staves=" << pInstrument->get_num_staves() << endl;
////        CHECK( pInstrument->get_num_staves() == 2 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Instrument_StavesError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid value 'two' for staves. Replaced by 1." << endl;
////        parser.parse_text("(instrument (staves two)(musicData))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoInstrument* pInstrument = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
////        CHECK( pInstrument != NULL );
////        //cout << "num.staves=" << pInstrument->get_num_staves() << endl;
////        CHECK( pInstrument->get_num_staves() == 1 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Instrument_AddedToScore)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(score (vers 1.6)(instrument (musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_score() == true );
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        CHECK( pScore->get_num_instruments() == 1 );
////        ImoInstrument* pInstr = pScore->get_instrument(0);
////        CHECK( pInstr != NULL );
////        CHECK( pInstr->get_num_staves() == 1 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Instrument_Name)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(instrument (name \"Guitar\")(musicData))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
////        CHECK( pInstr != NULL );
////        CHECK( pInstr->get_num_staves() == 1 );
////        CHECK( pInstr->get_name().get_text() == "Guitar" );
////        CHECK( pInstr->get_abbrev().get_text() == "" );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Instrument_Abbrev)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(instrument (abbrev \"G.\")(musicData))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
////        CHECK( pInstr != NULL );
////        CHECK( pInstr->get_num_staves() == 1 );
////        CHECK( pInstr->get_name().get_text() == "" );
////        CHECK( pInstr->get_abbrev().get_text() == "G." );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Instrument_NameAbbrev)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(instrument (name \"Guitar\")(abbrev \"G.\")(musicData))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
////        CHECK( pInstr != NULL );
////        CHECK( pInstr->get_num_staves() == 1 );
////        CHECK( pInstr->get_name().get_text() == "Guitar" );
////        CHECK( pInstr->get_abbrev().get_text() == "G." );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_MidiInfo_InstrErrorValue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid MIDI instrument (0..255). MIDI info ignored." << endl;
////        parser.parse_text("(infoMIDI piano 1)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
////        CHECK( pInfo == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_MidiInfo_InstrErrorRange)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid MIDI instrument (0..255). MIDI info ignored." << endl;
////        parser.parse_text("(infoMIDI 315 1)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
////        CHECK( pInfo == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_MidiInfo_InstrumentOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(infoMIDI 56)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        CHECK( pIModel->get_root()->is_midi_info() == true );
////        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_channel() == 0 );
////        CHECK( pInfo->get_instrument() == 56 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_MidiInfo_ChannelErrorValue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid MIDI channel (0..15). Channel info ignored." << endl;
////        parser.parse_text("(infoMIDI 56 25)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_channel() == 0 );
////        CHECK( pInfo->get_instrument() == 56 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_MidiInfo_InstrumentChannelOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(infoMIDI 56 10)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_channel() == 10 );
////        CHECK( pInfo->get_instrument() == 56 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Instrument_MidiInfo)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(instrument (infoMIDI 56 12)(musicData))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
////        CHECK( pInstr != NULL );
////        CHECK( pInstr->get_num_staves() == 1 );
////        CHECK( pInstr->get_name().get_text() == "" );
////        CHECK( pInstr->get_abbrev().get_text() == "" );
////        CHECK( pInstr->get_channel() == 12 );
////        CHECK( pInstr->get_instrument() == 56 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // systemInfo -----------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserSystemInfoBadType)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Expected 'first' or 'other' value but found 'third'. 'first' assumed." << endl;
////        parser.parse_text("(systemLayout third (systemMargins 0 0 0 2000))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_system_info() == true );
////        ImoSystemInfo* pSI = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
////        CHECK( pSI != NULL );
////        CHECK( pSI->is_first() );
////        CHECK( pSI->get_left_margin() == 0.0f );
////        CHECK( pSI->get_right_margin() == 0.0f );
////        CHECK( pSI->get_system_distance() == 0.0f );
////        CHECK( pSI->get_top_system_distance() == 2000.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserSystemInfoMissingMargins)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. systemLayout: missing mandatory element 'systemMargins'." << endl;
////        parser.parse_text("(systemLayout other)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSystemInfo* pSI = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
////        CHECK( pSI != NULL );
////        CHECK( !pSI->is_first() );
////        CHECK( pSI->get_left_margin() == 0.0f );
////        CHECK( pSI->get_right_margin() == 0.0f );
////        CHECK( pSI->get_system_distance() == 0.0f );
////        CHECK( pSI->get_top_system_distance() == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserSystemMargins)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(systemLayout other (systemMargins 0 100 0 2000))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_system_info() == true );
////        ImoSystemInfo* pSI = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
////        CHECK( pSI != NULL );
////        CHECK( !pSI->is_first() );
////        CHECK( pSI->get_left_margin() == 0.0f );
////        CHECK( pSI->get_right_margin() == 100.0f );
////        CHECK( pSI->get_system_distance() == 0.0f );
////        CHECK( pSI->get_top_system_distance() == 2000.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserSystemInfoOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. systemInfo: missing mandatory element 'systemMargins'." << endl;
////        parser.parse_text("(systemLayout other (systemMargins 0 100 0 2000))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSystemInfo* pSL = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
////        CHECK( pSL != NULL );
////        CHECK( !pSL->is_first() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // text -----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserText)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. systemInfo: missing mandatory element 'systemMargins'." << endl;
////        parser.parse_text("(text \"This is a text\")");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_score_text() == true );
////        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
////        CHECK( pText != NULL );
////        CHECK( pText->get_text() == "This is a text" );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, AnalyserTextMissingText)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. text: missing mandatory element 'string'." << endl;
////        parser.parse_text("(text)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
////        CHECK( pText == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Text_AlignStyle)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(text \"Moonlight sonata\" (style \"Header1\"))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
////        CHECK( pText != NULL );
////        CHECK( pText->get_text() == "Moonlight sonata" );
////        ImoStyle* pStyle = pText->get_style();
////        CHECK( pStyle == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Text_Location)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(text \"F. Chopin\" (style \"Composer\")(dy 30)(dx 20))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
////        CHECK( pText != NULL );
////        CHECK( pText->get_text() == "F. Chopin" );
////        CHECK( pText->get_user_location_x() == 20.0f );
////        CHECK( pText->get_user_location_y() == 30.0f );
////        ImoStyle* pStyle = pText->get_style();
////        CHECK( pStyle == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // metronome ------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_Value)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(metronome 88)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_metronome_mark() == true );
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 88 );
////        CHECK( pMM->is_visible() == true );
////        CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_Error)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing metronome parameters. Replaced by '(metronome 60)'." << endl;
////        parser.parse_text("(metronome)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 60 );
////        CHECK( pMM->is_visible() == true );
////        CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_NoteValue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(metronome e. 77)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_value );
////        CHECK( pMM->get_ticks_per_minute() == 77 );
////        CHECK( pMM->get_left_note_type() == k_eighth );
////        CHECK( pMM->get_left_dots() == 1 );
////        CHECK( pMM->is_visible() == true );
////        CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_NoteNote)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(metronome e. s)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_note );
////        CHECK( pMM->get_left_note_type() == k_eighth );
////        CHECK( pMM->get_left_dots() == 1 );
////        CHECK( pMM->get_right_note_type() == k_16th );
////        CHECK( pMM->get_right_dots() == 0 );
////        CHECK( pMM->is_visible() == true );
////        CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_Error2)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Error in metronome parameters. Replaced by '(metronome 60)'." << endl;
////        parser.parse_text("(metronome e. \"s\")");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 60 );
////        CHECK( pMM->is_visible() == true );
////        CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_LocationX)
////    {
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        parser.parse_text("(metronome 88 (dx 70))");
////        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 88 );
////        CHECK( pMM->is_visible() );
////        CHECK( pMM->get_user_location_x() == 70.0f );
////        CHECK( pMM->get_user_location_y() == 0.0f );
////        CHECK( pMM->is_visible() == true );
////        CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_NoVisible)
////    {
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        parser.parse_text("(metronome 88 noVisible)");
////        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 88 );
////        CHECK( pMM->get_user_location_x() == 0.0f );
////        CHECK( pMM->get_user_location_y() == 0.0f );
////        CHECK( pMM->is_visible() == false );
////        CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_Parenthesis)
////    {
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        parser.parse_text("(metronome 88 parenthesis (visible no))");
////        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 88 );
////        CHECK( pMM->get_user_location_x() == 0.0f );
////        CHECK( pMM->get_user_location_y() == 0.0f );
////        CHECK( pMM->is_visible() == false );
////        CHECK( pMM->has_parenthesis() == true );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_Ordering)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. ?" << endl;
////        parser.parse_text("(metronome 88 parenthesis (dx 7) noVisible)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 88 );
////        CHECK( pMM->is_visible() == false );
////        CHECK( pMM->has_parenthesis() == true );
////        CHECK( pMM->get_user_location_x() == 7.0f );
////        CHECK( pMM->get_user_location_y() == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Metronome_Error3)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'label:parentesis' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(metronome 88 parentesis (dx 7))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        CHECK( pMM != NULL );
////        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        CHECK( pMM->get_ticks_per_minute() == 88 );
////        CHECK( pMM->is_visible() == true );
////        CHECK( pMM->has_parenthesis() == false );
////        CHECK( pMM->get_user_location_x() == 7.0f );
////        CHECK( pMM->get_user_location_y() == 0.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // opt ------------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_BoolTrue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(opt StaffLines.StopAtFinalBarline true)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_option() == true );
////        ImoOptionInfo* pOpt = dynamic_cast<ImoOptionInfo*>( pIModel->get_root() );
////        CHECK( pOpt != NULL );
////        CHECK( pOpt->get_name() == "StaffLines.StopAtFinalBarline" );
////        CHECK( pOpt->get_type() == ImoOptionInfo::k_boolean );
////        CHECK( pOpt->get_bool_value() == true );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_BoolErrorValue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid value for option 'StaffLines.StopAtFinalBarline'. Option ignored." << endl;
////        parser.parse_text("(opt StaffLines.StopAtFinalBarline perhaps)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_ErrorName)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid option 'StaffLines.Funny'. Option ignored." << endl;
////        parser.parse_text("(opt StaffLines.Funny funny thing)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_LongOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(opt Render.SpacingValue 40)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoOptionInfo* pOpt = dynamic_cast<ImoOptionInfo*>( pIModel->get_root() );
////        CHECK( pOpt != NULL );
////        CHECK( pOpt->get_name() == "Render.SpacingValue" );
////        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_long );
////        CHECK( pOpt->get_long_value() == 40 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_LongErrorValue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid value for option 'Render.SpacingValue'. Option ignored." << endl;
////        parser.parse_text("(opt Render.SpacingValue perhaps)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_FloatOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(opt Render.SpacingFactor 0.536)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoOptionInfo* pOpt = dynamic_cast<ImoOptionInfo*>( pIModel->get_root() );
////        CHECK( pOpt != NULL );
////        CHECK( pOpt->get_name() == "Render.SpacingFactor" );
////        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_float );
////        CHECK( pOpt->get_float_value() == 0.536f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_FloatErrorValue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid value for option 'Render.SpacingFactor'. Option ignored." << endl;
////        parser.parse_text("(opt Render.SpacingFactor perhaps)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_ErrorMissingValue)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing value for option 'Render.SpacingFactor'. Option ignored." << endl;
////        parser.parse_text("(opt Render.SpacingFactor)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Opt_AddedToScore)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(score (vers 1.6)(opt StaffLines.StopAtFinalBarline true)(instrument (musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        CHECK( pScore->has_options() == true );
////        ImoOptionInfo* pOpt = pScore->get_option("StaffLines.StopAtFinalBarline");
////        CHECK( pOpt != NULL );
////        CHECK( pOpt->get_name() == "StaffLines.StopAtFinalBarline" );
////        CHECK( pOpt->get_type() == ImoOptionInfo::k_boolean );
////        CHECK( pOpt->get_bool_value() == true );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_DefaultOptReplaced)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(score (vers 1.6)"
////            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 30)"
////            "(instrument (musicData)))"
////        );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        CHECK( pScore->has_options() == true );
////        ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingMethod");
////        CHECK( pOpt != NULL );
////        CHECK( pOpt->get_name() == "Render.SpacingMethod" );
////        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_long );
////        CHECK( pOpt->get_long_value() == 1L );
////        pOpt = pScore->get_option("Render.SpacingValue");
////        CHECK( pOpt != NULL );
////        CHECK( pOpt->get_name() == "Render.SpacingValue" );
////        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_long );
////        CHECK( pOpt->get_long_value() == 30L );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_LastDefaultOptReplaced)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(score (vers 1.6)"
////            "(opt Render.SpacingFactor 4.0)(instrument (musicData)))"
////        );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        CHECK( pScore->has_options() == true );
////        ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
////        CHECK( pOpt != NULL );
////        CHECK( pOpt->get_name() == "Render.SpacingFactor" );
////        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_float );
////        CHECK( pOpt->get_float_value() == 4.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////
////    // nodes ----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_DeleteParameter)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'label:instrument' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(n c4 q instrument)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_note_type() == k_quarter );
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == k_step_C );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_DeleteNode)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid option 'StaffLines.Funny'. Option ignored." << endl;
////        parser.parse_text("(opt StaffLines.Funny true)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // spacer ---------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(spacer 70.5)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_spacer() == true );
////        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
////        CHECK( pSp != NULL );
////        CHECK( pSp->get_width() == 70.5f );
////        CHECK( pSp->get_staff() == 0 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_MissingWidth)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing width for spacer. Spacer ignored." << endl;
////        parser.parse_text("(spacer)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_Staff)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(spacer 70.5 p3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
////        CHECK( pSp != NULL );
////        CHECK( pSp->get_width() == 70.5f );
////        CHECK( pSp->get_staff() == 2 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_ErrorStaff)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid staff 'pan'. Replaced by 'p1'." << endl;
////        parser.parse_text("(spacer 70.5 pan)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
////        CHECK( pSp != NULL );
////        CHECK( pSp->get_width() == 70.5f );
////        CHECK( pSp->get_staff() == 0 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_ErrorMoreParams)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'label:more' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(spacer 70.5 more)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
////        CHECK( pSp != NULL );
////        CHECK( pSp->get_width() == 70.5f );
////        CHECK( pSp->get_staff() == 0 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_Attachment)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(spacer 70 (text \"andante\"))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
////        CHECK( pSp != NULL );
////        CHECK( pSp->get_width() == 70.0f );
////        CHECK( pSp->get_staff() == 0 );
////        CHECK( pSp->has_attachments() == true );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_ErrorAttachment)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'r' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(spacer 70 (r q))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
////        CHECK( pSp != NULL );
////        CHECK( pSp->get_width() == 70.0f );
////        CHECK( pSp->get_staff() == 0 );
////        CHECK( pSp->has_attachments() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Spacer_ErrorAttachment2)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'r' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(spacer 70 (r q)(text \"andante\"))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
////        CHECK( pSp != NULL );
////        CHECK( pSp->get_width() == 70.0f );
////        CHECK( pSp->get_staff() == 0 );
////        CHECK( pSp->has_attachments() == true );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // XmlTiesBuilder ----------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TiesBuilder_EndTieOk)
////    {
////        Document doc(m_libraryScope);
////        ImoNote* pStartNote = ImFactory::inject_note(&doc, k_step_E, 4, k_eighth, k_sharp);
////        ImoNote* pEndNote = ImFactory::inject_note(&doc, k_step_E, 4, k_eighth, k_sharp);
////
////        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pStartInfo->set_start(true);
////        pStartInfo->set_tie_number(12);
////        pStartInfo->set_note(pStartNote);
////
////        ImoTieDto* pEndInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pEndInfo->set_start(false);
////        pEndInfo->set_tie_number(12);
////        pEndInfo->set_note(pEndNote);
////
////        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
////        XmlTiesBuilder builder(cout, &a);
////        builder.add_item_info(pStartInfo);
////        builder.add_item_info(pEndInfo);
////
////        CHECK( pStartNote->is_tied_next() == true);
////        CHECK( pStartNote->is_tied_prev() == false);
////        CHECK( pEndNote->is_tied_next() == false);
////        CHECK( pEndNote->is_tied_prev() == true);
////
////        delete pStartNote;
////        delete pEndNote;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TiesBuilder_StartTieDuplicated)
////    {
////        Document doc(m_libraryScope);
////        stringstream errormsg;
////        stringstream expected;
////        expected << "Line 0. This tie has the same number than that defined in line 0. This tie will be ignored." << endl;
////
////        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pStartInfo->set_start(true);
////        pStartInfo->set_tie_number(12);
////
////        ImoTieDto* pOtherInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pOtherInfo->set_start(true);
////        pOtherInfo->set_tie_number(12);
////
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        XmlTiesBuilder builder(errormsg, &a);
////        builder.add_item_info(pStartInfo);
////        builder.add_item_info(pOtherInfo);
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TiesBuilder_ErrorNotesCanNotBeTied)
////    {
////        Document doc(m_libraryScope);
////        stringstream errormsg;
////        stringstream expected;
////        expected << "Line 0. Requesting to tie notes of different voice or pitch. Tie number 12 will be ignored." << endl;
////
////        ImoNote* startNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
////        startNote->set_octave(4);
////        startNote->set_step(2);
////        startNote->set_notated_accidentals(k_sharp);
////        startNote->set_note_type(k_eighth);
////
////        ImoNote* endNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
////        endNote->set_octave(3);
////        endNote->set_step(2);
////        endNote->set_notated_accidentals(k_sharp);
////        endNote->set_note_type(k_eighth);
////
////        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pStartInfo->set_start(true);
////        pStartInfo->set_tie_number(12);
////        pStartInfo->set_note(startNote);
////
////        ImoTieDto* pEndInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pEndInfo->set_start(false);
////        pEndInfo->set_tie_number(12);
////        pEndInfo->set_note(endNote);
////
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        XmlTiesBuilder builder(errormsg, &a);
////        builder.add_item_info(pStartInfo);
////        builder.add_item_info(pEndInfo);
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete startNote;
////        delete endNote;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TiesBuilder_ErrorNoStartInfo)
////    {
////        Document doc(m_libraryScope);
////        stringstream errormsg;
////        stringstream expected;
////        expected << "Line 0. No 'start/continue' elements for tie number 12. Tie ignored." << endl;
////
////        ImoTieDto* pEndInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pEndInfo->set_start(false);
////        pEndInfo->set_tie_number(12);
////
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        XmlTiesBuilder builder(errormsg, &a);
////        builder.add_item_info(pEndInfo);
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TiesBuilder_PendingTiesAtDeletion)
////    {
////        Document doc(m_libraryScope);
////        stringstream errormsg;
////        stringstream expected;
////        expected << "Line 0. No 'end' element for tie number 12. Tie ignored." << endl
////                 << "Line 0. No 'end' element for tie number 14. Tie ignored." << endl;
////
////        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pStartInfo->set_start(true);
////        pStartInfo->set_tie_number(12);
////
////        ImoTieDto* pOtherInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
////        pOtherInfo->set_start(true);
////        pOtherInfo->set_tie_number(14);
////
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        XmlTiesBuilder* pBuilder = LOMSE_NEW XmlTiesBuilder(errormsg, &a);
////        pBuilder->add_item_info(pStartInfo);
////        pBuilder->add_item_info(pOtherInfo);
////
////        delete pBuilder;
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TiesBuilder_InstrumentChangeError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. No 'end' element for tie number 12. Tie ignored." << endl
////                 << "Line 0. No 'start/continue' elements for tie number 12. Tie ignored." << endl;
////        parser.parse_text("(score (vers 1.6)(instrument (musicData (n c4 q (tie 12 start))))(instrument (musicData (n d4 e (tie 12 stop)))))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        ImoInstrument* pInstr = pScore->get_instrument(0);
////        ImoMusicData* pMusic = pInstr->get_musicdata();
////        CHECK( pMusic != NULL );
////
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == 0 );
////        CHECK( pNote->is_tied_next() == false );
////        CHECK( pNote->is_tied_prev() == false );
////
////        pInstr = pScore->get_instrument(1);
////        pMusic = pInstr->get_musicdata();
////        CHECK( pMusic != NULL );
////
////        it = pMusic->begin();
////
////        pNote = dynamic_cast<ImoNote*>(*it);
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == 1 );
////        CHECK( pNote->is_tied_next() == false );
////        CHECK( pNote->is_tied_prev() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // beam -----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Beam_Start)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(beam 12 +)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root()->is_beam_dto() == true );
////        ImoBeamDto* pInfo = dynamic_cast<ImoBeamDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_beam_number() == 12 );
////        CHECK( pInfo->get_beam_type(0) == ImoBeam::k_begin );
////        CHECK( pInfo->get_beam_type(1) == ImoBeam::k_none );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Beam_TreeLevels)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(beam 12 ++f)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoBeamDto* pInfo = dynamic_cast<ImoBeamDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_beam_number() == 12 );
////        CHECK( pInfo->get_beam_type(0) == ImoBeam::k_begin );
////        CHECK( pInfo->get_beam_type(1) == ImoBeam::k_begin );
////        CHECK( pInfo->get_beam_type(2) == ImoBeam::k_forward );
////        CHECK( pInfo->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pInfo->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pInfo->get_beam_type(5) == ImoBeam::k_none );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Beam_ErrorNum)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid beam number. Beam ignored." << endl;
////        parser.parse_text("(beam +)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root() == NULL );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Beam_ErrorType)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid beam type. Beam ignored." << endl;
////        parser.parse_text("(beam 34 empieza)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        CHECK( pIModel->get_root() == NULL );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamsBuilder_Destructor)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
////        parser.parse_text("(n c4 e (beam 14 +))");
////        MxlAnalyser* pA = LOMSE_NEW MxlAnalyser(errormsg, m_libraryScope, &doc);
////        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
////        delete pA;
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->is_beamed() == false );
////        CHECK( pNote->get_beam_type(0) == ImoBeam::k_none );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamsBuilder_BeamError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. No 'start/continue' elements for beam number 13. Beam ignored." << endl
////                 << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
////        parser.parse_text("(musicData (n c4 q. (beam 14 +)) (n d4 s (beam 13 -)))");
////        MxlAnalyser* pA = LOMSE_NEW MxlAnalyser(errormsg, m_libraryScope, &doc);
////        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
////        delete pA;
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1->is_beamed() == false );
////        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_none );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_beamed() == false );
////        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_none );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamsBuilder_BeamOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
////        parser.parse_text("(musicData (n c4 q. (beam 14 +)) (n d4 s (beam 14 -b)))");
////        MxlAnalyser* pA = LOMSE_NEW MxlAnalyser(errormsg, m_libraryScope, &doc);
////        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
////        delete pA;
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1->is_beamed() == true );
////        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
////        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_beamed() == true );
////        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
////        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_backward );
////        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamsBuilder_InstrumentChangeError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
////        parser.parse_text("(score (vers 1.6) (instrument (musicData (n c4 q. (beam 14 +)))) (instrument (musicData (n c4 e))))" );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        ImoInstrument* pInstr = pScore->get_instrument(0);
////        ImoMusicData* pMusic = pInstr->get_musicdata();
////        CHECK( pMusic != NULL );
////
////        ImoObj::children_iterator it = pMusic->begin();
////        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == 0 );
////        CHECK( pNote->is_beamed() == false );
////        CHECK( pNote->get_beam_type(0) == ImoBeam::k_none );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // beam (old syntax) ----------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamOld_ErrorInvalidG)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid parameter 'g+7'. Ignored." << endl;
////        parser.parse_text("(n c4 e g+7)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_note_type() == k_eighth );
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == k_step_C );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamOld_ErrorInvalidNote)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Requesting beaming a note longer than eighth. Beam ignored." << endl;
////        parser.parse_text("(n c4 w g+)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == 0 );
////        CHECK( pNote->get_note_type() == k_whole );
////        CHECK( pNote->is_beamed() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamOld_ErrorAlreadyOpen)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Requesting to start a beam (g+) but there is already an open beam. Beam ignored." << endl;
////        parser.parse_text("(musicData (n c4 s g+) (n e4 e g+))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_BeamOld_SES)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. No 'start' element for beam number 12. Tie ignored." << endl;
////        parser.parse_text("(musicData (n c4 s g+)(n e4 e)(n c4 s g-))");
////        MxlAnalyser* pA = LOMSE_NEW MxlAnalyser(errormsg, m_libraryScope, &doc);
////        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
////        delete pA;
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1->is_beamed() == true );
////        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
////        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_forward );
////        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2->is_beamed() == true );
////        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_continue );
////        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );
////
////        ++it;
////        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote3 != NULL );
////        CHECK( pNote3->is_beamed() == true );
////        CHECK( pNote3->get_beam_type(0) == ImoBeam::k_end );
////        CHECK( pNote3->get_beam_type(1) == ImoBeam::k_backward );
////        CHECK( pNote3->get_beam_type(2) == ImoBeam::k_none );
////        CHECK( pNote3->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pNote3->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pNote3->get_beam_type(5) == ImoBeam::k_none );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // XmlAutoBeamer -----------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_AutoBeamer_SE)
////    {
////        Document doc(m_libraryScope);
////
////        ImoBeam* pBeam = static_cast<ImoBeam*>( ImFactory::inject(k_imo_beam, &doc) );
////
////        ImoNote* pNote1 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
////        pNote1->set_note_type(k_16th);
////        ImoBeamDto dto1;
////        ImoBeamData* pData1 = ImFactory::inject_beam_data(&doc, &dto1);
////        pNote1->include_in_relation(&doc, pBeam, pData1);
////
////        ImoNote* pNote2 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
////        pNote2->set_note_type(k_eighth);
////        ImoBeamDto dto2;
////        ImoBeamData* pData2 = ImFactory::inject_beam_data(&doc, &dto2);
////        pNote2->include_in_relation(&doc, pBeam, pData2);
////
////        XmlAutoBeamer autobeamer(pBeam);
////        autobeamer.do_autobeam();
////
////        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
////        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_forward );
////        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );
////
////        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
////        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );
////
////        delete pNote1;
////        delete pNote2;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_AutoBeamer_EE)
////    {
////        Document doc(m_libraryScope);
////
////        ImoBeam* pBeam = static_cast<ImoBeam*>( ImFactory::inject(k_imo_beam, &doc) );
////        ImoNote* pNote1 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
////        pNote1->set_note_type(k_eighth);
////        ImoBeamDto dto1;
////        ImoBeamData* pData1 = ImFactory::inject_beam_data(&doc, &dto1);
////        pNote1->include_in_relation(&doc, pBeam, pData1);
////
////        ImoNote* pNote2 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
////        pNote2->set_note_type(k_eighth);
////        ImoBeamDto dto2;
////        ImoBeamData* pData2 = ImFactory::inject_beam_data(&doc, &dto2);
////        pNote2->include_in_relation(&doc, pBeam, pData2);
////
////        XmlAutoBeamer autobeamer(pBeam);
////        autobeamer.do_autobeam();
////
////        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
////        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );
////
////        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
////        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
////        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );
////
////        delete pNote1;
////        delete pNote2;
////    }
////
////    // tuplet new syntax ----------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tuplet_TypeError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid tuplet type. Tuplet ignored." << endl;
////        parser.parse_text("(t 5 start 3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tuplet_ActualNotes)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(t + 3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_tuplet_dto() == true );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_default );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tuplet_ErrorNormalNumRequired)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Tuplet: Missing or invalid normal notes number. Tuplet ignored." << endl;
////        parser.parse_text("(t 4 + 7)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletNormalNotes)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(t 2 + 7 4)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 7 );
////        CHECK( pInfo->get_normal_number() == 4 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_default );
////        CHECK( pInfo->get_show_number() == ImoTuplet::k_number_actual );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tuplet_NoBracket)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(t 2 + 3 2 (displayBracket no))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_no );
////        CHECK( pInfo->get_show_number() == ImoTuplet::k_number_actual );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tuplet_DisplayNormalNum)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(t 1 + 3 2 (displayNumber none))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_default );
////        CHECK( pInfo->get_show_number() == ImoTuplet::k_number_none );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Tuplet_ErrorLabelParameter)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid yes/no value 'false'. Replaced by default." << endl;
////        parser.parse_text("(t 1 + 3 2 (displayBracket false))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_default );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // tuplet old full syntax -----------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_TypeError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid tuplet type. Tuplet ignored." << endl;
////        parser.parse_text("(t start 3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ActualNotes)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(t + 3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root()->is_tuplet_dto() == true );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_default );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ErrorNormalNumRequired)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Tuplet: Missing or invalid normal notes number. Tuplet ignored." << endl;
////        parser.parse_text("(t + 7)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_NormalNotes)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(t + 7 4)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 7 );
////        CHECK( pInfo->get_normal_number() == 4 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_default );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_NoBracket)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(t + 3 noBracket)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_no );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ErrorLabelParameter)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'label:blue' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(t + 3 noBracket blue)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_no );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ErrorCompoundParameter)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'color' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(t + 3 <color>blue) noBracket)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_start_of_tuplet() == true );
////        CHECK( pInfo->get_actual_number() == 3 );
////        CHECK( pInfo->get_normal_number() == 2 );
////        CHECK( pInfo->get_show_bracket() == k_yesno_no );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletsBuilder_Destructor)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. No 'end' element for tuplet number 0. Tuplet ignored." << endl;
////        parser.parse_text("(n c4 e (t + 3))");
////        MxlAnalyser* pA = LOMSE_NEW MxlAnalyser(errormsg, m_libraryScope, &doc);
////        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
////        delete pA;
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pNote->find_attachment(k_imo_tuplet) == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletsBuilder_TupletOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(musicData "
////            "(n c4 e (t + 3)) (n e4 e) (n d4 e (t -)) )");
////        MxlAnalyser* pA = LOMSE_NEW MxlAnalyser(errormsg, m_libraryScope, &doc);
////        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
////        delete pA;
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL);
////        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
////        CHECK( pTuplet->is_tuplet() == true );
////        CHECK( pTuplet->get_num_objects() == 3 );
//////        cout << "num.objects = " << pTuplet->get_num_objects() << endl;
////        CHECK( pTuplet->get_actual_number() == 3 );
////        CHECK( pTuplet->get_normal_number() == 2 );
////
////        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
////            pTuplet->get_related_objects();
////        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
////        ImoNote* pNt1 = dynamic_cast<ImoNote*>( (*itN).first );
////        ++itN;
////        ImoNote* pNt2 = dynamic_cast<ImoNote*>( (*itN).first );
////        ++itN;
////        ImoNote* pNt3 = dynamic_cast<ImoNote*>( (*itN).first );
////
////
////        it = pMusic->begin();
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1 == pNt1 );
////        CHECK( pNote1->find_relation(k_imo_tuplet) == pTuplet );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2 == pNt2 );
////        CHECK( pNote2->find_relation(k_imo_tuplet) == pTuplet );
////
////        ++it;
////        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote3 != NULL );
////        CHECK( pNote3 == pNt3 );
////        CHECK( pNote3->find_relation(k_imo_tuplet) == pTuplet );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletsBuilder_InstrumentChangeError)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. No 'end' element for tuplet number 0. Tuplet ignored." << endl;
////        parser.parse_text("(score (vers 1.6) (instrument "
////            "(musicData (n c4 e (t + 3)))) (instrument (musicData (n c4 e))))" );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        ImoInstrument* pInstr = pScore->get_instrument(0);
////        ImoMusicData* pMusic = pInstr->get_musicdata();
////        CHECK( pMusic != NULL );
////
////        ImoObj::children_iterator it = pMusic->begin();
////        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == 0 );
////        CHECK( pNote->find_attachment(k_imo_tuplet_data) == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////
////    // tuplet (old tn/t- syntax) --------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ErrorInvalidParameter)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid parameter 't-7'. Ignored." << endl;
////        parser.parse_text("(n c4 e t-7)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_octave() == 4 );
////        CHECK( pNote->get_step() == 0 );
////        CHECK( pNote->find_attachment(k_imo_tuplet_data) == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(n c4 e t3)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ok2)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(n c4 e t7/6)");
////        MxlAnalyser* pA = LOMSE_NEW MxlAnalyser(errormsg, m_libraryScope, &doc);
////        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete pA;
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_ErrorAlreadyOpen)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Requesting to start a tuplet but there is already an open tuplet. Tuplet ignored." << endl;
////        parser.parse_text("(musicData (n c4 s t3) (n d4 e) (n e4 e t3))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL);
////        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
////        CHECK( pTuplet == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TupletOld_TupletOk)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(musicData (n c4 e t3) (n e4 e) (n d4 e t-))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL);
////        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
////        CHECK( pTuplet->get_num_objects() == 3 );
////        CHECK( pTuplet->get_actual_number() == 3 );
////        CHECK( pTuplet->get_normal_number() == 2 );
////
////        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
////            pTuplet->get_related_objects();
////        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
////        ImoNote* pNt1 = dynamic_cast<ImoNote*>( (*itN).first );
////        ++itN;
////        ImoNote* pNt2 = dynamic_cast<ImoNote*>( (*itN).first );
////        ++itN;
////        ImoNote* pNt3 = dynamic_cast<ImoNote*>( (*itN).first );
////
////        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote1 != NULL );
////        CHECK( pNote1 == pNt1 );
////
////        ++it;
////        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote2 != NULL );
////        CHECK( pNote2 == pNt2 );
////
////        ++it;
////        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote3 != NULL );
////        CHECK( pNote3 == pNt3 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // voice (element) ------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_Voice_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. ?" << endl;
////        parser.parse_text("(n c4 e (voice 7))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_voice() == 7 );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_Voice_Error)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid integer number 'no'. Replaced by '1'." << endl;
////        parser.parse_text("(n c4 e (voice no))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_voice() == 1 );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // staffNum (element) ---------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_StaffNum_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. ?" << endl;
////        parser.parse_text("(n c4 e (staffNum 2))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_staff() == 1 );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Note_StaffNum_Error)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid integer number 'alpha'. Replaced by '1'." << endl;
////        parser.parse_text("(n c4 e (staffNum alpha))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->get_staff() == 0 );
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // rest (full) ----------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Rest_Full)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(musicData "
////            "(r e (t + 3)(voice 3)(staffNum 2)) (r e (text \"Hello\")) (r e (t -)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////        ImoRest* pRest = dynamic_cast<ImoRest*>( *it );
////        CHECK( pRest != NULL);
////        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pRest->find_relation(k_imo_tuplet));
////        CHECK( pTuplet->get_num_objects() == 3 );
////
////        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
////            pTuplet->get_related_objects();
////        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
////        ImoRest* pNR1 = dynamic_cast<ImoRest*>( (*itN).first );
////        ++itN;
////        ImoRest* pNR2 = dynamic_cast<ImoRest*>( (*itN).first );
////        ++itN;
////        ImoRest* pNR3 = dynamic_cast<ImoRest*>( (*itN).first );
////
////        CHECK( (*it)->is_rest() == true );
////        ImoRest* pRest1 = dynamic_cast<ImoRest*>( *it );
////        CHECK( pRest1 != NULL );
////        CHECK( pRest1 == pNR1 );
////        CHECK( pRest1->get_voice() == 3 );
////        CHECK( pRest1->get_staff() == 1 );
////
////        ++it;
////        ImoRest* pRest2 = dynamic_cast<ImoRest*>( *it );
////        CHECK( pRest2 != NULL );
////        CHECK( pRest2 == pNR2 );
////        CHECK( pRest2->get_voice() == 3 );
////        CHECK( pRest2->get_staff() == 1 );
////        CHECK( pRest2->has_attachments() == true );
////
////        ++it;
////        ImoRest* pRest3 = dynamic_cast<ImoRest*>( *it );
////        CHECK( pRest3 != NULL );
////        CHECK( pRest3 == pNR3 );
////        CHECK( pRest3->get_voice() == 3 );
////        CHECK( pRest3->get_staff() == 1 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
//
//    // color ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, ImoColorDto)
//    {
//        ImoColorDto color;
//        CHECK( color.red() == 0 );
//        CHECK( color.green() == 0 );
//        CHECK( color.blue() == 0 );
//        CHECK( color.alpha() == 255 );
//        color.set_from_rgb_string("#ff3217");
//        CHECK( color.red() == 255 );
//        CHECK( color.green() == 50 );
//        CHECK( color.blue() == 23 );
//        CHECK( color.alpha() == 255 );
//        color.set_from_rgba_string("#fffe4580");
//        CHECK( color.red() == 255 );
//        CHECK( color.green() == 254 );
//        CHECK( color.blue() == 69 );
//        CHECK( color.alpha() == 128 );
//        CHECK( color.is_color_dto() == true );
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, ImoColor_Constructor)
//    {
//        ImoColorDto color(12,32,255,180);
//        CHECK( color.red() == 12 );
//        CHECK( color.green() == 32 );
//        CHECK( color.blue() == 255 );
//        CHECK( color.alpha() == 180 );
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, ImoColor_Error)
//    {
//        ImoColorDto color;
//        color.set_from_string("#ff3g17");
//        CHECK( color.red() == 0 );
//        CHECK( color.green() == 0 );
//        CHECK( color.blue() == 0 );
//        CHECK( color.alpha() == 255 );
//        color.set_from_string("#fff");
//        CHECK( color.red() == 0 );
//        CHECK( color.green() == 0 );
//        CHECK( color.blue() == 0 );
//        CHECK( color.alpha() == 255 );
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Color_ErrorInvalidData)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        expected << "Line 0. Missing or invalid color value. Must be #rrggbbaa. Color ignored." << endl;
//        parser.parse_text("<color>321700</color>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        CHECK( pIModel->get_root() == NULL );
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Color_Ok)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<color>#f0457f</color>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoColorDto* pColor = dynamic_cast<ImoColorDto*>( pIModel->get_root() );
//        CHECK( pColor != NULL );
//        CHECK( pColor->red() == 240 );
//        CHECK( pColor->green() == 69 );
//        CHECK( pColor->blue() == 127 );
//        CHECK( pColor->alpha() == 255 );
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        delete pIModel;
//    }
//
//    //TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Color_SetInParent)
//    //{
//    //    stringstream errormsg;
//    //    Document doc(m_libraryScope);
//    //    XmlParser parser;
//    //    stringstream expected;
//    //    //expected << "Line 0. " << endl;
//    //    parser.parse_text("(n c4 e <color>#f0457f))");
//    //    MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//    //    InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//    //    //cout << "[" << errormsg.str() << "]" << endl;
//    //    //cout << "[" << expected.str() << "]" << endl;
//    //    CHECK( errormsg.str() == expected.str() );
//
//    //    ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
//    //    CHECK( pNote != NULL );
//    //    Color& color = pNote->get_color();
//    //    CHECK( color.r == 240 );
//    //    CHECK( color.g == 69 );
//    //    CHECK( color.b == 127 );
//    //    CHECK( color.a == 255 );
//
//    //    delete pIModel;
//    //}
//
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Barline_Color)
////    {
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        parser.parse_text("(barline double <color>#ff0000))");
////        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
////        CHECK( pBarline != NULL );
////        CHECK( pBarline->get_type() == k_barline_double );
////        CHECK( pBarline->is_visible() );
////        CHECK( is_equal(pBarline->get_color(), Color(255,0,0)) );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // group ----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_All)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(group (name \"Group\")(abbrev \"G.\")"
////                "(symbol bracket)(joinBarlines no)"
////                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
////                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
////                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        CHECK( pIModel->get_root()->is_instr_group() == true );
////        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
////        CHECK( pGrp != NULL );
////        CHECK( pGrp->get_name() == "Group" );
////        CHECK( pGrp->get_abbrev() == "G." );
////        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
////        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
////        CHECK( pGrp->get_num_instruments() == 3 );
////
////        //AWARE: Group doesn't get ownership of instruments. Therefore, as
////        //group is not included in a score, we must delete instruments.
////        delete pGrp->get_instrument(0);
////        delete pGrp->get_instrument(1);
////        delete pGrp->get_instrument(2);
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_NoName)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(group (abbrev \"G.\")"
////                "(symbol bracket)(joinBarlines no)"
////                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
////                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
////                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
////        CHECK( pGrp != NULL );
////        CHECK( pGrp->get_name() == "" );
////        CHECK( pGrp->get_abbrev() == "G." );
////        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
////        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
////        CHECK( pGrp->get_num_instruments() == 3 );
////
////        //AWARE: Group doesn't get ownership of instruments. Therefore, as
////        //group is not included in a score, we must delete instruments.
////        delete pGrp->get_instrument(0);
////        delete pGrp->get_instrument(1);
////        delete pGrp->get_instrument(2);
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_NoAbbrev)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(group (name \"Group\")"
////                "(symbol bracket)(joinBarlines no)"
////                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
////                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
////                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
////        CHECK( pGrp != NULL );
////        CHECK( pGrp->get_name() == "Group" );
////        CHECK( pGrp->get_abbrev() == "" );
////        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
////        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
////        CHECK( pGrp->get_num_instruments() == 3 );
////
////        //AWARE: Group doesn't get ownership of instruments. Therefore, as
////        //group is not included in a score, we must delete instruments.
////        delete pGrp->get_instrument(0);
////        delete pGrp->get_instrument(1);
////        delete pGrp->get_instrument(2);
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_NoNameAbbrev)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(group (symbol bracket)(joinBarlines no)"
////                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
////                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
////                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
////        CHECK( pGrp != NULL );
////        CHECK( pGrp->get_name() == "" );
////        CHECK( pGrp->get_abbrev() == "" );
////        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
////        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
////        CHECK( pGrp->get_num_instruments() == 3 );
////
////        //AWARE: Group doesn't get ownership of instruments. Therefore, as
////        //group is not included in a score, we must delete instruments.
////        delete pGrp->get_instrument(0);
////        delete pGrp->get_instrument(1);
////        delete pGrp->get_instrument(2);
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_ErrorSymbol)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid group symbol. Must be 'none', 'brace' or 'bracket'. Group ignored." << endl;
////        parser.parse_text("(group (symbol good)(joinBarlines no)"
////                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
////                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
////                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        CHECK( pIModel->get_root() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_ErrorJoin)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid boolean value 'perhaps'. Replaced by '1'." << endl;
////        parser.parse_text("(group (symbol brace)(joinBarlines perhaps)"
////                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
////                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
////                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
////        CHECK( pGrp != NULL );
////        CHECK( pGrp->get_name() == "" );
////        CHECK( pGrp->get_abbrev() == "" );
////        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_standard );
////        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_brace );
////        CHECK( pGrp->get_num_instruments() == 3 );
////
////        //AWARE: Group doesn't get ownership of instruments. Therefore, as
////        //group is not included in a score, we must delete instruments.
////        delete pGrp->get_instrument(0);
////        delete pGrp->get_instrument(1);
////        delete pGrp->get_instrument(2);
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_ErrorMissingInstruments)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing instruments in group!. Group ignored." << endl;
////        parser.parse_text("(group (symbol brace)(joinBarlines true))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Group_ErrorInInstrument)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'n' unknown or not possible here. Ignored." << endl;
////        parser.parse_text("(group (symbol brace)(joinBarlines no)"
////                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
////                "(n c4 q)"
////                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
////        CHECK( pGrp != NULL );
////        CHECK( pGrp->get_name() == "" );
////        CHECK( pGrp->get_abbrev() == "" );
////        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
////        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_brace );
////        CHECK( pGrp->get_num_instruments() == 2 );
////
////        //AWARE: Group doesn't get ownership of instruments. Therefore, as
////        //group is not included in a score, we must delete instruments.
////        delete pGrp->get_instrument(0);
////        delete pGrp->get_instrument(1);
////        delete tree->get_root();
////        delete pIModel;
////    }
////    // pageLayout -----------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_PageLayout_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(pageLayout (pageSize 14000 10000)(pageMargins 1000 1200 3000 2500 4000) landscape)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoPageInfo* pInfo = dynamic_cast<ImoPageInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_page_info() == true );
////        CHECK( pInfo->get_left_margin() == 1000.0f );
////        CHECK( pInfo->get_top_margin() == 1200.0f );
////        CHECK( pInfo->get_right_margin() == 3000.0f );
////        CHECK( pInfo->get_bottom_margin() == 2500.0f );
////        CHECK( pInfo->get_binding_margin() == 4000.0f );
////        CHECK( pInfo->get_page_width() == 14000.0f );
////        CHECK( pInfo->get_page_height() == 10000.0f );
////        CHECK( pInfo->is_portrait() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_PageLayout_AddedToScore)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(score (vers 1.6)(pageLayout (pageSize 14000 10000)(pageMargins 1000 1200 3000 2500 4000) landscape)(instrument (musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        ImoPageInfo* pInfo = pScore->get_page_info();
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_page_info() == true );
////        CHECK( pInfo->get_left_margin() == 1000.0f );
////        CHECK( pInfo->get_top_margin() == 1200.0f );
////        CHECK( pInfo->get_right_margin() == 3000.0f );
////        CHECK( pInfo->get_bottom_margin() == 2500.0f );
////        CHECK( pInfo->get_binding_margin() == 4000.0f );
////        CHECK( pInfo->get_page_width() == 14000.0f );
////        CHECK( pInfo->get_page_height() == 10000.0f );
////        CHECK( pInfo->is_portrait() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_PageLayout_AddedToDocument)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("<lenmusdoc vers='0.0'>(pageLayout (pageSize 14000 10000)(pageMargins 1000 1200 3000 2500 4000) landscape)(content))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
////        CHECK( pDoc != NULL );
////        ImoPageInfo* pInfo = pDoc->get_page_info();
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_page_info() == true );
////        CHECK( pInfo->get_left_margin() == 1000.0f );
////        CHECK( pInfo->get_top_margin() == 1200.0f );
////        CHECK( pInfo->get_right_margin() == 3000.0f );
////        CHECK( pInfo->get_bottom_margin() == 2500.0f );
////        CHECK( pInfo->get_binding_margin() == 4000.0f );
////        CHECK( pInfo->get_page_width() == 14000.0f );
////        CHECK( pInfo->get_page_height() == 10000.0f );
////        CHECK( pInfo->is_portrait() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
//
//    // defineStyle ----------------------------------------------------------------------
//
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_DefineStyle)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("<defineStyle><name>Composer\" "
////            "<font><font-name>Times New Roman\" 14pt bold-italic) <color>#00fe0f7f))" );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Composer" );
////        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
////        CHECK( pStyle->font_name() == "Times New Roman" );
////        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
////        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
////        CHECK( pStyle->font_size() == 14 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_DefineStyle_StyleAdded)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(score (vers 1.6)<defineStyle><name>Header1\" <font><font-name>Times New Roman\" 14pt bold-italic) <color>#00fe0f7f))(instrument (musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        ImoStyle* pStyle = pScore->find_style("Header1");
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Header1" );
////        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
////        CHECK( pStyle->font_name() == "Times New Roman" );
////        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
////        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
////        CHECK( pStyle->font_size() == 14 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, DefineStyle_MarginBottom)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("<defineStyle><name>Composer\" "
////            "<color>#00fe0f7f)(margin-bottom 2) )" );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Composer" );
////        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
////        CHECK( pStyle->margin_bottom() == 2.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, DefineStyle_FontProperties)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<defineStyle>"
//                "<name>Composer</name>"
//                "<font-name>Arial</font-name>"
//                "<font-size>14pt</font-size>"
//                "<font-style>italic</font-style>"
//                "<font-weight>bold</font-weight>"
//                "<font-file>FreeSans.ttf</font-file>"
//            "</defineStyle>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
//        CHECK( pStyle != NULL );
//        CHECK( pStyle->get_name() == "Composer" );
//        CHECK( pStyle->font_name() == "Arial" );
//        CHECK( pStyle->font_file() == "FreeSans.ttf" );
//        CHECK( pStyle->font_size() == 14 );
//        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
//        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
//
//        delete pIModel;
//    }
//
////    TEST_FIXTURE(MxlAnalyserTestFixture, DefineStyle_MarginProperties)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("<defineStyle><name>Composer\" "
////            "(margin-top 3)(margin-bottom 2)(margin-left 5)(margin-right 7) )" );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Composer" );
////        CHECK( pStyle->margin_top() == 3.0f );
////        CHECK( pStyle->margin_bottom() == 2.0f );
////        CHECK( pStyle->margin_left() == 5.0f );
////        CHECK( pStyle->margin_right() == 7.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, DefineStyle_Margin)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("<defineStyle><name>Composer\" "
////            "(margin 0.5) )" );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Composer" );
////        CHECK( pStyle->margin_top() == 0.5f );
////        CHECK( pStyle->margin_bottom() == 0.5f );
////        CHECK( pStyle->margin_left() == 0.5f );
////        CHECK( pStyle->margin_right() == 0.5f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, DefineStyle_LineHeight)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("<defineStyle><name>Composer\" "
////            "(line-height 1.2) )" );
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Composer" );
////        CHECK( pStyle->line_height() == 1.2f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////
////    // title ----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Title_MissingAll)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. title: missing mandatory element 'label'." << endl
////                 << "Line 0. title: missing mandatory element 'string'." << endl;
////        parser.parse_text("(title)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Title_MissingString)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. title: missing mandatory element 'string'." << endl;
////        parser.parse_text("(title center)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Title_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(title center \"Moonlight sonata\")");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreTitle* pTitle = dynamic_cast<ImoScoreTitle*>( pIModel->get_root() );
////        CHECK( pTitle != NULL );
////        CHECK( pTitle->get_text() == "Moonlight sonata" );
////        CHECK( pTitle->get_h_align() == k_halign_center );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Title_AddedToScore)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(score (vers 1.6)(title center \"Moonlight sonata\")(instrument (musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        std::list<ImoScoreTitle*>& titles = pScore->get_titles();
////        std::list<ImoScoreTitle*>::iterator it = titles.begin();
////        CHECK( it != titles.end() );
////        ImoScoreTitle* pTitle = *it;
////        CHECK( pTitle != NULL );
////        CHECK( pTitle->get_text() == "Moonlight sonata" );
////        CHECK( pTitle->get_h_align() == k_halign_center );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Title_Style)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(title center \"Moonlight sonata\" (style \"Header1\"))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreTitle* pTitle = dynamic_cast<ImoScoreTitle*>( pIModel->get_root() );
////        CHECK( pTitle != NULL );
////        CHECK( pTitle->get_text() == "Moonlight sonata" );
////        CHECK( pTitle->get_h_align() == k_halign_center );
////        ImoStyle* pStyle = pTitle->get_style();
////        CHECK( pStyle == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Title_StyleAdded)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(score (vers 1.6)"
////            "<defineStyle><name>Header1\" <font><font-name>Times New Roman\" 14pt bold-italic) <color>#00fe0f7f))"
////            "(title center \"Moonlight sonata\" (style \"Header1\"))"
////            "(instrument (musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore != NULL );
////        std::list<ImoScoreTitle*>& titles = pScore->get_titles();
////        std::list<ImoScoreTitle*>::iterator it = titles.begin();
////        CHECK( it != titles.end() );
////        ImoScoreTitle* pTitle = *it;
////        CHECK( pTitle != NULL );
////        CHECK( pTitle->get_text() == "Moonlight sonata" );
////        CHECK( pTitle->get_h_align() == k_halign_center );
////        ImoStyle* pStyle = pTitle->get_style();
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Header1" );
////        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
////        CHECK( pStyle->font_name() == "Times New Roman" );
////        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
////        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
////        CHECK( pStyle->font_size() == 14 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Title_Location)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(title right \"F. Chopin\" (style \"Composer\")(dy 30))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreTitle* pTitle = dynamic_cast<ImoScoreTitle*>( pIModel->get_root() );
////        CHECK( pTitle != NULL );
////        CHECK( pTitle->get_text() == "F. Chopin" );
////        CHECK( pTitle->get_h_align() == k_halign_right );
////        CHECK( pTitle->get_user_location_x() == 0.0f );
////        CHECK( pTitle->get_user_location_y() == 30.0f );
////        ImoStyle* pStyle = pTitle->get_style();
////        CHECK( pStyle == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // line -----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Line_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))"
////            "(endPoint (dx 80.0)(dy -10.0))(width 2.0)<color>#ff0000)(lineStyle solid)"
////            "(lineCapStart arrowhead)(lineCapEnd none))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
////        CHECK( pLine != NULL );
////        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
////        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
////        CHECK( pLine->get_line_style() == k_line_solid );
////        CHECK( pLine->get_start_edge() == k_edge_normal );
////        CHECK( pLine->get_end_edge() == k_edge_normal );
////        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
////        CHECK( pLine->get_end_cap() == k_cap_none );
////        CHECK( is_equal(pLine->get_color(), Color(255,0,0,255)) );
////        CHECK( pLine->get_line_width() == 2.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Line_OnlyMandatory)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
////        CHECK( pLine != NULL );
////        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
////        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
////        CHECK( pLine->get_line_style() == k_line_solid );
////        CHECK( pLine->get_start_edge() == k_edge_normal );
////        CHECK( pLine->get_end_edge() == k_edge_normal );
////        CHECK( pLine->get_start_cap() == k_cap_none );
////        CHECK( pLine->get_end_cap() == k_cap_none );
////        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
////        CHECK( pLine->get_line_width() == 1.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Line_NoColor)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0))(width 2.0)(lineStyle solid)(lineCapStart arrowhead)(lineCapEnd diamond))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
////        CHECK( pLine != NULL );
////        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
////        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
////        CHECK( pLine->get_line_style() == k_line_solid );
////        CHECK( pLine->get_start_edge() == k_edge_normal );
////        CHECK( pLine->get_end_edge() == k_edge_normal );
////        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
////        CHECK( pLine->get_end_cap() == k_cap_diamond );
////        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
////        CHECK( pLine->get_line_width() == 2.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Line_ErrorCap)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'lineCap': Invalid value 'diamont'. Replaced by 'none'." << endl;
////        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0))(width 2.0)(lineStyle dot)(lineCapStart arrowhead)(lineCapEnd diamont))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
////        CHECK( pLine != NULL );
////        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
////        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
////        CHECK( pLine->get_line_style() == k_line_dot );
////        CHECK( pLine->get_start_edge() == k_edge_normal );
////        CHECK( pLine->get_end_edge() == k_edge_normal );
////        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
////        CHECK( pLine->get_end_cap() == k_cap_none );
////        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
////        CHECK( pLine->get_line_width() == 2.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Line_ErrorLineStyle)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Element 'lineStyle': Invalid value 'simple'. Replaced by 'solid'." << endl;
////        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0))(width 2.0)(lineStyle simple))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
////        CHECK( pLine != NULL );
////        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
////        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
////        CHECK( pLine->get_line_style() == k_line_solid );
////        CHECK( pLine->get_start_edge() == k_edge_normal );
////        CHECK( pLine->get_end_edge() == k_edge_normal );
////        CHECK( pLine->get_start_cap() == k_cap_none );
////        CHECK( pLine->get_end_cap() == k_cap_none );
////        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
////        CHECK( pLine->get_line_width() == 2.0f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // textBox --------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TextBox_Minimum)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. textbox: missing mandatory element 'dx'." << endl
////        //         << "Line 0. textbox: missing mandatory element 'dy'." << endl;
////        parser.parse_text("(textbox (dx 50)(dy 5)"
////            "(size (width 300)(height 150))"
////            "(text \"This is a test of a textbox\" (style \"Textbox\")))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoTextBox* pTB = dynamic_cast<ImoTextBox*>( pIModel->get_root() );
////        CHECK( pTB != NULL );
////        CHECK( pTB->get_text() == "This is a test of a textbox" );
////        CHECK( pTB->has_anchor_line() == false );
////        ImoTextBlockInfo* pInfo = pTB->get_box_info();
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_textblock_info() == true );
////        CHECK( pInfo->get_height() == 150.0f );
////        CHECK( pInfo->get_width() == 300.0f );
////        CHECK( pInfo->get_position() == TPoint(50.0f, 5.0f) );
////        CHECK( is_equal(pInfo->get_bg_color(), Color(255,255,255,255)) );
////        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,0,255)) );
////        CHECK( pInfo->get_border_width() == 1.0f );
////        CHECK( pInfo->get_border_style() == k_line_solid );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TextBox_Full)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. textbox: missing mandatory element 'dx'." << endl
////        //         << "Line 0. textbox: missing mandatory element 'dy'." << endl;
////        parser.parse_text("(textbox (dx 50)(dy 5)"
////            "(size (width 300)(height 150))"
////            "<color>#fffe0b)"
////            "(border (width 5)(lineStyle dot)<color>#0000fd))"
////            "(text \"This is a test of a textbox\" (style \"Textbox\"))"
////            "(anchorLine (dx 40)(dy 70)(lineStyle dot)<color>#ff0a00)(width 3.5)"
////                        "(lineCapEnd arrowhead)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoTextBox* pTB = dynamic_cast<ImoTextBox*>( pIModel->get_root() );
////        CHECK( pTB != NULL );
////        CHECK( pTB->get_text() == "This is a test of a textbox" );
////
////        ImoTextBlockInfo* pInfo = pTB->get_box_info();
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_textblock_info() == true );
////        CHECK( pInfo->get_height() == 150.0f );
////        CHECK( pInfo->get_width() == 300.0f );
////        CHECK( pInfo->get_position() == TPoint(50.0f, 5.0f) );
////        CHECK( is_equal(pInfo->get_bg_color(), Color(255,254,11,255)) );
////        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,253,255)) );
////        CHECK( pInfo->get_border_width() == 5.0f );
////        CHECK( pInfo->get_border_style() == k_line_dot );
////
////        CHECK( pTB->has_anchor_line() == true );
////        ImoLineStyle* pLine = pTB->get_anchor_line_info();
////        CHECK( pLine != NULL );
////        CHECK( pLine->is_line_style() == true );
////        CHECK( pLine->get_start_point() == TPoint(0.0f, 0.0f) );
////        CHECK( pLine->get_end_point() == TPoint(40.0f, 70.0f) );
////        CHECK( pLine->get_line_style() == k_line_dot );
////        CHECK( pLine->get_start_edge() == k_edge_normal );
////        CHECK( pLine->get_end_edge() == k_edge_normal );
////        CHECK( pLine->get_start_cap() == k_cap_none );
////        CHECK( pLine->get_end_cap() == k_cap_arrowhead );
////        CHECK( is_equal(pLine->get_color(), Color(255,10,0,255)) );
////        CHECK( pLine->get_width() == 3.5f );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_TextBox_AddedToNote)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(n c4 q (textbox (dx 50)(dy 5)"
////            "(size (width 300)(height 150))"
////            "(text \"This is a test of a textbox\" (style \"Textbox\"))))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
////        CHECK( pNote != NULL );
////        CHECK( pNote->has_attachments() == true );
////        ImoTextBox* pTB = dynamic_cast<ImoTextBox*>( pNote->get_attachment(0) );
////        CHECK( pTB != NULL );
////        CHECK( pTB->get_text() == "This is a test of a textbox" );
////
////        ImoTextBlockInfo* pInfo = pTB->get_box_info();
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->is_textblock_info() == true );
////        CHECK( pInfo->get_height() == 150.0f );
////        CHECK( pInfo->get_width() == 300.0f );
////        CHECK( pInfo->get_position() == TPoint(50.0f, 5.0f) );
////        CHECK( is_equal(pInfo->get_bg_color(), Color(255,255,255,255)) );
////        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,0,255)) );
////        CHECK( pInfo->get_border_width() == 1.0f );
////        CHECK( pInfo->get_border_style() == k_line_solid );
////
////        CHECK( pTB->has_anchor_line() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // cursor ---------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Cursor_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(cursor 1 2 64.0 34)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoCursorInfo* pInfo = dynamic_cast<ImoCursorInfo*>( pIModel->get_root() );
////        CHECK( pInfo->is_cursor_info() == true );
////        CHECK( pInfo->get_instrument() == 1 );
////        CHECK( pInfo->get_staff() == 2 );
////        CHECK( pInfo->get_time() == k_duration_quarter );
////        CHECK( pInfo->get_id() == 34L );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Cursor_AddedToScore)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. 'cursor' in score is obsolete. Now must be in 'lenmusdoc' element. Ignored." << endl;
////        parser.parse_text("(score (vers 1.6)(cursor 1 2 64.0 34)(instrument (musicData)))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
////        CHECK( pScore->is_score() == true );
////        //CHECK( pScore->get_instrument() == 1 );
////        //CHECK( pScore->get_staff() == 2 );
////        //CHECK( pScore->get_time() == k_duration_quarter );
////        //CHECK( pScore->get_id() == 34L );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    //TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Cursor_AddedToDocument)
////    //{
////    //    stringstream errormsg;
////    //    XmlParser parser;
////    //    stringstream expected;
////    //    //expected << "Line 0. " << endl;
////    //    parser.parse_text("<lenmusdoc vers='0.0'>"
////    //        "(settings (cursor 1 2 64.0 34)) (content))");
////    //    MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////    //    InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////    //    //cout << "[" << errormsg.str() << "]" << endl;
////    //    //cout << "[" << expected.str() << "]" << endl;
////    //    CHECK( errormsg.str() == expected.str() );
////
////    //    ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
////    //    CHECK( pDoc->is_document() == true );
////    //    //CHECK( pScore->get_instrument() == 1 );
////    //    //CHECK( pScore->get_staff() == 2 );
////    //    //CHECK( pScore->get_time() == k_duration_quarter );
////    //    //CHECK( pScore->get_id() == 34L );
////
////    //    delete tree->get_root();
////    //    delete pIModel;
////    //}
////
////    // figuredBass ----------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_FiguredBass_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(figuredBass \"7 5 2\")");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoFiguredBass* pFB = dynamic_cast<ImoFiguredBass*>( pIModel->get_root() );
////        CHECK( pFB->is_figured_bass() == true );
////        //cout << "FB ='" << pFB->get_figured_bass_string() << "'" << endl;
////        CHECK( pFB->get_figured_bass_string() == "7 5 2" );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    // staff ----------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Staff_NoNumber)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Missing or invalid staff number. Staff info ignored." << endl;
////        parser.parse_text("(staff (staffType ossia))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() == NULL );
////        //ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
////        //CHECK( pMM != NULL );
////        //CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
////        //CHECK( pMM->get_ticks_per_minute() == 88 );
////        //CHECK( pMM->is_visible() == true );
////        //CHECK( pMM->has_parenthesis() == false );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Staff_InvalidType)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid staff type 'bonito'. 'regular' staff assumed." << endl;
////        parser.parse_text("(staff 2 (staffType bonito))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() != NULL );
////        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_staff_number() == 1 );
////        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_regular );
////        CHECK( pInfo->get_staff_margin() == 1000.0f );
////        CHECK( pInfo->get_line_spacing() == 180.0f );
////        CHECK( pInfo->get_height() == 735.0f );
////        CHECK( pInfo->get_line_thickness() == 15.0f );
////        CHECK( pInfo->get_num_lines() == 5 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Staff_InvalidLines)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid staff. Num lines must be greater than zero. Five assumed." << endl;
////        parser.parse_text("(staff 2 (staffType ossia)(staffLines 0))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() != NULL );
////        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_staff_number() == 1 );
////        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
////        CHECK( pInfo->get_staff_margin() == 1000.0f );
////        CHECK( pInfo->get_line_spacing() == 180.0f );
////        CHECK( pInfo->get_height() == 735.0f );
////        CHECK( pInfo->get_line_thickness() == 15.0f );
////        CHECK( pInfo->get_num_lines() == 5 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Staff_InvalidLinesSpacing)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Invalid real number 'five'. Replaced by '180'." << endl;
////        parser.parse_text("(staff 2 (staffType ossia)(staffLines 5)"
////            "(staffSpacing five) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() != NULL );
////        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_staff_number() == 1 );
////        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
////        CHECK( pInfo->get_staff_margin() == 1000.0f );
////        CHECK( pInfo->get_line_spacing() == 180.0f );
////        CHECK( pInfo->get_height() == 735.0f );
////        CHECK( pInfo->get_line_thickness() == 15.0f );
////        CHECK( pInfo->get_num_lines() == 5 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Staff_LinesSpacing)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(staff 2 (staffType ossia)(staffLines 5)"
////            "(staffSpacing 200.0) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() != NULL );
////        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_staff_number() == 1 );
////        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
////        CHECK( pInfo->get_staff_margin() == 1000.0f );
////        CHECK( pInfo->get_line_spacing() == 200.0f );
////        CHECK( pInfo->get_height() == 815.0f );
////        CHECK( pInfo->get_line_thickness() == 15.0f );
////        CHECK( pInfo->get_num_lines() == 5 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Staff_Ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(staff 2 (staffType ossia)(staffLines 4)"
////            "(staffSpacing 200.0)(staffDistance 800)(lineThickness 20.5) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() != NULL );
////        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_staff_number() == 1 );
////        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
////        CHECK( pInfo->get_staff_margin() == 800.0f );
////        CHECK( pInfo->get_line_spacing() == 200.0f );
////        CHECK( pInfo->get_height() == 620.5f );
////        CHECK( pInfo->get_line_thickness() == 20.5f );
////        CHECK( pInfo->get_num_lines() == 4 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, Analyser_Staff_AddedToInstrument)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(instrument (staves 2)(staff 2 (staffType ossia)"
////            "(staffLines 4)(staffSpacing 200.0)(staffDistance 800)(lineThickness 20.5))"
////            "(musicData ))");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pIModel->get_root() != NULL );
////        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
////        CHECK( pInstr != NULL );
////        ImoStaffInfo* pInfo = pInstr->get_staff(1);
////        CHECK( pInfo != NULL );
////        CHECK( pInfo->get_staff_number() == 1 );
////        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
////        CHECK( pInfo->get_staff_margin() == 800.0f );
////        CHECK( pInfo->get_line_spacing() == 200.0f );
////        CHECK( pInfo->get_height() == 620.5f );
////        CHECK( pInfo->get_line_thickness() == 20.5f );
////        CHECK( pInfo->get_num_lines() == 4 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
//
//    // textItem -------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, TextItem)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<txt>This is a text</txt>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_text_item() == true );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
//        CHECK( pText != NULL );
//        CHECK( pText->get_text() == "This is a text" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_text_item)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("<txt id='10'>This is a text</txt>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_text_item() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, TextItem_MissingText)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        expected << "Line 0. txt: missing mandatory value 'string'. Element <txt> ignored." << endl;
//        parser.parse_text("<txt></txt>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
//        CHECK( pText == NULL );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, TextItem_Style)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>Title</name><color>#00fe0f7f</color></defineStyle>"
//                "</styles>"
//                "<content><para><txt style='Title'>This is a text</txt></para></content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        CHECK( pDoc->get_num_content_items() == 1 );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
//        CHECK( pPara != NULL );
//        CHECK( pPara->get_num_items() == 1 );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
//        CHECK( pText != NULL );
//        CHECK( pText->get_text() == "This is a text" );
//        ImoStyle* pStyle = pText->get_style();
//        CHECK( pStyle != NULL );
//        CHECK( pStyle->get_name() == "Title" );
//
//        delete pIModel;
//    }
//
//    // para -----------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, paragraph_creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src = "<para>This is a paragraph</para>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_paragraph() == true );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_paragraph)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("<para id='10'>This is a paragraph</para>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_paragraph() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Paragraph_TextItemAdded)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<para>This is a paragraph</para>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
//        CHECK( pPara->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
//        CHECK( pItem->get_text() == "This is a paragraph" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Paragraph_LinkItemAdded)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text(
//            "<para><link url='This is the url'>This is the link</link></para>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
//        CHECK( pPara->get_num_items() == 1 );
//        ImoLink* pLink = dynamic_cast<ImoLink*>( pPara->get_first_item() );
//        CHECK( pLink != NULL );
//        CHECK( pLink->get_url() == "This is the url" );
//        CHECK( pLink->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
//        CHECK( pItem->get_text() == "This is the link" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_link)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<link id='10' url='This is the url'>This is the link</link>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_link() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Paragraph_ManyItems)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src = "<para>This is a paragraph <txt style='bold'>with two items.</txt></para>";
//        parser.parse_text(src);
//        //parser.parse_text("<para>(txt \"This is a paragraph\")"
//        //    "(txt \" with two items.\") )");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
//        CHECK( pPara->get_num_items() == 2 );
//        TreeNode<ImoObj>::children_iterator it = pPara->begin();
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
//        CHECK( pItem->get_text() == "This is a paragraph " );
//        ++it;
//        pItem = dynamic_cast<ImoTextItem*>( *it );
//        CHECK( pItem->get_text() == "with two items." );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Paragraph_RecognizedAsContent)
//    {
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        parser.parse_text("<lenmusdoc vers='0.0'><content>"
//            "<para>Hello world!</para></content></lenmusdoc>");
//        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
//        CHECK( pPara != NULL );
//        CHECK( pPara->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
//        CHECK( pItem->get_text() == "Hello world!" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Paragraph_Style)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>Credits</name><color>#00fe0f7f</color></defineStyle>"
//                "</styles>"
//                "<content><para style='Credits'>Hello world!</para></content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
//        CHECK( pPara != NULL );
//        ImoStyle* pStyle = pPara->get_style();
//        CHECK( pStyle != NULL );
//        CHECK( pStyle->get_name() == "Credits" );
//        CHECK( pPara->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
//        CHECK( pItem->get_text() == "Hello world!" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Paragraph_DefaultStyle)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<lenmusdoc vers='0.0'><content>"
//            "<para>Hello world!</para></content></lenmusdoc>");
//        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
//        CHECK( pPara != NULL );
//        ImoStyle* pStyle = pPara->get_style();
//        CHECK( pStyle->get_name() == "Default style" );
//        CHECK( pPara->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
//        CHECK( pItem->get_text() == "Hello world!" );
//
//        delete pIModel;
//    }
//
//    // section --------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Section_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<section level='1'>This is a header</section>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_heading() == true );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_section)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("<section id='10' level='1'>This is a header</section>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_heading() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Section_TextItemAdded)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<section level='3'>This is a header</section>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
//        CHECK( pH->get_num_items() == 1 );
//        CHECK( pH->get_level() == 3 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
//        CHECK( pItem->get_text() == "This is a header" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Section_no_level)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        expected << "Line 0. section: missing 'level' attribute. Level 1 assumed." << endl;
//        parser.parse_text("<section>This is a header</section>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
//        CHECK( pH->get_num_items() == 1 );
//        CHECK( pH->get_level() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
//        CHECK( pItem->get_text() == "This is a header" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Section_invalid_level)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        expected << "Line 0. Invalid integer number 'apple'. Replaced by '1'." << endl;
//        parser.parse_text("<section level='apple'>This is a header</section>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
//        CHECK( pH->get_num_items() == 1 );
//        CHECK( pH->get_level() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
//        CHECK( pItem->get_text() == "This is a header" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Section_ManyItems)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<section level='1'>This is a header"
//            "<txt> with two items.</txt></section>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
//        CHECK( pH->get_num_items() == 2 );
//        TreeNode<ImoObj>::children_iterator it = pH->begin();
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
//        CHECK( pItem->get_text() == "This is a header" );
//        ++it;
//        pItem = dynamic_cast<ImoTextItem*>( *it );
//        CHECK( pItem->get_text() == " with two items." );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Section_RecognizedAsContent)
//    {
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        parser.parse_text("<lenmusdoc vers='0.0'><content>"
//            "<section level='1'>Hello world!</section></content></lenmusdoc>");
//        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
//        CHECK( pH != NULL );
//        CHECK( pH->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
//        CHECK( pItem->get_text() == "Hello world!" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Section_Style)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>Title</name><color>#00fe0f7f</color></defineStyle>"
//                "</styles>"
//                "<content><section level='1' style='Heading'>Hello world!</section></content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
//        CHECK( pH != NULL );
//        ImoStyle* pStyle = pH->get_style();
//        CHECK( pStyle != NULL );
//        CHECK( pH->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
//        CHECK( pItem->get_text() == "Hello world!" );
//
//        delete pIModel;
//    }
//
////    TEST_FIXTURE(MxlAnalyserTestFixture, Section_DefaultStyle)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("<lenmusdoc vers='0.0'> <content>"
////            "<section level='1'>Hello world!</section>");
////        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
////        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
////        CHECK( pH != NULL );
////        ImoStyle* pStyle = pH->get_style();
////        CHECK( pStyle->get_name() == "Default style" );
////        CHECK( pH->get_num_items() == 1 );
////        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
////        CHECK( pItem->get_text() == "Hello world!" );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
//
//    // styles ---------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Styles)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<styles>"
//                "<defineStyle>"
//                    "<name>Header1</name>"
//                    "<font-name>Times New Roman</font-name>"
//                    "<font-size>14pt</font-size>"
//                    "<font-style>italic</font-style>"
//                    "<font-weight>bold</font-weight>"
//                    "<color>#00fe0f7f</color>"
//                "</defineStyle>"
//            "</styles>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pIModel->get_root() );
//        CHECK( pStyles != NULL );
//
//        ImoStyle* pStyle = pStyles->find_style("Header1");
//        CHECK( pStyle->get_name() == "Header1" );
//        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
//        CHECK( pStyle->font_name() == "Times New Roman" );
//        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
//        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
//        CHECK( pStyle->font_size() == 14 );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Styles_all_simple_properties)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<styles>"
//                "<defineStyle>"
//                    "<name>Header1</name>"
//                        // color and background
//                    "<color>#00fe0f7f</color>"
//                    "<background-color>#00fc0c7c</background-color>"
//                        // font
//                    "<font-name>Times New Roman</font-name>"
//                    "<font-size>14pt</font-size>"
//                    "<font-style>italic</font-style>"
//                    "<font-weight>bold</font-weight>"
//                        // border width
//                    "<border-width-top>20</border-width-top>"
//                    "<border-width-right>21</border-width-right>"
//                    "<border-width-bottom>22</border-width-bottom>"
//                    "<border-width-left>23</border-width-left>"
//                        // margin
//                    "<margin-top>24</margin-top>"
//                    "<margin-right>25</margin-right>"
//                    "<margin-bottom>26</margin-bottom>"
//                    "<margin-left>27</margin-left>"
//                        // padding
//                    "<padding-top>28</padding-top>"
//                    "<padding-right>29</padding-right>"
//                    "<padding-bottom>30</padding-bottom>"
//                    "<padding-left>31</padding-left>"
//                        //text
//                    "<text-decoration>overline</text-decoration>"
//                    "<vertical-align>text-top</vertical-align>"
//                    "<text-align>center</text-align>"
//                    "<line-height>77</line-height>"
//                        //size
//                    "<min-height>32</min-height>"
//                    "<min-width>33</min-width>"
//                    "<max-height>34</max-height>"
//                    "<max-width>35</max-width>"
//                    "<height>36</height>"
//                    "<width>37</width>"
//                        //table
//                    "<table-col-width>38</table-col-width>"
//                "</defineStyle>"
//            "</styles>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pIModel->get_root() );
//        CHECK( pStyles != NULL );
//
//        ImoStyle* pStyle = pStyles->find_style("Header1");
//        CHECK( pStyle->get_name() == "Header1" );
//            // color and background
//        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
//        CHECK( is_equal(pStyle->background_color(), Color(0, 252 ,12, 124)) );
//            // font
//        CHECK( pStyle->font_name() == "Times New Roman" );
//        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
//        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
//        CHECK( pStyle->font_size() == 14 );
//            // border width
//        CHECK( pStyle->border_width_top() == 20.0f );
//        CHECK( pStyle->border_width_right() == 21.0f );
//        CHECK( pStyle->border_width_bottom() == 22.0f );
//        CHECK( pStyle->border_width_left() == 23.0f );
//            // margin
//        CHECK( pStyle->margin_top() == 24.0f );
//        CHECK( pStyle->margin_right() == 25.0f );
//        CHECK( pStyle->margin_bottom() == 26.0f );
//        CHECK( pStyle->margin_left() == 27.0f );
//            // padding
//        CHECK( pStyle->padding_top() == 28.0f );
//        CHECK( pStyle->padding_right() == 29.0f );
//        CHECK( pStyle->padding_bottom() == 30.0f );
//        CHECK( pStyle->padding_left() == 31.0f );
//            //text
//        CHECK( pStyle->text_decoration() == ImoStyle::k_decoration_overline );
//        CHECK( pStyle->vertical_align() == ImoStyle::k_valign_text_top );
//        CHECK( pStyle->text_align() == ImoStyle::k_align_center );
//        CHECK( pStyle->line_height() == 77.0f );
//            //size
//        CHECK( pStyle->min_height() == k_duration_eighth );
//        CHECK( pStyle->min_width() == 33.0f );
//        CHECK( pStyle->max_height() == 34.0f );
//        CHECK( pStyle->max_width() == 35.0f );
//        CHECK( pStyle->height() == 36.0f );
//        CHECK( pStyle->width() == 37.0f );
//            //table
//        CHECK( pStyle->table_col_width() == 38.0f );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Styles_all_compound_properties)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<styles>"
//                "<defineStyle>"
//                    "<name>Header1</name>"
//                    "<border-width>22.33</border-width>"
//                    "<margin>15.5</margin>"
//                    "<padding>12</padding>"
//                "</defineStyle>"
//            "</styles>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pIModel->get_root() );
//        CHECK( pStyles != NULL );
//
//        ImoStyle* pStyle = pStyles->find_style("Header1");
//        CHECK( pStyle->get_name() == "Header1" );
//            // border width
//        CHECK( pStyle->border_width_top() == 22.33f );
//        CHECK( pStyle->border_width_right() == 22.33f );
//        CHECK( pStyle->border_width_bottom() == 22.33f );
//        CHECK( pStyle->border_width_left() == 22.33f );
//            // margin
//        CHECK( pStyle->margin_top() == 15.5f );
//        CHECK( pStyle->margin_right() == 15.5f );
//        CHECK( pStyle->margin_bottom() == 15.5f );
//        CHECK( pStyle->margin_left() == 15.5f );
//            // padding
//        CHECK( pStyle->padding_top() == 12.0f );
//        CHECK( pStyle->padding_right() == 12.0f );
//        CHECK( pStyle->padding_bottom() == 12.0f );
//        CHECK( pStyle->padding_left() == 12.0f );
//
//        delete pIModel;
//    }
//
////    TEST_FIXTURE(MxlAnalyserTestFixture, Styles_Default)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("<lenmusdoc vers='0.0'> "
////            "<content>(text \"hello world\")) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
////        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(0) );
////        CHECK( pText != NULL );
////        CHECK( pText->get_text() == "hello world" );
////
////        ImoStyle* pStyle = pDoc->get_style_or_default("text");
////        CHECK( pStyle != NULL );
////        CHECK( pStyle->get_name() == "Default style" );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
//
//    // param ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, ParamInfo_Ok)
//    {
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        parser.parse_text("<param name='green'>this is green</param>");
//        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        CHECK( pIModel->get_root()->is_param_info() == true );
//        ImoParamInfo* pParam = dynamic_cast<ImoParamInfo*>( pIModel->get_root() );
//        CHECK( pParam != NULL );
//        CHECK( pParam->get_name() == "green" );
//        CHECK( pParam->get_value() == "this is green" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, ParamInfo_MissingName)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        expected << "Line 0. Missing name for element 'param'. Element ignored." << endl;
//        parser.parse_text("<param>this is green</param>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root() == NULL );
//
//        delete pIModel;
//    }
//
//
//    // dynamic --------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Dynamic_Ok)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<dynamic classid='test' />");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root()->is_dynamic() == true );
//        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pIModel->get_root() );
//        CHECK( pDyn != NULL );
//        CHECK( pDyn->get_classid() == "test" );
//        CHECK( pDyn->is_visible() );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Dynamic_AddedToContent)
//    {
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        parser.parse_text("<lenmusdoc vers='0.0'><content>"
//            "<dynamic classid='test' /></content></lenmusdoc>");
//        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pDoc->get_content_item(0) );
//        CHECK( pDyn != NULL );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_dynamic)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<dynamic id='10' classid='test'>"
//                "<param name='play'>all notes</param>"
//            "</dynamic>"
//        );
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->is_dynamic() == true );
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Dynamic_GeneratesRequest)
//    {
//        LomseDoorway* pDoorway = m_libraryScope.platform_interface();
//        pDoorway->set_request_callback(this, wrapper_lomse_request);
//
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        parser.parse_text("<lenmusdoc vers='0.0'><content>"
//            "<dynamic classid='test' /></content></lenmusdoc>");
//        MxlAnalyser a(cout, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//
//        CHECK( m_fRequestReceived == true );
//        CHECK( m_requestType == k_dynamic_content_request );
//        CHECK( m_pDoc == pDoc );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Dynamic_WithParams)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<dynamic classid='test'>"
//                "<param name='play'>all notes</param>"
//            "</dynamic>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pIModel->get_root() );
//        CHECK( pDyn->get_classid() == "test" );
//        std::list<ImoParamInfo*>& params = pDyn->get_params();
//        CHECK( params.size() == 1 );
//        ImoParamInfo* pParm = params.front();
//        CHECK( pParm->get_name() == "play" );
//        CHECK( pParm->get_value() == "all notes" );
//
//        delete pIModel;
//    }
//
//    // link -----------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Link_Ok)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text(
//            "<link url='#TheoryHarmony_ch3.lms'>Harmony exercise</link>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root()->is_link() == true );
//        ImoLink* pLink = dynamic_cast<ImoLink*>( pIModel->get_root() );
//        CHECK( pLink != NULL );
//        CHECK( pLink->get_url() == "#TheoryHarmony_ch3.lms" );
//        CHECK( pLink->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
//        CHECK( pItem->get_text() == "Harmony exercise" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Link_MissingUrl)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        expected << "Line 0. link: missing mandatory attribute 'url'." << endl;
//        parser.parse_text(
//            "<link>Harmony exercise</link>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root()->is_link() == true );
//        ImoLink* pLink = dynamic_cast<ImoLink*>( pIModel->get_root() );
//        CHECK( pLink != NULL );
//        CHECK( pLink->get_url() == "" );
//        CHECK( pLink->get_num_items() == 1 );
//        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
//        CHECK( pItem->get_text() == "Harmony exercise" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Link_Style)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>Button</name><color>#00fe0f7f</color></defineStyle>"
//                "</styles>"
//                "<content><para>"
//                    "<link style='Button' url='#TheoryHarmony_ch3.lms'>Harmony exercise</link>"
//                "</para></content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        CHECK( pDoc->get_num_content_items() == 1 );
//        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
//        CHECK( pPara != NULL );
//        CHECK( pPara->get_num_items() == 1 );
//        ImoLink* pLink = dynamic_cast<ImoLink*>( pPara->get_first_item() );
//        CHECK( pLink != NULL );
//        CHECK( pLink->get_url() == "#TheoryHarmony_ch3.lms" );
//        ImoStyle* pStyle = pLink->get_style();
//        CHECK( pStyle != NULL );
//        CHECK( pStyle->get_name() == "Button" );
//
//        delete pIModel;
//    }
//
//////    // image ----------------------------------------------------------------------------
//////
//////    TEST_FIXTURE(MxlAnalyserTestFixture, Image_Ok)
//////    {
//////        stringstream errormsg;
//////        Document doc(m_libraryScope);
//////        XmlParser parser;
//////        stringstream expected;
//////        //expected << "Line 0. " << endl;
//////        parser.parse_text(
//////            "(image (file \"test-image-1.png\"))");
//////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//////
////////        cout << "[" << errormsg.str() << "]" << endl;
////////        cout << "[" << expected.str() << "]" << endl;
//////        CHECK( errormsg.str() == expected.str() );
//////
//////        ImoImage* pImg = dynamic_cast<ImoImage*>( pIModel->get_root() );
//////        CHECK( pImg != NULL );
//////        //CHECK( pImg->get_locator() == "chopin.png" );
//////
//////        delete tree->get_root();
//////        delete pIModel;
//////    }
//
//    // list -----------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, Listitem_created)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. link: missing mandatory element 'url'." << endl;
//        parser.parse_text(
//            "<listitem>This is the first item</listitem>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root()->is_listitem() == true );
//        ImoListItem* pLI = dynamic_cast<ImoListItem*>( pIModel->get_root() );
//        CHECK( pLI->get_num_content_items() == 1 );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
//        CHECK( pText->get_text() == "This is the first item" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_listitem)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<listitem id='10'>This is the first item</listitem>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_listitem() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, List_created)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. link: missing mandatory element 'url'." << endl;
//        parser.parse_text(
//            "<itemizedlist>"
//                "<listitem>This is the first item</listitem>"
//                "<listitem>This is the second item</listitem>"
//            "</itemizedlist>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root()->is_list() == true );
//        ImoList* pList = dynamic_cast<ImoList*>( pIModel->get_root() );
//        CHECK( pList != NULL );
//        CHECK( pList->get_list_type() == ImoList::k_itemized );
//        CHECK( pList->get_num_content_items() == 2 );
//        ImoListItem* pLI = pList->get_list_item(0);
//        CHECK( pLI->get_num_content_items() == 1 );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
//        CHECK( pText->get_text() == "This is the first item" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_list)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<itemizedlist id='10'>"
//                "<listitem>This is the first item</listitem>"
//                "<listitem>This is the second item</listitem>"
//            "</itemizedlist>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_list() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
////    // graphic line  --------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, graphic_type_error)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        expected << "Line 0. Unknown type 'circle'. Element 'graphic' ignored." << endl;
////        parser.parse_text("(graphic circle 0.0 0.67)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        CHECK( pIModel->get_root() == NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, graphic_type_ok)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "Line 0. " << endl;
////        parser.parse_text("(graphic line 0.0 7.0 17.0 3.5)");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        CHECK( pIModel->get_root()->is_score_line() == true );
////        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pIModel->get_root() );
////        CHECK( pLine != NULL );
////        CHECK( pLine->get_x_start() == 0.0f );
////        CHECK( pLine->get_y_start() == 7.0f );
////        CHECK( pLine->get_x_end() == 17.0f );
////        CHECK( pLine->get_y_end() == 3.5f );
////        CHECK( pLine->get_line_width() == 1.0f );
////        CHECK( pLine->get_start_edge() == k_edge_normal );
////        CHECK( pLine->get_line_style() == k_line_solid );
////        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
////        CHECK( pLine->get_end_cap() == k_cap_none );
////        Color color = pLine->get_color();
////        CHECK( color.r == 0 );
////        CHECK( color.g == 0 );
////        CHECK( color.b == 0 );
////        CHECK( color.a == 255 );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
////
////    TEST_FIXTURE(MxlAnalyserTestFixture, graphic_is_anchored)
////    {
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        //expected << "" << endl;
////        parser.parse_text("(musicData "
////            "(n c4 q)(graphic line 0.0 7.0 17.0 3.5) )");
////        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        //cout << "[" << errormsg.str() << "]" << endl;
////        //cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////
////        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
////        CHECK( pMusic != NULL );
////        ImoObj::children_iterator it = pMusic->begin();
////
////        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != NULL );
////
////        ++it;
////        ImoSpacer* pSpacer = dynamic_cast<ImoSpacer*>( *it );
////        CHECK( pSpacer != NULL );
////        ImoAttachments* pAuxObjs = pSpacer->get_attachments();
////        CHECK( pAuxObjs != NULL );
////        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pAuxObjs->get_item(0) );
////        CHECK( pLine != NULL );
////
////        delete tree->get_root();
////        delete pIModel;
////    }
//
//
//    // scorePlayer ----------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, scorePlayer_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("<lenmusdoc vers='0.0'> <content>"
//            "<scorePlayer/></content></lenmusdoc>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
//        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
//        CHECK( pSP->is_score_player() == true );
//        CHECK( pSP->get_metronome_mm() == 60 );
//        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_score_player)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("<lenmusdoc vers='0.0'> <content>"
//            "<scorePlayer id='10'/></content></lenmusdoc>");
//        //parser.parse_text("<scorePlayer id='10' />");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
//        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
//        CHECK( pSP->is_score_player() == true );
//        CHECK( pSP->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, scorePlayer_metronome)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text(
//            "<lenmusdoc vers='0.0'> <content>"
//                "<scorePlayer><mm>65</mm></scorePlayer>"
//            "</content></lenmusdoc>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
//        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
//        CHECK( pSP->is_score_player() == true );
//        CHECK( pSP->get_metronome_mm() == 65 );
//        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, scorePlayer_label_play)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text(
//            "<lenmusdoc vers='0.0'> <content>"
//                "<scorePlayer>"
//                    "<playLabel>Tocar</playLabel>"
//                    "<stopLabel>Parar</stopLabel>"
//                    "<mm>65</mm>"
//                "</scorePlayer>"
//            "</content></lenmusdoc>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
//        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
//        CHECK( pSP->is_score_player() == true );
//        CHECK( pSP->get_metronome_mm() == 65 );
//        CHECK( pSP->get_play_label() == "Tocar" );
//        CHECK( pSP->get_stop_label() == "Parar" );
//        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;
//
//        delete pIModel;
//    }
//
//    // tableCell ------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, tableCell_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text(
//            "<tableCell>This is a cell</tableCell>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
//        CHECK( pCell->is_table_cell() == true );
//        CHECK( pCell->get_num_content_items() == 1 );
//        CHECK( pCell->get_rowspan() == 1 );
//        CHECK( pCell->get_colspan() == 1 );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
//        CHECK( pAB->get_num_items() == 1 );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
//        CHECK( pText->get_text() == "This is a cell" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_table_cell)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<tableCell id='10'>This is a cell</tableCell>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_table_cell() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, tableCell_rowspan)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text(
//            "<tableCell><rowspan>2</rowspan>This is a cell</tableCell>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
//        CHECK( pCell->is_table_cell() == true );
//        CHECK( pCell->get_num_content_items() == 1 );
//        CHECK( pCell->get_rowspan() == 2 );
//        CHECK( pCell->get_colspan() == 1 );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
//        CHECK( pAB->get_num_items() == 1 );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
//        CHECK( pText->get_text() == "This is a cell" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, tableCell_colspan)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text(
//            "<tableCell><colspan>2</colspan>This is a cell</tableCell>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
//        CHECK( pCell->is_table_cell() == true );
//        CHECK( pCell->get_num_content_items() == 1 );
//        CHECK( pCell->get_rowspan() == 1 );
//        CHECK( pCell->get_colspan() == 2 );
//        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
//        CHECK( pAB->get_num_items() == 1 );
//        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
//        CHECK( pText->get_text() == "This is a cell" );
//
//        delete pIModel;
//    }
//
//    // tableRow -------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, tableRow_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<tableRow>"
//                "<tableCell>This is cell 1</tableCell>"
//                "<tableCell>This is cell 2</tableCell>"
//            "</tableRow>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pIModel->get_root() );
//        CHECK( pRow->is_table_row() == true );
//        CHECK( pRow->get_num_cells() == 2 );
//        ImoTableCell* pImo = dynamic_cast<ImoTableCell*>( pRow->get_cell(0) );
//        CHECK( pImo->is_table_cell() == true );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_table_row)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<tableRow id='10'>"
//                "<tableCell>This is cell 1</tableCell>"
//                "<tableCell>This is cell 2</tableCell>"
//            "</tableRow>");
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_table_row() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    // tableHead ------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, tableHead_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<tableHead>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//            "</tableHead>";;
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTableHead* pHead = dynamic_cast<ImoTableHead*>( pIModel->get_root() );
//        CHECK( pHead->is_table_head() == true );
//        CHECK( pHead->get_num_items() == 2 );
//        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
//        CHECK( pRow->is_table_row() == true );
//        CHECK( pRow->get_num_cells() == 1 );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_table_head)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<tableHead id='10'>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//            "</tableHead>" );
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_table_head() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    // tableBody ------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, tableBody_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<tableBody>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//            "</tableBody>";;
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTableBody* pBody = dynamic_cast<ImoTableBody*>( pIModel->get_root() );
//        CHECK( pBody->is_table_body() == true );
//        CHECK( pBody->get_num_items() == 2 );
//        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
//        CHECK( pRow->is_table_row() == true );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_table_body)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<tableBody id='10'>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//            "</tableBody>" );
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_table_body() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    // table ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, table_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<table><tableBody>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//            "</tableBody></table>";;
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoTable* pTable = dynamic_cast<ImoTable*>( pIModel->get_root() );
//        CHECK( pTable != NULL );
//
//        CHECK( pTable->get_head() == NULL );
//
//        ImoTableBody* pBody = pTable->get_body();
//        CHECK( pBody != NULL );
//        CHECK( pBody->get_num_items() == 1 );
//        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
//        CHECK( pRow->is_table_row() == true );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, id_in_table)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text(
//            "<table id='10'><tableBody>"
//                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//            "</tableBody></table>" );
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root()->is_table() == true );
//        ImoObj* pImo = pIModel->get_root();
//        CHECK( pImo->get_id() == 10L );
//
//        delete pIModel;
//    }
//
//    // tableColumn ----------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, tableColumn_Creation)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
//                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
//                "</styles>"
//                "<content>"
//                    "<table>"
//                        "<tableColumn style='table1-col1' />"
//                        "<tableColumn style='table1-col2' />"
//                        "<tableBody>"
//                            "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//                        "</tableBody>"
//                    "</table>"
//                "</content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
//        CHECK( pTable != NULL );
//
//        std::list<ImoStyle*>& cols = pTable->get_column_styles();
//        CHECK( cols.size() == 2 );
//        std::list<ImoStyle*>::iterator it = cols.begin();
//        CHECK( (*it)->get_name() == "table1-col1" );
//        ++it;
//        CHECK( (*it)->get_name() == "table1-col2" );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, table_full_table)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
//                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
//                "</styles>"
//                "<content>"
//                    "<table>"
//                        "<tableColumn style='table1-col1' />"
//                        "<tableColumn style='table1-col2' />"
//                        "<tableHead>"
//                            "<tableRow>"
//                                "<tableCell>This is head cell 1</tableCell>"
//                                "<tableCell>This is head cell 2</tableCell>"
//                            "</tableRow>"
//                        "</tableHead>"
//                        "<tableBody>"
//                            "<tableRow>"
//                                "<tableCell>This is body cell 1</tableCell>"
//                                "<tableCell>This is body cell 2</tableCell>"
//                            "</tableRow>"
//                        "</tableBody>"
//                    "</table>"
//                "</content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
//        CHECK( pTable != NULL );
//
//        std::list<ImoStyle*>& cols = pTable->get_column_styles();
//        CHECK( cols.size() == 2 );
//
//        ImoTableHead* pHead = pTable->get_head();
//        CHECK( pHead != NULL );
//        CHECK( pHead->get_num_items() == 1 );
//        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
//        CHECK( pRow->is_table_row() == true );
//
//        ImoTableBody* pBody = pTable->get_body();
//        CHECK( pBody != NULL );
//        CHECK( pBody->get_num_items() == 1 );
//        pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
//        CHECK( pRow->is_table_row() == true );
//
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, table_added_to_content)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
//                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
//                "</styles>"
//                "<content>"
//                    "<table>"
//                        "<tableColumn style='table1-col1' />"
//                        "<tableColumn style='table1-col2' />"
//                        "<tableBody>"
//                            "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
//                        "</tableBody>"
//                    "</table>"
//                "</content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
//        CHECK( pTable != NULL );
//
//        delete pIModel;
//    }
//
//    // ldpmusic -------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, ldpmusic_ok)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        string src =
//            "<lenmusdoc vers='0.0'>"
//                "<styles>"
//                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
//                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
//                "</styles>"
//                "<content>"
//                    "<ldpmusic>"
//                        "(score (vers 1.6)(instrument (musicData (clef G)(n c4 q.))))"
//                    "</ldpmusic>"
//                "</content>"
//            "</lenmusdoc>";
//        parser.parse_text(src);
//        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
//        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//        CHECK( pScore != NULL );
//
//        delete pIModel;
//    }
}

