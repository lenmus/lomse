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
class MyEventOnClick : public EventMouse
{
public:
    MyEventOnClick(ImoContentObj* pImo) : EventMouse(k_on_click_event) { m_pImo = pImo; }
    ~MyEventOnClick() {}
};


//---------------------------------------------------------------------------------------
class MyDocument : public Document
{
public:
    MyDocument(LibraryScope& libraryScope) :  Document(libraryScope) {}
   ~MyDocument() {}

    int my_num_observers() { return  int( m_observers.size() ); }
    std::list<Observer*> my_get_observers() { return m_observers; }
    Observer* my_get_first_observer() { return m_observers.front(); }
};

//---------------------------------------------------------------------------------------
class MyEventHandler
{
protected:
    bool m_fEventReceived;

public:
    MyEventHandler() : m_fEventReceived(false) {}
    ~MyEventHandler() {}

    static void my_on_event_received_wrapper(void* pThis, SpEventInfo pEvent)
    {
        static_cast<MyEventHandler*>(pThis)->my_on_event_received(pEvent);
    }

    void my_on_event_received(SpEventInfo pEvent)
    {
        m_fEventReceived = true;
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
    void handle_event(SpEventInfo pEvent)
    {
        m_fEventReceived = true;
    }

    bool event_received() { return m_fEventReceived; }
};

//---------------------------------------------------------------------------------------
class DocumentEventsTestFixture
{
public:

    LibraryScope m_libraryScope;
    std::string m_scores_path;

    DocumentEventsTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~DocumentEventsTestFixture()    //TearDown fixture
    {
    }
};

SUITE(DocumentEventsTest)
{

    TEST_FIXTURE(DocumentEventsTestFixture, AddHandler_C)
    {
        MyDocument doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        MyEventHandler handler;
        pButton->add_event_handler(k_on_click_event, &handler,
                                   MyEventHandler::my_on_event_received_wrapper);

        std::list<Observer*> observers = doc.my_get_observers();
        CHECK( observers.size() == 1 );
        Observer* pObserver = observers.front();
        CHECK( pObserver->target() == pButton );
    }

    TEST_FIXTURE(DocumentEventsTestFixture, NotifyEvent_C)
    {
        MyDocument doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));
        MyEventHandler handler;
        pButton->add_event_handler(k_on_click_event, &handler,
                                   MyEventHandler::my_on_event_received_wrapper);

        CHECK( handler.event_received() == false );

        Observer* pObs = doc.my_get_first_observer();
        SpEventInfo ev( new MyEventOnClick(pButton) );
        pObs->notify(ev);

        CHECK( handler.event_received() == true );
    }

    TEST_FIXTURE(DocumentEventsTestFixture, AddHandler_CPP)
    {
        MyDocument doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));

        MyEventHandlerCPP handler;
        pButton->add_event_handler(k_on_click_event, &handler);

        std::list<Observer*> observers = doc.my_get_observers();
        CHECK( observers.size() == 1 );
        Observer* pObserver = observers.front();
        CHECK( pObserver->target() == pButton );
    }

    TEST_FIXTURE(DocumentEventsTestFixture, NotifyEvent_CPP)
    {
        MyDocument doc(m_libraryScope);
        doc.create_empty();
        ImoParagraph* pPara = doc.add_paragraph();
        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));
        MyEventHandlerCPP handler;
        pButton->add_event_handler(k_on_click_event, &handler);
        CHECK( handler.event_received() == false );

        Observer* pObs = doc.my_get_first_observer();
        SpEventInfo ev( new MyEventOnClick(pButton) );
        pObs->notify(ev);

        CHECK( handler.event_received() == true );
    }

//    TEST_FIXTURE(DocumentEventsTestFixture, ReplaceHandler)
//    {
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        ImoParagraph* pPara = doc.add_paragraph();
//        ImoButton* pButton = pPara->add_button("Click me", USize(1000.0f, 600.0f));
//
//        MyObserver observer(pButton);
//        MyEventHandler handler;
//        observer.add_handler(k_on_click_event, &handler,
//                             MyEventHandler::my_on_event_received_wrapper);
//        MyEventHandler handler2;
//        observer.add_handler(k_on_click_event, &handler2,
//                             MyEventHandler::my_on_event_received_wrapper);
//
//        CHECK( observer.my_num_observers() == 1 );
//    }

};

