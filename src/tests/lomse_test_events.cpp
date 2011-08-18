//---------------------------------------------------------------------------------------
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
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_internal_model.h"
#include "lomse_events.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class MyEventOnClick : public EventOnClick
{
public:
    MyEventOnClick(ImoObj* pImo) : EventOnClick() { m_pImo = pImo; }
    ~MyEventOnClick() {}
};


//---------------------------------------------------------------------------------------
class MyEventNotifier : public EventNotifier
{
public:
    MyEventNotifier(ImoObj* pImo) : EventNotifier(pImo) {}
    ~MyEventNotifier() {}

    int my_num_handlers() { return  int( m_handlers.size() ); }
};

//---------------------------------------------------------------------------------------
class MyEventHandler
{
protected:
    bool m_fEventReceived;

public:
    MyEventHandler() : m_fEventReceived(false) {}
    ~MyEventHandler() {}

    static void my_on_event_received_wrapper(void* pThis, EventInfo* pEvent)
    {
        static_cast<MyEventHandler*>(pThis)->my_on_event_received(pEvent);
    }

    void my_on_event_received(EventInfo* pEvent)
    {
        m_fEventReceived = true;
        delete pEvent;
    }

    bool event_received() { return m_fEventReceived; }
};

//---------------------------------------------------------------------------------------
class MyEventHandlerCPP : public EventHandler
{
protected:
    bool m_fEventReceived;

public:
    MyEventHandlerCPP() : m_fEventReceived(false) {}
    ~MyEventHandlerCPP() {}

    //mandatory override
    void handle_event(EventInfo* pEvent)
    {
        m_fEventReceived = true;
        delete pEvent;
    }

    bool event_received() { return m_fEventReceived; }
};

//---------------------------------------------------------------------------------------
class EventNotifierTestFixture
{
public:

    LibraryScope m_libraryScope;
    std::string m_scores_path;

    EventNotifierTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~EventNotifierTestFixture()    //TearDown fixture
    {
    }
};

SUITE(EventNotifierTest)
{

    TEST_FIXTURE(EventNotifierTestFixture, DocumentCreatesEventNotifier)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        EventNotifier* pNotifier = doc.get_event_notifier(pButton);

        CHECK( pNotifier != NULL );
        CHECK( pNotifier->target() == pButton );
    }

    TEST_FIXTURE(EventNotifierTestFixture, AddHandler)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        MyEventNotifier notifier(pButton);
        MyEventHandler handler;
        notifier.add_handler(k_on_click_event, &handler,
                             MyEventHandler::my_on_event_received_wrapper);

        CHECK( notifier.my_num_handlers() == 1 );
    }

    TEST_FIXTURE(EventNotifierTestFixture, ReplaceHandler)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        MyEventNotifier notifier(pButton);
        MyEventHandler handler;
        notifier.add_handler(k_on_click_event, &handler,
                             MyEventHandler::my_on_event_received_wrapper);
        MyEventHandler handler2;
        notifier.add_handler(k_on_click_event, &handler2,
                             MyEventHandler::my_on_event_received_wrapper);

        CHECK( notifier.my_num_handlers() == 1 );
    }

    TEST_FIXTURE(EventNotifierTestFixture, NotifyEvent)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        MyEventNotifier notifier(pButton);
        MyEventHandler handler;
        notifier.add_handler(k_on_click_event, &handler,
                             MyEventHandler::my_on_event_received_wrapper);

        CHECK( handler.event_received() == false );

        notifier.notify( new MyEventOnClick(pButton) );

        CHECK( handler.event_received() == true );
    }

    TEST_FIXTURE(EventNotifierTestFixture, AddHandlerCPP)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        MyEventNotifier notifier(pButton);
        MyEventHandlerCPP handler;
        notifier.add_handler(k_on_click_event, &handler);

        CHECK( notifier.my_num_handlers() == 1 );
    }

    TEST_FIXTURE(EventNotifierTestFixture, NotifyEventCPP)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        MyEventNotifier notifier(pButton);
        MyEventHandlerCPP handler;
        notifier.add_handler(k_on_click_event, &handler);

        CHECK( handler.event_received() == false );

        notifier.notify( new MyEventOnClick(pButton) );

        CHECK( handler.event_received() == true );
    }

};

