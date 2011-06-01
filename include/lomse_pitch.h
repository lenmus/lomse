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


//using namespace std;

namespace lomse
{


//typedef int FIntval;      // Intervals, in FPitch mode.




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


////---------------------------------------------------------------------------------------
//// Accidentals
//// No microtonal accidentals. Only traditional ones.
//#define LOMSE_FLAT_FLAT        -2
//#define LOMSE_FLAT             -1
//#define LOMSE_NO_ACCIDENTAL     0
//#define LOMSE_SHARP             1
//#define LOMSE_SHARP_SHARP       2
//
//#define LOMSE_NO_NOTE          -1  // DiatonicPitch = -1
//
////forward declarations
//class DiatonicPitch;
//class AbsolutePitch;
//class MidiPitch;
//class FPitch;

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
    ~DiatonicPitch() {}

    //operations
    //DiatonicPitch operator - (DiatonicPitch pitch) { return DiatonicPitch(m_dp - pitch); }
    DiatonicPitch operator -(int i) { return DiatonicPitch(m_dp - i); }
    DiatonicPitch operator +(int i) { return DiatonicPitch(m_dp + i); }

    // operator to cast to an int
    operator const int() { return m_dp; }

//    // conversion
//    MidiPitch to_midi_pitch();
//    FPitch to_FPitch(lmEKeySignatures nKey);
//    std::string get_english_note_name();
//    std::string get_ldp_name();
//    inline int get_value() { return m_dp; }
//
//    //components extraction
//    inline int step() { return (m_dp - 1) % 7; }
//    inline int octave() { return (m_dp - 1) / 7; }
//
//    // comparison operators
//    bool operator ==(DiatonicPitch dp) { return m_dp == dp.get_value(); }
//    bool operator !=(DiatonicPitch dp) { return m_dp != dp.get_value(); }
//    bool operator < (DiatonicPitch dp) { return m_dp < dp.get_value(); }
//    bool operator > (DiatonicPitch dp) { return m_dp > dp.get_value(); }
//    bool operator <= (DiatonicPitch dp) { return m_dp <= dp.get_value(); }
//    bool operator >= (DiatonicPitch dp) { return m_dp >= dp.get_value(); }

};

#define LOMSE_C4_DPITCH   29
#define LOMSE_NO_DPITCH   -1



////---------------------------------------------------------------------------------------
//// AbsolutePitch
////  It is defined by the diatonic pitch (1..68) plus the number of
////  accidentals (-2..+2)
////---------------------------------------------------------------------------------------
//class AbsolutePitch
//{
//private:
//    DiatonicPitch m_dp;
//    int m_nAcc;
//
//public:
//    AbsolutePitch() : m_dp(-1), m_nAcc(0) {}
//    AbsolutePitch(DiatonicPitch dp, int nAcc) : m_dp(dp), m_nAcc(nAcc) {}
//    AbsolutePitch(int step, int octave, int acc) : m_dp(step, octave), m_nAcc(acc) {}
//    AbsolutePitch(const std::string& note);
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
//    std::string get_ldp_name() const;
//    MidiPitch get_midi_pitch() const;
//
//};
//
//
//
////---------------------------------------------------------------------------------------
//// MidiPitch
////---------------------------------------------------------------------------------------
//class MidiPitch
//{
//protected:
//    int m_pitch;
//
//public:
//    MidiPitch(int note) : m_pitch(note) {}
//    MidiPitch(int step, int octave, int acc);
//
//    std::string to_ldp_name();
//    bool is_natural_note(int nKey);
//
//};
//
//#define LOMSE_C4_MPITCH   60
//
//
//
////---------------------------------------------------------------------------------------
//// FPitch
////  Base-40 absolute pitch representation. Interval-invariant. Only 2 accidentals
////---------------------------------------------------------------------------------------
//class FPitch
//{
//protected:
//    int m_fp;
//
//public:
//    //constructors
//    FPitch(int value) : m_fp(value) {}
//    FPitch(AbsolutePitch ap);
//    FPitch(DiatonicPitch dp, int nAcc);
//    FPitch(int nStep, int nOctave, int nAcc);
//    FPitch(const std::string& sLDPNote);
//    FPitch(int nStep, int nOctave, int nKey);
//
//    ~FPitch() {}
//
//    //validation
//    bool is_valid();
//
//    //components extraction
//    inline int step() { return (((m_fp - 1) % 40) + 1) / 6; }
//    inline int octave() { return (m_fp - 1) / 40; }
//    int accidentals();
//
//    //conversion
//    std::string FPitch_ToAbsLDPName();
//    std::string FPitch_ToRelLDPName(int nKey);
//    MidiPitch to_MidiPitch();
//    DiatonicPitch to_DiatonicPitch();
//    AbsolutePitch to_APitch();
//
//    //operations
//    FPitch add_semitone(int nKey);
//    FPitch add_semitone(bool fUseSharps);
//    // Interval between 2 steps
//    //extern FPitch FPitchStepsInterval(int nStep1, int nStep2, int nKey);
//    std::string to_abs_ldp_name();
//    std::string to_rel_ldp_name(int nKey);
//
//protected:
//    void create(int nStep, int nOctave, int nAcc);
//
//};
//
//#define LOMSE_C4_FPITCH     FPitch(163)
//#define LOMSE_C4_FPITCH   163
//
//////constructors
////extern FPitch FPitch(AbsolutePitch ap);
////extern FPitch FPitch(DiatonicPitch dp, int nAcc);
////extern FPitch FPitch(int nStep, int nOctave, int nAcc);
////extern FPitch FPitch(const std::string& sLDPNote);
////extern FPitch FPitchK(int nStep, int nOctave, LOMSE_EKeySignatures nKey);
////
//////validation
////extern bool FPitch_IsValid(FPitch fp);
////
//////components extraction
////#define FPitch_Step(fp) ((((fp - 1) % 40) + 1) / 6)
////#define FPitch_Octave(fp) ((fp - 1) / 40)
////extern int FPitch_Accidentals(FPitch fp);
////
//////conversion
////extern std::string FPitch_ToAbsLDPName(FPitch fp);
////extern std::string FPitch_ToRelLDPName(FPitch fp, LOMSE_EKeySignatures nKey);
////extern MidiPitch FPitch_ToMidiPitch(FPitch fp);
////extern DiatonicPitch FPitch_ToDiatonicPitch(FPitch fp);
////extern AbsolutePitch FPitch_ToAPitch(FPitch fp);
////
//////operations
////extern FPitch FPitch_AddSemitone(FPitch fpNote, LOMSE_EKeySignatures nKey);
////extern FPitch FPitch_AddSemitone(FPitch fpNote, bool fUseSharps);
////// Interval between 2 steps
////extern FPitch FPitchStepsInterval(int nStep1, int nStep2, LOMSE_EKeySignatures nKey);
////
////    // return the note letter (A .. G) corresponding to the step of the note, in FPitch notation
////    extern std::string FPitch_GetEnglishNoteName(FPitch fp);



} //namespace lomse

#endif    //__LOMSE_PITCH_H__
