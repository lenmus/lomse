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
    */
    static int get_applicable_clef_for(ImoScore* pScore,
                                       int iInstr, int iStaff, TimeUnits time);

    /** Look for a note starting at specified timepos
        @param pScore
        @param instr
        @param voice
        @param time
    */
    static ImoNoteRest* find_noterest_at(ImoScore* pScore,
                                         int instr, int voice, TimeUnits time);

    /**
    */
    static list<OverlappedNoteRest*>
            find_and_classify_overlapped_noterests_at(ImoScore* pScore,
                         int instr, int voice, TimeUnits time, TimeUnits duration);

    /** finds end timepos for given voice, lower or equal than maxTime
    */
    static TimeUnits find_end_time_for_voice(ImoScore* pScore,
                                             int instr, int voice, TimeUnits maxTime);

protected:
    static ColStaffObjsIterator find_barline_with_time_lower_or_equal(ImoScore* pScore,
                                             int instr, TimeUnits maxTime);

};


}   //namespace lomse

#endif      //__LOMSE_SCORE_ALGORITHMS_H__
