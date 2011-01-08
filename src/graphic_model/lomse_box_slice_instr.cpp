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

#include "lomse_box_slice_instr.h"

#include "lomse_internal_model.h"
#include "lomse_box_slice.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr::GmoBoxSliceInstr()  //, ImoInstrument* pInstr)
    : GmoBox(GmoObj::k_box_slice_instr)
    //, m_pSlice(pParent)
    //, m_pInstr(pInstr)
{
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr::~GmoBoxSliceInstr()
{
}

////---------------------------------------------------------------------------------------
//int GmoBoxSliceInstr::GetPageNumber() const
//{
//	return m_pSlice->GetPageNumber();
//}
//
////---------------------------------------------------------------------------------------
//GmoShapeStaff* GmoBoxSliceInstr::GetStaffShape(int nStaff)
//{
//    //nStaff = 1..n
//
//    wxASSERT(nStaff > 0 && nStaff <= m_pInstr->GetNumStaves());
//
//    return GetOwnerSystem()->GetStaffShape(m_pInstr, nStaff);
//}
//
////---------------------------------------------------------------------------------------
//GmoShapeStaff* GmoBoxSliceInstr::GetNearestStaff(lmUPoint& uPoint)
//{
//    //returns the nearest staff to point uPoint
//
//    return GetOwnerSystem()->GetStaffShape(m_pInstr, uPoint);
//}
//
////---------------------------------------------------------------------------------------
//void GmoBoxSliceInstr::DrawTimeGrid(lmPaper* pPaper)
//{
//	//as painting uses XOR we need the complementary color
//	wxColour color(192,192,192);    //TODO: User option
//	wxColour colorC(255 - (int)color.Red(), 255 - (int)color.Green(), 255 - (int)color.Blue() );
//	pPaper->SetLogicalFunction(wxXOR);
//
//    //Draw the limits rectangle
//    lmUPoint uTopLeft(m_uBoundsTop.x - m_uLeftSpace, m_uBoundsTop.y - m_uTopSpace);
//    lmUSize uSize( GetWidth() + m_uLeftSpace + m_uRightSpace,
//                   GetHeight() + m_uTopSpace + m_uBottomSpace );
//    //wxLogMessage(_T("[GmoBoxSliceInstr::DrawTimeGrid] rect=(%.2f, %.2f, %.2f, %.2f)"),
//    //             uTopLeft.x, uTopLeft.y, uTopLeft.x+uSize.GetWidth(), uTopLeft.y+uSize.GetHeight() );
//    pPaper->SketchRectangle(uTopLeft, uSize, colorC);
//
//    //draw vertical lines for existing times
//    ((GmoBoxSlice*)GetParentBox())->DrawTimeLines( pPaper, colorC, uTopLeft.y,
//                                                  uTopLeft.y + uSize.GetHeight() );
//}
//
////---------------------------------------------------------------------------------------
//void GmoBoxSliceInstr::DrawMeasureFrame(lmPaper* pPaper)
//{
//	//as painting uses XOR we need the complementary color
//	wxColour color(255,0,0);    //TODO: User option
//	wxColour colorC(255 - (int)color.Red(), 255 - (int)color.Green(), 255 - (int)color.Blue() );
//	pPaper->SetLogicalFunction(wxXOR);
//
//    //determine first and last staves
//    GmoShape* pFirstStaff = (GmoShape*)GetStaffShape(1);
//    GmoShape* pLastStaff = (GmoShape*)GetStaffShape( m_pInstr->GetNumStaves() );
//    lmLUnits yTop = pFirstStaff->GetBounds().GetLeftTop().y;
//    lmLUnits dyHalfLine = (pFirstStaff->GetBounds().GetLeftBottom().y - yTop) / 8.0;
//    lmLUnits yBottom = pLastStaff->GetBounds().GetLeftBottom().y;
//    yTop -= dyHalfLine;
//    yBottom += dyHalfLine;
//
//    //Draw the limits rectangle
//    lmUPoint uTopLeft(m_uBoundsTop.x - m_uLeftSpace, yTop);
//    lmUSize uSize( GetWidth() + m_uLeftSpace + m_uRightSpace, yBottom - yTop );
//    pPaper->SketchRectangle(uTopLeft, uSize, colorC);
//}


}  //namespace lomse
