//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#include "lomse_midi_table.h"

#include <algorithm>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_time.h"
#include "lomse_staffobjs_table.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_pitch.h"
#include "lomse_score_utilities.h"
#include "lomse_im_attributes.h"

using namespace std;

namespace lomse
{

//=======================================================================================
// SoundEventsTable: Manager for the events table
//
//    There are two tables to maintain:
//    - m_events (std::vector<SoundEvent*>):
//        Contains the MIDI events.
//    - m_measures (std::vector<int>):
//        Contains the index over m_events for the first event of each measure.
//
//    AWARE
//    Measures are numbered 1..n (musicians usual way) not 0..n-1. But tables
//    go 0..n+1 :
//      - Item 0 corresponds to control events before the start of the first
//        measure.
//      - Item n+1 corresponds to control events after the final bar, normally only
//        the EndOfTable control event.
//      - Items 1..n corresponds to the real measures 1..n.
//    In the events table m_events, all events not in a real measure (measures 1..n) are
//    marked as belonging to measure 0.
//
//    The two tables must be synchronized.
//=======================================================================================
SoundEventsTable::SoundEventsTable(ImoScore* pScore)
    : m_pScore(pScore)
    , m_numMeasures(0)
    , m_rAnacrusisMissingTime(0.0)
    , m_rAnacrusisExtraTime(0.0)
{
}

//---------------------------------------------------------------------------------------
SoundEventsTable::~SoundEventsTable()
{
    delete_events_table();
    delete_jumps_table();
    delete_measures_jumps_table();
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::delete_events_table()
{
    for (auto it : m_events)
        delete it;
    m_events.clear();
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::delete_jumps_table()
{
    for (auto it : m_jumps)
        delete it;
    m_jumps.clear();
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::delete_measures_jumps_table()
{
    for (auto it : m_measuresJumps)
        delete it;
    m_measuresJumps.clear();
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::create_table()
{
    program_sounds_for_instruments();
    create_events();
    sort_by_time();
    close_table();
    create_measures_table();
    replace_label_in_jumps();
    add_events_to_jumps();
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::program_sounds_for_instruments()
{
	int numInstruments = m_pScore->get_num_instruments();
    m_channels.resize(numInstruments);

    for (int iInstr = 0; iInstr < numInstruments; iInstr++)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        int channel = 0;
        int instr = 0;
        if (pInstr->get_num_sounds() > 0)
        {
            ImoSoundInfo* pInfo = pInstr->get_sound_info(0);
            ImoMidiInfo* pMidi = pInfo->get_midi_info();
            channel = pMidi->get_midi_channel();
            instr = pMidi->get_midi_program();
        }
        m_channels[iInstr] = channel;
        store_event(0, SoundEvent::k_prog_instr, channel, instr, 0, 0, nullptr, 0);
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::create_events()
{
    ImoStaffObj* pSO = nullptr;
    StaffObjsCursor cursor(m_pScore);
    m_semitones.assign(cursor.get_num_staves(), 0);

    //TODO change so that anacruxis measure is counted as 0
    int jumpToMeasure = 1;

    m_rAnacrusisMissingTime = cursor.anacruxis_missing_time();
    m_rAnacrusisExtraTime = cursor.anacruxis_extra_time();

    //iterate over the collection to create the MIDI events
    while(!cursor.is_end())
    {
        int measure = cursor.measure() + 1;     //start count in 1

        pSO = cursor.get_staffobj();
        if (pSO->is_note_rest())
        {
            add_noterest_events(cursor, measure);
        }
        else if (pSO->is_barline())
        {
            //only repetitions and volta brackets in first instrument are taken into
            //consideration. Otherwise redundant invalid jumps would be created.
            if (cursor.num_instrument() == 0)
            {
                ImoBarline* pBar = static_cast<ImoBarline*>(pSO);
                if (pBar->get_type() == k_barline_start_repetition)
                {
                    jumpToMeasure = measure+1;
                }
                else if (pBar->get_type() == k_barline_end_repetition)
                {
                    int times = pBar->get_num_repeats();
                    JumpEntry* pJump = create_jump(measure, jumpToMeasure, times);
                    add_jump(cursor, measure, pJump);
                }
                else if (pBar->get_type() == k_barline_double_repetition
                         || pBar->get_type() == k_barline_double_repetition_alt)
                {
                    int times = pBar->get_num_repeats();
                    JumpEntry* pJump = create_jump(measure, jumpToMeasure, times);
                    add_jump(cursor, measure, pJump);
                    jumpToMeasure = measure+1;
                }

                add_jumps_if_volta_bracket(cursor, pBar, measure);
            }
        }
        else if (pSO->is_time_signature())
        {
            add_rythm_change(measure, static_cast<ImoTimeSignature*>(pSO));
        }
        else if (pSO->is_direction())
        {
            ImoSoundChange* pSound = static_cast<ImoSoundChange*>(
                                          pSO->get_child_of_type(k_imo_sound_change));
            if (pSound)
            {
                int iInstr = cursor.num_instrument();
                int channel = m_channels[iInstr];
                process_sound_change(pSound, cursor, channel, iInstr, measure);
            }
        }
        else if (pSO->is_sound_change())
        {
            ImoSoundChange* pSound = static_cast<ImoSoundChange*>(pSO);
            int iInstr = cursor.num_instrument();
            int channel = m_channels[iInstr];
            process_sound_change(pSound, cursor, channel, iInstr, measure);
        }
        else if (pSO->is_transpose())
        {
            ImoTranspose* pTrp = static_cast<ImoTranspose*>(pSO);
            int iInstr = cursor.num_instrument();
            save_transposition_information(cursor, iInstr, pTrp);
        }

        cursor.move_next();
    }
}
//---------------------------------------------------------------------------------------
void SoundEventsTable::process_sound_change(ImoSoundChange* pSound,
                                            StaffObjsCursor& cursor,
                                            int UNUSED(channel),
                                            int UNUSED(iInstr), int measure)
{
    ImoAttr* pAttr = pSound->get_first_attribute();
    while (pAttr)
    {
        JumpEntry* pJump = nullptr;
        switch (pAttr->get_attrib_idx())
        {
            case k_attr_coda:
                m_targets.push_back(
                    pair<int, string>(measure, string("C" + pAttr->get_string_value())));
                break;

            case k_attr_dacapo:
                pJump = create_jump(measure, 1, 1);   //to measure 1, 1 time
                add_jump(cursor, measure, pJump);
                break;

            case k_attr_dalsegno:
                pJump = create_jump(measure, 0, 1);   //measure unknown, 1 time
                pJump->set_label("S" + pAttr->get_string_value());
                add_jump(cursor, measure, pJump);
                break;

            case k_attr_fine:
                pJump = create_jump(measure, -1, 1, 1);   //to end (-1), valid 1 time, the 2nd time
                add_jump(cursor, measure, pJump);
                break;

            case k_attr_segno:
                m_targets.push_back(
                    pair<int, string>(measure, string("S" + pAttr->get_string_value())));
                break;

            case k_attr_tocoda:
                pJump = create_jump(measure, 0, 1, 1);   //measure unknown, 1 time, the 2nd time
                pJump->set_label("C" + pAttr->get_string_value());
                add_jump(cursor, measure, pJump);
                break;

            case k_attr_dynamics:
                break;
            case k_attr_forward_repeat:
                break;
            case k_attr_time_only:
                break;
            case k_attr_tempo:
                break;
            default:
                break;
        }
        pAttr = pAttr->get_next_attrib();
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_jumps_if_volta_bracket(StaffObjsCursor& cursor,
                                                  ImoBarline* pBar, int measure)
{
    static vector<JumpEntry*> m_pending;
    static int m_iJump = 0;

    if (pBar->get_num_relations() > 0)
    {
        ImoRelations* pRelObjs = pBar->get_relations();
        int size = pRelObjs->get_num_items();
        for (int i=0; i < size; ++i)
        {
            ImoRelObj* pRO = pRelObjs->get_item(i);
            if (pRO->is_volta_bracket())
            {
                ImoVoltaBracket* pVB = static_cast<ImoVoltaBracket*>(pRO);
                if (pBar == pRO->get_start_object())
                {
                    if (pVB->is_first_repeat())
                    {
                        //First volta bracket of a repetition set starts here.
                        //Add all jumps for voltas in this set
                        m_pending.clear();

                        //jump for first volta
                        int times = pVB->get_number_of_repetitions();
                        JumpEntry* pJump = create_jump(measure, measure+1, times);
                        add_jump(cursor, measure, pJump);

                        //jumps for the other voltas in this set
                        int numVoltas = pVB->get_total_voltas();
                        for (int i=2; i <= numVoltas; ++i)
                        {
                            int times = (i == numVoltas ? 0 : 1);
                            pJump = create_jump(measure, 0, times);
                            add_jump(cursor, measure, pJump);
                            m_pending.push_back(pJump);
                        }
                        m_iJump = 0;
                    }
                    else
                    {
                        //volta bracket other than first starts here.
                        //Update:
                        //- measure to jump
                        //- number of repeat times if not last volta
                        JumpEntry* pJump = m_pending[m_iJump];
                        pJump->set_measure(measure+1);
                        if (pJump->get_times_valid() != 0)
                        {
                            int times = pVB->get_number_of_repetitions();
                            pJump->set_times_valid(times);
                        }
                        ++m_iJump;
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::store_event(TimeUnits rTime, int eventType, int channel,
                                   MidiPitch pitch, int volume, int step,
                                   ImoStaffObj* pSO, int measure)
{
    SoundEvent* pEvent = LOMSE_NEW SoundEvent(rTime, eventType, channel, pitch,
                                              volume, step, pSO, measure);
    m_events.push_back(pEvent);
    m_numMeasures = max(m_numMeasures, measure);
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::store_jump_event(TimeUnits rTime, JumpEntry* pJump, int measure)
{
    SoundEvent* pEvent = LOMSE_NEW SoundEvent(rTime, SoundEvent::k_jump, pJump, measure);
    m_events.push_back(pEvent);
    m_numMeasures = max(m_numMeasures, measure);
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_noterest_events(StaffObjsCursor& cursor, int measure)
{
    ImoNoteRest* pNR = static_cast<ImoNoteRest*>( cursor.get_staffobj() );
    ImoTimeSignature* pTS = cursor.get_applicable_time_signature();
    ImoNote* pNote = nullptr;
    int iInstr = cursor.num_instrument();
    int channel = m_channels[iInstr];
    int step = 0;
    int pitch = 0;
    if (pNR->is_note())
    {
        pNote = static_cast<ImoNote*>(pNR);
        int idx = cursor.staff_index();
        pitch = int(pNote->get_midi_pitch()) + m_semitones[idx];
    }

    //AWARE: Visual on/off events are implicit in note on/off events and these
    //implicit events are generated by the score player. Therefore, Visual on/off
    //events are explicitly generated for noterests that do not generate sound
    //(rests, tied notes...)

    //Generate Note ON event
    TimeUnits rTime = pNR->get_playback_time();
    if (pNR->is_note())
    {
        //It is a note. Generate Note On event
        if (!pNote->is_tied_prev())
        {
            //It is not tied to the previous one. Generate NoteOn event to
            //start the sound and highlight the note
            int volume = compute_volume(rTime, pTS, cursor.anacruxis_missing_time());
            store_event(rTime, SoundEvent::k_note_on, channel, pitch,
                        volume, step, pNR, measure);
        }
        else
        {
            //This note is tied to the previous one. Generate only a VisualOn event as the
            //sound is already started by the previous note.
            store_event(rTime, SoundEvent::k_visual_on, channel, pitch,
                        0, step, pNR, measure);
        }
    }
    else
    {
        //it is a rest. Generate only event for visual highlight
        if (pNR->is_visible())
            store_event(rTime, SoundEvent::k_visual_on, channel, 0, 0, 0, pNR, measure);
    }

    //generate NoteOff event
    rTime += pNR->get_playback_duration();
    if (pNR->is_note())
    {
        //It is a note
        if (!pNote->is_tied_next())
        {
            //It is not tied to next note. Generate NoteOff event to stop the sound and
            //un-highlight the note
            store_event(rTime, SoundEvent::k_note_off, channel, pitch,
                        0, step, pNR, measure);
        }
        else
        {
            //This note is tied to the next one. Generate only a VisualOff event so that
            //the note will be un-highlighted but the sound will not be stopped.
            store_event(rTime, SoundEvent::k_visual_off, channel, pitch,
                        0, step, pNR, measure);
        }
    }
    else
    {
        //Is a rest. Generate only a VisualOff event
        if (pNR->is_visible())
            store_event(rTime, SoundEvent::k_visual_off, channel, 0, 0, 0, pNR, measure);
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_rythm_change(int measure, ImoTimeSignature* pTS)
{
    TimeUnits rTime = pTS->get_time();
    int topNumber = pTS->get_top_number();
    int numBeats = pTS->get_num_pulses();
    int beatDuration = int( pTS->get_ref_note_duration() );

    store_event(rTime, SoundEvent::k_rhythm_change, 0, topNumber,
                numBeats, beatDuration, pTS, measure);
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::close_table()
{
    TimeUnits maxTime = 0.0;
    if (m_events.size() > 0)
        maxTime = TimeUnits(m_events.back()->DeltaTime);
    store_event(maxTime, SoundEvent::k_end_of_score, 0, 0, 0, 0, nullptr, 0);
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::create_measures_table()
{
    m_measures.reserve(m_numMeasures+2);          //initial & final control measures
    m_measures.push_back(0);
    for (int i=1; i < m_numMeasures+2; i++)
        m_measures.push_back(-1);

    for (int i=0; i < int(m_events.size()); i++)
    {
        if (m_measures[m_events[i]->Measure] == -1)
        {
            //Add index to the table
            m_measures[m_events[i]->Measure] = i;
        }
    }

    //Item n+1 corresponds to control events after the final bar, normally only
    //the EndOfTable control event.
    m_measures[m_numMeasures+1] = int(m_events.size()) - 1;
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::sort_by_time()
{
    // Sort events by time, measure and event type. Uses the bubble sort algorithm

    int j, k;
    bool fChanges;
    int nNumElements = int(m_events.size());
    SoundEvent* pEvAux;

    for (int i = 0; i < nNumElements; i++)
    {
        fChanges = false;
        j = nNumElements - 1;

        while ( j != i )
        {
            k = j - 1;
            if ((m_events[j]->DeltaTime < m_events[k]->DeltaTime) ||
                ((m_events[j]->DeltaTime == m_events[k]->DeltaTime) &&
                 ((m_events[j]->Measure < m_events[k]->Measure) ||
                  (m_events[j]->EventType < m_events[k]->EventType))))
            {
                //interchange elements
                pEvAux = m_events[j];
                m_events[j] = m_events[k];
                m_events[k] = pEvAux;
                fChanges = true;
            }
            j = k;
        }

        //If there were no changes in this loop step it implies that the table is ordered;
        //in this case exit loop to save time
        if (!fChanges) break;
    }
}

//---------------------------------------------------------------------------------------
string SoundEventsTable::dump_midi_events()
{
    string sMsg = dump_events_table();
    sMsg += dump_measures_table();
    return sMsg;
}

//---------------------------------------------------------------------------------------
string SoundEventsTable::dump_events_table()
{
    if (m_events.size() == 0)
        return "Midi events table is empty";

    //headers
    stringstream msg;
    msg << "Num.\tTime\t\tCh.\tMeas.\tEvent\t\tPitch\tStep\tVolume\n";

        for(int i=0; i < int(m_events.size()); i++)
        {
            //division line every four entries
            if (i % 4 == 0) {
                msg << "-------------------------------------------------------------\n";
            }

            //list current entry
            SoundEvent* pSE = m_events[i];
            msg << i << ":\t" << pSE->DeltaTime << "\t\t" << pSE->Channel << "\t"
                << pSE->Measure << "\t";

            bool fAddData = true;
            switch (pSE->EventType)
            {
                case SoundEvent::k_note_on:
                    msg << "ON        ";
                    break;
                case SoundEvent::k_note_off:
                    msg << "OFF       ";
                    break;
                case SoundEvent::k_visual_on:
                    msg << "VISUAL ON ";
                    break;
                case SoundEvent::k_visual_off:
                    msg << "VISUAL OFF";
                    break;
                case SoundEvent::k_end_of_score:
                    msg << "END TABLE ";
                    break;
                case SoundEvent::k_rhythm_change:
                    msg << "RYTHM CHG ";
                    break;
                case SoundEvent::k_prog_instr:
                    msg << "PRG INSTR ";
                    break;
                case SoundEvent::k_jump:
                    msg << "JUMP      ";
                    msg << pSE->pJump->dump_entry();
                    fAddData = false;
                    break;
                default:
                    msg << "?? " << pSE->EventType;
            }
            if (fAddData)
                msg << "\t" << pSE->NotePitch << "\t" << pSE->NoteStep
                    << "\t" << pSE->Volume << "\n";
        }

    return msg.str();
}

//---------------------------------------------------------------------------------------
string SoundEventsTable::dump_measures_table()
{
    if (m_measures.size() == 0)
        return "Measures table is empty";

    stringstream msg;

    // measures start time table and first event for each measure
    int num = int(m_measures.size()) - 2;
    msg << "\n\nMeasures start times and first event (" << num << " measures)\n\n";
    msg << "Num.\tTime\tEvent\n";
    for(int i=1; i < int(m_measures.size()); i++)
    {
        //division line every four entries
        if (i % 4 == 0)
            msg << "-------------------------------------------------------------\n";

        int nEntry = m_measures[i];
        if (nEntry >= 0)
        {
            SoundEvent* pSE = m_events[nEntry];
            msg << i << ":\t" << pSE->DeltaTime << "\t" << nEntry << "\n";
        }
        else
            msg << i << ":\tEmpty entry\n";
    }

    return msg.str();
}

//---------------------------------------------------------------------------------------
int SoundEventsTable::compute_volume(TimeUnits timePos, ImoTimeSignature* pTS,
                                     TimeUnits timeShift)
{
    // Volume should depend on several factors: beat (strong, medium, weak) on which
    // this note is, phrase, on dynamics information, etc. For now, I'm going to
    // consider only beat information
    //
    // Volume depends on beat (strong, medium, weak) on which the note is placed.

    if (!pTS)
        return 64;

    int pos = get_beat_position(timePos, pTS, timeShift);

    int volume = 60;       // volume for off-beat notes

    if (pos == 0)
        //on-beat notes on first beat
        volume = 85;
    else if (pos > 0)
        //on-beat notes on other beats
        volume = 75;
    else
        // off-beat notes
        volume = 60;

    return volume;
}

//---------------------------------------------------------------------------------------
JumpEntry* SoundEventsTable::get_jump(int i)
{
    if (i < int(m_jumps.size()))
        return m_jumps[i];
    return nullptr;
}

//---------------------------------------------------------------------------------------
JumpEntry* SoundEventsTable::create_jump(int inMeasure, int jumpTo, int timesValid,
                                         int timesBefore)
{
    JumpEntry* pJump = LOMSE_NEW JumpEntry(inMeasure, jumpTo, timesValid, timesBefore);
    m_jumps.push_back(pJump);
    return pJump;
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_jump(StaffObjsCursor& cursor, int measure, JumpEntry* pJump)
{
    ImoStaffObj* pSO = cursor.get_staffobj();
    TimeUnits rTime = pSO->get_time();
    store_jump_event(rTime, pJump, measure);
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::replace_label_in_jumps()
{
    vector<JumpEntry*>::iterator it;
    for (it=m_jumps.begin(); it != m_jumps.end(); ++it)
    {
        int measure = find_measure_for_label( (*it)->get_label() );
        if (measure > 0)
            (*it)->set_measure(measure);
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_events_to_jumps()
{
    vector<JumpEntry*>::iterator it;
    for (it=m_jumps.begin(); it != m_jumps.end(); ++it)
    {
        int measure = (*it)->get_to_measure();
        int nEntry = int(m_events.size() - 1);
        if (measure >= 0)
            nEntry = m_measures[measure];
        (*it)->set_event(nEntry);
    }
}

//---------------------------------------------------------------------------------------
int SoundEventsTable::find_measure_for_label(const string& label)
{
    vector< pair<int, string> >::iterator it;
    for (it=m_targets.begin(); it != m_targets.end(); ++it)
    {
        if ((*it).second == label)
            return (*it).first;
    }
    return 0;
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::reset_jumps()
{
    for(auto it : m_jumps)
        it->reset_entry();
}

//---------------------------------------------------------------------------------------
vector<MeasuresJumpsEntry*> SoundEventsTable::get_measures_jumps()
{
    LOMSE_LOG_DEBUG(Logger::k_mvc, std::string());

    //if already create just return it
    if (m_measuresJumps.size() > 0)
        return m_measuresJumps;

    //traverse the events table as if it were played back, and build the measures jumps table
    size_t maxEvent = m_events.size();
    if (m_events.size() == 0)
    {
        m_measuresJumps.push_back( LOMSE_NEW MeasuresJumpsEntry(0, 0.0, 0, 0.0, 0, 0.0));
        return m_measuresJumps;
    }

    //Execute control m_events that take place before firts play event
    size_t i = 0;
    while ((m_events[i]->EventType == SoundEvent::k_prog_instr)
           || (m_events[i]->EventType == SoundEvent::k_rhythm_change) )
    {
        ++i;
    }

    //Here i points to the first event to play
    //loop to process m_events
    int fromMeasure = m_events[i]->Measure;
    TimeUnits fromTime = TimeUnits(m_events[i]->DeltaTime);
    do
    {
        //if it is a jump event, execute the jump if applicable
        if (m_events[i]->EventType == SoundEvent::k_jump)
        {
            bool fExecuted = false;
            JumpEntry* pJump = m_events[i]->pJump;
            if (pJump->get_visited() >= pJump->get_times_before())
            {
                if (pJump->get_times_valid() == 0
                    || pJump->get_times_valid() > pJump->get_executed())
                {
                    int iCur = i;
                    long curTime =  m_events[iCur]->DeltaTime;     //the jmp entry time
                    i = pJump->get_event();
                    TimeUnits jmpTime = TimeUnits(m_events[i]->DeltaTime);
                    if (pJump->get_times_valid() > pJump->get_executed())
                        pJump->increment_applied();

                    //find previous timepos (cur timepos is jmp entry timepos,
                    //that is, barline timepos, the start of next measure timepos)
                    int j=iCur;
                    while (j > 0 && m_events[j]->DeltaTime == curTime)
                        --j;
                    curTime = m_events[j]->DeltaTime;

                    //create the entry
                    m_measuresJumps.push_back(
                        LOMSE_NEW MeasuresJumpsEntry(fromMeasure, fromTime,
                                                     pJump->get_in_measure(), TimeUnits(curTime),
                                                     int(i), jmpTime) );
                    //save start data
                    fromMeasure = pJump->get_to_measure();
                    fromTime = jmpTime;

                    fExecuted = true;
                }
            }

            pJump->increment_visited();

            if (!fExecuted)
                ++i;

            continue;   //needed if next event is also a jump
        }

        i++;

    } while (i < maxEvent);

    if (fromMeasure != -1)      //-1 = it finished before last measure (e.g. 'Fine' mark)
    {
        TimeUnits curTime = TimeUnits(m_events[maxEvent-2]->DeltaTime);
        m_measuresJumps.push_back(
            LOMSE_NEW MeasuresJumpsEntry(fromMeasure, fromTime, 0, curTime,       //0 = end of score
                                         int(maxEvent-2), curTime) );
    }

    reset_jumps();

    return m_measuresJumps;
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::save_transposition_information(StaffObjsCursor& cursor,
                                                      int iInstr, ImoTranspose* pTrp)
{
    ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
    int numStaves = pInstr->get_num_staves();

    //determine semitones to substract
    int semitones = pTrp->get_chromatic();

    //save semitones
    int iStaff = pTrp->get_applicable_staff();
    if (iStaff == -1)
    {
        for (int i=0; i < numStaves; ++i)
        {
            int idx = cursor.staff_index_for(iInstr, i);
            m_semitones[idx] = semitones;
        }
    }
    else
    {
        int idx = cursor.staff_index_for(iInstr, iStaff);
        m_semitones[idx] = semitones;
    }
}


//=======================================================================================
// JumpEntry implementation
//=======================================================================================
JumpEntry::JumpEntry(int inMeasure, int jumpTo, int timesValid, int timesBefore)
	: m_inMeasure(inMeasure)
	, m_toMeasure(jumpTo)
	, m_timesValid(timesValid)
	, m_timesBefore(timesBefore)
	, m_executed(0)
	, m_visited(0)
	, m_event(0)
{
}

//---------------------------------------------------------------------------------------
JumpEntry::~JumpEntry()
{
}

//---------------------------------------------------------------------------------------
string JumpEntry::dump_entry()
{
    stringstream s;
    s << "Jump: in_m=" << m_inMeasure
      << ", to_m=" << m_toMeasure
      << ", ev=" << m_event
      << ", b=" << m_timesBefore
      << ", v=" << m_timesValid
      << ", vs=" << m_visited
      << ", ex=" << m_executed << endl;
    return s.str();
}


//=======================================================================================
// MeasuresJumpsEntry implementation
//=======================================================================================
MeasuresJumpsEntry::MeasuresJumpsEntry(int fromMeasure, TimeUnits fromTimepos,
                                       int toMeasure, TimeUnits toTimepos,
                                       int jmpEvent, TimeUnits jmpTimepos)
	: m_fromMeasure(fromMeasure)
	, m_fromTimepos(fromTimepos)
	, m_toMeasure(toMeasure)
	, m_toTimepos(toTimepos)
	, m_jmpEvent(jmpEvent)
	, m_jmpTimepos(jmpTimepos)
{
}

//---------------------------------------------------------------------------------------
string MeasuresJumpsEntry::dump_entry()
{
    stringstream s;
    s << "Measures Jump Entry: from_m=" << m_fromMeasure << " (t=" << m_fromTimepos
      << "), to_m=" << m_toMeasure << " (t=" << m_toTimepos
      << "), jmp_ev=" << m_jmpEvent
      << ", jpm_t=" << m_jmpTimepos << endl;
    return s.str();
}



}   //namespace lomse

