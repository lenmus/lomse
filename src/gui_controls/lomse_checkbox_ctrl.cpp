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

#include "lomse_checkbox_ctrl.h"
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
// CheckboxCtrl implementation
//=======================================================================================

Vertex CheckboxCtrl::m_tickVertices[] = {
    { 384.0, 151.0, agg::path_cmd_move_to },
    { 323.0, 196.0, agg::path_cmd_curve3 },
    { 260.0, 279.0, agg::path_cmd_curve3 },    //on-curve
    { 201.0, 357.0, agg::path_cmd_curve3 },
    { 165.0, 431.0, agg::path_cmd_curve3 },    //on-curve
    { 137.0, 450.0, agg::path_cmd_curve3 },
    { 115.0, 467.0, agg::path_cmd_curve3 },    //on-curve
    {  82.0, 357.0, agg::path_cmd_curve3 },
    {  37.0, 339.0, agg::path_cmd_curve3 },    //on-curve
    {  62.0, 310.0, agg::path_cmd_curve3 },
    {  87.0, 310.0, agg::path_cmd_curve3 },    //on-curve
    { 108.0, 310.0, agg::path_cmd_curve3 },
    { 137.0, 380.0, agg::path_cmd_curve3 },    //on-curve
    { 234.0, 219.0, agg::path_cmd_curve3 },
    { 375.0, 138.0, agg::path_cmd_curve3 },    //on-curve
};

const int CheckboxCtrl::m_nNumVertices
            = sizeof(CheckboxCtrl::m_tickVertices)/sizeof(Vertex);

//---------------------------------------------------------------------------------------
CheckboxCtrl::CheckboxCtrl(LibraryScope& libScope, Control* pParent,
                           Document* pDoc, const string& label,
                           LUnits width, LUnits height, ImoStyle* pStyle)
    : Control(libScope, pDoc, pParent)
    , m_label(label)
    , m_pMainBox(nullptr)
    , m_width(width)
    , m_height(height)
    , m_xCenter(0.0f)
    , m_yCenter(0.0f)
    , m_hoverColor( Color(255, 0, 0) )      //red
    , m_status(false)
    , m_nCurVertex(0)
{
    m_style = (pStyle == nullptr ? create_default_style() : pStyle);

    m_normalColor = m_style->color();
    m_prevColor = m_normalColor;
    m_currentColor = m_normalColor;

    measure();

    if (pParent)
        pParent->take_ownership_of(this);
}

//---------------------------------------------------------------------------------------
ImoStyle* CheckboxCtrl::create_default_style()
{
    ImoStyle* style = m_pDoc->create_private_style();
    style->border_width(0.0f)->padding(0.0f)->margin(0.0f);
    style->color( Color(0,0,0) )->font_name("sans");
    style->text_align(ImoTextStyle::k_align_left);
    return style;
}

//---------------------------------------------------------------------------------------
USize CheckboxCtrl::measure()
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
GmoBoxControl* CheckboxCtrl::layout(LibraryScope& UNUSED(libraryScope), UPoint pos)
{
    m_pos = pos;
    m_pMainBox = LOMSE_NEW GmoBoxControl(this, m_pos, m_width, m_height, m_style);
    return m_pMainBox;
}

//---------------------------------------------------------------------------------------
void CheckboxCtrl::handle_event(SpEventInfo pEvent)
{
    if (m_fEnabled)
    {
        if (pEvent->is_mouse_in_event())
        {
            m_currentColor = m_hoverColor;
            m_pMainBox->set_dirty(true);
        }
        else if (pEvent->is_mouse_out_event())
        {
            m_currentColor = m_normalColor;
            m_pMainBox->set_dirty(true);
        }
        else if (pEvent->is_on_click_event())
        {
            m_status = !m_status;
            m_pMainBox->set_dirty(true);
            //force to repaint inmediately
            m_currentColor = m_normalColor;
            m_pDoc->set_dirty();
            m_pDoc->notify_if_document_modified();
        }

        m_pDoc->notify_observers(pEvent, this);
    }
}

//---------------------------------------------------------------------------------------
void CheckboxCtrl::set_text(const string& text)
{
    m_label = text;
    if (m_pMainBox)
        m_pMainBox->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void CheckboxCtrl::change_label(const string& text)
{
    set_text(text);
}

//---------------------------------------------------------------------------------------
URect CheckboxCtrl::determine_text_position_and_size()
{
    URect pos;

    //select_font();    //AWARE: font already selected
    TextMeter meter(m_libraryScope);
    pos.width = meter.measure_width(m_label);
    pos.height = meter.get_font_height();
    pos.y = m_pos.y + (pos.height + m_height) / 2.0f;
    pos.x = m_pos.x;

    return pos;
}

//---------------------------------------------------------------------------------------
void CheckboxCtrl::set_tooltip(const string& UNUSED(text))
{
    //TODO: CheckboxCtrl::set_tooltip
}

//---------------------------------------------------------------------------------------
void CheckboxCtrl::on_draw(Drawer* pDrawer, RenderOptions& UNUSED(opt))
{
    select_font();
    Color color = (m_fEnabled ? m_currentColor : Color(192, 192, 192));
    pDrawer->set_text_color(color);
    URect pos = determine_text_position_and_size();

    pDrawer->begin_path();
    pDrawer->fill( Color(0,0,0,0) );
    pDrawer->stroke(color);
    pDrawer->stroke_width( pos.height * 0.075f );
    pDrawer->rect(UPoint(m_pos.x, m_pos.y +100.0f), USize(400.0f, 400.0f), 50.0f);
    pDrawer->end_path();

    pDrawer->draw_text(pos.x + 600.0f, pos.y, m_label);

    if (m_status)
    {
//        pDrawer->draw_text(pos.x + 50.0f, pos.y, "X");
        pDrawer->begin_path();
        pDrawer->fill(color);
        pDrawer->add_path(*this);
        pDrawer->end_path();
    }
}

//---------------------------------------------------------------------------------------
void CheckboxCtrl::select_font()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_language,
                      m_style->font_file(),
                      m_style->font_name(),
                      m_style->font_size(),
                      m_style->is_bold(),
                      m_style->is_italic() );
}

//---------------------------------------------------------------------------------------
unsigned CheckboxCtrl::vertex(double* px, double* py)
{
	if(m_nCurVertex >= m_nNumVertices)
		return agg::path_cmd_stop;

	*px = m_tickVertices[m_nCurVertex].ux_coord + m_pos.x;
	*py = m_tickVertices[m_nCurVertex].uy_coord + m_pos.y;

	return m_tickVertices[m_nCurVertex++].cmd;
}


}   //namespace lomse
