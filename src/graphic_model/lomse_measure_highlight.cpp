//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_measure_highlight.h"

#include "lomse_bitmap_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_logger.h"
#include "lomse_graphic_view.h"
#include "lomse_box_system.h"
#include "lomse_shape_staff.h"
#include "lomse_vertex_source.h"
#include "agg_trans_affine.h"


namespace lomse
{


//=======================================================================================
// MeasureHighlight implementation
//=======================================================================================
MeasureHighlight::MeasureHighlight(GraphicView* view, LibraryScope& libraryScope)
    : ApplicationMark(view, libraryScope)
    , m_color(Color(255,0,0,64))   // transparent red
    , m_pBoxSystem(nullptr)
    , m_iPage(0)
    , m_xLeft(1000.0)       //arbitrary, to have a valid value
    , m_yTop(1000.0)        //arbitrary, to have a valid value
    , m_yBottom(2000.0)     //arbitrary, to have a valid value
    , m_extension(300.0)    //3.0 mm
{
}

//---------------------------------------------------------------------------------------
void MeasureHighlight::initialize(LUnits xLeft, LUnits xRight, GmoBoxSystem* pBoxSystem)
{
    //This method is protected. It is only invoked one time, when the mark is created.

    m_xLeft = xLeft;
    m_xRight = xRight;

    if (pBoxSystem)
    {
        m_pBoxSystem = pBoxSystem;
        m_iPage = pBoxSystem->get_page_number();

        top(0,0);       //top point in first instrument, first staff
        bottom(-1,-1);  //bottom point at last instrument, last staff

        //determine extensiton to reach system bounds
        LUnits height = m_yBottom - m_yTop;
        m_extension = (pBoxSystem->get_height() - height) / 2.0f;

        //fix xLeft with style margins
        ImoStyle* pStyle = pBoxSystem->get_style();
        if (pStyle)
        {
            m_xLeft += pStyle->margin_left();
            m_xRight += pStyle->margin_left();
        }
    }
}

//---------------------------------------------------------------------------------------
MeasureHighlight* MeasureHighlight::top(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0f;

    //set top point
    URect staffBounds = pStaff->get_bounds();
    m_yTop = staffBounds.y;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yTop -= pStyle->margin_top();

    return this;
}

//---------------------------------------------------------------------------------------
MeasureHighlight* MeasureHighlight::bottom(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0f;

    //set bottom point
    URect staffBounds = pStaff->get_bounds();
    m_yBottom = staffBounds.y + staffBounds.height;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yBottom -= pStyle->margin_top();

    return this;
}

//---------------------------------------------------------------------------------------
MeasureHighlight* MeasureHighlight::extra_height(Tenths value)
{
    if (!m_pBoxSystem)
        return this;

    m_extension = m_pBoxSystem->tenths_to_logical(value);
    return this;
}

//---------------------------------------------------------------------------------------
void MeasureHighlight::on_draw(BitmapDrawer* pDrawer)
{
    if (!m_pBoxSystem)
        return;

    LUnits yTop = m_yTop - m_extension;
    LUnits yBottom = m_yBottom + m_extension;

    UPoint org = m_pView->get_page_origin_for(m_iPage);
    pDrawer->set_shift(-org.x, -org.y);

    //draw the rectangle
    pDrawer->begin_path();
    pDrawer->fill(m_color);
    pDrawer->stroke_none();

    pDrawer->rect(UPoint(m_xLeft, yTop), USize(m_xRight - m_xLeft, yBottom-yTop), 0.0);

    m_bounds.left(m_xLeft);
    m_bounds.top(yTop);
    m_bounds.bottom(yBottom);
    m_bounds.right(m_xRight);

    pDrawer->end_path();

    pDrawer->render();
    pDrawer->remove_shift();
}

//---------------------------------------------------------------------------------------
URect MeasureHighlight::get_bounds()
{
    return m_bounds;
}


}  //namespace lomse
