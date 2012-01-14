//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_DYN_GENERATOR_H__
#define __LOMSE_DYN_GENERATOR_H__

#include "lomse_control.h"

namespace lomse
{

//forward declarations
class ImoDynamic;
class ImoDocument;


//---------------------------------------------------------------------------------------
// Abstract class from which all dynamic content generators must derive
class DynGenerator
{
protected:
    long m_dynId;
    list<Control*> m_controls;

    DynGenerator(long dynId) : m_dynId(dynId) {}

public:
    virtual ~DynGenerator() {
        list<Control*>::iterator it;
        for (it = m_controls.begin(); it != m_controls.end(); ++it)
            delete *it;
        m_controls.clear();
    }

    virtual void generate_content(ImoDynamic* pDyn, Document* pDoc) = 0;
    virtual long get_dynid() { return m_dynId; }

    void accept_control_ownership(Control* control) {
        m_controls.push_back(control);
    }
    void delete_control(Control* control) {
        list<Control*>::iterator it;
        for (it = m_controls.begin(); it != m_controls.end(); ++it)
        {
            if (*it == control)
            {
                delete *it;
                break;
            }
        }
    }

};


}   //namespace lomse

#endif      //__LOMSE_DYN_GENERATOR_H__
