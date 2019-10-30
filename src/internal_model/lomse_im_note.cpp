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

#include "lomse_im_note.h"

#include "lomse_im_attributes.h"


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
    , m_timeModifierTop(1)
    , m_timeModifierBottom(1)
    , m_duration(k_duration_quarter)
{
}

//---------------------------------------------------------------------------------------
int ImoNoteRest::get_beam_type(int level)
{
    ImoBeam* pBeam = static_cast<ImoBeam*>( find_relation(k_imo_beam) );
    if (pBeam)
    {
        ImoBeamData* pData = static_cast<ImoBeamData*>( pBeam->get_data_for(this) );
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
        ImoBeamData* pData = static_cast<ImoBeamData*>( pBeam->get_data_for(this) );
        return pData->is_end_of_beam();
    }
    else
        return ImoBeam::k_none;
}

//---------------------------------------------------------------------------------------
bool ImoNoteRest::is_in_tuplet()
{
    return find_relation(k_imo_tuplet) != nullptr;
}

//---------------------------------------------------------------------------------------
ImoTuplet* ImoNoteRest::get_first_tuplet()
{
    return static_cast<ImoTuplet*>( find_relation(k_imo_tuplet) );
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_time_modification(int numerator, int denominator)
{
    m_timeModifierTop = numerator;
    m_timeModifierBottom = denominator;

    double modifier = double(m_timeModifierTop) / double(m_timeModifierBottom);
    m_duration = to_duration(m_nNoteType, m_nDots) * modifier;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_note_type_and_dots(int noteType, int dots)
{
    m_nNoteType = noteType;
    m_nDots = dots;
    m_duration = to_duration(m_nNoteType, m_nDots);
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_type_dots_duration(int noteType, int dots, TimeUnits duration)
{
    m_nNoteType = noteType;
    m_nDots = dots;
    m_duration = duration;

    m_timeModifierTop = 1;
    m_timeModifierBottom = 1;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_int_attribute(TIntAttribute attrib, int value)
{
    switch(attrib)
    {
        case k_attr_note_type:
            //TODO
            set_dirty(true);
            break;

        case k_attr_dots:
            //TODO
            set_dirty(true);
            break;

        case k_attr_voice:
            //TODO
            set_dirty(true);
            break;

        case k_attr_time_modifier_top:
            //TODO
            set_dirty(true);
            break;

        case k_attr_time_modifier_bottom:
            //TODO
            set_dirty(true);
            break;

        default:
            ImoStaffObj::set_int_attribute(attrib, value);
    }
}

//---------------------------------------------------------------------------------------
int ImoNoteRest::get_int_attribute(TIntAttribute attrib)
{
    switch(attrib)
    {
        case k_attr_note_type:              return m_nNoteType;
        case k_attr_dots:                   return m_nDots;
        case k_attr_voice:                  return m_nVoice;
        case k_attr_time_modifier_top:      return m_timeModifierTop;
        case k_attr_time_modifier_bottom:   return m_timeModifierBottom;
        default:
            return ImoStaffObj::get_int_attribute(attrib);
    }
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoNoteRest::get_supported_attributes()
{
    list<TIntAttribute> supported = ImoStaffObj::get_supported_attributes();
    supported.push_back(k_attr_time_modifier_bottom);
    supported.push_back(k_attr_time_modifier_top);
    supported.push_back(k_attr_voice);
    supported.push_back(k_attr_dots);
    supported.push_back(k_attr_note_type);
    return supported;
}


//=======================================================================================
// ImoNote implementation
//=======================================================================================
ImoNote::ImoNote()
    : ImoNoteRest(k_imo_note)
    , m_step(k_no_pitch)
    , m_octave(4)
    , m_actual_acc(k_acc_not_computed)
    , m_notated_acc(k_invalid_accidentals)
    , m_options(0)
    , m_stemDirection(k_stem_default)
    , m_pTieNext(nullptr)
    , m_pTiePrev(nullptr)
{
}

//---------------------------------------------------------------------------------------
ImoNote::ImoNote(int step, int octave, int noteType, EAccidentals accidentals, int dots,
                 int staff, int voice, int stem)
    : ImoNoteRest(k_imo_note)
    , m_step(step)
    , m_octave(octave)
    , m_actual_acc(k_acc_not_computed)
    , m_notated_acc(accidentals)
    , m_options(0)
    , m_stemDirection(stem)
    , m_pTieNext(nullptr)
    , m_pTiePrev(nullptr)
{
    m_nVoice = voice;
    m_staff = staff;
    set_note_type_and_dots(noteType, dots);
}

//---------------------------------------------------------------------------------------
ImoNote::~ImoNote()
{
    //if tied, inform the other note
    if (m_pTieNext)
        m_pTieNext->get_end_note()->set_tie_prev(nullptr);

    if (m_pTiePrev)
        m_pTiePrev->get_start_note()->set_tie_next(nullptr);
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_in_chord()
{
    return get_chord() != nullptr;
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
bool ImoNote::has_beam()
{
    ImoChord* pChord = get_chord();
    if (pChord == nullptr)
        return is_beamed();
    else
    {
        ImoNote* pNote = static_cast<ImoNote*>(pChord->get_start_object());
        return pNote->is_beamed();
    }
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

    //TODO: ImoNote::get_frequency
    return 0.0f;
}


//---------------------------------------------------------------------------------------
MidiPitch ImoNote::get_midi_pitch()
{
    if (m_actual_acc == k_acc_not_computed)
        return k_undefined_midi_pitch;
    else
        return MidiPitch(m_step, m_octave, int(m_actual_acc));
        //TODO: deal with fractional values used for microtones.
}

//---------------------------------------------------------------------------------------
int ImoNote::get_midi_bend()
{
    //Midi bend (two bytes)

    //TODO: ImoNote::get_midi_bend
    return 0;
}

//---------------------------------------------------------------------------------------
float ImoNote::get_cents()
{
    //deviation from diatonic pitch implied by step and octave, in cents.

    //TODO: ImoNote::get_cents
    return 0.0f;
}

//---------------------------------------------------------------------------------------
void ImoNote::set_int_attribute(TIntAttribute attrib, int value)
{
    switch(attrib)
    {
        case k_attr_step:
            //TODO
            set_dirty(true);
            break;

        case k_attr_octave:
            //TODO
            set_dirty(true);
            break;

        case k_attr_notated_accidentals:
            //TODO
            set_dirty(true);
            break;

        case k_attr_stem_type:
            //TODO
            m_stemDirection = value;
            set_dirty(true);
            break;

        default:
            ImoNoteRest::set_int_attribute(attrib, value);
    }
}

//---------------------------------------------------------------------------------------
int ImoNote::get_int_attribute(TIntAttribute attrib)
{
    switch(attrib)
    {
        case k_attr_step:                   return m_step;
        case k_attr_octave:                 return m_octave;
        case k_attr_notated_accidentals:    return m_notated_acc;
        case k_attr_stem_type:              return m_stemDirection;
        default:
            return ImoNoteRest::get_int_attribute(attrib);
    }
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoNote::get_supported_attributes()
{
    list<TIntAttribute> supported = ImoNoteRest::get_supported_attributes();
    supported.push_back(k_attr_stem_type);
    supported.push_back(k_attr_notated_accidentals);
    supported.push_back(k_attr_octave);
    supported.push_back(k_attr_step);
    return supported;
}


//=======================================================================================
// ImoRest implementation
//=======================================================================================
void ImoRest::set_int_attribute(TIntAttribute attrib, int value)
{
    ImoNoteRest::set_int_attribute(attrib, value);
}

//---------------------------------------------------------------------------------------
int ImoRest::get_int_attribute(TIntAttribute attrib)
{
    return ImoNoteRest::get_int_attribute(attrib);
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoRest::get_supported_attributes()
{
    list<TIntAttribute> supported = ImoNoteRest::get_supported_attributes();
    return supported;
}



}  //namespace lomse
