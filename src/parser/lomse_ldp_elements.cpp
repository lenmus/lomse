//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//  
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include "lomse_ldp_elements.h"

#include <sstream>
#include "lomse_internal_model.h"
#include "lomse_basic_objects.h"

using namespace std;

namespace lomse
{

LdpElement::LdpElement()
    : m_type(k_undefined)
    , m_name("")
    , m_fSimple(false)
    , m_numLine(0)
    , m_id(0L)
    , m_pImo(NULL)
    , m_fProcessed(false)
{
}

LdpElement::~LdpElement()
{
    NodeInTree<LdpElement>::children_iterator it(this);
    it = begin();
    while (it != end())
    {
        LdpElement* child = *it;
        ++it;
	    delete child;
    }
}

void LdpElement::accept_in(BaseVisitor& v)
{
	Visitor<LdpElement>* p = dynamic_cast<Visitor<LdpElement>*>(&v);
	if (p)
    {
		p->start_visit(*this);
	}
}

void LdpElement::accept_out(BaseVisitor& v)
{
	Visitor<LdpElement>* p = dynamic_cast<Visitor<LdpElement>*>(&v);
	if (p)
    {
		p->end_visit(*this);
	}
}

bool LdpElement::operator ==(LdpElement& element)
{
	if (get_type() != element.get_type())
        return false;
	if (get_name() != element.get_name())
        return false;
    if (get_num_parameters() != element.get_num_parameters())
        return false;
    for (int i=0; i < get_num_parameters(); i++)
    {
        LdpElement* parmThis = get_parameter(i);
        LdpElement* parmElm = element.get_parameter(i);
	    if (parmThis->to_string() != parmElm->to_string())
            return false;
    }
	return true;
}

std::string LdpElement::get_ldp_value()
{
    if (m_type == k_string)
    {
        stringstream s;
        s << "\"" << m_value << "\"";
        return s.str();
    }
    else
        return m_value;
}

std::string LdpElement::to_string()
{
	stringstream s;
    if (is_simple())
	    s << get_ldp_value();
    else
    {
	    s << "(" << m_name;
        if (has_children())
        {
            NodeInTree<LdpElement>::children_iterator it(this);
            for (it = begin(); it != end(); ++it)
	            s << " " << (*it)->to_string();
        }
        else
            s << " " << get_ldp_value();

        s << ")";
    }
    return s.str();
}

std::string LdpElement::to_string_with_ids()
{
	stringstream s;
    if (is_simple())
	    s << get_ldp_value();
    else
    {
        s << "(" << m_name << "#" << m_id;
        if (has_children())
        {
            NodeInTree<LdpElement>::children_iterator it(this);
            for (it = begin(); it != end(); ++it)
	            s << " " << (*it)->to_string_with_ids();
        }
        else
            s << " " << get_ldp_value();

        s << ")";
    }
    return s.str();
}

int LdpElement::get_num_parameters()
{
    return get_num_children();
}

LdpElement* LdpElement::get_parameter(int i)
{
    // i = 1..n
    //assert( i > 0 && i <= get_num_parameters());
    NodeInTree<LdpElement>::children_iterator it(this);
    int numChild = 1;
    for (it=this->begin(); it != this->end() && numChild < i; ++it, ++numChild);
    if (it != this->end() && i == numChild)
        return *it;
    else
        throw std::runtime_error( "[LdpElement::get_parameter]. Num child greater than available children" );
}

float LdpElement::get_value_as_float()
{
    float rValue;
    std::istringstream iss(m_value);
    if ((iss >> std::dec >> rValue).fail())
        throw std::runtime_error( "[LdpElement::get_value_as_float]. Invalid conversion to number" );
    return rValue;
}

void LdpElement::set_imo(ImoObj* pImo)
{
    m_pImo = pImo;
    //if (pImo)
    //    pImo->set_id(this->get_id());
}



//---------------------------------------------------------------------------
//TO_REMOVE

    //------------------------------------------------------------
    // Compatibility with lmLDPNode
    //------------------------------------------------------------

LdpElement* LdpElement::GetParameter(int i)
{
    return get_parameter(i);
}

LdpElement* LdpElement::GetParameterFromName(const std::string& name)
{
    NodeInTree<LdpElement>::children_iterator it(this);
    for (it = begin(); it != end(); ++it)
    {
        if ((*it)->get_name() == name)
            return *it;
    }
    return NULL;
}

LdpElement* LdpElement::StartIterator(long iP, bool fOnlyNotProcessed)
{
    //Set initial position
    m_iP = iP;
    if (iP > get_num_parameters())
        return NULL;

    //return object
    if (fOnlyNotProcessed && !get_parameter(m_iP)->IsProcessed())
        return get_parameter(m_iP);
    else
        return GetNextParameter(fOnlyNotProcessed);
}

LdpElement* LdpElement::GetNextParameter(bool fOnlyNotProcessed)
{
    //advance to next one
    ++m_iP;
    while (m_iP <= get_num_parameters())
    {
        if (fOnlyNotProcessed)
        {
            if (!get_parameter(m_iP)->IsProcessed())
                return get_parameter(m_iP);
        }
        else
            return get_parameter(m_iP);

        ++m_iP;
    }

    //no more items or all processed
    if (m_iP > get_num_parameters())
        return NULL;
    return get_parameter(m_iP);
}



//END_TO_REMOVE
//---------------------------------------------------------------------------

// global functions related to elements

bool is_auxobj(int type) { return type >= k_auxobj_start && type <= k_auxobj_end; }
bool is_staffobj(int type) { return type >= k_staffobj_start && type <= k_staffobj_end; }


}   //namespace lomse
