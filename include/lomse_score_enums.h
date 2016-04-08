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

