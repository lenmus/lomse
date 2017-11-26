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

#include "lomse_time_grid.h"

#include "lomse_screen_drawer.h"
#include "lomse_graphic_view.h"
#include "lomse_logger.h"
#include "lomse_box_system.h"
#include "lomse_timegrid_table.h"


namespace lomse
{

//=======================================================================================
// TimeGrid implementation
//=======================================================================================
TimeGrid::TimeGrid(GraphicView* view, LibraryScope& libraryScope)
    : VisualEffect(view, libraryScope)
    , m_color( Color(192,192,192) )     //light grey  //TODO: user option
    , m_pBoxSystem(nullptr)
{
}

//---------------------------------------------------------------------------------------
void TimeGrid::on_draw(ScreenDrawer* pDrawer)
{
    m_bounds = URect(0.0, 0.0, 0.0, 0.0);
    if (!m_pBoxSystem)
        return;

    TimeGridTable* pGridTable = m_pBoxSystem->get_time_grid_table();

    //determine system top/bottom limits
    double yTop = double(m_pBoxSystem->get_top());
    double yBottom = double(m_pBoxSystem->get_bottom());
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
    {
        yTop -= double(pStyle->margin_top());
        yBottom -= double(pStyle->margin_bottom());
    }

    //draw the grid lines
    pDrawer->begin_path();
    pDrawer->fill(m_color);
    pDrawer->stroke(m_color);
    pDrawer->stroke_width(15.0);    //0.15 mm
    int iMax = pGridTable->get_size();
    for (int i=0; i < iMax; ++i)
    {
        double xLeft = double( pGridTable->get_x_pos(i));
        pDrawer->move_to(xLeft, yTop);
        pDrawer->vline_to(yBottom);
    }
    pDrawer->end_path();

    if (m_pBoxSystem)
    {
        UPoint org = m_pView->get_page_origin_for(m_pBoxSystem);
        pDrawer->set_shift(-org.x, -org.y);
    }
    pDrawer->render();
    pDrawer->remove_shift();

    if (iMax > 0)
        m_bounds = URect(UPoint(pGridTable->get_x_pos(0)-8, yTop),
                         UPoint(pGridTable->get_x_pos(iMax-1)+8, yBottom) );
}


}  //namespace lomse
