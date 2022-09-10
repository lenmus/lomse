//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_RELOBJ_CLONER_H__        //to avoid nested includes
#define __LOMSE_RELOBJ_CLONER_H__

#include "lomse_basic.h"

#include <unordered_map>


namespace lomse
{

//forward declarations
class ImoRelObj;
class ImoStaffObj;


//---------------------------------------------------------------------------------------
// RelObjCloner: encloses the algorithm to clone ImoRelObj objects
class RelObjCloner
{
protected:
    struct CloneData
    {
        int numObjs=0;
        ImoRelObj* relobj = nullptr;

        CloneData(ImoRelObj* pRO) : numObjs(1), relobj(pRO) {}
    };

    std::unordered_map<ImoId, CloneData*> m_pending;

public:
    RelObjCloner() {}
    ~RelObjCloner();

    ImoRelObj* clone_relobj(ImoRelObj* pRelObj);

};


}   //namespace lomse

#endif    // __LOMSE_RELOBJ_CLONER_H__

