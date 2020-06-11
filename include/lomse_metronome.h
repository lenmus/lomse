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

#ifndef __LOMSE_METRONOME_H__        //to avoid nested includes
#define __LOMSE_METRONOME_H__

#include "lomse_document.h"         //EBeatDuration
#include "lomse_internal_model.h"   //ENoteDuration


namespace lomse
{

//---------------------------------------------------------------------------------------
/** Abstract class defining the interface for any Metronome
*/
class Metronome
{
protected:
    long        m_nMM;          //metronome frequency: beats per minute
    long        m_nInterval;    //metronome period: milliseconds between beats
    bool        m_fMuted;       //metronome sound is muted
    bool        m_fRunning;     //true if start() invoked
    int         m_beatType;
    TimeUnits   m_beatDuration;


    Metronome(long nMM)
        : m_fMuted(false)
        , m_fRunning(false)
        , m_beatType(k_beat_implied)
        , m_beatDuration(k_duration_quarter)
    {
        set_mm(nMM);
    }

public:
    virtual ~Metronome() {}

    // setting speed. Two options:
    void set_mm(long nMM)
    {
        m_nInterval = (60000 / nMM);
        m_nMM = nMM;
    }
    void set_interval(long milliseconds)
    {
        m_nInterval = milliseconds;
        m_nMM = (long)((60000.0 / (float)milliseconds)+ 0.5);;
    }

    //setting beat type

    /** Define the duration for one beat, for metronome and score player related
        methods. Changes while an score is being played back are ignored until
        playback finishes.
        @param type A value from enum #EBeatDuration.
        @param duration The duration (in Lomse Time Units) for one beat. You can use
            a value from enum ENoteDuration casted to double. This parameter is
            required only when value for parameter `beatType` is `k_beat_specified`.
            For all other values, if a non-zero value is specified, the value
            will be used for the beat duration in scores without time signature.
    */
    inline void set_beat_type(int type, TimeUnits duration=0.0)
    {
        m_beatType = type;
        m_beatDuration = duration;
    }


    // accessors
    inline long get_mm() { return m_nMM; }
    inline long get_interval() { return m_nInterval; }
    inline bool is_muted() { return m_fMuted; }
    inline bool is_running() { return m_fRunning; }

    /** Return the beat type to use for scores in this document, for metronome and
        score player related methods.

        See set_beat_type()
    */
    inline int get_beat_type() { return m_beatType; }

    /** Return the duration for beats, for metronome and score player related
        methods, to use in scores without time signature and when
        beat type is `k_beat__specified`.

        See set_beat_type()
    */
    inline TimeUnits get_beat_duration() { return m_beatDuration; }

    // commands
    virtual void start() = 0;
    virtual void stop() = 0;
    inline void mute(bool value) { m_fMuted = value; }
};


}   // namespace lenmus

#endif    // __LOMSE_METRONOME_H__

