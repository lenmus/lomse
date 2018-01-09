//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
#include "lomse_events_dispatcher.h"
#include "lomse_control.h"
#include "lomse_document.h"
#include "lomse_logger.h"

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
        , m_pCFunct(nullptr)
        , m_pObject(pThis)
        , m_pHandler(nullptr)
    {
    }
    EventCallback(int eventType, EventHandler* pHandler)
        : m_eventType(eventType)
        , m_pCppFunct(nullptr)
        , m_pCFunct(nullptr)
        , m_pObject(nullptr)
        , m_pHandler(pHandler)
    {
    }
    EventCallback(int eventType, void (*pt2Func)(SpEventInfo event))
        : m_eventType(eventType)
        , m_pCppFunct(nullptr)
        , m_pCFunct(pt2Func)
        , m_pObject(nullptr)
        , m_pHandler(nullptr)
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
Observer::Observer(Observable* target)
    : m_target(target)
    , m_type(Observable::k_root)
    , m_id(0L)
{
}

//---------------------------------------------------------------------------------------
Observer::Observer(Observable* root, int childType, ImoId childId)
    : m_target(root)
    , m_type(childType)
    , m_id(childId)
{
}

//---------------------------------------------------------------------------------------
Observer::~Observer()
{
    std::list<EventCallback*>::iterator it;
    for (it = m_handlers.begin(); it != m_handlers.end(); ++it)
        delete *it;
    m_handlers.clear();
}

//---------------------------------------------------------------------------------------
Observable* Observer::target()
{
    if (m_type == Observable::k_root)
        return m_target;
    else
        return m_target->get_observable_child(m_type, m_id);
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

//---------------------------------------------------------------------------------------
void Observable::add_event_handler(int childType, ImoId childId, int eventType,
                           EventHandler* pHandler)
{
    EventNotifier* pNotifier = get_event_notifier();
    pNotifier->add_handler_for_child(this, childType, childId, eventType, pHandler);
}

//---------------------------------------------------------------------------------------
void Observable::add_event_handler(int childType, ImoId childId, int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) )
{
    EventNotifier* pNotifier = get_event_notifier();
    pNotifier->add_handler_for_child(this, childType, childId, eventType,
                                     pThis, pt2Func);
}

//---------------------------------------------------------------------------------------
void Observable::add_event_handler(int childType, ImoId childId, int eventType,
                           void (*pt2Func)(SpEventInfo event) )
{
    EventNotifier* pNotifier = get_event_notifier();
    pNotifier->add_handler_for_child(this, childType, childId, eventType, pt2Func);
}


//=======================================================================================
// EventNotifier implementation
//=======================================================================================
EventNotifier::~EventNotifier()
{
    delete_observers();
}

//---------------------------------------------------------------------------------------
bool EventNotifier::notify_observers(SpEventInfo pEvent, Observable* target)
{
    //returns true if event is dispatched to an observer

    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
    {
        Observable* observedTarget = (*it)->target();
        bool fNotify = (observedTarget == target);

        //bubbling phase. This observer is not observing target but might be
        //observing its parents
        if (!fNotify)
        {
            ImoObj* pImo = dynamic_cast<ImoObj*>(target);
            Observable* pObs = target;
            if (pImo && pObs)
            {
                while(pImo && pObs && pObs != observedTarget)
                {
                    pObs = pImo->get_observable_parent();
                    pImo = dynamic_cast<ImoObj*>( pObs );
                }
                fNotify = (pObs == observedTarget);
                //TODO: do notification and continue bubbling.
            }
        }

        if (fNotify)
        {
            LOMSE_LOG_DEBUG(Logger::k_events, "Posting event.");
            m_pDispatcher->post_event((*it), pEvent);
//            (*it)->notify(pEvent);
            return true;
            //TODO: remove 'return' when following problem is fixed:
            //    Object receiving notification might modify the document (i.e. link
            //    'new problem') and this will invalidate target and all remaining
            //    objects in m_observers (!!!!)
        }
    }
    LOMSE_LOG_DEBUG(Logger::k_events, "No observers. Event ignored");
    return false;
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
Observer* EventNotifier::add_observer_for_child(Observable* parent, int childType,
                                                ImoId childId)
{
    std::list<Observer*>::iterator it;
    for (it = m_observers.begin(); it != m_observers.end(); ++it)
    {
        Observable* target = parent->get_observable_child(childType, childId);
        if ((*it)->target() == target)
            return *it;
    }

    Observer* observer = LOMSE_NEW Observer(parent, childType, childId);
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

//---------------------------------------------------------------------------------------
void EventNotifier::add_handler_for_child(Observable* parent, int childType,
                                          ImoId childId, int eventType,
                                          EventHandler* pHandler)
{
    Observer* observer = add_observer_for_child(parent, childType, childId);
    observer->add_handler(eventType, pHandler);
}

//---------------------------------------------------------------------------------------
void EventNotifier::add_handler_for_child(Observable* parent, int childType,
                                          ImoId childId, int eventType,
                                          void (*pt2Func)(SpEventInfo event) )
{
    Observer* observer = add_observer_for_child(parent, childType, childId);
    observer->add_handler(eventType, pt2Func);
}

//---------------------------------------------------------------------------------------
void EventNotifier::add_handler_for_child(Observable* parent, int childType,
                                          ImoId childId, int eventType, void* pThis,
                                          void (*pt2Func)(void* pObj, SpEventInfo event) )
{
    Observer* observer = add_observer_for_child(parent, childType, childId);
    observer->add_handler(eventType, pThis, pt2Func);
}


//=======================================================================================
// EventMouse implementation
//=======================================================================================
ImoObj* EventMouse::get_imo_object()
{
    if (SpDocument sp = m_wpDoc.lock())
        return sp->get_pointer_to_imo(m_imoId);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
Observable* EventMouse::get_source()
{
    ImoObj* pImo = get_imo_object();
    if (pImo)
    {
        if (pImo->is_contentobj())
            return static_cast<Observable*>( static_cast<ImoContentObj*>(pImo) );
        else
            return pImo->get_observable_parent();
    }
    return nullptr;
}


//=======================================================================================
// EventControlPointMoved implementation
//=======================================================================================
EventControlPointMoved::EventControlPointMoved(EEventType type, WpInteractor wpInteractor,
                    GmoObj* pGmo, int iHandler, UPoint uShift, WpDocument wpDoc)
    : EventAction(type, wpInteractor, wpDoc)
    , m_iHandler(iHandler)
    , m_uShift(uShift)
{
    ImoObj* pImo = pGmo->get_creator_imo();
    m_imoId = pImo->get_id();
    m_gmoType = pGmo->get_gmobj_type();
    m_idx = (pGmo->is_shape() ? static_cast<GmoShape*>(pGmo)->get_shape_id() : -1);
}


}   //namespace lomse
