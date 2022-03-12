//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef _LOMSE_PROGRESS_BAR_CTRL_H__
#define _LOMSE_PROGRESS_BAR_CTRL_H__

#include "lomse_control.h"

#include "lomse_vertex_source.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
// A progress bar
class ProgressBarCtrl : public Control
{
protected:
    string m_label;
    float m_maxValue;
    GmoBoxControl* m_pMainBox;
    UPoint  m_pos;
    LUnits  m_width;
    LUnits  m_height;
    LUnits  m_xCenter;
    LUnits  m_yCenter;

    Color   m_textColor;
    Color   m_barColor;

    float   m_curValue;
    float   m_percent;
    bool    m_fDisplayPercentage;

public:
    ProgressBarCtrl(LibraryScope& libScope, Control* pParent, Document* pDoc,
                    float maxValue, LUnits width, LUnits height=-1.0f,
                    ImoStyle* pStyle=nullptr);
    virtual ~ProgressBarCtrl() {}

    //Control mandatory overrides
    USize measure() override;
    GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos) override;
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
    void handle_event(SpEventInfo pEvent) override;
    LUnits width() override { return m_width; }
    LUnits height() override { return m_height; }
    LUnits top() override { return m_pos.y; }
    LUnits bottom() override { return m_pos.y + m_height; }
    LUnits left() override { return m_pos.x; }
    LUnits right() override { return m_pos.x + m_width; }

    //specific methods
    void set_value(float value);
    inline float get_value() { return m_curValue; }

    inline void set_percentage_mode(bool value) { m_fDisplayPercentage = value; }
    inline bool is_in_percentage_mode() { return m_fDisplayPercentage; }

    void set_tooltip(const string& text);
    inline void set_bar_color(Color color) { m_barColor = color; }


protected:
    URect determine_text_position_and_size();
    ImoStyle* create_default_style();

};


} //namespace lomse

#endif    //_LOMSE_PROGRESS_BAR_CTRL_H__
