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

#ifndef __LOMSE_SCORE_PLAYER_H__        //to avoid nested includes
#define __LOMSE_SCORE_PLAYER_H__


#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
using namespace boost;

namespace lomse
{

//forward declarations
class ImoScore;
class ImoStaffObj;
class SoundEventsTable;
class SoundEvent;
class Interactor;
class LibraryScope;
class PlayerGui;

//some constants for greater code legibility
#define k_no_visual_tracking    false
#define k_visual_tracking       true
#define k_no_countoff           false
#define k_countoff              true

//---------------------------------------------------------------------------------------
typedef boost::thread SoundThread;
typedef boost::mutex SoundMutex;
typedef boost::unique_lock<boost::mutex> SoundLock;
typedef boost::condition_variable SoundFlag;

//---------------------------------------------------------------------------------------
// play modes
enum
{
    k_play_normal_instrument = 0,   //normal, (pitched instrument)
    k_play_rhythm_instrument,       //only rhythm (instrument, single pitch)
    k_play_rhythm_percussion,       //only rhythm (percussion, single pitch)
    k_play_rhythm_human_voice,      //only rhythm (solfege, human voice)
};


//---------------------------------------------------------------------------------------
// MidiServerBase: base class defining the interface for processing MIDI events
class MidiServerBase
{
public:
    MidiServerBase() {}
    virtual ~MidiServerBase() {}

    virtual void program_change(int channel, int instr) {}
    virtual void voice_change(int channel, int instr) {}
    virtual void note_on(int channel, int pitch, int volume) {}
    virtual void note_off(int channel, int pitch, int volume) {}
    virtual void all_sounds_off() {}
};


//---------------------------------------------------------------------------------------
// ScorePlayer: reponsible for score playback
class ScorePlayer
{
protected:
    LibraryScope&       m_libScope;
    SoundThread*        m_pThread;      //execution thread
    MidiServerBase*     m_pMidi;        //MIDI server to receive MIDI events
    bool                m_fPaused;      //execution is paused
    bool                m_fShouldStop;  //request to stop playback
    bool                m_fPlaying;     //playing (control in do_play loop)
    bool                m_fPostEvents;  //post events to application events loop
    ImoScore*           m_pScore;       //score to play
    SoundEventsTable*   m_pTable;
    SoundMutex          m_mutex;        //for pause/continue play back
    SoundFlag           m_canPlay;      //playback is not paused

    //metronome: MIDI parameters
    int m_MtrChannel;
    int m_MtrInstr;
    int m_MtrTone1;
    int m_MtrTone2;

    //current play parameters
    bool            m_fVisualTracking;
    bool            m_fCountOff;
    int             m_playMode;
    long            m_nMM;
    Interactor*     m_pInteractor;
    PlayerGui*      m_pPlayerGui;

    friend class Injector;
    ScorePlayer(LibraryScope& libScope, MidiServerBase* pMidi);

public:
    virtual ~ScorePlayer();

    //construction
    void load_score(ImoScore* pScore, PlayerGui* pPlayerGui,
                    int metronomeChannel=9, int metronomeInstr=0,
                    int tone1=60, int tone2=77);
    inline void post_highlight_events(bool value) { m_fPostEvents = value; }

    // playing
    void play(bool fVisualTracking = k_no_visual_tracking,
              bool fCountOff = k_no_countoff,
              int playMode = k_play_normal_instrument,
              long nMM = 0,
              Interactor* pInteractor = NULL);

    void play_measure(int nMeasure, bool fVisualTracking = k_no_visual_tracking,
                      int playMode = k_play_normal_instrument,
                      long nMM = 0,
                      Interactor* pInteractor = NULL);

    void play_from_measure(int nMeasure, bool fVisualTracking = k_no_visual_tracking,
                           bool fCountOff = k_no_countoff,
                           int playMode = k_play_normal_instrument,
                           long nMM = 0,
						   Interactor* pInteractor = NULL);
    void stop();
    void pause();

    inline bool is_playing() { return m_fPlaying; }

    //only to be used by SoundThread
    void do_play(int nEvStart, int nEvEnd, int playMode, bool fVisualTracking,
                 bool fCountOff, long nMM, Interactor* pInteractor );

protected:
    virtual void play_segment(int nEvStart, int nEvEnd);
    void thread_main(int nEvStart, int nEvEnd, int playMode,
                     bool fVisualTracking, bool fCountOff, long nMM,
                     Interactor* pInteractor);
    void wait_for_termination();
    void end_of_playback_housekeeping(bool fVisualTracking, Interactor* pInteractor);

    //helper, for do_play()
    //-----------------------------------------------------------------------------------
    long m_nMtrClickIntval;     //metronome interval duration, in milliseconds
    long m_nMtrPulseDuration;   //a beat duration, in DeltaTime units
//    long nMtrIntvalOff = min(7L, nMtrPulseDuration / 4L);            //click sound duration, in TU
//    long nMtrIntvalNextClick = nMtrPulseDuration - nMtrIntvalOff;    //interval from click off to next click
//    long nMeasureDuration = nMtrPulseDuration * 4;                   //in TU. Assume 4/4 time signature
//    long nMtrNumPulses = 4;                                          //assume 4/4 time signature

    inline long delta_to_milliseconds(long deltaTime) {
        return deltaTime * m_nMtrClickIntval / m_nMtrPulseDuration;
    }



};


}   //namespace lomse

#endif  // __LOMSE_SCORE_PLAYER_H__
