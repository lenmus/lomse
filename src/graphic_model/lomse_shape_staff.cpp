//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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

#include "lomse_shape_staff.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"
//#include <sstream>
//using namespace std;

namespace lomse
{

//#define lmNO_LEDGER_LINES   -100000.0f

//---------------------------------------------------------------------------------------
GmoShapeStaff::GmoShapeStaff(GmoBox* owner, ImoStaffInfo* pStaff, int iStaff,
                             LUnits indent, Color color)
    : GmoSimpleShape(owner, GmoObj::k_shape_staff, color) 
    , m_pStaff(pStaff)
	, m_iStaff(iStaff)
{
    //bounding box
    set_width(owner->get_content_width() - indent);
    set_height(m_pStaff->get_height());
}

//---------------------------------------------------------------------------------------
GmoShapeStaff::~GmoShapeStaff()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeStaff::on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin)
{
//    if (!m_fVisible) return;
//
//    //update selection rectangle
//    m_uSelRect = GetBounds();
//
    //draw the staff
    double xStart = m_origin.x + origin.x;
    double xEnd = xStart + m_size.width;
    double yPos = m_origin.y + origin.y;
    double spacing = m_pStaff->get_line_spacing();

    pDrawer->begin_path();
    pDrawer->stroke( m_color );
    pDrawer->stroke_width( m_pStaff->get_line_thickness() );
    for (int iL=0; iL < m_pStaff->get_num_lines(); iL++ )
	{
        pDrawer->move_to(xStart, yPos);
        pDrawer->line_to(xEnd, yPos);
        yPos += spacing;
    }
    pDrawer->end_path();

    GmoSimpleShape::on_draw(pDrawer, opt, origin);
}

////---------------------------------------------------------------------------------------
//wxString GmoShapeStaff::Dump(int nIndent)
//{
//	//TODO
//	wxString sDump = _T("");
//	sDump.append(nIndent * lmINDENT_STEP, _T(' '));
//	sDump += wxString::Format(_T("Idx: %d %s: "), m_nOwnerIdx, m_sGMOName.c_str());
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
//void GmoShapeStaff::Shift(LUnits xIncr, LUnits yIncr)
//{
//	//TODO
//    ShiftBoundsAndSelRec(xIncr, yIncr);
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((lmCompositeShape*)GetParentShape())->RecomputeBounds();
//}
//
////---------------------------------------------------------------------------------------
//int GmoShapeStaff::GetLineSpace(LUnits uyPos)
//{
//    //returns the position (line/space number) for the received point. Position is
//    //referred to the first ledger line of the staff:
//    //        0 - on first ledger line (C note in G clef)
//    //        1 - on next space (D in G clef)
//    //        2 - on first line (E not in G clef)
//    //        3 - on first space
//    //        4 - on second line
//    //        5 - on second space
//    //        etc.
//
//	//The received position could be approximated (i.e. mouse position). So, first, we must
//    //adjust position to the nearest valid line/half-line position
//
//    //int nSteps;
//    //lmCheckNoteNewPosition((lmStaff*)GetScoreOwner(), m_uBoundsTop.y, uyPos, &nSteps);
//    //return nSteps + 10;
//
//    //compute the number of steps (half lines) from line 5 (top staff line = step #10)
//    lmStaff* pStaff = (lmStaff*)GetScoreOwner();
//	LUnits uHalfLine = pStaff->TenthsToLogical(5.0f);
//    float rStep = (m_uBoundsTop.y - uyPos)/uHalfLine;
//    int nStep = (rStep > 0.0f ? (int)(rStep + 0.5f) : (int)(rStep - 0.5f) );
//    //wxLogMessage(_T("[GmoShapeStaff::GetLineSpace] uyPos=%.2f, uHalfLine=%.2f, m_uBoundsTop.y=%.2f, rStep=%.2f, nStep=%d"),
//    //             uyPos, uHalfLine, m_uBoundsTop.y, rStep, nStep);
//	return  10 + nStep;
//    //AWARE: Note that y axis is reversed. Therefore we return 10 + steps instead
//    // of 10 - steps. 
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeStaff::OnMouseStartMoving(lmPaper* pPaper, const UPoint& uPos)
//{
//    m_uxOldPos = lmNO_LEDGER_LINES;
//
//    //wxLogMessage(_T("[GmoShapeStaff::OnMouseStartMoving] nStaff=%d"), m_nStaff);
//    return OnMouseMoving(pPaper, uPos);
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeStaff::OnMouseMoving(lmPaper* pPaper, const UPoint& uPos)
//{
//	//The user continues moving the mouse (tool: insert note) over a valid area for
//    //inserting a note/rest in this staff. We receive the mouse position and we
//    //must return the valid notehead position. We must also erase any previously drawn ledger
//    //lines and draw new ones if necessary
//
//    //A note only can be placed in discrete vertical steps (staff lines/spaces)
//    UPoint pos = uPos;
//	int nSteps;
//    pos.y = lmCheckNoteNewPosition((lmStaff*)GetScoreOwner(), GetYTop(), uPos.y, &nSteps);
//
//		//draw leger lines
//
//    lmVStaff* pVStaff = (lmVStaff*)GetScoreOwner()->GetParentScoreObj();
//
//	//as painting uses XOR we need the complementary color
//	wxColour color = *wxBLUE;
//	wxColour colorC(255 - (int)color.Red(), 255 - (int)color.Green(), 255 - (int)color.Blue() );
//	pPaper->SetLogicalFunction(wxXOR);
//
//    //wxLogMessage(_T("[GmoShapeStaff::OnMouseMoving] VStaff=0x%x, nStaff=%d, OldSteps=%d, oldPos=%.2f, newSteps=%d, newPos=%.2f"),
//    //             pVStaff, m_nStaff, m_nOldSteps, m_uxOldPos, nSteps, uPos.x );
//
//	//remove old ledger lines
//    LUnits uLineLength = 2.5f * m_uSpacing;
//	if (m_uxOldPos != -100000.0f)
//        lmDrawLegerLines(m_nOldSteps+10, m_uxOldPos, pVStaff, m_nStaff,
//                         uLineLength, m_uBoundsTop.y, pPaper, colorC);
//
//	//draw new ledger lines and save data for erasing them the next time
//	m_uxOldPos = uPos.x - m_uSpacing;
//    lmDrawLegerLines(nSteps+10, m_uxOldPos, pVStaff, m_nStaff,
//                        uLineLength, m_uBoundsTop.y, pPaper, colorC);
//	m_nOldSteps = nSteps;
//
//	return pos;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeStaff::OnMouseEndMoving(lmPaper* pPaper, UPoint uPos)
//{
//	// End drag. Receives the final position of the object (logical units referred
//    // to page origin).
//	// This method must remove any XOR painted lines.
//
//	//remove old ledger lines
//	if (m_uxOldPos != lmNO_LEDGER_LINES)
//    {
//	    //as painting uses XOR we need the complementary color
//	    wxColour color = *wxBLUE;
//	    wxColour colorC(255 - (int)color.Red(), 255 - (int)color.Green(), 255 - (int)color.Blue() );
//	    pPaper->SetLogicalFunction(wxXOR);
//
//        LUnits uLineLength = 2.5f * m_uSpacing;
//        lmVStaff* pVStaff = (lmVStaff*)GetScoreOwner()->GetParentScoreObj();
//
// 	    //wxLogMessage(_T("[GmoShapeStaff::OnMouseEndMoving] VStaff=0x%x, nStaff=%d, OldSteps=%d, oldPos=%.2f"),
//		    //         pVStaff, m_nStaff, m_nOldSteps, m_uxOldPos);
//
//        lmDrawLegerLines(m_nOldSteps+10, m_uxOldPos, pVStaff, m_nStaff,
//                        uLineLength, m_uBoundsTop.y, pPaper, colorC);
//    }
//    //else
//	   // wxLogMessage(_T("[GmoShapeStaff::OnMouseEndMoving] No ledger lines to remove") );
//
//}
//
////---------------------------------------------------------------------------------------
//lmVStaff* GmoShapeStaff::GetOwnerVStaff()
//{
//    return ((lmStaff*)m_pOwner)->GetOwnerVStaff();
//}
//

}  //namespace lomse
