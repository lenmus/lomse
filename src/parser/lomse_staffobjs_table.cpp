//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#include "lomse_staffobjs_table.h"

#include <algorithm>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_ldp_exporter.h"
#include "lomse_time.h"
#include "lomse_im_factory.h"

#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// ColStaffObjsEntry implementation
//=======================================================================================
string ColStaffObjsEntry::dump()
{
    stringstream s;
    s << m_instr << "\t" << m_staff << "\t" << m_measure << "\t" << time() << "\t"
      << m_line << "\t" << to_string_with_ids() << endl;
    return s.str();
}

//---------------------------------------------------------------------------------------
std::string ColStaffObjsEntry::to_string()
{
    LdpExporter exporter;
    return exporter.get_source(m_pImo);
}

//---------------------------------------------------------------------------------------
std::string ColStaffObjsEntry::to_string_with_ids()
{
    LdpExporter exporter;
    exporter.set_add_id(true);
    exporter.set_remove_newlines(true);
    return exporter.get_source(m_pImo);
}



//=======================================================================================
// ColStaffObjs implementation
//=======================================================================================

//auxiliary, for sort: by time, object type (barlines before objects in other lines),
//line and staff.
//AWARE:
//   Current order is first pA, then pB
//   return TRUE to move pB before pA, FALSE to keep in current order

bool is_lower_entry(ColStaffObjsEntry* b, ColStaffObjsEntry* a)
{
    //swap if B has lower time than A
    if ( is_lower_time(b->time(), a->time()) )
        return true;

    //both have equal time
    else if ( is_equal_time(b->time(), a->time()) )
    {
        ImoStaffObj* pB = b->imo_object();
        ImoStaffObj* pA = a->imo_object();

        //barline must go before all other objects at same measure
        //TODO: not asking for measure but for line. Is this correct?
        if (pB->is_barline() && !pA->is_barline() && b->line() != a->line())
            return true;
        else if (pA->is_barline() && !pB->is_barline() && b->line() != a->line())
            return false;

////        //note/rest can not go before non-timed
////        else if (pA->is_note_rest() && pB->get_duration() == 0.0f)
////            return true;
////        else if (pB->is_note_rest() && pA->get_duration() == 0.0f)
////            return false;
////
////        //clef in other staff can not go after key or time signature
////        else if (pB->is_clef() && (pA->is_key_signature() || pA->is_time_signature())
////                 && b->staff() != a->staff())
////            return true;
////        else if (pA->is_clef() && (pB->is_key_signature() || pB->is_time_signature()))
////            return false;
//
//        //else order by staff and line
//        return (b->line() < a->line() || (b->line() == a->line()
//                                          && b->staff() < a->staff()) );
        //else, preserve definition order
        return false;
    }

    //time(pB) > time(pA). They are correctly ordered
    return false;
}

//---------------------------------------------------------------------------------------
ColStaffObjs::ColStaffObjs()
    : m_numLines(0)
    , m_numEntries(0)
    , m_rMissingTime(0.0)
    , m_pFirst(NULL)
    , m_pLast(NULL)
{
}

//---------------------------------------------------------------------------------------
ColStaffObjs::~ColStaffObjs()
{
    ColStaffObjs::iterator it;
    for (it=begin(); it != end(); ++it)
        delete *it;
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::add_entry(int measure, int instr, int voice, int staff,
                             ImoStaffObj* pImo)
{
    ColStaffObjsEntry* pEntry =
        LOMSE_NEW ColStaffObjsEntry(measure, instr, voice, staff, pImo);
    add_entry_to_list(pEntry);
    ++m_numEntries;
}

//---------------------------------------------------------------------------------------
string ColStaffObjs::dump()
{
    stringstream s;
    ColStaffObjs::iterator it;
    s << "Num.entries = " << num_entries() << endl;
    //    +.......+.......+.......+.......+.......+.......+
    s << "instr   staff   meas.   time    line    object" << endl;
    s << "----------------------------------------------------------------" << endl;
    for (it=begin(); it != end(); ++it)
    {
        s << (*it)->dump();
    }
    return s.str();
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::add_entry_to_list(ColStaffObjsEntry* pEntry)
{
    if (!m_pFirst)
    {
        //first entry
        m_pFirst = pEntry;
        m_pLast = pEntry;
        pEntry->set_prev( NULL );
        pEntry->set_next( NULL );
        return;
    }

    //insert in list in order
    ColStaffObjsEntry* pCurrent = m_pLast;
    while (pCurrent != NULL)
    {
        if (is_lower_entry(pEntry, pCurrent))
            pCurrent = pCurrent->get_prev();
        else
        {
            //insert after pCurrent
            ColStaffObjsEntry* pNext = pCurrent->get_next();
            pEntry->set_prev( pCurrent );
            pEntry->set_next( pNext );
            pCurrent->set_next( pEntry );
            if (pNext == NULL)
                m_pLast = pEntry;
            else
                pNext->set_prev( pEntry );
            return;
        }
    }

    //it is the first one
    pEntry->set_prev( NULL );
    pEntry->set_next( m_pFirst );
    m_pFirst->set_prev( pEntry );
    m_pFirst = pEntry;
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::delete_entry_for(ImoStaffObj* pSO)
{
    ColStaffObjsEntry* pEntry = find_entry_for(pSO);
    if (!pEntry)
    {
        LOMSE_LOG_ERROR("[ColStaffObjs::delete_entry_for] entry not found!");
        throw runtime_error("[ColStaffObjs::delete_entry_for] entry not found!");
    }

    ColStaffObjsEntry* pPrev = pEntry->get_prev();
    ColStaffObjsEntry* pNext = pEntry->get_next();
    delete pEntry;
    if (pPrev == NULL)
    {
        //removing the head of the list
        m_pFirst = pNext;
        if (pNext)
            pNext->set_prev(NULL);
    }
    else if (pNext == NULL)
    {
        //removing the tail of the list
        m_pLast = pPrev;
        pPrev->set_next(NULL);
    }
    else
    {
        //removing intermediate node
        pPrev->set_next( pNext );
        pNext->set_prev( pPrev );
    }
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* ColStaffObjs::find_entry_for(ImoStaffObj* pSO)
{
    ColStaffObjs::iterator it;
    for (it=begin(); it != end(); ++it)
    {
        if ((*it)->imo_object() == pSO)
            return *it;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::sort_table()
{
    //The table is created with entries in order. But edition operations forces to
    //reorder entries. The main requirement for the sort algorithm is:
    // * stable (preserve order of elements with equal keys)
    //The chosen algorithm is the insertion sort, that also has some advantages:
    // * good performance when table is nearly ordered
    // * simple to implement and musch better performance than other simple algorithms,
    //   such as the bubble sort
    // * in-place sort (does not require extra memory)

    ColStaffObjsEntry* pUnsorted = m_pFirst;
    m_pFirst = NULL;
    m_pLast = NULL;

    while (pUnsorted != NULL)
    {
        ColStaffObjsEntry* pCurrent = pUnsorted;

        //get next here, as insertion could change next element
        pUnsorted = pUnsorted->get_next();

        add_entry_to_list(pCurrent);
    }
}



//=======================================================================================
// ColStaffObjsBuilder implementation: algorithm to create a ColStaffObjs
//=======================================================================================
ColStaffObjsBuilder::ColStaffObjsBuilder()
{
}

//---------------------------------------------------------------------------------------
ColStaffObjs* ColStaffObjsBuilder::build(ImoScore* pScore)
{
    m_pColStaffObjs = LOMSE_NEW ColStaffObjs();
    m_pImScore = pScore;
    create_table();
    set_num_lines();
    m_pImScore->set_staffobjs_table(m_pColStaffObjs);
    return m_pColStaffObjs;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::create_table()
{
    int nTotalInstruments = m_pImScore->get_num_instruments();
    for (int nInstr = 0; nInstr < nTotalInstruments; nInstr++)
    {
        //find_voices_per_staff(nInstr);
        create_entries(nInstr);
        prepare_for_next_instrument();
    }
    collect_anacrusis_info();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::set_num_lines()
{
    m_pColStaffObjs->set_total_lines( m_lines.get_number_of_lines() );
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::find_voices_per_staff(int nInstr)
{
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::create_entries(int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    ImoMusicData* pMusicData = pInstr->get_musicdata();
    if (!pMusicData)
        return;

    ImoObj::children_iterator it = pMusicData->begin();
    reset_counters();
    while(it != pMusicData->end())
    {
        if ((*it)->is_go_back_fwd())
        {
            ImoGoBackFwd* pGBF = static_cast<ImoGoBackFwd*>(*it);
            update_time_counter(pGBF);
            ++it;
            delete_node(pGBF, pMusicData);
        }
        else if ((*it)->is_key_signature() || (*it)->is_time_signature())
        {
            add_entries_for_key_or_time_signature(*it, nInstr);
            ++it;
        }
        else
        {
            ImoStaffObj* pSO = static_cast<ImoStaffObj*>(*it);
            add_entry_for_staffobj(pSO, nInstr);
            update_measure(pSO);
            ++it;
        }
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::add_entry_for_staffobj(ImoObj* pImo, int nInstr)
{
    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    determine_timepos(pSO);
    int nStaff = pSO->get_staff();
    int nVoice = 0;
    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        nVoice = pNR->get_voice();
    }
    int nLine = get_line_for(nVoice, nStaff);
    m_pColStaffObjs->add_entry(m_nCurMeasure, nInstr, nLine, nStaff, pSO);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::add_entries_for_key_or_time_signature(ImoObj* pImo, int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    int numStaves = pInstr->get_num_staves();

    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    determine_timepos(pSO);
    for (int nStaff=0; nStaff < numStaves; nStaff++)
    {
        int nLine = get_line_for(0, nStaff);
        m_pColStaffObjs->add_entry(m_nCurMeasure, nInstr, nLine, nStaff, pSO);
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::prepare_for_next_instrument()
{
    m_lines.new_instrument();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::delete_node(ImoGoBackFwd* pGBF, ImoMusicData* pMusicData)
{
    pMusicData->remove_child(pGBF);
    delete pGBF;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::reset_counters()
{
    m_nCurMeasure = 0;
    m_rCurTime = 0.0;
    m_rMaxSegmentTime = 0.0;
    m_rStartSegmentTime = 0.0;
}

//---------------------------------------------------------------------------------------
int ColStaffObjsBuilder::get_line_for(int nVoice, int nStaff)
{
    return m_lines.get_line_assigned_to(nVoice, nStaff);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::determine_timepos(ImoStaffObj* pSO)
{
    pSO->set_time(m_rCurTime);

    if (pSO->is_note())
    {
        ImoNote* pNote = static_cast<ImoNote*>(pSO);
        if (!pNote->is_in_chord() || pNote->is_end_of_chord())
            m_rCurTime += pSO->get_duration();
    }
    else
        m_rCurTime += pSO->get_duration();
    m_rMaxSegmentTime = max(m_rMaxSegmentTime, m_rCurTime);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::update_measure(ImoStaffObj* pSO)
{
    if (pSO->is_barline())
    {
        ++m_nCurMeasure;
        m_rMaxSegmentTime = 0.0;
        m_rStartSegmentTime = m_rCurTime;
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::update_time_counter(ImoGoBackFwd* pGBF)
{
    if (pGBF->is_to_start())
        m_rCurTime = m_rStartSegmentTime;
    else if (pGBF->is_to_end())
        m_rCurTime = m_rMaxSegmentTime;
    else
    {
        m_rCurTime += pGBF->get_time_shift();
        m_rMaxSegmentTime = max(m_rMaxSegmentTime, m_rCurTime);
    }
}

//---------------------------------------------------------------------------------------
ImoSpacer* ColStaffObjsBuilder::anchor_object(ImoAuxObj* pAux)
{
    Document* pDoc = m_pImScore->get_the_document();
    ImoSpacer* pAnchor = static_cast<ImoSpacer*>(ImFactory::inject(k_imo_spacer, pDoc));
    pAnchor->add_attachment(pDoc, pAux);
    return pAnchor;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::collect_anacrusis_info()
{
    ColStaffObjsIterator it = m_pColStaffObjs->begin();
    ImoTimeSignature* pTS = NULL;
    TimeUnits rTime = -1.0;

    //find time signature
    while(it != m_pColStaffObjs->end())
    {
        ImoStaffObj* pSO = (*it)->imo_object();
        if (pSO->is_time_signature())
        {
            pTS = static_cast<ImoTimeSignature*>( pSO );
            break;
        }
        else if (pSO->is_note_rest())
            return;
        ++it;
    }
    if (pTS == NULL)
        return;

    // find first barline
    ++it;
    while(it != m_pColStaffObjs->end())
    {
        ImoStaffObj* pSO = (*it)->imo_object();
        if (pSO->is_barline())
        {
            rTime = (*it)->time();
            break;
        }
        ++it;
    }
    if (rTime <= 0.0)
        return;

    //determine if anacrusis
    TimeUnits measureDuration = pTS->get_measure_duration();
    m_pColStaffObjs->set_anacrusis_missing_time(measureDuration - rTime);
}


//=======================================================================================
// StaffVoiceLineTable implementation
//=======================================================================================
StaffVoiceLineTable::StaffVoiceLineTable()
    : m_lastAssignedLine(-1)
{
    assign_line_to(0, 0);

    //first voice in each staff not yet known
    for (int i=0; i < 10; i++)
        m_firstVoiceForStaff.push_back(0);
}

//---------------------------------------------------------------------------------------
int StaffVoiceLineTable::get_line_assigned_to(int nVoice, int nStaff)
{
    int key = form_key(nVoice, nStaff);
    std::map<int, int>::iterator it = m_lineForStaffVoice.find(key);
    if (it != m_lineForStaffVoice.end())
        return it->second;
    else
        return assign_line_to(nVoice, nStaff);
}

//---------------------------------------------------------------------------------------
int StaffVoiceLineTable::assign_line_to(int nVoice, int nStaff)
{
    int key = form_key(nVoice, nStaff);
    if (nVoice != 0)
    {
        if (m_firstVoiceForStaff[nStaff] == 0)
        {
            //No voice yet assigned to this staff. Save voice nVoice as
            //first voice in this staff and assign it the sSame line than
            //voice 0 (nStaff)
            m_firstVoiceForStaff[nStaff] = nVoice;
            int line = get_line_assigned_to(0, nStaff);
            m_lineForStaffVoice[key] = line;
            return line;
        }
        else if (m_firstVoiceForStaff[nStaff] == nVoice)
        {
            //voice nVoice is the first voice found in this staff.
            //Assign it the sSame line than voice 0 (nStaff)
            return get_line_assigned_to(0, nStaff);
        }
        //else, assig it a line
    }

    //voice == 0 or voice is not first voice for this staff.
    //assign it the next available line number
    int line = ++m_lastAssignedLine;
    m_lineForStaffVoice[key] = line;
    return line;
}

//---------------------------------------------------------------------------------------
void StaffVoiceLineTable::new_instrument()
{
    m_lineForStaffVoice.clear();

    assign_line_to(0, 0);

    //first voice in each staff not yet known
    for (int i=0; i < 10; i++)
        m_firstVoiceForStaff[i] = 0;
}


}  //namespace lomse
