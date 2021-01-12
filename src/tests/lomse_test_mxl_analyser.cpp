//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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
#include "private/lomse_document_p.h"
#include "lomse_xml_parser.h"
#include "lomse_mxl_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_events.h"
#include "lomse_doorway.h"
#include "lomse_im_factory.h"
#include "lomse_time.h"
#include "lomse_import_options.h"
#include "lomse_im_attributes.h"
#include "lomse_staffobjs_table.h"

#include <regex>

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
        , m_pDoc(nullptr)
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
            AObject dyn = pRq->get_object();
            m_pDoc = dyn.owner_document().internal_object()->get_im_root();
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

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_)
    {
        //@01 missing mandatory <part-list>. Returns empty document
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <score-partwise>: missing mandatory element <part-list>." << endl;
        parser.parse_text("<score-partwise version='3.0'></score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( a.get_musicxml_version() == 300 );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 0 );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_02)
    {
        //@02 invalid score version, 1.0 assumed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <score-partwise>: missing mandatory element <part-list>." << endl;
        parser.parse_text("<score-partwise version='3.a'></score-partwise>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( a.get_musicxml_version() == 100 );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        CHECK( errormsg.str() == expected.str() );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_03)
    {
        //@03 missing <score-part>. Returns empty document
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <part-list>: missing mandatory element <score-part>.\n"
                 << "Line 0. errors in <part-list>. Analysis stopped." << endl;
        parser.parse_text("<score-partwise version='3.0'><part-list/></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 0 );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_04)
    {
        //@04 missing <part>. Returns empty score v1.6
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_version_number() == 160 );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );
        CHECK( pMD->get_num_items() == 0 );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_05)
    {
        //@05 minimum score ok. Returns empty score with one instrument
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );
        CHECK( pMD->get_num_items() == 0 );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_06)
    {
        //@06 ImoScore created for <score-partwise>
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_07)
    {
        //@07 ImoInstrument created for <part-list>
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_08)
    {
        //@08 ImoMusicData created for <part>
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );
        CHECK( pMD->get_num_items() == 0 );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_partwise_09)
    {
        //@09 <part-list> with several <score-part>. Missing part
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );
                if (pInstr)
                {
                    ImoMusicData* pMD = pInstr->get_musicdata();
                    CHECK( pMD != nullptr );
                    if (pMD)
                    {
                        CHECK( pMD->get_num_items() == 0 );
                        pInstr = pScore->get_instrument(1);
                        CHECK( pInstr != nullptr );
                        pMD = pInstr->get_musicdata();
                        CHECK( pMD != nullptr );
                        CHECK( pMD->get_num_items() == 0 );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ score-part -------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_score_part_01)
    {
        //@01 <part-name> sets instrument name
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<score-part id='P1'><part-name>Guitar</part-name></score-part>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_instrument() == true );
        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pRoot );
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "Guitar" );
//        cout << "Name: '" << pInstr->get_name().get_text()
//             << "', Abbrev: '" << pInstr->get_abbrev().get_text() << "'" << endl;
        CHECK( pInstr->get_abbrev().get_text() == "" );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );
        CHECK( pMD->get_num_items() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ part-group -------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_01)
    {
        //@01 <part-group> type=start matched with type=stop
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = static_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != nullptr );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != nullptr );
        CHECK( pGroup && pGroup->get_instrument(0) != nullptr );
        CHECK( pGroup && pGroup->get_instrument(1) != nullptr );
        CHECK( pGroup && pGroup->get_abbrev_string() == "" );
        CHECK( pGroup && pGroup->get_name_string() == "" );
        CHECK( pGroup && pGroup->get_symbol() == k_group_symbol_none );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_02)
    {
        //@02 <part-group> type=start present but missing type=stop
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups == nullptr );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_03)
    {
        //@03 <part-group> missing number
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );

        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups == nullptr );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_04)
    {
        //@04 <part-group> missing type
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );

                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups == nullptr );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_05)
    {
        //@05 <part-group> missing type start for this number
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );

                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups == nullptr );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_06)
    {
        //@06 <part-group> type is not either 'start' or 'stop'
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );

                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups == nullptr );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_07)
    {
        //@07 <part-group> type=start for number already started and not stopped
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );

                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups != nullptr );
                ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
                CHECK( pGroup != nullptr );
                if (pGroup)
                {
                    CHECK( pGroup->get_instrument(0) != nullptr );
                    CHECK( pGroup->get_instrument(1) != nullptr );
                    CHECK( pGroup->get_abbrev_string() == "" );
                    CHECK( pGroup->get_name_string() == "" );
                    CHECK( pGroup->get_symbol() == k_group_symbol_none );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_08)
    {
        //@08 <part-group> group name and group abbrev ok
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );
                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups != nullptr );
                if (pGroups)
                {
                    ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
                    CHECK( pGroup != nullptr );
                    CHECK( pGroup && pGroup->get_instrument(0) != nullptr );
                    CHECK( pGroup && pGroup->get_instrument(1) != nullptr );
                    CHECK( pGroup && pGroup->get_abbrev_string() == "Grp" );
                    CHECK( pGroup && pGroup->get_name_string() == "Group" );
                    CHECK( pGroup && pGroup->get_symbol() == k_group_symbol_none );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_09)
    {
        //@09 <part-group> group symbol ok
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );

                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups != nullptr );
                ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
                CHECK( pGroup != nullptr );
                CHECK( pGroup && pGroup->get_instrument(0) != nullptr );
                CHECK( pGroup && pGroup->get_instrument(1) != nullptr );
                CHECK( pGroup && pGroup->get_abbrev_string() == "" );
                CHECK( pGroup && pGroup->get_name_string() == "Group" );
                CHECK( pGroup && pGroup->get_symbol() == k_group_symbol_brace );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_10)
    {
        //@10 <part-group> group symbol: invalid value
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );

                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups != nullptr );
                ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
                CHECK( pGroup != nullptr );
                CHECK( pGroup && pGroup->get_instrument(0) != nullptr );
                CHECK( pGroup && pGroup->get_instrument(1) != nullptr );
                CHECK( pGroup && pGroup->get_abbrev_string() == "" );
                CHECK( pGroup && pGroup->get_name_string() == "Group" );
                CHECK( pGroup && pGroup->get_symbol() == k_group_symbol_none );
                CHECK( pGroup && pGroup->join_barlines() == EJoinBarlines::k_joined_barlines );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_11)
    {
        //@11 <part-group> group-barline ok
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                pScore->end_of_changes();
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );

                ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                CHECK( pGroups != nullptr );
                if (pGroups)
                {
                    ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
                    CHECK( pGroup != nullptr );
                    CHECK( pGroup && pGroup->get_instrument(0) != nullptr );
                    CHECK( pGroup && pGroup->get_instrument(1) != nullptr );
                    CHECK( pGroup && pGroup->get_abbrev_string() == "" );
                    CHECK( pGroup && pGroup->get_name_string() == "Group" );
                    CHECK( pGroup && pGroup->get_symbol() == k_group_symbol_bracket );
                    CHECK( pGroup && pGroup->join_barlines() == EJoinBarlines::k_non_joined_barlines );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_part_group_12)
    {
        //@12 <part-group> group-barline mensurstrich ok
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                pScore->end_of_changes();
                CHECK( pScore->get_num_instruments() == 2 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );
                if (pInstr)
                {
                    CHECK( pInstr->get_barline_layout() == ImoInstrument::k_mensurstrich );
                    pInstr = pScore->get_instrument(1);
                    CHECK( pInstr != nullptr );
                    CHECK( pInstr->get_barline_layout() == ImoInstrument::k_nothing );

                    ImoInstrGroups* pGroups = pScore->get_instrument_groups();
                    CHECK( pGroups != nullptr );
                    if (pGroups)
                    {
                        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
                        CHECK( pGroup != nullptr );
                        CHECK( pGroup && pGroup->get_instrument(0) != nullptr );
                        CHECK( pGroup && pGroup->get_instrument(1) != nullptr );
                        CHECK( pGroup && pGroup->get_abbrev_string() == "" );
                        CHECK( pGroup && pGroup->get_name_string() == "Group" );
                        CHECK( pGroup && pGroup->get_symbol() == k_group_symbol_bracket );
                        CHECK( pGroup && pGroup->join_barlines() == EJoinBarlines::k_mensurstrich_barlines );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ attributes -------------------------------------------------------------------


    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_01)
    {
        //@01 attributes are added to MusicData in required order
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 1 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );
                if (pInstr)
                {
                    CHECK( pInstr->get_num_staves() == 1 );
                    ImoMusicData* pMD = pInstr->get_musicdata();
                    CHECK( pMD != nullptr );
                    CHECK( pMD->get_num_items() == 4 );
                    ImoObj::children_iterator it = pMD->begin();
                    CHECK( (*it)->is_clef() == true );
                    ++it;
                    CHECK( (*it)->is_key_signature() == true );
                    ++it;
                    CHECK( (*it)->is_time_signature() == true );
                    ++it;
                    CHECK( (*it)->is_barline() == true );
                }
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_02)
    {
        //@02 divisions
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr);
        CHECK( a.current_divisions() == 7.0f );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_03)
    {
        //@03 if no divisions assume 1
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "divisons:" << a.current_divisions() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( a.current_divisions() == 1.0f );

        delete pRoot;
    }


    //@ barline --------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_barline_01)
    {
        //@01 barline minimal content
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<barline><bar-style>light-light</bar-style></barline>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ backup --------------------------------------------------------------------------
    //@ forward -------------------------------------------------------------------------


//    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_backup_01)
//    {
//        //@01 backup
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        parser.parse_text("<backup><duration>18</duration></backup>");
//        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pRoot == nullptr);
//        //AWARE: initialy <divisions>==1 ==> duration is expressed in quarter notes
//        CHECK( is_equal_time(a.get_current_time(), -18.0f*k_duration_64th) );
//        cout << "420: timepos= " << a.get_current_time() << endl;
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }


    //@ clef -------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_clef_01)
    {
        //@01. minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<clef><sign>G</sign><line>2</line></clef>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_G2 );
        CHECK( pClef && pClef->get_staff() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_clef_02)
    {
        //@02. error in clef sign

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Unknown clef 'H'. Assumed 'G' in line 2." << endl;
        parser.parse_text("<clef><sign>H</sign><line>2</line></clef>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_G2 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_clef_03)
    {
        //@03. staff num parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<clef number='2'><sign>F</sign><line>4</line></clef>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_F4 );
        CHECK( pClef && pClef->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, clef_04)
    {
        //@04. clef with octave change

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<clef number='2'>"
                "<sign>G</sign>"
                "<line>2</line>"
                "<clef-octave-change>-1</clef-octave-change>"
            "</clef>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_G2_8 );
        CHECK( pClef && pClef->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, clef_05)
    {
        //@05. clef with invalid octave change

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Warning: <clef-octave-change> only supported for up to two octaves. Ignored."
            << endl;
        parser.parse_text(
            "<clef number='2'>"
                "<sign>F</sign>"
                "<line>4</line>"
                "<clef-octave-change>-3</clef-octave-change>"
            "</clef>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_F4 );
        CHECK( pClef && pClef->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, clef_06)
    {
        //@06. G clef on invalid line

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Warning: G clef only supported in lines 1 or 2. Clef G3 changed to G2."
            << endl;
        parser.parse_text(
            "<clef number='2'>"
                "<sign>G</sign>"
                "<line>3</line>"
            "</clef>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_G2 );
        CHECK( pClef && pClef->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, clef_07)
    {
        //@07. F clef on invalid line

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Warning: F clef only supported in lines 3, 4 or 5. Clef F2 changed to F4."
            << endl;
        parser.parse_text(
            "<clef number='2'>"
                "<sign>F</sign>"
                "<line>2</line>"
            "</clef>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_F4 );
        CHECK( pClef && pClef->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, clef_08)
    {
        //@08. C clef on invalid line

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Warning: C clef only supported in lines 1 to 5. Clef C6 changed to C1."
            << endl;
        parser.parse_text(
            "<clef number='2'>"
                "<sign>C</sign>"
                "<line>6</line>"
            "</clef>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef && pClef->get_clef_type() == k_clef_C1 );
        CHECK( pClef && pClef->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ direction -------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_01)
    {
        //@01. direction: minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><dynamics><fp/></dynamics></direction-type>"
            "</direction>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        if (pSO)
        {
            CHECK( pSO->get_num_attachments() == 1 );
            ImoDynamicsMark* pDM = dynamic_cast<ImoDynamicsMark*>( pSO->get_attachment(0) );
            CHECK( pDM && pDM != nullptr );
            CHECK( pDM && pDM->get_mark_type() == "fp" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_02)
    {
        //@02  <words> repeat-mark. Minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><words>To Coda</words></direction-type>"
            "<sound tocoda='coda'/>"
        "</direction>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        if (pSO)
        {
            CHECK( pSO->get_num_attachments() == 1 );
            CHECK( pSO->get_placement() == k_placement_default );
            CHECK( pSO->get_display_repeat() == k_repeat_to_coda );
            CHECK( pSO->get_sound_repeat() == k_repeat_none );

            ImoTextRepetitionMark* pAO = dynamic_cast<ImoTextRepetitionMark*>( pSO->get_attachment(0) );
            CHECK( pAO != nullptr );
            if (pAO)
            {
                CHECK( pAO->get_text() == "To Coda" );
                CHECK( pAO->get_repeat_mark() == k_repeat_to_coda );
                CHECK( pAO->get_language() == "it" );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_03)
    {
        //@03  <words> other. Minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><words>Andante</words></direction-type>"
        "</direction>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        if (pSO)
        {
            CHECK( pSO != nullptr );
            CHECK( pSO->get_num_attachments() == 1 );
            CHECK( pSO->get_placement() == k_placement_default );
            CHECK( pSO->get_display_repeat() == k_repeat_none );
            CHECK( pSO->get_sound_repeat() == k_repeat_none );

            ImoScoreText* pAO = dynamic_cast<ImoScoreText*>( pSO->get_attachment(0) );
            CHECK( pAO != nullptr );
            if (pAO)
            {
                CHECK( pAO->get_text() == "Andante" );
                CHECK( pAO->get_language() == "it" );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_04)
    {
        //@04. regex for 'Da Capo'

        CHECK (mxl_type_of_repetion_mark("Da Capo") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark("Da capo") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark(" da capo") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark(" da capo ") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark(" DaCapo") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark("Da capo") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark("dc") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark("d.c.") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark("d. c.") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark("d.c. ") == k_repeat_da_capo );
        CHECK (mxl_type_of_repetion_mark(" d.c. ") == k_repeat_da_capo );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_05)
    {
        //@05. regex for 'Da Capo Al Fine'

        CHECK (mxl_type_of_repetion_mark("da capo al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark("Da Capo al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark("Da capo al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark(" da capo al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark(" da capo  al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark(" DaCapo al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark("Da capo al fine ") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark("dc al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark("d.c. al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark("d. c. al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark("d.c.  al fine") == k_repeat_da_capo_al_fine );
        CHECK (mxl_type_of_repetion_mark(" d.c. al fine ") == k_repeat_da_capo_al_fine );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_06)
    {
        //@06. regex for 'Da Capo Al Coda'

        CHECK (mxl_type_of_repetion_mark("da capo al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark(" da capo al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark(" da capo  al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark(" DaCapo al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark("Da capo al coda ") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark("dc al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark("d.c. al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark("d. c. al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark("d.c.  al coda") == k_repeat_da_capo_al_coda );
        CHECK (mxl_type_of_repetion_mark(" d.c. al coda ") == k_repeat_da_capo_al_coda );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_07)
    {
        //@07. regex for 'Dal Segno'

        CHECK (mxl_type_of_repetion_mark("dal segno") == k_repeat_dal_segno );
        CHECK (mxl_type_of_repetion_mark("del segno") == k_repeat_dal_segno );
        CHECK (mxl_type_of_repetion_mark(" dal  segno ") == k_repeat_dal_segno );
        CHECK (mxl_type_of_repetion_mark("d.s.") == k_repeat_dal_segno );
        CHECK (mxl_type_of_repetion_mark(" d.s.") == k_repeat_dal_segno );
        CHECK (mxl_type_of_repetion_mark("d.s. ") == k_repeat_dal_segno );
        CHECK (mxl_type_of_repetion_mark(" d.s. ") == k_repeat_dal_segno );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_08)
    {
        //@08. regex for 'Dal Segno Al Fine'

        CHECK (mxl_type_of_repetion_mark("dal segno al fine") == k_repeat_dal_segno_al_fine );
        CHECK (mxl_type_of_repetion_mark("d.s. al fine") == k_repeat_dal_segno_al_fine );
        CHECK (mxl_type_of_repetion_mark(" dal  segno  al  fine ") == k_repeat_dal_segno_al_fine );
        CHECK (mxl_type_of_repetion_mark(" d.s.  al  fine ") == k_repeat_dal_segno_al_fine );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_09)
    {
        //@09. regex for 'Dal Segno Al Coda'

        CHECK (mxl_type_of_repetion_mark("dal segno al coda") == k_repeat_dal_segno_al_coda );
        CHECK (mxl_type_of_repetion_mark("del segno al coda") == k_repeat_dal_segno_al_coda );
        CHECK (mxl_type_of_repetion_mark(" dal  segno  al  coda ") == k_repeat_dal_segno_al_coda );
        CHECK (mxl_type_of_repetion_mark("d.s. al coda") == k_repeat_dal_segno_al_coda );
        CHECK (mxl_type_of_repetion_mark(" d.s. al coda") == k_repeat_dal_segno_al_coda );
        CHECK (mxl_type_of_repetion_mark("d.s.  al coda") == k_repeat_dal_segno_al_coda );
        CHECK (mxl_type_of_repetion_mark(" d.s.  al  coda ") == k_repeat_dal_segno_al_coda );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_10)
    {
        //@10. regex for 'Fine'

        CHECK (mxl_type_of_repetion_mark("fine") == k_repeat_fine );
        CHECK (mxl_type_of_repetion_mark(" fine") == k_repeat_fine );
        CHECK (mxl_type_of_repetion_mark(" fine ") == k_repeat_fine );
        CHECK (mxl_type_of_repetion_mark(" Fine") == k_repeat_fine );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_words_11)
    {
        //@11. regex for 'To Coda'

        CHECK (mxl_type_of_repetion_mark("to coda") == k_repeat_to_coda );
        CHECK (mxl_type_of_repetion_mark("to  coda") == k_repeat_to_coda );
        CHECK (mxl_type_of_repetion_mark(" to coda") == k_repeat_to_coda );
        CHECK (mxl_type_of_repetion_mark("to coda ") == k_repeat_to_coda );
        CHECK (mxl_type_of_repetion_mark(" to coda ") == k_repeat_to_coda );
        CHECK (mxl_type_of_repetion_mark(" to  coda ") == k_repeat_to_coda );
        CHECK (mxl_type_of_repetion_mark("tocoda") == k_repeat_to_coda );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_segno_12)
    {
        //@0012  <segno>. Minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><segno/></direction-type>"
        "</direction>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        if (pSO)
        {
            CHECK( pSO->get_num_attachments() == 1 );
            CHECK( pSO->get_placement() == k_placement_default );
            CHECK( pSO->get_display_repeat() == k_repeat_segno );
            CHECK( pSO->get_sound_repeat() == k_repeat_none );

            ImoSymbolRepetitionMark* pAO = dynamic_cast<ImoSymbolRepetitionMark*>( pSO->get_attachment(0) );
            CHECK( pAO != nullptr );
            CHECK( pAO && pAO->get_symbol() == ImoSymbolRepetitionMark::k_segno );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_direction_coda_13)
    {
        //@13  <coda>. Minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><coda/></direction-type>"
        "</direction>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        if (pSO)
        {
            CHECK( pSO->get_num_attachments() == 1 );
            CHECK( pSO->get_placement() == k_placement_default );
            CHECK( pSO->get_display_repeat() == k_repeat_coda );
            CHECK( pSO->get_sound_repeat() == k_repeat_none );

            ImoSymbolRepetitionMark* pAO
                = dynamic_cast<ImoSymbolRepetitionMark*>( pSO->get_attachment(0) );
            CHECK( pAO != nullptr );
            CHECK( pAO && pAO->get_symbol() == ImoSymbolRepetitionMark::k_coda );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ dynamics ------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_dynamics_01)
    {
        //@01. minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><dynamics><fp/></dynamics></direction-type>"
            "</direction>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        if (pSO)
        {
            CHECK( pSO->get_num_attachments() == 1 );
            ImoDynamicsMark* pDM = dynamic_cast<ImoDynamicsMark*>( pSO->get_attachment(0) );
            CHECK( pDM && pDM != nullptr );
            CHECK( pDM && pDM->get_mark_type() == "fp" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_dynamics_02)
    {
        //@02. placement imported ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><dynamics placement='below'><fp/></dynamics></direction-type>"
            "</direction>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        if (pSO)
        {
            CHECK( pSO->get_num_attachments() == 1 );
            ImoDynamicsMark* pDM = dynamic_cast<ImoDynamicsMark*>( pSO->get_attachment(0) );
            CHECK( pDM && pDM != nullptr );
            CHECK( pDM && pDM->get_mark_type() == "fp" );
            CHECK( pDM && pDM->get_placement() == k_placement_below );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_dynamics_03)
    {
        //@03. placement inherited from parent

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction placement='below'>"
            "<direction-type><dynamics><fp/></dynamics></direction-type>"
            "</direction>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        if (pSO)
        {
            CHECK( pSO->get_num_attachments() == 1 );
            ImoDynamicsMark* pDM = dynamic_cast<ImoDynamicsMark*>( pSO->get_attachment(0) );
            CHECK( pDM && pDM != nullptr );
            CHECK( pDM && pDM->get_mark_type() == "fp" );
            CHECK( pDM && pDM->get_placement() == k_placement_below );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_dynamics_04)
    {
        //@04. dynamics in direction transferred to next note

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'><measure number='1'>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>quarter</type></note>"
            "<direction placement='below'>"
            "<direction-type><dynamics><fp/></dynamics></direction-type>"
            "</direction>"
            "<note><pitch><step>E</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>quarter</type></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
                CHECK( pInfo == nullptr );
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 4 );
                ImoObj::children_iterator it = pMD->begin();    //note a4
                CHECK( (*it)->is_note() );
                ++it;   //empty ImoDirection
                CHECK( (*it)->is_direction() );
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 0 );
                    CHECK( pDir->has_relations() == false );
                }
                ++it;   //note e4
                CHECK( (*it)->is_note() );
                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    CHECK( pNote->get_num_attachments() == 1 );
                    ImoDynamicsMark* pDM = dynamic_cast<ImoDynamicsMark*>( pNote->get_attachment(0) );
                    CHECK( pDM && pDM != nullptr );
                    CHECK( pDM && pDM->get_mark_type() == "fp" );
                    CHECK( pDM && pDM->get_placement() == k_placement_below );
                }
                ++it;   //barline
                CHECK( (*it)->is_barline() );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_dynamics_05)
    {
        //@05. dynamics in direction not transferred when no notes after it

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'><measure number='1'>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>quarter</type></note>"
            "<direction placement='below'>"
            "<direction-type><dynamics><fp/></dynamics></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
                CHECK( pInfo == nullptr );
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 3 );
                ImoObj::children_iterator it = pMD->begin();    //note a4
                CHECK( (*it)->is_note() );
                ++it;   //ImoDirection with the dynamics
                CHECK( (*it)->is_direction() );
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->has_relations() == false );
                    CHECK( pDir->get_num_attachments() == 1 );
                    ImoDynamicsMark* pDM = dynamic_cast<ImoDynamicsMark*>( pDir->get_attachment(0) );
                    CHECK( pDM && pDM != nullptr );
                    CHECK( pDM && pDM->get_mark_type() == "fp" );
                    CHECK( pDM && pDM->get_placement() == k_placement_below );
                }
                ++it;   //barline
                CHECK( (*it)->is_barline() );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ key ------------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_01)
    {
        //@01 minimum content parsed ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<key><fifths>2</fifths></key>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKeySignature != nullptr );
        if (pKeySignature)
        {
            CHECK( pKeySignature->get_key_type() == k_key_D );
            CHECK( pKeySignature->get_staff() == 0 );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_02)
    {
        //@02 key in minor mode
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<key><fifths>5</fifths><mode>minor</mode></key>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKeySignature != nullptr );
        if (pKeySignature)
        {
            CHECK( pKeySignature->get_key_type() == k_key_gs );
            CHECK( pKeySignature->get_staff() == 0 );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ measure -------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, measure_01)
    {
        //@01. MeasuresInfo in Barline but not in Instrument

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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
                CHECK( pInfo == nullptr );
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 3 );
                ImoObj::children_iterator it = pMD->begin();    //note a4
                CHECK( (*it)->is_note() );
                ++it;   //note c3
                CHECK( (*it)->is_note() );
                ++it;   //barline
                CHECK( (*it)->is_barline() );
                ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
                CHECK( pBarline != nullptr );
                if (pBarline)
                {
                    CHECK( pBarline->get_type() == k_barline_simple );
                    CHECK( pBarline->is_visible() );
                    pInfo = pBarline->get_measure_info();
                    CHECK( pInfo != nullptr );
                    CHECK( pInfo->count == 1 );
            //        cout << test_name() << ": count=" << pInfo->count << endl;
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, measure_02)
    {
        //@02. Two measures

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
            "<measure number='2'>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
                CHECK( pInfo == nullptr );
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 6 );
                ImoObj::children_iterator it = pMD->begin();    //measure 1: note a4
                CHECK( (*it)->is_note() );
                ++it;   //note c3
                CHECK( (*it)->is_note() );
                ++it;   //barline
                CHECK( (*it)->is_barline() );
                ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
                CHECK( pBarline != nullptr );
                if (pBarline)
                {
                    CHECK( pBarline->get_type() == k_barline_simple );
                    CHECK( pBarline->is_visible() );
                    TypeMeasureInfo* pInfo1 = pBarline->get_measure_info();
                    CHECK( pInfo1 != nullptr );
                    CHECK( pInfo1->count == 1 );
                    CHECK( pInfo1->number == "1" );
            //        cout << test_name() << ": count=" << pInfo1->count
            //             << ", number=" << pInfo1->number << endl;

                    //measure 2
                    ++it;   //note a4
                    CHECK( (*it)->is_note() );
                    ++it;   //note c3
                    CHECK( (*it)->is_note() );
                    ++it;   //barline
                    CHECK( (*it)->is_barline() );
                    pBarline = dynamic_cast<ImoBarline*>( *it );
                    CHECK( pBarline != nullptr );
                    if (pBarline)
                    {
                        CHECK( pBarline->get_type() == k_barline_simple );
                        CHECK( pBarline->is_visible() );
                        TypeMeasureInfo* pInfo2 = pBarline->get_measure_info();
                        CHECK( pInfo2 != nullptr );
                        CHECK( pInfo2->count == 2 );
                        CHECK( pInfo2->number == "2" );
                //        cout << test_name() << ": count=" << pInfo2->count
                //             << ", number=" << pInfo2->number << endl;

                        CHECK( pInfo1 != pInfo2 );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, measure_03)
    {
        //@03. Explicit barline

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
            "<measure number='X2'>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
           "<barline location='right'>"
                "<bar-style>light-heavy</bar-style>"
           "</barline>"
           "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
                CHECK( pInfo == nullptr );
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 6 );
                ImoObj::children_iterator it = pMD->begin();    //measure 1: note a4
                CHECK( (*it)->is_note() );
                ++it;   //note c3
                CHECK( (*it)->is_note() );
                ++it;   //barline
                CHECK( (*it)->is_barline() );
                ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
                CHECK( pBarline != nullptr );
                if (pBarline)
                {
                    CHECK( pBarline->get_type() == k_barline_simple );
                    CHECK( pBarline->is_visible() );
                    TypeMeasureInfo* pInfo1 = pBarline->get_measure_info();
                    CHECK( pInfo1 != nullptr );
                    CHECK( pInfo1->count == 1 );
                    CHECK( pInfo1->number == "1" );
            //        cout << test_name() << ": count=" << pInfo1->count
            //             << ", number=" << pInfo1->number << endl;

                    //measure 2
                    ++it;   //note a4
                    CHECK( (*it)->is_note() );
                    ++it;   //note c3
                    CHECK( (*it)->is_note() );
                    ++it;   //barline
                    CHECK( (*it)->is_barline() );
                    pBarline = dynamic_cast<ImoBarline*>( *it );
                    CHECK( pBarline != nullptr );
                    if (pBarline)
                    {
                        CHECK( pBarline->get_type() == k_barline_end );
                        CHECK( pBarline->is_visible() );
                        TypeMeasureInfo* pInfo2 = pBarline->get_measure_info();
                        CHECK( pInfo2 != nullptr );
                        CHECK( pInfo2->count == 2 );
                        CHECK( pInfo2->number == "X2" );
                //        cout << test_name() << ": count=" << pInfo2->count
                //             << ", number=" << pInfo2->number << endl;

                        CHECK( pInfo1 != pInfo2 );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ metronome -----------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, metronome_01)
    {
        //@01. Metronome. note,dots = value

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction placement='above'>"
              "<direction-type>"
                "<metronome parentheses='no' default-x='20.00' default-y='20.00'>"
                  "<beat-unit>quarter</beat-unit>"
                  "<beat-unit-dot/>"
                  "<beat-unit-dot/>"
                  "<per-minute>55</per-minute>"
                "</metronome>"
              "</direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 2 );  //direction + barline
                CHECK( pMD != nullptr );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_direction() );
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    ImoMetronomeMark* pMM =
                        static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
                    CHECK( pMM->get_ticks_per_minute() == 55 );
                    CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_value );
                    CHECK( pMM->get_left_note_type() == k_quarter );
                    CHECK( pMM->get_left_dots() == 2 );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, metronome_02)
    {
        //@02. Metronome. note,dots = note,dots

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction placement='above'>"
              "<direction-type>"
                "<metronome parentheses='no' default-x='20.00' default-y='20.00'>"
                  "<beat-unit>long</beat-unit>"
                  "<beat-unit>32nd</beat-unit>"
                  "<beat-unit-dot/>"
                "</metronome>"
              "</direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 2 );  //direction + barline
                CHECK( pMD != nullptr );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_direction() );
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    ImoMetronomeMark* pMM =
                        static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
                    CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_note );
                    CHECK( pMM->get_left_note_type() == k_longa );
                    CHECK( pMM->get_left_dots() == 0 );
                    CHECK( pMM->get_right_note_type() == k_32nd );
                    CHECK( pMM->get_right_dots() == 1 );
                    CHECK( pMM->is_visible() == true );
                    CHECK( pMM->has_parenthesis() == false );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, metronome_03)
    {
        //@03. Metronome. error

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part 'P1', measure '1'. Error in metronome parameters. "
            "Replaced by '(metronome 60)'." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction placement='above'>"
              "<direction-type>"
                "<metronome parentheses='no' default-x='20.00' default-y='20.00'>"
                  "<beat-unit>quarter</beat-unit>"
                "</metronome>"
              "</direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 2 );  //direction + barline
                CHECK( pMD != nullptr );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_direction() );
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
                    CHECK( pMM->get_ticks_per_minute() == 60 );
                    CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
                    CHECK( pMM->is_visible() == true );
                    CHECK( pMM->has_parenthesis() == false );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

//    TEST_FIXTURE(MxlAnalyserTestFixture, metronome_04)
//    {
//        //@04. Metronome. metronome-note+, (metronome-relation, metronome-note+)?)
//
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        expected << "Line 0. Part 'P1', measure '1'. Error in metronome parameters. "
//            "Replaced by '(metronome 60)'." << endl;
//        parser.parse_text(
//            "<score-partwise version='3.0'><part-list>"
//            "<score-part id='P1'><part-name>Music</part-name></score-part>"
//            "</part-list><part id='P1'>"
//            "<measure number='1'>"
//            "<direction placement='above'>"
//              "<direction-type>"
//                "<metronome parentheses='no' default-x='20.00' default-y='20.00'>"
//                  "<beat-unit>quarter</beat-unit>"
//                "</metronome>"
//              "</direction-type>"
//            "</direction>"
//            "</measure>"
//            "</part></score-partwise>"
//        );
//        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//
////        cout << test_name() << endl;
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pRoot != nullptr);
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
//        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//        CHECK( pMD != nullptr );
//
//        CHECK( pMD->get_num_children() == 2 );  //direction + barline
//        CHECK( pMD != nullptr );
//        ImoObj::children_iterator it = pMD->begin();
//        CHECK( (*it)->is_direction() );
//        ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
//        CHECK( pDir != nullptr );
//        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
//        CHECK( pMM->get_ticks_per_minute() == 60 );
//        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
//        CHECK( pMM->is_visible() == true );
//        CHECK( pMM->has_parenthesis() == false );
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }


    //@ midi-device -----------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, midi_device_01)
    {
        //@01. midi-device

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument id='P1-I1'>"
                    "<instrument-name>Marimba</instrument-name>"
                "</score-instrument>"
                "<midi-device>Bank 1</midi-device>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_score_instr_name() == "Marimba" );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi && pMidi->get_midi_port() == 0 );
        CHECK( pMidi && pMidi->get_midi_device_name() == "Bank 1" );
        CHECK( pMidi && pMidi->get_midi_name() == "" );
        CHECK( pMidi && pMidi->get_midi_bank() == 0 );
        CHECK( pMidi && pMidi->get_midi_channel() == -1 );
        CHECK( pMidi && pMidi->get_midi_program() == 0 );
        CHECK( pMidi && pMidi->get_midi_unpitched() == 0 );
        CHECK( is_equal_float(pMidi->get_midi_volume(), 1.0f) );
        CHECK( pMidi && pMidi->get_midi_pan() == 0.0 );
        CHECK( pMidi && pMidi->get_midi_elevation() == 0.0 );
//        cout << test_name() << endl;
//        cout << "instr.name= " << pInfo->get_score_instr_name() << endl
//             << "instr.abbrev= " << pInfo->get_score_instr_abbrev() << endl
//             << "id= " << pInfo->get_score_instr_id() << endl
//             << "name= " << pInfo->get_score_instr_name() << endl
//             << "abbrev= " << pInfo->get_score_instr_abbrev() << endl
//             << "sound= " << pInfo->get_score_instr_sound() << endl
//             << "virt.library= " << pInfo->get_score_instr_virtual_library() << endl
//             << "virt.name= " << pInfo->get_score_instr_virtual_name() << endl
//             << "port= " << pMidi->get_midi_port() << endl
//             << "device name= " << pMidi->get_midi_device_name() << endl
//             << "midi name= " << pMidi->get_midi_name() << endl
//             << "bank= " << pMidi->get_midi_bank() << endl
//             << "channel= " << pMidi->get_midi_channel() << endl
//             << "program= " << pMidi->get_midi_program() << endl
//             << "unpitched= " << pMidi->get_midi_unpitched() << endl
//             << "volume= " << pMidi->get_midi_volume() << endl
//             << "pan= " << pMidi->get_midi_pan() << endl
//             << "elevation= " << pMidi->get_midi_elevation() << endl;

        delete pRoot;
    }

    //@ midi-instrument -----------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, midi_instrument_01)
    {
        //@01. midi-instrument. missing id

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. midi-instrument: missing mandatory attribute 'id'." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument id='P1-I1'>"
                    "<instrument-name>Marimba</instrument-name>"
                "</score-instrument>"
                "<midi-instrument>"
                    "<midi-channel>1</midi-channel>"
                "</midi-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_score_instr_name() == "Marimba" );
//        cout << test_name() << endl;
//        cout << "score-instr: id= " << pInfo->get_score_instr_id()
//             << ", name= " << pInfo->get_score_instr_name() << endl;

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, midi_instrument_02)
    {
        //@02. midi instrument. full information

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Flute 1</part-name>"
                "<part-abbreviation>Fl. 1</part-abbreviation>"
                "<score-instrument id='P1-I1'>"
                    "<instrument-name>ARIA Player</instrument-name>"
                    "<instrument-abbreviation>ARIA</instrument-abbreviation>"
                    "<instrument-sound>wind.flutes.flute</instrument-sound>"
                    "<virtual-instrument>"
                        "<virtual-library>Garritan Instruments for Finale</virtual-library>"
                        "<virtual-name>001. Woodwinds/1. Flutes/Flute Plr1</virtual-name>"
                    "</virtual-instrument>"
                "</score-instrument>"
                "<midi-device>Bank 1</midi-device>"
                "<midi-instrument id='P1-I1'>"
                    "<midi-channel>1</midi-channel>"
                    "<midi-program>1</midi-program>"
                    "<volume>80</volume>"
                    "<pan>-70</pan>"
                "</midi-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>"
        );
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_name().get_text() == "Flute 1" );
        CHECK( pInstr->get_abbrev().get_text() == "Fl. 1" );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_score_instr_name() == "ARIA Player" );
        CHECK( pInfo->get_score_instr_abbrev() == "ARIA" );
        CHECK( pInfo->get_score_instr_sound() == "wind.flutes.flute" );
        CHECK( pInfo->get_score_instr_virtual_library() == "Garritan Instruments for Finale" );
	    CHECK( pInfo->get_score_instr_virtual_name() == "001. Woodwinds/1. Flutes/Flute Plr1" );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi && pMidi->get_midi_port() == 0 );
        CHECK( pMidi && pMidi->get_midi_device_name() == "Bank 1" );
        CHECK( pMidi && pMidi->get_midi_name() == "" );
        CHECK( pMidi && pMidi->get_midi_bank() == 0 );
        CHECK( pMidi && pMidi->get_midi_channel() == 0 );
        CHECK( pMidi && pMidi->get_midi_program() == 0 );
        CHECK( pMidi && pMidi->get_midi_unpitched() == 0 );
        CHECK( is_equal_float(pMidi->get_midi_volume(), 0.8f) );
        CHECK( pMidi && pMidi->get_midi_pan() == -70.0 );
        CHECK( pMidi && pMidi->get_midi_elevation() == 0.0 );
//        cout << test_name() << endl;
//        cout << "instr.name= " << pInstr->get_name().get_text() << endl
//             << "instr.abbrev= " << pInstr->get_abbrev().get_text() << endl
//             << "id= " << pInfo->get_score_instr_id() << endl
//             << "name= " << pInfo->get_score_instr_name() << endl
//             << "abbrev= " << pInfo->get_score_instr_abbrev() << endl
//             << "sound= " << pInfo->get_score_instr_sound() << endl
//             << "virt.library= " << pInfo->get_score_instr_virtual_library() << endl
//             << "virt.name= " << pInfo->get_score_instr_virtual_name() << endl
//             << "port= " << pMidi->get_midi_port() << endl
//             << "device name= " << pMidi->get_midi_device_name() << endl
//             << "midi name= " << pMidi->get_midi_name() << endl
//             << "bank= " << pMidi->get_midi_bank() << endl
//             << "channel= " << pMidi->get_midi_channel() << endl
//             << "program= " << pMidi->get_midi_program() << endl
//             << "unpitched= " << pMidi->get_midi_unpitched() << endl
//             << "volume= " << pMidi->get_midi_volume() << endl
//             << "pan= " << pMidi->get_midi_pan() << endl
//             << "elevation= " << pMidi->get_midi_elevation() << endl;

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, midi_instrument_03)
    {
        //@03. midi-instrument. id doesn't match any score-instrument

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. id 'I1' doesn't match any <score-instrument>"
                 << ". <midi-instrument> ignored." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument id='P1-I1'>"
                    "<instrument-name>Marimba</instrument-name>"
                "</score-instrument>"
                "<midi-instrument id='I1'>"
                    "<midi-channel>1</midi-channel>"
                "</midi-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_score_instr_name() == "Marimba" );

        delete pRoot;
    }


    //@ note -----------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_00)
    {
        //@00 minimum content parsed ok. Note saved
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>E</step><octave>3</octave></pitch>"
            "<duration>4</duration><type>whole</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_whole );
        CHECK( pNote && pNote->get_octave() == 3 );
        CHECK( pNote && pNote->get_step() == k_step_E );
        CHECK( pNote && pNote->get_duration() == k_duration_whole );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );
        CHECK( a.get_last_note() == pNote );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_01)
    {
        //@01 invalid step returns C
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Unknown note step 'e'. Replaced by 'C'." << endl;
        parser.parse_text("<note><pitch><step>e</step><octave>4</octave></pitch>"
            "<duration>4</duration><type>whole</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_whole );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );
        CHECK( pNote && pNote->get_duration() == k_duration_whole );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_02)
    {
        //@02 invalid octave returns 4
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Unknown octave 'e'. Replaced by '4'." << endl;
        parser.parse_text("<note><pitch><step>D</step><octave>e</octave></pitch>"
            "<duration>1</duration><type>quarter</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_quarter );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_D );
        CHECK( pNote && pNote->get_duration() == k_duration_quarter );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_03)
    {
        //@03 alter. Duration different from type
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>G</step><alter>-1</alter>"
            "<octave>5</octave></pitch>"
            "<duration>4</duration><type>eighth</type></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( is_equal_float(pNote->get_actual_accidentals(), -1.0f) );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 5 );
        CHECK( pNote && pNote->get_step() == k_step_G );
        CHECK( pNote && pNote->get_duration() == k_duration_whole );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_04)
    {
        //@04 accidental
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_actual_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_notated_accidentals() == k_sharp );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_half );
        CHECK( pNote && pNote->get_octave() == 1 );
        CHECK( pNote && pNote->get_step() == k_step_B );
        CHECK( pNote && pNote->get_duration() == k_duration_quarter );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_05)
    {
        //@05 staff
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_actual_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_whole );
        CHECK( pNote && pNote->get_octave() == 3 );
        CHECK( pNote && pNote->get_step() == k_step_A );
        CHECK( pNote && pNote->get_duration() == k_duration_whole );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );
        CHECK( pNote && pNote->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_06)
    {
        //@06 stem
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_actual_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_quarter );
        CHECK( pNote && pNote->get_octave() == 3 );
        CHECK( pNote && pNote->get_step() == k_step_A );
        CHECK( pNote && pNote->get_duration() == k_duration_quarter );
        CHECK( pNote && pNote->get_stem_direction() == k_stem_down );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_07)
    {
        //@07 chord ok. start and end notes
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            ImoInstrument* pInstr = pScore->get_instrument(0);
            ImoMusicData* pMD = pInstr->get_musicdata();
            CHECK( pMD != nullptr );

            ImoObj::children_iterator it = pMD->begin();
            CHECK( pMD->get_num_children() == 3 );

            ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
            CHECK( pNote != nullptr );
            CHECK( pNote && pNote->is_in_chord() == true );
            CHECK( pNote && pNote->is_start_of_chord() == true );
            CHECK( pNote && pNote->is_end_of_chord() == false );

            ++it;
            pNote = dynamic_cast<ImoNote*>( *it );
            CHECK( pNote != nullptr );
            CHECK( pNote && pNote->is_in_chord() == true );
            CHECK( pNote && pNote->is_start_of_chord() == false );
            CHECK( pNote && pNote->is_end_of_chord() == true );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_08)
    {
        //@08 chord ok. intermediate notes
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

        ImoObj::children_iterator it = pMD->begin();
        CHECK( pMD->get_num_children() == 4 );

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_in_chord() == true );
        CHECK( pNote && pNote->is_start_of_chord() == true );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_in_chord() == true );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_in_chord() == true );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == true );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_09)
    {
        //@09 Type implied by duration
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><pitch><step>G</step><alter>-1</alter>"
            "<octave>5</octave></pitch>"
            "<duration>2</duration></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( is_equal_float(pNote->get_actual_accidentals(), -1.0f) );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_half );
        CHECK( pNote && pNote->get_octave() == 5 );
        CHECK( pNote && pNote->get_step() == k_step_G );
        CHECK( pNote && pNote->get_duration() == k_duration_half );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_10)
    {
        //@10 unpitched note

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><unpitched/>"
            "<duration>1</duration><type>half</type>"
            "</note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_actual_accidentals() == k_acc_not_computed );
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_half );
        CHECK( pNote && pNote->is_pitch_defined() == false );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_no_pitch );
        CHECK( pNote && pNote->get_duration() == k_duration_quarter );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_11)
    {
        //@11. Missing <duration> in regular note/rest

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Note/Rest: missing <duration> element. Assuming 1." << endl;
        parser.parse_text("<note><pitch><step>B</step><alter>2</alter>"
            "<octave>2</octave></pitch></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( is_equal_float(pNote->get_actual_accidentals(), 2.0f) );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_quarter );
        CHECK( pNote && pNote->get_octave() == 2 );
        CHECK( pNote && pNote->get_step() == k_step_B );
        CHECK( pNote && pNote->get_duration() == k_duration_quarter );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ grace notes ---------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_001)
    {
        //@001 grace created. relobj created. Defaults OK. Prev & Ppal notes identified
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace slash=\"yes\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            ImoInstrument* pInstr = pScore->get_instrument(0);
            ImoMusicData* pMD = pInstr->get_musicdata();
            CHECK( pMD != nullptr );
            CHECK( pMD->get_num_children() == 5 );

            ImoObj::children_iterator it = pMD->begin();
            CHECK( (*it) && (*it)->is_clef() );

            ++it;
            ImoNote* pNoteFirst = dynamic_cast<ImoNote*>( *it );
            CHECK( pNoteFirst != nullptr );
            CHECK( pNoteFirst && pNoteFirst->is_regular_note() == true );

            ++it;
            ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
            CHECK( pNote != nullptr );
            CHECK( pNote && pNote->is_grace_note() == true );
            ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
            CHECK( pGraceRO != nullptr );

            ++it;
            ImoNote* pNotePpal = dynamic_cast<ImoNote*>( *it );
            CHECK( pNotePpal != nullptr );
            CHECK( pNotePpal && pNotePpal->is_regular_note() == true );

            ++it;
            CHECK( (*it) && (*it)->is_barline() );

            //check grace relationship
            CHECK( pGraceRO->get_num_objects() == 1 );
            CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
            CHECK( pGraceRO->has_slash() == true );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_002)
    {
        //@002 grace relobj. Intermediate grace notes added to the group
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace slash=\"yes\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            ImoInstrument* pInstr = pScore->get_instrument(0);
            ImoMusicData* pMD = pInstr->get_musicdata();
            CHECK( pMD != nullptr );
            CHECK( pMD->get_num_children() == 6 );

            ImoObj::children_iterator it = pMD->begin();
            CHECK( (*it) && (*it)->is_clef() );

            ++it;
            ImoNote* pNoteFirst = dynamic_cast<ImoNote*>( *it );
            CHECK( pNoteFirst != nullptr );
            CHECK( pNoteFirst && pNoteFirst->is_regular_note() == true );

            ++it;
            ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
            CHECK( pNote != nullptr );
            CHECK( pNote && pNote->is_grace_note() == true );
            ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
            CHECK( pGraceRO != nullptr );

            ++it;
            pNote = dynamic_cast<ImoNote*>( *it );
            CHECK( pNote != nullptr );
            CHECK( pNote && pNote->is_grace_note() == true );
            CHECK( pNote->get_grace_relobj() == pGraceRO );

            ++it;
            ImoNote* pNotePpal = dynamic_cast<ImoNote*>( *it );
            CHECK( pNotePpal != nullptr );
            CHECK( pNotePpal && pNotePpal->is_regular_note() == true );
//            CHECK( pNote && pNote->is_start_of_chord() == false );
//            CHECK( pNote && pNote->is_end_of_chord() == true );

            ++it;
            CHECK( (*it) && (*it)->is_barline() );

            //check grace relationship
            CHECK( pGraceRO->get_num_objects() == 2 );
            CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
            CHECK( pGraceRO->has_slash() == true );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_003)
    {
    	//@003. grace notes. slash attribute processed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();
                //it -> clef
        ++it;   //first note
        ++it;   //first grace note
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_grace_note() == true );
        ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
        CHECK( pGraceRO != nullptr );

        //check grace relationship
        CHECK( pGraceRO->get_num_objects() == 1 );
        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
        CHECK( pGraceRO->has_slash() == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_004)
    {
    	//@004. grace notes. steal-time-previous attribute processed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace steal-time-previous=\"20\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();
                //it -> clef
        ++it;   //first note
        ++it;   //first grace note
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_grace_note() == true );
        ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
        CHECK( pGraceRO != nullptr );

        //check grace relationship
        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
        CHECK( pGraceRO->has_slash() == false );
        CHECK( pGraceRO->get_percentage() == 0.2f );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_005)
    {
    	//@005. grace notes. steal-time-following attribute processed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace slash=\"yes\" steal-time-following=\"30\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();
                //it -> clef
        ++it;   //first note
        ++it;   //first grace note
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_grace_note() == true );
        ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
        CHECK( pGraceRO != nullptr );

        //check grace relationship
        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_following );
        CHECK( pGraceRO->has_slash() == true );
        CHECK( pGraceRO->get_percentage() == 0.3f );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

//    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_006)
//    {
//    	//@006. grace notes. make-time attribute processed
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        parser.parse_text(
//            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
//            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
//            "<clef><sign>G</sign><line>2</line></clef></attributes>"
//            "<note><pitch><step>G</step><octave>4</octave></pitch>"
//                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
//            "<note><grace make-time=\"30\"/><pitch><step>D</step><octave>5</octave></pitch>"
//                "<voice>1</voice><type>eighth</type></note>"
//            "<note><pitch><step>C</step><octave>5</octave></pitch>"
//                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
//            "</measure></part></score-partwise>"
//        );
//        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//        XmlNode* tree = parser.get_tree_root();
//        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//
////        cout << test_name() << endl;
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pRoot != nullptr);
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
//        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//        ImoObj::children_iterator it = pMD->begin();
//                //it -> clef
//        ++it;   //first note
//        ++it;   //first grace note
//        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
//        CHECK( pNote != nullptr );
//        CHECK( pNote && pNote->is_grace_note() == true );
//        ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
//        CHECK( pGraceRO != nullptr );
//
//        //check grace relationship
//        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_following );
//        CHECK( pGraceRO->has_slash() == true );
//        CHECK( pGraceRO->get_time_to_make() == 0.3f );
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }

    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_007)
    {
    	//@007. slash yes (acciaccaturas) are played short. default percentage 0.1
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace slash=\"yes\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();
                //it -> clef
        ++it;   //first note
        ++it;   //first grace note
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_grace_note() == true );
        ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
        CHECK( pGraceRO != nullptr );

        //check grace relationship
        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
        CHECK( pGraceRO->has_slash() == true );
        CHECK( pGraceRO->get_percentage() == 0.2f );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_008)
    {
    	//@008. slash no (appoggiaturas) are played long. default percentage 0.5
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "<note><grace slash=\"no\"/><pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();
                //it -> clef
        ++it;   //first note
        ++it;   //first grace note
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_grace_note() == true );
        ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
        CHECK( pGraceRO != nullptr );

        //check grace relationship
        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
        CHECK( pGraceRO->has_slash() == false );
        CHECK( pGraceRO->get_percentage() == 0.5f );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ octave-shift --------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, octave_shift_01)
    {
        //@01. minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction><direction-type>"
                "<octave-shift type='down'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction><direction-type>"
                "<octave-shift type='stop'/>"
            "</direction-type></direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 2 );

//                cout << test_name() << endl;
//                cout << "num.children=" << pMD->get_num_children() << endl;
//                while (it != pMD->end())
//                {
//                    cout << (*it)->to_string() << ", " << (*it)->get_name() << endl;
//                    ++it;
//                }
//                it = pMD->begin();

                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    ImoOctaveShift* pOctave = dynamic_cast<ImoOctaveShift*>(
                                                    pNote->find_relation(k_imo_octave_shift) );
                    CHECK( pOctave != nullptr );
                    if (pOctave)
                    {
                        CHECK( pOctave->get_shift_steps() == -7 );
                        CHECK( pOctave->get_octave_shift_number() == 1 );
                        CHECK( !is_different(pOctave->get_color(), Color(0,0,0)) );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, octave_shift_02)
    {
        //@02. invalid size

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        expected << "Line 0. Invalid octave-shift size '6'. Changed to 8." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction><direction-type>"
                "<octave-shift type='up' size='6'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<note><pitch><step>E</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction><direction-type>"
                "<octave-shift type='stop'/>"
            "</direction-type></direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 3 );

//                cout << test_name() << endl;
//                cout << "num.children=" << pMD->get_num_children() << endl;
//                while (it != pMD->end())
//                {
//                    cout << (*it)->to_string() << ", " << (*it)->get_name() << endl;
//                    ++it;
//                }
//                it = pMD->begin();

                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    ImoOctaveShift* pOctave = dynamic_cast<ImoOctaveShift*>(
                                                    pNote->find_relation(k_imo_octave_shift) );
                    CHECK( pOctave != nullptr );
                    if (pOctave)
                    {
                        CHECK( pOctave->get_shift_steps() == 7 );
                        CHECK( pOctave->get_octave_shift_number() == 1 );
                        CHECK( !is_different(pOctave->get_color(), Color(0,0,0)) );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ rest --------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, rest_01)
    {
        //@01. staff number
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<note><rest/>"
            "<duration>1</duration><type>quarter</type>"
            "<staff>2</staff></note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_rest() == true );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pRoot );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_dots() == 0 );
        CHECK( pRest->get_note_type() == k_quarter );
        CHECK( pRest->get_duration() == k_duration_quarter );
        CHECK( pRest->get_staff() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, rest_02)
    {
        //@02. placement on the staff
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<note>"
                "<rest><display-step>E</display-step><display-octave>4</display-octave></rest>"
                "<duration>1</duration><type>quarter</type>"
                "<staff>2</staff>"
            "</note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_rest() == true );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pRoot );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_step() == k_step_E );
        CHECK( pRest->get_octave() == k_octave_4 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ score-instrument ----------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, score_instrument_01)
    {
        //@01. score_instrument. missing id

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. score-instrument: missing mandatory attribute 'id'." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument>"
                    "<instrument-name>Marimba</instrument-name>"
                "</score-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 0 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo == nullptr );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, score_instrument_02)
    {
        //@02. score_instrument has id

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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
//        cout << test_name() << endl;
//        cout << "score-instr: id= " << pInfo->get_score_instr_id() << endl;

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, score_instrument_03)
    {
        //@03. score_instrument. missing instrument name

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <score-instrument>: missing mandatory element <instrument-name>." << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument id='P1-I1'>"
                "</score-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_score_instr_name() == "" );
//        cout << test_name() << endl;
//        cout << "score-instr: id= " << pInfo->get_score_instr_id()
//             << ", name= " << pInfo->get_score_instr_name() << endl;

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, score_instrument_04)
    {
        //@04. score_instrument. instrument name

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument id='P1-I1'>"
                    "<instrument-name>Marimba</instrument-name>"
                "</score-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_score_instr_name() == "Marimba" );
//        cout << test_name() << endl;
//        cout << "score-instr: id= " << pInfo->get_score_instr_id()
//             << ", name= " << pInfo->get_score_instr_name() << endl;

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, score_instrument_05)
    {
        //@05. score_instrument. instrument name and sound

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list><score-part id='P1'>"
                "<part-name>Music</part-name>"
                "<score-instrument id='P1-I1'>"
                    "<instrument-name>ARIA Player</instrument-name>"
                    "<instrument-sound>wind.flutes.flute</instrument-sound>"
                "</score-instrument>"
            "</score-part></part-list><part id='P1'></part></score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_score_instr_name() == "ARIA Player" );
        CHECK( pInfo->get_score_instr_abbrev() == "" );
        CHECK( pInfo->get_score_instr_sound() == "wind.flutes.flute" );
//        cout << test_name() << endl;
//        cout << "score-instr: id= " << pInfo->get_score_instr_id()
//             << ", name= " << pInfo->get_score_instr_name()
//             << ", abbrev= " << pInfo->get_score_instr_abbrev()
//             << ", sound= " << pInfo->get_score_instr_sound() << endl;

        delete pRoot;
    }


    //@ slur ------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_slur_01)
    {
        //@01. double definition error detected

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        expected << "Line 0. A slur with the same number is already defined for this "
            "element in line 0. This slur will be ignored." << endl;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<slur number='1' type='start'/>"
                    "<slur number='1' type='stop'/>"
                "</notations>"
            "</note>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_slur_02)
    {
        //@02. valid slur, with end before start

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.1'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'><measure number='1'><attributes>"
            "<divisions>8</divisions><staves>2</staves>"
            "<clef number='1'><sign>G</sign><line>2</line></clef>"
            "<clef number='2'><sign>F</sign><line>4</line></clef>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>8</duration><voice>1</voice><type>quarter</type>"
                "<stem>up</stem><staff>1</staff></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch>"
                "<duration>8</duration><voice>1</voice><type>quarter</type>"
                "<stem>up</stem><staff>1</staff></note>"
            "<backup><duration>16</duration></backup>"
            "<forward><duration>8</duration><voice>2</voice><staff>1</staff></forward>"
            "<note><pitch><step>F</step><octave>4</octave></pitch>"
                "<duration>8</duration><voice>2</voice><type>quarter</type>"
                "<stem>down</stem><staff>1</staff>"
                "<notations><slur number='2' type='stop'/></notations></note>"
            "<backup><duration>16</duration></backup>"
            "<note><pitch><step>D</step><octave>3</octave></pitch>"
                "<duration>8</duration><voice>3</voice><type>quarter</type>"
                "<stem>up</stem><staff>2</staff>"
                "<notations><slur number='2' placement='above' type='start'/>"
                "</notations></note>"
            "<note><pitch><step>D</step><octave>4</octave></pitch>"
            "<duration>8</duration><voice>3</voice><type>quarter</type>"
            "<stem>down</stem><staff>2</staff></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
                CHECK( pInfo == nullptr );
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                CHECK( pMD->get_num_children() == 11 );

                ImoObj::children_iterator it = pMD->begin();    //clef G2
                CHECK( (*it)->is_clef() );
                ++it;   //clef F4
                CHECK( (*it)->is_clef() );
                ++it;   //note C5
                CHECK( (*it)->is_note() );
                ++it;   //note C5
                CHECK( (*it)->is_note() );
                ++it;   //goback-fwd
                CHECK( (*it)->is_go_back_fwd() );
                ++it;   //rest
                CHECK( (*it)->is_rest() );

                ++it;   //note F4. End of slur
                CHECK( (*it)->is_note() );
                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    CHECK( pNote->get_num_relations() == 1 );
                    if (pNote->get_num_relations() > 0)
                    {
                        ImoRelations* pRelObjs = pNote->get_relations();
                        CHECK( pRelObjs->get_num_items() == 1);
                        ImoRelObj* pRO = pRelObjs->get_item(0);
                        if (pRO)
                        {
                            CHECK( pRO->is_slur() );
                            ImoNote* pN1 = (static_cast<ImoSlur*>(pRO))->get_end_note();
                            CHECK( pN1 == pNote );
                        }
                    }
                }

                ++it;   //goback-fwd
                CHECK( (*it)->is_go_back_fwd() );

                ++it;   //note D3. Start of slur
                CHECK( (*it)->is_note() );
                pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    CHECK( pNote->get_num_relations() == 1 );
                    if (pNote->get_num_relations() > 0)
                    {
                        ImoRelations* pRelObjs = pNote->get_relations();
                        CHECK( pRelObjs->get_num_items() == 1);
                        ImoRelObj* pRO = pRelObjs->get_item(0);
                        if (pRO)
                        {
                            CHECK( pRO->is_slur() );
                            ImoNote* pN2 = (static_cast<ImoSlur*>(pRO))->get_start_note();
                            CHECK( pN2 == pNote );
                        }
                    }
                }
                ++it;   //note D4
                CHECK( (*it)->is_note() );
                ++it;   //barline
                CHECK( (*it)->is_barline() );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ sound ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_01)
    {
        //@01 empty <sound> ignored
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Empty <sound> element. Ignored." << endl;
        parser.parse_text("<sound></sound>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr);

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_02)
    {
        //@02 dacapo

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<sound dacapo='yes'></sound>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_sound_change() == true );
        ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>( pRoot );
        CHECK( pSC != nullptr );
        CHECK( pSC && pSC->get_bool_attribute(k_attr_dacapo) == true );
        CHECK( pSC && pSC->get_bool_attribute(k_attr_pizzicato) == false );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_03)
    {
        //@03 tempo

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<sound tempo='75'></sound>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_sound_change() == true );
        ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>( pRoot );
        CHECK( pSC != nullptr );
        CHECK( pSC && pSC->get_float_attribute(k_attr_tempo) == 75.0f );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_04)
    {
        //@04 tempo error

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Invalid real number '75,7'. Replaced by '70'." << endl;
        parser.parse_text("<sound tempo='75,7'/>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_sound_change() == true );
        ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>( pRoot );
        CHECK( pSC != nullptr );
        CHECK( pSC && pSC->get_float_attribute(k_attr_tempo) == 70.0f );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_05)
    {
        //@05 forward-repeat error

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Invalid value for 'forward-repeat' "
                    "attribute. When used, value must be 'yes'. Ignored." << endl;

        parser.parse_text("<sound forward-repeat='no' tempo='72.5' />");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_sound_change() == true );
        ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>( pRoot );
        CHECK( pSC != nullptr );
        CHECK( pSC && pSC->get_num_attributes() == 1 );
        CHECK( pSC && pSC->get_float_attribute(k_attr_tempo) == 72.5f );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_06)
    {
        //@06. <sound> inside a <direction>: attached as child

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text("<direction>"
            "<direction-type><words>To Coda</words></direction-type>"
            "<sound tocoda='coda'/>"
        "</direction>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pSO = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pSO != nullptr );
        CHECK( pSO->get_num_attachments() == 1 );
        CHECK( pSO->get_placement() == k_placement_default );
        CHECK( pSO->get_display_repeat() == k_repeat_to_coda );
        CHECK( pSO->get_sound_repeat() == k_repeat_none );

        ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>(
                                        pSO->get_child_of_type(k_imo_sound_change) );
        CHECK( pSC != nullptr );
        CHECK( pSC && pSC->get_attribute_node(k_attr_tocoda) != nullptr  );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_07)
    {
        //@07. <sound> inside a <measure>: attached as child

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<sound tempo='85'/>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>4</duration><type>16th</type></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 3 );          //third one is the barline

                ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>( *it );
                CHECK( pSC != nullptr );
                CHECK( pSC && pSC->get_attribute_node(k_attr_tempo) != nullptr  );

                ++it;
                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_08)
    {
        //@08. midi instrument for one sound-instrument inside <sound>

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<part-list>"
                "<score-part id='P1'>"
                    "<part-name>Flute 1</part-name>"
                    "<score-instrument id='P1-I1'>"
                        "<instrument-name>ARIA Player</instrument-name>"
                    "</score-instrument>"
                    "<midi-device>Bank 1</midi-device>"
                    "<midi-instrument id='P1-I1'>"
                        "<midi-channel>1</midi-channel>"
                        "<midi-program>1</midi-program>"
                        "<volume>80</volume>"
                        "<pan>-70</pan>"
                    "</midi-instrument>"
                "</score-part>"
            "</part-list>"
            "<part id='P1'>"
                "<measure number='1'>"
                "<sound>"
                    "<midi-instrument id='P1-I1'>"
                        "<midi-program>46</midi-program>"
                    "</midi-instrument>"
                "</sound>"
                "<note><pitch><step>A</step><octave>3</octave></pitch>"
                    "<duration>4</duration><type>16th</type></note>"
                "</measure>"
            "</part>"
            "</score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>(
                                            pMD->get_child_of_type(k_imo_sound_change) );
                CHECK( pSC != nullptr );
                if (pSC)
                {
                    CHECK( pSC->get_num_attributes() == 0 );
                    ImoMidiInfo* pMidi = dynamic_cast<ImoMidiInfo*>(
                                                pSC->get_child_of_type(k_imo_midi_info) );
                    CHECK( pMidi != nullptr );
                    if (pMidi)
                    {
                        CHECK( pMidi && pMidi->get_score_instr_id() == "P1-I1" );
                        CHECK( pMidi && pMidi->get_midi_port() == 0 );
                        CHECK( pMidi && pMidi->get_midi_device_name() == "Bank 1" );
                        CHECK( pMidi && pMidi->get_midi_name() == "" );
                        CHECK( pMidi && pMidi->get_midi_bank() == 0 );
                        CHECK( pMidi && pMidi->get_midi_channel() == 0 );
                        CHECK( pMidi && pMidi->get_midi_program() == 45 );
                        CHECK( pMidi && pMidi->get_midi_unpitched() == 0 );
                        CHECK( is_equal_float(pMidi->get_midi_volume(), 0.8f) );
                        CHECK( pMidi && pMidi->get_midi_pan() == -70.0 );
                        CHECK( pMidi && pMidi->get_midi_elevation() == 0.0 );
                //        cout << test_name() << endl;
                //        cout << "id= " << pMidi->get_score_instr_id() << endl
                //             << "port= " << pMidi->get_midi_port() << endl
                //             << "device name= " << pMidi->get_midi_device_name() << endl
                //             << "midi name= " << pMidi->get_midi_name() << endl
                //             << "bank= " << pMidi->get_midi_bank() << endl
                //             << "channel= " << pMidi->get_midi_channel() << endl
                //             << "program= " << pMidi->get_midi_program() << endl
                //             << "unpitched= " << pMidi->get_midi_unpitched() << endl
                //             << "volume= " << pMidi->get_midi_volume() << endl
                //             << "pan= " << pMidi->get_midi_pan() << endl
                //             << "elevation= " << pMidi->get_midi_elevation() << endl;
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_sound_09)
    {
        //@09. midi device for one sound-instrument inside <sound>

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<part-list>"
                "<score-part id='P1'>"
                    "<part-name>Flute 1</part-name>"
                    "<score-instrument id='P1-I1'>"
                        "<instrument-name>ARIA Player</instrument-name>"
                    "</score-instrument>"
                    "<midi-device>Bank 1</midi-device>"
                    "<midi-instrument id='P1-I1'>"
                        "<midi-channel>1</midi-channel>"
                        "<midi-program>1</midi-program>"
                        "<volume>80</volume>"
                        "<pan>-70</pan>"
                    "</midi-instrument>"
                "</score-part>"
            "</part-list>"
            "<part id='P1'>"
                "<measure number='1'>"
                "<sound>"
                    "<midi-device>Bank 3</midi-device>"
                    "<midi-instrument id='P1-I1'>"
                        "<midi-program>19</midi-program>"
                        "<volume>40</volume>"
                    "</midi-instrument>"
                "</sound>"
                "<note><pitch><step>A</step><octave>3</octave></pitch>"
                    "<duration>4</duration><type>16th</type></note>"
                "</measure>"
            "</part>"
            "</score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>(
                                            pMD->get_child_of_type(k_imo_sound_change) );
                CHECK( pSC != nullptr );
                if (pSC)
                {
                    CHECK( pSC->get_num_attributes() == 0 );
                    ImoMidiInfo* pMidi = dynamic_cast<ImoMidiInfo*>(
                                                pSC->get_child_of_type(k_imo_midi_info) );
                    CHECK( pMidi != nullptr );
                    if (pMidi)
                    {
                        CHECK( pMidi->get_score_instr_id() == "P1-I1" );
                        CHECK( pMidi->get_midi_port() == 0 );
                        CHECK( pMidi->get_midi_device_name() == "Bank 3" );
                        CHECK( pMidi->get_midi_name() == "" );
                        CHECK( pMidi->get_midi_bank() == 0 );
                        CHECK( pMidi->get_midi_channel() == 0 );
                        CHECK( pMidi->get_midi_program() == 18 );
                        CHECK( pMidi->get_midi_unpitched() == 0 );
                        CHECK( is_equal_float(pMidi->get_midi_volume(), 0.4f) );
                        CHECK( pMidi->get_midi_pan() == -70.0 );
                        CHECK( pMidi->get_midi_elevation() == 0.0 );
                //        cout << test_name() << endl;
                //        cout << "id= " << pMidi->get_score_instr_id() << endl
                //             << "port= " << pMidi->get_midi_port() << endl
                //             << "device name= " << pMidi->get_midi_device_name() << endl
                //             << "midi name= " << pMidi->get_midi_name() << endl
                //             << "bank= " << pMidi->get_midi_bank() << endl
                //             << "channel= " << pMidi->get_midi_channel() << endl
                //             << "program= " << pMidi->get_midi_program() << endl
                //             << "unpitched= " << pMidi->get_midi_unpitched() << endl
                //             << "volume= " << pMidi->get_midi_volume() << endl
                //             << "pan= " << pMidi->get_midi_pan() << endl
                //             << "elevation= " << pMidi->get_midi_elevation() << endl;
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ time ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_time_01)
    {
        //@01 minimum content parsed ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<time><beats>6</beats><beat-type>8</beat-type></time>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature && pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature && pTimeSignature->get_bottom_number() == 8 );
//        cout << test_name()
//             << ": top number=" << pTimeSignature->get_top_number()
//             << ", bottom: " << pTimeSignature->get_bottom_number() << endl;

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_time_02)
    {
        //@02 error in time signature
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <time>: missing mandatory element <beat-type>." << endl;
        parser.parse_text("<time><beats>6</beats></time>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature && pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature && pTimeSignature->get_bottom_number() == 4 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ time-modification ---------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, time_modification_01)
    {
        //@01 time-modification

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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_invalid_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 5 );
        CHECK( pNote && pNote->get_step() == k_step_C );
        CHECK( pNote && pNote->get_time_modifier_top() == 2 );
        CHECK( pNote && pNote->get_time_modifier_bottom() == 3 );
//        cout << "time_modifier_top= " << pNote->get_time_modifier_top()
//             << ", time_modifier_bottom= " << pNote->get_time_modifier_bottom() << endl;

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ transpose -----------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, transpose_01)
    {
        //@01 transpose parsed. Minimum content

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<transpose><chromatic>-2</chromatic></transpose>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_transpose() == true );
        ImoTranspose* pSO = dynamic_cast<ImoTranspose*>( pRoot );
        CHECK( pSO != nullptr );
        CHECK( pSO && pSO->get_chromatic() == -2 );
        CHECK( pSO && pSO->get_diatonic() == 0 );
        CHECK( pSO && pSO->get_doubled() == false );
        CHECK( pSO && pSO->get_applicable_staff() == -1 );
        CHECK( pSO && pSO->get_octave_change() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, transpose_02)
    {
        //@02 transpose parsed. All elements

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<transpose number='2'>"
                "<diatonic>-4</diatonic>"
                "<chromatic>-7</chromatic>"
                "<octave-change>1</octave-change>"
                "<double/>"
            "</transpose>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_transpose() == true );
        ImoTranspose* pSO = dynamic_cast<ImoTranspose*>( pRoot );
        CHECK( pSO != nullptr );
        CHECK( pSO && pSO->get_chromatic() == -7 );
        CHECK( pSO && pSO->get_diatonic() == -4 );
        CHECK( pSO && pSO->get_doubled() == true );
        CHECK( pSO && pSO->get_applicable_staff() == 1 );
        CHECK( pSO && pSO->get_octave_change() == 1 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, transpose_03)
    {
        //@03 transpose parsed. Error missing mandatory 'chromatic' element

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <transpose>: missing mandatory element <chromatic>."
                 << endl;
        parser.parse_text(
            "<transpose>"
                "<diatonic>-4</diatonic>"
            "</transpose>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_transpose() == true );
        ImoTranspose* pSO = dynamic_cast<ImoTranspose*>( pRoot );
        CHECK( pSO != nullptr );
        CHECK( pSO && pSO->get_chromatic() == 0 );
        CHECK( pSO && pSO->get_diatonic() == -4 );
        CHECK( pSO && pSO->get_doubled() == false );
        CHECK( pSO && pSO->get_applicable_staff() == -1 );
        CHECK( pSO && pSO->get_octave_change() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, transpose_04)
    {
        //@04 transpose added to MusicData in right order

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
                "<transpose><diatonic>-4</diatonic><chromatic>-7</chromatic></transpose>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 1 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );
                if (pInstr)
                {
                    CHECK( pInstr->get_num_staves() == 1 );
                    ImoMusicData* pMD = pInstr->get_musicdata();
                    CHECK( pMD != nullptr );
                    CHECK( pMD->get_num_items() == 5 );
                    ImoObj::children_iterator it = pMD->begin();
                    CHECK( (*it)->is_clef() == true );
//                    cout << (*it)->get_name() << endl;
                    ++it;
                    CHECK( (*it)->is_key_signature() == true );
//                    cout << (*it)->get_name() << endl;
                    ++it;
                    CHECK( (*it)->is_time_signature() == true );
//                    cout << (*it)->get_name() << endl;
                    ++it;
                    CHECK( (*it)->is_transpose() == true );
//                    cout << (*it)->get_name() << endl;
                    ++it;
                    CHECK( (*it)->is_barline() == true );
//                    cout << (*it)->get_name() << endl;
                }
            }
        }

        delete pRoot;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );

                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    ImoTuplet* pTuplet = pNote->get_first_tuplet();
                    CHECK( pTuplet != nullptr );
                    CHECK( pTuplet && pTuplet->get_actual_number() == 1 );
                    CHECK( pTuplet && pTuplet->get_normal_number() == 1 );
                    CHECK( pTuplet && pTuplet->get_show_bracket() == k_yesno_default );
                    CHECK( pTuplet && pTuplet->get_num_objects() == 3 );
                    CHECK( pTuplet && pTuplet->get_id() == 137L );

                    ++it;
                    ++it;
                    pNote = dynamic_cast<ImoNote*>( *it );
                    CHECK( pNote != nullptr );
                    if (pNote)
                    {
                        ImoTuplet* pTuplet2 = pNote->get_first_tuplet();
                        CHECK( pTuplet2 == pTuplet );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

        ImoObj::children_iterator it = pMD->begin();
        CHECK( pMD->get_num_children() == 4 );

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        if (pNote)
        {
            ImoTuplet* pTuplet = pNote->get_first_tuplet();
            CHECK( pTuplet != nullptr );
            CHECK( pTuplet && pTuplet->get_actual_number() == 1 );
            CHECK( pTuplet && pTuplet->get_normal_number() == 1 );
            CHECK( pTuplet && pTuplet->get_show_bracket() == k_yesno_yes );
            CHECK( pTuplet && pTuplet->get_num_objects() == 3 );
            CHECK( pTuplet && pTuplet->get_id() == 141L );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                if (pMD)
                {
                    ImoObj::children_iterator it = pMD->begin();
                    CHECK( pMD->get_num_children() == 4 );

                    ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                    CHECK( pNote != nullptr );
                    if (pNote)
                    {
                        ImoTuplet* pTuplet = pNote->get_first_tuplet();
                        CHECK( pTuplet != nullptr );
                        CHECK( pTuplet && pTuplet->get_actual_number() == 3 );
                        CHECK( pTuplet && pTuplet->get_normal_number() == 2 );
                        CHECK( pTuplet && pTuplet->get_show_bracket() == k_yesno_yes );
                        CHECK( pTuplet && pTuplet->get_num_objects() == 3 );
                        CHECK( pTuplet && pTuplet->get_id() == 141L );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 10 );
        //        cout << test_name() << endl;
                //cout << "num.children= " << pMD->get_num_children() << endl;

                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    ImoTuplet* pTuplet = pNote->get_first_tuplet();
                    CHECK( pTuplet != nullptr );
                    CHECK( pTuplet && pTuplet->get_actual_number() == 3 );
                    CHECK( pTuplet && pTuplet->get_normal_number() == 2 );

                    ++it;
                    ++it;
                    pNote = dynamic_cast<ImoNote*>( *it );
                    CHECK( pNote != nullptr );
                    if (pNote)
                    {
                        list<ImoTuplet*> tuplets = get_tuplets(pNote);
                        CHECK( tuplets.size() == 2 );
                        pTuplet = tuplets.front();
                        CHECK( pTuplet != nullptr );
                        CHECK( pTuplet && pTuplet->get_actual_number() == 5 );
                        CHECK( pTuplet && pTuplet->get_normal_number() == 2 );
                //        cout << test_name() << endl;
                //        cout << "Tuplet. actual=" << pTuplet->get_actual_number()
                //             << ", normal=" << pTuplet->get_normal_number() << endl;
                        pTuplet = tuplets.back();
                        CHECK( pTuplet != nullptr );
                        CHECK( pTuplet && pTuplet->get_actual_number() == 3 );
                        CHECK( pTuplet && pTuplet->get_normal_number() == 2 );
                //        cout << test_name() << endl;
                //        cout << "Tuplet. actual=" << pTuplet->get_actual_number()
                //             << ", normal=" << pTuplet->get_normal_number() << endl;
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );

                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    ImoTuplet* pTuplet = pNote->get_first_tuplet();
                    CHECK( pTuplet != nullptr );
                    CHECK( pTuplet && pTuplet->get_actual_number() == 5 );
                    CHECK( pTuplet && pTuplet->get_normal_number() == 3 );
                    CHECK( pTuplet && pTuplet->get_show_bracket() == k_yesno_yes );
                    CHECK( pTuplet && pTuplet->get_num_objects() == 3 );

        //            cout << test_name() << endl;
        //            cout << "[" << errormsg.str() << "]" << endl;
        //            cout << "[" << expected.str() << "]" << endl;
        //            cout << "Tuplet. actual=" << pTuplet->get_actual_number()
        //                 << ", normal=" << pTuplet->get_normal_number() << endl;
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, tuplet_06)
    {
        //@06. tuplet: missing number, replaced by 1

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
                "<notations><tuplet type='start' /></notations>"
            "</note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
                "<notations><tuplet type='stop' /></notations>"
            "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );

                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    ImoTuplet* pTuplet = pNote->get_first_tuplet();
                    CHECK( pTuplet != nullptr );
                    CHECK( pTuplet && pTuplet->get_actual_number() == 1 );
                    CHECK( pTuplet && pTuplet->get_normal_number() == 1 );
                    CHECK( pTuplet && pTuplet->get_show_bracket() == k_yesno_default );
                    CHECK( pTuplet && pTuplet->get_num_objects() == 3 );
                    CHECK( pTuplet && pTuplet->get_id() == 1L );

                    ++it;
                    ++it;
                    pNote = dynamic_cast<ImoNote*>( *it );
                    CHECK( pNote != nullptr );
                    if (pNote)
                    {
                        ImoTuplet* pTuplet2 = pNote->get_first_tuplet();
                        CHECK( pTuplet2 == pTuplet );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, tuplet_07)
    {
        //@07. tuplet: error invalid number

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Invalid tuplet number. Tuplet ignored." << endl
                 << "Line 0. Invalid tuplet number. Tuplet ignored." << endl;
                 //twice because there are two invalid tuplet elements
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<note><pitch><step>G</step><alter>-1</alter>"
                "<octave>5</octave></pitch><duration>4</duration><type>16th</type>"
                "<notations><tuplet type='start' number='one' /></notations>"
            "</note>"
            "<note><chord/><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<note><chord/><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
                "<notations><tuplet type='stop' number='one' /></notations>"
            "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );

                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote != nullptr );
                if (pNote)
                {
                    ImoTuplet* pTuplet = pNote->get_first_tuplet();
                    CHECK( pTuplet == nullptr );

                    ++it;
                    ++it;
                    pNote = dynamic_cast<ImoNote*>( *it );
                    CHECK( pNote != nullptr );
                    if (pNote)
                    {
                        ImoTuplet* pTuplet2 = pNote->get_first_tuplet();
                        CHECK( pTuplet2 == pTuplet );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ volta bracket -------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, volta_bracket_01)
    {
        //@01. volta bracket is created

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
                "<note><pitch><step>A</step><octave>5</octave></pitch>"
                    "<duration>4</duration><type>16th</type>"
                "</note>"
            "</measure>"
            "<measure number='2'>"
                "<barline location='left'>"
                    "<ending number='1' type='start'/>"
                "</barline>"
                "<note><pitch><step>G</step><octave>5</octave></pitch>"
                    "<duration>4</duration><type>16th</type>"
                "</note>"
                "<barline location='right'>"
                    "<bar-style>light-heavy</bar-style>"
                    "<ending number='1' type='stop'/>"
                    "<repeat direction='backward' winged='none'/>"
                "</barline>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );
        //        cout << test_name() << endl;
        //        cout << "num.children=" << pMD->get_num_children() << endl;
        //        while (it != pMD->end())
        //        {
        //            cout << (*it)->to_string() << endl;
        //            ++it;
        //        }

                it = pMD->begin();
                ++it;
                ImoBarline* pBarline1 = dynamic_cast<ImoBarline*>( *it );
                CHECK( pBarline1 != nullptr );
                if (pBarline1)
                {
                    CHECK( pBarline1->get_type() == k_barline_simple );
                    CHECK( pBarline1->is_visible() );
                    ImoVoltaBracket* pVB1 = dynamic_cast<ImoVoltaBracket*>(
                                                pBarline1->find_relation(k_imo_volta_bracket) );
                    CHECK( pVB1 != nullptr );

                    ++it;
                    ++it;
                    ImoBarline* pBarline2 = dynamic_cast<ImoBarline*>( *it );
                    CHECK( pBarline2 != nullptr );
                    if (pBarline2)
                    {
                        CHECK( pBarline2->get_type() == k_barline_end_repetition );
                        CHECK( pBarline2->is_visible() );

                        ImoVoltaBracket* pVB2 = dynamic_cast<ImoVoltaBracket*>(
                                                    pBarline2->find_relation(k_imo_volta_bracket) );
                        CHECK( pVB2 != nullptr );

                        CHECK( pVB1 == pVB2 );
                        CHECK( pVB1 && pVB1->get_start_barline() == pBarline1 );
                        CHECK( pVB1 && pVB1->get_stop_barline() == pBarline2 );
                        CHECK( pVB1 && pVB1->has_final_jog() == true );
                        CHECK( pVB1 && pVB1->get_volta_number() == "1" );
                        CHECK( pVB1 && pVB1->get_volta_text() == "" );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, volta_bracket_02)
    {
        //@02. volta bracket: text different from number

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
                "<note><pitch><step>A</step><octave>5</octave></pitch>"
                    "<duration>4</duration><type>16th</type>"
                "</note>"
            "</measure>"
            "<measure number='2'>"
                "<barline location='left'>"
                    "<ending number='1' type='start'>First time</ending>"
                "</barline>"
                "<note><pitch><step>G</step><octave>5</octave></pitch>"
                    "<duration>4</duration><type>16th</type>"
                "</note>"
                "<barline location='right'>"
                    "<bar-style>light-heavy</bar-style>"
                    "<ending number='1' type='stop'/>"
                    "<repeat direction='backward' winged='none'/>"
                "</barline>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );
        //        cout << test_name() << endl;
        //        cout << "num.children=" << pMD->get_num_children() << endl;
        //        while (it != pMD->end())
        //        {
        //            cout << (*it)->to_string() << endl;
        //            ++it;
        //        }

                it = pMD->begin();
                ++it;
                ImoBarline* pBarline1 = dynamic_cast<ImoBarline*>( *it );
                CHECK( pBarline1 != nullptr );
                if (pBarline1)
                {
                    CHECK( pBarline1->get_type() == k_barline_simple );
                    CHECK( pBarline1->is_visible() );
                    ImoVoltaBracket* pVB1 = dynamic_cast<ImoVoltaBracket*>(
                                                pBarline1->find_relation(k_imo_volta_bracket) );
                    CHECK( pVB1 != nullptr );

                    ++it;
                    ++it;
                    ImoBarline* pBarline2 = dynamic_cast<ImoBarline*>( *it );
                    CHECK( pBarline2 != nullptr );
                    if (pBarline2)
                    {
                        CHECK( pBarline2->get_type() == k_barline_end_repetition );
                        CHECK( pBarline2->is_visible() );

                        ImoVoltaBracket* pVB2 = dynamic_cast<ImoVoltaBracket*>(
                                                    pBarline2->find_relation(k_imo_volta_bracket) );
                        CHECK( pVB2 != nullptr );

                        CHECK( pVB1 == pVB2 );
                        CHECK( pVB1 && pVB1->get_start_barline() == pBarline1 );
                        CHECK( pVB1 && pVB1->get_stop_barline() == pBarline2 );
                        CHECK( pVB1 && pVB1->has_final_jog() == true );
                        CHECK( pVB1 && pVB1->get_volta_number() == "1" );
                        CHECK( pVB1 && pVB1->get_volta_text() == "First time" );
                //        cout << "Volta number = '" << pVB1->get_volta_number() << "'" << endl;
                //        cout << "Volta text = '" << pVB1->get_volta_text() << "'" << endl;
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_volta_bracket_03)
    {
        //@03. volta bracket: regex for validating ending

        CHECK (mxl_is_valid_ending_number("2") == true );
        CHECK (mxl_is_valid_ending_number("01") == false );
        CHECK (mxl_is_valid_ending_number(" ") == true );
        CHECK (mxl_is_valid_ending_number("   ") == true );
        CHECK (mxl_is_valid_ending_number("1, 2") == true );
        CHECK (mxl_is_valid_ending_number("1, 2, 3") == true );
        CHECK (mxl_is_valid_ending_number("1, 2, 3 ") == true );    //permissive
        CHECK (mxl_is_valid_ending_number("1,2") == true );         //permissive
        CHECK (mxl_is_valid_ending_number("1,2,3") == true );       //permissive
        CHECK (mxl_is_valid_ending_number("1-3") == false );
        CHECK (mxl_is_valid_ending_number("to coda") == false );
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_volta_bracket_04)
    {
        //@04. volta bracket: regex for extracting numbers

        vector<int> reps;
        mxl_extract_numbers_from_ending("2", &reps);
        CHECK ( reps.size() == 1 );
        CHECK ( reps[0] == 2 );

        reps.clear();
        mxl_extract_numbers_from_ending("1, 2", &reps);
        CHECK ( reps.size() == 2 );
        CHECK ( reps[0] == 1 );
        CHECK ( reps[1] == 2 );

        reps.clear();
        mxl_extract_numbers_from_ending("1,2", &reps);
        CHECK ( reps.size() == 2 );
        CHECK ( reps[0] == 1 );
        CHECK ( reps[1] == 2 );

        reps.clear();
        mxl_extract_numbers_from_ending(" ", &reps);
        CHECK ( reps.size() == 0 );
    }


    //@ wedge ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_wedge_01)
    {
        //@01. minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction>"
                "<direction-type><wedge type='crescendo'/></direction-type>"
            "</direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction>"
                "<direction-type><wedge type='stop'/></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );
//                cout << test_name() << endl;
//                cout << "num.children=" << pMD->get_num_children() << endl;
//                while (it != pMD->end())
//                {
//                    cout << (*it)->to_string() << ", " << (*it)->get_name() << endl;
//                    ++it;
//                }

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 0 );
                    CHECK( pDir->get_placement() == k_placement_default );

                    ImoWedge* pWedge = dynamic_cast<ImoWedge*>(
                                           pDir->find_relation(k_imo_wedge) );
                    CHECK( pWedge != nullptr );
                    if (pWedge)
                    {
                        CHECK( pWedge->get_start_spread() == 0.0f );
                        CHECK( pWedge->get_end_spread() == 15.0f );
                        CHECK( pWedge->is_niente() == false );
                        CHECK( pWedge->is_crescendo() == true );
                        CHECK( pWedge->get_wedge_number() == 1 );
                        CHECK( !is_different(pWedge->get_color(), Color(0,0,0)) );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_wedge_02)
    {
        //@02. (stop, crescendo) order parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction>"
                "<direction-type><wedge type='stop'/></direction-type>"
            "</direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction>"
                "<direction-type><wedge type='crescendo'/></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );
//                cout << test_name() << endl;
//                cout << "num.children=" << pMD->get_num_children() << endl;
//                while (it != pMD->end())
//                {
//                    cout << (*it)->to_string() << ", " << (*it)->get_name() << endl;
//                    ++it;
//                }

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 0 );
                    CHECK( pDir->get_placement() == k_placement_default );

                    ImoWedge* pWedge = dynamic_cast<ImoWedge*>(
                                           pDir->find_relation(k_imo_wedge) );
                    CHECK( pWedge != nullptr );
                    if (pWedge)
                    {
                        CHECK( pWedge->get_start_spread() == 0.0f );
                        CHECK( pWedge->get_end_spread() == 15.0f );
                        CHECK( pWedge->is_niente() == false );
                        CHECK( pWedge->is_crescendo() == true );
                        CHECK( pWedge->get_wedge_number() == 1 );
                        CHECK( !is_different(pWedge->get_color(), Color(0,0,0)) );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_wedge_03)
    {
        //@03. (stop, diminuendo) is valid

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction>"
                "<direction-type><wedge type='stop'/></direction-type>"
            "</direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction>"
                "<direction-type><wedge type='diminuendo'/></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );
//                cout << test_name() << endl;
//                cout << "num.children=" << pMD->get_num_children() << endl;
//                while (it != pMD->end())
//                {
//                    cout << (*it)->to_string() << ", " << (*it)->get_name() << endl;
//                    ++it;
//                }

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 0 );
                    CHECK( pDir->get_placement() == k_placement_default );

                    ImoWedge* pWedge = dynamic_cast<ImoWedge*>(
                                           pDir->find_relation(k_imo_wedge) );
                    CHECK( pWedge != nullptr );
                    if (pWedge)
                    {
                        CHECK( pWedge->get_start_spread() == 15.0f );
                        CHECK( pWedge->get_end_spread() == 0.0f );
                        CHECK( pWedge->is_niente() == false );
                        CHECK( pWedge->is_crescendo() == false );
                        CHECK( pWedge->get_wedge_number() == 1 );
                        CHECK( !is_different(pWedge->get_color(), Color(0,0,0)) );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_wedge_04)
    {
        //@04. niente at end
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction>"
                "<direction-type><wedge type='diminuendo' niente='yes'/></direction-type>"
            "</direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction>"
                "<direction-type><wedge type='stop'/></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );
//                cout << test_name() << endl;
//                cout << "num.children=" << pMD->get_num_children() << endl;
//                while (it != pMD->end())
//                {
//                    cout << (*it)->to_string() << ", " << (*it)->get_name() << endl;
//                    ++it;
//                }

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 0 );
                    CHECK( pDir->get_placement() == k_placement_default );

                    ImoWedge* pWedge = dynamic_cast<ImoWedge*>(
                                           pDir->find_relation(k_imo_wedge) );
                    CHECK( pWedge != nullptr );
                    if (pWedge)
                    {
                        CHECK( pWedge->get_start_spread() == 15.0f );
                        CHECK( pWedge->get_end_spread() == 0.0f );
                        CHECK( pWedge->is_niente() == true );
                        CHECK( pWedge->is_crescendo() == false );
                        CHECK( pWedge->get_wedge_number() == 1 );
                        CHECK( !is_different(pWedge->get_color(), Color(0,0,0)) );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_wedge_05)
    {
        //@05. niente at start
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction>"
                "<direction-type><wedge type='crescendo' niente='yes'/></direction-type>"
            "</direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction>"
                "<direction-type><wedge type='stop'/></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 4 );
//                cout << test_name() << endl;
//                cout << "num.children=" << pMD->get_num_children() << endl;
//                while (it != pMD->end())
//                {
//                    cout << (*it)->to_string() << ", " << (*it)->get_name() << endl;
//                    ++it;
//                }

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 0 );
                    CHECK( pDir->get_placement() == k_placement_default );

                    ImoWedge* pWedge = dynamic_cast<ImoWedge*>(
                                           pDir->find_relation(k_imo_wedge) );
                    CHECK( pWedge != nullptr );
                    if (pWedge)
                    {
                        CHECK( pWedge->get_start_spread() == 0.0f );
                        CHECK( pWedge->get_end_spread() == 15.0f );
                        CHECK( pWedge->is_niente() == true );
                        CHECK( pWedge->is_crescendo() == true );
                        CHECK( pWedge->get_wedge_number() == 1 );
                        CHECK( !is_different(pWedge->get_color(), Color(0,0,0)) );
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }


    //@ miscellaneous -------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, mxl_analyser_90001)
    {
        //@90001 Hello World
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            CHECK( pDoc->get_num_content_items() == 1 );
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                CHECK( pScore->get_num_instruments() == 1 );
                ImoInstrument* pInstr = pScore->get_instrument(0);
                CHECK( pInstr != nullptr );
                CHECK( pInstr->get_num_staves() == 1 );
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 5 );
                ImoObj* pImo = pMD->get_first_child();
                CHECK( pImo->is_clef() == true );
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, mxl_analyser_90010)
    {
        //@90010 malformed MusicXML: fix wrong beam

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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );

                ImoObj::children_iterator it = pMD->begin();
                CHECK( pMD->get_num_children() == 3 );          //#3 is barline

                ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
                CHECK( pNote1 != nullptr );
                if (pNote1)
                {
                    ++it;
                    ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
                    CHECK( pNote2 != nullptr );
                    if (pNote2)
                    {
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
                    }
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

}

