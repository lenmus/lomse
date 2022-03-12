//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SCORE_ENUMS_H__        //to avoid nested includes
#define __LOMSE_SCORE_ENUMS_H__

namespace lomse
{

//---------------------------------------------------------------------------------------
// Spacing methods for rendering scores
//
//    Two basic methods:
//    1. Fixed: the spacing between notes is constant, independently of note duration.
//    2. Proportional: the spacing is adjusted so that note position is proportional
//       to time.
//
//    In the proportional method several alternatives are posible [NOT IMPLEMENTED]:
//    1. Proportional constant: the proportion factor between time and space is fixed.
//       Two alternative methods for fixing this factor:
//          a) Fixed: is given by the vaule of a parameter
//          b) ShortNote: is computed for the shorter note in the score
//    2. Proportional variable: the proportion factor is computed for each bar. Two
//       alternatives:
//          a) ShortNote: is computed for the shorter note in the bar
//          b) NumBars: Computed so taht the number of bars in the system is a
//             predefined number
//
enum ESpacingMethod
{
    k_spacing_fixed = 1,
    k_spacing_proportional,
};

//---------------------------------------------------------------------------------------
// The symbol-size entity, taken from MusicXML, is used to indicate full vs. cue-sized
// vs. oversized symbols, when necessary. For instance, in clefs.
enum ESymbolSize
{
    k_size_default=0,   //no information. Do your best to decide the right size.
    k_size_full,        //normal size
    k_size_cue,         //cue size
    k_size_large,       //oversized symbols
};

//---------------------------------------------------------------------------------------
// Yes/No/Default values for several propperties
enum EYesNo
{
    k_yesno_default=0,  //no information. Use most suitable default value.
    k_yesno_yes,        //yes
    k_yesno_no,         //no
};




}   //namespace lomse

#endif    // __LOMSE_SCORE_ENUMS_H__

