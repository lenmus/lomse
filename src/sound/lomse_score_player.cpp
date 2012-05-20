//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_score_player.h"

#include "lomse_midi_table.h"
#include "lomse_internal_model.h"
#include "lomse_injectors.h"
#include "lomse_events.h"
#include "lomse_interactor.h"
#include "lomse_player_gui.h"

#include <algorithm>    //max(), min()
#include <ctime>        //clock()


namespace lomse
{

//---------------------------------------------------------------------------------------
ScorePlayer::ScorePlayer(LibraryScope& libScope, MidiServerBase* pMidi)
    : m_libScope(libScope)
    , m_pThread(NULL)
    , m_pMidi(pMidi)
    , m_fPaused(false)
    , m_fShouldStop(false)
    , m_fPlaying(false)
    , m_fPostEvents(true)
    , m_pScore(NULL)
    , m_pTable(NULL)
    , m_MtrChannel(9)
    , m_MtrInstr(0)
    , m_MtrTone1(60)
    , m_MtrTone2(77)
    , m_fVisualTracking(false)
    , m_fCountOff(false)
    , m_playMode(k_play_normal_instrument)
    , m_nMM(60)
    , m_pInteractor(NULL)
    , m_pPlayerGui(NULL)
{
}

//---------------------------------------------------------------------------------------
ScorePlayer::~ScorePlayer()
{
    stop();
}

//---------------------------------------------------------------------------------------
void ScorePlayer::load_score(ImoScore* pScore, PlayerGui* pPlayerGui,
                             int metronomeChannel, int metronomeInstr,
                             int tone1, int tone2)
{
    stop();

    m_pScore = pScore;
    m_pPlayerGui = pPlayerGui;
    m_MtrChannel = metronomeChannel;
    m_MtrInstr = metronomeInstr;
    m_MtrTone1 = tone1;
    m_MtrTone2 = tone2;

    m_pTable = m_pScore->get_midi_table();
}

//---------------------------------------------------------------------------------------
void ScorePlayer::play(bool fVisualTracking, bool fCountOff,
                       int playMode, long nMM, Interactor* pInteractor)
{
	//play all the score

    m_fVisualTracking = fVisualTracking;
    m_fCountOff = fCountOff;
    m_playMode = playMode;
    m_nMM = nMM;
    m_pInteractor = pInteractor;

    int evStart = m_pTable->get_first_event_for_measure(1);
    int evEnd = m_pTable->get_last_event();

    play_segment(evStart, evEnd);
}

//---------------------------------------------------------------------------------------
void ScorePlayer::play_measure(int nMeasure, bool fVisualTracking,
                               int playMode, long nMM, Interactor* pInteractor)
{
    // Play back measure n (n = 1 ... num_measures)

    m_fVisualTracking = fVisualTracking;
    m_fCountOff = false;
    m_playMode = playMode;
    m_nMM = nMM;
    m_pInteractor = pInteractor;

    //remember:
    //   real measures 1..n correspond to table items 1..n
    //   items 0 and n+1 are fictitius measures for pre and post control events
    int evStart = m_pTable->get_first_event_for_measure(nMeasure);
    int evEnd = m_pTable->get_first_event_for_measure(nMeasure + 1) - 1;

    play_segment(evStart, evEnd);
}

//---------------------------------------------------------------------------------------
void ScorePlayer::play_from_measure(int nMeasure, bool fVisualTracking, bool fCountOff,
                                    int playMode, long nMM, Interactor* pInteractor)
{
    // Play back from measure n (n = 1 ... num_measures) to end

    m_fVisualTracking = fVisualTracking;
    m_fCountOff = fCountOff;
    m_playMode = playMode;
    m_nMM = nMM;
    m_pInteractor = pInteractor;

    //remember:
    //   real measures 1..n correspond to table items 1..n
    //   items 0 and n+1 are fictitius measures for pre and post control events
    int nEvStart = m_pTable->get_first_event_for_measure(nMeasure);
    int numMeasures = m_pTable->get_num_measures();
    while (nEvStart == -1 && nMeasure < numMeasures)
    {
        //Current measure is empty. Start in next one
        nEvStart = m_pTable->get_first_event_for_measure(++nMeasure);
    }

    if (nEvStart == -1)
        return;     //all measures are empty after selected one!

    int nEvEnd = m_pTable->get_last_event();

    play_segment(nEvStart, nEvEnd);
}

//---------------------------------------------------------------------------------------
void ScorePlayer::play_segment(int nEvStart, int nEvEnd)
{
    //Create a new thread. It starts inmediately to execute do_play()
    delete m_pThread;
    m_pThread = LOMSE_NEW SoundThread(&ScorePlayer::thread_main, this,
                                nEvStart, nEvEnd, m_playMode, m_fVisualTracking,
                                m_fCountOff, m_nMM, m_pInteractor);
}

//---------------------------------------------------------------------------------------
void ScorePlayer::thread_main(int nEvStart, int nEvEnd, int playMode,
                              bool fVisualTracking, bool fCountOff, long nMM,
                              Interactor* pInteractor)
{
    try
    {
        {
            SoundLock lock(m_mutex);
            m_fPaused = false;
            m_canPlay.notify_one();
        }
        m_fPlaying = true;
        if (pInteractor && !m_fPostEvents)
            pInteractor->enable_view_updates(false);
        fVisualTracking &= (pInteractor != NULL);
        do_play(nEvStart, nEvEnd, playMode, fVisualTracking, fCountOff, nMM, pInteractor);
    }
    catch (boost::thread_interrupted&)
    {
    }

    end_of_playback_housekeeping(fVisualTracking, pInteractor);
}

//---------------------------------------------------------------------------------------
void ScorePlayer::stop()
{
    if (m_pThread)
    {
        ////request the tread to terminate
        //{
        //    SoundLock lock(m_mutex);
        //    m_fShouldStop = true;
        //}

        if (m_fPaused)                  //unlock if paused
            pause();

        if (m_fPlaying)
            m_pThread->interrupt();    //request the tread to terminate

        wait_for_termination();
    }
}

//---------------------------------------------------------------------------------------
void ScorePlayer::pause()
{
    if (!m_pThread) return;

    {
        SoundLock lock(m_mutex);
        m_fPaused = !m_fPaused;
    }
    if (m_fPaused)
        m_pMidi->all_sounds_off();
    else
        m_canPlay.notify_one();
}

//---------------------------------------------------------------------------------------
void ScorePlayer::wait_for_termination()
{
    if (m_pThread)
    {
        m_pThread->join();
        delete m_pThread;
        m_pThread = NULL;
    }
    //m_fShouldStop = false;
    //boost::this_thread::sleep(boost::posix_time::seconds(1)); 
}

//---------------------------------------------------------------------------------------
// Methods to be executed in the thread
//---------------------------------------------------------------------------------------

void ScorePlayer::do_play(int nEvStart, int nEvEnd, int playMode,
                              bool fVisualTracking, bool fCountOff,
                              long nMM, Interactor* pInteractor)
{
    // This is the real method doing the work. It is executed inside a
    // different thread.

    // if no MIDI server terminate or not inside a thread, return
    if (!m_pMidi || !m_pThread)
        return;

//    //TODO All issues related to sol-fa voice

    std::vector<SoundEvent*>& events = m_pTable->get_events();
    if (events.size() == 0)
        return;

    #define k_QUARTER_DURATION  64        //duration (LDP units) of a quarter note (to convert to milliseconds)
    #define k_SOLFA_NOTE        60        //pitch for sight reading with percussion sound
    int nPercussionChannel = m_MtrChannel;        //channel to use for percussion

//    //prepare metronome settings
//TODO    lmMetronome* pMtr = g_pMainFrame->GetMetronome();
    bool fPlayWithMetronome = false;    //TODO pMtr->IsRunning();
    //TODO bool fMetronomeEnabled = pMtr->IsEnabled();
//TODO    pMtr->Enable(false);    //mute sound

    //Prepare instrument for metronome. Instruments for music voices
    //are prepared by events of type ProgInstr
    m_pMidi->program_change(m_MtrChannel, m_MtrInstr);

    //-----------------------------------------------------------------------------------
    //Naming convention for variables:
    //  DeltaTime:  content is LenMus Time Units (TU). One quarte note = 64TU.
    //  Time: content is absolute time (milliseconds)
    //-----------------------------------------------------------------------------------

    //declaration of some time related variables.
    long nEvTime;           //time for next event
    long nMtrEvDeltaTime;   //time for next metronome click

    //default beat and metronome information. It is going to be properly set
    //when a SoundEvent::k_RhythmChange event is found (a time signature object). So these
    //default settings will be used when no time signature in the score.
    m_nMtrPulseDuration = k_QUARTER_DURATION;                     //a beat duration, in TU
    long nMtrIntvalOff = min(7L, m_nMtrPulseDuration / 4L);            //click sound duration, in TU
    long nMtrIntvalNextClick = m_nMtrPulseDuration - nMtrIntvalOff;    //interval from click off to next click
    long nMeasureDuration = m_nMtrPulseDuration * 4;                   //in TU. Assume 4/4 time signature
    long nMtrNumPulses = 4;                                          //assume 4/4 time signature

    //boost::this_thread::disable_interruption di;
    //from this point, interruptions disabled -------------------------------------------

    //Execute control events that take place before the segment to play, so that
    //instruments and tempo are properly programmed. Continue in the loop while
    //we find control events in segment to play.
    int i = 0;
    bool fContinue = true;
    while (fContinue)
    {
        if (events[i]->EventType == SoundEvent::k_prog_instr)
        {
            //change program
            switch (playMode)
            {
                case k_play_rhythm_instrument:
                    m_pMidi->voice_change(events[i]->Channel, 57);        //57 = Trumpet
                    break;
                case k_play_rhythm_percussion:
                    m_pMidi->voice_change(events[i]->Channel, 66);        //66 = High Timbale
                    break;
                case k_play_rhythm_human_voice:
                    //do nothing. Wave sound will be used
                    break;
                case k_play_normal_instrument:
                default:
                    m_pMidi->voice_change(events[i]->Channel, events[i]->Instrument);
            }
        }
        else if (events[i]->EventType == SoundEvent::k_rhythm_change)
        {
            //set up new beat and metronome information
            nMeasureDuration = events[i]->BeatDuration * events[i]->NumBeats;
            nMtrNumPulses = events[i]->NumPulses;
            m_nMtrPulseDuration = nMeasureDuration / nMtrNumPulses;       //a pulse duration, in TU
            nMtrIntvalOff = min(7L, m_nMtrPulseDuration / 4L);            //click sound duration (interval to click off), in TU
            nMtrIntvalNextClick = m_nMtrPulseDuration - nMtrIntvalOff;    //interval from click off to next click, in TU
        }
        else
        {
            // it is not a control event. Continue in the loop only
            // if we have not reached the start of the segment to play
            fContinue = (i < nEvStart);
        }
        if (fContinue) i++;
    }
    //Here i points to the first event of desired measure that is not a control event,
    //that is, to first event to play

    //Define and initialize time counter. If playback starts not at the begining but
	//in another measure, advance time counter to that measure
    long curTime = 0L;
	if (nEvStart > 1)
		curTime = delta_to_milliseconds( events[nEvStart]->DeltaTime );

    // metronome interval duration, in milliseconds
    m_nMtrClickIntval = (nMM == 0 ? 1000L : 60000L/nMM);

    //determine last metronome pulse before first note to play.
    //First note could be syncopated or an off-beat note. Round time to nearest
    //lower pulse time
    nMtrEvDeltaTime = ((events[i]->DeltaTime / m_nMtrPulseDuration) - 1) * m_nMtrPulseDuration;
    curTime = delta_to_milliseconds( nMtrEvDeltaTime );

    //define and prepare highlight event
    SpEventScoreHighlight pEvent(
        LOMSE_NEW EventScoreHighlight(pInteractor, m_pScore->get_id()) );


    bool fFirstBeatInMeasure = true;    //first beat of a measure
    bool fCountOffPulseActive = false;

    //generate count off metronome clicks. Number of pulses will be the necessary
    //pulses before first anacrusis note, or full measure if no anacrusis.
    //At least two pulses.
    bool fMtrOn = false;                //if true, next metronome event is start
    if (fCountOff)
    {
        //determine num pulses
        int numPulses = 0;
        float prevTime = m_pTable->get_anacrusis_missing_time();
        if (is_greater_time(prevTime, 0.0f))
        {
            numPulses = int(prevTime + 0.5f) / m_nMtrPulseDuration;

            //if anacrusis and first event is a rest (real or implicit), add
            //one additional pulse
            bool fAddExtraPulse = false;

            //check for implicit rest
            if (numPulses * m_nMtrPulseDuration < events[i]->DeltaTime)
                fAddExtraPulse = true;  //implicit rest

            //check for real rest
            else
            {
                fAddExtraPulse = true;      //assume real rest
                long time = events[i]->DeltaTime;
                while (events[i]->DeltaTime == time)
                {
                    if (events[i]->pSO && events[i]->pSO->is_note())
                    {
                        fAddExtraPulse = false;
                        break;
                    }
                    ++i;
                }
            }

            if (fAddExtraPulse)
            {
                ++numPulses;
                nMtrEvDeltaTime += m_nMtrPulseDuration;
                curTime = delta_to_milliseconds( nMtrEvDeltaTime );
            }
        }

        //force two pulses at least
        if (numPulses < 2)
            numPulses += nMtrNumPulses;

        //generate the pulses
        for (int j=numPulses; j > 1; --j)
        {
            //generate click
            m_pMidi->note_on(m_MtrChannel, m_MtrTone2, 127);
            boost::posix_time::milliseconds waitTime(m_nMtrClickIntval/2L);
            boost::this_thread::sleep(waitTime);
            m_pMidi->note_off(m_MtrChannel, m_MtrTone2, 127);
            boost::this_thread::sleep(waitTime);
        }
        //generate final click
        m_pMidi->note_on(m_MtrChannel, m_MtrTone1, 127);
        if (fVisualTracking)
        {
            pEvent->add_item(k_advance_tempo_line_event, events[i]->pSO);
            if (nMtrEvDeltaTime < events[i]->DeltaTime)
            {
                //last metronome click is previous to first event from table.
                //send highlight event
                if (m_fPostEvents)
                    m_libScope.post_event(pEvent);
                else if (pInteractor)
                    pInteractor->handle_event(pEvent);
                pEvent = SpEventScoreHighlight(
                            LOMSE_NEW EventScoreHighlight(pInteractor,
                                                          m_pScore->get_id()) );
            }
        }

        fMtrOn = true;
        nMtrEvDeltaTime += nMtrIntvalOff;
        fFirstBeatInMeasure = false;
        fCountOffPulseActive = true;
    }

    //loop to process events
    do
    {
        //Verify if next event is a metronome click
        if (nMtrEvDeltaTime <= events[i]->DeltaTime)
        {
            //Next event should be a metronome click or the click off event for the previous metronome click
            nEvTime = delta_to_milliseconds(nMtrEvDeltaTime);
            if (curTime < nEvTime)
            {
                //flush pending events
                long elapsed = 0L;
                if (fVisualTracking && pEvent->get_num_items() > 0)
                {
                    clock_t t1=clock();
                    if (m_fPostEvents)
                        m_libScope.post_event(pEvent);
                    else if (pInteractor)
                        pInteractor->handle_event(pEvent);
                    pEvent = SpEventScoreHighlight(
                                LOMSE_NEW EventScoreHighlight(pInteractor,
                                                              m_pScore->get_id()) );
                    clock_t t2=clock();
                    elapsed = long( ((t2-t1)*1000.0)/double(CLOCKS_PER_SEC));
                }

                //wait for current time
                long waitT = nEvTime - curTime - elapsed;
                if (waitT > 0L)
                {
                    boost::posix_time::milliseconds waitTime(waitT);
                    boost::this_thread::sleep(waitTime);
                }
                curTime = nEvTime;
            }

            if (fMtrOn)
            {
                //the event is the click off for the previous metronome click
                if (fPlayWithMetronome || fCountOffPulseActive)
                {
                    if (fFirstBeatInMeasure)
                        m_pMidi->note_off(m_MtrChannel, m_MtrTone1, 127);
                    else
                        m_pMidi->note_off(m_MtrChannel, m_MtrTone2, 127);

                    fCountOffPulseActive = false;
                }
                fMtrOn = false;
                nMtrEvDeltaTime += nMtrIntvalNextClick;
            }
            else
            {
                //the event is a metronome click
                fFirstBeatInMeasure = (nMtrEvDeltaTime % nMeasureDuration == 0);
                if (fPlayWithMetronome)
                {
                    if (fFirstBeatInMeasure)
                        m_pMidi->note_on(m_MtrChannel, m_MtrTone1, 127);
                    else
                        m_pMidi->note_on(m_MtrChannel, m_MtrTone2, 127);
                }
                if (fVisualTracking)
                    pEvent->add_item(k_advance_tempo_line_event, events[i]->pSO);

                fMtrOn = true;
                nMtrEvDeltaTime += nMtrIntvalOff;
            }
            curTime = nEvTime;

        }
        else
        {
            //next even comes from the table. Usually it will be a note on/off
            nEvTime = delta_to_milliseconds( events[i]->DeltaTime );
            if (nEvTime > curTime)
            {
                //flush acummulated events for curTime
                long elapsed = 0L;
                if (fVisualTracking && pEvent->get_num_items() > 0)
                {
                    clock_t t1=clock();
                    if (m_fPostEvents)
                        m_libScope.post_event(pEvent);
                    else if (pInteractor)
                        pInteractor->handle_event(pEvent);
                    pEvent = SpEventScoreHighlight(
                                LOMSE_NEW EventScoreHighlight(pInteractor,
                                                              m_pScore->get_id()) );
                    clock_t t2=clock();
                    elapsed = long( ((t2-t1)*1000.0)/double(CLOCKS_PER_SEC));
                }

                //wait until new time arives
                long waitT = nEvTime - curTime - elapsed;
                if (waitT > 0L)
                {
                    boost::posix_time::milliseconds waitTime(waitT);
                    boost::this_thread::sleep(waitTime);
                }
            }

            if (events[i]->EventType == SoundEvent::k_note_on)
            {
                //start of note
                switch(playMode)
                {
                    case k_play_rhythm_instrument:
                        m_pMidi->note_on(events[i]->Channel, k_SOLFA_NOTE,
                                        events[i]->Volume);
                        break;
                    case k_play_rhythm_percussion:
                        m_pMidi->note_on(nPercussionChannel, k_SOLFA_NOTE,
                                        events[i]->Volume);
                        break;
                    case k_play_rhythm_human_voice:
                        //WaveOn .NoteStep, events[i]->Volume);
                        break;
                    case k_play_normal_instrument:
                    default:
                        m_pMidi->note_on(events[i]->Channel, events[i]->NotePitch,
                                        events[i]->Volume);
                }

                if (fVisualTracking && events[i]->pSO->is_visible())
                    pEvent->add_item(k_highlight_on_event, events[i]->pSO);

            }
            else if (events[i]->EventType == SoundEvent::k_note_off)
            {
                //end of note
                switch(playMode)
                {
                    case k_play_rhythm_instrument:
                        m_pMidi->note_off(events[i]->Channel, k_SOLFA_NOTE, 127);
                        break;
                    case k_play_rhythm_percussion:
                        m_pMidi->note_off(nPercussionChannel, k_SOLFA_NOTE, 127);
                        break;
                    case k_play_rhythm_human_voice:
                        //WaveOff
                        break;
                    case k_play_normal_instrument:
                    default:
                        m_pMidi->note_off(events[i]->Channel, events[i]->NotePitch, 127);
                }

                if (fVisualTracking && events[i]->pSO->is_visible())
                    pEvent->add_item(k_highlight_off_event, events[i]->pSO);
            }
            else if (events[i]->EventType == SoundEvent::k_visual_on)
            {
                //set visual highlight
                if (fVisualTracking)
                    pEvent->add_item(k_highlight_on_event, events[i]->pSO);

            }
            else if (events[i]->EventType == SoundEvent::k_visual_off)
            {
                //remove visual highlight
                if (fVisualTracking)
                    pEvent->add_item(k_highlight_off_event, events[i]->pSO);

            }
            else if (events[i]->EventType == SoundEvent::k_end_of_score)
            {
                //end of table
                break;
            }
            else if (events[i]->EventType == SoundEvent::k_rhythm_change)
            {
                //set up new beat and metronome information
                nMeasureDuration = events[i]->BeatDuration * events[i]->NumBeats;
                nMtrNumPulses = events[i]->NumPulses;
                m_nMtrPulseDuration = nMeasureDuration / nMtrNumPulses;        //a pulse duration
                nMtrIntvalOff = min(7L, m_nMtrPulseDuration / 4L);            //click duration (interval to click off)
                nMtrIntvalNextClick = m_nMtrPulseDuration - nMtrIntvalOff;    //interval from click off to next click
            }
            else if (events[i]->EventType == SoundEvent::k_prog_instr)
            {
                //change program
                switch (playMode)
                {
                    case k_play_rhythm_instrument:
                        m_pMidi->voice_change(events[i]->Channel, 57);        //57 = Trumpet
                        break;
                    case k_play_rhythm_percussion:
                        m_pMidi->voice_change(events[i]->Channel, 66);        //66 = High Timbale
                        break;
                    case k_play_rhythm_human_voice:
                        //do nothing. Wave sound will be used
                        break;
                    case k_play_normal_instrument:
                    default:
                        m_pMidi->voice_change(events[i]->Channel, events[i]->NotePitch);
                }
            }
            else
            {
                //program error. Unknown event type. Ignore
                //TODO: log event
            }

            curTime = max(curTime, nEvTime);        //to avoid going backwards when no metronome
                                                //before start and progInstr events
            i++;
        }

        //check if the thread should be paused or stopped
        //boost::this_thread::interruption_point();
        {
            SoundLock lock(m_mutex);
            //if (m_fShouldStop)
            //    break;
            while(m_fPaused)
            {
                //if (m_fShouldStop)
                //    break;
                m_canPlay.wait(lock);
            }
        }

    } while (i <= nEvEnd);

    //ensure that all visual highlight is removed
    if (fVisualTracking)
    {
        SpEventScoreHighlight pEvent(
            LOMSE_NEW EventScoreHighlight(pInteractor, m_pScore->get_id()) );
        pEvent->add_item(k_end_of_higlight_event, NULL);
        if (m_fPostEvents)
            m_libScope.post_event(pEvent);
        else if (pInteractor)
            pInteractor->handle_event(pEvent);
    }
}

//---------------------------------------------------------------------------------------
void ScorePlayer::end_of_playback_housekeeping(bool fVisualTracking,
                                               Interactor* pInteractor)
{
    ////ensure that all visual highlight is removed
    //if (fVisualTracking)
    //{
    //    SpEventScoreHighlight pEvent(
    //        LOMSE_NEW EventScoreHighlight(pInteractor, m_pScore->get_id()) );
    //    pEvent->add_item(k_end_of_higlight_event, NULL);
    //    if (m_fPostEvents)
    //        m_libScope.post_event(pEvent);
    //    else if (pInteractor)
    //        pInteractor->handle_event(pEvent);
    //}

    //ensure that all sounds are off
    m_pMidi->all_sounds_off();

    // allow view updates
    if (pInteractor)
        pInteractor->enable_view_updates(true);

    if (m_pPlayerGui)
    {
        SpEventPlayScore event(
            LOMSE_NEW EventPlayScore(k_end_of_playback_event, pInteractor,
                                     m_pScore, m_pPlayerGui) );
        if (m_fPostEvents)
            m_libScope.post_event(event);
        else if (pInteractor)
            pInteractor->handle_event(event);
    }

    m_fPlaying = false;
}


}   //namespace lomse

