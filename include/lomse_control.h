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
class DynGenerator;


//---------------------------------------------------------------------------------------
// Base class for any GUI control
class Control : public EventHandler
              , public EventNotifier
              , public Observable
{
protected:
    DynGenerator*   m_pOwner;
    Document*       m_pDoc;
    ImoStyle*       m_pStyle;
    bool            m_fEnabled;

    Control(DynGenerator* pOwner, Document* pDoc)
        : EventHandler()
        , EventNotifier(pDoc->get_library_scope().get_events_dispatcher())
        , Observable()
        , m_pOwner(pOwner)
        , m_pDoc(pDoc)
        , m_pStyle(NULL)
        , m_fEnabled(true)
    {
    }

public:
    virtual ~Control() {}

    ///Any Control must know its size or how to determine it. This method is always
    ///invoked before layout() method.
    virtual USize measure() = 0;
    virtual LUnits width() = 0;
    virtual LUnits height() = 0;
    virtual LUnits top() = 0;
    virtual LUnits bottom() = 0;
    virtual LUnits left() = 0;
    virtual LUnits right() = 0;

    ///Any Control must know how to generate its graphic model
    virtual GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos) = 0;
    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt) = 0;
    inline void set_style(ImoStyle* pStyle) { m_pStyle = pStyle; }

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() { return this; }

    //other
    inline DynGenerator* get_owner() { return m_pOwner; }

    //getters
    inline bool is_enabled() { return m_fEnabled; }

    //setters
    inline void enable(bool value) { m_fEnabled = value; }

protected:
    ImoStyle* get_style() { return m_pStyle; }
};


} //namespace lomse

#endif    //__LOMSE_CONTROL_H__
