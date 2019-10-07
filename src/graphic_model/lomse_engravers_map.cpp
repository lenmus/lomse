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
