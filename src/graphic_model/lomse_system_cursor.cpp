//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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

#include "lomse_system_cursor.h"

//#include "lomse_basic_model.h"
//#include "lomse_gm_basic.h"
//#include "lomse_internal_model.h"
//#include <iostream>
//#include <iomanip>
//#include "lomse_im_note.h"


namespace lomse
{

//
//---------------------------------------------------------------------------------------
//SystemCursor::SystemCursor(ImoScore* pScore)
//{
//    //create iterators and point to start of each instrument
//    lmInstrument* pInstr = pScore->GetFirstInstrument();
//    for (; pInstr; pInstr = pScore->GetNextInstrument())
//    {
//        ScoreIterator* pIT = pInstr->GetVStaff()->CreateIterator();
//        pIT->AdvanceToMeasure(1);
//        m_iterators.push_back(pIT);
//        m_savedIterators.push_back( new ScoreIterator(pIT) );
//    }
//
//    m_rBreakTime = _LOMSE_NO_BREAK_TIME;
//}
//
//---------------------------------------------------------------------------------------
//SystemCursor::~SystemCursor()
//{
//    std::vector<ScoreIterator*>::iterator it;
//    for (it = m_iterators.begin(); it != m_iterators.end(); ++it)
//        delete *it;
//    m_iterators.clear();
//
//    for (it = m_savedIterators.begin(); it != m_savedIterators.end(); ++it)
//        delete *it;
//    m_savedIterators.clear();
//}
//
//---------------------------------------------------------------------------------------
//bool SystemCursor::ThereAreObjects()
//{
//    //Returns true if there are any object not yet processed in any staff
//
//    for (int i=0; i < (int)m_iterators.size(); i++)
//    {
//        if (!m_iterators[i]->EndOfCollection())
//            return true;
//    }
//    return false;
//}
//
//---------------------------------------------------------------------------------------
//lmContext* SystemCursor::GetStartOfColumnContext(int iInstr, int nStaff)
//{
//    //locate context for first note in this staff, in current segment
//
//    ScoreIterator* pIT = new ScoreIterator( GetIterator(iInstr) );
//    lmStaffObj* pSO = (lmStaffObj*)NULL;
//    //AWARE: if we are in an empty segment (last segment) and we move back to previous
//    //segment, it doesn't matter. In any case the context applying to found SO is the
//    //right context!
//    while(!pIT->EndOfCollection())
//    {
//        pSO = pIT->GetCurrent();
//        if (pSO->IsOnStaff(nStaff))
//            break;
//        pIT->MovePrev();
//    }
//    delete pIT;
//
//    if (pSO)
//        return pSO->GetCurrentContext(nStaff);
//    else
//        return (lmContext*)NULL;
//}
//
//---------------------------------------------------------------------------------------
//void SystemCursor::CommitCursors()
//{
//    //A column has been processed and it has been verified that it will be included in
//    //currente system. Therefore, SystemCursor is informed to consolidate current
//    //cursors' positions.
//    //Old saved positions are no longer needed. Current position is going to be the
//    //backup point, so save current cursors positions just in case we have to go back
//
//    for (int i=0; i < (int)m_iterators.size(); ++i)
//    {
//        delete m_savedIterators[i];
//        m_savedIterators[i] = new ScoreIterator(m_iterators[i]);
//    }
//
//    m_rBreakTime = _LOMSE_NO_BREAK_TIME;
//}
//
//---------------------------------------------------------------------------------------
//void SystemCursor::GoBackPrevPosition()
//{
//    //A column has been processed but there is not enough space for it in current system.
//    //This method is invoked to reposition cursors back to last saved positions
//
//    for (int i=0; i < (int)m_savedIterators.size(); ++i)
//    {
//        delete m_iterators[i];
//        m_iterators[i] = new ScoreIterator(m_savedIterators[i]);
//    }
//}
//
//---------------------------------------------------------------------------------------
//ImoBarline* SystemCursor::GetPreviousBarline(int iInstr)
//{
//    ScoreIterator* pIT = new ScoreIterator( GetIterator(iInstr) );
//    lmStaffObj* pSO = (lmStaffObj*)NULL;
//    while (!pIT->EndOfCollection())
//    {
//        pSO = pIT->GetCurrent();
//        if (pSO->IsBarline() || pIT->FirstOfCollection())
//            break;
//
//        pIT->MovePrev();
//    }
//    delete pIT;
//
//    if (pSO && pSO->IsBarline())
//        return (ImoBarline*)pSO;
//    else
//        return (ImoBarline*)NULL;
//}
//
//---------------------------------------------------------------------------------------
//int SystemCursor::GetNumMeasure(int iInstr)
//{
//    //returns current absolute measure number (1..n) for VStaff
//
//    return m_iterators[iInstr]->GetNumSegment() + 1;
//}
//
//---------------------------------------------------------------------------------------
//void SystemCursor::AdvanceAfterTimepos(float rTimepos)
//{
//    //advance all iterators so that last processed timepos is rTimepos. That is, pointed
//    //objects will be the firsts ones with timepos > rTimepos.
//
//    //THIS METHOD IS NO LONGER USED. BUT IT WORKS.
//    //Leaved just in case there is a need to use it again
//
//    for (int i=0; i < (int)m_iterators.size(); ++i)
//    {
//        ScoreIterator* pIT = m_iterators[i];
//        pIT->ResetFlags();
//        while (!pIT->ChangeOfMeasure() && !pIT->EndOfCollection())
//        {
//            lmStaffObj* pSO = pIT->GetCurrent();
//            if (IsHigherTime(pSO->GetTimePos(), rTimepos))
//                break;
//            pIT->MoveNext();
//        }
//    }
//}


}  //namespace lomse

