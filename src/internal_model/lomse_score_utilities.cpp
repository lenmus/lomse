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

#include "lomse_score_utilities.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_logger.h"

#include <cmath>                //for fabs

using namespace std;

namespace lomse
{

//=======================================================================================
// global functions related to TimeSignatures
//=======================================================================================

//---------------------------------------------------------------------------------------
int get_beat_position(TimeUnits timePos, ImoTimeSignature* pTS)
{
    // Some times it is necessary to know the type of beat (strong, medium, weak,
    // off-beat) at which a note or rest is positioned.
    // This function receives the time for a note/rest and the current time signature
    // and returns the type of beat: either an integer positive value 0..n, meaning
    // 'on-beat', where n is the beat number, or -1 meaning 'off-beat'

    int beatType = pTS->get_bottom_number();

    // coumpute beat duration
    int beatDuration;
    switch (beatType)
    {
        case 1: beatDuration = int( to_duration(k_whole, 0) ); break;
        case 2: beatDuration = int( to_duration(k_half, 0) ); break;
        case 4: beatDuration = int( to_duration(k_quarter, 0) ); break;
        case 8: beatDuration = 3 * int( to_duration(k_eighth, 0) ); break;
        case 16: beatDuration = int( to_duration(k_eighth, 0) ); break;
        default:
        {
            stringstream ss;
            ss << "[get_beat_position] BeatType " << beatType << " unknown.";
            LOMSE_LOG_ERROR(ss.str());
            throw runtime_error(ss.str());
        }
    }

    // compute relative position of this note/rest with reference to the beat
    int beatNum = int(timePos) / beatDuration;               //number of beat
    TimeUnits beatShift = fabs(timePos - TimeUnits(beatDuration * beatNum));

    if (beatShift < 1.0)
        //on-beat
        return beatNum;
    else
        // off-beat
        return k_off_beat;

}

//---------------------------------------------------------------------------------------
TimeUnits get_duration_for_ref_note(int bottomNumber)
{
    // returns beat duration (in LDP notes duration units)

    switch(bottomNumber) {
        case 1:
            return pow(2.0, (10 - k_whole));
        case 2:
            return pow(2.0, (10 - k_half));
        case 4:
            return pow(2.0, (10 - k_quarter));
        case 8:
            return pow(2.0, (10 - k_eighth));
        case 16:
            return pow(2.0, (10 - k_16th));
        case 32:
            return pow(2.0, (10 - k_32nd));
        case 64:
            return pow(2.0, (10 - k_64th));
        default:
        {
            stringstream ss;
            ss << "[get_duration_for_ref_note] Invalid bottom number " << bottomNumber;
            LOMSE_LOG_ERROR(ss.str());
            throw runtime_error(ss.str());
        }
    }
}


//=======================================================================================
// global functions related to key signatures
//=======================================================================================

void get_accidentals_for_key(int keyType, int nAccidentals[])
{
    // Given a key signature (keyType) this function fills the array
    // nAccidentals with the accidentals implied by the key signature.
    // Each element of the array refers to one note: 0=Do, 1=Re, 2=Mi, 3=Fa, ... , 6=Si
    // & its value can be one of:
    //     0  = no accidental
    //    -1  = a flat
    //     1  = a sharp

    // initialize array: no accidentals
    for (int i=0; i < 7; i++)
        nAccidentals[i] = 0;

    // accidentals implied by the key signature
    switch (keyType)
    {
        case k_key_C:
        case k_key_a:
            //no accidentals
            break;

        //Sharps ---------------------------------------
        case k_key_G:
        case k_key_e:
            nAccidentals[3] = 1;     //Fa #
            break;
        case k_key_D:
        case k_key_b:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            break;
        case k_key_A:
        case k_key_fs:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
             break;
       case k_key_E:
        case k_key_cs:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            break;
        case k_key_B:
        case k_key_gs:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            nAccidentals[5] = 1;     //La #
            break;
        case k_key_Fs:
        case k_key_ds:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            nAccidentals[5] = 1;     //La #
            nAccidentals[2] = 1;     //Mi #
            break;
        case k_key_Cs:
        case k_key_as:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            nAccidentals[5] = 1;     //La #
            nAccidentals[2] = 1;     //Mi #
            nAccidentals[6] = 1;     //Si #
             break;

        //Flats -------------------------------------------
        case k_key_F:
        case k_key_d:
            nAccidentals[6] = -1;         //Si b
            break;
        case k_key_Bf:
        case k_key_g:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
             break;
       case k_key_Ef:
        case k_key_c:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            break;
        case k_key_Af:
        case k_key_f:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
             break;
        case k_key_Df:
        case k_key_bf:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
            nAccidentals[4] = -1;         //Sol b
            break;
        case k_key_Gf:
        case k_key_ef:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
            nAccidentals[4] = -1;         //Sol b
            nAccidentals[0] = -1;         //Do b
            break;
        case k_key_Cf:
        case k_key_af:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
            nAccidentals[4] = -1;         //Sol b
            nAccidentals[0] = -1;         //Do b
            nAccidentals[3] = -1;         //Fa b
            break;
        default:
        {
            stringstream ss;
            ss << "[get_accidentals_for_key] Invalid key signature " << keyType;
            LOMSE_LOG_ERROR(ss.str());
            throw runtime_error(ss.str());
        }
    }

}

//---------------------------------------------------------------------------------------
int get_step_for_root_note(EKeySignature keyType)
{
    //returns the step (0..6, 0=Do, 1=Re, 3=Mi, ... , 6=Si) for the root note in
    //the Key signature. For example, if keyType is La sharp minor it returns
    //step = 5 (La)

    //compute root note
    switch(keyType)
    {
        case k_key_C:
        case k_key_c:
        case k_key_cs:
        case k_key_Cs:
        case k_key_Cf:
            return k_step_C;

        case k_key_D:
        case k_key_Df:
        case k_key_ds:
        case k_key_d:
            return k_step_D;

        case k_key_E:
        case k_key_e:
        case k_key_Ef:
        case k_key_ef:
            return k_step_E;

        case k_key_F:
        case k_key_fs:
        case k_key_Fs:
        case k_key_f:
            return k_step_F;

        case k_key_G:
        case k_key_gs:
        case k_key_g:
        case k_key_Gf:
            return k_step_G;

        case k_key_A:
        case k_key_a:
        case k_key_as:
        case k_key_Af:
        case k_key_af:
            return k_step_A;

        case k_key_b:
        case k_key_B:
        case k_key_Bf:
        case k_key_bf:
            return k_step_B;

        default:
        {
            stringstream ss;
            ss << "[get_step_for_root_note] Invalid key signature " << keyType;
            LOMSE_LOG_ERROR(ss.str());
            throw runtime_error(ss.str());
        }
    }
}

//---------------------------------------------------------------------------------------
bool is_major_key(EKeySignature keyType)
{
    return (keyType < k_key_a);
}

//---------------------------------------------------------------------------------------
bool is_minor_key(EKeySignature keyType)
{
    return (keyType >= k_key_a);
}

//---------------------------------------------------------------------------------------
int key_signature_to_num_fifths(int keyType)
{
    // Retunrs the number of fifths that corresponds to the encoded key signature

    int nFifths = 0;        //num accidentals to return (0..7)
    switch(keyType)
    {
        case k_key_C:
        case k_key_a:
            nFifths = 0;
            break;

        //Sharps ---------------------------------------
        case k_key_G:
        case k_key_e:
            nFifths = 1;
            break;
        case k_key_D:
        case k_key_b:
            nFifths = 2;
            break;
        case k_key_A:
        case k_key_fs:
            nFifths = 3;
            break;
        case k_key_E:
        case k_key_cs:
            nFifths = 4;
            break;
        case k_key_B:
        case k_key_gs:
            nFifths = 5;
            break;
        case k_key_Fs:
        case k_key_ds:
            nFifths = 6;
            break;
        case k_key_Cs:
        case k_key_as:
            nFifths = 7;
            break;

        //Flats -------------------------------------------
        case k_key_F:
        case k_key_d:
            nFifths = -1;
            break;
        case k_key_Bf:
        case k_key_g:
            nFifths = -2;
            break;
        case k_key_Ef:
        case k_key_c:
            nFifths = -3;
            break;
        case k_key_Af:
        case k_key_f:
            nFifths = -4;
            break;
        case k_key_Df:
        case k_key_bf:
            nFifths = -5;
            break;
        case k_key_Gf:
        case k_key_ef:
            nFifths = -6;
            break;
        case k_key_Cf:
        case k_key_af:
            nFifths = -7;
            break;
        default:
        {
            stringstream ss;
            ss << "[key_signature_to_num_fifths] Invalid key signature " << keyType;
            LOMSE_LOG_ERROR(ss.str());
            throw runtime_error(ss.str());
        }
    }
    return nFifths;
}

//---------------------------------------------------------------------------------------
EKeySignature get_relative_minor_key(EKeySignature nMajorKey)
{
    switch(nMajorKey) {
        case k_key_C:
            return k_key_a;
        case k_key_G:
            return k_key_e;
        case k_key_D:
            return k_key_b;
        case k_key_A:
            return k_key_fs;
        case k_key_E:
            return k_key_cs;
        case k_key_B:
            return k_key_gs;
        case k_key_Fs:
            return k_key_ds;
        case k_key_Cs:
            return k_key_as;
        case k_key_F:
            return k_key_d;
        case k_key_Bf:
            return k_key_g;
        case k_key_Ef:
            return k_key_c;
        case k_key_Af:
            return k_key_f;
        case k_key_Df:
            return k_key_bf;
        case k_key_Gf:
            return k_key_ef;
        case k_key_Cf:
            return k_key_af;
        default:
        {
            stringstream ss;
            ss << "[get_relative_minor_key] Invalid key signature " << nMajorKey;
            LOMSE_LOG_ERROR(ss.str());
            throw runtime_error(ss.str());
        }
    }

}

//---------------------------------------------------------------------------------------
EKeySignature get_relative_major_key(EKeySignature nMinorKey)
{
    switch(nMinorKey) {
        case k_key_a:
            return k_key_C;
        case k_key_e:
            return k_key_G;
        case k_key_b:
            return k_key_D;
        case k_key_fs:
            return k_key_A;
        case k_key_cs:
            return k_key_E;
        case k_key_gs:
            return k_key_B;
        case k_key_ds:
            return k_key_Fs;
        case k_key_as:
            return k_key_Cs;
        case k_key_d:
            return k_key_F;
        case k_key_g:
            return k_key_Bf;
        case k_key_c:
            return k_key_Ef;
        case k_key_f:
            return k_key_Af;
        case k_key_bf:
            return k_key_Df;
        case k_key_ef:
            return k_key_Gf;
        case k_key_af:
            return k_key_Cf;
        default:
        {
            stringstream ss;
            ss << "[get_relative_major_key] Invalid key signature " << nMinorKey;
            LOMSE_LOG_ERROR(ss.str());
            //throw runtime_error(ss.str());
            return k_key_c;
        }
    }

}


//=======================================================================================
// global functions related to Clefs
//=======================================================================================

//---------------------------------------------------------------------------------------
DiatonicPitch get_diatonic_pitch_for_first_line(EClef nClef)
{
    // Returns the diatonic pitch for first line, when using received clef.

    switch(nClef)
    {
        case k_clef_G2:           return DiatonicPitch(k_step_E, k_octave_4);
        case k_clef_F4:           return DiatonicPitch(k_step_G, k_octave_2);
        case k_clef_F3:           return DiatonicPitch(k_step_B, k_octave_2);
        case k_clef_C1:           return DiatonicPitch(k_step_C, k_octave_4);
        case k_clef_C2:           return DiatonicPitch(k_step_A, k_octave_3);
        case k_clef_C3:           return DiatonicPitch(k_step_F, k_octave_3);
        case k_clef_C4:           return DiatonicPitch(k_step_D, k_octave_3);
        case k_clef_C5:           return DiatonicPitch(k_step_B, k_octave_2);
        case k_clef_F5:           return DiatonicPitch(k_step_E, k_octave_2);
        case k_clef_G1:           return DiatonicPitch(k_step_G, k_octave_4);
        case k_clef_8_G2:         return DiatonicPitch(k_step_E, k_octave_5);  //8 above
        case k_clef_G2_8:         return DiatonicPitch(k_step_E, k_octave_3);  //8 below
        case k_clef_8_F4:         return DiatonicPitch(k_step_G, k_octave_3);  //8 above
        case k_clef_F4_8:         return DiatonicPitch(k_step_G, k_octave_1);  //8 below
        case k_clef_15_G2:        return DiatonicPitch(k_step_E, k_octave_6);  //15 above
        case k_clef_G2_15:        return DiatonicPitch(k_step_E, k_octave_2);  //15 below
        case k_clef_15_F4:        return DiatonicPitch(k_step_G, k_octave_4);  //15 above
        case k_clef_F4_15:        return DiatonicPitch(k_step_G, k_octave_0);  //15 below
        case k_clef_undefined:
        case k_clef_percussion:   return NO_DPITCH;
        default:
        {
            stringstream ss;
            ss << "[get_diatonic_pitch_for_first_line] Invalid clef " << nClef;
            LOMSE_LOG_ERROR(ss.str());
            throw runtime_error(ss.str());
        }
    }
    return NO_DPITCH;
}


}  //namespace lomse
