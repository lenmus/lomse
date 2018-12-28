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
    , rAnacrusisMissingTime(0.0)
{
}

//---------------------------------------------------------------------------------------
SoundEventsTable::~SoundEventsTable()
{
    delete_events_table();
    m_measures.clear();
    m_channels.clear();
    m_jumps.clear();
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::delete_events_table()
{
    std::vector<SoundEvent*>::iterator it;
    for (it = m_events.begin(); it != m_events.end(); ++it)
        delete *it;
    m_events.clear();
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
    StaffObjsCursor cursor(m_pScore);
    ImoStaffObj* pSO = nullptr;
                            //TODO change so that anacruxis measure is counted as 0
    int jumpToMeasure = 1;

    rAnacrusisMissingTime = cursor.anacrusis_missing_time();
    ImoKeySignature* pKey = nullptr;
    reset_accidentals(pKey);

    //iterate over the collection to create the MIDI events
    while(!cursor.is_end())
    {
        int measure = cursor.measure() + 1;     //start count in 1

        pSO = cursor.get_staffobj();
        if (pSO->is_note_rest())
        {
            int iInstr = cursor.num_instrument();
            int channel = m_channels[iInstr];
            add_noterest_events(cursor, channel, measure);
        }
        else if (pSO->is_barline())
        {
            reset_accidentals(pKey);

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
                    JumpEntry* pJump = create_jump(jumpToMeasure, times);
                    add_jump(cursor, measure, pJump);
                }
                else if (pBar->get_type() == k_barline_double_repetition
                         || pBar->get_type() == k_barline_double_repetition_alt)
                {
                    int times = pBar->get_num_repeats();
                    JumpEntry* pJump = create_jump(jumpToMeasure, times);
                    add_jump(cursor, measure, pJump);
                    jumpToMeasure = measure+1;
                }

                add_jumps_if_volta_bracket(cursor, pBar, measure);
            }
        }
        else if (pSO->is_time_signature())
        {
            add_rythm_change(cursor, measure, static_cast<ImoTimeSignature*>(pSO));
        }
        else if (pSO->is_key_signature())
        {
            pKey = static_cast<ImoKeySignature*>( pSO );
            reset_accidentals(pKey);
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
                pJump = create_jump(1, 1);   //to measure 1, 1 time
                add_jump(cursor, measure, pJump);
                break;

            case k_attr_dalsegno:
                pJump = create_jump(0, 1);   //measure unknown, 1 time
                pJump->set_label("S" + pAttr->get_string_value());
                add_jump(cursor, measure, pJump);
                m_pendingLabel.push_back(pJump);
                break;

            case k_attr_fine:
                pJump = create_jump(-1, 1, 1);   //to end (-1), valid 1 time, the 2nd time
                add_jump(cursor, measure, pJump);
                break;

            case k_attr_segno:
                m_targets.push_back(
                    pair<int, string>(measure, string("S" + pAttr->get_string_value())));
                break;

            case k_attr_tocoda:
                pJump = create_jump(0, 1, 1);   //measure unknown, 1 time, the 2nd time
                pJump->set_label("C" + pAttr->get_string_value());
                add_jump(cursor, measure, pJump);
                m_pendingLabel.push_back(pJump);
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
                        JumpEntry* pJump = create_jump(measure+1, times);
                        add_jump(cursor, measure, pJump);

                        //jumps for the other voltas in this set
                        int numVoltas = pVB->get_total_voltas();
                        for (int i=2; i <= numVoltas; ++i)
                        {
                            int times = (i == numVoltas ? 0 : 1);
                            pJump = create_jump(0, times);
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
void SoundEventsTable::add_noterest_events(StaffObjsCursor& cursor, int channel,
                                           int measure)
{
    ImoStaffObj* pSO = cursor.get_staffobj();
    ImoTimeSignature* pTS = cursor.get_applicable_time_signature();
    ImoNote* pNote = nullptr;
    int step = 0;
    int pitch = 0;
    if (pSO->is_note())
    {
        pNote = static_cast<ImoNote*>(pSO);
        pitch = int(pNote->get_midi_pitch());
    }

    //AWARE: Visual on/off events are implicit in note on/off events and these
    //implicit events are generated by the score player. Therefore, Visual on/off
    //events are explicitly generated for noterests that do not generate sound
    //(rests, tied notes...)

    //Generate Note ON event
    TimeUnits rTime = cursor.time();
    if (pSO->is_note())
    {
        //It is a note. Generate Note On event
        if (!pNote->is_tied_prev())
        {
            //It is not tied to the previous one. Generate NoteOn event to
            //start the sound and highlight the note
            int volume = compute_volume(rTime, pTS, cursor.anacrusis_missing_time());
            store_event(rTime, SoundEvent::k_note_on, channel, pitch,
                        volume, step, pSO, measure);
        }
        else
        {
            //This note is tied to the previous one. Generate only a VisualOn event as the
            //sound is already started by the previous note.
            store_event(rTime, SoundEvent::k_visual_on, channel, pitch,
                        0, step, pSO, measure);
        }
    }
    else
    {
        //it is a rest. Generate only event for visual highlight
        if (pSO->is_visible())
            store_event(rTime, SoundEvent::k_visual_on, channel, 0, 0, 0, pSO, measure);
    }

    //generate NoteOff event
    rTime += pSO->get_duration();
    if (pSO->is_note())
    {
        //It is a note
        if (!pNote->is_tied_next())
        {
            //It is not tied to next note. Generate NoteOff event to stop the sound and
            //un-highlight the note
            store_event(rTime, SoundEvent::k_note_off, channel, pitch,
                        0, step, pSO, measure);
        }
        else
        {
            //This note is tied to the next one. Generate only a VisualOff event so that
            //the note will be un-highlighted but the sound will not be stopped.
            store_event(rTime, SoundEvent::k_visual_off, channel, pitch,
                        0, step, pSO, measure);
        }
    }
    else
    {
        //Is a rest. Generate only a VisualOff event
        if (pSO->is_visible())
            store_event(rTime, SoundEvent::k_visual_off, channel, 0, 0, 0, pSO, measure);
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_rythm_change(StaffObjsCursor& cursor, int measure,
                                        ImoTimeSignature* pTS)
{
    TimeUnits rTime = cursor.time();
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
                    msg << "RITHM CHG ";
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
void SoundEventsTable::reset_accidentals(ImoKeySignature* pKey)
{
    if (pKey)
    {
        int keyType = pKey->get_key_type();
        KeyUtilities::get_accidentals_for_key(keyType, m_accidentals);
    }
    else
    {
        for (int iStep=0; iStep < 7; ++iStep)
            m_accidentals[iStep] = 0;
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::update_context_accidentals(ImoNote* pNote)
{
    int step = pNote->get_step();
    EAccidentals acc = pNote->get_notated_accidentals();
    switch (acc)
    {
        case k_no_accidentals:
            //do not modify context
            break;
        case k_natural:
            //force 'natural' (=no accidentals)
            m_accidentals[step] = 0;
            break;
        case k_flat:
            //lower one semitone
            m_accidentals[step] -= 1;
            break;
        case k_natural_flat:
            //Force one flat
            m_accidentals[step] = -1;
            break;
        case k_sharp:
            //raise one semitone
            m_accidentals[step] += 1;
            break;
        case k_natural_sharp:
            //force one sharp
            m_accidentals[step] = 1;
            break;
        case k_flat_flat:
            //lower two semitones
            m_accidentals[step] -= 2;
            break;
        case k_sharp_sharp:
        case k_double_sharp:
            //raise two semitones
            m_accidentals[step] += 2;
            break;
        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
JumpEntry* SoundEventsTable::get_jump(int i)
{
    if (i < int(m_jumps.size()))
        return m_jumps[i];
    return nullptr;
}

//---------------------------------------------------------------------------------------
JumpEntry* SoundEventsTable::create_jump(int jumpTo, int timesValid, int timesBefore)
{
    JumpEntry* pJump = LOMSE_NEW JumpEntry(jumpTo, timesValid, timesBefore);
    m_jumps.push_back(pJump);
    return pJump;
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_jump(StaffObjsCursor& cursor, int measure, JumpEntry* pJump)
{
    TimeUnits rTime = cursor.time();
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
        int measure = (*it)->get_measure();
        int nEntry = m_events.size() - 1;
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
    vector<JumpEntry*>::iterator it;
    for (it=m_jumps.begin(); it != m_jumps.end(); ++it)
        (*it)->reset_entry();
}


//=======================================================================================
// JumpEntry implementation
//=======================================================================================
JumpEntry::JumpEntry(int jumpTo, int timesValid, int timesBefore)
	: m_measure(jumpTo)
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
    s << "Jump: m=" << m_measure
      << ", ev=" << m_event
      << ", b=" << m_timesBefore
      << ", v=" << m_timesValid
      << ", vs=" << m_visited
      << ", ex=" << m_executed << endl;
    return s.str();
}



}   //namespace lomse

