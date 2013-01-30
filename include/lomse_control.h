//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_CONTROL_H__
#define __LOMSE_CONTROL_H__

#include "lomse_basic.h"
#include "lomse_document.h"
#include "lomse_injectors.h"
#include "lomse_events.h"               // EventHandler
#include "lomse_gm_basic.h"

namespace lomse
{

//forward declarations
class ImoContent;
class GmoBox;
class Drawer;
struct RenderOptions;


//---------------------------------------------------------------------------------------
// Base class for any GUI control
class Control : public EventHandler
              , public Observable
{
protected:
    Document*       m_pDoc;
    Control*        m_pParent;
    long            m_ownerImoId;
    ImoStyle*       m_pStyle;
    bool            m_fEnabled;
    bool            m_fVisible;
    long            m_id;
    list<Control*>  m_controls;

    Control(Document* pDoc, Control* pParent)
        : EventHandler()
        , Observable()
        , m_pDoc(pDoc)
        , m_pParent(pParent)
        , m_ownerImoId(-1L)
        , m_pStyle(NULL)
        , m_fEnabled(true)
        , m_fVisible(true)
        , m_id(-1L)
    {
        pDoc->assign_id(this);
    }

public:
    virtual ~Control() {
        list<Control*>::iterator it;
        for (it = m_controls.begin(); it != m_controls.end(); ++it)
            delete *it;
        m_controls.clear();
    }

    //any control can be build by using other controls
    void take_ownership_of(Control* control) {
        m_controls.push_back(control);
    }
    void delete_child_control(Control* control) {
        list<Control*>::iterator it;
        for (it = m_controls.begin(); it != m_controls.end(); ++it)
        {
            if (*it == control)
            {
                delete *it;
                m_controls.erase(it);
                break;
            }
        }
    }

    //Any Control must know its size or how to determine it. This method is always
    //invoked before layout() method.
    virtual USize measure() = 0;

    virtual LUnits width() = 0;
    virtual LUnits height() = 0;
    virtual LUnits top() = 0;
    virtual LUnits bottom() = 0;
    virtual LUnits left() = 0;
    virtual LUnits right() = 0;

    //Any Control must know how to generate its graphical model
    virtual GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos) = 0;
    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt) = 0;
    inline void set_style(ImoStyle* pStyle) { m_pStyle = pStyle; }

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return m_pDoc->get_event_notifier(); }

    //overrides for Observable children
    void add_event_handler(int eventType, EventHandler* pHandler)
    {
        m_pDoc->add_event_handler(Observable::k_control, m_id, eventType, pHandler);
    }
    void add_event_handler(int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) )
    {
        m_pDoc->add_event_handler(Observable::k_control, m_id, eventType,
                                  pThis, pt2Func);
    }
    void add_event_handler(int eventType, void (*pt2Func)(SpEventInfo event) )
    {
        m_pDoc->add_event_handler(Observable::k_control, m_id, eventType, pt2Func);
    }

    //accessors
    inline Control* get_parent_control() { return m_pParent; }

    //getters
    inline bool is_enabled() { return m_fEnabled; }
    inline bool is_visible() { return m_fVisible; }
    inline long get_id() { return m_id; }
    inline long get_owner_imo_id() { return m_ownerImoId; }

    ImoControl* get_owner_imo() {
        if (m_ownerImoId != -1L)
            return static_cast<ImoControl*>( m_pDoc->get_pointer_to_imo(m_ownerImoId) );
        else if (m_pParent)
            return m_pParent->get_owner_imo();
        else
            return NULL;
    }

    //setters
    inline void enable(bool value) { m_fEnabled = value; }
    inline void set_visible(bool value) { m_fVisible = value; }
    inline void set_id(long id) { m_id = id; }
    inline void set_owner_imo(ImoControl* pImo) { m_ownerImoId = pImo->get_id(); }

protected:
    ImoStyle* get_style() { return m_pStyle; }
};


} //namespace lomse

#endif    //__LOMSE_CONTROL_H__
