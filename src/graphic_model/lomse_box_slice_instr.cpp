//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_box_slice_instr.h"

#include "lomse_internal_model.h"
#include "lomse_box_slice.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr::GmoBoxSliceInstr(ImoInstrument* pInstr)
    : GmoBox(GmoObj::k_box_slice_instr, pInstr)
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
