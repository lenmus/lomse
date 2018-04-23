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
