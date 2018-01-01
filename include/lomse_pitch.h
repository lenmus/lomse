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

#ifndef __LOMSE_PITCH_H__
#define __LOMSE_PITCH_H__

#include "lomse_internal_model.h"        //for EKeySignature enum

#include <string>
using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond


//---------------------------------------------------------------------------------------
// Note steps: 'step' refers to the diatonic note name in the octave
/** @ingroup enumerations

    This enum describes valid note steps. 'step' refers to the diatonic note name in the octave

    @#include <lomse_pitch.h>
*/
enum ESteps
{
    k_no_pitch = -1,    ///< No pitch assigned
    k_step_C = 0,       ///< C note (Do)
    k_step_D,           ///< D note (Re)
    k_step_E,           ///< E note (Mi)
    k_step_F,           ///< F note (Fa)
    k_step_G,           ///< G note (Sol)
    k_step_A,           ///< A note (La)
    k_step_B,           ///< B note (Si)
};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes valid octave numbers.
    The octave is represented by a number in the range 0..9 (scientific notation).
    This is the same meaning as in MIDI (note A in octave 4 = 440Hz).
    The lowest MIDI octave (-1) is not defined.

    @#include <lomse_pitch.h>
*/
enum EOctave
{
    k_octave_0 = 0,     ///< Octave 0. C0 = 16.352 Hz
    k_octave_1,         ///< Octave 1 (first octave, contra octave). C1 = 32.703 Hz
    k_octave_2,         ///< Octave 2
    k_octave_3,         ///< Octave 3
    k_octave_4,         ///< Octave 4. C4 = middle C. A4 = 440 Hz
    k_octave_5,         ///< Octave 5
    k_octave_6,         ///< Octave 6
    k_octave_7,         ///< Octave 7
    k_octave_8,         ///< Octave 8
    k_octave_9,         ///< Octave 9. B9 = 15,804.3 Hz
};


//---------------------------------------------------------------------------------------
// Accidentals
/** @ingroup enumerations

    This enum describes valid accidental signs.
    No microtonal accidentals are considered, only traditional ones.

    @#include <lomse_pitch.h>
*/
enum EAccidentals
{
    k_invalid_accidentals = -1,     ///< Invalid value for accidentals
    k_no_accidentals = 0,           ///< No accidental sign
    k_natural,                      ///< Natural accidental sign
    k_flat,                         ///< Flat accidental sign (b)
    k_sharp,                        ///< Natural accidental sign (#)
    k_flat_flat,                    ///< Two consecutive flat signs (bb)
    k_double_sharp,                 ///< The double sharp symbol (x)
    k_sharp_sharp,                  ///< Two consecutive sharp signs (##)
    k_natural_flat,                 ///< Natural sign follwed by flat sign
    k_natural_sharp,                ///< Natural sign follwed by sharp sign
};

//---------------------------------------------------------------------------------------
// global functions
/** Global function for converting an enumerated type to an integer representing the
    number of accidentals. Negative numbers correspond to flats, positive to sharps,
    and 0 to no accidentals.
*/
extern int accidentals_to_number(EAccidentals accidentals);


//---------------------------------------------------------------------------------------
//forward declarations
class DiatonicPitch;
class AbsolutePitch;
class MidiPitch;
class FPitch;


//---------------------------------------------------------------------------------------
// DiatonicPitch
/** Class DiatonicPitch represents the note in a diatonic scale. Only Step and Octave
    information. It has the following properties:
    - It is a very compact representation (just an int number).
    - Accidentals are not represented. Incomplete information.
    - Useful for sweeps along the diatonic notes range.
*/
class DiatonicPitch
{
protected:
    int m_dp;

public:
    /// Constructor from an int value representing a DiatonicPitch
    DiatonicPitch(int value) : m_dp(value) {}
    /// Constructor from pitch components.
    DiatonicPitch(int step, int octave);
    /// Default constructor. Invalid pitch.
    DiatonicPitch() : m_dp(-1) {}
    /// Destructor
    ~DiatonicPitch() {}

    ///@{
    /// Increment / decrement pitch by a certain number of steps.
    //DiatonicPitch operator - (DiatonicPitch pitch) { return DiatonicPitch(m_dp - pitch); }
    DiatonicPitch operator -(int i) { return DiatonicPitch(m_dp - i); }
    DiatonicPitch operator +(int i) { return DiatonicPitch(m_dp + i); }
    DiatonicPitch operator -=(int i) {
        m_dp -= i;
        return *this;
    }
    DiatonicPitch operator +=(int i) {
        m_dp += i;
        return *this;
    }
    ///@}

    /// Operator to cast to an int
    operator int() { return m_dp; }

    ///@{
    /// Pitch conversion to other formats.
    MidiPitch to_midi_pitch();
    FPitch to_FPitch(EKeySignature nKey);
    string get_english_note_name();
    string get_ldp_name();
    ///@}

    ///@{
    /// Components extraction
    inline int step() { return (m_dp - 1) % 7; }
    inline int octave() { return (m_dp - 1) / 7; }
    ///@}

    ///@{
    /// Comparison operators
    bool operator ==(DiatonicPitch dp) { return m_dp == int(dp); }
    bool operator !=(DiatonicPitch dp) { return m_dp != int(dp); }
    bool operator < (DiatonicPitch dp) { return m_dp < int(dp); }
    bool operator > (DiatonicPitch dp) { return m_dp > int(dp); }
    bool operator <= (DiatonicPitch dp) { return m_dp <= int(dp); }
    bool operator >= (DiatonicPitch dp) { return m_dp >= int(dp); }
    ///@}

protected:
    inline bool is_valid() { return m_dp > 0; }
};

#define C4_DPITCH   DiatonicPitch(29)
#define NO_DPITCH   DiatonicPitch(-1)



////---------------------------------------------------------------------------------------
//// AbsolutePitch
////  It is defined by the diatonic pitch (1..68) plus the number of
////  accidentals (-2..+2)
////---------------------------------------------------------------------------------------
//class AbsolutePitch
//{
//private:
//    DiatonicPitch m_dp;
//    int m_nAcc;   <== convert to float to allow for microtonality?
//
//public:
//    AbsolutePitch() : m_dp(-1), m_nAcc(0) {}
//    AbsolutePitch(DiatonicPitch dp, int nAcc) : m_dp(dp), m_nAcc(nAcc) {}
//    AbsolutePitch(int step, int octave, int acc) : m_dp(step, octave), m_nAcc(acc) {}
//    AbsolutePitch(const string& note);
//
//    ~AbsolutePitch() {}
//
//    inline DiatonicPitch to_diatonic_pitch() { return m_dp; }
//    inline int accidentals() { return m_nAcc; }
//
//    // comparison operators
//    bool operator ==(AbsolutePitch& ap) { return m_dp == ap.to_diatonic_pitch() && m_nAcc == ap.accidentals(); }
//    bool operator !=(AbsolutePitch& ap) { return m_dp != ap.to_diatonic_pitch() || m_nAcc != ap.accidentals(); }
//    bool operator < (AbsolutePitch& ap) { return m_dp < ap.to_diatonic_pitch() ||
//        (m_dp == ap.to_diatonic_pitch() && m_nAcc < ap.accidentals()); }
//    bool operator > (AbsolutePitch& ap) { return m_dp > ap.to_diatonic_pitch() ||
//        (m_dp == ap.to_diatonic_pitch() && m_nAcc > ap.accidentals()); }
//    bool operator <= (AbsolutePitch& ap) { return m_dp < ap.to_diatonic_pitch() ||
//        (m_dp == ap.to_diatonic_pitch() && m_nAcc <= ap.accidentals()); }
//    bool operator >= (AbsolutePitch& ap) { return m_dp > ap.to_diatonic_pitch() ||
//        (m_dp == ap.to_diatonic_pitch() && m_nAcc >= ap.accidentals()); }
//
//    //operations
//    inline DiatonicPitch IncrStep() { return ++m_dp; }
//    inline DiatonicPitch DecrStep() { return --m_dp; }
//
//    void Set(int nStep, int nOctave, int nAcc) {
//        m_nAcc = nAcc;
//        m_dp = nOctave * 7 + nStep + 1;
//    }
//    void Set(AbsolutePitch& ap) {
//        m_dp = ap.to_diatonic_pitch();
//        m_nAcc = ap.accidentals();
//    }
//    void Set(DiatonicPitch dp, int acc) {
//        m_dp = dp;
//        m_nAcc = acc;
//    }
//
//    void SetAccidentals(int nAcc) { m_nAcc = nAcc; }
//    void SetDiatonicPitch(DiatonicPitch dp) { m_dp = dp; }
//
//    //conversions
//    inline int step() const { return (m_dp - 1) % 7; }
//    inline int octave()const { return (m_dp - 1) / 7; }
//    string get_ldp_name() const;
//    MidiPitch to_MidiPitch() const;
//
//};



//---------------------------------------------------------------------------------------
// MidiPitch
/** Class MidiPitch is just an int number representing the pitch in a chromatic scale.
    It is the same as the MIDI key number used in MIDI for representing pitch.
    It has the following properties:
    - Very compact.
    - Equal temperament tunning assumed. This implies that there is no possibility to
        differentiate between enharmonic sounds, for instance, C sharp and D flat.
    - It is intended to be used only for sound generation.
    - It is usable for sweeps along the equal temperament sounds.
*/
class MidiPitch
{
protected:
    int m_pitch;

public:
    /// Constructor from a MIDI value.
    MidiPitch(int note) : m_pitch(note) {}
    /// Constructor from the pitch components.
    MidiPitch(int step, int octave, int acc=0);

    //operations
    //MidiPitch operator - (MidiPitch pitch) { return MidiPitch(m_pitch - pitch); }
    ///@{
    /// Increment / decrement pitch by an arbitrary number of MIDI steps
    MidiPitch operator -(int i) { return MidiPitch(m_pitch - i); }
    MidiPitch operator +(int i) { return MidiPitch(m_pitch + i); }
    ///@}

    /// Operator to cast to an int
    operator int() { return m_pitch; }

    /** Returns the name of this pitch in LPD format, i.e. MidiPitch 60 will
        return "c4".    */
    string get_ldp_name();

    /** Returns TRUE if the pitch corresponds to a diatonic pitch in the given
        key signature.    */
    bool is_natural_note_for(EKeySignature nKey);

};

#define C4_MPITCH               MidiPitch(60)
#define k_undefined_midi_pitch  MidiPitch(-1)



//---------------------------------------------------------------------------------------
// FPitch
/** Class FPitch is a compact base-40 absolute pitch representation.
    It is interval-invariant. And is restricted to a maximum of 2 accidentals
    - 'F' stands for <b>Fourty</b>.
    - It is an absolute pitch representation, with an interesting property: is
        interval-invariant, that is, the number obtained by subtracting two pitches
        represents the interval between them. Therefore, this representation is
        specially useful to deal with intervals.
    - Each octave requires an range of 40 values. So to represent 10 octaves (C0-B9) a
        range of 400 is required. An <i>int</i> is enough.

    FPitch representation has shown to be optimal for representing pitch in cases where
    music analysis is an objective, as it greatly simplifies the analysis of intervals
    and chords.

    This representation was taken from Walter B. Hewlett paper "<i>A Base-40 Number-line
    Representation of Musical Pitch Notation</i>", CCARH Publications, Stanford,
    California, 1986.
*/
class FPitch
{
protected:
    int m_fp;

public:
    //constructors
    /// Default constructor. Initializes with C4 pitch.
    FPitch() : m_fp(163) {}     //C4
    /// Constructor using an integer representing an FPitch value.
    FPitch(int value) : m_fp(value) {}
//    FPitch(AbsolutePitch ap);
    /// Constructor using a DiatonicPitch plus accidentals information.
    FPitch(DiatonicPitch dp, int nAcc);
    /// Constructor from pitch components.
    FPitch(int nStep, int nOctave, int nAcc);
    /// Constructor from an string representing the pitch in LDP name, i.e. "+c4".
    FPitch(const string& note);
//    FPitch(int nStep, int nOctave, EKeySignature nKey);

    /// Destructor
    ~FPitch() {}

    /// Operator to cast to an int.
    operator int() { return m_fp; }

    ///@{
    /// Comparison operator
    bool operator ==(FPitch fp) { return m_fp == int(fp); }
    bool operator !=(FPitch fp) { return m_fp != int(fp); }
    bool operator < (FPitch fp) { return m_fp < int(fp); }
    bool operator > (FPitch fp) { return m_fp > int(fp); }
    bool operator <= (FPitch fp) { return m_fp <= int(fp); }
    bool operator >= (FPitch fp) { return m_fp >= int(fp); }
    ///@}

    /** Returns TRUE if pitch value is valid. Invalid pitches correspond to
        numbers 6, 12, 23, 29 & 35 and all obtained by adding 40, 80, 120, etc. to them.    */
    bool is_valid();

    ///@{
    /// Components extraction
    inline int step() { return (((m_fp - 1) % 40) + 1) / 6; }
    inline int octave() { return (m_fp - 1) / 40; }
    int num_accidentals();
    EAccidentals accidentals();
    EAccidentals notated_accidentals_for(EKeySignature nKey);
    ///@}

    ///@{
    /// Conversion
    string to_abs_ldp_name();
    string to_rel_ldp_name(EKeySignature nKey);
    MidiPitch to_midi_pitch();
    DiatonicPitch to_diatonic_pitch();
//    AbsolutePitch to_APitch();
    ///@}

    ///@{
    /// Operations
    FPitch add_semitone(EKeySignature nKey);
    FPitch add_semitone(bool fUseSharps);
    ///@}

    // Interval between 2 steps
    //extern FPitch FPitchStepsInterval(int nStep1, int nStep2, EKeySignature nKey);

    //other
    /// Returns TRUE if pitch is a diatonic note in the given key signature.
    bool is_natural_note_for(EKeySignature nKey);

protected:
    void create(int nStep, int nOctave, int nAcc);

};

#define C4_FPITCH   FPitch(163)
#define k_undefined_fpitch   FPitch(-1)


//---------------------------------------------------------------------------------------
// UPitch: Microtonal pitch
/** Class UPitch represents a chromatic absolute pitch plus deviation in cents.
*/
class UPitch
{
private:
    int m_step;
    int m_octave;
    float m_acc;

public:
    //UPitch() : m_dp(-1), m_nAcc(0) {}
    //UPitch(DiatonicPitch dp, int nAcc) : m_dp(dp), m_nAcc(nAcc) {}
    ///Constructor from pitch components.
    UPitch(int step, int octave, float acc=0.0f)
        : m_step(step), m_octave(octave), m_acc(acc)
    {
    }
    //UPitch(const string& note);

    /// Destructor
    ~UPitch() {}

    ///@{
    /// Access to pitch components
    inline int step() const { return m_step; }
    inline int octave()const { return m_octave; }
    inline float accidentals() const { return m_acc; }
    ///@}

    //// comparison operators
    //bool operator ==(UPitch& up) { return m_dp == up.to_diatonic_pitch() && m_nAcc == up.accidentals(); }
    //bool operator !=(UPitch& up) { return m_dp != up.to_diatonic_pitch() || m_nAcc != up.accidentals(); }
    //bool operator < (UPitch& up) { return m_dp < up.to_diatonic_pitch() ||
    //    (m_dp == up.to_diatonic_pitch() && m_nAcc < up.accidentals()); }
    //bool operator > (UPitch& up) { return m_dp > up.to_diatonic_pitch() ||
    //    (m_dp == up.to_diatonic_pitch() && m_nAcc > up.accidentals()); }
    //bool operator <= (UPitch& up) { return m_dp < up.to_diatonic_pitch() ||
    //    (m_dp == up.to_diatonic_pitch() && m_nAcc <= up.accidentals()); }
    //bool operator >= (UPitch& up) { return m_dp > up.to_diatonic_pitch() ||
    //    (m_dp == up.to_diatonic_pitch() && m_nAcc >= up.accidentals()); }

    ////operations
    //inline DiatonicPitch IncrStep() { return ++m_dp; }
    //inline DiatonicPitch DecrStep() { return --m_dp; }

    ////conversions
    //string get_ldp_name() const;
    //MidiPitch to_MidiPitch() const;
};

} //namespace lomse

#endif    //__LOMSE_PITCH_H__
