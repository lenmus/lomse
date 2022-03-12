//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SCORE_UTILITIES_H__
#define __LOMSE_SCORE_UTILITIES_H__

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"        //for EKeySignature enum
#include "lomse_pitch.h"
#include "lomse_im_note.h"        //for EKeySignature enum


//---------------------------------------------------------------------------------------
// This module contains a collection of global methods for auxiliary computations
// related to scores
//---------------------------------------------------------------------------------------

namespace lomse
{

//forward declarations
class ImoTimeSignature;


//---------------------------------------------------------------------------------------
// Time signature related functions
//
//  measure -> duration of a whole measure
//
//  ref_note -> the note implied by bottom number
//
//  num_beats (num_pulses) -> number of beats (metronome pulses) per measure. This is the top
//                            number for simple meters, or top/3 for compound meters.
//
//  beat_duration -> duration of a beat. This is the ref_note duration for simple meters, or
//                   3 times the ref_note duration for compound meters.

    //-----------------------------------------------------------------------------------
    // Determines if a timePos is in on-beat, off-beat position
    enum { k_off_beat = -1, };
    extern int get_beat_position(TimeUnits timePos, ImoTimeSignature* pTS,
                                 TimeUnits timeShift=0.0);

    //-----------------------------------------------------------------------------------
    // returns ref.note duration (in LDP notes duration units)
    extern TimeUnits get_duration_for_ref_note(int bottomNumber);


//---------------------------------------------------------------------------------------
// Key signature related functions
class KeyUtilities
{
public:
    /** This function fills the array nAccidentals with the accidentals implied by
        the key signature. Each element of the array refers to one note: 0=Do, 1=Re,
        2=Mi, 3=Fa, ... , 6=Si, and its value can be one of:
         0  = no accidental
        -1  = a flat
         1  = a sharp
    */
    static void get_accidentals_for_key(int keyType, int nAccidentals[]);

    //-----------------------------------------------------------------------------------
    static int key_signature_to_num_fifths(int keyType);

    //-----------------------------------------------------------------------------------
    //returns the step (0..6, 0=Do, 1=Re, 3=Mi, ... , 6=Si) for the root note in
    //the Key signature. For example, if keyType is A sharp minor it returns 5 (step A)
    static int get_step_for_root_note(EKeySignature keyType);

    //-----------------------------------------------------------------------------------
    //returns the step (0..6, 0=Do, 1=Re, 3=Mi, ... , 6=Si) for the leading note in
    //the Key signature. For example, if keyType is A sharp minor it returns 4 (step G)
    static int get_step_for_leading_note(EKeySignature keyType);

    //-----------------------------------------------------------------------------------
    static bool is_major_key(EKeySignature keyType);
    static bool is_minor_key(EKeySignature keyType);
    static EKeySignature get_relative_minor_key(EKeySignature majorKey);
    static EKeySignature get_relative_major_key(EKeySignature minorKey);

    //-----------------------------------------------------------------------------------
    static EKeySignature key_components_to_key_type(int fifths, EKeyMode mode);
    static EKeyMode get_key_mode(EKeySignature type);

    //-----------------------------------------------------------------------------------
    static EKeySignature transpose(EKeySignature oldKey, FIntval interval, bool fUp);

    /** Return the pitch for the root note in the tonality implied by the provided
        key signature. The octave is arbitrarily set to octave 4.
    */
    static FPitch get_root_pitch(EKeySignature key);

    //-----------------------------------------------------------------------------------
    static EKeySignature key_type_for_root_pitch(FPitch fp, bool fMajor);

    /** Returns the interval for transposing up from <i>oldKey</i> to <i>newKey</i>.
        Be aware that interval is always ascending.
    */
    static FIntval up_interval(EKeySignature oldKey, EKeySignature newKey);

    /** Returns the interval for transposing down from <i>oldKey</i> to <i>newKey</i>.
        Be aware that interval is always descending (negative).
    */
    static FIntval down_interval(EKeySignature oldKey, EKeySignature newKey);

    /** Returns the shortest interval for transposing from <i>oldKey</i> to <i>newKey</i>.
        Be aware that interval can be ascending (positive) or descending (negative).
    */
    static FIntval closest_interval(EKeySignature oldKey, EKeySignature newKey);

};


//---------------------------------------------------------------------------------------
// Clef related functions

    //-----------------------------------------------------------------------------------
    // Returns the diatonic pitch for first line, when using received clef.
    extern DiatonicPitch get_diatonic_pitch_for_first_line(EClef nClef);

}   //namespace lomse

#endif      //__LOMSE_SCORE_UTILITIES_H__
