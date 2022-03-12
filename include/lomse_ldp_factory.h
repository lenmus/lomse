//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
