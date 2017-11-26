//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_box_system.h"

#include "lomse_box_slice.h"
#include "lomse_internal_model.h"
#include "lomse_shape_staff.h"
#include "lomse_timegrid_table.h"
#include "lomse_drawer.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
GmoBoxSystem::GmoBoxSystem(ImoObj* pCreatorImo)    //, int nNumPage, int iSystem,
    : GmoBox(GmoObj::k_box_system, pCreatorImo)
    , m_pGridTable(nullptr)
//    , m_nNumPage(nNumPage)
//    , m_pBPage(pParent)
//	, m_pTopSpacer(nullptr)
{
//    set_left_margin(pScore->GetSystemLeftSpace(iSystem));
}

//---------------------------------------------------------------------------------------
GmoBoxSystem::~GmoBoxSystem()
{
    delete m_pGridTable;
}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GmoBoxSystem::add_staff_shape(GmoShapeStaff* pShape)
{
	m_staffShapes.push_back(pShape);
    add_shape(pShape, GmoShape::k_layer_staff);
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GmoBoxSystem::get_staff_shape(int absStaff)
{
	//returns the shape for staff absStaff (0..n-1). absStaff is the staff number
    //referred to total staves in system

    return m_staffShapes[absStaff];
}

//---------------------------------------------------------------------------------------
GmoShapeStaff* GmoBoxSystem::get_staff_shape(int iInstr, int iStaff)
{
    if (iInstr == 0)
        return m_staffShapes[iStaff];
    else
        return m_staffShapes[ m_firstStaff[iInstr-1] + iStaff ];
}

//---------------------------------------------------------------------------------------
void GmoBoxSystem::add_num_staves_for_instrument(int staves)
{
    if (m_firstStaff.size() == 0)
        m_firstStaff.push_back(staves);
    else
        m_firstStaff.push_back( m_firstStaff.back() + staves);
}

//---------------------------------------------------------------------------------------
int GmoBoxSystem::instr_number_for_staff(int absStaff)
{
    int maxInstr = int( m_firstStaff.size() );
    for (int i=0; i < maxInstr; ++i)
    {
        if (absStaff < m_firstStaff[i])
            return i;
    }
    return maxInstr;
}

//---------------------------------------------------------------------------------------
int GmoBoxSystem::staff_number_for(int absStaff, int UNUSED(iInstr))
{
    if (absStaff < m_firstStaff[0])
        return absStaff;

    int staff = absStaff;
    int maxInstr = int( m_firstStaff.size() );
    for (int i=1; i < maxInstr; ++i)
    {
        if (staff < m_firstStaff[i])
            return staff - m_firstStaff[i-1];
    }
    return staff - m_firstStaff.back();
}

//---------------------------------------------------------------------------------------
int GmoBoxSystem::nearest_staff_to_point(LUnits y)
{
    //The y coordinate should be within system box limits.

    int maxStaff = int( m_staffShapes.size() );
    int iStaff = 0;
    GmoShapeStaff* pStaff = m_staffShapes[iStaff++];
    LUnits bottomPrev = pStaff->get_bottom();
    while (iStaff < maxStaff)
    {
        GmoShapeStaff* pStaff = m_staffShapes[iStaff++];
        LUnits limit = bottomPrev + (pStaff->get_top() - bottomPrev) / 2.0;
        if (y < limit)
            return iStaff-2;
        bottomPrev = pStaff->get_bottom();
    }
    return maxStaff-1;
}


}  //namespace lomse
