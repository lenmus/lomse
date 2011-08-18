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

#ifndef __LOMSE_MIDI_TABLE_H__        //to avoid nested includes
#define __LOMSE_MIDI_TABLE_H__

#include "lomse_pitch.h"
#include "lomse_time.h"

#include <vector>
#include <string>
using namespace std;


namespace lomse
{

//forward declarations
class ImoScore;
class ImoInstrument;
class ImoStaffObj;
class ImoKeySignature;
class ImoTimeSignature;
class ImoNote;
class StaffObjsCursor;


//---------------------------------------------------------------------------------------
//auxiliary class SoundEvent describes a sound event
class SoundEvent
{
public:
    SoundEvent(float rTime, int nEventType, int nChannel,
               MidiPitch midiPitch, int nVolume, int nStep,
               ImoStaffObj* pStaffObj, int nMeasure)
        : DeltaTime(long(rTime + 0.5f))
        , EventType(nEventType)
        , Channel(nChannel)
        , NotePitch(midiPitch)
        , Volume(nVolume)
        , NoteStep(nStep)
        , pSO(pStaffObj)
        , Measure(nMeasure)
    {
    }
    ~SoundEvent() {}

    enum
    {
        // AWARE Event type value is used to sort the events table.
        // MUST keep order for priority
        k_prog_instr = 1,       //program a new instrument
        k_note_off,             //sound off
        k_visual_off,           //remove visual highlight. No effect on sound
        k_rhythm_change,        //change in rithm (time signature)
        k_note_on,              //sound on
        k_visual_on,            //add visual highlight. No effect on sound
        k_end_of_score,         //end of table
    };

    long        DeltaTime;      //Relative to metronome speed
    int         EventType;
    int         Channel;
    union {
        int     NotePitch;      //MIDI pitch
        int     Instrument;     //MIDI instrument for ProgInstr events
        int     NumBeats;       //for RhythmChange events
    };
    union {
        int     Volume;         //for notes
        int     NumPulses;      //For time signatures
    };
    union {
        int     NoteStep;       //Note step 0..6 : 0-Do, 1-Re, ... 6-Si
        int     BeatDuration;   //for RhythmChange events. In LDP duration units
    };
    ImoStaffObj*    pSO;        //staffobj who originated the event (for visual highlight)
    int             Measure;    //measure number containing this staffobj

};

//---------------------------------------------------------------------------------------
//Class SoundEventsTable stores and manages all sound events related to a score
class SoundEventsTable
{
protected:
    ImoScore* m_pScore;
    int m_numMeasures;
    std::vector<SoundEvent*> m_events;
    std::vector<int> m_measures;
    std::vector<int> m_channels;
    float rAnacrusisMissingTime;
    int m_accidentals[7];

public:
    SoundEventsTable(ImoScore* pScore);
    virtual ~SoundEventsTable();

    void create_table();

    inline int num_events() { return int(m_events.size()); }
    std::vector<SoundEvent*>& get_events() { return m_events; }
    std::vector<int>& get_channels() { return m_channels; }
    inline int get_first_event_for_measure(int nMeasure) { return m_measures[nMeasure]; }
    inline int get_last_event() { return int(m_events.size()) - 1; }
    inline int get_num_measures() { return m_numMeasures; }
    inline float get_anacrusis_missing_time() { return rAnacrusisMissingTime; }
    inline bool is_anacrusis_start() { return is_greater_time(rAnacrusisMissingTime, 0.0f); }

    //debug
    string dump_midi_events();


protected:
    void store_event(float rTime, int eventType, int channel, MidiPitch pitch,
                     int volume, int step, ImoStaffObj* pSO, int measure);
    void program_sounds_for_instruments();
    void create_events();
    void close_table();
    void sort_by_time();
    void create_measures_table();
    void add_noterest_events(StaffObjsCursor& cursor, int channel, int measure);
    void add_rythm_change(StaffObjsCursor& cursor, int measure, ImoTimeSignature* pTS);
    void delete_events_table();
    int compute_volume(float timePos, ImoTimeSignature* pTS);
    void reset_accidentals(ImoKeySignature* pKey);
    void update_context_accidentals(ImoNote* pNote);

    //debug
    string dump_events_table();
    string dump_measures_table();
};


}   //namespace lomse

#endif  // __LOMSE_MIDI_TABLE_H__
