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

#ifndef __LOMSE_CARET_H__
#define __LOMSE_CARET_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"

//other
#include <iostream>
using namespace std;

#include "lomse_events_dispatcher.h"
#if (LOMSE_USE_BOOST_ASIO == 1)
    #include <boost/asio.hpp>
    #include <boost/bind.hpp>
#endif

namespace lomse
{

//forward declarations
class ScreenDrawer;
class GraphicView;


//---------------------------------------------------------------------------------------
// Caret: a blinking cursor showing the position that will be affected by next command.
class Caret
{
protected:
    LibraryScope& m_libraryScope;
    GraphicView* m_pView;        //the view to which this caret is associated
    Color m_color;
    Color m_topLevelSelected;
    bool m_fVisible;        //the caret must be displayed
    bool m_fBlinkStateOn;   //currently displayed
    bool m_fBlinkEnabled;   //blinking enabled
    URect m_box;            //bounding box for top level element
    UPoint m_pos;           //caret position
    USize m_size;           //caret size
    int m_type;
    string m_timecode;      //timecode for current position
#if (LOMSE_USE_BOOST_ASIO == 1)
    int m_blinkTime;            //milliseconds.
    boost::asio::deadline_timer m_timer;
#endif

public:
    Caret(GraphicView* view, LibraryScope& libraryScope);
    virtual ~Caret() {}

    //operations
    inline void toggle_caret() { m_fBlinkStateOn = !m_fBlinkStateOn; }
    inline void show_caret(bool fVisible=true) { m_fVisible = fVisible; }
    inline void hide_caret() { m_fVisible = false; }

    virtual void on_draw(ScreenDrawer* pDrawer);

    //caret shapes
    enum { k_top_level=0, k_box, k_line, k_block, };

    //getters
    inline Color get_color() const { return m_color; }
    inline UPoint get_position() const { return m_pos; }
    inline USize get_size() const { return m_size; }
    inline int get_type() const { return m_type; }
    inline const string& get_timecode() const { return m_timecode; }

    //set properties
    inline void set_color(Color color) { m_color = color; }
    inline void set_top_level_box(URect box) { m_box = box; }
    inline void set_position(UPoint pos) { m_pos = pos; }
    inline void set_size(USize size) { m_size = size; }
    inline void set_type(int type) { m_type = type; }
    inline void enable_blink(bool value) { m_fBlinkEnabled = value; }
    inline void set_timecode(const string& value) { m_timecode = value; }

    //properties
//    bool is_currently shown() const { return m_countVisible > 0; }
    inline bool is_visible() const { return m_fVisible; }
    inline bool is_blink_enabled() const { return m_fBlinkEnabled; }
    inline bool is_displayed() const { return m_fBlinkStateOn; }

#if (LOMSE_USE_BOOST_ASIO == 1)
    inline int get_blink_time() { return m_blinkTime; }
    inline void set_blink_time(int milliseconds) { m_blinkTime = milliseconds; }
    void handle_timeout(boost::system::error_code const& cError);
#endif

protected:
    void schedule_the_timer();
    void draw_caret(ScreenDrawer* pDrawer);
    void remove_caret(ScreenDrawer* pDrawer);

    void draw_caret_as_top_level(ScreenDrawer* pDrawer);
    void draw_caret_as_block(ScreenDrawer* pDrawer);
    void draw_caret_as_box(ScreenDrawer* pDrawer);
    void draw_caret_as_line(ScreenDrawer* pDrawer);
    void draw_top_level_box(ScreenDrawer* pDrawer);

};


}   //namespace lomse

#endif      //__LOMSE_CARET_H__
