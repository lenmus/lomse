//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_shape_barline.h"

#include "lomse_internal_model.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"


namespace lomse
{

//=======================================================================================
// GmoShapeBarline implementation
//=======================================================================================
GmoShapeBarline::GmoShapeBarline(ImoObj* pCreatorImo, ShapeId idx, int nBarlineType,
                                 LUnits xPos, LUnits yTop,
						         LUnits yBottom, LUnits uThinLineWidth,
                                 LUnits uThickLineWidth, LUnits uSpacing,
                                 LUnits uRadius, Color color, LUnits uMinWidth)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_barline, idx, color)
    , m_nBarlineType(nBarlineType)
    , m_uxLeft(xPos)
    , m_uThinLineWidth(uThinLineWidth)
    , m_uThickLineWidth(uThickLineWidth)
    , m_uSpacing(uSpacing)
    , m_uRadius(uRadius)
{
    m_origin.x = xPos;
    m_origin.y = yTop;
    m_size.height = yBottom - yTop;
    m_size.width = uMinWidth;
    compute_width();
    determine_lines_relative_positions();
}

//---------------------------------------------------------------------------------------
GmoShapeBarline::~GmoShapeBarline()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::compute_width()
{
    LUnits width;
    switch(m_nBarlineType)
    {
        case k_barline_double:
            width = m_uThinLineWidth + m_uSpacing + m_uThinLineWidth;
            break;

        case k_barline_end_repetition:
            width = m_uRadius + m_uRadius + m_uSpacing + m_uThinLineWidth +
                    m_uSpacing + m_uThickLineWidth;
            break;

        case k_barline_start_repetition:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth +
                    m_uSpacing + m_uRadius + m_uRadius;
            break;

        case k_barline_double_repetition:
            width = m_uRadius + m_uSpacing + m_uRadius + m_uThinLineWidth + m_uSpacing +
                    m_uThinLineWidth + m_uSpacing + m_uRadius + m_uRadius;
            break;

        case k_barline_double_repetition_alt:
            width = m_uRadius + m_uSpacing + m_uRadius + m_uThickLineWidth + m_uSpacing +
                    m_uThickLineWidth + m_uSpacing + m_uRadius + m_uRadius;
            break;

        case k_barline_start:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth;
            break;

        case k_barline_end:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth;
            break;

        case k_barline_simple:
            width = m_uThinLineWidth;
            break;

        default:
            LOMSE_LOG_ERROR("Invalid barline type %d", m_nBarlineType);
            width = m_uThinLineWidth;
            break;
    }

    if (width < m_size.width)
        m_origin.x -= (m_size.width - width) / 2.0f;
    else
        m_size.width = width;
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::shift_origin(const USize& shift)
{
    GmoObj::shift_origin(shift);
    m_uxLeft += shift.width;
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::determine_lines_relative_positions()
{
    LUnits uxPos = 0;

    switch(m_nBarlineType)
    {
        case k_barline_double:
            m_xLeftLine = uxPos;
            uxPos += m_uThinLineWidth + m_uSpacing;
            m_xRightLine = uxPos + m_uThinLineWidth;
            break;

        case k_barline_end_repetition:
            uxPos += m_uRadius * 2.7f;   //BUG-BYPASS: Need to shift right the drawing
            uxPos += m_uRadius + m_uSpacing;
            m_xLeftLine = uxPos;
            uxPos += m_uThinLineWidth + m_uSpacing;
            m_xRightLine = uxPos + m_uThickLineWidth;
            break;

        case k_barline_start_repetition:
            m_xLeftLine = uxPos;
            uxPos += m_uThickLineWidth + m_uSpacing;
            m_xRightLine = uxPos + m_uThickLineWidth;
            break;

        case k_barline_double_repetition:
            uxPos += m_uRadius;
            uxPos += m_uSpacing + m_uRadius;
            m_xLeftLine = uxPos;
            uxPos += m_uThinLineWidth + m_uSpacing;
            m_xRightLine = uxPos + m_uThinLineWidth;
            break;

        case k_barline_double_repetition_alt:
            uxPos += m_uRadius;
            uxPos += m_uSpacing + m_uRadius;
            m_xLeftLine = uxPos;
            uxPos += m_uThickLineWidth + m_uSpacing;
            m_xRightLine = uxPos + m_uThickLineWidth;
            break;

        case k_barline_start:
            m_xLeftLine = uxPos;
            uxPos += m_uThickLineWidth + m_uSpacing;
            m_xRightLine = uxPos + m_uThinLineWidth;
            break;

        case k_barline_end:
            m_xLeftLine = uxPos;
            uxPos += m_uThinLineWidth + m_uSpacing;
            m_xRightLine = uxPos + m_uThickLineWidth;
            break;

        case k_barline_simple:
            m_xLeftLine = uxPos;
            m_xRightLine = uxPos + m_uThinLineWidth;
            break;
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color =  determine_color_to_use(opt) ;
    LUnits uxPos = m_uxLeft;
    LUnits uyTop = m_origin.y;
    LUnits uyBottom = m_origin.y + m_size.height;

    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation(get_notation_id(), get_notation_class());

    switch(m_nBarlineType)
    {
        case k_barline_double:
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            break;

        case k_barline_end_repetition:
            //uxPos += m_uRadius;
            uxPos += m_uRadius * 2.7f;   //BUG-BYPASS: Need to shift right the drawing
            draw_repeat_dots_for_all_staves(pDrawer, uxPos, uyTop, color);
            uxPos += m_uRadius + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            break;

        case k_barline_start_repetition:
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            uxPos += m_uThickLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing + m_uRadius;
            draw_repeat_dots_for_all_staves(pDrawer, uxPos, uyTop, color);
            break;

        case k_barline_double_repetition:
            uxPos += m_uRadius;
            draw_repeat_dots_for_all_staves(pDrawer, uxPos, uyTop, color);
            uxPos += m_uSpacing + m_uRadius;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing + m_uRadius;
            draw_repeat_dots_for_all_staves(pDrawer, uxPos, uyTop, color);
            break;

        case k_barline_double_repetition_alt:
            uxPos += m_uRadius;
            draw_repeat_dots_for_all_staves(pDrawer, uxPos, uyTop, color);
            uxPos += m_uSpacing + m_uRadius;
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            uxPos += m_uThickLineWidth + m_uSpacing;
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            uxPos += m_uThickLineWidth + m_uSpacing + m_uRadius;
            draw_repeat_dots_for_all_staves(pDrawer, uxPos, uyTop, color);
            break;

        case k_barline_start:
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            uxPos += m_uThickLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            break;

        case k_barline_end:
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            break;

        case k_barline_simple:
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            break;
    }
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_thin_line(Drawer* pDrawer, LUnits uxPos, LUnits uyTop,
                                     LUnits uyBottom, Color color)
{
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->stroke(color);
    pDrawer->line(uxPos + m_uThinLineWidth/2, uyTop,
                  uxPos + m_uThinLineWidth/2, uyBottom,
                  m_uThinLineWidth, k_edge_normal);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_thick_line(Drawer* pDrawer, LUnits uxPos, LUnits uyTop,
                                      LUnits uWidth, LUnits uHeight, Color color)
{
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->stroke(color);
    pDrawer->line(uxPos + uWidth/2, uyTop,
                  uxPos + uWidth/2, uyTop + uHeight,
                  uWidth, k_edge_normal);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_two_dots(Drawer* pDrawer, LUnits uxPos, LUnits uyPos,
                                    Color color)
{
    LUnits uShift1 = m_uSpacing * 3.7f;
    LUnits uShift2 = m_uSpacing * 6.1f;
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->stroke(color);
    pDrawer->circle(uxPos, uyPos + uShift1, m_uRadius);
    pDrawer->circle(uxPos, uyPos + uShift2, m_uRadius);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_repeat_dots_for_all_staves(Drawer* pDrawer, LUnits uxPos, LUnits uyPos, Color color)
{
    if (m_relStaffTopPositions.empty())
    {
        draw_two_dots(pDrawer, uxPos, uyPos, color);
        return;
    }

    for (LUnits relStaffTop : m_relStaffTopPositions)
    {
        const LUnits absStaffTop = uyPos + relStaffTop;
        draw_two_dots(pDrawer, uxPos, absStaffTop, color);
    }
}

////---------------------------------------------------------------------------------------
//wxBitmap* GmoShapeBarline::OnBeginDrag(double rScale, wxDC* pDC)
//{
//	// A dragging operation is started. The view invokes this method to request the
//	// bitmap to be used as drag image. No other action is required.
//	// If no bitmap is returned drag is cancelled.
//	//
//	// So this method returns the bitmap to use with the drag image.
//
//
//    // allocate the bitmap
//    // convert size to pixels
//    int wD = (int)pDC->LogicalToDeviceXRel((wxCoord)m_uWidth);
//    int hD = (int)pDC->LogicalToDeviceYRel((wxCoord)(m_uyBottom - m_uyTop));
//    wxBitmap bitmap(wD+2, hD+2);
//
//    // allocate a memory DC for drawing into a bitmap
//    wxMemoryDC dc2;
//    dc2.SelectObject(bitmap);
//    dc2.SetMapMode(lmDC_MODE);
//    dc2.SetUserScale(rScale, rScale);
//
//    // draw onto the bitmap
//    dc2.SetBackground(*wxRED_BRUSH);	//*wxWHITE_BRUSH);
//    dc2.Clear();
//    dc2.SetBackgroundMode(wxTRANSPARENT);
//    dc2.SetTextForeground(g_pColors->ScoreSelected());
//    //dc2.DrawText(sGlyph, 0, 0);
//
//    dc2.SelectObject(wxNullBitmap);
//
//    // Make the bitmap masked
//    wxImage image = bitmap.ConvertToImage();
//    image.SetMaskColour(255, 255, 255);
//    wxBitmap* pBitmap = LOMSE_NEW wxBitmap(image);
//
//    ////DBG -----------
//    //wxString sFileName = _T("ShapeGlyp2.bmp");
//    //pBitmap->SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    return pBitmap;
//}



}  //namespace lomse
