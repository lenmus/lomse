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
    : m_idCounter(k_no_imoid)
{
}

//---------------------------------------------------------------------------------------
void IdAssigner::reset()
{
    m_idToImo.clear();
    m_idCounter = k_no_imoid;
}

//---------------------------------------------------------------------------------------
void IdAssigner::assign_id(ImoObj* pImo)
{
    ImoId id = pImo->get_id();
    if (id == k_no_imoid)
    {
        pImo->set_id(++m_idCounter);
        m_idToImo[m_idCounter] = pImo;
    }
    else
    {
        m_idToImo[id] = pImo;
        m_idCounter = max(id, m_idCounter);
    }
}

//---------------------------------------------------------------------------------------
ImoId IdAssigner::reserve_id(ImoId id)
{
    if (id == k_no_imoid)
    {
        return ++m_idCounter;
    }
    else
    {
        m_idCounter = max(id, m_idCounter);
        return id;
    }
}

//---------------------------------------------------------------------------------------
void IdAssigner::assign_id(Control* pControl)
{
    ImoId id = pControl->get_control_id();
    if (id == k_no_imoid)
        pControl->set_control_id(++m_idCounter);
    else
        m_idCounter = max(id, m_idCounter);

    m_idToControl[m_idCounter] = pControl;
}

//---------------------------------------------------------------------------------------
void IdAssigner::remove(ImoObj* pImo)
{
    ImoId id = pImo->get_id();
    if (id != k_no_imoid)
    {
        m_idToImo.erase(id);
        pImo->set_id(k_no_imoid);
    }
}

//---------------------------------------------------------------------------------------
ImoObj* IdAssigner::get_pointer_to_imo(ImoId id) const
{
	map<ImoId, ImoObj*>::const_iterator it = m_idToImo.find( id );
	if (it != m_idToImo.end())
		return it->second;
    else
      return nullptr;
}

//---------------------------------------------------------------------------------------
Control* IdAssigner::get_pointer_to_control(ImoId id) const
{
	map<ImoId, Control*>::const_iterator it = m_idToControl.find( id );
	if (it != m_idToControl.end())
		return it->second;
    else
      return nullptr;
}

//---------------------------------------------------------------------------------------
string IdAssigner::dump() const
{
    stringstream data;
    data << "Imo: " << endl;
	map<ImoId, ImoObj*>::const_iterator it;
	for (it = m_idToImo.begin(); it != m_idToImo.end(); ++it)
		data << it->first << "-" << it->second->get_name() << endl;
    data << endl;

	map<ImoId, Control*>::const_iterator itC = m_idToControl.begin();
	if (itC != m_idToControl.end())
    {
        data << "Control: " << endl;
        for (; itC != m_idToControl.end(); ++itC)
            data << itC->first << endl;
    }

    return data.str();
}

//---------------------------------------------------------------------------------------
void IdAssigner::copy_ids_to(IdAssigner* assigner, ImoId idMin)
{
	map<ImoId, ImoObj*>::const_iterator it;
	for (it = m_idToImo.begin(); it != m_idToImo.end(); ++it)
    {
        if (it->first >= idMin)
            assigner->add_id(it->first, it->second);
    }

	map<ImoId, Control*>::const_iterator itC;
	for (itC = m_idToControl.begin(); itC != m_idToControl.end(); ++itC)
        assigner->add_control_id(itC->first, itC->second);
}

//---------------------------------------------------------------------------------------
void IdAssigner::add_id(ImoId id, ImoObj* pImo)
{
    m_idToImo[id] = pImo;
}

//---------------------------------------------------------------------------------------
void IdAssigner::add_control_id(ImoId id, Control* pControl)
{
    m_idToControl[id] = pControl;
}


}  //namespace lomse
