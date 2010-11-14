//---------------------------------------------------------------------------------------
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
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ENGRAVING_OPTIONS_H__        //to avoid nested includes
#define __LOMSE_ENGRAVING_OPTIONS_H__

//#include "lomse_basic.h"

namespace lomse
{


//Engraving options (all measures in Tenths, unless otherwise stated)
//---------------------------------------------------------------------------------------

//Instruments / Groups
#define LOMSE_GRP_SPACE_AFTER_NAME 10.0f    //"InstrGroup/Space after name/"
#define LOMSE_GRP_BRACKET_WIDTH 12.5f       //"InstrGroup/Width of bracket/"
#define LOMSE_GRP_BRACKET_GAP 5.0f          //"InstrGroup/Space after bracket/"
#define LOMSE_GRP_BRACE_WIDTH 6.0f          //"InstrGroup/Width of brace/"
#define LOMSE_GRP_BRACE_GAP 5.0f            //"InstrGroup/Space after brace bar/"

//Instruments
#define LOMSE_INSTR_SPACE_AFTER_NAME 10.0f  //"Instr/Space after name/"

//Beams
#define LOMSE_BEAM_THICKNESS 5.0f           //"Beam/Thickness of beam line/"
#define LOMSE_BEAM_SPACING 3.0f             //"Beam/Space between beam lines/"

//System layouter
//    //spacing function parameters
#define LOMSE_DMIN = (float)e32thDuration       //Dmin: min. duration to consider
#define LOMSE_MIN_SPACE   10.0f		            //Smin: space for Dmin
//    //space
#define LOMSE_SPACE_AFTER_PROLOG   25.0f        //The first note in each bar should be about one note-head's width away from the barline.
#define LOMSE_MIN_SPACE_BETWEEN_NOTE_AND_CLEF   10.0f
#define LOMSE_EXCEPTIONAL_MIN_SPACE   2.5f


}   //namespace lomse

#endif    // __LOMSE_ENGRAVING_OPTIONS_H__

