//---------------------------------------------------------------------------------------
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

#ifndef __LOMSE_SORE_UTILITIES_H__
#define __LOMSE_SORE_UTILITIES_H__

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"        //for EKeySignature enum


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

    //-----------------------------------------------------------------------------------
    // Determines if a timePos is in on-beat, off-beat position
    enum { k_off_beat = -1, };
    extern int get_beat_position(float timePos, ImoTimeSignature* pTS);

    //-----------------------------------------------------------------------------------
    // returns the number of pulses (metronome pulses) implied by the time signature
    //int lmGetNumPulsesForTimeSignature(lmETimeSignature nTimeSign)

    //-----------------------------------------------------------------------------------
    // returns the numerator of time signature fraction
    //int lmGetNumBeatsFromTimeSignType(lmETimeSignature nTimeSign)

    //-----------------------------------------------------------------------------------
    //int GetBeatTypeFromTimeSignType(lmETimeSignature nTimeSign)

    //-----------------------------------------------------------------------------------
    // returns beat duration (in LDP notes duration units)
    //float GetBeatDuration(lmETimeSignature nTimeSign)
    //float GetBeatDuration(int nBeatType)

    //-----------------------------------------------------------------------------------
    // Returns the required duration for a measure in the received time signature
    //float GetMeasureDuration(lmETimeSignature nTimeSign)


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
    //int lmGetRootNoteStep(EKeySignature keyType)

    //-----------------------------------------------------------------------------------
    bool is_major_key(EKeySignature keyType);
    bool is_minor_key(EKeySignature keyType);
    EKeySignature get_relative_minor_key(EKeySignature majorKey);
    EKeySignature get_relative_major_key(EKeySignature minorKey);


}   //namespace lomse

#endif      //__LOMSE_SORE_UTILITIES_H__
