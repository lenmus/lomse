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
RelObjCloner::~RelObjCloner()
{
    //AWARE: For a properly cloned tree m_pending must be empty
    if (m_pending.size() > 0)
    {
        stringstream msg;
        msg << "RelObjCloner not empty! Num.elements= " << m_pending.size()<< ", ";

        unordered_map<ImoId, CloneData*>::iterator it;
        for (it=m_pending.begin(); it != m_pending.end(); ++it)
        {
            CloneData* data = it->second;
            ImoRelObj* pNewRelObj = data->relobj;
            msg << pNewRelObj->get_name() << "#" << it->first << ", ";
            delete data;
        }
        LOMSE_LOG_ERROR(msg.str());
    }

    m_pending.clear();
}

//---------------------------------------------------------------------------------------
ImoRelObj* RelObjCloner::clone_relobj(ImoRelObj* pRelObj)
{
    ImoRelObj* pNewRelObj = nullptr;

    //find RelObj in map of pending RelObjs
    ImoId id = pRelObj->get_id();
	unordered_map<ImoId, CloneData*>::const_iterator it = m_pending.find(id);
	if (it != m_pending.end())
    {
        //increment numObjs. If numObjs == ImoRelObj.num_participants() remove ImoRelObj
        //from the map.
        CloneData* data = it->second;
        pNewRelObj = data->relobj;
        if (++data->numObjs == pNewRelObj->get_num_objects())
        {
            delete data;
            m_pending.erase(it);
        }
    }
    else
    {
        //clone the ImoRelObj and add it to the pending map
        pNewRelObj = static_cast<ImoRelObj*>( ImFactory::clone(pRelObj));
        if (pRelObj->get_num_objects() > 1)    //AWARE: grace notes can have only one
            m_pending.emplace(id, LOMSE_NEW CloneData(pNewRelObj));
    }

    //in any case, return the cloned ImoRelObj to the caller ImoRelations
    return pNewRelObj;
}


}  //namespace lomse
