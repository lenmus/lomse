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

#include "lomse_shapes.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"
#include "lomse_glyphs.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
// GmoShapeGlyph object implementation
//---------------------------------------------------------------------------------------
GmoShapeGlyph::GmoShapeGlyph(int type, int idx, unsigned int nGlyph,
                             UPoint pos, Color color, LibraryScope& libraryScope)
    : GmoSimpleShape(type, idx, color)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
{
    m_glyph = glyphs_lmbasic2[nGlyph].GlyphChar;

    pos.y += 1080.0f;     //TODO-LOG TenthsToLogical(60);

    //glyph bounds
    TextMeter meter(m_libraryScope);
    meter.select_font("LenMus basic", 21.0);
    URect bbox = meter.bounding_rectangle(m_glyph);

    m_origin.x = pos.x + bbox.x;
    m_origin.y = pos.y + bbox.y;
    m_size.width = bbox.width;
    m_size.height = bbox.height;

    m_shiftToDraw.width = pos.x - m_origin.x;
    m_shiftToDraw.height = pos.y - m_origin.y;
}

//---------------------------------------------------------------------------------------
void GmoShapeGlyph::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    pDrawer->select_font("LenMus basic", 21.0);
    pDrawer->set_text_color( determine_color_to_use(opt) );
    LUnits x = m_shiftToDraw.width + m_origin.x;
    LUnits y = m_shiftToDraw.height + m_origin.y;
    pDrawer->draw_glyph(x, y, m_glyph);

    GmoSimpleShape::on_draw(pDrawer, opt);
}

////---------------------------------------------------------------------------------------
//double GmoShapeGlyph::GetPointSize()
//{
//    lmComponentObj* pSO = ((lmComponentObj*)m_pOwner);
//    return pSO->GetStaff()->GetMusicFontSize();
//}
//
////---------------------------------------------------------------------------------------
//wxString GmoShapeGlyph::Dump(int nIndent)
//{
//	wxString sDump = _T("");
//	sDump.append(nIndent * lmINDENT_STEP, _T(' '));
//	sDump += wxString::Format(_T("Idx: %d %s: pos=(%.2f,%.2f), "),
//        m_nOwnerIdx, m_sGMOName.c_str(), m_uGlyphPos.x, m_uGlyphPos.y);
//    sDump += DumpBounds();
//    sDump += _T("\n");
//
//    //base class
//    sDump += GmoShape::Dump(nIndent);
//
//	return sDump;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeGlyph::Shift(LUnits xIncr, LUnits yIncr)
//{
//    m_uGlyphPos.x += xIncr;
//    m_uGlyphPos.y += yIncr;
//
//    ShiftBoundsAndSelRec(xIncr, yIncr);
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((lmCompositeShape*)GetParentShape())->RecomputeBounds();
//}
//
////---------------------------------------------------------------------------------------
//wxBitmap* GmoShapeGlyph::OnBeginDrag(double rScale, wxDC* pDC)
//{
//	// A dragging operation is started. The view invokes this method to request the
//	// bitmap to be used as drag image. No other action is required.
//
//    return GetBitmapFromShape(rScale, g_pColors->ScoreSelected());
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeGlyph::OnDrag(lmPaper* pPaper, const UPoint& uPos)
//{
//	// The view informs that the user continues dragging. We receive the new desired
//	// shape position and we must return the new allowed shape position.
//	//
//	// The default behaviour is to return the received position, so the view redraws
//	// the drag image at that position. No action must be performed by the shape on
//	// the score and score objects.
//	//
//	// The received new desired shape position is in logical units and referred to page
//	// origin. The returned new allowed shape position must also be in in logical units
//	// and referred to page origin.
//
//	return uPos;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeGlyph::OnEndDrag(lmPaper* pPaper, lmInteractor* pCanvas, const UPoint& uPos)
//{
//	// End drag. Receives the command processor associated to the view and the
//	// final position of the object (logical units referred to page origin).
//	// This method must validate/adjust final position and, if ok, it must
//	// send a move object command to the Interactor.
//
//	//send a move object command to the Interactor
//	pCanvas->MoveObject(this, uPos);
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeGlyph::GetObjectOrigin()
//{
//	//returns the origin of this shape
//	return m_uBoundsTop;    //m_uGlyphPos;
//}
//
////---------------------------------------------------------------------------------------
//wxBitmap* GmoShapeGlyph::GetBitmapFromShape(double rScale, Color colorF, Color colorB)
//{
//    //Returns a bitmap with the glyph. The bitmap is only the bounding box.
//    //Ownership of bitmap is transferred to caller method. It must delete it.
//
//    return GetBitmapForGlyph(rScale, m_nGlyph, GetPointSize(), colorF, colorB);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeGlyph::RenderHighlighted(wxDC* pDC, Color colorC)
//{
//    //The DC is scaled and its origin is positioned according current scrolling and
//    //page origin in the view
//
//    //get the bitmap
//    double rScaleX, rScaleY;
//    pDC->GetUserScale(&rScaleX, &rScaleY);
//    wxBitmap* pBitmap = GetBitmapFromShape(rScaleX, colorC, *wxBLACK);
//
//    //blend it with current displayed page
//    lmPixels vxDest = pDC->LogicalToDeviceX(m_uBoundsTop.x),
//             vyDest = pDC->LogicalToDeviceY(m_uBoundsTop.y);
//    const wxBitmap& bitmap = *pBitmap;
//
//    ////DBG -----------
//    //wxString sFileName = _T("draw.bmp");
//    //pBitmap->SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    pDC->SetUserScale(1.0, 1.0);
//    pDC->SetMapMode(wxMM_TEXT);
//	pDC->SetDeviceOrigin(0, 0);
//    //pDC->SetLogicalFunction(wxINVERT);
//    //pDC->DrawBitmap(bitmap, vxDest, vyDest, true);     //true -> transparent
//
//    wxMemoryDC dc;
//    dc.SetMapMode(wxMM_TEXT);
//    dc.SelectObject(*pBitmap);
//    pDC->Blit(vxDest, vyDest, pBitmap->GetWidth(), pBitmap->GetHeight(), &dc, 0, 0, wxXOR, true);
//
//
//    //TODO: During playback the bitmap for the black notehead is constantly used. It could save
//    //a lot of processing time if this particular bitmap is cached.
//
//    delete pBitmap;
//}



//---------------------------------------------------------------------------------------
// GmoShapeClef
//---------------------------------------------------------------------------------------
GmoShapeClef::GmoShapeClef(int idx, int nGlyph, UPoint pos, bool fSmallClef,
                           Color color, LibraryScope& libraryScope)
    : GmoShapeGlyph(GmoObj::k_shape_clef, idx, nGlyph, pos, color,
                    libraryScope)
//    , m_fSmallClef(fSmallClef)
{
//    //In ShapeGlyph constructor, bounds have been set without taking into accont
//    //that this is a clef and could be small size. So bounds have been computed for
//    //normal size. If this clef is small size, recompute bounds
//    if (m_fSmallClef)
//    {
//        // store boundling rectangle position and size
//        lmComponentObj* pSO = ((lmComponentObj*)m_pOwner);
//        wxString sGlyph( glyphs_lmbasic2[m_nGlyph].GlyphChar );
//        double rPointSize = GetPointSize();
//        pPaper->FtSetFontSize(rPointSize);
//        lmURect bbox = ((lmAggDrawer*)(pPaper->GetDrawer()))->FtGetGlyphBounds( (unsigned int)sGlyph.GetChar(0) );
//
//	    m_uBoundsTop.x = m_uGlyphPos.x + bbox.x;
//	    m_uBoundsTop.y = m_uGlyphPos.y + bbox.y + pSO->TenthsToLogical(60);
//	    m_uBoundsBottom.x = m_uBoundsTop.x + bbox.width;
//	    m_uBoundsBottom.y = m_uBoundsTop.y + bbox.height;
//
//        // store selection rectangle position and size
//	    m_uSelRect = GetBounds();
//    }
}

////---------------------------------------------------------------------------------------
//double GmoShapeClef::GetPointSize()
//{
//    lmStaffObj* pSO = ((lmStaffObj*)m_pOwner);
//    lmStaff* pStaff = pSO->GetVStaff()->GetStaff(pSO->GetStaffNum());
//    return (m_fSmallClef ? pStaff->GetMusicFontSize() * 0.8 : pStaff->GetMusicFontSize());
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeClef::OnDrag(lmPaper* pPaper, const UPoint& uPos)
//{
//	// The view informs that the user continues dragging. We receive the new desired
//	// shape position and we must return the new allowed shape position.
//	//
//	// The default behaviour is to return the received position, so the view redraws
//	// the drag image at that position. No action must be performed by the shape on
//	// the score and score objects.
//	//
//	// The received new desired shape position is in logical units and referred to page
//	// origin. The returned new allowed shape position must also be in in logical units
//	// and referred to page origin.
//
//	if (g_fFreeMove) return uPos;
//
//    // A clef only can be moved horizonatlly
//    return UPoint(uPos.x, GetYTop());
//
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeClef::OnEndDrag(lmPaper* pPaper, lmInteractor* pCanvas, const UPoint& uPos)
//{
//	// End drag. Receives the command processor associated to the view and the
//	// final position of the object (logical units referred to page origin).
//	// This method must validate/adjust final position and, if ok, it must
//	// send a move object command to the Interactor.
//
//	UPoint uFinalPos(uPos.x, uPos.y);
//	if (!g_fFreeMove)
//	{
//		//free movement not allowed. Only x position can be changed
//		uFinalPos.y = GetYTop();
//	}
//
//	//send a move object command to the Interactor
//	pCanvas->MoveObject(this, uFinalPos);
//
//}



//---------------------------------------------------------------------------------------
// GmoShapeSimpleLine implementation
//---------------------------------------------------------------------------------------
GmoShapeSimpleLine::GmoShapeSimpleLine(int type,
                                       LUnits xStart, LUnits yStart, LUnits xEnd,
                                       LUnits yEnd, LUnits uWidth,
                                       LUnits uBoundsExtraWidth, Color color,
                                       ELineEdge nEdge)
    : GmoSimpleShape(type, 0, color)
{
    set_new_values(xStart, yStart, xEnd, yEnd, uWidth, uBoundsExtraWidth, color, nEdge);
}

//---------------------------------------------------------------------------------------
void GmoShapeSimpleLine::set_new_values(LUnits xStart, LUnits yStart,
                                        LUnits xEnd, LUnits yEnd,
                                        LUnits uWidth, LUnits uBoundsExtraWidth,
                                        Color color, ELineEdge nEdge)
{
    m_xStart = xStart;
    m_yStart = yStart;
    m_xEnd = xEnd;
    m_yEnd = yEnd;
    m_uWidth = uWidth;
	m_uBoundsExtraWidth = uBoundsExtraWidth;
	m_nEdge = nEdge;

    m_origin.x = xStart;
    m_origin.y = yStart;
    m_size.width = xEnd - xStart + uBoundsExtraWidth;
    m_size.height = yEnd - yStart + uBoundsExtraWidth;

///*
//	//TODO
//    // if line is neither vertical nor horizontal, should we use a strait rectangle or a
//    // leaned rectangle sorrounding the line?
//
//    //width of rectangle = width of line + 2 pixels
//    uWidth += 2.0 / g_r;
//
//    //line angle
//    double alpha = atan((yEnd - yStart) / (xEnd - xStart));
//
//    //boundling rectangle
//    {
//    LUnits uIncrX = (LUnits)( (uWidth * sin(alpha)) / 2.0 );
//    LUnits uIncrY = (LUnits)( (uWidth * cos(alpha)) / 2.0 );
//    UPoint uPoints[] = {
//        UPoint(xStart+uIncrX, yStart-uIncrY),
//        UPoint(xStart-uIncrX, yStart+uIncrY),
//        UPoint(xEnd-uIncrX, yEnd+uIncrY),
//        UPoint(xEnd+uIncrX, yEnd-uIncrY)
//    };
//    SolidPolygon(4, uPoints, color);
//*/
//
//	//For now assume the line is either vertical or horizontal
//	//TODO
//    // store boundling rectangle position and size
//	LUnits uWidthRect = (m_uWidth + uBoundsExtraWidth) / 2.0f;
//	if (xStart == xEnd)
//	{
//		//vertical line
//		m_uBoundsTop.x = xStart - uWidthRect;
//		m_uBoundsTop.y = yStart;
//		m_uBoundsBottom.x = xEnd + uWidthRect;
//		m_uBoundsBottom.y = yEnd;
//	}
//	else
//	{
//		//Horizontal line
//		m_uBoundsTop.x = xStart;
//		m_uBoundsTop.y = yStart - uWidthRect;
//		m_uBoundsBottom.x = xEnd;
//		m_uBoundsBottom.y = yEnd + uWidthRect;
//	}
//
//	NormaliceBoundsRectangle();
//
//    // store selection rectangle position and size
//	m_uSelRect = GetBounds();

}

//---------------------------------------------------------------------------------------
void GmoShapeSimpleLine::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_uWidth);
    pDrawer->move_to(m_xStart, m_yStart);
    pDrawer->line_to(m_xEnd, m_yEnd);
    pDrawer->end_path();
    pDrawer->render(true);

    GmoSimpleShape::on_draw(pDrawer, opt);
}

////---------------------------------------------------------------------------------------
//wxString GmoShapeSimpleLine::Dump(int nIndent)
//{
//	wxString sDump = _T("");
//	sDump.append(nIndent * lmINDENT_STEP, _T(' '));
//	sDump += wxString::Format(_T("Idx: %d %s: start=(%.2f, %.2f), end=(%.2f, %.2f), line width=%.2f, "),
//                m_nOwnerIdx, m_sGMOName.c_str(), m_xStart, m_yStart, m_xEnd, m_yEnd, m_uWidth );
//    sDump += DumpBounds();
//    sDump += _T("\n");
//
//    //base class
//    sDump += GmoShape::Dump(nIndent);
//
//	return sDump;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeSimpleLine::Shift(LUnits xIncr, LUnits yIncr)
//{
//    m_xStart += xIncr;
//    m_yStart += yIncr;
//    m_xEnd += xIncr;
//    m_yEnd += yIncr;
//
//    ShiftBoundsAndSelRec(xIncr, yIncr);
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((lmCompositeShape*)GetParentShape())->RecomputeBounds();
//}
//
//
//
////---------------------------------------------------------------------------------------
//// GmoShapeInvisible
////---------------------------------------------------------------------------------------
//GmoShapeInvisible::GmoShapeInvisible(GmoBox* owner, int nShapeIdx, UPoint uPos, lmUSize uSize,
//                                   wxString sName)
//	: GmoSimpleShape(GmoObj::k_shape_Invisible, pOwner, nShapeIdx, sName, lmNO_DRAGGABLE, lmNO_SELECTABLE)
//{
//    m_uBoundsTop.x = uPos.x;
//    m_uBoundsTop.y = uPos.y;
//    m_uBoundsBottom.x = uPos.x + uSize.x;
//    m_uBoundsBottom.y = uPos.y + uSize.y;
//}
//
////---------------------------------------------------------------------------------------
//wxString GmoShapeInvisible::Dump(int nIndent)
//{
//	//TODO
//    wxString sDump = _T("GmoShapeInvisible\n");
//
//    //base class
//    sDump += GmoShape::Dump(nIndent);
//
//	return sDump;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeInvisible::Render(lmPaper* pPaper, Color color)
//{
//    //if (g_fDrawInvisible)       //TODO
//    {
//    }
//}
//
//
////---------------------------------------------------------------------------------------
//// GmoShapeRectangle: a rectangle with optional rounded corners
////---------------------------------------------------------------------------------------
////TODO: remove this backwards compatibility constructor
//GmoShapeRectangle::GmoShapeRectangle(GmoBox* owner, LUnits uxLeft, LUnits uyTop,
//                                   LUnits uxRight, LUnits uyBottom, LUnits uWidth,
//                                   Color color, wxString sName,
//				                   bool fDraggable, bool fSelectable,
//                                   bool fVisible)
//	: GmoSimpleShape(GmoObj::k_shape_Rectangle, pOwner, 0, sName, fDraggable, fSelectable,
//                    fVisible)
//{
//    Create(uxLeft, uyTop, uxRight, uyBottom, uWidth, color, *wxWHITE);
//}
//
////---------------------------------------------------------------------------------------
//GmoShapeRectangle::GmoShapeRectangle(GmoBox* owner,
//                     //position and size
//                     LUnits uxLeft, LUnits uyTop, LUnits uxRight, LUnits uyBottom,
//                     //border
//                     LUnits uBorderWidth, Color nBorderColor,
//                     //content
//                     Color nBgColor,
//                     //other
//                     int nShapeIdx, wxString sName,
//				     bool fDraggable, bool fSelectable, bool fVisible)
//	: GmoSimpleShape(GmoObj::k_shape_Rectangle, pOwner, nShapeIdx, sName, fDraggable,
//                    fSelectable, fVisible)
//{
//    Create(uxLeft, uyTop, uxRight, uyBottom, uBorderWidth, nBorderColor, nBgColor);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::Create(LUnits uxLeft, LUnits uyTop, LUnits uxRight,
//                              LUnits uyBottom, LUnits uBorderWidth,
//                              Color nBorderColor, Color nBgColor)
//{
//    m_uCornerRadius = 0.0f;
//    m_uBorderWidth = uBorderWidth;
//    m_nBorderColor = nBorderColor;
//    m_nBorderStyle = lm_eLine_None;
//    m_nBgColor = nBgColor;
//
//    //store rectangle points and compute centers of sides
//    m_uPoint[lmID_TOP_LEFT].x = uxLeft;
//    m_uPoint[lmID_TOP_LEFT].y = uyTop;
//    m_uPoint[lmID_TOP_RIGHT].x = uxRight;
//    m_uPoint[lmID_TOP_RIGHT].y = uyTop;
//    m_uPoint[lmID_BOTTOM_RIGHT].x = uxRight;
//    m_uPoint[lmID_BOTTOM_RIGHT].y = uyBottom;
//    m_uPoint[lmID_BOTTOM_LEFT].x = uxLeft;
//    m_uPoint[lmID_BOTTOM_LEFT].y = uyBottom;
//    ComputeCenterPoints();
//
//    //Create the handlers
//    m_pHandler[lmID_TOP_LEFT] = new lmHandlerSquare(m_pOwner, this, lmID_TOP_LEFT, wxCURSOR_SIZENWSE);
//    m_pHandler[lmID_TOP_RIGHT] = new lmHandlerSquare(m_pOwner, this, lmID_TOP_RIGHT, wxCURSOR_SIZENESW);
//    m_pHandler[lmID_BOTTOM_RIGHT] = new lmHandlerSquare(m_pOwner, this, lmID_BOTTOM_RIGHT, wxCURSOR_SIZENWSE);
//    m_pHandler[lmID_BOTTOM_LEFT] = new lmHandlerSquare(m_pOwner, this, lmID_BOTTOM_LEFT, wxCURSOR_SIZENESW);
//    m_pHandler[lmID_LEFT_CENTER] = new lmHandlerSquare(m_pOwner, this, lmID_LEFT_CENTER, wxCURSOR_SIZEWE);
//    m_pHandler[lmID_TOP_CENTER] = new lmHandlerSquare(m_pOwner, this, lmID_TOP_CENTER, wxCURSOR_SIZENS);
//    m_pHandler[lmID_RIGHT_CENTER] = new lmHandlerSquare(m_pOwner, this, lmID_RIGHT_CENTER, wxCURSOR_SIZEWE);
//    m_pHandler[lmID_BOTTOM_CENTER] = new lmHandlerSquare(m_pOwner, this, lmID_BOTTOM_CENTER, wxCURSOR_SIZENS);
//
//    UpdateBounds();
//}
//
////---------------------------------------------------------------------------------------
//GmoShapeRectangle::~GmoShapeRectangle()
//{
//    //delete handlers
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//    {
//        if (m_pHandler[i])
//            delete m_pHandler[i];
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::UpdateBounds()
//{
//    // store boundling rectangle position and size
//    LUnits uWidthRect = m_uBorderWidth / 2.0;
//
//    m_uBoundsTop = m_uPoint[lmID_TOP_LEFT];
//    m_uBoundsBottom = m_uPoint[lmID_BOTTOM_RIGHT];
//
//    NormaliceBoundsRectangle();
//
//    // store selection rectangle position and size
//    m_uSelRect = GetBounds();
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::SavePoints()
//{
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//    {
//        //save points and update handlers position
//        m_uSavePoint[i] = m_uPoint[i];
//        m_pHandler[i]->SetHandlerCenterPoint(m_uPoint[i].x, m_uPoint[i].y);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::Render(lmPaper* pPaper, Color color)
//{
//    //if selected, book to be rendered with handlers when posible
//    if (IsSelected())
//    {
//        //book to be rendered with handlers
//        GetOwnerBoxPage()->OnNeedToDrawHandlers(this);
//        SavePoints();
//    }
//    else
//    {
//        //draw the rectangle
//        DrawRectangle(pPaper, color, false);        //false -> anti-aliased
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::RenderNormal(lmPaper* pPaper, Color color)
//{
//        //draw the rectangle
//        DrawRectangle(pPaper, color, false);        //false -> anti-aliased
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::RenderWithHandlers(lmPaper* pPaper)
//{
//    //render the textbox and its handlers
//
//    //as painting uses XOR we need the complementary color
//    Color color = *wxBLUE;      //TODO User options
//    Color colorC = Color(255 - (int)color.Red(),
//                               255 - (int)color.Green(),
//                               255 - (int)color.Blue() );
//
//    //prepare to render
//    pPaper->SetLogicalFunction(wxXOR);
//
//    //draw the handlers
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//    {
//        m_pHandler[i]->Render(pPaper, colorC);
//        GetOwnerBoxPage()->AddActiveHandler(m_pHandler[i]);
//    }
//
//    //draw the rectangle
//    DrawRectangle(pPaper, colorC, true);        //true -> Sketch
//
//    //terminate renderization
//    pPaper->SetLogicalFunction(wxCOPY);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::DrawRectangle(lmPaper* pPaper, Color color, bool fSketch)
//{
//    //draw backgroung only if not selected
//    if (!fSketch)
//        pPaper->SolidPolygon(4, m_uPoint, m_nBgColor);
//
//    //draw borders
//    ELineEdges nEdge = lm_eEdgeNormal;
//    //m_nBorderStyle = lm_eLine_None;
//    pPaper->SolidLine(m_uPoint[lmID_TOP_LEFT].x, m_uPoint[lmID_TOP_LEFT].y,
//                      m_uPoint[lmID_TOP_RIGHT].x, m_uPoint[lmID_TOP_RIGHT].y,
//                      m_uBorderWidth, nEdge, m_nBorderColor);
//    pPaper->SolidLine(m_uPoint[lmID_TOP_RIGHT].x, m_uPoint[lmID_TOP_RIGHT].y,
//                      m_uPoint[lmID_BOTTOM_RIGHT].x, m_uPoint[lmID_BOTTOM_RIGHT].y,
//                      m_uBorderWidth, nEdge, m_nBorderColor);
//    pPaper->SolidLine(m_uPoint[lmID_BOTTOM_RIGHT].x, m_uPoint[lmID_BOTTOM_RIGHT].y,
//                      m_uPoint[lmID_BOTTOM_LEFT].x, m_uPoint[lmID_BOTTOM_LEFT].y,
//                      m_uBorderWidth, nEdge, m_nBorderColor);
//    pPaper->SolidLine(m_uPoint[lmID_BOTTOM_LEFT].x, m_uPoint[lmID_BOTTOM_LEFT].y,
//                      m_uPoint[lmID_TOP_LEFT].x, m_uPoint[lmID_TOP_LEFT].y,
//                      m_uBorderWidth, nEdge, m_nBorderColor);
//
//    GmoSimpleShape::Render(pPaper, color);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::SetCornerRadius(LUnits uRadius)
//{
//    m_uCornerRadius = uRadius;
//}
//
////---------------------------------------------------------------------------------------
//wxString GmoShapeRectangle::Dump(int nIndent)
//{
//	wxString sDump = _T("");
//	sDump.append(nIndent * lmINDENT_STEP, _T(' '));
//	sDump += wxString::Format(_T("Idx: %d %s: left-top=(%.2f, %.2f), right-bottom=(%.2f, %.2f), line width=%.2f, "),
//                m_nOwnerIdx, m_sGMOName.c_str(),
//                m_uPoint[lmID_TOP_LEFT].x, m_uPoint[lmID_TOP_LEFT].y,
//                m_uPoint[lmID_BOTTOM_RIGHT].x, m_uPoint[lmID_BOTTOM_RIGHT].y,
//                m_uBorderWidth );
//    sDump += DumpBounds();
//    sDump += _T("\n");
//
//    //base class
//    sDump += GmoShape::Dump(nIndent);
//
//	return sDump;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::Shift(LUnits uxIncr, LUnits uyIncr)
//{
//    m_uPoint[lmID_TOP_LEFT].x += uxIncr;
//    m_uPoint[lmID_TOP_LEFT].y += uyIncr;
//    m_uPoint[lmID_TOP_RIGHT].x += uxIncr;
//    m_uPoint[lmID_TOP_RIGHT].y += uyIncr;
//    m_uPoint[lmID_BOTTOM_RIGHT].x += uxIncr;
//    m_uPoint[lmID_BOTTOM_RIGHT].y += uyIncr;
//    m_uPoint[lmID_BOTTOM_LEFT].x += uxIncr;
//    m_uPoint[lmID_BOTTOM_LEFT].y += uyIncr;
//
//    ComputeCenterPoints();
//
//    ShiftBoundsAndSelRec(uxIncr, uyIncr);
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((lmCompositeShape*)GetParentShape())->RecomputeBounds();
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::ComputeCenterPoints()
//{
//    m_uPoint[lmID_TOP_CENTER].x = (m_uPoint[lmID_TOP_LEFT].x + m_uPoint[lmID_TOP_RIGHT].x) / 2.0f;
//    m_uPoint[lmID_TOP_CENTER].y = m_uPoint[lmID_TOP_LEFT].y;
//
//    m_uPoint[lmID_RIGHT_CENTER].x = m_uPoint[lmID_TOP_RIGHT].x;
//    m_uPoint[lmID_RIGHT_CENTER].y = (m_uPoint[lmID_TOP_RIGHT].y + m_uPoint[lmID_BOTTOM_RIGHT].y) / 2.0f;
//
//    m_uPoint[lmID_BOTTOM_CENTER].x = m_uPoint[lmID_TOP_CENTER].x;
//    m_uPoint[lmID_BOTTOM_CENTER].y = m_uPoint[lmID_BOTTOM_RIGHT].y;
//
//    m_uPoint[lmID_LEFT_CENTER].x = m_uPoint[lmID_TOP_LEFT].x;
//    m_uPoint[lmID_LEFT_CENTER].y = m_uPoint[lmID_RIGHT_CENTER].y;
//}
//
////---------------------------------------------------------------------------------------
//wxBitmap* GmoShapeRectangle::OnBeginDrag(double rScale, wxDC* pDC)
//{
//	// A dragging operation is started. The view invokes this method to request the
//	// bitmap to be used as drag image. No other action is required.
//	// If no bitmap is returned drag is cancelled.
//	//
//	// So this method returns the bitmap to use with the drag image.
//
//    //as this is a shape defined by points: save all points position
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//        m_uSavePoint[i] = m_uPoint[i];
//
//    // allocate a bitmap whose size is that of the box area
//    // convert size to pixels
//    int wD = (int)pDC->LogicalToDeviceXRel( m_uBoundsBottom.x - m_uBoundsTop.x );
//    int hD = (int)pDC->LogicalToDeviceYRel( m_uBoundsBottom.y - m_uBoundsTop.y );
//    wxBitmap bitmap(wD+2, hD+2);
//
//    // allocate a memory DC for drawing into a bitmap
//    wxMemoryDC dc2;
//    dc2.SelectObject(bitmap);
//    dc2.SetMapMode(lmDC_MODE);
//    dc2.SetUserScale(rScale, rScale);
//    //dc2.SetFont(*m_pFont);
//
//    // draw onto the bitmap
//    dc2.SetBackground(* wxWHITE_BRUSH);
//    dc2.Clear();
//    dc2.SetBackgroundMode(wxTRANSPARENT);
//    dc2.SetPen(*wxBLACK_PEN);
//    dc2.DrawRectangle(m_uBoundsTop.x, m_uBoundsTop.y, GetBounds().GetWidth(),
//                      GetBounds().GetHeight() );
//    //dc2.SetTextForeground(g_pColors->ScoreSelected());
//    //dc2.DrawText(m_sClippedText, m_uTextPos.x - m_uBoundsTop.x, m_uTextPos.y - m_uBoundsTop.y);
//    dc2.SelectObject(wxNullBitmap);
//
//    // Make the bitmap masked
//    wxImage image = bitmap.ConvertToImage();
//    image.SetMaskColour(255, 255, 255);
//    wxBitmap* pBitmap = new wxBitmap(image);
//
//    ////DBG -----------
//    //wxString sFileName = _T("GmoShapeTextbox.bmp");
//    //pBitmap->SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    return pBitmap;
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeRectangle::OnDrag(lmPaper* pPaper, const UPoint& uPos)
//{
//	// The view informs that the user continues dragging. We receive the new desired
//	// shape position and we must return the new allowed shape position.
//	//
//	// The default behaviour is to return the received position, so the view redraws
//	// the drag image at that position. No action must be performed by the shape on
//	// the score and score objects.
//	//
//	// The received new desired shape position is in logical units and referred to page
//	// origin. The returned new allowed shape position must also be in in logical units
//	// and referred to page origin.
//
//    //this is a shape defined by points. Therefore it is necessary to
//    //update all handler points and object points
//    UPoint uShift(uPos - this->GetBounds().GetTopLeft());
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//    {
//        m_pHandler[i]->SetHandlerTopLeftPoint( uShift + m_pHandler[i]->GetBounds().GetLeftTop() );
//        m_uPoint[i] = m_pHandler[i]->GetHandlerCenterPoint();
//    }
//    UpdateBounds();
//
//    return UPoint(uPos.x, uPos.y);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::OnEndDrag(lmPaper* pPaper, lmInteractor* pCanvas, const UPoint& uPos)
//{
//	// End drag. Receives the command processor associated to the view and the
//	// final position of the object (logical units referred to page origin).
//	// This method must validate/adjust final position and, if ok, it must
//	// send a move object command to the Interactor.
//
//    //compute shift from start of drag point
//    UPoint uShift = uPos - m_uSavePoint[0];
//
//    //restore shape position to that of start of drag start so that MoveObject() or
//    //MoveObjectPoints() commands can apply shifts from original points.
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//        m_uPoint[i] = m_uSavePoint[i];
//    UpdateBounds();
//
//    //as this is an object defined by points, instead of MoveObject() command we have to issue
//    //a MoveObjectPoints() command.
//    UPoint uShifts[lmID_NUM_HANDLERS];
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//        uShifts[i] = uShift;
//    pCanvas->MoveObjectPoints(this, uShifts, 4, false);  //false-> do not update views
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeRectangle::OnHandlerDrag(lmPaper* pPaper, const UPoint& uPos,
//                                    long nHandlerID)
//{
//	// The view informs that the user continues dragging. We receive the new desired
//	// shape position and we must return the new allowed shape position.
//	//
//	// The default behaviour is to return the received position, so the view redraws
//	// the drag image at that position. No action must be performed by the shape on
//	// the score and score objects.
//	//
//	// The received new desired shape position is in logical units and referred to page
//	// origin. The returned new allowed shape position must also be in in logical units
//	// and referred to page origin.
//
//    //erase previous draw
//    RenderWithHandlers(pPaper);
//
//    //compute new rectangle and handlers positions
//    ComputeNewPointsAndHandlersPositions(uPos, nHandlerID);
//
//    //draw at new position
//    RenderWithHandlers(pPaper);
//
//    return uPos;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::ComputeNewPointsAndHandlersPositions(const UPoint& uPos,
//                                                            long nHandlerID)
//{
//    //Common code to OnHandlerDrag and OnHandlerEndDrag to compute new coordinares
//    //for rectangle and handlers points
//
//    //aux. to compute new rectangle coordinates
//    LUnits xLeft = m_uPoint[lmID_TOP_LEFT].x;
//    LUnits yTop = m_uPoint[lmID_TOP_LEFT].y;
//    LUnits xRight = m_uPoint[lmID_BOTTOM_RIGHT].x;
//    LUnits yBottom = m_uPoint[lmID_BOTTOM_RIGHT].y;
//
//    //maintain coherence in points, so that the shape continues being a rectangle
//    UPoint point;
//    switch(nHandlerID)
//    {
//        case lmID_TOP_LEFT:
//            //free movement
//            m_pHandler[lmID_TOP_LEFT]->SetHandlerTopLeftPoint(uPos);
//            point = m_pHandler[lmID_TOP_LEFT]->GetHandlerCenterPoint();
//            xLeft = point.x;
//            yTop = point.y;
//            break;
//
//        case lmID_TOP_RIGHT:
//            //free movement
//            m_pHandler[lmID_TOP_RIGHT]->SetHandlerTopLeftPoint(uPos);
//            point = m_pHandler[lmID_TOP_RIGHT]->GetHandlerCenterPoint();
//            xRight = point.x;
//            yTop = point.y;
//            break;
//
//        case lmID_BOTTOM_RIGHT:
//            //free movement
//            m_pHandler[lmID_BOTTOM_RIGHT]->SetHandlerTopLeftPoint(uPos);
//            point = m_pHandler[lmID_BOTTOM_RIGHT]->GetHandlerCenterPoint();
//            xRight = point.x;
//            yBottom = point.y;
//            break;
//
//        case lmID_BOTTOM_LEFT:
//            //free movement
//            m_pHandler[lmID_BOTTOM_LEFT]->SetHandlerTopLeftPoint(uPos);
//            point = m_pHandler[lmID_BOTTOM_LEFT]->GetHandlerCenterPoint();
//            xLeft = point.x;
//            yBottom = point.y;
//            break;
//
//        case lmID_LEFT_CENTER:
//            //clip horizontally
//            m_pHandler[lmID_LEFT_CENTER]->SetHandlerTopLeftPoint(uPos);
//            xLeft = m_pHandler[lmID_LEFT_CENTER]->GetHandlerCenterPoint().x;
//            break;
//
//        case lmID_TOP_CENTER:
//            //clip vertically
//            m_pHandler[lmID_TOP_CENTER]->SetHandlerTopLeftPoint(uPos);
//            yTop = m_pHandler[lmID_TOP_CENTER]->GetHandlerCenterPoint().y;
//            break;
//
//        case lmID_RIGHT_CENTER:
//            //clip horizontally
//            m_pHandler[lmID_RIGHT_CENTER]->SetHandlerTopLeftPoint(uPos);
//            xRight = m_pHandler[lmID_RIGHT_CENTER]->GetHandlerCenterPoint().x;
//            break;
//
//        case lmID_BOTTOM_CENTER:
//            //clip vertically
//            m_pHandler[lmID_BOTTOM_CENTER]->SetHandlerTopLeftPoint(uPos);
//            yBottom = m_pHandler[lmID_BOTTOM_CENTER]->GetHandlerCenterPoint().y;
//            break;
//
//        default:
//            wxASSERT(false);
//    }
//
//    //store rectangle points and compute centers of sides
//    m_uPoint[lmID_TOP_LEFT].x = xLeft;
//    m_uPoint[lmID_TOP_LEFT].y = yTop;
//    m_uPoint[lmID_TOP_RIGHT].x = xRight;
//    m_uPoint[lmID_TOP_RIGHT].y = yTop;
//    m_uPoint[lmID_BOTTOM_RIGHT].x = xRight;
//    m_uPoint[lmID_BOTTOM_RIGHT].y = yBottom;
//    m_uPoint[lmID_BOTTOM_LEFT].x = xLeft;
//    m_uPoint[lmID_BOTTOM_LEFT].y = yBottom;
//    ComputeCenterPoints();
//
//    //set handlers
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//    {
//        m_pHandler[i]->SetHandlerCenterPoint(m_uPoint[i].x, m_uPoint[i].y);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::OnHandlerEndDrag(lmInteractor* pCanvas, const UPoint& uPos,
//                                   long nHandlerID)
//{
//	// End drag. Receives the command processor associated to the view and the
//	// final position of the object (logical units referred to page origin).
//	// This method must validate/adjust final position and, if ok, it must
//	// send a move object command to the Interactor.
//
//    //compute new rectangle and handlers positions
//    ComputeNewPointsAndHandlersPositions(uPos, nHandlerID);
//
//    //Compute shifts from start of drag points
//    UPoint uShifts[lmID_NUM_HANDLERS];
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//    {
//        uShifts[i] = m_uPoint[i] - m_uSavePoint[i];
//    }
//
//    //MoveObjectPoints() apply shifts computed from drag start points. As handlers and
//    //shape points are already displaced, it is necesary to restore the original positions to
//    //avoid double displacements.
//    for (int i=0; i < lmID_NUM_HANDLERS; i++)
//        m_uPoint[i] = m_uSavePoint[i];
//
//    UpdateBounds();
//
//    pCanvas->MoveObjectPoints(this, uShifts, 4, false);  //false-> do not update views
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeRectangle::MovePoints(int nNumPoints, int nShapeIdx, UPoint* pShifts,
//                                  bool fAddShifts)
//{
//    //Each time a commnad is issued to change the rectangle, we will receive a call
//    //back to update the shape
//
//    for (int i=0; i < nNumPoints; i++)
//    {
//        if (fAddShifts)
//        {
//            m_uPoint[i].x += (*(pShifts+i)).x;
//            m_uPoint[i].y += (*(pShifts+i)).y;
//        }
//        else
//        {
//            m_uPoint[i].x -= (*(pShifts+i)).x;
//            m_uPoint[i].y -= (*(pShifts+i)).y;
//        }
//
//        m_pHandler[i]->SetHandlerCenterPoint(m_uPoint[i]);
//    }
//    ComputeCenterPoints();
//    UpdateBounds();
//}



//---------------------------------------------------------------------------------------
// GmoShapeStem implementation: a vertical line
//---------------------------------------------------------------------------------------
GmoShapeStem::GmoShapeStem(LUnits xPos, LUnits yStart,
                           LUnits uExtraLength, LUnits yEnd, bool fStemDown,
                           LUnits uWidth, Color color)
	: GmoShapeSimpleLine(GmoObj::k_shape_stem, xPos, yStart, xPos, yEnd,
                         uWidth, 0.0f, color, k_edge_horizontal)
	, m_fStemDown(fStemDown)
    , m_uExtraLength(uExtraLength)
{
}

////---------------------------------------------------------------------------------------
//void GmoShapeStem::SetLength(LUnits uLenght, bool fModifyTop)
//{
//	if (fModifyTop)
//	{
//		if (m_yStart < m_yEnd)
//			m_yStart = m_yEnd - uLenght;
//		else
//			m_yEnd = m_yStart - uLenght;
//	}
//	else
//	{
//		if (m_yStart < m_yEnd)
//			m_yEnd = m_yStart + uLenght;
//		else
//			m_yStart = m_yEnd + uLenght;
//	}
//
//	//re-create the shape
//	Create(m_xStart, m_yStart, m_xEnd, m_yEnd, m_uWidth, m_uBoundsExtraWidth,
//		   m_color, m_nEdge);
//
//}

//---------------------------------------------------------------------------------------
void GmoShapeStem::adjust(LUnits xPos, LUnits yStart, LUnits yEnd, bool fStemDown)
{
	m_fStemDown = fStemDown;
	set_new_values(xPos, yStart, xPos, yEnd, m_uWidth, m_uBoundsExtraWidth,
                   m_color, m_nEdge);
}

////---------------------------------------------------------------------------------------
//LUnits GmoShapeStem::GetYStartStem()
//{
//	//Start of stem is the nearest position to the notehead
//
//	return (m_fStemDown ? GetYTop() : GetYBottom());
//}
//
////---------------------------------------------------------------------------------------
//LUnits GmoShapeStem::GetYEndStem()
//{
//	//End of stem is the farthest position from the notehead
//
//	return (m_fStemDown ? GetYBottom() : GetYTop());
//}
//
////---------------------------------------------------------------------------------------
//LUnits GmoShapeStem::GetXCenterStem()
//{
//	//returns the stem x position. This position is in the middle of the line width
//	return m_xStart;
//}
//
//
//
////---------------------------------------------------------------------------------------
//// GmoShapeFiguredBass object implementation: a composite shape that can have
//// attached shapes.
////---------------------------------------------------------------------------------------
//void GmoShapeFiguredBass::Shift(LUnits uxIncr, LUnits uyIncr)
//{
//	lmCompositeShape::Shift(uxIncr, uyIncr);
//	InformAttachedShapes(uxIncr, uyIncr, lmSHIFT_EVENT);
//}
//
//
//
////---------------------------------------------------------------------------------------
//// GmoShapeWindow object implementation: an auxiliary shape to embbed any wxWindow
////  (Button, TextCtrol, etc.) on the score
////---------------------------------------------------------------------------------------
//GmoShapeWindow::GmoShapeWindow(GmoBox* owner, int nShapeIdx,
//                  //position and size
//                  LUnits uxLeft, LUnits uyTop, LUnits uxRight, LUnits uyBottom,
//                  //border
//                  LUnits uBorderWidth, Color nBorderColor,
//                  //content
//                  Color nBgColor,
//                  //other
//                  wxString sName,
//				  bool fDraggable, bool fSelectable, bool fVisible)
//    : GmoShapeRectangle(pOwner, uxLeft, uyTop, uxRight, uyBottom, uBorderWidth,
//                       nBorderColor, nBgColor, nShapeIdx, sName,
//                       fDraggable, fSelectable, fVisible)
//    , m_pWidget((wxWindow*)NULL)
//{
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeWindow::Render(lmPaper* pPaper, Color color)
//{
//    lmBoxPage* pBPage = this->GetOwnerBoxPage();
//    wxWindow* pWindow = pBPage->GetRenderWindow();
//    wxPoint& vOffset = pBPage->GetRenderWindowOffset();
//
//    wxPoint pos(pPaper->LogicalToDeviceX(m_uBoundsTop.x) + vOffset.x,
//                pPaper->LogicalToDeviceY(m_uBoundsTop.y) + vOffset.y );
//    wxSize size(pPaper->LogicalToDeviceX(GetBounds().GetWidth()),
//                pPaper->LogicalToDeviceX(GetBounds().GetHeight()) );
//
//    m_pWidget =
//        new wxTextCtrl(pWindow, wxID_ANY,
//                       _T("This is a text using a wxTextCtrl window!"),
//                       pos, size );
//}
//
//
////---------------------------------------------------------------------------------------
////global functions defined in this module
////---------------------------------------------------------------------------------------
//wxBitmap* GetBitmapForGlyph(double rScale, int nGlyph, double rPointSize, Color colorF, Color colorB)
//{
//    //Returns the bitmap for the glyph. The bitmap size is the bounding box.
//    //Ownership of bitmap is transferred to caller. It must delete it.
//
//
//    //allocate an empty drawer for measurements
//    wxBitmap dummyBitmap(1, 1);
//    lmAggDrawer* pDrawer = new lmAggDrawer(&dummyBitmap, rScale);
//
//    // Get size of glyph, in logical units
//    wxString sGlyph( glyphs_lmbasic2[nGlyph].GlyphChar );
//    //wxLogMessage(_T("[Shapes/GetBitmapForGlyph] rPointSize=%.2f, rScale=%.2f, sGlyph='%s'"),
//    //             rPointSize, rScale, sGlyph.c_str());
//    pDrawer->FtSetFontSize(rPointSize);
//    wxRect vBox = pDrawer->FtGetGlyphBoundsInPixels( (unsigned int)sGlyph.GetChar(0) );
//
//    //allocate a bitmap for the glyph
//    wxBitmap bitmap(vBox.width + 1, vBox.height + 1);
//
//     //use this bitmap as rendering buffer
//    delete pDrawer;
//    pDrawer = new lmAggDrawer(&bitmap, rScale);
//    pDrawer->FtSetFontSize(rPointSize);
//
//    //render the glyph
//    pDrawer->SetTextForeground(colorF);
//    pDrawer->SetTextBackground(colorB);
//    pDrawer->Clear();
//    pDrawer->FtSetTextPositionPixels(- vBox.x, - vBox.y);
//    pDrawer->FtDrawChar( (unsigned int)sGlyph.GetChar(0) );
//
//    //get the image buffer and create a bitmap from it
//    wxImage& image = pDrawer->GetImageBuffer();
//
//    // Make the bitmap masked
//    image.SetMaskColour(colorB.Red(), colorB.Green(), colorB.Blue());
//    wxBitmap* pBitmap = new wxBitmap(image);
//    delete pDrawer;
//
//    ////DBG -----------
//    //wxLogMessage(_T("[Shapes: GetBitmapForGlyph] Bitmap saved as BitmapGlyp.bmp"));
//    //wxString sFileName = _T("BitmapGlyp.bmp");
//    //pBitmap->SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    return pBitmap;
//}

}  //namespace lomse
