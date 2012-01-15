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

#ifndef __LOMSE_CONTROL_H__
#define __LOMSE_CONTROL_H__

#include "lomse_basic.h"
//#include "lomse_dyn_generator.h"
#include "lomse_injectors.h"
#include "lomse_events.h"               // EventHandler
#include "lomse_gm_basic.h"

namespace lomse
{

//forward declarations
class ImoContent;
class Document;
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
        , EventNotifier()
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
