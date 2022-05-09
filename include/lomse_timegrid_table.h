//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TIMEGRID_TABLE_H__
#define __LOMSE_TIMEGRID_TABLE_H__

#include "lomse_basic.h"

//using namespace std;

namespace lomse
{


//an entry in the TimeGridTable
typedef struct
{
    TimeUnits rTimepos;
    TimeUnits rDuration;
    LUnits uxPos;
}
TimeGridTableEntry;

//---------------------------------------------------------------------------------------
/** %TimeGridTable object is responsible for storing and managing a table with
    the relation timepos --> position for all occupied times in the score.

    The table is, in practice, split in several %TimeGridTable objects, and there is
    one %TimeGridTable object stored in each GmoBoxSystem object, to contain and
    manage the timepos --> position for each system.

    The recorded positions are for the center of note heads or rests. The last position
    is for the barline (if exists).

    This object is responsible for supplying all valid timepos and their positions
    so that other objects could, for instance:
        a) Determine the timepos to assign to a mouse click in a certain position.
        b) Draw a grid of valid timepos
        c) To determine the position for a beat.

    Important: There can exist many entries for a given timepos, the first ones are the
    x position for the non-timed staffobjs, and the last one is
    the x position for the notes/rests at that timepos. For example,
    a barline and the next note do have the same timepos, but they are placed at
    different positions. This also happens when there exist non-timed staffobjs, such as
    clefs, key  signatures and time signatures.
*/
class TimeGridTable
{
protected:
    std::vector<TimeGridTableEntry> m_PosTimes;         //the table

public:
    TimeGridTable();

    //creation
    void add_entries(std::vector<TimeGridTableEntry>& entries);
    void add_entry(TimeGridTableEntry& entry);

    //info
    inline int get_size() { return (int)m_PosTimes.size(); }
    TimeUnits start_time();
    TimeUnits end_time();

    //access to an entry values
    inline TimeUnits get_timepos(int iItem) { return m_PosTimes[iItem].rTimepos; }
    inline TimeUnits get_duration(int iItem) { return m_PosTimes[iItem].rDuration; }
    inline LUnits get_x_pos(int iItem) { return m_PosTimes[iItem].uxPos; }
    inline TimeGridTableEntry& get_entry(int iItem) { return m_PosTimes[iItem]; }
    inline std::vector<TimeGridTableEntry>& get_entries() { return m_PosTimes; }

    //access by position
    TimeUnits get_time_for_position(LUnits uxPos);

    //access by time
    /** Returns the x position for the given timepos. This method only takes notes and
        rests into account, that is, the returned value is the x position at which
        notes/rest are aligned ignoring other staff objects at the same timepos, such
        as a barline or a clef. If there are no nets/rests at the requested timepos,
        this method provides an approximated interpolated value.
        @param timepos Absolute time units for the requested position.

        See get_x_for_barline_at_time()
    */
    LUnits get_x_for_note_rest_at_time(TimeUnits timepos);

    /** Returns the x position for the given timepos. This method takes only barlines
        into account. Therefore, the returned value is the x position occupied by the
        first found barline at the provided @c timepos.
        @param timepos Absolute time units for the requested position.

        See get_x_for_note_rest_at_time()
    */
    LUnits get_x_for_barline_at_time(TimeUnits timepos);

    //debug
    std::string dump();

protected:

};



}   //namespace lomse

#endif      //__LOMSE_TIMEGRID_TABLE_H__
