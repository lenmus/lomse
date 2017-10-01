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

#include "lomse_midi_table.h"

#include <algorithm>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_time.h"
#include "lomse_staffobjs_table.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_pitch.h"
#include "lomse_score_utilities.h"

#include <boost/format.hpp>

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
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::program_sounds_for_instruments()
{
	int numInstruments = m_pScore->get_num_instruments();
    m_channels.reserve(numInstruments);

    for (int iInstr = 0; iInstr < numInstruments; iInstr++)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        int channel = pInstr->get_midi_channel();
        m_channels[iInstr] = channel;
        int instr = pInstr->get_midi_program();
        store_event(0, SoundEvent::k_prog_instr, channel, instr, 0, 0, NULL, 0);
    }
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::create_events()
{
    StaffObjsCursor cursor(m_pScore);
    ImoStaffObj* pSO = NULL;
    int measure = 1;        //to count measures (1 based, normal musicians way)
                            //TODO anacruxis measure is counted as 0

    rAnacrusisMissingTime = cursor.anacrusis_missing_time();
    ImoKeySignature* pKey = NULL;
    reset_accidentals(pKey);

    //iterate over the collection to create the MIDI events
    while(!cursor.is_end())
    {
        pSO = cursor.get_staffobj();
        if (pSO->is_note_rest())
        {
            int iInstr = cursor.num_instrument();
            int channel = m_channels[iInstr];
            add_noterest_events(cursor, channel, measure);
        }
        else if (pSO->is_barline())
        {
            measure++;
            reset_accidentals(pKey);
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

        cursor.move_next();
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
void SoundEventsTable::add_noterest_events(StaffObjsCursor& cursor, int channel,
                                           int measure)
{
    ImoStaffObj* pSO = cursor.get_staffobj();
    ImoTimeSignature* pTS = cursor.get_applicable_time_signature();
    ImoNote* pNote = NULL;
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
    TimeUnits rTime = cursor.time() + cursor.anacrusis_missing_time();
    if (pSO->is_rest())
    {
        //it is a rest. Generate only event for visual highlight
        if (pSO->is_visible())
            store_event(rTime, SoundEvent::k_visual_on, channel, 0, 0, 0, pSO, measure);
    }
    else
    {
        //It is a note. Generate Note On event
        if (!pNote->is_tied_prev())
        {
            //It is not tied to the previous one. Generate NoteOn event to
            //start the sound and highlight the note
            int volume = compute_volume(rTime, pTS);
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

    //generate NoteOff event
    rTime += pSO->get_duration();
    if (pSO->is_rest())
    {
        //Is a rest. Generate only a VisualOff event
        if (pSO->is_visible())
            store_event(rTime, SoundEvent::k_visual_off, channel, 0, 0, 0, pSO, measure);
    }
    else
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
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::add_rythm_change(StaffObjsCursor& cursor, int measure,
                                        ImoTimeSignature* pTS)
{
    TimeUnits rTime = cursor.time() + cursor.anacrusis_missing_time();
    int measureDuration = int( pTS->get_measure_duration() );
    int numBeats = pTS->get_num_pulses();

    store_event(rTime, SoundEvent::k_rhythm_change, 0, numBeats,
                measureDuration, 0, pTS, measure);
}

//---------------------------------------------------------------------------------------
void SoundEventsTable::close_table()
{
    TimeUnits maxTime = 0.0;
    if (m_events.size() > 0)
        maxTime = TimeUnits(m_events.back()->DeltaTime);
    store_event(maxTime, SoundEvent::k_end_of_score, 0, 0, 0, 0, NULL, 0);
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
    string msg = "Num.\tTime\t\tCh.\tMeas.\tEvent\t\tPitch\tStep\tVolume\n";

        for(int i=0; i < int(m_events.size()); i++)
        {
            //division line every four entries
            if (i % 4 == 0) {
                msg += "-------------------------------------------------------------\n";
            }

            //list current entry
            SoundEvent* pSE = m_events[i];
            msg += str( boost::format("%4d:\t%d\t\t%d\t%d\t") %
                        i % pSE->DeltaTime % pSE->Channel % pSE->Measure );

            switch (pSE->EventType)
            {
                case SoundEvent::k_note_on:
                    msg += "ON        ";
                    break;
                case SoundEvent::k_note_off:
                    msg += "OFF       ";
                    break;
                case SoundEvent::k_visual_on:
                    msg += "VISUAL ON ";
                    break;
                case SoundEvent::k_visual_off:
                    msg += "VISUAL OFF";
                    break;
                case SoundEvent::k_end_of_score:
                    msg += "END TABLE ";
                    break;
                case SoundEvent::k_rhythm_change:
                    msg += "RITHM CHG ";
                    break;
                case SoundEvent::k_prog_instr:
                    msg += "PRG INSTR ";
                    break;
                default:
                    msg += str( boost::format("?? %d") % pSE->EventType );
            }
            msg += str( boost::format("\t%d\t%d\t%d\n") %
                        pSE->NotePitch % pSE->NoteStep % pSE->Volume );
        }

    return msg;
}

//---------------------------------------------------------------------------------------
string SoundEventsTable::dump_measures_table()
{
    if (m_measures.size() == 0)
        return "Measures table is empty";

    // measures start time table and first event for each measure
    int num = int(m_measures.size()) - 2;
    string msg =
        str( boost::format("\n\nMeasures start times and first event (%d measures)\n\n")
                           % num );
    msg += "Num.\tTime\tEvent\n";
    for(int i=1; i < int(m_measures.size()); i++)
    {
        //division line every four entries
        if (i % 4 == 0)
            msg += "-------------------------------------------------------------\n";

        int nEntry = m_measures[i];
        if (nEntry >= 0)
        {
            SoundEvent* pSE = m_events[nEntry];
            msg += str( boost::format("%4d:\t%d\t%d\n") % i % pSE->DeltaTime % nEntry );
        }
        else
            msg += str( boost::format("%4d:\tEmpty entry\n") % i );
    }

    return msg;
}

//---------------------------------------------------------------------------------------
int SoundEventsTable::compute_volume(TimeUnits timePos, ImoTimeSignature* pTS)
{
    // Volume should depend on several factors: beat (strong, medium, weak) on which
    // this note is, phrase, on dynamics information, etc. For now, I'm going to
    // consider only beat information
    //
    // Volume depends on beat (strong, medium, weak) on which the note is placed.

    if (!pTS)
        return 64;

    int pos = get_beat_position(timePos, pTS);

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
        get_accidentals_for_key(keyType, m_accidentals);
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


}   //namespace lomse

