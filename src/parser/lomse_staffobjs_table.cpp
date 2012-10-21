//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
    s << m_measure << "\t" << m_time << "\t" << m_instr << "\t"
      << m_line << "\t" << m_staff << "\t" << to_string_with_ids() << endl;
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
    return exporter.get_source(m_pImo);
}



//=======================================================================================
// ColStaffObjs implementation
//=======================================================================================

//auxiliary, for sort: by time, object type (barlines before objects in other lines),
//line and staff
bool is_lower_entry(ColStaffObjsEntry* a, ColStaffObjsEntry* b)
{
    if ( is_lower_time(a->time(), b->time()) )
        return true;
    else if ( is_equal_time(a->time(), b->time()) )
    {
        ImoObj* pA = a->imo_object();
        ImoObj* pB = b->imo_object();
        if (pA->is_barline() && !pB->is_barline() && a->line() != b->line())
            return true;
        else if (pB->is_barline() && !pA->is_barline() && a->line() != b->line())
            return false;
        //else continue
    }
    return (is_equal_time(a->time(), b->time()) && a->line() < b->line())
            || (is_equal_time(a->time(), b->time())
                && a->line() == b->line() && a->staff() < b->staff()) ;
}

//---------------------------------------------------------------------------------------
ColStaffObjs::ColStaffObjs()
    : m_numLines(0)
    , m_rMissingTime(0.0f)
{
}

//---------------------------------------------------------------------------------------
ColStaffObjs::~ColStaffObjs()
{
    std::vector<ColStaffObjsEntry*>::iterator it;
    for (it=m_table.begin(); it != m_table.end(); ++it)
        delete *it;
    m_table.clear();
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::AddEntry(int measure, float time, int instr, int voice, int staff,
                            ImoObj* pImo)
{
    ColStaffObjsEntry* pEntry =
        LOMSE_NEW ColStaffObjsEntry(measure, time, instr, voice, staff, pImo);
    m_table.push_back(pEntry);
}

//---------------------------------------------------------------------------------------
string ColStaffObjs::dump()
{
    stringstream s;
    std::vector<ColStaffObjsEntry*>::iterator it;
    s << "Num.entries = " << num_entries() << endl;
    //    +.......+.......+.......+.......+.......+.......+
    s << "seg.    time    instr   line    staff   object" << endl;
    s << "----------------------------------------------------------------" << endl;
    for (it=m_table.begin(); it != m_table.end(); ++it)
    {
        s << (*it)->dump();
    }
    return s.str();
}

//---------------------------------------------------------------------------------------
void ColStaffObjs::sort()
{
    std::stable_sort(m_table.begin(), m_table.end(), is_lower_entry);
}



//=======================================================================================
// ColStaffObjsBuilder implementation: algorithm to create a ColStaffObjs
//=======================================================================================
ColStaffObjsBuilder::ColStaffObjsBuilder()
{
}

//---------------------------------------------------------------------------------------
ColStaffObjs* ColStaffObjsBuilder::build(ImoScore* pScore, bool fSort)
{
    //param fSort is to prevent sorting the table for unit tests

    m_pColStaffObjs = LOMSE_NEW ColStaffObjs();
    m_pImScore = pScore;
    create_table();
    set_num_lines();
    sort_table(fSort);
    m_pImScore->set_staffobjs_table(m_pColStaffObjs);
    return m_pColStaffObjs;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::create_table()
{
    int nTotalInstruments = m_pImScore->get_num_instruments();
    for (int nInstr = 0; nInstr < nTotalInstruments; nInstr++)
    {
        find_voices_per_staff(nInstr);
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
        }
        else if ((*it)->is_key_signature() || (*it)->is_time_signature())
        {
            add_entries_for_key_or_time_signature(*it, nInstr);
        }
        else
        {
            ImoStaffObj* pSO = static_cast<ImoStaffObj*>(*it);
            add_entry_for_staffobj(pSO, nInstr);
            update_measure(pSO);
        }
        ++it;
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::add_entry_for_staffobj(ImoObj* pImo, int nInstr)
{
    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    float rTime = determine_timepos(pSO);
    int nStaff = pSO->get_staff();
    int nVoice = 0;
    if (pSO->is_note_rest())
    {
        ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
        nVoice = pNR->get_voice();
    }
    int nLine = get_line_for(nVoice, nStaff);
    m_pColStaffObjs->AddEntry(m_nCurSegment, rTime, nInstr, nLine, nStaff, pSO);
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::add_entries_for_key_or_time_signature(ImoObj* pImo, int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    int numStaves = pInstr->get_num_staves();

    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    float rTime = determine_timepos(pSO);
    for (int nStaff=0; nStaff < numStaves; nStaff++)
    {
        int nLine = get_line_for(0, nStaff);
        m_pColStaffObjs->AddEntry(m_nCurSegment, rTime, nInstr, nLine, nStaff, pSO);
    }
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::prepare_for_next_instrument()
{
    m_lines.new_instrument();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::sort_table(bool fSort)
{
    if (fSort)
        m_pColStaffObjs->sort();
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::reset_counters()
{
    m_nCurSegment = 0;
    m_rCurTime = 0.0f;
    m_nCurStaff = 0;
    m_rMaxSegmentTime = 0.0f;
    m_rStartSegmentTime = 0.0f;
}

//---------------------------------------------------------------------------------------
int ColStaffObjsBuilder::get_line_for(int nVoice, int nStaff)
{
    return m_lines.get_line_assigned_to(nVoice, nStaff);
}

//---------------------------------------------------------------------------------------
float ColStaffObjsBuilder::determine_timepos(ImoStaffObj* pSO)
{
    float rTime = m_rCurTime;
    if (pSO->is_note())
    {
        ImoNote* pNote = static_cast<ImoNote*>(pSO);
        if (!pNote->is_in_chord() || pNote->is_end_of_chord())
            m_rCurTime += pSO->get_duration();
    }
    else
        m_rCurTime += pSO->get_duration();
    m_rMaxSegmentTime = max(m_rMaxSegmentTime, m_rCurTime);
    return rTime;
}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::update_measure(ImoStaffObj* pSO)
{
    if (pSO->is_barline())
    {
        ++m_nCurSegment;
        m_rMaxSegmentTime = 0.0f;
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

////---------------------------------------------------------------------------------------
//void ColStaffObjsBuilder::update(LdpElement* pElmScore)
//{
//    ColStaffObjs* pOldColStaffObjs = m_pImScore->get_staffobjs_table();
//    delete pOldColStaffObjs;
//
//    //For now, rebuild the table
//    ImoScore* pScore = dynamic_cast<ImoScore*>( pElmScore->get_imobj() );
//    this->build(pScore);
//}

//---------------------------------------------------------------------------------------
void ColStaffObjsBuilder::update(ImoScore* pScore)
{
    //For now, rebuild the table
    m_pImScore = pScore;
    this->build(pScore);
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
    ColStaffObjs::iterator it = m_pColStaffObjs->begin();
    ImoTimeSignature* pTS = NULL;
    float rTime = -1.0f;

    //find time signature
    while(it != m_pColStaffObjs->end())
    {
        ImoObj* pSO = (*it)->imo_object();
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
        ImoObj* pSO = (*it)->imo_object();
        if (pSO->is_barline())
        {
            rTime = (*it)->time();
            break;
        }
        ++it;
    }
    if (rTime <= 0.0f)
        return;

    //determine if anacrusis
    float measureDuration = pTS->get_measure_duration();
    m_pColStaffObjs->set_anacrusis_missing_time(measureDuration - rTime);
}


//---------------------------------------------------------------------------------------
// StaffVoiceLineTable implementation
//---------------------------------------------------------------------------------------
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
