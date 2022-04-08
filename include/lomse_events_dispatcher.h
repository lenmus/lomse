//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_EVENTS_DISPATCHER_H__
#define __LOMSE_EVENTS_DISPATCHER_H__

#include "lomse_build_options.h"
#include "lomse_injectors.h"
#include "lomse_events.h"

//TODO: For now, direct invocation without enqueuing the event in the thread.
#define LOMSE_DIRECT_INVOCATION     1       //1=do not use events thread


#if (LOMSE_DIRECT_INVOCATION == 1)
namespace lomse
{

//=======================================================================================
// EventsDispatcher
//  Class to manage the event-dispatch loop.
//  This class is a singleton maintained in Lomse LibraryScope object
class EventsDispatcher
{
public:
    EventsDispatcher() {}

    inline void start_events_loop() {}
    inline void stop_events_loop() {}

    inline void post_event(Observer* pObserver, SpEventInfo pEvent)
    {
        pObserver->notify(pEvent);
    }
};

#else
#include <thread>
#include <mutex>
#include <queue>

namespace lomse
{

//forward declarations


//---------------------------------------------------------------------------------------
typedef std::thread EventsThread;
typedef std::mutex QueueMutex;
typedef std::unique_lock<std::mutex> QueueLock;



//=======================================================================================
// EventsDispatcher
//  Class to manage the event-dispatch loop.
//  This class is a singleton maintained in Lomse LibraryScope object
class EventsDispatcher
{
protected:
    EventsThread* m_pThread = nullptr;        //execution thread
    QueueMutex m_mutex;             //to control queue access
    bool m_fStopLoop = false;
    std::queue< std::pair<SpEventInfo, Observer*> > m_events;

public:
    EventsDispatcher() {}
;

    void start_events_loop();
    void stop_events_loop();

    void post_event(Observer* pObserver, SpEventInfo pEvent);

protected:
    inline bool stop_event_received() { return m_fStopLoop; }
    void run_events_loop();
    void thread_main();
    inline bool pending_events() { return !m_events.empty(); }
    void dispatch_next_event();

};
#endif

}   //namespace lomse

#endif      //__LOMSE_EVENTS_DISPATCHER_H__
