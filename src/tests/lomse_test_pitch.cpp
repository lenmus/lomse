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
#include "lomse_ldp_factory.h"
#include "private/lomse_document_p.h"
#include "lomse_staffobjs_table.h"
#include "lomse_internal_model.h"
#include "lomse_pitch.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class PitchTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    PitchTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~PitchTestFixture()    //TearDown fixture
    {
    }
};

SUITE(PitchTest)
{

    // DiatonicPitch --------------------------------------------------------------------

    TEST_FIXTURE(PitchTestFixture, DPitch_C4)
    {
        DiatonicPitch dp(0, 4);
        CHECK( int(dp) == C4_DPITCH );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_D4)
    {
        DiatonicPitch dp(1, 4);
        CHECK( int(dp) == C4_DPITCH + 1 );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_C5)
    {
        DiatonicPitch dp(0, 5);
        CHECK( int(dp) == C4_DPITCH + 7 );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_No)
    {
        DiatonicPitch dp(k_no_pitch, 5);
        CHECK( int(dp) == NO_DPITCH );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_Plus)
    {
        DiatonicPitch dp(0, 4);
        dp = dp + 5;
        CHECK( int(dp) == C4_DPITCH + 5 );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_Minus)
    {
        DiatonicPitch dp(0, 4);
        dp = dp - 5;
        CHECK( int(dp) == C4_DPITCH - 5 );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_get_ldp_name)
    {
        DiatonicPitch dp(0, 4);
        CHECK( dp.get_ldp_name() == "c4" );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_to_FPitch)
    {
        DiatonicPitch dp(k_step_F, 4);
        CHECK( dp.to_FPitch(k_key_C) == FPitch(k_step_F, 4, 0) );
        CHECK( dp.to_FPitch(k_key_D) == FPitch(k_step_F, 4, 1) );
    }

    TEST_FIXTURE(PitchTestFixture, DPitch_to_midi_pitch)
    {
        DiatonicPitch dp(k_step_F, 4);
        CHECK( dp.to_midi_pitch() == MidiPitch(k_step_F, 4, 0) );
        CHECK( C4_DPITCH.to_midi_pitch() == C4_MPITCH );
    }


    //@ MidiPitch -----------------------------------------------------------------------

    TEST_FIXTURE(PitchTestFixture, MidiPitch_C4)
    {
        MidiPitch mp(0, 4);
        CHECK( int(mp) == C4_MPITCH );
    }

    TEST_FIXTURE(PitchTestFixture, MidiPitch_A0)
    {
        MidiPitch mp(k_step_A, 0);
        CHECK( int(mp) == 21 );
    }

    TEST_FIXTURE(PitchTestFixture, MidiPitch_get_ldp_name)
    {
        MidiPitch mp(k_step_A, k_octave_5);
        CHECK( mp.get_ldp_name() == "a5" );
        CHECK( C4_MPITCH.get_ldp_name() == "c4" );
    }

    TEST_FIXTURE(PitchTestFixture, MidiPitch_04)
    {
        //@04. Extract components
        MidiPitch mp(k_step_F, k_octave_4, 1);  //+f4
        EKeySignature key = k_key_C;
        CHECK( mp.step(key) == k_step_F );
        CHECK( mp.octave(key) == 4 );
        CHECK( mp.accidentals(key) == 1 );

//        int pitch = int(mp);
//        cout << "Midi pitch = " << pitch
//             << ", step=" << mp.step(key)
//             << ", octave=" << mp.octave(key)
//             << ", acc=" << mp.accidentals(key) << endl;
    }

    TEST_FIXTURE(PitchTestFixture, MidiPitch_05)
    {
        //@05. Extract components. B+ in key D
        MidiPitch mp(60);  //c4 = +b3   key D ==> +b3
        EKeySignature key = k_key_D;
        CHECK( mp.step(key) == k_step_B );
        CHECK( mp.octave(key) == 3 );
        CHECK( mp.accidentals(key) == 1 );
    }

    TEST_FIXTURE(PitchTestFixture, MidiPitch_06)
    {
        //@06. Extract components. B+ in key C
        MidiPitch mp(60);  //c4 = +b3   key C ==> c4
        EKeySignature key = k_key_C;
        CHECK( mp.step(key) == k_step_C );
        CHECK( mp.octave(key) == 4 );
        CHECK( mp.accidentals(key) == 0 );
    }

    TEST_FIXTURE(PitchTestFixture, MidiPitch_07)
    {
        //@07. Extract components. C- in key C
        MidiPitch mp(59);  //-c4 = b3   key C ==> b3
        EKeySignature key = k_key_C;
        CHECK( mp.step(key) == k_step_B );
        CHECK( mp.octave(key) == 3 );
        CHECK( mp.accidentals(key) == 0 );
    }

    TEST_FIXTURE(PitchTestFixture, MidiPitch_08)
    {
        //@08. Extract components. C- in key C-
        MidiPitch mp(59);  //c4 = +b3   key C- ==> -c4
        EKeySignature key = k_key_Cf;
        CHECK( mp.step(key) == k_step_C );
        CHECK( mp.octave(key) == 4 );
        CHECK( mp.accidentals(key) == -1 );
    }


    // FPitch ------------------------------------------------------------------------

    TEST_FIXTURE(PitchTestFixture, FPitch_ConstructorStepOctaveAcc)
    {
        FPitch fp(k_step_C, k_octave_4, 0);
        CHECK( int(fp) == C4_FPITCH );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_ExtractComponents)
    {
        FPitch fp(k_step_C, k_octave_4, -2);
        CHECK( fp.step() == 0 );
        CHECK( fp.octave() == 4 );
        CHECK( fp.num_accidentals() == -2 );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_to_rel_ldp_name)
    {
        FPitch fp(k_step_F, k_octave_4, 1);

        CHECK( fp.to_rel_ldp_name(k_key_C) == "+f4" );
        CHECK( fp.to_rel_ldp_name(k_key_D) == "f4" );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_ConstructorDiatonicPitch)
    {
        FPitch fp(C4_DPITCH, 1);
        CHECK( fp.to_rel_ldp_name(k_key_C) == "+c4" );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_is_valid)
    {
        CHECK( C4_FPITCH.is_valid() == true );
        CHECK( FPitch(6).is_valid() == false );
        CHECK( FPitch(12).is_valid() == false );
        CHECK( FPitch(23).is_valid() == false );
        CHECK( FPitch(69).is_valid() == false );
        CHECK( FPitch(115).is_valid() == false );
        CHECK( FPitch(47).is_valid() == true );
        CHECK( FPitch(116).is_valid() == true );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_to_abs_ldp_name)
    {
        FPitch fp(k_step_F, k_octave_4, 1);

        CHECK( fp.to_abs_ldp_name() == "+f4" );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_to_midi_pitch)
    {
        FPitch fp(k_step_F, k_octave_4, 1);

        CHECK( fp.to_midi_pitch() == MidiPitch(k_step_F, 4, 1) );
        CHECK( C4_FPITCH.to_midi_pitch() == C4_MPITCH );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_to_diatonic_pitch)
    {
        FPitch fp(k_step_F, k_octave_4, 1);

        CHECK( fp.to_diatonic_pitch() == DiatonicPitch(k_step_F, 4) );
        CHECK( C4_FPITCH.to_diatonic_pitch() == C4_DPITCH );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_add_semitone)
    {
        FPitch fp1(k_step_D, k_octave_4, 1);
        FPitch fp2(k_step_E, k_octave_4, -1);
        FPitch fp3(k_step_D, k_octave_4, 0);
        FPitch fp4(k_step_F, k_octave_4, 0);
        FPitch fp5(k_step_E, k_octave_4, 0);
        fp1.add_semitone(true /*use sharps*/);
        fp2.add_semitone(true /*use sharps*/);
        fp3.add_semitone(true /*use sharps*/);
        fp4.add_semitone(false /*use flats*/);
        fp5.add_semitone(false /*use flats*/);

//        cout << "fp2 = " << fp2.to_abs_ldp_name() << endl;
        CHECK( fp1 == FPitch(k_step_E, k_octave_4, 0) );
        CHECK( fp2 == FPitch(k_step_E, k_octave_4, 0) );
        CHECK( fp3 == FPitch(k_step_D, k_octave_4, 1) );
        CHECK( fp4 == FPitch(k_step_G, k_octave_4, -1) );
        CHECK( fp5 == FPitch(k_step_F, k_octave_4, 0) );
    }

    TEST_FIXTURE(PitchTestFixture, FPitch_from_ldp_string)
    {
        FPitch fp1("+c4");
        CHECK( fp1.step() == k_step_C );
        CHECK( fp1.octave() == 4 );
        CHECK( fp1.num_accidentals() == 1 );

        FPitch fp2("f5");
        CHECK( fp2.step() == k_step_F );
        CHECK( fp2.octave() == 5 );
        CHECK( fp2.num_accidentals() == 0 );

        FPitch fp3("--d3");
        CHECK( fp3.step() == k_step_D );
        CHECK( fp3.octave() == 3 );
        CHECK( fp3.num_accidentals() == -2 );

        FPitch fp4("z7");   //error -> returns c4
        CHECK( fp4.step() == k_step_C );
        CHECK( fp4.octave() == 4 );
        CHECK( fp4.num_accidentals() == 0 );
    }

}


