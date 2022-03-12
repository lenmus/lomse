//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
int get_beat_position(TimeUnits timePos, ImoTimeSignature* pTS, TimeUnits timeShift)
{
    // Some times it is necessary to know the type of beat (strong, medium, weak,
    // off-beat) at which a note or rest is positioned.
    // This function receives the time for a note/rest and the current time signature
    // and returns the type of beat: either an integer positive value 0..n, meaning
    // 'on-beat', where n is the beat number, or -1 meaning 'off-beat'
    //
    // Parameter timeShift can be useful for taking into account anacrusis start

    int beatType = pTS->get_bottom_number();
    TimeUnits beatDuration = pTS->get_ref_note_duration();

    if (pTS->is_compound_meter()|| (beatType == 8 && pTS->get_top_number() == 3))
        beatDuration *= 3.0;

    // compute relative position of this note/rest with reference to the beat
    TimeUnits time = timePos + timeShift;
    int beatNum = int( (time / beatDuration) + 0.1);   //number of beat
    TimeUnits beatShift = fabs(time - beatDuration * TimeUnits(beatNum));

    if (beatShift < 1.0)
        //on-beat
        return beatNum % pTS->get_num_pulses();
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
            return pow(2.0, (10 - k_quarter));
        }
    }
}


//=======================================================================================
// KeyUtilities implementation
//=======================================================================================

void KeyUtilities::get_accidentals_for_key(int keyType, int nAccidentals[])
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
        }
    }

}

//---------------------------------------------------------------------------------------
int KeyUtilities::get_step_for_root_note(EKeySignature keyType)
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
            return k_step_C;
        }
    }
}

//---------------------------------------------------------------------------------------
int KeyUtilities::get_step_for_leading_note(EKeySignature keyType)
{
    int nRoot = get_step_for_root_note(keyType);
    return (nRoot == 0 ? 6 : nRoot-1);      //scale degree seven
}

//---------------------------------------------------------------------------------------
bool KeyUtilities::is_major_key(EKeySignature keyType)
{
    return (keyType < k_key_a);
}

//---------------------------------------------------------------------------------------
bool KeyUtilities::is_minor_key(EKeySignature keyType)
{
    return (keyType >= k_key_a);
}

//---------------------------------------------------------------------------------------
int KeyUtilities::key_signature_to_num_fifths(int keyType)
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
            return 0;
        }
    }
    return nFifths;
}

//---------------------------------------------------------------------------------------
EKeySignature KeyUtilities::get_relative_minor_key(EKeySignature nMajorKey)
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
            return k_key_C;
        }
    }

}

//---------------------------------------------------------------------------------------
EKeySignature KeyUtilities::get_relative_major_key(EKeySignature nMinorKey)
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
            return k_key_c;
        }
    }

}

//---------------------------------------------------------------------------------------
EKeySignature KeyUtilities::key_components_to_key_type(int fifths, EKeyMode mode)
{
    if (mode == k_key_mode_major)
    {
        switch(fifths)
        {
            case -7:        return k_key_Cf;
            case -6:        return k_key_Gf;
            case -5:        return k_key_Df;
            case -4:        return k_key_Af;
            case -3:        return k_key_Ef;
            case -2:        return k_key_Bf;
            case -1:        return k_key_F;
            case 0:         return k_key_C;
            case 1:         return k_key_G;
            case 2:         return k_key_D;
            case 3:         return k_key_A;
            case 4:         return k_key_E;
            case 5:         return k_key_B;
            case 6:         return k_key_Fs;
            case 7:         return k_key_Cs;
            default:
            {
                //too many fifths. Not possible to convert to enum
                return k_key_C;
            }
        }
    }
    else if (mode == k_key_mode_minor)
    {
        switch(fifths)
        {
            case -7:        return k_key_af;
            case -6:        return k_key_ef;
            case -5:        return k_key_bf;
            case -4:        return k_key_f;
            case -3:        return k_key_c;
            case -2:        return k_key_g;
            case -1:        return k_key_d;
            case 0:         return k_key_a;
            case 1:         return k_key_e;
            case 2:         return k_key_b;
            case 3:         return k_key_fs;
            case 4:         return k_key_cs;
            case 5:         return k_key_gs;
            case 6:         return k_key_ds;
            case 7:         return k_key_as;
            default:
            {
                //too many fifths. Not possible to convert to enum
                return k_key_a;
            }
        }
    }
    else
    {
        stringstream ss;
        ss << "[get_key_type] Invalid key mode " << mode;
        LOMSE_LOG_ERROR(ss.str());
        return k_key_C;
    }
}

//---------------------------------------------------------------------------------------
EKeyMode KeyUtilities::get_key_mode(EKeySignature type)
{
    return (is_major_key(type) ? k_key_mode_major : k_key_mode_minor);
}

//---------------------------------------------------------------------------------------
EKeySignature KeyUtilities::transpose(EKeySignature oldKey, FIntval interval, bool fUp)
{
    FPitch fp = KeyUtilities::get_root_pitch(oldKey);
    bool fMajor = KeyUtilities::is_major_key(oldKey);

    if (fUp)
        fp += interval;
    else
        fp -= interval;


    return KeyUtilities::key_type_for_root_pitch(fp, fMajor);
}

//---------------------------------------------------------------------------------------
FPitch KeyUtilities::get_root_pitch(EKeySignature key)
{
    switch(key)
    {
        case k_key_C:
        case k_key_c:
            return FPitch(k_step_C, 4, 0);
        case k_key_cs:
        case k_key_Cs:
            return FPitch(k_step_C, 4, 1);
        case k_key_Cf:
            return FPitch(k_step_C, 4, -1);

        case k_key_D:
        case k_key_d:
            return FPitch(k_step_D, 4, 0);
        case k_key_Df:
            return FPitch(k_step_D, 4, -1);
        case k_key_ds:
            return FPitch(k_step_D, 4, +1);

        case k_key_E:
        case k_key_e:
            return FPitch(k_step_E, 4, 0);
        case k_key_Ef:
        case k_key_ef:
            return FPitch(k_step_E, 4, -1);

        case k_key_F:
        case k_key_f:
            return FPitch(k_step_F, 4, 0);
        case k_key_fs:
        case k_key_Fs:
            return FPitch(k_step_F, 4, +1);

        case k_key_G:
        case k_key_g:
            return FPitch(k_step_G, 4, 0);
        case k_key_gs:
            return FPitch(k_step_G, 4, +1);
        case k_key_Gf:
            return FPitch(k_step_G, 4, -1);

        case k_key_A:
        case k_key_a:
            return FPitch(k_step_A, 4, 0);
        case k_key_as:
            return FPitch(k_step_A, 4, +1);
        case k_key_Af:
        case k_key_af:
            return FPitch(k_step_A, 4, -1);

        case k_key_B:
        case k_key_b:
            return FPitch(k_step_B, 4, 0);
        case k_key_Bf:
        case k_key_bf:
            return FPitch(k_step_B, 4, -1);

        default:
        {
            stringstream ss;
            ss << "[get_root_pitch] Invalid key signature " << key;
            LOMSE_LOG_ERROR(ss.str());
            return FPitch(k_step_C, 4, 0);
        }
    }
}

//---------------------------------------------------------------------------------------
EKeySignature KeyUtilities::key_type_for_root_pitch(FPitch fp, bool fMajor)
{
    int step = fp.step();
    int acc = fp.num_accidentals();
    switch(step)
    {
        case k_step_C:
        {
            if (acc == 0)   return (fMajor ? k_key_C : k_key_c);
            if (acc == 1)   return (fMajor ? k_key_Cs : k_key_cs);
            if (acc == -1)  return k_key_Cf;
            break;
        }
        case k_step_D:
        {
            if (acc == 0)   return (fMajor ? k_key_D : k_key_d);
            if (acc == 1)   return k_key_ds;
            if (acc == -1)  return k_key_Df;
            break;
        }
        case k_step_E:
        {
            if (acc == 0)   return (fMajor ? k_key_E : k_key_e);
            if (acc == -1)  return (fMajor ? k_key_Ef : k_key_ef);
            break;
        }
        case k_step_F:
        {
            if (acc == 0)   return (fMajor ? k_key_F : k_key_f);
            if (acc == 1)   return (fMajor ? k_key_Fs : k_key_fs);
            break;
        }
        case k_step_G:
        {
            if (acc == 0)   return (fMajor ? k_key_G : k_key_g);
            if (acc == 1)   return k_key_gs;
            if (acc == -1)  return k_key_Gf;
            break;
        }
        case k_step_A:
        {
            if (acc == 0)   return (fMajor ? k_key_A : k_key_a);
            if (acc == 1)   return k_key_as;
            if (acc == -1)  return (fMajor ? k_key_Af : k_key_af);
            break;
        }
        case k_step_B:
        {
            if (acc == 0)   return (fMajor ? k_key_B : k_key_b);
            if (acc == -1)  return (fMajor ? k_key_Bf : k_key_bf);
            break;
        }

        default:
        {
            stringstream ss;
            ss << "[key_type_for_root_pitch] Invalid step " << step
               << ", root=" << int(fp);
            LOMSE_LOG_ERROR(ss.str());
            return k_key_C;
        }
    }

    stringstream ss;
    ss << "[key_type_for_root_pitch] Invalid accidentals " << acc
       << ", root=" << int(fp);
    LOMSE_LOG_ERROR(ss.str());
    return k_key_C;
}

//---------------------------------------------------------------------------------------
FIntval KeyUtilities::up_interval(EKeySignature oldKey, EKeySignature newKey)
{
    FPitch oldPitch = KeyUtilities::get_root_pitch(oldKey);
    FPitch newPitch = KeyUtilities::get_root_pitch(newKey);
    FIntval fp(newPitch - oldPitch);
    if (fp.is_ascending())
        return fp;
    else
        return k_interval_p8 + fp;
}

//---------------------------------------------------------------------------------------
FIntval KeyUtilities::down_interval(EKeySignature oldKey, EKeySignature newKey)
{
    FPitch oldPitch = KeyUtilities::get_root_pitch(oldKey);
    FPitch newPitch = KeyUtilities::get_root_pitch(newKey);
    FIntval fp(newPitch - oldPitch);
    if (fp.is_ascending())
        return fp - k_interval_p8;
    else
        return fp;
}

//---------------------------------------------------------------------------------------
FIntval KeyUtilities::closest_interval(EKeySignature oldKey, EKeySignature newKey)
{
    FIntval up = up_interval(oldKey, newKey);
    FIntval down = down_interval(oldKey, newKey);
    if (abs(up) > abs(down))
        return down;
    else
        return up;
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
        case k_clef_percussion:
        case k_clef_TAB:
        case k_clef_none:
            return NO_DPITCH;

        default:
        {
            stringstream ss;
            ss << "[get_diatonic_pitch_for_first_line] Invalid clef " << nClef;
            LOMSE_LOG_ERROR(ss.str());
            return NO_DPITCH;
        }
    }
    return NO_DPITCH;
}


}  //namespace lomse
