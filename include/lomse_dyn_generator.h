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
