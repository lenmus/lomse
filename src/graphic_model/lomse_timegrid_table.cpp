//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_timegrid_table.h"

#include "lomse_time.h"

//std
#include <sstream>
#include <iomanip>
using namespace std;

namespace lomse
{

//=======================================================================================
//TimeGridTable:
//  A table with the relation timepos <=> position for all valid positions to insert
//  a note.
//  This object is responsible for supplying all valid timepos and their positions so
//  that other objects could:
//      a) Determine the timepos to assign to a mouse click in a certain position.
//      b) Draw a grid of valid timepos
//=======================================================================================
TimeGridTable::TimeGridTable()
{
}

//---------------------------------------------------------------------------------------
TimeGridTable::~TimeGridTable()
{
}

//---------------------------------------------------------------------------------------
void TimeGridTable::add_entries(vector<TimeGridTableEntry>& entries)
{
    int iMax = int(entries.size());
    for (int i=0; i < iMax; ++i)
    {
        add_entry(entries[i]);
    }
}

//---------------------------------------------------------------------------------------
void TimeGridTable::add_entry(TimeGridTableEntry& entry)
{
    TimeGridTableEntry tPosTime = {entry.rTimepos, entry.rDuration, entry.uxPos};
    m_PosTimes.push_back(tPosTime);
}

//---------------------------------------------------------------------------------------
string TimeGridTable::dump()
{
    stringstream s;
                //...........+..........+.............
    s << endl << "     timepos        Dur          Pos" << endl;
    vector<TimeGridTableEntry>::iterator it;
    for (it = m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
    {
        s << fixed << setprecision(2) << setfill(' ')
                   << setw(11) << (*it).rTimepos
                   << setw(11) << (*it).rDuration
                   << setw(14) << setprecision(5) << (*it).uxPos
                   << endl;
    }
    return s.str();
}

//---------------------------------------------------------------------------------------
TimeUnits TimeGridTable::get_time_for_position(LUnits uxPos)
{
    //timepos = 0 if table is empty
    if (m_PosTimes.size() == 0)
        return 0.0;

    //timepos = 0 if xPos < first entry xPos
    if (uxPos <= m_PosTimes.front().uxPos)
        return 0.0;

    //otherwise find in table
    std::vector<TimeGridTableEntry>::iterator it = m_PosTimes.begin();
    for (++it; it != m_PosTimes.end(); ++it)
    {
        if (uxPos <= (*it).uxPos)
            return (*it).rTimepos;
    }

    //if not found return last entry timepos
    return m_PosTimes.back().rTimepos;
}

//---------------------------------------------------------------------------------------
LUnits TimeGridTable::get_x_for_note_rest_at_time(TimeUnits timepos)
{
    //xPos = 0 if table is empty or timepos < first entry timepos
    if (m_PosTimes.size() == 0 || is_lower_time(timepos, m_PosTimes.front().rTimepos))
        return 0.0;       //<--------------------------- Test 100

    //otherwise find in table
    vector<TimeGridTableEntry>::iterator it = m_PosTimes.begin();
    TimeUnits prevTimepos = (*it).rTimepos;
    LUnits xPrev = (*it).uxPos;

    for (; it != m_PosTimes.end(); ++it)
    {
        if (is_lower_time(timepos, (*it).rTimepos))
        {
            //interpolate                  //<---------------- Test 104
            double dx = double((*it).uxPos - xPrev) / double((*it).rTimepos - prevTimepos);
            return xPrev + LUnits( double(timepos - prevTimepos) * dx );
        }
        else if (is_equal_time(timepos, (*it).rTimepos))
        {
            if ((*it).rDuration > 0.0)
                return (*it).uxPos;       //<--------------------------- Test 101

            //try next entry
            vector<TimeGridTableEntry>::iterator itNext = it;
            LUnits lastPos = (*it).uxPos;
            ++itNext;
            while (itNext != m_PosTimes.end()
                   && is_equal_time(timepos, (*itNext).rTimepos))
            {
                lastPos = (*itNext).uxPos;
                ++itNext;
            }
            return lastPos;            //<-------------- Tests 102 & T103
        }

        prevTimepos = (*it).rTimepos;
        xPrev = (*it).uxPos;
    }

    //if not found return last entry xPos. Or should return system xRight?
    return m_PosTimes.back().uxPos;       //<--------------------------- Test 105
}

//---------------------------------------------------------------------------------------
LUnits TimeGridTable::get_x_for_barline_at_time(TimeUnits timepos)
{
    //xPos = 0 if table is empty or timepos < first entry timepos
    if (m_PosTimes.size() == 0 || is_lower_time(timepos, m_PosTimes.front().rTimepos))
        return 0.0;       //<--------------------------- Test 200

    //otherwise find in table
    vector<TimeGridTableEntry>::iterator it = m_PosTimes.begin();
    TimeUnits prevTimepos = (*it).rTimepos;
    LUnits xPrev = (*it).uxPos;

    for (; it != m_PosTimes.end(); ++it)
    {
        if (is_equal_time(timepos, (*it).rTimepos))//<--------------- Test 201 (case =)
            return (*it).uxPos;
        else if (is_lower_time(timepos, (*it).rTimepos))//<---------- Test 202 (case <)
        {
            //interpolate
            double dx = double((*it).uxPos - xPrev) / double((*it).rTimepos - prevTimepos);
            return xPrev + LUnits( double(timepos - prevTimepos) * dx );
        }

        prevTimepos = (*it).rTimepos;
        xPrev = (*it).uxPos;
    }

    //if not found return last entry xPos. Or should return system xRight?
    return m_PosTimes.back().uxPos;       //<--------------------------- Test 203
}

//---------------------------------------------------------------------------------------
TimeUnits TimeGridTable::start_time()
{
    //start time == 0 if table is empty
    if (m_PosTimes.size() == 0)
        return 0.0;

    return m_PosTimes.front().rTimepos;
}

//---------------------------------------------------------------------------------------
TimeUnits TimeGridTable::end_time()
{
    //end time == 0 if table is empty
    if (m_PosTimes.size() == 0)
        return 0.0;

    return m_PosTimes.back().rTimepos + m_PosTimes.back().rDuration;
}


}  //namespace lomse

