//-------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------

#ifndef __LOMSE_SCORE_ENUMS_H__        //to avoid nested includes
#define __LOMSE_SCORE_ENUMS_H__

namespace lomse
{


// enums for common values: aligment, justification, placement, etc.
//---------------------------------------------------------------------------------------

//line styles
enum ELineStyle { k_line_none=0, k_line_solid, k_line_long_dash,
                  k_line_short_dash, k_line_dot, k_line_dot_dash, };

//line termination styles
enum ELineCap { k_cap_none = 0, k_cap_arrowhead, k_cap_arrowtail,
                k_cap_circle, k_cap_square, k_cap_diamond, };

//line edges
enum ELineEdge {
    k_edge_normal = 0,        // edge is perpendicular to line
    k_edge_vertical,          // edge is always a vertical line
    k_edge_horizontal         // edge is always a horizontal line
};

//---------------------------------------------------------------------------------------
//EVAlign is used to indicate vertical alignment within a block: to the top,
//middle or bottom
enum EVAlign
{
    k_valign_default = 0,   //alignment is not specified
    k_valign_top,
    k_valign_middle,
    k_valign_bottom,
};

//---------------------------------------------------------------------------------------
// EHAlign is used to indicate object justification
enum EHAlign
{
    k_halign_default = 0,   //alignment is not specified
    k_halign_left,          //object aligned on left side
    k_halign_right,         //object aligned on right side
    k_halign_justify,       //object justified on both sides
    k_halign_center,        //object centered
};

//---------------------------------------------------------------------------------------
// EPlacement indicates whether something is above or below another element,
// such as a note or a notation.
enum EPlacement
{
    k_placement_default = 0,
    k_placement_above,
    k_placement_below
};

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




}   //namespace lomse

#endif    // __LOMSE_SCORE_ENUMS_H__

