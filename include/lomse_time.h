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

#ifndef __LOMSE_TIME_H__
#define __LOMSE_TIME_H__

#include "lomse_basic.h"

#include <chrono>

namespace lomse
{
//some constants
#define LOMSE_NO_DURATION   100000000000000.0f  //any too high value for a note duration
#define LOMSE_NO_TIME       100000000000000.0f  //any impossible high value for a timepos

//helper functions to compare times (two floating point numbers)

extern bool is_equal_time(TimeUnits t1, TimeUnits t2);
extern bool is_lower_time(TimeUnits t1, TimeUnits t2);
extern bool is_greater_time(TimeUnits t1, TimeUnits t2);

#define is_higher_time  is_greater_time

//helper function to implement round-half-up algorithm

TimeUnits round_half_up(TimeUnits num);

string to_simple_string(chrono::time_point<chrono::system_clock> time, bool microsec = false);

}   //namespace lomse

#endif      //__LOMSE_TIME_H__
