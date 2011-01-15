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
#include "lomse_internal_model.h"
//#include <iostream>
//#include <iomanip>
//#include "lomse_im_note.h"


namespace lomse
{


//---------------------------------------------------------------------------------------
SystemCursor::SystemCursor(ImoScore* pScore)
    : m_scoreIt( pScore->get_staffobjs_table() )
    , m_savedPos(m_scoreIt)
    , m_numInstruments( pScore->get_num_instruments() )
    , m_fScoreIsEmpty( m_scoreIt.is_end() )
    , m_pLastBarline(NULL)
{
    initialize_clefs_keys(pScore);

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
}

//---------------------------------------------------------------------------------------
SystemCursor::~SystemCursor()
{
//    std::vector<ScoreIterator*>::iterator it;
//    for (it = m_iterators.begin(); it != m_iterators.end(); ++it)
//        delete *it;
//    m_iterators.clear();
//
//    for (it = m_savedIterators.begin(); it != m_savedIterators.end(); ++it)
//        delete *it;
//    m_savedIterators.clear();
}

//---------------------------------------------------------------------------------------
void SystemCursor::initialize_clefs_keys(ImoScore* pScore)
{
    m_staffIndex.reserve(m_numInstruments);
    int staves = 0;
    for (int i=0; i < m_numInstruments; ++i)
    {
        m_staffIndex[i] = staves;
        staves += pScore->get_instrument(i)->get_num_staves();
    }

    m_clefs.reserve(staves);
    m_clefs.assign(staves, (ColStaffObjsEntry*)NULL);     //GCC complais if NULL not casted

    m_keys.reserve(m_numInstruments);
    m_keys.assign(m_numInstruments, (ColStaffObjsEntry*)NULL);
}

//---------------------------------------------------------------------------------------
void SystemCursor::move_next()
{
    ImoObj* pSO = imo_object();
    if (pSO->is_clef())
        save_clef();
    else if (pSO->is_key_signature())
        save_key_signature();
//    else if (pSO->is_time_signature())
//        save_time_signature();
    else if (pSO->is_barline())
        save_barline();

    ++m_scoreIt;
}

//---------------------------------------------------------------------------------------
void SystemCursor::save_clef()
{
    int iInstr = num_instrument();
    int iStaff = staff();
    int idx = m_staffIndex[iInstr] + iStaff;
    m_clefs[idx] = *m_scoreIt;
}

//---------------------------------------------------------------------------------------
void SystemCursor::save_key_signature()
{
    int iInstr = num_instrument();
    m_keys[iInstr] = *m_scoreIt;
}

//---------------------------------------------------------------------------------------
void SystemCursor::save_barline()
{
    m_pLastBarline = dynamic_cast<ImoBarline*>( imo_object() );
}

////---------------------------------------------------------------------------------------
//void SystemCursor::save_time_signature()
//{
//    //TODO
//}

//---------------------------------------------------------------------------------------
ImoStaffObj* SystemCursor::get_staffobj()
{
    return dynamic_cast<ImoStaffObj*>( imo_object() );
}

//---------------------------------------------------------------------------------------
int SystemCursor::get_num_instruments()
{
    return m_numInstruments;
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* SystemCursor::get_clef_entry_for_instr_staff(int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
    return m_clefs[idx];
}

//---------------------------------------------------------------------------------------
ImoClef* SystemCursor::get_clef_for_instr_staff(int iInstr, int iStaff)
{
    ColStaffObjsEntry* pEntry = get_clef_entry_for_instr_staff(iInstr, iStaff);
    if (pEntry)
        return dynamic_cast<ImoClef*>( pEntry->imo_object() );
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
int SystemCursor::get_clef_type_for_instr_staff(int iInstr, int iStaff)
{
    ImoClef* pClef = get_clef_for_instr_staff(iInstr, iStaff);
    if (pClef)
        return pClef->get_clef_type();
    else
        return ImoClef::k_undefined;
}

//---------------------------------------------------------------------------------------
ImoClef* SystemCursor::get_applicable_clef()
{
    if (m_fScoreIsEmpty)
        return NULL;
    else
        return get_clef_for_instr_staff( num_instrument(), staff() );
}

//---------------------------------------------------------------------------------------
int SystemCursor::get_applicable_clef_type()
{
    if (m_fScoreIsEmpty)
        return ImoClef::k_undefined;
    else
        return get_clef_type_for_instr_staff( num_instrument(), staff() );
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* SystemCursor::get_key_entry_for_instrument(int iInstr)
{
    return m_keys[iInstr];
}

//---------------------------------------------------------------------------------------
ImoKeySignature* SystemCursor::get_key_for_instrument(int iInstr)
{
    ColStaffObjsEntry* pEntry = get_key_entry_for_instrument(iInstr);
    if (pEntry)
        return dynamic_cast<ImoKeySignature*>( pEntry->imo_object() );
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
ImoKeySignature* SystemCursor::get_applicable_key()
{
    if (m_fScoreIsEmpty)
        return NULL;
    else
        return get_key_for_instrument( num_instrument() );
}

//---------------------------------------------------------------------------------------
int SystemCursor::get_applicable_key_type()
{
    if (m_fScoreIsEmpty)
        return ImoKeySignature::k_undefined;
    else
        return get_key_type_for_instrument( num_instrument() );
}

//---------------------------------------------------------------------------------------
int SystemCursor::get_key_type_for_instrument(int iInstr)
{
    ImoKeySignature* pKey = get_key_for_instrument(iInstr);
    if (pKey)
        return pKey->get_key_type();
    else
        return ImoKeySignature::k_undefined;
}

////---------------------------------------------------------------------------------------
//int SystemCursor::get_time_signature_for_instrument(int iInstr)
//{
//    return 0;
//}

////---------------------------------------------------------------------------------------
//int SystemCursor::num_instrument()
//{
//    return (m_scoreIt.is_end() ? LOMSE_NO_OBJECT : (*m_scoreIt)->num_instrument());
//}
//
////---------------------------------------------------------------------------------------
//int SystemCursor::staff()
//{
//    return (m_scoreIt.is_end() ? -1 : (*m_scoreIt)->staff();
//}
//
////---------------------------------------------------------------------------------------
//int SystemCursor::line()
//{
//    return (*m_scoreIt)->line();
//}
//
////---------------------------------------------------------------------------------------
//float SystemCursor::time()
//{
//    return (*m_scoreIt)->time();
//}
//
////---------------------------------------------------------------------------------------
//ImoObj* SystemCursor::imo_object()
//{
//    return (*m_scoreIt)->imo_object();
//}

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

//---------------------------------------------------------------------------------------
void SystemCursor::save_position()
{
    m_savedPos = m_scoreIt;
}

//---------------------------------------------------------------------------------------
void SystemCursor::go_back_to_saved_position()
{
    m_scoreIt = m_savedPos;
}

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

