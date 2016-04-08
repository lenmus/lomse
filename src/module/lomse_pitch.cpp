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

#include "lomse_pitch.h"
#include "lomse_score_utilities.h"
#include "lomse_ldp_analyser.h"
#include "lomse_logger.h"

#include <sstream>
using namespace std;

namespace lomse
{


static const string m_sNoteName[7] = { "c",  "d", "e", "f", "g", "a", "b" };
static const string m_sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                      "7", "8", "9", "10", "11", "12"  };


//=======================================================================================
// implementation of global methods
//=======================================================================================
int accidentals_to_number(EAccidentals accidentals)
{
    switch(accidentals)
    {
        case k_invalid_accidentals:     return 0;   break;
        case k_no_accidentals:          return 0;   break;
        case k_sharp:                   return +1;  break;
        case k_sharp_sharp:             return +2;  break;
        case k_double_sharp:            return +2;  break;
        case k_natural_sharp:           return +1;  break;
        case k_flat:                    return -1;  break;
        case k_flat_flat:               return -2;  break;
        case k_natural_flat:            return -1;  break;
        case k_natural:                 return 0;   break;
        default:                        return 0;   break;
    }
}


//=======================================================================================
// implementation of FPitch: Absolute pitch, interval=invariant (base=40)
//=======================================================================================
FPitch::FPitch(DiatonicPitch dp, int nAcc)
{
    create(dp.step(), dp.octave(), nAcc);
}

////---------------------------------------------------------------------------------------
//FPitch::FPitch(AbsolutePitch ap)
//{
//    create(ap.step(), ap.octave(), ap.accidentals());
//}

//---------------------------------------------------------------------------------------
FPitch::FPitch(int nStep, int nOctave, int nAcc)
{
    create(nStep, nOctave, nAcc);
}

//---------------------------------------------------------------------------------------
void FPitch::create(int nStep, int nOctave, int nAcc)
{
    //  Cbb Cb  C   C#  C## -   Dbb Db  D   D#  D## -   Ebb Eb  E   E#  E## Fbb Fb  F   F#  F## -
    //  1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23
    //
    //  Gbb Gb  G   G#  G## -   Abb Ab  A   A#  A## -   Bbb Bb  B   B#  B##
    //  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40
    //
    //  Pitch generation:
    //  Note:       C=3, D=9, E=15, F=20, G=26, A=32, B=38
    //  Diff:           6    6     5     6     6    6     5
    //  Step:         0    1    2     3     4     5     6
    //  step*6+2:     2    8    14    20    26    32    38
    //  if s<3  ++    3    9    15    20    26    32    38
    //  +=Acc
    //
    //  Step extraction:
    //  Note:       C=1-5, D=7-11, E=13-17, F=18-22, G=24-28, A=30-34, B=36-40
    //  Step:         0    1       2        3        4        5        6
    //  n/6:          0-0  1-1     2-2      3-3      4-4      5-5      6-6
    //
    //  Octave extraction:
    //  Note (n):     0=1-40, 1=41-80, 2=81-120, m=(m*40)+1 - (m+1)*40
    //  (n-1)/40:     0       2        3         m
    //
    //  Accidentals extraction
    //  Note (n):        0=1-40, 1=41-80, 2=81-120, m=(m*40)+1 - (m+1)*40
    //  x=((n-1)%40)+1:  1-40    1-40     1-40      1-40
    //  step=n/6
    //  pitch approx. = step*6+2
    //  acc = x - pitch
    //  if step<3 acc--

    int fpPitch = (nOctave * 40) + (nStep * 6) + 2;
    if (nStep < 3) fpPitch++;
    m_fp = fpPitch + nAcc;

}

////---------------------------------------------------------------------------------------
//FPitch::FPitch(int nStep, int nOctave, EKeySignature nKey)
//{
//    int nAccidentals[7];
//    get_accidentals_for_key(nKey, nAccidentals);
//
//    create(nStep, nOctave, nAccidentals[nStep]);
//}

//---------------------------------------------------------------------------------------
bool FPitch::is_valid()
{
    //returns false for invalid pitches (6, 12, 23, 29 & 35)

    int x = ((m_fp - 1) % 40 ) + 1;
    return !(x == 6 || x == 12 || x == 23 || x == 29 || x == 35);
}

//---------------------------------------------------------------------------------------
FPitch::FPitch(const string& note)
{
    // 'note' must be a note name in ldp format: a letter followed by a number
    // (i.e.: "c4" ) optionally precedeed by accidentals (i.e.: "++c4")
    // It is assumed that 'note' is trimmed (no spaces before or after data)
    // and lower case.

    int step, octave;
    EAccidentals accidentals;
    if (LdpAnalyser::ldp_pitch_to_components(note, &step, &octave, &accidentals))
    {
        LOMSE_LOG_ERROR(note);
        create(k_step_C, k_octave_4, 0);        //error
    }
    else
    {
        int acc = accidentals_to_number(accidentals);
        create(step, octave, acc);
    }
}

//---------------------------------------------------------------------------------------
int FPitch::num_accidentals()
{
    //  Accidentals extraction
    //  Note (n):        0=1-40, 1=41-80, 2=81-120, m=(m*40)+1 - (m+1)*40
    //  x=((n-1)%40)+1:  1-40    1-40     1-40      1-40
    //  step=x/6
    //  compute pitch
    //  acc = x - pitch

    int nReduction = ((m_fp - 1) % 40 ) + 1;
    int nStep = nReduction / 6;
    int nPitch = (nStep * 6) + 2;
    if (nStep < 3) nPitch++;
    return nReduction - nPitch;
}

//---------------------------------------------------------------------------------------
EAccidentals FPitch::accidentals()
{
    int acc = num_accidentals();
    switch(acc)
    {
        case -2:    return k_flat_flat;
        case -1:    return k_flat;
        case 0:     return k_no_accidentals;
        case 1:     return k_sharp;
        case 2:     return k_double_sharp;
        default:    return k_invalid_accidentals;
    }
}

//---------------------------------------------------------------------------------------
string FPitch::to_abs_ldp_name()
{
    // The absolute LDP none name is returned
    // If note is invalid (more than two accidentals) returns empty string

    string sAnswer;
    switch(num_accidentals()) {
        case -2: sAnswer ="--"; break;
        case -1: sAnswer ="-"; break;
        case 0:  sAnswer =""; break;
        case 1:  sAnswer ="+"; break;
        case 2:  sAnswer ="x"; break;
        default:
            return "";
    }
    sAnswer += m_sNoteName[step()];
    sAnswer += m_sOctave[octave()];
    return sAnswer;
}

//---------------------------------------------------------------------------------------
string FPitch::to_rel_ldp_name(EKeySignature nKey)
{
    // The note LDP name, relative to received key signature, is returned.
    // For instace F4 sharp in C major will return "+f4" but in D major
    // will return "f4" as the accidental is implied by the key signature

    if (m_fp < 0)
        return "*";

    // Get the accidentals implied by the key signature.
    // Each element of the array refers to one note: 0=Do, 1=Re, 2=Mi, 3=Fa, ... , 6=Si
    // and its value can be one of: 0=no accidental, -1 = a flat, 1 = a sharp
    int nAccidentals[7];
    get_accidentals_for_key(nKey, nAccidentals);

    //compute note accidentals
    string sAnswer;
    int nAbsAcc = num_accidentals();
    switch(nAbsAcc)
    {
        case -2: sAnswer ="--"; break;
        case -1: sAnswer ="-"; break;
        case 0:  sAnswer =""; break;
        case 1:  sAnswer ="+"; break;
        case 2:  sAnswer ="x"; break;
        default:
            sAnswer = "";
    }

    //change note accidentals to take key into account
    int nStep = step();
    if (nAccidentals[nStep] != 0)
    {
        if (nAbsAcc == nAccidentals[nStep])
            sAnswer = "";   //replace note accidental by key accidental
        else if (nAbsAcc == 0)
            sAnswer = "=";  //force a natural
        //else
            //leave note accidentals
    }

    // add step letter and octave number
    sAnswer += m_sNoteName[nStep];
    sAnswer += m_sOctave[octave()];

    return sAnswer;
}

//---------------------------------------------------------------------------------------
EAccidentals FPitch::notated_accidentals_for(EKeySignature key)
{
    int accidentals[7];
    get_accidentals_for_key(key, accidentals);

    int step = this->step();
    int keyAcc = accidentals[step];
    int numAcc = num_accidentals();
    if (keyAcc == 0)
    {
        switch(numAcc)
        {
            case -2:    return k_flat_flat;
            case -1:    return k_flat;
            case 0:     return k_no_accidentals;
            case 1:     return k_sharp;
            case 2:     return k_double_sharp;
            default:    return k_invalid_accidentals;
        }
    }

    else if (keyAcc == numAcc)
        return k_no_accidentals;

    else if (keyAcc == -2 && numAcc == -1)
        return k_natural_flat;

    else if (keyAcc == -1 && numAcc == 0)
        return k_natural;

    else if (keyAcc == 2 && numAcc == 1)
        return k_natural_sharp;

    else if (keyAcc == 1 && numAcc == 0)
        return k_natural;

    else
        return k_invalid_accidentals;
}

//---------------------------------------------------------------------------------------
MidiPitch FPitch::to_midi_pitch()
{
    MidiPitch nMidi = MidiPitch( (octave()+1) * 12 );

    switch( int(to_diatonic_pitch()) % 7)
    {
        case 0:  //si
            nMidi = nMidi + 11;
            break;
        case 1:  //do
            //nothing to add. The value is ok
            break;
        case 2:  //re
            nMidi = nMidi + 2;
            break;
        case 3:  //mi
            nMidi = nMidi + 4;
            break;
        case 4:  //fa
            nMidi = nMidi + 5;
            break;
        case 5:  //sol
            nMidi = nMidi + 7;
            break;
        case 6:  //la
            nMidi = nMidi + 9;
            break;
    }

    return nMidi + num_accidentals();

}

////---------------------------------------------------------------------------------------
DiatonicPitch FPitch::to_diatonic_pitch()
{
    return DiatonicPitch(step(), octave());
}

////---------------------------------------------------------------------------------------
//AbsolutePitch FPitch::to_APitch()
//{
//    return AbsolutePitch(to_DiatonicPitch(), num_accidentals());
//}
//
////---------------------------------------------------------------------------------------
FPitch FPitch::add_semitone(EKeySignature nKey)
{
    // This function adds one semitone returns the resulting pitch.
    // Step and accidentals of new note are adjusted to fit 'naturally' in the key
    // signature received: one accidental at maximum, of the same type than the
    // accidentals in the key signature.

    return add_semitone(key_signature_to_num_fifths(nKey) >= 0);
}

////---------------------------------------------------------------------------------------
FPitch FPitch::add_semitone(bool fUseSharps)
{
    // This function adds one semitone and returns the resulting pitch.
    // Step and accidentals of new note are adjusted to use one accidental at
    // maximum, of the same type as requested by fUseSharps

    // step 0   0   1   1   2   3   3   4   4   5   5   6
        //  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
        //  3   4   9   10  15  20  21  26  27  32  33  38

    // step 0   1   1   2   2   3   4   4   5   5   6   6
        //  C   Db  D   Eb  E   F   Gb  G   Ab  A   Bb  B
        //  3   8   9   14  15  20  25  26  31  32  37  38


    // extract components
    int nStep = step();
    int nAcc = num_accidentals();
    int nOctave = octave();

    //Determine what type of accidentals to use for semitones: flats or sharps
    if (fUseSharps)
    {
        //Key signature is using sharps.
        //Only one accidental allowed and must be a sharp
        if (nAcc == 0)
        {
            //if no accidentals add one sharp or advance step, depending on current step
            if (nStep == 2 || nStep == 6)
                nStep++;
            else
                nAcc++;
        }
        else
        {
            if (nAcc < 0)
                //assume one flat: advance natural step
                nAcc = 0;
            else
            {
                //assume one sharp: advance to next natural step
                nStep++;
                nAcc=0;
            }
        }

        // Adjust octave
        if (nStep==7)
        {
            nStep = 0;
            nOctave++;
        }
    }
    else
    {
        //Key signature is using flats.
        //Only one accidental allowed and must be a flat
        if (nAcc == 0)
        {
            //if no accidentals advance step and, depending on new step, add one flat
            nStep++;
            if (nStep == 7) {
                nStep = 0;
                nOctave++;
            }
            else if (nStep != 3)
                nAcc--;
        }
        else
        {
            //assume one flat: remove accidentals to advance to the natural step
            nAcc=0;
        }
    }

    create(nStep, nOctave, nAcc);
    return *this;
}

//---------------------------------------------------------------------------------------
bool FPitch::is_natural_note_for(EKeySignature nKey)
{
    int keyAccidentals[7];
    get_accidentals_for_key(nKey, keyAccidentals);
    int noteAcc = num_accidentals();
    return noteAcc == keyAccidentals[step()];
}

////---------------------------------------------------------------------------------------
// // Interval step2 - step1
//FPitch FPitchStepsInterval(int nStep1, int nStep2, EKeySignature nKey)
//{
//    int nAccidentals[7];
//    get_accidentals_for_key(nKey, nAccidentals);
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
////---------------------------------------------------------------------------------------
//// returns the note letter (a .. g) corresponding to the step of the note, in FPitch notation
//string FPitch_GetEnglishNoteName(FPitch fp)
//{
//    return m_sNoteName[fp.step()].c_str();
//}
//

////-------------------------------------------------------------------------------------
//// implementation of class AbsolutePitch: Absoulte pitch
////-------------------------------------------------------------------------------------
//
//AbsolutePitch::AbsolutePitch(const string& sNote)
//{
//    // sNote must be letter followed by a number (i.e.: "c4" ) optionally precedeed by
//    // accidentals (i.e.: "++c4")
//    // It is assumed that sNote is trimmed (no spaces before or after data)
//    // and lower case.
//
//    //split the string: accidentals and name
//    string sAccidentals;
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
//            m_nDiatonicPitch = C4_DPITCH;
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
//        m_nDiatonicPitch = C4_DPITCH;
//        m_nAcc = lmNO_ACCIDENTAL;
//        return;
//    }
//
//    //compute octave
//    string sOctave = sNote.substr(1, 1);
//    long nOctave;
//    if (!sOctave.ToLong(&nOctave)) {
//        wxLogMessage("[AbsolutePitch constructor] Bad note name '%s'", sNote.c_str());
//        m_nDiatonicPitch = C4_DPITCH;
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
//        m_nDiatonicPitch = C4_DPITCH;
//        m_nAcc = lmNO_ACCIDENTAL;
//        return;
//    }
//
//    m_nDiatonicPitch = (int)nOctave * 7 + nStep + 1;;
//    m_nAcc = nAccidentals;
//
//}
//
//string AbsolutePitch::LDPName() const
//{
//    string sAnswer;
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
//    sAnswer += string::Format("%d", (m_nDiatonicPitch - 1) / 7 );
//    return sAnswer;
//}
//
//MidiPitch AbsolutePitch::GetMidiPitch() const
//{
//    int nOctave = Octave() + 1;
//    wxASSERT(C4_DPITCH == 29);    //AWARE It's assumed that we start in C0
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
        m_dp = NO_DPITCH;
}

//---------------------------------------------------------------------------------------
MidiPitch DiatonicPitch::to_midi_pitch()
{
    int nOctave = (m_dp - 1) / 7;
    int nRemainder = m_dp % 7;

    int nMidiPitch = nOctave * 12;
    switch (nRemainder) {
        case 0:  //si
            nMidiPitch = nMidiPitch + 11;   break;
        case 1:  //do
            //do nothing. Value is OK
            break;
        case 2:  //re
            nMidiPitch += 2;   break;
        case 3:  //mi
            nMidiPitch += 4;   break;
        case 4:  //fa
            nMidiPitch += 5;   break;
        case 5:  //sol
            nMidiPitch += 7;   break;
        case 6:  //la
            nMidiPitch += 9;   break;
    }
    nMidiPitch += 12;

    return MidiPitch(nMidiPitch);

}

//---------------------------------------------------------------------------------------
string DiatonicPitch::get_ldp_name()
{
    // Returns the LDP note name (without accidentals). For example,
    // pitch 29 will return "c4".

    if (is_valid())
        return m_sNoteName[step()] + m_sOctave[octave()];
    else
    {
        LOMSE_LOG_ERROR("Operation on non-valid DiatonicPitch");
        return "*";
        //throw runtime_error("Operation on non-valid DiatonicPitch");
    }
}

//---------------------------------------------------------------------------------------
string DiatonicPitch::get_english_note_name()
{
    return get_ldp_name();
}

//---------------------------------------------------------------------------------------
FPitch DiatonicPitch::to_FPitch(EKeySignature nKey)
{
    // Get the accidentals implied by the key signature.

    // Each element of the array nAccidentals refers to one note: 0=Do, 1=Re,
    // 2=Mi, 3=Fa, ... , 6=Si and its value can be one of: 0=no accidental,
    // -1 = a flat, 1 = a sharp
    int nAccidentals[7];
    get_accidentals_for_key(nKey, nAccidentals);

    int nStep = step();
    return FPitch(nStep, octave(), nAccidentals[nStep]);
}


//=======================================================================================
// MidiPitch implementation
//=======================================================================================
MidiPitch::MidiPitch(int step, int octave, int acc)
{
    if (step != k_no_pitch)
    {
		int stepToPitch[] = { 0, 2, 4, 5, 7, 9, 11 };
		m_pitch = (octave+1) * 12 + stepToPitch[step] + acc;
	}
    else
        m_pitch = -1;
}

//---------------------------------------------------------------------------------------
string MidiPitch::get_ldp_name()
{
    // Returns the LDP diatonic pitch that corresponds to the received MIDI pitch.
    // AWARE: It is assumed C major key signature.

    int octave = (m_pitch - 12) / 12;
    int remainder = m_pitch % 12;
    static const string sNote[] = {
        "c", "+c", "d", "+d", "e", "f", "+f", "g", "+g", "a", "+a", "b" };
    return sNote[remainder] + m_sOctave[octave];

}

//---------------------------------------------------------------------------------------
bool MidiPitch::is_natural_note_for(EKeySignature nKey)
{
    // Returns true if the Midi note is natural for the key signature scale

    //Prepare string with "1" in natural tones of the scale
    string sScale;
    switch (nKey)
    {
        case k_key_C:
        case k_key_a:
            //        C D EF G A B
            sScale = "101011010101";   break;

        //sharps ---------------------------------------
        case k_key_G:
        case k_key_e:
            //        C D EF G A B
            sScale = "101010110101";   break;
        case k_key_D:
        case k_key_b:
            //        C D EF G A B
            sScale = "011010110101";   break;
        case k_key_A:
        case k_key_fs:
            //        C D EF G A B
            sScale = "011010101101";   break;
        case k_key_E:
        case k_key_cs:
            //        C D EF G A B
            sScale = "010110101101";   break;
        case k_key_B:
        case k_key_gs:
            //        C D EF G A B
            sScale = "010110101011";   break;
        case k_key_Fs:
        case k_key_ds:
            //        C D EF G A B
            sScale = "010101101011";   break;
        case k_key_Cs:
        case k_key_as:
            //        C D EF G A B
            sScale = "110101101010";   break;

        //flats -------------------------------------------
        case k_key_F:
        case k_key_d:
            //        C D EF G A B
            sScale = "101011010110";   break;
        case k_key_Bf:
        case k_key_g:
            //        C D EF G A B
            sScale = "101101010110";   break;
        case k_key_Ef:
        case k_key_c:
            //        C D EF G A B
            sScale = "101101011010";   break;
        case k_key_Af:
        case k_key_f:
            //        C D EF G A B
            sScale = "110101011010";   break;
        case k_key_Df:
        case k_key_bf:
            //        C D EF G A B
            sScale = "110101101010";   break;
        case k_key_Gf:
        case k_key_ef:
            //        C D EF G A B
            sScale = "010101101011";   break;
        case k_key_Cf:
        case k_key_af:
            //        C D EF G A B
            sScale = "010110101011";   break;
        default:
        {
            LOMSE_LOG_ERROR("[MidiPitch::is_natural_note_for]. Invalid key signature");
            throw runtime_error("[MidiPitch::is_natural_note_for]. Invalid key signature");
        }
    }

    int nRemainder = m_pitch % 12;      //nRemainder goes from 0 (Do) to 11 (Si)
    return (sScale.substr(nRemainder, 1) == "1");

}


} //namespace lomse
