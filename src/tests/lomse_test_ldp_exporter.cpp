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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_exporter.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_ldp_compiler.h"
#include "private/lomse_document_p.h"
#include "lomse_im_factory.h"
#include "lomse_staffobjs_table.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// test for LdpExporter
//=======================================================================================

class LdpExporterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    LdpExporterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LdpExporterTestFixture()    //TearDown fixture
    {
    }

    void dump_colection(ImoScore* pScore)
    {
        ColStaffObjs* pCol = pScore->get_staffobjs_table();
        cout << pCol->dump();
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};

//---------------------------------------------------------------------------------------
SUITE(LdpExporterTest)
{
//    TEST_FIXTURE(LdpExporterTestFixture, visual)
//    {
//        //visual test to display the exported score
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)"
//            "(n c4 q)(n e4 q)(goBack 64)"
//            "(chord (n e4 e. g+ v2 (stem up))(n g4 e. v2))"
//            "(chord (n d4 s g- v2 (stem up))(n f4 s v2))"
//            ")))" );
////        doc.from_file(m_scores_path + "00205-multimetric.lms" );
////        doc.from_file(m_scores_path + "50120-fermatas.lms" );
////        doc.from_file(m_scores_path + "50051-tie-bezier.lms" );
////        doc.from_file(m_scores_path + "00110-triplet-against-5-tuplet-4.14.lms" );
////        doc.from_file(m_scores_path + "50130-metronome.lms" );
////        doc.from_file(m_scores_path + "50180-new-system-tag.lms" );
////        doc.from_file(m_scores_path + "50110-graphic-line-text.lms" );
////        doc.from_file("/datos/cecilio/Desarrollo_wx/lomse/samples/chopin_prelude20_v16.lms" );
//        ImoDocument* pRoot = doc.get_im_root();
//
//        LdpExporter exporter(&m_libraryScope);
//        exporter.set_add_id(true);
//        string source = exporter.get_source(pRoot);
//        cout << "----------------------------------------------------" << endl;
//        cout << source << endl;
//        cout << "----------------------------------------------------" << endl;
//    }

    //@ barline -------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, barline_0)
    {
        //@00. barline, type

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6) (instrument (musicData (barline end))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoBarline* pImo = static_cast<ImoBarline*>( pMD->get_child_of_type(k_imo_barline) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(barline end)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, barline_1)
    {
        //@01. barline id

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6) (instrument#100 (musicData (barline#105 end))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoBarline* pImo = static_cast<ImoBarline*>( pMD->get_child_of_type(k_imo_barline) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(barline#105 end)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, barline_2)
    {
        //@02. barline. middle

        Document doc(m_libraryScope);
        ImoBarline* pImo = static_cast<ImoBarline*>(ImFactory::inject(k_imo_barline, &doc));
        pImo->set_type(k_barline_double);
        pImo->set_middle(true);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(barline double middle)" );
        delete pImo;
    }

    TEST_FIXTURE(LdpExporterTestFixture, barline_3)
    {
        //@03. barline. no visible

        Document doc(m_libraryScope);
        ImoBarline* pImo = static_cast<ImoBarline*>(ImFactory::inject(k_imo_barline, &doc));
        pImo->set_type(k_barline_start_repetition);
        pImo->set_visible(false);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(barline startRepetition (visible no))" );
        delete pImo;
    }

    TEST_FIXTURE(LdpExporterTestFixture, barline_4)
    {
        //@04. barline. Attachments
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (barline end (text \"Hello\")) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoBarline* pImo = static_cast<ImoBarline*>(
                                        pMD->get_child_of_type(k_imo_barline) );
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(barline end (text \"Hello\"))" );
    }

    // @beam ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, beam_0)
    {
        //@00. beam
        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 1.6)(instrument#100 (musicData "
            "(clef G)(n g5 s g+)(n f5 s)(n g5 e g-)(barline))) )"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "167: \"" << source << "\"" << endl;
        string expected =
            "(musicData (clef G p1)(n g5 s v1 p1 (beam 106 ++))"
            "(n f5 s v1 p1 (beam 106 =-))(n g5 e v1 p1 (beam 106 -))(barline simple))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, beam_1)
    {
        //@01. chord: beam only in base note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#100 (musicData (clef G)"
            "(chord (n e4 e. g+ (stem up))(n g4 e.))"
            "(chord (n d4 s g- (stem up))(n f4 s))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1)"
            "(chord (n e4 e. v1 p1 (stem up)(beam 111 +))(n g4 e. v1 p1))"
            "(chord (n d4 s v1 p1 (stem up)(beam 111 -b))(n f4 s v1 p1)))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, beam_2)
    {
        //@02. chord after goBack: beam only in base note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#100 (musicData (clef G)"
            "(n c4 q)(n e4 q)(goBack 64)"
            "(chord (n e4 e. g+ v2 (stem up))(n g4 e. v2))"
            "(chord (n d4 s g- v2 (stem up))(n f4 s v2))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1)"
            "(n c4 q v1 p1)(n e4 q v1 p1)(goFwd 64 v2 p1)"
            "(chord (n e4 e. v2 p1 (stem up)(beam 114 +))(n g4 e. v2 p1))"
            "(chord (n d4 s v2 p1 (stem up)(beam 114 -b))(n f4 s v2 p1)))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, beam_3)
    {
        //@03. beamed notes must be generated in sequence
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument#100 (musicData "
            "(clef F4)(n e3 e g+)(n g3 e)(n c4 e g-)"
            "(goBack start)(n c2 w v3)(barline)"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "238: \"" << source << "\"" << endl;
        string expected = "(musicData (clef F4 p1)"
            "(n e3 e v1 p1 (beam 106 +))(n c2 w v3 p1)(n g3 e v1 p1 (beam 106 =))"
            "(n c4 e v1 p1 (beam 106 -))(barline simple))";
        CHECK( source == expected );
    }

    //@ clef ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, clef_0)
    {
        //@00 clef, type and base staffobj

        Document doc(m_libraryScope);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_F4);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pClef);
//        cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(clef F4 p1)" );
        delete pClef;
    }

    TEST_FIXTURE(LdpExporterTestFixture, clef_1)
    {
        //@01 clef & id, type and base staffobj

        Document doc(m_libraryScope);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc, 27L));
        pClef->set_clef_type(k_clef_F4);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pClef);
//        cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(clef#27 F4 p1)" );
        delete pClef;
    }

    TEST_FIXTURE(LdpExporterTestFixture, clef_2)
    {
        //@02 clef. symbolSize

        Document doc(m_libraryScope);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_F4);
        pClef->set_symbol_size(k_size_cue);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pClef);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(clef F4 (symbolSize cue) p1)" );
        delete pClef;
    }


    //@ direction -----------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, direction_0)
    {
        //@00. empty direction having width -> spacer

        Document doc(m_libraryScope);
        ImoDirection* pImo = static_cast<ImoDirection*>(
                            ImFactory::inject(k_imo_direction, &doc));
        pImo->set_width(53.2f);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(spacer 53.2 p1)" );
        delete pImo;
    }

    TEST_FIXTURE(LdpExporterTestFixture, direction_1)
    {
        //@01. direction id

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (spacer#123 50) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoDirection* pImo = static_cast<ImoDirection*>(
                                        pMD->get_child_of_type(k_imo_direction) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(spacer#123 50 p1)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, direction_2)
    {
        //@02. direction with attachments
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (spacer 50 (text \"Hello\")) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoDirection* pImo = static_cast<ImoDirection*>(
                                        pMD->get_child_of_type(k_imo_direction) );
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(dir 50 p1 (text \"Hello\"))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, direction_3)
    {
        //@03. direction, no width, no attachments -> not exported
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (spacer 0)) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoDirection* pImo = static_cast<ImoDirection*>(
                                        pMD->get_child_of_type(k_imo_direction) );
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(dir unknown)" );
    }


    //@ dynamics marks -------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, dyn_01)
    {
        //@01. dynamics marks

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(key C)"
            "(n g4  q (dyn \"fff\" below))"
            "(n g4  q (dyn \"ppp\"))"
            "(n g4  q (dyn \"sfz\" above))"
            "(n g4  q (dyn \"sfz\" above (dx 50)(dy -70)(color #ff0000)))"
            ")))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)(instrument P1 (staves 1)(musicData "
            "(clef G p1)(key C)"
            "(n g4 q v1 p1 (dyn \"fff\" below))"
            "(n g4 q v1 p1 (dyn \"ppp\"))"
            "(n g4 q v1 p1 (dyn \"sfz\" above))"
            "(n g4 q v1 p1 (dyn \"sfz\" above (color #ff0000ff)(dx 50)(dy -70)))"
            ")))";
        CHECK( source == expected );
    }


    // ContentObjLdpGenerator
    // ErrorLdpGenerator
    // FermataLdpGenerator
    // GoBackFwdLdpGenerator ------------------------------------------------------------

    //@ instrument ----------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, instrument_0)
    {
        //@00. staves
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData)))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(instrument P1 (staves 1)(musicData))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_1)
    {
        //@01. staff data
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument "
            "(staff 1 (staffType ossia)(staffLines 5)(staffSpacing 250.00)"
            "         (staffDistance 2000.00)(lineThickness 15.00))"
            "(musicData)))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(instrument P1 (staves 1)(staff 1 (staffType ossia)"
            "(staffLines 5)(staffSpacing 250)(staffDistance 2000)"
            "(lineThickness 15))(musicData))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_2)
    {
        //@02. midi data
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (infoMIDI 9 12)(musicData)))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(instrument P1 (staves 1)(infoMIDI 9 12)(musicData))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_3)
    {
        //@03. name, abbrev
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (name \"Guitar\")(abbrev \"G.\")(musicData)))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(instrument P1 (name \"Guitar\")(abbrev \"G.\")(staves 1)(musicData))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_4)
    {
        //@04. name, abbrev with style
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(defineStyle \"Best1\" (font \"Times New Roman\" 10pt bold) (color #000000))"
            "(defineStyle \"Best2\" (font \"Times New Roman\" 10pt bold) (color #000000))"
            "(instrument (name \"Guitar\" (style \"Best1\"))"
            "(abbrev \"G.\" (style \"Best2\"))(musicData)))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(instrument P1 (name \"Guitar\" (style \"Best1\"))"
            "(abbrev \"G.\" (style \"Best2\"))(staves 1)(musicData))";
        CHECK( source == expected );
    }

    //@ key -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, key_0)
    {
        //@00 key, type

        Document doc(m_libraryScope);
        ImoKeySignature* pImo = static_cast<ImoKeySignature*>(
                                        ImFactory::inject(k_imo_key_signature, &doc));
        pImo->set_key_type(k_key_A);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(key A)" );
        delete pImo;
    }

    TEST_FIXTURE(LdpExporterTestFixture, key_1)
    {
        //@01 key id
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100 (musicData (key#123 A))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoKeySignature* pImo = static_cast<ImoKeySignature*>(
                                        pMD->get_child_of_type(k_imo_key_signature) );
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(key#123 A)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, key_2)
    {
        //@02 key. Attachments
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (key A (text \"Hello\")) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoKeySignature* pImo = static_cast<ImoKeySignature*>(
                                        pMD->get_child_of_type(k_imo_key_signature) );
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(key A (text \"Hello\"))" );
    }

    // LenmusdocLdpGenerator

    //@ Lyric ---------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, lyric_0)
    {
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n c4 q (lyric 1 \"This\")(lyric 2 \"A\"))"
            "(n d4 q (lyric 1 \"is\")(lyric 2 \"se\" -))"
            "(n e4 q (lyric 1 \"line\")(lyric 2 \"cond\"))"
            "(n f4 q (lyric 1 \"one.\")(lyric 2 \"line.\"))"
            "(barline))))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(musicData (clef G p1)"
            "(n c4 q v1 p1 (lyric 1 \"This\" below)(lyric 2 \"A\" below))"
            "(n d4 q v1 p1 (lyric 1 \"is\" below)(lyric 2 \"se\" - below))"
            "(n e4 q v1 p1 (lyric 1 \"line\" below)(lyric 2 \"cond\" below))"
            "(n f4 q v1 p1 (lyric 1 \"one.\" below)(lyric 2 \"line.\" below))"
            "(barline simple))" );
    }

    //@ metronome -----------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, metronome_0)
    {
        //@00. metronome: note-value

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (metronome q 55) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoDirection* pDir = static_cast<ImoDirection*>(
                                        pMD->get_child_of_type(k_imo_direction) );
        ImoMetronomeMark* pImo = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(metronome q 55)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, metronome_1)
    {
        //@01. metronome: id, note-note

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (metronome#123 q q.) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoDirection* pDir = static_cast<ImoDirection*>(
                                        pMD->get_child_of_type(k_imo_direction) );
        ImoMetronomeMark* pImo = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(metronome#123 q q.)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, metronome_2)
    {
        //@02. metronome. parenthesis
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (metronome q 80 parenthesis) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoDirection* pDir = static_cast<ImoDirection*>(
                                        pMD->get_child_of_type(k_imo_direction) );
        ImoMetronomeMark* pImo = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(metronome q 80 parenthesis)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, metronome_3)
    {
        //@03. metronome. no visible
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (metronome q 80 noVisible) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoDirection* pDir = static_cast<ImoDirection*>(
                                        pMD->get_child_of_type(k_imo_direction) );
        ImoMetronomeMark* pImo = static_cast<ImoMetronomeMark*>(pDir->get_attachment(0));
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(metronome q 80 (visible no))" );
    }

    // MusicDataLdpGenerator ------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, musicData_0)
    {
        //empty musicData
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData)))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
//        cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(musicData)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_1)
    {
        //in time sequence ok
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (musicData (clef G)(r q)(barline)(n c4 q)) )"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(musicData (clef G p1)(r q v1 p1)(barline simple)(n c4 q v1 p1))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_2)
    {
        //skips instruments
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (musicData (clef G)(r q)(barline)(n c4 q)) )"
            "(instrument (musicData (clef C3)(n f4 e)) )"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(1);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(musicData (clef C3 p1)(n f4 e v1 p1))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_3)
    {
        //goFwd
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (musicData (clef G)(n c4 q p1)(goFwd 32)(n a3 e p1)) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source ==
            "(musicData (clef G p1)(n c4 q v1 p1)(goFwd 32 v1 p1)(n a3 e v1 p1))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_4)
    {
        //skip secondary objects (key, time)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)(n c4 e v1 p1)"
            "(goBack start)(n g2 e v3 p2)(n c3 e v3)(n e3 e v3)(n g3 e v3)(barline)) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected =
            "(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)(n c4 e v1 p1)"
            "(n g2 e v3 p2)(n c3 e v3 p2)(n e3 e v3 p2)(n g3 e v3 p2)(barline simple))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_5)
    {
        //chord: no goFwd beetween chord notes
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)"
            "(n c5 q)(chord (n c4 e)(n e4 e)(n g4 e))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1)"
            "(n c5 q v1 p1)"
            "(chord (n c4 e v1 p1)(n e4 e v1 p1)(n g4 e v1 p1)))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_6)
    {
        //chord: no goFwd beetween chord notes when chord displaced
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)"
            "(goFwd q)(chord (n c4 e)(n e4 e)(n g4 e))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1)"
            "(goFwd 64 v1 p1)"
            "(chord (n c4 e v1 p1)(n e4 e v1 p1)(n g4 e v1 p1)))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_7)
    {
        //multimetrics import 1.6 / export 2.0 example
        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 1.6)(instrument#100 (musicData "
            "(clef G)(key G)(time 3 4)(chord (n g3 q)(n d4 q))(r e)(n g5 e)"
            "(n g5 s g+)(n f5 s)(n g5 e g-)(barline)"
            "(chord (n a4 q)(n e5 q))(r q)(chord (n d4 q)(n g4 q)(n f5 q))"
            "(barline)))"
            "(instrument (musicData (clef G)(key G)(time 2 4)"
            "(n g4 q)(n d5 e g+)(n d5 e g-)(barline)"
            "(n b5 e g+)(n a5 s)(n g5 s g-)(n g5 e g+)(n g5 e g-)(barline)"
            "(n e5 e g+)(n d5 s)(n c5 s g-)(n e5 e g+)(n e5 e g-)(barline))) )"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected =
            "(musicData (clef G p1)(key G)(time 3 4)(chord (n g3 q v1 p1)(n d4 q v1 p1))"
            "(r e v1 p1)(n g5 e v1 p1)(n g5 s v1 p1 (beam 115 ++))(n f5 s v1 p1 (beam 115 =-))"
            "(n g5 e v1 p1 (beam 115 -))(barline simple)"
            "(chord (n a4 q v1 p1)(n e5 q v1 p1))(r q v1 p1)"
            "(chord (n d4 q v1 p1)(n g4 q v1 p1)(n f5 q v1 p1))(barline simple))";
        CHECK( source == expected );
    }

    //@ note ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, note_0)
    {
        //@00. tuplet exports also time-modification

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content "
            "(score (vers 1.6)(instrument#100 (musicData "
            "(clef G)(n c4 e (t + 3 2))(n e4 e)(n g4 e (t -))"
            "))) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)(instrument P1 (staves 1)(musicData "
            "(clef G p1)(n c4 e v1 p1 (tm 2 3)(t 106 + 3 2))(n e4 e v1 p1 (tm 2 3))"
            "(n g4 e v1 p1 (tm 2 3)(t 106 -))"
            ")))";

        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, note_1)
    {
        //@01. note with accidentals

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (clef G)(n +d4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoNote* pImo = static_cast<ImoNote*>(
                                        pMD->get_child_of_type(k_imo_note_regular) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(n +d4 q v1 p1)" );
    }

    // RestLdpGenerator -----------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, rest_1)
    {
        //goFwd 2.0
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content "
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(n c4 e v1)(goFwd e v1)(n e4 e v1)"
            "))) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)(instrument P1 (staves 1)(musicData "
            "(clef G p1)(n c4 e v1 p1)(goFwd e v1 p1)(n e4 e v1 p1))))";

        CHECK( source == expected );
    }

    // ScoreLdpGenerator ----------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, score_0)
    {
        //style
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)"
            "(styles (defineStyle \"Score1\" (font \"Arial\" 14pt bold)(color #00fe0f7f)))"
            "(content "
            "(score (vers 1.6)(style \"Score1\")"
            "(instrument (musicData (clef G))))"
            "))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)(style \"Score1\")"
            "(instrument P1 (staves 1)(musicData (clef G p1))))";

        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, score_1)
    {
        //defineStyle
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content (score (vers 1.6)"
            "(defineStyle \"Score1\" (font \"Arial\" 14pt bold)(color #00fe0f7f))"
            "(instrument (musicData (clef G))))"
            "))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)"
            "(defineStyle \"Score1\" (font-name \"Arial\")(font-size 14pt)"
            "(font-style normal)(font-weight bold)(color #00fe0f7f))"
            "(instrument P1 (staves 1)(musicData (clef G p1))))";

        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, score_2)
    {
        //opt
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content (score (vers 1.6)"
            "(opt Render.SpacingValue 40)(opt StaffLines.Truncate 0)"
            "(instrument (musicData (clef G))))"
            "))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected =
            "(score (vers 2.0)"
            "(opt Render.SpacingValue 40)(opt StaffLines.Truncate 0)"
            "(instrument P1 (staves 1)(musicData (clef G p1))))";

        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, score_3)
    {
        //systemLayout
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content (score (vers 1.6)"
            "(systemLayout first (systemMargins 0 0 1700 1200))"
            "(systemLayout other (systemMargins 0 0 1800 2000))"
            "(instrument (musicData (clef G))))"
            "))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected =
            "(score (vers 2.0)"
            "(systemLayout first (systemMargins 0 0 1700 1200))"
            "(systemLayout other (systemMargins 0 0 1800 2000))"
            "(instrument P1 (staves 1)(musicData (clef G p1))))";

        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, score_4)
    {
        //systemLayout
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(systemLayout first (systemMargins 0 0 1500 1500))"
            "(systemLayout other (systemMargins 0 0 1500 1000)) "
            "(opt Score.FillPageWithEmptyStaves true)"
            "(opt StaffLines.StopAtFinalBarline false) "
            "(instrument (musicData)))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected =
            "(score (vers 2.0)(systemLayout first (systemMargins 0 0 1500 1500))"
            "(systemLayout other (systemMargins 0 0 1500 1000))"
            "(opt Score.FillPageWithEmptyStaves true)"
            "(opt StaffLines.Truncate 0)"
            "(instrument P1 (staves 1)(musicData)))";

        CHECK( source == expected );
    }

    // ScoreLineLdpGenerator ------------------------------------------------------------

    // ScoreObjLdpGenerator -------------------------------------------------------------

    // ScoreTextLdpGenerator ------------------------------------------------------------

#if LOMSE_COMPATIBILITY_LDP_1_5
    TEST_FIXTURE(LdpExporterTestFixture, score_text_0)
    {
        //text
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.5)"
            "(defineStyle \"Notations\" (font \"Times New Roman\" 10pt bold) (color #000000))"
            "(instrument (musicData (clef G)"
            "(text \"Largo\" (style \"Notations\")(dx -20)(dy -45)) ))"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1)"
            "(dir 0 p1 (text \"Largo\" (style \"Notations\")(dx -20)(dy -45))))";

        CHECK( source == expected );
    }
#endif

    //@ slur ----------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, slur_01)
    {
        //@01
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (n c4 q (slur 1 start))"
            "(n e4 q (slur 1 stop)) )))"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        string expected = "(musicData (n c4 q v1 p1 (slur 1 start))"
            "(n e4 q v1 p1 (slur 1 stop)))";

//        cout << test_name() << endl;
//        cout << test_name() << endl << "\"" << source << "\"" << endl;

        CHECK( source == expected );
    }

    // StaffObjLdpGenerator -------------------------------------------------------------

    // SystemBreakLdpGenerator ----------------------------------------------------------
    //@ tie -----------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, tie_01)
    {
        //@01
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (n c4 q (tie 1 start))"
            "(n c4 q (tie 1 stop)) )))"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        string expected = "(musicData (n c4 q v1 p1 (tie 1 start))"
            "(n c4 q v1 p1 (tie 1 stop)))";

        //cout << test_name() << endl << "\"" << source << "\"" << endl;

        CHECK( source == expected );
    }

    //@ time signature ------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, time_signature_0)
    {
        //@00. time, type

        Document doc(m_libraryScope);
        ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
                            ImFactory::inject(k_imo_time_signature, &doc));
        pImo->set_type(ImoTimeSignature::k_single_number);
        pImo->set_top_number(10);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(time single-number 10)" );
        delete pImo;
    }

    TEST_FIXTURE(LdpExporterTestFixture, time_signature_1)
    {
        //@01. time id

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (time#123 5 4) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
                                        pMD->get_child_of_type(k_imo_time_signature) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(time#123 5 4)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, time_signature_2)
    {
        //@02. time. Attachments
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument#100"
                "(musicData (time common (text \"Hello\")) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
                                        pMD->get_child_of_type(k_imo_time_signature) );
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        CHECK( source == "(time common (text \"Hello\"))" );
    }

//    // score ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, score)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
//            "(instrument (musicData (clef G)(key D)(n c4 q)(barline) ))"
//            ")))" );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pScore);
//        cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source ==
//              "(score (vers 1.6)\n"
//              "   (instrument \n"
//              "      (musicData \n"
//              "         (clef G p1)\n"
//              "         (key D)\n"
//              "         (n c4 q p1)\n"
//              "         (barline )\n"
//              ")\n)\n)" );
//    }
//
//    // note ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, Note)
//    {
//        Document doc(m_libraryScope);
//        ImoNote* pImo = ImFactory::inject_note(&doc, k_step_D, k_octave_4,
//                            k_eighth, k_no_accidentals);
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImo);
//        //cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source == "(n d4 e p1)" );
//        delete pImo;
//    }
//
//    TEST_FIXTURE(LdpExporterTestFixture, Note_dots)
//    {
//        Document doc(m_libraryScope);
//        ImoNote* pImo = ImFactory::inject_note(&doc, k_step_B, k_octave_7,
//                            k_whole, k_sharp, 2);
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImo);
//        //cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source == "(n +b7 w.. p1)" );
//        delete pImo;
//    }
//
//
//    // color ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, color)
//    {
//        Document doc(m_libraryScope);
//        ImoClef* pImo = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
//        pImo->set_clef_type(k_clef_F4);
//        pImo->set_color( Color(127, 40, 12, 128) );
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImo);
////        cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source == "(clef F4 p1 (color #7f280c80))" );
//        delete pImo;
//    }
//
////    // user location ----------------------------------------------------------------------------
////
////    TEST_FIXTURE(LdpExporterTestFixture, ExportLdp_user_location)
////    {
////        ImoClef obj;
////        obj.set_type(k_clef_G2);
////        obj.set_user_location_x(30.0f);
////        obj.set_user_location_y(-7.05f);
////        LdpExporter exporter(&m_libraryScope);
////        string source = exporter.get_source(&obj);
////        cout << test_name() << endl << "\"" << source << "\"" << endl;
////        CHECK( source == "(clef G3 p1 (dx 30.0000) (dy -7.0500))" );
////    }


    //@ tuplet --------------------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, tuplet_0)
    {
        //@00. tuplet v1.6: tm implicit
        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 1.6)(instrument#100 (musicData (clef G)"
            "(n c4 e (t 100 + 2 3)(beam 110 +))"
            "(n d4 s (beam 110 =+))"
            "(n c4 s (beam 110 ==))"
            "(n b3 e (t 100 -)(beam 110 --))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected =
            "(musicData (clef G p1)"
            "(n c4 e v1 p1 (tm 3 2)(beam 112 +)(t 107 + 2 3))"
            "(n d4 s v1 p1 (tm 3 2)(beam 112 =+))"
            "(n c4 s v1 p1 (tm 3 2)(beam 112 ==))"
            "(n b3 e v1 p1 (tm 3 2)(beam 112 --)(t 107 -))"
            ")";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, tuplet_1)
    {
        //@01. tuplet v2.0: tm explicit
        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.0)(instrument#100 (musicData (clef G)"
            "(n c4 e (t 100 + 2 3)(tm 3 2)(beam 131 +))"
            "(n d4 s (beam 131 =+)(tm 3 2))"
            "(n c4 s (beam 131 ==)(tm 3 2))"
            "(n b3 e (t 100 -)(tm 3 2)(beam 131 --))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << test_name() << endl << "\"" << source << "\"" << endl;
        string expected =
            "(musicData (clef G p1)"
            "(n c4 e v1 p1 (tm 3 2)(beam 112 +)(t 107 + 2 3))"
            "(n d4 s v1 p1 (tm 3 2)(beam 112 =+))"
            "(n c4 s v1 p1 (tm 3 2)(beam 112 ==))"
            "(n b3 e v1 p1 (tm 3 2)(beam 112 --)(t 107 -))"
            ")";
        CHECK( source == expected );
    }

    //@ Order of elements in the score --------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, order_01)
    {
        //@01 Note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument#100 (staves 2)(musicData (n c4 q p1 (stem down))"
            "(n c4 q (slur 1 start))(n e4 q (slur 1 stop))"
            "(n c4 q (tie 2 start))(n c4 q (tie 2 stop))"
            "(n c4 q (lyric 1 \"This\")(lyric 2 \"A\"))"
            "(n g5 s g+)(n f5 s)(n g5 e g-)"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        string expected = "(musicData (n c4 q v1 p1 (stem down))"
            "(n c4 q v1 p1 (slur 1 start))(n e4 q v1 p1 (slur 1 stop))"
            "(n c4 q v1 p1 (tie 2 start))(n c4 q v1 p1 (tie 2 stop))"
            "(n c4 q v1 p1 (lyric 1 \"This\" below)(lyric 2 \"A\" below))"
            "(n g5 s v1 p1 (beam 126 ++))(n f5 s v1 p1 (beam 126 =-))"
            "(n g5 e v1 p1 (beam 126 -)))";

        //cout << test_name() << endl << "\"" << source << "\"" << endl;

        CHECK( source == expected );
    }

};
