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
#include "lomse_document.h"
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_ldp_compiler.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"
#include "lomse_staffobjs_table.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_xml_parser.h"
#include "lomse_mxl_analyser.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class ModelBuilderTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ModelBuilderTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ModelBuilderTestFixture()    //TearDown fixture
    {
    }
};

SUITE(ModelBuilderTest)
{

    TEST_FIXTURE(ModelBuilderTestFixture, ModelBuilderScore)
    {
        //This just checks that compiler creates a model builder and it
        //structurizes the score (creates the associated ColStaffObjs
        // and the JumpsTable objects)

        Document doc(m_libraryScope);
        //ModelBuilder* builder = Injector::inject_ModelBuilder(doc.get_scope());
        LdpCompiler compiler(m_libraryScope, &doc);
        InternalModel* pIModel = compiler.compile_string(
            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
            "(instrument (musicData (n c4 q) (barline simple))))))" );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pIModel->get_root());
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_staffobjs_table() != NULL );
//        CHECK( pScore->get_jumps_table() != NULL );

        delete pIModel;
    }

}


//=======================================================================================
class PitchAssignerTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    PitchAssignerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~PitchAssignerTestFixture()    //TearDown fixture
    {
    }
};

SUITE(PitchAssignerTest)
{

    TEST_FIXTURE(PitchAssignerTestFixture, AssignPitch_FPitch)
    {
        Document doc(m_libraryScope);
        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, &doc));
        ImoInstrument* pInstr = pScore->add_instrument();
        pInstr->add_clef(k_clef_G2);
        pInstr->add_key_signature(k_key_D);
        pInstr->add_time_signature(4 ,4, k_no_visible);
        ImoNote* pNote1 = static_cast<ImoNote*>( pInstr->add_object("(n f4 q)") );
        ImoNote* pNote2 = static_cast<ImoNote*>( pInstr->add_object("(n +d4 q)") );
        ImoNote* pNote3 = static_cast<ImoNote*>( pInstr->add_object("(n d4 q)") );
        pInstr->add_barline(k_barline_end);

        ColStaffObjsBuilder builder;
        builder.build(pScore);

        CHECK( pNote1->get_fpitch() == k_undefined_fpitch );
        CHECK( pNote2->get_fpitch() == k_undefined_fpitch );
        CHECK( pNote3->get_fpitch() == k_undefined_fpitch );

        PitchAssigner tuner;
        tuner.assign_pitch(pScore);

        CHECK( pNote1->get_fpitch() == FPitch("+f4") );
        CHECK( pNote2->get_fpitch() == FPitch("+d4") );
        CHECK( pNote3->get_fpitch() == FPitch("+d4") );

        delete pScore;
    }

    TEST_FIXTURE(PitchAssignerTestFixture, AssignPitch_MidiPitch)
    {
        Document doc(m_libraryScope);
        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, &doc));
        ImoInstrument* pInstr = pScore->add_instrument();
        pInstr->add_clef(k_clef_G2);
        pInstr->add_key_signature(k_key_D);
        pInstr->add_time_signature(4 ,4, k_no_visible);
        ImoNote* pNote1 = static_cast<ImoNote*>( pInstr->add_object("(n f4 q)") );
        ImoNote* pNote2 = static_cast<ImoNote*>( pInstr->add_object("(n +d4 q)") );
        ImoNote* pNote3 = static_cast<ImoNote*>( pInstr->add_object("(n d4 q)") );
        pInstr->add_barline(k_barline_end);

        ColStaffObjsBuilder builder;
        builder.build(pScore);

        CHECK( pNote1->get_midi_pitch() == k_undefined_midi_pitch );
        CHECK( pNote2->get_midi_pitch() == k_undefined_midi_pitch );
        CHECK( pNote3->get_midi_pitch() == k_undefined_midi_pitch );

        PitchAssigner tuner;
        tuner.assign_pitch(pScore);

        CHECK( pNote1->get_midi_pitch() == MidiPitch(k_step_F, k_octave_4, +1) );
        CHECK( pNote2->get_midi_pitch() == MidiPitch(k_step_D, k_octave_4, +1) );
        CHECK( pNote3->get_midi_pitch() == MidiPitch(k_step_D, k_octave_4, +1) );

        delete pScore;
   }

    TEST_FIXTURE(PitchAssignerTestFixture, CompilerAssignsPitch)
    {
        Document doc(m_libraryScope);
        doc.from_string(
            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
            "(instrument (musicData (clef G)(key D)(n f4 q)(n +d4 q)(n d4 q)(barline) ))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        while(!cursor.is_end() && !cursor.get_staffobj()->is_note())
        {
            cursor.move_next();
        }
        ImoNote* pNote1 = static_cast<ImoNote*>( cursor.get_staffobj() );
        cursor.move_next();
        ImoNote* pNote2 = static_cast<ImoNote*>( cursor.get_staffobj() );
        cursor.move_next();
        ImoNote* pNote3 = static_cast<ImoNote*>( cursor.get_staffobj() );

        CHECK( pNote1->get_fpitch() == FPitch("+f4") );
        CHECK( pNote2->get_fpitch() == FPitch("+d4") );
        CHECK( pNote3->get_fpitch() == FPitch("+d4") );
    }

    TEST_FIXTURE(PitchAssignerTestFixture, CloseScoreAssignsPitch_FPitch)
    {
        Document doc(m_libraryScope);
        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, &doc));
        ImoInstrument* pInstr = pScore->add_instrument();
        pInstr->add_clef(k_clef_G2);
        pInstr->add_key_signature(k_key_D);
        pInstr->add_time_signature(4 ,4, k_no_visible);
        ImoNote* pNote1 = static_cast<ImoNote*>( pInstr->add_object("(n f4 q)") );
        ImoNote* pNote2 = static_cast<ImoNote*>( pInstr->add_object("(n +d4 q)") );
        ImoNote* pNote3 = static_cast<ImoNote*>( pInstr->add_object("(n d4 q)") );
        pInstr->add_barline(k_barline_end);

        CHECK( pNote1->get_fpitch() == k_undefined_fpitch );
        CHECK( pNote2->get_fpitch() == k_undefined_fpitch );
        CHECK( pNote3->get_fpitch() == k_undefined_fpitch );

        pScore->close();

        CHECK( pNote1->get_fpitch() == FPitch("+f4") );
        CHECK( pNote2->get_fpitch() == FPitch("+d4") );
        CHECK( pNote3->get_fpitch() == FPitch("+d4") );

        delete pScore;
    }

    TEST_FIXTURE(PitchAssignerTestFixture, NoPitchWhenPattern)
    {
        Document doc(m_libraryScope);
        doc.from_string(
            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
            "(instrument (musicData (clef G)(key D)(n * q)(n * q)(n * q)(barline) ))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        while(!cursor.is_end() && !cursor.get_staffobj()->is_note())
        {
            cursor.move_next();
        }
        ImoNote* pNote1 = static_cast<ImoNote*>( cursor.get_staffobj() );
        cursor.move_next();
        ImoNote* pNote2 = static_cast<ImoNote*>( cursor.get_staffobj() );
        cursor.move_next();
        ImoNote* pNote3 = static_cast<ImoNote*>( cursor.get_staffobj() );

        CHECK( pNote1->get_fpitch() == k_undefined_fpitch );
        CHECK( pNote2->get_fpitch() == k_undefined_fpitch );
        CHECK( pNote3->get_fpitch() == k_undefined_fpitch );
    }

};


//=======================================================================================
// MidiAssigner tests
//=======================================================================================

//---------------------------------------------------------------------------------------
//Derived class to access protected members
class MyMidiAssigner : public MidiAssigner
{
protected:

public:
    MyMidiAssigner()
        : MidiAssigner()
    {
    }
    virtual ~MyMidiAssigner() {}

    //access to protected member methods
    void my_collect_sounds_info(ImoScore* pScore) { collect_sounds_info(pScore); }
    void my_assign_score_instr_id() { assign_score_instr_id(); }
    int my_num_sounds() { return int(m_sounds.size()); }
    list<ImoSoundInfo*>& get_sounds() { return m_sounds; }
};

//---------------------------------------------------------------------------------------
// MidiAssigner test fixture
//---------------------------------------------------------------------------------------
class MidiAssignerTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    MidiAssignerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~MidiAssignerTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }
};

SUITE(MidiAssignerTest)
{

    TEST_FIXTURE(MidiAssignerTestFixture, midi_assigner_01)
    {
        //@01. collect_sound_info() adds sound info when missing

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<part-list>"
                "<score-part id='P1'>"
                    "<part-name>Music</part-name>"
                "</score-part>"
            "</part-list>"
            "<part id='P1'></part>"
            "</score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.my_collect_sounds_info(pScore);
        assigner.my_assign_score_instr_id();

        CHECK( assigner.my_num_sounds() == 1 );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
    }

    TEST_FIXTURE(MidiAssignerTestFixture, midi_assigner_02)
    {
        //@02. collect_sound_info() adds sound info when missing. Many instruments

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<part-list>"
                "<score-part id='P1'>"
                    "<part-name>Music</part-name>"
                "</score-part>"
                "<score-part id='P2'>"
                "</score-part>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.my_collect_sounds_info(pScore);
        assigner.my_assign_score_instr_id();

        CHECK( assigner.my_num_sounds() == 2 );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
        pInstr = pScore->get_instrument(1);
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "SOUND-2" );
        list<ImoSoundInfo*>& sounds = assigner.get_sounds();
        list<ImoSoundInfo*>::iterator it = sounds.begin();
        CHECK( (*it)->get_score_instr_id() == "SOUND-1" );
        ++it;
        CHECK( (*it)->get_score_instr_id() == "SOUND-2" );
    }

    TEST_FIXTURE(MidiAssignerTestFixture, midi_assigner_03)
    {
        //@03. collect_sound_info() collects all sound info elements and
        //@    adds sound info when missing.

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<score-partwise version='3.0'>"
            "<part-list>"
                "<score-part id='P1'>"
                    "<part-name>Music</part-name>"
                    "<score-instrument id='P1-I1'>"
                        "<instrument-name>Marimba</instrument-name>"
                    "</score-instrument>"
                    "<midi-instrument id='P1-I1'>"
                        "<midi-channel>1</midi-channel>"
                    "</midi-instrument>"
                "</score-part>"
                "<score-part id='P2'>"
                "</score-part>"
            "</part-list>"
            "<part id='P1'></part>"
            "<part id='P2'></part>"
            "</score-partwise>");
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
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
        CHECK( pInstr->get_num_sounds() == 1 );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.my_collect_sounds_info(pScore);
        assigner.my_assign_score_instr_id();

        CHECK( assigner.my_num_sounds() == 2 );
        pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr->get_num_sounds() == 1 );
        CHECK( pInfo != NULL );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
        list<ImoSoundInfo*>& sounds = assigner.get_sounds();
        list<ImoSoundInfo*>::iterator it = sounds.begin();
        CHECK( (*it)->get_score_instr_id() == "P1-I1" );
        ++it;
        CHECK( (*it)->get_score_instr_id() == "SOUND-1" );
    }

    TEST_FIXTURE(MidiAssignerTestFixture, midi_assigner_04)
    {
        //@04. channel not modified when port missing

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        parser.parse_text("(score (vers 2.0) (instrument (infoMIDI 56 2)"
            "(musicData (metronome q 55) )))" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "" );
        CHECK( pInfo->get_midi_channel() == 2 );
        CHECK( pInfo->get_midi_port() == -1 );

        MyMidiAssigner assigner;
        assigner.assign_midi_data(pScore);

        CHECK( pInstr->get_num_sounds() == 1 );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_midi_channel() == 2 );
        CHECK( pInfo->get_midi_port() == 0 );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
    }

    TEST_FIXTURE(MidiAssignerTestFixture, midi_assigner_05)
    {
        //@05. assign channel and port

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        parser.parse_text("(score (vers 2.0)"
            "(instrument (infoMIDI 56 0)(musicData))"
            "(instrument (musicData))"
            ")" );
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo->get_score_instr_id() == "" );
        CHECK( pInfo->get_midi_channel() == 0 );
        CHECK( pInfo->get_midi_port() == -1 );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_sounds() == 1 );

        MyMidiAssigner assigner;
        assigner.assign_midi_data(pScore);

        CHECK( assigner.my_num_sounds() == 2 );

        pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_sounds() == 1 );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_midi_channel() == 0 );
        CHECK( pInfo->get_midi_port() == 0 );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );

        pInstr = pScore->get_instrument(1);
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "SOUND-2" );
        CHECK( pInfo->get_midi_channel() == 1 );
        CHECK( pInfo->get_midi_port() == 0 );
    }

    TEST_FIXTURE(MidiAssignerTestFixture, midi_assigner_06)
    {
        //@06. assign channel when port specified

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<score-partwise version='3.0'>"
                "<part-list>"
                    "<score-part id='P1'>"
                        "<part-name>Music</part-name>"
                        "<score-instrument id='P1-I1'>"
                            "<instrument-name>Marimba</instrument-name>"
                        "</score-instrument>"
                        "<midi-device id='P1-I1' port='3'>SoundCard</midi-device>"
                    "</score-part>"
                    "<score-part id='P2'>"
                    "</score-part>"
                "</part-list>"
                "<part id='P1'></part>"
                "<part id='P2'></part>"
            "</score-partwise>"
        );
        MxlAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
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
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_midi_channel() == -1 );
        CHECK( pInfo->get_midi_port() == 2 );

        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != NULL );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.assign_midi_data(pScore);

        CHECK( assigner.my_num_sounds() == 2 );

        pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_sounds() == 1 );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        CHECK( pInfo->get_midi_channel() == 0 );
        CHECK( pInfo->get_midi_port() == 2 );

        pInstr = pScore->get_instrument(1);
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != NULL );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
        CHECK( pInfo->get_midi_channel() == 0 );
        CHECK( pInfo->get_midi_port() == 0 );
    }

//"<part-list>"
//	"<score-part id='P1'>"
//		"<part-name>Drums</part-name>"
//		"<score-instrument id='P1-X4'>"
//			"<instrument-name>Snare Drum</instrument-name>"
//		"</score-instrument>"
//		"<score-instrument id='P1-X2'>"
//			"<instrument-name>Kick Drum</instrument-name>"
//		"</score-instrument>"
//		"<score-instrument id='P1-X13'>"
//			"<instrument-name>Crash Cymbal</instrument-name>"
//		"</score-instrument>"
//		"<score-instrument id='P1-X6'>"
//			"<instrument-name>Hi-Hat%g Closed</instrument-name>"
//		"</score-instrument>"
//		"<midi-instrument id='P1-X4'>"
//			"<midi-channel>10</midi-channel>"
//			"<midi-program>1</midi-program>"
//			"<midi-unpitched>39</midi-unpitched>"
//		"</midi-instrument>"
//		"<midi-instrument id='P1-X2'>"
//			"<midi-channel>10</midi-channel>"
//			"<midi-program>1</midi-program>"
//			"<midi-unpitched>37</midi-unpitched>"
//		"</midi-instrument>"
//		"<midi-instrument id='P1-X13'>"
//			"<midi-channel>10</midi-channel>"
//			"<midi-program>1</midi-program>"
//			"<midi-unpitched>50</midi-unpitched>"
//		"</midi-instrument>"
//		"<midi-instrument id='P1-X6'>"
//			"<midi-channel>10</midi-channel>"
//			"<midi-program>1</midi-program>"
//			"<midi-unpitched>43</midi-unpitched>"
//		"</midi-instrument>"
//	"</score-part>"
//	"<score-part id='P2'>"
//		"<part-name>Cowbell</part-name>"
//		"<score-instrument id='P2-X1'>"
//			"<instrument-name>Cowbell</instrument-name>"
//		"</score-instrument>"
//		"<midi-instrument id='P2-X1'>"
//			"<midi-channel>10</midi-channel>"
//			"<midi-program>1</midi-program>"
//			"<midi-unpitched>57</midi-unpitched>"
//		"</midi-instrument>"
//	"</score-part>"
//"</part-list>"

};




