//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_button_ctrl.h"
#include "lomse_internal_model.h"
#include "lomse_shapes.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_text.h"
#include "lomse_drawer.h"
#include "lomse_calligrapher.h"
#include "lomse_events.h"

namespace lomse
{

//=======================================================================================
// ButtonCtrl implementation
//=======================================================================================
ButtonCtrl::ButtonCtrl(LibraryScope& libScope, Control* pParent,
                       Document* pDoc, const string& label,
                       LUnits width, LUnits height, ImoStyle* pStyle)
    : Control(libScope, pDoc, pParent)
    , m_label(label)
    , m_width(width)
    , m_height(height)
    , m_normalColor( Color(255,255,255) )       //white
    , m_overColor( Color(255,200,0) )           //orange
    , m_fMouseIn(false)
    , m_pMainBox(nullptr)
    , m_xLabel(0.0f)
    ,  m_yLabel(0.0f)
{
    m_style = (pStyle == nullptr ? create_default_style() : pStyle);

    measure();

    if (pParent)
        pParent->take_ownership_of(this);
}

//---------------------------------------------------------------------------------------
ImoStyle* ButtonCtrl::create_default_style()
{
    return m_pDoc->get_default_style();
}

//---------------------------------------------------------------------------------------
USize ButtonCtrl::measure()
{
    if (m_width < 0.0f || m_height < 0.0f)
    {
        select_font();
        TextMeter meter(m_libraryScope);
        if (m_width < 0.0f)
            m_width = meter.measure_width(m_label);
        if (m_height < 0.0f)
            m_height = meter.get_font_height();
    }
    return USize(m_width, m_height);
}

//---------------------------------------------------------------------------------------
GmoBoxControl* ButtonCtrl::layout(LibraryScope& UNUSED(libraryScope), UPoint pos)
{
    m_pos = pos;
    m_pMainBox = LOMSE_NEW GmoBoxControl(this, m_pos, m_width, m_height, m_style);
    return m_pMainBox;
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::handle_event(SpEventInfo pEvent)
{
    m_pDoc->notify_observers(pEvent, this);
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::set_label(const string& text)
{
    m_label = text;
    if (m_pMainBox)
        m_pMainBox->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::set_bg_color(Color color)
{
    m_normalColor = color;
    if (m_pMainBox)
        m_pMainBox->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::set_mouse_over_color(Color color)
{
    m_overColor = color;
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::set_mouse_in(bool fValue)
{
    m_fMouseIn = fValue;
    if (m_pMainBox)
        m_pMainBox->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::center_text()
{
    TextMeter meter(m_libraryScope);
    LUnits height = meter.get_font_height();
    LUnits width = meter.measure_width(m_label);

    m_xLabel = (m_width - width) / 2.0f;
    m_yLabel = (m_height + height) / 2.0f;     //reference is at text bottom
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::set_tooltip(const string& UNUSED(text))
{
    //TODO: ButtonCtrl::set_tooltip
}

//---------------------------------------------------------------------------------------
void ButtonCtrl::on_draw(Drawer* pDrawer, RenderOptions& UNUSED(opt))
{
    if (!is_visible())
        return;

    Color textColor = is_enabled() ? Color(0, 0, 0) : Color(128, 128, 128);
    Color strokeColor = Color(128, 128, 128);
    Color bgColor = (m_fMouseIn ? m_overColor : m_normalColor);

    //draw button gradient and border
    pDrawer->begin_path();
    pDrawer->fill( m_normalColor );
    pDrawer->stroke( strokeColor );
    pDrawer->stroke_width(15.0);
    Color white(255, 255, 255);
    Color dark(bgColor);
    dark.a = 45;
    Color light(dark);
    light = light.gradient(white, 0.2);
    pDrawer->gradient_color(white, 0.0, 0.1);
    pDrawer->gradient_color(white, dark, 0.1, 0.7);
    pDrawer->gradient_color(dark, light, 0.7, 1.0);
    pDrawer->fill_linear_gradient(m_pos.x, m_pos.y,
                                  m_pos.x, m_pos.y + m_height);
    pDrawer->rect(m_pos, USize(m_width, m_height), 100.0f);
    pDrawer->end_path();

    //draw text
    select_font();
    center_text();
    pDrawer->set_text_color( textColor );
    LUnits x = m_pos.x + m_xLabel;
    LUnits y = m_pos.y + m_yLabel;
    pDrawer->draw_text(x, y, m_label);
}


}   //namespace lomse
