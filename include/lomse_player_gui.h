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
    virtual void on_end_of_playback() {}
    virtual int get_play_mode() { return m_playMode; }
    virtual int get_metronome_mm() { return m_metronomeMM; }
    virtual Metronome* get_metronome() { return NULL; }
    virtual bool countoff_status() { return m_countoffStatus; }
    virtual bool metronome_status() { return m_metronomeStatus; }

    //setters
    inline void set_play_mode(int value) { m_playMode = value; };
    inline void set_metronome_mm(int value) { m_metronomeMM = value; };
    inline void countoff_status(bool value) { m_countoffStatus = value; }
    inline void metronome_status(bool value) { m_metronomeStatus = value; }

};


} //namespace lomse

#endif    //_LOMSE_PLAYER_GUI_H__
