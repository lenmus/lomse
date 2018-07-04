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

        case k_barline_start:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth;
            break;

        case k_barline_end:
            width = m_uThinLineWidth + m_uSpacing + m_uThickLineWidth;
            break;

        case k_barline_simple:
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
            draw_two_dots(pDrawer, uxPos, uyTop);
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
            draw_two_dots(pDrawer, uxPos, uyTop);
            break;

        case k_barline_double_repetition:
            uxPos += m_uRadius;
            draw_two_dots(pDrawer, uxPos, uyTop);
            uxPos += m_uSpacing + m_uRadius;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing;
            draw_thin_line(pDrawer, uxPos, uyTop, uyBottom, color);
            uxPos += m_uThinLineWidth + m_uSpacing + m_uRadius;
            draw_two_dots(pDrawer, uxPos, uyTop);
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
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_thin_line(Drawer* pDrawer, LUnits uxPos, LUnits uyTop,
                                     LUnits uyBottom, Color color)
{
    pDrawer->begin_path();
    pDrawer->fill(color);
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
    pDrawer->line(uxPos + uWidth/2, uyTop,
                  uxPos + uWidth/2, uyTop + uHeight,
                  uWidth, k_edge_normal);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeBarline::draw_two_dots(Drawer* pDrawer, LUnits uxPos, LUnits uyPos)
{
    LUnits uShift1 = m_uSpacing * 3.7f;
    LUnits uShift2 = m_uSpacing * 6.1f;
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
