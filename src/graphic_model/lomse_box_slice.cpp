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

#include "lomse_box_slice.h"

#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_box_slice_instr.h"
//#include "lomse_internal_model.h"
//#include <iostream>
//#include <iomanip>
//#include "lomse_im_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
GmoBoxSlice::GmoBoxSlice(int nAbsMeasure)    //, int nNumInSystem,
					     //LUnits xStart, LUnits xEnd)
    : GmoBox(GmoBox::k_box_slice)
//    , m_nAbsMeasure(nAbsMeasure)
//	, m_nNumInSystem(nNumInSystem)
//    , m_xStart(xStart)
//    , m_xEnd(xEnd)
//    , m_pGridTable((lmTimeGridTable*)NULL)
{
}

//---------------------------------------------------------------------------------------
GmoBoxSlice::~GmoBoxSlice()
{
//    if (m_pGridTable)
//        delete m_pGridTable;
}

GmoBoxSystem* GmoBoxSlice::get_system_box()
{
    return dynamic_cast<GmoBoxSystem*>(m_pParentBox);
}

//--------------------------------------------------------------------------------------
GmoBoxSliceInstr* GmoBoxSlice::add_box_for_instrument(ImoInstrument* pInstr)
{
    GmoBoxSliceInstr* pBox = new GmoBoxSliceInstr();    //, pInstr);
    add_child_box(pBox);
    return pBox;
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
//float GmoBoxSlice::GetGridTimeForPosition(LUnits uxPos)
//{
//    if (m_pGridTable)
//        return m_pGridTable->GetTimeForPosititon(uxPos);
//    else
//        return 0.0f;
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
