//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#include "lomse_fragment_mark.h"

#include "lomse_screen_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_logger.h"
#include "lomse_graphic_view.h"
#include "lomse_box_system.h"
#include "lomse_shape_staff.h"


namespace lomse
{

//=======================================================================================
// FragmentMark implementation
//=======================================================================================
FragmentMark::FragmentMark(GraphicView* view, LibraryScope& libraryScope)
    : VisualEffect(view, libraryScope)
    , m_type(k_mark_line)
    , m_color(Color(255,0,0,128))   // transparent red
    , m_lineStyle(k_line_solid)
    , m_pBoxSystem(nullptr)
    , m_iPage(0)
    , m_yTop(1000.0)        //arbitrary, to have a valid value
    , m_yBottom(2000.0)     //arbitrary, to have a valid value
    , m_extension(300.0)    //3.0 mm
{
    m_bounds.width = 100.0;     // 1.00 mm
}

//---------------------------------------------------------------------------------------
void FragmentMark::move_to(LUnits xPos, GmoBoxSystem* pBoxSystem)
{
    //This method is protected. It is only invoked one time, when the mark is created.

    LUnits xLeft = xPos;

    if (pBoxSystem)
    {
        m_pBoxSystem = pBoxSystem;
        m_iPage = pBoxSystem->get_page_number();

        top(0,0);       //top point in first instrument, first staff
        bottom(-1,-1);  //bottom point at last instrument, last staff

        //determine extensiton to reach system bounds
        LUnits height = m_yBottom - m_yTop;
        m_extension = (pBoxSystem->get_height() - height) / 2.0;

        //fix xLeft with style margins
        ImoStyle* pStyle = pBoxSystem->get_style();
        if (pStyle)
            xLeft += pStyle->margin_left();
    }

    m_bounds.left(xLeft);
}

//---------------------------------------------------------------------------------------
FragmentMark* const FragmentMark::top(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0;

    //set top point
    URect staffBounds = pStaff->get_bounds();
    m_yTop = staffBounds.y;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yTop -= double(pStyle->margin_top());

    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* const FragmentMark::bottom(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0;

    //set bottom point
    URect staffBounds = pStaff->get_bounds();
    m_yBottom = staffBounds.y + staffBounds.height;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yBottom -= double(pStyle->margin_top());

    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* const FragmentMark::x_shift(LUnits dx)
{
    m_bounds.left( m_bounds.left() + dx );
    return this;
}

//---------------------------------------------------------------------------------------
void FragmentMark::on_draw(ScreenDrawer* pDrawer)
{
    LUnits yTop = m_yTop - m_extension;
    LUnits yBottom = m_yBottom + m_extension;

    pDrawer->begin_path();
    pDrawer->fill(m_color);
    pDrawer->stroke(m_color);
    pDrawer->stroke_width(m_bounds.width);

    UPoint org = m_pView->get_page_origin_for(m_iPage);
    pDrawer->set_shift(-org.x, -org.y);

    pDrawer->move_to(m_bounds.left(), yTop);
    pDrawer->vline_to(yBottom);

    pDrawer->end_path();
    pDrawer->render();
    pDrawer->remove_shift();

    m_bounds.top(yTop);
    m_bounds.bottom(yBottom);
}

//---------------------------------------------------------------------------------------
URect FragmentMark::get_bounds()
{
    return m_bounds;
}


}  //namespace lomse
