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

#ifndef __LOMSE_IM_ATTRIBUTES_H__        //to avoid nested includes
#define __LOMSE_IM_ATTRIBUTES_H__

//lomse
#include "lomse_build_options.h"

//other
#include <string>
#include <list>
using namespace std;


namespace lomse
{


//-----------------------------------------------------------------------------
//attributes
enum EImoAttribute
{
    k_attr_barline=0,
    k_attr_color,
    k_attr_dots,
    k_attr_notated_accidentals,
    k_attr_note_type,
    k_attr_octave,
    k_attr_staff_num,
    k_attr_stem_type,
    k_attr_step,
    k_attr_style,
    k_attr_time_modifier_bottom,
    k_attr_time_modifier_top,
    k_attr_visible,
    k_attr_voice,
    k_attr_xpos,
    k_attr_ypos,
};

//data types
enum EDataType
{
    k_type_bool=0,
    k_type_color,
    k_type_double,
    k_type_int,
    k_type_string,
};


//---------------------------------------------------------------------------------------
// Fully describes an attribute
struct AttributesData
{
    int         attrb;          //referred attribute
    EDataType   type;           //data type
    string      label;          //short name, to be used as lablel in dialogs
    string      description;    //description, for tooltips or other
    string      units;          //units or empty
    int         minimum;        //minimum allowed value (for EDataType == k_type_int)
    int         maximum;        //maximum allowed value (for EDataType == k_type_int)

    AttributesData(int a, EDataType t, const string& lbl,
                   const string& descr)
        : attrb(a)
        , type(t)
        , label(lbl)
        , description(descr)
        , units("")
        , minimum(0)
        , maximum(0)
    {
    }
    AttributesData(int a, EDataType t, const string& lbl,
                   const string& descr, string u, int mn, int mx)
        : attrb(a)
        , type(t)
        , label(lbl)
        , description(descr)
        , units(u)
        , minimum(mn)
        , maximum(mx)
    {
    }

};

//---------------------------------------------------------------------------------------
// provides information about existing attributes
class AttributesTable
{
protected:

public:
    AttributesTable() {}
	~AttributesTable() {}

    static AttributesData get_data_for(int attrb);

};


}   //namespace lomse

#endif    // __LOMSE_IM_ATTRIBUTES_H__

