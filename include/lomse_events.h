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

#include "lomse_build_options.h"

#include <list>
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class ImoDynamic;
class ImoDocument;
class ImoStaffObj;
class Interactor;
class GmoObj;
class DynGenerator;
class Document;
class EventCallback;


//---------------------------------------------------------------------------------------
// enum for even types
enum EEventType
{
    k_null_event = -1,

    k_view_level_event = 0,         //U: events for user app, L: events for library
        k_update_window_event,      //U: now, rebuild gmodel and update view
        k_force_redraw_event,       //U: when suitable, ask Interactor to repaint
        k_on_click_event,           //U: click on a ctrol object

        k_highlight_event,
            k_highlight_on_event,           //L: add highlight to a note/rest
            k_highlight_off_event,          //L: remove highlight from a note/rest
            k_end_of_higlight_event,        //L: end of score play back. Remove all highlight.
            k_advance_tempo_line_event,

    k_doc_level_event = 1000,
        k_doc_modified_event,

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

    virtual ImoObj* get_originator_imo() { return NULL; }


    //classification

    inline int get_event_type() { return m_type; }

        //view level events
    inline bool is_view_level_event() { return m_type >= k_view_level_event
                                            && m_type < k_doc_level_event;
    }
    inline bool is_update_window_event() { return m_type == k_update_window_event; }
    inline bool is_force_redraw_event() { return m_type == k_force_redraw_event; }
    inline bool is_on_click_event() { return m_type == k_on_click_event; }
        //higlight events
//    inline bool is_highlight_event() { return m_type == k_highlight_event
//                                            && m_type < k_doc_level_event;
//    }
    inline bool is_highlight_event() { return m_type == k_highlight_event; }
    inline bool is_highlight_on_event() { return m_type == k_highlight_on_event; }
    inline bool is_highlight_off_event() { return m_type == k_highlight_off_event; }
    inline bool is_end_of_higlight_event() { return m_type == k_end_of_higlight_event; }
    inline bool is_advance_tempo_line_event() { return m_type == k_advance_tempo_line_event; }

        //document level events
    inline bool is_doc_level_event() { return m_type >= k_doc_level_event
                                            && m_type < k_library_level_event;
    }
    inline bool is_doc_modified_event() { return m_type == k_doc_modified_event; }

        //library level events
    inline bool is_library_level_event() { return m_type >= k_library_level_event; }

};

//---------------------------------------------------------------------------------------
class EventDoc : public EventInfo
{
protected:
   Document* m_pDoc;

public:
    EventDoc(int type, Document* pDoc)
        : EventInfo(type)
        , m_pDoc(pDoc)
    {
    }
    ~EventDoc() {}

    inline Document* get_document() { return m_pDoc; }
};

//---------------------------------------------------------------------------------------
class EventView : public EventInfo
{
protected:
    Interactor* m_pInteractor;

public:
    EventView(int type, Interactor* pInteractor)
        : EventInfo(type)
        , m_pInteractor(pInteractor)
    {
    }
    ~EventView() {}

    inline Interactor* get_interactor() { return m_pInteractor; }
};

//---------------------------------------------------------------------------------------
// click on a control object
class EventOnClick : public EventView
{
protected:
    GmoObj* m_pGmo;
    ImoObj* m_pImo;
    DynGenerator* m_pGenerator;

    EventOnClick(): EventView(k_on_click_event, NULL) {}    //for unit tests

public:
    EventOnClick(Interactor* pInteractor, GmoObj* pGmo)
        : EventView(k_on_click_event, pInteractor)
        , m_pGmo(pGmo)
        , m_pImo( find_originator_imo(pGmo) )
        , m_pGenerator( find_generator(pGmo) )
    {
    }

    //// copy constructor
    //EventOnClick(const EventOnClick& event)
    //    : EventView(event.m_type, event.m_pInteractor)
    //    , m_pGmo(event.m_pGmo)
    //    , m_pImo(event.m_pImo)
    //    , m_pGenerator(event.m_pGenerator)
    //{
    //}

    // accessors
    inline GmoObj* get_gm_object() { return m_pGmo; }
    inline DynGenerator* get_generator() { return m_pGenerator; }
    inline ImoObj* get_originator_imo() { return m_pImo; }


protected:
    DynGenerator* find_generator(GmoObj* pGmo);
    ImoObj* find_originator_imo(GmoObj* pGmo);

};

//---------------------------------------------------------------------------------------
// EventScoreHighlight: An event to signal different actions related to
//      highlighting / unhighlighting notes while they are being played.
class EventScoreHighlight : public EventView
{
protected:
    long m_nID;             //ID of the target score for the event
    std::list< pair<int, ImoStaffObj*> > m_items;

public:
    EventScoreHighlight(Interactor* pInteractor, long nScoreID)
        : EventView(k_highlight_event, pInteractor)
        , m_nID(nScoreID)
    {
    }

    // copy constructor
    EventScoreHighlight(const EventScoreHighlight& event)
        : EventView(event.m_type, event.m_pInteractor)
        , m_nID(event.m_nID)
        , m_items(event.m_items)
    {
    }

    //construction
    void add_item(int type, ImoStaffObj* pSO)
    {
        m_items.push_back( make_pair(type, pSO) );
    }

    // accessors
    inline long get_scoreID() { return m_nID; }
    inline int get_num_items() { return int( m_items.size() ); }
    inline std::list< pair<int, ImoStaffObj*> >&  get_items() { return m_items; }
};

////-----------------------------------------------------------------------------------------
//// lmEndOfPlayEvent: An event to signal end of playback
////-----------------------------------------------------------------------------------------
//
//DECLARE_EVENT_TYPE( lmEVT_END_OF_PLAY, -1 )
//
//class lmEndOfPlayEvent : public wxEvent
//{
//public:
//    lmEndOfPlayEvent(int id = 0 )    : wxEvent(id, lmEVT_END_OF_PLAY)
//        {    m_propagationLevel = wxEVENT_PROPAGATE_MAX; }
//
//    // copy constructor
//    lmEndOfPlayEvent(const lmEndOfPlayEvent& event) : wxEvent(event) {}
//
//    // clone constructor. Required for sending with wxPostEvent()
//    virtual wxEvent *Clone() const { return new lmEndOfPlayEvent(*this); }
//
//};


//=======================================================================================
// Requests
//=======================================================================================


//---------------------------------------------------------------------------------------
// enum for request types
enum ERequestType
{
    k_null_request = -1,
    k_dynamic_content_request,      //request for dynamic content creation
    k_get_font_filename,            //request the file for the given font
};

//---------------------------------------------------------------------------------------
// Abstract class for information about a request
class Request
{
protected:
    int m_type;

    Request(int type) : m_type(type) {}

public:
    virtual ~Request() {}

    //classification
    inline int get_request_type() { return m_type; }
    inline bool is_dynamic_content_request() { return m_type == k_dynamic_content_request; }
    inline bool is_get_font_filename() { return m_type == k_get_font_filename; }
};

//---------------------------------------------------------------------------------------
class RequestDynamic : public Request
{
protected:
    Document* m_pDoc;
    ImoObj* m_pObj;

public:
    RequestDynamic(Document* pDoc, ImoObj* pObj)
        : Request(k_dynamic_content_request)
        , m_pDoc(pDoc)
        , m_pObj(pObj)
    {
    }
    ~RequestDynamic() {}

    //getters
    inline ImoObj* get_object() { return m_pObj; }
    inline Document* get_document() { return m_pDoc; }

    //for setting requested data
    inline void set_object(ImoObj* pObj) { m_pObj = pObj; }

};

//---------------------------------------------------------------------------------------
class RequestFont : public Request
{
protected:
    const string& m_fontname;
    bool m_bold;
    bool m_italic;
    string m_fullName;

public:
    RequestFont(const string& fontname, bool bold, bool italic)
        : Request(k_get_font_filename)
        , m_fontname(fontname)
        , m_bold(bold)
        , m_italic(italic)
    {
        set_default_fullname();
    }
    ~RequestFont() {}

    //getters
    inline const string& get_fontname() { return m_fontname; }
    inline bool get_bold() { return m_bold; }
    inline bool get_italic() { return m_italic; }
    inline string get_font_fullname() { return m_fullName; }

    //for setting requested data
    inline void set_font_fullname(const string& name) { m_fullName = name; }

protected:

    void set_default_fullname()
    {
        //This is just a mock method to avoid crashes when using the library without
        //initializing it

        m_fullName = LOMSE_FONTS_PATH;

        if (m_fontname == "LenMus basic")
            m_fullName += "lmbasic2.ttf";
        else
            m_fullName += "LiberationSerif-Regular.ttf";
    }

};


//=======================================================================================
// EventHandler
// Any class that wants to receive events from a Document must be derived from
// EventHandler and implement the handling methods.
//=======================================================================================
class EventHandler
{
public:
	virtual ~EventHandler() {}

	virtual void handle_event(EventInfo* pEvent) = 0;
};


//=======================================================================================
// Observer
//  Any object that wants to generate events must create Observer object,
//  specifiying the changes/actions to listen to and the methods to handle each
//  event type. Then it must register the Observer
//=======================================================================================
class Observer
{
protected:
    ImoObj* m_target;
    std::list<EventCallback*> m_handlers;

    friend class Document;
    Observer(ImoObj* pImo) : m_target(pImo) {}

public:
    virtual ~Observer();

    inline ImoObj* target() { return m_target; }
    void notify(EventInfo* pEvent);
    void add_handler(int eventType, void* pThis,
                     void (*pt2Func)(void* pObj, EventInfo* event) );
    void add_handler(int eventType, EventHandler* pHandler);
    void remove_handler(int evtType);

protected:
    std::list<EventCallback*>::iterator find_handler(int eventType);
    void remove_old_handler(int eventType);

};

}   //namespace lomse

#endif      //__LOMSE_EVENTS_H__
