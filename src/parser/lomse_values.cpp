//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include "lomse_values.h"

#include <iostream>

using namespace std;

namespace lomse
{

/// Returns true if the string is not a valid ldp duration representation.
/// Duration is represented by a letter for the type of note (see table) and,
/// optionally, any number of dots ".".
///
///  USA           UK                      ESP               LDP     NoteType
///  -----------   --------------------    -------------     ---     ---------
///  long          longa                   longa             l       eLonga = 0
///  double whole  breve                   cuadrada, breve   d       eBreve = 1
///  whole         semibreve               redonda           w       eWhole = 2
///  half          minim                   blanca            h       eHalf = 3
///  quarter       crochet                 negra             q       eQuarter = 4
///  eighth        quaver                  corchea           e       eEighth = 5
///  sixteenth     semiquaver              semicorchea       s       e16th = 6
///  32nd          demisemiquaver          fusa              t       e32th = 7
///  64th          hemidemisemiquaver      semifusa          i       e64th = 8
///  128th         semihemidemisemiquaver  garrapatea        o       e128th = 9
///  256th         ???                     semigarrapatea    f       e256th = 10
///
bool LdpValues::CheckDuration(const std::string& str)
{

    //check duration
    std::string valid = "ldwhqestiof";
    if (valid.find(str[0]) == string::npos)
        return true;    //error

    //check dots
    for (size_t i = 1; i < str.size(); i++)
        if (str[i] != '.')
            return true;    //error

    return false;       //no error
}

/// Returns true is the string is not a valid ldp pitch representation.
/// Pitch is represented by an string combining accidentals, step and octave.
/// Accidentals are represented by using signs before the note name, as follows:
///     -  : flat
///     -- : double flat
///     +  : sharp
///     ++ : sharp-sharp (two sharps)
///     x  : double sharp
///     =  : natural
///     =- : natural-flat
///     =+ : natural-sharp
/// Step is represented by the English note name in lower case letter (a,b,c,d,e,f,g).
/// Octave is represented by a digit 0..9
///
/// examples:  a4, ++c3, b2, =+c3, +c3, =c4, -d3, --c3, =-e5, xf3
///
bool LdpValues::CheckPitch(const std::string& str)
{
    //split the string: accidentals, step and octave
    std::string accidentals;
    int iPos;
    switch (str.size())
    {
        case 2:
            accidentals = "";
            iPos = 0;
            break;
        case 3:
            accidentals = str.substr(0, 1);
            iPos = 1;
            break;
        case 4:
            accidentals = str.substr(0, 2);
            iPos = 2;
            break;
        default:
            return true;   //error
    }
    const char& step = str[iPos];
    const char& octave = str[iPos+1];

    //check step
    std::string steps = "abcdefg";
    if (steps.find(step) == string::npos)
        return true;        //error

    //check octave
    if (octave < '0' || octave > '9')
        return true;    //error

    //check accidentals
    return !(accidentals == ""
             || accidentals == "+"
             || accidentals == "++"
             || accidentals == "x"
             || accidentals == "-"
             || accidentals == "--"
             || accidentals == "="
             || accidentals == "=+"
             || accidentals == "=-"
            );
}



}   //namespace lomse
