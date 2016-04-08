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

#ifndef __LOMSE_ID_ASSIGNER_H__
#define __LOMSE_ID_ASSIGNER_H__

#include "lomse_basic.h"

#include <map>
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class Control;

//---------------------------------------------------------------------------------------
//IdAssigner: responsible for assigning/re-assigning ids to ImoObj and Control
// objects and providing access to them by Id
class IdAssigner
{
protected:
    ImoId m_idCounter;
    std::map<ImoId, ImoObj*> m_idToImo;
    std::map<ImoId, Control*> m_idToControl;

public:
    IdAssigner();
    ~IdAssigner() {}

    void reset();

    void assign_id(ImoObj* pImo);
    void assign_id(Control* pControl);
    ImoId reserve_id(ImoId id);
    ImoObj* get_pointer_to_imo(ImoId id) const;
    Control* get_pointer_to_control(ImoId id) const;
    void remove(ImoObj* pImo);
    void copy_ids_to(IdAssigner* assigner, ImoId idMin);

    //debug
    string dump() const;
    inline size_t size() const { return m_idToImo.size(); }

protected:
    void add_id(ImoId id, ImoObj* pImo);
    void add_control_id(ImoId id, Control* pControl);

};


} //namespace lomse

#endif    //__LOMSE_ID_ASSIGNER_H__
