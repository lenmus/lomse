//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_engravers_map.h"

#include "lomse_engraver.h"
#include "lomse_internal_model.h"


namespace lomse
{

//=======================================================================================
// EngraversMap implementation
//=======================================================================================
Engraver* EngraversMap::get_engraver(ImoObj* pImo)
{
    map<ImoObj*, Engraver*>::const_iterator it = m_engravers.find(pImo);
    if (it !=  m_engravers.end())
        return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
Engraver* EngraversMap::get_engraver(const string& tag)
{
    map<string, Engraver*>::const_iterator it = m_engravers2.find(tag);
    if (it !=  m_engravers2.end())
        return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void EngraversMap::delete_engravers()
{
	std::map<ImoObj*, Engraver*>::const_iterator it;
    for (it = m_engravers.begin(); it != m_engravers.end(); ++it)
    {
        delete it->second;
    }

	std::map<string, Engraver*>::const_iterator it2;
    for (it2 = m_engravers2.begin(); it2 != m_engravers2.end(); ++it2)
    {
        delete it2->second;
    }
}



}  //namespace lomse
