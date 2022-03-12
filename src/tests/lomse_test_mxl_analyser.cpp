//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

//---------------------------------------------------------------------------------------
// helper macros
#define CHECK_MD_OBJECT(it, _object) \
            CHECK( (*it)->to_string() == _object );   \
            ++it;


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

    std::map<int, long>& my_get_voice_times() { return m_timeKeeper.dbg_get_voice_times(); }
    long my_get_timepos_for_voice(int voice) { return m_timeKeeper.get_timepos_for_voice(voice); }
    size_t my_get_num_voices() { return my_get_voice_times().size(); }

};

//---------------------------------------------------------------------------------------
class MxlAnalyserTestFixture
{
public:
    LibraryScope m_libraryScope;
    int m_requestType;
    bool m_fRequestReceived;
    ImoDocument* m_pDoc;
    std::string m_scores_path;

    MxlAnalyserTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_requestType(k_null_request)
        , m_fRequestReceived(false)
        , m_pDoc(nullptr)
        , m_scores_path(TESTLIB_SCORES_PATH)
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

    void dump_timepos_for_voices(MyMxlAnalyser& a)
    {
        std::map<int, long>& voices = a.my_get_voice_times();
        for (map<int, long>::iterator it=voices.begin(); it!=voices.end(); ++it)
        {
            cout << test_name() << ", voice=" << it->first << ", time=" << it->second << endl;
        }
    }

    void dump_music_data(ImoMusicData* pMD)
    {
        TreeNode<ImoObj>::children_iterator it = pMD->begin();
        int numObjs = pMD->get_num_children();
        for (int i=0; i < numObjs; ++i, ++it)
        {
            cout << test_name() << " i=," << i << ", " << (*it)->to_string() << endl;
        }
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
        CHECK( pScore && pScore->get_version_number() == 200 );
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
        CHECK( pInstr->get_name().text == "Guitar" );
//        cout << "Name: '" << pInstr->get_name().text
//             << "', Abbrev: '" << pInstr->get_abbrev().text << "'" << endl;
        CHECK( pInstr->get_abbrev().text == "" );
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
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<divisions>7</divisions>"
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
        CHECK( pRoot );
        CHECK( a.current_divisions() == 7L );

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
        CHECK( a.current_divisions() == 1L );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_04)
    {
        //@04 setting number of staves
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
                "<staves>3</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_staves() == 3 );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_attributes_05)
    {
        //@05 repeatedly setting <staves> value in the mid-score <attributes> should not lead to adding extra staves
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
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "<measure number='2'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_staves() == 2 );

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
        expected << "Line 0. Error: <clef-octave-change> only supported for up to two octaves. Ignored."
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
        expected << "Line 0. Part '', measure ''. Error: G clef only supported in lines 1 or 2. Line changed to 2."
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
        expected << "Line 0. Part '', measure ''. Error: F clef only supported in lines 3, 4 or 5. Line changed to 4."
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
        expected << "Line 0. Part '', measure ''. Error: C clef only supported in lines 1 to 5. Line changed to 1."
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


    //@ defaults --------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_01)
    {
        //@01. defaults: scaling parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<scaling>"
                  "<millimeters>6.35</millimeters>"
                  "<tenths>40</tenths>"
                "</scaling>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore );
        CHECK( is_equal_float( pScore->tenths_to_logical(40.0f), 635.0f) );
//        cout << test_name() << ". LUnits=" << pScore->tenths_to_logical(40.0f) << endl;

        //page has default values (in LUnits)
        ImoPageInfo* pInfo = pDoc->get_page_info();
        CHECK( is_equal_float(pInfo->get_left_margin_odd(), 1500.0f) );
        CHECK( is_equal_float(pInfo->get_right_margin_odd(), 1500.0f) );
        CHECK( is_equal_float(pInfo->get_top_margin_odd(), 2000.0f) );
        CHECK( is_equal_float(pInfo->get_bottom_margin_odd(), 2000.0f) );
        CHECK( is_equal_float(pInfo->get_left_margin_even(), 1500.0f) );
        CHECK( is_equal_float(pInfo->get_right_margin_even(), 1500.0f) );
        CHECK( is_equal_float(pInfo->get_top_margin_even(), 2000.0f) );
        CHECK( is_equal_float(pInfo->get_bottom_margin_even(), 2000.0f) );
        CHECK( is_equal_float(pInfo->get_page_width(), 21000.0f) ); //DIN A4 (210.0 x 297.0 mm)
        CHECK( is_equal_float(pInfo->get_page_height(), 29700.0f) );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_02)
    {
        //@02. defaults: page-layout parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<page-layout>"
                  "<page-height>1760</page-height>"
                  "<page-width>1360</page-width>"
                  "<page-margins type='both'>"
                    "<left-margin>80</left-margin>"
                    "<right-margin>100</right-margin>"
                    "<top-margin>60</top-margin>"
                    "<bottom-margin>120</bottom-margin>"
                  "</page-margins>"
                "</page-layout>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore );
        CHECK( is_equal_float( pScore->tenths_to_logical(40.0f), 720.0f) );
//        cout << test_name() << ". LUnits=" << pScore->tenths_to_logical(40.0f) << endl;

        ImoPageInfo* pInfo = pDoc->get_page_info();
        CHECK( is_equal_float(pInfo->get_left_margin_odd(), 1440.0f) );
        CHECK( is_equal_float(pInfo->get_right_margin_odd(), 1800.0f) );
        CHECK( is_equal_float(pInfo->get_top_margin_odd(), 1080.0f) );
        CHECK( is_equal_float(pInfo->get_bottom_margin_odd(), 2160.0f) );
        CHECK( is_equal_float(pInfo->get_left_margin_even(), 1440.0f) );
        CHECK( is_equal_float(pInfo->get_right_margin_even(), 1800.0f) );
        CHECK( is_equal_float(pInfo->get_top_margin_even(), 1080.0f) );
        CHECK( is_equal_float(pInfo->get_bottom_margin_even(), 2160.0f) );
        CHECK( is_equal_float(pInfo->get_page_width(), 24480.0f) );
        CHECK( is_equal_float(pInfo->get_page_height(), 31680.0f) );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_03)
    {
        //@03. defaults: system-layout parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<system-layout>"
                  "<system-margins>"
                    "<left-margin>80</left-margin>"
                    "<right-margin>40</right-margin>"
                  "</system-margins>"
                  "<system-distance>120</system-distance>"
                  "<top-system-distance>160</top-system-distance>"
                "</system-layout>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore );
        CHECK( is_equal_float( pScore->tenths_to_logical(40.0f), 720.0f) );
//        cout << test_name() << ". LUnits=" << pScore->tenths_to_logical(40.0f) << endl;

        ImoSystemInfo* pInfo = pScore->get_first_system_info();
        CHECK( pInfo->is_first() == true );
        CHECK( is_equal_float(pInfo->get_left_margin(), 1440.0f) );
        CHECK( is_equal_float(pInfo->get_right_margin(), 720.0f) );
        CHECK( is_equal_float(pInfo->get_system_distance(), 2160.0f) );
        CHECK( is_equal_float(pInfo->get_top_system_distance(), 2880.0f) );
        pInfo = pScore->get_other_system_info();
        CHECK( pInfo->is_first() == false );
        CHECK( is_equal_float(pInfo->get_left_margin(), 1440.0f) );
        CHECK( is_equal_float(pInfo->get_right_margin(), 720.0f) );
        CHECK( is_equal_float(pInfo->get_system_distance(), 2160.0f) );
        CHECK( is_equal_float(pInfo->get_top_system_distance(), 2880.0f) );

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_04)
    {
        //@04. defaults: staff-layout parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<staff-layout>"
                  "<staff-distance>80</staff-distance>"
                "</staff-layout>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        if (pScore)
        {
            CHECK( pScore->get_num_instruments() == 1 );
            ImoInstrument* pInstr = pScore->get_instrument(0);
            CHECK( pInstr != nullptr );
            if (pInstr)
            {
                ImoStaffInfo* pInfo = pInstr->get_staff(0);
                CHECK( is_equal_float( pInfo->get_staff_margin(), 1440.0f) );
                pInfo = pInstr->get_staff(1);
                CHECK( is_equal_float( pInfo->get_staff_margin(), 1000.0f) );
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_05)
    {
        //@05. defaults: staff-layout transferred when more staves added

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<staff-layout number='1'>"
                  "<staff-distance>80</staff-distance>"
                "</staff-layout>"
                "<staff-layout number='2'>"
                  "<staff-distance>120</staff-distance>"
                "</staff-layout>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        if (pScore)
        {
            CHECK( pScore->get_num_instruments() == 1 );
            ImoInstrument* pInstr = pScore->get_instrument(0);
            CHECK( pInstr != nullptr );
            if (pInstr)
            {
                ImoStaffInfo* pInfo = pInstr->get_staff(0);
                CHECK( is_equal_float( pInfo->get_staff_margin(), 1440.0f) );
                pInfo = pInstr->get_staff(1);
                CHECK( is_equal_float( pInfo->get_staff_margin(), 2160.0f) );
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_06)
    {
        //@06. defaults: word-font and music-font parsed and transferred

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<music-font font-family='Maestro,engraved' font-size='20.5'/>"
                "<word-font font-family='Times New Roman' font-size='9'/>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ImoStyle* pStyle = pScore->get_default_style();
            CHECK( pStyle->font_name() == "Times New Roman" );
            CHECK( is_equal_float(pStyle->font_size(), 9.0f) );
//            cout << test_name() << ", name='" << pStyle->font_name()
//                << "', size=" << pStyle->font_size() << endl;

            ImoFontStyleDto* pFont = a.get_music_font();
            CHECK( pFont );
            if (pFont)
            {
                CHECK( pFont->name == "Maestro,engraved" );
                CHECK( is_equal_float(pFont->size, 20.5f) );
//                cout << test_name() << ", name='" << pFont->name
//                    << "', size=" << pFont->size << endl;
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_07)
    {
        //@07. defaults: lyric-font parsed and transferred. No: number, language

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<lyric-font font-family='ＭＳ ゴシック' font-size='10.25'/>"
                "<lyric-language xml:lang='ja'/>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ImoStyle* pStyle = pScore->find_style("Lyrics");
            CHECK( pStyle->font_name() == "ＭＳ ゴシック" );
            CHECK( is_equal_float(pStyle->font_size(), 10.25f) );
//            cout << test_name() << ", name='" << pStyle->font_name()
//                << "', size=" << pStyle->font_size() << endl;
        }

        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_defaults_08)
    {
        //@08. defaults: lyric-font for two lines

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<lyric-font number='1' font-family='Book Antiqua' font-size='10'/>"
                "<lyric-font number='2' font-family='ＭＳ ゴシック' font-size='10.25'/>"
                "<lyric-language number='1' xml:lang='en'/>"
                "<lyric-language number='2' xml:lang='ja'/>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<staves>2</staves>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ImoStyle* pStyle = pScore->find_style("Lyric-1");
            CHECK( pStyle );
            if (pStyle)
            {
                CHECK( pStyle->font_name() == "Book Antiqua" );
                CHECK( is_equal_float(pStyle->font_size(), 10.0f) );
//                cout << test_name() << ", name='" << pStyle->font_name()
//                    << "', size=" << pStyle->font_size() << endl;
            }
            CHECK( pStyle );
            pStyle = pScore->find_style("Lyric-2");
            if (pStyle)
            {
                CHECK( pStyle->font_name() == "ＭＳ ゴシック" );
                CHECK( is_equal_float(pStyle->font_size(), 10.25f) );
//                cout << test_name() << ", name='" << pStyle->font_name()
//                    << "', size=" << pStyle->font_size() << endl;
            }

            //languages
            CHECK( a.get_lyric_language(1) == "en" );
            CHECK( a.get_lyric_language(2) == "ja" );
        }

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


    //@ fingering -----------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_fingering_01)
    {
        //@01. minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<technical><fingering>3</fingering></technical>"
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
        if (pNote)
        {
            CHECK( pNote->get_num_attachments() == 1 );
            ImoFingering* pFing = dynamic_cast<ImoFingering*>( pNote->get_attachment(0) );
            CHECK( pFing != nullptr );
            if (pFing)
            {
                CHECK( pFing->num_fingerings() == 1 );
                const list<FingerData>& fingerings = pFing->get_fingerings();
                const FingerData& data = fingerings.front();
                CHECK( data.value == "3" );
                CHECK( data.attribs == nullptr );
                CHECK( data.flags == 0 );
                CHECK( data.is_substitution() == false );
                CHECK( data.is_alternative() == false );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_fingering_02)
    {
        //@02. empty fingering is ignored

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<technical><fingering/></technical>"
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
        CHECK( pNote && pNote->get_num_attachments() == 0 );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_fingering_03)
    {
        //@03. substitution fingering

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations><technical>"
                    "<fingering>5</fingering>"
                    "<fingering substitution='yes'>3</fingering>"
                "</technical></notations>"
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
        if (pNote)
        {
            CHECK( pNote->get_num_attachments() == 1 );
            ImoFingering* pFing = dynamic_cast<ImoFingering*>( pNote->get_attachment(0) );
            CHECK( pFing != nullptr );
            if (pFing)
            {
                CHECK( pFing->num_fingerings() == 2 );
                list<FingerData>::const_iterator it = (pFing->get_fingerings()).begin();
                const FingerData& data1 = *it;
                CHECK( data1.value == "5" );
                CHECK( data1.attribs == nullptr );
                CHECK( data1.flags == 0 );
                ++it;
                const FingerData& data2 = *it;
                CHECK( data2.value == "3" );
                CHECK( data2.attribs == nullptr );
                CHECK( data2.is_substitution() == true );
                CHECK( data2.is_alternative() == false );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_fingering_04)
    {
        //@04. alternative fingering

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        stringstream expected;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations><technical>"
                    "<fingering>4</fingering>"
                    "<fingering alternate='yes'>5</fingering>"
                "</technical></notations>"
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
        if (pNote)
        {
            CHECK( pNote->get_num_attachments() == 1 );
            ImoFingering* pFing = dynamic_cast<ImoFingering*>( pNote->get_attachment(0) );
            CHECK( pFing != nullptr );
            if (pFing)
            {
                CHECK( pFing->num_fingerings() == 2 );
                list<FingerData>::const_iterator it = (pFing->get_fingerings()).begin();
                const FingerData& data1 = *it;
                CHECK( data1.value == "4" );
                CHECK( data1.attribs == nullptr );
                CHECK( data1.flags == 0 );
                ++it;
                const FingerData& data2 = *it;
                CHECK( data2.value == "5" );
                CHECK( data2.attribs == nullptr );
                CHECK( data2.is_substitution() == false );
                CHECK( data2.is_alternative() == true );
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
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            CHECK( pKey->get_key_type() == k_key_D );
            CHECK( pKey->get_staff() == -1 );
            CHECK( pKey->is_standard() == true );
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
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            CHECK( pKey->get_key_type() == k_key_gs );
            CHECK( pKey->get_staff() == -1 );
            CHECK( pKey->is_standard() == true );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_03)
    {
        //@03 non-standard key. Error in step
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Unknown note step 'Z'. Ignored." << endl
            << "Line 0. Part '', measure ''. Invalid step 'Z'. Key signature ignored." << endl;
        parser.parse_text("<key><key-step>Z</key-step><key-alter>-1</key-alter></key>");

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

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_04)
    {
        //@04 non-standard key. Error in accidental, key accepted
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Part '', measure ''. Invalid or not supported <accidentals> value 'round-flat'."
            << endl;
        parser.parse_text(
            "<key>"
                "<key-step>A</key-step>"
                "<key-alter>-1</key-alter>"
                "<key-accidental>round-flat</key-accidental>"
            "</key>"
        );

        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            CHECK( pKey->is_standard() == false );
            CHECK( pKey->get_staff() == -1 );
            CHECK( pKey->get_key_type() == k_key_non_standard );
            CHECK( pKey->has_accidentals() == true );

            KeyAccidental& acc = pKey->get_accidental(0);
            CHECK( acc.step == k_step_A );
            CHECK( is_equal_float(acc.alter, -1.0f) );
            CHECK( acc.accidental == k_flat );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_05)
    {
        //@05 non-standard key. Several accidentals. Accidentals computed from alter
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<key>"
                "<key-step>G</key-step>"
                "<key-alter>-1.5</key-alter>"
                "<key-step>A</key-step>"
                "<key-alter>1.5</key-alter>"
                "<key-step>B</key-step>"
                "<key-alter>-0.5</key-alter>"
            "</key>"
        );

        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            CHECK( pKey->is_standard() == false );
            CHECK( pKey->get_staff() == -1 );
            CHECK( pKey->get_key_type() == k_key_non_standard );
            CHECK( pKey->has_accidentals() == true );

            CHECK( pKey->get_octave(0) == -1 );
            CHECK( pKey->get_octave(1) == -1 );
            CHECK( pKey->get_octave(2) == -1 );
            CHECK( pKey->get_octave(3) == -1 );

            KeyAccidental& acc = pKey->get_accidental(0);
            CHECK( acc.step == k_step_G );
            CHECK( is_equal_float(acc.alter, -1.5f) );
            CHECK( acc.accidental == k_acc_three_quarters_flat );

            acc = pKey->get_accidental(1);
            CHECK( acc.step == k_step_A );
            CHECK( is_equal_float(acc.alter, 1.5f) );
            CHECK( acc.accidental == k_acc_three_quarters_sharp );

            acc = pKey->get_accidental(2);
            CHECK( acc.step == k_step_B );
            CHECK( is_equal_float(acc.alter, -0.5f) );
            CHECK( acc.accidental == k_acc_quarter_flat );

            acc = pKey->get_accidental(3);
            CHECK( acc.step == k_step_undefined );
            CHECK( is_equal_float(acc.alter, 0.0f) );
            CHECK( acc.accidental == k_no_accidentals );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_06)
    {
        //@06 non-standard key. Several accidentals with octaves
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<key>"
              "<key-step>C</key-step>"
              "<key-alter>-2</key-alter>"
              "<key-step>G</key-step>"
              "<key-alter>2</key-alter>"
              "<key-step>D</key-step>"
              "<key-alter>-1</key-alter>"
              "<key-octave number='1'>2</key-octave>"
              "<key-octave number='2'>3</key-octave>"
              "<key-octave number='3'>6</key-octave>"
            "</key>"
        );

        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            CHECK( pKey->is_standard() == false );
            CHECK( pKey->get_staff() == -1 );
            CHECK( pKey->get_key_type() == k_key_non_standard );
            CHECK( pKey->has_accidentals() == true );

            CHECK( pKey->get_octave(0) == 2 );
            CHECK( pKey->get_octave(1) == 3 );
            CHECK( pKey->get_octave(2) == 6 );
            CHECK( pKey->get_octave(3) == -1 );

            KeyAccidental& acc = pKey->get_accidental(0);
            CHECK( acc.step == k_step_C );
            CHECK( is_equal_float(acc.alter, -2.0f) );
            CHECK( acc.accidental == k_flat_flat );

            acc = pKey->get_accidental(1);
            CHECK( acc.step == k_step_G );
            CHECK( is_equal_float(acc.alter, 2.0f) );
            CHECK( acc.accidental == k_double_sharp );

            acc = pKey->get_accidental(2);
            CHECK( acc.step == k_step_D );
            CHECK( is_equal_float(acc.alter, -1.0f) );
            CHECK( acc.accidental == k_flat );

            acc = pKey->get_accidental(3);
            CHECK( acc.step == k_step_undefined );
            CHECK( is_equal_float(acc.alter, 0.0f) );
            CHECK( acc.accidental == k_no_accidentals );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_key_10)
    {
        //@10 standard key, with key-octave

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<key number='1'>"
                "<fifths>3</fifths>"
                "<key-octave number='1'>4</key-octave>"
                "<key-octave number='3'>4</key-octave>"
            "</key>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            CHECK( pKey->get_key_type() == k_key_A );
            CHECK( pKey->get_staff() == 0 );
            CHECK( pKey->is_standard() == true );
            CHECK( pKey->get_octave(0) == 4 );
            CHECK( pKey->get_octave(1) == -1 );
            CHECK( pKey->get_octave(2) == 4 );
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

    TEST_FIXTURE(MxlAnalyserTestFixture, measure_04)
    {
        //@04. Left and right barlines handling

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
                "<barline location='left'>"
                    "<bar-style>heavy-light</bar-style>"
                    "<repeat direction='forward'/>"
                "</barline>"
                "<note><pitch><step>A</step><octave>3</octave></pitch>"
                    "<duration>4</duration><type>whole</type></note>"
                "<barline location='right'>"
                    "<bar-style>light-heavy</bar-style>"
                    "<repeat direction='backward'/>"
                "</barline>"
            "</measure>"
            "<measure number='2'>"
                "<barline location='left'>"
                    "<bar-style>heavy-light</bar-style>"
                    "<repeat direction='forward'/>"
                "</barline>"
                "<note><pitch><step>A</step><octave>3</octave></pitch>"
                    "<duration>4</duration><type>whole</type></note>"
                "<barline location='right'>"
                    "<bar-style>light-heavy</bar-style>"
                    "<repeat direction='backward'/>"
                "</barline>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );

        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );
        CHECK( pMD->get_num_children() == 5 );

        ImoObj::children_iterator it = pMD->begin(); //measure 1: the first start repeat barline
        CHECK( (*it)->is_barline() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline->get_type() == k_barline_start_repetition );

        ++it; //measure 1: note
        CHECK( (*it)->is_note() );

        ++it; //measure 1-2: a combined repeat barline
        CHECK( (*it)->is_barline() );
        pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline->get_type() == k_barline_double_repetition );

        ++it; //measure 2: note
        CHECK( (*it)->is_note() );

        ++it; //measure 2: end repeat barline
        CHECK( (*it)->is_barline() );
        pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline->get_type() == k_barline_end_repetition );

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
        CHECK( pInstr->get_name().text == "Flute 1" );
        CHECK( pInstr->get_abbrev().text == "Fl. 1" );
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
//        cout << "instr.name= " << pInstr->get_name().text << endl
//             << "instr.abbrev= " << pInstr->get_abbrev().text << endl
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
        CHECK( pNote && pNote->is_unpitched() == true );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_undefined );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_note_12)
    {
        //@12 note updates time counter
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
            "<part id=\"P1\"><measure number=\"1\">"
            "<note><pitch><step>B</step><alter>2</alter><octave>2</octave></pitch>"
            "<duration>3</duration><type>half</type></note>"
            "</measure></part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

       //AWARE: initialy <divisions>==1
        CHECK( is_equal_time(a.get_current_time(), 3.0f*k_duration_quarter) );

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
//            dump_music_data(pMD);

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

    //@ pedal --------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, pedal_01)
    {
        //@01. pedal line with pedal marks

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
                "<pedal type='start' line='yes' sign='yes'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction><direction-type>"
                "<pedal type='stop' line='yes' sign='yes'/>"
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
                CHECK( pMD->get_num_children() == 4 );

                ImoPedalLine* pActivePedalLine = nullptr;
                ImoObj::children_iterator it;

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 1 );
                    ImoPedalMark* pPedalMark = dynamic_cast<ImoPedalMark*>( pDir->get_attachment(0) );
                    CHECK( pPedalMark != nullptr );
                    if (pPedalMark)
                    {
                        CHECK( pPedalMark->get_type() == k_pedal_mark_start );
                        CHECK( pPedalMark->is_abbreviated() == false );
                    }

                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine != nullptr );
                    if (pPedalLine)
                    {
                        CHECK( pPedalLine->get_draw_start_corner() == true );
                        CHECK( pPedalLine->get_draw_end_corner() == true );
                        CHECK( pPedalLine->get_draw_continuation_text() == true );
                        CHECK( pPedalLine->is_sostenuto() == false );
                        pActivePedalLine = pPedalLine;
                    }
                }

                ++it;
                CHECK( (*it)->is_note() );

                ++it;
                pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 1 );
                    ImoPedalMark* pPedalMark = dynamic_cast<ImoPedalMark*>( pDir->get_attachment(0) );
                    CHECK( pPedalMark != nullptr );
                    if (pPedalMark)
                    {
                        CHECK( pPedalMark->get_type() == k_pedal_mark_stop );
                        CHECK( pPedalMark->is_abbreviated() == false );
                    }

                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine == pActivePedalLine );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, pedal_02)
    {
        //@02. standalone pedal mark

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
                "<pedal type='start' line='no' sign='yes'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
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
            CHECK( pScore != nullptr );
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_children() == 3 );

                ImoObj::children_iterator it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 1 );
                    ImoPedalMark* pPedalMark = dynamic_cast<ImoPedalMark*>( pDir->get_attachment(0) );
                    CHECK( pPedalMark != nullptr );
                    if (pPedalMark)
                    {
                        CHECK( pPedalMark->get_type() == k_pedal_mark_start );
                        CHECK( pPedalMark->is_abbreviated() == false );
                    }

                    //this is a standalone pedal mark, no pedal line should be created
                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine == nullptr );
                }

                ++it;
                CHECK( (*it)->is_note() );
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, pedal_03)
    {
        //@03. pedal line without pedal marks

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
                "<pedal type='start' line='yes' sign='no'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction><direction-type>"
                "<pedal type='stop' line='yes' sign='no'/>"
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
                CHECK( pMD->get_num_children() == 4 );

                ImoPedalLine* pActivePedalLine = nullptr;
                ImoObj::children_iterator it;

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    //no pedal marks should be created
                    CHECK( pDir->get_num_attachments() == 0 );

                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine != nullptr );
                    if (pPedalLine)
                    {
                        CHECK( pPedalLine->get_draw_start_corner() == true );
                        CHECK( pPedalLine->get_draw_end_corner() == true );
                        CHECK( pPedalLine->get_draw_continuation_text() == false ); //disabled as pedal marks are not used here
                        CHECK( pPedalLine->is_sostenuto() == false );
                        pActivePedalLine = pPedalLine;
                    }
                }

                ++it;
                CHECK( (*it)->is_note() );

                ++it;
                pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    //no pedal marks should be created
                    CHECK( pDir->get_num_attachments() == 0 );

                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine == pActivePedalLine );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, pedal_04)
    {
        //@04. pedal line types: pedal changes, resume and discontinue pedal types

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
                "<pedal type='resume' line='yes'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction><direction-type>"
                "<pedal type='change' line='yes'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction><direction-type>"
                "<pedal type='discontinue' line='yes'/>"
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
                CHECK( pMD->get_num_children() == 6 );

                ImoPedalLine* pActivePedalLine = nullptr;
                ImoObj::children_iterator it;

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine != nullptr );
                    if (pPedalLine)
                    {
                        CHECK( pPedalLine->get_start_object() == pDir );
                        //check pedal properties
                        CHECK( pPedalLine->get_draw_start_corner() == false ); //pedal start type is 'resume'
                        CHECK( pPedalLine->get_draw_end_corner() == false ); //pedal end type is 'discontinue'
                        CHECK( pPedalLine->get_draw_continuation_text() == false ); //disabled as pedal marks are not used here
                        CHECK( pPedalLine->is_sostenuto() == false );
                        pActivePedalLine = pPedalLine;
                    }
                }

                ++it;
                CHECK( (*it)->is_note() );

                ++it;
                pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine == pActivePedalLine );
                    //this is a pedal change, this ImoDirection is a middle object for the relation
                    CHECK( pPedalLine->get_start_object() != pDir );
                    CHECK( pPedalLine->get_end_object() != pDir );
                }

                ++it;
                CHECK( (*it)->is_note() );

                ++it;
                pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine == pActivePedalLine );
                    CHECK( pPedalLine->get_end_object() == pDir );
                }
            }
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, pedal_05)
    {
        //@05. other pedal types and properties: sostetuto, abbreviated pedal marks

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
                "<pedal type='sostenuto' line='yes' sign='yes' abbreviated='yes'/>"
            "</direction-type></direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction><direction-type>"
                "<pedal type='stop' line='yes' sign='yes'/>"
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
                CHECK( pMD->get_num_children() == 4 );

                ImoPedalLine* pActivePedalLine = nullptr;
                ImoObj::children_iterator it;

                it = pMD->begin();
                ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 1 );
                    ImoPedalMark* pPedalMark = dynamic_cast<ImoPedalMark*>( pDir->get_attachment(0) );
                    CHECK( pPedalMark != nullptr );
                    if (pPedalMark)
                    {
                        CHECK( pPedalMark->get_type() == k_pedal_mark_sostenuto_start );
                        CHECK( pPedalMark->is_abbreviated() == true );
                    }

                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine != nullptr );
                    if (pPedalLine)
                    {
                        CHECK( pPedalLine->get_draw_start_corner() == true );
                        CHECK( pPedalLine->get_draw_end_corner() == true );
                        CHECK( pPedalLine->get_draw_continuation_text() == true );
                        CHECK( pPedalLine->is_sostenuto() == true );
                        pActivePedalLine = pPedalLine;
                    }
                }

                ++it;
                CHECK( (*it)->is_note() );

                ++it;
                pDir = dynamic_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                if (pDir)
                {
                    CHECK( pDir->get_num_attachments() == 1 );
                    ImoPedalMark* pPedalMark = dynamic_cast<ImoPedalMark*>( pDir->get_attachment(0) );
                    CHECK( pPedalMark != nullptr );
                    if (pPedalMark)
                    {
                        CHECK( pPedalMark->get_type() == k_pedal_mark_stop );
                        CHECK( pPedalMark->is_abbreviated() == false );
                    }

                    ImoPedalLine* pPedalLine = dynamic_cast<ImoPedalLine*>( pDir->find_relation(k_imo_pedal_line) );
                    CHECK( pPedalLine == pActivePedalLine );
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

//                dump_music_data(pMD);
                CHECK( pMD->get_num_children() == 9 );
                if (pMD->get_num_children() == 9)
                {
                    ImoObj::children_iterator it = pMD->begin();
                    CHECK_MD_OBJECT(it, "(clef G p1)" );
                    CHECK_MD_OBJECT(it, "(clef F4 p2)" );
                    CHECK_MD_OBJECT(it, "(n c5 q v1 p1 (stem up))" );
                    CHECK_MD_OBJECT(it, "(n c5 q v1 p1 (stem up))" );
                    CHECK_MD_OBJECT(it, "(goFwd q v2 p1)" );
                    ImoObj::children_iterator itN2 = it;
                    CHECK_MD_OBJECT(it, "(n f4 q v2 p1 (stem down)(slur 1 stop))" );
                    ImoObj::children_iterator itN1 = it;
                    CHECK_MD_OBJECT(it, "(n d3 q v3 p2 (stem up)(slur 1 start))" );
                    CHECK_MD_OBJECT(it, "(n d4 q v3 p2 (stem down))" );
                    CHECK_MD_OBJECT(it, "(barline simple)" );

                    //note D3. Start of slur
                    ImoNote* pNoteStart = dynamic_cast<ImoNote*>( *itN1 );
                    CHECK( pNoteStart != nullptr );
                    if (pNoteStart)
                    {
                        CHECK( pNoteStart->get_num_relations() == 1 );
                        if (pNoteStart->get_num_relations() > 0)
                        {
                            ImoRelations* pRelObjs = pNoteStart->get_relations();
                            CHECK( pRelObjs->get_num_items() == 1);
                            ImoRelObj* pRO = pRelObjs->get_item(0);
                            if (pRO)
                            {
                                CHECK( pRO->is_slur() );
                                ImoNote* pN2 = (static_cast<ImoSlur*>(pRO))->get_start_note();
                                CHECK( pN2 == pNoteStart );
                            }
                        }
                    }
                    //note F4. End of slur
                    ImoNote* pNoteEnd = dynamic_cast<ImoNote*>( *itN2 );
                    CHECK( pNoteEnd != nullptr );
                    if (pNoteEnd)
                    {
                        CHECK( pNoteEnd->get_num_relations() == 1 );
                        if (pNoteEnd->get_num_relations() > 0)
                        {
                            ImoRelations* pRelObjs = pNoteEnd->get_relations();
                            CHECK( pRelObjs->get_num_items() == 1);
                            ImoRelObj* pRO = pRelObjs->get_item(0);
                            if (pRO)
                            {
                                CHECK( pRO->is_slur() );
                                ImoNote* pN1 = (static_cast<ImoSlur*>(pRO))->get_start_note();
                                ImoNote* pN2 = (static_cast<ImoSlur*>(pRO))->get_end_note();
                                CHECK( pN1 == pNoteStart );
                                CHECK( pN2 == pNoteEnd );
                            }
                        }
                    }
                }
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
        CHECK( pSC && pSC->get_attribute(k_attr_tocoda) != nullptr  );

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
                CHECK( pSC && pSC->get_attribute(k_attr_tempo) != nullptr  );

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


    //@ staff-details -------------------------------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_staff_details_01)
    {
        //@01. minimum content parsed ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<staff-details><staff-lines>1</staff-lines></staff-details>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot && pRoot->is_staff_info() == true );
        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        if (pInfo)
        {
            CHECK( pInfo->get_staff_number() == 0 );
            CHECK( pInfo->get_num_lines() == 1 );
            CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_regular );
            CHECK( is_equal_float(pInfo->get_line_spacing(), LOMSE_STAFF_LINE_SPACING) );
            CHECK( is_equal_float(pInfo->get_line_thickness(), LOMSE_STAFF_LINE_THICKNESS) );
            CHECK( is_equal_float(pInfo->get_staff_margin(), LOMSE_STAFF_TOP_MARGIN) );
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
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
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
                        CHECK( pWedge->get_end_object() == pDir ); //the first ImoDirection is the end of this edge
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


    //@ conversion to IM without goBackFwd ----------------------------------------------

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_01)
    {
        //@01 backup parsed. It updates time counter but negative values not possible
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text("<backup><duration>18</duration></backup>");
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr);
        //AWARE: initialy <divisions>==1
        CHECK( is_equal_time(a.get_current_time(), 0.0f) );

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_02)
    {
        //@02 notes update voices time. Consecutive notes
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>A</step><octave>5</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<note><pitch><step>F</step><octave>5</octave></pitch>"
                    "<duration>3</duration><voice>1</voice><type>eighth</type><dot/>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 1 );
        //AWARE: initialy <divisions>==1
        CHECK( a.my_get_timepos_for_voice(1) == 7L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 3 );
        if (pMD->get_num_children() == 3)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(n a5 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f5 e. v1 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_03)
    {
        //@03 notes update voices time. No <voice> elements
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>A</step><octave>5</octave></pitch>"
                    "<duration>4</duration><type>quarter</type>"
                "</note>"
                "<note><pitch><step>F</step><octave>5</octave></pitch>"
                    "<duration>3</duration><type>eighth</type><dot/>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        CHECK( a.my_get_num_voices() == 1 );
        CHECK( a.my_get_timepos_for_voice(1) == 7L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 3 );
        if (pMD->get_num_children() == 3)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(n a5 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f5 e. v1 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_04)
    {
        //@04 notes in chord do not increment timepos
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>A</step><octave>5</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<note><chord/><pitch><step>C</step><octave>5</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 1 );
        CHECK( a.my_get_timepos_for_voice(1) == 4L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 3 );
        if (pMD->get_num_children() == 3)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(chord (n a5 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n c5 q v1 p1))" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_05)
    {
        //@05. backup to middle replaced by goFwd
        //(n c5 q v1)(n b4 e. v1)(n a4 s v1) (back q) (n f4 q v2)
        //-> (n c5 q v1)(n b4 e. v1)(n a4 s v1) (goFwd q v2) (n f4 q v2)

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>C</step><octave>5</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<note><pitch><step>B</step><octave>4</octave></pitch>"
                    "<duration>3</duration><voice>1</voice><type>eighth</type><dot/>"
                "</note>"
                "<note><pitch><step>A</step><octave>4</octave></pitch>"
                    "<duration>1</duration><voice>1</voice><type>16th</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>2</voice><type>quarter</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        CHECK( a.my_get_num_voices() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 8L );
        CHECK( a.my_get_timepos_for_voice(2) == 8L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 6 );
        if (pMD->get_num_children() == 6)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(n c5 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n b4 e. v1 p1)" );
            CHECK_MD_OBJECT(it, "(n a4 s v1 p1)" );
            CHECK_MD_OBJECT(it, "(goFwd q v2 p1)" );
            CHECK_MD_OBJECT(it, "(n f4 q v2 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_06)
    {
        //@06. backup to start does not create goFwd
        //(n b4 e. v1)(n a4 s v1) (back q) (n f4 q v2)
        //-> (n b4 e. v1)(n a4 s v1) (n f4 q v2)

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>B</step><octave>4</octave></pitch>"
                    "<duration>3</duration><voice>1</voice><type>eighth</type><dot/>"
                "</note>"
                "<note><pitch><step>A</step><octave>4</octave></pitch>"
                    "<duration>1</duration><voice>1</voice><type>16th</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>2</voice><type>quarter</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 4L );
        CHECK( a.my_get_timepos_for_voice(2) == 4L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 4 );
        if (pMD->get_num_children() == 4)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(n b4 e. v1 p1)" );
            CHECK_MD_OBJECT(it, "(n a4 s v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f4 q v2 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_07)
    {
        //@07. backup to last note in same voice does not create goFwd
        //(n b4 q v1)(back q)(n f4 q v2)(n c4 q v1) (back q) (n g4 q v2)
        //-> (n b4 q v1)(n f4 q v2)(n c4 q v1) (n g4 q v2)

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>B</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>2</voice><type>quarter</type>"
                "</note>"
                "<note><pitch><step>C</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>G</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>2</voice><type>quarter</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 8L );
        CHECK( a.my_get_timepos_for_voice(2) == 8L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 5 );
        if (pMD->get_num_children() == 5)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(n b4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f4 q v2 p1)" );
            CHECK_MD_OBJECT(it, "(n c4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n g4 q v2 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_08)
    {
        //@08. backup after last note in same voice creates goFwd
        //(n b4 q v1)(back q)(n f4 e v2)(n c4 q v1) (back q) (n g4 e v2)
        //-> (n b4 q v1)(n f4 e v2)(n c4 q v1) (goFwd e v2) (n g4 e v2)

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>B</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>2</duration><voice>2</voice><type>eighth</type>"
                "</note>"
                "<note><pitch><step>C</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>G</step><octave>4</octave></pitch>"
                    "<duration>2</duration><voice>2</voice><type>eighth</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 8L );
        CHECK( a.my_get_timepos_for_voice(2) == 6L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 6 );
        if (pMD->get_num_children() == 6)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(n b4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f4 e v2 p1)" );
            CHECK_MD_OBJECT(it, "(n c4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(goFwd e v2 p1)" );
            CHECK_MD_OBJECT(it, "(n g4 e v2 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_09)
    {
        //@09. backup to start and no voices assigns voices correctly
        //@    (n b4 e.)(n a4 s) (back q) (n f4 q)
        //@    -> (n b4 e. v1)(n a4 s v1) (n f4 q v2))

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>B</step><octave>4</octave></pitch>"
                    "<duration>3</duration><type>eighth</type><dot/>"
                "</note>"
                "<note><pitch><step>A</step><octave>4</octave></pitch>"
                    "<duration>1</duration><type>16th</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>4</duration><type>quarter</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 4L );
        CHECK( a.my_get_timepos_for_voice(2) == 4L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 4 );
        if (pMD->get_num_children() == 4)
        {
            ImoObj::children_iterator it = pMD->begin();
            //                  time,  scr
            CHECK_MD_OBJECT(it, "(n b4 e. v1 p1)" );
            CHECK_MD_OBJECT(it, "(n a4 s v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f4 q v2 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_10)
    {
        //@10. backup to last note. No <voice> elements. Voices assigned in order
        //@    (n b4 q) (back q) (n f4 q)(n c4 q) (back q) (n g4 q)
        //@    ->(n b4 q v1) (n f4 q v2)(n c4 q v1) (n g4 q v2)

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>B</step><octave>4</octave></pitch>"
                    "<duration>4</duration><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>4</duration><type>quarter</type>"
                "</note>"
                "<note><pitch><step>C</step><octave>4</octave></pitch>"
                    "<duration>4</duration><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>G</step><octave>4</octave></pitch>"
                    "<duration>4</duration><type>quarter</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 8L );
        CHECK( a.my_get_timepos_for_voice(2) == 8L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 5 );
        if (pMD->get_num_children() == 5)
        {
            ImoObj::children_iterator it = pMD->begin();
            CHECK_MD_OBJECT(it, "(n b4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f4 q v2 p1)" );
            CHECK_MD_OBJECT(it, "(n c4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n g4 q v2 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_11)
    {
        //@11. backup after last note creates goFwd. No <voice> in 2nd voice
        //@    (n b4 q v1)(back q)(n f4 e)(n c4 q v1)(back q)(n g4 e)
        //@    -> (n b4 q v1)(n f4 e v2)(n c4 q v1) (goFwd e v2) (n g4 e v2)

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes><divisions>4</divisions></attributes>"
                "<note><pitch><step>B</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>2</duration><type>eighth</type>"
                "</note>"
                "<note><pitch><step>C</step><octave>4</octave></pitch>"
                    "<duration>4</duration><voice>1</voice><type>quarter</type>"
                "</note>"
                "<backup><duration>4</duration></backup>"
                "<note><pitch><step>G</step><octave>4</octave></pitch>"
                    "<duration>2</duration><type>eighth</type>"
                "</note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 8L );
        CHECK( a.my_get_timepos_for_voice(2) == 6L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 6 );
        if (pMD->get_num_children() == 6)
        {
            ImoObj::children_iterator it = pMD->begin();
            CHECK_MD_OBJECT(it, "(n b4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(n f4 e v2 p1)" );
            CHECK_MD_OBJECT(it, "(n c4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(goFwd e v2 p1)" );
            CHECK_MD_OBJECT(it, "(n g4 e v2 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_12)
    {
        //@12. directions between chord notes. Preserve definition order
        //@    (n a4 q v1)(dir 1)(na f4 q v1)(dir 2)(na d4 q v1)(r q v1)(r h v1)
        //@    -> the same

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/conversion/12-directions-in-chord.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );

        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 10 );
        if (pMD->get_num_children() == 10)
        {
            ImoObj::children_iterator it = pMD->begin();
            CHECK_MD_OBJECT(it, "(clef G p1)" );
            CHECK_MD_OBJECT(it, "(time 4 4)" );
            CHECK_MD_OBJECT(it, "(chord (n a4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(dir 0 p1 (TODO:  No LdpGenerator for Imo. Name=symbol-repetition-mark))" );
            CHECK_MD_OBJECT(it, "(n f4 q v1 p1)" );
            CHECK_MD_OBJECT(it, "(dir empty)" );
            CHECK_MD_OBJECT(it, "(n d4 q v1 p1 (dyn \"p\")))" );
            CHECK_MD_OBJECT(it, "(r q v1 p1)" );
            CHECK_MD_OBJECT(it, "(r h v1 p1)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_13)
    {
        //@13. two tied chords. To fix a bug: <notations> were processed before setting
        //@    the voice and thus, start-end of ties were not properly matched

        stringstream errormsg;
        stringstream expected;
        Document doc(m_libraryScope, errormsg);
        doc.from_file(m_scores_path + "unit-tests/conversion/13-tied-chords.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( errormsg.str() == expected.str() );

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 7 );
        if (pMD->get_num_children() == 7)
        {
            ImoObj::children_iterator it = pMD->begin();
            CHECK_MD_OBJECT(it, "(clef F4 p1)" );
            CHECK_MD_OBJECT(it, "(time 3 4)" );
            CHECK_MD_OBJECT(it, "(chord (n e2 h v3 p1 (stem up)(tie 1 start))" );
            CHECK_MD_OBJECT(it, "(n e3 h v3 p1 (stem up)(tie 2 start)))" );
            CHECK_MD_OBJECT(it, "(chord (n e2 q v3 p1 (stem up)(tie 1 stop))" );
            CHECK_MD_OBJECT(it, "(n e3 q v3 p1 (stem up)(tie 2 stop)))" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_14)
    {
        //@14. words. To fix a bug: <direction> incorrectly moved after first note
        //@    of second voice

        stringstream errormsg;
        stringstream expected;
        Document doc(m_libraryScope, errormsg);
        doc.from_file(m_scores_path + "unit-tests/conversion/14-words.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( errormsg.str() == expected.str() );

//        cout << test_name() << endl;
//        ColStaffObjs* pTable = pScore->get_staffobjs_table();
//        cout << pTable->dump();

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 11 );
        if (pMD->get_num_children() == 11)
        {
            ImoObj::children_iterator it = pMD->begin();
            CHECK_MD_OBJECT(it, "(clef F4 p1)" );
            CHECK_MD_OBJECT(it, "(clef F4 p2)" );
            CHECK_MD_OBJECT(it, "(time 3 4)" );
            CHECK_MD_OBJECT(it, "(r e v1 p1)" );
            CHECK_MD_OBJECT(it, "(dir 0 p1 (text \"dolce\"))" );
            CHECK_MD_OBJECT(it, "(n e3 e v1 p1 (stem down))" );
            CHECK_MD_OBJECT(it, "(n e3 q v1 p1 (stem down))" );
            CHECK_MD_OBJECT(it, "(n c4 q v1 p1 (stem down))" );
            CHECK_MD_OBJECT(it, "(n a2 q v2 p2 (stem up))" );
            CHECK_MD_OBJECT(it, "(r h v2 p2)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }
    }

    TEST_FIXTURE(MxlAnalyserTestFixture, MxlAnalyser_conversion_15)
    {
        //@15. key change after backup. To fix a bug

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part></part-list>"
            "<part id='P1'>"
            "<measure number='1'>"
                "<attributes>"
                    "<divisions>4</divisions>"
                    "<key number='1'><fifths>0</fifths></key>"
                    "<time><beats>4</beats><beat-type>4</beat-type></time>"
                    "<staves>2</staves>"
                    "<clef number='1'><sign>G</sign><line>2</line></clef>"
                "</attributes>"
                "<note><pitch><step>F</step><octave>4</octave></pitch>"
                    "<duration>16</duration><voice>1</voice><type>whole</type>"
                    "<staff>1</staff></note>"
                "<backup><duration>16</duration></backup>"
                "<attributes>"
                    "<key number='2'><fifths>2</fifths></key>"
                    "<clef number='2'><sign>F</sign><line>4</line></clef>"
                "</attributes>"
                "<note><pitch><step>B</step><octave>2</octave></pitch>"
                    "<duration>16</duration><voice>2</voice><type>whole</type>"
                    "<staff>2</staff></note>"
            "</measure>"
            "</part></score-partwise>"
        );
        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        a.dbg_do_not_reset_voice_times();
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot && pRoot->is_document() );

        std::map<int, long>& voices = a.my_get_voice_times();
        CHECK( voices.size() == 2 );
        CHECK( a.my_get_timepos_for_voice(1) == 16L );
        CHECK( a.my_get_timepos_for_voice(2) == 16L );
//        dump_timepos_for_voices(a);

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

//        dump_music_data(pMD);
        CHECK( pMD->get_num_children() == 8 );
        if (pMD->get_num_children() == 8)
        {
            ImoObj::children_iterator it = pMD->begin();
            CHECK_MD_OBJECT(it, "(clef G p1)" );
            CHECK_MD_OBJECT(it, "(key C)" );
            CHECK_MD_OBJECT(it, "(time 4 4)" );
            CHECK_MD_OBJECT(it, "(n f4 w v1 p1)" );
            CHECK_MD_OBJECT(it, "(clef F4 p2)" );
            CHECK_MD_OBJECT(it, "(key D)" );
            CHECK_MD_OBJECT(it, "(n b2 w v2 p2)" );
            CHECK_MD_OBJECT(it, "(barline simple)" );
        }
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

