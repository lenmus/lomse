//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
                                               const vector<LUnits>& heights,
                                               const vector<LUnits>& barlinesHeight,
                                               const vector<vector<LUnits>>& relStaffTopPositions,
                                               LUnits bottomMarginIncr,
                                               SystemLayouter* pSysLayouter)

{
    vector<GmoBox*>::iterator it;
    int iInstr = 0;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it, ++iInstr)
    {
        GmoBoxSliceInstr* pSlice = static_cast<GmoBoxSliceInstr*>(*it);
        pSlice->reposition_slices_and_shapes(yOrgShifts, heights,
                                             barlinesHeight[iInstr], relStaffTopPositions[iInstr],
                                             pSysLayouter);
    }

    //increase height
    m_size.height += (yOrgShifts.back() + bottomMarginIncr);
}

//---------------------------------------------------------------------------------------
void GmoBoxSlice::reduce_last_instrument_height(LUnits space)
{
    //reduce height of this slice
    m_size.height -= space;

    //reduce height of last instrument slice
    GmoBoxSliceInstr* pSlice = static_cast<GmoBoxSliceInstr*>(m_childBoxes.back());
    pSlice->set_height( pSlice->get_height() - space );
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
