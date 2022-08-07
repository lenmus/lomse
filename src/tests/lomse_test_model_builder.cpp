//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "private/lomse_document_p.h"
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
#include "lomse_im_measures_table.h"

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
        //This just checks that compiler creates a model builder and
        //structurizes the score (that is, creates the associated ColStaffObjs)

        Document doc(m_libraryScope);
        //ModelBuilder* builder = Injector::inject_ModelBuilder(doc.get_scope());
        LdpCompiler compiler(m_libraryScope, &doc);
        ImoObj* pRoot =  compiler.compile_string(
            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
            "(instrument (musicData (n c4 q) (barline simple))))))" );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>(pRoot);
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_staffobjs_table() != nullptr );

        if (pRoot && !pRoot->is_document()) delete pRoot;
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

    TEST_FIXTURE(PitchAssignerTestFixture, AssignPitch_FPitch_01)
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

    TEST_FIXTURE(PitchAssignerTestFixture, AssignPitch_MidiPitch_02)
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

    TEST_FIXTURE(PitchAssignerTestFixture, CompilerAssignsPitch_03)
    {
        Document doc(m_libraryScope);
        doc.from_string(
            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
            "(instrument (musicData (clef G)(key D)(n f4 q)(n +d4 q)(n d4 q)(barline) ))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
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

    TEST_FIXTURE(PitchAssignerTestFixture, CloseScoreAssignsPitch_FPitch_04)
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

        pScore->end_of_changes();

        CHECK( pNote1->get_fpitch() == FPitch("+f4") );
        CHECK( pNote2->get_fpitch() == FPitch("+d4") );
        CHECK( pNote3->get_fpitch() == FPitch("+d4") );

        delete pScore;
    }

    TEST_FIXTURE(PitchAssignerTestFixture, NoPitchWhenPattern_05)
    {
        Document doc(m_libraryScope);
        doc.from_string(
            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
            "(instrument (musicData (clef G)(key D)(n * q)(n * q)(n * q)(barline) ))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
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

    TEST_FIXTURE(PitchAssignerTestFixture, pitch_assigner_06)
    {
        //@06. MusicXML. One instrument
        Document doc(m_libraryScope);
        doc.from_string(    //score 50301-pitch.xml
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes><key><fifths>-3</fifths></key>"
                "<time><beats>3</beats><beat-type>4</beat-type></time>"
                "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>A</step><alter>-1</alter><octave>4</octave></pitch>"
                "<duration>12</duration><type>quarter</type></note>"
            "<note><pitch><step>A</step><octave>4</octave></pitch>"
                "<duration>12</duration><type>quarter</type>"
                "<accidental>natural</accidental></note>"
            "<note><pitch><step>A</step><alter>-1</alter><octave>4</octave></pitch>"
                "<duration>12</duration><type>quarter</type>"
                "<accidental>flat</accidental></note>"
            "</measure>"
            "</part></score-partwise>"
            , Document::k_format_mxl);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
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

        CHECK( pNote1->get_notated_accidentals() == k_no_accidentals );
        CHECK( pNote2->get_notated_accidentals() == k_natural );
        CHECK( pNote3->get_notated_accidentals() == k_flat );
    }

    TEST_FIXTURE(PitchAssignerTestFixture, pitch_assigner_07)
    {
        //@07. MusicXML. Two instruments
        Document doc(m_libraryScope);
        doc.from_string(    //score 50302-pitch.xml
            "<score-partwise version='3.1'><part-list>"
                "<score-part id='P1'><part-name>Voice</part-name></score-part>"
                "<score-part id='P2'><part-name>Piano</part-name></score-part>"
                "</part-list>"
            "<part id='P1'><measure number='1'>"
                "<attributes><divisions>24</divisions><key><fifths>-3</fifths></key>"
                    "<time><beats>3</beats><beat-type>4</beat-type></time>"
                    "<clef><sign>G</sign><line>2</line></clef></attributes>"
                "<note><pitch><step>B</step><alter>-1</alter><octave>4</octave></pitch>"
                    "<duration>24</duration><type>quarter</type></note>"
                "<note><pitch><step>A</step><octave>4</octave></pitch>"
                    "<duration>24</duration><type>quarter</type><accidental>natural</accidental></note>"
                "<note><pitch><step>A</step><alter>-1</alter><octave>4</octave></pitch>"
                    "<duration>24</duration><type>quarter</type><accidental>flat</accidental></note>"
            "</measure></part>"
            "<part id='P2'><measure number='1'>"
                "<attributes><divisions>24</divisions><key><fifths>-3</fifths></key>"
                    "<time><beats>3</beats><beat-type>4</beat-type></time>"
                    "<clef><sign>G</sign><line>2</line></clef></attributes>"
                "<note><pitch><step>B</step><alter>-1</alter><octave>4</octave></pitch>"
                    "<duration>24</duration><type>quarter</type></note>"
                "<note><pitch><step>A</step><octave>4</octave></pitch>"
                    "<duration>24</duration><type>quarter</type><accidental>natural</accidental></note>"
                "<note><pitch><step>A</step><alter>-1</alter><octave>4</octave></pitch>"
                    "<duration>24</duration><type>quarter</type><accidental>flat</accidental></note>"
                "</measure>"
            "</part></score-partwise>"
            , Document::k_format_mxl);

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_lines() == 2 );
        CHECK( pTable->num_entries() == 14 );
        CHECK( pTable->is_anacrusis_start() == false );

        ColStaffObjsIterator it = pTable->begin();
        while(!(*it)->imo_object()->is_note())
            ++it;

        ImoNote* pNote = static_cast<ImoNote*>( (*it)->imo_object() );
        CHECK( pNote->get_step() == k_step_B );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );

        ++it;
        pNote = static_cast<ImoNote*>( (*it)->imo_object() );
        CHECK( pNote->get_step() == k_step_B );
        CHECK( pNote->get_notated_accidentals() == k_no_accidentals );

        ++it;
        pNote = static_cast<ImoNote*>( (*it)->imo_object() );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->get_notated_accidentals() == k_natural );

        ++it;
        pNote = static_cast<ImoNote*>( (*it)->imo_object() );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->get_notated_accidentals() == k_natural );

        ++it;
        pNote = static_cast<ImoNote*>( (*it)->imo_object() );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->get_notated_accidentals() == k_flat );

        ++it;
        pNote = static_cast<ImoNote*>( (*it)->imo_object() );
        CHECK( pNote->get_step() == k_step_A );
        CHECK( pNote->get_notated_accidentals() == k_flat );
    }

    TEST_FIXTURE(PitchAssignerTestFixture, pitch_assigner_08)
    {
        //@08. Incoherent accidentals.
        Document doc(m_libraryScope);
        doc.from_string(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(key C)(n xc4 q)(n +c4 q)(barline)"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor cursor(pScore);
        while(!cursor.is_end() && !cursor.get_staffobj()->is_note())
        {
            cursor.move_next();
        }
        ImoNote* pNote1 = static_cast<ImoNote*>( cursor.get_staffobj() );
        cursor.move_next();
        ImoNote* pNote2 = static_cast<ImoNote*>( cursor.get_staffobj() );

        CHECK( pNote1->get_notated_accidentals() == k_double_sharp );
        CHECK( pNote2->get_notated_accidentals() == k_sharp );
    }

    TEST_FIXTURE(PitchAssignerTestFixture, pitch_assigner_09)
    {
        //@09. issue with natural after barline: context is reset when no key sign.

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/other/01-issue-with-natural.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = dynamic_cast<ImoScore*>( doc.get_content_item(0) );
        CHECK( pScore != nullptr );
        StaffObjsCursor cursor(pScore);
        while(!cursor.is_end() && !cursor.get_staffobj()->is_note())
        {
            cursor.move_next();
        }
        ImoNote* pNote1 = static_cast<ImoNote*>( cursor.get_staffobj() );
        cursor.move_next();     //barline
        cursor.move_next();
        ImoNote* pNote2 = static_cast<ImoNote*>( cursor.get_staffobj() );

        CHECK( pNote1->get_notated_accidentals() == k_sharp );
        CHECK( pNote2->get_notated_accidentals() == k_no_accidentals );
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        CHECK( pRoot != nullptr);
        CHECK( pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.my_collect_sounds_info(pScore);
        assigner.my_assign_score_instr_id();

        CHECK( assigner.my_num_sounds() == 1 );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );

        delete pRoot;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        CHECK( pRoot != nullptr);
        CHECK( pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.my_collect_sounds_info(pScore);
        assigner.my_assign_score_instr_id();

        CHECK( assigner.my_num_sounds() == 2 );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
        pInstr = pScore->get_instrument(1);
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "SOUND-2" );
        list<ImoSoundInfo*>& sounds = assigner.get_sounds();
        list<ImoSoundInfo*>::iterator it = sounds.begin();
        CHECK( (*it)->get_score_instr_id() == "SOUND-1" );
        ++it;
        CHECK( (*it)->get_score_instr_id() == "SOUND-2" );

        delete pRoot;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        CHECK( pRoot != nullptr);
        CHECK( pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc != nullptr );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.my_collect_sounds_info(pScore);
        assigner.my_assign_score_instr_id();

        CHECK( assigner.my_num_sounds() == 2 );
        pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr->get_num_sounds() == 1 );
        CHECK( pInfo != nullptr );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
        list<ImoSoundInfo*>& sounds = assigner.get_sounds();
        list<ImoSoundInfo*>::iterator it = sounds.begin();
        CHECK( (*it)->get_score_instr_id() == "P1-I1" );
        ++it;
        CHECK( (*it)->get_score_instr_id() == "SOUND-1" );

        delete pRoot;
    }

    TEST_FIXTURE(MidiAssignerTestFixture, midi_assigner_04)
    {
        //@04. channel not modified when port missing

        stringstream errormsg;
        Document doc(m_libraryScope);
        LdpParser parser(errormsg, m_libraryScope.ldp_factory());
        parser.parse_text("(score (vers 2.0) (instrument (infoMIDI 56 2)"
            "(musicData (metronome q 55) )))" );
        LdpTree* pTree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pImo = a.analyse_tree_and_get_object(pTree);
        delete pTree->get_root();
        ImoScore* pScore = dynamic_cast<ImoScore*>(pImo);

        CHECK( pScore != nullptr );
        CHECK( pScore->get_num_instruments() == 1 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "" );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 2 );
        CHECK( pMidi->get_midi_port() == -1 );

        MyMidiAssigner assigner;
        assigner.assign_midi_data(pScore);

        CHECK( pInstr->get_num_sounds() == 1 );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 2 );
        CHECK( pMidi->get_midi_port() == 0 );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );

//        delete pRoot;
        delete pScore;
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
        LdpTree* pTree = parser.get_ldp_tree();
        LdpAnalyser a(errormsg, m_libraryScope, &doc);
        ImoObj* pImo = a.analyse_tree_and_get_object(pTree);
        delete pTree->get_root();
        ImoScore* pScore = dynamic_cast<ImoScore*>(pImo);

        CHECK( pScore != nullptr );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo->get_score_instr_id() == "" );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 0 );
        CHECK( pMidi->get_midi_port() == -1 );
        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );

        MyMidiAssigner assigner;
        assigner.assign_midi_data(pScore);

        CHECK( assigner.my_num_sounds() == 2 );

        pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_sounds() == 1 );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 0 );
        CHECK( pMidi->get_midi_port() == 0 );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );

        pInstr = pScore->get_instrument(1);
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "SOUND-2" );
        pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 1 );
        CHECK( pMidi->get_midi_port() == 0 );

        delete pScore;
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
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");
        CHECK( pRoot != nullptr);
        CHECK( pRoot->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pRoot );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != nullptr );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 1 );
        ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == -1 );
        CHECK( pMidi->get_midi_port() == 2 );

        pInstr = pScore->get_instrument(1);
        CHECK( pInstr != nullptr );
        CHECK( pInstr->get_num_sounds() == 0 );

        MyMidiAssigner assigner;
        assigner.assign_midi_data(pScore);

        CHECK( assigner.my_num_sounds() == 2 );

        pInstr = pScore->get_instrument(0);
        CHECK( pInstr->get_num_sounds() == 1 );
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "P1-I1" );
        pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 0 );
        CHECK( pMidi->get_midi_port() == 2 );

        pInstr = pScore->get_instrument(1);
        pInfo = pInstr->get_sound_info(0);
        CHECK( pInfo != nullptr );
        CHECK( pInfo->get_score_instr_id() == "SOUND-1" );
        pMidi = pInfo->get_midi_info();
        CHECK( pMidi->get_midi_channel() == 0 );
        CHECK( pMidi->get_midi_port() == 0 );

        delete pRoot;
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



//=======================================================================================
// MeasuresTableBuilder tests
//=======================================================================================
class MeasuresTableBuilderTestFixture
{
public:
    LibraryScope m_libraryScope;
    ImoScore* m_pScore;
    Document* m_pDoc;
    LdpFactory* m_pLdpFactory;

    MeasuresTableBuilderTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_pScore(nullptr)
        , m_pDoc(nullptr)
    {
        m_pLdpFactory = m_libraryScope.ldp_factory();
    }

    ~MeasuresTableBuilderTestFixture()    //TearDown fixture
    {
        delete m_pScore;
        delete m_pDoc;
    }

    void create_score(const string &ldp)
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        LdpParser parser(cout, m_libraryScope.ldp_factory());
        parser.parse_text(ldp);
        LdpTree* pTree = parser.get_ldp_tree();
        LdpAnalyser a(cout, m_libraryScope, m_pDoc);
        ImoObj* pImo = a.analyse_tree_and_get_object(pTree);
        delete pTree->get_root();
        m_pScore = dynamic_cast<ImoScore*>(pImo);
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }
};

SUITE(MeasuresTableBuilderTest)
{

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_001)
    {
        //@001. empty score doesn't create table

        create_score(
            "(score (vers 2.0)(instrument (musicData "
            ")))" );
        ColStaffObjsBuilder csoBuilder;
        csoBuilder.build(m_pScore);
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();

        CHECK( pTable == nullptr );
    }

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_002)
    {
        //@002. one staffobj creates the table

        create_score(
            "(score (vers 2.0)(instrument (musicData (clef G)"
            ")))"
        );
        ColStaffObjsBuilder csoBuilder;
        ColStaffObjs* pCSO = csoBuilder.build(m_pScore);
//        cout << test_name() << endl;
//        cout << pCSO->dump();
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 1 );

        ImMeasuresTableEntry* pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_start_entry() == pCSO->front() );
        ColStaffObjsEntry* pCSOEntry = pMeasure->get_start_entry();
        CHECK( pCSOEntry && pCSOEntry->imo_object()->is_clef() );
        pCSOEntry = pMeasure->get_end_entry();
        CHECK( pCSOEntry == nullptr );
        CHECK( pMeasure->get_timepos() == 0.0f );
        CHECK( pMeasure->get_implied_beat_duration() == LOMSE_NO_DURATION );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == LOMSE_NO_DURATION );
//        cout << test_name() << endl << pTable->dump();
    }

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_003)
    {
        //@003. first measure created for all instruments

        create_score(
            "(score (vers 2.0) (instrument (musicData (clef G)))"
            "(instrument (staves 2)(musicData (clef G p1)(clef F4 p2)"
            ")))"
        );
        ColStaffObjsBuilder csoBuilder;
        ColStaffObjs* pCSO = csoBuilder.build(m_pScore);
//        cout << test_name() << endl;
//        cout << pCSO->dump();
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 1 );

        ImMeasuresTableEntry* pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_start_entry() == pCSO->front() );
        CHECK( pMeasure->get_timepos() == 0.0f );
        CHECK( pMeasure->get_implied_beat_duration() == LOMSE_NO_DURATION );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == LOMSE_NO_DURATION );
        CHECK( pMeasure->get_barline() == nullptr );

        pInstr = m_pScore->get_instrument(1);
        pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 1 );

        pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 0.0f );
        CHECK( pMeasure->get_implied_beat_duration() == LOMSE_NO_DURATION );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == LOMSE_NO_DURATION );
        CHECK( pMeasure->get_barline() == nullptr );
    }

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_004)
    {
        //@004. beat duration updated when TS found

        create_score(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(key C)(time 2 4) ))"
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
            ")))"
        );
        ColStaffObjsBuilder csoBuilder;
        ColStaffObjs* pCSO = csoBuilder.build(m_pScore);
//        cout << test_name() << endl;
//        cout << pCSO->dump();
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 1 );

        ImMeasuresTableEntry* pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_start_entry() == pCSO->front() );
        CHECK( pMeasure->get_timepos() == 0.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_barline() == nullptr );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        pInstr = m_pScore->get_instrument(1);
        pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 1 );

        pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 0.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_barline() == nullptr );
//        cout << test_name() << endl;
//        cout << pTable->dump();

    }

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_005)
    {
        //@005. barline finish measure

        create_score(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 3 4)(n c4 q)(barline)"
            ")))"
        );
        ColStaffObjsBuilder csoBuilder;
        ColStaffObjs* pCSO = csoBuilder.build(m_pScore);
//        cout << test_name() << endl;
//        cout << pCSO->dump();
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 1 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 0.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_start_entry() == pCSO->front() );
        CHECK( pMeasure->get_barline() && pMeasure->get_barline()->is_barline() );
    }

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_006)
    {
        //@006. object after barline starts a new measure. timepos is updated. TS inherited

        create_score(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 3 4)(n c4 q)(barline)(n e4 q)"
            ")))"
        );
        ColStaffObjsBuilder csoBuilder;
        ColStaffObjs* pCSO = csoBuilder.build(m_pScore);
//        cout << test_name() << endl;
//        cout << pCSO->dump();
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 2 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_start_entry() == pCSO->front() );

        pMeasure = pTable->get_measure(1);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 64.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_barline() == nullptr );
    }

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_007)
    {
        //@007. New TS changes beat duration

        create_score(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(time 6 8)(n e4 q.)(n g4 q)(n c5 e)"
            ")))"
        );
        ColStaffObjsBuilder csoBuilder;
        ColStaffObjs* pCSO = csoBuilder.build(m_pScore);
//        cout << test_name() << endl;
//        cout << pCSO->dump();
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 3 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_start_entry() == pCSO->front() );

        pMeasure = pTable->get_measure(1);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 128.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_barline() && pMeasure->get_barline()->is_barline() );

        pMeasure = pTable->get_measure(2);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 256.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 96.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 32.0f );
        CHECK( pMeasure->get_barline() == nullptr );
    }

    TEST_FIXTURE(MeasuresTableBuilderTestFixture, measures_table_builder_008)
    {
        //@008. clef change at start of measure

        create_score(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(clef F4)(n e3 q)(n g3 q)(barline)"
            "(n e3 q.)(n c3 e)(barline)"
            ")))"
        );
        ColStaffObjsBuilder csoBuilder;
        ColStaffObjs* pCSO = csoBuilder.build(m_pScore);
//        cout << test_name() << endl;
//        cout << pCSO->dump();
        MeasuresTableBuilder builder;

        builder.build(m_pScore);

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 3 );
//        cout << test_name() << endl << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure(0);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_start_entry() == pCSO->front() );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_barline() && pMeasure->get_barline()->is_barline() );

        pMeasure = pTable->get_measure(1);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_start_entry()->imo_object()->is_clef() );
        CHECK( pMeasure->get_timepos() == 128.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_barline() && pMeasure->get_barline()->is_barline() );

        pMeasure = pTable->get_measure(2);
        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 256.0f );
        CHECK( pMeasure->get_implied_beat_duration() == 64.0f );
        CHECK( pMeasure->get_bottom_ts_beat_duration() == 64.0f );
        CHECK( pMeasure->get_barline() && pMeasure->get_barline()->is_barline() );
    }

};

