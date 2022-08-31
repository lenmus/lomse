//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_relobj_cloner.h"

#include "lomse_internal_model.h"
#include "lomse_im_factory.h"

using namespace std;


namespace lomse
{

//=======================================================================================
// RelObjCloner implementation
//=======================================================================================
ImoRelObj* RelObjCloner::clone_relobj(ImoRelObj* pRelObj)
{
    ImoRelObj* pNewRelObj = nullptr;

    //find RelObj in map of pending RelObjs
	unordered_map<ImoId, CloneData*>::const_iterator it = m_pending.find(pRelObj->get_id());
	if (it != m_pending.end())
    {
        //increment numObjs. If numObjs == ImoRelObj.num_participants() remove ImoRelObj
        //from the map.
        CloneData* data = it->second;
        pNewRelObj = data->relobj;
        if (++data->numObjs == pNewRelObj->get_num_objects())
            m_pending.erase(pRelObj->get_id());
    }
    else
    {
        //clone the ImoRelObj and add it to the pending map
        pNewRelObj = static_cast<ImoRelObj*>( ImFactory::clone(pRelObj));
        m_pending[pRelObj->get_id()] = LOMSE_NEW CloneData(pNewRelObj);
    }

    //in any case, return the cloned ImoRelObj to the caller ImoRelations
    return pNewRelObj;
}


}  //namespace lomse
