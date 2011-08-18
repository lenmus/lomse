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
    void (*m_pFunction)(void* pThis, EventInfo* event);
    void* m_pObject;
    EventHandler* m_pHandler;

public:
    EventCallback(int eventType, void* pThis, void (*pt2Func)(void* pObj, EventInfo* event))
        : m_eventType(eventType)
        , m_pFunction(pt2Func)
        , m_pObject(pThis)
        , m_pHandler(NULL)
    {
    }
    EventCallback(int eventType, EventHandler* pHandler)
        : m_eventType(eventType)
        , m_pFunction(NULL)
        , m_pObject(NULL)
        , m_pHandler(pHandler)
    {
    }
    ~EventCallback() {}

    inline int get_event_type() { return m_eventType; }

    void notify(EventInfo* pEvent)
    {
        if (m_pFunction)
            m_pFunction(m_pObject, pEvent);
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
void Observer::notify(EventInfo* pEvent)
{
    std::list<EventCallback*>::iterator it = find_handler(pEvent->get_event_type());
    if (it != m_handlers.end())
        (*it)->notify(pEvent);
}

//---------------------------------------------------------------------------------------
void Observer::add_handler(int eventType, void* pThis,
                                void (*pt2Func)(void* pObj, EventInfo* event) )
{
    remove_old_handler(eventType);
    EventCallback* pData = new EventCallback(eventType, pThis, pt2Func);
    m_handlers.push_back(pData);
}

//---------------------------------------------------------------------------------------
void Observer::add_handler(int eventType, EventHandler* pHandler)
{
    remove_old_handler(eventType);
    EventCallback* pData = new EventCallback(eventType, pHandler);
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
// EventOnClick implementation
//=======================================================================================
DynGenerator* EventOnClick::find_generator(GmoObj* pGmo)
{
    ImoObj* pParent = m_pImo;
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
ImoObj* EventOnClick::find_originator_imo(GmoObj* pGmo)
{
    ImoObj* pParent = m_pGmo->get_creator_imo();
    while (pParent && !pParent->is_link())
        pParent = dynamic_cast<ImoContentObj*>( pParent->get_parent() );

    if (pParent && pParent->is_link())
        return pParent;
    else
        return m_pGmo->get_creator_imo();
}

}   //namespace lomse
