//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include "lomse_staffobjs_table.h"

#include <algorithm>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_ldp_exporter.h"

using namespace std;

namespace lomse
{

//-------------------------------------------------------------------------------------
// ColStaffObjsEntry implementation
//-------------------------------------------------------------------------------------

void ColStaffObjsEntry::dump()
{
    //cout << to_string() << ", time=" << m_time << ", staff=" << m_staff
    //     << ", line=" << m_line << endl;
    //segment     time       instr     line     staff     object
    cout << m_segment << "\t" << m_time << "\t" << m_instr << "\t"
         << m_line << "\t" << m_staff << "\t" << to_string_with_ids() << endl;

}

std::string ColStaffObjsEntry::to_string()
{
    LdpExporter exporter;
    return exporter.get_source(m_pImo);
}

std::string ColStaffObjsEntry::to_string_with_ids()
{
    LdpExporter exporter;
    exporter.set_add_id(true);
    return exporter.get_source(m_pImo);
}


//-------------------------------------------------------------------------------------
// ColStaffObjs implementation
//-------------------------------------------------------------------------------------

//auxiliary, for sort: by segment, time, line and staff
bool is_lower_entry(ColStaffObjsEntry* a, ColStaffObjsEntry* b)
{
    return a->segment() < b->segment()
        || (a->segment() == b->segment() && a->time() < b->time())
        || (a->segment() == b->segment() && a->time() == b->time()
            && a->line() < b->line())
        || (a->segment() == b->segment() && a->time() == b->time()
            && a->line() == b->line() && a->staff() < b->staff()) ;
}

ColStaffObjs::ColStaffObjs()
{
}

ColStaffObjs::~ColStaffObjs()
{
    std::vector<ColStaffObjsEntry*>::iterator it;
    for (it=m_table.begin(); it != m_table.end(); ++it)
        delete *it;
    m_table.clear();
}

void ColStaffObjs::AddEntry(int segment, float time, int instr, int voice, int staff,
                            ImoObj* pImo)
{
    ColStaffObjsEntry* pEntry =
        new ColStaffObjsEntry(segment, time, instr, voice, staff, pImo, NULL);
    m_table.push_back(pEntry);
}

void ColStaffObjs::dump()
{
    std::vector<ColStaffObjsEntry*>::iterator it;
    cout << "Num.entries = " << num_entries() << endl;
    //       +.......+.......+.......+.......+.......+.......+
    cout << "seg.    time    instr   line    staff   object" << endl;
    cout << "----------------------------------------------------------------" << endl;
    for (it=m_table.begin(); it != m_table.end(); ++it)
    {
        (*it)->dump();
    }
}

void ColStaffObjs::sort()
{
    std::stable_sort(m_table.begin(), m_table.end(), is_lower_entry);
}



//-------------------------------------------------------------------------------------
// ColStaffObjsBuilder implementation: algorithm to create a ColStaffObjs
//-------------------------------------------------------------------------------------

ColStaffObjsBuilder::ColStaffObjsBuilder()
{
}

//ColStaffObjs* ColStaffObjsBuilder::build(LdpElement* pScore, bool fSort)
//{
//    //param fSort is to prevent sorting the table for unit tests
//
//    m_pColStaffObjs = new ColStaffObjs();
//    m_pScore = pScore;
//    m_pImScore = dynamic_cast<ImoScore*>( m_pScore->get_imobj() );
//    create_table();
//    sort_table(fSort);
//    m_pImScore->set_staffobjs_table(m_pColStaffObjs);
//    return m_pColStaffObjs;
//}

ColStaffObjs* ColStaffObjsBuilder::build(ImoScore* pScore, bool fSort)
{
    //param fSort is to prevent sorting the table for unit tests

    m_pColStaffObjs = new ColStaffObjs();
    m_pScore = NULL;
    m_pImScore = pScore;
    create_table();
    sort_table(fSort);
    m_pImScore->set_staffobjs_table(m_pColStaffObjs);
    return m_pColStaffObjs;
}

void ColStaffObjsBuilder::create_table()
{
    int nTotalInstruments = m_pImScore->get_num_instruments();
    for (int nInstr = 0; nInstr < nTotalInstruments; nInstr++)
    {
        find_voices_per_staff(nInstr);
        create_entries(nInstr);
        prepare_for_next_instrument();
    }
}

void ColStaffObjsBuilder::find_voices_per_staff(int nInstr)
{
}

void ColStaffObjsBuilder::create_entries(int nInstr)
{
    ImoInstrument* pInstr = m_pImScore->get_instrument(nInstr);
    ImoMusicData* pMusicData = pInstr->get_musicdata();
    ImoObj::children_iterator it = pMusicData->begin();
    reset_counters();
    while(it != pMusicData->end())
    {
        ImoGoBackFwd* pGBF = dynamic_cast<ImoGoBackFwd*>(*it);
        if (pGBF)
        {
            update_time_counter(pGBF);
        }
        else
        {
            ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>(*it);
            if (pKey)
            {
                add_entries_for_key_signature(pKey, nInstr);
            }
            else
            {
                ImoStaffObj* pSO = dynamic_cast<ImoStaffObj*>(*it);
                add_entry_for_staffobj(pSO, nInstr);
                update_segment(pSO);
            }
        }
        ++it;
    }
}

void ColStaffObjsBuilder::add_entry_for_staffobj(ImoObj* pImo, int nInstr)
{
    ImoStaffObj* pSO = static_cast<ImoStaffObj*>(pImo);
    float rTime = determine_timepos(pSO);
    int nStaff = pSO->get_staff();
    int nVoice = 0;
    ImoNoteRest* pNR = dynamic_cast<ImoNoteRest*>(pSO);
    if (pNR)
        nVoice = pNR->get_voice();
    int nLine = get_line_for(nVoice, nStaff);
    m_pColStaffObjs->AddEntry(m_nCurSegment, rTime, nInstr, nLine, nStaff, pSO);
}

void ColStaffObjsBuilder::add_entries_for_key_signature(ImoObj* pImo, int nInstr)
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

void ColStaffObjsBuilder::prepare_for_next_instrument()
{
    m_lines.new_instrument();
}

void ColStaffObjsBuilder::sort_table(bool fSort)
{
    if (fSort)
        m_pColStaffObjs->sort();
}

void ColStaffObjsBuilder::reset_counters()
{
    m_nCurSegment = 0;
    m_rCurTime = 0.0f;
    m_nCurStaff = 0;
    m_rMaxTime = 0.0f;
}

int ColStaffObjsBuilder::get_line_for(int nVoice, int nStaff)
{
    return m_lines.get_line_assigned_to(nVoice, nStaff);
}

float ColStaffObjsBuilder::determine_timepos(ImoStaffObj* pSO)
{
    float rTime = m_rCurTime;
    m_rCurTime += pSO->get_duration();
    m_rMaxTime = max(m_rMaxTime, m_rCurTime);
    return rTime;
}

void ColStaffObjsBuilder::update_segment(ImoStaffObj* pSO)
{
    ImoBarline* pBL = dynamic_cast<ImoBarline*>(pSO);
    if (pBL)
    {
        ++m_nCurSegment;
        m_rMaxTime = 0.0f;
        m_rCurTime = 0.0f;
    }
}

void ColStaffObjsBuilder::update_time_counter(ImoGoBackFwd* pGBF)
{
    if (pGBF->is_to_start())
        m_rCurTime = 0.0f;
    else if (pGBF->is_to_end())
        m_rCurTime = m_rMaxTime;
    else
    {
        m_rCurTime += pGBF->get_time_shift();
        m_rMaxTime = max(m_rMaxTime, m_rCurTime);
    }
}

void ColStaffObjsBuilder::update(LdpElement* pElmScore)
{
//    ColStaffObjs* pOldColStaffObjs = m_pImScore->get_staffobjs_table();
//    delete pOldColStaffObjs;
//
//    //For now, rebuild the table
//    ImoScore* pScore = dynamic_cast<ImoScore*>( pElmScore->get_imobj() );
//    this->build(pScore);
}

void ColStaffObjsBuilder::update(ImoScore* pScore)
{
    ColStaffObjs* pOldColStaffObjs = pScore->get_staffobjs_table();
    delete pOldColStaffObjs;

    //For now, rebuild the table
    m_pImScore = pScore;
    this->build(pScore);
}

ImoSpacer* ColStaffObjsBuilder::anchor_object(ImoAuxObj* pAux)
{
    ImoSpacer* pAnchor = new ImoSpacer();
    pAnchor->attach(pAux);
    return pAnchor;
}


//-------------------------------------------------------------------------------------
// StaffVoiceLineTable implementation
//-------------------------------------------------------------------------------------

StaffVoiceLineTable::StaffVoiceLineTable()
    : m_lastAssignedLine(-1)
{
    assign_line_to(0, 0);

    //first voice in each staff not yet known
    for (int i=0; i < 10; i++)
        m_firstVoiceForStaff.push_back(0);
}

int StaffVoiceLineTable::get_line_assigned_to(int nVoice, int nStaff)
{
    int key = form_key(nVoice, nStaff);
    std::map<int, int>::iterator it = m_lineForStaffVoice.find(key);
    if (it != m_lineForStaffVoice.end())
        return it->second;
    else
        return assign_line_to(nVoice, nStaff);
}

int StaffVoiceLineTable::assign_line_to(int nVoice, int nStaff)
{
    int key = form_key(nVoice, nStaff);
    if (nVoice != 0)
    {
        if (m_firstVoiceForStaff[nStaff] == 0)
        {
            //first voice found in this staff. Same line as voice 0 (nStaff)
            m_firstVoiceForStaff[nStaff] = nVoice;
            int line = get_line_assigned_to(0, nStaff);
            m_lineForStaffVoice[key] = line;
            return line;
        }
        else if (m_firstVoiceForStaff[nStaff] == nVoice)
            //first voice found in this staff. Same line as voice 0 (nStaff)
            return get_line_assigned_to(0, nStaff);
    }
    int line = ++m_lastAssignedLine;
    m_lineForStaffVoice[key] = line;
    return line;
}

void StaffVoiceLineTable::new_instrument()
{
    m_lineForStaffVoice.clear();

    assign_line_to(0, 0);

    //first voice in each staff not yet known
    for (int i=0; i < 10; i++)
        m_firstVoiceForStaff[i] = 0;
}


}  //namespace lomse
