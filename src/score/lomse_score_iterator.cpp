//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------


#include "lomse_score_iterator.h"

using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
// StaffObjsIterator implementation
//---------------------------------------------------------------------------------------
StaffObjsIterator::StaffObjsIterator(ColStaffObjs* pColStaffObjs)
    : m_pColStaffObjs(pColStaffObjs)
{
    first();
}

//---------------------------------------------------------------------------------------
void StaffObjsIterator::first()
{
    m_it = m_pColStaffObjs->begin();
}



}  //namespace lomse
