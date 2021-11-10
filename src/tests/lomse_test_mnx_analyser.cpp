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
#include "lomse_mnx_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_events.h"
#include "lomse_doorway.h"
#include "lomse_im_factory.h"
#include "lomse_time.h"
#include "lomse_import_options.h"
#include "lomse_im_attributes.h"

#include <regex>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
//Helper, to access protected members
class MyMnxAnalyser : public MnxAnalyser
{
public:
    MyMnxAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc,
                  XmlParser* parser)
        : MnxAnalyser(reporter, libraryScope, pDoc, parser)
    {
    }

    virtual ~MyMnxAnalyser() {}

    map<string, MeasuresVector*>& my_get_globals() { return m_globals; }
};

//---------------------------------------------------------------------------------------
class MnxAnalyserTestFixture
{
public:
    LibraryScope m_libraryScope;
    int m_requestType;
    bool m_fRequestReceived;
    ImoDocument* m_pDoc;
    std::string m_scores_path;

    MnxAnalyserTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_requestType(k_null_request)
        , m_fRequestReceived(false)
        , m_pDoc(nullptr)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~MnxAnalyserTestFixture()    //TearDown fixture
    {
    }

    static void wrapper_lomse_request(void* pThis, Request* pRequest)
    {
        static_cast<MnxAnalyserTestFixture*>(pThis)->on_lomse_request(pRequest);
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

};


SUITE(MnxAnalyserTest)
{

    //@ beams ---------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, beams_001)
    {
        //@001. beam, main beam

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
               "<global><measure/></global>"
               "<part>"
                  "<part-name/>"
                  "<measure barline='regular'>"
                     "<directions><clef sign='G' line='2'/></directions>"
                     "<sequence>"
                        "<beams><beam events='ev1 ev2'/></beams>"
                        "<event id='ev1' value='/8'><note pitch='C4'/></event>"
                        "<event id='ev2' value='/8'><note pitch='E4'/></event>"
                     "</sequence>"
                  "</measure>"
               "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 4 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_clef() == true );
                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote1 = static_cast<ImoNote*>( *it );
                CHECK( pNote1->get_xml_id() == "ev1" );
                CHECK( pNote1->is_beamed() == true );
                CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
                CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
                CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote2 = static_cast<ImoNote*>( *it );
                CHECK( pNote2->get_xml_id() == "ev2" );
                CHECK( pNote2->is_beamed() == true );
                CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
                CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
                CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, beams_002)
    {
        //@002. beam, secondary beam

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
               "<global><measure/></global>"
               "<part>"
                  "<part-name/>"
                  "<measure barline='regular'>"
                     "<directions><clef sign='G' line='2'/></directions>"
                     "<sequence>"
                        "<beams>"
                            "<beam events='ev1 ev2 ev3 ev4'>"
                                "<beam events='ev2 ev3'/></beam>"
                        "</beams>"
                        "<event id='ev1' value='/8'><note pitch='C4'/></event>"
                        "<event id='ev2' value='/16'><note pitch='E4'/></event>"
                        "<event id='ev3' value='/16'><note pitch='G4'/></event>"
                        "<event id='ev4' value='/8'><note pitch='C5'/></event>"
                     "</sequence>"
                  "</measure>"
               "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 6 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_clef() == true );
                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote1 = static_cast<ImoNote*>( *it );
                CHECK( pNote1->get_xml_id() == "ev1" );
                CHECK( pNote1->is_beamed() == true );
                CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
                CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
                CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote2 = static_cast<ImoNote*>( *it );
                CHECK( pNote2->get_xml_id() == "ev2" );
                CHECK( pNote2->is_beamed() == true );
                CHECK( pNote2->get_beam_type(0) == ImoBeam::k_continue );
                CHECK( pNote2->get_beam_type(1) == ImoBeam::k_begin );
                CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote3 = static_cast<ImoNote*>( *it );
                CHECK( pNote3->get_xml_id() == "ev3" );
                CHECK( pNote3->is_beamed() == true );
                CHECK( pNote3->get_beam_type(0) == ImoBeam::k_continue );
                CHECK( pNote3->get_beam_type(1) == ImoBeam::k_end );
                CHECK( pNote3->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote4 = static_cast<ImoNote*>( *it );
                CHECK( pNote4->get_xml_id() == "ev4" );
                CHECK( pNote4->is_beamed() == true );
                CHECK( pNote4->get_beam_type(0) == ImoBeam::k_end );
                CHECK( pNote4->get_beam_type(1) == ImoBeam::k_none );
                CHECK( pNote4->get_beam_type(2) == ImoBeam::k_none );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, beams_003)
    {
        //@003. beam, secondary beams, breaks

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
               "<global><measure/></global>"
               "<part>"
                  "<part-name/>"
                  "<measure barline='regular'>"
                     "<directions><clef sign='G' line='2'/></directions>"
                     "<sequence>"
                        "<beams>"
                            "<beam events='ev1 ev2 ev3 ev4'>"
                                "<beam events='ev1 ev2'/>"
                                "<beam events='ev3 ev4'/></beam>"
                        "</beams>"
                        "<event id='ev1' value='/16'><note pitch='C4'/></event>"
                        "<event id='ev2' value='/16'><note pitch='E4'/></event>"
                        "<event id='ev3' value='/16'><note pitch='G4'/></event>"
                        "<event id='ev4' value='/16'><note pitch='C5'/></event>"
                     "</sequence>"
                  "</measure>"
               "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 6 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_clef() == true );
                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote1 = static_cast<ImoNote*>( *it );
                CHECK( pNote1->get_xml_id() == "ev1" );
                CHECK( pNote1->is_beamed() == true );
                CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
                CHECK( pNote1->get_beam_type(1) == ImoBeam::k_begin );
                CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote2 = static_cast<ImoNote*>( *it );
                CHECK( pNote2->get_xml_id() == "ev2" );
                CHECK( pNote2->is_beamed() == true );
                CHECK( pNote2->get_beam_type(0) == ImoBeam::k_continue );
                CHECK( pNote2->get_beam_type(1) == ImoBeam::k_end );
                CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote3 = static_cast<ImoNote*>( *it );
                CHECK( pNote3->get_xml_id() == "ev3" );
                CHECK( pNote3->is_beamed() == true );
                CHECK( pNote3->get_beam_type(0) == ImoBeam::k_continue );
                CHECK( pNote3->get_beam_type(1) == ImoBeam::k_begin );
                CHECK( pNote3->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote4 = static_cast<ImoNote*>( *it );
                CHECK( pNote4->get_xml_id() == "ev4" );
                CHECK( pNote4->is_beamed() == true );
                CHECK( pNote4->get_beam_type(0) == ImoBeam::k_end );
                CHECK( pNote4->get_beam_type(1) == ImoBeam::k_end );
                CHECK( pNote4->get_beam_type(2) == ImoBeam::k_none );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, beams_004)
    {
        //@004. beam, hooks

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
               "<global><measure/></global>"
               "<part>"
                  "<part-name/>"
                  "<measure barline='regular'>"
                     "<directions><clef sign='G' line='2'/></directions>"
                     "<sequence>"
                        "<beams>"
                            "<beam events='ev1 ev2 ev3'>"
                                "<beam-hook direction='right' event='ev1'/>"
                                "<beam-hook direction='left' event='ev3'/>"
                            "</beam>"
                        "</beams>"
                        "<event id='ev1' value='/16'><note pitch='C4'/></event>"
                        "<event id='ev2' value='/8'><note pitch='E4'/></event>"
                        "<event id='ev3' value='/16'><note pitch='G4'/></event>"
                     "</sequence>"
                  "</measure>"
               "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 5 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_clef() == true );
                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote1 = static_cast<ImoNote*>( *it );
                CHECK( pNote1->get_xml_id() == "ev1" );
                CHECK( pNote1->is_beamed() == true );
                CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
                CHECK( pNote1->get_beam_type(1) == ImoBeam::k_forward );
                CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote2 = static_cast<ImoNote*>( *it );
                CHECK( pNote2->get_xml_id() == "ev2" );
                CHECK( pNote2->is_beamed() == true );
                CHECK( pNote2->get_beam_type(0) == ImoBeam::k_continue );
                CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
                CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

                ++it;
                CHECK( (*it)->is_note() == true );
                ImoNote* pNote3 = static_cast<ImoNote*>( *it );
                CHECK( pNote3->get_xml_id() == "ev3" );
                CHECK( pNote3->is_beamed() == true );
                CHECK( pNote3->get_beam_type(0) == ImoBeam::k_end );
                CHECK( pNote3->get_beam_type(1) == ImoBeam::k_backward );
                CHECK( pNote3->get_beam_type(2) == ImoBeam::k_none );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }


    //@ event ---------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, event_001)
    {
        //@001. event id associated to note

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
               "<global><measure/></global>"
               "<part>"
                  "<part-name/>"
                  "<measure barline=\"regular\">"
                     "<directions><clef sign=\"G\" line=\"2\"/></directions>"
                     "<sequence>"
                        "<event id=\"ev1\" value=\"/1\"><note pitch=\"C4\"/></event>"
                     "</sequence>"
                  "</measure>"
               "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 3 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_clef() == true );
                ++it;
                CHECK( (*it)->is_note() == true );
                CHECK( (*it)->get_xml_id() == "ev1" );
//                cout << "event id: '" << (*it)->get_xml_id() << "'" << endl;

//                pScore->end_of_changes();
//                cout << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, event_002)
    {
        //@002. event id associated to rest

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
               "<global><measure/></global>"
               "<part>"
                  "<part-name/>"
                  "<measure barline=\"regular\">"
                     "<directions><clef sign=\"G\" line=\"2\"/></directions>"
                     "<sequence>"
                        "<event id=\"ev1\" value=\"/1\"><rest/></event>"
                     "</sequence>"
                  "</measure>"
               "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 3 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_clef() == true );
                ++it;
                CHECK( (*it)->is_rest() == true );
                CHECK( (*it)->get_xml_id() == "ev1" );
                ImoObj* pImo = doc.get_pointer_to_imo("ev1");
                CHECK( pImo == *it );
//                cout << "event id: '" << (*it)->get_xml_id() << "'" << endl;

//                pScore->end_of_changes();
//                cout << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, event_003)
    {
        //@003. event id associated to chord base note

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
               "<global><measure/></global>"
               "<part>"
                  "<part-name/>"
                  "<measure barline=\"regular\">"
                     "<directions><clef sign=\"G\" line=\"2\"/></directions>"
                     "<sequence>"
                        "<event id=\"ev1\" value=\"/1\">"
                            "<note pitch='C4'/><note pitch='E4'/><note pitch='G4'/>"
                        "</event>"
                     "</sequence>"
                  "</measure>"
               "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 5 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it)->is_clef() == true );
                ++it;
                CHECK( (*it)->is_note() == true );
                CHECK( (*it)->get_xml_id() == "ev1" );
//                cout << "event id: '" << (*it)->get_xml_id() << "'" << endl;
                ++it;
                CHECK( (*it)->is_note() == true );
                CHECK( (*it)->get_xml_id() == "" );
                ++it;
                CHECK( (*it)->is_note() == true );
                CHECK( (*it)->get_xml_id() == "" );

//                pScore->end_of_changes();
//                cout << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }


    //@ global --------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, global_001)
    {
        //@001. Error: at least one global element is required.
        //@     Doc with empty score returned

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <mnx>: missing mandatory element <global>." << endl;
        parser.parse_text(
            "<mnx>"
                "<part>"
                "</part>"
            "</mnx>");
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( doc.is_dirty() == true );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );

//        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//        pScore->end_of_changes();
//        cout << pScore->to_string_with_ids() << endl;

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, global_002)
    {

        //@002. Error: at least one measure required in global

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <global>: missing mandatory element <measure>." << endl;
        parser.parse_text(
            "<mnx>"
                "<global></global>"
                "<part>"
                "</part>"
            "</mnx>");
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( doc.is_dirty() == true );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, global_003)
    {

        //@003. Global without parts. Only one vector of measures

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <global>: missing mandatory element <measure>." << endl;
        parser.parse_text(
            "<mnx>"
                "<global><measure/></global>"
                "<part>"
                "</part>"
            "</mnx>");
        MyMnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );

        map<string, MeasuresVector*>& globals = a.my_get_globals();
        CHECK (globals.size() == 1);
        map<string, MeasuresVector*>::iterator it;
        it=globals.begin();
        CHECK( it->first == "*$SINGLE-PART$*");

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, global_004)
    {

        //@004. Global with several parts. One vector of measures for all

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <global>: missing mandatory element <measure>." << endl;
        parser.parse_text(
            "<mnx>"
                "<global parts='P1 P2'><measure/></global>"
                "<part>"
                "</part>"
            "</mnx>");
        MyMnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );

        map<string, MeasuresVector*>& globals = a.my_get_globals();
        CHECK (globals.size() == 2);
        map<string, MeasuresVector*>::iterator it;
        it=globals.begin();
        MeasuresVector* pMeasures1 = it->second;
        CHECK( it->first == "P1");
        ++it;
        MeasuresVector* pMeasures2 = it->second;
        CHECK (it->first == "P2");
        CHECK (pMeasures1 == pMeasures2);

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, global_005)
    {

        //@005. Global measures points to measures

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. <global>: missing mandatory element <measure>." << endl;
        parser.parse_text(
            "<mnx>"
                "<global>"
                    "<measure><directions><time signature='4/4'/></directions></measure>"
                    "<measure/>"
                    "<measure><directions><time signature='2/4'/></directions></measure>"
                "</global>"
                "<part>"
                "</part>"
            "</mnx>");
        MyMnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );

        map<string, MeasuresVector*>& globals = a.my_get_globals();
        CHECK (globals.size() == 1);
        map<string, MeasuresVector*>::iterator it;
        it=globals.begin();
        MeasuresVector* pMeasures = it->second;
        CHECK (pMeasures->size() == 3);

        XmlNode node = pMeasures->at(0);
        CHECK (node.name() == "measure");
        XmlNode child = node.first_child();
        CHECK (child.name() == "directions");
        XmlNode time = child.first_child();
        CHECK (time.name() == "time");
        CHECK (time.attribute_value("signature") == "4/4");

        node = pMeasures->at(1);
        CHECK (node.name() == "measure");
        CHECK (node.value() == "");

        node = pMeasures->at(2);
        CHECK (node.name() == "measure");
        child = node.first_child();
        CHECK (child.name() == "directions");
        time = child.first_child();
        CHECK (time.name() == "time");
        CHECK (time.attribute_value("signature") == "2/4");

        delete pRoot;
    }


    //@ grace notes ---------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, grace_001)
    {
        //@001. grace created. relobj created. Defaults OK. Prev & Ppal notes identified

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
                "<global><measure>"
                    "<directions><time signature='4/4'/></directions>"
                "</measure></global>"
                "<part><part-name/>"
                    "<measure barline='regular'>"
                        "<directions><clef sign='G' line='2'/></directions>"
                        "<sequence>"
                            "<event value='/4'><note pitch='G4'/></event>"
                            "<grace><event value='/8'><note pitch='D5'/></event></grace>"
                            "<event value='/4'><note pitch='C5'/></event>"
                        "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 6 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it) && (*it)->is_time_signature() == true );

                ++it;
                CHECK( (*it) && (*it)->is_clef() == true );

                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNoteFirst = static_cast<ImoNote*>( *it );
                CHECK( pNoteFirst && pNoteFirst->is_regular_note() == true );

                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNote = static_cast<ImoNote*>( *it );
                CHECK( pNote && pNote->is_grace_note() == true );
                ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
                CHECK( pGraceRO != nullptr );

                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNotePpal = static_cast<ImoNote*>( *it );
                CHECK( pNotePpal && pNotePpal->is_regular_note() == true );

                ++it;
                CHECK( (*it) && (*it)->is_barline() );

                //check grace relationship
                if (pGraceRO)
                {
                    CHECK( pGraceRO->get_num_objects() == 1 );
                    CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
                    CHECK( pGraceRO->has_slash() == true );
                }

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_002)
//    {
//        //@002 grace relobj. Intermediate grace notes added to the group
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
//            "<note><grace slash=\"yes\"/><pitch><step>D</step><octave>5</octave></pitch>"
//                "<voice>1</voice><type>eighth</type></note>"
//            "<note><grace slash=\"no\"/><pitch><step>B</step><octave>4</octave></pitch>"
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
//        if (pDoc)
//        {
//            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//            ImoInstrument* pInstr = pScore->get_instrument(0);
//            ImoMusicData* pMD = pInstr->get_musicdata();
//            CHECK( pMD != nullptr );
//            CHECK( pMD->get_num_children() == 6 );
//
//            ImoObj::children_iterator it = pMD->begin();
//            CHECK( (*it) && (*it)->is_clef() );
//
//            ++it;
//            ImoNote* pNoteFirst = dynamic_cast<ImoNote*>( *it );
//            CHECK( pNoteFirst != nullptr );
//            CHECK( pNoteFirst && pNoteFirst->is_regular_note() == true );
//
//            ++it;
//            ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
//            CHECK( pNote != nullptr );
//            CHECK( pNote && pNote->is_grace_note() == true );
//            ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
//            CHECK( pGraceRO != nullptr );
//
//            ++it;
//            pNote = dynamic_cast<ImoNote*>( *it );
//            CHECK( pNote != nullptr );
//            CHECK( pNote && pNote->is_grace_note() == true );
//            CHECK( pNote->get_grace_relobj() == pGraceRO );
//
//            ++it;
//            ImoNote* pNotePpal = dynamic_cast<ImoNote*>( *it );
//            CHECK( pNotePpal != nullptr );
//            CHECK( pNotePpal && pNotePpal->is_regular_note() == true );
////            CHECK( pNote && pNote->is_start_of_chord() == false );
////            CHECK( pNote && pNote->is_end_of_chord() == true );
//
//            ++it;
//            CHECK( (*it) && (*it)->is_barline() );
//
//            //check grace relationship
//            CHECK( pGraceRO->get_num_objects() == 2 );
//            CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
//            CHECK( pGraceRO->has_slash() == true );
//        }
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_003)
//    {
//    	//@003. grace notes. slash attribute processed
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
//            "<note><grace slash=\"no\"/><pitch><step>D</step><octave>5</octave></pitch>"
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
//        CHECK( pGraceRO->get_num_objects() == 1 );
//        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
//        CHECK( pGraceRO->has_slash() == false );
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_004)
//    {
//    	//@004. grace notes. steal-time-previous attribute processed
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
//            "<note><grace steal-time-previous=\"20\"/><pitch><step>D</step><octave>5</octave></pitch>"
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
//        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
//        CHECK( pGraceRO->has_slash() == false );
//        CHECK( pGraceRO->get_percentage() == 0.2f );
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_005)
//    {
//    	//@005. grace notes. steal-time-following attribute processed
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
//            "<note><grace slash=\"yes\" steal-time-following=\"30\"/><pitch><step>D</step><octave>5</octave></pitch>"
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
//        CHECK( pGraceRO->get_percentage() == 0.3f );
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }
//
////    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_006)
////    {
////    	//@006. grace notes. make-time attribute processed
////        stringstream errormsg;
////        Document doc(m_libraryScope);
////        XmlParser parser;
////        stringstream expected;
////        parser.parse_text(
////            "<score-partwise><part-list><score-part id=\"P1\" /></part-list>"
////            "<part id=\"P1\"><measure number=\"1\"><attributes><divisions>2</divisions>"
////            "<clef><sign>G</sign><line>2</line></clef></attributes>"
////            "<note><pitch><step>G</step><octave>4</octave></pitch>"
////                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
////            "<note><grace make-time=\"30\"/><pitch><step>D</step><octave>5</octave></pitch>"
////                "<voice>1</voice><type>eighth</type></note>"
////            "<note><pitch><step>C</step><octave>5</octave></pitch>"
////                "<duration>2</duration><voice>1</voice><type>quarter</type></note>"
////            "</measure></part></score-partwise>"
////        );
////        MyMxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
////        XmlNode* tree = parser.get_tree_root();
////        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
////
//////        cout << test_name() << endl;
//////        cout << "[" << errormsg.str() << "]" << endl;
//////        cout << "[" << expected.str() << "]" << endl;
////        CHECK( errormsg.str() == expected.str() );
////        CHECK( pRoot != nullptr);
////        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
////        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
////        ImoInstrument* pInstr = pScore->get_instrument(0);
////        ImoMusicData* pMD = pInstr->get_musicdata();
////        ImoObj::children_iterator it = pMD->begin();
////                //it -> clef
////        ++it;   //first note
////        ++it;   //first grace note
////        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
////        CHECK( pNote != nullptr );
////        CHECK( pNote && pNote->is_grace_note() == true );
////        ImoGraceRelObj* pGraceRO = pNote->get_grace_relobj();
////        CHECK( pGraceRO != nullptr );
////
////        //check grace relationship
////        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_following );
////        CHECK( pGraceRO->has_slash() == true );
////        CHECK( pGraceRO->get_time_to_make() == 0.3f );
////
////        a.do_not_delete_instruments_in_destructor();
////        delete pRoot;
////    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_007)
//    {
//    	//@007. slash yes (acciaccaturas) are played short. default percentage 0.1
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
//            "<note><grace slash=\"yes\"/><pitch><step>D</step><octave>5</octave></pitch>"
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
//        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
//        CHECK( pGraceRO->has_slash() == true );
//        CHECK( pGraceRO->get_percentage() == 0.2f );
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }
//
//    TEST_FIXTURE(MxlAnalyserTestFixture, grace_notes_008)
//    {
//    	//@008. slash no (appoggiaturas) are played long. default percentage 0.5
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
//            "<note><grace slash=\"no\"/><pitch><step>D</step><octave>5</octave></pitch>"
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
//        CHECK( pGraceRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous );
//        CHECK( pGraceRO->has_slash() == false );
//        CHECK( pGraceRO->get_percentage() == 0.5f );
//
//        a.do_not_delete_instruments_in_destructor();
//        delete pRoot;
//    }


    //@ jump ----------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, jump_001)
    {
        //@001. jump direction created correctly

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
                "<global><measure>"
                    "<directions><time signature='4/4'/></directions>"
                "</measure><measure>"
                    "<directions><jump type='segno'/></directions>"
                "</measure></global>"
                "<part><part-name/>"
                    "<measure>"
                        "<directions><clef sign='G' line='2'/></directions>"
                        "<sequence>"
                            "<event value='/1'><note pitch='G4'/></event>"
                        "</sequence>"
                    "</measure>"
                    "<measure>"
                        "<sequence>"
                            "<event value='/1'><note pitch='E4'/></event>"
                        "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 8 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it) && (*it)->is_time_signature() == true );

                ++it;
                CHECK( (*it) && (*it)->is_clef() );

                ++it;
                CHECK( (*it) && (*it)->is_note() );

                ++it;
                CHECK( (*it) && (*it)->is_barline() );

                ++it;
                CHECK( (*it) && (*it)->is_note() );

                ++it;
                CHECK( (*it) && (*it)->is_direction() );
                ImoDirection* pDir = static_cast<ImoDirection*>( *it );
                CHECK( pDir != nullptr );
                CHECK( pDir && pDir->get_num_attachments() == 1 );
                CHECK( pDir && pDir->get_placement() == k_placement_default );
                CHECK( pDir && pDir->get_display_repeat() == k_repeat_dal_segno );
                CHECK( pDir && pDir->get_sound_repeat() == k_repeat_none );

                ImoTextRepetitionMark* pTxt = dynamic_cast<ImoTextRepetitionMark*>(
                                                pDir->find_attachment(k_imo_text_repetition_mark) );
                CHECK( pTxt != nullptr );
                CHECK( pTxt && pTxt->get_repeat_mark() == k_repeat_dal_segno );

                ++it;
                CHECK( (*it) && (*it)->is_sound_change() );
                ImoSoundChange* pSC = static_cast<ImoSoundChange*>( *it );
                CHECK( pSC != nullptr );
                CHECK( pSC && pSC->get_attribute_node(k_attr_segno) != nullptr  );

                ++it;
                CHECK( (*it) && (*it)->is_barline() );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }


    //@ measure -------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, measure_001)
    {
        //@001. MeasuresInfo in Barline but not in Instrument

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <mnx>: missing mandatory element <global>." << endl;
        parser.parse_text(
            "<mnx>"
                "<part>"
                    "<part-name>Piano</part-name>"
                    "<measure>"
                      "<directions>"
                        "<staves number='1'/>"
                        "<clef line='2' sign='G'/>"
                      "</directions>"
                      "<sequence staff='1'>"
                        "<event value='/4'><note pitch='C4'/></event>"
                        "<event value='/2'><note pitch='E4'/></event>"
                      "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>");
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
        CHECK( pInfo == nullptr );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

        CHECK( pMD && pMD->get_num_children() == 4 );
        ImoObj::children_iterator it = pMD->begin();    //clef
        CHECK( (*it)->is_clef() );
        ++it;   //note c4
        CHECK( (*it)->is_note() );
        ++it;   //note e4
        CHECK( (*it)->is_note() );
        ++it;   //barline
        CHECK( (*it)->is_barline() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_simple );
        CHECK( pBarline && pBarline->is_visible() );
        pInfo = pBarline->get_measure_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->count == 1 );
//        cout << test_name() << ": count=" << pInfo->count << endl;

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, measure_002)
    {
        //@002. Two measures

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <mnx>: missing mandatory element <global>." << endl;
        parser.parse_text(
            "<mnx>"
                "<part>"
                    "<part-name>Piano</part-name>"
                    "<measure number='1'>"
                      "<directions>"
                        "<staves number='1'/>"
                        "<clef line='2' sign='G'/>"
                      "</directions>"
                      "<sequence staff='1'>"
                        "<event value='/4'><note pitch='C4'/></event>"
                        "<event value='/2'><note pitch='E4'/></event>"
                      "</sequence>"
                    "</measure>"
                    "<measure number='2'>"
                      "<sequence staff='1'>"
                        "<event value='/4'><note pitch='C4'/></event>"
                        "<event value='/2'><note pitch='E4'/></event>"
                      "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>");
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
        CHECK( pInfo == nullptr );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

        CHECK( pMD && pMD->get_num_children() == 7 );
        ImoObj::children_iterator it = pMD->begin();    //clef
        CHECK( (*it)->is_clef() );
        ++it;   //note c4
        CHECK( (*it)->is_note() );
        ++it;   //note e4
        CHECK( (*it)->is_note() );
        ++it;   //barline
        CHECK( (*it)->is_barline() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_simple );
        CHECK( pBarline && pBarline->is_visible() );
        TypeMeasureInfo* pInfo1 = pBarline->get_measure_info();
        CHECK( pInfo1 != nullptr );
        CHECK( pInfo1->count == 1 );
        CHECK( pInfo1->number == "1" );
//        cout << test_name() << ": count=" << pInfo1->count
//             << ", number=" << pInfo1->number << endl;

        //measure 2
        ++it;   //note c4
        CHECK( (*it)->is_note() );
        ++it;   //note e4
        CHECK( (*it)->is_note() );
        ++it;   //barline
        CHECK( (*it)->is_barline() );
        pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_simple );
        CHECK( pBarline && pBarline->is_visible() );
        TypeMeasureInfo* pInfo2 = pBarline->get_measure_info();
        CHECK( pInfo2 != nullptr );
        CHECK( pInfo2->count == 2 );
        CHECK( pInfo2->number == "2" );
//        cout << test_name() << ": count=" << pInfo2->count
//             << ", number=" << pInfo2->number << endl;

        CHECK( pInfo1 != pInfo2 );

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, measure_003)
    {
        //@003. Global content copied in part

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<mnx>"
                "<global>"
                    "<measure><directions>"
                        "<time signature='4/4'/>"
                        "<key fifths=\"4\"/>"
                    "</directions></measure>"
                "</global>"
                "<part>"
                    "<part-name>Piano</part-name>"
                    "<measure>"
                      "<directions>"
                        "<staves number='1'/>"
                        "<clef line='2' sign='G'/>"
                      "</directions>"
                      "<sequence staff='1'>"
                        "<event value='/4'><note pitch='C4'/></event>"
                        "<event value='/2'><note pitch='E4'/></event>"
                      "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>");
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );

//        cout << test_name() << endl;
//        pScore->end_of_changes();
//        cout << pScore->to_string_with_ids() << endl;

        ImoInstrument* pInstr = pScore->get_instrument(0);
        TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
        CHECK( pInfo == nullptr );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

        CHECK( pMD && pMD->get_num_children() == 6 );
        ImoObj::children_iterator it = pMD->begin();    //key signature
        CHECK( (*it)->is_key_signature() );
        ++it;   //time signature
        CHECK( (*it)->is_time_signature() );
        ++it;   //clef
        CHECK( (*it)->is_clef() );
        ++it;   //note c4
        CHECK( (*it)->is_note() );
        ++it;   //note e4
        CHECK( (*it)->is_note() );
        ++it;   //barline
        CHECK( (*it)->is_barline() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_simple );
        CHECK( pBarline && pBarline->is_visible() );
        pInfo = pBarline->get_measure_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->count == 1 );

        delete pRoot;
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, measure_004)
    {
        //@004. Global content copied in part. Two measures

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_file(m_scores_path + "unit-tests/global/01-key-signatures.mnx");
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );

//        cout << test_name() << endl;
//        pScore->end_of_changes();
//        cout << pScore->to_string_with_ids() << endl;

        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != nullptr );

        CHECK( pMD && pMD->get_num_children() == 18 );
        ImoObj::children_iterator it = pMD->begin();    //key signature
        CHECK( (*it)->is_key_signature() );
        ++it;   //time signature
        CHECK( (*it)->is_time_signature() );
        ++it;   //clef
        CHECK( (*it)->is_clef() );
        ++it;   //note e5
        CHECK( (*it)->is_note() );
        ++it;   //note f5
        CHECK( (*it)->is_note() );
         ++it;   //note g5
        CHECK( (*it)->is_note() );
        ++it;   //note e5
        CHECK( (*it)->is_note() );
       ++it;   //barline
        CHECK( (*it)->is_barline() );
         ++it;   //note g5
        CHECK( (*it)->is_note() );
       ++it;   //barline
        CHECK( (*it)->is_barline() );
        ++it;   //key
        CHECK( (*it)->is_key_signature() );
        ++it;   //note a4
        CHECK( (*it)->is_note() );
        ++it;   //note b4
        CHECK( (*it)->is_note() );

        delete pRoot;
    }


    //@ mnx element ---------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, mnx_001)
    {
        //@001. empty content: returns empty score
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. <mnx>: missing mandatory element <global>." << endl
            << "Line 0. <mnx>: missing mandatory element <part>." << endl;
        parser.parse_text("<mnx></mnx>");
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore );

        delete pRoot;
    }


    //@ octave-shift --------------------------------------------------------------------

//    TEST_FIXTURE(MnxAnalyserTestFixture, octave_shift_001)
//    {
//        //@001. octave-shift minimum content parsed ok and created
//
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        XmlParser parser;
//        stringstream expected;
//        parser.parse_text(
//            "<mnx>"
//                "<global><measure>"
//                    "<directions><time signature='4/4'/></directions>"
//                "</measure></global>"
//                "<part><part-name/>"
//                    "<measure barline='regular'>"
//                        "<directions><clef sign='G' line='2'/></directions>"
//                        "<sequence>"
//                            "<event value='/4'><note pitch='C5'/></event>"
//                            "<event value='/4'><note pitch='E5'/></event>"
//                            "<directions><octave-shift type='-8' end='2:1/2'/></directions>"
//                            "<event value='/2'><note pitch='C7'/></event>"
//                        "</sequence>"
//                    "</measure>"
//                    "<measure>"
//                        "<sequence>"
//                            "<event value='/2'><note pitch='E7'/></event>"
//                            "<event value='/4'><note pitch='C7'/></event>"
//                            "<event value='/4'><note pitch='C6'/></event>"
//                        "</sequence>"
//                    "</measure>"
//                "</part>"
//            "</mnx>"
//        );
//        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);
//
//        XmlNode* tree = parser.get_tree_root();
//        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
//
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
//
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( doc.is_dirty() == true );
//        CHECK( pRoot != nullptr );
//        CHECK( pRoot && pRoot->is_document() == true );
//        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
//        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
//        if (pDoc)
//        {
//            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
//            CHECK( pScore != nullptr);
//            if (pScore)
//            {
//                ImoInstrument* pInstr = pScore->get_instrument(0);
//                ImoMusicData* pMD = pInstr->get_musicdata();
//                CHECK( pMD != nullptr );
//                CHECK( pMD->get_num_items() == 10 );
//
//                ImoObj::children_iterator it = pMD->begin();
//                CHECK( (*it) && (*it)->is_time_signature() );
//                ++it;
//                CHECK( (*it) && (*it)->is_clef() );
//
//                ++it;
//                CHECK( (*it) && (*it)->is_note() );
//
//                ++it;
//                CHECK( (*it) && (*it)->is_note() );
//                ImoNote* pNote = static_cast<ImoNote*>( *it );
//                if (pNote)
//                {
//                    ImoOctaveShift* pOctave = dynamic_cast<ImoOctaveShift*>(
//                                                    pNote->find_relation(k_imo_octave_shift) );
//                    CHECK( pOctave != nullptr );
//                    if (pOctave)
//                    {
//                        CHECK( pOctave->get_shift_steps() == -7 );
//                        CHECK( pOctave->get_octave_shift_number() == 1 );
//                        CHECK( !is_different(pOctave->get_color(), Color(0,0,0)) );
//                    }
//                }
//
////                ++it;
////                CHECK( (*it) && (*it)->is_barline() );
//
//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
//            }
//        }
//
//        delete pRoot;
//    }


    //@ repeat --------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, repeat_001)
    {
        //@001. repeat at end created correctly

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
                "<global><measure>"
                    "<directions><time signature='4/4'/><repeat type='end'/></directions>"
                "</measure></global>"
                "<part><part-name/>"
                    "<measure>"
                        "<directions><clef sign='G' line='2'/></directions>"
                        "<sequence>"
                            "<event value='/1'><note pitch='G4'/></event>"
                        "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 4 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it) && (*it)->is_time_signature() == true );

                ++it;
                CHECK( (*it) && (*it)->is_clef() );

                ++it;
                CHECK( (*it) && (*it)->is_note() );

                ++it;
                CHECK( (*it) && (*it)->is_barline() );
                ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
                CHECK( pBarline && pBarline->get_type() == k_barline_end_repetition );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }


    //@ segno ---------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, segno_001)
    {
        //@001. segno direction created correctly

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
                "<global><measure>"
                    "<directions><time signature='4/4'/></directions>"
                "</measure><measure>"
                    "<directions><segno/></directions>"
                "</measure></global>"
                "<part><part-name/>"
                    "<measure>"
                        "<directions><clef sign='G' line='2'/></directions>"
                        "<sequence>"
                            "<event value='/1'><note pitch='G4'/></event>"
                        "</sequence>"
                    "</measure>"
                    "<measure>"
                        "<sequence>"
                            "<event value='/1'><note pitch='E4'/></event>"
                        "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 7 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it) && (*it)->is_time_signature() == true );

                ++it;
                CHECK( (*it) && (*it)->is_clef() );

                ++it;
                CHECK( (*it) && (*it)->is_note() );

                ++it;
                CHECK( (*it) && (*it)->is_barline() );

                ++it;
                CHECK( (*it) && (*it)->is_direction() );
                ImoDirection* pSO = static_cast<ImoDirection*>( *it );
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

                ++it;
                CHECK( (*it) && (*it)->is_note() );

                ++it;
                CHECK( (*it) && (*it)->is_barline() );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }


    //@ tied ----------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, tied_001)
    {
        //@001. notes tied correctly

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
                "<global><measure>"
                "</measure></global>"
                "<part><part-name/>"
                    "<measure barline='regular'>"
                        "<directions><clef sign='G' line='2'/></directions>"
                        "<sequence>"
                            "<event value='/4'><note pitch='G4' id='note1'>"
                                "<tied target='note2'/></note></event>"
                            "<event value='/4'><note pitch='G4' id='note2'/></event>"
                        "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 4 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it) && (*it)->is_clef() == true );

                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNote1 = static_cast<ImoNote*>( *it );
                CHECK( pNote1 && pNote1->is_regular_note() == true );
                ImoTie* pTie = (pNote1 ? pNote1->get_tie_next() : nullptr);
                if (pNote1)
                {
                    CHECK( pNote1->get_xml_id() == "note1" );
                    CHECK( pNote1->is_tied_next() == true );
                    CHECK( pNote1->is_tied_prev() == false );
                    CHECK( pTie && pTie->get_start_note() == pNote1 );
                }

                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNote2 = static_cast<ImoNote*>( *it );
                CHECK( pNote2 && pNote2->is_regular_note() == true );
                if (pNote2)
                {
                    CHECK( pNote2->get_xml_id() == "note2" );
                    CHECK( pNote2->is_tied_next() == false );
                    CHECK( pNote2->is_tied_prev() == true );
                    CHECK( pTie && pTie->get_end_note() == pNote2 );
                }

                ++it;
                CHECK( (*it) && (*it)->is_barline() );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }

//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Notes_Tied)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "Line 0. No 'start' element for tie number 12. Tie ignored." << endl;
//        parser.parse_text("(musicData (n c4 q (tie 12 start)) (n c4 e (tie 12 stop)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
//        CHECK( pMusic != nullptr );
//        ImoObj::children_iterator it = pMusic->begin();
//
//        ImoNote* pNote1 = static_cast<ImoNote*>( *it );
//        ImoTie* pTie = pNote1->get_tie_next();
//        CHECK( pNote1->is_tied_next() == true );
//        CHECK( pNote1->is_tied_prev() == false );
//        CHECK( pTie->get_start_note() == pNote1 );
//
//        ++it;
//        ImoNote* pNote2 = static_cast<ImoNote*>( *it );
//        CHECK( pNote2->is_tied_next() == false );
//        CHECK( pNote2->is_tied_prev() == true );
//        CHECK( pTie->get_end_note() == pNote2 );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Notes_Tied_Impossible)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Requesting to tie notes of different voice or pitch. Tie number 12 will be ignored." << endl;
//        parser.parse_text("(musicData (n c4 q (tie 12 start)) (n c3 e (tie 12 stop)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
//        CHECK( pMusic != nullptr );
//        ImoObj::children_iterator it = pMusic->begin();
//
//        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
//        CHECK( pNote1 != nullptr );
//        CHECK( pNote1->is_tied_next() == false );
//        CHECK( pNote1->is_tied_prev() == false );
//
//        ++it;
//        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
//        CHECK( pNote2 != nullptr );
//        CHECK( pNote2->is_tied_next() == false );
//        CHECK( pNote2->is_tied_prev() == false );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }


    //@ tuplet --------------------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, tuplet_001)
    {
        //@001. tuplet created correctly

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        parser.parse_text(
            "<mnx>"
                "<global><measure>"
                "</measure></global>"
                "<part><part-name/>"
                    "<measure>"
                        "<directions><clef line='2' sign='G'/></directions>"
                        "<sequence>"
                            "<beams><beam events='event3 event4 event5'/></beams>"
                            "<tuplet inner='3/8' outer='2/8'>"
                                "<event id='event3' value='/8'><note pitch='E4'/></event>"
                                "<event id='event4' value='/8'><note pitch='F4'/></event>"
                                "<event id='event5' value='/8'><note pitch='G4'/></event>"
                            "</tuplet>"
                        "</sequence>"
                    "</measure>"
                "</part>"
            "</mnx>"
        );
        MnxAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( errormsg.str() == expected.str() );
        CHECK( doc.is_dirty() == true );
        CHECK( pRoot != nullptr );
        CHECK( pRoot && pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );
        if (pDoc)
        {
            ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
            CHECK( pScore != nullptr);
            if (pScore)
            {
                ImoInstrument* pInstr = pScore->get_instrument(0);
                ImoMusicData* pMD = pInstr->get_musicdata();
                CHECK( pMD != nullptr );
                CHECK( pMD->get_num_items() == 5 );
                ImoObj::children_iterator it = pMD->begin();
                CHECK( (*it) && (*it)->is_clef() == true );

                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNote1 = static_cast<ImoNote*>( *it );
                CHECK( pNote1 && pNote1->is_regular_note() == true );
                CHECK( pNote1 && pNote1->is_beamed() == true );
                if (pNote1)
                {
                    ImoTuplet* pTuplet = pNote1->get_first_tuplet();
                    CHECK( pTuplet != nullptr );
                    CHECK( pTuplet && pTuplet->get_actual_number() == 3 );
                    CHECK( pTuplet && pTuplet->get_normal_number() == 2 );
                    CHECK( pTuplet && pTuplet->get_show_bracket() == k_yesno_default );
                    CHECK( pTuplet && pTuplet->get_num_objects() == 3 );
                }
                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNote2 = static_cast<ImoNote*>( *it );
                CHECK( pNote2 && pNote2->is_regular_note() == true );
                CHECK( pNote2 && pNote2->is_beamed() == true );

                ++it;
                CHECK( (*it) && (*it)->is_note() );
                ImoNote* pNote3 = static_cast<ImoNote*>( *it );
                CHECK( pNote3 && pNote3->is_regular_note() == true );
                CHECK( pNote3 && pNote3->is_beamed() == true );
                if (pNote3)
                {
                    ImoTuplet* pTuplet2 = pNote3->get_first_tuplet();
                    CHECK( pTuplet2 == pNote1->get_first_tuplet() );
                }

                ++it;
                CHECK( (*it) && (*it)->is_barline() );

//                pScore->end_of_changes();
//                cout << test_name() << endl << pScore->to_string_with_ids() << endl;
            }
        }

        delete pRoot;
    }


    //@ z. miscellaneous ----------------------------------------------------------------

    TEST_FIXTURE(MnxAnalyserTestFixture, miscellaneous_001)
    {
        //@001. pitch_to_components() method

        int step;
        int octave;
        float alt;

        CHECK (MnxAnalyser::pitch_to_components("C4", &step, &octave, &alt) == false );
        CHECK (step == k_step_C);
        CHECK (octave == 4);
        CHECK (alt == 0.0f);

        CHECK (MnxAnalyser::pitch_to_components("C#4", &step, &octave, &alt) == false );
        CHECK (step == k_step_C);
        CHECK (octave == 4);
        CHECK (alt == 1.0f);

        CHECK (MnxAnalyser::pitch_to_components("Db4", &step, &octave, &alt) == false );
        CHECK (step == k_step_D);
        CHECK (octave == 4);
        CHECK (alt == -1.0f);

        CHECK (MnxAnalyser::pitch_to_components("G3+0.5", &step, &octave, &alt) == false );
        CHECK (step == k_step_G);
        CHECK (octave == 3);
        CHECK (alt == 0.5f);

        CHECK (MnxAnalyser::pitch_to_components("B5+1.5", &step, &octave, &alt) == false );
        CHECK (step == k_step_B);
        CHECK (octave == 5);
        CHECK (alt == 1.5f);

        CHECK (MnxAnalyser::pitch_to_components("C4-0.5", &step, &octave, &alt) == false );
        CHECK (step == k_step_C);
        CHECK (octave == 4);
        CHECK (alt == -0.5f);

        CHECK (MnxAnalyser::pitch_to_components("#c4", &step, &octave, &alt) == true );
        CHECK (MnxAnalyser::pitch_to_components("C+4", &step, &octave, &alt) == true );
        CHECK (MnxAnalyser::pitch_to_components("C12", &step, &octave, &alt) == true );
        CHECK (MnxAnalyser::pitch_to_components("C4##", &step, &octave, &alt) == true );
        CHECK (MnxAnalyser::pitch_to_components("C4+1,25", &step, &octave, &alt) == true );
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, miscellaneous_005)
    {
        //@005. get_note_value_quantity() method

        int noteType;
        int dots;
        int multiplier;

        CHECK (MnxAnalyser::get_note_value_quantity("/8", &noteType, &dots, &multiplier));
        CHECK (noteType == k_eighth);
        CHECK (dots == 0);
        CHECK (multiplier == 1);

        CHECK (MnxAnalyser::get_note_value_quantity("6/8", &noteType, &dots, &multiplier));
        CHECK (noteType == k_eighth);
        CHECK (dots == 0);
        CHECK (multiplier == 6);

        CHECK (MnxAnalyser::get_note_value_quantity("6/8d", &noteType, &dots, &multiplier));
        CHECK (noteType == k_eighth);
        CHECK (dots == 1);
        CHECK (multiplier == 6);

        CHECK (MnxAnalyser::get_note_value_quantity("5/1", &noteType, &dots, &multiplier));
        CHECK (noteType == k_whole);
        CHECK (dots == 0);
        CHECK (multiplier == 5);
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, miscellaneous_010)
    {
        //@010. note_value_quantity_to_duration() method

        TimeUnits duration;

        CHECK (MnxAnalyser::note_value_quantity_to_duration("/8", &duration));
        CHECK (duration == 32.0);

        CHECK (MnxAnalyser::note_value_quantity_to_duration("6/8", &duration));
        CHECK (duration == 32.0 * 6.0);

        CHECK (MnxAnalyser::note_value_quantity_to_duration("6/8d", &duration));
        CHECK (duration == (32.0 + 16.0)*6.0 );

        CHECK (MnxAnalyser::note_value_quantity_to_duration("5/1", &duration));
        CHECK (duration == 5.0 * 256.0);
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, miscellaneous_100)
    {
        //@100. tokenizer. Single space

        vector<string> result = MnxAnalyser::tokenize_spaces("p1 p2 p3");

        CHECK (result.size() == 3);
        CHECK (result[0] == "p1");
        CHECK (result[1] == "p2");
        CHECK (result[2] == "p3");
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, miscellaneous_101)
    {
        //@101. tokenizer. Single item

        vector<string> result = MnxAnalyser::tokenize_spaces("p1");

        CHECK (result.size() == 1);
        CHECK (result[0] == "p1");
    }

    TEST_FIXTURE(MnxAnalyserTestFixture, miscellaneous_102)
    {
        //@102. tokenizer. Several spaces

        vector<string> result = MnxAnalyser::tokenize_spaces("p1  p2   p3");

        CHECK (result.size() == 3);
        CHECK (result[0] == "p1");
        CHECK (result[1] == "p2");
        CHECK (result[2] == "p3");
    }

}

