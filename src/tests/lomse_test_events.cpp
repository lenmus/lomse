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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_internal_model.h"
#include "lomse_events.h"
#include "lomse_hyperlink_ctrl.h"
#include "lomse_button_ctrl.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class MyEventOnClick : public EventMouse
{
public:
    MyEventOnClick(ImoContentObj* pImo, WpDocument wpDoc)
        : EventMouse(k_on_click_event)
    {
        m_wpDoc = wpDoc;
        m_imoId = pImo->get_id();
    }
    MyEventOnClick(Control* pCtrl, WpDocument wpDoc)
        : EventMouse(k_on_click_event)
    {
        m_wpDoc = wpDoc;
        m_imoId = pCtrl->get_owner_imo_id();
    }
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

    void my_on_event_received(SpEventInfo UNUSED(pEvent))
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
    void handle_event(SpEventInfo UNUSED(pEvent))
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
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~DocumentEventsTestFixture()    //TearDown fixture
    {
    }
};

SUITE(DocumentEventsTest)
{

    TEST_FIXTURE(DocumentEventsTestFixture, AddHandler_C)
    {
        SpDocument spDoc( new MyDocument(m_libraryScope) );
        spDoc->create_empty();
        ImoParagraph* pPara = spDoc->add_paragraph();
        ImoLink* pLink = pPara->add_link("Click me");

        MyEventHandler handler;
        pLink->add_event_handler(k_on_click_event, &handler,
                                   MyEventHandler::my_on_event_received_wrapper);

        MyDocument* pDoc = static_cast<MyDocument*>( spDoc.get() );
        std::list<Observer*> observers = pDoc->my_get_observers();
        CHECK( observers.size() == 1 );
        Observer* pObserver = observers.front();
        CHECK( pObserver && pObserver->target() == pLink );
    }

    TEST_FIXTURE(DocumentEventsTestFixture, NotifyEvent_C)
    {
        SpDocument spDoc( new MyDocument(m_libraryScope) );
        spDoc->create_empty();
        ImoParagraph* pPara = spDoc->add_paragraph();
        ImoLink* pLink = pPara->add_link("Click me");
        MyEventHandler handler;
        pLink->add_event_handler(k_on_click_event, &handler,
                                 MyEventHandler::my_on_event_received_wrapper);

        CHECK( handler.event_received() == false );

        SpEventInfo ev( new MyEventOnClick(pLink, WpDocument(spDoc)) );
        spDoc->notify_observers(ev, pLink);

        CHECK( handler.event_received() == true );
    }

    TEST_FIXTURE(DocumentEventsTestFixture, AddHandler_CPP)
    {
        SpDocument spDoc( new MyDocument(m_libraryScope) );
        spDoc->create_empty();
        ImoParagraph* pPara = spDoc->add_paragraph();
        ImoLink* pLink = pPara->add_link("Click me");

        MyEventHandlerCPP handler;
        pLink->add_event_handler(k_on_click_event, &handler);

        MyDocument* pDoc = static_cast<MyDocument*>( spDoc.get() );
        std::list<Observer*> observers = pDoc->my_get_observers();
        CHECK( observers.size() == 1 );
        Observer* pObserver = observers.front();
        CHECK( pObserver && pObserver->target() == pLink );
    }

    TEST_FIXTURE(DocumentEventsTestFixture, NotifyEvent_CPP)
    {
        SpDocument spDoc( new MyDocument(m_libraryScope) );
        spDoc->create_empty();
        ImoParagraph* pPara = spDoc->add_paragraph();
        ImoLink* pLink = pPara->add_link("Click me");
        MyEventHandlerCPP handler;
        pLink->add_event_handler(k_on_click_event, &handler);
        CHECK( handler.event_received() == false );

        SpEventInfo ev( new MyEventOnClick(pLink,WpDocument(spDoc)) );
        spDoc->notify_observers(ev, ev->get_source() );

        CHECK( handler.event_received() == true );
    }

    TEST_FIXTURE(DocumentEventsTestFixture, control)
    {
        SpDocument spDoc( new MyDocument(m_libraryScope) );
        spDoc->create_empty();
        ImoParagraph* pPara = spDoc->add_paragraph();
        HyperlinkCtrl* pLink = new HyperlinkCtrl(m_libraryScope, nullptr, spDoc.get(), "link");
        ImoControl* pControl = pPara->add_control(pLink);
        MyEventHandlerCPP handler;
        pLink->add_event_handler(k_on_click_event, &handler);

        CHECK( handler.event_received() == false );

        SpEventInfo ev( new MyEventOnClick(pControl, WpDocument(spDoc)) );
        pLink->handle_event(ev);

        CHECK( handler.event_received() == true );
    }

////    TEST_FIXTURE(DocumentEventsTestFixture, ReplaceHandler)
////    {
////        Document doc(m_libraryScope);
////        spDoc->create_empty();
////        ImoParagraph* pPara = spDoc->add_paragraph();
////        ImoButton* pButton = pPara->add_button(m_libraryScope, "Click me", USize(1000.0f, 600.0f));
////
////        MyObserver observer(pButton);
////        MyEventHandler handler;
////        observer.add_handler(k_on_click_event, &handler,
////                             MyEventHandler::my_on_event_received_wrapper);
////        MyEventHandler handler2;
////        observer.add_handler(k_on_click_event, &handler2,
////                             MyEventHandler::my_on_event_received_wrapper);
////
////        CHECK( observer.my_num_observers() == 1 );
////    }

};

