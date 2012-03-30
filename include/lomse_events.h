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

#ifndef __LOMSE_EVENTS_H__
#define __LOMSE_EVENTS_H__

#include "lomse_build_options.h"

#include <boost/shared_ptr.hpp>

#include <list>
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class ImoContentObj;
class ImoDynamic;
class ImoDocument;
class ImoScore;
class ImoStaffObj;
class Interactor;
class GmoObj;
class DynGenerator;
class Document;
class GraphicModel;
class PlayerGui;

//observer pattern
class EventNotifier;
class Observer;
class Observable;
class EventHandler;
class EventCallback;

//---------------------------------------------------------------------------------------
// enum for even types
enum EEventType
{
    k_null_event = -1,

    k_view_level_event = 0,

        k_update_window_event,      //ask user app to update window with current bitmap

        k_mouse_event,
            k_mouse_in_event,           //mouse goes over an object
            k_mouse_out_event,          //mouse goes out from an object
            k_on_click_event,           //Document, ImoContentObj: click on object

        k_highlight_event,          //score playbaxk (highlight)
            k_highlight_on_event,           //add highlight to a note/rest
            k_highlight_off_event,          //remove highlight from a note/rest
            k_end_of_higlight_event,        //end of score play back. Remove all highlight.
            k_advance_tempo_line_event,

        k_play_score_event,         //score playback (sound)
            k_do_play_score_event,          //start/resume playback
            k_pause_score_event,            //pause playback
            k_end_of_playback_event,        //end of playback

    k_doc_level_event = 1000,
        k_doc_modified_event,
};

//---------------------------------------------------------------------------------------
// Abstract class for information about an event
class EventInfo
{
protected:
    EEventType m_type;

    EventInfo(EEventType type) : m_type(type) {}

public:
    virtual ~EventInfo() {}

    virtual Observable* get_source() { return NULL; }


    //classification

    inline EEventType get_event_type() { return m_type; }

        //view level events
    inline bool is_view_level_event() { return m_type >= k_view_level_event
                                            && m_type < k_doc_level_event;
    }
    inline bool is_update_window_event() { return m_type == k_update_window_event; }
    inline bool is_mouse_event() { return m_type == k_on_click_event
                                       || m_type == k_mouse_in_event
                                       || m_type == k_mouse_out_event;
    }
    inline bool is_play_score_event() { return m_type == k_do_play_score_event
                                            || m_type == k_pause_score_event
                                            || m_type == k_end_of_playback_event;
    }
    inline bool is_mouse_in_event() { return m_type == k_mouse_in_event; }
    inline bool is_mouse_out_event() { return m_type == k_mouse_out_event; }
    inline bool is_on_click_event() { return m_type == k_on_click_event; }
    inline bool is_highlight_event() { return m_type == k_highlight_event; }
    inline bool is_highlight_on_event() { return m_type == k_highlight_on_event; }
    inline bool is_highlight_off_event() { return m_type == k_highlight_off_event; }
    inline bool is_end_of_higlight_event() { return m_type == k_end_of_higlight_event; }
    inline bool is_advance_tempo_line_event() { return m_type == k_advance_tempo_line_event; }
    inline bool is_end_of_playback_event() { return m_type == k_end_of_playback_event; }
    inline bool is_do_play_score_event() { return m_type == k_do_play_score_event; }
    inline bool is_pause_score_event() { return m_type == k_pause_score_event; }

        //document level events
    inline bool is_doc_level_event() { return m_type >= k_doc_level_event; }
    inline bool is_doc_modified_event() { return m_type == k_doc_modified_event; }

};

typedef boost::shared_ptr<EventInfo>  SpEventInfo;


//---------------------------------------------------------------------------------------
class EventDoc : public EventInfo
{
protected:
   Document* m_pDoc;

public:
    EventDoc(EEventType type, Document* pDoc)
        : EventInfo(type)
        , m_pDoc(pDoc)
    {
    }
    ~EventDoc() {}

    inline Document* get_document() { return m_pDoc; }
};

typedef boost::shared_ptr<EventDoc>  SpEventDoc;


//---------------------------------------------------------------------------------------
class EventView : public EventInfo
{
protected:
    Interactor* m_pInteractor;

public:
    EventView(EEventType type, Interactor* pInteractor)
        : EventInfo(type)
        , m_pInteractor(pInteractor)
    {
    }
    ~EventView() {}

    inline Interactor* get_interactor() { return m_pInteractor; }
};

typedef boost::shared_ptr<EventView>  SpEventView;


//---------------------------------------------------------------------------------------
// Mouse event
class EventMouse : public EventView
{
protected:
    GmoObj* m_pGmo;
    ImoContentObj* m_pImo;
    DynGenerator* m_pGenerator;
    GraphicModel* m_pGModel;

    EventMouse(EEventType type) : EventView(type, NULL) {}    //for unit tests

public:
    EventMouse(EEventType type, Interactor* pInteractor, GmoObj* pGmo,
               GraphicModel* pGModel)
        : EventView(type, pInteractor)
        , m_pGmo(pGmo)
        , m_pImo( find_originator_imo(pGmo) )
        , m_pGenerator( find_generator(pGmo) )
        , m_pGModel(pGModel)
    {
    }

    // accessors
    inline GmoObj* get_gm_object() { return m_pGmo; }
    inline ImoContentObj* get_imo_object() { return m_pImo; }
    inline DynGenerator* get_generator() { return m_pGenerator; }
    Observable* get_source();
    inline GraphicModel* get_gmodel() { return m_pGModel; }


protected:
    DynGenerator* find_generator(GmoObj* pGmo);
    ImoContentObj* find_originator_imo(GmoObj* pGmo);

};

typedef boost::shared_ptr<EventMouse>  SpEventMouse;


//---------------------------------------------------------------------------------------
// EventPlayScore: several events related to score playback
class EventPlayScore : public EventView
{
protected:
    ImoScore* m_pScore;
    PlayerGui* m_pPlayer;

    EventPlayScore(EEventType evType) : EventView(evType, NULL) {}    //for unit tests

public:
    EventPlayScore(EEventType evType, Interactor* pInteractor, ImoScore* pScore,
                   PlayerGui* pPlayer)
        : EventView(evType, pInteractor)
        , m_pScore(pScore)
        , m_pPlayer(pPlayer)
    {
    }

    // accessors
    inline ImoScore* get_score() const { return m_pScore; }
    inline PlayerGui* get_player() const { return m_pPlayer; }
};

typedef boost::shared_ptr<EventPlayScore>  SpEventPlayScore;


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

typedef boost::shared_ptr<EventScoreHighlight>  SpEventScoreHighlight;


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
//  Any class that wants to receive events must derive from EventHandler and
//  must implement the event handling methods.
//=======================================================================================
class EventHandler
{
public:
	virtual ~EventHandler() {}

	virtual void handle_event(SpEventInfo pEvent) = 0;
};


//=======================================================================================
// Observer
//  Auxiliary object responsible for keeping information about an Observable object
//  beign observed, the events to listen to and the methods that must invoke to
//  distpatch each event. It is also responsible for doing this dispatch when
//  requested.
//=======================================================================================
class Observer
{
protected:
    Observable* m_target;
    std::list<EventCallback*> m_handlers;

    friend class EventNotifier;
    Observer(Observable* target) : m_target(target) {}

public:
    virtual ~Observer();

    inline Observable* target() { return m_target; }
    void notify(SpEventInfo pEvent);
    void add_handler(int eventType, void (*pt2Func)(SpEventInfo event) );
    void add_handler(int eventType, void* pThis,
                     void (*pt2Func)(void* pObj, SpEventInfo event) );
    void add_handler(int eventType, EventHandler* pHandler);
    void remove_handler(int evtType);

protected:
    std::list<EventCallback*>::iterator find_handler(int eventType);
    void remove_old_handler(int eventType);

};


//=======================================================================================
// Observable
//  Any object generating events must derive from Observable and must implement
//  method "get_event_notifier()".
//
//  Objects generating events can be part of a hierarchy (i.e. ImoObjs in a Document).
//  In order to allow for diferento event management models, responsibilities for
//  event generation and event dispatching are decoupled. For this, the Observable
//  pattern is splitted into two objects: the Observable itself is just a facade
//  object providing the interface for addind/removing observers, and delegates in
//  the EventNotifier object for doing the real work and event dispatching.
//=======================================================================================
class Observable
{
public:
	virtual ~Observable() {}

	virtual EventNotifier* get_event_notifier() = 0;

    void add_event_handler(int eventType, EventHandler* pHandler);
    void add_event_handler(int eventType, void* pThis,
                           void (*pt2Func)(void* pObj, SpEventInfo event) );
    void add_event_handler(int eventType, void (*pt2Func)(SpEventInfo event) );
};


//=======================================================================================
// EventNotifier
//  Any object that wants to dispatch events must derive from EventNotifier class
//=======================================================================================
class EventNotifier
{
protected:
    std::list<Observer*> m_observers;

public:
    EventNotifier() {}
    virtual ~EventNotifier();

    //Event notification
    void notify_observers(SpEventInfo pEvent, Observable* target);
    void remove_observer(Observer* observer);
    Observer* add_observer_for(Observable* target);

protected:
    friend class Observable;

    void add_handler(Observable* target, int eventType, EventHandler* pHandler);
    void add_handler(Observable* target, int eventType, void* pThis,
                     void (*pt2Func)(void* pObj, SpEventInfo event) );
    void add_handler(Observable* target, int eventType,
                     void (*pt2Func)(SpEventInfo event) );

protected:
    void delete_observers();

};


}   //namespace lomse

#endif      //__LOMSE_EVENTS_H__