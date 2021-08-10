//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#include "lomse_staffobjs_table.h"

#include <algorithm>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_ldp_exporter.h"
#include "lomse_time.h"
#include "lomse_im_factory.h"


#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// ColStaffObjsEntry implementation
//=======================================================================================
string ColStaffObjsEntry::dump(bool fWithIds)
{
    stringstream s;
    s << fixed << setprecision(0) << setfill(' ');
    s << m_instr << "\t" << m_staff << "\t" << m_measure << "\t" << time();
    if (m_pImo->is_note_rest())
    {
        if (m_pImo->is_grace_note())
        {
            ImoGraceNote* pGrace = static_cast<ImoGraceNote*>(m_pImo);
            s << "-" << pGrace->get_align_timepos() - time();
        }
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(m_pImo);
        s << "\t" << setprecision(2)
          << pNR->get_playback_time() << "\t" << pNR->get_playback_duration()
          << setprecision(0);
    }
    else
    {
        s << "\t" << "-" << "\t" << "-";
    }
    s << "\t" << m_line
      << "\t" << (fWithIds ? to_string_with_ids() : to_string()) << endl;

    return s.str();
}

//---------------------------------------------------------------------------------------
std::string ColStaffObjsEntry::to_string()
{
    return m_pImo->to_string();
}

//---------------------------------------------------------------------------------------
std::string ColStaffObjsEntry::to_string_with_ids()
{
    return m_pImo->to_string_with_ids();
}



//=======================================================================================
// ColStaffObjs implementation
//=======================================================================================
ColStaffObjs::ColStaffObjs()
    : m_numLines(0)
    , m_numEntries(0)
    , m_rMissingTime(0.0)
    , m_rAnacrusisExtraTime(0.0)
    , m_minNoteDuration(LOMSE_NO_NOTE_DURATION)
    , m_pFirst(nullptr)
    , m_pLast(nullptr)
{
}

//---------------------------------------------------------------------------------------
ColStaffObjs::~ColStaffObjs()
{
    ColStaffObjs::iterator it;
    for (it=begin(); it != end(); ++it)
        delete *it;
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* ColStaffObjs::add_entry(int measure, int instr, int voice, int staff,
                                           ImoStaffObj* pImo)
{
    ColStaffObjsEntry* pEntry =
        LOMSE_NEW ColStaffObjsEntry(measure, instr, voice, staff, pImo);
    add_entry_to_list(pEntry);
    ++m_numEntries;
    return pEntry;
}

//---------------------------------------------------------------------------------------
string ColStaffObjs::dump(bool fWithIds)
{
    stringstream s;
    ColStaffObjs::iterator it;
    s << "Num.entries = " << num_entries()
      << ", anacrusis: missing time = " << m_rMissingTime
      << ", extra time = " << m_rAnacrusisExtraTime << endl;
    //    +.......+.......+.......+.......+.......+.......+.......+.......+
    s << "instr   staff   meas.   time    play    pdur    line    object" << endl;
    s << "----------------------------------------------------------------" << endl;
    for (it=begin(); it != end(); ++it)
    {
        s << (*it)->dump(fWithIds);
    }
    return s.str();
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::add_entry_to_list(ColStaffObjsEntry* pEntry)
{
    if (!m_pFirst)
    {
        //first entry
        m_pFirst = pEntry;
        m_pLast = pEntry;
        pEntry->set_prev( nullptr );
        pEntry->set_next( nullptr );
        return;
    }

    //insert in list in order
    ColStaffObjsEntry* pCurrent = m_pLast;
    while (pCurrent != nullptr)
    {
        if (is_lower_entry(pEntry, pCurrent))
            pCurrent = pCurrent->get_prev();
        else
        {
            //insert after pCurrent
            ColStaffObjsEntry* pNext = pCurrent->get_next();
            pEntry->set_prev( pCurrent );
            pEntry->set_next( pNext );
            pCurrent->set_next( pEntry );
            if (pNext == nullptr)
                m_pLast = pEntry;
            else
                pNext->set_prev( pEntry );
            return;
        }
    }

    //it is the first one
    pEntry->set_prev( nullptr );
    pEntry->set_next( m_pFirst );
    m_pFirst->set_prev( pEntry );
    m_pFirst = pEntry;
}

//---------------------------------------------------------------------------------------
bool ColStaffObjs::is_lower_entry(ColStaffObjsEntry* b, ColStaffObjsEntry* a)
{
    //auxiliary, for sort: by time, object type (barlines before objects in other lines),
    //line and staff.
    //AWARE:
    //   Current order is first pA (existing object), then pB (new object to insert)
    //   return TRUE to insert pB before pA, FALSE to keep in current order
    //this is:
    //  TRUE: Continue searching for the insertion point. pA = pA->prev
    //  FALSE: Insert B after object A


    //R1. All staffobjs must be ordered by timepos
    {
        //R1.1 swap if B has lower time than A
        if ( is_lower_time(b->time(), a->time()) )
            return true;    //B cannot go after A, Try with A-1

        //R1.2 time(pB) > time(pA). They are correctly ordered
        if ( is_greater_time(b->time(), a->time()) )
            return false;   //insert B after A
    }

    //Here A & B have the same timepos
    ImoStaffObj* pB = b->imo_object();
    ImoStaffObj* pA = a->imo_object();

    //R7. Graces in the same timepos must go ordered by align timepos
    if (pA->is_grace_note() && pB->is_grace_note())
    {
        ImoGraceNote* pGA = static_cast<ImoGraceNote*>(pA);
        ImoGraceNote* pGB = static_cast<ImoGraceNote*>(pB);

        if ( is_lower_time(pGB->get_align_timepos(), pGA->get_align_timepos()) )
            return true;    //B cannot go after A, Try with A-1

        if ( is_greater_time(pGB->get_align_timepos(), pGA->get_align_timepos()) )
            return false;   //insert B after A
    }

    //R8. Insertion order of graces and barlines in an instrument must be preserved.
    //    Barlines defined after grace notes in other instruments must go before those
    //    grace notes (two parts)
    if (pA->is_grace_note() && pB->is_barline()
            && (a->num_instrument() == b->num_instrument())
       )
        return false;   //preserve definition order: insert barline B after grace A

    //R4. Any object must go before all other objects at same timepos having
    //    high measure number
    if (b->measure() < a->measure())
        return true;    //B (high measure number) cannot go after A, Try with A-1

    //R6. Graces in the same timepos must go before note/rest in that timepos
    if (pB->is_grace_note() && (pA->is_rest() || pA->is_regular_note() || pA->is_cue_note()) )
        return true;    //B (grace) cannot go after A, Try with A-1

    //R7. Graces in the same timepos go ordered by align timepos
    if (pB->is_grace_note() && (pA->is_rest() || pA->is_regular_note() || pA->is_cue_note()) )
        return true;    //B (grace) cannot go after A, Try with A-1

    //R5. Non-timed must go before barlines at the same timepos with equal measure
    //    number. But if non-timed is also a barline preserve insertion order
    if (pB->is_barline() && pA->is_barline())
        return false;   //insert B after A (preserve order of barlines)
    if (pA->is_barline() && pB->get_duration() == 0.0f && (b->measure() == a->measure()) )
        return true;    //B (non-timed) cannot go after A (barline), Try with A-1

    //R2. note/rest can not go before non-timed at same timepos
    if (pA->is_note_rest() && pB->get_duration() == 0.0f && !pB->is_grace_note())
        return true;    //B cannot go after A, Try with A-1

    //R3. <direction>, <sound> and <transpose> can not go between clefs/key/time ==>
    //    clef/key/time can not go after direction. Missing: in other instruments/staves
    if ((pA->is_direction() || pA->is_sound_change() || pA->is_transpose())
        && (pB->is_clef() || pB->is_time_signature() || pB->is_key_signature()))
    {
        return true;    //move clef/key/time before 'A' object
    }

    //R11. Clefs must go before barlines at the same timepos
    if (pB->is_clef() && pA->is_barline())
        return true;   //move B after A


    //R999. Both have equal time. If no other rule triggers preserve definition order
    return false;   //insert B after A
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::delete_entry_for(ImoStaffObj* pSO)
{
    ColStaffObjsEntry* pEntry = find_entry_for(pSO);
    if (!pEntry)
    {
        LOMSE_LOG_ERROR("[ColStaffObjs::delete_entry_for] entry not found!");
        throw runtime_error("[ColStaffObjs::delete_entry_for] entry not found!");
    }

    ColStaffObjsEntry* pPrev = pEntry->get_prev();
    ColStaffObjsEntry* pNext = pEntry->get_next();
    delete pEntry;
    if (pPrev == nullptr)
    {
        //removing the head of the list
        m_pFirst = pNext;
        if (pNext)
            pNext->set_prev(nullptr);
    }
    else if (pNext == nullptr)
    {
        //removing the tail of the list
        m_pLast = pPrev;
        pPrev->set_next(nullptr);
    }
    else
    {
        //removing intermediate node
        pPrev->set_next( pNext );
        pNext->set_prev( pPrev );
    }
    --m_numEntries;
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* ColStaffObjs::find_entry_for(ImoStaffObj* pSO)
{
    ColStaffObjs::iterator it;
    for (it=begin(); it != end(); ++it)
    {
        if ((*it)->imo_object() == pSO)
            return *it;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::sort_table()
{
    //The table is created with entries in order. But edition operations forces to
    //reorder entries. The main requirement for the sort algorithm is:
    // * stable (preserve order of elements with equal keys)
    //The chosen algorithm is the insertion sort, that also has some advantages:
    // * good performance when table is nearly ordered
    // * simple to implement and much better performance than other simple algorithms,
    //   such as the bubble sort
    // * in-place sort (does not require extra memory)

    ColStaffObjsEntry* pUnsorted = m_pFirst;
    m_pFirst = nullptr;
    m_pLast = nullptr;

    while (pUnsorted != nullptr)
    {
        ColStaffObjsEntry* pCurrent = pUnsorted;

        //get next here, as insertion could change next element
        pUnsorted = pUnsorted->get_next();

        add_entry_to_list(pCurrent);
    }
}



//=======================================================================================
// ColStaffObjsBuilder implementation: algorithm to create a ColStaffObjs
//=======================================================================================
ColStaffObjs* ColStaffObjsBuilder::build(ImoScore* pScore)
{
    ColStaffObjsBuilderEngine* builder = create_builder_engine(pScore);
    ColStaffObjs* pColStaffObjs = builder->do_build();
    pScore->set_staffobjs_table(pColStaffObjs);

    delete builder;

    return pColStaffObjs;
}

//---------------------------------------------------------------------------------------
ColStaffObjsBuilderEngine* ColStaffObjsBuilder::create_builder_engine(ImoScore* pScore)
{
    int major = pScore->get_version_major();
    if (major < 2)
        return LOMSE_NEW ColStaffObjsBuilderEngine1x(pScore);
    else
        return LOMSE_NEW ColStaffObjsBuilderEngine2x(pScore);
}


//=======================================================================================
// ColStaffObjsBuilderEngine
//=======================================================================================
ColStaffObjs* ColStaffObjsBuilderEngine::do_build()
{
    create_table();
    set_num_lines();
    set_min_note_duration();
    return m_pColStaffObjs;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::create_table()
{
    initializations();
    int totalInstruments = m_pImScore->get_num_instruments();
    for (int instr = 0; instr < totalInstruments; instr++)
    {
        create_entries_for_instrument(instr);
        prepare_for_next_instrument();
    }
    compute_grace_notes_playback_time();
    collect_anacrusis_info();
    compute_arpeggiated_chords_playback_time();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::collect_anacrusis_info()
{
    ColStaffObjsIterator it = m_pColStaffObjs->begin();
    ImoTimeSignature* pTS = nullptr;
    TimeUnits rTime = -1.0;

    //find time signature
    while(it != m_pColStaffObjs->end())
    {
        ImoStaffObj* pSO = (*it)->imo_object();
        if (pSO->is_time_signature())
        {
            pTS = static_cast<ImoTimeSignature*>( pSO );
            break;
        }
        else if (pSO->is_note_rest())
            break;
        ++it;
    }
    if (pTS == nullptr)
    {
        if (m_gracesAnacrusisTime > 0.0)
        {
            fix_negative_playback_times();
            m_pColStaffObjs->set_anacrusis_missing_time(64.0 - m_gracesAnacrusisTime);
            m_pColStaffObjs->set_anacrusis_extra_time(m_gracesAnacrusisTime);
        }
        return;
    }

    // find first barline
    ++it;
    while(it != m_pColStaffObjs->end())
    {
        ImoStaffObj* pSO = (*it)->imo_object();
        if (pSO->is_barline())
        {
            rTime = (*it)->time();
            break;
        }
        ++it;
    }
    if (rTime <= 0.0)
    {
    //TODO: test case can not be generated with MusicXML
//        if (m_gracesAnacrusisTime > 0.0)
//        {
//            fix_negative_playback_times();
//            m_pColStaffObjs->set_anacrusis_missing_time(64.0 - m_gracesAnacrusisTime);
//            m_pColStaffObjs->set_anacrusis_extra_time(m_gracesAnacrusisTime);
//        }
        return;
    }

    //set anacrusis missing time
    TimeUnits measureTime = pTS->get_measure_duration();
    TimeUnits anacrusis = (is_lower_time(rTime, measureTime) ? rTime : 0.0);

    if (m_gracesAnacrusisTime > 0.0)
    {
        fix_negative_playback_times();
        m_pColStaffObjs->set_anacrusis_extra_time(m_gracesAnacrusisTime);
        anacrusis += m_gracesAnacrusisTime;
    }
    if (anacrusis > 0.0)
        m_pColStaffObjs->set_anacrusis_missing_time(max(0.0, measureTime - anacrusis));
}

//---------------------------------------------------------------------------------------
int ColStaffObjsBuilderEngine::get_line_for(int nVoice, int nStaff)
{
    return m_lines.get_line_assigned_to(nVoice, nStaff);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::set_num_lines()
{
    m_pColStaffObjs->set_total_lines( m_lines.get_number_of_lines() );
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::add_entries_for_key_or_time_signature(ImoObj* pImo, int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    int numStaves = pInstr->get_num_staves();

    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    determine_timepos(pSO);
    for (int nStaff=0; nStaff < numStaves; nStaff++)
    {
        int nLine = get_line_for(0, nStaff);
        m_pColStaffObjs->add_entry(m_nCurMeasure, nInstr, nLine, nStaff, pSO);
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::set_min_note_duration()
{
    m_pColStaffObjs->set_min_note(m_minNoteDuration);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::compute_grace_notes_playback_time()
{
    for (auto pEntry : m_graces)
    {
        ImoGraceNote* pGrace = static_cast<ImoGraceNote*>(pEntry->imo_object());
        ImoGraceRelObj* pGraceRO = pGrace->get_grace_relobj();
        if (static_cast<ImoGraceNote*>(pGraceRO->get_start_object()) == pGrace)
            process_grace_relobj(pGrace, pGraceRO, pEntry);
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::compute_arpeggiated_chords_playback_time()
{
    for (auto pBaseNote : m_arpeggios)
    {
        //determine arpeggio direction: top-down or bottom-up
        bool fBottomUp = pBaseNote->get_arpeggio()->get_type() != k_arpeggio_arrow_down;

        //collect arpeggio notes and sort them by pitch, as required by arpeggio type
        std::list<ImoNote*> arpeggioNotes;
        ImoChord* pChord = pBaseNote->get_chord();
        list< pair<ImoStaffObj*, ImoRelDataObj*> >& chordNotes = pChord->get_related_objects();
        list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itC;
        for (itC=chordNotes.begin(); itC != chordNotes.end(); ++itC)
        {
            ImoNote* pNote = static_cast<ImoNote*>((*itC).first);
            save_arpeggiated_note(pNote, fBottomUp, arpeggioNotes);
        }

        //assign duration to the arpeggio
        TimeUnits arpeggioDuration =
            min(pBaseNote->get_playback_duration() * 0.7, TimeUnits(k_duration_16th));

        //determine time shift per note
        TimeUnits shift = arpeggioDuration / double(chordNotes.size() - 1);

        //Compute notes start time and duration
        TimeUnits totalShift = 0.0;
        for (auto pNote : arpeggioNotes)
        {
            pNote->set_playback_time( pNote->get_playback_time() + totalShift);
            pNote->set_playback_duration( pNote->get_playback_duration() - totalShift);
            totalShift += shift;
        }
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::save_arpeggiated_note(ImoNote* pNote, bool fBottomUp,
                                                      list<ImoNote*>& chordNotes)
{
    //notes are inserted to keep them sorted by pitch

    if (chordNotes.size() == 0)
    {
	    chordNotes.push_back(pNote);
    }
    else
    {
        MidiPitch newPitch( pNote->get_midi_pitch() );
        std::list<ImoNote*>::iterator it;
        for (it = chordNotes.begin(); it != chordNotes.end(); ++it)
        {
            if ( (fBottomUp && newPitch < (*it)->get_midi_pitch())
                 || (!fBottomUp && newPitch > (*it)->get_midi_pitch())
               )
            {
	            chordNotes.insert(it, 1, pNote);
                return;
            }
        }
	    chordNotes.push_back(pNote);
    }
}

//----------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::process_grace_relobj(ImoGraceNote* pGrace,
                                                     ImoGraceRelObj* pGRO,
                                                     ColStaffObjsEntry* pEntry)
{
    //this method is invoked only for the first grace note of each grace relobj.


    //default playback time of first grace in the group
    TimeUnits gracePlayTime = pGrace->get_playback_time();

    //decide were to take time from and locate the associated regular note
    ImoNote* pTarget = nullptr;
    if (pGRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_following)
        pTarget = locate_grace_principal_note(pEntry);
    else
        pTarget = locate_grace_previous_note(pEntry);


//    //Include the regular note as part of the Grace RelObj
//    //This is not currently necessary but, perhaps in future it could be necessary
//    //to determine if a regular note has graces associated to it. In that case
//    //this code shows how to do it.
//    if (pTarget)
//    {
//        //add principal or previous note to the relationship
//        Document* pDoc = m_pImScore->get_the_document();
//        pTarget->include_in_relation(pDoc, pGRO);
//    }


    //determine time to steal
    double percentage = pGRO->get_percentage();
    TimeUnits dur = 0.0;

    //if not make time, discount time from next/prev
    TimeUnits newTargetDur = 0.0;
    if (pGRO->get_grace_type() != ImoGraceRelObj::k_grace_make_time)
    {
        if (pTarget)
        {
            TimeUnits targetDur = pTarget->get_duration();
            dur = targetDur * percentage;
            newTargetDur = targetDur - dur;
            pTarget->set_playback_duration(newTargetDur);

            //deal with chords
            if (pTarget->is_note())
            {
                ImoNote* pNote = static_cast<ImoNote*>(pTarget);
                if (pNote->is_in_chord())
                {
                    ImoChord* pChord = pNote->get_chord();
                    list< pair<ImoStaffObj*, ImoRelDataObj*> >& chordNotes = pChord->get_related_objects();
                    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itC;
                    for (itC=chordNotes.begin(); itC != chordNotes.end(); ++itC)
                    {
                        ImoNote* pNote = static_cast<ImoNote*>((*itC).first);
                        pNote->set_playback_duration(newTargetDur);
                    }
                }
            }
        }
        else
        {
            //use a quarter note
            dur = TimeUnits(k_duration_quarter) * percentage;
        }

        //set prev or ppal playback time and duration
        if (pTarget)
        {
            if (pGRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous)
            {
                //steal from prev. Fix playback time of grace note
                gracePlayTime = pTarget->get_playback_time() + pTarget->get_playback_duration();
            }
            else
            {
                //steal from next. Grace playback time is the timepos of ppal note
                gracePlayTime = pTarget->get_playback_time();
                TimeUnits newPlaytime = gracePlayTime + dur;
                pTarget->set_playback_time(newPlaytime);

                //deal with chords
                if (pTarget->is_note())
                {
                    ImoNote* pNote = static_cast<ImoNote*>(pTarget);
                    if (pNote->is_in_chord())
                    {
                        ImoChord* pChord = pNote->get_chord();
                        list< pair<ImoStaffObj*, ImoRelDataObj*> >& chordNotes = pChord->get_related_objects();
                        list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itC;
                        for (itC=chordNotes.begin(); itC != chordNotes.end(); ++itC)
                        {
                            ImoNote* pNote = static_cast<ImoNote*>((*itC).first);
                            pNote->set_playback_time(newPlaytime);
                        }
                    }
                }
            }
        }
    }


    //if no previous shift back graces play time.
    //test score: unit-tests/grace-notes/214-grace-chord-two-voices-previos-note.xml
    if (!pTarget && (pGRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous))
    {
        gracePlayTime -= dur;
        if (gracePlayTime < 0.0)
            m_gracesAnacrusisTime = max(m_gracesAnacrusisTime, -gracePlayTime);
    }


    //count number of grace notes
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes
                                 = pGRO->get_related_objects();
    int numGraces = 0;
    for (auto p : notes)
    {
        ImoNote* pN = static_cast<ImoNote*>(p.first);
        if (pN->is_grace_note() &&
            ((pN->is_in_chord() && pN->is_start_of_chord()) || !pN->is_in_chord()) )
            ++numGraces;
        else
            break;
    }

    //assign duration to each grace note
    dur /= double(numGraces);
    for (auto p : notes)
    {
        if (p.first->is_grace_note())
        {
            ImoNote* pN = static_cast<ImoNote*>(p.first);
            pN->set_playback_duration(dur);
            pN->set_playback_time(gracePlayTime);
            if (!pN->is_in_chord() || pN->is_end_of_chord())
                gracePlayTime += dur;
        }
        else
            break;
    }

    pGRO = nullptr;
}

//----------------------------------------------------------------------------------
ImoNote* ColStaffObjsBuilderEngine::locate_grace_principal_note(ColStaffObjsEntry* pEntry)
{
    int line = pEntry->line();
    int instr = pEntry->num_instrument();

    pEntry = pEntry->get_next();
    while(pEntry)
    {
        ImoStaffObj* pSO = pEntry->imo_object();
        if (pSO->is_regular_note()
            && pEntry->num_instrument() == instr
            && pEntry->line() == line
           )
        {
            return static_cast<ImoNote*>(pSO);
        }
        pEntry = pEntry->get_next();
    }
    return nullptr;
}

//----------------------------------------------------------------------------------
ImoNote* ColStaffObjsBuilderEngine::locate_grace_previous_note(ColStaffObjsEntry* pEntry)
{
    int line = pEntry->line();
    int instr = pEntry->num_instrument();

    pEntry = pEntry->get_prev();
    while(pEntry)
    {
        ImoStaffObj* pSO = pEntry->imo_object();
        if (pSO->is_regular_note()
            && pEntry->num_instrument() == instr
            && pEntry->line() == line
           )
        {
            return static_cast<ImoNote*>(pSO);
        }
        pEntry = pEntry->get_prev();
    }
    return nullptr;
}

//----------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::fix_negative_playback_times()
{
    ColStaffObjsIterator it = m_pColStaffObjs->begin();
    while(it != m_pColStaffObjs->end())
    {
        ImoStaffObj* pSO = (*it)->imo_object();
        if (pSO->is_note_rest())
        {
            ImoNoteRest* pNR = static_cast<ImoNoteRest*>( pSO );
            TimeUnits playtime = pNR->get_playback_time();
            pNR->set_playback_time(playtime + m_gracesAnacrusisTime);
        }
        ++it;
    }
}


//=======================================================================================
// ColStaffObjsBuilderEngine1x implementation: algorithm to create a ColStaffObjs
//=======================================================================================
void ColStaffObjsBuilderEngine1x::initializations()
{
    m_pColStaffObjs = LOMSE_NEW ColStaffObjs();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::create_entries_for_instrument(int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    ImoMusicData* pMusicData = pInstr->get_musicdata();
    if (!pMusicData)
        return;

    ImoObj::children_iterator it = pMusicData->begin();
    reset_counters();
    while(it != pMusicData->end())
    {
        if ((*it)->is_go_back_fwd())
        {
            ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>(*it);
            update_time_counter(pGBF);
            ++it;

            //delete_node(pGBF, pMusicData);
            //CSG: goBack/Fwd nodes can not be removed. They are necessary in score edition
            //when ColStaffObjs must be rebuild
        }
        else if ((*it)->is_key_signature() || (*it)->is_time_signature())
        {
            add_entries_for_key_or_time_signature(*it, nInstr);
            ++it;
        }
        else
        {
            ImoStaffObj* pSO = static_cast<ImoStaffObj*>(*it);
            add_entry_for_staffobj(pSO, nInstr);
            update_measure(pSO);
            ++it;
        }
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::add_entry_for_staffobj(ImoObj* pImo, int nInstr)
{
    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    determine_timepos(pSO);
    int nStaff = pSO->get_staff();
    int nVoice = 0;
    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        nVoice = pNR->get_voice();
        if (!pNR->is_grace_note())
            m_minNoteDuration = min(m_minNoteDuration, pNR->get_duration());

        if (pNR->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pNR);
            if (pNote->is_start_of_chord() && pNote->get_arpeggio())
                m_arpeggios.push_back(pNote);
        }
    }
    int nLine = get_line_for(nVoice, nStaff);
    ColStaffObjsEntry* pEntry = m_pColStaffObjs->add_entry(m_nCurMeasure, nInstr,
                                                           nLine, nStaff, pSO);
    if (pSO->is_grace_note())
        m_graces.push_back(pEntry);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::delete_node(ImoGoBackFwd* pGBF, ImoMusicData* pMusicData)
{
    pMusicData->remove_child(pGBF);
    delete pGBF;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::reset_counters()
{
    m_nCurMeasure = 0;
    m_rCurTime = 0.0;
    m_rCurAlignTime = 0.0;
    m_rMaxSegmentTime = 0.0;
    m_rStartSegmentTime = 0.0;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::determine_timepos(ImoStaffObj* pSO)
{
    pSO->set_time(m_rCurTime);

    if (pSO->is_note())
    {
        if (pSO->is_grace_note())
        {
            ImoGraceNote* pGrace = static_cast<ImoGraceNote*>(pSO);
            pGrace->set_align_timepos(m_rCurAlignTime);
            if (!pGrace->is_in_chord() || pGrace->is_end_of_chord())
                m_rCurAlignTime += 1.0;
        }
        else
        {
            ImoNote* pNote = static_cast<ImoNote*>(pSO);
            if (!pNote->is_in_chord() || pNote->is_end_of_chord())
                m_rCurTime += pSO->get_duration();
            m_rCurAlignTime = m_rCurTime;
        }
    }
    else if (pSO->is_barline())
        pSO->set_time(m_rMaxSegmentTime);
    else
        m_rCurTime += pSO->get_duration();

    m_rMaxSegmentTime = max(m_rMaxSegmentTime, m_rCurTime);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::update_measure(ImoStaffObj* pSO)
{
    if (pSO->is_barline())
    {
        ++m_nCurMeasure;
        m_rMaxSegmentTime = 0.0;
        m_rStartSegmentTime = m_rCurTime;
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::update_time_counter(ImoGoBackFwd* pGBF)
{
    if (pGBF->is_to_start())
        m_rCurTime = m_rStartSegmentTime;
    else if (pGBF->is_to_end())
        m_rCurTime = m_rMaxSegmentTime;
    else
    {
        TimeUnits time = m_rCurTime + pGBF->get_time_shift();
        m_rCurTime = (time < m_rStartSegmentTime ? m_rStartSegmentTime : time);
        m_rMaxSegmentTime = max(m_rMaxSegmentTime, m_rCurTime);
    }
    m_rCurAlignTime = m_rCurTime;
}

//---------------------------------------------------------------------------------------
ImoDirection* ColStaffObjsBuilderEngine1x::anchor_object(ImoAuxObj* pAux)
{
    Document* pDoc = m_pImScore->get_the_document();
    ImoDirection* pAnchor =
            static_cast<ImoDirection*>(ImFactory::inject(k_imo_direction, pDoc));
    pAnchor->add_attachment(pDoc, pAux);
    return pAnchor;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::prepare_for_next_instrument()
{
    m_lines.new_instrument();
}



//=======================================================================================
// ColStaffObjsBuilderEngine2x implementation: algorithm to create a ColStaffObjs
//=======================================================================================
void ColStaffObjsBuilderEngine2x::initializations()
{
    m_rCurTime.reserve(k_max_voices);
    m_pColStaffObjs = LOMSE_NEW ColStaffObjs();
    m_curVoice = 0;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::create_entries_for_instrument(int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    ImoMusicData* pMusicData = pInstr->get_musicdata();
    if (!pMusicData)
        return;

    reset_counters();
    ImoObj::children_iterator it;
    for(it = pMusicData->begin(); it != pMusicData->end(); ++it)
    {
        if ((*it)->is_key_signature() || (*it)->is_time_signature())
        {
            add_entries_for_key_or_time_signature(*it, nInstr);
        }
        else
        {
            ImoStaffObj* pSO = static_cast<ImoStaffObj*>(*it);
            add_entry_for_staffobj(pSO, nInstr);
            if (pSO->is_barline())
                update_measure();
        }
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::add_entry_for_staffobj(ImoObj* pImo, int nInstr)
{
    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    determine_timepos(pSO);
    int nStaff = pSO->get_staff();
    int nVoice = 0;
    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        nVoice = pNR->get_voice();
        m_curVoice = nVoice;

        if (pNR->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pNR);
            if (pNote->is_start_of_chord() && pNote->get_arpeggio())
                m_arpeggios.push_back(pNote);
        }
    }
    else if (pSO->is_barline())
        nVoice = 0;
    else
        nVoice = m_curVoice;

    int nLine = get_line_for(nVoice, nStaff);
    m_pColStaffObjs->add_entry(m_nCurMeasure, nInstr, nLine, nStaff, pSO);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::reset_counters()
{
    m_curVoice = 0;
    m_nCurMeasure = 0;
    m_rMaxSegmentTime = 0.0;
    m_rStartSegmentTime = 0.0;
    m_rCurTime.assign(k_max_voices, 0.0);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::determine_timepos(ImoStaffObj* pSO)
{
    TimeUnits time;
    TimeUnits duration = pSO->get_duration();

    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        int voice = pNR->get_voice();
        time = m_rCurTime[voice];

        if (pSO->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pSO);
            if (pNote->is_in_chord() && !pNote->is_end_of_chord())
                duration = 0.0;
        }
        m_rCurTime[voice] += duration;

        if (duration > 0.0)
            m_minNoteDuration = min(m_minNoteDuration, duration);
    }
    else if (pSO->is_barline())
    {
        time = m_rMaxSegmentTime;
    }
    else
    {
        time = m_rCurTime[m_curVoice];
    }

    pSO->set_time(time);
    m_rMaxSegmentTime = max(m_rMaxSegmentTime, time+duration);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::update_measure()
{
    ++m_nCurMeasure;
    m_rStartSegmentTime = m_rMaxSegmentTime;
    m_rCurTime.assign(k_max_voices, m_rMaxSegmentTime);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::prepare_for_next_instrument()
{
    m_lines.new_instrument();
    m_curVoice = 0;
}



//=======================================================================================
// StaffVoiceLineTable implementation
//=======================================================================================
StaffVoiceLineTable::StaffVoiceLineTable()
    : m_lastAssignedLine(-1)
{
    assign_line_to(0, 0);

    //first voice in each staff not yet known
    for (int i=0; i < 10; i++)
        m_firstVoiceForStaff.push_back(0);
}

//---------------------------------------------------------------------------------------
int StaffVoiceLineTable::get_line_assigned_to(int nVoice, int nStaff)
{
    int key = form_key(nVoice, nStaff);
    std::map<int, int>::iterator it = m_lineForStaffVoice.find(key);
    if (it != m_lineForStaffVoice.end())
        return it->second;
    else
        return assign_line_to(nVoice, nStaff);
}

//---------------------------------------------------------------------------------------
int StaffVoiceLineTable::assign_line_to(int nVoice, int nStaff)
{
    int key = form_key(nVoice, nStaff);
    if (nVoice != 0)
    {
        if (m_firstVoiceForStaff[nStaff] == 0)
        {
            //No voice yet assigned to this staff. Save voice nVoice as
            //first voice in this staff and assign it the sSame line than
            //voice 0 (nStaff)
            m_firstVoiceForStaff[nStaff] = nVoice;
            int line = get_line_assigned_to(0, nStaff);
            m_lineForStaffVoice[key] = line;
            return line;
        }
        else if (m_firstVoiceForStaff[nStaff] == nVoice)
        {
            //voice nVoice is the first voice found in this staff.
            //Assign it the sSame line than voice 0 (nStaff)
            return get_line_assigned_to(0, nStaff);
        }
        //else, assig it a line
    }

    //voice == 0 or voice is not first voice for this staff.
    //assign it the next available line number
    int line = ++m_lastAssignedLine;
    m_lineForStaffVoice[key] = line;
    return line;
}

//---------------------------------------------------------------------------------------
void StaffVoiceLineTable::new_instrument()
{
    m_lineForStaffVoice.clear();

    assign_line_to(0, 0);

    //first voice in each staff not yet known
    for (int i=0; i < 10; i++)
        m_firstVoiceForStaff[i] = 0;
}


}  //namespace lomse
