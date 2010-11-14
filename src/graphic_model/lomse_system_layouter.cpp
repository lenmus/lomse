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

#include "lomse_system_layouter.h"

#include "lomse_box_slice_instr.h"
#include "lomse_engraving_options.h"
//#include "lomse_basic_model.h"
//#include "lomse_gm_basic.h"
//#include "lomse_internal_model.h"
//
//// Encapsulation of the table and management algoritms to compute the positioning
//// data for each ImoStaffObj, when a bar column must be rendered.
//
////#include <vector>
////#include <list>
////#include <algorithm>
////#include <math.h>


namespace lomse
{

//class BreaksTable;
//
//#define lmDUMP_TABLES   0
//
//#define lmNO_DURATION   100000000000.0f     //any impossible high value
//#define lmNO_TIME       100000000000.0f     //any impossible high value
//#define lmNO_POSITION   100000000000.0f     //any impossible high value



////=====================================================================================
////BreaksTable implementation
////=====================================================================================
//
////---------------------------------------------------------------------------------------
//BreaksTable::BreaksTable()
//{
//}
//
////---------------------------------------------------------------------------------------
//BreaksTable::~BreaksTable()
//{
//    std::list<BreaksTimeEntry*>::iterator it;
//    for (it = m_BreaksTable.begin(); it != m_BreaksTable.end(); ++it)
//        delete *it;
//    m_BreaksTable.clear();
//}
//
////---------------------------------------------------------------------------------------
//void BreaksTable::add_entry(float rTime, LUnits uxStart, LUnits uWidth, bool fInBeam,
//                             LUnits uxBeam, float rPriority)
//{
//    BreaksTimeEntry* pBTE = new BreaksTimeEntry;
//    pBTE->rPriority = rPriority;
//    pBTE->rTimepos = rTime;
//    pBTE->uxStart = uxStart;
//    pBTE->uxEnd = uxStart + uWidth;
//    pBTE->fInBeam = fInBeam;
//    pBTE->uxBeam = uxBeam;
//
//    m_BreaksTable.push_back(pBTE);
//}
//
////---------------------------------------------------------------------------------------
//void BreaksTable::add_entry(BreaksTimeEntry* pBTE)
//{
//    add_entry(pBTE->rTimepos, pBTE->uxStart, pBTE->uxEnd - pBTE->uxStart, pBTE->fInBeam,
//             pBTE->uxBeam, pBTE->rPriority);
//}
//
////---------------------------------------------------------------------------------------
//void BreaksTable::change_priority(int iEntry, float rMultiplier)
//{
//}
//
////---------------------------------------------------------------------------------------
//std::string BreaksTable::dump()
//{
//    std::string sMsg = _T("Breaks table\n");
//    sMsg += _T("===================================================================\n\n");
//
//    if (m_BreaksTable.size() == 0)
//    {
//        sMsg += _T("The table is empty.");
//        return sMsg;
//    }
//
//    //          +....... +....... +....... +....... +..... +.......
//    sMsg += _T("Piority  ColumnLayouter  xStart   xEnd     InBeam xBeam\n");
//    sMsg += _T("-------- -------- -------- -------- ------ --------\n");
//    std::list<BreaksTimeEntry*>::iterator it;
//    for (it = m_BreaksTable.begin(); it != m_BreaksTable.end(); ++it)
//    {
//        sMsg += std::string::Format(_T("%8.2f %8.2f %8.2f %8.2f %6s %8.2f\n"),
//                    (*it)->rPriority, (*it)->rTimepos, (*it)->uxStart, (*it)->uxEnd,
//                    ((*it)->fInBeam ? _T("yes   ") : _T("no    ")), (*it)->uxBeam );
//    }
//    return sMsg;
//}
//
////---------------------------------------------------------------------------------------
//BreaksTimeEntry* BreaksTable::get_first()
//{
//    m_it = m_BreaksTable.begin();
//    if (m_it == m_BreaksTable.end())
//        return (BreaksTimeEntry*)NULL;
//
//    return *m_it;
//}
//
////---------------------------------------------------------------------------------------
//BreaksTimeEntry* BreaksTable::get_next()
//{
//    //advance to next one
//    ++m_it;
//    if (m_it != m_BreaksTable.end())
//        return *m_it;
//
//    //no more items
//    return (BreaksTimeEntry*)NULL;
//}
//
//
//
//
//
//
////=====================================================================================
////LineEntry implementation
////=====================================================================================
//
////---------------------------------------------------------------------------------------
//LineEntry::LineEntry(ImoStaffObj* pSO, GmoShape* pShape,
//                         bool fProlog)
//    : m_fIsBarlineEntry(false)
//    , m_pSO(pSO)
//    , m_pShape(pShape)
//	, m_fProlog(fProlog)
//    , m_xFinal(0.0f)
//    , m_uFixedSpace(0.0f)
//    , m_uVariableSpace(0.0f)
//    , m_rTimePos((pSO && pSO->IsAligned()) ? pSO->GetTimePos() : -1.0f )
//    , m_uSize(pShape ? pShape->GetWidth() : 0.0f )
//    , m_xLeft(pShape ? pShape->GetXLeft() : 0.0f )
//    , m_uxAnchor((pSO && pSO->IsNote()) ? - pSO->GetAnchorPos(): 0.0f )
//{
//}
//
////---------------------------------------------------------------------------------------
//void LineEntry::assign_fixed_and_variable_space(ColumnLayouter* pColFmt, float rFactor)
//{
//	//assign fixed and variable after spaces to this object and compute the xFinal pos
//
//    if (m_fIsBarlineEntry)
//    {
//		if (!m_pSO)
//            m_uSize = 0.0f;
//    }
//    else
//    {
//		if (!m_pSO->IsVisible())
//            assign_no_space();
//		else
//		{
//			if (m_pSO->is_note_rest())
//			{
//				set_note_rest_space(pColFmt, rFactor);
//			}
//			else if (m_pSO->IsClef() || m_pSO->IsKeySignature() || m_pSO->IsTimeSignature())
//			{
//                m_uFixedSpace = pColFmt->TenthsToLogical(LOMSE_EXCEPTIONAL_MIN_SPACE, 1);
//                m_uVariableSpace = pColFmt->TenthsToLogical(LOMSE_MIN_SPACE, 1) - m_uFixedSpace;
//			}
//			else if (m_pSO->IsSpacer() || m_pSO->IsScoreAnchor())
//			{
//                m_uFixedSpace = 0.0f;
//                m_uVariableSpace = m_uSize;
//			}
//			else
//                assign_no_space();
//		}
//    }
//
//    //compute final position
//    m_xFinal = m_xLeft + get_total_size();
//}
//
////---------------------------------------------------------------------------------------
//void LineEntry::set_note_rest_space(ColumnLayouter* pColFmt, float rFactor)
//{
//    assign_minimum_fixed_space(pColFmt);
//    LUnits uIdeal = compute_ideal_distance(pColFmt, rFactor);
//    assign_variable_space(uIdeal);
//}
//
////---------------------------------------------------------------------------------------
//void LineEntry::assign_minimum_fixed_space(ColumnLayouter* pColFmt)
//{
//    m_uFixedSpace = pColFmt->TenthsToLogical(LOMSE_EXCEPTIONAL_MIN_SPACE, 1);
//}
//
////---------------------------------------------------------------------------------------
//void LineEntry::assign_variable_space(LUnits uIdeal)
//{
//    m_uVariableSpace = uIdeal - m_uSize - m_uFixedSpace - m_uxAnchor;
//    if (m_uVariableSpace < 0)
//        m_uVariableSpace = 0.0f;
//}
//
////---------------------------------------------------------------------------------------
//void LineEntry::assign_no_space()
//{
//    //Doesn't have after space requirements
//    m_uFixedSpace = 0.0f;
//    m_uVariableSpace = 0.0f;
//
//    //Doesn't consume time-pos grid space.
//    m_uSize = 0.0f;
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineEntry::compute_ideal_distance(ColumnLayouter* pColFmt, float rFactor)
//{
//    if (pColFmt->IsProportionalSpacing())
//        return compute_ideal_distance_proportional(pColFmt, rFactor);
//    else
//        return compute_ideal_distance_fixed(pColFmt);
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineEntry::compute_ideal_distance_fixed(ColumnLayouter* pColFmt)
//{
//	int iStaff = m_pSO->GetStaffNum();
//    return pColFmt->TenthsToLogical(pColFmt->GetFixedSpacingValue(), iStaff);
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineEntry::compute_ideal_distance_proportional(ColumnLayouter* pColFmt,
//                                                       float rFactor)
//{
//	static const float rLog2 = 0.3010299956640f;		// log(2)
//	int iStaff = m_pSO->GetStaffNum();
//
//	//spacing function:   Space(Di) = Smin*[1 + A*log2(Di/Dmin)]
//	LUnits uSmin = pColFmt->TenthsToLogical(LOMSE_MIN_SPACE, iStaff);
//    float rVar = log(((lmNoteRest*)m_pSO)->get_duration() / LOMSE_DMIN) / rLog2;     //log2(Di/Dmin)
//    if (rVar > 0.0f)
//        return uSmin * (1.0f + rFactor * rVar);
//    else
//        return uSmin;
//}
//
////---------------------------------------------------------------------------------------
//void LineEntry::reposition_at(LUnits uxNewXLeft)
//{
//    m_xLeft = uxNewXLeft;
//    m_xFinal = m_xLeft + get_total_size();
//}
//
////---------------------------------------------------------------------------------------
//void LineEntry::move_shape()
//{
//    if (m_pSO && m_pShape)
//    {
//        LUnits uShift = m_xLeft - m_pShape->GetXLeft();
//        m_pSO->StoreOriginAndShiftShapes( uShift, m_pShape->GetOwnerIDX() );
//    }
//}
//
////---------------------------------------------------------------------------------------
//std::string LineEntry::dump_header()
//{
//    //         ...+  ..+   ...+ ..+   +  ..........+........+........+........+........+........+........+........+......+
//    return _T("item    Type      ID Prolog   Timepos  xAnchor    xLeft     Size  SpFixed    SpVar    Space   xFinal ShpIdx\n");
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineEntry::get_shift_to_noterest_center()
//{
//    if (m_pSO && m_pSO->is_note_rest())
//    {
//        //determine notehead width or rest width
//        LUnits uxWidth = 0.0f;
//        if (m_pSO->IsRest())
//            uxWidth = m_pShape->GetWidth();
//        else if (m_pSO->IsNote())
//            uxWidth = ((lmShapeNote*)m_pShape)->GetNoteHead()->GetWidth();
//
//        return uxWidth / 2.0f;
//    }
//    else
//        return 0.0f;
//}
//
////---------------------------------------------------------------------------------------
//std::string LineEntry::dump(int iEntry)
//{
//    std::string sMsg = std::string::Format(_T("%4d: "), iEntry);
//    if (m_fIsBarlineEntry)
//    {
//        sMsg += _T("  Omega");
//        if (m_pSO)
//            sMsg += std::string::Format(_T("%3d          "), m_pSO->GetScoreObjType() );
//        else
//            sMsg += _T("  -          ");
//    }
//    else
//    {
//		sMsg += std::string::Format(_T("  pSO %4d %3d   %s  "),
//								m_pSO->GetScoreObjType(),
//								m_pSO->GetID(),
//								(m_fProlog ? _T("S") : _T(" ")) );
//    }
//
//    sMsg += std::string::Format(_T("%11.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f"),
//                m_rTimePos, m_uxAnchor, m_xLeft, m_uSize, m_uFixedSpace,
//                m_uVariableSpace, get_total_size(), m_xFinal );
//
//    if (m_pShape)
//        sMsg += std::string::Format(_T("  %4d\n"), m_pShape->GetOwnerIDX());
//    else
//        sMsg += _T("    --\n");
//
//    return sMsg;
//}




//=======================================================================================
//ColumnSplitter:
//  Algorithm to determine optimum break points to split a column
//=======================================================================================
ColumnSplitter::ColumnSplitter(LineTable* pLineTable)
    : m_pLineTable(pLineTable)
{
}

//---------------------------------------------------------------------------------------
ColumnSplitter::~ColumnSplitter()
{
}

////---------------------------------------------------------------------------------------
//void ColumnSplitter::ComputeBreakPoints(BreaksTable* pBT)
//{
//    //This method computes the break points for this line and adds them to received
//    //break points table.
//    //
//    //Algorithm:
//    //
//    //In a first approach, add an entry for each timepos at which there is an object placed.
//    //Assign priority 0.8 to all entries.
//    //
//    //Now lower or raise priority of some entries according to empiric rules:
//    //
//    //  1. If there is a time signature, strongly penalize those timepos not in beat
//    //     position (priority *= 0.5)
//    //
//    //  2. Do not split notes/rests. Penalize those entries occupied in some
//    //     line (priority *= 0.7).
//    //
//    //  3. Do not to break beams. Penalize those entries  in which, at some line, there
//    //     is a beam (priority *=0.9).
//    //
//    //Finally, when all priorities have been computed, sort the table by priority (high to
//    //low) and by space (max to min).
//    //
//    //In order to accelerate the computation of this table, LineTables must have all
//    //necesary data so that it doesn't become necessary to traverse the StaffObjs
//    //colection again.
//
//    //TODO: Add filters for priority
//    const LineEntryIterator itEnd = m_pLineTable->end();
//	LineEntryIterator it = m_pLineTable->begin();
//
//    //skip initial non-timed entries
//	for (it = m_pLineTable->begin(); it != itEnd && IsLowerTime((*it)->get_timepos(), 0.0f); ++it);
//    if (it == itEnd) return;
//
//    //process current time
//    float rTime = (*it)->get_timepos();
//    LUnits uxStart = (*it)->get_position();
//    LUnits uxWidth = (*it)->get_shape_size();
//    LUnits uxBeam = 0.0f;
//    bool fInBeam = false;
//    ImoStaffObj* pSO = (*it)->m_pSO;
//    if (pSO && pSO->is_note_rest() && ((lmNoteRest*)pSO)->IsBeamed())
//    {
//        fInBeam = true;
//        ImoStaffObj* pSOEnd = ((lmNoteRest*)pSO)->GetBeam()->GetEndNoteRest();
//        GmoShape* pShape = pSOEnd->GetShape();
//        uxBeam = pShape->GetXLeft() + pShape->GetWidth();
//    }
//
//	while (it != itEnd)
//    {
//		if (IsEqualTime((*it)->get_timepos(), rTime) || IsLowerTime((*it)->get_timepos(), 0.0f))
//        {
//		    //skip any not-timed entry
//            if (IsEqualTime((*it)->get_timepos(), rTime))
//            {
//                uxWidth = wxMax(uxWidth, (*it)->get_shape_size());
//                ImoStaffObj* pSO = (*it)->m_pSO;
//                if (pSO && pSO->is_note_rest() && ((lmNoteRest*)pSO)->IsBeamed())
//                {
//                    fInBeam = true;
//                    ImoStaffObj* pSOEnd = ((lmNoteRest*)pSO)->GetBeam()->GetEndNoteRest();
//                    GmoShape* pShape = pSOEnd->GetShape();
//                    uxBeam = wxMax(uxBeam, pShape->GetXLeft() + pShape->GetWidth());
//                }
//            }
//        }
//        else
//        {
//            //new timepos. Add entry for previous timepos
//            pBT->add_entry(rTime, uxStart, uxWidth, fInBeam, uxBeam);
//
//            //start collecting data for new timepos
//            rTime = (*it)->get_timepos();
//            uxStart = (*it)->get_position();
//            uxWidth = (*it)->get_shape_size();
//            ImoStaffObj* pSO = (*it)->m_pSO;
//            if (pSO && pSO->is_note_rest() && ((lmNoteRest*)pSO)->IsBeamed())
//            {
//                fInBeam = true;
//                ImoStaffObj* pSOEnd = ((lmNoteRest*)pSO)->GetBeam()->GetEndNoteRest();
//                GmoShape* pShape = pSOEnd->GetShape();
//                uxBeam = pShape->GetXLeft() + pShape->GetWidth();
//            }
//            else
//            {
//                uxBeam = 0.0f;
//                fInBeam = false;
//            }
//       }
//		++it;
//    }
//
//    pBT->add_entry(rTime, uxStart, uxWidth, fInBeam, uxBeam);
//
//    //wxLogMessage( pBT->dump() );
//}



//=======================================================================================
//LineTable:
//  An object to encapsulate positioning data for a line
//=======================================================================================
LineTable::LineTable(int nInstr, int nVoice, LUnits uxStart, LUnits uSpace)
    : m_nInstr(nInstr)
	, m_nVoice(nVoice)
    , m_uxLineStart(uxStart)
    , m_uInitialSpace(uSpace)
{
}

//---------------------------------------------------------------------------------------
LineTable::~LineTable()
{
//    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
//		delete *it;
//
//    m_LineEntries.clear();
}

////---------------------------------------------------------------------------------------
//LineEntry* LineTable::add_entry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog)
//{
//    LineEntry* pEntry = new LineEntry(pSO, pShape, fProlog);
//    push_back(pEntry);
//	return pEntry;
//}
//
////---------------------------------------------------------------------------------------
//LineEntry* LineTable::add_final_entry(ImoStaffObj* pSO, GmoShape* pShape)
//{
//    LineEntry* pEntry = new LineEntry(pSO, pShape, false);
//    pEntry->mark_as_barline_entry();
//    push_back(pEntry);
//	return pEntry;
//}
//
////---------------------------------------------------------------------------------------
//bool LineTable::ContainsBarline()
//{
//    LineEntry* pEntry = get_last_entry();
//    return pEntry->is_barline_entry() && pEntry->has_barline();
//}
//
////---------------------------------------------------------------------------------------
//std::string LineTable::DumpMainTable()
//{
//    std::string sMsg = std::string::Format(_T("Line table dump. Instr=%d, voice=%d, xStart=%.2f, initSpace=%.2f\n"),
//									 GetInstrument(), GetVoice(), GetLineStartPosition(),
//                                     GetSpaceAtBeginning() );
//    sMsg += _T("===================================================================\n\n");
//
//    if (Size() == 0)
//    {
//        sMsg += _T("The table is empty.");
//        return sMsg;
//    }
//
//    //headers
//    sMsg += LineEntry::dump_header();
//
//    //loop to dump table entries
//    LineEntry* pTE;
//    for (int i = 0; i < (int)Size(); i++)
//    {
//        if (i % 4 == 0) {
//            sMsg += wxT("----------------------------------------------------------------------------\n");
//        }
//        pTE = item(i);
//        sMsg += pTE->dump(i);
//    }
//
//    sMsg += _T("=== end of table ==================================================\n\n");
//    return sMsg;
//
//}
//
////---------------------------------------------------------------------------------------
//void LineTable::ClearDirtyFlags()
//{
//    // clear the 'Dirty' flag of all StaffObjs in this line
//
//    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
//	{
//        if ((*it)->m_pSO)
//		    (*it)->m_pSO->SetDirty(false, true);    //true-> propagate change
//    }
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineTable::GetLineWidth()
//{
//	//Return the size of the measure represented by this line or zero if invalid line
//
//	if (m_LineEntries.size() > 0 && m_LineEntries.back()->is_barline_entry())
//        return m_LineEntries.back()->m_xFinal - GetLineStartPosition();
//    else
//        return 0.0f;
//}


//=======================================================================================
//ColumnLayouter
//=======================================================================================
ColumnLayouter::ColumnLayouter(ColumnStorage* pStorage, float rSpacingFactor,
                                     ESpacingMethod nSpacingMethod, Tenths nSpacingValue)
    : m_pColStorage(pStorage)
    , m_rSpacingFactor(rSpacingFactor)
    , m_nSpacingMethod(nSpacingMethod)
    , m_rSpacingValue(nSpacingValue)
{
}

//---------------------------------------------------------------------------------------
ColumnLayouter::~ColumnLayouter()
{
//    DeleteLineSpacers();
}

////---------------------------------------------------------------------------------------
//LUnits ColumnLayouter::TenthsToLogical(Tenths rTenths, int nStaff)
//{
//    return m_pColStorage->TenthsToLogical(rTenths, nStaff);
//}
//
////---------------------------------------------------------------------------------------
//void ColumnLayouter::DeleteLineSpacers()
//{
//    LineSpacersIterator it;
//    for (it = m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
//        delete *it;
//    m_LineSpacers.clear();
//}
//
////---------------------------------------------------------------------------------------
//bool ColumnLayouter::IsThereBarline()
//{
//    //returns true if there is at least one line containing a barline
//
//    for (LinesIterator it=m_pColStorage->begin(); it != m_pColStorage->end(); ++it)
//    {
//        if ((*it)->ContainsBarline())
//            return true;
//    }
//    return false;
//}
//
////---------------------------------------------------------------------------------------
//void ColumnLayouter::DoSpacing(bool fTrace)
//{
//    //computes the minimum space required by this column
//
//#if lmDUMP_TABLES
//    wxLogMessage( m_pColStorage->DumpColumnStorage() );
//#endif
//
//    m_uMinColumnSize = ComputeSpacing();
//
//#if lmDUMP_TABLES
//    wxLogMessage( m_pColStorage->DumpColumnStorage() );
//#endif
//}
//
////---------------------------------------------------------------------------------------
//LUnits ColumnLayouter::ComputeSpacing()
//{
//    //Spacing algorithm. Returns the resulting minimum column width
//
//    CreateLineSpacers();
//    ProcessNonTimedAtProlog();
//    ProcessTimedAtCurrentTimepos();
//    while (ThereAreObjects())
//    {
//        ProcessNonTimedAtCurrentTimepos();
//        ProcessTimedAtCurrentTimepos();
//    }
//
//    DeleteLineSpacers();
//	return m_pColStorage->GetColumnWitdh();
//}
//
////---------------------------------------------------------------------------------------
//void ColumnLayouter::CreateLineSpacers()
//{
//    const LinesIterator itEnd = m_pColStorage->end();
//    for (LinesIterator it=m_pColStorage->begin(); it != itEnd; ++it)
//	{
//        LineSpacer* pLinSpacer = new LineSpacer(*it, this, m_rSpacingFactor);
//        m_LineSpacers.push_back(pLinSpacer);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void ColumnLayouter::ProcessNonTimedAtProlog()
//{
//    LUnits uSpaceAfterProlog = TenthsToLogical(LOMSE_SPACE_AFTER_PROLOG, 1);
//    m_rCurrentTime = lmNO_TIME;           //any impossible high value
//    m_rCurrentPos = 0.0f;
//    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
//	{
//        (*it)->ProcessNonTimedAtProlog(uSpaceAfterProlog);
//        LUnits uxNextPos = (*it)->GetNextPosition();
//        m_rCurrentTime = wxMin(m_rCurrentTime, (*it)->GetNextAvailableTime());
//        m_rCurrentPos = wxMax(m_rCurrentPos, uxNextPos);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void ColumnLayouter::ProcessTimedAtCurrentTimepos()
//{
//    m_fThereAreObjects = false;
//    float rNextTime = lmNO_TIME;           //any impossible high value
//    LUnits uxPosForNextTime = lmNO_POSITION;    //any impossible high value
//    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
//	{
//        if ((*it)->CurrentTimeIs(m_rCurrentTime) && (*it)->ThereAreTimedObjs())
//        {
//            (*it)->ProcessTimedAtCurrentTimepos(m_rCurrentPos);
//            LUnits uxNextPos = (*it)->GetNextPosition();
//            uxPosForNextTime = wxMin(uxPosForNextTime, uxNextPos);
//        }
//        if ((*it)->ThereAreMoreObjects())
//        {
//            m_fThereAreObjects = true;
//            rNextTime = wxMin(rNextTime, (*it)->GetNextAvailableTime());
//        }
//    }
//
//    m_rCurrentTime = rNextTime;
//    if (uxPosForNextTime < lmNO_POSITION)
//        m_rCurrentPos = uxPosForNextTime;
//}
//
////---------------------------------------------------------------------------------------
//void ColumnLayouter::ProcessNonTimedAtCurrentTimepos()
//{
//    LUnits uxPosForNextTime = 0.0f;
//    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
//	{
//        (*it)->ProcessNonTimedAtCurrentTimepos(m_rCurrentPos);
//        LUnits uxNextPos = (*it)->GetNextPosition();
//        uxPosForNextTime = wxMax(uxPosForNextTime, uxNextPos);
//    }
//    m_rCurrentPos = uxPosForNextTime;
//}




//=======================================================================================
// SystemLayouter implementation
//=======================================================================================
SystemLayouter::SystemLayouter()    //float rSpacingFactor, ESpacingMethod nSpacingMethod,
                                    //Tenths rSpacingValue)
//    : m_rSpacingFactor(rSpacingFactor)
//    , m_nSpacingMethod(nSpacingMethod)
//    , m_rSpacingValue(rSpacingValue)
{
}

//---------------------------------------------------------------------------------------
SystemLayouter::~SystemLayouter()
{
    std::vector<ColumnLayouter*>::iterator itF;
    for (itF=m_ColLayouters.begin(); itF != m_ColLayouters.end(); ++itF)
        delete *itF;
    m_ColLayouters.clear();

    std::vector<ColumnStorage*>::iterator itS;
    for (itS=m_ColStorage.begin(); itS != m_ColStorage.end(); ++itS)
        delete *itS;
    m_ColStorage.clear();

    std::vector<LinesBuilder*>::iterator itLB;
    for (itLB=m_LinesBuilder.begin(); itLB != m_LinesBuilder.end(); ++itLB)
        delete *itLB;
    m_LinesBuilder.clear();
}

//---------------------------------------------------------------------------------------
//void SystemLayouter::EndOfSystemMeasurements()
//{
//    //caller informs that all data for this system has been suplied.
//    //This is the right place to do any preparatory work, not to be repeated if re-spacing.
//
//    //Nothing to do for current implementation
//}

//---------------------------------------------------------------------------------------
void SystemLayouter::start_bar_measurements(int iCol, LUnits uxStart, LUnits uSpace)
{
    //prepare to receive data for a new bar in column iCol [0..n-1].

    //If not yet created, create ColumnLayouter object to store measurements
    //LinesBuilder* pLB;
    if (m_ColLayouters.size() == (size_t)iCol)
    {
        //create storage for this column
        ColumnStorage* pStorage = new ColumnStorage();
        m_ColStorage.push_back(pStorage);

        //create a lines builder object for this column
        LinesBuilder* pLB = new LinesBuilder(pStorage);
        m_LinesBuilder.push_back(pLB);

        //create the column formatter object
        ColumnLayouter* pColFmt = new ColumnLayouter(pStorage, m_rSpacingFactor,
                                                     m_nSpacingMethod, m_rSpacingValue);
        m_ColLayouters.push_back(pColFmt);
    }
    //else
    //    pLB = m_LinesBuilder[iCol];

    ////start lines
    //pLB->start_measurements_for_instrument(nInstr, uxStart, pInstr, uSpace);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::include_object(int iCol, int iInstr, ImoStaffObj* pSO, bool fProlog,
                                    int nStaff, GmoShape* pShape)
{
    //caller sends data about one staffobj in current bar, for column iCol [0..n-1]

    m_LinesBuilder[iCol]->include_object(iInstr, pSO, fProlog, nStaff);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::include_barline_and_terminate_bar_measurements(int iCol,
                                                                    ImoStaffObj* pSO,
                                                                    LUnits xStart)
{
    //caller sends lasts object to store in current bar, for column iCol [0..n-1].

    m_LinesBuilder[iCol]->close_line(pSO, xStart);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::terminate_bar_measurements_without_barline(int iCol, LUnits xStart)
{
    //caller informs that there are no barline and no more objects in column iCol [0..n-1].

    m_LinesBuilder[iCol]->close_line(NULL, xStart);
}

////---------------------------------------------------------------------------------------
//void SystemLayouter::DiscardMeasurementsForColumn(int iCol)
//{
//    //caller request to ignore measurements for column iCol [0..n-1]
//
//    m_ColStorage[iCol]->Initialize();
//    m_ColLayouters[iCol]->Initialize();
//    m_LinesBuilder[iCol]->Initialize();
//}
//
////---------------------------------------------------------------------------------------
//void SystemLayouter::DoColumnSpacing(int iCol, bool fTrace)
//{
//    m_ColLayouters[iCol]->DoSpacing(fTrace);
//}
//
////---------------------------------------------------------------------------------------
//LUnits SystemLayouter::RedistributeSpace(int iCol, LUnits uNewStart)
//{
//    LUnits uNewBarSize = m_ColLayouters[iCol]->GetMinimumSize();
//    ColumnResizer oResizer(m_ColStorage[iCol], uNewBarSize);
//	oResizer.RepositionShapes(uNewStart);
//
//    LUnits uBarFinalPosition = uNewStart + uNewBarSize;
//    return uBarFinalPosition;
//}
//
////---------------------------------------------------------------------------------------
//void SystemLayouter::AddTimeGridToBoxSlice(int iCol, GmoBoxSlice* pBSlice)
//{
//    //create the time-grid table and transfer it (and its ownership) to GmoBoxSlice
//    pBSlice->SetTimeGridTable( new TimeGridTable(m_ColStorage[iCol]) );
//}
//
////---------------------------------------------------------------------------------------
//void SystemLayouter::IncrementColumnSize(int iCol, LUnits uIncr)
//{
//    m_ColLayouters[iCol]->IncrementColumnSize(uIncr);
//}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_shapes(std::vector<GmoBoxSliceInstr*>& sliceInstrBoxes)
{
    int iInstr = 0;
    std::vector<GmoBoxSliceInstr*>::iterator it;
    for (it = sliceInstrBoxes.begin(); it != sliceInstrBoxes.end(); ++it, ++iInstr)
    {
  //      GmoBoxSliceInstr* pBox = *it;
  //      ImoObj* pImo = NULL;
  //      ClefEngraver engraver(pBox, pImo);
		//GmoShape* pShape = engraver.create_shape(pBox, pImo);
  //                     //(GmoBox* pBox, UPoint uPos, Color colorC, bool fSmallClef);
  //      (*it)->add_shape(pShape, GmoShape::k_layer_notes);
    }
}

////---------------------------------------------------------------------------------------
//LUnits SystemLayouter::GetStartPositionForColumn(int iCol)
//{
//    return m_ColStorage[iCol]->GetStartOfBarPosition();
//}
//
////---------------------------------------------------------------------------------------
//LUnits SystemLayouter::GetMinimumSize(int iCol)
//{
//    return m_ColLayouters[iCol]->GetMinimumSize();
//}
//
////---------------------------------------------------------------------------------------
//bool SystemLayouter::GetOptimumBreakPoint(int iCol, LUnits uAvailable,
//                                        float* prTime, LUnits* puWidth)
//{
//    //return m_ColLayouters[iCol]->GetOptimumBreakPoint(uAvailable, prTime, puWidth);
//    BreakPoints oBreakPoints(m_ColStorage[iCol]);
//    if (oBreakPoints.FindOptimunBreakPointForSpace(uAvailable))
//    {
//        *prTime = oBreakPoints.GetOptimumTimeForFoundBreakPoint();
//        *puWidth = oBreakPoints.GetOptimumPosForFoundBreakPoint();
//        return false;
//    }
//    else
//        return true;
//}
//
////---------------------------------------------------------------------------------------
//bool SystemLayouter::ColumnHasBarline(int iCol)
//{
//    return m_ColLayouters[iCol]->IsThereBarline();
//}
//
////---------------------------------------------------------------------------------------
//void SystemLayouter::ClearDirtyFlags(int iCol)
//{
//    m_ColStorage[iCol]->ClearDirtyFlags();
//}
//
////---------------------------------------------------------------------------------------
//std::string SystemLayouter::DumpColumnData(int iCol)
//{
//    return m_ColStorage[iCol]->DumpColumnStorage();
//}
//
////------------------------------------------------
//// Debug build: methods coded only for Unit Tests
////------------------------------------------------
//#if defined(_LM_DEBUG_)
//
//int SystemLayouter::GetNumObjectsInColumnLine(int iCol, int iLine)
//{
//    //iCol, iLine = [0..n-1]
//    return m_ColStorage[iCol]->GetNumObjectsInLine(iLine);
//}
//
//#endif



//=======================================================================================
// ColumnStorage implementation: encapsulates the table of lines for a column
//=======================================================================================
ColumnStorage::ColumnStorage()
{
}

//---------------------------------------------------------------------------------------
ColumnStorage::~ColumnStorage()
{
//    DeleteLines();
}

////---------------------------------------------------------------------------------------
//void ColumnStorage::Initialize()
//{
//    DeleteLines();
//}
//
////---------------------------------------------------------------------------------------
//void ColumnStorage::DeleteLines()
//{
//	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
//	{
//		delete *it;
//	}
//	m_Lines.clear();
//}
//
////---------------------------------------------------------------------------------------
//LUnits ColumnStorage::TenthsToLogical(Tenths rTenths, int nStaff)
//{
//    wxASSERT(nStaff > 0);
//	return m_pStaff[nStaff-1]->TenthsToLogical(rTenths);
//}
//
////---------------------------------------------------------------------------------------
//LinesIterator ColumnStorage::FindLineForInstrAndVoice(int nInstr, int nVoice)
//{
//    //return m_pColStorage->FindLineForInstrAndVoice(nInstr, nVoice);
//	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
//    {
//		if ((*it)->IsLineForInstrument(nInstr) && (*it)->IsLineForVoice(nVoice) )
//            return it;
//	}
//    return m_Lines.end();
//}
//
////---------------------------------------------------------------------------------------
//LineTable* ColumnStorage::OpenNewLine(int nInstr, int nVoice, LUnits uxStart,
//                                               LUnits uSpace)
//{
//    LineTable* pLineTable = new LineTable(nInstr, nVoice, uxStart, uSpace);
//    m_Lines.push_back(pLineTable);
//    return pLineTable;
//}
//
////---------------------------------------------------------------------------------------
//std::string ColumnStorage::DumpColumnStorage()
//{
//    std::string sMsg = _T("Start of dump. ColumnStorage\n");
//	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
//	{
//        sMsg += (*it)->DumpMainTable();
//    }
//    return sMsg;
//}
//
////---------------------------------------------------------------------------------------
//void ColumnStorage::ClearDirtyFlags()
//{
//    //clear flag 'Dirty' in all StaffObjs of this table. This has nothing to do with
//    //ColumnStorage purposes, but its is a convenient place to write a method
//    //for doing this.
//
//	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
//		(*it)->ClearDirtyFlags();
//}
//
////---------------------------------------------------------------------------------------
//LUnits ColumnStorage::GetColumnWitdh()
//{
//    LUnits uColWidth = 0;
//	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
//        uColWidth = wxMax(uColWidth, (*it)->GetLineWidth());
//
//    return uColWidth;
//}
//
////---------------------------------------------------------------------------------------
//LUnits ColumnStorage::GetStartOfBarPosition()
//{
//    //returns the x position for the start of the bar column
//
//    return m_Lines.front()->GetLineStartPosition();
//}



//=======================================================================================
// LinesBuilder implementation: encapsulates the algorithms to split a column
// into lines and to store them in the received column storage
//=======================================================================================
LinesBuilder::LinesBuilder(ColumnStorage* pStorage)
    : m_pColStorage(pStorage)
	, m_pCurEntry((LineEntry*)NULL)
{
//    Initialize();
}

//---------------------------------------------------------------------------------------
LinesBuilder::~LinesBuilder()
{
}

////---------------------------------------------------------------------------------------
//void LinesBuilder::Initialize()
//{
//    ResetDefaultStaffVoices();
//}
//
////---------------------------------------------------------------------------------------
//void LinesBuilder::ResetDefaultStaffVoices()
//{
//    for(int i=0; i < lmMAX_STAFF; i++)
//        m_nStaffVoice[i] = 0;
//}
//
////---------------------------------------------------------------------------------------
//void LinesBuilder::start_measurements_for_instrument(int nInstr, LUnits uxStart,
//                                                      ImoInstrument* pInstr, LUnits uSpace)
//{
//    CreateLinesForEachStaff(nInstr, uxStart, pInstr, uSpace);
//}
//
////---------------------------------------------------------------------------------------
//void LinesBuilder::CreateLinesForEachStaff(int nInstr, LUnits uxStart,
//                                                ImoInstrument* pInstr, LUnits uSpace)
//{
//    //We need at least one line for each staff, for the music on each staff.
//    //As we don'y know yet which voice number will be the first note/rest on each staff we
//    //cannot yet assign voice to these lines. Therefore, we will assign voice 0 (meaning
//    //'no voice assigned yet') and voice will be updated when finding the first note/rest.
//
//	int nNumStaves = pInstr->GetNumStaves();
//    wxASSERT(nNumStaves < lmMAX_STAFF);
//
//    for(int iS=0; iS < nNumStaves; iS++)
//    {
//        m_pColStorage->SaveStaffPointer(iS, pInstr->GetStaff(iS+1));
//        m_nStaffVoice[iS] = iS+1;
//        StartLine(nInstr, iS+1, uxStart, uSpace);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void LinesBuilder::StartLineInheritInitialPostionAndSpace(int nInstr, int nVoice)
//{
//    LUnits uxStart = m_pColStorage->front()->GetLineStartPosition();
//    LUnits uSpace = m_pColStorage->front()->GetSpaceAtBeginning();
//    StartLine(nInstr, nVoice, uxStart, uSpace);
//}
//
////---------------------------------------------------------------------------------------
//void LinesBuilder::StartLine(int nInstr, int nVoice, LUnits uxStart, LUnits uSpace)
//{
//    //Start a new line for instrument nInstr (0..n-1), to be used for voice nVoice.
//    //The line starts at position uxStart and space before first object must be uSpace.
//
//    //create the line and store it
//    LineTable* pLineTable = m_pColStorage->OpenNewLine(nInstr, nVoice, uxStart, uSpace);
//
//    //created line is set as 'current line' to receive new data.
//    m_itCurLine = m_pColStorage->GetLastLine();
//
//    //as line is empty, pointer to last added entry is NULL
//	m_pCurEntry = (LineEntry*)NULL;
//}

//---------------------------------------------------------------------------------------
void LinesBuilder::close_line(ImoStaffObj* pSO, LUnits xStart)  //GmoShape* pShape,
{
//	//close current line.
//
//    m_pCurEntry = (*m_itCurLine)->add_final_entry(pSO, pShape);
//    m_pCurEntry->set_position(xStart);
}

//---------------------------------------------------------------------------------------
void LinesBuilder::include_object(int iInstr, ImoStaffObj* pSO, bool fProlog, int nStaff)
{
//    int nVoice = DecideVoiceToUse(pSO, nStaff);
//    m_itCurLine = m_pColStorage->FindLineForInstrAndVoice(nInstr, nVoice);
//
//    //if doesn't exist, start it
//    if (m_pColStorage->IsEndOfTable(m_itCurLine))
//    {
//        wxASSERT(nVoice != 0);          //it must be a valid voice. Otherwise the default
//                                        //line must have been found!
//        StartLineInheritInitialPostionAndSpace(nInstr, nVoice);
//    }
//
//    //add new entry for this object
//	m_pCurEntry = (*m_itCurLine)->add_entry(pSO, pShape, fProlog);
//
//	//if line found was the default one for the staff, assigne voice to this line and to
//    //the staff if not yet assigned
//    if (nVoice != 0 && (*m_itCurLine)->IsVoiceNotYetDefined())
//    {
//		(*m_itCurLine)->SetVoice(nVoice);
//    }
}

////---------------------------------------------------------------------------------------
//int LinesBuilder::DecideVoiceToUse(ImoStaffObj* pSO, int nStaff)
//{
//	if (nStaff != 0)    //multi-shaped object (clef, key)
//    {
//        wxASSERT(pSO->IsMultishaped());
//        return m_nStaffVoice[nStaff - 1];
//    }
//    else    //single shape object
//    {
//		if (pSO->is_note_rest())
//            //pSO has voice: return it
//			return ((lmNoteRest*)pSO)->GetVoice();
//		else
//            //pSO has no voice. Use voice assigned to the staff in which this pSO is placed
//			return m_nStaffVoice[ pSO->GetStaffNum() - 1 ];
//	}
//}
//
////---------------------------------------------------------------------------------------
//void LinesBuilder::EndOfData()
//{
//    //this method is invoked to inform that all data has been suplied. Therefore, we
//    //can do any preparatory work, not to be repeated if re-spacing.
//}



//=======================================================================================
// LineResizer implementation:
//      encapsulates the methods to recompute shapes positions so that the column
//      will have the desired width, and to move the shapes to those positions
//=======================================================================================
LineResizer::LineResizer(LineTable* pTable, LUnits uOldBarSize,
                             LUnits uNewBarSize, LUnits uNewStart)
    : m_pTable(pTable)
    , m_uOldBarSize(uOldBarSize)
    , m_uNewBarSize(uNewBarSize)
    , m_uNewStart(uNewStart)
{
}

////---------------------------------------------------------------------------------------
//float LineResizer::MovePrologShapes()
//{
//    //all non-timed entries, at beginning, marked as fProlog must be only re-located
//    //returns the first timepos found after the prolog or 0 if no valid timepos
//
//    LUnits uLineStartPos = m_pTable->GetLineStartPosition();
//    LUnits uLineShift = m_uNewStart - uLineStartPos;
//    LineEntryIterator it = m_pTable->begin();
//    while (it != m_pTable->end() && (*it)->get_timepos() < 0.0f)
//    {
//        if ((*it)->m_pShape)
//        {
//            if ((*it)->m_fProlog)
//            {
//                LUnits uNewPos = uLineShift + (*it)->get_position();
//                (*it)->reposition_at(uNewPos);
//                (*it)->move_shape();
//            }
//            else
//			    break;
//        }
//        ++it;
//    }
//    m_itCurrent = it;
//
//    //return first timepos in this line
//    if (it != m_pTable->end())
//    {
//        if ((*it)->get_timepos() < 0.0f)
//            return 0.0f;
//        else
//            return (*it)->get_timepos();
//    }
//    else
//        return 0.0f;
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineResizer::GetTimeLinePositionIfTimeIs(float rFirstTime)
//{
//    if (m_itCurrent != m_pTable->end() && (*m_itCurrent)->get_timepos() == rFirstTime)
//        return (*m_itCurrent)->get_position() - (*m_itCurrent)->get_anchor();
//    else
//        return 0.0f;
//}
//
////---------------------------------------------------------------------------------------
//void LineResizer::ReassignPositionToAllOtherObjects(LUnits uFizedSizeAtStart)
//{
//    if (m_itCurrent == m_pTable->end())
//        return;
//
//    //Compute proportion factor
//    LUnits uLineStartPos = m_pTable->GetLineStartPosition();
//    LUnits uLineShift = m_uNewStart - uLineStartPos;
//    LUnits uDiscount = uFizedSizeAtStart - uLineStartPos;
//    float rProp = (m_uNewBarSize-uDiscount) / (m_uOldBarSize-uDiscount);
//
//	//Reposition the remainder entries
//    for (LineEntryIterator it = m_itCurrent; it != m_pTable->end(); ++it)
//	{
//        if ((*it)->is_barline_entry())
//        {
//            LUnits uNewPos = m_uNewStart + m_uNewBarSize - (*it)->get_shape_size();
//            (*it)->reposition_at(uNewPos);
//            (*it)->move_shape();
//        }
//        else
//        {
//            LUnits uOldPos = (*it)->get_position() - (*it)->get_anchor();
//            LUnits uShift = uDiscount + (m_uNewStart + (uOldPos - uFizedSizeAtStart) * rProp) - uOldPos;
//            LUnits uNewPos = uOldPos + uShift + (*it)->get_anchor();;
//            (*it)->reposition_at(uNewPos);
//            (*it)->move_shape();
//        }
//    }
//}
//
////---------------------------------------------------------------------------------------
//void LineResizer::InformAttachedObjs()
//{
//    //StaffObj shapes has been moved to their final positions. This method is invoked
//    //to inform some attached AuxObjs (i.e. ties) so that they can compute their
//    //final positions.
//
//    for (LineEntryIterator it = m_pTable->begin(); it != m_pTable->end(); ++it)
//	{
//        if (!(*it)->is_barline_entry())
//        {
//            if ((*it)->m_pSO->IsNote()
//                && ((lmNote*)(*it)->m_pSO)->IsTiedToPrev() )
//            {
//                //end of tie note. Inform the tie shape.
//                wxASSERT((*it)->m_pShape);
//				((lmShapeNote*)(*it)->m_pShape)->ApplyUserShiftsToTieShape();
//            }
//        }
//    }
//}



//=======================================================================================
//LineSpacer:
//  encapsulates the algorithm to assign spaces and positions to a single line
//=======================================================================================
LineSpacer::LineSpacer(LineTable* pLineTable, ColumnLayouter* pColFmt,
                           float rFactor)
    : m_pTable(pLineTable)
    , m_rFactor(rFactor)
    , m_pColFmt(pColFmt)
    , m_itCur(pLineTable->end())
    , m_rCurTime(0.0f)
	, m_uxCurPos(0.0f)
    , m_uxRemovable(0.0f)
{
//    InitializeForTraversing();
}

////---------------------------------------------------------------------------------------
//void LineSpacer::InitializeForTraversing()
//{
//    //initialize iteration control data, to traverse by timepos
//
//    m_itCur = m_pTable->begin();
//    m_rCurTime = GetNextAvailableTime();
//    m_uxCurPos = m_pTable->GetLineStartPosition() + m_pTable->GetSpaceAtBeginning();
//    m_itNonTimedAtCurPos = m_pTable->end();
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::ProcessNonTimedAtCurrentTimepos(LUnits uxPos)
//{
//    //update current pos with new xPos required for column alignment
//    m_uxRemovable += uxPos - m_uxCurPos;
//    m_uxCurPos = uxPos;
//
//    //proceed if there are non-timed objects
//    if (CurrentObjectIsNonTimed())
//    {
//        ComputeMaxAndMinOcuppiedSpace();
//        PositionNonTimed();
//    }
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineSpacer::GetNextPosition()
//{
//    return m_uxCurPos;
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::ComputeMaxAndMinOcuppiedSpace()
//{
//	//Starting at current position, explores the not-timed objects until next timed
//    //or end of line. Computes the maximum and minimum space they could occupy.
//    //Current position is not altered
//
//    m_uxMaxOcuppiedSpace = 0.0f;
//    m_uxMinOcuppiedSpace = 0.0f;
//    LineEntryIterator it = m_itCur;
//	while (IsNonTimedObject(it))
//    {
//        (*it)->assign_fixed_and_variable_space(m_pColFmt, m_rFactor);
//        LUnits uxMax = (*it)->get_total_size();
//        m_uxMaxOcuppiedSpace += uxMax;
//        m_uxMinOcuppiedSpace += uxMax - (*it)->get_variable_space();
//        ++it;
//    }
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::PositionNonTimed()
//{
//    m_itNonTimedAtCurPos = m_itCur;
//    if (m_uxRemovable >= m_uxMaxOcuppiedSpace)
//    {
//        PositionUsingMaxSpaceWithShift(m_uxRemovable - m_uxMaxOcuppiedSpace);
//    }
//    else if (m_uxRemovable >= m_uxMinOcuppiedSpace)
//    {
//        LUnits uShift = m_uxRemovable - m_uxMinOcuppiedSpace;
//        PositionUsingMinSpaceWithShift(uShift);
//    }
//    else
//    {
//        PositionUsingMinSpaceWithShift(0.0f);
//    }
//    m_uxNotTimedFinalPos = m_uxCurPos;
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::PositionUsingMaxSpaceWithShift(LUnits uShift)
//{
//    LUnits uxNextPos = m_uxCurPos - m_uxRemovable + uShift;
//	while (CurrentObjectIsNonTimed())
//    {
//        (*m_itCur)->assign_fixed_and_variable_space(m_pColFmt, m_rFactor);
//        (*m_itCur)->reposition_at(uxNextPos);
//
//        uxNextPos += (*m_itCur)->get_total_size();
//        ++m_itCur;
//    }
//
//    //update iteration data
//    m_uxCurPos = uxNextPos;
//    m_uxRemovable = 0.0f;
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::PositionUsingMinSpaceWithShift(LUnits uShift)
//{
//    LUnits uxNextPos = m_uxCurPos - m_uxRemovable + uShift;
//	while (CurrentObjectIsNonTimed())
//    {
//        (*m_itCur)->assign_fixed_and_variable_space(m_pColFmt, m_rFactor);
//        (*m_itCur)->set_variable_space(0.0f);
//        (*m_itCur)->reposition_at(uxNextPos);
//
//        uxNextPos += (*m_itCur)->get_total_size();
//        ++m_itCur;
//    }
//
//    //update iteration data
//    m_uxCurPos = uxNextPos;
//    m_uxRemovable = 0.0f;
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::ProcessNonTimedAtProlog(LUnits uSpaceAfterProlog)
//{
//    if (CurrentObjectIsNonTimed())
//    {
//        LUnits uxNextPos = m_uxCurPos;
//	    while (CurrentObjectIsNonTimed())
//        {
//            (*m_itCur)->assign_fixed_and_variable_space(m_pColFmt, m_rFactor);
//            (*m_itCur)->reposition_at(uxNextPos);
//
//            uxNextPos += (*m_itCur)->get_total_size();
//            ++m_itCur;
//        }
//
//        //update iteration data and add some additional space after prolog
//        m_uxCurPos = uxNextPos + uSpaceAfterProlog;
//        m_uxRemovable = uSpaceAfterProlog;
//    }
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::ProcessTimedAtCurrentTimepos(LUnits uxPos)
//{
//	//Starting at current position, explores the line to set the position of all timed
//    //objects placed at current time, until we reach a time greater that current
//    //time or end of line
//
//    //update current pos with new xPos required for column alignment
//    m_uxRemovable += uxPos - m_uxCurPos;
//    m_uxCurPos = uxPos;
//
//    DragAnyPreviousCleftToPlaceItNearThisNote();
//
//    //procced to process this timepos
//    LUnits uxRequiredPos = m_uxCurPos + ComputeShiftToAvoidOverlapWithPrevious();
//    LUnits uxNextPos = uxRequiredPos;
//    LUnits uxMinNextPos = 0.0f;
//    LUnits uxMargin = 0.0f;
//    LineEntryIterator itLast;
//	while (ThereAreTimedObjs())
//    {
//        //AssignPositionToCurrentEntry();
//		(*m_itCur)->set_position( uxRequiredPos + (*m_itCur)->get_anchor() );
//
//        //AssignFixedAndVariableSpacingToCurrentEntry();
//        (*m_itCur)->assign_fixed_and_variable_space(m_pColFmt, m_rFactor);
//
//        //DetermineSpaceRequirementsForCurrentEntry();
//        if ((*m_itCur)->is_note_rest())
//		    uxNextPos = wxMax(uxNextPos, (*m_itCur)->m_xFinal);
//        else
//            uxMinNextPos = wxMax(uxMinNextPos, (*m_itCur)->m_xFinal);
//
//        uxMargin = (uxMargin==0.0f ?
//                        (*m_itCur)->m_uVariableSpace
//                        : wxMin(uxMargin, (*m_itCur)->m_uVariableSpace) );
//
//        //AdvanceToNextEntry();
//        itLast = m_itCur++;
//    }
//
//    //update iteration data
//    if (uxNextPos == uxRequiredPos)     //No note/rest found
//        m_uxCurPos = uxRequiredPos + uxMinNextPos;
//    else
//        m_uxCurPos = uxNextPos;
//
//    m_uxRemovable = uxMargin;
//    m_rCurTime = GetNextAvailableTime();
//}
//
////---------------------------------------------------------------------------------------
//LUnits LineSpacer::ComputeShiftToAvoidOverlapWithPrevious()
//{
//	//Starting at current position, explores the objects placed at current time
//    //to check if there is enought removable space to deal with any anchor left shifted
//    //object. If not, computes the required additional space that should be added to
//    //'removable' space.
//
//    LineEntryIterator it = m_itCur;
//    LUnits uxNextPos = m_uxCurPos;
//    LUnits uxShift = 0.0f;
//    const LineEntryIterator itEnd = m_pTable->end();
//	while (it != itEnd && IsEqualTime((*it)->m_rTimePos, m_rCurTime))
//    {
//        LUnits uAnchor = - (*it)->get_anchor();     // > 0 if need to shift left
//        if (uAnchor > 0.0f && m_uxRemovable < uAnchor)
//            uxShift = wxMax(uxShift, uAnchor - m_uxRemovable);
//
//        it++;
//    }
//    return uxShift;
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::ShiftNonTimed(LUnits uxShift)
//{
//    LineEntryIterator it = m_itNonTimedAtCurPos;
//	while (IsNonTimedObject(it))
//    {
//        LUnits uxCurPos = (*it)->get_position();
//        (*it)->reposition_at(uxCurPos + uxShift);
//        ++it;
//    }
//}
//
////---------------------------------------------------------------------------------------
//float LineSpacer::GetNextAvailableTime()
//{
//	LineEntryIterator it = m_itCur;
//    if (it != m_pTable->end())
//    {
//        while (IsNonTimedObject(it))
//            ++it;
//
//        if (IsTimedObject(it))
//            return (*it)->get_timepos();
//        else
//            return lmNO_TIME;
//    }
//    else
//        return lmNO_TIME;
//}
//
////---------------------------------------------------------------------------------------
//void LineSpacer::DragAnyPreviousCleftToPlaceItNearThisNote()
//{
//    if (m_itNonTimedAtCurPos != m_pTable->end() && m_uxCurPos > m_uxNotTimedFinalPos)
//    {
//        ShiftNonTimed(m_uxCurPos - m_uxNotTimedFinalPos);
//    }
//    m_itNonTimedAtCurPos = m_pTable->end();     //no longer needed. Discart value now to avoid problmes at next timepos
//}



//=======================================================================================
//BreakPoints:
//  encloses the algorithm to determine optimum break points to split a column
//=======================================================================================
BreakPoints::BreakPoints(ColumnStorage* pColStorage)
    : m_pColStorage(pColStorage)
    , m_pPossibleBreaks((BreaksTable*)NULL)
    , m_pOptimumEntry((BreaksTimeEntry*)NULL)
{
}

//---------------------------------------------------------------------------------------
BreakPoints::~BreakPoints()
{
//    DeleteBreaksTable();
}

////---------------------------------------------------------------------------------------
//void BreakPoints::DeleteBreaksTable()
//{
//    if (m_pPossibleBreaks)
//    {
//        delete m_pPossibleBreaks;
//        m_pPossibleBreaks = (BreaksTable*)NULL;
//    }
//}
//
////---------------------------------------------------------------------------------------
//bool BreakPoints::FindOptimunBreakPointForSpace(LUnits uAvailable)
//{
//    //returns false if no break point found (exceptional case).
//    //In all other cases updates m_pOptimumEntry and returns true
//
//    if (!m_pPossibleBreaks)
//        ComputeBreaksTable();
//
//    //select highest entry with space <= uAvailable
//    BreaksTimeEntry* pBTE = m_pPossibleBreaks->get_first();
//    m_pOptimumEntry = (BreaksTimeEntry*)NULL;
//    while (pBTE && pBTE->uxEnd <= uAvailable)
//    {
//        m_pOptimumEntry = pBTE;
//        pBTE = m_pPossibleBreaks->get_next();
//    }
//    if (!m_pOptimumEntry)
//        return false;        //big problem: no break points!
//
//    //wxLogMessage(_T("[ColumnLayouter::GetOptimumBreakPoint] uAvailable=%.2f, returned=%.2f, time=%.2f"),
//    //             uAvailable, m_pOptimumEntry->uxEnd, m_pOptimumEntry->rTimepos);
//
//    return true;       //no problems. There are break points
//}
//
////---------------------------------------------------------------------------------------
//float BreakPoints::GetOptimumTimeForFoundBreakPoint()
//{
//    return m_pOptimumEntry->rTimepos;
//}
//
////---------------------------------------------------------------------------------------
//LUnits BreakPoints::GetOptimumPosForFoundBreakPoint()
//{
//    return m_pOptimumEntry->uxEnd;
//}
//
////---------------------------------------------------------------------------------------
//void BreakPoints::ComputeBreaksTable()
//{
//    //This method computes the BreaksTable. This is a table sumarizing break points
//    //information, that is, suitable places through all staves and voices where it is
//    //possible to break a system and start a new one. The best break locations are
//    //usually are the bar lines common to all staves. But in certain rare cases (i.e.
//    //scores without time signature or having instrumens not sharing a common
//    //time signature, or when it is requested to render the score in very narrow
//    //paper, etc.) it is necessary to split music in unnusual points.
//
//    //Step 1. Build a table for each line
//    std::vector<BreaksTable*> partialTables;
//	for (LinesIterator itTL = m_pColStorage->begin(); itTL != m_pColStorage->end(); ++itTL)
//	{
//        BreaksTable* pBT = new BreaksTable();
//        ColumnSplitter oSplitter(*itTL);
//        oSplitter.ComputeBreakPoints(pBT);
//        partialTables.push_back(pBT);
//    }
//
//
//    //Step 2. Combine the partial tables
//    if (m_pPossibleBreaks)
//        DeleteBreaksTable();
//    m_pPossibleBreaks = new BreaksTable();
//
//    std::vector<BreaksTable*>::iterator itBT;
//    for (itBT = partialTables.begin(); itBT != partialTables.end(); ++itBT)
//    {
//        if (m_pPossibleBreaks->is_empty())
//        {
//            //just copy entries
//            BreaksTimeEntry* pEP = (*itBT)->get_first();       //pEP Entry from Partial list
//            while (pEP)
//            {
//                m_pPossibleBreaks->add_entry(pEP);
//                pEP = (*itBT)->get_next();
//            }
//        }
//        else
//        {
//            //merge current table with total table
//            //BreaksTimeEntry* pEP = (*itBT)->get_first();       //pEP Entry from Partial list
//            //while (pEP)
//            //{
//            //    m_pPossibleBreaks->add_entry(pEP);
//            //    pEP = (*itBT)->get_next();
//            //}
//        }
//    }
//
//
//    //Delete partial tables, no longer needed
//    for (itBT = partialTables.begin(); itBT != partialTables.end(); ++itBT)
//        delete *itBT;
//    partialTables.clear();
//
//    //wxLogMessage(_T("Total Breaks Table:"));
//    //wxLogMessage( m_pPossibleBreaks->dump() );
//
//    //Step 3. Sort breaks table by priority and final x position
//    //TODO
//}
//
//
//
//////----------------------------------------------------------------------------------------
////DirtyFlagsCleaner:
////
//////----------------------------------------------------------------------------------------
//
//DirtyFlagsCleaner::DirtyFlagsCleaner(ColumnStorage* pColStorage)
//    : m_pColStorage(pColStorage)
//{
//}
//
////---------------------------------------------------------------------------------------
//void DirtyFlagsCleaner::ClearDirtyFlags()
//{
//    //clear flag 'Dirty' in all StaffObjs of this column. This has nothing to do with
//    //formatting, but its is a convenient place for doing this as all affected objects
//    //are those in the column
//
//    m_pColStorage->ClearDirtyFlags();
//
//	//for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
//	//	(*it)->ClearDirtyFlags();
//}
//
//
//
//
//////----------------------------------------------------------------------------------------
////TimeGridTable:
////  A table with the relation timepos <-> position for all valid positions to insert
////  a note.
////  This object is responsible for supplying all valid timepos and their positions so
////  that other objects (in fact only GmoBoxSlice) could:
////      a) Determine the timepos to assign to a mouse click in a certain position.
////      b) Draw a grid of valid timepos
//////----------------------------------------------------------------------------------------
//
//TimeGridTable::TimeGridTable(ColumnStorage* pColStorage)
//    : m_pColStorage(pColStorage)
//{
//    //build the table
//
//    CreateLineExplorers();
//    while (ThereAreObjects())
//    {
//        SkipNonTimedAtCurrentTimepos();
//        if (TimedObjectsFound())
//        {
//            FindShortestNoteRestAtCurrentTimepos();
//            CreateTableEntry();
//        }
//    }
//    InterpolateMissingTimes();
//    DeleteLineExplorers();
//}
//
////---------------------------------------------------------------------------------------
//TimeGridTable::~TimeGridTable()
//{
//    m_PosTimes.clear();
//}
//
////---------------------------------------------------------------------------------------
//bool TimeGridTable::ThereAreObjects()
//{
//    std::vector<TimeGridLineExplorer*>::iterator it;
//    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
//    {
//        if ((*it)->ThereAreObjects())
//            return true;
//    }
//    return false;
//}
//
////---------------------------------------------------------------------------------------
//void TimeGridTable::CreateTableEntry()
//{
//    PosTimeItem tPosTime = {m_rCurrentTime, m_rMinDuration, m_uCurPos };
//    m_PosTimes.push_back(tPosTime);
//}
//
////---------------------------------------------------------------------------------------
//void TimeGridTable::DeleteLineExplorers()
//{
//    std::vector<TimeGridLineExplorer*>::iterator it;
//    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
//        delete *it;
//    m_LineExplorers.clear();
//}
//
////---------------------------------------------------------------------------------------
//void TimeGridTable::CreateLineExplorers()
//{
//    const LinesIterator itEnd = m_pColStorage->end();
//    for (LinesIterator it=m_pColStorage->begin(); it != itEnd; ++it)
//	{
//        TimeGridLineExplorer* pLinExplorer = new TimeGridLineExplorer(*it);
//        m_LineExplorers.push_back(pLinExplorer);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void TimeGridTable::SkipNonTimedAtCurrentTimepos()
//{
//    m_fTimedObjectsFound = false;
//    std::vector<TimeGridLineExplorer*>::iterator it;
//    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
//	{
//        m_fTimedObjectsFound |= (*it)->SkipNonTimedAtCurrentTimepos();
//    }
//}
//
////---------------------------------------------------------------------------------------
//void TimeGridTable::FindShortestNoteRestAtCurrentTimepos()
//{
//    GetCurrentTime();
//    m_rMinDuration = lmNO_DURATION;
//    m_uCurPos = lmNO_POSITION;
//    std::vector<TimeGridLineExplorer*>::iterator it;
//    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
//	{
//        if (m_rCurrentTime == (*it)->GetCurrentTime())
//        {
//            (*it)->FindShortestNoteRestAtCurrentTimepos();
//            if (m_rMinDuration > (*it)->GetDurationForFoundEntry())
//            {
//                m_rMinDuration = (*it)->GetDurationForFoundEntry();
//                m_uCurPos = wxMin(m_uCurPos, (*it)->GetPositionForFoundEntry());
//            }
//        }
//    }
//}
//
////---------------------------------------------------------------------------------------
//void TimeGridTable::GetCurrentTime()
//{
//    m_rCurrentTime = lmNO_TIME;
//    std::vector<TimeGridLineExplorer*>::iterator it;
//    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
//	{
//        m_rCurrentTime = wxMin(m_rCurrentTime, (*it)->GetCurrentTime());
//    }
//}
//
////---------------------------------------------------------------------------------------
//std::string TimeGridTable::dump()
//{
//                      //   .......+.......+.......+
//    std::string sDump = _T("\n timepos     Dur     Pos\n");
//    std::vector<PosTimeItem>::iterator it;
//    for (it = m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
//    {
//        sDump += std::string::Format(_T("%8.2f %8.2f %8.2f\n"),
//                                (*it).rTimepos, (*it).rDuration, (*it).uxPos );
//    }
//    return sDump;
//}
//
////---------------------------------------------------------------------------------------
//float TimeGridTable::GetTimeForPosititon(LUnits uxPos)
//{
//    //timepos = 0 if measure is empty
//    if (m_PosTimes.size() == 0)
//        return 0.0f;
//
//    //timepos = 0 if xPos < first entry xPos
//    float rTime = 0.0f;
//    LUnits uxPrev = m_PosTimes.front().uxPos;
//    if (uxPos <= uxPrev)
//        return rTime;
//
//    //otherwise find in table
//    std::vector<PosTimeItem>::iterator it = m_PosTimes.begin();
//    for (++it; it != m_PosTimes.end(); ++it)
//    {
//        int uxLimit = uxPrev + ((*it).uxPos - uxPrev) / 2.0;
//        if (uxPos <= uxLimit)
//            return rTime;
//        uxPrev = (*it).uxPos;
//        rTime = (*it).rTimepos;
//    }
//
//    //if not found return last entry timepos
//    return m_PosTimes.back().rTimepos;
//}
//
////---------------------------------------------------------------------------------------
//void TimeGridTable::InterpolateMissingTimes()
//{
//    TimeInserter oInserter(m_PosTimes);
//    oInserter.InterpolateMissingTimes();
//}
//
//
//
//////----------------------------------------------------------------------------------------
////TimeInserter
//// helper class to interpolate missing entries
//////----------------------------------------------------------------------------------------
//
//TimeInserter::TimeInserter(std::vector<PosTimeItem>& oPosTimes)
//    : m_PosTimes(oPosTimes)
//{
//}
//
////---------------------------------------------------------------------------------------
//void TimeInserter::InterpolateMissingTimes()
//{
//    for (int i=0; i < (int)m_PosTimes.size(); ++i)
//    {
//        float rNextTime = m_PosTimes[i].rTimepos + m_PosTimes[i].rDuration;
//        if (!IsTimeInTable(rNextTime))
//        {
//            FindInsertionPoint(rNextTime);
//            InsertTimeInterpolatingPosition(rNextTime);
//        }
//    }
//}
//
////---------------------------------------------------------------------------------------
//bool TimeInserter::IsTimeInTable(float rTimepos)
//{
//    if (m_PosTimes.size() == 0)
//        return false;
//
//    std::vector<PosTimeItem>::iterator it;
//    for (it=m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
//    {
//        if (IsEqualTime(rTimepos, (*it).rTimepos))
//            return true;
//    }
//    return false;
//}
//
////---------------------------------------------------------------------------------------
//void TimeInserter::FindInsertionPoint(float rTimepos)
//{
//    m_uPositionBeforeInsertionPoint = m_PosTimes.front().uxPos;
//    m_rTimeBeforeInsertionPoint = m_PosTimes.front().rTimepos;
//
//    std::vector<PosTimeItem>::iterator it;
//    for (it=m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
//    {
//        if (IsHigherTime((*it).rTimepos, rTimepos))
//            break;
//        m_uPositionBeforeInsertionPoint = (*it).uxPos;
//        m_rTimeBeforeInsertionPoint = (*it).rTimepos;
//    }
//    m_itInsertionPoint = it;
//}
//
////---------------------------------------------------------------------------------------
//void TimeInserter::InsertTimeInterpolatingPosition(float rTimepos)
//{
//    PosTimeItem oItem;
//    oItem.rTimepos = rTimepos;
//    oItem.rDuration = 0.0f;
//    oItem.uxPos = m_uPositionBeforeInsertionPoint;
//
//    if (m_itInsertionPoint == m_PosTimes.end())
//    {
//        //insert at the end
//        oItem.uxPos += 1000;       //TODO: Estimate space based on measure duration
//        m_PosTimes.push_back(oItem);
//    }
//    else
//    {
//        //insert before item pointed by iterator
//        float rTimeGap = (*m_itInsertionPoint).rTimepos - m_rTimeBeforeInsertionPoint;
//        float rPosGap = (*m_itInsertionPoint).uxPos - m_uPositionBeforeInsertionPoint;
//        float rTimeIncrement = rTimepos - m_rTimeBeforeInsertionPoint;
//        oItem.uxPos += rTimeIncrement * (rPosGap / rTimeGap);
//        m_PosTimes.insert(m_itInsertionPoint, oItem);
//    }
//}
//
//
//////----------------------------------------------------------------------------------------
////TimeGridLineExplorer:
////  line traversal algorithm for creating the time-pos table
//////----------------------------------------------------------------------------------------
//
//TimeGridLineExplorer::TimeGridLineExplorer(LineTable* pLineTable)
//    : m_pTable(pLineTable)
//{
//    m_itCur = m_pTable->begin();
//}
//
////---------------------------------------------------------------------------------------
//TimeGridLineExplorer::~TimeGridLineExplorer()
//{
//}
//
////---------------------------------------------------------------------------------------
//bool TimeGridLineExplorer::SkipNonTimedAtCurrentTimepos()
//{
//    //returns true if there are timed objects after the skipped non-timed
//
//	while (CurrentObjectIsNonTimed())
//        ++m_itCur;
//
//    return CurrentObjectIsTimed();
//}
//
////---------------------------------------------------------------------------------------
//bool TimeGridLineExplorer::FindShortestNoteRestAtCurrentTimepos()
//{
//    //returns true if there are more objects after current timepos
//
//	if (CurrentObjectIsTimed())
//    {
//        m_rCurTime = (*m_itCur)->get_timepos();
//        m_uCurPos = (*m_itCur)->get_position() - (*m_itCur)->get_anchor();
//        m_rMinDuration = (*m_itCur)->get_duration();
//        m_uShiftToNoteRestCenter = (*m_itCur)->get_shift_to_noterest_center();
//
//	    while (CurrentObjectIsTimed() && (*m_itCur)->get_timepos() == m_rCurTime)
//        {
//            m_rMinDuration = wxMin(m_rMinDuration, (*m_itCur)->get_duration());
//            if (m_uShiftToNoteRestCenter == 0.0f)
//                m_uShiftToNoteRestCenter = (*m_itCur)->get_shift_to_noterest_center();
//
//            ++m_itCur;
//        }
//    }
//    return ThereAreObjects();
//}
//
////---------------------------------------------------------------------------------------
//float TimeGridLineExplorer::GetCurrentTime()
//{
//    if (CurrentObjectIsTimed())
//        return (*m_itCur)->get_timepos();
//    else
//        return lmNO_TIME;
//}
//
////---------------------------------------------------------------------------------------
//float TimeGridLineExplorer::GetDurationForFoundEntry()
//{
//    return m_rMinDuration;
//}
//
////---------------------------------------------------------------------------------------
//LUnits TimeGridLineExplorer::GetPositionForFoundEntry()
//{
//    return m_uCurPos + m_uShiftToNoteRestCenter;
//}
//
//
//
//////----------------------------------------------------------------------------------------
////ColumnResizer: encapsulates the methods to recompute shapes positions so that the
////column will have the desired width, and to move the shapes to those positions
//////----------------------------------------------------------------------------------------
//
//ColumnResizer::ColumnResizer(ColumnStorage* pColStorage, LUnits uNewBarSize)
//    : m_pColStorage(pColStorage)
//    , m_uNewBarSize(uNewBarSize)
//{
//}
//
////---------------------------------------------------------------------------------------
//void ColumnResizer::RepositionShapes(LUnits uNewStart)
//{
//    m_uNewStart = uNewStart;
//    m_uOldBarSize = m_pColStorage->GetColumnWitdh();
//
//    CreateLineResizers();
//    MovePrologShapesAndGetInitialTime();
//    DetermineFixedSizeAtStartOfColumn();
//    RepositionAllOtherShapes();
//    DeleteLineResizers();
//}
//
////---------------------------------------------------------------------------------------
//void ColumnResizer::CreateLineResizers()
//{
//	for (LinesIterator it=m_pColStorage->begin(); it != m_pColStorage->end(); ++it)
//	{
//        LineResizer* pResizer = new LineResizer(*it, m_uOldBarSize, m_uNewBarSize,
//                                                    m_uNewStart);
//        m_LineResizers.push_back(pResizer);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void ColumnResizer::MovePrologShapesAndGetInitialTime()
//{
//    m_rFirstTime = lmNO_TIME;
//    std::vector<LineResizer*>::iterator itR;
//	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
//	{
//        m_rFirstTime = wxMin(m_rFirstTime, (*itR)->MovePrologShapes());
//    }
//}
//
////---------------------------------------------------------------------------------------
//void ColumnResizer::DetermineFixedSizeAtStartOfColumn()
//{
//    m_uFixedPart = 0.0f;
//    std::vector<LineResizer*>::iterator itR;
//	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
//	{
//        m_uFixedPart = wxMax(m_uFixedPart, (*itR)->GetTimeLinePositionIfTimeIs(m_rFirstTime));
//    }
//}
//
////---------------------------------------------------------------------------------------
//void ColumnResizer::RepositionAllOtherShapes()
//{
//    std::vector<LineResizer*>::iterator itR;
//	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
//		(*itR)->ReassignPositionToAllOtherObjects(m_uFixedPart);
//}
//
////---------------------------------------------------------------------------------------
//void ColumnResizer::DeleteLineResizers()
//{
//    std::vector<LineResizer*>::iterator itR;
//	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
//		delete *itR;
//    m_LineResizers.clear();
//}


}  //namespace lomse
