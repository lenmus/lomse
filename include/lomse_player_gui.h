//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef _LOMSE_PLAYER_GUI_H__
#define _LOMSE_PLAYER_GUI_H__

namespace lomse
{

//forward declaration
class Metronome;

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
// PlayerGui: Interface for any GUI control for score playback. It will receive
//  end_of_playback events and is responsible for ensuring GUI consistency (i.e.
//  changing the state of "playing now" display or "stop play" button)
class PlayerGui
{
protected:
    PlayerGui() {}

public:
    virtual ~PlayerGui() {}

    //mandatory overrides
    virtual void on_end_of_playback() = 0;
    virtual int get_play_mode() = 0;
    virtual int get_metronome_mm() = 0;
    virtual Metronome* get_metronome() = 0;
    virtual bool countoff_status() = 0;
    virtual bool metronome_status() = 0;

};

//---------------------------------------------------------------------------------------
// PlayerNoGui: a PlayerGui without visible controls
class PlayerNoGui : public PlayerGui
{
protected:
    int m_playMode;
    int m_metronomeMM;
    bool m_countoffStatus;
    bool m_metronomeStatus;

public:
    PlayerNoGui(int metronomeMM=60, bool countoffStatus=false,
                bool metronomeStatus=false, int playMode=k_play_normal_instrument)
        : PlayerGui()
        , m_playMode(playMode)
        , m_metronomeMM(metronomeMM)
        , m_countoffStatus(countoffStatus)
        , m_metronomeStatus(metronomeStatus)
    {
    }
    virtual ~PlayerNoGui() {}

    //mandatory overrides
    void on_end_of_playback() override {}
    int get_play_mode() override { return m_playMode; }
    int get_metronome_mm() override { return m_metronomeMM; }
    Metronome* get_metronome() override { return nullptr; }
    bool countoff_status() override { return m_countoffStatus; }
    bool metronome_status() override { return m_metronomeStatus; }

    //setters
    inline void set_play_mode(int value) { m_playMode = value; };
    inline void set_metronome_mm(int value) { m_metronomeMM = value; };
    inline void countoff_status(bool value) { m_countoffStatus = value; }
    inline void metronome_status(bool value) { m_metronomeStatus = value; }

};


} //namespace lomse

#endif    //_LOMSE_PLAYER_GUI_H__
