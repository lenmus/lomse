//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_PITCH_H__
#define __LOMSE_PITCH_H__

#include "lomse_internal_model.h"       //for EKeySignature enum
#include "lomse_interval.h"             //for class FIntval

#include <string>
using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond


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
    ///@{
    /// Increment / decrement pitch by an arbitrary number of MIDI steps
    MidiPitch operator -(int i) { return MidiPitch(m_pitch - i); }
    MidiPitch operator +(int i) { return MidiPitch(m_pitch + i); }
    MidiPitch operator +=(int i) { m_pitch += i; return *this; }
    MidiPitch operator -=(int i) { m_pitch -= i; return *this; }
    ///@}

    /// Operator to cast to an int
    operator int() { return m_pitch; }

    /** Returns the name of this pitch in LPD format, e.g., MidiPitch 60 will
        return "c4".    */
    string get_ldp_name();

    /** Returns TRUE if the pitch corresponds to a diatonic pitch in the given
        key signature.    */
    bool is_natural_note_for(EKeySignature nKey);

    ///@{
    ///Components extraction
    int octave(EKeySignature nKey);
    int step(EKeySignature nKey);
    int accidentals(EKeySignature nKey);
    void get_components(EKeySignature nKey, int* step, int* octave, int* acc);
    ///@}

protected:
    int extract_octave(int step);
    int extract_accidentals(int step, int octave);
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
    - Each octave requires a range of 40 values. So to represent 10 octaves (C0-B9) a
        range of 400 is required. An <i>int</i> is enough.

    FPitch representation has shown to be optimal for representing pitch in cases where
    music analysis is an objective, as it greatly simplifies the analysis of intervals
    and chords.

    This representation was taken from Walter B. Hewlett paper <i>"A Base-40 Number-line
    Representation of Musical Pitch Notation"</i>, CCARH Publications, Stanford,
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
    /// Constructor from an string representing the pitch in LDP name, e.g., "+c4".
    FPitch(const string& note);
//    FPitch(int nStep, int nOctave, EKeySignature nKey);

    /// Destructor
    ~FPitch() {}

    /// Operator to cast to an int.
    operator int() { return m_fp; }

    ///@{
    /// Comparison operators
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

    ///@{
    /// Add/substract an interval
    FPitch operator -(FIntval intv) { return FPitch(m_fp - int(intv)); }
    FPitch operator +(FIntval intv) { return FPitch(m_fp + int(intv)); }
    FPitch operator -=(FIntval intv) {
        m_fp -= int(intv);
        return FPitch(m_fp);
    }
    FPitch operator +=(FIntval intv) {
        m_fp += int(intv);
        return FPitch(m_fp);
    }
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


// UPitch not yet implemented. This is just tentative skeleton code.
///@cond INTERNALS

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
///@endcond

} //namespace lomse

#endif    //__LOMSE_PITCH_H__
