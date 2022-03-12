//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ENGRAVERS_MAP_H__        //to avoid nested includes
#define __LOMSE_ENGRAVERS_MAP_H__

#include "lomse_basic.h"

#include <map>
using namespace std;

namespace lomse
{

//forward declarations
class Engraver;
class ImoObj;


//---------------------------------------------------------------------------------------
// Helper class to store the engravers for shapes under construction so that they can
//  later be retrieved.
//  It is used when the shap creation process involves two or more ImoStaffObj objects.
//  This is the case, for example, of lyrics, chords, ImoRelObj and ImoAuxRelObj.
class EngraversMap
{
protected:
	std::map<ImoObj*, Engraver*> m_engravers;   //engraver for an ImoObj
	std::map<string, Engraver*> m_engravers2;   //engraver for a tag (for lyrics)

public:
    EngraversMap() {}
    ~EngraversMap() {}

    //engravers
    inline void save_engraver(Engraver* pEngrv, ImoObj* pImo) {
        m_engravers[pImo] = pEngrv;
    }
    inline void save_engraver(Engraver* pEngrv, const string& tag) {
        m_engravers2[tag] = pEngrv;
    }
    Engraver* get_engraver(ImoObj* pImo);
    Engraver* get_engraver(const string& tag);
    inline void remove_engraver(ImoObj* pImo) { m_engravers.erase(pImo); }
    inline void remove_engraver(const string& tag) { m_engravers2.erase(tag); }

    //suppor for debug and unit tests
    void delete_engravers();

};


}   //namespace lomse

#endif    // __LOMSE_ENGRAVERS_MAP_H__

