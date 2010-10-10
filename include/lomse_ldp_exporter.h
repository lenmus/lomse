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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE__LDP_EXPORTER_H__        //to avoid nested includes
#define __LOMSE__LDP_EXPORTER_H__

#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class LdpGenerator;
struct rgba16;


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
    static std::string color_to_ldp(rgba16& color);
    static std::string float_to_string(float num);

protected:
    LdpGenerator* new_generator(ImoObj* pImo);

};


}   //namespace lomse

#endif    // __LOMSE__LDP_EXPORTER_H__

