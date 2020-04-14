//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_im_figured_bass.h"
#include "lomse_pitch.h"
#include "lomse_events.h"
#include "lomse_doorway.h"
#include "lomse_im_factory.h"
#include "lomse_time.h"
#include "lomse_file_system.h"
#include "lomse_autobeamer.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LdpAnalyserTestFixture
{
public:
    LibraryScope m_libraryScope;
    string m_scores_path;
    int m_requestType;
    bool m_fRequestReceived;


    LdpAnalyserTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_requestType(k_null_request)
        , m_fRequestReceived(false)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        m_libraryScope.set_unit_test(true);
    }

    ~LdpAnalyserTestFixture()    //TearDown fixture
    {
    }

    static void wrapper_lomse_request(void* pThis, Request* pRequest)
    {
        static_cast<LdpAnalyserTestFixture*>(pThis)->on_lomse_request(pRequest);
    }

    void on_lomse_request(Request* pRequest)
    {
        m_fRequestReceived = true;
        m_requestType = pRequest->get_request_type();
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};


SUITE(LdpAnalyserTest)
{

    //@ score ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserMissingMandatoryElementNoElements)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. score: missing mandatory element 'vers'." << endl
                 << "Line 0. score: missing mandatory element 'instrument'." << endl;
        parser.parse_text("(score )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( pRoot != nullptr );
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserMissingMandatoryElementMoreElements)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. score: missing mandatory element 'vers'." << endl
                 << "Line 0. score: missing mandatory element 'instrument'." << endl;
        parser.parse_text("(score)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << score->get_root()->to_string() << endl;
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( pRoot != nullptr );
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserLanguageRemoved)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. score: missing mandatory element 'vers'." << endl
                 << "Line 0. score: missing mandatory element 'instrument'." << endl;
        parser.parse_text("(score (language en utf-8))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( pRoot != nullptr );
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserHasMandatoryElement)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. score: missing mandatory element 'instrument'." << endl;
        parser.parse_text("(score (vers 1.6))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_version_string() == "1.6" );
        CHECK( pScore && pScore->get_num_instruments() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserOneOrMoreMissingFirst)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. score: missing mandatory element 'instrument'." << endl;
        parser.parse_text("(score (vers 1.6))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << score->get_root()->to_string() << endl;
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserOneOrMorePresentOne)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        //CHECK( pScore && pScore->get_num_instruments() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserOneOrMorePresentMore)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)(instrument (musicData))(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, score_get_instr_number)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)(instrument (musicData))(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        if (pScore)
        {
            CHECK( pScore->get_num_instruments() == 2 );
            ImoInstrument* pInstr = pScore->get_instrument(1);
            CHECK( pScore->get_instr_number_for(pInstr) == 1 );
        }

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ articulation ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_articulation_01)
    {
        //@ 01. articulation minimum content ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(tenuto)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_articulation() == true );
        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_placement() == k_placement_default );
        CHECK( pImo && pImo->get_articulation_type() == k_articulation_tenuto );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

//    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_articulation_02)
//    {
//        //@ 02. id in articulation
//
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(dyn#30 \"ppp\")");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pRoot );
//        CHECK( pImo != nullptr );
//        CHECK( pImo && pImo->get_placement() == k_placement_default );
//        CHECK( pImo && pImo->get_mark_type() == "ppp" );
//        CHECK( pImo && pImo->get_id() == 30L );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_articulation_03)
//    {
//        //@ 03. articulation with placement
//
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(dyn \"sf\" above)");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pRoot );
//        CHECK( pImo != nullptr );
//        CHECK( pImo && pImo->get_placement() == k_placement_above );
//        CHECK( pImo && pImo->get_mark_type() == "sf" );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_articulation_04)
//    {
//        //@ 04. articulation with location
//
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(dyn \"sf\" above (dx 70))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pRoot );
//        CHECK( pImo != nullptr );
//        CHECK( pImo && pImo->get_placement() == k_placement_above );
//        CHECK( pImo && pImo->get_mark_type() == "sf" );
//        CHECK( pImo && pImo->get_user_location_x() == 70.0f );
//        CHECK( pImo && pImo->get_user_location_y() == 0.0f );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_articulation_05)
//    {
//        //@ 05. Note with attached articulation
//
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(n c4 e (stem up)(dyn \"sf\" above (dx 70)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
//        CHECK( pNote != nullptr );
//        CHECK( pNote && pNote->is_stem_up() == true );
//        CHECK( pNote && pNote->get_num_attachments() == 1 );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pNote->get_attachment(0) );
//        CHECK( pImo != nullptr );
//        CHECK( pImo && pImo->get_placement() == k_placement_above );
//        CHECK( pImo && pImo->get_mark_type() == "sf" );
//        CHECK( pImo && pImo->get_user_location_x() == 70.0f );
//        CHECK( pImo && pImo->get_user_location_y() == 0.0f );
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }

    //@ barline --------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_OptionalElementMissing)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_simple );
        CHECK( pBarline && pBarline->is_visible() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_OptionalElementPresent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->is_middle() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_InvalidType)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown barline type 'invalid'. 'simple' barline assumed." << endl;
        parser.parse_text("(barline invalid)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_simple );
        CHECK( pBarline && pBarline->is_visible() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_Visible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (visible yes))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_BadVisible)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'label:invisible' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(barline double invisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_Middle)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double middle)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->is_middle() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->get_user_location_x() == 70.0f );
        CHECK( pBarline && pBarline->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_LocationY)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (dy 60.5))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->get_user_location_x() == 0.0f );
        CHECK( pBarline && pBarline->get_user_location_y() == 60.5f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_LocationXY)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (dx 70)(dy 20.3))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->get_user_location_x() == 70.0f );
        CHECK( pBarline && pBarline->get_user_location_y() == 20.3f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_BadLocationX)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid real number 'seven'. Replaced by '0'." << endl;
        parser.parse_text("(barline double (dx seven))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->get_user_location_x() == 0.0f );
        CHECK( pBarline && pBarline->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_BadLocationY)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid real number 'six'. Replaced by '0'." << endl;
        parser.parse_text("(barline double (dy six))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->get_user_location_x() == 0.0f );
        CHECK( pBarline && pBarline->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_LocationOrder)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Element 'barline': too many parameters. Extra parameters from 'number' have been ignored." << endl;
        parser.parse_text("(barline double (dy 70)(dx 20.3))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( pBarline && pBarline->get_user_location_x() == 20.3f );
        CHECK( pBarline && pBarline->get_user_location_y() == 70.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, barline_012)
    {
        //@012. Barline has id

        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline#7 double)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpElement* pBarline = tree->get_root();

        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_id() == 7 );

        delete tree->get_root();
    }

    // musicData ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserOneOrMoreOptionalAlternativesError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'instrument' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(musicData (n c4 q)(instrument 3))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_music_data() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserOneOrMoreOptionalAlternativesErrorRemoved)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'instrument' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(musicData (n c4 q)(instrument 3)(n d4 e))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserOneOrMoreOptionalAlternativesTwo)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData (n c4 q)(n d4 e.))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

#if LOMSE_COMPATIBILITY_LDP_1_5
    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MusicData_AuxobjIsAnchored)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData (n c4 q)(text \"Hello world\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }
#endif

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_musicData)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData#12 (n#10 c4 q))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_music_data() == true );
        ImoMusicData* pImo = static_cast<ImoMusicData*>( pRoot );
        CHECK( pImo && pImo->get_id() == 12L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, musicData_access_to_instrument)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument (musicData (n c4 q)(n d4 e)) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoInstrument* pInstr = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstr != nullptr );
        if (pInstr)
        {
            ImoMusicData* pMD = pInstr->get_musicdata();
            CHECK( pMD != nullptr );
            CHECK( pMD->get_instrument() == pInstr );
        }

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ note -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(n +d3 e.)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_note() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_sharp );
        CHECK( pNote && pNote->get_dots() == 1 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 3 );
        CHECK( pNote && pNote->get_step() == k_step_D );
        CHECK( pNote && pNote->get_duration() == 48.0f );
        CHECK( pNote && pNote->is_in_chord() == false );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_PitchError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown note pitch 'j17'. Replaced by 'c4'." << endl;
        parser.parse_text("(n j17 q)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_quarter );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_DurationErrorLetter)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown note/rest duration 'j.'. Replaced by 'q'." << endl;
        parser.parse_text("(n c4 j.)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_quarter );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_DurationErrorDots)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(n c4 e.1)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_quarter );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_Staff)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(n c4 e p7)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );
        CHECK( pNote && pNote->get_staff() == 6 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_StaffError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid staff 'pz'. Replaced by 'p1'." << endl;
        parser.parse_text("(n c4 e pz)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_staff() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_Voice)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(n c4 e v3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );
        CHECK( pNote && pNote->get_voice() == 3 );
        CHECK( pNote && pNote->is_tied_next() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_VoiceError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
        parser.parse_text("(n c4 e vx)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_voice() == 1 );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_note)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(n#10 c4 q)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_note() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_Attachment)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(n c4 e v3 (text \"andante\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote && pNote->get_dots() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );
        CHECK( pNote && pNote->get_voice() == 3 );
        CHECK( pNote && pNote->has_attachments() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_TieStart)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'end' element for tie number 12. Tie ignored." << endl;
        parser.parse_text("(n c4 e (tie 12 start))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pAnalyser = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pAnalyser->analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_tied_next() == false );
        CHECK( pNote && pNote->is_tied_prev() == false );
        delete pAnalyser;
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_TieStop)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'start/continue' elements for tie number 12. Tie ignored." << endl;
        parser.parse_text("(n c4 e (tie 12 stop))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote && pNote->is_tied_next() == false );
        CHECK( pNote && pNote->is_tied_prev() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Notes_Tied)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'start' element for tie number 12. Tie ignored." << endl;
        parser.parse_text("(musicData (n c4 q (tie 12 start)) (n c4 e (tie 12 stop)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = static_cast<ImoNote*>( *it );
        ImoTie* pTie = pNote1->get_tie_next();
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pTie->get_start_note() == pNote1 );

        ++it;
        ImoNote* pNote2 = static_cast<ImoNote*>( *it );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pTie->get_end_note() == pNote2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Notes_Tied_Impossible)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Requesting to tie notes of different voice or pitch. Tie number 12 will be ignored." << endl;
        parser.parse_text("(musicData (n c4 q (tie 12 start)) (n c3 e (tie 12 stop)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_tied_next() == false );
        CHECK( pNote1->is_tied_prev() == false );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Several_Notes)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'start' element for tie number 12. Tie ignored." << endl;
        parser.parse_text("(musicData (n c4 q)(n d4 q)(n e4 q))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it;
        int numNotes;
        for(numNotes=0, it=pMusic->begin(); it != pMusic->end(); ++it, ++numNotes)
            CHECK( (*it)->is_note() == true );
        CHECK( numNotes == 3 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ stem

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_StemUp)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. ?" << endl;
        parser.parse_text("(n c4 e (stem up))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_stem_up() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_StemDown)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. ?" << endl;
        parser.parse_text("(n c4 e (stem down))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_stem_down() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_StemError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid value 'no' for stem type. Default stem asigned." << endl;
        parser.parse_text("(n c4 e (stem no))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_stem_default() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );
        CHECK( pNote && pNote->get_stem_direction() == k_stem_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_StemTie)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(musicData (n c4 q (tie 12 start)(stem down)) (n c4 e (stem up)(tie 12 stop)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pNote1->is_stem_down() == true );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pNote2->is_stem_up() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_SeveralOldParams)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(n +d3 e. g+ p2)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_note() == true );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_notated_accidentals() == k_sharp );
        CHECK( pNote && pNote->get_dots() == 1 );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 3 );
        CHECK( pNote && pNote->get_step() == k_step_D );
        CHECK( pNote && pNote->get_duration() == 48.0f );
        CHECK( pNote && pNote->get_staff() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ tie (old syntax) -----------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TieOld)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(musicData (n c4 e l)(n c4 q))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        if (pNote1)
        {
            ImoTie* pTie = pNote1->get_tie_next();
            CHECK( pNote1->is_tied_next() == true );
            CHECK( pNote1->is_tied_prev() == false );
            CHECK( pTie->get_start_note() == pNote1 );
            ImoRelations* pRelObjs = pNote1->get_relations();
            CHECK( pRelObjs != nullptr );
            CHECK( pRelObjs->get_item(0) == pTie );

            ++it;
            ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
            CHECK( pNote2 != nullptr );
            if (pNote2)
            {
                CHECK( pNote2->is_tied_next() == false );
                CHECK( pNote2->is_tied_prev() == true );
                CHECK( pTie->get_end_note() == pNote2 );
                pRelObjs = pNote2->get_relations();
                CHECK( pRelObjs != nullptr );
                CHECK( pRelObjs->get_item(0) == pTie );
            }
        }

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TieOld_Error1)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No note found to match old syntax tie. Tie ignored." << endl;
        parser.parse_text("(musicData (n c4 e l)(n d4 q))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_tied_next() == false );
        CHECK( pNote1->is_tied_prev() == false );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TieOld_IntermediateNote)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'start' element for tie number 12. Tie ignored." << endl;
        parser.parse_text("(musicData (n c4 q v1 l)(n e4 q v2)(n c4 e v1))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        ImoTie* pTie = pNote1->get_tie_next();
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pTie->get_start_note() == pNote1 );

        ++it;
        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pTie->get_end_note() == pNote2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TieOld_IntermediateBarline)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'start' element for tie number 12. Tie ignored." << endl;
        parser.parse_text("(musicData (n c4 q v1 l)(barline simple)(n c4 e v1))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        ImoTie* pTie = pNote1->get_tie_next();
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pTie->get_start_note() == pNote1 );
        CHECK( pTie->is_tie() == true );

        ++it;
        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pTie->get_end_note() == pNote2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TieOld_Several)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData (n c4 q l)(n c4 e l)(n c4 e))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        ImoTie* pTie1 = pNote1->get_tie_next();
        CHECK( pTie1->get_start_note() == pNote1 );
        ImoRelations* pRelObjs = pNote1->get_relations();
        CHECK( pRelObjs != nullptr );
        CHECK( pRelObjs->get_item(0) == pTie1 );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_tied_next() == true );
        CHECK( pNote2->is_tied_prev() == true );
        ImoTie* pTie2 = pNote2->get_tie_next();
        CHECK( pTie1->get_end_note() == pNote2 );
        CHECK( pTie2->get_start_note() == pNote2 );
        pRelObjs = pNote2->get_relations();
        CHECK( pRelObjs != nullptr );
        CHECK( pRelObjs->get_item(0) == pTie1 );
        CHECK( pRelObjs->get_item(1) == pTie2 );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != nullptr );
        CHECK( pNote3->is_tied_next() == false );
        CHECK( pNote3->is_tied_prev() == true );
        CHECK( pTie2->get_end_note() == pNote3 );
        pRelObjs = pNote3->get_relations();
        CHECK( pRelObjs != nullptr );
        CHECK( pRelObjs->get_item(0) == pTie2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ tie ------------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tie_ParsedStop)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(tie 12 stop)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_tie_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == false );
        CHECK( pInfo && pInfo->get_tie_number() == 12 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        CHECK( pInfo && pInfo->get_bezier() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tie_ParsedStart)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(tie 15 start)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == true );
        CHECK( pInfo && pInfo->get_tie_number() == 15 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        CHECK( pInfo && pInfo->get_bezier() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tie_Bezier)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(tie 15 start (bezier (ctrol2-x -25)(start-y 36.765)) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == true );
        CHECK( pInfo && pInfo->get_tie_number() == 15 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        ImoBezierInfo* pBezier = pInfo->get_bezier();
        CHECK( pBezier != nullptr );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 36.765f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == -25.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tie_ParsedError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid tie type. Tie ignored." << endl;
        parser.parse_text("(tie 15 end)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tie_Color)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(tie 12 stop (color #00ff00))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_tie_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == false );
        CHECK( pInfo && pInfo->get_tie_number() == 12 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        CHECK( pInfo && pInfo->get_bezier() == nullptr );
        CHECK( is_equal(pInfo->get_color(), Color(0,255,0,255)) );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ bezier ---------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Bezier_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(bezier ctrol1-x:-25 (start-x 36.765) ctrol1-y:55)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_bezier_info() == true );
        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pRoot );
        CHECK( pBezier != nullptr );
        //cout << "start.x = " << pBezier->get_point(ImoBezierInfo::k_start).x << endl;
        //cout << "start.y = " << pBezier->get_point(ImoBezierInfo::k_start).y << endl;
        //cout << "end.x = " << pBezier->get_point(ImoBezierInfo::k_end).x << endl;
        //cout << "end.y = " << pBezier->get_point(ImoBezierInfo::k_end).y << endl;
        //cout << "ctrol1.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).x << endl;
        //cout << "ctrol1.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).y << endl;
        //cout << "ctrol2.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).x << endl;
        //cout << "ctrol2.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).y << endl;
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 36.765f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == -25.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 55.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Bezier_Error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown tag 'startx'." << endl <<
            "Line 0. Element 'undefined' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(bezier (startx 36.765) ctrol1-x:-25 ctrol1-y:55)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pRoot );
        CHECK( pBezier != nullptr );
        //cout << "start.x = " << pBezier->get_point(ImoBezierInfo::k_start).x << endl;
        //cout << "start.y = " << pBezier->get_point(ImoBezierInfo::k_start).y << endl;
        //cout << "end.x = " << pBezier->get_point(ImoBezierInfo::k_end).x << endl;
        //cout << "end.y = " << pBezier->get_point(ImoBezierInfo::k_end).y << endl;
        //cout << "ctrol1.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).x << endl;
        //cout << "ctrol1.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).y << endl;
        //cout << "ctrol2.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).x << endl;
        //cout << "ctrol2.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).y << endl;
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == -25.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 55.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Bezier_MissingValues)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(bezier (start-x 36.765))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_bezier_info() == true );
        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pRoot );
        CHECK( pBezier != nullptr );
        //cout << "start.x = " << pBezier->get_point(ImoBezierInfo::k_start).x << endl;
        //cout << "start.y = " << pBezier->get_point(ImoBezierInfo::k_start).y << endl;
        //cout << "end.x = " << pBezier->get_point(ImoBezierInfo::k_end).x << endl;
        //cout << "end.y = " << pBezier->get_point(ImoBezierInfo::k_end).y << endl;
        //cout << "ctrol1.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).x << endl;
        //cout << "ctrol1.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol1).y << endl;
        //cout << "ctrol2.x = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).x << endl;
        //cout << "ctrol2.y = " << pBezier->get_point(ImoBezierInfo::k_ctrol2).y << endl;
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 36.765f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // slur -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Slur_ParsedStop)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(slur 12 stop)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_slur_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == false );
        CHECK( pInfo && pInfo->get_slur_number() == 12 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        CHECK( pInfo && pInfo->get_bezier() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Slur_ParsedStart)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(slur 15 start)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == true );
        CHECK( pInfo && pInfo->get_slur_number() == 15 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        CHECK( pInfo && pInfo->get_bezier() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Slur_ParsedContinue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(slur 15 continue)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pRoot );
        CHECK( pInfo != nullptr );
        //TODO
        //CHECK( pInfo && pInfo->is_continue() == true );
        CHECK( pInfo && pInfo->get_slur_number() == 15 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        CHECK( pInfo && pInfo->get_bezier() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Slur_Bezier)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
        parser.parse_text("(slur 27 start (bezier (ctrol2-x -25)(start-y 36.765)) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == true );
        CHECK( pInfo && pInfo->get_slur_number() == 27 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        ImoBezierInfo* pBezier = pInfo->get_bezier();
        CHECK( pBezier != nullptr );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 36.765f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == -25.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Slur_Color)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(slur 12 start (color #00ff00))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_slur_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start() == true );
        CHECK( pInfo && pInfo->get_slur_number() == 12 );
        CHECK( pInfo && pInfo->get_note() == nullptr );
        CHECK( pInfo && pInfo->get_bezier() == nullptr );
        CHECK( is_equal(pInfo->get_color(), Color(0,255,0,255)) );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Slur_ParsedError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid slur type. Slur ignored." << endl;
        parser.parse_text("(slur 15 end)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_NotesSlurred)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(musicData (n c4 q (slur 12 start)) (n c4 e (slur 12 stop)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        ImoSlur* pSlur = static_cast<ImoSlur*>( pNote->find_relation(k_imo_slur) );
        CHECK( pSlur->get_slur_number() == 12 );
        CHECK( pSlur->get_num_objects() == 2 );
        ImoNote* pSlurNote1 = pSlur->get_start_note();
        ImoNote* pSlurNote2 = pSlur->get_end_note();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 == pSlurNote1 );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 == pSlurNote2 );


        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // rest -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Rest)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(r e.)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_rest() == true );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pRoot );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->has_attachments() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Rest_StaffNum)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(r e. p2)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pRoot );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->get_staff() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Rest_DefaultStaffNum)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(r e.)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pRoot );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->get_staff() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Rest_StaffNumInherited)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData (r e. p2)(n c4 q)(n d4 e p3)(r q))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        LdpTree::iterator it = tree->begin();
        ++it;
        ImoRest* pRest = dynamic_cast<ImoRest*>( (*it)->get_imo() );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_staff() == 1 );
        ++it;
        ++it;
        ++it;
        ImoNote* pNote = dynamic_cast<ImoNote*>( (*it)->get_imo() );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_staff() == 1 );
        ++it;
        ++it;
        ++it;
        pNote = dynamic_cast<ImoNote*>( (*it)->get_imo() );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_staff() == 2 );
        ++it;
        ++it;
        ++it;
        ++it;
        pRest = dynamic_cast<ImoRest*>( (*it)->get_imo() );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_staff() == 2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_rest)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(r#10 q)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_rest() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Rest_Attachment)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(r e. (text \"andante\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pRoot );
        CHECK( pRest != nullptr );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->has_attachments() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ fermata -------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_01)
    {
        //@ 01. fermata minimum ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(fermata)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_fermata() == true );
        ImoFermata* pFerm = static_cast<ImoFermata*>( pRoot );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_02)
    {
        //@ 02. id_in_fermata

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(fermata#10)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_fermata() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_03)
    {
        //@ 03. fermata: placement ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(fermata below)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_fermata() == true );
        ImoFermata* pFerm = static_cast<ImoFermata*>( pRoot );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_below );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_04)
    {
        //@ 04. fermata: symbol ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(fermata short)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_fermata() == true );
        ImoFermata* pFerm = static_cast<ImoFermata*>( pRoot );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_default );
        CHECK( pFerm->get_symbol() == ImoFermata::k_short );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_05)
    {
        //@ 05. fermata: placement & symbol ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(fermata long)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_fermata() == true );
        ImoFermata* pFerm = static_cast<ImoFermata*>( pRoot );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_default );
        CHECK( pFerm->get_symbol() == ImoFermata::k_long );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_06)
    {
        //@ fermata: symbol & placement error

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Parameter 'under' not supported. Ignored." << endl;
        parser.parse_text("(fermata under)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoFermata* pFerm = static_cast<ImoFermata*>( pRoot );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_07)
    {
        //@ 07. fermata: location

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(fermata above (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoFermata* pFerm = static_cast<ImoFermata*>( pRoot );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_above );
        CHECK( pFerm->get_user_location_x() == 70.0f );
        CHECK( pFerm->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_08)
    {
        //@ 08. fermata: error more elements

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'fermata': too many parameters. Extra parameters from 'fermata' have been ignored." << endl;
        parser.parse_text("(fermata above (dx 70)(fermata below))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoFermata* pFerm = static_cast<ImoFermata*>( pRoot );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_above );
        CHECK( pFerm->get_user_location_x() == 70.0f );
        CHECK( pFerm->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_fermata_09)
    {
        //@ 09. fermata attached to note

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. ?" << endl;
        parser.parse_text("(n c4 e (stem up)(fermata above (dx 70)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_stem_up() == true );
        CHECK( pNote && pNote->get_num_attachments() == 1 );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pNote->get_attachment(0) );
        CHECK( pFerm != nullptr );
        CHECK( pFerm->get_placement() == k_placement_above );
        CHECK( pFerm->get_user_location_x() == 70.0f );
        CHECK( pFerm->get_user_location_y() == 0.0f );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ dynamics ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_dynamics_01)
    {
        //@ 01. dynamics minimum content ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(dyn \"ppp\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_dynamics_mark() == true );
        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_placement() == k_placement_default );
        CHECK( pImo && pImo->get_mark_type() == "ppp" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_dynamics_02)
    {
        //@ 02. id in dynamics

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(dyn#30 \"ppp\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_placement() == k_placement_default );
        CHECK( pImo && pImo->get_mark_type() == "ppp" );
        CHECK( pImo && pImo->get_id() == 30L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_dynamics_03)
    {
        //@ 03. dynamics with placement

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(dyn \"sf\" above)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_placement() == k_placement_above );
        CHECK( pImo && pImo->get_mark_type() == "sf" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_dynamics_04)
    {
        //@ 04. dynamics with location

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(dyn \"sf\" above (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_placement() == k_placement_above );
        CHECK( pImo && pImo->get_mark_type() == "sf" );
        CHECK( pImo && pImo->get_user_location_x() == 70.0f );
        CHECK( pImo && pImo->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_dynamics_05)
    {
        //@ 05. Note with attached dynamics

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(n c4 e (stem up)(dyn \"sf\" above (dx 70)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_stem_up() == true );
        CHECK( pNote && pNote->get_num_attachments() == 1 );
        ImoDynamicsMark* pImo = dynamic_cast<ImoDynamicsMark*>( pNote->get_attachment(0) );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_placement() == k_placement_above );
        CHECK( pImo && pImo->get_mark_type() == "sf" );
        CHECK( pImo && pImo->get_user_location_x() == 70.0f );
        CHECK( pImo && pImo->get_user_location_y() == 0.0f );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ goFwd ---------------------------------------------------------------------------
    //@ goBack --------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoBackStart)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
        parser.parse_text("(goBack start)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_go_back_fwd() == true );
        ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>( pRoot );
        CHECK( pGBF != nullptr );
        CHECK( pGBF->is_to_start() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_goBack)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(goBack#10 start)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_go_back_fwd() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoBackEnd)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
        parser.parse_text("(goBack end)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoBackQ)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
        parser.parse_text("(goBack q)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>( pRoot );
        CHECK( pGBF != nullptr );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == -64.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoFwdEnd)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
        parser.parse_text("(goFwd end)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>( pRoot );
        CHECK( pGBF != nullptr );
        CHECK( pGBF->is_to_end() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_goFwd)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(goFwd#10 end)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_go_back_fwd() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoFwdStart)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'goFwd' has an incoherent value: go forward to start?. Element ignored." << endl;
        parser.parse_text("(goFwd start)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoFwdH)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
        parser.parse_text("(goFwd h)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>( pRoot );
        CHECK( pGBF != nullptr );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == 128.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoFwdNum)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
        parser.parse_text("(goFwd 128)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>( pRoot );
        CHECK( pGBF != nullptr );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == 128.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoFwdBadNumber)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Negative value for element 'goFwd/goBack'. Element ignored." << endl;
        parser.parse_text("(goFwd -128.3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserGoBackNum)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Element 'goBack' has an incoherent value: go backwards to end?. Element ignored." << endl;
        parser.parse_text("(goBack 21.3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>( pRoot );
        CHECK( pGBF != nullptr );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == -21.3f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ goFwd 2.0 -----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, fwd_1)
    {
        //has voice
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(goFwd h v3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        a.set_score_version("2.0");
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_rest() == true );
        ImoRest* pImo = static_cast<ImoRest*>( pRoot );
        CHECK( pImo && pImo->is_go_fwd() == true );
        CHECK( pImo && pImo->get_duration() == 128.0 );
        CHECK( pImo && pImo->get_voice() == 3 );
        CHECK( pImo && pImo->is_visible() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ clef ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(clef G)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_clef() == true );
        ImoClef* pClef = static_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_G2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_clef)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(clef#10 G)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_clef() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_Error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown clef type 'Fa4'. Assumed 'G'." << endl;
        parser.parse_text("(clef Fa4)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoClef* pClef = static_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_G2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef G (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoClef* pClef = static_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_G2 );
        CHECK( pClef->is_visible() );
        CHECK( pClef->get_user_location_x() == 70.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef C2 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoClef* pClef = static_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == false );
        CHECK( pClef->get_staff() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_NoVisible_Staff2)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef C2 p2 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoClef* pClef = static_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == false );
        CHECK( pClef->get_staff() == 1 );
        CHECK( pClef->get_symbol_size() == k_size_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_SymbolSizeOk)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef C2 (symbolSize cue))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoClef* pClef = static_cast<ImoClef*>( pRoot );
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == true );
        CHECK( pClef->get_staff() == 0 );
        CHECK( pClef->get_symbol_size() == k_size_cue );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_SymbolSizeError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid symbol size 'small'. 'full' size assumed." << endl;
        parser.parse_text("(clef C2 (symbolSize small))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoClef* pClef = static_cast<ImoClef*>( pRoot );

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pClef != nullptr );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == true );
        CHECK( pClef->get_staff() == 0 );
        CHECK( pClef->get_symbol_size() == k_size_full );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ instrument ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_Staves)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument (staves 2)(musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_instrument() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoInstrument* pInstrument = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstrument != nullptr );
        //cout << "num.staves=" << pInstrument->get_num_staves() << endl;
        CHECK( pInstrument->get_num_staves() == 2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_instrument)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument#10 (musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_instrument() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_StavesError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid value 'two' for staves. Replaced by 1." << endl;
        parser.parse_text("(instrument (staves two)(musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoInstrument* pInstrument = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstrument != nullptr );
        //cout << "num.staves=" << pInstrument->get_num_staves() << endl;
        CHECK( pInstrument->get_num_staves() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_AddedToScore)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_Name)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument (name \"Guitar\")(musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "Guitar" );
        CHECK( pInstr->get_abbrev().get_text() == "" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_Abbrev)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument (abbrev \"G.\")(musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "" );
        CHECK( pInstr->get_abbrev().get_text() == "G." );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_NameAbbrev)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument (name \"Guitar\")(abbrev \"G.\")(musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "Guitar" );
        CHECK( pInstr->get_abbrev().get_text() == "G." );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrErrorValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid MIDI instrument (0..127). MIDI info ignored." << endl;
        parser.parse_text("(infoMIDI piano 1)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>( pRoot );
        CHECK( pInfo == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrErrorRange)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid MIDI instrument (0..127). MIDI info ignored." << endl;
        parser.parse_text("(infoMIDI 315 1)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>( pRoot );
        CHECK( pInfo == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrumentOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(infoMIDI 56)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_sound_info() == true );
        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == -1 );
        CHECK( pMidi->get_midi_program() == 56 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_ChannelErrorValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid MIDI channel (0..15). Channel info ignored." << endl;
        parser.parse_text("(infoMIDI 56 25)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == -1 );
        CHECK( pMidi->get_midi_program() == 56 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrumentChannelOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(infoMIDI 56 10)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 10 );
        CHECK( pMidi->get_midi_program() == 56 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Instrument_MidiInfo)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument (infoMIDI 56 12)(musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "" );
        CHECK( pInstr->get_abbrev().get_text() == "" );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 12 );
        CHECK( pMidi->get_midi_program() == 56 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ key -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserKey)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(key G)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_key_signature() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoKeySignature* pKeySignature = static_cast<ImoKeySignature*>( pRoot );
        CHECK( pKeySignature != nullptr );
        CHECK( pKeySignature->get_key_type() == k_key_G );
        CHECK( pKeySignature->get_staff() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_key)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(key#10 G)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_key_signature() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserKeyError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown key 'Sol'. Assumed 'C'." << endl;
        parser.parse_text("(key Sol)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoKeySignature* pKeySignature = static_cast<ImoKeySignature*>( pRoot );
        CHECK( pKeySignature != nullptr );
        CHECK( pKeySignature->get_key_type() == k_key_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Key_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(key d (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoKeySignature* pKeySignature = static_cast<ImoKeySignature*>( pRoot );
        CHECK( pKeySignature != nullptr );
        CHECK( pKeySignature->get_key_type() == k_key_d );
        CHECK( pKeySignature->get_staff() == 0 );
        CHECK( pKeySignature->is_visible() );
        CHECK( pKeySignature->get_user_location_x() == 70.0f );
        CHECK( pKeySignature->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Key_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(key E- noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoKeySignature* pKeySignature = static_cast<ImoKeySignature*>( pRoot );
        CHECK( pKeySignature != nullptr );
        CHECK( pKeySignature->get_key_type() == k_key_Ef );
        CHECK( pKeySignature->get_user_location_x() == 0.0f );
        CHECK( pKeySignature->get_user_location_y() == 0.0f );
        CHECK( pKeySignature->is_visible() == false );


        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ lyric ---------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_01)
    {
        //@ 01. lyric minimum content ok

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lyric \"te\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_lyric() == true );
        ImoLyric* pImo = static_cast<ImoLyric*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_number() == 1 );
        CHECK( pImo && pImo->get_placement() == k_placement_below );
        CHECK( pImo && pImo->is_laughing() == false );
        CHECK( pImo && pImo->is_humming() == false );
        CHECK( pImo && pImo->is_end_line() == false );
        CHECK( pImo && pImo->is_end_paragraph() == false );
        CHECK( pImo && pImo->has_melisma() == false );
        CHECK( pImo && pImo->has_hyphenation() == false );
        CHECK( pImo && pImo->get_prev_lyric() == nullptr );
        CHECK( pImo && pImo->get_next_lyric() == nullptr );
        CHECK( pImo && pImo->get_num_text_items() == 1 );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != nullptr );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "te" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == nullptr );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_02)
    {
        //@ 02. id in lyric

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lyric#37 \"te\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_lyric() == true );
        ImoLyric* pImo = static_cast<ImoLyric*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_id() == 37L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_03)
    {
        //@ 03. lyric with two syllables

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lyric \"De\" \"A\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_lyric() == true );
        ImoLyric* pImo = static_cast<ImoLyric*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_number() == 1 );
        CHECK( pImo && pImo->get_placement() == k_placement_below );
        CHECK( pImo && pImo->is_laughing() == false );
        CHECK( pImo && pImo->is_humming() == false );
        CHECK( pImo && pImo->is_end_line() == false );
        CHECK( pImo && pImo->is_end_paragraph() == false );
        CHECK( pImo && pImo->has_hyphenation() == false );
        CHECK( pImo && pImo->has_melisma() == false );
        CHECK( pImo && pImo->get_prev_lyric() == nullptr );
        CHECK( pImo && pImo->get_next_lyric() == nullptr );
        CHECK( pImo && pImo->get_num_text_items() == 2 );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != nullptr );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == nullptr );
        CHECK( pSyl->has_elision() == true );
        CHECK( pSyl->get_elision_text() == "." );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl != nullptr );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "A" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == nullptr );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_04)
    {
        //@ 04. lyric: error no syllable

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. <lyric>: Missing syllable text. <lyric> ignored." << endl;
        parser.parse_text("(lyric (melisma))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_05)
    {
        //@ 05. lyric with hyphenation

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lyric \"De\" -)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_lyric() == true );
        ImoLyric* pImo = static_cast<ImoLyric*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_number() == 1 );
        CHECK( pImo && pImo->get_placement() == k_placement_below );
        CHECK( pImo && pImo->is_laughing() == false );
        CHECK( pImo && pImo->is_humming() == false );
        CHECK( pImo && pImo->is_end_line() == false );
        CHECK( pImo && pImo->is_end_paragraph() == false );
        CHECK( pImo && pImo->has_hyphenation() == true );
        CHECK( pImo && pImo->has_melisma() == false );
        CHECK( pImo && pImo->get_prev_lyric() == nullptr );
        CHECK( pImo && pImo->get_next_lyric() == nullptr );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != nullptr );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == nullptr );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_06)
    {
        //@ 06. lyric: error in hyphenation

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. <lyric>: Unknown parameter 'hyphen'. Ignored." << endl;
        parser.parse_text("(lyric \"De\" hyphen)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_lyric() == true );
        ImoLyric* pImo = static_cast<ImoLyric*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_number() == 1 );
        CHECK( pImo && pImo->get_placement() == k_placement_below );
        CHECK( pImo && pImo->is_laughing() == false );
        CHECK( pImo && pImo->is_humming() == false );
        CHECK( pImo && pImo->is_end_line() == false );
        CHECK( pImo && pImo->is_end_paragraph() == false );
        CHECK( pImo && pImo->has_hyphenation() == false );
        CHECK( pImo && pImo->has_melisma() == false );
        CHECK( pImo && pImo->get_prev_lyric() == nullptr );
        CHECK( pImo && pImo->get_next_lyric() == nullptr );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != nullptr );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == nullptr );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_07)
    {
        //@ 07. lyric with melisma

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lyric \"De\" (melisma))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_lyric() == true );
        ImoLyric* pImo = static_cast<ImoLyric*>( pRoot );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_number() == 1 );
        CHECK( pImo && pImo->get_placement() == k_placement_below );
        CHECK( pImo && pImo->is_laughing() == false );
        CHECK( pImo && pImo->is_humming() == false );
        CHECK( pImo && pImo->is_end_line() == false );
        CHECK( pImo && pImo->is_end_paragraph() == false );
        CHECK( pImo && pImo->has_hyphenation() == false );
        CHECK( pImo && pImo->has_melisma() == true );
        CHECK( pImo && pImo->get_prev_lyric() == nullptr );
        CHECK( pImo && pImo->get_next_lyric() == nullptr );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != nullptr );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == nullptr );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_08)
    {
        //@ 08. Note with attached lyric

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)(instrument (musicData "
            "(n c4 e (lyric \"De\"))"
            ")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_num_attachments() == 1 );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pImo != nullptr );
        CHECK( pImo && pImo->get_number() == 1 );
        CHECK( pImo && pImo->get_prev_lyric() == nullptr );
        CHECK( pImo && pImo->get_next_lyric() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_09)
    {
        //@ 09. lyrics are linked

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)(instrument (musicData "
            "(n c4 e (lyric \"Ah\"))"
            "(n d4 e (lyric \"Be\"))"
            "(n e4 e (lyric \"Ce\"))"
            ")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 0 );
        CHECK( pNote && pNote->get_num_attachments() == 1 );
        ImoLyric* pLyric1 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric1 != nullptr );
        CHECK( pLyric1->get_number() == 1 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 1);
        CHECK( pNote && pNote->get_num_attachments() == 1 );
        ImoLyric* pLyric2 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric2 != nullptr );
        CHECK( pLyric2->get_number() == 1 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 2);
        CHECK( pNote && pNote->get_num_attachments() == 1 );
        ImoLyric* pLyric3 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric3 != nullptr );
        CHECK( pLyric3->get_number() == 1 );

        CHECK( pLyric1->get_prev_lyric() == nullptr );
        CHECK( pLyric1->get_next_lyric() == pLyric2 );
        CHECK( pLyric2->get_prev_lyric() == pLyric1 );
        CHECK( pLyric2->get_next_lyric() == pLyric3 );
        CHECK( pLyric3->get_prev_lyric() == pLyric2 );
        CHECK( pLyric3->get_next_lyric() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, LdpAnalyser_lyric_10)
    {
        //@ 10. lyrics in different lines are linked by lines

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)(instrument (musicData "
            "(n c4 e (lyric 1 \"Ah1\")(lyric 2 \"Ah2\"))"
            "(n d4 e (lyric 1 \"Be1\")(lyric 2 \"Be2\"))"
            "(n e4 e (lyric 1 \"Ce1\")(lyric 2 \"Ce2\"))"
            ")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 0 );
        CHECK( pNote && pNote->get_num_attachments() == 2 );
        ImoLyric* pLyric11 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric11 != nullptr );
        CHECK( pLyric11->get_number() == 1 );
        ImoLyric* pLyric12 = dynamic_cast<ImoLyric*>( pNote->get_attachment(1) );
        CHECK( pLyric12 != nullptr );
        CHECK( pLyric12->get_number() == 2 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 1);
        CHECK( pNote && pNote->get_num_attachments() == 2 );
        ImoLyric* pLyric21 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric21 != nullptr );
        CHECK( pLyric21->get_number() == 1 );
        ImoLyric* pLyric22 = dynamic_cast<ImoLyric*>( pNote->get_attachment(1) );
        CHECK( pLyric22 != nullptr );
        CHECK( pLyric22->get_number() == 2 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 2);
        CHECK( pNote && pNote->get_num_attachments() == 2 );
        ImoLyric* pLyric31 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric31 != nullptr );
        CHECK( pLyric31->get_number() == 1 );
        ImoLyric* pLyric32 = dynamic_cast<ImoLyric*>( pNote->get_attachment(1) );
        CHECK( pLyric32 != nullptr );
        CHECK( pLyric32->get_number() == 2 );

        CHECK( pLyric11->get_prev_lyric() == nullptr );
        CHECK( pLyric11->get_next_lyric() == pLyric21 );
        CHECK( pLyric21->get_prev_lyric() == pLyric11 );
        CHECK( pLyric21->get_next_lyric() == pLyric31 );
        CHECK( pLyric31->get_prev_lyric() == pLyric21 );
        CHECK( pLyric31->get_next_lyric() == nullptr );

        CHECK( pLyric12->get_prev_lyric() == nullptr );
        CHECK( pLyric12->get_next_lyric() == pLyric22 );
        CHECK( pLyric22->get_prev_lyric() == pLyric12 );
        CHECK( pLyric22->get_next_lyric() == pLyric32 );
        CHECK( pLyric32->get_prev_lyric() == pLyric22 );
        CHECK( pLyric32->get_next_lyric() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ metronome -----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, metronome_00)
    {
        //@00. metronome, note = value

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0) (instrument#100"
                "(musicData (metronome q 55) )))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoDirection* pDir = static_cast<ImoDirection*>(*it);
        ImoMetronomeMark* pImo = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));

        CHECK( pImo && pImo->get_ticks_per_minute() == 55 );
        CHECK( pImo && pImo->get_mark_type() == ImoMetronomeMark::k_note_value );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, metronome_01)
    {
        //@01. metronome, just value

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(metronome 88)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, metronome_02)
    {
        //@02. metronome has id
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(metronome#10 88)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_Error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing metronome parameters. Replaced by '(metronome 60)'." << endl;
        parser.parse_text("(metronome)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 60 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_NoteValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(metronome e. 77)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_value );
        CHECK( pMM->get_ticks_per_minute() == 77 );
        CHECK( pMM->get_left_note_type() == k_eighth );
        CHECK( pMM->get_left_dots() == 1 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_NoteNote)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(metronome e. s)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_note );
        CHECK( pMM->get_left_note_type() == k_eighth );
        CHECK( pMM->get_left_dots() == 1 );
        CHECK( pMM->get_right_note_type() == k_16th );
        CHECK( pMM->get_right_dots() == 0 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_Error2)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Error in metronome parameters. Replaced by '(metronome 60)'." << endl;
        parser.parse_text("(metronome e. \"s\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 60 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(metronome 88 (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() );
        CHECK( pMM->get_user_location_x() == 70.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(metronome 88 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->get_user_location_x() == 0.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );
        CHECK( pMM->is_visible() == false );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_Parenthesis)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(metronome 88 parenthesis (visible no))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->get_user_location_x() == 0.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );
        CHECK( pMM->is_visible() == false );
        CHECK( pMM->has_parenthesis() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_Ordering)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. ?" << endl;
        parser.parse_text("(metronome 88 parenthesis (dx 7) noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() == false );
        CHECK( pMM->has_parenthesis() == true );
        CHECK( pMM->get_user_location_x() == 7.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_Error3)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'label:parentesis' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(metronome 88 parentesis (dx 7))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pRoot);
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );
        CHECK( pMM->get_user_location_x() == 7.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, metronome_12)
    {
        //@12. metronome directly attached to MusicData

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. Element 'label:parentesis' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(musicData (metronome 88 (dx 7)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();
        ImoDirection* pDir = dynamic_cast<ImoDirection*>( *it );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );
        CHECK( pMM->get_user_location_x() == 7.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, metronome_13)
    {
        //@13. attached to direction

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(dir (metronome 60))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDirection* pDir = static_cast<ImoDirection*>( pRoot );
        CHECK( pDir != nullptr );
        ImoMetronomeMark* pMM = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
        CHECK( pMM != nullptr );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 60 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, metronome_14)
    {
        //@14. Error: metronome attached to spacer

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected <<
            "Line 0. Element 'metronome' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(spacer 10 (metronome 60))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 10.0f );
        CHECK( pSp && pSp->get_staff() == 0 );
        CHECK( pSp && pSp->get_attachment(0) == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    //@ time signature -------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(time 6 8)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature->get_bottom_number() == 8 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_time_signature)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(time#10 2 4)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_time_signature() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_Error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. time: missing mandatory element 'number'." << endl;
        parser.parse_text("(time 2)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 2 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(time 3 4 (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 3 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );
        CHECK( pTimeSignature->is_visible() );
        CHECK( pTimeSignature->get_user_location_x() == 70.0f );
        CHECK( pTimeSignature->get_user_location_y() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(time 6 8 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature->get_bottom_number() == 8 );
        CHECK( pTimeSignature->get_user_location_x() == 0.0f );
        CHECK( pTimeSignature->get_user_location_y() == 0.0f );
        CHECK( pTimeSignature->is_visible() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_common)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(time common)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->is_common() );
        CHECK( pTimeSignature->get_top_number() == 4 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_cut)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(time cut)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->is_cut() );
        CHECK( pTimeSignature->get_top_number() == 2 );
        CHECK( pTimeSignature->get_bottom_number() == 2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_type_error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Time signature: invalid type 'novalid'. 'normal' assumed." << endl
                 << "Line 0. time: missing mandatory element 'number'." << endl
                 << "Line 0. time: missing mandatory element 'number'." << endl;
        parser.parse_text("(time novalid)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->get_top_number() == 2 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );
        CHECK( pTimeSignature->is_normal() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, timeSignature_10)
    {
        //@10. time signature. single number
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(time single-number 10)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = static_cast<ImoTimeSignature*>( pRoot );
        CHECK( pTimeSignature != nullptr );
        CHECK( pTimeSignature->is_single_number() );
        CHECK( pTimeSignature->get_top_number() == 10 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // systemInfo -----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserSystemInfoBadType)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Expected 'first' or 'other' value but found 'third'. 'first' assumed." << endl;
        parser.parse_text("(systemLayout third (systemMargins 0 0 0 2000))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_system_info() == true );
        ImoSystemInfo* pSI = static_cast<ImoSystemInfo*>( pRoot );
        CHECK( pSI != nullptr );
        CHECK( pSI->is_first() );
        CHECK( pSI->get_left_margin() == 0.0f );
        CHECK( pSI->get_right_margin() == 0.0f );
        CHECK( pSI->get_system_distance() == 0.0f );
        CHECK( pSI->get_top_system_distance() == 2000.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserSystemInfoMissingMargins)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. systemLayout: missing mandatory element 'systemMargins'." << endl;
        parser.parse_text("(systemLayout other)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSystemInfo* pSI = static_cast<ImoSystemInfo*>( pRoot );
        CHECK( pSI != nullptr );
        CHECK( !pSI->is_first() );
        CHECK( pSI->get_left_margin() == 0.0f );
        CHECK( pSI->get_right_margin() == 0.0f );
        CHECK( pSI->get_system_distance() == 0.0f );
        CHECK( pSI->get_top_system_distance() == 0.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserSystemMargins)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(systemLayout other (systemMargins 0 100 0 2000))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_system_info() == true );
        ImoSystemInfo* pSI = static_cast<ImoSystemInfo*>( pRoot );
        CHECK( pSI != nullptr );
        CHECK( !pSI->is_first() );
        CHECK( pSI->get_left_margin() == 0.0f );
        CHECK( pSI->get_right_margin() == 100.0f );
        CHECK( pSI->get_system_distance() == 0.0f );
        CHECK( pSI->get_top_system_distance() == 2000.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserSystemInfoOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. systemInfo: missing mandatory element 'systemMargins'." << endl;
        parser.parse_text("(systemLayout other (systemMargins 0 100 0 2000))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSystemInfo* pSL = static_cast<ImoSystemInfo*>( pRoot );
        CHECK( pSL != nullptr );
        CHECK( !pSL->is_first() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // text -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserText)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. systemInfo: missing mandatory element 'systemMargins'." << endl;
        parser.parse_text("(text \"This is a text\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_score_text() == true );
        ImoScoreText* pText = static_cast<ImoScoreText*>( pRoot );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "This is a text" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_score_text)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(text#10 \"This is a text\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_score_text() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, AnalyserTextMissingText)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. text: missing mandatory element 'string'." << endl;
        parser.parse_text("(text)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScoreText* pText = static_cast<ImoScoreText*>( pRoot );
        CHECK( pText == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Text_AlignStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(text \"Moonlight sonata\" (style \"Header1\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreText* pText = static_cast<ImoScoreText*>( pRoot );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "Moonlight sonata" );
        ImoStyle* pStyle = pText->get_style();
        CHECK( pStyle == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Text_Location)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(text \"F. Chopin\" (style \"Composer\")(dy 30)(dx 20))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreText* pText = static_cast<ImoScoreText*>( pRoot );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "F. Chopin" );
        CHECK( pText && pText->get_user_location_x() == 20.0f );
        CHECK( pText && pText->get_user_location_y() == 30.0f );
        ImoStyle* pStyle = pText->get_style();
        CHECK( pStyle == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    // opt ------------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_BoolTrue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(opt StaffLines.Hide true)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_option() == true );
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>( pRoot );
        CHECK( pOpt != nullptr );
        CHECK( pOpt->get_name() == "StaffLines.Hide" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_boolean );
        CHECK( pOpt->get_bool_value() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_BoolErrorValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid value for option 'StaffLines.Hide'. Option ignored." << endl;
        parser.parse_text("(opt StaffLines.Hide perhaps)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_ErrorName)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid option 'StaffLines.Funny'. Option ignored." << endl;
        parser.parse_text("(opt StaffLines.Funny funny thing)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_LongOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(opt StaffLines.Truncate 2)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>( pRoot );
        CHECK( pOpt != nullptr );
        CHECK( pOpt->get_name() == "StaffLines.Truncate" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_long );
        CHECK( pOpt->get_long_value() == k_truncate_barline_any );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_LongErrorValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid value for option 'Render.SpacingValue'. Option ignored." << endl;
        parser.parse_text("(opt Render.SpacingValue perhaps)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_FloatOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(opt Render.SpacingFactor 0.536)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>( pRoot );
        CHECK( pOpt != nullptr );
        CHECK( pOpt->get_name() == "Render.SpacingFactor" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_float );
        CHECK( pOpt->get_float_value() == 0.536f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_FloatErrorValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid value for option 'Render.SpacingFactor'. Option ignored." << endl;
        parser.parse_text("(opt Render.SpacingFactor perhaps)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_ErrorMissingValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing value for option 'Render.SpacingFactor'. Option ignored." << endl;
        parser.parse_text("(opt Render.SpacingFactor)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Opt_AddedToScore)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)(opt StaffLines.Hide true)(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->has_options() == true );
        ImoOptionInfo* pOpt = pScore->get_option("StaffLines.Hide");
        CHECK( pOpt != nullptr );
        CHECK( pOpt->get_name() == "StaffLines.Hide" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_boolean );
        CHECK( pOpt->get_bool_value() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_DefaultOptReplaced)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)"
            "(opt Render.SpacingMethod 1)(opt Render.SpacingValue 30)"
            "(opt Render.SpacingFactor 0.778)(instrument (musicData)))"
        );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->has_options() == true );
        ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingMethod");
        CHECK( pOpt && pOpt->get_name() == "Render.SpacingMethod" );
        CHECK( pOpt && pOpt->get_type() == ImoOptionInfo::k_number_long );
        CHECK( pOpt && pOpt->get_long_value() == 1L );
        pOpt = pScore->get_option("Render.SpacingValue");
        CHECK( pOpt && pOpt->get_name() == "Render.SpacingValue" );
        CHECK( pOpt && pOpt->get_type() == ImoOptionInfo::k_number_long );
        CHECK( pOpt && pOpt->get_long_value() == 30L );
        pOpt = pScore->get_option("Render.SpacingFactor");
        CHECK( pOpt && pOpt->get_name() == "Render.SpacingFactor" );
        CHECK( pOpt && pOpt->get_type() == ImoOptionInfo::k_number_float );
        CHECK( pOpt && is_equal_float(pOpt->get_float_value(), 0.778f) );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_LastDefaultOptReplaced)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)"
            "(opt Render.SpacingFactor 4.0)(instrument (musicData)))"
        );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->has_options() == true );
        ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
        CHECK( pOpt != nullptr );
        CHECK( pOpt->get_name() == "Render.SpacingFactor" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_float );
        CHECK( pOpt->get_float_value() == 4.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    // nodes ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_DeleteParameter)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'label:instrument' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(n c4 q instrument)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_note_type() == k_quarter );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_DeleteNode)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid option 'StaffLines.Funny'. Option ignored." << endl;
        parser.parse_text("(opt StaffLines.Funny true)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // spacer ---------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(spacer 70.5)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_spacer() == true );
        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 70.5f );
        CHECK( pSp && pSp->get_staff() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_spacer)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(spacer#10 88)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_spacer() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_MissingWidth)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing width for spacer. Spacer ignored." << endl;
        parser.parse_text("(spacer)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_Staff)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(spacer 70.5 p3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 70.5f );
        CHECK( pSp && pSp->get_staff() == 2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_ErrorStaff)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid staff 'pan'. Replaced by 'p1'." << endl;
        parser.parse_text("(spacer 70.5 pan)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 70.5f );
        CHECK( pSp && pSp->get_staff() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_ErrorMoreParams)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'label:more' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(spacer 70.5 more)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 70.5f );
        CHECK( pSp && pSp->get_staff() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_Attachment)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(spacer 70 (text \"andante\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 70.0f );
        CHECK( pSp && pSp->get_staff() == 0 );
        CHECK( pSp && pSp->has_attachments() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_ErrorAttachment)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'r' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(spacer 70 (r q))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 70.0f );
        CHECK( pSp && pSp->get_staff() == 0 );
        CHECK( pSp && pSp->has_attachments() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Spacer_ErrorAttachment2)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'r' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(spacer 70 (r q)(text \"andante\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDirection* pSp = static_cast<ImoDirection*>( pRoot );
        CHECK( pSp != nullptr );
        CHECK( pSp && pSp->get_width() == 70.0f );
        CHECK( pSp && pSp->get_staff() == 0 );
        CHECK( pSp && pSp->has_attachments() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // lenmusdoc ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Lenmusdoc_ReturnsImobj)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc (vers 0.0)(content ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr);
        CHECK( pRoot->is_document() == true );
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_lenmusdoc)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc#10 (vers 0.0)(content ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_document() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Lenmusdoc_language)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc (vers 0.0)(language \"zh_CN\")(content ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        //cout << "language='" << pDoc->get_language() << "'" << endl;
        CHECK( pDoc && pDoc->get_language() == "zh_CN" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Lenmusdoc_HasContent)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc (vers 0.0)(content (text \"hello world\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc && pDoc->get_num_content_items() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Lenmusdoc_GetContentItemText)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0)(content (text \"hello world\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(0) );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "hello world" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Lenmusdoc_GetContentItemScore)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0)(content (score (vers 1.6)(instrument (musicData))) (text \"hello world\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        CHECK( pDoc && pDoc->get_num_content_items() == 2 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(1) );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "hello world" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_score)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc#11 (vers 0.0)(content (score#10 (vers 1.6)(instrument (musicData))) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoObj* pImo = pDoc->get_content_item(0);
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_content)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc (vers 0.0)(content#60 (score#9 (vers 1.6)(instrument (musicData))) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoObj* pImo = pDoc->get_content();
        CHECK( pImo && pImo->get_id() == 60L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, lenmusdoc_10)
    {
        ///10. ImoDocument: default styles created
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc (vers 0.0)(content (score (vers 2.0)(instrument (musicData))) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoStyle* pStyle = pDoc->find_style("Heading-1");
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Heading-1" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // TiesBuilder ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TiesBuilder_EndTieOk)
    {
        Document doc(m_libraryScope);
        ImoNote* pStartNote = ImFactory::inject_note(&doc, k_step_E, 4, k_eighth, k_sharp);
        ImoNote* pEndNote = ImFactory::inject_note(&doc, k_step_E, 4, k_eighth, k_sharp);

        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pStartInfo->set_start(true);
        pStartInfo->set_tie_number(12);
        pStartInfo->set_note(pStartNote);

        ImoTieDto* pEndInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pEndInfo->set_start(false);
        pEndInfo->set_tie_number(12);
        pEndInfo->set_note(pEndNote);

        LdpAnalyser a(cout, m_libraryScope, &doc);
        TiesBuilder builder(cout, &a);
        builder.add_item_info(pStartInfo);
        builder.add_item_info(pEndInfo);

        CHECK( pStartNote->is_tied_next() == true);
        CHECK( pStartNote->is_tied_prev() == false);
        CHECK( pEndNote->is_tied_next() == false);
        CHECK( pEndNote->is_tied_prev() == true);

        delete pStartNote;
        delete pEndNote;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TiesBuilder_StartTieDuplicated)
    {
        Document doc(m_libraryScope);
        stringstream errormsg;
        stringstream expected;
        expected << "Line 0. A tie with the same number is already defined for this "
            "element in line 0. This tie will be ignored." << endl;

        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pStartInfo->set_start(true);
        pStartInfo->set_tie_number(12);

        ImoTieDto* pOtherInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pOtherInfo->set_start(true);
        pOtherInfo->set_tie_number(12);

        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        TiesBuilder builder(errormsg, &a);
        builder.add_item_info(pStartInfo);
        builder.add_item_info(pOtherInfo);

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TiesBuilder_ErrorNotesCanNotBeTied)
    {
        Document doc(m_libraryScope);
        stringstream errormsg;
        stringstream expected;
        expected << "Line 0. Requesting to tie notes of different voice or pitch. Tie number 12 will be ignored." << endl;

        ImoNote* startNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        startNote->set_notated_pitch(2, k_octave_4, k_sharp);
        startNote->set_note_type(k_eighth);

        ImoNote* endNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        endNote->set_notated_pitch(2, k_octave_3, k_sharp);
        endNote->set_note_type(k_eighth);

        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pStartInfo->set_start(true);
        pStartInfo->set_tie_number(12);
        pStartInfo->set_note(startNote);

        ImoTieDto* pEndInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pEndInfo->set_start(false);
        pEndInfo->set_tie_number(12);
        pEndInfo->set_note(endNote);

        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        TiesBuilder builder(errormsg, &a);
        builder.add_item_info(pStartInfo);
        builder.add_item_info(pEndInfo);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete startNote;
        delete endNote;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TiesBuilder_ErrorNoStartInfo)
    {
        Document doc(m_libraryScope);
        stringstream errormsg;
        stringstream expected;
        expected << "Line 0. No 'start/continue' elements for tie number 12. Tie ignored." << endl;

        ImoTieDto* pEndInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pEndInfo->set_start(false);
        pEndInfo->set_tie_number(12);

        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        TiesBuilder builder(errormsg, &a);
        builder.add_item_info(pEndInfo);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TiesBuilder_PendingTiesAtDeletion)
    {
        Document doc(m_libraryScope);
        stringstream errormsg;
        stringstream expected;
        expected << "Line 0. No 'end' element for tie number 12. Tie ignored." << endl
                 << "Line 0. No 'end' element for tie number 14. Tie ignored." << endl;

        ImoTieDto* pStartInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pStartInfo->set_start(true);
        pStartInfo->set_tie_number(12);

        ImoTieDto* pOtherInfo = static_cast<ImoTieDto*>(ImFactory::inject(k_imo_tie_dto, &doc));
        pOtherInfo->set_start(true);
        pOtherInfo->set_tie_number(14);

        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        TiesBuilder* pBuilder = LOMSE_NEW TiesBuilder(errormsg, &a);
        pBuilder->add_item_info(pStartInfo);
        pBuilder->add_item_info(pOtherInfo);

        delete pBuilder;

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TiesBuilder_InstrumentChangeError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'end' element for tie number 12. Tie ignored." << endl
                 << "Line 0. No 'start/continue' elements for tie number 12. Tie ignored." << endl;
        parser.parse_text("(score (vers 1.6)(instrument (musicData (n c4 q (tie 12 start))))(instrument (musicData (n d4 e (tie 12 stop)))))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 0 );
        CHECK( pNote && pNote->is_tied_next() == false );
        CHECK( pNote && pNote->is_tied_prev() == false );

        pInstr = pScore->get_instrument(1);
        pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );

        it = pMusic->begin();

        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 1 );
        CHECK( pNote && pNote->is_tied_next() == false );
        CHECK( pNote && pNote->is_tied_prev() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ beam ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Beam_Start)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(beam 12 +)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_beam_dto() == true );
        ImoBeamDto* pInfo = static_cast<ImoBeamDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_beam_number() == 12 );
        CHECK( pInfo && pInfo->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pInfo && pInfo->get_beam_type(1) == ImoBeam::k_none );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Beam_TreeLevels)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(beam 12 ++f)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBeamDto* pInfo = static_cast<ImoBeamDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_beam_number() == 12 );
        CHECK( pInfo && pInfo->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pInfo && pInfo->get_beam_type(1) == ImoBeam::k_begin );
        CHECK( pInfo && pInfo->get_beam_type(2) == ImoBeam::k_forward );
        CHECK( pInfo && pInfo->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pInfo && pInfo->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pInfo && pInfo->get_beam_type(5) == ImoBeam::k_none );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Beam_ErrorNum)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid beam number. Beam ignored." << endl;
        parser.parse_text("(beam +)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot == nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Beam_ErrorType)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid beam type. Beam ignored." << endl;
        parser.parse_text("(beam 34 empieza)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot == nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamsBuilder_Destructor)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        parser.parse_text("(n c4 e (beam 14 +))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_beamed() == false );
        CHECK( pNote && pNote->get_beam_type(0) == ImoBeam::k_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamsBuilder_BeamError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'start/continue' elements for beam number 13. Beam ignored." << endl
                 << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        parser.parse_text("(musicData (n c4 q. (beam 14 +)) (n d4 s (beam 13 -)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_beamed() == false );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_beamed() == false );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamsBuilder_BeamOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        parser.parse_text("(musicData (n c4 q. (beam 14 +)) (n d4 s (beam 14 -b)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_beamed() == true );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_beamed() == true );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_backward );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamsBuilder_InstrumentChangeError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        parser.parse_text("(score (vers 1.6) (instrument (musicData (n c4 q. (beam 14 +)))) (instrument (musicData (n c4 e))))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );

        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 0 );
        CHECK( pNote && pNote->is_beamed() == false );
        CHECK( pNote && pNote->get_beam_type(0) == ImoBeam::k_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, beam_10)
    {
        //bug found: beam fails when preceeded by stem and staff
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        string src = "(musicData (n c4 e. p1 (stem up) (beam 14 +))"
            "(n d4 s p1 (stem up) (beam 14 -b)))";
        parser.parse_text(src);
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_beamed() == true );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_beamed() == true );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_backward );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, beam_11)
    {
        //duration ok when not beamed note in middle of beamed group
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        string src =
            "(score (vers 1.7) (instrument (musicData "
            "(clef F4 p1)(n e3 e v1 p1 (beam 37 +)(t + 3 2))"
            "(goBack start)(n c2 w v1 p1)(goBack 234.667)"
            "(n g3 e v1 p1 (beam 37 =))(n c4 e v1 p1 (beam 37 -)(t -)) )))";
        parser.parse_text(src);
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        CHECK( (*it)->is_clef() == true );
        ++it;
        CHECK( (*it)->is_note() == true );
        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == true );
        CHECK( is_equal_time(pNote1->get_duration(), k_duration_eighth) );
        ++it;
        CHECK( (*it)->is_go_back_fwd() == true );
        ++it;
        CHECK( (*it)->is_note() == true );
        pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == false );
        CHECK( is_equal_time(pNote1->get_duration(), k_duration_whole) );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ beam (old syntax) ---------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamOld_ErrorInvalidG)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid parameter 'g+7'. Ignored." << endl;
        parser.parse_text("(n c4 e g+7)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_note_type() == k_eighth );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == k_step_C );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamOld_ErrorInvalidNote)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Requesting beaming a note longer than eighth. Beam ignored." << endl;
        parser.parse_text("(n c4 w g+)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 0 );
        CHECK( pNote && pNote->get_note_type() == k_whole );
        CHECK( pNote && pNote->is_beamed() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamOld_ErrorAlreadyOpen)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Requesting to start a beam (g+) but there is already an open beam. Beam ignored." << endl;
        parser.parse_text("(musicData (n c4 s g+) (n e4 e g+))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_BeamOld_SES)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'start' element for beam number 12. Tie ignored." << endl;
        parser.parse_text("(musicData (n c4 s g+)(n e4 e)(n c4 s g-))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1->is_beamed() == true );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_forward );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2->is_beamed() == true );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_continue );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != nullptr );
        CHECK( pNote3->is_beamed() == true );
        CHECK( pNote3->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote3->get_beam_type(1) == ImoBeam::k_backward );
        CHECK( pNote3->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote3->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote3->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote3->get_beam_type(5) == ImoBeam::k_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ AutoBeamer ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_AutoBeamer_SE)
    {
        Document doc(m_libraryScope);

        ImoBeam* pBeam = static_cast<ImoBeam*>( ImFactory::inject(k_imo_beam, &doc) );

        ImoNote* pNote1 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote1->set_note_type(k_16th);
        ImoBeamDto dto1;
        ImoBeamData* pData1 = ImFactory::inject_beam_data(&doc, &dto1);
        pNote1->include_in_relation(&doc, pBeam, pData1);

        ImoNote* pNote2 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote2->set_note_type(k_eighth);
        ImoBeamDto dto2;
        ImoBeamData* pData2 = ImFactory::inject_beam_data(&doc, &dto2);
        pNote2->include_in_relation(&doc, pBeam, pData2);

        AutoBeamer autobeamer(pBeam);
        autobeamer.do_autobeam();

        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_forward );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );

        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );

        delete pNote1;
        delete pNote2;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_AutoBeamer_EE)
    {
        Document doc(m_libraryScope);

        ImoBeam* pBeam = static_cast<ImoBeam*>( ImFactory::inject(k_imo_beam, &doc) );
        ImoNote* pNote1 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote1->set_note_type(k_eighth);
        ImoBeamDto dto1;
        ImoBeamData* pData1 = ImFactory::inject_beam_data(&doc, &dto1);
        pNote1->include_in_relation(&doc, pBeam, pData1);

        ImoNote* pNote2 = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        pNote2->set_note_type(k_eighth);
        ImoBeamDto dto2;
        ImoBeamData* pData2 = ImFactory::inject_beam_data(&doc, &dto2);
        pNote2->include_in_relation(&doc, pBeam, pData2);

        AutoBeamer autobeamer(pBeam);
        autobeamer.do_autobeam();

        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );

        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );

        delete pNote1;
        delete pNote2;
    }

    //@ tuplet new syntax ---------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tuplet_TypeError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid tuplet type. Tuplet ignored." << endl;
        parser.parse_text("(t 5 start 3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tuplet_ActualNotes)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(t + 3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_tuplet_dto() == true );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tuplet_ErrorNormalNumRequired)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Tuplet: Missing or invalid normal notes number. Tuplet ignored." << endl;
        parser.parse_text("(t 4 + 7)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletNormalNotes)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(t 2 + 7 4)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 7 );
        CHECK( pInfo && pInfo->get_normal_number() == 4 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_default );
        CHECK( pInfo && pInfo->get_show_number() == ImoTuplet::k_number_actual );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tuplet_NoBracket)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(t 2 + 3 2 (displayBracket no))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_no );
        CHECK( pInfo && pInfo->get_show_number() == ImoTuplet::k_number_actual );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tuplet_DisplayNormalNum)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(t 1 + 3 2 (displayNumber none))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_default );
        CHECK( pInfo && pInfo->get_show_number() == ImoTuplet::k_number_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Tuplet_ErrorLabelParameter)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid yes/no value 'false'. Replaced by default." << endl;
        parser.parse_text("(t 1 + 3 2 (displayBracket false))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, tuplet_1)
    {
        //is score version < 1.6 tuplet also affects notes duration
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        string src =
            "(score (vers 1.6) (instrument (musicData "
            "(clef F4 p1)(n e3 e v1 p1 (beam 37 +)(t + 3 2))"
            "(n g3 e v1 p1 (beam 37 =))(n c4 e v1 p1 (beam 37 -)(t -)) )))";
        parser.parse_text(src);
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        CHECK( (*it)->is_clef() == true );
        ++it;
        CHECK( (*it)->is_note() == true );
        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == true );
        CHECK( is_equal_time(pNote1->get_duration(), 21.3333) );
        ++it;
        CHECK( (*it)->is_note() == true );
        pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == true );
        CHECK( is_equal_time(pNote1->get_duration(), 21.3333) );
        ++it;
        CHECK( (*it)->is_note() == true );
        pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == true );
        CHECK( is_equal_time(pNote1->get_duration(), 21.3333) );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, tuplet_2)
    {
        //if score version > 1.6 tuplet does not affect notes duration
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. No 'end' element for beam number 14. Beam ignored." << endl;
        string src =
            "(score (vers 1.7) (instrument (musicData "
            "(clef F4 p1)(n e3 e v1 p1 (beam 37 +)(t + 3 2))"
            "(n g3 e v1 p1 (beam 37 =))(n c4 e v1 p1 (beam 37 -)(t -)) )))";
        parser.parse_text(src);
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        CHECK( (*it)->is_clef() == true );
        ++it;
        CHECK( (*it)->is_note() == true );
        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == true );
        CHECK( is_equal_time(pNote1->get_duration(), k_duration_eighth) );
        ++it;
        CHECK( (*it)->is_note() == true );
        pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == true );
        CHECK( is_equal_time(pNote1->get_duration(), k_duration_eighth) );
        ++it;
        CHECK( (*it)->is_note() == true );
        pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1->is_beamed() == true );
        CHECK( is_equal_time(pNote1->get_duration(), k_duration_eighth) );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, tuplet_3)
    {
        //@03. tuplet. Nested tuplets

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        string src =   //test score 01014-nested-tuplets.lms
            "(score (vers 2.0)(instrument (musicData"
            "(clef G)(key A)(time 2 4)"
            "(n b4 e (tm 2 3)(beam 1 +)(t 1 + 3 2))"
            "(n b4 e (tm 2 3)(beam 1 -))"
            "(n b4 e (tm 4 15)(beam 2 +)(t 2 + 5 2))"
            "(n b4 e (tm 4 15)(beam 2 =))"
            "(n b4 e (tm 4 15)(beam 2 =))"
            "(n b4 e (tm 4 15)(beam 2 =))"
            "(n b4 e (tm 4 15)(beam 2 -)(t 2 -))"
            "(n b4 e (tm 2 3)(beam 3 +))"
            "(n b4 e (tm 2 3)(beam 3 -)(t 1 -)) )))";
        parser.parse_text(src);
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        CHECK( (*it)->is_clef() == true );
        ++it;
        CHECK( (*it)->is_key_signature() == true );
        ++it;
        CHECK( (*it)->is_time_signature() == true );
        ++it;
        CHECK( (*it)->is_note() == true );
        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        ImoTuplet* pTuplet1 = pNote1->get_first_tuplet();
        CHECK( pTuplet1->get_actual_number() == 3 );
        CHECK( pTuplet1->get_normal_number() == 2 );
        ++it;
        CHECK( (*it)->is_note() == true );
        ++it;
        CHECK( (*it)->is_note() == true );
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        ImoTuplet* pTuplet2 = pNote2->get_first_tuplet();
        CHECK( pTuplet2->get_actual_number() == 5 );
        CHECK( pTuplet2->get_normal_number() == 2 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ tuplet old full syntax ----------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_TypeError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid tuplet type. Tuplet ignored." << endl;
        parser.parse_text("(t start 3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ActualNotes)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(t + 3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_tuplet_dto() == true );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ErrorNormalNumRequired)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Tuplet: Missing or invalid normal notes number. Tuplet ignored." << endl;
        parser.parse_text("(t + 7)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_NormalNotes)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(t + 7 4)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 7 );
        CHECK( pInfo && pInfo->get_normal_number() == 4 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_NoBracket)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(t + 3 noBracket)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_no );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ErrorLabelParameter)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'label:blue' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(t + 3 noBracket blue)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_no );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ErrorCompoundParameter)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'color' unknown or not possible here. Ignored." << endl;
        parser.parse_text("(t + 3 (color blue) noBracket)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = static_cast<ImoTupletDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo && pInfo->get_actual_number() == 3 );
        CHECK( pInfo && pInfo->get_normal_number() == 2 );
        CHECK( pInfo && pInfo->get_show_bracket() == k_yesno_no );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletsBuilder_Destructor)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'end' element for tuplet number 0. Tuplet ignored." << endl;
        parser.parse_text("(n c4 e (t + 3))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote && pNote->find_attachment(k_imo_tuplet) == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletsBuilder_TupletOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(musicData "
            "(n c4 e (t + 3)) (n e4 e) (n d4 e (t -)) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        delete pA;

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
        CHECK( pTuplet->is_tuplet() == true );
        CHECK( pTuplet->get_num_objects() == 3 );
        CHECK( pTuplet->get_actual_number() == 3 );
        CHECK( pTuplet->get_normal_number() == 2 );

        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
            pTuplet->get_related_objects();
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
        ImoNote* pNt1 = dynamic_cast<ImoNote*>( (*itN).first );
        ++itN;
        ImoNote* pNt2 = dynamic_cast<ImoNote*>( (*itN).first );
        ++itN;
        ImoNote* pNt3 = dynamic_cast<ImoNote*>( (*itN).first );


        it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1 == pNt1 );
        CHECK( pNote1->find_relation(k_imo_tuplet) == pTuplet );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2 == pNt2 );
        CHECK( pNote2->find_relation(k_imo_tuplet) == pTuplet );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != nullptr );
        CHECK( pNote3 == pNt3 );
        CHECK( pNote3->find_relation(k_imo_tuplet) == pTuplet );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletsBuilder_InstrumentChangeError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. No 'end' element for tuplet number 0. Tuplet ignored." << endl;
        parser.parse_text("(score (vers 1.6) (instrument "
            "(musicData (n c4 e (t + 3)))) (instrument (musicData (n c4 e))))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );

        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 0 );
        CHECK( pNote && pNote->get_first_tuplet() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    //@ tuplet (old tn/t- syntax) -------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ErrorInvalidParameter)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid parameter 't-7'. Ignored." << endl;
        parser.parse_text("(n c4 e t-7)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_octave() == 4 );
        CHECK( pNote && pNote->get_step() == 0 );
        CHECK( pNote && pNote->get_first_tuplet() == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(n c4 e t3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ok2)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(n c4 e t7/6)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser* pA = LOMSE_NEW LdpAnalyser(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = pA->analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete pA;
        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_ErrorAlreadyOpen)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Requesting to start a tuplet but there is already an open tuplet. Tuplet ignored." << endl;
        parser.parse_text("(musicData (n c4 s t3) (n d4 e) (n e4 e t3))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
        CHECK( pTuplet == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TupletOld_TupletOk)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(musicData (n c4 e t3) (n e4 e) (n d4 e t-))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
        CHECK( pTuplet->get_num_objects() == 3 );
        CHECK( pTuplet->get_actual_number() == 3 );
        CHECK( pTuplet->get_normal_number() == 2 );

        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
            pTuplet->get_related_objects();
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
        ImoNote* pNt1 = dynamic_cast<ImoNote*>( (*itN).first );
        ++itN;
        ImoNote* pNt2 = dynamic_cast<ImoNote*>( (*itN).first );
        ++itN;
        ImoNote* pNt3 = dynamic_cast<ImoNote*>( (*itN).first );

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != nullptr );
        CHECK( pNote1 == pNt1 );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != nullptr );
        CHECK( pNote2 == pNt2 );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != nullptr );
        CHECK( pNote3 == pNt3 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    //@ timeModification ----------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, timeModification_0)
    {
        //error
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. tm: missing mandatory element 'number'." << endl;
        parser.parse_text("(tm 5)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, timeModification_1)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(tm 2 3)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_time_modification_dto() == true );
        ImoTimeModificationDto* pInfo =
                dynamic_cast<ImoTimeModificationDto*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_top_number() == 2 );
        CHECK( pInfo && pInfo->get_bottom_number() == 3 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ voice (element) -----------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_Voice_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. ?" << endl;
        parser.parse_text("(n c4 e (voice 7))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_voice() == 7 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_Voice_Error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid integer number 'no'. Replaced by '1'." << endl;
        parser.parse_text("(n c4 e (voice no))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_voice() == 1 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ staffNum (element) --------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_StaffNum_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. ?" << endl;
        parser.parse_text("(n c4 e (staffNum 2))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_staff() == 1 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Note_StaffNum_Error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid integer number 'alpha'. Replaced by '1'." << endl;
        parser.parse_text("(n c4 e (staffNum alpha))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->get_staff() == 0 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ rest (full) ---------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Rest_Full)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(musicData "
            "(r e (t + 3)(voice 3)(staffNum 2)) (r e (text \"Hello\")) (r e (t -)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();
        ImoRest* pRest = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest != nullptr);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pRest->find_relation(k_imo_tuplet));
        CHECK( pTuplet->get_num_objects() == 3 );

        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
            pTuplet->get_related_objects();
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
        ImoRest* pNR1 = dynamic_cast<ImoRest*>( (*itN).first );
        ++itN;
        ImoRest* pNR2 = dynamic_cast<ImoRest*>( (*itN).first );
        ++itN;
        ImoRest* pNR3 = dynamic_cast<ImoRest*>( (*itN).first );

        CHECK( (*it)->is_rest() == true );
        ImoRest* pRest1 = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest1 != nullptr );
        CHECK( pRest1 == pNR1 );
        CHECK( pRest1->get_voice() == 3 );
        CHECK( pRest1->get_staff() == 1 );

        ++it;
        ImoRest* pRest2 = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest2 != nullptr );
        CHECK( pRest2 == pNR2 );
        CHECK( pRest2->get_voice() == 3 );
        CHECK( pRest2->get_staff() == 1 );
        CHECK( pRest2->has_attachments() == true );

        ++it;
        ImoRest* pRest3 = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest3 != nullptr );
        CHECK( pRest3 == pNR3 );
        CHECK( pRest3->get_voice() == 3 );
        CHECK( pRest3->get_staff() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // color ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, ImoColorDto)
    {
        ImoColorDto color;
        CHECK( color.red() == 0 );
        CHECK( color.green() == 0 );
        CHECK( color.blue() == 0 );
        CHECK( color.alpha() == 255 );
        color.set_from_rgb_string("#ff3217");
        CHECK( color.red() == 255 );
        CHECK( color.green() == 50 );
        CHECK( color.blue() == 23 );
        CHECK( color.alpha() == 255 );
        color.set_from_rgba_string("#fffe4580");
        CHECK( color.red() == 255 );
        CHECK( color.green() == 254 );
        CHECK( color.blue() == 69 );
        CHECK( color.alpha() == 128 );
        CHECK( color.is_color_dto() == true );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, ImoColor_Constructor)
    {
        ImoColorDto color(12,32,255,180);
        CHECK( color.red() == 12 );
        CHECK( color.green() == 32 );
        CHECK( color.blue() == 255 );
        CHECK( color.alpha() == 180 );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, ImoColor_Error)
    {
        ImoColorDto color;
        color.set_from_string("#ff3g17");
        CHECK( color.red() == 0 );
        CHECK( color.green() == 0 );
        CHECK( color.blue() == 0 );
        CHECK( color.alpha() == 255 );
        color.set_from_string("#fff");
        CHECK( color.red() == 0 );
        CHECK( color.green() == 0 );
        CHECK( color.blue() == 0 );
        CHECK( color.alpha() == 255 );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Color_ErrorInvalidData)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid color value. Must be #rrggbbaa. Color ignored." << endl;
        parser.parse_text("(color 321700)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot == nullptr );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Color_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(color #f0457f)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoColorDto* pColor = static_cast<ImoColorDto*>( pRoot );
        CHECK( pColor != nullptr );
        CHECK( pColor->red() == 240 );
        CHECK( pColor->green() == 69 );
        CHECK( pColor->blue() == 127 );
        CHECK( pColor->alpha() == 255 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Color_SetInParent)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(n c4 e (color #f0457f))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        Color& color = pNote->get_color();
        CHECK( color.r == 240 );
        CHECK( color.g == 69 );
        CHECK( color.b == 127 );
        CHECK( color.a == 255 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_Color)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (color #ff0000))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pRoot );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_double );
        CHECK( pBarline && pBarline->is_visible() );
        CHECK( is_equal(pBarline->get_color(), Color(255,0,0)) );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_barline)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(barline#10)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_barline() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    //@ parts ---------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_01)
    {
        //@01. parts defined. error: instrument requires <instrId>
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. instrument: missing 'partId'. Instrument ignored." << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1))(instrument (musicData)) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        CHECK( pScore && pScore->get_instrument("P1") != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_02)
    {
        //@02. parts, minimum content: one instr id.

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1))(instrument P1 (musicData)) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        CHECK( pScore && pScore->get_instrument("P1") != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_03)
    {
        //@03. error: <partId> not defined in <parts>

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. 'partId' is not defined in <parts> element. Instrument ignored."
                 << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1))(instrument P2 (musicData)) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        CHECK( pScore && pScore->get_instrument("P1") != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_04)
    {
        //04. parts. error: duplicated <partId>

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. parts: duplicated <partId> will be ignored."
                 << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1 P1))(instrument P1 (musicData)) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 1 );
        CHECK( pScore && pScore->get_instrument("P1") != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_05)
    {
        //@05. parts, error: at least one <partId> is required

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. parts: at least one <partId> is required." << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts)(instrument (musicData)) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_06)
    {
        //@06. parts, with one group. ok.

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1 P2)(group P1 P2))"
            "(instrument P1 (musicData))"
            "(instrument P2 (musicData))"
            ")" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != nullptr );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != nullptr );
        CHECK( pGroup->get_instrument(0) != nullptr );
        CHECK( pGroup->get_instrument(1) != nullptr );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_none );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_07)
    {
        //@07. parts, one group with grpSymbol.

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1 P2)(group P1 P2 (symbol bracket)))"
            "(instrument P1 (musicData))"
            "(instrument P2 (musicData))"
            ")" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != nullptr );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != nullptr );
        CHECK( pGroup->get_instrument(0) != nullptr );
        CHECK( pGroup->get_instrument(1) != nullptr );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_08)
    {
        //@08. parts, one group joinBarlines transferred to instruments.

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1 P2)"
            "(group P1 P2 (symbol bracket)(joinBarlines mensurstrich)) )"
            "(instrument P1 (musicData))"
            "(instrument P2 (musicData))"
            ")" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != nullptr );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != nullptr );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );
        CHECK( pGroup->join_barlines() == ImoInstrGroup::k_mensurstrich );
        ImoInstrument* pInstr = pGroup->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_mensurstrich );
        pInstr = pGroup->get_instrument(1);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_nothing );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, parts_09)
    {
        //@09. parts, all instruments added to the group.

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)"
            "(group P1 P3 (symbol bracket)(joinBarlines yes)) )"
            "(instrument P1 (musicData))"
            "(instrument P2 (musicData))"
            "(instrument P3 (musicData))"
            ")" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score() == true );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->get_num_instruments() == 3 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != nullptr );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != nullptr );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );
        CHECK( pGroup->join_barlines() == ImoInstrGroup::k_standard );
        //barlines layout transferred to instruments
        ImoInstrument* pInstr = pGroup->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_joined );
        pInstr = pGroup->get_instrument(1);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_joined );
        pInstr = pGroup->get_instrument(2);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_isolated );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // group ----------------------------------------------------------------------------

//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_All)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(group (name \"Group\")(abbrev \"G.\")"
//                "(symbol bracket)(joinBarlines no)"
//                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
//                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
//                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pRoot->is_instr_group() == true );
//        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>( pRoot );
//        CHECK( pGrp != nullptr );
//        CHECK( pGrp->get_name_string() == "Group" );
//        CHECK( pGrp->get_abbrev_string() == "G." );
//        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
//        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
//        CHECK( pGrp->get_num_instruments() == 3 );
//
//        //AWARE: Group doesn't get ownership of instruments. Therefore, as
//        //group is not included in a score, we must delete instruments.
//        delete pGrp->get_instrument(0);
//        delete pGrp->get_instrument(1);
//        delete pGrp->get_instrument(2);
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_NoName)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(group (abbrev \"G.\")"
//                "(symbol bracket)(joinBarlines no)"
//                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
//                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
//                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>( pRoot );
//        CHECK( pGrp != nullptr );
//        CHECK( pGrp->get_name_string() == "" );
//        CHECK( pGrp->get_abbrev_string() == "G." );
//        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
//        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
//        CHECK( pGrp->get_num_instruments() == 3 );
//
//        //AWARE: Group doesn't get ownership of instruments. Therefore, as
//        //group is not included in a score, we must delete instruments.
//        delete pGrp->get_instrument(0);
//        delete pGrp->get_instrument(1);
//        delete pGrp->get_instrument(2);
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_NoAbbrev)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(group (name \"Group\")"
//                "(symbol bracket)(joinBarlines no)"
//                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
//                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
//                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>( pRoot );
//        CHECK( pGrp != nullptr );
//        CHECK( pGrp->get_name_string() == "Group" );
//        CHECK( pGrp->get_abbrev_string() == "" );
//        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
//        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
//        CHECK( pGrp->get_num_instruments() == 3 );
//
//        //AWARE: Group doesn't get ownership of instruments. Therefore, as
//        //group is not included in a score, we must delete instruments.
//        delete pGrp->get_instrument(0);
//        delete pGrp->get_instrument(1);
//        delete pGrp->get_instrument(2);
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_NoNameAbbrev)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "" << endl;
//        parser.parse_text("(group (symbol bracket)(joinBarlines no)"
//                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
//                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
//                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>( pRoot );
//        CHECK( pGrp != nullptr );
//        CHECK( pGrp->get_name_string() == "" );
//        CHECK( pGrp->get_abbrev_string() == "" );
//        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
//        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_bracket );
//        CHECK( pGrp->get_num_instruments() == 3 );
//
//        //AWARE: Group doesn't get ownership of instruments. Therefore, as
//        //group is not included in a score, we must delete instruments.
//        delete pGrp->get_instrument(0);
//        delete pGrp->get_instrument(1);
//        delete pGrp->get_instrument(2);
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_ErrorSymbol)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Missing or invalid group symbol. Must be 'none', 'brace' or 'bracket'. Group ignored." << endl;
//        parser.parse_text("(group (symbol good)(joinBarlines no)"
//                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
//                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
//                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pRoot == nullptr );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_ErrorJoin)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Invalid value for joinBarlines. Must be "
//                    "'yes', 'no' or 'mensurstrich'. 'yes' assumed." << endl;
//        parser.parse_text("(group (symbol brace)(joinBarlines perhaps)"
//                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
//                "(instrument (name \"Tenor\")(abbrev \"T\")(musicData))"
//                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>( pRoot );
//        CHECK( pGrp != nullptr );
//        CHECK( pGrp->get_name_string() == "" );
//        CHECK( pGrp->get_abbrev_string() == "" );
//        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_standard );
//        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_brace );
//        CHECK( pGrp->get_num_instruments() == 3 );
//
//        //AWARE: Group doesn't get ownership of instruments. Therefore, as
//        //group is not included in a score, we must delete instruments.
//        delete pGrp->get_instrument(0);
//        delete pGrp->get_instrument(1);
//        delete pGrp->get_instrument(2);
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_ErrorMissingInstruments)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Missing instruments in group!. Group ignored." << endl;
//        parser.parse_text("(group (symbol brace)(joinBarlines yes))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pRoot == nullptr );
//
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }
//
//    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Group_ErrorInInstrument)
//    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        expected << "Line 0. Element 'n' unknown or not possible here. Ignored." << endl;
//        parser.parse_text("(group (symbol brace)(joinBarlines no)"
//                "(instrument (name \"Soprano\")(abbrev \"S\")(musicData))"
//                "(n c4 q)"
//                "(instrument (name \"Bass\")(abbrev \"B\")(musicData)))");
//        LdpTree* tree = parser.get_ldp_tree();
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>( pRoot );
//        CHECK( pGrp != nullptr );
//        CHECK( pGrp->get_name_string() == "" );
//        CHECK( pGrp->get_abbrev_string() == "" );
//        CHECK( pGrp->join_barlines() == ImoInstrGroup::k_no );
//        CHECK( pGrp->get_symbol() == ImoInstrGroup::k_brace );
//        CHECK( pGrp->get_num_instruments() == 2 );
//
//        //AWARE: Group doesn't get ownership of instruments. Therefore, as
//        //group is not included in a score, we must delete instruments.
//        delete pGrp->get_instrument(0);
//        delete pGrp->get_instrument(1);
//        delete tree->get_root();
//        // coverity[check_after_deref]
//        if (pRoot && !pRoot->is_document()) delete pRoot;
//    }

    // chord ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Chord_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData (chord (n c4 q)(n e4 q)(n g4 q)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );

        ImoObj::children_iterator it = pMusic->begin();
        CHECK( pMusic->get_num_children() == 3 );

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

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_NoteInChord_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData (n c4 q)(na e4 q)(na g4 q))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );

        ImoObj::children_iterator it = pMusic->begin();
        CHECK( pMusic->get_num_children() == 3 );

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

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Chord_Beamed)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData "
            "(chord (n a3 e (beam 1 +)) (n d3 e))"
            "(chord (n a3 e (beam 1 -))(n d3 e)) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );

        ImoObj::children_iterator it = pMusic->begin();
        CHECK( pMusic->get_num_children() == 4 );

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_in_chord() == true );
        CHECK( pNote && pNote->is_start_of_chord() == true );
        CHECK( pNote && pNote->is_end_of_chord() == false );
        CHECK( pNote && pNote->is_beamed() == true );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_in_chord() == true );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == true );
        CHECK( pNote && pNote->is_beamed() == false );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_in_chord() == true );
        CHECK( pNote && pNote->is_start_of_chord() == true );
        CHECK( pNote && pNote->is_end_of_chord() == false );
        CHECK( pNote && pNote->is_beamed() == true );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->is_in_chord() == true );
        CHECK( pNote && pNote->is_start_of_chord() == false );
        CHECK( pNote && pNote->is_end_of_chord() == true );
        CHECK( pNote && pNote->is_beamed() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // pageLayout -----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_PageLayout_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(pageLayout (pageSize 14000 10000)(pageMargins 1000 1200 3000 2500 4000) landscape)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoPageInfo* pInfo = static_cast<ImoPageInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_page_info() == true );
        CHECK( pInfo && pInfo->get_left_margin() == 1000.0f );
        CHECK( pInfo && pInfo->get_top_margin() == 1200.0f );
        CHECK( pInfo && pInfo->get_right_margin() == 3000.0f );
        CHECK( pInfo && pInfo->get_bottom_margin() == 2500.0f );
        CHECK( pInfo && pInfo->get_binding_margin() == 4000.0f );
        CHECK( pInfo && pInfo->get_page_width() == 14000.0f );
        CHECK( pInfo && pInfo->get_page_height() == 10000.0f );
        CHECK( pInfo && pInfo->is_portrait() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_PageLayout_AddedToScore)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)(pageLayout (pageSize 14000 10000)(pageMargins 1000 1200 3000 2500 4000) landscape)(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        ImoPageInfo* pInfo = pScore->get_page_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_page_info() == true );
        CHECK( pInfo && pInfo->get_left_margin() == 1000.0f );
        CHECK( pInfo && pInfo->get_top_margin() == 1200.0f );
        CHECK( pInfo && pInfo->get_right_margin() == 3000.0f );
        CHECK( pInfo && pInfo->get_bottom_margin() == 2500.0f );
        CHECK( pInfo && pInfo->get_binding_margin() == 4000.0f );
        CHECK( pInfo && pInfo->get_page_width() == 14000.0f );
        CHECK( pInfo && pInfo->get_page_height() == 10000.0f );
        CHECK( pInfo && pInfo->is_portrait() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_PageLayout_AddedToDocument)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc (vers 0.0)(pageLayout (pageSize 14000 10000)(pageMargins 1000 1200 3000 2500 4000) landscape)(content))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        ImoPageInfo* pInfo = pDoc->get_page_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_page_info() == true );
        CHECK( pInfo && pInfo->get_left_margin() == 1000.0f );
        CHECK( pInfo && pInfo->get_top_margin() == 1200.0f );
        CHECK( pInfo && pInfo->get_right_margin() == 3000.0f );
        CHECK( pInfo && pInfo->get_bottom_margin() == 2500.0f );
        CHECK( pInfo && pInfo->get_binding_margin() == 4000.0f );
        CHECK( pInfo && pInfo->get_page_width() == 14000.0f );
        CHECK( pInfo && pInfo->get_page_height() == 10000.0f );
        CHECK( pInfo && pInfo->is_portrait() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // font -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Font)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(font \"Trebuchet\" 12pt bold)" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = static_cast<ImoFontStyleDto*>( pRoot );
        CHECK( pFont != nullptr );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_bold );
        CHECK( pFont->size == 12 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Font_StyleError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown font style 'grey'. Replaced by 'normal'." << endl;
        parser.parse_text("(font \"Trebuchet\" 8pt grey)" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = static_cast<ImoFontStyleDto*>( pRoot );
        CHECK( pFont != nullptr );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_normal );
        CHECK( pFont->size == 8 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Font_SizeError)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid size 'six'. Replaced by '12'." << endl;
        parser.parse_text("(font \"Trebuchet\" six bold)" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = static_cast<ImoFontStyleDto*>( pRoot );
        CHECK( pFont != nullptr );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_bold );
        CHECK( pFont->size == 12 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Font_SizeNew)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(font \"Trebuchet\" 17 normal)" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = static_cast<ImoFontStyleDto*>( pRoot );
        CHECK( pFont != nullptr );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_normal );
        CHECK( pFont->size == 17 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // defineStyle ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_DefineStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(font \"Times New Roman\" 14pt bold-italic) (color #00fe0f7f))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = static_cast<ImoStyle*>( pRoot );
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Composer" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle && pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle && pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle && pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle && pStyle->font_size() == 14 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_DefineStyle_StyleAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(score (vers 1.6)(defineStyle \"Header1\" (font \"Times New Roman\" 14pt bold-italic) (color #00fe0f7f))(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        ImoStyle* pStyle = pScore->find_style("Header1");
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle && pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle && pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle && pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle && pStyle->font_size() == 14 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, DefineStyle_MarginBottom)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(color #00fe0f7f)(margin-bottom 2) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = static_cast<ImoStyle*>( pRoot );
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Composer" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle && pStyle->margin_bottom() == 2.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, DefineStyle_FontProperties)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(font-name \"Arial\")(font-size 14pt)"
            "(font-style italic)(font-weight bold) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = static_cast<ImoStyle*>( pRoot );
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Composer" );
        CHECK( pStyle && pStyle->font_name() == "Arial" );
        CHECK( pStyle && pStyle->font_size() == 14 );
        CHECK( pStyle && pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle && pStyle->font_weight() == ImoStyle::k_font_weight_bold );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, DefineStyle_FontFile)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(font-name \"Arial\")(font-size 14pt)"
            "(font-style italic)(font-weight bold) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = static_cast<ImoStyle*>( pRoot );
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Composer" );
        CHECK( pStyle && pStyle->font_name() == "Arial" );
        CHECK( pStyle && pStyle->font_size() == 14 );
        CHECK( pStyle && pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle && pStyle->font_weight() == ImoStyle::k_font_weight_bold );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, DefineStyle_MarginProperties)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(margin-top 3)(margin-bottom 2)(margin-left 5)(margin-right 7) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = static_cast<ImoStyle*>( pRoot );
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Composer" );
        CHECK( pStyle && pStyle->margin_top() == 3.0f );
        CHECK( pStyle && pStyle->margin_bottom() == 2.0f );
        CHECK( pStyle && pStyle->margin_left() == 5.0f );
        CHECK( pStyle && pStyle->margin_right() == 7.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, DefineStyle_Margin)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(margin 0.5) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = static_cast<ImoStyle*>( pRoot );
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Composer" );
        CHECK( pStyle && pStyle->margin_top() == 0.5f );
        CHECK( pStyle && pStyle->margin_bottom() == 0.5f );
        CHECK( pStyle && pStyle->margin_left() == 0.5f );
        CHECK( pStyle && pStyle->margin_right() == 0.5f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, DefineStyle_LineHeight)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(line-height 1.2) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = static_cast<ImoStyle*>( pRoot );
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Composer" );
        CHECK( pStyle && pStyle->line_height() == 1.2f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    // title ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Title_MissingAll)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. title: missing mandatory element 'string'." << endl;
        parser.parse_text("(title)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Title_MissingString)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. title: missing mandatory element 'string'." << endl;
        parser.parse_text("(title center)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Title_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(title center \"Moonlight sonata\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreTitle* pTitle = static_cast<ImoScoreTitle*>( pRoot );
        CHECK( pTitle != nullptr );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_score_title)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(title#10 center \"Moonlight sonata\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_score_title() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Title_AddedToScore)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 1.6)(title center \"Moonlight sonata\")(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        std::list<ImoScoreTitle*>& titles = pScore->get_titles();
        std::list<ImoScoreTitle*>::iterator it = titles.begin();
        CHECK( it != titles.end() );
        ImoScoreTitle* pTitle = *it;
        CHECK( pTitle != nullptr );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Title_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(title center \"Moonlight sonata\" (style \"Header1\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreTitle* pTitle = static_cast<ImoScoreTitle*>( pRoot );
        CHECK( pTitle != nullptr );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );
        ImoStyle* pStyle = pTitle->get_style();
        CHECK( pStyle == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Title_StyleAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(score (vers 1.6)"
            "(defineStyle \"Header1\" (font \"Times New Roman\" 14pt bold-italic) (color #00fe0f7f))"
            "(title center \"Moonlight sonata\" (style \"Header1\"))"
            "(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore != nullptr );
        std::list<ImoScoreTitle*>& titles = pScore->get_titles();
        std::list<ImoScoreTitle*>::iterator it = titles.begin();
        CHECK( it != titles.end() );
        ImoScoreTitle* pTitle = *it;
        CHECK( pTitle != nullptr );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );
        ImoStyle* pStyle = pTitle->get_style();
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle && pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle && pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle && pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle && pStyle->font_size() == 14 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Title_Location)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(title right \"F. Chopin\" (style \"Composer\")(dy 30))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreTitle* pTitle = static_cast<ImoScoreTitle*>( pRoot );
        CHECK( pTitle != nullptr );
        CHECK( pTitle->get_text() == "F. Chopin" );
        CHECK( pTitle->get_h_align() == k_halign_right );
        CHECK( pTitle->get_user_location_x() == 0.0f );
        CHECK( pTitle->get_user_location_y() == 30.0f );
        ImoStyle* pStyle = pTitle->get_style();
        CHECK( pStyle == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // line -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Line_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))"
            "(endPoint (dx 80.0)(dy -10.0))(width 2.0)(color #ff0000)(lineStyle solid)"
            "(lineCapStart arrowhead)(lineCapEnd none))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pRoot );
        CHECK( pLine != nullptr );
        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
        CHECK( pLine->get_line_style() == k_line_solid );
        CHECK( pLine->get_start_edge() == k_edge_normal );
        CHECK( pLine->get_end_edge() == k_edge_normal );
        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
        CHECK( pLine->get_end_cap() == k_cap_none );
        CHECK( is_equal(pLine->get_color(), Color(255,0,0,255)) );
        CHECK( pLine->get_line_width() == 2.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Line_OnlyMandatory)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pRoot );
        CHECK( pLine != nullptr );
        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
        CHECK( pLine->get_line_style() == k_line_solid );
        CHECK( pLine->get_start_edge() == k_edge_normal );
        CHECK( pLine->get_end_edge() == k_edge_normal );
        CHECK( pLine->get_start_cap() == k_cap_none );
        CHECK( pLine->get_end_cap() == k_cap_none );
        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
        CHECK( pLine->get_line_width() == 1.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_score_line)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(line#10 (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_score_line() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Line_NoColor)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0))(width 2.0)(lineStyle solid)(lineCapStart arrowhead)(lineCapEnd diamond))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pRoot );
        CHECK( pLine != nullptr );
        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
        CHECK( pLine->get_line_style() == k_line_solid );
        CHECK( pLine->get_start_edge() == k_edge_normal );
        CHECK( pLine->get_end_edge() == k_edge_normal );
        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
        CHECK( pLine->get_end_cap() == k_cap_diamond );
        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
        CHECK( pLine->get_line_width() == 2.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Line_ErrorCap)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'lineCap': Invalid value 'diamont'. Replaced by 'none'." << endl;
        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0))(width 2.0)(lineStyle dot)(lineCapStart arrowhead)(lineCapEnd diamont))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pRoot );
        CHECK( pLine != nullptr );
        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
        CHECK( pLine->get_line_style() == k_line_dot );
        CHECK( pLine->get_start_edge() == k_edge_normal );
        CHECK( pLine->get_end_edge() == k_edge_normal );
        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
        CHECK( pLine->get_end_cap() == k_cap_none );
        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
        CHECK( pLine->get_line_width() == 2.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Line_ErrorLineStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Element 'lineStyle': Invalid value 'simple'. Replaced by 'solid'." << endl;
        parser.parse_text("(line (startPoint (dx 5.0)(dy 6.0))(endPoint (dx 80.0)(dy -10.0))(width 2.0)(lineStyle simple))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pRoot );
        CHECK( pLine != nullptr );
        CHECK( pLine->get_start_point() == TPoint(5.0f, 6.0f) );
        CHECK( pLine->get_end_point() == TPoint( 80.0f, -10.0f) );
        CHECK( pLine->get_line_style() == k_line_solid );
        CHECK( pLine->get_start_edge() == k_edge_normal );
        CHECK( pLine->get_end_edge() == k_edge_normal );
        CHECK( pLine->get_start_cap() == k_cap_none );
        CHECK( pLine->get_end_cap() == k_cap_none );
        CHECK( is_equal(pLine->get_color(), Color(0,0,0,255)) );
        CHECK( pLine->get_line_width() == 2.0f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // textBox --------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TextBox_Minimum)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. textbox: missing mandatory element 'dx'." << endl
        //         << "Line 0. textbox: missing mandatory element 'dy'." << endl;
        parser.parse_text("(textbox (dx 50)(dy 5)"
            "(size (width 300)(height 150))"
            "(text \"This is a test of a textbox\" (style \"Textbox\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoTextBox* pTB = static_cast<ImoTextBox*>( pRoot );
        CHECK( pTB != nullptr );
        CHECK( pTB->get_text() == "This is a test of a textbox" );
        CHECK( pTB->has_anchor_line() == false );
        ImoTextBlockInfo* pInfo = pTB->get_box_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_textblock_info() == true );
        CHECK( pInfo && pInfo->get_height() == 150.0f );
        CHECK( pInfo && pInfo->get_width() == 300.0f );
        CHECK( pInfo && pInfo->get_position() == TPoint(50.0f, 5.0f) );
        CHECK( is_equal(pInfo->get_bg_color(), Color(255,255,255,255)) );
        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,0,255)) );
        CHECK( pInfo && pInfo->get_border_width() == 1.0f );
        CHECK( pInfo && pInfo->get_border_style() == k_line_solid );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_text_box)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(textbox#10 (dx 50)(dy 5)"
            "(size (width 300)(height 150))"
            "(text \"This is a test of a textbox\" (style \"Textbox\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_text_box() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TextBox_Full)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. textbox: missing mandatory element 'dx'." << endl
        //         << "Line 0. textbox: missing mandatory element 'dy'." << endl;
        parser.parse_text("(textbox (dx 50)(dy 5)"
            "(size (width 300)(height 150))"
            "(color #fffe0b)"
            "(border (width 5)(lineStyle dot)(color #0000fd))"
            "(text \"This is a test of a textbox\" (style \"Textbox\"))"
            "(anchorLine (dx 40)(dy 70)(lineStyle dot)(color #ff0a00)(width 3.5)"
                        "(lineCapEnd arrowhead)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoTextBox* pTB = static_cast<ImoTextBox*>( pRoot );
        CHECK( pTB != nullptr );
        CHECK( pTB->get_text() == "This is a test of a textbox" );

        ImoTextBlockInfo* pInfo = pTB->get_box_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_textblock_info() == true );
        CHECK( pInfo && pInfo->get_height() == 150.0f );
        CHECK( pInfo && pInfo->get_width() == 300.0f );
        CHECK( pInfo && pInfo->get_position() == TPoint(50.0f, 5.0f) );
        CHECK( is_equal(pInfo->get_bg_color(), Color(255,254,11,255)) );
        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,253,255)) );
        CHECK( pInfo && pInfo->get_border_width() == 5.0f );
        CHECK( pInfo && pInfo->get_border_style() == k_line_dot );

        CHECK( pTB->has_anchor_line() == true );
        ImoLineStyle* pLine = pTB->get_anchor_line_info();
        CHECK( pLine != nullptr );
        CHECK( pLine->is_line_style() == true );
        CHECK( pLine->get_start_point() == TPoint(0.0f, 0.0f) );
        CHECK( pLine->get_end_point() == TPoint(40.0f, 70.0f) );
        CHECK( pLine->get_line_style() == k_line_dot );
        CHECK( pLine->get_start_edge() == k_edge_normal );
        CHECK( pLine->get_end_edge() == k_edge_normal );
        CHECK( pLine->get_start_cap() == k_cap_none );
        CHECK( pLine->get_end_cap() == k_cap_arrowhead );
        CHECK( is_equal(pLine->get_color(), Color(255,10,0,255)) );
        CHECK( pLine->get_width() == 3.5f );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TextBox_AddedToNote)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(n c4 q (textbox (dx 50)(dy 5)"
            "(size (width 300)(height 150))"
            "(text \"This is a test of a textbox\" (style \"Textbox\"))))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoNote* pNote = static_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        CHECK( pNote && pNote->has_attachments() == true );
        ImoTextBox* pTB = dynamic_cast<ImoTextBox*>( pNote->get_attachment(0) );
        CHECK( pTB != nullptr );
        CHECK( pTB->get_text() == "This is a test of a textbox" );

        ImoTextBlockInfo* pInfo = pTB->get_box_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->is_textblock_info() == true );
        CHECK( pInfo && pInfo->get_height() == 150.0f );
        CHECK( pInfo && pInfo->get_width() == 300.0f );
        CHECK( pInfo && pInfo->get_position() == TPoint(50.0f, 5.0f) );
        CHECK( is_equal(pInfo->get_bg_color(), Color(255,255,255,255)) );
        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,0,255)) );
        CHECK( pInfo && pInfo->get_border_width() == 1.0f );
        CHECK( pInfo && pInfo->get_border_style() == k_line_solid );

        CHECK( pTB->has_anchor_line() == false );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // cursor ---------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Cursor_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(cursor 1 2 64.0 34)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoCursorInfo* pInfo = static_cast<ImoCursorInfo*>( pRoot );
        CHECK( pInfo && pInfo->is_cursor_info() == true );
        CHECK( pInfo && pInfo->get_instrument() == 1 );
        CHECK( pInfo && pInfo->get_staff() == 2 );
        CHECK( pInfo && pInfo->get_time() == 64.0f );
        CHECK( pInfo && pInfo->get_id() == 34L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Cursor_AddedToScore)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. 'cursor' in score is obsolete. Now must be in 'lenmusdoc' element. Ignored." << endl;
        parser.parse_text("(score (vers 1.6)(cursor 1 2 64.0 34)(instrument (musicData)))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        CHECK( pScore && pScore->is_score() == true );
        //CHECK( pScore && pScore->get_instrument() == 1 );
        //CHECK( pScore && pScore->get_staff() == 2 );
        //CHECK( pScore && pScore->get_time() == 64.0f );
        //CHECK( pScore && pScore->get_id() == 34L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Cursor_AddedToDocument)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_libraryScope.ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. " << endl;
    //    parser.parse_text("(lenmusdoc (vers 0.0)"
    //        "(settings (cursor 1 2 64.0 34)) (content))");
    //    LdpAnalyser a(errormsg, m_libraryScope, &doc);
    //    ImoObj* pRoot = a.analyse_tree(tree, "string:");

    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );

    //    ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
    //    CHECK( pDoc && pDoc->is_document() == true );
    //    //CHECK( pScore && pScore->get_instrument() == 1 );
    //    //CHECK( pScore && pScore->get_staff() == 2 );
    //    //CHECK( pScore && pScore->get_time() == 64.0f );
    //    //CHECK( pScore && pScore->get_id() == 34L );

    //    delete tree->get_root();
    //    if (pRoot && !pRoot->is_document()) delete pRoot;
    //}

    // figuredBass ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_FiguredBass_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(figuredBass \"7 5 2\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFiguredBass* pFB = static_cast<ImoFiguredBass*>( pRoot );
        CHECK( pFB->is_figured_bass() == true );
        //cout << "FB ='" << pFB->get_figured_bass_string() << "'" << endl;
        CHECK( pFB->get_figured_bass_string() == "7 5 2" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // staff ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Staff_NoNumber)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid staff number. Staff info ignored." << endl;
        parser.parse_text("(staff (staffType ossia))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Staff_InvalidType)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid staff type 'bonito'. 'regular' staff assumed." << endl;
        parser.parse_text("(staff 2 (staffType bonito))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_staff_number() == 1 );
        CHECK( pInfo && pInfo->get_staff_type() == ImoStaffInfo::k_staff_regular );
        CHECK( pInfo && pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo && pInfo->get_line_spacing() == 180.0f );
        CHECK( pInfo && pInfo->get_height() == 735.0f );
        CHECK( pInfo && pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo && pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Staff_InvalidLines)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid staff. Num lines must be greater than zero. Five assumed." << endl;
        parser.parse_text("(staff 2 (staffType ossia)(staffLines 0))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_staff_number() == 1 );
        CHECK( pInfo && pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo && pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo && pInfo->get_line_spacing() == 180.0f );
        CHECK( pInfo && pInfo->get_height() == 735.0f );
        CHECK( pInfo && pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo && pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Staff_InvalidLinesSpacing)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Invalid real number 'five'. Replaced by '180'." << endl;
        parser.parse_text("(staff 2 (staffType ossia)(staffLines 5)"
            "(staffSpacing five) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_staff_number() == 1 );
        CHECK( pInfo && pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo && pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo && pInfo->get_line_spacing() == 180.0f );
        CHECK( pInfo && pInfo->get_height() == 735.0f );
        CHECK( pInfo && pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo && pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Staff_LinesSpacing)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(staff 2 (staffType ossia)(staffLines 5)"
            "(staffSpacing 200.0) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_staff_number() == 1 );
        CHECK( pInfo && pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo && pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo && pInfo->get_line_spacing() == 200.0f );
        CHECK( pInfo && pInfo->get_height() == 815.0f );
        CHECK( pInfo && pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo && pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Staff_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(staff 2 (staffType ossia)(staffLines 4)"
            "(staffSpacing 200.0)(staffDistance 800)(lineThickness 20.5) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>( pRoot );
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_staff_number() == 1 );
        CHECK( pInfo && pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo && pInfo->get_staff_margin() == 800.0f );
        CHECK( pInfo && pInfo->get_line_spacing() == 200.0f );
        CHECK( pInfo && pInfo->get_height() == 620.5f );
        CHECK( pInfo && pInfo->get_line_thickness() == 20.5f );
        CHECK( pInfo && pInfo->get_num_lines() == 4 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Staff_AddedToInstrument)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(instrument (staves 2)(staff 2 (staffType ossia)"
            "(staffLines 4)(staffSpacing 200.0)(staffDistance 800)(lineThickness 20.5))"
            "(musicData))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot != nullptr );
        ImoInstrument* pInstr = static_cast<ImoInstrument*>( pRoot );
        CHECK( pInstr != nullptr );
        ImoStaffInfo* pInfo = pInstr->get_staff(1);
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->get_staff_number() == 1 );
        CHECK( pInfo && pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo && pInfo->get_staff_margin() == 800.0f );
        CHECK( pInfo && pInfo->get_line_spacing() == 200.0f );
        CHECK( pInfo && pInfo->get_height() == 620.5f );
        CHECK( pInfo && pInfo->get_line_thickness() == 20.5f );
        CHECK( pInfo && pInfo->get_num_lines() == 4 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // textItem -------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, TextItem)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(txt \"This is a text\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_text_item() == true );
        ImoTextItem* pText = static_cast<ImoTextItem*>( pRoot );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "This is a text" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_text_item)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(txt#10 \"This is a text\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_text_item() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //TEST_FIXTURE(LdpAnalyserTestFixture, TextItem_MissingText)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_libraryScope.ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. text: missing mandatory element 'string'." << endl;
    //    parser.parse_text("(text)");
    //    LdpAnalyser a(errormsg, m_libraryScope, &doc);
    //    ImoObj* pRoot = a.analyse_tree(tree, "string:");
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImoTextItem* pText = static_cast<ImoTextItem*>( pRoot );
    //    CHECK( pText == nullptr );

    //    delete tree->get_root();
    //    if (pRoot && !pRoot->is_document()) delete pRoot;
    //}

    TEST_FIXTURE(LdpAnalyserTestFixture, TextItem_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(txt (style \"Header1\") \"This is a text\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoTextItem* pText = static_cast<ImoTextItem*>( pRoot );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "This is a text" );
        ImoStyle* pStyle = pText->get_style();
        CHECK( pStyle == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, TextItem_DefaultStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope, errormsg);
        stringstream expected;
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(para (txt \"Hello world!\")) ))");

        ImoDocument* pDoc = doc.get_im_root();
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != nullptr );
        if (pPara)
        {
            CHECK( pPara->get_num_items() == 1 );
            ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
            CHECK( pItem && pItem->get_text() == "Hello world!" );
            CHECK( pItem && pItem->get_style() != nullptr );
        }
    }

    //TEST_FIXTURE(LdpAnalyserTestFixture, TextItem_Location)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_libraryScope.ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. " << endl;
    //    parser.parse_text("(text \"F. Chopin\" (style \"Composer\")(dy 30)(dx 20))");
    //    LdpAnalyser a(errormsg, m_libraryScope, &doc);
    //    ImoObj* pRoot = a.analyse_tree(tree, "string:");

    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );

    //    ImoTextItem* pText = static_cast<ImoTextItem*>( pRoot );
    //    CHECK( pText != nullptr );
    //    CHECK( pText && pText->get_text() == "F. Chopin" );
    //    CHECK( pText && pText->get_user_location_x() == 20.0f );
    //    CHECK( pText && pText->get_user_location_y() == 30.0f );
    //    ImoStyle* pStyle = pText->get_style();
    //    CHECK( pStyle == nullptr );

    //    delete tree->get_root();
    //    if (pRoot && !pRoot->is_document()) delete pRoot;
    //}

    // para -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(para (txt \"This is a paragraph\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_paragraph() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_paragraph)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(para#10 (txt \"This is a paragraph\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_paragraph() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_TextItemAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(para (txt \"This is a paragraph\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = static_cast<ImoParagraph*>( pRoot );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem && pItem->get_text() == "This is a paragraph" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_LinkItemAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(para (link (url \"This is the url\")(txt \"This is the link\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = static_cast<ImoParagraph*>( pRoot );
        CHECK( pPara->get_num_items() == 1 );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pPara->get_first_item() );
        CHECK( pLink != nullptr );
        CHECK( pLink && pLink->get_url() == "This is the url" );
        CHECK( pLink && pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem && pItem->get_text() == "This is the link" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_ManyItems)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(para (txt \"This is a paragraph\")"
            "(txt \" with two items.\") )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = static_cast<ImoParagraph*>( pRoot );
        CHECK( pPara->get_num_items() == 2 );
        TreeNode<ImoObj>::children_iterator it = pPara->begin();
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem && pItem->get_text() == "This is a paragraph" );
        ++it;
        pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem && pItem->get_text() == " with two items." );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_RecognizedAsContent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(para (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != nullptr );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem && pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(para (style \"Paragraph\") (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != nullptr );
        if (pPara)
        {
            ImoStyle* pStyle = pPara->get_style();
            CHECK( pStyle != nullptr );
            CHECK( pPara->get_num_items() == 1 );
            ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
            CHECK( pItem && pItem->get_text() == "Hello world!" );
        }

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_DefaultStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope, errormsg);
        stringstream expected;
        //expected << "Line 0. " << endl;
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(para (txt \"Hello world!\")) ))");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = doc.get_im_root();
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != nullptr );
        ImoStyle* pStyle = pPara->get_style();
        CHECK( pStyle && pStyle->get_name() == "Paragraph" );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem && pItem->get_text() == "Hello world!" );
    }

    // heading --------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(heading 1 (txt \"This is a header\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_heading() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_heading)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(heading#10 1 (txt \"This is a header\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_heading() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_TextItemAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(heading 1 (txt \"This is a header\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pRoot );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem && pItem->get_text() == "This is a header" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_ManyItems)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(heading 1 (txt \"This is a header\")"
            "(txt \" with two items.\") )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pRoot );
        CHECK( pH->get_num_items() == 2 );
        TreeNode<ImoObj>::children_iterator it = pH->begin();
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem && pItem->get_text() == "This is a header" );
        ++it;
        pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem && pItem->get_text() == " with two items." );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_RecognizedAsContent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(heading 1 (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != nullptr );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem && pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(heading 1 (style \"Heading\") (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != nullptr );
        ImoStyle* pStyle = pH->get_style();
        CHECK( pStyle != nullptr );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem && pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_DefaultStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope, errormsg);
        stringstream expected;
        //expected << "Line 0. " << endl;
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(heading 1 (txt \"Hello world!\")) ))");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = doc.get_im_root();
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != nullptr );
        if (pH)
        {
            ImoStyle* pStyle = pH->get_style();
            CHECK( pStyle && pStyle->get_name() == "Heading-1" );
            CHECK( pH->get_num_items() == 1 );
            ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
            CHECK( pItem && pItem->get_text() == "Hello world!" );
        }
    }

    // styles ---------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Styles)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(styles (defineStyle \"Header1\" "
            "(font \"Times New Roman\" 14pt bold-italic) (color #00fe0f7f)) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pRoot );
        CHECK( pStyles != nullptr );

        ImoStyle* pStyle = pStyles->find_style("Header1");
        CHECK( pStyle && pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle && pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle && pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle && pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle && pStyle->font_size() == 14 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Styles_AddedToDocument)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) "
            "(styles (defineStyle \"Header1\" "
                "(font \"Times New Roman\" 14pt bold-italic) (color #00fe0f7f)) )"
            "(content (text \"hello world\")) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(0) );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "hello world" );

        ImoStyle* pStyle = pDoc->find_style("Header1");
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle && pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle && pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle && pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle && pStyle->font_size() == 14 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Styles_Default)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) "
            "(content (text \"hello world\")) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(0) );
        CHECK( pText != nullptr );
        CHECK( pText && pText->get_text() == "hello world" );

        ImoStyle* pStyle = pDoc->get_style_or_default("text");
        CHECK( pStyle != nullptr );
        CHECK( pStyle && pStyle->get_name() == "Default style" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // param ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, ParamInfo_Ok)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(param green \"this is green\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        CHECK( pRoot->is_param_info() == true );
        ImoParamInfo* pParam = dynamic_cast<ImoParamInfo*>( pRoot );
        CHECK( pParam != nullptr );
        CHECK( pParam->get_name() == "green" );
        CHECK( pParam->get_value() == "this is green" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, ParamInfo_MissingName)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing name for element 'param' (should be a label). Element ignored." << endl;
        parser.parse_text("(param \"green\" \"this is green\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // dynamic --------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Dynamic_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(dynamic (classid test))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_dynamic() == true );
        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pRoot );
        CHECK( pDyn != nullptr );
        CHECK( pDyn->get_classid() == "test" );
        CHECK( pDyn->is_visible() );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_dynamic)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(dynamic#10 (classid test))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_dynamic() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Dynamic_AddedToContent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0)(content "
            "(dynamic (classid test)) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pDoc->get_content_item(0) );
        CHECK( pDyn != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Dynamic_GeneratesRequest)
    {
        LomseDoorway* pDoorway = m_libraryScope.platform_interface();
        pDoorway->set_request_callback(static_cast<LdpAnalyserTestFixture*>(this), wrapper_lomse_request);

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content "
            "(dynamic (classid test)) ))");

        CHECK( m_fRequestReceived == true );
        CHECK( m_requestType == k_dynamic_content_request );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Dynamic_WithParams)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(dynamic (classid test)"
            "(param play \"all notes\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pRoot );
        CHECK( pDyn->get_classid() == "test" );
        std::list<ImoParamInfo*>& params = pDyn->get_params();
        CHECK( params.size() == 1 );
        ImoParamInfo* pParm = params.front();
        CHECK( pParm->get_name() == "play" );
        CHECK( pParm->get_value() == "all notes" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // link -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Link_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(link (url \"#TheoryHarmony_ch3.lms\")(txt \"Harmony exercise\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_link() == true );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pRoot );
        CHECK( pLink != nullptr );
        CHECK( pLink && pLink->get_url() == "#TheoryHarmony_ch3.lms" );
        CHECK( pLink && pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem && pItem->get_text() == "Harmony exercise" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_link)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(link#10 (url \"#TheoryHarmony_ch3.lms\")(txt \"Harmony exercise\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_link() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Link_MissingUrl)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. link: missing mandatory element 'url'." << endl;
        parser.parse_text(
            "(link (txt \"Harmony exercise\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_link() == true );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pRoot );
        CHECK( pLink != nullptr );
        if (pLink)
        {
            CHECK( pLink->get_url() == "" );
            CHECK( pLink->get_num_items() == 1 );
            ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
            CHECK( pItem && pItem->get_text() == "Harmony exercise" );
        }

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

#if (LOMSE_ENABLE_PNG == 1)
    // image ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Image_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(image (file \"test-image-1.png\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, m_scores_path);

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoImage* pImg = dynamic_cast<ImoImage*>( pRoot );
        CHECK( pImg != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }
#endif // LOMSE_ENABLE_PNG

    // list -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Listitem_created)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. link: missing mandatory element 'url'." << endl;
        parser.parse_text(
            "(listitem (txt \"This is the first item\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_listitem() == true );
        ImoListItem* pLI = dynamic_cast<ImoListItem*>( pRoot );
        CHECK( pLI->get_num_content_items() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText && pText->get_text() == "This is the first item" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_listitem)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(listitem#10 (txt \"This is the first item\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_listitem() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, List_created)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. link: missing mandatory element 'url'." << endl;
        parser.parse_text(
            "(itemizedlist (listitem (txt \"This is the first item\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_list() == true );
        ImoList* pList = dynamic_cast<ImoList*>( pRoot );
        CHECK( pList != nullptr );
        CHECK( pList->get_list_type() == ImoList::k_itemized );
        CHECK( pList->get_num_content_items() == 1 );
        ImoListItem* pLI = pList->get_list_item(0);
        CHECK( pLI->get_num_content_items() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText && pText->get_text() == "This is the first item" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_list)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(itemizedlist#10 (listitem (txt \"This is the first item\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_list() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

#if LOMSE_COMPATIBILITY_LDP_1_5
    // graphic line  --------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, graphic_type_error)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Unknown type 'circle'. Element 'graphic' ignored." << endl;
        parser.parse_text("(graphic circle 0.0 0.67)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, graphic_type_ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(graphic line 0.0 7.0 17.0 3.5)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pRoot->is_score_line() == true );
        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pRoot );
        CHECK( pLine != nullptr );
        CHECK( pLine->get_x_start() == 0.0f );
        CHECK( pLine->get_y_start() == 7.0f );
        CHECK( pLine->get_x_end() == 17.0f );
        CHECK( pLine->get_y_end() == 3.5f );
        CHECK( pLine->get_line_width() == 1.0f );
        CHECK( pLine->get_start_edge() == k_edge_normal );
        CHECK( pLine->get_line_style() == k_line_solid );
        CHECK( pLine->get_start_cap() == k_cap_arrowhead );
        CHECK( pLine->get_end_cap() == k_cap_none );
        Color color = pLine->get_color();
        CHECK( color.r == 0 );
        CHECK( color.g == 0 );
        CHECK( color.b == 0 );
        CHECK( color.a == 255 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_graphic_type)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(graphic#10 line 0.0 7.0 17.0 3.5)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_score_line() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, graphic_is_anchored)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData "
            "(n c4 q)(graphic line 0.0 7.0 17.0 3.5) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = static_cast<ImoMusicData*>( pRoot );
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != nullptr );

        ++it;
        ImoDirection* pSpacer = dynamic_cast<ImoDirection*>( *it );
        CHECK( pSpacer != nullptr );
        ImoAttachments* pAuxObjs = pSpacer->get_attachments();
        CHECK( pAuxObjs != nullptr );
        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pAuxObjs->get_item(0) );
        CHECK( pLine != nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }
#endif

    // scorePlayer ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, scorePlayer_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope, errormsg);
        stringstream expected;
        //expected << "Line 0. " << endl;
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer) ))");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = doc.get_im_root();
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        CHECK( pAB != nullptr );
        if (pAB)
        {
            ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
            CHECK( pSP && pSP->is_score_player() == true );
            CHECK( pSP && pSP->get_metronome_mm() == 60 );
            //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;
        }
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_score_player)
    {
        stringstream errormsg;
        Document doc(m_libraryScope, errormsg);
        stringstream expected;
        //expected << "" << endl;
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer#10) ))");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = doc.get_im_root();
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        CHECK( pAB != nullptr );
        if (pAB)
        {
            ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
            CHECK( pSP && pSP->get_id() == 10L );
        }
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, scorePlayer_metronome)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        stringstream expected;
        //expected << "Line 0. " << endl;
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer (mm 65)) ))");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = doc.get_im_root();
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        CHECK( pAB != nullptr );
        if (pAB)
        {
            ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
            CHECK( pSP && pSP->is_score_player() == true );
            CHECK( pSP && pSP->get_metronome_mm() == 65 );
            //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;
        }
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, scorePlayer_label_play)
    {
        stringstream errormsg;
        Document doc(m_libraryScope, errormsg);
        stringstream expected;
        //expected << "Line 0. " << endl;
        doc.from_string("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer (mm 65)(playLabel \"Tocar\")(stopLabel \"Parar\")) ))");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = doc.get_im_root();
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        CHECK( pAB != nullptr );
        if (pAB)
        {
            ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
            CHECK( pSP && pSP->is_score_player() == true );
            CHECK( pSP && pSP->get_metronome_mm() == 65 );
            CHECK( pSP && pSP->get_play_label() == "Tocar" );
            CHECK( pSP && pSP->get_stop_label() == "Parar" );
            //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;
        }
    }

    // tableCell ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, tableCell_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(tableCell (txt \"This is a cell\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pRoot );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 1 );
        CHECK( pCell->get_colspan() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText && pText->get_text() == "This is a cell" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_table_cell)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(tableCell#10 (txt \"This is a cell\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_table_cell() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, tableCell_rowspan)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(tableCell (rowspan 2)(txt \"This is a cell\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pRoot );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 2 );
        CHECK( pCell->get_colspan() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText && pText->get_text() == "This is a cell" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, tableCell_colspan)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(tableCell (colspan 2)(txt \"This is a cell\"))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pRoot );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 1 );
        CHECK( pCell->get_colspan() == 2 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText && pText->get_text() == "This is a cell" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // tableRow -------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, tableRow_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(tableRow (tableCell (txt \"This is cell 1\"))"
            "          (tableCell (txt \"This is cell 2\"))"
            ")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pRoot );
        CHECK( pRow->is_table_row() == true );
        CHECK( pRow->get_num_cells() == 2 );
        ImoTableCell* pImo = dynamic_cast<ImoTableCell*>( pRow->get_cell(0) );
        CHECK( pImo && pImo->is_table_cell() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_table_row)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(tableRow#10 (tableCell (txt \"This is cell 1\"))"
            "          (tableCell (txt \"This is cell 2\"))"
            ")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_table_row() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    // tableHead ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, tableHead_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(tableHead (tableRow (tableCell (txt \"This is a cell\")) )"
            "           (tableRow (tableCell (txt \"This is a cell\")) )"
            ")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableHead* pHead = dynamic_cast<ImoTableHead*>( pRoot );
        CHECK( pHead->is_table_head() == true );
        CHECK( pHead->get_num_items() == 2 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
        CHECK( pRow->is_table_row() == true );
        CHECK( pRow->get_num_cells() == 1 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_table_head)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(tableHead#10 (tableRow (tableCell (txt \"This is a cell\")) )"
            "           (tableRow (tableCell (txt \"This is a cell\")) )"
            ")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_table_head() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    // tableBody ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, tableBody_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(tableBody (tableRow (tableCell (txt \"This is a cell\")) )"
            "           (tableRow (tableCell (txt \"This is a cell\")) )"
            ")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableBody* pBody = dynamic_cast<ImoTableBody*>( pRoot );
        CHECK( pBody->is_table_body() == true );
        CHECK( pBody->get_num_items() == 2 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_table_body)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(tableBody#10 (tableRow (tableCell (txt \"This is a cell\")) )"
            "           (tableRow (tableCell (txt \"This is a cell\")) )"
            ")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_table_body() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    // table ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, table_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(table (tableBody (tableRow (tableCell (txt \"This is a cell\")) )))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pRoot );
        CHECK( pTable != nullptr );

        CHECK( pTable && pTable->get_head() == nullptr );

        ImoTableBody* pBody = pTable->get_body();
        CHECK( pBody != nullptr );
        CHECK( pBody->get_num_items() == 1 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_table)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "(table#10 (tableBody (tableRow (tableCell (txt \"This is a cell\")) )))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pRoot->is_table() == true );
        ImoObj* pImo = pRoot;
        CHECK( pImo && pImo->get_id() == 10L );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }


    // tableColumn ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, tableColumn_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(lenmusdoc (vers 0.0)"
            "(styles"
            "   (defineStyle \"table1-col1\" (width 70))"
            "   (defineStyle \"table1-col2\" (width 80))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableBody"
                    "(tableRow (tableCell (txt \"This is a cell\")) )"
                ")"
            ")) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
        CHECK( pTable != nullptr );

        std::list<ImoStyle*>& cols = pTable->get_column_styles();
        CHECK( cols.size() == 2 );
        std::list<ImoStyle*>::iterator it = cols.begin();
        CHECK( (*it)->get_name() == "table1-col1" );
        ++it;
        CHECK( (*it)->get_name() == "table1-col2" );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, table_full_table)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(lenmusdoc (vers 0.0)"
            "(styles"
            "   (defineStyle \"table1-col1\" (width 70))"
            "   (defineStyle \"table1-col2\" (width 80))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableHead (tableRow"
                    "(tableCell (txt \"This is head cell 1\"))"
                    "(tableCell (txt \"This is head cell 2\"))"
                "))"
                "(tableBody (tableRow"
                    "(tableCell (txt \"This is body cell 1\"))"
                    "(tableCell (txt \"This is body cell 2\"))"
                "))"
            ")) )");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = static_cast<ImoDocument*>( pRoot );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
        CHECK( pTable != nullptr );

        std::list<ImoStyle*>& cols = pTable->get_column_styles();
        CHECK( cols.size() == 2 );

        ImoTableHead* pHead = pTable->get_head();
        CHECK( pHead != nullptr );
        CHECK( pHead->get_num_items() == 1 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        ImoTableBody* pBody = pTable->get_body();
        CHECK( pBody != nullptr );
        CHECK( pBody->get_num_items() == 1 );
        pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    //@ measures ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, measures_001)
    {
        //@001. Score ends in barline. MeasuresInfo in Barline but not in Instrument

        stringstream errormsg;
        stringstream expected;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        parser.parse_text("(score (vers 2.0)"
            "(instrument (musicData "
            "(clef G)(key D)(time 2 4)(n c4 q)(n e4 q)(barline)"
            ")))"
        );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_last_measure_info() == nullptr );
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();  //clef
        CHECK( (*it)->is_clef() );
        ++it;   //key
        CHECK( (*it)->is_key_signature() );
        ++it;   //TS
        CHECK( (*it)->is_time_signature() );
        ++it;   //n c4
        CHECK( (*it)->is_note() );
        ++it;   //n eq
        CHECK( (*it)->is_note() );
        ++it;   //barline
        CHECK( (*it)->is_barline() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( *it );
        CHECK( pBarline != nullptr );
        CHECK( pBarline && pBarline->get_type() == k_barline_simple );
        CHECK( pBarline && pBarline->is_visible() );
        TypeMeasureInfo* pInfo = pBarline->get_measure_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->count == 1 );
//        cout << test_name() << ": count=" << pInfo->count << endl;

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, measures_002)
    {
        //@002. Score does not end in barline. MeasuresInfo in Barline and Instrument

        stringstream errormsg;
        stringstream expected;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        parser.parse_text("(score (vers 2.0)"
            "(instrument (musicData "
            "(clef G)(key D)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n g4 q)"
            ")))"
        );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
        CHECK( pInfo != nullptr );
        CHECK( pInfo && pInfo->count == 2 );
//        cout << test_name() << ": count=" << pInfo->count << endl;

        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != nullptr );
        ImoObj::children_iterator it = pMusic->begin();  //clef
        CHECK( (*it)->is_clef() );
        ++it;   //key
        CHECK( (*it)->is_key_signature() );
        ++it;   //TS
        CHECK( (*it)->is_time_signature() );
        ++it;   //n c4
        CHECK( (*it)->is_note() );
        ++it;   //n eq
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

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, measures_003)
    {
        //@003. Empty score. No MeasuresInfo in Instrument

        stringstream errormsg;
        stringstream expected;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        parser.parse_text("(score (vers 2.0)"
            "(instrument (musicData "
            ")))"
        );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        TypeMeasureInfo* pInfo = pInstr->get_last_measure_info();
        CHECK( pInfo == nullptr );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }

    // other test to fix detected errors ------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, staff_reset_when_new_instrument)
    {
        //current saved values (staff number reset when new instrument
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "(score (vers 1.6)"
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key D)(time 4 4)(n c4 e p1)(n e4 s)(goBack start)"
            "(n g3 q p2)(n e3 e)(goFwd 160)(barline)"
            "))"
            "(instrument (musicData (clef C3)(key D)(time 4 4)"
            "(n c4 h.)(n e4 s)(goFwd 48)(barline)"
            "))"
            ")"
        );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pRoot = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = static_cast<ImoScore*>( pRoot );
        ImoInstrument* pInstr = pScore->get_instrument(1);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoClef* pClef = dynamic_cast<ImoClef*>( pMD->get_child(0) );
        CHECK( pClef->get_clef_type() == k_clef_C3 );
        CHECK( pClef->get_staff() == 0 );

        delete tree->get_root();
        // coverity[check_after_deref]
        if (pRoot && !pRoot->is_document()) delete pRoot;
    }
}

