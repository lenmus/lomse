//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SYSTEM_CURSOR_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_CURSOR_H__

#include "lomse_basic.h"
#include "lomse_score_iterator.h"

namespace lomse
{

//forward declarations
class ImoScore;
class ImoBarline;

//-----------------------------------------------------------------------------------------
// SystemCursor
// To determine possible "break points" (points to finish current system and start a new
// one) we have to traverse all instruments in parallel. Class SystemCursor keeps information
// about the current traversing position.
//-----------------------------------------------------------------------------------------

//#define _LOMSE_NO_BREAK_TIME  100000000000000.0f         //any too big value

class SystemCursor
{
private:
    std::vector<ScoreIterator*> m_iterators;
    std::vector<ScoreIterator*> m_savedIterators;
    float m_rBreakTime;       //last time to include in current column

public:
    SystemCursor(ImoScore* pScore);
    ~SystemCursor();

//    bool ThereAreObjects();
//
//    //locate context for first note in this staff, in current segment
//	lmContext* GetStartOfColumnContext(int iInstr, int nStaff);
//
//    //locate previous barline in this instrument
//    ImoBarline* GetPreviousBarline(int iInstr);
//
//    //returns current absolute measure number (1..n) for VStaff
//    int GetNumMeasure(int iInstr);
//
//    //break time
//    inline float GetBreakTime() { return m_rBreakTime; }
//    inline void SetBreakTime(float rBreakTime) { m_rBreakTime = rBreakTime; }
//
//    //iterators management
//    inline ScoreIterator* get_iterator(int iInstr) { return m_iterators[iInstr]; }
//    void GoBackPrevPosition();
//    void CommitCursors();
//    void AdvanceAfterTimepos(float rTimepos);

};



}   //namespace lomse

#endif    // __LOMSE_SYSTEM_CURSOR_H__

