//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_static_text_ctrl.h"
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
// StaticTextCtrl implementation
//=======================================================================================
StaticTextCtrl::StaticTextCtrl(LibraryScope& libScope, Control* pParent,
                               Document* pDoc, const string& label,
                               LUnits width, LUnits height, ImoStyle* pStyle)
    : Control(libScope, pDoc, pParent)
    , m_label(label)
    , m_pMainBox(nullptr)
    , m_width(width)
    , m_height(height)
{
    m_style = (pStyle == nullptr ? create_default_style() : pStyle);

    measure();

    if (pParent)
        pParent->take_ownership_of(this);
}

//---------------------------------------------------------------------------------------
ImoStyle* StaticTextCtrl::create_default_style()
{
    return m_pDoc->get_default_style();
}

//---------------------------------------------------------------------------------------
USize StaticTextCtrl::measure()
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
GmoBoxControl* StaticTextCtrl::layout(LibraryScope& UNUSED(libraryScope), UPoint pos)
{
    m_pos = pos;
    m_pMainBox = LOMSE_NEW GmoBoxControl(this, m_pos, m_width, m_height, m_style);
    return m_pMainBox;
}

//---------------------------------------------------------------------------------------
void StaticTextCtrl::handle_event(SpEventInfo pEvent)
{
//    if (pEvent->is_mouse_in_event())
//        wxLogMessage(_T("Mouse In at StaticTextCtrl"));
//    else if (pEvent->is_mouse_out_event())
//        wxLogMessage(_T("Mouse Out at StaticTextCtrl"));
//    else if (pEvent->is_on_click_event())
//    {
//        wxLogMessage(_T("On Click at StaticTextCtrl"));
//    }
    m_pDoc->notify_observers(pEvent, this);
}

//---------------------------------------------------------------------------------------
void StaticTextCtrl::set_text(const string& text)
{
    m_label = text;
    m_pMainBox->set_dirty(true);
}

//---------------------------------------------------------------------------------------
URect StaticTextCtrl::determine_text_position_and_size()
{
    int align = m_style->text_align();
    URect pos;

    //select_font();    //AWARE: not needed as font is already selected
    TextMeter meter(m_libraryScope);
    pos.width = meter.measure_width(m_label);
    pos.height = meter.get_font_height();
    pos.y = m_pos.y + (pos.height + m_height) / 2.0f;

    switch (align)
    {
        case ImoTextStyle::k_align_left:
        {
            pos.x = m_pos.x;
            break;
        }
        case ImoTextStyle::k_align_right:
        {
            pos.x = m_pos.x + m_width - pos.width;
            break;
        }
        case ImoTextStyle::k_align_center:
        {
            pos.x = m_pos.x + (m_width - pos.width) / 2.0f;
            break;
        }
    }
    return pos;
}

//---------------------------------------------------------------------------------------
void StaticTextCtrl::set_tooltip(const string& UNUSED(text))
{
    //TODO: StaticTextCtrl::set_tooltip
}

//---------------------------------------------------------------------------------------
void StaticTextCtrl::on_draw(Drawer* pDrawer, RenderOptions& UNUSED(opt))
{
    select_font();
    Color color = m_style->color();
    pDrawer->set_text_color(color);
    URect pos = determine_text_position_and_size();
    pDrawer->draw_text(pos.x, pos.y, m_label);

    //text decoration
    if (m_style->text_decoration() == ImoStyle::k_decoration_underline)
    {
        LUnits y = pos.y + pos.height * 0.12f;
        pDrawer->begin_path();
        pDrawer->fill(color);
        pDrawer->stroke(color);
        pDrawer->stroke_width( pos.height * 0.075f );
        pDrawer->move_to(pos.x, y);
        pDrawer->hline_to( pos.right() );
        pDrawer->end_path();
    }
}


}   //namespace lomse
