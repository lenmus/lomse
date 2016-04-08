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

#ifndef __LOMSE_LDP_FACTORY_H__
#define __LOMSE_LDP_FACTORY_H__

#include <string>
#include <map>

#include "lomse_build_options.h"
#include "lomse_functor.h"
#include "lomse_ldp_elements.h"

namespace lomse
{

//forward declarations
class LdpElement;
class LdpFunctor;

//---------------------------------------------------------------------------------------
// A factory to create Ldp elements. Should have library scope
class LOMSE_EXPORT LdpFactory
{
protected:
	std::map<std::string, LdpFunctor*> m_NameToFunctor;
	std::map<ELdpElement, std::string>	m_TypeToName;

public:
    LdpFactory();
	virtual ~LdpFactory();

	LdpElement* create(const std::string& name, int numLine=0) const;
	LdpElement* create(ELdpElement type, int numLine=0) const;

    const std::string& get_name(ELdpElement type) const;

    //utility methods
    LdpElement* new_element(ELdpElement type, LdpElement* value, int UNUSED(numLine) =0)
    {
	    LdpElement* elm = create(type);
	    elm->append_child(value);
	    return elm;
    }

    LdpElement* new_value(ELdpElement type, const std::string& value, int numLine=0)
    {
	    LdpElement* elm = create(type, numLine);
        elm->set_simple();
	    elm->set_value(value);
	    return elm;
    }

    LdpElement* new_label(const std::string& value, int numLine=0) {
        return new_value(k_label, value, numLine);
    }

    LdpElement* new_string(const std::string& value, int numLine=0) {
        return new_value(k_string, value, numLine);
    }

    LdpElement* new_number(const std::string& value, int numLine=0) {
        return new_value(k_number, value, numLine);
    }

};

}   //namespace lomse

#endif      //__LOMSE_LDP_FACTORY_H__
