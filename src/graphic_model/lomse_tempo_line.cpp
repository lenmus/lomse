//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_tempo_line.h"

#include "lomse_bitmap_drawer.h"
#include "lomse_graphic_view.h"
#include "lomse_logger.h"
#include "lomse_box_system.h"
#include "lomse_timegrid_table.h"


namespace lomse
{

//=======================================================================================
// TempoLine implementation
//=======================================================================================
TempoLine::TempoLine(GraphicView* view, LibraryScope& libraryScope)
    : VisualEffect(view, libraryScope)
    , m_color( Color(0, 255, 0) )       // solid green
    , m_width(100.0)                    // 1.00 mm
    , m_pBoxSystem(nullptr)
    , m_iPage(0)
{
}

//---------------------------------------------------------------------------------------
void TempoLine::move_to(GmoShape* pShape, GmoBoxSystem* pBoxSystem)
{
    m_pBoxSystem = pBoxSystem;
    m_bounds = pShape->get_bounds();
}

//---------------------------------------------------------------------------------------
void TempoLine::move_to(LUnits xPos, GmoBoxSystem* pBoxSystem, int iPage)
{
    m_pBoxSystem = pBoxSystem;
    m_bounds.left(xPos);
    m_iPage = iPage;
}

//---------------------------------------------------------------------------------------
void TempoLine::on_draw(BitmapDrawer* pDrawer)
{
    if (!m_pBoxSystem)
        return;

    //determine system top/bottom limits
    double yTop = double(m_pBoxSystem->get_top());
    double yBottom = double(m_pBoxSystem->get_bottom());
    double xLeft = m_bounds.get_x();
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
    {
        yTop -= double(pStyle->margin_top());
        yBottom -= double(pStyle->margin_bottom());
        xLeft += double(pStyle->margin_left());
    }
    m_bounds.top(float(yTop));
    m_bounds.bottom(float(yBottom));
    m_bounds.width = m_width;

    //draw the tempo line
    pDrawer->begin_path();
    pDrawer->fill(m_color);
    pDrawer->stroke(m_color);
    pDrawer->stroke_width(m_bounds.width);

    UPoint org = m_pView->get_page_origin_for(m_iPage);
    pDrawer->set_shift(-org.x, -org.y);

    pDrawer->move_to(xLeft, yTop);
    pDrawer->vline_to(yBottom);

    pDrawer->end_path();
    pDrawer->render();
    pDrawer->remove_shift();
}

//---------------------------------------------------------------------------------------
void TempoLine::remove_tempo_line()
{
    m_pBoxSystem = nullptr;
}


}  //namespace lomse
