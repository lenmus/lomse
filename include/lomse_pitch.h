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

#ifndef __LOMSE_PITCH_H__
#define __LOMSE_PITCH_H__

#include "lomse_internal_model.h"        //for EKeySignature enum

#include <string>
using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
// Note steps: 'step' refers to the diatonic note name in the octave
enum ESteps
{
    k_no_pitch = -1,
    k_step_C = 0,
    k_step_D,
    k_step_E,
    k_step_F,
    k_step_G,
    k_step_A,
    k_step_B,
};

//---------------------------------------------------------------------------------------
// Octaves
// The octave is represented by a number in the range 0..9.
// Same meaning as in MIDI. The lowest MIDI octave (-1) is not defined
enum EOctave
{
    k_octave_0 = 0,
    k_octave_1,
    k_octave_2,
    k_octave_3,
    k_octave_4,
    k_octave_5,
    k_octave_6,
    k_octave_7,
    k_octave_8,
    k_octave_9,
};


//---------------------------------------------------------------------------------------
// Accidentals
// No microtonal accidentals. Only traditional ones.
// sharp_sharp is two consecutive sharp signs.
// double_sharp is the 'x' duoble shrp symbol

enum EAccidentals
{
    k_invalid_accidentals = -1,
    k_no_accidentals = 0,
    k_sharp,
    k_sharp_sharp,
    k_double_sharp,
    k_natural_sharp,
    k_flat,
    k_flat_flat,
    k_natural_flat,
    k_natural,
};


//---------------------------------------------------------------------------------------
//global functions
extern int accidentals_to_number(EAccidentals accidentals);


//---------------------------------------------------------------------------------------
//forward declarations
class DiatonicPitch;
class AbsolutePitch;
class MidiPitch;
class FPitch;


//---------------------------------------------------------------------------------------
// DiatonicPitch
// Represents the note in a diatonic scale. Only Step and Octave information.
// - Accidentals not represented. Incomplete information.
// - Usefull for sweeps along the diatonic notes range
//---------------------------------------------------------------------------------------
class DiatonicPitch
{
protected:
    int m_dp;

public:
    DiatonicPitch(int value) : m_dp(value) {}
    DiatonicPitch(int step, int octave);
    DiatonicPitch() : m_dp(-1) {}
    ~DiatonicPitch() {}

    //operations
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

    // operator to cast to an int
    operator const int() { return m_dp; }

    // conversion
    MidiPitch to_midi_pitch();
    FPitch to_FPitch(EKeySignature nKey);
    string get_english_note_name();
    string get_ldp_name();

    //components extraction
    inline int step() { return (m_dp - 1) % 7; }
    inline int octave() { return (m_dp - 1) / 7; }

    // comparison operators
    bool operator ==(DiatonicPitch dp) { return m_dp == int(dp); }
    bool operator !=(DiatonicPitch dp) { return m_dp != int(dp); }
    bool operator < (DiatonicPitch dp) { return m_dp < int(dp); }
    bool operator > (DiatonicPitch dp) { return m_dp > int(dp); }
    bool operator <= (DiatonicPitch dp) { return m_dp <= int(dp); }
    bool operator >= (DiatonicPitch dp) { return m_dp >= int(dp); }

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
//---------------------------------------------------------------------------------------
class MidiPitch
{
protected:
    int m_pitch;

public:
    MidiPitch(int note) : m_pitch(note) {}
    MidiPitch(int step, int octave, int acc=0);

    //operations
    //MidiPitch operator - (MidiPitch pitch) { return MidiPitch(m_pitch - pitch); }
    MidiPitch operator -(int i) { return MidiPitch(m_pitch - i); }
    MidiPitch operator +(int i) { return MidiPitch(m_pitch + i); }

    // operator to cast to an int
    operator const int() { return m_pitch; }

    string get_ldp_name();
    bool is_natural_note_for(EKeySignature nKey);

};

#define C4_MPITCH               MidiPitch(60)
#define k_undefined_midi_pitch  MidiPitch(-1)



//---------------------------------------------------------------------------------------
// FPitch
//  Base-40 absolute pitch representation. Interval-invariant. Only 2 accidentals
//---------------------------------------------------------------------------------------
class FPitch
{
protected:
    int m_fp;

public:
    //constructors
    FPitch() : m_fp(163) {}     //C4
    FPitch(int value) : m_fp(value) {}
//    FPitch(AbsolutePitch ap);
    FPitch(DiatonicPitch dp, int nAcc);
    FPitch(int nStep, int nOctave, int nAcc);
    FPitch(const string& note);
//    FPitch(int nStep, int nOctave, EKeySignature nKey);

    ~FPitch() {}

    // operator to cast to an int
    operator const int() { return m_fp; }

    // comparison operators
    bool operator ==(FPitch fp) { return m_fp == int(fp); }
    bool operator !=(FPitch fp) { return m_fp != int(fp); }
    bool operator < (FPitch fp) { return m_fp < int(fp); }
    bool operator > (FPitch fp) { return m_fp > int(fp); }
    bool operator <= (FPitch fp) { return m_fp <= int(fp); }
    bool operator >= (FPitch fp) { return m_fp >= int(fp); }

    //validation
    bool is_valid();

    //components extraction
    inline int step() { return (((m_fp - 1) % 40) + 1) / 6; }
    inline int octave() { return (m_fp - 1) / 40; }
    int num_accidentals();
    EAccidentals accidentals();
    EAccidentals notated_accidentals_for(EKeySignature nKey);

    //conversion
    string to_abs_ldp_name();
    string to_rel_ldp_name(EKeySignature nKey);
    MidiPitch to_midi_pitch();
    DiatonicPitch to_diatonic_pitch();
//    AbsolutePitch to_APitch();

    //operations
    FPitch add_semitone(EKeySignature nKey);
    FPitch add_semitone(bool fUseSharps);

    // Interval between 2 steps
    //extern FPitch FPitchStepsInterval(int nStep1, int nStep2, EKeySignature nKey);

    //other
    bool is_natural_note_for(EKeySignature nKey);

protected:
    void create(int nStep, int nOctave, int nAcc);

};

#define C4_FPITCH   FPitch(163)
#define k_undefined_fpitch   FPitch(-1)


//---------------------------------------------------------------------------------------
// UPitch: Microtonal pitch
//  Chromatic absolute pitch plus deviation in cents.
//---------------------------------------------------------------------------------------
class UPitch
{
private:
    int m_step;
    int m_octave;
    float m_acc;

public:
    //UPitch() : m_dp(-1), m_nAcc(0) {}
    //UPitch(DiatonicPitch dp, int nAcc) : m_dp(dp), m_nAcc(nAcc) {}
    UPitch(int step, int octave, float acc=0.0f) 
        : m_step(step), m_octave(octave), m_acc(acc)
    {
    }
    //UPitch(const string& note);

    ~UPitch() {}

    //access to components
    inline int step() const { return m_step; }
    inline int octave()const { return m_octave; }
    inline float accidentals() const { return m_acc; }

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
