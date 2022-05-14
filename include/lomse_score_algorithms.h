//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

    /** Look for a note that can be tied (as end of tie) with pStartNote     */
    static ImoNote* find_possible_end_of_tie(ColStaffObjs* pColStaffObjs,
                                             ImoNote* pStartNote);

    /** Look for the key signature that is applicable to a note     */
    static ImoKeySignature* get_applicable_key(ImoScore* pScore, ImoNote* pNote);

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

    /** Return the time position for the specified measure locator.
        @param pScore Pointer to the score to wich all other parameters refer.
        @param ml The measure locator to convert.
    */
    static TimeUnits get_timepos_for(ImoScore* pScore, const MeasureLocator& ml);

protected:
    static ColStaffObjsIterator find_barline_with_time_lower_or_equal(ImoScore* pScore,
                                             int instr, TimeUnits maxTime);

};


}   //namespace lomse

#endif      //__LOMSE_SCORE_ALGORITHMS_H__
