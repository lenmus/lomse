//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_CONTROL_H__
#define __LOMSE_CONTROL_H__

#include "lomse_basic.h"
#include "private/lomse_document_p.h"
#include "lomse_injectors.h"
#include "lomse_events.h"               // EventHandler
#include "lomse_gm_basic.h"
#include "lomse_calligrapher.h"         //TextMeter

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
    LibraryScope&   m_libraryScope;
    Document*       m_pDoc;
    Control*        m_pParent;
    ImoId           m_ownerImoId;
    string          m_language;
    ImoId           m_styleId;
    bool            m_fEnabled;
    bool            m_fVisible;
    ImoId           m_id;
    list<Control*>  m_controls;

    Control(LibraryScope& libraryScope, Document* pDoc, Control* pParent)
        : EventHandler()
        , Observable()
        , m_libraryScope(libraryScope)
        , m_pDoc(pDoc)
        , m_pParent(pParent)
        , m_ownerImoId(k_no_imoid)
        , m_language()
        , m_styleId(k_no_imoid)
        , m_fEnabled(true)
        , m_fVisible(true)
        , m_id(k_no_imoid)
    {
        //default language
        ImoDocument* pImoDoc = m_pDoc->get_im_root();
        m_language = pImoDoc->get_language();

        //assign id
        DocModel* pModel = pImoDoc->get_doc_model();
        pModel->assign_id(this);
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
    inline void set_style(ImoStyle* pStyle) { m_styleId = pStyle->get_id(); }

    //mandatory overrides from Observable
    EventNotifier* get_event_notifier() override { return m_pDoc->get_event_notifier(); }

    //overrides for Observable children
    void add_event_handler(int eventType, EventHandler* pHandler) override
    {
        m_pDoc->add_event_handler(Observable::k_control, m_id, eventType, pHandler);
    }
    void add_event_handler(int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) ) override
    {
        m_pDoc->add_event_handler(Observable::k_control, m_id, eventType,
                                  pThis, pt2Func);
    }
    void add_event_handler(int eventType, void (*pt2Func)(SpEventInfo event) ) override
    {
        m_pDoc->add_event_handler(Observable::k_control, m_id, eventType, pt2Func);
    }

    //accessors
    inline Control* get_parent_control() { return m_pParent; }
    ImoStyle* get_style() const {
        return static_cast<ImoStyle*>( m_pDoc->get_doc_model()->get_pointer_to_imo(m_styleId) );
    }

    //getters
    inline bool is_enabled() { return m_fEnabled; }
    inline bool is_visible() { return m_fVisible; }
    inline ImoId get_control_id() { return m_id; }
    inline ImoId get_owner_imo_id() { return m_ownerImoId; }

    ImoControl* get_owner_imo() {
        if (m_ownerImoId != k_no_imoid)
            return static_cast<ImoControl*>( m_pDoc->get_pointer_to_imo(m_ownerImoId) );
        else if (m_pParent)
            return m_pParent->get_owner_imo();
        else
            return nullptr;
    }

    //setters
    inline void enable(bool value) { m_fEnabled = value; }
    inline void set_visible(bool value) { m_fVisible = value; }
    inline void set_control_id(ImoId id) { m_id = id; }
    inline void set_owner_imo(ImoControl* pImo) { m_ownerImoId = pImo->get_id(); }

protected:

    void select_font()
    {
        if (m_styleId != k_no_imoid)
        {
            TextMeter meter(m_libraryScope);
            ImoStyle* pStyle = get_style();
            if (pStyle)
            {
                meter.select_font(m_language,
                                  pStyle->font_file(),
                                  pStyle->font_name(),
                                  pStyle->font_size(),
                                  pStyle->is_bold(),
                                  pStyle->is_italic() );
            }
        }
    }

};


} //namespace lomse

#endif    //__LOMSE_CONTROL_H__
