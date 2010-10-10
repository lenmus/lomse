//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_ID_ASSIGNER_H__
#define __LOMSE_ID_ASSIGNER_H__

#include "lomse_ldp_elements.h"

namespace lomse
{

//IdAssigner definition
class IdAssigner
{
protected:
    long m_idCounter;

public:
    IdAssigner();
    ~IdAssigner() {}

    void reassign_ids(LdpElement* pElm);
    void reassign_ids(LdpTree* pTree);
    inline void set_last_id(long id) { m_idCounter = id + 1L; }
    inline long get_last_id() { return m_idCounter - 1L; }

protected:
    long find_min_id(LdpTree* pTree);
    void shift_ids(LdpTree* pTree, long shift);

};


} //namespace lomse

#endif    //__LOMSE_ID_ASSIGNER_H__
