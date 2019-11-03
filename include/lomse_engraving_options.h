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

#ifndef __LOMSE_ENGRAVING_OPTIONS_H__        //to avoid nested includes
#define __LOMSE_ENGRAVING_OPTIONS_H__

namespace lomse
{


//Engraving options (all measures in Tenths, unless otherwise stated)
//---------------------------------------------------------------------------------------

//Barlines
#define LOMSE_THIN_LINE_WIDTH            1.5f   // thin line width
#define LOMSE_THICK_LINE_WIDTH           6.0f   // thick line width
#define LOMSE_LINES_SPACING              4.0f   // space between lines: 4 tenths
#define LOMSE_BARLINE_RADIUS             2.0f   // dots radius: 2 tenths

//Beams
//  according to http://www2.coloradocollege.edu/dept/mu/Musicpress/engraving.html
//  distance between primary and secondary beams should be 1/4 space (2.5 tenths)
//  I'm using 3 tenths (2.5 up rounding).
#define LOMSE_BEAM_THICKNESS             5.0f   //"Beam/Thickness of beam line/"
#define LOMSE_BEAM_SPACING               3.0f   //"Beam/Space between beam lines/"
#define LOMSE_BEAM_HOOK_LENGTH          11.0f

//Instruments
#define LOMSE_INSTR_SPACE_AFTER_NAME    10.0f   //"Instr/Space after name/"

//Instruments / Groups
#define LOMSE_GRP_SPACE_AFTER_NAME      10.0f   //"InstrGroup/Space after name"
#define LOMSE_GRP_BRACKET_WIDTH          5.0f   //"InstrGroup/Width of bracket"
#define LOMSE_GRP_BRACKET_GAP            5.0f   //"InstrGroup/Space after bracket"
#define LOMSE_GRP_BRACKET_HOOK          10.0f   //"InstrGroup/Length of hook for bracket"
#define LOMSE_GRP_BRACE_WIDTH           12.5f   //"InstrGroup/Width of brace"
#define LOMSE_GRP_BRACE_GAP              5.0f   //"InstrGroup/Space after brace"
#define LOMSE_GRP_SQBRACKET_WIDTH        8.0f   //"InstrGroup/Width of squared bracket"
#define LOMSE_GRP_SQBRACKET_LINE         2.0f   //"InstrGroup/line thickness for squared bracket"

//Notes / accidentals
#define LOMSE_STEM_THICKNESS             1.2f
#define LOMSE_SPACE_BEFORE_DOT           5.0f
#define LOMSE_SPACE_AFTER_ACCIDENTALS    1.5f
#define LOMSE_SPACE_BETWEEN_ACCIDENTALS  1.5f
#define LOMSE_LEGER_LINE_OUTGOING        5.0f

//System layouter
    //spacing function parameters
#define LOMSE_MIN_SPACE                 10.0f   //Smin: space for Dmin (tenths)
    //space
#define LOMSE_EXCEPTIONAL_MIN_SPACE      2.5f
#define LOMSE_SPACE_AFTER_BARLINE       14.0f
#define LOMSE_SPACE_AFTER_SMALL_CLEF    10.0f
#define LOMSE_MIN_SPACE_BEFORE_BARLINE  10.0f
    //prolog (opening measures) [Stone80, p.44]
#define LOMSE_SPACE_BEFORE_PROLOG        7.5f
#define LOMSE_PROLOG_GAP_BEORE_KEY      10.0f
#define LOMSE_PROLOG_GAP_BEFORE_TIME    10.0f
#define LOMSE_SPACE_AFTER_PROLOG        15.0f
    //staves distance
#define LOMSE_MIN_SPACING_STAVES        15.0f   //Min. vertical space bitween staves


//tuplets
#define LOMSE_TUPLET_BORDER_LENGHT      10.0f
#define LOMSE_TUPLET_BRACKET_DISTANCE    3.0f
#define LOMSE_TUPLET_BRACKET_THICKNESS   1.0f
#define LOMSE_TUPLET_SPACE_TO_NUMBER     3.0f
#define LOMSE_TUPLET_NESTED_DISTANCE    20.0f

//ties
#define LOMSE_TIE_VERTICAL_SPACE         3.0f   //vertical distance from notehead
#define LOMSE_TIE_MAX_THICKNESS          4.0f   //tie thickness at center

//volta brackets
#define LOMSE_VOLTA_JOG_LENGHT         20.0f    //volta jog lenght
#define LOMSE_VOLTA_BRACKET_DISTANCE   35.0f    //top staff line to volta line distance
#define LOMSE_VOLTA_BRACKET_THICKNESS   1.0f    //volta line thickness
#define LOMSE_VOLTA_LEFT_SPACE_TO_TEXT  4.0f    //distance between jog line and text
#define LOMSE_VOLTA_TOP_SPACE_TO_TEXT   3.0f    //distance between volta line and text

//wedges
#define LOMSE_WEDGE_LINE_THICKNESS      1.5f    //line thickness for wedges/hairpins
#define LOMSE_WEDGE_NIENTE_RADIUS       4.0f    //radius for niente circles in wedges

//octave-shift lines
#define LOMSE_OCTAVE_SHIFT_LINE_THICKNESS  1.0f //thickness for octave-shift lines
#define LOMSE_OCTAVE_SHIFT_SPACE_TO_LINE   2.0f //space from numeral glyph to line start
#define LOMSE_OCTAVE_SHIFT_LINE_SHIFT      3.0f //vertical line shift to compensate glyph baseline

//articulations
#define LOMSE_SPACING_STACKED_ARTICULATIONS  2.0f //space between two stacked articulation marks
#define LOMSE_SPACING_NOTEHEAD_ARTICULATION  5.0f //space between notehead and articulation marks

//lyrics
#define LOMSE_LYRICS_SPACE_TO_MUSIC     12.0f //space between first lyric line and other music notation
#define LOMSE_LYRICS_LINES_EXTRA_SPACE   3.0f //additional space between two lyric lines


}   //namespace lomse

#endif    // __LOMSE_ENGRAVING_OPTIONS_H__

