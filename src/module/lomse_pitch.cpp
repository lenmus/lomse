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

#include "lomse_pitch.h"

#include <sstream>
using namespace std;

namespace lomse
{


//static std::string m_sNoteName[7] = { "c",  "d", "e", "f", "g", "a", "b" };
//
////interval names. The index in this array is the FInterval value
//std::string m_sFIntvalCode[41] = {
//    "p1", "a1", "no", "no", "d2", "m2", "M2", "a2",
//    "no", "no", "d3", "m3", "M3", "a3", "no", "no",
//    "d4", "p4", "a4", "no", "no", "no", "d5", "p5",
//    "a5", "no", "no", "d6", "m6", "M6", "a6", "no",
//    "no", "d7", "m7", "M7", "a7", "no", "no", "d8",
//    "p8"
//};
//
//
////-------------------------------------------------------------------------------------
//// implementation of FPitch: Absolute pitch, interval-invariant (base-40)
////-------------------------------------------------------------------------------------
//FPitch::FPitch(DiatonicPitch dp, int nAcc)
//{
//    create(dp.step(), dp.octave(), nAcc);
//}
//
//FPitch::FPitch(AbsolutePitch ap)
//{
//    create(ap.step(), ap.octave(), ap.accidentals());
//}
//
//FPitch::FPitch(int nStep, int nOctave, int nAcc)
//{
//    create(nStep, nOctave, nAcc);
//}
//
//void FPitch::create(int nStep, int nOctave, int nAcc)
//{
//    //  Cbb Cb  C   C#  C## -   Dbb Db  D   D#  D## -   Ebb Eb  E   E#  E## Fbb Fb  F   F#  F## -
//    //  1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23
//    //
//    //  Gbb Gb  G   G#  G## -   Abb Ab  A   A#  A## -   Bbb Bb  B   B#  B##
//    //  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40
//    //
//    //  Pitch generation:
//    //  Note:       C=3, D=9, E=15, F=20, G=26, A=32, B=38
//    //  Diff:           6    6     5     6     6    6     5
//    //  Step:         0    1    2     3     4     5     6
//    //  step*6+2:     2    8    14    20    26    32    38
//    //  if s<3  ++    3    9    15    20    26    32    38
//    //  +=Acc
//    //
//    //  Step extraction:
//    //  Note:       C=1-5, D=7-11, E=13-17, F=18-22, G=24-28, A=30-34, B=36-40
//    //  Step:         0    1       2        3        4        5        6
//    //  n/6:          0-0  1-1     2-2      3-3      4-4      5-5      6-6
//    //
//    //  Octave extraction:
//    //  Note (n):     0=1-40, 1=41-80, 2=81-120, m=(m*40)+1 - (m+1)*40
//    //  (n-1)/40:     0       2        3         m
//    //
//    //  Accidentals extraction
//    //  Note (n):        0=1-40, 1=41-80, 2=81-120, m=(m*40)+1 - (m+1)*40
//    //  x=((n-1)%40)+1:  1-40    1-40     1-40      1-40
//    //  step=n/6
//    //  pitch approx. = step*6+2
//    //  acc = x - pitch
//    //  if step<3 acc--
//
//    int fpPitch = (nOctave * 40) + (nStep * 6) + 2;
//    if (nStep < 3) fpPitch++;
//    m_fp = fpPitch + nAcc;
//
//}
//
//FPitch::FPitch(int nStep, int nOctave, lmEKeySignatures nKey)
//{
//    int nAccidentals[7];
//    lmComputeAccidentals(nKey, nAccidentals);
//
//    create(nStep, nOctave, nAccidentals[nStep]);
//}
//
//bool FPitch::is_valid()
//{
//    //returns false for invalid pitches (6, 12, 23, 29 & 35)
//
//    int x = ((m_fp - 1) % 40 ) + 1;
//    return !(x == 6 || x == 12 || x == 23 || x == 29 || x == 35);
//}
//
//FPitch::FPitch(const std::string& sNote)
//{
//    // sNote must be letter followed by a number (i.e.: "c4" ) optionally precedeed by
//    // accidentals (i.e.: "++c4")
//    // It is assumed that sNote is trimmed (no spaces before or after data)
//    // and lower case.
//
//    //split the string: accidentals and name
//    std::string sAccidentals;
//    switch (sNote.length()) {
//        case 2:
//            sAccidentals = "";
//            break;
//        case 3:
//            sAccidentals = sNote.substr(0, 1);
//            sNote = sNote.substr(1, 2);
//            break;
//        case 4:
//            sAccidentals = sNote.substr(0, 2);
//            sNote = sNote.substr(2, 2);
//            break;
//        default:
//            wxLogMessage("[FPitch constructor] Bad note name '%s'", sNote.c_str());
//            return lmC4_FPITCH;
//    }
//
//    //compute step
//    wxChar sStep = sNote.at(0);
//    int nStep;
//    if (sStep == _T('c')) nStep = 0;
//    else if (sStep == _T('d')) nStep =  1;
//    else if (sStep == _T('e')) nStep =  2;
//    else if (sStep == _T('f')) nStep =  3;
//    else if (sStep == _T('g')) nStep =  4;
//    else if (sStep == _T('a')) nStep =  5;
//    else if (sStep == _T('b')) nStep =  6;
//    else {
//        wxLogMessage("[FPitch constructor] Bad note name '%s'", sNote.c_str());
//        return lmC4_FPITCH;
//    }
//
//    //compute octave
//    std::string sOctave = sNote.substr(1, 1);
//    long nOctave;
//    if (!sOctave.ToLong(&nOctave)) {
//        wxLogMessage("[FPitch constructor] Bad note name '%s'", sNote.c_str());
//        return lmC4_FPITCH;
//    }
//
//    //compute accidentals
//    int nAccidentals;
//    if (sAccidentals == ""))          nAccidentals = 0;
//    else if (sAccidentals == "-"))    nAccidentals = -1;
//    else if (sAccidentals == "--"))   nAccidentals = -2;
//    else if (sAccidentals == "+"))    nAccidentals = 1;
//    else if (sAccidentals == "++"))   nAccidentals = 2;
//    else if (sAccidentals == "x"))    nAccidentals = 2;
//    else {
//        wxLogMessage("[FPitch constructor] Bad note name '%s'", sNote.c_str());
//        return lmC4_FPITCH;
//    }
//
//    create(nStep, nOctave, nAccidentals);
//
//}
//
//int FPitch::accidentals()
//{
//    //  Accidentals extraction
//    //  Note (n):        0=1-40, 1=41-80, 2=81-120, m=(m*40)+1 - (m+1)*40
//    //  x=((n-1)%40)+1:  1-40    1-40     1-40      1-40
//    //  step=x/6
//    //  compute pitch
//    //  acc = x - pitch
//
//    int nReduction = ((m_fp - 1) % 40 ) + 1;
//    int nStep = nReduction / 6;
//    int nPitch = (nStep * 6) + 2;
//    if (nStep < 3) nPitch++;
//    return nReduction - nPitch;
//}
//
//std::string FPitch::to_abs_ldp_name()
//{
//    // The absolute LDP none name is returned
//    // If note is invalid (more than two accidentals) returns empty string
//
//    std::string sAnswer;
//    switch(accidentals()) {
//        case -2: sAnswer ="--"; break;
//        case -1: sAnswer ="-"; break;
//        case 0:  sAnswer =""; break;
//        case 1:  sAnswer ="+"; break;
//        case 2:  sAnswer ="x"; break;
//        default:
//            return "";
//    }
//    sAnswer += m_sNoteName[FPitch_Step(fp)];
//    sAnswer += std::string::Format("%d", FPitch_Octave(fp));
//    return sAnswer;
//}
//
//std::string FPitch::to_rel_ldp_name(lmEKeySignatures nKey)
//{
//    // The note LDP name, relative to received key signature, is returned
//
//    // Get the accidentals implied by the key signature.
//    // Each element of the array refers to one note: 0=Do, 1=Re, 2=Mi, 3=Fa, ... , 6=Si
//    // and its value can be one of: 0=no accidental, -1 = a flat, 1 = a sharp
//    int nAccidentals[7];
//    lmComputeAccidentals(nKey, nAccidentals);
//
//    //compute note accidentals
//    std::string sAnswer;
//    int nAbsAcc = accidentals();
//    switch(nAbsAcc) {
//        case -2: sAnswer ="--"; break;
//        case -1: sAnswer ="-"; break;
//        case 0:  sAnswer =""; break;
//        case 1:  sAnswer ="+"; break;
//        case 2:  sAnswer ="x"; break;
//        default:
//            sAnswer = "";
//    }
//
//    //change note accidentals to take key into account
//    int nStep = FPitch_Step(fp);
//    if (nAccidentals[nStep] != 0) {
//        if (nAbsAcc == nAccidentals[nStep])
//            sAnswer = "";   //replace note accidental by key accidental
//        else if (nAbsAcc == 0)
//            sAnswer = "=";  //force a natural
//        //else
//            //leave note accidentals
//    }
//
//    // add step letter and octave number
//    sAnswer += m_sNoteName[nStep];
//    sAnswer += std::string::Format("%d", octave());
//
//    return sAnswer;
//}
//
//MidiPitch FPitch::to_MidiPitch()
//{
//    ASSERT(lmC4_DPITCH == 29);    //AWARE It's assumed that we start in C0
//    MidiPitch nMidi = (MidiPitch)((octave()+1) * 12);
//
//    switch(FPitch_ToDiatonicPitch(fp) % 7)
//    {
//        case 0:  //si
//            nMidi = nMidi + 11;
//            break;
//        case 1:  //do
//            //nothing to add. The value is ok
//            break;
//        case 2:  //re
//            nMidi = nMidi + 2;
//            break;
//        case 3:  //mi
//            nMidi = nMidi + 4;
//            break;
//        case 4:  //fa
//            nMidi = nMidi + 5;
//            break;
//        case 5:  //sol
//            nMidi = nMidi + 7;
//            break;
//        case 6:  //la
//            nMidi = nMidi + 9;
//            break;
//    }
//
//    return nMidi + accidentals();
//
//}
//
//DiatonicPitch FPitch::to_DiatonicPitch()
//{
//    return DiatonicPitch(step(), octave());
//}
//
//AbsolutePitch FPitch::to_APitch()
//{
//    return AbsolutePitch(to_DiatonicPitch(), accidentals());
//}
//
//FPitch& FPitch::add_semitone(lmEKeySignatures nKey)
//{
//    // This function adds one semitone to note fpNote and returns the resulting
//    // note.
//    // Step and accidentals of new note are adjusted to fit 'naturally' in the key
//    // signature received: one accidental at maximum, of the same type than the
//    // accidentals in the key signature.
//    return add_semitone(KeySignatureToNumFifths(nKey) >= 0);
//}
//
//
//FPitch& FPitch::add_semitone(bool fUseSharps)
//{
//    // This function adds one semitone to note fpNote and returns the resulting
//    // note.
//    // Step and accidentals of new note are adjusted to use one accidental at
//    // maximum, of the same type as requested by fUseSharps
//
//    // step 0   0   1   1   2   3   3   4   4   5   5   6
//        //  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
//        //  3   4   9   10  15  20  21  26  27  32  33  38
//
//    // step 0   1   1   2   2   3   4   4   5   5   6   6
//        //  C   Db  D   Eb  E   F   Gb  G   Ab  A   Bb  B
//        //  3   8   9   14  15  20  25  26  31  32  37  38
//
//
//    // extract components
//    int nStep = step();
//    int nAcc = accidentals();
//    int nOctave = octave();
//
//    //Determine what type of accidentals to use for semitones: flats or sharps
//    if (fUseSharps)
//    {
//        //Key signature is using sharps.
//        //Only one accidental allowed and must be a sharp
//        if (nAcc == 0)
//        {
//            //if no accidentals add one sharp or advance step, depending on current step
//            if (nStep == 2 || nStep == 6)
//                nStep++;
//            else
//                nAcc++;
//        }
//        else
//        {
//            //assume one sharp: advance to next natural step
//            nStep++;
//            nAcc=0;
//        }
//
//        // Adjust octave
//        if (nStep==7) {
//            nStep = 0;
//            nOctave++;
//        }
//    }
//    else
//    {
//        //Key signature is using flats.
//        //Only one accidental allowed and must be a flat
//        if (nAcc == 0)
//        {
//            //if no accidentals advance step and, depending on new step, add one flat
//            nStep++;
//            if (nStep == 7) {
//                nStep = 0;
//                nOctave++;
//            }
//            else if (nStep != 3)
//                nAcc--;
//        }
//        else
//        {
//            //assume one flat: remove accidentals to advance to the natural step
//            nAcc=0;
//        }
//    }
//
//    create(nStep, nOctave, nAcc);
//    return *this;
//}
//
// // Interval step2 - step1
//FPitch FPitchStepsInterval(int nStep1, int nStep2, lmEKeySignatures nKey)
//{
//    int nAccidentals[7];
//    lmComputeAccidentals(nKey, nAccidentals);
//    int nOctaveInStep2 = 0;
//    if (nStep1 > nStep2) // the step2 must always be higher
//        nOctaveInStep2 = 1;
//
//    FPitch fVS2 = FPitch(nStep2, nOctaveInStep2, nAccidentals[nStep2]);
//    FPitch fVS1 = FPitch(nStep1, 0, nAccidentals[nStep1]);
//
//    wxLogMessage("  FPitchStepsInterval  (Step %d oct:%d) %d - (Step %d, octave 0) %d = %d")
//         , nStep2, nOctaveInStep2, fVS2, nStep1, fVS1, fVS2-fVS1);
//    return fVS2 - fVS1;
//}
//
//// return the note letter (A .. G) corresponding to the step of the note, in FPitch notation
//std::string FPitch_GetEnglishNoteName(FPitch fp)
//{
//    return m_sNoteName[FPitch_Step(fp)].c_str();
//}
//
////-------------------------------------------------------------------------------------
//// implementation of class AbsolutePitch: Absoulte pitch
////-------------------------------------------------------------------------------------
//
//AbsolutePitch::AbsolutePitch(const std::string& sNote)
//{
//    // sNote must be letter followed by a number (i.e.: "c4" ) optionally precedeed by
//    // accidentals (i.e.: "++c4")
//    // It is assumed that sNote is trimmed (no spaces before or after data)
//    // and lower case.
//
//    //split the string: accidentals and name
//    std::string sAccidentals;
//    int iStepPos;
//    switch (sNote.length()) {
//        case 2:
//            sAccidentals = "";
//            iStepPos = 0;
//            break;
//        case 3:
//            sAccidentals = sNote.substr(0, 1);
//            iStepPos = 1;
//            break;
//        case 4:
//            sAccidentals = sNote.substr(0, 2);
//            iStepPos = 2;
//            break;
//        default:
//            wxLogMessage("[AbsolutePitch constructor] Bad note name '%s'", sNote.c_str());
//            m_nDiatonicPitch = lmC4_DPITCH;
//            m_nAcc = lmNO_ACCIDENTAL;
//            return;
//    }
//
//    //compute step
//    wxChar sStep = sNote.at(iStepPos);
//    int nStep;
//    if (sStep == _T('c')) nStep = 0;
//    else if (sStep == _T('d')) nStep =  1;
//    else if (sStep == _T('e')) nStep =  2;
//    else if (sStep == _T('f')) nStep =  3;
//    else if (sStep == _T('g')) nStep =  4;
//    else if (sStep == _T('a')) nStep =  5;
//    else if (sStep == _T('b')) nStep =  6;
//    else {
//        wxLogMessage("[AbsolutePitch constructor] Bad note name '%s'", sNote.c_str());
//        m_nDiatonicPitch = lmC4_DPITCH;
//        m_nAcc = lmNO_ACCIDENTAL;
//        return;
//    }
//
//    //compute octave
//    std::string sOctave = sNote.substr(1, 1);
//    long nOctave;
//    if (!sOctave.ToLong(&nOctave)) {
//        wxLogMessage("[AbsolutePitch constructor] Bad note name '%s'", sNote.c_str());
//        m_nDiatonicPitch = lmC4_DPITCH;
//        m_nAcc = lmNO_ACCIDENTAL;
//        return;
//    }
//
//    //compute accidentals
//    int nAccidentals;
//    if (sAccidentals == ""))          nAccidentals = 0;
//    else if (sAccidentals == "-"))    nAccidentals = -1;
//    else if (sAccidentals == "--"))   nAccidentals = -2;
//    else if (sAccidentals == "+"))    nAccidentals = 1;
//    else if (sAccidentals == "++"))   nAccidentals = 2;
//    else if (sAccidentals == "x"))    nAccidentals = 2;
//    else {
//        wxLogMessage("[AbsolutePitch constructor] Bad note name '%s'", sNote.c_str());
//        m_nDiatonicPitch = lmC4_DPITCH;
//        m_nAcc = lmNO_ACCIDENTAL;
//        return;
//    }
//
//    m_nDiatonicPitch = (int)nOctave * 7 + nStep + 1;;
//    m_nAcc = nAccidentals;
//
//}
//
//std::string AbsolutePitch::LDPName() const
//{
//    std::string sAnswer;
//    switch(m_nAcc) {
//        case -2: sAnswer ="--"; break;
//        case -1: sAnswer ="-"; break;
//        case 0:  sAnswer =""; break;
//        case 1:  sAnswer ="+"; break;
//        case 2:  sAnswer ="++"; break;
//        default:
//            sAnswer = "";
//    }
//    sAnswer += m_sNoteName[(m_nDiatonicPitch - 1) % 7];
//    sAnswer += std::string::Format("%d", (m_nDiatonicPitch - 1) / 7 );
//    return sAnswer;
//}
//
//MidiPitch AbsolutePitch::GetMidiPitch() const
//{
//    int nOctave = Octave() + 1;
//    wxASSERT(lmC4_DPITCH == 29);    //AWARE It's assumed that we start in C0
//    MidiPitch nMidi = (MidiPitch)(nOctave * 12);
//
//    switch(m_nDiatonicPitch % 7)
//    {
//        case 0:  //si
//            nMidi = nMidi + 11;
//            break;
//        case 1:  //do
//            //nothing to add. The value is ok
//            break;
//        case 2:  //re
//            nMidi = nMidi + 2;
//            break;
//        case 3:  //mi
//            nMidi = nMidi + 4;
//            break;
//        case 4:  //fa
//            nMidi = nMidi + 5;
//            break;
//        case 5:  //sol
//            nMidi = nMidi + 7;
//            break;
//        case 6:  //la
//            nMidi = nMidi + 9;
//            break;
//    }
//
//    return nMidi + m_nAcc;
//
//}

//=======================================================================================
// DiatonicPitch implementation
//=======================================================================================
DiatonicPitch::DiatonicPitch(int step, int octave)
{
    if (step != k_no_pitch)
        m_dp = octave * 7 + step + 1;
    else
        m_dp = LOMSE_NO_DPITCH;
}

//MidiPitch DiatonicPitch::to_midi_pitch()
//{
//    int nOctave = (m_dp - 1) / 7;
//    int nRemainder = m_dp % 7;
//
//    int nMidiPitch = nOctave * 12;
//    switch (nRemainder) {
//        case 0:  //si
//            nMidiPitch = nMidiPitch + 11;   break;
//        case 1:  //do
//            //do nothing. Value is OK
//            break;
//        case 2:  //re
//            nMidiPitch += 2;   break;
//        case 3:  //mi
//            nMidiPitch += 4;   break;
//        case 4:  //fa
//            nMidiPitch += 5;   break;
//        case 5:  //sol
//            nMidiPitch += 7;   break;
//        case 6:  //la
//            nMidiPitch += 9;   break;
//    }
//    nMidiPitch += 12;
//
//    return MidiPitch(nMidiPitch);
//
//}
//
//std::string DiatonicPitch::to:ldp_name()
//{
//    // Returns the LDP note name that corresponds to the received pitch. For example,
//    // pitch 29 will return "c4".
//
//    int nOctave = octave();
//    int nStep = step();
//    return m_sNoteName[nStep] + std::string::Format("%d", nOctave);
//}
//
//std::string DiatonicPitch::get_english_note_name()
//{
//    int iNota = (m_dp - 1) % 7;
//    int nOctava = (m_dp - 1) / 7;
//    return std::string::Format("%s%d", m_sNoteName[iNota].c_str(), nOctava );
//}
//
//FPitch DiatonicPitch::to_FPitch(lmEKeySignatures nKey)
//{
//    // Get the accidentals implied by the key signature.
//    // Each element of the array refers to one note: 0=Do, 1=Re, 2=Mi, 3=Fa, ... , 6=Si
//    // and its value can be one of: 0=no accidental, -1 = a flat, 1 = a sharp
//    int nAccidentals[7];
//    lmComputeAccidentals(nKey, nAccidentals);
//    int nStep = step();
//    return FPitch(nStep, octave(), nAccidentals[nStep]);
//}
//
//
////---------------------------------------------------------------------------------------
//// MidiPitch implementation
////---------------------------------------------------------------------------------------
//MidiPitch::MidiPitch(int step, int octave, int acc)
//{
//    if (step != k_unpitched)
//    {
//			int stepToPitch [] = { 0, 2, 4, 5, 7, 9, 11 };
//			m_pitch = (octave * 12) + stepToPitch[step] + acc;
//	}
//    m_pitch = -1;
//}
//
//bool MidiPitch::is_natural_note(lmEKeySignatures nKey)
//{
//    // Returns true if the Midi note is natural for the key signature scale
//
//    //Prepare string with "1" in natural tones of the scale
//    std::string sScale;
//    switch (nKey) {
//        case earmDo:
//        case earmLam:
//            //        C D EF G A B
//            sScale = "101011010101";   break;
//
//        //Sostenidos ---------------------------------------
//        case earmSol:
//        case earmMim:
//            //        C D EF G A B
//            sScale = "101010110101";   break;
//        case earmRe:
//        case earmSim:
//            //        C D EF G A B
//            sScale = "011010110101";   break;
//        case earmLa:
//        case earmFasm:
//            //        C D EF G A B
//            sScale = "011010101101";   break;
//        case earmMi:
//        case earmDosm:
//            //        C D EF G A B
//            sScale = "010110101101";   break;
//        case earmSi:
//        case earmSolsm:
//            //        C D EF G A B
//            sScale = "010110101011";   break;
//        case earmFas:
//        case earmResm:
//            //           C D EF G A B
//            sScale = "010101101011";   break;
//        case earmDos:
//        case earmLasm:
//            //        C D EF G A B
//            sScale = "110101101010";   break;
//
//        //Bemoles -------------------------------------------
//        case earmFa:
//        case earmRem:
//            //        C D EF G A B
//            sScale = "101011010110";   break;
//        case earmSib:
//        case earmSolm:
//            //        C D EF G A B
//            sScale = "101101010110";   break;
//        case earmMib:
//        case earmDom:
//            //        C D EF G A B
//            sScale = "101101011010";   break;
//        case earmLab:
//        case earmFam:
//            //        C D EF G A B
//            sScale = "110101011010";   break;
//        case earmReb:
//        case earmSibm:
//            //           C D EF G A B
//            sScale = "110101101010";   break;
//        case earmSolb:
//        case earmMibm:
//            //        C D EF G A B
//            sScale = "010101101011";   break;
//        case earmDob:
//        case earmLabm:
//            //        C D EF G A B
//            sScale = "010110101011";   break;
//        default:
//            wxASSERT(false);
//    }
//
//    int nRemainder = m_pitch % 12;      //nRemainder goes from 0 (Do) to 11 (Si)
//    return (sScale.substr(nRemainder, 1) == "1"));
//
//}
//
//std::string MidiPitch::to_ldp_name()
//{
//    // Returns the LDP diatonic pitch that corresponds to the received MIDI pitch.
//    // AWARE: It is assumed C major key signature.
//
//    int nOctave = (m_pitch - 12) / 12;
//    int nRemainder = m_pitch % 12;
//    std::string sNote[] = {"c", "+c", "d", "+d", "e", "f", "+f",
//                        "g", "+g", "a", "+a", "b") };
//    return sNote[nRemainder] + std::string::Format("%d", nOctave);
//
//}


} //namespace lomse
