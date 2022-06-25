//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_config.h"
#include "lomse_time.h"

#include <cmath>
#include <chrono>
#include <sstream>
#include <time.h>
#include <iomanip>
using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
//global helper functions to compare times (two floating point numbers)
bool is_equal_time(TimeUnits t1, TimeUnits t2)
{
    return (fabs(t1 - t2) < 0.1);
}

//---------------------------------------------------------------------------------------
bool is_lower_time(TimeUnits t1, TimeUnits t2)
{
    return (t1 < t2) && (fabs(t2 - t1) >= 0.1);
}

//---------------------------------------------------------------------------------------
bool is_greater_time(TimeUnits t1, TimeUnits t2)
{
    return (t1 > t2) && (fabs(t1 - t2) >= 0.1);
}

//---------------------------------------------------------------------------------------
//global function for implementing round-half-up rounding algorithm
TimeUnits round_half_up(TimeUnits num)
{
    return floor(num * 100.0 + 0.5) / 100.0;
}

//---------------------------------------------------------------------------------------
string to_simple_string(chrono::time_point<chrono::system_clock> time, bool microsec)
{
    time_t time_sec = chrono::system_clock::to_time_t(time);
    tm time_tm;

#if (LOMSE_COMPILER_MSVC == 1)
    localtime_s(&time_tm, &time_sec);
#else
    localtime_r(&time_sec, &time_tm);
#endif

    stringstream ss;

    // we could do 'ss << std::put_time(&time_tm, "%Y-%b-%d %X")' but it's not available in GCC until 5.0
    // using 'strftime' instead

    vector<char> buf(100);
    strftime(buf.data(), buf.size(), "%Y-%b-%d %X", &time_tm);
    ss << buf.data();

    if (microsec)
    {
        long long in_micros = chrono::duration_cast<chrono::microseconds>(time.time_since_epoch()).count();
        long long in_seconds = chrono::duration_cast<chrono::seconds>(time.time_since_epoch()).count();
        long long micros = in_micros - in_seconds * 1000000;
        ss << "." << setfill('0') << setw(6) << micros;
    }

    return ss.str();
}


}   //namespace lomse
