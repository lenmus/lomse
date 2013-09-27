//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
        //structurizes the score (creates the associated ColStaffObjs)

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

        delete pIModel;
    }

}


//---------------------------------------------------------------------------------------
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

}



