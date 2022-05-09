//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

std::string to_simple_string(std::chrono::time_point<std::chrono::system_clock> time,
                             bool microsec = false);

}   //namespace lomse

#endif      //__LOMSE_TIME_H__
