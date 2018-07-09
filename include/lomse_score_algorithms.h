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

#ifndef __LOMSE_SCORE_ALGORITHMS_H__
#define __LOMSE_SCORE_ALGORITHMS_H__

#include "lomse_time.h"
#include "lomse_staffobjs_table.h"

#include <list>
using namespace std;


namespace lomse
{

//forward declarations
class ImoNoteRest;
class ImoNote;
class ImoScore;


enum EOverlaps
{                           //old note      ===========       overlap marked as ++++++
    k_overlap_at_end=0,     //new note           ++++++...
    k_overlap_full,         //new note   ...+++++++++++...
    k_overlap_at_start,     //new note   ...+++++++
};

//---------------------------------------------------------------------------------------
// OverlappedNoteRest. Helper struct with info about an overlapped note/rest
class OverlappedNoteRest
{
public:
    ImoNoteRest* pNR;
    int          type;
    TimeUnits    overlap;

    OverlappedNoteRest(ImoNoteRest* p, int t, TimeUnits d)
        : pNR(p), type(t), overlap(d) {}
    OverlappedNoteRest(ImoNoteRest* p)
        : pNR(p), type(-1), overlap(0.0) {}
};

//---------------------------------------------------------------------------------------
/** ScoreAlgorithms
    General static methods for dealing with the staffobjs collection
**/
class ScoreAlgorithms
{
protected:

public:
    ScoreAlgorithms() {}
    ~ScoreAlgorithms() {}

    /** Look for a note that can be tied (as end of tie) with pStartNote
    */
    static ImoNote* find_possible_end_of_tie(ColStaffObjs* pColStaffObjs,
                                             ImoNote* pStartNote);

    /** Look for applicable clef at specified timepos
        @param pScore Pointer to the score to wich all other parameters refer.
    */
    static int get_applicable_clef_for(ImoScore* pScore,
                                       int iInstr, int iStaff, TimeUnits time);

    /** Look for a note starting at specified timepos
        @param pScore Pointer to the score to wich all other parameters refer.
        @param instr
        @param voice
        @param time
    */
    static ImoNoteRest* find_noterest_at(ImoScore* pScore,
                                         int instr, int voice, TimeUnits time);

    /** Find all notes/rest that are in the range from the given timepos and
        given duration, and classify the overlap for each found note/rest.
        @param pScore Pointer to the score to wich all other parameters refer.
        @param instr
        @param voice
        @param time
        @param duration
    */
    static list<OverlappedNoteRest*>
            find_and_classify_overlapped_noterests_at(ImoScore* pScore,
                         int instr, int voice, TimeUnits time, TimeUnits duration);

    /** finds end timepos for given voice, lower or equal than maxTime
    */
    static TimeUnits find_end_time_for_voice(ImoScore* pScore,
                                             int instr, int voice, TimeUnits maxTime);

    /** Return a measure locator for the specified instrument and timepos.
        @param pScore Pointer to the score to wich all other parameters refer.
        @param timepos
        @param iInstr Number of the instrument (0..m) to which the measures refer to.
            Take into account that for polymetric music (music in which not all
            instruments have the same time signature), the measure number is not an
            absolute value, common to all the score instruments (score parts), but it
            is relative to an instrument. For normal scores, just providing measure
            number and location will do the job.
    */
    static MeasureLocator get_locator_for(ImoScore* pScore, TimeUnits timepos,
                                          int iInstr=0);

    /** Return the time position for the specified measure and beat.
        @param pScore Pointer to the score to wich all other parameters refer.
        @param iMeasure Measure number (0..n) in instrument iInstr.
        @param iBeat Beat number (0..m) relative to the measure.
        @param iInstr Number of the instrument (0..m) to which the measures refer to.
            Take into account that for polymetric music (music in which not all
            instruments have the same time signature), the measure number is not an
            absolute value, common to all the score instruments (score parts), but it
            is relative to an instrument. For normal scores, just providing measure
            number and location will do the job.
    */
    static TimeUnits get_timepos_for(ImoScore* pScore, int iMeasure, int iBeat,
                                     int iInstr=0);

protected:
    static ColStaffObjsIterator find_barline_with_time_lower_or_equal(ImoScore* pScore,
                                             int instr, TimeUnits maxTime);

    static TimeUnits get_beat_duration_for(ImoScore* pScore,
                                           ImMeasuresTableEntry* measure);

};


}   //namespace lomse

#endif      //__LOMSE_SCORE_ALGORITHMS_H__
