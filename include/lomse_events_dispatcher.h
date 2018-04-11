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

#ifndef __LOMSE_EVENTS_DISPATCHER_H__
#define __LOMSE_EVENTS_DISPATCHER_H__

#define LOMSE_USE_BOOST_ASIO        0

#include "lomse_build_options.h"
#include "lomse_injectors.h"
#include "lomse_events.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#if (LOMSE_USE_BOOST_ASIO == 1)
    #include <boost/asio.hpp>
#endif

#include <queue>
using namespace std;

namespace lomse
{

//forward declarations


//---------------------------------------------------------------------------------------
typedef boost::thread EventsThread;
typedef boost::mutex QueueMutex;
typedef boost::unique_lock<boost::mutex> QueueLock;



//=======================================================================================
// EventsDispatcher
//  Class to manage the event-dispatch loop.
//  This class is a singleton maintained in Lomse LibraryScope object
class EventsDispatcher
{
protected:
    EventsThread* m_pThread;        //execution thread
    QueueMutex m_mutex;             //to control queue access
    bool m_fStopLoop;
    queue< pair<SpEventInfo, Observer*> > m_events;
#if (LOMSE_USE_BOOST_ASIO == 1)
    boost::asio::io_service m_ioService;
    boost::asio::io_service::work* m_pFakeWork;
    boost::thread_group m_threads;
#endif

public:
    EventsDispatcher();
    ~EventsDispatcher();

    void start_events_loop();
    void stop_events_loop();

    void post_event(Observer* pObserver, SpEventInfo pEvent);
#if (LOMSE_USE_BOOST_ASIO == 1)
    inline boost::asio::io_service& get_io_service() { return m_ioService; }
#endif

protected:
    inline bool stop_event_received() { return m_fStopLoop; }
    void run_events_loop();
    void thread_main();
    inline bool pending_events() { return !m_events.empty(); }
    void dispatch_next_event();

};


}   //namespace lomse

#endif      //__LOMSE_EVENTS_DISPATCHER_H__
