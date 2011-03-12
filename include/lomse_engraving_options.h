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
#define LOMSE_BARLINE_RADIOUS            2.0f   // dots radius: 2 tenths

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
#define LOMSE_GRP_SPACE_AFTER_NAME      10.0f   //"InstrGroup/Space after name/"
#define LOMSE_GRP_BRACKET_WIDTH         12.5f   //"InstrGroup/Width of bracket/"
#define LOMSE_GRP_BRACKET_GAP            5.0f   //"InstrGroup/Space after bracket/"
#define LOMSE_GRP_BRACE_WIDTH            6.0f   //"InstrGroup/Width of brace/"
#define LOMSE_GRP_BRACE_GAP              5.0f   //"InstrGroup/Space after brace bar/"

//Notes / accidentals
#define LOMSE_STEM_THICKNESS             1.2f
#define LOMSE_SPACE_BEFORE_DOT           5.0f
#define LOMSE_SPACE_AFTER_ACCIDENTALS    1.5f
#define LOMSE_SPACE_BETWEEN_ACCIDENTALS  1.5f
#define LOMSE_LEGER_LINE_OUTGOING        5.0f

//System layouter
    //spacing function parameters
#define LOMSE_DMIN                       8.0f   //TODO: float(e32thDuration)        //Dmin: min. duration to consider
#define LOMSE_MIN_SPACE                 10.0f   //Smin: space for Dmin
    //space
#define LOMSE_SPACE_BEFORE_PROLOG        7.5f
#define LOMSE_SPACE_AFTER_PROLOG        25.0f   //The first note in each bar should be about one note-head's width away from the barline.
#define LOMSE_MIN_SPACE_BETWEEN_NOTE_AND_CLEF   10.0f
#define LOMSE_EXCEPTIONAL_MIN_SPACE      2.5f
#define LOMSE_SPACE_AFTER_BARLINE       20.0f

//tuplets
#define LOMSE_TUPLET_BORDER_LENGHT      10.0f
#define LOMSE_TUPLET_BRACKET_DISTANCE   10.0f
#define LOMSE_TUPLET_NUMBER_DISTANCE     5.0f
#define LOMSE_TUPLET_BRACKET_THICKNESS   2.0f
#define LOMSE_TUPLET_SPACE_TO_NUMBER     3.0f

//ties
#define LOMSE_TIE_VERTICAL_SPACE         3.0f   //vertical distance from notehead
#define LOMSE_TIE_MAX_THICKNESS          4.0f   //tie thickness at center




}   //namespace lomse

#endif    // __LOMSE_ENGRAVING_OPTIONS_H__

