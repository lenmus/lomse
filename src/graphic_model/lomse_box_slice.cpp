//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#include "lomse_box_slice.h"

#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_box_slice_instr.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
GmoBoxSlice::GmoBoxSlice(int UNUSED(nAbsMeasure), ImoObj* pCreatorImo)
    : GmoBox(GmoBox::k_box_slice, pCreatorImo)
{
}

//---------------------------------------------------------------------------------------
GmoBoxSlice::~GmoBoxSlice()
{
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* GmoBoxSlice::get_system_box()
{
    return dynamic_cast<GmoBoxSystem*>(m_pParentBox);
}

//--------------------------------------------------------------------------------------
GmoBoxSliceInstr* GmoBoxSlice::add_box_for_instrument(ImoInstrument* pInstr,
                                                      int idxStaff)
{
    GmoBoxSliceInstr* pBox = LOMSE_NEW GmoBoxSliceInstr(pInstr, idxStaff);
    add_child_box(pBox);
    return pBox;
}

//---------------------------------------------------------------------------------------
void GmoBoxSlice::reposition_slices_and_shapes(const vector<LUnits>& yOrgShifts,
                                               vector<LUnits>& heights,
                                               vector<LUnits>& barlinesHeight,
                                               SystemLayouter* pSysLayouter)

{
    vector<GmoBox*>::iterator it;
    int iInstr = 0;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it, ++iInstr)
    {
        GmoBoxSliceInstr* pSlice = static_cast<GmoBoxSliceInstr*>(*it);
        pSlice->reposition_slices_and_shapes(yOrgShifts, heights,
                                             barlinesHeight[iInstr], pSysLayouter);
    }

    //shift origin and increase height
    m_origin.y += yOrgShifts[0];
    m_size.height += yOrgShifts.back() + heights[0];
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr* GmoBoxSlice::get_instr_slice(int iInstr)
{
    return static_cast<GmoBoxSliceInstr*>(m_childBoxes[iInstr]);
}

//---------------------------------------------------------------------------------------
GmoBoxSliceStaff* GmoBoxSlice::get_slice_staff_for(int iInstr, int iStaff)
{
    GmoBoxSliceInstr* pSlice = static_cast<GmoBoxSliceInstr*>(m_childBoxes[iInstr]);
    return pSlice->get_slice_staff_for(iStaff);
}

////--------------------------------------------------------------------------------------
//void GmoBoxSlice::DrawSelRectangle(lmPaper* pPaper)
//{
//	//draw system border in red
//	get_system_box()->DrawBounds(pPaper, *wxRED);
//
//    //draw a border around slice region in cyan
//	LUnits yTop = get_system_box()->GetYTop();
//    LUnits yBottom = get_system_box()->GetYBottom();
//
//    pPaper->SketchRectangle(lmUPoint(m_xStart, yTop),
//                            lmUSize(m_xEnd - m_xStart, yBottom - yTop),
//                            *wxCYAN);
//
//}
//
////--------------------------------------------------------------------------------------
//int GmoBoxSlice::GetPageNumber() const
//{
//	return get_system_box()->GetPageNumber();
//}
//
////--------------------------------------------------------------------------------------
//GmoBoxScore* GmoBoxSlice::GetOwnerBoxScore()
//{
//    return get_system_box()->GetOwnerBoxScore();
//}
//
////--------------------------------------------------------------------------------------
//GmoBoxPage* GmoBoxSlice::GetOwnerBoxPage()
//{
//    return get_system_box()->GetOwnerBoxPage();
//}
//
////--------------------------------------------------------------------------------------
//void GmoBoxSlice::SetBottomSpace(LUnits uyValue)
//{
//    //overrided. To propagate bottom space to last instrument
//
//    m_uBottomSpace = uyValue;
//    m_Boxes.back()->SetBottomSpace(uyValue);
//}
//
////--------------------------------------------------------------------------------------
//TimeUnits GmoBoxSlice::GetGridTimeForPosition(LUnits uxPos)
//{
//    if (m_pGridTable)
//        return m_pGridTable->GetTimeForPosititon(uxPos);
//    else
//        return 0.0;
//}
//
////--------------------------------------------------------------------------------------
//void GmoBoxSlice::DrawTimeLines(lmPaper* pPaper, wxColour color, LUnits uyTop,
//                               LUnits uyBottom)
//{
//    //Draw lines for available times in posTimes table. Last timepos corresponds to
//    //barline and is not drawed.
//    //Paper is already set in XOR mode
//
//    for (int i=0; i < m_pGridTable->GetSize()-1; ++i)
//    {
//        LUnits uxPos = m_pGridTable->GetXPos(i);
//        pPaper->SketchLine(uxPos, uyTop, uxPos, uyBottom, color);
//    }
//}


}  //namespace lomse
