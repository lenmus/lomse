//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_MIDI_TABLE_H__        //to avoid nested includes
#define __LOMSE_MIDI_TABLE_H__

#include "lomse_pitch.h"
#include "lomse_time.h"

#include <vector>
#include <string>


namespace lomse
{

//forward declarations
class ImoScore;
class ImoInstrument;
class ImoStaffObj;
class ImoKeySignature;
class ImoTimeSignature;
class ImoNote;
class ImoTranspose;
class StaffObjsCursor;
class SoundEvent;
class SoundEventsTable;

//---------------------------------------------------------------------------------------
//JumpsEntry: An entry in the JumpsTable. It describes a jump in playback.
class JumpEntry
{
protected:
	int m_inMeasure;        //number of the measure containing this jump
	int m_toMeasure;        //number of the measure to jump to
	int m_timesValid;   	//num.times the jump must be executed
	int m_timesBefore;      //num.times the jump has to be visited before executing
	int m_executed;         //num.times the jumpTo has been executed
	int m_visited;          //num.times the jumpTo has been visited
	int m_event;            //index to event to jump to
	std::string m_label;         //label to jump to (for To Coda, Dal Segno)

public:

    JumpEntry(int inMeasure, int jumpTo, int timesValid, int timesBefore);
    virtual ~JumpEntry();

	inline void reset_entry() { m_executed = 0; m_visited = 0; }

	inline int get_to_measure() { return m_toMeasure; }
	inline int get_in_measure() { return m_inMeasure; }
	inline int get_times_valid() { return m_timesValid; }
	inline int get_executed() { return m_executed; }
	inline int get_times_before() { return m_timesBefore; }
	inline int get_visited() { return m_visited; }
	inline int get_event() { return m_event; }
	inline std::string& get_label() { return m_label; }

    inline void set_event(int iEvent) { m_event = iEvent; }
    inline void set_measure(int measure) { m_toMeasure = measure; }
    inline void set_times_valid(int times) { m_timesValid = times; }
    inline void increment_applied() { ++m_executed; }
    inline void set_times_before(int times) { m_timesBefore = times; }
    inline void increment_visited() { ++m_visited; }
    inline void set_label(const std::string& label) { m_label = label; }


    //debug
    std::string dump_entry();

};


//---------------------------------------------------------------------------------------
//JumpsEntry: An entry in the MeasuresJumps table. It describes a measure jump in playback.
class MeasuresJumpsEntry
{
protected:
	int m_fromMeasure;          //number of start measure
	TimeUnits m_fromTimepos;    //start timepos
	int m_toMeasure;            //number of last played measure
	TimeUnits m_toTimepos;      //last timepos

	int m_jmpEvent;            //index to event to jump to
	TimeUnits m_jmpTimepos;    //timepos to jump to

public:

    MeasuresJumpsEntry(int fromMeasure, TimeUnits fromTimepos,
                       int toMeasure, TimeUnits toTimepos,
                       int jmpEvent, TimeUnits jmpTimepos);
    virtual ~MeasuresJumpsEntry() = default;

    inline int get_from_measure() { return m_fromMeasure; }
    inline int get_to_measure() { return m_toMeasure; }
    inline TimeUnits get_from_timepos() { return m_fromTimepos; }
    inline TimeUnits get_to_timepos() { return m_toTimepos; }
    inline int get_jmp_event() { return m_jmpEvent; }
    inline TimeUnits get_jmp_timepos() { return m_jmpTimepos; }

    //debug
    std::string dump_entry();

};


//---------------------------------------------------------------------------------------
//auxiliary class SoundEvent describes a sound event
class SoundEvent
{
public:
    SoundEvent(TimeUnits rTime, int nEventType, int nChannel,
               MidiPitch midiPitch, int nVolume, int nStep,
               ImoStaffObj* pStaffObj, int nMeasure)
        : DeltaTime(long(rTime + 0.5f))
        , EventType(nEventType)
        , Channel(nChannel)
        , NotePitch(midiPitch)
        , NoteStep(nStep)
        , Volume(nVolume)
        , pSO(pStaffObj)
        , Measure(nMeasure)
    {
    }
    SoundEvent(TimeUnits rTime, int nEventType, JumpEntry* pJumpEntry, int nMeasure)
        : DeltaTime(long(rTime + 0.5f))
        , EventType(nEventType)
        , Channel(0)
        , NotePitch(0)
        , NoteStep(0)
        , Volume(0)
        , pJump(pJumpEntry)
        , Measure(nMeasure)
    {
    }
    ~SoundEvent() {}

    enum
    {
        // AWARE Event type value is used to sort the events table.
        // MUST keep order for priority
        k_prog_instr = 1,       //program a new sound
        k_note_off,             //sound off
        k_visual_off,           //remove visual highlight. No effect on sound
        k_rhythm_change,        //change in rhythm (time signature)
        k_jump,                 //jump in playback (repetition mark, volta bracket,...)
        k_note_on,              //sound on
        k_visual_on,            //add visual highlight. No effect on sound
        k_end_of_score,         //end of table
    };

    long        DeltaTime;      //Relative to metronome speed
    int         EventType;
    int         Channel;
    union {
        int     NotePitch;      //k_note_xxx: MIDI pitch
        int     Instrument;     //k_prog_instr: MIDI instrument
        int     TopNumber;      //k_rhythm_change: top number of TS
    };
    union {
        int     NoteStep;       //k_note_xxx: Note step 0..6 : 0-Do, ... 6-Si
        int     BeatDuration;   //k_rhythm_change: bottom num. of TS (as duration)
    };
    union {
        int     Volume;         //k_note_xxx: for notes
        int     NumPulses;      //k_rhythm_change: implied number of beats per measure
    };
    union {
        ImoStaffObj*    pSO;        //staffobj who originated the event (for visual highlight)
        JumpEntry*      pJump;      //jump entry, for playback jumps
    };
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
    std::vector<int> m_semitones;       //transposition for each staff
    std::vector<JumpEntry*> m_jumps;
    std::vector<MeasuresJumpsEntry*> m_measuresJumps;
    std::vector< std::pair<int, std::string> > m_targets;          //pair measure, label
    TimeUnits m_rAnacrusisMissingTime;
    TimeUnits m_rAnacrusisExtraTime;


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
    inline TimeUnits get_anacrusis_missing_time() { return m_rAnacrusisMissingTime; }
    inline TimeUnits get_anacrusis_extra_time() { return m_rAnacrusisExtraTime; }

    //jumps table
    inline int num_jumps() { return int(m_jumps.size()); }
    JumpEntry* get_jump(int i);
    void reset_jumps();

    //to create systems jumps
    std::vector<MeasuresJumpsEntry*> get_measures_jumps();

    //debug
    std::string dump_midi_events();


protected:
    void store_event(TimeUnits rTime, int eventType, int channel, MidiPitch pitch,
                     int volume, int step, ImoStaffObj* pSO, int measure);
    void store_jump_event(TimeUnits rTime, JumpEntry* pJump, int measure);
    void program_sounds_for_instruments();
    void create_events();
    void close_table();
    void sort_by_time();
    void create_measures_table();
    void save_transposition_information(StaffObjsCursor& cursor, int iInstr, ImoTranspose* pTrp);
    void add_jumps_if_volta_bracket(StaffObjsCursor& cursor, ImoBarline* pBar,
                                    int measure);
    void add_events_to_jumps();
    void replace_label_in_jumps();
    int find_measure_for_label(const string& label);
    void add_noterest_events(StaffObjsCursor& cursor, int measure);
    void add_rythm_change(int measure, ImoTimeSignature* pTS);
    void add_jump(StaffObjsCursor& cursor, int measure, JumpEntry* pJump);
    void delete_events_table();
    void delete_jumps_table();
    void delete_measures_jumps_table();
    int compute_volume(TimeUnits timePos, ImoTimeSignature* pTS, TimeUnits timeShift);
    JumpEntry* create_jump(int inMeasure, int jumpTo, int timesValid, int timesBefore=0);
    void process_sound_change(ImoSoundChange* pSound, StaffObjsCursor& cursor,
                              int channel, int iInstr, int measure);


    //debug
    std::string dump_events_table();
    std::string dump_measures_table();
};


}   //namespace lomse

#endif  // __LOMSE_MIDI_TABLE_H__
