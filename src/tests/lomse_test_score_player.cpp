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

#define LOMSE_INTERNAL_API
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
            boost::this_thread::sleep( boost::posix_time::milliseconds(100) );
        }
        delete m_pThread;
        m_pThread = nullptr;
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
            boost::this_thread::sleep( boost::posix_time::milliseconds(100) );
        }
        delete m_pThread;
        m_pThread = nullptr;
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
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
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
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
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
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
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
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
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
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
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
        //cout << "notifications = " << m_notifications.size() << endl;
        CHECK( int(m_notifications.size()) == 4 );
        std::list<SpEventInfo>::iterator itN = m_notifications.begin();
        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        SpEventInfo evt = *itN;
        SpEventScoreHighlight pEv( static_pointer_cast<EventScoreHighlight>(evt) );
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        std::list< pair<int, ImoId> >& items = pEv->get_items();
        std::list< pair<int, ImoId> >::iterator itItem = items.begin();
        CHECK( (*itItem).first == EventScoreHighlight::k_advance_tempo_line );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);

        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 2);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == EventScoreHighlight::k_advance_tempo_line );
        //cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == EventScoreHighlight::k_highlight_on );
        //cout << "item type: " << (*itItem).first << endl;
        ++itN;

        //cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);
        //cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == EventScoreHighlight::k_end_of_higlight );
        //cout << "item type: " << (*itItem).first << endl;
    }

    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_NoCountoff_HighlightChord)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        pLomse->set_notify_callback(nullptr, MyScorePlayer::my_callback);
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
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

        //cout << "notifications = " << m_notifications.size() << endl;
        CHECK( m_notifications.size() == 4 );
        std::list<SpEventInfo>::iterator itN = m_notifications.begin();
//        cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        SpEventScoreHighlight pEv(
            static_pointer_cast<EventScoreHighlight>(*itN) );
//        cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        std::list< pair<int, ImoId> >& items = pEv->get_items();
        std::list< pair<int, ImoId> >::iterator itItem = items.begin();
        CHECK( (*itItem).first == EventScoreHighlight::k_advance_tempo_line );
//        cout << "item type: " << (*itItem).first << endl;
        ++itN;

//        cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);
//        cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 4);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == EventScoreHighlight::k_advance_tempo_line );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == EventScoreHighlight::k_highlight_on );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == EventScoreHighlight::k_highlight_on );
//        cout << "item type: " << (*itItem).first << endl;
        ++itItem;
        CHECK( (*itItem).first == EventScoreHighlight::k_highlight_on );
//        cout << "item type: " << (*itItem).first << endl;
        ++itN;

//        cout << "notif.type = " << (*itN)->get_event_type() << endl;
        CHECK( (*itN)->get_event_type() == k_highlight_event );
        pEv = static_pointer_cast<EventScoreHighlight>(*itN);
//        cout << "num.items = " << pEv->get_num_items() << endl;
        CHECK( pEv->get_num_items() == 1);
        items = pEv->get_items();
        itItem = items.begin();
        CHECK( (*itItem).first == EventScoreHighlight::k_end_of_higlight );
//        cout << "item type: " << (*itItem).first << endl;
    }

    TEST_FIXTURE(ScorePlayerTestFixture, EndOfPlayEventReceived)
    {
        LomseDoorway* pLomse = m_libraryScope.platform_interface();
        MyEventHandlerCPP2 handler;
        pLomse->set_notify_callback(&handler, MyEventHandlerCPP2::wrapper_for_handler);
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
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


//    TEST_FIXTURE(ScorePlayerTestFixture, DoPlay_Metronme_Highlight)
//    {
//        LomseDoorway* pLomse = m_libraryScope.platform_interface();
//        pLomse->set_notify_callback(MyScorePlayer::my_callback);
//        SpDocument spDoc( new Document(m_libraryScope) );
//        spDoc->from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) )) )))" );
//        ImoScore* pScore = static_cast<ImoScore*>( spDoc->get_im_root()->get_content_item(0) );
//        MyMidiServer midi;
//        MyScorePlayer player(m_libraryScope, &midi);
//        player.load_score(pScore, nullptr);
//        int nEvMax = player.my_get_table()->num_events() - 1;
//        player.my_do_play(0, nEvMax, k_play_normal_instrument, k_do_visual_tracking,
//                          k_no_countoff, 60L, nullptr);
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
//        CHECK( *(itN++) == EventScoreHighlight::k_highlight_on );
//        CHECK( *(itN++) == k_highlight_off_event );
//        CHECK( *(itN++) == k_end_of_higlight_event );
//    }

}


