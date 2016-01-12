//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2016 Cecilio Salmeron. All rights reserved.
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
#include "lomse_document.h"
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
////        doc.from_file("/datos/USR/Desarrollo_wx/lomse/samples/chopin_prelude20_v16.lms" );
//        ImoDocument* pRoot = doc.get_imodoc();
//
//        LdpExporter exporter(&m_libraryScope);
//        exporter.set_add_id(true);
//        string source = exporter.get_source(pRoot);
//        cout << "----------------------------------------------------" << endl;
//        cout << source << endl;
//        cout << "----------------------------------------------------" << endl;
//    }

    // BarlineLdpGenerator --------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, barline)
    {
        //@ barline, type

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6) (instrument (musicData (barline end))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoBarline* pImo = static_cast<ImoBarline*>( pMD->get_child_of_type(k_imo_barline) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(barline end)" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, barline_id)
    {
        //@ barline, type, id

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6) (instrument (musicData (barline end))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoBarline* pImo = static_cast<ImoBarline*>( pMD->get_child_of_type(k_imo_barline) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pImo);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(barline#22 end)" );
    }

    // BeamLdpGenerator -----------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, beam_0)
    {
        //beam
        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 1.6)(instrument (musicData "
            "(clef G)(n g5 s g+)(n f5 s)(n g5 e g-)(barline))) )"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected =
            "(musicData (clef G p1 )(n g5 s v1  p1 (beam 26 ++))"
            "(n f5 s v1  p1 (beam 26 =-))(n g5 e v1  p1 (beam 26 -))(barline simple))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, beam_1)
    {
        //chord: beam only in base note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)"
            "(chord (n e4 e. g+ (stem up))(n g4 e.))"
            "(chord (n d4 s g- (stem up))(n f4 s))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1 )"
            "(chord (n e4 e. v1  (stem up) p1 (beam 31 +))(n g4 e. v1  p1 ))"
            "(chord (n d4 s v1  (stem up) p1 (beam 31 -b))(n f4 s v1  p1 )))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, beam_2)
    {
        //chord after goBack: beam only in base note
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)"
            "(n c4 q)(n e4 q)(goBack 64)"
            "(chord (n e4 e. g+ v2 (stem up))(n g4 e. v2))"
            "(chord (n d4 s g- v2 (stem up))(n f4 s v2))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1 )"
            "(n c4 q v1  p1 )(n e4 q v1  p1 )(goFwd 64 v2 p1)"
            "(chord (n e4 e. v2  (stem up) p1 (beam 34 +))(n g4 e. v2  p1 ))"
            "(chord (n d4 s v2  (stem up) p1 (beam 34 -b))(n f4 s v2  p1 )))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, beam_3)
    {
        //beamed notes must be generated in sequence
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData "
            "(clef F4)(n e3 e g+)(n g3 e)(n c4 e g-)"
            "(goBack start)(n c2 w v3)(barline)"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef F4 p1 )"
            "(n e3 e v1  p1 (beam 26 +))(n c2 w v3  p1 )(n g3 e v1  p1 (beam 26 =))"
            "(n c4 e v1  p1 (beam 26 -))(barline simple))";
        CHECK( source == expected );
    }

    // ClefLdpGenerator -----------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, clef)
    {
        //@ clef, type and base staffobj

        Document doc(m_libraryScope);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_F4);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pClef);
//        cout << "\"" << source << "\"" << endl;
        CHECK( source == "(clef F4 p1 )" );
        delete pClef;
    }

    TEST_FIXTURE(LdpExporterTestFixture, clef_id)
    {
        //@ clef & id, type and base staffobj

        Document doc(m_libraryScope);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_F4);
        LdpExporter exporter(&m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_add_id(true);
        string source = exporter.get_source(pClef);
//        cout << "\"" << source << "\"" << endl;
        CHECK( source == "(clef#0 F4 p1 )" );
        delete pClef;
    }

    // BeamLdpGenerator
    // ContentObjLdpGenerator
    // ErrorLdpGenerator
    // FermataLdpGenerator
    // ImObjLdpGenerator
    // GoBackFwdLdpGenerator ------------------------------------------------------------

    // InstrumentLdpGenerator -----------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, instrument_0)
    {
        //staves
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(instrument (staves 1)(musicData ))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_1)
    {
        //staff data
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument "
            "(staff 1 (staffType ossia)(staffLines 5)(staffSpacing 250.00)"
            "         (staffDistance 2000.00)(lineThickness 15.00))"
            "(musicData )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(instrument (staves 1)(staff 1 (staffType ossia)"
            "(staffLines 5)(staffSpacing 250)(staffDistance 2000)"
            "(lineThickness 15))(musicData ))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_2)
    {
        //midi data
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (infoMIDI 9 12)(musicData )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(instrument (staves 1)(infoMIDI 9 12)(musicData ))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_3)
    {
        //name, abbrev
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (name \"Guitar\")(abbrev \"G.\")(musicData )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(instrument (name \"Guitar\")(abbrev \"G.\")(staves 1)(musicData ))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, instrument_4)
    {
        //name, abbrev with style
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(defineStyle \"Best1\" (font \"Times New Roman\" 10pt bold) (color #000000))"
            "(defineStyle \"Best2\" (font \"Times New Roman\" 10pt bold) (color #000000))"
            "(instrument (name \"Guitar\" (style \"Best1\"))"
            "(abbrev \"G.\" (style \"Best2\"))(musicData )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pInstr);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(instrument (name \"Guitar\" (style \"Best1\"))"
            "(abbrev \"G.\" (style \"Best2\"))(staves 1)(musicData ))";
        CHECK( source == expected );
    }

    // KeySignatureLdpGenerator ---------------------------------------------------------
    // LenmusdocLdpGenerator
    // MetronomeLdpGenerator

    // MusicDataLdpGenerator ------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, musicData_0)
    {
        //empty musicData
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData )))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
//        cout << "\"" << source << "\"" << endl;
        CHECK( source == "(musicData )" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_1)
    {
        //in time sequence ok
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (musicData (clef G)(r q)(barline)(n c4 q)) )"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(musicData (clef G p1 )(r q v1  p1 )(barline simple)(n c4 q v1  p1 ))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_2)
    {
        //skips instruments
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (musicData (clef G)(r q)(barline)(n c4 q)) )"
            "(instrument (musicData (clef C3)(n f4 e)) )"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(1);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source == "(musicData (clef C3 p1 )(n f4 e v1  p1 ))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_3)
    {
        //goFwd
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (musicData (clef G)(n c4 q p1)(goFwd 32)(n a3 e p1)) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        CHECK( source ==
            "(musicData (clef G p1 )(n c4 q v1  p1 )(goFwd 32 v1 p1)(n a3 e v1  p1 ))" );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_4)
    {
        //skip secondary objects (key, time)
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)(n c4 e v1 p1)"
            "(goBack start)(n g2 e v3 p2)(n c3 e v3)(n e3 e v3)(n g3 e v3)(barline)) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected =
            "(musicData "
            "(clef G p1 )(clef F4 p2 )(key C)(time 2 4)(n c4 e v1  p1 )"
            "(n g2 e v3  p2 )(n c3 e v3  p2 )(n e3 e v3  p2 )(n g3 e v3  p2 )(barline simple))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_5)
    {
        //chord: no goFwd beetween chord notes
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)"
            "(n c5 q)(chord (n c4 e)(n e4 e)(n g4 e))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1 )"
            "(n c5 q v1  p1 )"
            "(chord (n c4 e v1  p1 )(n e4 e v1  p1 )(n g4 e v1  p1 )))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_6)
    {
        //chord: no goFwd beetween chord notes when chord displaced
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)(instrument (musicData (clef G)"
            "(goFwd q)(chord (n c4 e)(n e4 e)(n g4 e))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        //dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1 )"
            "(goFwd 64 v1 p1)"
            "(chord (n c4 e v1  p1 )(n e4 e v1  p1 )(n g4 e v1  p1 )))";
        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, musicData_7)
    {
        //multimetrics import 1.6 / export 2.0 example
        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 1.6)(instrument (musicData "
            "(clef G)(key G)(time 3 4)(chord (n g3 q)(n d4 q))(r e)(n g5 e)"
            "(n g5 s g+)(n f5 s)(n g5 e g-)(barline)"
            "(chord (n a4 q)(n e5 q))(r q)(chord (n d4 q)(n g4 q)(n f5 q))"
            "(barline)))"
            "(instrument (musicData (clef G)(key G)(time 2 4)"
            "(n g4 q)(n d5 e g+)(n d5 e g-)(barline)"
            "(n b5 e g+)(n a5 s)(n g5 s g-)(n g5 e g+)(n g5 e g-)(barline)"
            "(n e5 e g+)(n d5 s)(n c5 s g-)(n e5 e g+)(n e5 e g-)(barline))) )"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        dump_colection(pScore);
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected =
            "(musicData (clef G p1 )(key G)(time 3 4)(chord (n g3 q v1  p1 )(n d4 q v1  p1 ))"
            "(r e v1  p1 )(n g5 e v1  p1 )(n g5 s v1  p1 (beam 35 ++))(n f5 s v1  p1 (beam 35 =-))"
            "(n g5 e v1  p1 (beam 35 -))(barline simple)"
            "(chord (n a4 q v1  p1 )(n e5 q v1  p1 ))(r q v1  p1 )"
            "(chord (n d4 q v1  p1 )(n g4 q v1  p1 )(n f5 q v1  p1 ))(barline simple))";
        CHECK( source == expected );
    }

    // NoteLdpGenerator -----------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, note_0)
    {
        //tuplet exports also time-modification
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content "
            "(score (vers 1.6)(instrument (musicData "
            "(clef G)(n c4 e (t + 3 2))(n e4 e)(n g4 e (t -))"
            "))) ))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)(instrument (staves 1)(musicData "
            "(clef G p1 )(n c4 e v1 (tm 2 3) p1 (t + 3 2))(n e4 e v1 (tm 2 3) p1 )"
            "(n g4 e v1 (tm 2 3) p1 (t -))"
            ")))";

        CHECK( source == expected );
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
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)(instrument (staves 1)(musicData "
            "(clef G p1 )(n c4 e v1  p1 )(goFwd e v1 p1)(n e4 e v1  p1 ))))";

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
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0) (style \"Score1\")"
            "(instrument (staves 1)(musicData (clef G p1 ))))";

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
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(score (vers 2.0)"
            "(defineStyle \"Score1\" (font-name \"Arial\")(font-size 14pt)"
            "(font-style normal)(font-weight bold)(color #00fe0f7f))"
            "(instrument (staves 1)(musicData (clef G p1 ))))";

        CHECK( source == expected );
    }

    TEST_FIXTURE(LdpExporterTestFixture, score_2)
    {
        //opt
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0)(content (score (vers 1.6)"
            "(opt Render.SpacingValue 40)(opt StaffLines.StopAtFinalBarline false)"
            "(instrument (musicData (clef G))))"
            "))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        //cout << "\"" << source << "\"" << endl;
        string expected =
            "(score (vers 2.0)"
            "(opt Render.SpacingValue 40)(opt StaffLines.StopAtFinalBarline false)"
            "(instrument (staves 1)(musicData (clef G p1 ))))";

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
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
//        cout << "\"" << source << "\"" << endl;
        string expected =
            "(score (vers 2.0)"
            "(systemLayout first (systemMargins 0 0 1700 1200))"
            "(systemLayout other (systemMargins 0 0 1800 2000))"
            "(instrument (staves 1)(musicData (clef G p1 ))))";

        CHECK( source == expected );
    }

    // ScoreLineLdpGenerator ------------------------------------------------------------

    // ScoreObjLdpGenerator -------------------------------------------------------------

    // ScoreTextLdpGenerator ------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, score_text_0)
    {
        //text
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 1.6)"
            "(defineStyle \"Notations\" (font \"Times New Roman\" 10pt bold) (color #000000))"
            "(instrument (musicData (clef G)"
            "(text \"Largo\" (style \"Notations\")(dx -20)(dy -45)) ))"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1 )"
            "(spacer 0 p1 (text \"Largo\" (style \"Notations\")(dx -20)(dy -45))))";

        CHECK( source == expected );
    }

    // StaffObjLdpGenerator -------------------------------------------------------------

    // SystemBreakLdpGenerator ----------------------------------------------------------
    // SpacerLdpGenerator
    // TieLdpGenerator
    // TimeSignatureLdpGenerator
    // TupletLdpGenerator

//    // lenmusdoc ----------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, lenmusdoc_empty)
//    {
//        Document doc(m_libraryScope);
//        ImoDocument* pImoDoc = static_cast<ImoDocument*>(
//                                        ImFactory::inject(k_imo_document, &doc));
//        pImoDoc->set_version("2.3");
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pImoDoc);
////        cout << source << endl;
//        CHECK( source ==
//            "(lenmusdoc (vers 2.3)\n"
//            "   //LDP file generated by Lomse, version \n"
//            "   (content\n"
//            "))\n"
//        );
//        delete pImoDoc;
//    }
//
//    TEST_FIXTURE(LdpExporterTestFixture, ErrorNotImplemented)
//    {
//        Document doc(m_libraryScope);
//        ImoTie* pTie = static_cast<ImoTie*>(ImFactory::inject(k_imo_tie, &doc));
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pTie);
////        cout << source << endl;
//        CHECK( source == "(TODO: tie   type=76, id=0 )\n" );
//        delete pTie;
//    }
//
//    // score ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(LdpExporterTestFixture, score)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
//            "(instrument (musicData (clef G)(key D)(n c4 q)(barline) ))"
//            ")))" );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        LdpExporter exporter(&m_libraryScope);
//        string source = exporter.get_source(pScore);
//        cout << "\"" << source << "\"" << endl;
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
//        //cout << "\"" << source << "\"" << endl;
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
//        //cout << "\"" << source << "\"" << endl;
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
////        cout << "\"" << source << "\"" << endl;
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
////        cout << "\"" << source << "\"" << endl;
////        CHECK( source == "(clef G3 p1 (dx 30.0000) (dy -7.0500))" );
////    }

    // TimeSignatureLdpGenerator ------------------------------------------------------------

    TEST_FIXTURE(LdpExporterTestFixture, time_signature_0)
    {
        //text
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(key C)(time common)(n c4 q)))"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        LdpExporter exporter(&m_libraryScope);
        exporter.set_current_score(pScore);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pMD);
        //cout << "\"" << source << "\"" << endl;
        string expected = "(musicData (clef G p1 )"
            "(key C)(time common)(n c4 q v1  p1 ))";

        CHECK( source == expected );
    }

};
