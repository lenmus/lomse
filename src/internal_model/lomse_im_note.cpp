//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_im_note.h"

#include <math.h>               //pow

namespace lomse
{


//=======================================================================================
// ImoNoteRest implementation
//=======================================================================================
ImoNoteRest::ImoNoteRest(int objtype)
    : ImoStaffObj(objtype)
    , m_nNoteType(k_quarter)
    , m_nDots(0)
    , m_nVoice(1)
{
}

//---------------------------------------------------------------------------------------
int ImoNoteRest::get_beam_type(int level)
{
    ImoBeam* pBeam = static_cast<ImoBeam*>( find_relation(k_imo_beam) );
    if (pBeam)
    {
        ImoBeamData* pData = dynamic_cast<ImoBeamData*>( pBeam->get_data_for(this) );
        return pData->get_beam_type(level);
    }
    else
        return ImoBeam::k_none;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_beam_type(int level, int type)
{
    ImoBeam* pBeam = get_beam();
    ImoBeamData* pData = static_cast<ImoBeamData*>( pBeam->get_data_for(this) );
    pData->set_beam_type(level, type);
}

//---------------------------------------------------------------------------------------
ImoBeam* ImoNoteRest::get_beam()
{
    return static_cast<ImoBeam*>( find_relation(k_imo_beam) );
}

//---------------------------------------------------------------------------------------
bool ImoNoteRest::is_end_of_beam()
{
    ImoBeam* pBeam = static_cast<ImoBeam*>( find_relation(k_imo_beam) );
    if (pBeam)
    {
        ImoBeamData* pData = dynamic_cast<ImoBeamData*>( pBeam->get_data_for(this) );
        return pData->is_end_of_beam();
    }
    else
        return ImoBeam::k_none;
}

//---------------------------------------------------------------------------------------
bool ImoNoteRest::is_in_tuplet()
{
    return find_relation(k_imo_tuplet) != NULL;
}

//---------------------------------------------------------------------------------------
ImoTuplet* ImoNoteRest::get_tuplet()
{
    return static_cast<ImoTuplet*>( find_relation(k_imo_tuplet) );
}

//---------------------------------------------------------------------------------------
float ImoNoteRest::get_duration()
{
    float rTime = to_duration(m_nNoteType, m_nDots);
    ImoTuplet* pTuplet = get_tuplet();
    if (pTuplet)
    {
        rTime *= float(pTuplet->get_normal_number());
        rTime /= float(pTuplet->get_actual_number());
    }
    return rTime;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_note_type_and_dots(int noteType, int dots)
{
    m_nNoteType = noteType;
    m_nDots = dots;
}


//=======================================================================================
// ImoNote implementation
//=======================================================================================
ImoNote::ImoNote()
    : ImoNoteRest(k_imo_note)
    , m_step(k_no_pitch)
    , m_octave(4)
    , m_notated_acc(k_no_accidentals)
    , m_actual_acc(k_acc_not_computed)
    , m_stemDirection(k_stem_default)
    , m_pTieNext(NULL)
    , m_pTiePrev(NULL)
    , m_pChord(NULL)
{
}

//---------------------------------------------------------------------------------------
ImoNote::ImoNote(int step, int octave, int noteType, EAccidentals accidentals, int dots,
                 int staff, int voice, int stem)
    : ImoNoteRest(k_imo_note)
    , m_step(step)
    , m_octave(octave)
    , m_notated_acc(accidentals)
    , m_actual_acc(k_acc_not_computed)
    , m_stemDirection(stem)
    , m_pTieNext(NULL)
    , m_pTiePrev(NULL)
    , m_pChord(NULL)
{
    m_nNoteType = noteType;
    m_nDots = dots;
    m_nVoice = voice;
    m_staff = staff;
}

//---------------------------------------------------------------------------------------
ImoNote::~ImoNote()
{
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_in_chord()
{
    return get_chord() != NULL;
}

//---------------------------------------------------------------------------------------
ImoChord* ImoNote::get_chord()
{
    return static_cast<ImoChord*>( find_relation(k_imo_chord) );
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_start_of_chord()
{
    ImoChord* pChord = get_chord();
    return pChord && dynamic_cast<ImoNote*>(pChord->get_start_object()) == this;
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_end_of_chord()
{
    ImoChord* pChord = get_chord();
    return pChord && pChord->get_end_object() == this;
}

//---------------------------------------------------------------------------------------
FPitch ImoNote::get_fpitch()
{
    //FPitch. Ignores fractional part of actual accidentals

    if (m_actual_acc == k_acc_not_computed)
        return k_undefined_fpitch;
    else
        return FPitch(m_step, m_octave, int(m_actual_acc));
}

//---------------------------------------------------------------------------------------
float ImoNote::get_frequency()
{
    //frequecy, in Herzs

    //TODO
    return 0.0f;
}


//---------------------------------------------------------------------------------------
MidiPitch ImoNote::get_midi_pitch()
{
    if (m_actual_acc == k_acc_not_computed)
        return k_undefined_midi_pitch;
    else
        return MidiPitch(m_step, m_octave, int(m_actual_acc));
}


//---------------------------------------------------------------------------------------
int ImoNote::get_midi_bend()
{
    //Midi bend (two bytes)

    //TODO
    return 0;
}


//---------------------------------------------------------------------------------------
float ImoNote::get_cents()
{
    //deviation from diatonic pitch implied by step and octave, in cents.

    //TODO
    return 0.0f;
}



}  //namespace lomse
