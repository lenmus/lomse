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
#include "lomse_score_player.h"
#include "lomse_events.h"
#include "lomse_doorway.h"
#include "lomse_interactor.h"

#include <list>


using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
//Helper, to save Highlight events
std::list<SpEventInfo> m_notifications;

//---------------------------------------------------------------------------------------
//Helper, as mock class and for accessing protected members
class MyScorePlayer : public ScorePlayer
{
protected:
    bool m_fPlaySegmentInvoked;

public:
    MyScorePlayer(LibraryScope& libScope, MidiServerBase* pMidi)
        : ScorePlayer(libScope, pMidi)
        , m_fPlaySegmentInvoked(false)
    {
    }
    virtual ~MyScorePlayer() {
        m_notifications.clear();
    }

    //std::vector<SoundEvent*>& my_get_events() { return m_events; }
    SoundEventsTable* my_get_table() { return m_pTable; }
    bool my_play_segment_invoked() { return m_fPlaySegmentInvoked; }
    void my_do_play(int nEvStart, int nEvEnd, int playMode, bool fVisualTracking,
                    bool fCountOff, long nMM, Interactor* pInteractor )
    {
        m_fVisualTracking = fVisualTracking;
        m_fCountOff = fCountOff;
        m_playMode = playMode;
        m_nMM = nMM;
        m_pInteractor = pInteractor;
        ScorePlayer::play_segment(nEvStart, nEvEnd);
    }

    //overrides
    void play_segment(int nEvStart, int nEvEnd)
    {
        m_fPlaySegmentInvoked = true;
    }

    static void my_callback(void* pThis, SpEventInfo event)
    {
        m_notifications.push_back(event);
    }

    void my_wait_for_termination() { wait_for_termination(); }

};

//---------------------------------------------------------------------------------------
//Helper, mock class
class MyMidiServer : public MidiServerBase
{
protected:
    std::list<int> m_events;

public:
    MyMidiServer() : MidiServerBase() {}
    virtual ~MyMidiServer() {
        m_events.clear();
    }

    enum { k_program_change=0, k_voice_change, k_note_on, k_note_off,
           k_all_sounds_off, };

    //overrides
    void program_change(int channel, int instr) {
        m_events.push_back(k_program_change);
    }
    void voice_change(int channel, int instr) {
        m_events.push_back(k_voice_change);
    }
    void note_on(int channel, int pitch, int volume) {
        m_events.push_back(k_note_on);
    }
    void note_off(int channel, int pitch, int volume) {
        m_events.push_back(k_note_off);
    }
    void all_sounds_off() {
        m_events.push_back(k_all_sounds_off);
    }

    std::list<int>& my_get_events() { return m_events; }
};

//---------------------------------------------------------------------------------------
class MyEventHandlerCPP2 : public EventHandler
{
protected:
    bool m_fEventReceived;
    int m_nLastEventType;

public:
    MyEventHandlerCPP2() : m_fEventReceived(false), m_nLastEventType(-1) {}
    ~MyEventHandlerCPP2() {}

    //mandatory override
    void handle_event(SpEventInfo pEvent)
    {
        m_fEventReceived = true;
        m_nLastEventType = pEvent->get_event_type();
    }

    static void wrapper_for_handler(void* pThis, SpEventInfo pEvent) {
        static_cast<MyEventHandlerCPP2*>(pThis)->handle_event(pEvent);
    }

    bool event_received() { return m_fEventReceived; }
    int my_last_event_type() { return m_nLastEventType; }

};


//---------------------------------------------------------------------------------------
class ScorePlayerTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ScorePlayerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~ScorePlayerTestFixture()    //TearDown fixture
    {
    }
};

SUITE(ScorePlayerTest)
{

    TEST_FIXTURE(ScorePlayerTestFixture, PrepareToPlay)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MidiServerBase midi;
        MyScorePlayer player(m_libraryScope, &midi);
        player.prepare_to_play(pScore);

        SoundEventsTable* pTable = player.my_get_table();

        CHECK( pTable != NULL );
        CHECK( pTable->num_events() == 4 );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, PlaySegmentInvoked)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MidiServerBase midi;
        MyScorePlayer player(m_libraryScope, &midi);
        player.prepare_to_play(pScore);
        player.play();
        player.my_wait_for_termination();

        CHECK( player.my_play_segment_invoked() == true );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoMidi)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(NULL, MyScorePlayer::my_callback);
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MyScorePlayer player(m_libraryScope, NULL);
        player.prepare_to_play(pScore);
        player.do_play(0, 3, k_play_normal_instrument, k_no_visual_tracking,
                       k_no_countoff, 60L, NULL);

        CHECK( m_notifications.size() == 0 );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoCountoff_NoHighlight)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(NULL, MyScorePlayer::my_callback);
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MyMidiServer midi;
        MyScorePlayer player(m_libraryScope, &midi);
        player.prepare_to_play(pScore);
        int nEvMax = player.my_get_table()->num_events() - 1;
        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_no_visual_tracking,
                          k_no_countoff, 60L, NULL);
        player.my_wait_for_termination();

        std::list<int>& events = midi.my_get_events();
        std::list<int>::iterator it = events.begin();
        CHECK( events.size() == 5 );
//        cout << "midi events = " << events.size() << endl;
//        cout << *it << endl;
        CHECK( *(it++) == MyMidiServer::k_program_change );
//        cout << *it << endl;
        CHECK( *(it++) == MyMidiServer::k_voice_change );
//        cout << *it << endl;
        CHECK( *(it++) == MyMidiServer::k_note_on );
//        cout << *it << endl;
        CHECK( *(it++) == MyMidiServer::k_note_off );
//        cout << *it << endl;
        CHECK( *(it++) == MyMidiServer::k_all_sounds_off );
        CHECK( m_notifications.size() == 1 );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoCountoff_HighlightNote)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(NULL, MyScorePlayer::my_callback);
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MyMidiServer midi;
        MyScorePlayer player(m_libraryScope, &midi);
        player.prepare_to_play(pScore);
        int nEvMax = player.my_get_table()->num_events() - 1;
        Interactor inter(m_libraryScope, &doc, NULL);
        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_visual_tracking,
                          k_no_countoff, 60L, &inter);
        player.my_wait_for_termination();

        std::list<int>& events = midi.my_get_events();
        std::list<int>::iterator it = events.begin();
        CHECK( events.size() == 5 );
//        cout << "midi events = " << events.size() << endl;
        CHECK( *(it++) == MyMidiServer::k_program_change );
        CHECK( *(it++) == MyMidiServer::k_voice_change );
        CHECK( *(it++) == MyMidiServer::k_note_on );
        CHECK( *(it++) == MyMidiServer::k_note_off );
        CHECK( *(it++) == MyMidiServer::k_all_sounds_off );
        //cout << "notifications = " << m_notifications.size() << endl;
        CHECK( int(m_notifications.size()) == 4 );
        std::list<SpEventInfo>::iterator itN = m_notifications.begin();
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        SpEventInfo evt = *itN;
        SpEventScoreHighlight pEv( static_pointer_cast<EventScoreHighlight>(evt) );
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        std::list< pair<int, ImoStaffObj*> >& items = pEv->get_items();
        std::list< pair<int, ImoStaffObj*> >::iterator itItem = items.begin();
        CHECK( (*itItem).first == k_advance_tempo_line_event );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);

        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 2);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == k_advance_tempo_line_event );
        //cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_on_event );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 3);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == k_advance_tempo_line_event );
        //cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_off_event );
        //cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_end_of_higlight_event );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoCountoff_HighlightChord)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(NULL, MyScorePlayer::my_callback);
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)(n g4 q)) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MyMidiServer midi;
        MyScorePlayer player(m_libraryScope, &midi);
        player.prepare_to_play(pScore);
        int nEvMax = player.my_get_table()->num_events() - 1;
        Interactor inter(m_libraryScope, &doc, NULL);
        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_visual_tracking,
                          k_no_countoff, 60L, &inter);
        player.my_wait_for_termination();

        std::list<int>& events = midi.my_get_events();
        std::list<int>::iterator it = events.begin();
        CHECK( events.size() == 9 );
//        cout << "midi events = " << events.size() << endl;
        CHECK( *(it++) == MyMidiServer::k_program_change );
        CHECK( *(it++) == MyMidiServer::k_voice_change );
        CHECK( *(it++) == MyMidiServer::k_note_on );
        CHECK( *(it++) == MyMidiServer::k_note_on );
        CHECK( *(it++) == MyMidiServer::k_note_on );
        CHECK( *(it++) == MyMidiServer::k_note_off );
        CHECK( *(it++) == MyMidiServer::k_note_off );
        CHECK( *(it++) == MyMidiServer::k_note_off );
        CHECK( *(it++) == MyMidiServer::k_all_sounds_off );

        //cout << "notifications = " << m_notifications.size() << endl;
        CHECK( m_notifications.size() == 4 );
        std::list<SpEventInfo>::iterator itN = m_notifications.begin();
//        cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        SpEventScoreHighlight pEv(
            static_pointer_cast<EventScoreHighlight>(*itN) );
//        cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        std::list< pair<int, ImoStaffObj*> >& items = pEv->get_items();
        std::list< pair<int, ImoStaffObj*> >::iterator itItem = items.begin();
        CHECK( (*itItem).first == k_advance_tempo_line_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itN;

//        cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);
//        cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 4);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == k_advance_tempo_line_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_on_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_on_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_on_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itN;

//        cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);
//        cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 5);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == k_advance_tempo_line_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_off_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_off_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_highlight_off_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == k_end_of_higlight_event );
//        cout << "item type: " << (*itItem).first << endl;
        ++itN;
    }

    TEST_FIXTURE(ScorePlayerTestFixture, EndOfPlayEventReceived)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        MyEventHandlerCPP2 handler;
        pLomse->set_notify_callback(&handler, MyEventHandlerCPP2::wrapper_for_handler);
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)(n g4 q)) )) )))" );
        ImoScore* pScore = doc.get_score(0);
        MyMidiServer midi;
        ScorePlayer player(m_libraryScope, &midi);

        CHECK( handler.event_received() == false );
        player.prepare_to_play(pScore);
        player.play(k_no_visual_tracking, k_no_countoff, k_play_normal_instrument,
                    60L, NULL);
        //player.my_wait_for_termination();

        CHECK( handler.event_received() == true );
        CHECK( handler.my_last_event_type() == k_end_of_playback_event );
    }


//    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_Metronme_Highlight)
//    {
//        LomseDoorway* pLomse = m_libraryScope.platform_interface();
//        pLomse->set_notify_callback(MyScorePlayer::my_callback);
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) )) )))" );
//        ImoScore* pScore = doc.get_score(0);
//        MyMidiServer midi;
//        MyScorePlayer player(m_libraryScope, &midi);
//        player.prepare_to_play(pScore);
//        int nEvMax = player.my_get_table()->num_events() - 1;
//        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_visual_tracking,
//                          k_no_countoff, 60L, NULL);
//
//        std::list<int>& events = midi.my_get_events();
//        std::list<int>::iterator it = events.begin();
//        CHECK( events.size() == 5 );
////        cout << "midi events = " << events.size() << endl;
//        CHECK( *(it++) == MyMidiServer::k_program_change );
//        CHECK( *(it++) == MyMidiServer::k_voice_change );
//        CHECK( *(it++) == MyMidiServer::k_note_on );
//        CHECK( *(it++) == MyMidiServer::k_note_off );
//        CHECK( *(it++) == MyMidiServer::k_all_sounds_off );
////        cout << "notifications = " << m_notifications.size() << endl;
//        CHECK( m_notifications.size() == 4 );
//        std::list<int>::iterator itN = m_notifications.begin();
////        cout << "notif.type = " << *itN << endl;
//        CHECK( *(itN++) == k_prepare_for_highlight_event );
//        CHECK( *(itN++) == k_highlight_on_event );
//        CHECK( *(itN++) == k_highlight_off_event );
//        CHECK( *(itN++) == k_end_of_higlight_event );
//    }

}


