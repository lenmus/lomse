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

#ifndef __LOMSE_IM_NOTE_H__        //to avoid nested includes
#define __LOMSE_IM_NOTE_H__

#include "lomse_internal_model.h"
#include "lomse_pitch.h"

using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
//noteheads
enum ENoteHeads
{
    k_notehead_longa = 1,
    k_notehead_breve,
    k_notehead_whole,              //Whole note
    k_notehead_half,               //Half note
    k_notehead_quarter,            //Quarter note
    k_notehead_cross,              //Cross (for percussion)
};

//---------------------------------------------------------------------------------------
//stem
enum ENoteStem
{
    k_stem_default = 0,
    k_stem_up,
    k_stem_down,
    k_stem_double,
    k_stem_none,
};

//---------------------------------------------------------------------------------------
// actaul accidentals are, normally, not specified in LDP. Therefore, note pitch is
// computed from notated accidentals. But for this, it is necessary to know when it
// is specified and when not.
#define k_acc_not_computed      10000.0f    //an absurd value


//---------------------------------------------------------------------------------------
class ImoNoteRest : public ImoStaffObj
{
protected:
    int         m_nNoteType;
    int         m_nDots;
    int         m_nVoice;
    int         m_timeModifierTop;
    int         m_timeModifierBottom;
    TimeUnits   m_duration;

public:
    ImoNoteRest(int objtype);
    virtual ~ImoNoteRest() {}

    //getters
    inline int get_note_type() { return m_nNoteType; }
    inline TimeUnits get_duration() { return m_duration; }
    inline int get_dots() { return m_nDots; }
    inline int get_voice() { return m_nVoice; }
    inline int get_time_modifier_top() { return m_timeModifierTop; }
    inline int get_time_modifier_bottom() { return m_timeModifierBottom; }

    //setters
    inline void set_note_type(int noteType) { m_nNoteType = noteType; }
    inline void set_dots(int dots) { m_nDots = dots; }
    inline void set_voice(int voice) { m_nVoice = voice; }
    void set_note_type_and_dots(int noteType, int dots);
    void set_time_modification(int numerator, int denominator);
    void set_type_dots_duration(int noteType, int dots, TimeUnits duration);

    //beam
    ImoBeam* get_beam();
    inline bool is_beamed() { return find_relation(k_imo_beam) != nullptr; }
    void set_beam_type(int level, int type);
    int get_beam_type(int level);
    bool is_end_of_beam();

    //tuplets
    bool is_in_tuplet();
    ImoTuplet* get_first_tuplet();

    //IM attributes interface
    virtual void set_int_attribute(TIntAttribute attrib, int value);
    virtual int get_int_attribute(TIntAttribute attrib);
    virtual list<TIntAttribute> get_supported_attributes();

};

//---------------------------------------------------------------------------------------
class ImoRest : public ImoNoteRest
{
protected:
    bool m_fGoFwd;
    bool m_fFullMeasureRest;

    friend class ImFactory;
    ImoRest() : ImoNoteRest(k_imo_rest), m_fGoFwd(false), m_fFullMeasureRest(false) {}

    friend class GoBackFwdAnalyser;
    friend class GoBackFwdLmdAnalyser;
    friend class FwdBackMxlAnalyser;
    inline void mark_as_go_fwd() { m_fGoFwd = true; }

    friend class NoteRestMxlAnalyser;
    friend class EventMnxAnalyser;
    inline void mark_as_full_measure(bool value) { m_fFullMeasureRest = value; }

    //IM attributes interface
    virtual void set_int_attribute(TIntAttribute attrib, int value);
    virtual int get_int_attribute(TIntAttribute attrib);
    virtual list<TIntAttribute> get_supported_attributes();

public:
    virtual ~ImoRest() {}

    inline bool is_go_fwd() { return m_fGoFwd; }
    inline bool is_full_measure() { return m_fFullMeasureRest; }
};

//---------------------------------------------------------------------------------------
class ImoNote : public ImoNoteRest
{
protected:
    int     m_step;
    int     m_octave;
    EAccidentals m_notated_acc;
    float   m_actual_acc;           //number of semitones (i.e, -1 for flat). Decimal
                                    //values like 0.5 (quarter tone sharp) are also valid.
    int     m_stemDirection;
    ImoTie* m_pTieNext;
    ImoTie* m_pTiePrev;

    friend class ImFactory;
    ImoNote();
    ImoNote(int step, int octave, int noteType, EAccidentals accidentals=k_no_accidentals,
            int dots=0, int staff=0, int voice=0, int stem=k_stem_default);

    //IM attributes interface
    virtual void set_int_attribute(TIntAttribute attrib, int value);
    virtual int get_int_attribute(TIntAttribute attrib);
    virtual list<TIntAttribute> get_supported_attributes();

public:
    virtual ~ImoNote();

    //pitch
    inline int get_step() { return m_step; }
    inline int get_octave() { return m_octave; }
    inline EAccidentals get_notated_accidentals() { return m_notated_acc; }
    inline float get_actual_accidentals() { return m_actual_acc; }
    inline bool is_pitch_defined() { return m_step != k_no_pitch; }
    inline bool accidentals_are_cautionary() { return false; }  //TODO method accidentals_are_cautionary
    inline void set_step(int step) { m_step = step; }
    inline void set_octave(int octave) { m_octave = octave; }
    inline void set_notated_accidentals(EAccidentals accidentals) { m_notated_acc = accidentals; }
    inline void set_notated_pitch(int step, int octave, EAccidentals accidentals)
    {
        m_step = step;
        m_octave = octave;
        m_notated_acc = accidentals;
        m_actual_acc = k_acc_not_computed;
    }
    void set_actual_accidentals(float value) { m_actual_acc = value; }
    void set_pitch(FPitch fp) {
        m_step = fp.step();
        m_octave = fp.octave();
        m_notated_acc = fp.accidentals();
        m_actual_acc = float(fp.num_accidentals());
    }



    //ties
    inline ImoTie* get_tie_next() { return m_pTieNext; }
    inline ImoTie* get_tie_prev() { return m_pTiePrev; }
    inline bool is_tied_next() { return m_pTieNext != nullptr; }
    inline bool is_tied_prev() { return m_pTiePrev != nullptr; }
    inline void set_tie_next(ImoTie* pStartTie) { m_pTieNext = pStartTie; }
    inline void set_tie_prev(ImoTie* pEndTie) { m_pTiePrev = pEndTie; }

    //stem
    inline bool has_stem() { return m_nNoteType >= k_half; }
    inline int get_stem_direction() { return m_stemDirection; }
    inline void set_stem_direction(int value) { m_stemDirection = value; }
    inline bool is_stem_up() { return m_stemDirection == k_stem_up; }
    inline bool is_stem_down() { return m_stemDirection == k_stem_down; }
    inline bool is_stem_default() { return m_stemDirection == k_stem_default; }

    //in chord
    bool is_in_chord();
    ImoChord* get_chord();
    bool is_start_of_chord();
    bool is_end_of_chord();

    //pitch. Only valid when m_actual_acc is computed
    FPitch get_fpitch();            //FPitch. Ignores fractional part of actual accidentals
    float get_frequency();          //frequecy, in Herzs
    MidiPitch get_midi_pitch();     //MidiPitch
    int get_midi_bend();            //Midi bend (two bytes)
    float get_cents();              //deviation from diatonic pitch implied by step and octave, in cents.

};


}   //namespace lomse

#endif    // __LOMSE_IM_NOTE_H__

