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

#include "lomse_shape_note.h"

#include "lomse_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// GmoShapeNote implementation
//---------------------------------------------------------------------------------------
GmoShapeNote::GmoShapeNote(LUnits x, LUnits y, Color color, LibraryScope& libraryScope)
    : GmoCompositeShape(GmoObj::k_shape_note, 0, color)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
    , m_pNoteheadShape(NULL)
	, m_pStemShape(NULL)
{
//    m_uxLeft = xLeft;
//    m_uyTop = yTop;
//	m_color = color;
//
//	//initializations
//	m_nNoteHead = -1;		// -1 = no shape
//	m_pBeamShape = (GmoShapeBeam*)NULL;
//	m_pStemShape = (GmoShapeStem*)NULL;
//    m_pTieShape[0] = (GmoShapeTie*)NULL;
//    m_pTieShape[1] = (GmoShapeTie*)NULL;



//    //bounds
//    select_font();
//    TextMeter meter(m_libraryScope);
//    m_size.width = meter.measure_width(text);
//    m_size.height = meter.get_font_height();
//
//    //position
//    m_origin.x = x;
//    m_origin.y = y - m_size.height;     //reference is at text bottom
//
//    //color
//    if (m_pStyle)
//        m_color = m_pStyle->get_color();
}

//---------------------------------------------------------------------------------------
GmoShapeNote::~GmoShapeNote()
{
//    //TODO. If this note is deleted and it has attachements to any other note,
//    //the common attached shapes must also be deleted. This problem was detected
//    //with ties, when the next note is in the next system. [000.00.error6]. But
//    //it will happen with beams [000.00.error7] and possibly with other objects.
//    //BUG_BYPASS. Specific code to deal with ties
// //   lmNote* pNote = (lmNote*)m_pOwner;
// //   if (pNote->IsTiedToPrev())
// //   {
// //       //Delete the tie,
// //       //When this note is re-layouted, the tie will be created again.
//	//    std::list<lmAttachPoint*>::iterator pItem;
//	//    for (pItem = m_cAttachments.begin(); pItem != m_cAttachments.end(); pItem++)
//	//    {
//	//	    if ( (*pItem)->pShape->IsShapeTie() )
// //           {
// //               //get
// //           }
// //       }
//	//if (pItem != m_cAttachments.end())
//	//	m_cAttachments.erase(pItem);
// //       void GmoShape::Detach(GmoShape* pShape)
//
// //   }
//
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
//    //set selection rectangle as the notehead rectangle
//	if (m_nNoteHead >= 0)
//        m_uSelRect = GetNoteHead()->GetBounds();

    GmoCompositeShape::on_draw(pDrawer, opt);

//    LUnits uxPos = GetNoteHead()->GetXLeft();
//	DrawLegerLines(m_nPosOnStaff, uxPos, pPaper, color);
}

void GmoShapeNote::add_stem(GmoShapeStem* pShape)
{
	add(pShape);
	m_pStemShape = pShape;
}

void GmoShapeNote::add_notehead(GmoShapeNotehead* pShape)
{
	add(pShape);
	m_pNoteheadShape = pShape;
}

void GmoShapeNote::add_flag(GmoShapeFlag* pShape)
{
	add(pShape);
}

//void GmoShapeNote::AddAccidental(GmoShape* pShape)
//{
//	Add(pShape);
//}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_note_in_block(GmoShape* pShape)
{
	add(pShape);
}

//void GmoShapeNote::Shift(LUnits uxIncr, LUnits uyIncr)
//{
//	GmoCompositeShape::Shift(uxIncr, uyIncr);
//
//	m_uxLeft += uxIncr;
//    m_uyTop += uyIncr;
//
//	InformAttachedShapes(uxIncr, uyIncr, lmSHIFT_EVENT);
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((GmoCompositeShape*)GetParentShape())->RecomputeBounds();
//}
//
//GmoShape* GmoShapeNote::GetNoteHead()
//{
//	if (m_nNoteHead < 0)
//		return (GmoShape*)NULL;
//
//	return GetShape(m_nNoteHead);
//}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_width()
{
	if (m_pNoteheadShape)
		return m_pNoteheadShape->get_width();

	return 0.0f;
}

//void GmoShapeNote::SetStemLength(LUnits uLength)
//{
//	GmoShapeStem* pStem = GetStem();
//	if (!pStem) return;
//
//
//	if (StemGoesDown())
//	{
//		//adjust bottom point
//		pStem->SetLength(uLength, false);
//	}
//	else
//	{
//		//adjust top point
//		pStem->SetLength(uLength, true);
//	}
//
//	RecomputeBounds();
//
//}
//
//LUnits GmoShapeNote::GetStemThickness()
//{
//	GmoShapeStem* pStem = GetStem();
//	if (!pStem) return 0.0;
//
//	return pStem->GetXRight() - pStem->GetXLeft();
//}
//
//bool GmoShapeNote::StemGoesDown()
//{
//	return ((lmNote*)m_pOwner)->StemGoesDown();
//}
//
//void GmoShapeNote::ApplyUserShiftsToTieShape()
//{
//    //This note is the end note of a tie. And the note has been moved, during layout,
//    //to its final position. Then, this method is invoked to inform the tie, so that
//    //it can to apply user shifts to bezier points
//
//    if (m_pTieShape[0])
//        m_pTieShape[0]->ApplyUserShifts();
//
//    if (m_pTieShape[1])
//        m_pTieShape[1]->ApplyUserShifts();
//}
//
//wxBitmap* GmoShapeNote::OnBeginDrag(double rScale, wxDC* pDC)
//{
//	m_nOldSteps = 0;
//	m_uxOldPos = -100000.0f;		//any absurd value
//	return GmoCompositeShape::OnBeginDrag(rScale, pDC);
//}
//
//UPoint GmoShapeNote::OnDrag(lmPaper* pPaper, const UPoint& uPos)
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
//	if (g_fFreeMove)
//        return uPos;
//
//    // A note only can be moved in discrete vertical steps (staff lines/spaces)
//    //return UPoint(uPos.x, GetYTop());	//only horizontal movement
//    //return UPoint(uPos.x, uPos.y);		//free movement
//    UPoint pos = uPos;
//	int nSteps;
//    pos.y = lmCheckNoteNewPosition(((lmNote*)m_pOwner)->GetStaff(), GetYTop(), uPos.y, &nSteps);
//
//		//draw leger lines
//
//	//as painting uses XOR we need the complementary color
//	Color color = *wxBLUE;
//	Color colorC(255 - (int)color.Red(), 255 - (int)color.Green(), 255 - (int)color.Blue() );
//	pPaper->SetLogicalFunction(wxXOR);
//
//	//wxLogMessage(_T("[GmoShapeNote::OnDrag] OldSteps=%d, oldPos=%.2f, newSteps=%d, newPos=%.2f"),
//	//	m_nOldSteps, m_uxOldPos, nSteps, uPos.x );
//	//remove old ledger lines
//	if (m_uxOldPos != -100000.0f)
//		DrawLegerLines(m_nPosOnStaff + m_nOldSteps, m_uxOldPos, pPaper, colorC);
//
//	//draw new ledger lines
//	DrawLegerLines(m_nPosOnStaff + nSteps, uPos.x, pPaper, colorC);
//
//	//save data for next time
//	m_nOldSteps = nSteps;
//	m_uxOldPos = uPos.x;
//
//	return pos;
//}
//
//void GmoShapeNote::OnEndDrag(lmPaper* pPaper, lmController* pCanvas, const UPoint& uPos)
//{
//	// End drag. Receives the command processor associated to the view and the
//	// final position of the object (logical units referred to page origin).
//	// This method must validate/adjust final position and, if ok, it must
//	// send a move object command to the controller.
//
//	UPoint uFinalPos(uPos.x, uPos.y);
//	int nSteps = 0;
//	if (!g_fFreeMove)
//	{
//		//free movement not allowed. The note must be moved in discrete
//		//vertical steps (half lines)
//		uFinalPos.y = lmCheckNoteNewPosition(((lmNote*)m_pOwner)->GetStaff(), GetYTop(), uPos.y,
//                                             &nSteps);
//	}
//
//	//send a move note command to the controller
//	pCanvas->MoveNote(this, uFinalPos, nSteps);
//}
//
//void GmoShapeNote::AddLegerLinesInfo(int nPosOnStaff, LUnits uyStaffTopLine)
//{
//	m_nPosOnStaff = nPosOnStaff;
//	m_uyStaffTopLine = uyStaffTopLine;
//}
//
//void GmoShapeNote::DrawLegerLines(int nPosOnStaff, LUnits uxLine, lmPaper* pPaper, Color color)
//{
//	lmVStaff* pVStaff = ((lmNote*)m_pOwner)->GetVStaff();
//	int nStaff = ((lmNote*)m_pOwner)->GetStaffNum();
//    GmoShape* pNoteHead = GetNoteHead();
//    LUnits uLineLength = pNoteHead->GetWidth() + pVStaff->TenthsToLogical(8, nStaff);
//
//    lmDrawLegerLines(nPosOnStaff, uxLine, pVStaff, nStaff, uLineLength, m_uyStaffTopLine,
//                     pPaper, color);
//}
//
//
//
////-----------------------------------------------------------------------------------------------
//// global functions
////-----------------------------------------------------------------------------------------------
//
//void lmDrawLegerLines(int nPosOnStaff, LUnits uxLine, lmVStaff* pVStaff, int nStaff,
//                      LUnits uLineLength, LUnits uyStaffTopLine, lmPaper* pPaper,
//                      Color color)
//{
//	//During note drag or while entering notes with mouse, it is necessary to display new leger
//    //lines overlayed on the drag or mouse cursor image. This methos does it.
//    //Parameters:
//    //  nPosOnStaff - notehead position on staff (line/space: 0-first ledger line below staff)
//    //  uLineLength - lenght of ledger lines
//    //  uyStaffTopLine - y position of staff top line (5th line)
//
//    //if note is on staff, nothing to draw
//    if (nPosOnStaff > 0 && nPosOnStaff < 12)
//        return;
//
//    LUnits uThick = pVStaff->GetStaffLineThick(nStaff);
//    uxLine -= pVStaff->TenthsToLogical(4, nStaff);
//
//	//wxLogMessage(_T("[GmoShapeNot/lmDrawLegerLines] uxLine=%.2f"), uxLine );
//
//	//force to paint lines of at least 1 px
//	LUnits uOnePixel = pPaper->DeviceToLogicalY(1);
//	uThick = uOnePixel;
//
//    if (nPosOnStaff > 11)
//	{
//        // pos on staff > 11  ==> lines at top
//        LUnits uDsplz = pVStaff->GetOptionLong(_T("Staff.UpperLegerLines.Displacement"));
//        LUnits uyStart = uyStaffTopLine - pVStaff->TenthsToLogical(uDsplz, nStaff);
//        for (int i=12; i <= nPosOnStaff; i++)
//        {
//            if (i % 2 == 0) {
//                int nTenths = 5 * (i - 10);
//                LUnits uyPos = uyStart - pVStaff->TenthsToLogical(nTenths, nStaff);
//				//draw the line
//				pPaper->SolidLine(uxLine, uyPos, uxLine + uLineLength, uyPos,
//								  uThick, lm_eEdgeNormal, color);
//           }
//        }
//
//    }
//	else
//	{
//        // nPosOnStaff < 1  ==>  lines at bottom
//        for (int i=nPosOnStaff; i <= 0; i++)
//        {
//            if (i % 2 == 0)
//			{
//                int nTenths = 5 * (10 - i);
//                LUnits uyPos = uyStaffTopLine + pVStaff->TenthsToLogical(nTenths, nStaff);
//				//draw the line
//				pPaper->SolidLine(uxLine, uyPos, uxLine + uLineLength, uyPos,
//								  uThick, lm_eEdgeNormal, color);
//				//wxLogMessage(_T("[lmDrawLegerLines] Line from (%.2f, %.2f) to (%.2f, %.2f)"),
//				//	uxLine, uyPos, uxLine + uLineLength, uyPos);
//            }
//        }
//    }
//
//}

}  //namespace lomse
