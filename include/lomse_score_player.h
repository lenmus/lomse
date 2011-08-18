//----------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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
// ScorePlayer: abstract class for implementing score players
class ScorePlayer
{
protected:
    LibraryScope&       m_libScope;
    SoundThread*        m_pThread;      //execution thread
    MidiServerBase*     m_pMidi;        //MIDI server to receive MIDI events
    Interactor*         m_pInteractor;  //Interactor to receive SCORE_HIGHLIGHT events
    bool                m_fPaused;      //execution is paused
    bool                m_fShouldStop;  //request to stop playback
    bool                m_fPlaying;     //playing (control in do_play loop)
    ImoScore*           m_pScore;       //score to play
    SoundEventsTable*   m_pTable;
    int                 m_nMM;          //metronome speed (beats per minute)
    SoundMutex          m_mutex;        //for pause/continue play back
    SoundFlag           m_canPlay;      //playback is not paused

    //metronome: MIDI parameters
    int m_MtrChannel;
    int m_MtrInstr;
    int m_MtrTone1;
    int m_MtrTone2;

public:
    ScorePlayer(LibraryScope& libScope, MidiServerBase* pMidi);
    ~ScorePlayer();

    //construction
    void prepare_to_play(ImoScore* pScore, int nMM=60,
                         int metronomeChannel=9, int metronomeInstr=0,
                         int tone1=60, int tone2=77, Interactor* pInteractor=NULL);

    // playing
    void play(bool fVisualTracking = k_no_visual_tracking,
              bool fCountOff = k_no_countoff,
              int playMode = k_play_normal_instrument,
              long nMM = 0,
              Interactor* pInteractor = NULL );

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
    void wait_for_termination();

    //inline void EndOfThread() { m_pThread = (SoundThreadBase*)NULL; }

    inline bool is_playing() { return m_fPlaying; }

    //only to be used by SoundThread
    void do_play(int nEvStart, int nEvEnd, int playMode, bool fVisualTracking,
                 bool fCountOff, long nMM, Interactor* pInteractor );

protected:
    virtual void play_segment(int nEvStart, int nEvEnd, int playMode,
                              bool fVisualTracking, bool fCountOff, long nMM,
                              Interactor* pInteractor);

    void thread_main(int nEvStart, int nEvEnd, int playMode,
                     bool fVisualTracking, bool fCountOff, long nMM, Interactor* pInteractor);

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
