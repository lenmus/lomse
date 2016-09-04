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
    ImoDocument* m_pDoc;


    LdpAnalyserTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_requestType(k_null_request)
        , m_fRequestReceived(false)
        , m_pDoc(NULL)
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( pIModel->get_root() != NULL );
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << score->get_root()->to_string() << endl;
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( pIModel->get_root() != NULL );
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( pIModel->get_root() != NULL );
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->get_version_string() == "1.6" );
        CHECK( pScore->get_num_instruments() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << score->get_root()->to_string() << endl;
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        //CHECK( pScore->get_num_instruments() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(1);
        CHECK( pScore->get_instr_number_for(pInstr) == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_articulation() == true );
        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_placement() == k_placement_default );
        CHECK( pImo->get_articulation_type() == k_articulation_tenuto );

        delete tree->get_root();
        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pIModel->get_root() );
//        CHECK( pImo != NULL );
//        CHECK( pImo->get_placement() == k_placement_default );
//        CHECK( pImo->get_mark_type() == "ppp" );
//        CHECK( pImo->get_id() == 30L );
//
//        delete tree->get_root();
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pIModel->get_root() );
//        CHECK( pImo != NULL );
//        CHECK( pImo->get_placement() == k_placement_above );
//        CHECK( pImo->get_mark_type() == "sf" );
//
//        delete tree->get_root();
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pIModel->get_root() );
//        CHECK( pImo != NULL );
//        CHECK( pImo->get_placement() == k_placement_above );
//        CHECK( pImo->get_mark_type() == "sf" );
//        CHECK( pImo->get_user_location_x() == 70.0f );
//        CHECK( pImo->get_user_location_y() == 0.0f );
//
//        delete tree->get_root();
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
//        CHECK( pNote != NULL );
//        CHECK( pNote->is_stem_up() == true );
//        CHECK( pNote->get_num_attachments() == 1 );
//        ImoArticulationSymbol* pImo = dynamic_cast<ImoArticulationSymbol*>( pNote->get_attachment(0) );
//        CHECK( pImo != NULL );
//        CHECK( pImo->get_placement() == k_placement_above );
//        CHECK( pImo->get_mark_type() == "sf" );
//        CHECK( pImo->get_user_location_x() == 70.0f );
//        CHECK( pImo->get_user_location_y() == 0.0f );
////        cout << "[" << errormsg.str() << "]" << endl;
////        cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        delete tree->get_root();
//        delete pIModel;
//    }

    //@ barline --------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_OptionalElementMissing)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_simple );
        CHECK( pBarline->is_visible() );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_OptionalElementPresent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->is_middle() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_barline() == true );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_simple );
        CHECK( pBarline->is_visible() );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_Visible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (visible yes))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_Middle)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double middle)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->is_middle() == true );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->get_user_location_x() == 70.0f );
        CHECK( pBarline->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_LocationY)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (dy 60.5))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->get_user_location_x() == 0.0f );
        CHECK( pBarline->get_user_location_y() == 60.5f );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_LocationXY)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (dx 70)(dy 20.3))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->get_user_location_x() == 70.0f );
        CHECK( pBarline->get_user_location_y() == 20.3f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->get_user_location_x() == 0.0f );
        CHECK( pBarline->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->get_user_location_x() == 0.0f );
        CHECK( pBarline->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( pBarline->get_user_location_x() == 20.3f );
        CHECK( pBarline->get_user_location_y() == 70.0f );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_HasId)
    {
//        stringstream errormsg;
//        Document doc(m_libraryScope);
//        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
//        stringstream expected;
//        //expected << "Line 0. " << endl;
//        parser.parse_text("(barline#7 double)");
//        LdpAnalyser a(errormsg, m_libraryScope, &doc);
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
//        CHECK( pBarline != NULL );
//        CHECK( pBarline->get_id() == 7 );
//
//        delete tree->get_root();
//        delete pIModel;
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline#7 double)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpElement* pBarline = tree->get_root();

        CHECK( pBarline != NULL );
        CHECK( pBarline->get_id() == 7 );

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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_music_data() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
    }

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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
    }

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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_music_data() == true );
        ImoMusicData* pImo = static_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pImo->get_id() == 12L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstr != NULL );
        ImoMusicData* pMD = pInstr->get_musicdata();
        CHECK( pMD != NULL );
        CHECK( pMD->get_instrument() == pInstr );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_note() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_sharp );
        CHECK( pNote->get_dots() == 1 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 3 );
        CHECK( pNote->get_step() == k_step_D );
        CHECK( pNote->get_duration() == 48.0f );
        CHECK( pNote->is_in_chord() == false );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_quarter );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_quarter );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_quarter );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->get_staff() == 6 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_staff() == 0 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->get_voice() == 3 );
        CHECK( pNote->is_tied_next() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_voice() == 1 );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_note() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote->get_dots() == 0 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->get_voice() == 3 );
        CHECK( pNote->has_attachments() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pAnalyser->analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );
        delete pAnalyser;
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        ImoTie* pTie = pNote1->get_tie_next();
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pTie->get_start_note() == pNote1 );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pTie->get_end_note() == pNote2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_tied_next() == false );
        CHECK( pNote1->is_tied_prev() == false );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it;
        int numNotes;
        for(numNotes=0, it=pMusic->begin(); it != pMusic->end(); ++it, ++numNotes)
            CHECK( (*it)->is_note() == true );
        CHECK( numNotes == 3 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->is_stem_up() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->is_stem_down() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->is_stem_default() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );
        CHECK( pNote->get_stem_direction() == k_stem_default );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pNote1->is_stem_down() == true );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pNote2->is_stem_up() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_note() == true );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_notated_accidentals() == k_sharp );
        CHECK( pNote->get_dots() == 1 );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 3 );
        CHECK( pNote->get_step() == k_step_D );
        CHECK( pNote->get_duration() == 48.0f );
        CHECK( pNote->get_staff() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        ImoTie* pTie = pNote1->get_tie_next();
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pTie->get_start_note() == pNote1 );
        ImoRelations* pRelObjs = pNote1->get_relations();
        CHECK( pRelObjs != NULL );
        CHECK( pRelObjs->get_item(0) == pTie );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pTie->get_end_note() == pNote2 );
        pRelObjs = pNote2->get_relations();
        CHECK( pRelObjs != NULL );
        CHECK( pRelObjs->get_item(0) == pTie );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_tied_next() == false );
        CHECK( pNote1->is_tied_prev() == false );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        ImoTie* pTie = pNote1->get_tie_next();
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pTie->get_start_note() == pNote1 );

        ++it;
        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pTie->get_end_note() == pNote2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        ImoTie* pTie = pNote1->get_tie_next();
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        CHECK( pTie->get_start_note() == pNote1 );
        CHECK( pTie->is_tie() == true );

        ++it;
        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == false );
        CHECK( pNote2->is_tied_prev() == true );
        CHECK( pTie->get_end_note() == pNote2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_tied_next() == true );
        CHECK( pNote1->is_tied_prev() == false );
        ImoTie* pTie1 = pNote1->get_tie_next();
        CHECK( pTie1->get_start_note() == pNote1 );
        ImoRelations* pRelObjs = pNote1->get_relations();
        CHECK( pRelObjs != NULL );
        CHECK( pRelObjs->get_item(0) == pTie1 );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_tied_next() == true );
        CHECK( pNote2->is_tied_prev() == true );
        ImoTie* pTie2 = pNote2->get_tie_next();
        CHECK( pTie1->get_end_note() == pNote2 );
        CHECK( pTie2->get_start_note() == pNote2 );
        pRelObjs = pNote2->get_relations();
        CHECK( pRelObjs != NULL );
        CHECK( pRelObjs->get_item(0) == pTie1 );
        CHECK( pRelObjs->get_item(1) == pTie2 );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != NULL );
        CHECK( pNote3->is_tied_next() == false );
        CHECK( pNote3->is_tied_prev() == true );
        CHECK( pTie2->get_end_note() == pNote3 );
        pRelObjs = pNote3->get_relations();
        CHECK( pRelObjs != NULL );
        CHECK( pRelObjs->get_item(0) == pTie2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_tie_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == false );
        CHECK( pInfo->get_tie_number() == 12 );
        CHECK( pInfo->get_note() == NULL );
        CHECK( pInfo->get_bezier() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == true );
        CHECK( pInfo->get_tie_number() == 15 );
        CHECK( pInfo->get_note() == NULL );
        CHECK( pInfo->get_bezier() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == true );
        CHECK( pInfo->get_tie_number() == 15 );
        CHECK( pInfo->get_note() == NULL );
        ImoBezierInfo* pBezier = pInfo->get_bezier();
        CHECK( pBezier != NULL );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 36.765f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == -25.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_tie_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTieDto* pInfo = dynamic_cast<ImoTieDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == false );
        CHECK( pInfo->get_tie_number() == 12 );
        CHECK( pInfo->get_note() == NULL );
        CHECK( pInfo->get_bezier() == NULL );
        CHECK( is_equal(pInfo->get_color(), Color(0,255,0,255)) );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_bezier_info() == true );
        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pIModel->get_root() );
        CHECK( pBezier != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pIModel->get_root() );
        CHECK( pBezier != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_bezier_info() == true );
        ImoBezierInfo* pBezier = dynamic_cast<ImoBezierInfo*>( pIModel->get_root() );
        CHECK( pBezier != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_slur_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == false );
        CHECK( pInfo->get_slur_number() == 12 );
        CHECK( pInfo->get_note() == NULL );
        CHECK( pInfo->get_bezier() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == true );
        CHECK( pInfo->get_slur_number() == 15 );
        CHECK( pInfo->get_note() == NULL );
        CHECK( pInfo->get_bezier() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        //TODO
        //CHECK( pInfo->is_continue() == true );
        CHECK( pInfo->get_slur_number() == 15 );
        CHECK( pInfo->get_note() == NULL );
        CHECK( pInfo->get_bezier() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == true );
        CHECK( pInfo->get_slur_number() == 27 );
        CHECK( pInfo->get_note() == NULL );
        ImoBezierInfo* pBezier = pInfo->get_bezier();
        CHECK( pBezier != NULL );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_start).y == 36.765f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_end).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).x == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol1).y == 0.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).x == -25.0f );
        CHECK( pBezier->get_point(ImoBezierInfo::k_ctrol2).y == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_slur_dto() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSlurDto* pInfo = dynamic_cast<ImoSlurDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start() == true );
        CHECK( pInfo->get_slur_number() == 12 );
        CHECK( pInfo->get_note() == NULL );
        CHECK( pInfo->get_bezier() == NULL );
        CHECK( is_equal(pInfo->get_color(), Color(0,255,0,255)) );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_rest() == true );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
        CHECK( pRest != NULL );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->has_attachments() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
        CHECK( pRest != NULL );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->get_staff() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
        CHECK( pRest != NULL );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->get_staff() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        LdpTree::iterator it = tree->begin();
        ++it;
        ImoRest* pRest = dynamic_cast<ImoRest*>( (*it)->get_imo() );
        CHECK( pRest != NULL );
        CHECK( pRest->get_staff() == 1 );
        ++it;
        ++it;
        ++it;
        ImoNote* pNote = dynamic_cast<ImoNote*>( (*it)->get_imo() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_staff() == 1 );
        ++it;
        ++it;
        ++it;
        pNote = dynamic_cast<ImoNote*>( (*it)->get_imo() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_staff() == 2 );
        ++it;
        ++it;
        ++it;
        ++it;
        pRest = dynamic_cast<ImoRest*>( (*it)->get_imo() );
        CHECK( pRest != NULL );
        CHECK( pRest->get_staff() == 2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_rest() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoRest* pRest = dynamic_cast<ImoRest*>( pIModel->get_root() );
        CHECK( pRest != NULL );
        CHECK( pRest->get_dots() == 1 );
        CHECK( pRest->get_note_type() == k_eighth );
        CHECK( pRest->has_attachments() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_fermata() == true );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_default );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_fermata() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_fermata() == true );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_below );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_fermata() == true );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_default );
        CHECK( pFerm->get_symbol() == ImoFermata::k_short );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_fermata() == true );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_default );
        CHECK( pFerm->get_symbol() == ImoFermata::k_long );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_default );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_above );
        CHECK( pFerm->get_user_location_x() == 70.0f );
        CHECK( pFerm->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pIModel->get_root() );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_above );
        CHECK( pFerm->get_user_location_x() == 70.0f );
        CHECK( pFerm->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->is_stem_up() == true );
        CHECK( pNote->get_num_attachments() == 1 );
        ImoFermata* pFerm = dynamic_cast<ImoFermata*>( pNote->get_attachment(0) );
        CHECK( pFerm != NULL );
        CHECK( pFerm->get_placement() == k_placement_above );
        CHECK( pFerm->get_user_location_x() == 70.0f );
        CHECK( pFerm->get_user_location_y() == 0.0f );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_dynamics_mark() == true );
        ImoDynamicsMark* pImo = dynamic_cast<ImoDynamicsMark*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_placement() == k_placement_default );
        CHECK( pImo->get_mark_type() == "ppp" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDynamicsMark* pImo = dynamic_cast<ImoDynamicsMark*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_placement() == k_placement_default );
        CHECK( pImo->get_mark_type() == "ppp" );
        CHECK( pImo->get_id() == 30L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDynamicsMark* pImo = dynamic_cast<ImoDynamicsMark*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_placement() == k_placement_above );
        CHECK( pImo->get_mark_type() == "sf" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDynamicsMark* pImo = dynamic_cast<ImoDynamicsMark*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_placement() == k_placement_above );
        CHECK( pImo->get_mark_type() == "sf" );
        CHECK( pImo->get_user_location_x() == 70.0f );
        CHECK( pImo->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->is_stem_up() == true );
        CHECK( pNote->get_num_attachments() == 1 );
        ImoDynamicsMark* pImo = dynamic_cast<ImoDynamicsMark*>( pNote->get_attachment(0) );
        CHECK( pImo != NULL );
        CHECK( pImo->get_placement() == k_placement_above );
        CHECK( pImo->get_mark_type() == "sf" );
        CHECK( pImo->get_user_location_x() == 70.0f );
        CHECK( pImo->get_user_location_y() == 0.0f );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_go_back_fwd() == true );
        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
        CHECK( pGBF != NULL );
        CHECK( pGBF->is_to_start() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_go_back_fwd() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
        CHECK( pGBF != NULL );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == -64.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
        CHECK( pGBF != NULL );
        CHECK( pGBF->is_to_end() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_go_back_fwd() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
        CHECK( pGBF != NULL );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == 128.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
        CHECK( pGBF != NULL );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == 128.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>( pIModel->get_root() );
        CHECK( pGBF != NULL );
        CHECK( !pGBF->is_to_start() );
        CHECK( !pGBF->is_to_end() );
        CHECK( pGBF->get_time_shift() == -21.3f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_rest() == true );
        ImoRest* pImo = static_cast<ImoRest*>( pIModel->get_root() );
        CHECK( pImo->is_go_fwd() == true );
        CHECK( pImo->get_duration() == 128.0 );
        CHECK( pImo->get_voice() == 3 );
        CHECK( pImo->is_visible() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_clef() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef G (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );
        CHECK( pClef->is_visible() );
        CHECK( pClef->get_user_location_x() == 70.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef C2 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == false );
        CHECK( pClef->get_staff() == 0 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_NoVisible_Staff2)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef C2 p2 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == false );
        CHECK( pClef->get_staff() == 1 );
        CHECK( pClef->get_symbol_size() == k_size_default );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Clef_SymbolSizeOk)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(clef C2 (symbolSize cue))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == true );
        CHECK( pClef->get_staff() == 0 );
        CHECK( pClef->get_symbol_size() == k_size_cue );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == true );
        CHECK( pClef->get_staff() == 0 );
        CHECK( pClef->get_symbol_size() == k_size_full );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_instrument() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoInstrument* pInstrument = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstrument != NULL );
        //cout << "num.staves=" << pInstrument->get_num_staves() << endl;
        CHECK( pInstrument->get_num_staves() == 2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_instrument() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoInstrument* pInstrument = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstrument != NULL );
        //cout << "num.staves=" << pInstrument->get_num_staves() << endl;
        CHECK( pInstrument->get_num_staves() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "Guitar" );
        CHECK( pInstr->get_abbrev().get_text() == "" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "" );
        CHECK( pInstr->get_abbrev().get_text() == "G." );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "Guitar" );
        CHECK( pInstr->get_abbrev().get_text() == "G." );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrErrorValue)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid MIDI instrument (0..255). MIDI info ignored." << endl;
        parser.parse_text("(infoMIDI piano 1)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
        CHECK( pInfo == NULL );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_MidiInfo_InstrErrorRange)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        expected << "Line 0. Missing or invalid MIDI instrument (0..255). MIDI info ignored." << endl;
        parser.parse_text("(infoMIDI 315 1)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
        CHECK( pInfo == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_midi_info() == true );
        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_channel() == 0 );
        CHECK( pInfo->get_instrument() == 56 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_channel() == 0 );
        CHECK( pInfo->get_instrument() == 56 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMidiInfo* pInfo = dynamic_cast<ImoMidiInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_channel() == 10 );
        CHECK( pInfo->get_instrument() == 56 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_staves() == 1 );
        CHECK( pInstr->get_name().get_text() == "" );
        CHECK( pInstr->get_abbrev().get_text() == "" );
        CHECK( pInstr->get_channel() == 12 );
        CHECK( pInstr->get_instrument() == 56 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_key_signature() == true );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pIModel->get_root() );
        CHECK( pKeySignature != NULL );
        CHECK( pKeySignature->get_key_type() == k_key_G );
        CHECK( pKeySignature->get_staff() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_key_signature() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pIModel->get_root() );
        CHECK( pKeySignature != NULL );
        CHECK( pKeySignature->get_key_type() == k_key_C );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Key_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(key d (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pIModel->get_root() );
        CHECK( pKeySignature != NULL );
        CHECK( pKeySignature->get_key_type() == k_key_d );
        CHECK( pKeySignature->get_staff() == 0 );
        CHECK( pKeySignature->is_visible() );
        CHECK( pKeySignature->get_user_location_x() == 70.0f );
        CHECK( pKeySignature->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Key_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(key E- noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoKeySignature* pKeySignature = dynamic_cast<ImoKeySignature*>( pIModel->get_root() );
        CHECK( pKeySignature != NULL );
        CHECK( pKeySignature->get_key_type() == k_key_Ef );
        CHECK( pKeySignature->get_user_location_x() == 0.0f );
        CHECK( pKeySignature->get_user_location_y() == 0.0f );
        CHECK( pKeySignature->is_visible() == false );


        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_lyric() == true );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_number() == 1 );
        CHECK( pImo->get_placement() == k_placement_below );
        CHECK( pImo->is_laughing() == false );
        CHECK( pImo->is_humming() == false );
        CHECK( pImo->is_end_line() == false );
        CHECK( pImo->is_end_paragraph() == false );
        CHECK( pImo->has_melisma() == false );
        CHECK( pImo->has_hyphenation() == false );
        CHECK( pImo->get_prev_lyric() == NULL );
        CHECK( pImo->get_next_lyric() == NULL );
        CHECK( pImo->get_num_text_items() == 1 );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != NULL );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "te" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == NULL );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_lyric() == true );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_id() == 37L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_lyric() == true );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_number() == 1 );
        CHECK( pImo->get_placement() == k_placement_below );
        CHECK( pImo->is_laughing() == false );
        CHECK( pImo->is_humming() == false );
        CHECK( pImo->is_end_line() == false );
        CHECK( pImo->is_end_paragraph() == false );
        CHECK( pImo->has_hyphenation() == false );
        CHECK( pImo->has_melisma() == false );
        CHECK( pImo->get_prev_lyric() == NULL );
        CHECK( pImo->get_next_lyric() == NULL );
        CHECK( pImo->get_num_text_items() == 2 );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != NULL );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == NULL );
        CHECK( pSyl->has_elision() == true );
        CHECK( pSyl->get_elision_text() == "." );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl != NULL );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "A" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == NULL );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_lyric() == true );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_number() == 1 );
        CHECK( pImo->get_placement() == k_placement_below );
        CHECK( pImo->is_laughing() == false );
        CHECK( pImo->is_humming() == false );
        CHECK( pImo->is_end_line() == false );
        CHECK( pImo->is_end_paragraph() == false );
        CHECK( pImo->has_hyphenation() == true );
        CHECK( pImo->has_melisma() == false );
        CHECK( pImo->get_prev_lyric() == NULL );
        CHECK( pImo->get_next_lyric() == NULL );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != NULL );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == NULL );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_lyric() == true );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_number() == 1 );
        CHECK( pImo->get_placement() == k_placement_below );
        CHECK( pImo->is_laughing() == false );
        CHECK( pImo->is_humming() == false );
        CHECK( pImo->is_end_line() == false );
        CHECK( pImo->is_end_paragraph() == false );
        CHECK( pImo->has_hyphenation() == false );
        CHECK( pImo->has_melisma() == false );
        CHECK( pImo->get_prev_lyric() == NULL );
        CHECK( pImo->get_next_lyric() == NULL );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != NULL );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == NULL );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_lyric() == true );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pIModel->get_root() );
        CHECK( pImo != NULL );
        CHECK( pImo->get_number() == 1 );
        CHECK( pImo->get_placement() == k_placement_below );
        CHECK( pImo->is_laughing() == false );
        CHECK( pImo->is_humming() == false );
        CHECK( pImo->is_end_line() == false );
        CHECK( pImo->is_end_paragraph() == false );
        CHECK( pImo->has_hyphenation() == false );
        CHECK( pImo->has_melisma() == true );
        CHECK( pImo->get_prev_lyric() == NULL );
        CHECK( pImo->get_next_lyric() == NULL );

        ImoLyricsTextInfo* pSyl = pImo->get_text_item(0);
        CHECK( pSyl != NULL );
        CHECK( pSyl->get_syllable_type() == ImoLyricsTextInfo::k_single );
        CHECK( pSyl->get_syllable_text() == "De" );
        CHECK( pSyl->get_syllable_language() == "" );
        CHECK( pSyl->get_syllable_style() == NULL );
        CHECK( pSyl->has_elision() == false );
        CHECK( pSyl->get_elision_text() == "" );

        pSyl = pImo->get_text_item(1);
        CHECK( pSyl == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != NULL );
        CHECK( pNote->get_num_attachments() == 1 );
        ImoLyric* pImo = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pImo != NULL );
        CHECK( pImo->get_number() == 1 );
        CHECK( pImo->get_prev_lyric() == NULL );
        CHECK( pImo->get_next_lyric() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 0 );
        CHECK( pNote->get_num_attachments() == 1 );
        ImoLyric* pLyric1 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric1 != NULL );
        CHECK( pLyric1->get_number() == 1 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 1);
        CHECK( pNote->get_num_attachments() == 1 );
        ImoLyric* pLyric2 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric2 != NULL );
        CHECK( pLyric2->get_number() == 1 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 2);
        CHECK( pNote->get_num_attachments() == 1 );
        ImoLyric* pLyric3 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric3 != NULL );
        CHECK( pLyric3->get_number() == 1 );

        CHECK( pLyric1->get_prev_lyric() == NULL );
        CHECK( pLyric1->get_next_lyric() == pLyric2 );
        CHECK( pLyric2->get_prev_lyric() == pLyric1 );
        CHECK( pLyric2->get_next_lyric() == pLyric3 );
        CHECK( pLyric3->get_prev_lyric() == pLyric2 );
        CHECK( pLyric3->get_next_lyric() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 0 );
        CHECK( pNote->get_num_attachments() == 2 );
        ImoLyric* pLyric11 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric11 != NULL );
        CHECK( pLyric11->get_number() == 1 );
        ImoLyric* pLyric12 = dynamic_cast<ImoLyric*>( pNote->get_attachment(1) );
        CHECK( pLyric12 != NULL );
        CHECK( pLyric12->get_number() == 2 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 1);
        CHECK( pNote->get_num_attachments() == 2 );
        ImoLyric* pLyric21 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric21 != NULL );
        CHECK( pLyric21->get_number() == 1 );
        ImoLyric* pLyric22 = dynamic_cast<ImoLyric*>( pNote->get_attachment(1) );
        CHECK( pLyric22 != NULL );
        CHECK( pLyric22->get_number() == 2 );

        ++it;
        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 2);
        CHECK( pNote->get_num_attachments() == 2 );
        ImoLyric* pLyric31 = dynamic_cast<ImoLyric*>( pNote->get_attachment(0) );
        CHECK( pLyric31 != NULL );
        CHECK( pLyric31->get_number() == 1 );
        ImoLyric* pLyric32 = dynamic_cast<ImoLyric*>( pNote->get_attachment(1) );
        CHECK( pLyric32 != NULL );
        CHECK( pLyric32->get_number() == 2 );

        CHECK( pLyric11->get_prev_lyric() == NULL );
        CHECK( pLyric11->get_next_lyric() == pLyric21 );
        CHECK( pLyric21->get_prev_lyric() == pLyric11 );
        CHECK( pLyric21->get_next_lyric() == pLyric31 );
        CHECK( pLyric31->get_prev_lyric() == pLyric21 );
        CHECK( pLyric31->get_next_lyric() == NULL );

        CHECK( pLyric12->get_prev_lyric() == NULL );
        CHECK( pLyric12->get_next_lyric() == pLyric22 );
        CHECK( pLyric22->get_prev_lyric() == pLyric12 );
        CHECK( pLyric22->get_next_lyric() == pLyric32 );
        CHECK( pLyric32->get_prev_lyric() == pLyric22 );
        CHECK( pLyric32->get_next_lyric() == NULL );

        delete tree->get_root();
        delete pIModel;
    }

    //@ metronome -----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, metronome_0)
    {
        //@00. metronome, note value

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(score (vers 2.0) (instrument#100"
                "(musicData (metronome q 55) )))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoMetronomeMark* pImo = static_cast<ImoMetronomeMark*>(*it);
        CHECK( pImo->get_ticks_per_minute() == 55 );
        CHECK( pImo->get_mark_type() == ImoMetronomeMark::k_note_value );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature->get_bottom_number() == 8 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 2 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(time 3 4 (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 3 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );
        CHECK( pTimeSignature->is_visible() );
        CHECK( pTimeSignature->get_user_location_x() == 70.0f );
        CHECK( pTimeSignature->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TimeSignature_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(time 6 8 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->is_normal() );
        CHECK( pTimeSignature->get_top_number() == 6 );
        CHECK( pTimeSignature->get_bottom_number() == 8 );
        CHECK( pTimeSignature->get_user_location_x() == 0.0f );
        CHECK( pTimeSignature->get_user_location_y() == 0.0f );
        CHECK( pTimeSignature->is_visible() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->is_common() );
        CHECK( pTimeSignature->get_top_number() == 4 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->is_cut() );
        CHECK( pTimeSignature->get_top_number() == 2 );
        CHECK( pTimeSignature->get_bottom_number() == 2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->get_top_number() == 2 );
        CHECK( pTimeSignature->get_bottom_number() == 4 );
        CHECK( pTimeSignature->is_normal() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_time_signature() == true );
        ImoTimeSignature* pTimeSignature = dynamic_cast<ImoTimeSignature*>( pIModel->get_root() );
        CHECK( pTimeSignature != NULL );
        CHECK( pTimeSignature->is_single_number() );
        CHECK( pTimeSignature->get_top_number() == 10 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_system_info() == true );
        ImoSystemInfo* pSI = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
        CHECK( pSI != NULL );
        CHECK( pSI->is_first() );
        CHECK( pSI->get_left_margin() == 0.0f );
        CHECK( pSI->get_right_margin() == 0.0f );
        CHECK( pSI->get_system_distance() == 0.0f );
        CHECK( pSI->get_top_system_distance() == 2000.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSystemInfo* pSI = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
        CHECK( pSI != NULL );
        CHECK( !pSI->is_first() );
        CHECK( pSI->get_left_margin() == 0.0f );
        CHECK( pSI->get_right_margin() == 0.0f );
        CHECK( pSI->get_system_distance() == 0.0f );
        CHECK( pSI->get_top_system_distance() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_system_info() == true );
        ImoSystemInfo* pSI = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
        CHECK( pSI != NULL );
        CHECK( !pSI->is_first() );
        CHECK( pSI->get_left_margin() == 0.0f );
        CHECK( pSI->get_right_margin() == 100.0f );
        CHECK( pSI->get_system_distance() == 0.0f );
        CHECK( pSI->get_top_system_distance() == 2000.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSystemInfo* pSL = dynamic_cast<ImoSystemInfo*>( pIModel->get_root() );
        CHECK( pSL != NULL );
        CHECK( !pSL->is_first() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_score_text() == true );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "This is a text" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_score_text() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
        CHECK( pText == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "Moonlight sonata" );
        ImoStyle* pStyle = pText->get_style();
        CHECK( pStyle == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "F. Chopin" );
        CHECK( pText->get_user_location_x() == 20.0f );
        CHECK( pText->get_user_location_y() == 30.0f );
        ImoStyle* pStyle = pText->get_style();
        CHECK( pStyle == NULL );

        delete tree->get_root();
        delete pIModel;
    }

    // metronome ------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_Value)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(metronome 88)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_metronome_mark() == true );
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_metronome)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(metronome#10 88)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_metronome_mark() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 60 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_value );
        CHECK( pMM->get_ticks_per_minute() == 77 );
        CHECK( pMM->get_left_note_type() == k_eighth );
        CHECK( pMM->get_left_dots() == 1 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_note_note );
        CHECK( pMM->get_left_note_type() == k_eighth );
        CHECK( pMM->get_left_dots() == 1 );
        CHECK( pMM->get_right_note_type() == k_16th );
        CHECK( pMM->get_right_dots() == 0 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 60 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_LocationX)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(metronome 88 (dx 70))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() );
        CHECK( pMM->get_user_location_x() == 70.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_NoVisible)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(metronome 88 noVisible)");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->get_user_location_x() == 0.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );
        CHECK( pMM->is_visible() == false );
        CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Metronome_Parenthesis)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(metronome 88 parenthesis (visible no))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->get_user_location_x() == 0.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );
        CHECK( pMM->is_visible() == false );
        CHECK( pMM->has_parenthesis() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() == false );
        CHECK( pMM->has_parenthesis() == true );
        CHECK( pMM->get_user_location_x() == 7.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        CHECK( pMM != NULL );
        CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        CHECK( pMM->get_ticks_per_minute() == 88 );
        CHECK( pMM->is_visible() == true );
        CHECK( pMM->has_parenthesis() == false );
        CHECK( pMM->get_user_location_x() == 7.0f );
        CHECK( pMM->get_user_location_y() == 0.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_option() == true );
        ImoOptionInfo* pOpt = dynamic_cast<ImoOptionInfo*>( pIModel->get_root() );
        CHECK( pOpt != NULL );
        CHECK( pOpt->get_name() == "StaffLines.Hide" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_boolean );
        CHECK( pOpt->get_bool_value() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoOptionInfo* pOpt = dynamic_cast<ImoOptionInfo*>( pIModel->get_root() );
        CHECK( pOpt != NULL );
        CHECK( pOpt->get_name() == "StaffLines.Truncate" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_long );
        CHECK( pOpt->get_long_value() == 2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoOptionInfo* pOpt = dynamic_cast<ImoOptionInfo*>( pIModel->get_root() );
        CHECK( pOpt != NULL );
        CHECK( pOpt->get_name() == "Render.SpacingFactor" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_float );
        CHECK( pOpt->get_float_value() == 0.536f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->has_options() == true );
        ImoOptionInfo* pOpt = pScore->get_option("StaffLines.Hide");
        CHECK( pOpt != NULL );
        CHECK( pOpt->get_name() == "StaffLines.Hide" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_boolean );
        CHECK( pOpt->get_bool_value() == true );

        delete tree->get_root();
        delete pIModel;
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
            "(instrument (musicData)))"
        );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->has_options() == true );
        ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingMethod");
        CHECK( pOpt != NULL );
        CHECK( pOpt->get_name() == "Render.SpacingMethod" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_long );
        CHECK( pOpt->get_long_value() == 1L );
        pOpt = pScore->get_option("Render.SpacingValue");
        CHECK( pOpt != NULL );
        CHECK( pOpt->get_name() == "Render.SpacingValue" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_float );
        CHECK( pOpt->get_float_value() == 30.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->has_options() == true );
        ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
        CHECK( pOpt != NULL );
        CHECK( pOpt->get_name() == "Render.SpacingFactor" );
        CHECK( pOpt->get_type() == ImoOptionInfo::k_number_float );
        CHECK( pOpt->get_float_value() == 4.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_note_type() == k_quarter );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_spacer() == true );
        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
        CHECK( pSp != NULL );
        CHECK( pSp->get_width() == 70.5f );
        CHECK( pSp->get_staff() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_spacer() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
        CHECK( pSp != NULL );
        CHECK( pSp->get_width() == 70.5f );
        CHECK( pSp->get_staff() == 2 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
        CHECK( pSp != NULL );
        CHECK( pSp->get_width() == 70.5f );
        CHECK( pSp->get_staff() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
        CHECK( pSp != NULL );
        CHECK( pSp->get_width() == 70.5f );
        CHECK( pSp->get_staff() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
        CHECK( pSp != NULL );
        CHECK( pSp->get_width() == 70.0f );
        CHECK( pSp->get_staff() == 0 );
        CHECK( pSp->has_attachments() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
        CHECK( pSp != NULL );
        CHECK( pSp->get_width() == 70.0f );
        CHECK( pSp->get_staff() == 0 );
        CHECK( pSp->has_attachments() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoSpacer* pSp = dynamic_cast<ImoSpacer*>( pIModel->get_root() );
        CHECK( pSp != NULL );
        CHECK( pSp->get_width() == 70.0f );
        CHECK( pSp->get_staff() == 0 );
        CHECK( pSp->has_attachments() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 0 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_document() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        //cout << "language='" << pDoc->get_language() << "'" << endl;
        CHECK( pDoc->get_language() == "zh_CN" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Lenmusdoc_GetContentItemText)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0)(content (text \"hello world\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(0) );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "hello world" );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Lenmusdoc_GetContentItemScore)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0)(content (score (vers 1.6)(instrument (musicData))) (text \"hello world\")))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc->get_num_content_items() == 2 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(1) );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "hello world" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoObj* pImo = pDoc->get_content_item(0);
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoObj* pImo = pDoc->get_content();
        CHECK( pImo->get_id() == 60L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoStyle* pStyle = pDoc->find_style("Heading-1");
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Heading-1" );

        delete tree->get_root();
        delete pIModel;
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
        expected << "Line 0. This tie has the same number than that defined in line 0. This tie will be ignored." << endl;

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

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_TiesBuilder_ErrorNotesCanNotBeTied)
    {
        Document doc(m_libraryScope);
        stringstream errormsg;
        stringstream expected;
        expected << "Line 0. Requesting to tie notes of different voice or pitch. Tie number 12 will be ignored." << endl;

        ImoNote* startNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        startNote->set_octave(4);
        startNote->set_step(2);
        startNote->set_notated_accidentals(k_sharp);
        startNote->set_note_type(k_eighth);

        ImoNote* endNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, &doc));
        endNote->set_octave(3);
        endNote->set_step(2);
        endNote->set_notated_accidentals(k_sharp);
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 0 );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

        pInstr = pScore->get_instrument(1);
        pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );

        it = pMusic->begin();

        pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 1 );
        CHECK( pNote->is_tied_next() == false );
        CHECK( pNote->is_tied_prev() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_beam_dto() == true );
        ImoBeamDto* pInfo = dynamic_cast<ImoBeamDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_beam_number() == 12 );
        CHECK( pInfo->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pInfo->get_beam_type(1) == ImoBeam::k_none );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBeamDto* pInfo = dynamic_cast<ImoBeamDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_beam_number() == 12 );
        CHECK( pInfo->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pInfo->get_beam_type(1) == ImoBeam::k_begin );
        CHECK( pInfo->get_beam_type(2) == ImoBeam::k_forward );
        CHECK( pInfo->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pInfo->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pInfo->get_beam_type(5) == ImoBeam::k_none );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root() == NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root() == NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote != NULL );
        CHECK( pNote->is_beamed() == false );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_beamed() == false );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_beamed() == false );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_beamed() == true );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_beamed() == true );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_backward );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );

        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 0 );
        CHECK( pNote->is_beamed() == false );
        CHECK( pNote->get_beam_type(0) == ImoBeam::k_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_beamed() == true );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_beamed() == true );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_backward );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_note_type() == k_eighth );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == k_step_C );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 0 );
        CHECK( pNote->get_note_type() == k_whole );
        CHECK( pNote->is_beamed() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1->is_beamed() == true );
        CHECK( pNote1->get_beam_type(0) == ImoBeam::k_begin );
        CHECK( pNote1->get_beam_type(1) == ImoBeam::k_forward );
        CHECK( pNote1->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote1->get_beam_type(5) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->is_beamed() == true );
        CHECK( pNote2->get_beam_type(0) == ImoBeam::k_continue );
        CHECK( pNote2->get_beam_type(1) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote2->get_beam_type(5) == ImoBeam::k_none );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != NULL );
        CHECK( pNote3->is_beamed() == true );
        CHECK( pNote3->get_beam_type(0) == ImoBeam::k_end );
        CHECK( pNote3->get_beam_type(1) == ImoBeam::k_backward );
        CHECK( pNote3->get_beam_type(2) == ImoBeam::k_none );
        CHECK( pNote3->get_beam_type(3) == ImoBeam::k_none );
        CHECK( pNote3->get_beam_type(4) == ImoBeam::k_none );
        CHECK( pNote3->get_beam_type(5) == ImoBeam::k_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_tuplet_dto() == true );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 7 );
        CHECK( pInfo->get_normal_number() == 4 );
        CHECK( pInfo->get_show_bracket() == k_yesno_default );
        CHECK( pInfo->get_show_number() == ImoTuplet::k_number_actual );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_no );
        CHECK( pInfo->get_show_number() == ImoTuplet::k_number_actual );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_default );
        CHECK( pInfo->get_show_number() == ImoTuplet::k_number_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_tuplet_dto() == true );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 7 );
        CHECK( pInfo->get_normal_number() == 4 );
        CHECK( pInfo->get_show_bracket() == k_yesno_default );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_no );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_no );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTupletDto* pInfo = dynamic_cast<ImoTupletDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_start_of_tuplet() == true );
        CHECK( pInfo->get_actual_number() == 3 );
        CHECK( pInfo->get_normal_number() == 2 );
        CHECK( pInfo->get_show_bracket() == k_yesno_no );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pNote->find_attachment(k_imo_tuplet) == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        delete pA;

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
        CHECK( pTuplet->is_tuplet() == true );
        CHECK( pTuplet->get_num_objects() == 2 );
//        cout << "num.objects = " << pTuplet->get_num_objects() << endl;
        CHECK( pTuplet->get_actual_number() == 3 );
        CHECK( pTuplet->get_normal_number() == 2 );

        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
            pTuplet->get_related_objects();
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
        ImoNote* pNt1 = dynamic_cast<ImoNote*>( (*itN).first );
        ++itN;
        ImoNote* pNt3 = dynamic_cast<ImoNote*>( (*itN).first );


        it = pMusic->begin();

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1 == pNt1 );
        CHECK( pNote1->find_relation(k_imo_tuplet) == pTuplet );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );
        CHECK( pNote2->find_relation(k_imo_tuplet) == NULL );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != NULL );
        CHECK( pNote3 == pNt3 );
        CHECK( pNote3->find_relation(k_imo_tuplet) == pTuplet );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMusic = pInstr->get_musicdata();
        CHECK( pMusic != NULL );

        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>(*it);
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 0 );
        CHECK( pNote->find_attachment(k_imo_tuplet_data) == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_octave() == 4 );
        CHECK( pNote->get_step() == 0 );
        CHECK( pNote->find_attachment(k_imo_tuplet_data) == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = pA->analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete pA;
        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
        CHECK( pTuplet == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pNote->find_relation(k_imo_tuplet));
        CHECK( pTuplet->get_num_objects() == 2 );
        CHECK( pTuplet->get_actual_number() == 3 );
        CHECK( pTuplet->get_normal_number() == 2 );

        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
            pTuplet->get_related_objects();
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
        ImoNote* pNt1 = dynamic_cast<ImoNote*>( (*itN).first );
        ++itN;
        ImoNote* pNt3 = dynamic_cast<ImoNote*>( (*itN).first );

        ImoNote* pNote1 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote1 != NULL );
        CHECK( pNote1 == pNt1 );

        ++it;
        ImoNote* pNote2 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote2 != NULL );

        ++it;
        ImoNote* pNote3 = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote3 != NULL );
        CHECK( pNote3 == pNt3 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_time_modification_dto() == true );
        ImoTimeModificationDto* pInfo =
                dynamic_cast<ImoTimeModificationDto*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_top_number() == 2 );
        CHECK( pInfo->get_bottom_number() == 3 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_voice() == 7 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_voice() == 1 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_staff() == 1 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->get_staff() == 0 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();
        ImoRest* pRest = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest != NULL);
        ImoTuplet* pTuplet = static_cast<ImoTuplet*>(pRest->find_relation(k_imo_tuplet));
        CHECK( pTuplet->get_num_objects() == 2 );

        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes =
            pTuplet->get_related_objects();
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itN = notes.begin();
        ImoRest* pNR1 = dynamic_cast<ImoRest*>( (*itN).first );
        ++itN;
        ImoRest* pNR3 = dynamic_cast<ImoRest*>( (*itN).first );

        CHECK( (*it)->is_rest() == true );
        ImoRest* pRest1 = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest1 != NULL );
        CHECK( pRest1 == pNR1 );
        CHECK( pRest1->get_voice() == 3 );
        CHECK( pRest1->get_staff() == 1 );

        ++it;
        ImoRest* pRest2 = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest2 != NULL );
        CHECK( pRest2->get_voice() == 3 );
        CHECK( pRest2->get_staff() == 1 );
        CHECK( pRest2->has_attachments() == true );

        ++it;
        ImoRest* pRest3 = dynamic_cast<ImoRest*>( *it );
        CHECK( pRest3 != NULL );
        CHECK( pRest3 == pNR3 );
        CHECK( pRest3->get_voice() == 3 );
        CHECK( pRest3->get_staff() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root() == NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoColorDto* pColor = dynamic_cast<ImoColorDto*>( pIModel->get_root() );
        CHECK( pColor != NULL );
        CHECK( pColor->red() == 240 );
        CHECK( pColor->green() == 69 );
        CHECK( pColor->blue() == 127 );
        CHECK( pColor->alpha() == 255 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        Color& color = pNote->get_color();
        CHECK( color.r == 240 );
        CHECK( color.g == 69 );
        CHECK( color.b == 127 );
        CHECK( color.a == 255 );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Analyser_Barline_Color)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(barline double (color #ff0000))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoBarline* pBarline = dynamic_cast<ImoBarline*>( pIModel->get_root() );
        CHECK( pBarline != NULL );
        CHECK( pBarline->get_type() == k_barline_double );
        CHECK( pBarline->is_visible() );
        CHECK( is_equal(pBarline->get_color(), Color(255,0,0)) );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_barline() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_none );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );
        CHECK( pGroup->join_barlines() == ImoInstrGroup::k_mensurstrich );
        ImoInstrument* pInstr = pGroup->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_mensurstrich );
        pInstr = pGroup->get_instrument(1);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_nothing );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << UnitTest::CurrentTest::Details()->testName << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 3 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );
        CHECK( pGroup->join_barlines() == ImoInstrGroup::k_standard );
        //barlines layout transferred to instruments
        ImoInstrument* pInstr = pGroup->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_joined );
        pInstr = pGroup->get_instrument(1);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_joined );
        pInstr = pGroup->get_instrument(2);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_barline_layout() == ImoInstrument::k_isolated );

        delete tree->get_root();
        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root()->is_instr_group() == true );
//        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
//        CHECK( pGrp != NULL );
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
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
//        CHECK( pGrp != NULL );
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
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
//        CHECK( pGrp != NULL );
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
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
//        CHECK( pGrp != NULL );
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
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        CHECK( pIModel->get_root() == NULL );
//
//        delete tree->get_root();
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
//        CHECK( pGrp != NULL );
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
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//        CHECK( pIModel->get_root() == NULL );
//
//        delete tree->get_root();
//        delete pIModel;
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
//        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//
//        //cout << "[" << errormsg.str() << "]" << endl;
//        //cout << "[" << expected.str() << "]" << endl;
//        CHECK( errormsg.str() == expected.str() );
//
//        ImoInstrGroup* pGrp = dynamic_cast<ImoInstrGroup*>( pIModel->get_root() );
//        CHECK( pGrp != NULL );
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
//        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );

        ImoObj::children_iterator it = pMusic->begin();
        CHECK( pMusic->get_num_children() == 3 );

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

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );

        ImoObj::children_iterator it = pMusic->begin();
        CHECK( pMusic->get_num_children() == 3 );

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

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );

        ImoObj::children_iterator it = pMusic->begin();
        CHECK( pMusic->get_num_children() == 4 );

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == true );
        CHECK( pNote->is_end_of_chord() == false );
        CHECK( pNote->is_beamed() == true );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == true );
        CHECK( pNote->is_beamed() == false );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == true );
        CHECK( pNote->is_end_of_chord() == false );
        CHECK( pNote->is_beamed() == true );

        ++it;
        pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );
        CHECK( pNote->is_in_chord() == true );
        CHECK( pNote->is_start_of_chord() == false );
        CHECK( pNote->is_end_of_chord() == true );
        CHECK( pNote->is_beamed() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoPageInfo* pInfo = dynamic_cast<ImoPageInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_page_info() == true );
        CHECK( pInfo->get_left_margin() == 1000.0f );
        CHECK( pInfo->get_top_margin() == 1200.0f );
        CHECK( pInfo->get_right_margin() == 3000.0f );
        CHECK( pInfo->get_bottom_margin() == 2500.0f );
        CHECK( pInfo->get_binding_margin() == 4000.0f );
        CHECK( pInfo->get_page_width() == 14000.0f );
        CHECK( pInfo->get_page_height() == 10000.0f );
        CHECK( pInfo->is_portrait() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        ImoPageInfo* pInfo = pScore->get_page_info();
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_page_info() == true );
        CHECK( pInfo->get_left_margin() == 1000.0f );
        CHECK( pInfo->get_top_margin() == 1200.0f );
        CHECK( pInfo->get_right_margin() == 3000.0f );
        CHECK( pInfo->get_bottom_margin() == 2500.0f );
        CHECK( pInfo->get_binding_margin() == 4000.0f );
        CHECK( pInfo->get_page_width() == 14000.0f );
        CHECK( pInfo->get_page_height() == 10000.0f );
        CHECK( pInfo->is_portrait() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        ImoPageInfo* pInfo = pDoc->get_page_info();
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_page_info() == true );
        CHECK( pInfo->get_left_margin() == 1000.0f );
        CHECK( pInfo->get_top_margin() == 1200.0f );
        CHECK( pInfo->get_right_margin() == 3000.0f );
        CHECK( pInfo->get_bottom_margin() == 2500.0f );
        CHECK( pInfo->get_binding_margin() == 4000.0f );
        CHECK( pInfo->get_page_width() == 14000.0f );
        CHECK( pInfo->get_page_height() == 10000.0f );
        CHECK( pInfo->is_portrait() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = dynamic_cast<ImoFontStyleDto*>( pIModel->get_root() );
        CHECK( pFont != NULL );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_bold );
        CHECK( pFont->size == 12 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = dynamic_cast<ImoFontStyleDto*>( pIModel->get_root() );
        CHECK( pFont != NULL );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_normal );
        CHECK( pFont->size == 8 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = dynamic_cast<ImoFontStyleDto*>( pIModel->get_root() );
        CHECK( pFont != NULL );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_bold );
        CHECK( pFont->size == 12 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFontStyleDto* pFont = dynamic_cast<ImoFontStyleDto*>( pIModel->get_root() );
        CHECK( pFont != NULL );
        CHECK( pFont->name == "Trebuchet" );
        CHECK( pFont->style == ImoStyle::k_font_style_normal );
        CHECK( pFont->weight == ImoStyle::k_font_weight_normal );
        CHECK( pFont->size == 17 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle->font_size() == 14 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        ImoStyle* pStyle = pScore->find_style("Header1");
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle->font_size() == 14 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle->margin_bottom() == 2.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( pStyle->font_name() == "Arial" );
        CHECK( pStyle->font_size() == 14 );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, DefineStyle_FontFile)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(defineStyle \"Composer\" "
            "(font-file \"wqy-zenhei.ttc\")"
            "(font-name \"Arial\")(font-size 14pt)"
            "(font-style italic)(font-weight bold) )" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( pStyle->font_file() == "wqy-zenhei.ttc" );
        CHECK( pStyle->font_name() == "Arial" );
        CHECK( pStyle->font_size() == 14 );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( pStyle->margin_top() == 3.0f );
        CHECK( pStyle->margin_bottom() == 2.0f );
        CHECK( pStyle->margin_left() == 5.0f );
        CHECK( pStyle->margin_right() == 7.0f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( pStyle->margin_top() == 0.5f );
        CHECK( pStyle->margin_bottom() == 0.5f );
        CHECK( pStyle->margin_left() == 0.5f );
        CHECK( pStyle->margin_right() == 0.5f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( pStyle->line_height() == 1.2f );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreTitle* pTitle = dynamic_cast<ImoScoreTitle*>( pIModel->get_root() );
        CHECK( pTitle != NULL );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_score_title() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        std::list<ImoScoreTitle*>& titles = pScore->get_titles();
        std::list<ImoScoreTitle*>::iterator it = titles.begin();
        CHECK( it != titles.end() );
        ImoScoreTitle* pTitle = *it;
        CHECK( pTitle != NULL );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreTitle* pTitle = dynamic_cast<ImoScoreTitle*>( pIModel->get_root() );
        CHECK( pTitle != NULL );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );
        ImoStyle* pStyle = pTitle->get_style();
        CHECK( pStyle == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        std::list<ImoScoreTitle*>& titles = pScore->get_titles();
        std::list<ImoScoreTitle*>::iterator it = titles.begin();
        CHECK( it != titles.end() );
        ImoScoreTitle* pTitle = *it;
        CHECK( pTitle != NULL );
        CHECK( pTitle->get_text() == "Moonlight sonata" );
        CHECK( pTitle->get_h_align() == k_halign_center );
        ImoStyle* pStyle = pTitle->get_style();
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle->font_size() == 14 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreTitle* pTitle = dynamic_cast<ImoScoreTitle*>( pIModel->get_root() );
        CHECK( pTitle != NULL );
        CHECK( pTitle->get_text() == "F. Chopin" );
        CHECK( pTitle->get_h_align() == k_halign_right );
        CHECK( pTitle->get_user_location_x() == 0.0f );
        CHECK( pTitle->get_user_location_y() == 30.0f );
        ImoStyle* pStyle = pTitle->get_style();
        CHECK( pStyle == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
        CHECK( pLine != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
        CHECK( pLine != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_score_line() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
        CHECK( pLine != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
        CHECK( pLine != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pIModel->get_root() );
        CHECK( pLine != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoTextBox* pTB = dynamic_cast<ImoTextBox*>( pIModel->get_root() );
        CHECK( pTB != NULL );
        CHECK( pTB->get_text() == "This is a test of a textbox" );
        CHECK( pTB->has_anchor_line() == false );
        ImoTextBlockInfo* pInfo = pTB->get_box_info();
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_textblock_info() == true );
        CHECK( pInfo->get_height() == 150.0f );
        CHECK( pInfo->get_width() == 300.0f );
        CHECK( pInfo->get_position() == TPoint(50.0f, 5.0f) );
        CHECK( is_equal(pInfo->get_bg_color(), Color(255,255,255,255)) );
        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,0,255)) );
        CHECK( pInfo->get_border_width() == 1.0f );
        CHECK( pInfo->get_border_style() == k_line_solid );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_text_box() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoTextBox* pTB = dynamic_cast<ImoTextBox*>( pIModel->get_root() );
        CHECK( pTB != NULL );
        CHECK( pTB->get_text() == "This is a test of a textbox" );

        ImoTextBlockInfo* pInfo = pTB->get_box_info();
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_textblock_info() == true );
        CHECK( pInfo->get_height() == 150.0f );
        CHECK( pInfo->get_width() == 300.0f );
        CHECK( pInfo->get_position() == TPoint(50.0f, 5.0f) );
        CHECK( is_equal(pInfo->get_bg_color(), Color(255,254,11,255)) );
        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,253,255)) );
        CHECK( pInfo->get_border_width() == 5.0f );
        CHECK( pInfo->get_border_style() == k_line_dot );

        CHECK( pTB->has_anchor_line() == true );
        ImoLineStyle* pLine = pTB->get_anchor_line_info();
        CHECK( pLine != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoNote* pNote = dynamic_cast<ImoNote*>( pIModel->get_root() );
        CHECK( pNote != NULL );
        CHECK( pNote->has_attachments() == true );
        ImoTextBox* pTB = dynamic_cast<ImoTextBox*>( pNote->get_attachment(0) );
        CHECK( pTB != NULL );
        CHECK( pTB->get_text() == "This is a test of a textbox" );

        ImoTextBlockInfo* pInfo = pTB->get_box_info();
        CHECK( pInfo != NULL );
        CHECK( pInfo->is_textblock_info() == true );
        CHECK( pInfo->get_height() == 150.0f );
        CHECK( pInfo->get_width() == 300.0f );
        CHECK( pInfo->get_position() == TPoint(50.0f, 5.0f) );
        CHECK( is_equal(pInfo->get_bg_color(), Color(255,255,255,255)) );
        CHECK( is_equal(pInfo->get_border_color(), Color(0,0,0,255)) );
        CHECK( pInfo->get_border_width() == 1.0f );
        CHECK( pInfo->get_border_style() == k_line_solid );

        CHECK( pTB->has_anchor_line() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoCursorInfo* pInfo = dynamic_cast<ImoCursorInfo*>( pIModel->get_root() );
        CHECK( pInfo->is_cursor_info() == true );
        CHECK( pInfo->get_instrument() == 1 );
        CHECK( pInfo->get_staff() == 2 );
        CHECK( pInfo->get_time() == 64.0f );
        CHECK( pInfo->get_id() == 34L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->is_score() == true );
        //CHECK( pScore->get_instrument() == 1 );
        //CHECK( pScore->get_staff() == 2 );
        //CHECK( pScore->get_time() == 64.0f );
        //CHECK( pScore->get_id() == 34L );

        delete tree->get_root();
        delete pIModel;
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
    //    InternalModel* pIModel = a.analyse_tree(tree, "string:");

    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );

    //    ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
    //    CHECK( pDoc->is_document() == true );
    //    //CHECK( pScore->get_instrument() == 1 );
    //    //CHECK( pScore->get_staff() == 2 );
    //    //CHECK( pScore->get_time() == 64.0f );
    //    //CHECK( pScore->get_id() == 34L );

    //    delete tree->get_root();
    //    delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoFiguredBass* pFB = dynamic_cast<ImoFiguredBass*>( pIModel->get_root() );
        CHECK( pFB->is_figured_bass() == true );
        //cout << "FB ='" << pFB->get_figured_bass_string() << "'" << endl;
        CHECK( pFB->get_figured_bass_string() == "7 5 2" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() == NULL );
        //ImoMetronomeMark* pMM = dynamic_cast<ImoMetronomeMark*>( pIModel->get_root() );
        //CHECK( pMM != NULL );
        //CHECK( pMM->get_mark_type() == ImoMetronomeMark::k_value );
        //CHECK( pMM->get_ticks_per_minute() == 88 );
        //CHECK( pMM->is_visible() == true );
        //CHECK( pMM->has_parenthesis() == false );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL );
        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_staff_number() == 1 );
        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_regular );
        CHECK( pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo->get_line_spacing() == 180.0f );
        CHECK( pInfo->get_height() == 735.0f );
        CHECK( pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL );
        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_staff_number() == 1 );
        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo->get_line_spacing() == 180.0f );
        CHECK( pInfo->get_height() == 735.0f );
        CHECK( pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL );
        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_staff_number() == 1 );
        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo->get_line_spacing() == 180.0f );
        CHECK( pInfo->get_height() == 735.0f );
        CHECK( pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL );
        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_staff_number() == 1 );
        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo->get_staff_margin() == 1000.0f );
        CHECK( pInfo->get_line_spacing() == 200.0f );
        CHECK( pInfo->get_height() == 815.0f );
        CHECK( pInfo->get_line_thickness() == 15.0f );
        CHECK( pInfo->get_num_lines() == 5 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL );
        ImoStaffInfo* pInfo = dynamic_cast<ImoStaffInfo*>( pIModel->get_root() );
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_staff_number() == 1 );
        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo->get_staff_margin() == 800.0f );
        CHECK( pInfo->get_line_spacing() == 200.0f );
        CHECK( pInfo->get_height() == 620.5f );
        CHECK( pInfo->get_line_thickness() == 20.5f );
        CHECK( pInfo->get_num_lines() == 4 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL );
        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>( pIModel->get_root() );
        CHECK( pInstr != NULL );
        ImoStaffInfo* pInfo = pInstr->get_staff(1);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_staff_number() == 1 );
        CHECK( pInfo->get_staff_type() == ImoStaffInfo::k_staff_ossia );
        CHECK( pInfo->get_staff_margin() == 800.0f );
        CHECK( pInfo->get_line_spacing() == 200.0f );
        CHECK( pInfo->get_height() == 620.5f );
        CHECK( pInfo->get_line_thickness() == 20.5f );
        CHECK( pInfo->get_num_lines() == 4 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_text_item() == true );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "This is a text" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_text_item() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
    }

    //TEST_FIXTURE(LdpAnalyserTestFixture, TextItem_MissingText)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_libraryScope.ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. text: missing mandatory element 'string'." << endl;
    //    parser.parse_text("(text)");
    //    LdpAnalyser a(errormsg, m_libraryScope, &doc);
    //    InternalModel* pIModel = a.analyse_tree(tree, "string:");
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
    //    CHECK( pText == NULL );

    //    delete tree->get_root();
    //    delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "This is a text" );
        ImoStyle* pStyle = pText->get_style();
        CHECK( pStyle == NULL );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, TextItem_DefaultStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(para (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );
        CHECK( pItem->get_style() != NULL );

        delete tree->get_root();
        delete pIModel;
    }

    //TEST_FIXTURE(LdpAnalyserTestFixture, TextItem_Location)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_libraryScope.ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. " << endl;
    //    parser.parse_text("(text \"F. Chopin\" (style \"Composer\")(dy 30)(dx 20))");
    //    LdpAnalyser a(errormsg, m_libraryScope, &doc);
    //    InternalModel* pIModel = a.analyse_tree(tree, "string:");

    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );

    //    ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
    //    CHECK( pText != NULL );
    //    CHECK( pText->get_text() == "F. Chopin" );
    //    CHECK( pText->get_user_location_x() == 20.0f );
    //    CHECK( pText->get_user_location_y() == 30.0f );
    //    ImoStyle* pStyle = pText->get_style();
    //    CHECK( pStyle == NULL );

    //    delete tree->get_root();
    //    delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_paragraph() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_paragraph() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "This is a paragraph" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
        CHECK( pPara->get_num_items() == 1 );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pPara->get_first_item() );
        CHECK( pLink != NULL );
        CHECK( pLink->get_url() == "This is the url" );
        CHECK( pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem->get_text() == "This is the link" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
        CHECK( pPara->get_num_items() == 2 );
        TreeNode<ImoObj>::children_iterator it = pPara->begin();
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == "This is a paragraph" );
        ++it;
        pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == " with two items." );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_RecognizedAsContent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(para (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        delete pIModel;
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

        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        ImoStyle* pStyle = pPara->get_style();
        CHECK( pStyle != NULL );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Paragraph_DefaultStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(para (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        ImoStyle* pStyle = pPara->get_style();
        CHECK( pStyle->get_name() == "Paragraph" );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_heading() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_heading() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "This is a header" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
        CHECK( pH->get_num_items() == 2 );
        TreeNode<ImoObj>::children_iterator it = pH->begin();
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == "This is a header" );
        ++it;
        pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == " with two items." );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_RecognizedAsContent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(heading 1 (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != NULL );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        delete pIModel;
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

        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != NULL );
        ImoStyle* pStyle = pH->get_style();
        CHECK( pStyle != NULL );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Heading_DefaultStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(heading 1 (txt \"Hello world!\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != NULL );
        ImoStyle* pStyle = pH->get_style();
        CHECK( pStyle->get_name() == "Heading-1" );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pIModel->get_root() );
        CHECK( pStyles != NULL );

        ImoStyle* pStyle = pStyles->find_style("Header1");
        CHECK( pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle->font_size() == 14 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(0) );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "hello world" );

        ImoStyle* pStyle = pDoc->find_style("Header1");
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle->font_size() == 14 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScoreText* pText = dynamic_cast<ImoScoreText*>( pDoc->get_content_item(0) );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "hello world" );

        ImoStyle* pStyle = pDoc->get_style_or_default("text");
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Default style" );

        delete tree->get_root();
        delete pIModel;
    }

    // param ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, ParamInfo_Ok)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(param green \"this is green\")");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_param_info() == true );
        ImoParamInfo* pParam = dynamic_cast<ImoParamInfo*>( pIModel->get_root() );
        CHECK( pParam != NULL );
        CHECK( pParam->get_name() == "green" );
        CHECK( pParam->get_value() == "this is green" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_dynamic() == true );
        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pIModel->get_root() );
        CHECK( pDyn != NULL );
        CHECK( pDyn->get_classid() == "test" );
        CHECK( pDyn->is_visible() );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_dynamic() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Dynamic_AddedToContent)
    {
        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0)(content "
            "(dynamic (classid test)) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pDoc->get_content_item(0) );
        CHECK( pDyn != NULL );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, Dynamic_GeneratesRequest)
    {
        LomseDoorway* pDoorway = m_libraryScope.platform_interface();
        pDoorway->set_request_callback(this, wrapper_lomse_request);

        Document doc(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text("(lenmusdoc (vers 0.0)(content "
            "(dynamic (classid test)) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );

        CHECK( m_fRequestReceived == true );
        CHECK( m_requestType == k_dynamic_content_request );
        CHECK( m_pDoc == pDoc );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pIModel->get_root() );
        CHECK( pDyn->get_classid() == "test" );
        std::list<ImoParamInfo*>& params = pDyn->get_params();
        CHECK( params.size() == 1 );
        ImoParamInfo* pParm = params.front();
        CHECK( pParm->get_name() == "play" );
        CHECK( pParm->get_value() == "all notes" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_link() == true );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pIModel->get_root() );
        CHECK( pLink != NULL );
        CHECK( pLink->get_url() == "#TheoryHarmony_ch3.lms" );
        CHECK( pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem->get_text() == "Harmony exercise" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_link() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_link() == true );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pIModel->get_root() );
        CHECK( pLink != NULL );
        CHECK( pLink->get_url() == "" );
        CHECK( pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem->get_text() == "Harmony exercise" );

        delete tree->get_root();
        delete pIModel;
    }

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
        InternalModel* pIModel = a.analyse_tree(tree, m_scores_path);

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoImage* pImg = dynamic_cast<ImoImage*>( pIModel->get_root() );
        CHECK( pImg != NULL );

        delete tree->get_root();
        delete pIModel;
    }

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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_listitem() == true );
        ImoListItem* pLI = dynamic_cast<ImoListItem*>( pIModel->get_root() );
        CHECK( pLI->get_num_content_items() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is the first item" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_listitem() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_list() == true );
        ImoList* pList = dynamic_cast<ImoList*>( pIModel->get_root() );
        CHECK( pList != NULL );
        CHECK( pList->get_list_type() == ImoList::k_itemized );
        CHECK( pList->get_num_content_items() == 1 );
        ImoListItem* pLI = pList->get_list_item(0);
        CHECK( pLI->get_num_content_items() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is the first item" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_list() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
    }

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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root() == NULL );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score_line() == true );
        ImoScoreLine* pLine = static_cast<ImoScoreLine*>( pIModel->get_root() );
        CHECK( pLine != NULL );
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
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_score_line() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoMusicData* pMusic = dynamic_cast<ImoMusicData*>( pIModel->get_root() );
        CHECK( pMusic != NULL );
        ImoObj::children_iterator it = pMusic->begin();

        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        CHECK( pNote != NULL );

        ++it;
        ImoSpacer* pSpacer = dynamic_cast<ImoSpacer*>( *it );
        CHECK( pSpacer != NULL );
        ImoAttachments* pAuxObjs = pSpacer->get_attachments();
        CHECK( pAuxObjs != NULL );
        ImoScoreLine* pLine = dynamic_cast<ImoScoreLine*>( pAuxObjs->get_item(0) );
        CHECK( pLine != NULL );

        delete tree->get_root();
        delete pIModel;
    }

    // scorePlayer ----------------------------------------------------------------------

    TEST_FIXTURE(LdpAnalyserTestFixture, scorePlayer_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->is_score_player() == true );
        CHECK( pSP->get_metronome_mm() == 60 );
        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, id_in_score_player)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer#10) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, scorePlayer_metronome)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer (mm 65)) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->is_score_player() == true );
        CHECK( pSP->get_metronome_mm() == 65 );
        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;

        delete tree->get_root();
        delete pIModel;
    }

    TEST_FIXTURE(LdpAnalyserTestFixture, scorePlayer_label_play)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("(lenmusdoc (vers 0.0) (content "
            "(scorePlayer (mm 65)(playLabel \"Tocar\")(stopLabel \"Parar\")) ))");
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->is_score_player() == true );
        CHECK( pSP->get_metronome_mm() == 65 );
        CHECK( pSP->get_play_label() == "Tocar" );
        CHECK( pSP->get_stop_label() == "Parar" );
        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 1 );
        CHECK( pCell->get_colspan() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is a cell" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_cell() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 2 );
        CHECK( pCell->get_colspan() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is a cell" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 1 );
        CHECK( pCell->get_colspan() == 2 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is a cell" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pIModel->get_root() );
        CHECK( pRow->is_table_row() == true );
        CHECK( pRow->get_num_cells() == 2 );
        ImoTableCell* pImo = dynamic_cast<ImoTableCell*>( pRow->get_cell(0) );
        CHECK( pImo->is_table_cell() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_row() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableHead* pHead = dynamic_cast<ImoTableHead*>( pIModel->get_root() );
        CHECK( pHead->is_table_head() == true );
        CHECK( pHead->get_num_items() == 2 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
        CHECK( pRow->is_table_row() == true );
        CHECK( pRow->get_num_cells() == 1 );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_head() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableBody* pBody = dynamic_cast<ImoTableBody*>( pIModel->get_root() );
        CHECK( pBody->is_table_body() == true );
        CHECK( pBody->get_num_items() == 2 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_body() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pIModel->get_root() );
        CHECK( pTable != NULL );

        CHECK( pTable->get_head() == NULL );

        ImoTableBody* pBody = pTable->get_body();
        CHECK( pBody != NULL );
        CHECK( pBody->get_num_items() == 1 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
        CHECK( pTable != NULL );

        std::list<ImoStyle*>& cols = pTable->get_column_styles();
        CHECK( cols.size() == 2 );
        std::list<ImoStyle*>::iterator it = cols.begin();
        CHECK( (*it)->get_name() == "table1-col1" );
        ++it;
        CHECK( (*it)->get_name() == "table1-col2" );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
        CHECK( pTable != NULL );

        std::list<ImoStyle*>& cols = pTable->get_column_styles();
        CHECK( cols.size() == 2 );

        ImoTableHead* pHead = pTable->get_head();
        CHECK( pHead != NULL );
        CHECK( pHead->get_num_items() == 1 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        ImoTableBody* pBody = pTable->get_body();
        CHECK( pBody != NULL );
        CHECK( pBody->get_num_items() == 1 );
        pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete tree->get_root();
        delete pIModel;
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
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        ImoInstrument* pInstr = pScore->get_instrument(1);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoClef* pClef = dynamic_cast<ImoClef*>( pMD->get_child(0) );
        CHECK( pClef->get_clef_type() == k_clef_C3 );
        CHECK( pClef->get_staff() == 0 );

        delete tree->get_root();
        delete pIModel;
    }
}

