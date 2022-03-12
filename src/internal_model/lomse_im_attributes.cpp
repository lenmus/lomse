//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

//lomse
#include "lomse_im_attributes.h"

#include "lomse_internal_model.h"
#include "lomse_logger.h"


namespace lomse
{


//=======================================================================================
// AttributesTable implementation
//=======================================================================================
AttributesData AttributesTable::get_data_for(int attrb)
{
    switch(attrb)
    {
        // ImoBarline
        case k_attr_barline:
            return AttributesData(k_attr_barline, k_type_int,
                                  "Barline type", "Barline type (i.e simple)",
                                  "", k_barline_simple, k_max_barline-1 );
        // ImoNote
        case k_attr_step:
            return AttributesData(k_attr_step, k_type_int,
                                  "Note step", "Note step (0..6)" );
        case k_attr_octave:
            return AttributesData(k_attr_octave, k_type_int,
                                  "Octave", "Octave (0..9)", "", 0, 9 );
        case k_attr_notated_accidentals:
            return AttributesData(k_attr_notated_accidentals, k_type_int,
                                  "Accidentals", "Notated accidentals" );
        case k_attr_stem_type:
            return AttributesData(k_attr_stem_type, k_type_int,
                                  "Stem type", "Stem type (i.e up, down, double, none, ...)",
                                  "", 0, 4 );
        // ImoNoteRest
        case k_attr_note_type:
            return AttributesData(k_attr_note_type, k_type_int,
                                  "Note type", "Note type (i.e quarter, eighth, ...)" );
        case k_attr_dots:
            return AttributesData(k_attr_dots, k_type_int,
                                  "Dots", "Number of dots)", "", 0, 4 );
        case k_attr_voice:
            return AttributesData(k_attr_voice, k_type_int,
                                  "Voice", "Voice (0..n)" );
        case k_attr_time_modifier_top:
            return AttributesData(k_attr_time_modifier_top, k_type_int,
                                  "Time modifier top", "Time modifier top" );
        case k_attr_time_modifier_bottom:
            return AttributesData(k_attr_time_modifier_bottom, k_type_int,
                                  "Time modifier bottom", "Time modifier bottom" );
    // ImoStaffObj
        case k_attr_staff_num:
            return AttributesData(k_attr_staff_num, k_type_int,
                                  "Staff number", "Number of staff (1..n)" );
    //ImoScoreObj
        case k_attr_color:
            return AttributesData(k_attr_color, k_type_color,
                                  "Color", "Object color" );
    //ImoContentObj
        case k_attr_style:
            return AttributesData(k_attr_style, k_type_string,
                                  "Style name", "Name of existing style" );
        case k_attr_xpos:
            return AttributesData(k_attr_xpos, k_type_double,
                                  "x position", "User position (tenths)", "Tenths", 0, 0 );
        case k_attr_ypos:
            return AttributesData(k_attr_ypos, k_type_double,
                                 "y position", "User position (tenths)", "Tenths", 0, 0 );
        case k_attr_visible:
            return AttributesData(k_attr_visible, k_type_bool,
                                  "Visible", "Object visibility" );
        default:
            //LOMSE_LOG_ERROR();
            return AttributesData(-1, k_type_bool, "", "");
    }
}


}  //namespace lomse
