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

namespace lomse
{

//---------------------------------------------------------------------------------------
GmoBoxSystem::GmoBoxSystem(ImoObj* pCreatorImo)    //, int nNumPage, int iSystem,
    : GmoBox(GmoObj::k_box_system, pCreatorImo)
//    , m_nNumPage(nNumPage)
//    , m_pBPage(pParent)
//	, m_pTopSpacer(NULL)
{
//    set_left_margin(pScore->GetSystemLeftSpace(iSystem));
}

//---------------------------------------------------------------------------------------
GmoBoxSystem::~GmoBoxSystem()
{
}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GmoBoxSystem::add_staff_shape(GmoShapeStaff* pShape)
{
	m_staffShapes.push_back(pShape);
    add_shape(pShape, GmoShape::k_layer_staff);
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GmoBoxSystem::get_staff_shape(int iStaff)
{
	//returns the shape for staff iStaff (0..n-1). iStaff is the staff number
    //relative to total staves in system

    return m_staffShapes[iStaff];
}


}  //namespace lomse
