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

#include "lomse_ldp_elements.h"

#include <sstream>
#include "lomse_internal_model.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
LdpElement::LdpElement()
    : m_type(k_undefined)
    , m_name("")
    , m_fSimple(false)
    , m_numLine(0)
    , m_id(k_no_imoid)
    , m_pImo(nullptr)
    //, m_fProcessed(false)
{
}

//---------------------------------------------------------------------------------------
LdpElement::~LdpElement()
{
    TreeNode<LdpElement>::children_iterator it(this);
    it = begin();
    while (it != end())
    {
        LdpElement* child = *it;
        ++it;
	    delete child;
    }
}

//---------------------------------------------------------------------------------------
void LdpElement::accept_visitor(BaseVisitor& v)
{
	Visitor<LdpElement>* p = dynamic_cast<Visitor<LdpElement>*>(&v);
	if (p)
		p->start_visit(this);
}

//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
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
            TreeNode<LdpElement>::children_iterator it(this);
            for (it = begin(); it != end(); ++it)
	            s << " " << (*it)->to_string();
        }
        else
            s << " " << get_ldp_value();

        s << ")";
    }
    return s.str();
}

//---------------------------------------------------------------------------------------
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
            TreeNode<LdpElement>::children_iterator it(this);
            for (it = begin(); it != end(); ++it)
	            s << " " << (*it)->to_string_with_ids();
        }
        else
            s << " " << get_ldp_value();

        s << ")";
    }
    return s.str();
}

//---------------------------------------------------------------------------------------
int LdpElement::get_num_parameters()
{
    return get_num_children();
}

//---------------------------------------------------------------------------------------
LdpElement* LdpElement::get_parameter(int i)
{
    // i = 1..n
    //assert( i > 0 && i <= get_num_parameters());
    TreeNode<LdpElement>::children_iterator it(this);
    int numChild = 1;
    for (it=this->begin(); it != this->end() && numChild < i; ++it, ++numChild);
    if (it != this->end() && i == numChild)
        return *it;
    else
    {
        LOMSE_LOG_ERROR("[LdpElement::get_parameter]. Num child greater than available children" );
        throw runtime_error("[LdpElement::get_parameter]. Num child greater than available children" );
    }
}

//---------------------------------------------------------------------------------------
float LdpElement::get_value_as_float()
{
    float rValue;
    std::istringstream iss(m_value);
    if ((iss >> std::dec >> rValue).fail())
    {
        LOMSE_LOG_ERROR("[LdpElement::get_value_as_float]. Invalid conversion to number" );
        throw runtime_error("[LdpElement::get_value_as_float]. Invalid conversion to number" );
    }
    return rValue;
}

//---------------------------------------------------------------------------------------
void LdpElement::set_imo(ImoObj* pImo)
{
    m_pImo = pImo;
    //if (pImo)
    //    pImo->set_id(this->get_id());
}


}   //namespace lomse
