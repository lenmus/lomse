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
