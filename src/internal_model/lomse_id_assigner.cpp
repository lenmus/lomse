//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_id_assigner.h"

#include "lomse_internal_model.h"
#include "lomse_control.h"
#include "lomse_visitor.h"

#include <algorithm>        //for max, min
#include <sstream>
using namespace std;

namespace lomse
{


//=======================================================================================
// IdAssigner implementation
//=======================================================================================
void IdAssigner::reset()
{
    m_idToImo.clear();
    m_idToXmlId.clear();
    m_xmlIdToId.clear();
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
void IdAssigner::set_control_id(ImoId id, Control* pControl)
{
    if (id != k_no_imoid)
        m_idToControl[id] = pControl;
}

//---------------------------------------------------------------------------------------
void IdAssigner::remove(ImoObj* pImo)
{
    ImoId id = pImo->get_id();
    if (id != k_no_imoid)
    {
        m_idToImo.erase(id);        //this assumes that id exists; crash otherwise
        string xmlId = get_xml_id_for(id);
        if (!xmlId.empty())
            m_xmlIdToId.erase(xmlId);
        m_idToXmlId.erase(id);
        pImo->set_id(k_no_imoid);
    }
}

//---------------------------------------------------------------------------------------
string IdAssigner::get_xml_id_for(ImoId id)
{
    if (id != k_no_imoid)
    {
        unordered_map<ImoId, string>::const_iterator it = m_idToXmlId.find( id );
        if (it != m_idToXmlId.end())
            return it->second;
    }
    return "";
}

//---------------------------------------------------------------------------------------
void IdAssigner::set_xml_id_for(ImoId id, const string& xmlId)
{
    if (id != k_no_imoid)
    {
        m_idToXmlId[id] = xmlId;
        m_xmlIdToId[xmlId] = id;
    }
}

//---------------------------------------------------------------------------------------
ImoObj* IdAssigner::get_pointer_to_imo(ImoId id) const
{
	unordered_map<ImoId, ImoObj*>::const_iterator it = m_idToImo.find( id );
	if (it != m_idToImo.end())
		return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoObj* IdAssigner::get_pointer_to_imo(const string& xmlId) const
{
	map<std::string, ImoId>::const_iterator it = m_xmlIdToId.find( xmlId );
	if (it != m_xmlIdToId.end())
        return get_pointer_to_imo(it->second);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
Control* IdAssigner::get_pointer_to_control(ImoId id) const
{
	unordered_map<ImoId, Control*>::const_iterator it = m_idToControl.find( id );
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
	unordered_map<ImoId, ImoObj*>::const_iterator it;
	for (it = m_idToImo.begin(); it != m_idToImo.end(); ++it)
		data << it->first << "-" << it->second->get_name() << endl;
    data << endl;

	unordered_map<ImoId, Control*>::const_iterator itC = m_idToControl.begin();
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
	unordered_map<ImoId, ImoObj*>::const_iterator it;
	for (it = m_idToImo.begin(); it != m_idToImo.end(); ++it)
    {
        if (it->first >= idMin)
            assigner->add_id(it->first, it->second);
    }

	unordered_map<ImoId, Control*>::const_iterator itC;
	for (itC = m_idToControl.begin(); itC != m_idToControl.end(); ++itC)
        assigner->add_control_id(itC->first, itC->second);

	unordered_map<ImoId, string>::const_iterator itS;
	for (itS = m_idToXmlId.begin(); itS != m_idToXmlId.end(); ++itS)
        assigner->set_xml_id_for(itS->first, itS->second);
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

//---------------------------------------------------------------------------------------
void IdAssigner::copy_strings_from(IdAssigner* pIdAssigner)
{
    //copy strings from original IdAssigner
	unordered_map<ImoId, string>::const_iterator itS;
	for (itS = pIdAssigner->m_idToXmlId.begin(); itS != pIdAssigner->m_idToXmlId.end(); ++itS)
        set_xml_id_for(itS->first, itS->second);
}

//---------------------------------------------------------------------------------------
bool IdAssigner::check_ids(IdAssigner* pCopy, stringstream& reporter, const string& label)
{
    //for debug purposes, to check that both assigners contain the same ids
    //Returns:
    //  true if both are equal; in this case nothing has been logged in 'reporter'.
    //  false - there are differences, and have been logged in 'reporter'

    bool fOK = true;
    if (label == "copy" && this->size() != pCopy->size())
    {
        fOK = false;
        reporter << "    IdAssigners have different sizes: original = " << this->size()
            << ", copy = " << pCopy->size() << endl;
    }

	unordered_map<ImoId, ImoObj*>::const_iterator it;
	for (it = m_idToImo.begin(); it != m_idToImo.end(); ++it)
    {
        if (pCopy->get_pointer_to_imo(it->first) == nullptr)
        {
            fOK = false;
            reporter << "        Imo id " << it->first << " missing in " << label << ": "
                << (it->second)->get_name() << endl;
        }
    }

	unordered_map<ImoId, Control*>::const_iterator itC;
	for (itC = m_idToControl.begin(); itC != m_idToControl.end(); ++itC)
    {
        if (pCopy->get_pointer_to_control(itC->first) == nullptr)
        {
            fOK = false;
            reporter << "    Control id " << itC->first << " missing in " << label << ". ";
            ImoControl* pImo = (itC->second)->get_owner_imo();
            if (pImo)
            {
                reporter << "parent Imo is " << pImo->get_name() << " id= "
                    << pImo->get_id();
            }
            reporter << endl;
        }
    }

    return fOK;
}


}  //namespace lomse
