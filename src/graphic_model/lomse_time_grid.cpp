//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_time_grid.h"

#include "lomse_bitmap_drawer.h"
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
void TimeGrid::on_draw(BitmapDrawer* pDrawer)
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
        m_bounds = URect(UPoint(pGridTable->get_x_pos(0)-8, Tenths(yTop)),
                         UPoint(pGridTable->get_x_pos(iMax-1)+8, Tenths(yBottom)) );
}


}  //namespace lomse
