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

#ifndef __LOMSE_EVENTS_H__
#define __LOMSE_EVENTS_H__


namespace lomse
{

//forward declarations
class ImoObj;
class ImoDocument;


//---------------------------------------------------------------------------------------
// enum for even types
enum EEventType
{
    k_null_event = -1,

    k_view_level_event = 0,
        k_update_window_event,      //now, InvalidateRect
        k_force_redraw_event,       //when suitable, ask View to repaint
        k_weblink_event,            //click on external link (URL)
        k_ilink_event,              //click on internal link

    k_doc_level_event = 1000,
        k_dynamic_content_event,    //request for dynamic content creation

    k_library_level_event = 5000,
};

//---------------------------------------------------------------------------------------
// Abstract class for information about an event
class EventInfo
{
protected:
    int m_type;

    EventInfo(int type) : m_type(type) {}

public:
    virtual ~EventInfo() {}

    //classification

        //view level events
    inline bool is_view_level_event() { return m_type >= k_dynamic_content_event
                                            && m_type < k_doc_level_event;
    }
    inline bool is_update_window_event() { return m_type == k_update_window_event; }
    inline bool is_force_redraw_event() { return m_type == k_force_redraw_event; }
    inline bool is_weblink_event() { return m_type == k_weblink_event; }
    inline bool is_ilink_event() { return m_type == k_ilink_event; }

        //document level events
    inline bool is_doc_level_event() { return m_type >= k_doc_level_event
                                            && m_type < k_library_level_event;
    }
    inline bool is_dynamic_content_event() { return m_type == k_dynamic_content_event; }

        //library level events
    inline bool is_library_level_event() { return m_type >= k_library_level_event; }

};

//---------------------------------------------------------------------------------------
class EventDoc : public EventInfo
{
public:
    ~EventDoc() {}

};

//---------------------------------------------------------------------------------------
class EventView : public EventInfo
{
protected:
    View* m_pView;

public:
    EventView(int type, View* pView) : EventInfo(type), m_pView(pView) {}
    ~EventView() {}

};

//---------------------------------------------------------------------------------------
class EventDynamic : public EventInfo
{
protected:
    ImoDocument* m_pDoc;
    ImoObj* m_pObj;

public:
    EventDynamic(int type, ImoDocument* pDoc, ImoObj* pObj) 
        : EventInfo(type)
        , m_pDoc(pDoc)
        , m_pObj(pObj)
    {
    }
    ~EventDynamic() {}

    //getters
    inline ImoObj* get_object() { return m_pObj; }
    inline ImoDocument* get_document() { return m_pDoc; }

    //setters
    inline void set_object(ImoObj* pObj) { m_pObj = pObj; }

};


}   //namespace lomse

#endif      //__LOMSE_EVENTS_H__
