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
    extern int get_beat_position(TimeUnits timePos, ImoTimeSignature* pTS);

    //-----------------------------------------------------------------------------------
    // returns ref.note duration (in LDP notes duration units)
    extern TimeUnits get_duration_for_ref_note(int bottomNumber);


//---------------------------------------------------------------------------------------
// Key signature related functions

    //-----------------------------------------------------------------------------------
    // This function fills the array nAccidentals with the accidentals implied by
    // the key signature. Each element of the array refers to one note: 0=Do, 1=Re,
    // 2=Mi, 3=Fa, ... , 6=Si, and its value can be one of:
    //     0  = no accidental
    //    -1  = a flat
    //     1  = a sharp
    extern void get_accidentals_for_key(int keyType, int nAccidentals[]);

    //-----------------------------------------------------------------------------------
    extern int key_signature_to_num_fifths(int keyType);

    //-----------------------------------------------------------------------------------
    //returns the step (0..6, 0=Do, 1=Re, 3=Mi, ... , 6=Si) for the root note in
    //the Key signature. For example, if keyType is A sharp minor it returns 5 (step A)
    int get_step_for_root_note(EKeySignature keyType);

    //-----------------------------------------------------------------------------------
    extern bool is_major_key(EKeySignature keyType);
    extern bool is_minor_key(EKeySignature keyType);
    extern EKeySignature get_relative_minor_key(EKeySignature majorKey);
    extern EKeySignature get_relative_major_key(EKeySignature minorKey);


//---------------------------------------------------------------------------------------
// Clef related functions

    //-----------------------------------------------------------------------------------
    // Returns the diatonic pitch for first line, when using received clef.
    extern DiatonicPitch get_diatonic_pitch_for_first_line(EClef nClef);

}   //namespace lomse

#endif      //__LOMSE_SCORE_UTILITIES_H__
