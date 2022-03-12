//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef _LOMSE_STATIC_TEXT_CTRL_H__
#define _LOMSE_STATIC_TEXT_CTRL_H__

#include "lomse_control.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
// A Control for displaying one or more lines of read-only text, inside an optional box
class StaticTextCtrl : public Control
{
protected:
    string m_label;
    GmoBoxControl* m_pMainBox;
    UPoint  m_pos;
    LUnits  m_width;
    LUnits  m_height;
    LUnits  m_xCenter;
    LUnits  m_yCenter;

public:
    StaticTextCtrl(LibraryScope& libScope, Control* pParent, Document* pDoc,
                   const string& label, LUnits width=-1.0f, LUnits height=-1.0f,
                   ImoStyle* pStyle=nullptr);
    virtual ~StaticTextCtrl() {}

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
    void set_text(const string& text);
    void set_tooltip(const string& text);

protected:
    URect determine_text_position_and_size();
    ImoStyle* create_default_style();

};


} //namespace lomse

#endif    //_LOMSE_STATIC_TEXT_CTRL_H__
