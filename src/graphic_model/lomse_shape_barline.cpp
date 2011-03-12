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

#include "lomse_shape_barline.h"

#include "lomse_internal_model.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"


namespace lomse
{

//=======================================================================================
// GmoShapeBarline implementation
//=======================================================================================
GmoShapeBarline::GmoShapeBarline(ImoObj* pCreatorImo, int idx, int nBarlineType,
                                 LUnits xPos, LUnits yTop,
						         LUnits yBottom, LUnits uThinLineWidth,
                                 LUnits uThickLineWidth, LUnits uSpacing,
                                 LUnits uRadius, Color color, LUnits uMinWidth)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_barline, idx, color)
    , m_nBarlineType(nBarlineType)
    , m_uxTop(xPos)
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
        case ImoBarline::k_double:
            width = m_uThinLineWidth + m_uSpacing + m_uThinLineWidth;
            break;

        case ImoBarline::k_end_repetition:
            width = m_uRadius + m_uSpacing + m_uRadius + m_uThinLineWidth +
                    m_uSpacing + m_uThickLineWidth;
            break;

        case ImoBarline::k_start_repetition:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth +
                    m_uSpacing + m_uRadius + m_uRadius;
            break;

        case ImoBarline::k_double_repetition:
            width = m_uRadius + m_uSpacing + m_uRadius + m_uThinLineWidth + m_uSpacing +
                    m_uThinLineWidth + m_uSpacing + m_uRadius + m_uRadius;
            break;

        case ImoBarline::k_start:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth;
            break;

        case ImoBarline::k_end:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth;
            break;

        case ImoBarline::k_simple:
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
    m_uxTop += shift.width;
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color =  determine_color_to_use(opt) ;
    LUnits uxPos = m_uxTop;
    LUnits uyTop = m_origin.y;
    LUnits uyBottom = m_origin.y + m_size.height;

    switch(m_nBarlineType)
    {
        case ImoBarline::k_double:
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            break;

        case ImoBarline::k_end_repetition:
            uxPos += m_uRadius;
            draw_two_dots(pDrawer, uxPos, uyTop);
            uxPos += m_uSpacing + m_uRadius;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            break;

        case ImoBarline::k_start_repetition:
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            uxPos += m_uThickLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing + m_uRadius;
            draw_two_dots(pDrawer, uxPos, uyTop);
            break;

        case ImoBarline::k_double_repetition:
            uxPos += m_uRadius;
            draw_two_dots(pDrawer, uxPos, uyTop);
            uxPos += m_uSpacing + m_uRadius;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing + m_uRadius;
            draw_two_dots(pDrawer, uxPos, uyTop);
            break;

        case ImoBarline::k_start:
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            uxPos += m_uThickLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            break;

        case ImoBarline::k_end:
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thick_line(pDrawer, uxPos, uyTop, m_uThickLineWidth, uyBottom-uyTop, color);
            break;

        case ImoBarline::k_simple:
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            break;
    }
    pDrawer->render(true);

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_thin_line(Drawer* pDrawer, LUnits uxPos, LUnits uyTop, LUnits uyBottom,
                             Color color)
{
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->line(uxPos + m_uThinLineWidth/2, uyTop,
                  uxPos + m_uThinLineWidth/2, uyBottom,
                  m_uThinLineWidth, k_edge_normal);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_thick_line(Drawer* pDrawer, LUnits xLeft, LUnits uyTop, LUnits uWidth,
                              LUnits uHeight, Color color)
{
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->line(xLeft + uWidth/2, uyTop,
                  xLeft + uWidth/2, uyTop + uHeight,
                  uWidth, k_edge_normal);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_two_dots(Drawer* pDrawer, LUnits uxPos, LUnits uyPos)
{
    //TODO
    LUnits uShift1 = m_uSpacing * 3.0f;   //((lmStaffObj*)m_pOwner)->TenthsToLogical(15);	// 1.5 lines
    LUnits uShift2 = m_uSpacing * 5.0f;   //((lmStaffObj*)m_pOwner)->TenthsToLogical(25); // 2.5 lines
    pDrawer->begin_path();
    pDrawer->circle(uxPos, uyPos + uShift1, m_uRadius);
    pDrawer->circle(uxPos, uyPos + uShift2, m_uRadius);
    pDrawer->end_path();
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
//    wxBitmap* pBitmap = new wxBitmap(image);
//
//    ////DBG -----------
//    //wxString sFileName = _T("ShapeGlyp2.bmp");
//    //pBitmap->SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    return pBitmap;
//}



}  //namespace lomse
