//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_events_dispatcher.h"

namespace lomse
{

//TODO: For now, direct invocation without enqueuing the event in the thread.
#define LOMSE_DIRECT_INVOCATION     1       //1=do not use events thread

//=======================================================================================
// EventsDispatcher implementation
//=======================================================================================
EventsDispatcher::EventsDispatcher()
    : m_pThread(nullptr)
    , m_fStopLoop(false)
{
}

//---------------------------------------------------------------------------------------
EventsDispatcher::~EventsDispatcher()
{
}

//---------------------------------------------------------------------------------------
void EventsDispatcher::start_events_loop()
{
    //Create the thread. It starts inmediately to execute the events loop (method
    //run_events_loop())

    //AWARE: this method is only intended to be invoked by Lomse, when the library is
    //initialized. This method only returns when the stop_events_loop() method
    //is invoked.

#if (LOMSE_DIRECT_INVOCATION == 0)
    delete m_pThread;
    m_pThread = LOMSE_NEW EventsThread(&EventsDispatcher::thread_main, this);
#endif
}

//---------------------------------------------------------------------------------------
void EventsDispatcher::stop_events_loop()
{
    //stops the events dispatch loop

    //AWARE: this method is only intended to be run by Lomse, when the
    //Lomse LibraryScope object is destroyed.

    m_fStopLoop = true;
}

//---------------------------------------------------------------------------------------
void EventsDispatcher::thread_main()
{
#if (LOMSE_DIRECT_INVOCATION == 0)
    run_events_loop();
#endif
}

//---------------------------------------------------------------------------------------
void EventsDispatcher::post_event(Observer* pObserver, SpEventInfo pEvent)
{
#if (LOMSE_DIRECT_INVOCATION == 1)
    pObserver->notify(pEvent);
#else
    {
        QueueLock lock(m_mutex);
        m_events.push( make_pair(pEvent, pObserver));
    }
#endif
}

//---------------------------------------------------------------------------------------
// Methods to be executed in the thread
//---------------------------------------------------------------------------------------

void EventsDispatcher::run_events_loop()
{
    std::chrono::milliseconds waitTime(5);   //5ms

    while (!stop_event_received())
    {
        if (pending_events())
            dispatch_next_event();
        else
            std::this_thread::sleep_for(waitTime);
    }
}

//---------------------------------------------------------------------------------------
void EventsDispatcher::dispatch_next_event()
{
    pair<SpEventInfo, Observer*> event;

    {
        QueueLock lock(m_mutex);
        event = m_events.front();
        m_events.pop();
    }

    SpEventInfo pEvent = event.first;
    Observer* pObserver = event.second;
    pObserver->notify(pEvent);
}


}   //namespace lomse
