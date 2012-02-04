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

#ifndef __LOMSE__LDP_EXPORTER_H__        //to avoid nested includes
#define __LOMSE__LDP_EXPORTER_H__

#include "lomse_basic.h"

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class LdpGenerator;
//struct Color;


// LdpExporter: Generates LDP source code for a basic model object
//----------------------------------------------------------------------------------
class LdpExporter
{
protected:
    int m_nIndent;
    bool m_fAddId;

public:
    LdpExporter();
    virtual ~LdpExporter();

    //settings
    void set_indent(int value) { m_nIndent = value; }
    void set_add_id(bool value) { m_fAddId = value; }

    //getters for settings
    inline int get_indent() { return m_nIndent; }
    inline bool get_add_id() { return m_fAddId; }

    //the main method
    std::string get_source(ImoObj* pImo);

    //static methods for ldp names to types conversion
    static std::string clef_type_to_ldp(int clefType);
    static std::string color_to_ldp(Color color);
    static std::string float_to_string(float num);

protected:
    LdpGenerator* new_generator(ImoObj* pImo);

};


}   //namespace lomse

#endif    // __LOMSE__LDP_EXPORTER_H__

