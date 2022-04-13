//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_config.h"
#if (LOMSE_ENABLE_THREADS == 1)

#define LOMSE_INTERNAL_API
#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_midi_table.h"
#include "private/lomse_document_p.h"
#include "lomse_internal_model.h"
#include "lomse_score_player.h"
#include "lomse_events.h"
#include "lomse_doorway.h"
#include "lomse_interactor.h"
#include "lomse_player_gui.h"

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
    void my_do_play(int nEvStart, int nEvEnd, int UNUSED(playMode), bool fVisualTracking,
                    bool UNUSED(fCountOff), long nMM, Interactor* pInteractor )
    {
        m_fVisualTracking = fVisualTracking;
        m_nMM = nMM;
        m_pInteractor = pInteractor;
        ScorePlayer::play_segment(nEvStart, nEvEnd);
    }

    //overrides
    void play_segment(int UNUSED(nEvStart), int UNUSED(nEvEnd))
    {
        m_fPlaySegmentInvoked = true;
    }

    static void my_callback(void* UNUSED(pThis), SpEventInfo event)
    {
        m_notifications.push_back(event);
    }

    //access to protected members
    void my_wait_for_termination()
    {
        //force to wait until the score if fully played
        while (m_fPlaying)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(100) );
        }
        if (m_pThread && m_pThread->joinable())
        {
            m_pThread->join();
            m_pThread.reset();
        }
        m_pThread = std::unique_ptr<SoundThread>(nullptr);
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    void dump_notifications()
    {
        cout << test_name() << endl;
        cout << "notifications = " << m_notifications.size() << endl;
        list<SpEventInfo>::iterator itN = m_notifications.begin();
        while (itN != m_notifications.end())
        {
            cout << "notif.type = " << (*itN)->get_event_type() << endl;
            ++itN;
        }
        cout << endl;
    }

};

//---------------------------------------------------------------------------------------
//Helper, to define my_wait_for_termination()
class MyScorePlayer2 : public ScorePlayer
{
public:
    MyScorePlayer2(LibraryScope& libScope, MidiServerBase* pMidi)
        : ScorePlayer(libScope, pMidi)
    {
    }

    void my_wait_for_termination()
    {
        //force to wait until the score if fully played
        while (m_fPlaying)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(100) );
        }
        if (m_pThread && m_pThread->joinable())
        {
            m_pThread->join();
            m_pThread.reset();
        }
        m_pThread = std::unique_ptr<SoundThread>(nullptr);
    }
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
    void program_change(int UNUSED(channel), int UNUSED(instr))
    {
        m_events.push_back(k_program_change);
    }
    void voice_change(int UNUSED(channel), int UNUSED(instr))
    {
        m_events.push_back(k_voice_change);
    }
    void note_on(int UNUSED(channel), int UNUSED(pitch), int UNUSED(volume))
    {
        m_events.push_back(k_note_on);
    }
    void note_off(int UNUSED(channel), int UNUSED(pitch), int UNUSED(volume))
    {
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
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ScorePlayerTestFixture()    //TearDown fixture
    {
    }
};

SUITE(ScorePlayerTest)
{

    TEST_FIXTURE(ScorePlayerTestFixture, PrepareToPlay)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
        MidiServerBase midi;
        MyScorePlayer player(m_libraryScope, &midi);
        PlayerNoGui gui;
        player.load_score(pScore, &gui);

        SoundEventsTable* pTable = player.my_get_table();

        CHECK( pTable != nullptr );
        CHECK( pTable->num_events() == 4 );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, PlaySegmentInvoked)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
        MidiServerBase midi;
        MyScorePlayer player(m_libraryScope, &midi);
        PlayerNoGui gui;
        player.load_score(pScore, &gui);
        player.play();
        player.my_wait_for_termination();

        CHECK( player.my_play_segment_invoked() == true );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoMidi)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(nullptr, MyScorePlayer::my_callback);
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
        MyScorePlayer player(m_libraryScope, nullptr);
        PlayerNoGui gui;
        player.load_score(pScore, &gui);
        player.do_play(0, 3, k_no_visual_tracking, 60L, nullptr);

        CHECK( m_notifications.size() == 0 );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoCountoff_NoHighlight)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(nullptr, MyScorePlayer::my_callback);
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
        MyMidiServer midi;
        MyScorePlayer player(m_libraryScope, &midi);
        PlayerNoGui playGui;
        player.load_score(pScore, &playGui);
        int nEvMax = player.my_get_table()->num_events() - 1;
        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_no_visual_tracking,
                          k_no_countoff, 60L, nullptr);
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
        pLomse->set_notify_callback(nullptr, MyScorePlayer::my_callback);
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
        MyMidiServer midi;
        MyScorePlayer player(m_libraryScope, &midi);
        PlayerNoGui playGui;
        player.load_score(pScore, &playGui);
        int nEvMax = player.my_get_table()->num_events() - 1;
        SpInteractor inter( LOMSE_NEW Interactor(m_libraryScope, WpDocument(spDoc), nullptr, nullptr) );
        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_do_visual_tracking,
                          k_no_countoff, 60L, inter.get());
        player.my_wait_for_termination();

        std::list<int>& events = midi.my_get_events();
        std::list<int>::iterator it = events.begin();
        CHECK( events.size() == 5 );
        //cout << "midi events = " << events.size() << endl;
        CHECK( *(it++) == MyMidiServer::k_program_change );
        CHECK( *(it++) == MyMidiServer::k_voice_change );
        CHECK( *(it++) == MyMidiServer::k_note_on );
        CHECK( *(it++) == MyMidiServer::k_note_off );
        CHECK( *(it++) == MyMidiServer::k_all_sounds_off );

        //player.dump_notifications();
        CHECK( int(m_notifications.size()) == 3 );

        //1. move_tempo_line, t=0 + highlight on: note c4 q
        std::list<SpEventInfo>::iterator itN = m_notifications.begin();
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_tracking_event );
        SpEventVisualTracking pEv( static_pointer_cast<EventVisualTracking>(*itN) );
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 2);
        list< pair<int, ImoId> >& items = pEv->get_items();
        list< pair<int, ImoId> >::iterator itItem = items.begin();
        CHECK( (*itItem).first == EventVisualTracking::k_move_tempo_line );
        //cout << "item type: " << (*itItem).first << endl;
        //cout << "timepos = " << pEv->get_timepos() << endl;
        CHECK( pEv->get_timepos() == 0.0f);
        ++itItem;
        CHECK( (*itItem).first == EventVisualTracking::k_highlight_on );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //2. k_end_of_visual_tracking
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_tracking_event );
        pEv = static_pointer_cast<EventVisualTracking>(*itN);
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == EventVisualTracking::k_end_of_visual_tracking );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //3. end_of_playback
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_end_of_playback_event );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoCountoff_HighlightChord)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(nullptr, MyScorePlayer::my_callback);
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)(n g4 q)) )) )))" );
        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
        MyMidiServer midi;
        MyScorePlayer player(m_libraryScope, &midi);
        PlayerNoGui playGui;
        player.load_score(pScore, &playGui);
        int nEvMax = player.my_get_table()->num_events() - 1;
        SpInteractor inter( LOMSE_NEW Interactor(m_libraryScope, WpDocument(spDoc), nullptr, nullptr) );
        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_do_visual_tracking,
                          k_no_countoff, 60L, inter.get());
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

        //player.dump_notifications();
        CHECK( m_notifications.size() == 3 );

        //1. move_tempo_line, t=0 + highlight on: the three notes
        std::list<SpEventInfo>::iterator itN = m_notifications.begin();
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_tracking_event );
        SpEventVisualTracking pEv( static_pointer_cast<EventVisualTracking>(*itN) );
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 4);
        list< pair<int, ImoId> >& items = pEv->get_items();
        list< pair<int, ImoId> >::iterator itItem = items.begin();
        CHECK( (*itItem).first == EventVisualTracking::k_move_tempo_line );
        //cout << "item type: " << (*itItem).first << endl;
        //cout << "timepos = " << pEv->get_timepos() << endl;
        CHECK( pEv->get_timepos() == 0.0f);
        ++itItem;
        CHECK( (*itItem).first == EventVisualTracking::k_highlight_on );
        //cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == EventVisualTracking::k_highlight_on );
        //cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == EventVisualTracking::k_highlight_on );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //2. k_end_of_visual_tracking
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_tracking_event );
        pEv = static_pointer_cast<EventVisualTracking>(*itN);
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == EventVisualTracking::k_end_of_visual_tracking );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //3. end_of_playback
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_end_of_playback_event );
    }

    TEST_FIXTURE(ScorePlayerTestFixture, EndOfPlayEventReceived)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        MyEventHandlerCPP2 handler;
        pLomse->set_notify_callback(&handler, MyEventHandlerCPP2::wrapper_for_handler);
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(chord (n c4 q)(n e4 q)(n g4 q)) )) )))" );
        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
        MyMidiServer midi;
        MyScorePlayer2 player(m_libraryScope, &midi);

        CHECK( handler.event_received() == false );
        PlayerNoGui playGui;
        player.load_score(pScore, &playGui);
        player.play(k_no_visual_tracking, 60L, nullptr);
        player.my_wait_for_termination(); //AWARE: need to wait. Otherwise events arrive *after* CHECKs

        CHECK( handler.event_received() == true );
        CHECK( handler.my_last_event_type() == k_end_of_playback_event );
    }

}

#endif  //LOMSE_ENABLE_THREADS == 1
