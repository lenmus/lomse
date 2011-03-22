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

//=======================================================================================
// GmoShapeNote implementation
//=======================================================================================
GmoShapeNote::GmoShapeNote(ImoObj* pCreatorImo, LUnits x, LUnits y, Color color,
                           LibraryScope& libraryScope)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_note, 0, color)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
    , m_pNoteheadShape(NULL)
	, m_pStemShape(NULL)
    , m_pAccidentalsShape(NULL)
    , m_pFlagShape(NULL)
    , m_uAnchorOffset(0.0f)
{
//	m_pBeamShape = (GmoShapeBeam*)NULL;
//    m_pTieShape[0] = (GmoShapeTie*)NULL;
//    m_pTieShape[1] = (GmoShapeTie*)NULL;
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
    ////DBG: Draw anchor line
    //pDrawer->begin_path();
    //pDrawer->fill(Color(0, 0, 0, 0));
    //pDrawer->stroke(Color(255, 0, 255));
    //pDrawer->stroke_width(15.0);
    //pDrawer->move_to(m_origin.x - get_anchor_offset(), m_origin.y);
    //pDrawer->vline_to(m_origin.y + m_size.height);
    //pDrawer->end_path();

    draw_leger_lines(pDrawer);
    GmoCompositeShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::draw_leger_lines(Drawer* pDrawer)
{
    //if note is on staff, nothing to draw
    if (m_nPosOnStaff > 0 && m_nPosOnStaff < 12)
        return;

    pDrawer->begin_path();
    pDrawer->fill(Color(0, 0, 0, 0));
    pDrawer->stroke(Color(0, 0, 0));
    pDrawer->stroke_width(m_uLineThickness);

    LUnits xPos = get_notehead_left() - m_uLineOutgoing;
    LUnits lineLength = get_notehead_width() + 2.0f * m_uLineOutgoing;

    if (m_nPosOnStaff > 11)     //lines at top
	{
        LUnits yPos = m_uyStaffTopLine + get_notehead_top() - m_lineSpacing;
        for (int i=12; i <= m_nPosOnStaff; i+=2)
        {
            pDrawer->move_to(xPos, yPos);
            pDrawer->hline_to(xPos + lineLength);
            yPos -= m_lineSpacing;
        }
    }
	else    //lines at bottom
	{
        LUnits yPos = m_uyStaffTopLine + get_notehead_top() + m_lineSpacing * 5.0f;
        for (int i=0; i >= m_nPosOnStaff; i-=2)
        {
            pDrawer->move_to(xPos, yPos);
            pDrawer->hline_to(xPos + lineLength);
            yPos += m_lineSpacing;
        }
    }

    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_stem(GmoShapeStem* pShape)
{
	add(pShape);
	m_pStemShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_notehead(GmoShapeNotehead* pShape)
{
	add(pShape);
	m_pNoteheadShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_flag(GmoShapeFlag* pShape)
{
	add(pShape);
    m_pFlagShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_accidentals(GmoShapeAccidentals* pShape)
{
	add(pShape);
    m_pAccidentalsShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_leger_lines_info(int posOnStaff, LUnits yStaffTopLine,
                                        LUnits lineOutgoing, LUnits lineThickness,
                                        LUnits lineSpacing)
{
	m_nPosOnStaff = posOnStaff;
	m_uyStaffTopLine = yStaffTopLine;   //relative to notehead top
    m_uLineOutgoing = lineOutgoing;
    m_uLineThickness = lineThickness;
    m_lineSpacing = lineSpacing;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_note_in_block(GmoShape* pShape)
{
	add(pShape);
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::set_stem_down(bool down)
{
    set_up_oriented(!down);

    if (!m_pStemShape)
        return;

    if (down && !m_pStemShape->is_stem_down())
    {
        LUnits xLeft = get_notehead_left();
        LUnits yNote = get_notehead_bottom() - m_pStemShape->get_y_note()
                       + get_notehead_top();
        m_pStemShape->set_stem_down(xLeft, yNote);
    }
    else if (!down && m_pStemShape->is_stem_down())
    {
        LUnits xRight = get_notehead_right();
        LUnits yNote = get_notehead_bottom() - m_pStemShape->get_y_note()
                       + get_notehead_top();
        m_pStemShape->set_stem_up(xRight, yNote);
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::set_stem_length(LUnits length)
{
    if (m_pStemShape)
        m_pStemShape->change_length(length);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_width() const
{
    return m_pNoteheadShape->get_width();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_left() const
{
    return m_pNoteheadShape->get_left();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_right() const
{
    return m_pNoteheadShape->get_right();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_height() const
{
    return m_pNoteheadShape->get_height();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_top() const
{
    return m_pNoteheadShape->get_top();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_bottom() const
{
    return m_pNoteheadShape->get_bottom();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_width() const
{
    return (m_pStemShape ? m_pStemShape->get_width() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_height() const
{
    return (m_pStemShape ? m_pStemShape->get_height() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_left() const
{
    return (m_pStemShape ? m_pStemShape->get_left() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_y_flag() const
{
    return (m_pStemShape ? m_pStemShape->get_y_flag() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_y_note() const
{
    return (m_pStemShape ? m_pStemShape->get_y_note() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_extra_length() const
{
    return (m_pStemShape ? m_pStemShape->get_extra_length() : 0.0f);
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
//



//=======================================================================================
// GmoShapeNote implementation
//=======================================================================================
GmoShapeRest::GmoShapeRest(ImoObj* pCreatorImo, int idx, LUnits x, LUnits y, Color color,
                           LibraryScope& libraryScope)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_rest, idx, color)
    , m_libraryScope(libraryScope)
	, m_pBeamShape(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShapeRest::~GmoShapeRest()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeRest::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
//    //base class method is overrided to deal with rests inside a beamed group.
//    //The beam and the stems are rendered *after* noteheads and rests are rendered.
//    //Therefore, when rendering the beam there is no option to adjust rests positions
//    //to fit gracefuly inside the beamed group.
//    //By overriding this method, if the rest is inside a beamed group and it is
//    //the first rest in that beamed group, will force the beam shape to compute stems,
//    //and, therefore, to adjust all rests' positions.
//
//
//    //if the rest is inside of a beamed group ensure that beam is layouted
//    if (m_pBeamShape)
//        m_pBeamShape->AdjustStemsIfNeeded();
//
//    //now, we can safely render the rest
    GmoCompositeShape::on_draw(pDrawer, opt);
}


}  //namespace lomse
