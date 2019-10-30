//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
// actual accidentals are, normally, not specified in LDP. Therefore, note pitch is
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
    void set_beam_type(int level, int type);
    int get_beam_type(int level);
    bool is_end_of_beam();
    /** Method ImoNoteRest::is_beamed() informs if the note has an attached beam
        relation, and not if the note is in a beamed group. Take into account that
        in beamed chords, the beam relation is only placed on the base note
        for each chord. Therefore, for notes in chord, method ImoNoteRest::is_beamed()
        is not reliable.
        Method ImoNote::has_been() is reliable in all circumstances and cases.
    */
    inline bool is_beamed() { return find_relation(k_imo_beam) != nullptr; }

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
/** ImoNote represents a note in the score.
**/
class ImoNote : public ImoNoteRest
{
protected:
    //pitch (step, octave, actual_acc) represents the sound, not what is notated.
    int     m_step;
    int     m_octave;
    float   m_actual_acc;           //number of semitones (i.e, -1 for flat). Decimal
                                    //values like 0.5 (quarter tone sharp) are also valid.

    //notated accidentals. Accidentals to display are deduced from key signature and
    //context, but can be overridden
    EAccidentals m_notated_acc;
    int          m_options;

    //stem and ties
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

    //options for notated accidentals
    enum ENotatedAcc
    {
        k_computed      = 0x0001, ///< Flag for detecting the first time notated accidentals are computed
        k_force         = 0x0002, ///< Force to display accidentals
        k_also_natural  = 0x0004, ///< When 'forced', this option will also display 'natural' accidentals.
        k_parenthesis   = 0x0008, ///< Add parentheses to displayed accidentals
        k_never         = 0x0010, ///< Do not display accidentals
        k_non_standard  = 0x0020, ///< Use non-standard acc. if necessary
    };


    //pitch
    inline int get_step() { return m_step; }
    inline int get_octave() { return m_octave; }
    inline float get_actual_accidentals() { return m_actual_acc; }
    inline bool is_pitch_defined() { return m_step != k_no_pitch; }

    /** Using this method could create inconsistencies between new pitch and notated
        accidentals unless it is used in conjunction with set_actual_accidentals()
        or PitchAssigner is later used for computing actual accidentals.
    */
    inline void set_notated_pitch(int step, int octave, EAccidentals accidentals)
    {
        m_step = step;
        m_octave = octave;
        m_notated_acc = accidentals;
        m_actual_acc = k_acc_not_computed;
    }

    /** Using this method could create inconsistencies between new pitch and notated
        accidentals unless it is used in conjunction with set_notated_accidentals()
        or PitchAssigner is later used for computing notated accidentals.
    */
    void set_pitch(FPitch fp) {
        m_step = fp.step();
        m_octave = fp.octave();
        m_actual_acc = float(fp.num_accidentals());
        m_notated_acc = k_invalid_accidentals;
    }

    /** Using this method could create inconsistencies between new pitch and notated
        accidentals unless it is used in conjunction with set_notated_accidentals()
        or PitchAssigner is later used for computing notated accidentals.
    */
    void set_pitch(int step, int octave, float actualAcc)
    {
        m_step = step;
        m_octave = octave;
        m_actual_acc = actualAcc;
    }


    ///@{
    /** Methods for doing atomic changes in ImoNote internal variables. They will create
        inconsistencies unless you know what you are doing.
        These methods are intended for notes's construction/modification and should not
        be used by your application unless you understand well what you are doing.
    */
    inline void set_step(int step) { m_step = step; }
    inline void set_octave(int octave) { m_octave = octave; }
    inline void set_actual_accidentals(float value) { m_actual_acc = value; }
    inline void set_notated_accidentals(EAccidentals accidentals) { m_notated_acc = accidentals; }
    inline void request_pitch_recomputation() { m_actual_acc = k_acc_not_computed; }
    ///@}

    //notated accidentals
    inline EAccidentals get_notated_accidentals() { return m_notated_acc; }
    inline void force_to_display_accidentals() { m_options |= k_force; }
    inline bool is_display_accidentals_forced() { return (m_options & k_force) != 0; }
    inline void force_to_display_naturals() { m_options |= k_also_natural; }
    inline bool is_display_naturals_forced() { return (m_options & k_also_natural) != 0; }
    inline void mark_notated_accidentals_as_computed() { m_options |= k_computed; }
    inline bool notated_accidentals_never_computed() { return !(m_options & k_computed); }

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

    /** Method ImoNoteRest::is_beamed() informs if the note has an attached beam
        relation, and not if the note is in a beamed group. Take into account that
        in beamed chords, the beam relation is only placed on the base note
        for each chord. Therefore, for notes in chord, method ImoNoteRest::is_beamed()
        is not reliable.
        Method ImoNote::has_been() is reliable in all circumstances and cases.
    */
    bool has_beam();

    //pitch. Only valid when m_actual_acc is computed
    FPitch get_fpitch();            //FPitch. Ignores fractional part of actual accidentals
    float get_frequency();          //frequecy, in Herzs
    MidiPitch get_midi_pitch();     //MidiPitch
    int get_midi_bend();            //Midi bend (two bytes)
    float get_cents();              //deviation from diatonic pitch implied by step and octave, in cents.

};


}   //namespace lomse

#endif    // __LOMSE_IM_NOTE_H__

