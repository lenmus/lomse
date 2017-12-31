//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

///@cond INTERNAL
namespace lomse
{
///@endcond


//-----------------------------------------------------------------------------
//attributes
/** @ingroup enumerations

    This enum describes values for valid attributes for CmdChangeAttribute
    commands, as well as valid attribute indexes for ImoAttr.

    @#include <lomse_im_attributes.h>
*/
enum EImoAttribute
{
    k_attr_barline=0,               ///< Barline type
    k_attr_color,                   ///< Color value
    k_attr_dots,                    ///< Number of dots (dotted notes & rests)
    k_attr_notated_accidentals,     ///< Notated accidentals for notes
    k_attr_note_type,               ///< Note type
    k_attr_octave,                  ///< Octave (note pitch)
    k_attr_staff_num,               ///< Number of staff
    k_attr_stem_type,               ///< Stem type
    k_attr_step,                    ///< Step (note pitch)
    k_attr_style,                   ///< Style
    k_attr_time_modifier_bottom,    ///< Value for time modifier, bottom number (tuplets)
    k_attr_time_modifier_top,       ///< Value for time modifier, top number (tuplets)
    k_attr_visible,                 ///< Visible (display or hide the element)
    k_attr_voice,                   ///< Voice (for notes & rests)
    k_attr_xpos,                    ///< X position (to place an element)
    k_attr_ypos,                    ///< Y position (to place an element)

    k_attr_coda,                //string
    k_attr_dacapo,              //bool
    k_attr_dalsegno,            //string
    k_attr_dynamics,            //float
    k_attr_fine,                //bool
    k_attr_forward_repeat,      //bool
    k_attr_pizzicato,           //bool
    k_attr_segno,               //string
    k_attr_tempo,               //float
    k_attr_time_only,           //string
    k_attr_tocoda,              //string
};

//data types
/** @ingroup enumerations

    This enum describes data type for an attribute.

    @#include <lomse_im_attributes.h>
*/
enum EDataType
{
    k_type_bool=0,      ///< Boolean
    k_type_color,       ///< Color
    k_type_double,      ///< Float number, double precission
    k_type_int,         ///< Integer number
    k_type_string,      ///< String of characters
};

///@cond INTERNAL
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

///@endcond

}   //namespace lomse

#endif    // __LOMSE_IM_ATTRIBUTES_H__

