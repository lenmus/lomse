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

#ifndef __LOMSE_ID_ASSIGNER_H__
#define __LOMSE_ID_ASSIGNER_H__

#include "lomse_ldp_elements.h"

namespace lomse
{

//forward declarations
class ImoObj;

//---------------------------------------------------------------------------------------
//IdAssigner: responsible for assigning/re-assingning ids to ImoObj objects
class IdAssigner
{
protected:
    long m_idCounter;

public:
    IdAssigner();
    ~IdAssigner() {}

    void reassign_ids(LdpElement* pElm);
    void reassign_ids(SpLdpTree pTree);
    inline void set_last_id(long id) { m_idCounter = id + 1L; }
    inline long get_last_id() { return m_idCounter - 1L; }

    void assign_id(ImoObj* pImo);

protected:
    long find_min_id(SpLdpTree pTree);
    void shift_ids(SpLdpTree pTree, long shift);

};


} //namespace lomse

#endif    //__LOMSE_ID_ASSIGNER_H__
