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

#include "lomse_score_utilities.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"

//other
#include <boost/format.hpp>
#include <cmath>                //for fabs

using namespace std;

namespace lomse
{

//=======================================================================================
// global functions related to TimeSignatures
//=======================================================================================

//---------------------------------------------------------------------------------------
int get_beat_position(float timePos, ImoTimeSignature* pTS)
{
    // Some times it is necessary to know the type of beak (strong, medium, weak,
    // off-beat) at which a note or rest is positioned.
    // This function receives the time for a note/rest and the current time signature
    // and returns the type of beat: either an integer positive value 0..n, meaning
    // 'on-beat', where n is the beat number, or -1 meaning 'off-beat'

    int beatType = pTS->get_beat_type();

    // coumpute beat duration
    int beatDuration;
    switch (beatType)
    {
        case 1: beatDuration = int( to_duration(k_whole, 0) ); break;
        case 2: beatDuration = int( to_duration(k_half, 0) ); break;
        case 4: beatDuration = int( to_duration(k_quarter, 0) ); break;
        case 8: beatDuration = 3 * int( to_duration(k_eighth, 0) ); break;
        case 16: beatDuration = int( to_duration(k_eighth, 0) ); break;
        default:
            string msg = str( boost::format("[get_beat_position] BeatType %d unknown.")
                              % beatType );
            throw std::runtime_error(msg);
    }

    // compute relative position of this note/rest with reference to the beat
    int beatNum = int(timePos) / beatDuration;               //number of beat
    float beatShift = fabs(timePos - float(beatDuration * beatNum));

    if (beatShift < 1.0f)
        //on-beat
        return beatNum;
    else
        // off-beat
        return k_off_beat;

}



//int lmGetNumPulsesForTimeSignature(lmETimeSignature nTimeSign)
//{
//    //returns the number of pulses (metronome pulses) implied by the received
//    //time signature
//
//    switch (nTimeSign) {
//        case emtr24:
//            return 2;
//        case emtr34:
//            return 3;
//        case emtr44:
//            return 4;
//        case emtr28:
//            return 2;
//        case emtr38:
//            return 3;
//        case emtr22:
//            return 2;
//        case emtr32:
//            return 3;
//        case emtr68:
//            return 2;
//        case emtr98:
//            return 3;
//        case emtr128:
//            return 4;
//        default:
//            wxASSERT(false);
//            return 4;
//    }
//}
//
//int lmGetNumBeatsFromTimeSignType(lmETimeSignature nTimeSign)
//{
//    //returns the numerator of time signature fraction
//
//    switch (nTimeSign) {
//        case emtr24:
//            return 2;
//        case emtr34:
//            return 3;
//        case emtr44:
//            return 4;
//        case emtr28:
//            return 2;
//        case emtr38:
//            return 3;
//        case emtr22:
//            return 2;
//        case emtr32:
//            return 3;
//        case emtr68:
//            return 6;
//        case emtr98:
//            return 9;
//        case emtr128:
//            return 12;
//        default:
//            wxASSERT(false);
//            return 4;
//    }
//}
//
//int GetBeatTypeFromTimeSignType(lmETimeSignature nTimeSign)
//{
//    switch (nTimeSign) {
//        case emtr24:
//        case emtr34:
//        case emtr44:
//            return 4;
//
//        case emtr28:
//        case emtr38:
//        case emtr68:
//        case emtr98:
//        case emtr128:
//            return 8;
//
//        case emtr22:
//        case emtr32:
//            return 2;
//
//        default:
//            wxASSERT(false);
//            return 4;
//    }
//}
//
//float GetBeatDuration(lmETimeSignature nTimeSign)
//{
//    // returns beat duration (in LDP notes duration units)
//
//    int nBeatType = GetBeatTypeFromTimeSignType(nTimeSign);
//    return GetBeatDuration(nBeatType);
//}
//
//float GetBeatDuration(int nBeatType)
//{
//    // returns beat duration (in LDP notes duration units)
//
//    switch(nBeatType) {
//        case 1:
//            return pow(2.0f, (10 - ImoNoteRest::k_whole));
//        case 2:
//            return pow(2.0f, (10 - ImoNoteRest::k_half));
//        case 4:
//            return pow(2.0f, (10 - ImoNoteRest::k_quarter));
//        case 8:
//            return pow(2.0f, (10 - ImoNoteRest::k_eighth));
//        case 16:
//            return pow(2.0f, (10 - ImoNoteRest::k_16th));
//        default:
//            wxASSERT(false);
//            return 0;     //compiler happy
//    }
//}
//
//float GetMeasureDuration(lmETimeSignature nTimeSign)
//{
//    // Returns the required duration for a measure in the received time signature
//
//    float rNumBeats = (float)lmGetNumBeatsFromTimeSignType(nTimeSign);
//    return rNumBeats * GetBeatDuration(nTimeSign);
//}
//
//int GetBeatPosition(float rTimePos, float rDuration, int nBeats, int nBeatType)
//{
//    // Some times it is necessary to know if a note/rest sounds in beat part.
//    // This method receives the time for a note/rest and the current time signature
//    // and returns the beat number if the note sounds in beat part (starts in beat or
//    // is sounding at beat time, or returns -1 (lmNOT_ON_BEAT) if non-chord note
//
//    // coumpute beat duration
//    int nBeatDuration;
//    switch (nBeatType) {
//        case 1: nBeatDuration = (int)eWholeDuration; break;
//        case 2: nBeatDuration = (int)eHalfDuration; break;
//        case 4: nBeatDuration = (int)eQuarterDuration; break;
//        case 8: nBeatDuration = 3* (int)eEighthDuration; break;
//        case 16: nBeatDuration = (int)e16thDuration; break;
//        default:
//            wxLogMessage(_T("[GetPositionBeatType] BeatType %d unknown."), nBeatType);
//            wxASSERT(false);
//    }
//
//    // compute start and end time
//    // Duration: quarter = 64, eighth = 32
//    // Example. 3/4 time, quarter note starting on third beat:
//    //      rTimePos= 128
//    //      nStartBeat = 128 / 64 = 2
//    //      rStartShift = 128 - 2*64 = 0  --> starts on beat
//    //
//    // Example. 3/4 time, off-baet quarter note starting before third beat (rTimePos=96)
//    //      rTimePos = 64+32= 96
//    //      nStartBeat = 96/64 = 1
//    //      rStartShift = 96 - 1*64 = 32 ==> Doesn't start on beat
//    //      rEndTimePos = 96+64 = 160
//    //      nEndBeat = 160/64 = 2
//    //      rEndShift = 160 - 64*2 = 160-128 = 32
//
//    // compute relative position of this note/rest with reference to the beat
//    int nStartBeat = (int)rTimePos / nBeatDuration;               //start beat number (minus 1)
//    float rStartShift = fabs(rTimePos - (float)(nBeatDuration * nStartBeat));
//
//    float rEndTimePos = rTimePos + rDuration;
//    int nEndBeat = (int)rEndTimePos /nBeatDuration;
//    float rEndShift = fabs(rEndTimePos - (float)(nBeatDuration * nEndBeat));
//
//    // note is on chord if it starts on beat or start point on beat N and end point on beat N+1.
//    // 1.0 is the duration of a 256th note. I use 1.0 instead of e256thDuration to avoid
//    // conversions
//    if (rStartShift < 1.0 )
//        return nStartBeat;
//    else if (nStartBeat != nEndBeat && rEndShift > 1.0)
//        return nStartBeat+1;    //AWARE: The note might last for many beats. So nEndBeat is
//                                // not the right answer.
//    else
//        return lmNOT_ON_BEAT;
//
//}






//=======================================================================================
// global functions related to key signatures
//=======================================================================================

void get_accidentals_for_key(int keyType, int nAccidentals[])
{
    // Given a key signature (keyType) this function fills the array
    // nAccidentals with the accidentals implied by the key signature.
    // Each element of the array refers to one note: 0=Do, 1=Re, 2=Mi, 3=Fa, ... , 6=Si
    // & its value can be one of:
    //     0  = no accidental
    //    -1  = a flat
    //     1  = a sharp

    // initialize array: no accidentals
    for (int i=0; i < 7; i++) {
        nAccidentals[i] = 0;
    }

    // accidentals implied by the key signature
    switch (keyType) {
        case k_key_C:
        case k_key_a:
            //no accidentals
            break;

        //Sharps ---------------------------------------
        case k_key_G:
        case k_key_e:
            nAccidentals[3] = 1;     //Fa #
            break;
        case k_key_D:
        case k_key_b:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            break;
        case k_key_A:
        case k_key_fs:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
             break;
       case k_key_E:
        case k_key_cs:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            break;
        case k_key_B:
        case k_key_gs:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            nAccidentals[5] = 1;     //La #
            break;
        case k_key_Fs:
        case k_key_ds:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            nAccidentals[5] = 1;     //La #
            nAccidentals[2] = 1;     //Mi #
            break;
        case k_key_Cs:
        case k_key_as:
            nAccidentals[3] = 1;     //Fa #
            nAccidentals[0] = 1;     //Do #
            nAccidentals[4] = 1;     //Sol #
            nAccidentals[1] = 1;     //Re #
            nAccidentals[5] = 1;     //La #
            nAccidentals[2] = 1;     //Mi #
            nAccidentals[6] = 1;     //Si #
             break;

        //Flats -------------------------------------------
        case k_key_F:
        case k_key_d:
            nAccidentals[6] = -1;         //Si b
            break;
        case k_key_Bf:
        case k_key_g:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
             break;
       case k_key_Ef:
        case k_key_c:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            break;
        case k_key_Af:
        case k_key_f:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
             break;
        case k_key_Df:
        case k_key_bf:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
            nAccidentals[4] = -1;         //Sol b
            break;
        case k_key_Gf:
        case k_key_ef:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
            nAccidentals[4] = -1;         //Sol b
            nAccidentals[0] = -1;         //Do b
            break;
        case k_key_Cf:
        case k_key_af:
            nAccidentals[6] = -1;         //Si b
            nAccidentals[2] = -1;         //Mi b
            nAccidentals[5] = -1;         //La b
            nAccidentals[1] = -1;         //Re b
            nAccidentals[4] = -1;         //Sol b
            nAccidentals[0] = -1;         //Do b
            nAccidentals[3] = -1;         //Fa b
            break;
        default:
            string msg = str( boost::format(
                                "[get_accidentals_for_key] Invalid key signature %d")
                                % keyType );
            throw std::runtime_error(msg);
    }

}

//int lmGetRootNoteStep(lmEKeySignatures keyType)
//{
//    //returns the stpe (0..6, 0=Do, 1=Re, 3=Mi, ... , 6=Si) for the root note in
//    //the Key signature. For example, if keyType is La sharp minor it returns
//    //step = 5 (La)
//
//    //compute root note
//    int nRootNote;
//    switch(keyType) {
//        case k_key_C:
//        case k_key_c:
//        case k_key_cs:
//        case k_key_Cs:
//        case k_key_Cf:
//            nRootNote = 0;
//            break;
//        case k_key_D:
//        case k_key_Df:
//        case k_key_ds:
//        case k_key_d:
//            nRootNote = 1;
//            break;
//        case k_key_E:
//        case k_key_e:
//        case k_key_Ef:
//        case k_key_ef:
//            nRootNote = 2;
//            break;
//        case k_key_F:
//        case k_key_fs:
//        case k_key_Fs:
//        case k_key_f:
//            nRootNote = 3;
//            break;
//        case k_key_G:
//        case k_key_gs:
//        case k_key_g:
//        case k_key_Gf:
//            nRootNote = 4;
//            break;
//        case k_key_A:
//        case k_key_a:
//        case k_key_as:
//        case k_key_Af:
//        case k_key_af:
//            nRootNote = 5;
//            break;
//        case k_key_b:
//        case k_key_B:
//        case k_key_Bf:
//        case k_key_bf:
//            nRootNote = 6;
//            break;
//        default:
//            wxASSERT(false);
//    }
//
//    return nRootNote;
//
//}
//
//bool lmIsMajorKey(lmEKeySignatures keyType)
//{
//    return (keyType < k_key_a);
//}
//
//bool lmIsMinorKey(lmEKeySignatures keyType)
//{
//    return (keyType >= k_key_a);
//}
//
//const wxString& lmGetKeySignatureName(lmEKeySignatures keyType)
//{
//    static bool fStringsLoaded = false;
//
//    if (!fStringsLoaded)
//    {
//        //language dependent strings. Can not be statically initiallized because
//        //then they do not get translated
//        m_sKeySignatureName[0] = _("C Major");
//        m_sKeySignatureName[1] = _("G Major");
//        m_sKeySignatureName[2] = _("D Major");
//        m_sKeySignatureName[3] = _("A Major");
//        m_sKeySignatureName[4] = _("E Major");
//        m_sKeySignatureName[5] = _("B Major");
//        m_sKeySignatureName[6] = _("F # Major");
//        m_sKeySignatureName[7] = _("C # Major");
//        m_sKeySignatureName[8] = _("C b Major");
//        m_sKeySignatureName[9] = _("G b Major");
//        m_sKeySignatureName[10] = _("D b Major");
//        m_sKeySignatureName[11] = _("A b Major");
//        m_sKeySignatureName[12] = _("E b Major");
//        m_sKeySignatureName[13] = _("B b Major");
//        m_sKeySignatureName[14] = _("F Major");
//        m_sKeySignatureName[15] = _("A minor");
//        m_sKeySignatureName[16] = _("E minor");
//        m_sKeySignatureName[17] = _("B minor");
//        m_sKeySignatureName[18] = _("F # minor");
//        m_sKeySignatureName[19] = _("C # minor");
//        m_sKeySignatureName[20] = _("G # minor");
//        m_sKeySignatureName[21] = _("D # minor");
//        m_sKeySignatureName[22] = _("A # minor");
//        m_sKeySignatureName[23] = _("A b minor");
//        m_sKeySignatureName[24] = _("E b minor");
//        m_sKeySignatureName[25] = _("B b minor");
//        m_sKeySignatureName[26] = _("F minor");
//        m_sKeySignatureName[27] = _("C minor");
//        m_sKeySignatureName[28] = _("G minor");
//        m_sKeySignatureName[29] = _("D minor");
//        fStringsLoaded = true;
//    }
//
//    return m_sKeySignatureName[keyType - lmMIN_KEY];
//}
//
//int KeySignatureToNumFifths(lmEKeySignatures keyType)
//{
//    // Retunrs the number of fifths that corresponds to the encoded key signature
//
//    int nFifths = 0;        //num accidentals to draw (0..7)
//    switch(keyType) {
//        case k_key_C:
//        case k_key_a:
//            nFifths = 0;
//            break;
//
//        //Sharps ---------------------------------------
//        case k_key_G:
//        case k_key_e:
//            nFifths = 1;
//            break;
//        case k_key_D:
//        case k_key_b:
//            nFifths = 2;
//            break;
//        case k_key_A:
//        case k_key_fs:
//            nFifths = 3;
//            break;
//        case k_key_E:
//        case k_key_cs:
//            nFifths = 4;
//            break;
//        case k_key_B:
//        case k_key_gs:
//            nFifths = 5;
//            break;
//        case k_key_Fs:
//        case k_key_ds:
//            nFifths = 6;
//            break;
//        case k_key_Cs:
//        case k_key_as:
//            nFifths = 7;
//            break;
//
//        //Flats -------------------------------------------
//        case k_key_F:
//        case k_key_d:
//            nFifths = -1;
//            break;
//        case k_key_Bf:
//        case k_key_g:
//            nFifths = -2;
//            break;
//        case k_key_Ef:
//        case k_key_c:
//            nFifths = -3;
//            break;
//        case k_key_Af:
//        case k_key_f:
//            nFifths = -4;
//            break;
//        case k_key_Df:
//        case k_key_bf:
//            nFifths = -5;
//            break;
//        case k_key_Gf:
//        case k_key_ef:
//            nFifths = -6;
//            break;
//        case k_key_Cf:
//        case k_key_af:
//            nFifths = -7;
//            break;
//        default:
//            wxASSERT(false);
//    }
//    return nFifths;
//
//}
//
//lmEKeySignatures lmGetRelativeMinorKey(lmEKeySignatures nMajorKey)
//{
//    switch(nMajorKey) {
//        case k_key_C:
//            return k_key_a;
//        case k_key_G:
//            return k_key_e;
//        case k_key_D:
//            return k_key_b;
//        case k_key_A:
//            return k_key_fs;
//        case k_key_E:
//            return k_key_cs;
//        case k_key_B:
//            return k_key_gs;
//        case k_key_Fs:
//            return k_key_ds;
//        case k_key_Cs:
//            return k_key_as;
//        case k_key_F:
//            return k_key_d;
//        case k_key_Bf:
//            return k_key_g;
//        case k_key_Ef:
//            return k_key_c;
//        case k_key_Af:
//            return k_key_f;
//        case k_key_Df:
//            return k_key_bf;
//        case k_key_Gf:
//            return k_key_ef;
//        case k_key_Cf:
//            return k_key_af;
//        default:
//            wxASSERT(false);
//            return k_key_c;
//    }
//
//}
//
//lmEKeySignatures lmGetRelativeMajorKey(lmEKeySignatures nMinorKey)
//{
//    switch(nMinorKey) {
//        case k_key_a:
//            return k_key_C;
//        case k_key_e:
//            return k_key_G;
//        case k_key_b:
//            return k_key_D;
//        case k_key_fs:
//            return k_key_A;
//        case k_key_cs:
//            return k_key_E;
//        case k_key_gs:
//            return k_key_B;
//        case k_key_ds:
//            return k_key_Fs;
//        case k_key_as:
//            return k_key_Cs;
//        case k_key_d:
//            return k_key_F;
//        case k_key_g:
//            return k_key_Bf;
//        case k_key_c:
//            return k_key_Ef;
//        case k_key_f:
//            return k_key_Af;
//        case k_key_bf:
//            return k_key_Df;
//        case k_key_ef:
//            return k_key_Gf;
//        case k_key_af:
//            return k_key_Cf;
//        default:
//            wxASSERT(false);
//            return k_key_c;
//    }
//
//}
//
//wxString GetKeyLDPNameFromType(lmEKeySignatures keyType)
//{
//    return m_sLDPKeyName[keyType];
//}

}  //namespace lomse
