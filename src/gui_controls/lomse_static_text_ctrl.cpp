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

#include "lomse_static_text_ctrl.h"
#include "lomse_internal_model.h"
#include "lomse_shapes.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_text.h"
#include "lomse_drawer.h"
#include "lomse_calligrapher.h"
#include "lomse_events.h"
#include "lomse_dyn_generator.h"

namespace lomse
{

//=======================================================================================
// StaticTextCtrl implementation
//=======================================================================================
StaticTextCtrl::StaticTextCtrl(LibraryScope& libScope, DynGenerator* pOwner,
                               Document* pDoc, const string& label,
                               LUnits width, LUnits height, ImoStyle* pStyle)
    : Control(pOwner, pDoc)
    , m_libraryScope(libScope)
    , m_label(label)
    , m_pMainBox(NULL)
    , m_style(pStyle)
    , m_width(width)
    , m_height(height)
{
    if (!m_style)
        m_style = create_default_style();

    measure();

    pOwner->accept_control_ownership(this);
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
GmoBoxControl* StaticTextCtrl::layout(LibraryScope& libraryScope, UPoint pos)
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
    notify_observers(pEvent, this);
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
    int align = m_style->get_int_property(ImoStyle::k_text_align);
    URect pos;

    //select_font();    //AWARE: font already selected
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
void StaticTextCtrl::set_tooltip(const string& text)
{
    //TODO
}

//---------------------------------------------------------------------------------------
void StaticTextCtrl::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    select_font();
    Color color = m_style->get_color_property(ImoStyle::k_color);
    pDrawer->set_text_color(color);
    URect pos = determine_text_position_and_size();
    pDrawer->draw_text(pos.x, pos.y, m_label);

    //text decoration
    if (m_style->get_int_property(ImoStyle::k_text_decoration) == ImoStyle::k_decoration_underline)
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

//---------------------------------------------------------------------------------------
void StaticTextCtrl::select_font()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_style->get_string_property(ImoStyle::k_font_name),
                      m_style->get_float_property(ImoStyle::k_font_size),
                      m_style->is_bold(),
                      m_style->is_italic() );
}


}   //namespace lomse
