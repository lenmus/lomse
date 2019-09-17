//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_GM_MEASURES_TABLE_H__
#define __LOMSE_GM_MEASURES_TABLE_H__

#include "lomse_basic.h"

//using namespace std;

namespace lomse
{


//an entry in the GmMeasuresTable
typedef struct
{
    TimeUnits rTimepos;
    TimeUnits rDuration;
    LUnits uxPos;
}
GmMeasuresTableEntry;

//---------------------------------------------------------------------------------------
/** %GmMeasuresTable object is responsible for storing and managing a table with
    the measures graphical information.

    The table is, in practice, split in several %GmMeasuresTable objects, and there is
    one %GmMeasuresTable object stored in each GmoBoxSystem object, to contain and
    manage the measures contained in each system.
*/
class GmMeasuresTable
{
protected:
    vector<GmMeasuresTableEntry> m_measures;         //the table

public:
    GmMeasuresTable();
    ///Destructor
    ~GmMeasuresTable();

//    //creation
//    void add_entries(vector<GmMeasuresTableEntry>& entries);
//    void add_entry(GmMeasuresTableEntry& entry);
//
//    //info
//    inline int get_size() { return (int)m_measures.size(); }
//    TimeUnits start_time();
//    TimeUnits end_time();
//
//    //access to an entry values
//    inline TimeUnits get_timepos(int iItem) { return m_measures[iItem].rTimepos; }
//    inline TimeUnits get_duration(int iItem) { return m_measures[iItem].rDuration; }
//    inline LUnits get_x_pos(int iItem) { return m_measures[iItem].uxPos; }
//    inline GmMeasuresTableEntry& get_entry(int iItem) { return m_measures[iItem]; }
//    inline vector<GmMeasuresTableEntry>& get_entries() { return m_measures; }
//
//    //access by position
//
//    //debug
//    string dump();

protected:

};



}   //namespace lomse

#endif      //__LOMSE_GM_MEASURES_TABLE_H__
