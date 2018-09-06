//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
string ColStaffObjsEntry::dump(bool fWithIds)
{
    stringstream s;
    s << m_instr << "\t" << m_staff << "\t" << m_measure << "\t" << time() << "\t"
      << m_line << "\t" << (fWithIds ? to_string_with_ids() : to_string()) << endl;
    return s.str();
}

//---------------------------------------------------------------------------------------
std::string ColStaffObjsEntry::to_string()
{
    return m_pImo->to_string();
}

//---------------------------------------------------------------------------------------
std::string ColStaffObjsEntry::to_string_with_ids()
{
    return m_pImo->to_string_with_ids();
}



//=======================================================================================
// ColStaffObjs implementation
//=======================================================================================
ColStaffObjs::ColStaffObjs()
    : m_numLines(0)
    , m_numEntries(0)
    , m_rMissingTime(0.0)
    , m_minNoteDuration(LOMSE_NO_NOTE_DURATION)
    , m_pFirst(nullptr)
    , m_pLast(nullptr)
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
string ColStaffObjs::dump(bool fWithIds)
{
    stringstream s;
    ColStaffObjs::iterator it;
    s << "Num.entries = " << num_entries() << endl;
    //    +.......+.......+.......+.......+.......+.......+
    s << "instr   staff   meas.   time    line    object" << endl;
    s << "----------------------------------------------------------------" << endl;
    for (it=begin(); it != end(); ++it)
    {
        s << (*it)->dump(fWithIds);
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
        pEntry->set_prev( nullptr );
        pEntry->set_next( nullptr );
        return;
    }

    //insert in list in order
    ColStaffObjsEntry* pCurrent = m_pLast;
    while (pCurrent != nullptr)
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
            if (pNext == nullptr)
                m_pLast = pEntry;
            else
                pNext->set_prev( pEntry );
            return;
        }
    }

    //it is the first one
    pEntry->set_prev( nullptr );
    pEntry->set_next( m_pFirst );
    m_pFirst->set_prev( pEntry );
    m_pFirst = pEntry;
}

//---------------------------------------------------------------------------------------
bool ColStaffObjs::is_lower_entry(ColStaffObjsEntry* b, ColStaffObjsEntry* a)
{
    //auxiliary, for sort: by time, object type (barlines before objects in other lines),
    //line and staff.
    //AWARE:
    //   Current order is first pA, then pB
    //   return TRUE to move pB before pA, FALSE to keep in current order


    //swap if B has lower time than A
    if ( is_lower_time(b->time(), a->time()) )
        return true;

    //both have equal time
    else if ( is_equal_time(b->time(), a->time()) )
    {
        ImoStaffObj* pB = b->imo_object();
        ImoStaffObj* pA = a->imo_object();

        //barline must go before all other objects at same measure
        if (pB->is_barline() && !pA->is_barline() && b->measure() != a->measure())
            return true;
        if (pA->is_barline() && !pB->is_barline() && b->measure() != a->measure())
            return false;

        //note/rest can not go before non-timed at same timepos
        if (pA->is_note_rest() && pB->get_duration() == 0.0f)
            return true;

        //<direction> and <sound> can not go between clefs/key/time ==>
        //clef/key/time can not go after direction in other instruments/staves
        if ((pA->is_direction() || pA->is_sound_change())
            && (pB->is_clef() || pB->is_time_signature() || pB->is_key_signature()))
        {
            return true;    //(a->line() != b->line());    //move clef/key/time before 'A' object
        }

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
    if (pPrev == nullptr)
    {
        //removing the head of the list
        m_pFirst = pNext;
        if (pNext)
            pNext->set_prev(nullptr);
    }
    else if (pNext == nullptr)
    {
        //removing the tail of the list
        m_pLast = pPrev;
        pPrev->set_next(nullptr);
    }
    else
    {
        //removing intermediate node
        pPrev->set_next( pNext );
        pNext->set_prev( pPrev );
    }
    --m_numEntries;
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
    return nullptr;
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::sort_table()
{
    //The table is created with entries in order. But edition operations forces to
    //reorder entries. The main requirement for the sort algorithm is:
    // * stable (preserve order of elements with equal keys)
    //The chosen algorithm is the insertion sort, that also has some advantages:
    // * good performance when table is nearly ordered
    // * simple to implement and much better performance than other simple algorithms,
    //   such as the bubble sort
    // * in-place sort (does not require extra memory)

    ColStaffObjsEntry* pUnsorted = m_pFirst;
    m_pFirst = nullptr;
    m_pLast = nullptr;

    while (pUnsorted != nullptr)
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
ColStaffObjs* ColStaffObjsBuilder::build(ImoScore* pScore)
{
    ColStaffObjsBuilderEngine* builder = create_builder_engine(pScore);
    ColStaffObjs* pColStaffObjs = builder->do_build();
    pScore->set_staffobjs_table(pColStaffObjs);

    delete builder;

    return pColStaffObjs;
}

//---------------------------------------------------------------------------------------
ColStaffObjsBuilderEngine* ColStaffObjsBuilder::create_builder_engine(ImoScore* pScore)
{
    int major = pScore->get_version_major();
    if (major < 2)
        return LOMSE_NEW ColStaffObjsBuilderEngine1x(pScore);
    else
        return LOMSE_NEW ColStaffObjsBuilderEngine2x(pScore);
}


//=======================================================================================
// ColStaffObjsBuilderEngine
//=======================================================================================
ColStaffObjs* ColStaffObjsBuilderEngine::do_build()
{
    create_table();
    set_num_lines();
    set_min_note_duration();
    return m_pColStaffObjs;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::create_table()
{
    initializations();
    int totalInstruments = m_pImScore->get_num_instruments();
    for (int instr = 0; instr < totalInstruments; instr++)
    {
        create_entries(instr);
        prepare_for_next_instrument();
    }
    collect_anacrusis_info();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::collect_anacrusis_info()
{
    ColStaffObjsIterator it = m_pColStaffObjs->begin();
    ImoTimeSignature* pTS = nullptr;
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
    if (pTS == nullptr)
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

//---------------------------------------------------------------------------------------
int ColStaffObjsBuilderEngine::get_line_for(int nVoice, int nStaff)
{
    return m_lines.get_line_assigned_to(nVoice, nStaff);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::set_num_lines()
{
    m_pColStaffObjs->set_total_lines( m_lines.get_number_of_lines() );
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine::add_entries_for_key_or_time_signature(ImoObj* pImo, int nInstr)
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
void ColStaffObjsBuilderEngine::set_min_note_duration()
{
    m_pColStaffObjs->set_min_note(m_minNoteDuration);
}


//=======================================================================================
// ColStaffObjsBuilderEngine1x implementation: algorithm to create a ColStaffObjs
//=======================================================================================
void ColStaffObjsBuilderEngine1x::initializations()
{
    m_pColStaffObjs = LOMSE_NEW ColStaffObjs();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::create_entries(int nInstr)
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

            //delete_node(pGBF, pMusicData);
            //CSG: goBack/Fwd nodes can not be removed. They are necessary in score edition
            //when ColStaffObjs must be rebuild
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
void ColStaffObjsBuilderEngine1x::add_entry_for_staffobj(ImoObj* pImo, int nInstr)
{
    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    determine_timepos(pSO);
    int nStaff = pSO->get_staff();
    int nVoice = 0;
    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        nVoice = pNR->get_voice();
        m_minNoteDuration = min(m_minNoteDuration, pNR->get_duration());
    }
    int nLine = get_line_for(nVoice, nStaff);
    m_pColStaffObjs->add_entry(m_nCurMeasure, nInstr, nLine, nStaff, pSO);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::delete_node(ImoGoBackFwd* pGBF, ImoMusicData* pMusicData)
{
    pMusicData->remove_child(pGBF);
    delete pGBF;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::reset_counters()
{
    m_nCurMeasure = 0;
    m_rCurTime = 0.0;
    m_rMaxSegmentTime = 0.0;
    m_rStartSegmentTime = 0.0;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::determine_timepos(ImoStaffObj* pSO)
{
    pSO->set_time(m_rCurTime);

    if (pSO->is_note())
    {
        ImoNote* pNote = static_cast<ImoNote*>(pSO);
        if (!pNote->is_in_chord() || pNote->is_end_of_chord())
            m_rCurTime += pSO->get_duration();
    }
    else if (pSO->is_barline())
        pSO->set_time(m_rMaxSegmentTime);
    else
        m_rCurTime += pSO->get_duration();

    m_rMaxSegmentTime = max(m_rMaxSegmentTime, m_rCurTime);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::update_measure(ImoStaffObj* pSO)
{
    if (pSO->is_barline())
    {
        ++m_nCurMeasure;
        m_rMaxSegmentTime = 0.0;
        m_rStartSegmentTime = m_rCurTime;
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::update_time_counter(ImoGoBackFwd* pGBF)
{
    if (pGBF->is_to_start())
        m_rCurTime = m_rStartSegmentTime;
    else if (pGBF->is_to_end())
        m_rCurTime = m_rMaxSegmentTime;
    else
    {
        TimeUnits time = m_rCurTime + pGBF->get_time_shift();
        m_rCurTime = (time < m_rStartSegmentTime ? m_rStartSegmentTime : time);
        m_rMaxSegmentTime = max(m_rMaxSegmentTime, m_rCurTime);
    }
}

//---------------------------------------------------------------------------------------
ImoDirection* ColStaffObjsBuilderEngine1x::anchor_object(ImoAuxObj* pAux)
{
    Document* pDoc = m_pImScore->get_the_document();
    ImoDirection* pAnchor =
            static_cast<ImoDirection*>(ImFactory::inject(k_imo_direction, pDoc));
    pAnchor->add_attachment(pDoc, pAux);
    return pAnchor;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine1x::prepare_for_next_instrument()
{
    m_lines.new_instrument();
}



//=======================================================================================
// ColStaffObjsBuilderEngine2x implementation: algorithm to create a ColStaffObjs
//=======================================================================================
void ColStaffObjsBuilderEngine2x::initializations()
{
    m_rCurTime.reserve(k_max_voices);
    m_pColStaffObjs = LOMSE_NEW ColStaffObjs();
    m_curVoice = 0;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::create_entries(int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    ImoMusicData* pMusicData = pInstr->get_musicdata();
    if (!pMusicData)
        return;

    reset_counters();
    ImoObj::children_iterator it;
    for(it = pMusicData->begin(); it != pMusicData->end(); ++it)
    {
        if ((*it)->is_key_signature() || (*it)->is_time_signature())
        {
            add_entries_for_key_or_time_signature(*it, nInstr);
        }
        else
        {
            ImoStaffObj* pSO = static_cast<ImoStaffObj*>(*it);
            add_entry_for_staffobj(pSO, nInstr);
            if (pSO->is_barline())
                update_measure();
        }
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::add_entry_for_staffobj(ImoObj* pImo, int nInstr)
{
    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    determine_timepos(pSO);
    int nStaff = pSO->get_staff();
    int nVoice = 0;
    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        nVoice = pNR->get_voice();
        m_curVoice = nVoice;
    }
    else if (pSO->is_barline())
        nVoice = 0;
    else
        nVoice = m_curVoice;

    int nLine = get_line_for(nVoice, nStaff);
    m_pColStaffObjs->add_entry(m_nCurMeasure, nInstr, nLine, nStaff, pSO);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::reset_counters()
{
    m_curVoice = 0;
    m_nCurMeasure = 0;
    m_rMaxSegmentTime = 0.0;
    m_rStartSegmentTime = 0.0;
    m_rCurTime.assign(k_max_voices, 0.0);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::determine_timepos(ImoStaffObj* pSO)
{
    TimeUnits time;
    TimeUnits duration = pSO->get_duration();

    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        int voice = pNR->get_voice();
        time = m_rCurTime[voice];

        if (pSO->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pSO);
            if (pNote->is_in_chord() && !pNote->is_end_of_chord())
                duration = 0.0;
        }
        m_rCurTime[voice] += duration;

        if (duration > 0.0)
            m_minNoteDuration = min(m_minNoteDuration, duration);
    }
    else if (pSO->is_barline())
    {
        time = m_rMaxSegmentTime;
    }
    else
    {
        time = m_rCurTime[m_curVoice];
    }

    pSO->set_time(time);
    m_rMaxSegmentTime = max(m_rMaxSegmentTime, time+duration);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::update_measure()
{
    ++m_nCurMeasure;
    m_rStartSegmentTime = m_rMaxSegmentTime;
    m_rCurTime.assign(k_max_voices, m_rMaxSegmentTime);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilderEngine2x::prepare_for_next_instrument()
{
    m_lines.new_instrument();
    m_curVoice = 0;
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
