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

#include "lomse_interval.h"

#include <sstream>
using namespace std;


namespace lomse
{

//=======================================================================================
// Implementation of FIntval
//=======================================================================================

//interval short names. The index in this array is the FIntval inernal value
static string m_sFIntvalCode[41] = {
    "p1",  "a1",  "da1", "dd2", "d2", "m2",  "M2",  "a2",
    "da2", "dd3", "d3",  "m3",  "M3", "a3",  "da3", "dd4",
    "d4",  "p4",  "a4",  "da4", "--", "dd5", "d5",  "p5",
    "a5",  "da5", "dd6", "d6",  "m6", "M6",  "a6",  "da6",
    "dd7", "d7",  "m7",  "M7",  "a7", "da7", "dd8", "d8",
    "p8"
};

// an entry for the table of intervals data
typedef struct {
    EIntervalType   type;
    int             numIntv;
    int             numSemitones;
} IntervalData;

static const IntervalData m_intvlData[43] =
{
    /*                          type                num   semitones */
    {/*  0 - k_interval_p1  */  k_perfect,            1,    0 },
    {/*  1 - k_interval_a1  */  k_augmented,          1,    1 },
    {/*  2 - k_interval_da1 */  k_double_augmented,   1,    2 },
    {/*  3 - k_interval_dd2 */  k_double_diminished,  2,   -1 },
    {/*  4 - k_interval_d2  */  k_diminished,         2,    0 },
    {/*  5 - k_interval_m2  */  k_minor,              2,    1 },
    {/*  6 - k_interval_M2  */  k_major,              2,    2 },
    {/*  7 - k_interval_a2  */  k_augmented,          2,    3 },
    {/*  8 - k_interval_da2 */  k_double_augmented,   2,    4 },
    {/*  9 - k_interval_dd3 */  k_double_diminished,  3,    1 },
    {/* 10 - k_interval_d3  */  k_diminished,         3,    2 },
    {/* 11 - k_interval_m3  */  k_minor,              3,    3 },
    {/* 12 - k_interval_M3  */  k_major,              3,    4 },
    {/* 13 - k_interval_a3  */  k_augmented,          3,    5 },
    {/* 14 - k_interval_da3 */  k_double_augmented,   3,    6 },
    {/* 15 - k_interval_dd4 */  k_double_diminished,  4,    3 },
    {/* 16 - k_interval_d4  */  k_diminished,         4,    4 },
    {/* 17 - k_interval_p4  */  k_perfect,            4,    5 },
    {/* 18 - k_interval_a4  */  k_augmented,          4,    6 },
    {/* 19 - k_interval_da4 */  k_double_augmented,   4,    7 },
    {/*empty*/                  (EIntervalType)0,     0,    0 },
    {/* 21 - k_interval_dd5 */  k_double_diminished,  5,    5 },
    {/* 22 - k_interval_d5  */  k_diminished,         5,    6 },
    {/* 23 - k_interval_p5  */  k_perfect,            5,    7 },
    {/* 24 - k_interval_a5  */  k_augmented,          5,    8 },
    {/* 25 - k_interval_da5 */  k_double_augmented,   5,    9 },
    {/* 26 - k_interval_dd6 */  k_double_diminished,  6,    6 },
    {/* 27 - k_interval_d6  */  k_diminished,         6,    7 },
    {/* 28 - k_interval_m6  */  k_minor,              6,    8 },
    {/* 29 - k_interval_M6  */  k_major,              6,    9 },
    {/* 30 - k_interval_a6  */  k_augmented,          6,   10 },
    {/* 31 - k_interval_da6 */  k_double_augmented,   6,   11 },
    {/* 32 - k_interval_dd7 */  k_double_diminished,  7,    8 },
    {/* 33 - k_interval_d7  */  k_diminished,         7,    9 },
    {/* 34 - k_interval_m7  */  k_minor,              7,   10 },
    {/* 35 - k_interval_M7  */  k_major,              7,   11 },
    {/* 36 - k_interval_a7  */  k_augmented,          7,   12 },
    {/* 37 - k_interval_da7 */  k_double_augmented,   7,   13 },
    {/* 38 - k_interval_dd8 */  k_double_diminished,  8,   10 },
    {/* 39 - k_interval_d8  */  k_diminished,         8,   11 },
    {/* 40 - k_interval_p8  */  k_perfect,            8,   12 },
    {/* 41 - k_interval_a8  */  k_augmented,          8,   13 },
    {/* 42 - k_interval_da8 */  k_double_augmented,   8,   14 },
};

//---------------------------------------------------------------------------------------
FIntval::FIntval()
{
    m_interval = k_interval_null;
}

//---------------------------------------------------------------------------------------
FIntval::FIntval(const string& code, bool fDescending)
{
    // unison
    if (code == "p1") m_interval = k_interval_p1;
    else if (code == "a1") m_interval = k_interval_a1;
    else if (code == "da1") m_interval = k_interval_da1;
    // second
    else if (code == "dd2") m_interval = k_interval_dd2;
    else if (code == "d2") m_interval = k_interval_d2;
    else if (code == "m2") m_interval = k_interval_m2;
    else if (code == "M2") m_interval = k_interval_M2;
    else if (code == "a2") m_interval = k_interval_a2;
    else if (code == "da2") m_interval = k_interval_da2;
    // third
    else if (code == "dd3") m_interval = k_interval_dd3;
    else if (code == "d3") m_interval = k_interval_d3;
    else if (code == "m3") m_interval = k_interval_m3;
    else if (code == "M3") m_interval = k_interval_M3;
    else if (code == "a3") m_interval = k_interval_a3;
    else if (code == "da3") m_interval = k_interval_da3;
    // fourth
    else if (code == "dd4") m_interval = k_interval_dd4;
    else if (code == "d4") m_interval = k_interval_d4;
    else if (code == "p4") m_interval = k_interval_p4;
    else if (code == "a4") m_interval = k_interval_a4;
    else if (code == "da4") m_interval = k_interval_da4;
    // fifth
    else if (code == "dd5") m_interval = k_interval_dd5;
    else if (code == "d5") m_interval = k_interval_d5;
    else if (code == "p5") m_interval = k_interval_p5;
    else if (code == "a5") m_interval = k_interval_a5;
    else if (code == "da5") m_interval = k_interval_da5;
    //sixth
    else if (code == "dd6") m_interval = k_interval_dd6;
    else if (code == "d6") m_interval = k_interval_d6;
    else if (code == "m6") m_interval = k_interval_m6;
    else if (code == "M6") m_interval = k_interval_M6;
    else if (code == "a6") m_interval = k_interval_a6;
    else if (code == "da6") m_interval = k_interval_da6;
    // seventh
    else if (code == "dd7") m_interval = k_interval_dd7;
    else if (code == "d7") m_interval = k_interval_d7;
    else if (code == "m7") m_interval = k_interval_m7;
    else if (code == "M7") m_interval = k_interval_M7;
    else if (code == "a7") m_interval = k_interval_a7;
    else if (code == "da7") m_interval = k_interval_da7;
    // octave
    else if (code == "dd8") m_interval = k_interval_dd8;
    else if (code == "d8") m_interval = k_interval_d8;
    else if (code == "p8") m_interval = k_interval_p8;
    else if (code == "a8") m_interval = k_interval_a8;
    else if (code == "da8") m_interval = k_interval_da8;
    else
        m_interval = k_interval_null;

    if (fDescending)
        make_descending();
}

//---------------------------------------------------------------------------------------
FIntval::FIntval (int number, EIntervalType type)
{
    int num = abs(number);
    int octaves = (num - 1) / 7;     // 0 .. n
    num = ((num-1) % 7) + 1;          // 1 .. 8

    int i;
    for (i=0; i < 43; i++)
    {
        if (m_intvlData[i].numIntv == num && m_intvlData[i].type == type)
            break;
    }

    if (i < 43)
    {
        m_interval = i + octaves * 40;
        if (number < 0)
            m_interval = -m_interval;
    }
    else
        m_interval = k_interval_null;    //invalid interval
}

//---------------------------------------------------------------------------------------
int FIntval::get_number()
{
    int interval = abs(m_interval);
    int octaves = (interval / 40);    //octaves = 0..n
    int num = interval % 40;          //num = 0..39

    return m_intvlData[num].numIntv + octaves * 7;
}

//---------------------------------------------------------------------------------------
string FIntval::get_code()
{
    stringstream ss;
    switch (get_type())
    {
        case k_diminished:        ss << "d";    break;
        case k_minor:             ss << "m";    break;
        case k_major:             ss << "M";    break;
        case k_augmented:         ss << "a";    break;
        case k_perfect:           ss << "p";    break;
        case k_double_augmented:  ss << "da";   break;
        case k_double_diminished: ss << "dd";   break;
        default:
            return "invalid!";
    }

    ss << get_number();
    return ss.str();
}

//---------------------------------------------------------------------------------------
EIntervalType FIntval::get_type()
{
    int interval = abs(m_interval);
    int num = interval % 40;   //num = 0..39
    return m_intvlData[num].type;
}

//---------------------------------------------------------------------------------------
int FIntval::get_num_semitones()
{
    int interval = abs(m_interval);
    int num = interval % 40;   //num = 0..39
    int octaves = (interval / 40) * 12;
    return m_intvlData[num].numSemitones + octaves;
}


}   //namespace lomse
