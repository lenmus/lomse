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
//--------------------------------------------------------------------------------------

#include "lomse_box_system.h"

#include "lomse_box_slice.h"
#include "lomse_internal_model.h"
#include "lomse_shape_staff.h"
//#include <sstream>
//using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
GmoBoxSystem::GmoBoxSystem()    //, int nNumPage, int iSystem,
    : GmoBox(GmoObj::k_box_system)
//    , m_nNumPage(nNumPage)
//    , m_pBPage(pParent)
//	, m_pTopSpacer(NULL)
{
//    set_left_margin(pScore->GetSystemLeftSpace(iSystem));
}

//---------------------------------------------------------------------------------------
GmoBoxSystem::~GmoBoxSystem()
{
//    ClearStaffShapesTable();
}

//---------------------------------------------------------------------------------------
GmoBoxSlice* GmoBoxSystem::add_slice(int nAbsMeasure)   //, LUnits xStart, LUnits xEnd)
{
    GmoBoxSlice* pBSlice = new GmoBoxSlice(nAbsMeasure); //, (int)m_childBoxes.size(), xStart, xEnd);
    add_child_box(pBSlice);
    return pBSlice;
}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GmoBoxSystem::add_staff_shape(GmoShapeStaff* pShape)
{
	m_staffShapes.push_back(pShape);
    GmoBox::add_shape(pShape, GmoShape::k_layer_staff);
    return pShape;
}

//---------------------------------------------------------------------------------------
//void GmoBoxSystem::set_origin(LUnits xPos, LUnits yPos)
//{
//	m_xPos = xPos;
//	m_yPos = yPos;
//
//	//reposition the handlers
//	if (m_pTopSpacer)
//	{
//		m_pTopSpacer->SetYBottom(yPos);
//		m_pTopSpacer->set_top(yPos);
//	}
//}
//
//---------------------------------------------------------------------------------------
//void GmoBoxSystem::DeleteLastSlice()
//{
//    //This method is used during layout phase, to delete a column when finally it is decided not
//    //to include it in current system
//
//    //delete last slice
//	delete m_childBoxes.back();
//	m_childBoxes.pop_back();
//}
//
//---------------------------------------------------------------------------------------
//LUnits GmoBoxSystem::GetYTopFirstStaff()
//{
//	// Returns the Y top position of first staff
//
//	return m_staffShapes[0]->pShape->GetYTop();
//}
//
//---------------------------------------------------------------------------------------
//GmoShapeStaff* GmoBoxSystem::FindStaffAtPosition(lmUPoint& uPoint)
//{
//	//is it any staff?
//    for (int i=0; i < (int)m_staffShapes.size(); i++)
//    {
//        if (m_staffShapes[i]->pShape->BoundsContainsPoint(uPoint))
//			return m_staffShapes[i]->pShape;
//    }
//	return (GmoShapeStaff*)NULL;
//}

//---------------------------------------------------------------------------------------
//void GmoBoxSystem::UpdateXRight(LUnits xRight)
//{
//    //override to update only last slice of this system and the ShapeStaff final position
//
//    SetXRight(xRight);
//
//	//propagate change to last slice of this system
//	if (m_childBoxes.size() > 0)
//		((GmoBoxSlice*)m_childBoxes.back())->UpdateXRight(xRight);
//
//	//update the ShapeStaff final position
//    for (int i=0; i < (int)m_staffShapes.size(); i++)
//    {
//        m_staffShapes[i]->pShape->SetXRight(xRight);
//    }
//}
//
//
//---------------------------------------------------------------------------------------
//int GmoBoxSystem::GetPageNumber() const
//{
//	return m_pBPage->GetPageNumber();
//}
//
//---------------------------------------------------------------------------------------
//int GmoBoxSystem::GetSystemNumber()
//{
//	//return number of this system (1..n)
//
//	return m_pBPage->GetSystemNumber(this);
//}
//
//---------------------------------------------------------------------------------------
//GmoStubScore* GmoBoxSystem::GetBoxScore()
//{
//	//return owner BoxScore
//
//	return m_pBPage->GetBoxScore();
//}
//
//---------------------------------------------------------------------------------------
//GmoBoxSlice* GmoBoxSystem::FindBoxSliceAt(LUnits uxPos)
//{
//	//return slice located at uxPos
//
//	return (GmoBoxSlice*)FindChildBoxAt(uxPos);
//}
//
//---------------------------------------------------------------------------------------
//int GmoBoxSystem::GetNumMeasureAt(LUnits uxPos)
//{
//	GmoBoxSlice* pSlice = FindBoxSliceAt(uxPos);
//	if (!pSlice)
//		return 0;
//	else
//		return pSlice->GetNumMeasure();
//}
//
//---------------------------------------------------------------------------------------
//GmoStubScore* GmoBoxSystem::GetOwnerBoxScore()
//{
//    return m_pBPage->GetOwnerBoxScore();
//}
//
//---------------------------------------------------------------------------------------
//void GmoBoxSystem::SetBottomSpace(LUnits uyValue)
//{
//    //overrided. To propagate bottom space to slice boxes
//
//    m_uBottomSpace = uyValue;
//
//	//propagate change
//    std::vector<GmoBox*>::iterator itB;
//	for (itB=m_childBoxes.begin(); itB != m_childBoxes.end(); ++itB)
//        (*itB)->SetBottomSpace(uyValue);
//}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GmoBoxSystem::get_staff_shape(int iStaff)
{
	//returns the shape for staff iStaff (0..n-1). iStaff is the staff number
    //relative to total staves in system

    return m_staffShapes[iStaff];
}

//---------------------------------------------------------------------------------------
//GmoShapeStaff* GmoBoxSystem::get_staff_shape(ImoInstrument* pInstr, int nStaff)
//{
//	//returns the shape for staff nStaff (1..n) in instrument pInstr.
//    //That is, nStaff is relative to the number of staves in the instrument, not
//    //to the total number of staves in the system
//
//    wxASSERT(nStaff > 0  && nStaff <= pInstr->GetNumStaves());
//
//    std::vector<ShapeStaffData*>::iterator it;
//    for (it=m_staffShapes.begin(); it != m_staffShapes.end(); ++it)
//    {
//        if ((*it)->pInstr == pInstr && (*it)->nStaff == nStaff)
//            return (*it)->pShape;
//    }
//    wxASSERT(false);    //impossible. It should have found the shape!
//    return (GmoShapeStaff*)NULL;
//}
//
//---------------------------------------------------------------------------------------
//GmoShapeStaff* GmoBoxSystem::get_staff_shape(ImoInstrument* pInstr, lmUPoint uPoint)
//{
//	//For instrument nInstr, returns the nearest staff to point. That is, the staff
//    //belongs to instrument nInstr.
//
//    LUnits uDistance = 10000000000.0f;                    //any impossible big value
//    GmoShapeStaff* pShapeStaff = (GmoShapeStaff*)NULL;        //nearest staff
//    std::vector<ShapeStaffData*>::iterator it;
//    for (it=m_staffShapes.begin(); it != m_staffShapes.end(); ++it)
//    {
//        if ((*it)->pInstr == pInstr)
//        {
//            lmURect uBounds = (*it)->pShape->GetBounds();
//            //in GCC there are problems to use 'abs'. std::abs is definded only for int
//            //LUnits uThisDistance = wxMin(abs((double)(uPoint.y - uBounds.GetLeftTop().y) ),
//            //                               abs((double)(uPoint.y - uBounds.GetBottomLeft().y) ));
//            LUnits uUpDistance = uPoint.y - uBounds.GetLeftTop().y;
//            if (uUpDistance < 0.0f)
//                uUpDistance = - uUpDistance;
//            LUnits uDownDistance = uPoint.y - uBounds.GetBottomLeft().y;
//            if (uDownDistance < 0.0f)
//                uDownDistance = - uDownDistance;
//            LUnits uThisDistance = wxMin(uUpDistance, uDownDistance);
//
//            if (uDistance > uThisDistance)
//            {
//                uDistance = uThisDistance;
//                pShapeStaff = (*it)->pShape;
//            }
//        }
//        else if (pShapeStaff)
//            return pShapeStaff;
//    }
//    wxASSERT(pShapeStaff);    //It should have found a shape!
//    return pShapeStaff;
//}
//
//---------------------------------------------------------------------------------------
//void GmoBoxSystem::ClearStaffShapesTable()
//{
//    std::vector<ShapeStaffData*>::iterator it;
//    for (it=m_staffShapes.begin(); it != m_staffShapes.end(); ++it)
//        delete *it;
//
//    m_staffShapes.clear();
//}
//
//---------------------------------------------------------------------------------------
//int GmoBoxSystem::GetNumMeasures()
//{
//    return (int)m_childBoxes.size();
//}

//---------------------------------------------------------------------------------------


}  //namespace lomse
