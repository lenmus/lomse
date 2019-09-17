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

#include "lomse_score_algorithms.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_time.h"
#include "lomse_staffobjs_table.h"
#include "lomse_im_measures_table.h"

//specific for ScoreAlgorithms
#include "lomse_pitch.h"


#include <algorithm>
#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// ScoreAlgorithms implementation
//=======================================================================================
ImoNote* ScoreAlgorithms::find_possible_end_of_tie(ColStaffObjs* pColStaffObjs,
                                                   ImoNote* pStartNote)
{
    // This method explores forwards to try to find a note ("the candidate note") that
    // can be tied (as end of tie) with pStartNote.
    //
    // Algorithm:
    // Find the first comming note of the same pitch and voice, and verify that
    // distance (in timepos) is equal to start note duration.
    // The search will fail as soon as we find a rest or a note with different pitch.

    //get target pitch and voice
    FPitch pitch = pStartNote->get_fpitch();
    int voice = pStartNote->get_voice();

    //define a forwards iterator and find start note
    ColStaffObjsIterator it = pColStaffObjs->find(pStartNote);
    if (it == pColStaffObjs->end())
        return nullptr;    //pStartNote not found ??????

    int instr = (*it)->num_instrument();

    //do search
    ++it;
    while(it != pColStaffObjs->end())
    {
        ImoStaffObj* pSO = (*it)->imo_object();
        if ((*it)->num_instrument() == instr
            && pSO->is_note_rest()
            && static_cast<ImoNoteRest*>(pSO)->get_voice() == voice)
        {
            if (pSO->is_note())
            {
                if (static_cast<ImoNote*>(pSO)->get_fpitch() == pitch)
                    return static_cast<ImoNote*>(pSO);    // candidate found
                else
                    // a note in the same voice with different pitch found.
                    // Imposible to tie
                    return nullptr;
            }
            else
                // a rest in the same voice found. Imposible to tie
                return nullptr;
        }
        ++it;
    }
    return nullptr;        //no suitable note found
}

//---------------------------------------------------------------------------------------
ImoKeySignature* ScoreAlgorithms::get_applicable_key(ImoScore* pScore, ImoNote* pNote)
{
    ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
    ImoKeySignature* pKey = nullptr;
    ImoInstrument* pInstr = pNote->get_instrument();
    int iInstr = pScore->get_instr_number_for(pInstr);
    int iStaff = pNote->get_staff();
    ColStaffObjsIterator it;
    for (it=pColStaffObjs->begin(); it != pColStaffObjs->end(); ++it)
    {
        if ((*it)->imo_object() == static_cast<ImoObj*>(pNote))
            break;
        if ((*it)->num_instrument() == iInstr && (*it)->staff() == iStaff)
        {
            ImoObj* pImo = (*it)->imo_object();
            if (pImo->is_key_signature())
                pKey = static_cast<ImoKeySignature*>(pImo);
        }
    }
    return pKey;
}

//---------------------------------------------------------------------------------------
int ScoreAlgorithms::get_applicable_clef_for(ImoScore* pScore,
                                             int iInstr, int iStaff, TimeUnits time)
{
    ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
    int clef = k_clef_undefined;
    ColStaffObjsIterator it;
    for (it=pColStaffObjs->begin(); it != pColStaffObjs->end(); ++it)
    {
        if (is_greater_time((*it)->time(), time))
            break;
        if ((*it)->num_instrument() == iInstr && (*it)->staff() == iStaff)
        {
            ImoObj* pImo = (*it)->imo_object();
            if (pImo->is_clef())
                clef = static_cast<ImoClef*>(pImo)->get_clef_type();
        }
    }
    return clef;
}

//---------------------------------------------------------------------------------------
ImoNoteRest* ScoreAlgorithms::find_noterest_at(ImoScore* pScore,
                                               int instr, int voice, TimeUnits time)
{
    ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
    ColStaffObjsIterator it;
    for (it=pColStaffObjs->begin(); it != pColStaffObjs->end(); ++it)
    {
        if (is_greater_time((*it)->time(), time))
            break;
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_note_rest())
        {
            ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pImo);
            if ((*it)->num_instrument() == instr)
            {
                if (pNR->get_voice() == voice
                    && !is_greater_time(time, (*it)->time() + pNR->get_duration()) )
                    return pNR;
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
list<OverlappedNoteRest*> ScoreAlgorithms::find_and_classify_overlapped_noterests_at(
        ImoScore* pScore, int instr, int voice, TimeUnits time, TimeUnits duration)
{
    list<OverlappedNoteRest*> overlaps;
    ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
    ColStaffObjsIterator it;
    for (it=pColStaffObjs->begin(); it != pColStaffObjs->end(); ++it)
    {
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_note_rest())
        {
            ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pImo);
            TimeUnits nrTime = (*it)->time();
            TimeUnits nrDuration = pNR->get_duration();
            if ((*it)->num_instrument() == instr
                && pNR->get_voice() == voice
                && is_greater_time(time + duration, nrTime)     //starts before end of inserted one
                && is_lower_time(time, nrTime + nrDuration)     //ends after start of inserted one
               )
            {
                OverlappedNoteRest* pOV = LOMSE_NEW OverlappedNoteRest(pNR);
                if (is_equal_time(nrTime, time))
                {
                    //both start at same time
                    if (is_lower_time(duration, nrDuration))
                    {
                        //test 4
                        pOV->type = k_overlap_at_start;
                        pOV->overlap = duration;
                    }
                    else
                    {
                        //test 1
                        pOV->type = k_overlap_full;
                        pOV->overlap = nrDuration;
                    }
                }
                else if (is_lower_time(time, nrTime))
                {
                    //starts after inserted one: overlap at_start or full
                    pOV->overlap = duration - (nrTime - time);
                    if (is_lower_time(pOV->overlap, nrDuration))
                    {
                        //test 5
                        pOV->type = k_overlap_at_start;
                    }
                    else
                    {
                        //test 3
                        pOV->type = k_overlap_full;
                        pOV->overlap = nrDuration;
                    }
                }
                else
                {
                    //starts before inserted one: overlap at_end
                    //test 2, 3, 5
                    pOV->overlap = nrDuration - (time - nrTime);
                    pOV->type = k_overlap_at_end;
                }

                overlaps.push_back(pOV);
            }
            else if (is_lower_time(time + duration, nrTime))
                break;
        }
    }
    return overlaps;
}

//---------------------------------------------------------------------------------------
TimeUnits ScoreAlgorithms::find_end_time_for_voice(ImoScore* pScore,
                                        int instr, int voice, TimeUnits maxTime)
{

    ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
    ColStaffObjsIterator it =
                        find_barline_with_time_lower_or_equal(pScore, instr, maxTime);

    TimeUnits endTime = 0.0;
    if (it != pColStaffObjs->end())
        endTime = (*it)->time();

    for (; it != pColStaffObjs->end(); ++it)
    {
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_note_rest())
        {
            ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pImo);
            TimeUnits time = (*it)->time();
            TimeUnits duration = pNR->get_duration();
            if ((*it)->num_instrument() == instr && pNR->get_voice() == voice)
            {
                if (!is_greater_time(time + duration, maxTime))
                    endTime = max(endTime, time+duration);
            }

            if (is_greater_time(time, maxTime))
                break;
        }
    }
    return endTime;
}

//---------------------------------------------------------------------------------------
ColStaffObjsIterator ScoreAlgorithms::find_barline_with_time_lower_or_equal(
            ImoScore* pScore, int UNUSED(instr), TimeUnits maxTime)
{
    ColStaffObjs* pColStaffObjs = pScore->get_staffobjs_table();
    ColStaffObjsIterator it = pColStaffObjs->begin();
    ColStaffObjsIterator itLastBarline = it;
    for (; it != pColStaffObjs->end(); ++it)
    {
        ImoObj* pImo = (*it)->imo_object();
        if (pImo->is_barline())
        {
            if (is_greater_time((*it)->time(), maxTime))
                break;
            itLastBarline = it;
        }
    }
    return itLastBarline;
}

//---------------------------------------------------------------------------------------
MeasureLocator ScoreAlgorithms::get_locator_for(ImoScore* pScore, TimeUnits timepos,
                                                int iInstr)
{
    MeasureLocator ml;
    ml.iInstr = iInstr;

    ImoInstrument* pInstr = pScore->get_instrument(iInstr);
    ImMeasuresTable* pTable = pInstr->get_measures_table();
    ImMeasuresTableEntry* measure = pTable->get_measure_at(timepos);
    if (measure)
    {
        ml.iMeasure = measure->get_table_index();
        ml.location = timepos - measure->get_timepos();
    }

    return ml;
}

//---------------------------------------------------------------------------------------
TimeUnits ScoreAlgorithms::get_timepos_for(ImoScore* pScore, int iMeasure, int iBeat,
                                           int iInstr)
{
    if (iInstr < 0 || iInstr >= pScore->get_num_instruments())
        return 0.0;

    ImoInstrument* pInstr = pScore->get_instrument(iInstr);
    ImMeasuresTable* pTable = pInstr->get_measures_table();
    ImMeasuresTableEntry* measure = pTable->get_measure(iMeasure);
    TimeUnits timepos = 0.0f;
    if (measure)
    {
        timepos = measure->get_timepos();
        if (iBeat >= 0)
        {
            timepos += get_beat_duration_for(pScore, measure) * iBeat;
        }
    }
    return timepos;
}

//---------------------------------------------------------------------------------------
TimeUnits ScoreAlgorithms::get_timepos_for(ImoScore* pScore, const MeasureLocator& ml)
{
    if (ml.iInstr < 0 || ml.iInstr >= pScore->get_num_instruments())
        return 0.0;

    ImoInstrument* pInstr = pScore->get_instrument(ml.iInstr);
    ImMeasuresTable* pTable = pInstr->get_measures_table();
    ImMeasuresTableEntry* measure = pTable->get_measure(ml.iMeasure);
    TimeUnits timepos = 0.0f;
    if (measure)
    {
        timepos = measure->get_timepos() + ml.location;
    }
    return timepos;
}

//---------------------------------------------------------------------------------------
TimeUnits ScoreAlgorithms::get_beat_duration_for(ImoScore* pScore,
                                                 ImMeasuresTableEntry* measure)
{
    //get current definition for 'beat'
    Document* pDoc = pScore->get_the_document();
    TimeUnits beatType = pDoc->get_beat_type();

    //use current definition for providing beat duration
    if (beatType == k_beat_implied)
        return measure->get_implied_beat_duration();

    if (beatType == k_beat_bottom_ts)
        return measure->get_bottom_ts_beat_duration();

    return pDoc->get_beat_duration();
}


}  //namespace lomse
