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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_midi_table.h"
#include "lomse_document.h"
#include "lomse_internal_model.h"
//#include "lomse_barline_engraver.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
//Helper, to access protected members
class MySoundEventsTable : public SoundEventsTable
{
public:
    MySoundEventsTable(ImoScore* pScore) : SoundEventsTable(pScore) {}
    virtual ~MySoundEventsTable() {}

    void my_program_sounds_for_instruments() { program_sounds_for_instruments(); }
    void my_create_events() { create_events(); }
    void my_close_table() { close_table(); }
    void my_sort_by_time() { sort_by_time(); }

};

//---------------------------------------------------------------------------------------
class MidiTableTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    MidiTableTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~MidiTableTestFixture()    //TearDown fixture
    {
    }
};

SUITE(MidiTableTest)
{

    TEST_FIXTURE(MidiTableTestFixture, ProgramSounds)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();

        //cout << "num.events = " << table.num_events() << endl;
        CHECK( table.num_events() == 1 );
        std::vector<SoundEvent*>& events = table.get_events();
        SoundEvent* ev = events.front();
        CHECK( ev->Channel == 0 );
        CHECK( ev->Instrument == 0 );
        CHECK( ev->EventType == SoundEvent::k_prog_instr );
    }

    TEST_FIXTURE(MidiTableTestFixture, CreateEvents_OneNote)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();

        CHECK( table.num_events() == 3 );
        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_on );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_off );
    }

    TEST_FIXTURE(MidiTableTestFixture, CreateEvents_OneRest)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(r q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();

        //cout << "num.events = " << table.num_events() << endl;
        CHECK( table.num_events() == 3 );
        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_visual_on );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_visual_off );
    }

    TEST_FIXTURE(MidiTableTestFixture, CreateEvents_TwoNotesTied)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q l)(n c4 e) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();

        CHECK( table.num_events() == 5 );
        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_on );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_visual_off );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_visual_on );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_off );
    }

    TEST_FIXTURE(MidiTableTestFixture, BarlineIncrementsMeasureCount)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n c4 e) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();

        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_on );
        CHECK( (*it)->Measure == 1 );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_off );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_on );
        CHECK( (*it)->Measure == 2 );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_off );
    }

    TEST_FIXTURE(MidiTableTestFixture, TimeSignatureAddsRythmChange)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();

        CHECK( table.num_events() == 2 );
        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_rhythm_change );
        CHECK( (*it)->NumBeats == 2 );
        CHECK( (*it)->NumPulses == 2 );
        CHECK( (*it)->BeatDuration == 64 );
        //cout << "NumBeats = " << (*it)->NumBeats 
        //     << ", NumPulses = " << (*it)->NumPulses
        //     << ", BeatDuration = " << (*it)->BeatDuration << endl;
    }

    TEST_FIXTURE(MidiTableTestFixture, TimeSignatureInfoOk)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 6 8) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();

        CHECK( table.num_events() == 2 );
        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_rhythm_change );
        CHECK( (*it)->NumBeats == 6 );
        CHECK( (*it)->NumPulses == 2 );
        CHECK( (*it)->BeatDuration == 32 );
        //cout << "NumBeats = " << (*it)->NumBeats 
        //     << ", NumPulses = " << (*it)->NumPulses
        //     << ", BeatDuration = " << (*it)->BeatDuration << endl;
    }

    TEST_FIXTURE(MidiTableTestFixture, CloseTableAddsEvent)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (r q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_close_table();

        CHECK( table.num_events() == 1 );
        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_end_of_score );
        CHECK( (*it)->DeltaTime == 0.0f );
    }

    TEST_FIXTURE(MidiTableTestFixture, CloseTableFinalTime)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (r q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();
        table.my_close_table();

        CHECK( table.num_events() == 4 );
        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_visual_on );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_visual_off );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_end_of_score );
        CHECK( (*it)->DeltaTime == 64.0f );
    }

    TEST_FIXTURE(MidiTableTestFixture, EventsSorted)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.my_program_sounds_for_instruments();
        table.my_create_events();
        table.my_close_table();
        table.my_sort_by_time();

        std::vector<SoundEvent*>& events = table.get_events();
        std::vector<SoundEvent*>::iterator it = events.begin();
        CHECK( (*it)->EventType == SoundEvent::k_prog_instr );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_on );
        CHECK( (*it)->DeltaTime == 0.0f );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_on );
        CHECK( (*it)->DeltaTime == 0.0f );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_off );
        CHECK( (*it)->DeltaTime == 64.0f );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_note_off );
        CHECK( (*it)->DeltaTime == 64.0f );
        ++it;
        CHECK( (*it)->EventType == SoundEvent::k_end_of_score );
        CHECK( (*it)->DeltaTime == 64.0f );
    }

    TEST_FIXTURE(MidiTableTestFixture, MeasuresTable)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.create_table();

        std::vector<SoundEvent*>& events = table.get_events();

        int iEv = table.get_first_event_for_measure(1);
        CHECK( iEv == 1 );
        CHECK( events[iEv]->EventType == SoundEvent::k_note_on );

        iEv = table.get_last_event();
        CHECK( iEv == 5 );
        CHECK( events[iEv]->EventType == SoundEvent::k_end_of_score );

        CHECK( table.get_num_measures() == 1 );
    }

    TEST_FIXTURE(MidiTableTestFixture, MeasuresTable_ExtraFinalMeasure)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.create_table();

        std::vector<SoundEvent*>& events = table.get_events();

        int iEv = table.get_first_event_for_measure(2);
        CHECK( iEv == 5 );
        CHECK( events[iEv]->EventType == SoundEvent::k_end_of_score );
    }

    TEST_FIXTURE(MidiTableTestFixture, MeasuresTable_InitialControlMeasure)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.create_table();

        std::vector<SoundEvent*>& events = table.get_events();

        int iEv = table.get_first_event_for_measure(0);
        CHECK( iEv == 0 );
        CHECK( events[iEv]->EventType == SoundEvent::k_prog_instr );
    }

    TEST_FIXTURE(MidiTableTestFixture, MeasuresTable_TwoMeasures)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n c4 e) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MySoundEventsTable table(pScore);
        table.create_table();

        std::vector<SoundEvent*>& events = table.get_events();

        int iEv = table.get_first_event_for_measure(0);
        CHECK( iEv == 0 );
        CHECK( events[iEv]->EventType == SoundEvent::k_prog_instr );

        iEv = table.get_first_event_for_measure(1);
        CHECK( iEv == 1 );
        CHECK( events[iEv]->EventType == SoundEvent::k_note_on );

        iEv = table.get_first_event_for_measure(2);
        CHECK( iEv == 3 );
        CHECK( events[iEv]->EventType == SoundEvent::k_note_on );

        iEv = table.get_first_event_for_measure(3);
        CHECK( iEv == 5 );
        CHECK( events[iEv]->EventType == SoundEvent::k_end_of_score );

        iEv = table.get_last_event();
        CHECK( iEv == 5 );
        CHECK( events[iEv]->EventType == SoundEvent::k_end_of_score );

        CHECK( table.get_num_measures() == 2 );
    }

}


