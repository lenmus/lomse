//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

