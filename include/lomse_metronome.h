//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_METRONOME_H__        //to avoid nested includes
#define __LOMSE_METRONOME_H__

#include "lomse_document.h"         //EBeatDuration
#include "lomse_internal_model.h"   //ENoteDuration


namespace lomse
{

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum defines the duration of one beat, for metronome related methods.

    The metronome default behaviour corresponds to value <i>k_beat_implied</i>. In this
    mode, the note implied a by a metronome beat is is given by the
    time signature, e.g., 4/4 = four beats and each beat is a quarter note; 6/8 = two
    beats and each beat is a dotted eight note; 3/8 = one beat and each beat is a dotted
    eight note, etc.

    But in some cases, you would like to subdivide or to group beats. For instance, to
    subdivide a 6/8 metric, each beat should be an eight note, that is, the value
    implied by the time signature bottom number. The value <i>k_beat_bottom_ts</i>
    means this: to take the beat value from the note implied by the time signature
    bottom number.

    Notice that <i>k_beat_bottom_ts</i> is not useful to subdivide, for instance, a 4/4
    metric, as the value implied by the time signature is ... a quarter note! So, for
    other cases, value <i>k_beat_specified</i> uses as beat duration the note value
    provided by the user.

    @see Metronome::set_beat_type()

    @#include <lomse_metronome.h>
*/
enum EBeatDuration
{
    k_beat_implied = 0,     ///< Implied by the time signature; e.g. 4/4 = four
                            ///< beats, 6/8 = two beats, 3/8 = one beat.
                            ///< The number of implied beats for a time signature is
                            ///< provided by method ImoTimeSignature::num_pulses().
                            ///< Basically, for simple time signatures, such as 4/4,
                            ///< 3/4, 2/4, 3/8, and 2/2, the number of beats is given by
                            ///< the time signature top number, with the exception of
                            ///< 3/8 which is normally conducted in one beat. In compound
                            ///< time signatures (6/x, 12/x, and 9/x) the number of beats
                            ///< is given by dividing the top number by three.

    k_beat_bottom_ts,       ///< Use the note duration implied by the time signature
                            ///< bottom number; e.g. 3/8 = use eighth notes. Notice
                            ///< that the number of beats will coincide with the
                            ///< time signature top number, e.g. 3 beats for 3/8.

    k_beat_specified,       ///< Use specified note value for beat duration.
};


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

    /** Defines the duration for one beat, for metronome and score player related
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

