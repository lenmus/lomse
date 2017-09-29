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
#include "lomse_import_options.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// MusicXmlOptions tests
//=======================================================================================

class MusicXmlOptionsTestFixture
{
public:
    LibraryScope m_libraryScope;

    MusicXmlOptionsTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
    }

    ~MusicXmlOptionsTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }
};


SUITE(MusicXmlOptionsTest)
{

    // import options builder -----------------------------------------------------------

    TEST_FIXTURE(MusicXmlOptionsTestFixture, MusicXmlOptions_1)
    {
        //@01. default values are correct
        MusicXmlOptions* opt = m_libraryScope.get_musicxml_options();

        CHECK( opt->fix_beams() == true );
        CHECK( opt->use_default_clefs() == true );
    }

    TEST_FIXTURE(MusicXmlOptionsTestFixture, MusicXmlOptions_2)
    {
        //@02. fix beams
        MusicXmlOptions* opt = m_libraryScope.get_musicxml_options();
        opt->fix_beams(false);

        MusicXmlOptions* newopt = m_libraryScope.get_musicxml_options();
        CHECK( newopt->fix_beams() == false );
        CHECK( newopt->use_default_clefs() == true );
    }

    TEST_FIXTURE(MusicXmlOptionsTestFixture, MusicXmlOptions_3)
    {
        //@03. use default clefs
        MusicXmlOptions* opt = m_libraryScope.get_musicxml_options();
        opt->use_default_clefs(false);

        MusicXmlOptions* newopt = m_libraryScope.get_musicxml_options();
        CHECK( newopt->fix_beams() == true );
        CHECK( newopt->use_default_clefs() == false );
    }

    TEST_FIXTURE(MusicXmlOptionsTestFixture, MusicXmlOptions_4)
    {
        //@04. two settings
        MusicXmlOptions* opt = m_libraryScope.get_musicxml_options();
        opt->use_default_clefs(false);
        opt->fix_beams(false);

        MusicXmlOptions* newopt = m_libraryScope.get_musicxml_options();
        CHECK( newopt->fix_beams() == false );
        CHECK( newopt->use_default_clefs() == false );
    }

};



//=======================================================================================
// MxlAnalyser tests
//=======================================================================================

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

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    list<ImoTuplet*> get_tuplets(ImoNoteRest* pNR)
    {
        list<ImoTuplet*> tuplets;
        if (pNR->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = pNR->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                if (pRO->is_tuplet())
                    tuplets.push_back(static_cast<ImoTuplet*>(pRO));
            }
        }
        return tuplets;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_mensurstrich );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_nothing );

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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature->get_bottom_number() == 8 );
//        cout << test_name()
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( is_equal(pNote->get_actual_accidentals(), -1.0f) );
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( is_equal(pNote->get_actual_accidentals(), -1.0f) );
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

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_310)
    {
        //@00310 unpitched note

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><unpitched/>"
            "<duration>1</duration><type>half</type>"
            "</note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_actual_accidentals() == k_acc_not_computed );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_half );
        CHECK( pNote->is_pitch_defined() == false );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_no_pitch );
        CHECK( pNote->get_duration() == k_duration_quarter );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

//    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_311)
//    {
//        //@00311 timepos is computed right
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
////        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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

//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
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


    //@ midi-instrument -----------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, midi_instrument_01)
    {
        //@01. midi_instrument

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <score-partwise>: missing mandatory element <part>." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<midi-instrument id='P1'>"
                    "<midi-channel>1</midi-channel>"
                    "<midi-program>56</midi-program>"
                "</midi-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
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
        CHECK( pInstr->get_midi_channel() == 0 );
        CHECK( pInstr->get_midi_program() == 56 );
        cout << test_name() << endl;
        cout << "midi: channel= " << pInstr->get_midi_channel()
             << ", program= " << pInstr->get_midi_program() << endl;

        delete pIModel;
    }

//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrErrorValue)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Missing or invalid MIDI instrument (0..255). MIDI info ignored." << endl;
//        parser.parse_text("(infoMIDI piano 1)");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
//        CHECK( pInfo == NULL );
//
//        delete tree->get_root();
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrErrorRange)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Missing or invalid MIDI instrument (0..255). MIDI info ignored." << endl;
//        parser.parse_text("(infoMIDI 315 1)");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
//        CHECK( pInfo == NULL );
//
//        delete tree->get_root();
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrumentOk)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("(infoMIDI 56)");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root()->is_midi_info() == true );
//        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
//        CHECK( pInfo != NULL );
//        CHECK( pInfo->get_midi_channel() == 0 );
//        CHECK( pInfo->get_midi_program() == 56 );
//
//        delete tree->get_root();
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_ChannelErrorValue)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Invalid MIDI channel (0..15). Channel info ignored." << endl;
//        parser.parse_text("(infoMIDI 56 25)");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
//        CHECK( pInfo != NULL );
//        CHECK( pInfo->get_midi_channel() == 0 );
//        CHECK( pInfo->get_midi_program() == 56 );
//
//        delete tree->get_root();
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrumentChannelOk)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("(infoMIDI 56 10)");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
//        CHECK( pInfo != NULL );
//        CHECK( pInfo->get_midi_channel() == 10 );
//        CHECK( pInfo->get_midi_program() == 56 );
//
//        delete tree->get_root();
//        delete pIModel;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_MidiInfo)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(instrument (infoMIDI 56 12)(musicData))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
//        CHECK( pInstr != NULL );
//        CHECK( pInstr->get_num_staves() == 1 );
//        CHECK( pInstr->get_name().get_text() == "" );
//        CHECK( pInstr->get_abbrev().get_text() == "" );
//        CHECK( pInstr->get_midi_channel() == 12 );
//        CHECK( pInstr->get_midi_program() == 56 );
//
//        delete tree->get_root();
//        delete pIModel;
//    }


    //@ score-instrument ----------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, score_instrument_01)
    {
        //@01. score_instrument

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <score-partwise>: missing mandatory element <part>." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument id='P1-I1'>"
                    "<instrument-name>Marimba</instrument-name>"
                "</score-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
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
        CHECK( pInstr->get_midi_channel() == 0 );
        CHECK( pInstr->get_midi_program() == 56 );
        cout << test_name() << endl;
        cout << "midi: channel= " << pInstr->get_midi_channel()
             << ", program= " << pInstr->get_midi_program() << endl;

        delete pIModel;
    }


    //@ time-modification ---------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, time_modification_0)
    {
        //@0. time-modification

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>C</step><octave>5</octave></pitch>"
            "<duration>136</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>3</actual-notes>"
            "<normal-notes>2</normal-notes>"
            "</time-modification>"
            "</note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 5 );
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->get_time_modifier_top() == 2 );
        CHECK( pNote->get_time_modifier_bottom() == 3 );
//        cout << "time_modifier_top= " << pNote->get_time_modifier_top()
//             << ", time_modifier_bottom= " << pNote->get_time_modifier_bottom() << endl;

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    //@ tuplet --------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, tuplet_01)
    {
        //@01. tuplet

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>G</step><alter>-1</alter>"
                "<octave>5</octave></pitch><duration>4</duration><type>16th</type>"
                "<notations><tuplet type='start' number='137' /></notations>"
            "</note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
                "<notations><tuplet type='stop' number='137' /></notations>"
            "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
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
        ImoTuplet* pTuplet = pNote->get_first_tuplet();
        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 1 );
        CHECK( pTuplet->get_normal_number() == 1 );
        CHECK( pTuplet->get_show_bracket() == k_yesno_default );
        CHECK( pTuplet->get_num_objects() == 3 );
        CHECK( pTuplet->get_id() == 137L );

        ++it;
        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        ImoTuplet* pTuplet2 = pNote->get_first_tuplet();
        CHECK( pTuplet2 == pTuplet );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, tuplet_02)
    {
        //@02. tuplet. bracket

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>G</step><alter>-1</alter>"
                "<octave>5</octave></pitch><duration>4</duration><type>16th</type>"
                "<notations><tuplet type='start' number='141' bracket='yes'/></notations>"
            "</note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
                "<notations><tuplet type='stop' number='141' /></notations>"
            "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
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
        ImoTuplet* pTuplet = pNote->get_first_tuplet();
        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 1 );
        CHECK( pTuplet->get_normal_number() == 1 );
        CHECK( pTuplet->get_show_bracket() == k_yesno_yes );
        CHECK( pTuplet->get_num_objects() == 3 );
        CHECK( pTuplet->get_id() == 141L );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, tuplet_03)
    {
        //@03. tuplet. data from time modification

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>G</step><alter>-1</alter>"
                "<octave>5</octave></pitch><duration>4</duration><type>16th</type>"
                "<time-modification><actual-notes>3</actual-notes>"
                "<normal-notes>2</normal-notes>"
                "</time-modification>"
                "<notations><tuplet type='start' number='141' bracket='yes'/></notations>"
            "</note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
                "<notations><tuplet type='stop' number='141' /></notations>"
            "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
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
        ImoTuplet* pTuplet = pNote->get_first_tuplet();
        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 3 );
        CHECK( pTuplet->get_normal_number() == 2 );
        CHECK( pTuplet->get_show_bracket() == k_yesno_yes );
        CHECK( pTuplet->get_num_objects() == 3 );
        CHECK( pTuplet->get_id() == 141L );

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, tuplet_04)
    {
        //@04. tuplet. data from time modification when nested tuplets

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(      //23d-Tuplets-Nested.xml (simplified)
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>3</actual-notes>"
            "    <normal-notes>2</normal-notes></time-modification>"
            "    <notations><tuplet bracket='yes' number='1' type='start'/>"
            "    </notations></note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>3</actual-notes>"
            "    <normal-notes>2</normal-notes></time-modification>"
            "    </note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>15</actual-notes>"
            "    <normal-notes>4</normal-notes></time-modification>"
            "    <notations><tuplet bracket='yes' number='2' type='start'/>"
            "    </notations></note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>15</actual-notes>"
            "    <normal-notes>4</normal-notes></time-modification>"
            "    </note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>15</actual-notes>"
            "    <normal-notes>4</normal-notes></time-modification>"
            "    </note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>15</actual-notes>"
            "    <normal-notes>4</normal-notes></time-modification>"
            "    </note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>15</actual-notes>"
            "    <normal-notes>4</normal-notes></time-modification>"
            "    <notations><tuplet number='2' type='stop'/>"
            "    </notations></note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>3</actual-notes>"
            "    <normal-notes>2</normal-notes></time-modification>"
            "    </note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "    <duration>10</duration><type>eighth</type>"
            "    <time-modification><actual-notes>3</actual-notes>"
            "    <normal-notes>2</normal-notes></time-modification>"
            "    <notations><tuplet number='1' type='stop'/>"
            "    </notations></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
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
        CHECK( pMD->get_num_children() == 10 );
//        cout << test_name() << endl;
        //cout << "num.children= " << pMD->get_num_children() << endl;

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        ImoTuplet* pTuplet = pNote->get_first_tuplet();
        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 3 );
        CHECK( pTuplet->get_normal_number() == 2 );

        ++it;
        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        list<ImoTuplet*> tuplets = get_tuplets(pNote);
        CHECK( tuplets.size() == 2 );
        pTuplet = tuplets.front();
        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 5 );
        CHECK( pTuplet->get_normal_number() == 2 );
//        cout << test_name() << endl;
//        cout << "Tuplet. actual=" << pTuplet->get_actual_number()
//             << ", normal=" << pTuplet->get_normal_number() << endl;
        pTuplet = tuplets.back();
        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 3 );
        CHECK( pTuplet->get_normal_number() == 2 );
//        cout << test_name() << endl;
//        cout << "Tuplet. actual=" << pTuplet->get_actual_number()
//             << ", normal=" << pTuplet->get_normal_number() << endl;

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    TEST_FIXTURE(MxlAnalyserTestFixture, tuplet_05)
    {
        //@05. tuplet. actual and normal numbers

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>G</step><alter>-1</alter>"
                "<octave>5</octave></pitch><duration>4</duration><type>16th</type>"
                "<time-modification><actual-notes>3</actual-notes>"
                "<normal-notes>2</normal-notes>"
                "</time-modification>"
                "<notations><tuplet type='start' number='141' bracket='yes'>"
                    "<tuplet-actual>"
                    "  <tuplet-number>5</tuplet-number>"
                    "  <tuplet-type>eighth</tuplet-type>"
                    "</tuplet-actual>"
                    "<tuplet-normal>"
                    "  <tuplet-number>3</tuplet-number>"
                    "  <tuplet-type>eighth</tuplet-type>"
                    "</tuplet-normal>"
                "</tuplet></notations>"
            "</note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
                "<notations><tuplet type='stop' number='141' /></notations>"
            "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

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
        ImoTuplet* pTuplet = pNote->get_first_tuplet();
        CHECK( pTuplet != NULL );
        CHECK( pTuplet->get_actual_number() == 5 );
        CHECK( pTuplet->get_normal_number() == 3 );
        CHECK( pTuplet->get_show_bracket() == k_yesno_yes );
        CHECK( pTuplet->get_num_objects() == 3 );

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        cout << "Tuplet. actual=" << pTuplet->get_actual_number()
//             << ", normal=" << pTuplet->get_normal_number() << endl;

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


    //@ Options for dealing with malformed MusicXML files -------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_500)
    {
        //@500 fix wrong beam

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
                "<note><pitch><step>E</step><octave>5</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>16th</type>"
                    "<beam number='1'>begin</beam>"
                "</note>"
                "<note><pitch><step>D</step><octave>5</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>16th</type>"
                    "<beam number='1'>end</beam>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );

        ImoObj::children_iterator it = pMD->begin();
        CHECK( pMD->get_num_children() == 3 );          //#3 is barline

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );

        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );

        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );

//        cout << test_name() << endl;
//        cout << "errormsg: " << errormsg.str() << endl;
//        cout << "note 1 beams: " << pNote1->get_beam_type(0)
//             << ", " << pNote1->get_beam_type(1)
//             << ", " << pNote1->get_beam_type(2)
//             << ", " << pNote1->get_beam_type(3)
//             << ", " << pNote1->get_beam_type(4)
//             << ", " << pNote1->get_beam_type(5) << endl;
//        cout << "note 2 beams: " << pNote2->get_beam_type(0)
//             << ", " << pNote2->get_beam_type(1)
//             << ", " << pNote2->get_beam_type(2)
//             << ", " << pNote2->get_beam_type(3)
//             << ", " << pNote2->get_beam_type(4)
//             << ", " << pNote2->get_beam_type(5) << endl;

        a.do_not_delete_instruments_in_destructor();
        delete pIModel;
    }


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

//        cout << test_name() << endl;
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

}

