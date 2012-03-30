//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_events.h"
#include "lomse_internal_model.h"
#include "lomse_gm_basic.h"

namespace lomse
{


//---------------------------------------------------------------------------------------
// helper class to contain callback data
class EventCallback
{
protected:
    int m_eventType;
    void (*m_pCppFunct)(void* pThis, SpEventInfo event);
    void (*m_pCFunct)(SpEventInfo event);
    void* m_pObject;
    EventHandler* m_pHandler;

public:
    EventCallback(int eventType, void* pThis,
                  void (*pt2Func)(void* pObj, SpEventInfo event))
        : m_eventType(eventType)
        , m_pCppFunct(pt2Func)
        , m_pCFunct(NULL)
        , m_pObject(pThis)
        , m_pHandler(NULL)
    {
    }
    EventCallback(int eventType, EventHandler* pHandler)
        : m_eventType(eventType)
        , m_pCppFunct(NULL)
        , m_pCFunct(NULL)
        , m_pObject(NULL)
        , m_pHandler(pHandler)
    {
    }
    EventCallback(int eventType, void (*pt2Func)(SpEventInfo event))
        : m_eventType(eventType)
        , m_pCppFunct(NULL)
        , m_pCFunct(pt2Func)
        , m_pObject(NULL)
        , m_pHandler(NULL)
    {
    }
    ~EventCallback() {}

    inline int get_event_type() { return m_eventType; }

    void notify(SpEventInfo pEvent)
    {
        if (m_pCppFunct)
            m_pCppFunct(m_pObject, pEvent);
        else if (m_pCFunct)
            m_pCFunct(pEvent);
        else if (m_pHandler)
            m_pHandler->handle_event(pEvent);
    }
};


//=======================================================================================
// Observer implementation
//=======================================================================================
Observer::~Observer()
{
    std::list<EventCallback*>::iterator it;
    for (it = m_handlers.begin(); it != m_handlers.end(); ++it)
        delete *it;
    m_handlers.clear();
}

//---------------------------------------------------------------------------------------
std::list<EventCallback*>::iterator Observer::find_handler(int eventType)
{
    std::list<EventCallback*>::iterator it;
    for (it = m_handlers.begin(); it != m_handlers.end(); ++it)
    {
        EventCallback* pCB = *it;
        if (pCB->get_event_type() == eventType)
            break;
    }
    return it;
}

//---------------------------------------------------------------------------------------
void Observer::notify(SpEventInfo pEvent)
{
    std::list<EventCallback*>::iterator it = find_handler(pEvent->get_event_type());
    if (it != m_handlers.end())
        (*it)->notify(pEvent);
}

//---------------------------------------------------------------------------------------
void Observer::add_handler(int eventType, void (*pt2Func)(SpEventInfo event) )
{
    remove_old_handler(eventType);
    EventCallback* pData = LOMSE_NEW EventCallback(eventType, pt2Func);
    m_handlers.push_back(pData);
}

//---------------------------------------------------------------------------------------
void Observer::add_handler(int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) )
{
    remove_old_handler(eventType);
    EventCallback* pData = LOMSE_NEW EventCallback(eventType, pThis, pt2Func);
    m_handlers.push_back(pData);
}

//---------------------------------------------------------------------------------------
void Observer::add_handler(int eventType, EventHandler* pHandler)
{
    remove_old_handler(eventType);
    EventCallback* pData = LOMSE_NEW EventCallback(eventType, pHandler);
    m_handlers.push_back(pData);
}

//---------------------------------------------------------------------------------------
void Observer::remove_old_handler(int eventType)
{
    std::list<EventCallback*>::iterator it = find_handler(eventType);
    if (it != m_handlers.end())
    {
        EventCallback* pCB = *it;
        m_handlers.erase(it);
        delete pCB;
    }
}


//=======================================================================================
// Observable implementation
//=======================================================================================
void Observable::add_event_handler(int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) )
{
    EventNotifier* pNotifier = get_event_notifier();
    pNotifier->add_handler(this, eventType, pThis, pt2Func);
}

//---------------------------------------------------------------------------------------
void Observable::add_event_handler(int eventType, void (*pt2Func)(SpEventInfo event) )
{
    EventNotifier* pNotifier = get_event_notifier();
    pNotifier->add_handler(this, eventType, pt2Func);
}

//---------------------------------------------------------------------------------------
void Observable::add_event_handler(int eventType, EventHandler* pHandler)
{
    EventNotifier* pNotifier = get_event_notifier();
    pNotifier->add_handler(this, eventType, pHandler);
}


//=======================================================================================
// EventNotifier implementation
//=======================================================================================
EventNotifier::~EventNotifier()
{
    delete_observers();
}

//---------------------------------------------------------------------------------------
void EventNotifier::notify_observers(SpEventInfo pEvent, Observable* target)
{
    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
    {
        bool fNotify = ((*it)->target() == target);
        //event on target is also an event on its parents: bubbling phase
        if (!fNotify)
        {
            ImoContentObj* pImo = dynamic_cast<ImoContentObj*>(target);
            if (pImo)
            {
                while(pImo && pImo != (*it)->target())
                {
                    pImo = dynamic_cast<ImoContentObj*>( pImo->get_parent() );
                }
                fNotify = (pImo == (*it)->target());
            }
        }
        if (fNotify)
        {
            (*it)->notify(pEvent);
            return;
            //TODO: remove 'return' when following problem is fixed:
            //    Object receiving notification might modify the document (i.e. link
            //    'new problem') and this will invalidate target and all remaining
            //    objects in m_observers (!!!!)
        }
    }
}

//---------------------------------------------------------------------------------------
Observer* EventNotifier::add_observer_for(Observable* target)
{
    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
    {
        if ((*it)->target() == target)
            return *it;
    }

    Observer* observer = LOMSE_NEW Observer(target);
    m_observers.push_back(observer);
    return observer;
}

//---------------------------------------------------------------------------------------
void EventNotifier::remove_observer(Observer* observer)
{
    m_observers.remove(observer);
    delete observer;
}

//---------------------------------------------------------------------------------------
void EventNotifier::delete_observers()
{
    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
        delete *it;
    m_observers.clear();
}

//---------------------------------------------------------------------------------------
void EventNotifier::add_handler(Observable* pSource, int eventType,
                                EventHandler* pHandler)
{
    Observer* observer = add_observer_for(pSource);
    observer->add_handler(eventType, pHandler);
}

//---------------------------------------------------------------------------------------
void EventNotifier::add_handler(Observable* pSource, int eventType, void* pThis,
                                void (*pt2Func)(void* pObj, SpEventInfo event) )
{
    Observer* observer = add_observer_for(pSource);
    observer->add_handler(eventType, pThis, pt2Func);
}

//---------------------------------------------------------------------------------------
void EventNotifier::add_handler(Observable* pSource, int eventType,
                                void (*pt2Func)(SpEventInfo event) )
{
    Observer* observer = add_observer_for(pSource);
    observer->add_handler(eventType, pt2Func);
}


//=======================================================================================
// EventMouse implementation
//=======================================================================================
DynGenerator* EventMouse::find_generator(GmoObj* pGmo)
{
    ImoContentObj* pParent = m_pImo;
    while (pParent && !pParent->is_dynamic())
        pParent = dynamic_cast<ImoContentObj*>( pParent->get_parent() );
    if (pParent && pParent->is_dynamic())
    {
        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pParent );
        return pDyn->get_generator();
    }
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoContentObj* EventMouse::find_originator_imo(GmoObj* pGmo)
{
    ImoContentObj* pParent = dynamic_cast<ImoContentObj*>( m_pGmo->get_creator_imo() );
    while (pParent && !pParent->is_link())
        pParent = dynamic_cast<ImoContentObj*>( pParent->get_parent() );

    if (pParent && pParent->is_link())
        return pParent;
    else
        return dynamic_cast<ImoContentObj*>( m_pGmo->get_creator_imo() );
}

//---------------------------------------------------------------------------------------
Observable* EventMouse::get_source()
{
    return m_pImo;
}


}   //namespace lomse