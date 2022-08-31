//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_im_note.h"

#include "lomse_im_attributes.h"
#include "private/lomse_document_p.h"


namespace lomse
{


//=======================================================================================
// ImoNoteRest implementation
//=======================================================================================
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
void ImoNoteRest::set_time_modifiers_and_duration(int numerator, int denominator)
{
    m_timeModifierTop = numerator;
    m_timeModifierBottom = denominator;

    double modifier = double(m_timeModifierTop) / double(m_timeModifierBottom);
    m_duration = to_duration(m_nNoteType, m_nDots) * modifier;
    m_playDuration = m_duration;
    m_eventDuration = m_duration;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_time_modifiers(int numerator, int denominator)
{
    m_timeModifierTop = numerator;
    m_timeModifierBottom = denominator;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_note_type_and_dots(int noteType, int dots)
{
    m_nNoteType = noteType;
    m_nDots = dots;
    m_duration = to_duration(m_nNoteType, m_nDots);
    m_playDuration = m_duration;
    m_eventDuration = m_duration;
}

//---------------------------------------------------------------------------------------
void ImoNoteRest::set_type_dots_duration(int noteType, int dots, TimeUnits duration)
{
    m_nNoteType = noteType;
    m_nDots = dots;
    m_duration = duration;
    m_playDuration = m_duration;
    m_eventDuration = m_duration;

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
ImoNote::ImoNote(int type)
    : ImoNoteRest(type)
    , m_actual_acc(k_acc_not_computed)
    , m_notated_acc(k_invalid_accidentals)
    , m_options(0)
    , m_stemDirection(k_stem_default)
    , m_idTieNext(k_no_imoid)
    , m_idTiePrev(k_no_imoid)
    , m_computedStem(k_computed_stem_undecided)
{
}

//---------------------------------------------------------------------------------------
ImoNote::ImoNote(int step, int octave, int noteType, EAccidentals accidentals, int dots,
                 int staff, int voice, int stem)
    : ImoNoteRest(k_imo_note_regular)
    , m_actual_acc(k_acc_not_computed)
    , m_notated_acc(accidentals)
    , m_options(0)
    , m_stemDirection(stem)
    , m_idTieNext(k_no_imoid)
    , m_idTiePrev(k_no_imoid)
    , m_computedStem(k_computed_stem_undecided)
{
    m_step = step;
    m_octave = octave;
    m_nVoice = voice;
    m_staff = staff;
    set_note_type_and_dots(noteType, dots);
}

//---------------------------------------------------------------------------------------
ImoNote::~ImoNote()
{
    //if tied, inform the other note
    //AWARE: Ties will be deleted in ImoStaffObj destructor. But it is necesary to
    //inform the other notes to remove the tie Ids

    if (m_idTieNext != k_no_imoid)
        get_tie_next()->get_end_note()->set_tie_prev(nullptr);

    if (m_idTiePrev != k_no_imoid)
        get_tie_prev()->get_start_note()->set_tie_next(nullptr);
}

//---------------------------------------------------------------------------------------
ImoTie* ImoNote::get_tie_next()
{
    if (m_pDocModel && m_idTieNext != k_no_imoid)
        return static_cast<ImoTie*>( m_pDocModel->get_pointer_to_imo(m_idTieNext) );
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoTie* ImoNote::get_tie_prev()
{
    if (m_pDocModel && m_idTiePrev != k_no_imoid)
        return static_cast<ImoTie*>( m_pDocModel->get_pointer_to_imo(m_idTiePrev) );
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoNote::set_tie_next(ImoTie* pStartTie)
{
    m_idTieNext = (pStartTie ? pStartTie->get_id() : k_no_imoid);
}

//---------------------------------------------------------------------------------------
void ImoNote::set_tie_prev(ImoTie* pEndTie)
{
    m_idTiePrev = (pEndTie ? pEndTie->get_id() : k_no_imoid);
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
bool ImoNote::is_cross_staff_chord()
{
    ImoChord* pChord = get_chord();
    return pChord && pChord->is_cross_staff();
}

//---------------------------------------------------------------------------------------
void ImoNote::mute(EMuteType value)
{
    m_fMute = (value != k_mute_off);
}

//---------------------------------------------------------------------------------------
bool ImoNote::is_muted()
{
    return m_fMute != k_mute_off;
}

//---------------------------------------------------------------------------------------
bool ImoNote::has_beam()
{
    ImoChord* pChord = get_chord();
    if (pChord == nullptr)
        return is_beamed();
    else
    {
        ImoNote* pNote = pChord->get_start_note();
        return pNote->is_beamed();
    }
}

//---------------------------------------------------------------------------------------
ImoGraceRelObj* ImoNote::get_grace_relobj()
{
    return static_cast<ImoGraceRelObj*>( find_relation(k_imo_grace_relobj) );
}

//---------------------------------------------------------------------------------------
ImoArpeggio* ImoNote::get_arpeggio()
{
    return static_cast<ImoArpeggio*>( find_relation(k_imo_arpeggio) );
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
