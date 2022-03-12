//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_AUTOCLEF_H__
#define __LOMSE_AUTOCLEF_H__

#include "lomse_pitch.h"

#include <vector>
#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ImoScore;
class ImoStaffObj;
class StaffObjsCursor;

//---------------------------------------------------------------------------------------
//Class AutoClef implements the algorithm for fixing malformed MusicXML files that
//do not specify clef (see issue #68).
//
//The following rules are used:
// - if an score part has pitched notes but the clef is missing, a clef will be inserted.
//   It will be a G or an F4 clef depending on notes pitch range
//
class AutoClef
{
protected:
    ImoScore* m_pScore;
    StaffObjsCursor* m_pCursor;

    vector<bool> m_fNeedsClef;      //true if staff i needs clef
    vector<ImoStaffObj*> m_pAt;     //first staffobj in the staff i
    vector<FPitch> m_maxPitch;      //max pitch in staff i
    vector<FPitch> m_minPitch;      //min pitch in staff i
    vector<int> m_numNotes;         //number of accounted notes in staff i

public:
    AutoClef(ImoScore* pScore);
    virtual ~AutoClef();

    void do_autoclef();


protected:
    void find_staves_needing_clef();
    void add_clefs();
    void add_clef(int idx);
    string decide_clef(int idx, int iStaff);

};


}   //namespace lomse

#endif      //__LOMSE_AUTOCLEF_H__
