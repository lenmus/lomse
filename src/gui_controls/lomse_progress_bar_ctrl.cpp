//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_progress_bar_ctrl.h"
#include "lomse_internal_model.h"
#include "lomse_shapes.h"
#include "private/lomse_document_p.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_text.h"
#include "lomse_drawer.h"
#include "lomse_calligrapher.h"
#include "lomse_events.h"

namespace lomse
{

//=======================================================================================
// ProgressBarCtrl implementation
//=======================================================================================
ProgressBarCtrl::ProgressBarCtrl(LibraryScope& libScope, Control* pParent,
                                 Document* pDoc, float maxValue,
                                 LUnits width, LUnits height, ImoStyle* pStyle)
    : Control(libScope, pDoc, pParent)
    , m_label("0.0%")
    , m_maxValue(maxValue)
    , m_pMainBox(nullptr)
    , m_width(width)
    , m_height(height)
    , m_xCenter(0.0f)
    , m_yCenter(0.0f)
    , m_curValue(0.0f)
    , m_percent(0.0f)
    , m_fDisplayPercentage(true)
{
    pStyle = (pStyle == nullptr ? create_default_style() : pStyle);
    m_styleId = pStyle->get_id();

    m_textColor = pStyle->color();
    m_barColor = Color(0,255,0,255);  //green solid

    measure();

    if (pParent)
        pParent->take_ownership_of(this);
}

//---------------------------------------------------------------------------------------
ImoStyle* ProgressBarCtrl::create_default_style()
{
    ImoStyle* style = m_pDoc->create_private_style();
    style->border_width(0.0f)->padding(0.0f)->margin(0.0f);
    style->color( Color(0,0,0) )->font_name("sans")->font_size(10.0f);
    style->text_align(ImoTextStyle::k_align_left);
    return style;
}

//---------------------------------------------------------------------------------------
USize ProgressBarCtrl::measure()
{
    if (m_height < 0.0f)
    {
        select_font();
        TextMeter meter(m_libraryScope);
        m_height = meter.get_font_height() * 1.20f;
    }
    return USize(m_width, m_height);
}

//---------------------------------------------------------------------------------------
GmoBoxControl* ProgressBarCtrl::layout(LibraryScope& UNUSED(libraryScope), UPoint pos)
{
    m_pos = pos;
    m_pMainBox = LOMSE_NEW GmoBoxControl(this, m_pos, m_width, m_height, get_style());
    return m_pMainBox;
}

//---------------------------------------------------------------------------------------
void ProgressBarCtrl::handle_event(SpEventInfo UNUSED(pEvent))
{
}

//---------------------------------------------------------------------------------------
void ProgressBarCtrl::set_value(float value)
{
    m_curValue = value;

    if (m_maxValue > 0.0f)
        m_percent = value / m_maxValue;
    else
        m_percent = 0.0f;

    stringstream ss;
    if (m_fDisplayPercentage)
        ss << fixed << setprecision(1) << (m_percent * 100.0f) << "%";
    else
        ss << fixed << setprecision(0) << value;
    m_label = ss.str();

    if (m_pMainBox)
        m_pMainBox->set_dirty(true);
}

//---------------------------------------------------------------------------------------
URect ProgressBarCtrl::determine_text_position_and_size()
{
    URect pos;

    //select_font();    //AWARE: when invoked, font is already selected
    TextMeter meter(m_libraryScope);
    pos.width = meter.measure_width(m_label);
    pos.height = meter.get_font_height();
    pos.y = m_pos.y + m_height;
    pos.x = m_pos.x + (m_width - pos.width)/2.0f;

    return pos;
}

//---------------------------------------------------------------------------------------
void ProgressBarCtrl::set_tooltip(const string& UNUSED(text))
{
    //TODO: ProgressBarCtrl::set_tooltip
}

//---------------------------------------------------------------------------------------
void ProgressBarCtrl::on_draw(Drawer* pDrawer, RenderOptions& UNUSED(opt))
{
    select_font();
    URect pos = determine_text_position_and_size();

    //progress bar
    if (m_percent > 0.0f)
    {
        pDrawer->begin_path();
        pDrawer->fill( m_barColor );
        pDrawer->stroke( m_barColor );
        pDrawer->stroke_width( pos.height * 0.075f );
        pDrawer->rect(UPoint(m_pos.x, m_pos.y +100.0f),
                             USize(m_width * m_percent, m_height), 50.0f);
        pDrawer->end_path();
    }

    //border
    pDrawer->begin_path();
    pDrawer->fill( Color(0,0,0,0) );  //black transparent
    pDrawer->stroke( Color(0,0,0,255) );  //black solid
    pDrawer->stroke_width( pos.height * 0.075f );
    pDrawer->rect(UPoint(m_pos.x, m_pos.y +100.0f), USize(m_width, m_height), 50.0f);
    pDrawer->end_path();

    //text
    pDrawer->set_text_color( m_textColor );
    pDrawer->draw_text(pos.x, pos.y, m_label);
}


}   //namespace lomse
