//----------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_factory.h"
#include "lomse_document.h"
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
        m_scores_path = LOMSE_TEST_SCORES_PATH;
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


    // MidiPitch ------------------------------------------------------------------------

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
        CHECK( fp.accidentals() == -2 );
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

}


