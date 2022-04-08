//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SCORE_PLAYER_H__        //to avoid nested includes
#define __LOMSE_SCORE_PLAYER_H__

#if (LOMSE_ENABLE_THREADS == 1)

#include "lomse_basic.h"
#include "lomse_internal_model.h"


#include <vector>
#include <thread>
#include <condition_variable>

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class ImoScore;
class ImoStaffObj;
class SoundEventsTable;
class SoundEvent;
class Interactor;
class LibraryScope;
class PlayerGui;
class Metronome;

//some constants for greater code legibility
#define k_no_visual_tracking    false
#define k_do_visual_tracking    true
#define k_no_countoff           false
#define k_do_countoff           true
#define k_no_metronome          false
#define k_play_metronome        true

//---------------------------------------------------------------------------------------
typedef std::thread SoundThread;
typedef std::condition_variable SoundFlag;


//---------------------------------------------------------------------------------------
/** Class %MidiServerBase is a base class defining the interface for any class
    that would like to process the requests from ScorePlayer to generate
    sound, either by generating MIDI messages and passing them to a MIDI synthesizer
    or by other methods.

    Lomse sends sound events directly to your %MidiServerBase derived class just by
    invoking any of the virtual methods. Invocation of these methods is done in real
    time, that is, Lomse will determine the exact time at which a note on / note off
    has to take place, and will invoke the respective method, ``note_on()`` or
    ``note_off()``, at the appropriate time. This implies that your midi server
    implementation is only responsible for generating or stopping sounds when requested,
    and no time computations are needed.

    Lomse does not impose any restriction about how to generate sounds other than
    low latency. Perhaps, the simpler method to generate sounds is to rely on the MIDI
    synthesizer of the PC sound card.

    @attention As playback is a real-time task, your code must return quickly. If
    it needs to do some significant amount of work then you must schedule this work
    asynchronously, for example by posting a windows message, or you should use a
    separate thread. Your application should not retain control for much time as this
    would result in freezing Lomse playback thread.

*/
class MidiServerBase
{
public:
    ///Constructor
    MidiServerBase() {}
    ///Destructor
    virtual ~MidiServerBase() {}


    /** %Request to generate a 'program change' MIDI message for selecting the MIDI
        parameters (channel, program) for metronome clicks,
        or to perform an equivalent action if sound is generated by other methods.

        It is similar to voice_change(), but program_change() is used only for
        metronome, whereas voice_change() is used for score parts (instruments).
    */
    virtual void program_change(int UNUSED(channel), int UNUSED(instr)) {}

    /** %Request to generate a 'program change' MIDI message for selecting the MIDI
        parameters (channel, program) for the different score parts (instruments),
        or to perform an equivalent action if sound is generated by other methods.

        It is similar to program_change() but voice_change() is used for
        score parts (instruments), but program_change() is
        only used for metronome sounds.
    */
    virtual void voice_change(int UNUSED(channel), int UNUSED(instr)) {}

    /** %Request to generate a 'note on' MIDI message,
        or to perform an equivalent action if sound is generated by other methods.
    */
    virtual void note_on(int UNUSED(channel), int UNUSED(pitch), int UNUSED(volume)) {}

    /** %Request to generate a 'note off' MIDI message,
        or to perform an equivalent action if sound is generated by other methods.
    */
    virtual void note_off(int UNUSED(channel), int UNUSED(pitch), int UNUSED(volume)) {}

    /** %Request to generate an 'All Sound Off' MIDI message for muting all sounding
        notes or to perform an equivalent action if sound is generated by other
        methods.
    */
    virtual void all_sounds_off() {}
};


//---------------------------------------------------------------------------------------
/** %ScorePlayer class is responsible for managing score playback.
    It provides the necessary methods for controlling all playback (start, stop, pause,
    etc.).

    The %ScorePlayer constructor method is protected. To create an %ScorePlayer object
    your application will have to request it to Lomse by invoking
    LomseDoorway::create_score_player() method and passing it the MidiServer object
    to use (a class, in your application, derived from MidiServerBase):

    @code
        MyAppMidiServer* pMidi = new MyAppMidiServer();
        ScorePlayer* pPlayer = pLomse->create_score_player(pMidi);
    @endcode

    This can be done only once if your application saves the %ScorePlayer instance in
    a global variable and ensures the appropriate life scope for your @c MyAppMidiServer
    object.

    Once you have the ScorePlayer instance, playback is just loading the score to play
    (by invoking ScorePlayer::load_score() method) and invoking the appropriate methods,
    such as ScorePlayer::play() or ScorePlayer::stop() or ScorePlayer::pause();


	See:
	- @ref page-sound-generation.

*/
class ScorePlayer
{
protected:
    LibraryScope&       m_libScope;
    std::unique_ptr<SoundThread> m_pThread;      //execution thread
    std::mutex          m_startMutex;   //mutex so synchronize thread start
    MidiServerBase*     m_pMidi;        //MIDI server to receive MIDI events
    bool                m_fPaused;      //execution is paused
    bool                m_fRunning;     //method do_play has not finished.
    bool                m_fShouldStop;  //request to stop playback
    bool                m_fPlaying;     //playing (control in do_play loop)
    bool                m_fPostEvents;  //post events to application events loop
    bool                m_fQuit;        //the request to stop is for application quit
    bool                m_fFinalEventSent;      //to avoid duplicating final event
    ImoScore*           m_pScore;       //score to play
    SoundEventsTable*   m_pTable;
    SoundFlag           m_canPlay;      //playback is not paused

    //metronome: MIDI parameters
    int m_MtrChannel;
    int m_MtrInstr;
    int m_MtrTone1;
    int m_MtrTone2;

    //current play parameters
    bool            m_fVisualTracking;
    long            m_nMM;
    Interactor*     m_pInteractor;
    PlayerGui*      m_pPlayerGui;
    Metronome*      m_pMtr;

    friend class Injector;
    ScorePlayer(LibraryScope& libScope, MidiServerBase* pMidi);

public:
    /// Destructor
    virtual ~ScorePlayer();

    /** Load the score to play and set some options. Playback does not start until you
        invoke any of the play methods: play(), play_measure(), etc.
        @param score The score to play.
        @param pPlayerGui
        @param metronomeChannel Midi channel (0..15) to use for metronome clicks.
            Default value is channel 9, normally used for percussion.
        @param metronomeInstr Midi program number (0..127) to use for metronome clicks.
            Default value is program number 0.
        @param tone1 Pitch to use for the first metronome click in each measure. Default
            value is 60. When using channel 9 this value corresponds to the 'Hi Bongo'
            sound.
        @param tone2 Pitch to use for all other metronome clicks in each measure.
            Default value is 77. When using channel 9 this value corresponds to the
            'Low Wood Block' sound.
    */
    void load_score(AScore score, PlayerGui* pPlayerGui,
                    int metronomeChannel=9, int metronomeInstr=0,
                    int tone1=60, int tone2=77);

///@cond INTERNAL
    //TODO: Not possible to deprecate for now. Events still passes a ptr to the score
    //instead of a AScore object
    //LOMSE_DEPRECATED_MSG("use method receiving AScore instead of ptr.to score");
    void load_score(ImoScore* pScore, PlayerGui* pPlayerGui,
                    int metronomeChannel=9, int metronomeInstr=0,
                    int tone1=60, int tone2=77);
///@endcond


    // methods to start playback
    /** @name Methods to start playback   */
    //@{

    /** Start the playback and play until the end of the score.
        Before invoking this method, an score must have been loaded in the player by
        invoking the load_score() method.

        @param fVisualTracking Flag to signal if visual tracking effects
            are desired. Default value is @FALSE (k_no_visual_tracking).
        @param nMM Tempo speed for playback. Value 0 (default) means that tempo will
            be controlled by the metronome control (in PlayerGui object) that
            was specified in method load_score(). A non-zero value for @c nMM forces
            to use that metronome speed. It is expressed in BPM (beats per minute).
        @param pInteractor Pointer to the Interactor associated to the View in which the
            score is displayed or @nullptr in case the score is not displayed.
    */
    void play(bool fVisualTracking = k_no_visual_tracking,
              long nMM = 0,
              Interactor* pInteractor = nullptr);

    /** Start the playback on the indicated measure and play only that measure.
        Before invoking this method, an score must have been loaded in the player by
        invoking the load_score() method.

        @param nMeasure Number of measure to play (1..n).
        @param fVisualTracking Flag to signal if visual tracking effects
            are desired. Default value is @FALSE (k_no_visual_tracking).
        @param nMM Tempo speed for playback. Value 0 (default) means that tempo will
            be controlled by the metronome control (in PlayerGui object) that
            was specified in method load_score(). A non-zero value for @c nMM forces
            to use that metronome speed. It is expressed in BPM (beats per minute).
        @param pInteractor Pointer to the Interactor associated to the View in which the
            score is displayed or @nullptr in case the score is not displayed.
    */
    void play_measure(int nMeasure, bool fVisualTracking = k_no_visual_tracking,
                      long nMM = 0,
                      Interactor* pInteractor = nullptr);

    /** Start the playback on the indicated measure and play until the end of the score.
        Before invoking this method, an score must have been loaded in the player by
        invoking the load_score() method.

        @param nMeasure Number of measure to play (1..n).
        @param fVisualTracking Flag to signal if visual tracking effects
            are desired. Default value is @FALSE (k_no_visual_tracking).
        @param nMM Tempo speed for playback. Value 0 (default) means that tempo will
            be controlled by the metronome control (in PlayerGui object) that
            was specified in method load_score(). A non-zero value for @c nMM forces
            to use that metronome speed. It is expressed in BPM (beats per minute).
        @param pInteractor Pointer to the Interactor associated to the View in which the
            score is displayed or @nullptr in case the score is not displayed.
    */
    void play_from_measure(int nMeasure, bool fVisualTracking = k_no_visual_tracking,
                           long nMM = 0,
						   Interactor* pInteractor = nullptr);

    /** Start the playback on the indicated measure and play only for a certain number
        of measures.
        Before invoking this method, an score must have been loaded in the player by
        invoking the load_score() method.

        @param startMeasure Number of measure to start playback (1..n).
        @param numMeasures Number of measures to play.
        @param fVisualTracking Flag to signal if visual tracking effects
            are desired. Default value is @FALSE (k_no_visual_tracking).
        @param nMM Tempo speed for playback. Value 0 (default) means that tempo will
            be controlled by the metronome control (in PlayerGui object) that
            was specified in method load_score(). A non-zero value for @c nMM forces
            to use that metronome speed. It is expressed in BPM (beats per minute).
        @param pInteractor Pointer to the Interactor associated to the View in which the
            score is displayed or @nullptr in case the score is not displayed.
    */
    void play_measures(int startMeasure, int numMeasures,
                       bool fVisualTracking = k_no_visual_tracking,
                       long nMM = 0,
                       Interactor* pInteractor = nullptr);

    //@}    //Methods to start playback

    /** Finish current playback. To start a new playback you must invoke
        any of the play methods.
    */
    void stop();

    /** Pause/resume current playback. The playback has to be initiated by invoking
        any of the play methods. The first invocation of this method pauses the
        playback. To resume it you must invoke this method again.
    */
    void pause();

    /** Stop the player without generating repaint or other events. Your application
        should invoke this method when it quits, to ensure that the player is stopped
        and that no more processing is needed for finishing.
    */
    void quit();

    /** Returns @TRUE if %ScorePlayer is currently playing back an score, that is, when
        any of the play methods has been invoked but the score has not yet finished and
        methods stop() and pause() have not been invoked.
    */
    inline bool is_playing() { return m_fPlaying; }


///@cond INTERNALS
//excluded from public API. Only for internal use.

    //For selecting method to send events to user application
    inline void post_tracking_events(bool value) { m_fPostEvents = value; }

    //only to be used by SoundThread
    void do_play(int nEvStart, int nEvEnd, bool fVisualTracking,
                 long nMM, Interactor* pInteractor );

///@endcond

protected:
    virtual void play_segment(int nEvStart, int nEvEnd);
    void thread_main(int nEvStart, int nEvEnd, bool fVisualTracking, long nMM,
                     Interactor* pInteractor);
    void end_of_playback_housekeeping(bool fVisualTracking, Interactor* pInteractor);
    void set_new_beat_information(SoundEvent* pEvent);

    //helper, for do_play()
    //-----------------------------------------------------------------------------------
    int m_beatType;     //beat definition to use, from EBeatDuration: k_beat_specified,
                        //  k_beat_implied or k_beat_bottom_ts
    long m_prevGuiBpm;      //last known value of metronome setting in GUI, for detecting
                            // user changes in metronome setting during playback
    long m_nMtrPulseDuration;       //a beat duration, in Time Units
    float m_conversionFactor;       //to convert TimeUnits (delta time) to millisecs

    //current time signature (TS) info
    long m_nCurMeasureDuration;     //current TS: measure duration, in TU
    long m_nCurNumPulses;           //current TS: number of beats per measure
    long m_nCurMtrIntval;           //current TS: metronome click interval, in milliseconds

    //previous TS info, required to adjust metronome clicks interval for maintaining
    //notes duration equivalence when a time signature change.
    long m_nPrevMeasureDuration;    //previous TS: measure duration, in TU
    long m_nPrevNumPulses;          //previous TS: number of beats per measure
    long m_nPrevMtrIntval;          //previous TS: metronome click interval, in milliseconds


    //helper, to conver TimeUnits to milliseconds. Depends on current metronome setting
    inline long time_units_to_milliseconds(long deltaTime) {
        return long( float(deltaTime) * m_conversionFactor );
    }


};


}   //namespace lomse

#enif   //LOMSE_ENABLE_THREADS == 1

#endif  // __LOMSE_SCORE_PLAYER_H__
