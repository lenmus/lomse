//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#include "lomse_id_assigner.h"

#include "lomse_internal_model.h"
#include "lomse_control.h"

#include <algorithm>        //for max, min
#include <sstream>
using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
IdAssigner::IdAssigner()
    : m_idCounter(-1L)
{
}

//---------------------------------------------------------------------------------------
void IdAssigner::reset()
{
    m_idToImo.clear();
    m_idCounter = -1L;
}

//---------------------------------------------------------------------------------------
void IdAssigner::assign_id(ImoObj* pImo)
{
    long id = pImo->get_id();
    if (id == -1L)
        pImo->set_id(++m_idCounter);
    else
        m_idCounter = max(id, m_idCounter);

    m_idToImo[m_idCounter] = pImo;
}

//---------------------------------------------------------------------------------------
void IdAssigner::assign_id(Control* pControl)
{
    long id = pControl->get_id();
    if (id == -1L)
        pControl->set_id(++m_idCounter);
    else
        m_idCounter = max(id, m_idCounter);

    m_idToControl[m_idCounter] = pControl;
}

//---------------------------------------------------------------------------------------
void IdAssigner::remove(ImoObj* pImo)
{
    long id = pImo->get_id();
    if (id != -1L)
        m_idToImo.erase(id);
}

//---------------------------------------------------------------------------------------
ImoObj* IdAssigner::get_pointer_to_imo(long id) const
{
	map<long, ImoObj*>::const_iterator it = m_idToImo.find( id );
	if (it != m_idToImo.end())
		return it->second;
    else
      return NULL;
}

//---------------------------------------------------------------------------------------
Control* IdAssigner::get_pointer_to_control(long id) const
{
	map<long, Control*>::const_iterator it = m_idToControl.find( id );
	if (it != m_idToControl.end())
		return it->second;
    else
      return NULL;
}

//---------------------------------------------------------------------------------------
string IdAssigner::dump() const
{
    stringstream data;
    data << "Imo: ";
	map<long, ImoObj*>::const_iterator it;
	for (it = m_idToImo.begin(); it != m_idToImo.end(); ++it)
		data << it->first << ", ";

    data << endl << "Control: ";
	map<long, Control*>::const_iterator itC;
	for (itC = m_idToControl.begin(); itC != m_idToControl.end(); ++itC)
		data << itC->first << ", ";

    return data.str();
}


}  //namespace lomse
