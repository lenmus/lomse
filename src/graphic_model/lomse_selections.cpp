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

#include "lomse_selections.h"

#include "lomse_gm_basic.h"


namespace lomse
{

//=======================================================================================
// SelectionSet implementation
//=======================================================================================
SelectionSet::SelectionSet()
{
}

//---------------------------------------------------------------------------------------
SelectionSet::~SelectionSet()
{
    m_objects.clear();
}

//---------------------------------------------------------------------------------------
void SelectionSet::add(GmoObj* pGmo, unsigned flags)
{
    m_objects.push_back(pGmo);
    pGmo->set_selected(true);
}

//---------------------------------------------------------------------------------------
bool SelectionSet::contains(GmoObj* pGmo)
{
    std::list<GmoObj*>::iterator it;
    for (it = m_objects.begin(); it != m_objects.end(); ++it)
    {
        if (*it == pGmo)
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void SelectionSet::clear()
{
    std::list<GmoObj*>::iterator it;
    for (it = m_objects.begin(); it != m_objects.end(); ++it)
    {
        (*it)->set_selected(false);
    }
    m_objects.clear();
}



}  //namespace lomse
