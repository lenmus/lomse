//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_CARET_H__
#define __LOMSE_CARET_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_visual_effect.h"

//other
#include <iostream>
using namespace std;

#include "lomse_events_dispatcher.h"

namespace lomse
{

//forward declarations
class BitmapDrawer;
class GraphicView;
class GmoBoxSystem;


//---------------------------------------------------------------------------------------
// Caret: a blinking cursor showing the position that will be affected by next command.
class Caret : public VisualEffect
{
protected:
    Color m_color;
    Color m_topLevelSelected;
    bool m_fBlinkStateOn;   //currently displayed
    bool m_fBlinkEnabled;   //blinking enabled
    URect m_box;            //bounding box for top level element
    UPoint m_pos;           //caret position
    USize m_size;           //caret size
    int m_type;
    string m_timecode;      //timecode for current position
    GmoBoxSystem* m_pBoxSystem;     //active system: the one on which the caret is placed
    URect m_bounds;         //the real bounds of the caret drawing

public:
    Caret(GraphicView* view, LibraryScope& libraryScope);
    virtual ~Caret() {}

    //operations
    inline void toggle_caret() { m_fBlinkStateOn = !m_fBlinkStateOn; }
    inline void show_caret(bool fVisible=true) { m_fVisible = fVisible; }
    inline void hide_caret() { m_fVisible = false; }

    //mandatory overrides from VisualEffect
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override { return m_bounds; }

    //caret shapes
    enum { k_top_level=0, k_box, k_line, k_block, };

    //getters
    inline Color get_color() const { return m_color; }
    inline UPoint get_position() const { return m_pos; }
    inline USize get_size() const { return m_size; }
    inline int get_type() const { return m_type; }
    inline const string& get_timecode() const { return m_timecode; }
    inline GmoBoxSystem* get_active_system() { return m_pBoxSystem; }

    //set properties
    inline void set_color(Color color) { m_color = color; }
    inline void set_top_level_box(URect box) { m_box = box; }
    inline void set_position(UPoint pos) { m_pos = pos; }
    inline void set_size(USize size) { m_size = size; }
    inline void set_type(int type) { m_type = type; }
    inline void enable_blink(bool value) { m_fBlinkEnabled = value; }
    inline void set_timecode(const string& value) { m_timecode = value; }
    inline void set_active_system(GmoBoxSystem* pSystem) { m_pBoxSystem = pSystem; }

    //properties
    inline bool is_blink_enabled() const { return m_fBlinkEnabled; }
    inline bool is_displayed() const { return m_fBlinkStateOn; }

protected:
    void draw_caret(BitmapDrawer* pDrawer);

    void draw_caret_as_top_level(BitmapDrawer* pDrawer);
    void draw_caret_as_block(BitmapDrawer* pDrawer);
    void draw_caret_as_box(BitmapDrawer* pDrawer);
    void draw_caret_as_line(BitmapDrawer* pDrawer);
    void draw_top_level_box(BitmapDrawer* pDrawer);

};


}   //namespace lomse

#endif      //__LOMSE_CARET_H__
