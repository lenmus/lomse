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

#include "lomse_system_layouter.h"

#include "lomse_box_slice.h"
#include "lomse_box_slice_instr.h"
#include "lomse_engraving_options.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_shape_note.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <math.h>
using namespace std;

//to save tables into a file, for debugging
#define LOMSE_DUMP_TABLES   1

#if (LOMSE_DUMP_TABLES)
    ofstream m_debugFile;
#endif


namespace lomse
{

//forward declarations
class BreaksTable;


#define LOMSE_NO_DURATION   100000000000.0f     //any impossible high value
#define LOMSE_NO_TIME       100000000000.0f     //any impossible high value
#define LOMSE_NO_POSITION   100000000000.0f     //any impossible high value



//=======================================================================================
//BreaksTable implementation
//=======================================================================================
BreaksTable::BreaksTable()
{
}

//---------------------------------------------------------------------------------------
BreaksTable::~BreaksTable()
{
    std::list<BreaksTimeEntry*>::iterator it;
    for (it = m_BreaksTable.begin(); it != m_BreaksTable.end(); ++it)
        delete *it;
    m_BreaksTable.clear();
}

//---------------------------------------------------------------------------------------
void BreaksTable::add_entry(float rTime, LUnits uxStart, LUnits uWidth, bool fInBeam,
                             LUnits uxBeam, float rPriority)
{
    BreaksTimeEntry* pBTE = new BreaksTimeEntry;
    pBTE->rPriority = rPriority;
    pBTE->rTimepos = rTime;
    pBTE->uxStart = uxStart;
    pBTE->uxEnd = uxStart + uWidth;
    pBTE->fInBeam = fInBeam;
    pBTE->uxBeam = uxBeam;

    m_BreaksTable.push_back(pBTE);
}

//---------------------------------------------------------------------------------------
void BreaksTable::add_entry(BreaksTimeEntry* pBTE)
{
    add_entry(pBTE->rTimepos, pBTE->uxStart, pBTE->uxEnd - pBTE->uxStart, pBTE->fInBeam,
             pBTE->uxBeam, pBTE->rPriority);
}

//---------------------------------------------------------------------------------------
void BreaksTable::change_priority(int iEntry, float rMultiplier)
{
}

////---------------------------------------------------------------------------------------
//std::string BreaksTable::dump()
//{
//#if (LOMSE_DUMP_TABLES)

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
//#endif
//}

//---------------------------------------------------------------------------------------
BreaksTimeEntry* BreaksTable::get_first()
{
    m_it = m_BreaksTable.begin();
    if (m_it == m_BreaksTable.end())
        return (BreaksTimeEntry*)NULL;

    return *m_it;
}

//---------------------------------------------------------------------------------------
BreaksTimeEntry* BreaksTable::get_next()
{
    ++m_it;
    if (m_it != m_BreaksTable.end())
        return *m_it;

    return (BreaksTimeEntry*)NULL;
}



//=====================================================================================
//LineEntry implementation
//=====================================================================================
LineEntry::LineEntry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog, float rTime)
    : m_fIsBarlineEntry(false)
    , m_pSO(pSO)
    , m_pShape(pShape)
	, m_fProlog(fProlog)
    , m_rTimePos(rTime)
    , m_xLeft(pShape ? pShape->get_left() : 0.0f )
    , m_uxAnchor(0.0f)     //TODO (pSO && pSO->is_note()) ? - pSO->GetAnchorPos(): 0.0f )
    , m_xFinal(0.0f)
    , m_uSize(pShape ? pShape->get_width() : 0.0f )
    , m_uFixedSpace(0.0f)
    , m_uVariableSpace(0.0f)
{
}

//---------------------------------------------------------------------------------------
bool LineEntry::is_note_rest()
{
    return m_pSO && m_pSO->is_note_rest();
}

//---------------------------------------------------------------------------------------
bool LineEntry::has_barline()
{
    return m_pSO && m_pSO->is_barline();
}

//---------------------------------------------------------------------------------------
void LineEntry::reposition_at(LUnits uxNewXLeft)
{
    m_xLeft = uxNewXLeft;
    m_xFinal = m_xLeft + get_total_size();
}

//---------------------------------------------------------------------------------------
void LineEntry::move_shape()
{
    if (m_pSO && m_pShape)
    {
        //TODO
        //LUnits uShift = m_xLeft - m_pShape->get_left();
//        m_pSO->StoreOriginAndShiftShapes( uShift, m_pShape->GetOwnerIDX() );
        m_pShape->set_left(m_xLeft);
    }
}

//---------------------------------------------------------------------------------------
void LineEntry::dump_header()
{
#if (LOMSE_DUMP_TABLES)
    //              ...+  ..+   ...+ ..+   +  ..........+........+........+........+........+........+........+........+......+
    m_debugFile << "item    Type      ID Prolog   Timepos  xAnchor    xLeft     size  SpFixed    SpVar    Space   xFinal ShpIdx\n";
#endif
}

//---------------------------------------------------------------------------------------
LUnits LineEntry::get_shift_to_noterest_center()
{
    if (m_pSO && m_pSO->is_note_rest())
    {
        //determine notehead width or rest width
        LUnits uxWidth = 0.0f;
        if (m_pSO->is_rest())
            uxWidth = m_pShape->get_width();
        else
            uxWidth = (dynamic_cast<GmoShapeNote*>(m_pShape))->get_notehead_width();

        return uxWidth / 2.0f;
    }
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
void LineEntry::dump(int iEntry)
{
#if (LOMSE_DUMP_TABLES)
    m_debugFile << setw(4) << iEntry << ": ";      //"%4d: "
    if (m_fIsBarlineEntry)
    {
        m_debugFile << "  Omega";
        if (m_pSO)
            m_debugFile << setw(4) << m_pSO->get_obj_type();
        else
            m_debugFile << "  - ";
        m_debugFile << "         ";
    }
    else
    {
		m_debugFile << "  pSO "
					<< setw(4) << m_pSO->get_obj_type()
					<< setw(4) << m_pSO->get_id()
					<< (m_fProlog ? "   S  " : "      ");
    }

    m_debugFile << fixed << setprecision(2) << setfill(' ')
                << setw(11) << m_rTimePos
                << setw(9) << m_uxAnchor
                << setw(9) << m_xLeft
                << setw(9) << m_uSize
                << setw(9) << m_uFixedSpace
                << setw(9) << m_uVariableSpace
                << setw(9) << get_total_size()
                << setw(9) << m_xFinal;

    //if (m_pShape)
    //    m_debugFile << "  " << setw(4) << m_pShape->GetOwnerIDX() << "\n";
    //else
        m_debugFile << "    --\n";

#endif
}




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

//---------------------------------------------------------------------------------------
void ColumnSplitter::compute_break_points(BreaksTable* pBT)
{
    //This method computes the break points for this line and adds them to received
    //break points table.
    //
    //Algorithm:
    //
    //In a first approach, add an entry for each timepos at which there is an object placed.
    //Assign priority 0.8 to all entries.
    //
    //Now lower or raise priority of some entries according to empiric rules:
    //
    //  1. If there is a time signature, strongly penalize those timepos not in beat
    //     position (priority *= 0.5)
    //
    //  2. Do not split notes/rests. Penalize those entries occupied in some
    //     line (priority *= 0.7).
    //
    //  3. Do not to break beams. Penalize those entries  in which, at some line, there
    //     is a beam (priority *=0.9).
    //
    //Finally, when all priorities have been computed, sort the table by priority (high to
    //low) and by space (max to min).
    //
    //In order to accelerate the computation of this table, LineTables must have all
    //necesary data so that it doesn't become necessary to traverse the StaffObjs
    //colection again.

    //TODO: Add filters for priority
    const LineEntryIterator itEnd = m_pLineTable->end();
	LineEntryIterator it = m_pLineTable->begin();

    //skip initial non-timed entries
	for (it = m_pLineTable->begin(); it != itEnd && is_lower_time((*it)->get_timepos(), 0.0f); ++it);
    if (it == itEnd) return;

    //process current time
    float rTime = (*it)->get_timepos();
    LUnits uxStart = (*it)->get_position();
    LUnits uxWidth = (*it)->get_shape_size();
    LUnits uxBeam = 0.0f;
    bool fInBeam = false;
    ImoStaffObj* pSO = (*it)->get_staffobj();
    if (pSO && pSO->is_note_rest() && static_cast<ImoNoteRest*>(pSO)->is_beamed())
    {
        fInBeam = true;
        //TODO
        //ImoStaffObj* pSOEnd = static_cast<ImoNoteRest*>(pSO)->GetBeam()->GetEndNoteRest();
        //GmoShape* pShape = pSOEnd->get_shape();
        //uxBeam = pShape->get_left() + pShape->get_width();
    }

	while (it != itEnd)
    {
		if (is_equal_time((*it)->get_timepos(), rTime) || is_lower_time((*it)->get_timepos(), 0.0f))
        {
		    //skip any not-timed entry
            if (is_equal_time((*it)->get_timepos(), rTime))
            {
                uxWidth = max(uxWidth, (*it)->get_shape_size());
                //TODO
//                ImoStaffObj* pSO = (*it)->m_pSO;
                //if (pSO && pSO->is_note_rest() && static_cast<ImoNoteRest*>(pSO)->is_beamed())
                //{
                //    fInBeam = true;
                //    ImoStaffObj* pSOEnd = (static_cast<ImoNoteRest*>(pSO)->GetBeam()->GetEndNoteRest();
                //    GmoShape* pShape = pSOEnd->get_shape();
                //    uxBeam = max(uxBeam, pShape->get_left() + pShape->get_width());
                //}
            }
        }
        else
        {
            //new timepos. Add entry for previous timepos
            pBT->add_entry(rTime, uxStart, uxWidth, fInBeam, uxBeam);

            //start collecting data for new timepos
            rTime = (*it)->get_timepos();
            uxStart = (*it)->get_position();
            uxWidth = (*it)->get_shape_size();
            ImoStaffObj* pSO = (*it)->get_staffobj();
            if (pSO && pSO->is_note_rest() && static_cast<ImoNoteRest*>(pSO)->is_beamed())
            {
                fInBeam = true;
                //TODO
                //ImoStaffObj* pSOEnd = static_cast<ImoNoteRest*>(pSO)->GetBeam()->GetEndNoteRest();
                //GmoShape* pShape = pSOEnd->get_shape();
                //uxBeam = pShape->get_left() + pShape->get_width();
            }
            else
            {
                uxBeam = 0.0f;
                fInBeam = false;
            }
       }
		++it;
    }

    pBT->add_entry(rTime, uxStart, uxWidth, fInBeam, uxBeam);

    //cout << pBT->dump() );
}



//=======================================================================================
//LineTable:
//  An object to encapsulate positioning data for a line
//=======================================================================================
LineTable::LineTable(int line, int nInstr, LUnits uxStart, LUnits uSpace)
    : m_line(line)
    , m_nInstr(nInstr)
	, m_nVoice(line+1)
    , m_uxLineStart(uxStart)
    , m_uInitialSpace(uSpace)
{
}

//---------------------------------------------------------------------------------------
LineTable::~LineTable()
{
    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
		delete *it;
    m_LineEntries.clear();
}

//---------------------------------------------------------------------------------------
LineEntry* LineTable::add_entry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog,
                                float rTime)
{
    LineEntry* pEntry = new LineEntry(pSO, pShape, fProlog, rTime);
    push_back(pEntry);
	return pEntry;
}

//---------------------------------------------------------------------------------------
LineEntry* LineTable::add_final_entry(ImoStaffObj* pSO, GmoShape* pShape, float rTime)
{
    LineEntry* pEntry = new LineEntry(pSO, pShape, false, rTime);
    pEntry->mark_as_barline_entry();
    push_back(pEntry);
	return pEntry;
}

//---------------------------------------------------------------------------------------
void LineTable::add_shapes(GmoBoxSliceInstr* pSliceInstrBox)
{
    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
    {
        if ((*it)->get_shape())
            pSliceInstrBox->add_shape((*it)->get_shape(), GmoShape::k_layer_notes);
    }
}

//---------------------------------------------------------------------------------------
bool LineTable::contains_barline()
{
    LineEntry* pEntry = get_last_entry();
    return pEntry->is_barline_entry() && pEntry->has_barline();
}

//---------------------------------------------------------------------------------------
void LineTable::dump_main_table()
{
#if (LOMSE_DUMP_TABLES)

    m_debugFile << fixed << setprecision(2) << setfill(' ')
                << "Line table dump. Instr=" << get_instrument()
                << ", voice=" << get_voice()
                << ", xStart=" << setw(2) << get_line_start_position()
                << ", initSpace=" << setw(2) << get_space_at_beginning()
                << "\n"
                << "===================================================================\n\n";

    if (size() == 0)
    {
        m_debugFile << "The table is empty.";
        return;
    }

    //headers
    LineEntry::dump_header();

    //loop to dump table entries
    for (int i = 0; i < (int)size(); i++)
    {
        if (i % 4 == 0) {
            m_debugFile << "----------------------------------------------------------------------------\n";
        }
        LineEntry* pTE = item(i);
        pTE->dump(i);
    }

    m_debugFile << "=== end of table ==================================================\n\n";
#endif
}

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

//---------------------------------------------------------------------------------------
LUnits LineTable::get_line_width()
{
	//Return the size of the measure represented by this line or zero if invalid line

	if (m_LineEntries.size() > 0 && m_LineEntries.back()->is_barline_entry())
        return m_LineEntries.back()->get_x_final() - get_line_start_position();
    else
        return 0.0f;
}


//=======================================================================================
//ColumnLayouter
//=======================================================================================
ColumnLayouter::ColumnLayouter(ColumnStorage* pStorage, ScoreMeter* pScoreMeter)
    : m_pColStorage(pStorage)
    , m_pScoreMeter(pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
ColumnLayouter::~ColumnLayouter()
{
    delete_line_spacers();
}

//---------------------------------------------------------------------------------------
LUnits ColumnLayouter::tenths_to_logical(Tenths value, int iInstr, int iStaff)
{
    return m_pScoreMeter->tenths_to_logical(value, iInstr, iStaff);
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::delete_line_spacers()
{
    LineSpacersIterator it;
    for (it = m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
        delete *it;
    m_LineSpacers.clear();
}

//---------------------------------------------------------------------------------------
bool ColumnLayouter::is_there_barline()
{
    //returns true if there is at least one line containing a barline

    for (LinesIterator it=m_pColStorage->begin(); it != m_pColStorage->end(); ++it)
    {
        if ((*it)->contains_barline())
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::do_spacing(bool fTrace)
{
    //computes the minimum space required by this column

#if LOMSE_DUMP_TABLES
    m_pColStorage->dump_column_storage();
#endif

    m_uMinColumnSize = compute_spacing();

#if LOMSE_DUMP_TABLES
    m_pColStorage->dump_column_storage();
#endif
}

//---------------------------------------------------------------------------------------
LUnits ColumnLayouter::compute_spacing()
{
    //Spacing algorithm. Returns the resulting minimum column width

    if (there_are_lines())
    {
        create_line_spacers();
        process_non_timed_at_prolog();
        process_timed_at_current_timepos();
        while (there_are_objects())
        {
            process_non_timed_at_current_timepos();
            process_timed_at_current_timepos();
        }

        delete_line_spacers();
    }
	return m_pColStorage->get_column_width();
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::create_line_spacers()
{
    const LinesIterator itEnd = m_pColStorage->end();
    for (LinesIterator it=m_pColStorage->begin(); it != itEnd; ++it)
	{
        LineSpacer* pLinSpacer
            = new LineSpacer(*it, this, m_pScoreMeter->get_spacing_factor());
        m_LineSpacers.push_back(pLinSpacer);
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_non_timed_at_prolog()
{
    LUnits uSpaceAfterProlog = tenths_to_logical(LOMSE_SPACE_AFTER_PROLOG, 0, 0);
    m_rCurrentTime = LOMSE_NO_TIME;           //any impossible high value
    m_rCurrentPos = 0.0f;
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        (*it)->process_non_timed_at_prolog(uSpaceAfterProlog);
        LUnits uxNextPos = (*it)->get_next_position();
        m_rCurrentTime = min(m_rCurrentTime, (*it)->get_next_available_time());
        m_rCurrentPos = max(m_rCurrentPos, uxNextPos);
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_timed_at_current_timepos()
{
    m_fThereAreObjects = false;
    float rNextTime = LOMSE_NO_TIME;           //any impossible high value
    LUnits uxPosForNextTime = LOMSE_NO_POSITION;    //any impossible high value
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        if ((*it)->current_time_is(m_rCurrentTime) && (*it)->are_there_timed_objs())
        {
            (*it)->process_timed_at_current_timepos(m_rCurrentPos);
            LUnits uxNextPos = (*it)->get_next_position();
            uxPosForNextTime = min(uxPosForNextTime, uxNextPos);
        }
        if ((*it)->are_there_more_objects())
        {
            m_fThereAreObjects = true;
            rNextTime = min(rNextTime, (*it)->get_next_available_time());
        }
    }

    m_rCurrentTime = rNextTime;
    if (uxPosForNextTime < LOMSE_NO_POSITION)
        m_rCurrentPos = uxPosForNextTime;
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_non_timed_at_current_timepos()
{
    LUnits uxPosForNextTime = 0.0f;
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        (*it)->process_non_timed_at_current_timepos(m_rCurrentPos);
        LUnits uxNextPos = (*it)->get_next_position();
        uxPosForNextTime = max(uxPosForNextTime, uxNextPos);
    }
    m_rCurrentPos = uxPosForNextTime;
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr* ColumnLayouter::create_slice_instr(ImoInstrument* pInstr, LUnits yTop)
{
    GmoBoxSliceInstr* pBSI = m_pBoxSlice->add_box_for_instrument(pInstr);
	pBSI->set_top(yTop);
	pBSI->set_left( m_pBoxSlice->get_left() );
	pBSI->set_width( m_pBoxSlice->get_width() );
    m_sliceInstrBoxes.push_back( pBSI );
    return pBSI;
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::add_shapes_to_boxes()
{
    for (int iInstr=0; iInstr < int(m_sliceInstrBoxes.size()); ++iInstr)
        m_pColStorage->add_shapes( m_sliceInstrBoxes[iInstr], iInstr);
}




//=======================================================================================
// SystemLayouter implementation
//=======================================================================================
SystemLayouter::SystemLayouter(ScoreMeter* pScoreMeter)
    : m_pScoreMeter(pScoreMeter)
    , m_uPrologWidth(0.0f)
{
#if (LOMSE_DUMP_TABLES)
    m_debugFile.open("dbg_tables.txt", fstream::out | fstream::app);
#endif
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

#if (LOMSE_DUMP_TABLES)
    m_debugFile.close();
#endif
}

//---------------------------------------------------------------------------------------
void SystemLayouter::prepare_for_new_column(GmoBoxSlice* pBoxSlice)
{
    //create storage for this column
    ColumnStorage* pStorage = new ColumnStorage();
    m_ColStorage.push_back(pStorage);

    //create a lines builder object for this column
    LinesBuilder* pLB = new LinesBuilder(pStorage);
    m_LinesBuilder.push_back(pLB);

    //create the column layouter object
    ColumnLayouter* pColLyt = new ColumnLayouter(pStorage, m_pScoreMeter);
    pColLyt->set_slice_box(pBoxSlice);
    m_ColLayouters.push_back(pColLyt);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::end_of_system_measurements()
{
    //caller informs that all data for this system has been suplied.
    //This is the right place to do any preparatory work, not to be repeated if re-spacing.

    //Nothing to do for current implementation
}

//---------------------------------------------------------------------------------------
void SystemLayouter::start_bar_measurements(int iCol, LUnits uxStart, LUnits uSpace)
{
    //prepare to receive data for a new bar in column iCol [0..n-1].

    LinesBuilder* pLB = m_LinesBuilder[iCol];
    pLB->set_start_position(uxStart);
    pLB->set_initial_space(uSpace);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::include_object(int iCol, int iLine, int iInstr, ImoInstrument* pInstr,
                                    ImoStaffObj* pSO, float rTime, bool fProlog,
                                    int nStaff, GmoShape* pShape)
{
    //caller sends data about one staffobj in current bar, for column iCol [0..n-1]

    m_LinesBuilder[iCol]->include_object(iLine, iInstr, pInstr, pSO, rTime,
                                         fProlog, nStaff, pShape);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::include_barline_and_terminate_bar_measurements(int iCol,
                                                                    ImoStaffObj* pSO,
                                                                    GmoShape* pShape,
                                                                    LUnits xStart,
                                                                    float rTime)
{
    //caller sends lasts object to store in current bar, for column iCol [0..n-1].

    m_LinesBuilder[iCol]->close_line(pSO, pShape, xStart, rTime);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::terminate_bar_measurements_without_barline(int iCol, LUnits xStart,
                                                                float rTime)
{
    //caller informs that there are no barline and no more objects in column iCol [0..n-1].

    m_LinesBuilder[iCol]->close_line(NULL, NULL, xStart, rTime);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::discard_measurements_for_column(int iCol)
{
    //caller request to ignore measurements for column iCol [0..n-1]

    m_ColStorage[iCol]->initialize();
    m_ColLayouters[iCol]->initialize();
    //m_LinesBuilder[iCol]->initialize();
}

//---------------------------------------------------------------------------------------
void SystemLayouter::do_column_spacing(int iCol, bool fTrace)
{
    m_ColLayouters[iCol]->do_spacing(fTrace);
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::redistribute_space(int iCol, LUnits uNewStart)
{
    LUnits uNewBarSize = m_ColLayouters[iCol]->get_minimum_size();
    ColumnResizer oResizer(m_ColStorage[iCol], uNewBarSize);
	oResizer.reposition_shapes(uNewStart);

    LUnits uBarFinalPosition = uNewStart + uNewBarSize;
    return uBarFinalPosition;
}

////---------------------------------------------------------------------------------------
//void SystemLayouter::AddTimeGridToBoxSlice(int iCol, GmoBoxSlice* pBSlice)
//{
//    //create the time-grid table and transfer it (and its ownership) to GmoBoxSlice
//    pBSlice->SetTimeGridTable( new TimeGridTable(m_ColStorage[iCol]) );
//}

//---------------------------------------------------------------------------------------
void SystemLayouter::increment_column_size(int iCol, LUnits uIncr)
{
    m_ColLayouters[iCol]->increment_column_size(uIncr);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_shapes_to_column(int iCol)
{
    m_ColLayouters[iCol]->add_shapes_to_boxes();
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::get_start_position_for_column(int iCol)
{
    return m_ColStorage[iCol]->get_start_of_bar_position();
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::get_minimum_size(int iCol)
{
    return m_ColLayouters[iCol]->get_minimum_size();
}

//---------------------------------------------------------------------------------------
bool SystemLayouter::get_optimum_break_point(int iCol, LUnits uAvailable,
                                        float* prTime, LUnits* puWidth)
{
    //return m_ColLayouters[iCol]->get_optimum_break_point(uAvailable, prTime, puWidth);
    BreakPoints oBreakPoints(m_ColStorage[iCol]);
    if (oBreakPoints.find_optimum_break_point_for_space(uAvailable))
    {
        *prTime = oBreakPoints.get_optimum_time_for_found_break_point();
        *puWidth = oBreakPoints.get_optimum_position_for_break_point();
        return false;
    }
    else
        return true;
}

//---------------------------------------------------------------------------------------
bool SystemLayouter::column_has_barline(int iCol)
{
    return m_ColLayouters[iCol]->is_there_barline();
}

////---------------------------------------------------------------------------------------
//void SystemLayouter::ClearDirtyFlags(int iCol)
//{
//    m_ColStorage[iCol]->ClearDirtyFlags();
//}

//---------------------------------------------------------------------------------------
void SystemLayouter::dump_column_data(int iCol)
{
#if (LOMSE_DUMP_TABLES)
    m_ColStorage[iCol]->dump_column_storage();
#endif
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr* SystemLayouter::create_slice_instr(int iCol,
                                                     ImoInstrument* pInstr,
                                                     LUnits yTop)
{
    return m_ColLayouters[iCol]->create_slice_instr(pInstr, yTop);
}


////------------------------------------------------
//// Debug build: methods coded only for Unit Tests
////------------------------------------------------
//#if defined(_LM_DEBUG_)
//
//int SystemLayouter::get_num_objects_in_column_line(int iCol, int iLine)
//{
//    //iCol, iLine = [0..n-1]
//    return m_ColStorage[iCol]->get_num_objects_in_line(iLine);
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
    delete_lines();
}

//---------------------------------------------------------------------------------------
void ColumnStorage::initialize()
{
    delete_lines();
}

//---------------------------------------------------------------------------------------
void ColumnStorage::delete_lines()
{
	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
	{
		delete *it;
	}
	m_Lines.clear();
}

//---------------------------------------------------------------------------------------
LinesIterator ColumnStorage::find_line(int line)
{
	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
    {
		if ((*it)->is_for_line(line))
            return it;
	}
    return m_Lines.end();
}

////---------------------------------------------------------------------------------------
//LinesIterator ColumnStorage::FindLineForInstrAndVoice(int nInstr, int nVoice)
//{
//    //return m_pColStorage->FindLineForInstrAndVoice(nInstr, nVoice);
//	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
//    {
//		if ((*it)->is_line_for_instrument(nInstr) && (*it)->is_line_for_voice(nVoice) )
//            return it;
//	}
//    return m_Lines.end();
//}

//---------------------------------------------------------------------------------------
LineTable* ColumnStorage::open_new_line(int line, int instr, LUnits uxStart, LUnits uSpace)
{
    LineTable* pLineTable = new LineTable(line, instr, uxStart, uSpace);
    m_Lines.push_back(pLineTable);
    return pLineTable;
}

//---------------------------------------------------------------------------------------
void ColumnStorage::dump_column_storage()
{
#if (LOMSE_DUMP_TABLES)
    m_debugFile << "Start of dump. ColumnStorage" <<endl;
	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
	{
        (*it)->dump_main_table();
    }
#endif
}

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

//---------------------------------------------------------------------------------------
LUnits ColumnStorage::get_column_width()
{
    LUnits uColWidth = 0;
	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
        uColWidth = max(uColWidth, (*it)->get_line_width());

    return uColWidth;
}

//---------------------------------------------------------------------------------------
LUnits ColumnStorage::get_start_of_bar_position()
{
    //returns the x position for the start of the bar column

    return m_Lines.front()->get_line_start_position();
}

//---------------------------------------------------------------------------------------
void ColumnStorage::add_shapes(GmoBoxSliceInstr* pSliceInstrBox, int iInstr)
{
	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
    {
        if ((*it)->get_instrument() == iInstr)
            (*it)->add_shapes(pSliceInstrBox);
    }
}



//=======================================================================================
// LinesBuilder implementation: encapsulates the algorithms to split a column
// into lines and to store them in the received column storage
//=======================================================================================
LinesBuilder::LinesBuilder(ColumnStorage* pStorage)
    : m_pColStorage(pStorage)
	, m_pCurEntry((LineEntry*)NULL)
{
    //initialize();
}

//---------------------------------------------------------------------------------------
LinesBuilder::~LinesBuilder()
{
}

////---------------------------------------------------------------------------------------
//void LinesBuilder::initialize()
//{
//    reset_default_voices();
//}
//
////---------------------------------------------------------------------------------------
//void LinesBuilder::reset_default_voices()
//{
//    for(size_t i=0; i < m_nStaffVoice.size(); i++)
//        m_nStaffVoice[i] = 0;
//}

////---------------------------------------------------------------------------------------
//void LinesBuilder::start_measurements_for_instrument(int nInstr, LUnits uxStart,
//                                                      ImoInstrument* pInstr, LUnits uSpace)
//{
//    create_lines_for_each_staff(nInstr, uxStart, pInstr, uSpace);
//}

////---------------------------------------------------------------------------------------
//void LinesBuilder::create_lines_for_each_staff(int nInstr, LUnits uxStart,
//                                               ImoInstrument* pInstr, LUnits uSpace)
//{
//    //We need at least one line for each staff, for the music on each staff.
//    //As we don'y know yet which voice number will be the first note/rest on each staff we
//    //cannot yet assign voice to these lines. Therefore, we will assign voice 0 (meaning
//    //'no voice assigned yet') and voice will be updated when finding the first note/rest.
//
//	int numStaves = pInstr->get_num_staves();
//    for(int iS=0; iS < numStaves; iS++)
//    {
//        m_pColStorage->set_staff_spacing(iS, pInstr->get_line_spacing_for_staff(iS));
//        m_nStaffVoice.reserve(iS+1);
//        m_nStaffVoice[iS] = iS+1;
//        start_line(nInstr, iS+1, uxStart, uSpace);
//    }
//}

////---------------------------------------------------------------------------------------
//void LinesBuilder::start_line_inherit_initial_postion_and_space(int line, int instr,
//                                                                int voice)
//{
//    //LUnits uxStart = m_pColStorage->front()->get_line_start_position();
//    //LUnits uSpace = m_pColStorage->front()->get_space_at_beginning();
//    //start_line(line, instr, voice, uxStart, uSpace);
//    start_line(line, instr, voice, m_uxStart, m_uInitialSpace);
//}

//---------------------------------------------------------------------------------------
void LinesBuilder::start_line(int line, int instr)
{
    //Start a new line for instrument instr (0..n-1), to be used for voice voice.
    //The line starts at position uxStart and space before first object must be uSpace.

    //create the line
    m_pColStorage->open_new_line(line, instr, m_uxStart, m_uInitialSpace);

    //created line is set as 'current line' to receive new data.
    m_itCurLine = m_pColStorage->get_last_line();

    //as line is empty, pointer to last added entry is NULL
	m_pCurEntry = (LineEntry*)NULL;
}

//---------------------------------------------------------------------------------------
void LinesBuilder::close_line(ImoStaffObj* pSO, GmoShape* pShape, LUnits xStart,
                              float rTime)
{
	//close current line.

    if (is_there_current_line())
    {
        m_pCurEntry = (*m_itCurLine)->add_final_entry(pSO, pShape, rTime);
        m_pCurEntry->set_position(xStart);
    }
}

//---------------------------------------------------------------------------------------
void LinesBuilder::include_object(int line, int instr, ImoInstrument* pInstr,
                                  ImoStaffObj* pSO, float rTime, bool fProlog,
                                  int nStaff, GmoShape* pShape)
{
    //int voice = decide_voice_to_use(pSO, nStaff);

    //if doesn't exist, start it
    m_itCurLine = m_pColStorage->find_line(line);
    if (m_pColStorage->is_end_of_table(m_itCurLine))
    {
        start_line(line, instr);
    }

    //add new entry for this object
	m_pCurEntry = (*m_itCurLine)->add_entry(pSO, pShape, fProlog, rTime);

	////if line found was the default one for the staff, assigne voice to this line and to
 //   //the staff if not yet assigned
 //   if (voice != 0 && !(*m_itCurLine)->is_voiced_defined())
 //   {
	//	(*m_itCurLine)->set_voice(voice);
 //   }
}

////---------------------------------------------------------------------------------------
//int LinesBuilder::decide_voice_to_use(ImoStaffObj* pSO, int nStaff)
//{
////	if (nStaff != 0)    //multi-shaped object (clef, key)
////    {
////        wxASSERT(pSO->IsMultishaped());
////        return m_nStaffVoice[nStaff - 1];
////    }
////    else    //single shape object
////    {
////		if (pSO->is_note_rest())
////            //pSO has voice: return it
////			return ((lmNoteRest*)pSO)->get_voice();
////		else
////            //pSO has no voice. Use voice assigned to the staff in which this pSO is placed
////			return m_nStaffVoice[ pSO->GetStaffNum() - 1 ];
////	}
//    return 0;
//}

//---------------------------------------------------------------------------------------
void LinesBuilder::end_of_data()
{
    //this method is invoked to inform that all data has been suplied. Therefore, we
    //can do any preparatory work, not to be repeated when re-spacing.
}



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

//---------------------------------------------------------------------------------------
float LineResizer::move_prolog_shapes()
{
    //all non-timed entries, at beginning, marked as fProlog must be only re-located
    //returns the first timepos found after the prolog or 0 if no valid timepos

    LUnits uLineStartPos = m_pTable->get_line_start_position();
    LUnits uLineShift = m_uNewStart - uLineStartPos;
    LineEntryIterator it = m_pTable->begin();
    while (it != m_pTable->end() && (*it)->get_timepos() < 0.0f)
    {
        if ((*it)->get_shape())
        {
            if ((*it)->is_prolog_object())
            {
                LUnits uNewPos = uLineShift + (*it)->get_position();
                (*it)->reposition_at(uNewPos);
                (*it)->move_shape();
            }
            else
			    break;
        }
        ++it;
    }
    m_itCurrent = it;

    //return first timepos in this line
    if (it != m_pTable->end())
    {
        if ((*it)->get_timepos() < 0.0f)
            return 0.0f;
        else
            return (*it)->get_timepos();
    }
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
LUnits LineResizer::get_time_line_position_for_time(float rFirstTime)
{
    if (m_itCurrent != m_pTable->end() && (*m_itCurrent)->get_timepos() == rFirstTime)
        return (*m_itCurrent)->get_position() - (*m_itCurrent)->get_anchor();
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
void LineResizer::reasign_position_to_all_other_objects(LUnits uFizedSizeAtStart)
{
    if (m_itCurrent == m_pTable->end())
        return;

    //Compute proportion factor
    LUnits uLineStartPos = m_pTable->get_line_start_position();
    LUnits uDiscount = uFizedSizeAtStart - uLineStartPos;
    float rProp = (m_uNewBarSize-uDiscount) / (m_uOldBarSize-uDiscount);

	//Reposition the remainder entries
    for (LineEntryIterator it = m_itCurrent; it != m_pTable->end(); ++it)
	{
        if ((*it)->is_barline_entry())
        {
            LUnits uNewPos = m_uNewStart + m_uNewBarSize - (*it)->get_shape_size();
            (*it)->reposition_at(uNewPos);
            (*it)->move_shape();
        }
        else
        {
            LUnits uOldPos = (*it)->get_position() - (*it)->get_anchor();
            LUnits uShift = uDiscount + (m_uNewStart + (uOldPos - uFizedSizeAtStart) * rProp) - uOldPos;
            LUnits uNewPos = uOldPos + uShift + (*it)->get_anchor();;
            (*it)->reposition_at(uNewPos);
            (*it)->move_shape();
        }
    }
}

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
//				((GmoShapeNote*)(*it)->m_pShape)->ApplyUserShiftsToTieShape();
//            }
//        }
//    }
//}



//=======================================================================================
//LineSpacer:
//  encapsulates the algorithm to assign spaces and positions to a single line
//=======================================================================================
LineSpacer::LineSpacer(LineTable* pLineTable, ColumnLayouter* pColLyt,
                       float rFactor)
    : m_pTable(pLineTable)
    , m_rFactor(rFactor)
    , m_pColLyt(pColLyt)
    , m_itCur(pLineTable->end())
    , m_rCurTime(0.0f)
	, m_uxCurPos(0.0f)
    , m_uxRemovable(0.0f)
{
    prepare_for_traversing();
}

//---------------------------------------------------------------------------------------
void LineSpacer::prepare_for_traversing()
{
    //initialize iteration control data, to traverse by timepos

    m_itCur = m_pTable->begin();
    m_rCurTime = get_next_available_time();
    m_uxCurPos = m_pTable->get_line_start_position() + m_pTable->get_space_at_beginning();
    m_itNonTimedAtCurPos = m_pTable->end();
}

//---------------------------------------------------------------------------------------
void LineSpacer::process_non_timed_at_current_timepos(LUnits uxPos)
{
    //update current pos with new xPos required for column alignment
    m_uxRemovable += uxPos - m_uxCurPos;
    m_uxCurPos = uxPos;

    //proceed if there are non-timed objects
    if (is_current_object_non_timed())
    {
        compute_max_and_min_occupied_space();
        position_non_timed();
    }
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::get_next_position()
{
    return m_uxCurPos;
}

//---------------------------------------------------------------------------------------
void LineSpacer::compute_max_and_min_occupied_space()
{
	//Starting at current position, explores the not-timed objects until next timed
    //or end of line. Computes the maximum and minimum space they could occupy.
    //Current position is not altered

    m_uxMaxOcuppiedSpace = 0.0f;
    m_uxMinOcuppiedSpace = 0.0f;
    LineEntryIterator it = m_itCur;
	while (is_non_timed_object(it))
    {
        //(*it)->assign_fixed_and_variable_space(m_pColLyt, m_rFactor);
        assign_fixed_and_variable_space(*it);
        LUnits uxMax = (*it)->get_total_size();
        m_uxMaxOcuppiedSpace += uxMax;
        m_uxMinOcuppiedSpace += uxMax - (*it)->get_variable_space();
        ++it;
    }
}

//---------------------------------------------------------------------------------------
void LineSpacer::position_non_timed()
{
    m_itNonTimedAtCurPos = m_itCur;
    if (m_uxRemovable >= m_uxMaxOcuppiedSpace)
    {
        position_using_max_space_with_shift(m_uxRemovable - m_uxMaxOcuppiedSpace);
    }
    else if (m_uxRemovable >= m_uxMinOcuppiedSpace)
    {
        LUnits uShift = m_uxRemovable - m_uxMinOcuppiedSpace;
        position_using_min_space_with_shift(uShift);
    }
    else
    {
        position_using_min_space_with_shift(0.0f);
    }
    m_uxNotTimedFinalPos = m_uxCurPos;
}

//---------------------------------------------------------------------------------------
void LineSpacer::position_using_max_space_with_shift(LUnits uShift)
{
    LUnits uxNextPos = m_uxCurPos - m_uxRemovable + uShift;
	while (is_current_object_non_timed())
    {
        //(*m_itCur)->assign_fixed_and_variable_space(m_pColLyt, m_rFactor);
        assign_fixed_and_variable_space(*m_itCur);
        (*m_itCur)->reposition_at(uxNextPos);

        uxNextPos += (*m_itCur)->get_total_size();
        ++m_itCur;
    }

    //update iteration data
    m_uxCurPos = uxNextPos;
    m_uxRemovable = 0.0f;
}

//---------------------------------------------------------------------------------------
void LineSpacer::position_using_min_space_with_shift(LUnits uShift)
{
    LUnits uxNextPos = m_uxCurPos - m_uxRemovable + uShift;
	while (is_current_object_non_timed())
    {
        //(*m_itCur)->assign_fixed_and_variable_space(m_pColLyt, m_rFactor);
        assign_fixed_and_variable_space(*m_itCur);
        (*m_itCur)->set_variable_space(0.0f);
        (*m_itCur)->reposition_at(uxNextPos);

        uxNextPos += (*m_itCur)->get_total_size();
        ++m_itCur;
    }

    //update iteration data
    m_uxCurPos = uxNextPos;
    m_uxRemovable = 0.0f;
}

//---------------------------------------------------------------------------------------
void LineSpacer::process_non_timed_at_prolog(LUnits uSpaceAfterProlog)
{
    if (is_current_object_non_timed())
    {
        LUnits uxNextPos = m_uxCurPos;
	    while (is_current_object_non_timed())
        {
            //(*m_itCur)->assign_fixed_and_variable_space(m_pColLyt, m_rFactor);
            assign_fixed_and_variable_space(*m_itCur);
            (*m_itCur)->reposition_at(uxNextPos);

            uxNextPos += (*m_itCur)->get_total_size();
            ++m_itCur;
        }

        //update iteration data and add some additional space after prolog
        m_uxCurPos = uxNextPos + uSpaceAfterProlog;
        m_uxRemovable = uSpaceAfterProlog;
    }
}

//---------------------------------------------------------------------------------------
void LineSpacer::process_timed_at_current_timepos(LUnits uxPos)
{
	//Starting at current position, explores the line to set the position of all timed
    //objects placed at current time, until we reach a time greater that current
    //time or end of line

    //update current pos with new xPos required for column alignment
    m_uxRemovable += uxPos - m_uxCurPos;
    m_uxCurPos = uxPos;

    drag_any_previous_clef_to_place_it_near_this_one();

    //procced to process this timepos
    LUnits uxRequiredPos = m_uxCurPos + compute_shift_to_avoid_overlap_with_previous();
    LUnits uxNextPos = uxRequiredPos;
    LUnits uxMinNextPos = 0.0f;
    LUnits uxMargin = 0.0f;
    LineEntryIterator itLast;
	while (are_there_timed_objs())
    {
        //AssignPositionToCurrentEntry();
		(*m_itCur)->set_position( uxRequiredPos + (*m_itCur)->get_anchor() );

        //AssignFixedAndVariableSpacingToCurrentEntry();
        //(*m_itCur)->assign_fixed_and_variable_space(m_pColLyt, m_rFactor);
        assign_fixed_and_variable_space(*m_itCur);

        //DetermineSpaceRequirementsForCurrentEntry();
        if ((*m_itCur)->is_note_rest())
		    uxNextPos = max(uxNextPos, (*m_itCur)->get_x_final());
        else
            uxMinNextPos = max(uxMinNextPos, (*m_itCur)->get_x_final());

        uxMargin = (uxMargin==0.0f ?
                        (*m_itCur)->get_variable_space()
                        : min(uxMargin, (*m_itCur)->get_variable_space()) );

        //AdvanceToNextEntry();
        itLast = m_itCur++;
    }

    //update iteration data
    if (uxNextPos == uxRequiredPos)     //No note/rest found
        m_uxCurPos = uxRequiredPos + uxMinNextPos;
    else
        m_uxCurPos = uxNextPos;

    m_uxRemovable = uxMargin;
    m_rCurTime = get_next_available_time();
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::compute_shift_to_avoid_overlap_with_previous()
{
	//Starting at current position, explores the objects placed at current time
    //to check if there is enought removable space to deal with any anchor left shifted
    //object. If not, computes the required additional space that should be added to
    //'removable' space.

    LineEntryIterator it = m_itCur;
    LUnits uxShift = 0.0f;
    const LineEntryIterator itEnd = m_pTable->end();
	while (it != itEnd && is_equal_time((*it)->get_timepos(), m_rCurTime))
    {
        LUnits uAnchor = - (*it)->get_anchor();     // > 0 if need to shift left
        if (uAnchor > 0.0f && m_uxRemovable < uAnchor)
            uxShift = max(uxShift, uAnchor - m_uxRemovable);

        it++;
    }
    return uxShift;
}

//---------------------------------------------------------------------------------------
void LineSpacer::shift_non_timed(LUnits uxShift)
{
    LineEntryIterator it = m_itNonTimedAtCurPos;
	while (is_non_timed_object(it))
    {
        LUnits uxCurPos = (*it)->get_position();
        (*it)->reposition_at(uxCurPos + uxShift);
        ++it;
    }
}

//---------------------------------------------------------------------------------------
float LineSpacer::get_next_available_time()
{
	LineEntryIterator it = m_itCur;
    if (it != m_pTable->end())
    {
        while (is_non_timed_object(it))
            ++it;

        if (is_timed_object(it))
            return (*it)->get_timepos();
        else
            return LOMSE_NO_TIME;
    }
    else
        return LOMSE_NO_TIME;
}

//---------------------------------------------------------------------------------------
void LineSpacer::drag_any_previous_clef_to_place_it_near_this_one()
{
    if (m_itNonTimedAtCurPos != m_pTable->end() && m_uxCurPos > m_uxNotTimedFinalPos)
    {
        shift_non_timed(m_uxCurPos - m_uxNotTimedFinalPos);
    }
    m_itNonTimedAtCurPos = m_pTable->end();     //no longer needed. Discart value now to avoid problmes at next timepos
}

//---------------------------------------------------------------------------------------
void LineSpacer::assign_fixed_and_variable_space(LineEntry* pEntry)
{
	//assign fixed and variable after spaces to this entry and compute the xFinal pos

    if (pEntry->is_barline_entry())
    {
		if (!pEntry->get_staffobj())
            pEntry->set_size(0.0f);
    }
    else
    {
        ImoStaffObj* pSO = pEntry->get_staffobj();
		if (!pSO->is_visible())
            assign_no_space(pEntry);
		else
		{
			if (pSO->is_note_rest())
			{
				set_note_rest_space(pEntry);
			}
			else if (pSO->is_clef()
                    || pSO->is_key_signature()
                    || pSO->is_time_signature() )
			{
			    int iInstr = m_pTable->get_instrument();
			    int iStaff = pSO->get_staff();
			    LUnits fixed = m_pColLyt->tenths_to_logical(LOMSE_EXCEPTIONAL_MIN_SPACE,
                                                            iInstr, iStaff);
                pEntry->set_fixed_space(fixed);
                pEntry->set_variable_space(
                    m_pColLyt->tenths_to_logical(LOMSE_MIN_SPACE, iInstr, iStaff) - fixed );
			}
			else if (pSO->is_spacer())    //TODO || pSO->is_score_anchor())
			{
                pEntry->set_fixed_space(0.0f);
                pEntry->set_variable_space( pEntry->get_shape_size() );
			}
			else
                assign_no_space(pEntry);
		}
    }
    pEntry->update_x_final();
}

//---------------------------------------------------------------------------------------
void LineSpacer::set_note_rest_space(LineEntry* pEntry)
{
    assign_minimum_fixed_space(pEntry);
    LUnits uIdeal = compute_ideal_distance(pEntry);
    assign_variable_space(pEntry, uIdeal);
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::compute_ideal_distance(LineEntry* pEntry)
{
    if (m_pColLyt->is_proportional_spacing())
        return compute_ideal_distance_proportional(pEntry);
    else
        return compute_ideal_distance_fixed(pEntry);
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::compute_ideal_distance_fixed(LineEntry* pEntry)
{
    int iInstr = m_pTable->get_instrument();
    ImoStaffObj* pSO = pEntry->get_staffobj();
	int iStaff = pSO->get_staff();
    return m_pColLyt->tenths_to_logical(m_pColLyt->get_fixed_spacing_value(),
                                        iInstr, iStaff);
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::compute_ideal_distance_proportional(LineEntry* pEntry)
{
	static const float rLog2 = 0.3010299956640f;		// log(2)
    ImoStaffObj* pSO = pEntry->get_staffobj();
	int iStaff = pSO->get_staff();
    int iInstr = m_pTable->get_instrument();

	//spacing function:   Space(Di) = Smin*[1 + A*log2(Di/Dmin)]
	LUnits uSmin = m_pColLyt->tenths_to_logical(LOMSE_MIN_SPACE, iInstr, iStaff);
    ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
    float rVar = log(pNR->get_duration() / LOMSE_DMIN) / rLog2;     //log2(Di/Dmin)
    if (rVar > 0.0f)
        return uSmin * (1.0f + m_rFactor * rVar);
    else
        return uSmin;
}

//---------------------------------------------------------------------------------------
void LineSpacer::assign_variable_space(LineEntry* pEntry, LUnits uIdeal)
{
    LUnits space = max(0.0f, uIdeal - pEntry->get_shape_size()
                                    - pEntry->get_fixed_space()
                                    - pEntry->get_anchor());

   pEntry->set_variable_space( space );
}

//---------------------------------------------------------------------------------------
void LineSpacer::assign_no_space(LineEntry* pEntry)
{
    pEntry->set_fixed_space(0.0f);
    pEntry->set_variable_space(0.0f);
    pEntry->set_size(0.0f);
}

//---------------------------------------------------------------------------------------
void LineSpacer::assign_minimum_fixed_space(LineEntry* pEntry)
{
    int iInstr = m_pTable->get_instrument();
    ImoStaffObj* pSO = pEntry->get_staffobj();
	int iStaff = pSO->get_staff();
    pEntry->set_fixed_space( m_pColLyt->tenths_to_logical(LOMSE_EXCEPTIONAL_MIN_SPACE,
                                                          iInstr, iStaff) );
}



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
    delete_breaks_table();
}

//---------------------------------------------------------------------------------------
void BreakPoints::delete_breaks_table()
{
    if (m_pPossibleBreaks)
    {
        delete m_pPossibleBreaks;
        m_pPossibleBreaks = (BreaksTable*)NULL;
    }
}

//---------------------------------------------------------------------------------------
bool BreakPoints::find_optimum_break_point_for_space(LUnits uAvailable)
{
    //returns false if no break point found (exceptional case).
    //In all other cases updates m_pOptimumEntry and returns true

    if (!m_pPossibleBreaks)
        compute_breaks_table();

    //select highest entry with space <= uAvailable
    BreaksTimeEntry* pBTE = m_pPossibleBreaks->get_first();
    m_pOptimumEntry = (BreaksTimeEntry*)NULL;
    while (pBTE && pBTE->uxEnd <= uAvailable)
    {
        m_pOptimumEntry = pBTE;
        pBTE = m_pPossibleBreaks->get_next();
    }
    if (!m_pOptimumEntry)
        return false;        //big problem: no break points!

    //wxLogMessage(_T("[ColumnLayouter::get_optimum_break_point] uAvailable=%.2f, returned=%.2f, time=%.2f"),
    //             uAvailable, m_pOptimumEntry->uxEnd, m_pOptimumEntry->rTimepos);

    return true;       //no problems. There are break points
}

//---------------------------------------------------------------------------------------
float BreakPoints::get_optimum_time_for_found_break_point()
{
    return m_pOptimumEntry->rTimepos;
}

//---------------------------------------------------------------------------------------
LUnits BreakPoints::get_optimum_position_for_break_point()
{
    return m_pOptimumEntry->uxEnd;
}

//---------------------------------------------------------------------------------------
void BreakPoints::compute_breaks_table()
{
    //This method computes the BreaksTable. This is a table sumarizing break points
    //information, that is, suitable places through all staves and voices where it is
    //possible to break a system and start a new one. The best break locations are
    //usually are the bar lines common to all staves. But in certain rare cases (i.e.
    //scores without time signature or having instrumens not sharing a common
    //time signature, or when it is requested to render the score in very narrow
    //paper, etc.) it is necessary to split music in unnusual points.

    //Step 1. Build a table for each line
    std::vector<BreaksTable*> partialTables;
	for (LinesIterator itTL = m_pColStorage->begin(); itTL != m_pColStorage->end(); ++itTL)
	{
        BreaksTable* pBT = new BreaksTable();
        ColumnSplitter oSplitter(*itTL);
        oSplitter.compute_break_points(pBT);
        partialTables.push_back(pBT);
    }


    //Step 2. Combine the partial tables
    if (m_pPossibleBreaks)
        delete_breaks_table();
    m_pPossibleBreaks = new BreaksTable();

    std::vector<BreaksTable*>::iterator itBT;
    for (itBT = partialTables.begin(); itBT != partialTables.end(); ++itBT)
    {
        if (m_pPossibleBreaks->is_empty())
        {
            //just copy entries
            BreaksTimeEntry* pEP = (*itBT)->get_first();       //pEP Entry from Partial list
            while (pEP)
            {
                m_pPossibleBreaks->add_entry(pEP);
                pEP = (*itBT)->get_next();
            }
        }
        else
        {
            ////merge current table with total table
            ////BreaksTimeEntry* pEP = (*itBT)->get_first();       //pEP Entry from Partial list
            ////while (pEP)
            ////{
            ////    m_pPossibleBreaks->add_entry(pEP);
            ////    pEP = (*itBT)->get_next();
            ////}
        }
    }


    //Delete partial tables, no longer needed
    for (itBT = partialTables.begin(); itBT != partialTables.end(); ++itBT)
        delete *itBT;
    partialTables.clear();

    //wxLogMessage(_T("Total Breaks Table:"));
    //cout << m_pPossibleBreaks->dump() );

    //Step 3. Sort breaks table by priority and final x position
    //TODO
}



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




//=======================================================================================
//TimeGridTable:
//  A table with the relation timepos <=> position for all valid positions to insert
//  a note.
//  This object is responsible for supplying all valid timepos and their positions so
//  that other objects (in fact only GmoBoxSlice) could:
//      a) Determine the timepos to assign to a mouse click in a certain position.
//      b) Draw a grid of valid timepos
//=======================================================================================
TimeGridTable::TimeGridTable(ColumnStorage* pColStorage)
    : m_pColStorage(pColStorage)
{
    //build the table

    create_line_explorers();
    while (there_are_objects())
    {
        skip_non_timed_at_current_timepos();
        if (timed_objects_found())
        {
            find_shortest_noterest_at_current_timepos();
            create_table_entry();
        }
    }
    interpolate_missing_times();
    delete_line_explorers();
}

//---------------------------------------------------------------------------------------
TimeGridTable::~TimeGridTable()
{
    m_PosTimes.clear();
}

//---------------------------------------------------------------------------------------
bool TimeGridTable::there_are_objects()
{
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
    {
        if ((*it)->there_are_objects())
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void TimeGridTable::create_table_entry()
{
    PosTimeItem tPosTime = {m_rCurrentTime, m_rMinDuration, m_uCurPos };
    m_PosTimes.push_back(tPosTime);
}

//---------------------------------------------------------------------------------------
void TimeGridTable::delete_line_explorers()
{
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
        delete *it;
    m_LineExplorers.clear();
}

//---------------------------------------------------------------------------------------
void TimeGridTable::create_line_explorers()
{
    const LinesIterator itEnd = m_pColStorage->end();
    for (LinesIterator it=m_pColStorage->begin(); it != itEnd; ++it)
	{
        TimeGridLineExplorer* pLinExplorer = new TimeGridLineExplorer(*it);
        m_LineExplorers.push_back(pLinExplorer);
    }
}

//---------------------------------------------------------------------------------------
void TimeGridTable::skip_non_timed_at_current_timepos()
{
    m_fTimedObjectsFound = false;
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
	{
        m_fTimedObjectsFound |= (*it)->skip_non_timed_at_current_timepos();
    }
}

//---------------------------------------------------------------------------------------
void TimeGridTable::find_shortest_noterest_at_current_timepos()
{
    get_current_time();
    m_rMinDuration = LOMSE_NO_DURATION;
    m_uCurPos = LOMSE_NO_POSITION;
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
	{
        if (m_rCurrentTime == (*it)->get_current_time())
        {
            (*it)->find_shortest_noterest_at_current_timepos();
            if (m_rMinDuration > (*it)->get_duration_for_found_entry())
            {
                m_rMinDuration = (*it)->get_duration_for_found_entry();
                m_uCurPos = min(m_uCurPos, (*it)->get_position_for_found_entry());
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeGridTable::get_current_time()
{
    m_rCurrentTime = LOMSE_NO_TIME;
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
	{
        m_rCurrentTime = min(m_rCurrentTime, (*it)->get_current_time());
    }
}

////---------------------------------------------------------------------------------------
//std::string TimeGridTable::dump()
//{
//#if (LOMSE_DUMP_TABLES)
//                      //   .......+.......+.......+
//    std::string sDump = _T("\n timepos     Dur     Pos\n");
//    std::vector<PosTimeItem>::iterator it;
//    for (it = m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
//    {
//        sDump += std::string::Format(_T("%8.2f %8.2f %8.2f\n"),
//                                (*it).rTimepos, (*it).rDuration, (*it).uxPos );
//    }
//#endif
//}

//---------------------------------------------------------------------------------------
float TimeGridTable::get_time_for_position(LUnits uxPos)
{
    //timepos = 0 if measure is empty
    if (m_PosTimes.size() == 0)
        return 0.0f;

    //timepos = 0 if xPos < first entry xPos
    float rTime = 0.0f;
    LUnits uxPrev = m_PosTimes.front().uxPos;
    if (uxPos <= uxPrev)
        return rTime;

    //otherwise find in table
    std::vector<PosTimeItem>::iterator it = m_PosTimes.begin();
    for (++it; it != m_PosTimes.end(); ++it)
    {
        LUnits uxLimit = uxPrev + ((*it).uxPos - uxPrev) / 2.0f;
        if (uxPos <= uxLimit)
            return rTime;
        uxPrev = (*it).uxPos;
        rTime = (*it).rTimepos;
    }

    //if not found return last entry timepos
    return m_PosTimes.back().rTimepos;
}

//---------------------------------------------------------------------------------------
void TimeGridTable::interpolate_missing_times()
{
    TimeInserter oInserter(m_PosTimes);
    oInserter.interpolate_missing_times();
}



//=======================================================================================
//TimeInserter
// helper class to interpolate missing entries
//=======================================================================================
TimeInserter::TimeInserter(std::vector<PosTimeItem>& oPosTimes)
    : m_PosTimes(oPosTimes)
{
}

//---------------------------------------------------------------------------------------
void TimeInserter::interpolate_missing_times()
{
    for (int i=0; i < (int)m_PosTimes.size(); ++i)
    {
        float rNextTime = m_PosTimes[i].rTimepos + m_PosTimes[i].rDuration;
        if (!is_time_in_table(rNextTime))
        {
            find_insertion_point(rNextTime);
            insert_time_interpolating_position(rNextTime);
        }
    }
}

//---------------------------------------------------------------------------------------
bool TimeInserter::is_time_in_table(float rTimepos)
{
    if (m_PosTimes.size() == 0)
        return false;

    std::vector<PosTimeItem>::iterator it;
    for (it=m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
    {
        if (is_equal_time(rTimepos, (*it).rTimepos))
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void TimeInserter::find_insertion_point(float rTimepos)
{
    m_uPositionBeforeInsertionPoint = m_PosTimes.front().uxPos;
    m_rTimeBeforeInsertionPoint = m_PosTimes.front().rTimepos;

    std::vector<PosTimeItem>::iterator it;
    for (it=m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
    {
        if (is_greater_time((*it).rTimepos, rTimepos))
            break;
        m_uPositionBeforeInsertionPoint = (*it).uxPos;
        m_rTimeBeforeInsertionPoint = (*it).rTimepos;
    }
    m_itInsertionPoint = it;
}

//---------------------------------------------------------------------------------------
void TimeInserter::insert_time_interpolating_position(float rTimepos)
{
    PosTimeItem oItem;
    oItem.rTimepos = rTimepos;
    oItem.rDuration = 0.0f;
    oItem.uxPos = m_uPositionBeforeInsertionPoint;

    if (m_itInsertionPoint == m_PosTimes.end())
    {
        //insert at the end
        oItem.uxPos += 1000;       //TODO: Estimate space based on measure duration
        m_PosTimes.push_back(oItem);
    }
    else
    {
        //insert before item pointed by iterator
        float rTimeGap = (*m_itInsertionPoint).rTimepos - m_rTimeBeforeInsertionPoint;
        float rPosGap = (*m_itInsertionPoint).uxPos - m_uPositionBeforeInsertionPoint;
        float rTimeIncrement = rTimepos - m_rTimeBeforeInsertionPoint;
        oItem.uxPos += rTimeIncrement * (rPosGap / rTimeGap);
        m_PosTimes.insert(m_itInsertionPoint, oItem);
    }
}


//=======================================================================================
//TimeGridLineExplorer:
//  line traversal algorithm for creating the time-pos table
//=======================================================================================
TimeGridLineExplorer::TimeGridLineExplorer(LineTable* pLineTable)
    : m_pTable(pLineTable)
{
    m_itCur = m_pTable->begin();
}

//---------------------------------------------------------------------------------------
TimeGridLineExplorer::~TimeGridLineExplorer()
{
}

//---------------------------------------------------------------------------------------
bool TimeGridLineExplorer::skip_non_timed_at_current_timepos()
{
    //returns true if there are timed objects after the skipped non-timed

	while (is_current_object_non_timed())
        ++m_itCur;

    return current_object_is_timed();
}

//---------------------------------------------------------------------------------------
bool TimeGridLineExplorer::find_shortest_noterest_at_current_timepos()
{
    //returns true if there are more objects after current timepos

	if (current_object_is_timed())
    {
        m_rCurTime = (*m_itCur)->get_timepos();
        m_uCurPos = (*m_itCur)->get_position() - (*m_itCur)->get_anchor();
        m_rMinDuration = (*m_itCur)->get_duration();
        m_uShiftToNoteRestCenter = (*m_itCur)->get_shift_to_noterest_center();

	    while (current_object_is_timed() && (*m_itCur)->get_timepos() == m_rCurTime)
        {
            m_rMinDuration = min(m_rMinDuration, (*m_itCur)->get_duration());
            if (m_uShiftToNoteRestCenter == 0.0f)
                m_uShiftToNoteRestCenter = (*m_itCur)->get_shift_to_noterest_center();

            ++m_itCur;
        }
    }
    return there_are_objects();
}

//---------------------------------------------------------------------------------------
float TimeGridLineExplorer::get_current_time()
{
    if (current_object_is_timed())
        return (*m_itCur)->get_timepos();
    else
        return LOMSE_NO_TIME;
}

//---------------------------------------------------------------------------------------
float TimeGridLineExplorer::get_duration_for_found_entry()
{
    return m_rMinDuration;
}

//---------------------------------------------------------------------------------------
LUnits TimeGridLineExplorer::get_position_for_found_entry()
{
    return m_uCurPos + m_uShiftToNoteRestCenter;
}



//=======================================================================================
//ColumnResizer: encapsulates the methods to recompute shapes positions so that the
//column will have the desired width, and to move the shapes to those positions
//=======================================================================================
ColumnResizer::ColumnResizer(ColumnStorage* pColStorage, LUnits uNewBarSize)
    : m_pColStorage(pColStorage)
    , m_uNewBarSize(uNewBarSize)
{
}

//-------------------------------------------------------------------------------------
void ColumnResizer::reposition_shapes(LUnits uNewStart)
{
    m_uNewStart = uNewStart;
    m_uOldBarSize = m_pColStorage->get_column_width();

    create_line_resizers();
    move_prolog_shapes_and_get_initial_time();
    determine_fixed_size_at_start_of_column();
    reposition_all_other_shapes();
    delete_line_resizers();
}

//---------------------------------------------------------------------------------------
void ColumnResizer::create_line_resizers()
{
	for (LinesIterator it=m_pColStorage->begin(); it != m_pColStorage->end(); ++it)
	{
        LineResizer* pResizer = new LineResizer(*it, m_uOldBarSize, m_uNewBarSize,
                                                    m_uNewStart);
        m_LineResizers.push_back(pResizer);
    }
}

//---------------------------------------------------------------------------------------
void ColumnResizer::move_prolog_shapes_and_get_initial_time()
{
    m_rFirstTime = LOMSE_NO_TIME;
    std::vector<LineResizer*>::iterator itR;
	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
	{
        m_rFirstTime = min(m_rFirstTime, (*itR)->move_prolog_shapes());
    }
}

//---------------------------------------------------------------------------------------
void ColumnResizer::determine_fixed_size_at_start_of_column()
{
    m_uFixedPart = 0.0f;
    std::vector<LineResizer*>::iterator itR;
	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
	{
        m_uFixedPart = max(m_uFixedPart, (*itR)->get_time_line_position_for_time(m_rFirstTime));
    }
}

//---------------------------------------------------------------------------------------
void ColumnResizer::reposition_all_other_shapes()
{
    std::vector<LineResizer*>::iterator itR;
	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
		(*itR)->reasign_position_to_all_other_objects(m_uFixedPart);
}

//---------------------------------------------------------------------------------------
void ColumnResizer::delete_line_resizers()
{
    std::vector<LineResizer*>::iterator itR;
	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
		delete *itR;
    m_LineResizers.clear();
}



//=======================================================================================
//ScoreMeter implementation
//=======================================================================================
ScoreMeter::ScoreMeter(ImoScore* pScore)
    : m_numInstruments( pScore->get_num_instruments() )
{
    get_options(pScore);
    get_staff_spacing(pScore);
}

//---------------------------------------------------------------------------------------
void ScoreMeter::get_options(ImoScore* pScore)
{
    ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
    m_rSpacingFactor = pOpt->get_float_value();

    pOpt = pScore->get_option("Render.SpacingMethod");
    m_nSpacingMethod = static_cast<ESpacingMethod>( pOpt->get_long_value() );

    pOpt = pScore->get_option("Render.SpacingValue");
    m_rSpacingValue = static_cast<Tenths>( pOpt->get_long_value() );

    pOpt = pScore->get_option("Staff.DrawLeftBarline");
    m_fDrawLeftBarline = pOpt->get_bool_value();
}

//---------------------------------------------------------------------------------------
void ScoreMeter::get_staff_spacing(ImoScore* pScore)
{
    int instruments = pScore->get_num_instruments();
    m_staffIndex.reserve(instruments);
    int staves = 0;
    for (int iInstr=0; iInstr < instruments; ++iInstr)
    {
        m_staffIndex[iInstr] = staves;
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        int numStaves = pInstr->get_num_staves();
        staves += numStaves;
        for (int iStaff=0; iStaff < numStaves; ++iStaff)
            m_lineSpace.push_back( pInstr->get_line_spacing_for_staff(iStaff) );
    }
}

//---------------------------------------------------------------------------------------
LUnits ScoreMeter::tenths_to_logical(Tenths value, int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return (value * m_lineSpace[idx]) / 10.0f;
}

//---------------------------------------------------------------------------------------
LUnits ScoreMeter::line_spacing_for_instr_staff(int iInstr, int iStaff)
{
    int idx = m_staffIndex[iInstr] + iStaff;
	return m_lineSpace[idx];
}


}  //namespace lomse
