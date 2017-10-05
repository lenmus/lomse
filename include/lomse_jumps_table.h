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

#ifndef __LOMSE_PLAYBACK_TABLE_H__        //to avoid nested includes
#define __LOMSE_PLAYBACK_TABLE_H__

//#include "lomse_pitch.h"
//#include "lomse_time.h"

#include <vector>
#include <string>
using namespace std;


namespace lomse
{

//forward declarations
class ImoScore;
class ImoInstrument;
class ImoStaffObj;
class ImoKeySignature;
class ImoTimeSignature;
class ImoNote;
class StaffObjsCursor;


//---------------------------------------------------------------------------------------
//JumpsTable: algorithm for creating and managing playback repetitions created by
//repetition dots in barlines and by repetition marks (Da Capo, Al Segno, etc.)
class JumpsTable
{
protected:
    ImoScore* m_pScore;

public:
    JumpsTable(ImoScore* pScore);
    virtual ~JumpsTable();

    void create_table();

    inline int num_entries() { return 0; }  //int(m_events.size()); }

//    //debug
//    string dump_midi_events();
//
//
//protected:
//
//    //debug
//    string dump_events_table();
//    string dump_measures_table();
};


}   //namespace lomse

#endif  // __LOMSE_PLAYBACK_TABLE_H__
