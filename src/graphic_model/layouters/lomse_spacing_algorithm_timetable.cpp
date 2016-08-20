//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_spacing_algorithm_timetable.h"

#include "lomse_staffobjs_table.h"
#include "lomse_score_iterator.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_box_slice.h"
#include "lomse_box_slice_instr.h"
#include "lomse_system_layouter.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_instrument_engraver.h"
#include "lomse_score_layouter.h"
#include "lomse_system_layouter.h"
#include "lomse_shape_note.h"
#include "lomse_timegrid_table.h"

namespace lomse
{

#define LOMSE_NO_DURATION   100000000000000.0     //any impossible high value
#define LOMSE_NO_TIME       100000000000000.0     //any impossible high value
#define LOMSE_NO_POSITION   100000000000000.0f    //any impossible high value

//=====================================================================================
//SpAlgorithmTimetable implementation
//=====================================================================================
SpAlgorithmTimetable::SpAlgorithmTimetable(LibraryScope& libraryScope,
                                           ScoreMeter* pScoreMeter,
                                           ScoreLayouter* pScoreLyt,
                                           ImoScore* pScore,
                                           ShapesStorage& shapesStorage,
                                           ShapesCreator* pShapesCreator,
                                           PartsEngraver* pPartsEngraver)
    :SpAlgColumn(libraryScope, pScoreMeter, pScoreLyt, pScore, shapesStorage,
                 pShapesCreator, pPartsEngraver)
{
}

//---------------------------------------------------------------------------------------
SpAlgorithmTimetable::~SpAlgorithmTimetable()
{
    delete_column_layouters();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgorithmTimetable::get_end_hook_width(int iCol)
{
    return m_ColLayouters[iCol]->get_end_hook_width();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgorithmTimetable::get_start_hook_width(int iCol)
{
    return m_ColLayouters[iCol]->get_start_hook_width();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgorithmTimetable::get_main_width(int iCol)
{
    return m_ColLayouters[iCol]->get_main_width();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgorithmTimetable::get_trimmed_width(int iCol)
{
    return m_ColLayouters[iCol]->get_trimmed_width();
}

//---------------------------------------------------------------------------------------
bool SpAlgorithmTimetable::column_has_barline(int iCol)
{
    return m_ColLayouters[iCol]->column_has_barline();
}

//---------------------------------------------------------------------------------------
TimeGridTable* SpAlgorithmTimetable::create_time_grid_table_for_column(int iCol)
{
    return m_ColLayouters[iCol]->create_time_grid_table_for_column();
}
//
////---------------------------------------------------------------------------------------
//ColStaffObjsEntry* SpAlgorithmTimetable::get_prolog_clef(int iCol, ShapeId idx)
//{
//    return m_ColLayouters[iCol]->get_prolog_clef(idx);
//}
//
////---------------------------------------------------------------------------------------
//ColStaffObjsEntry* SpAlgorithmTimetable::get_prolog_key(int iCol, ShapeId idx)
//{
//    return m_ColLayouters[iCol]->get_prolog_key(idx);
//}

////---------------------------------------------------------------------------------------
//int SpAlgorithmTimetable::get_num_columns()
//{
//    return int(m_ColLayouters.size());
//}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::delete_column_layouters()
{
    std::vector<ColumnLayouter*>::iterator itF;
    for (itF=m_ColLayouters.begin(); itF != m_ColLayouters.end(); ++itF)
        delete *itF;
    m_ColLayouters.clear();
}

//---------------------------------------------------------------------------------------
bool SpAlgorithmTimetable::is_empty_column(int iCol)
{
    return m_ColLayouters[iCol]->is_empty_column();
}
//
////---------------------------------------------------------------------------------------
//void SpAlgorithmTimetable::delete_box_and_shapes(int iCol, ShapesStorage* pStorage)
//{
//    m_ColLayouters[iCol]->delete_box_and_shapes(pStorage);
//}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::dump_column_data(int iCol, ostream& outStream)
{
    m_ColLayouters[iCol]->dump_column_data(outStream);
}

//---------------------------------------------------------------------------------------
float SpAlgorithmTimetable::get_penalty_factor(int iCol)
{
    return m_ColLayouters[iCol]->get_penalty_factor();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgorithmTimetable::aditional_space_before_adding_column(int iCol)
{
    //Column iCol is going to be added to current system containing more columns.
    //Returns any additional space (positive or negative) that the spacing algorithm
    //would like to add before adding column iCol to the system.

    LUnits uEndHookWidth = get_end_hook_width(iCol-1);
    LUnits uStartHookWidth = get_start_hook_width(iCol);
    if (uEndHookWidth > uStartHookWidth)
        return uEndHookWidth - uStartHookWidth;
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
LUnits SpAlgorithmTimetable::get_column_width(int iCol, bool fFirstColumnInSystem)
{
    //Returns the width for this column. Flag fFirstColumnInSystem could be
    //used in case the spacing algorithm would like to remove space from start
    //of column when the column is going to be placed as first column of a system

    LUnits size = get_main_width(iCol);
    if (!fFirstColumnInSystem)
    {
        LUnits uEndHook = get_end_hook_width(iCol-1);
        LUnits uStartHook = get_start_hook_width(iCol);
        if (uEndHook > uStartHook)
            size += (uEndHook - uStartHook);
    }
    return size;
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
                                                           LUnits yShift,
                                                           LUnits* yMin, LUnits* yMax)
{
    GmoBoxSlice* pFirstSlice = get_slice_box(iFirstCol);
    LUnits xLeft = pFirstSlice->get_left();
    LUnits yTop = pFirstSlice->get_top();
    LUnits xStartPos = m_ColLayouters[iFirstCol]->get_start_of_column();

    for (int iCol = iFirstCol; iCol < iLastCol; ++iCol)
    {
        //reposition boxes
        set_slice_final_position(iCol, xLeft, yTop);
        xLeft += m_ColLayouters[iCol]->get_justified_width();

        //reposition staffobjs
        LUnits xEndPos = redistribute_space(iCol, xStartPos, yShift);

        //collect information about system vertical limits
        if (m_ColLayouters[iCol]->has_shapes())
        {
            *yMin = min(*yMin, m_ColLayouters[iCol]->get_y_min());
            *yMax = max(*yMax, m_ColLayouters[iCol]->get_y_max());
        }

        //assign justified width to boxes
        set_slice_width(iCol, xEndPos - xStartPos);
    }
}

//---------------------------------------------------------------------------------------
LUnits SpAlgorithmTimetable::redistribute_space(int iCol, LUnits uNewStart, LUnits yShift)
{
    LUnits uNewWidth = m_ColLayouters[iCol]->get_justified_width();
    GmoBoxSlice* pBox = get_slice_box(iCol);

    UPoint org = pBox->get_origin();
    org.y += yShift;

    return m_ColLayouters[iCol]->redistribute_space(uNewStart, uNewWidth, org);
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::justify_system(int iFirstCol, int iLastCol,
                                       LUnits uSpaceIncrement)
{
    //resize columns from iFirstCol to iLastCol (excluded) so that the total space
    //increment is uSpaceIncrement

    if (uSpaceIncrement <= 0.0f)
        return;           //no space to distribute

    //Space is redistributed proportionally to actual width

    //compute total occupied
    LUnits uTotal = 0.0f;
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
         uTotal += m_ColLayouters[i]->get_trimmed_width();
    }

    //proportion factor
    float alpha = (uSpaceIncrement + uTotal) / uTotal;

    //assign new size to columns
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
        LUnits newSize = alpha * m_ColLayouters[i]->get_trimmed_width();
        m_ColLayouters[i]->set_justified_width(newSize);
    }


#if 0
    //Space is redistributed to try to have all columns with equal witdh.

    //compute average column size and total occupied
    LUnits uTotal = 0.0f;
    int nColumnsInSystem = m_iLastCol - m_iFirstCol;
    for (int i = m_iFirstCol; i < m_iLastCol; ++i)
    {
         uTotal += m_pSpAlgorithm->get_trimmed_width(i);
    }
    LUnits uAverage = (uTotal + uSpaceIncrement) / nColumnsInSystem;

    //For each column, compute the diference between its size and the average target size.
    //Sum up all the diferences in uDifTotal
    std::vector<LUnits> uDif(nColumnsInSystem, 0.0f);
    LUnits uDifTotal = 0;
    int nNumSmallerColumns = 0;      //num of columns smaller than average
    for (int i = 0; i < nColumnsInSystem; i++)
    {
        uDif[i] = uAverage - m_pSpAlgorithm->get_trimmed_width(i + m_iFirstCol);
        if (uDif[i] > 0.0f)
        {
            uDifTotal += uDif[i];
            nNumSmallerColumns++;
        }
    }

    //distribute space
    if (uDifTotal > uSpaceIncrement)
    {
        //not enough space to make all columns equal size
        LUnits uReduce = (uDifTotal - uSpaceIncrement) / nNumSmallerColumns;
        for (int i = 0; i < nColumnsInSystem; i++)
        {
            if (uDif[i] > 0.0f)
            {
                uDif[i] -= uReduce;
                m_pSpAlgorithm->increment_justified_width(uDif[i+m_iFirstCol, i]);
            }
        }
    }
    else
    {
        //enough space to make all columns equal size
        for (int i = 0; i < nColumnsInSystem; i++)
        {
            if (uDif[i] > 0.0f)
                m_pSpAlgorithm->increment_justified_width(i+m_iFirstCol, uDif[i]);
        }
    }
#endif
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::finish_column_measurements(int iCol, LUnits xStart)
{
    m_ColLayouters[iCol]->finish_column_measurements(xStart);
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::include_object(ColStaffObjsEntry* pCurEntry, int iCol, int iLine, int iInstr, ImoStaffObj* pSO,
                                    TimeUnits rTime, int iStaff, GmoShape* pShape,
                                    bool fInProlog)
{
    m_ColLayouters[iCol]->include_object(iLine, iInstr, pSO, rTime, iStaff, pShape,
                                         fInProlog);
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::start_column_measurements(int iCol, LUnits uxStart,
                                                 LUnits fixedSpace)
{
    m_ColLayouters[iCol]->start_column_measurements(uxStart, fixedSpace);
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::prepare_for_new_column(int UNUSED(iCol))
{
    ColumnStorage* pStorage = LOMSE_NEW ColumnStorage();
    ColumnLayouter* pLyt = LOMSE_NEW ColumnLayouter(m_libraryScope, m_pScoreMeter,
                           pStorage, this);
    m_ColLayouters.push_back( pLyt );
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::assign_width_to_column(int iCol)
{
    LUnits size = m_ColLayouters[iCol]->get_main_width();
    if (iCol > 0)
    {
        LUnits uEndVarSp = m_ColLayouters[iCol-1]->get_end_hook_width();
        LUnits uStartVarSp = m_ColLayouters[iCol]->get_start_hook_width();
        if (uEndVarSp > uStartVarSp)
        {
            LUnits prevSize = m_ColLayouters[iCol-1]->get_trimmed_width();
            prevSize += (uEndVarSp - uStartVarSp);
            m_ColLayouters[iCol-1]->set_trimmed_width(prevSize);
        }
    }
    m_ColLayouters[iCol]->set_trimmed_width(size);
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::do_spacing(int iCol, bool fTrace, int level)
{
    m_ColLayouters[iCol]->do_spacing(fTrace, level);
}

//---------------------------------------------------------------------------------------
bool SpAlgorithmTimetable::column_has_visible_barline(int iCol)
{
    return m_ColLayouters[iCol]->column_has_visible_barline();
}

////---------------------------------------------------------------------------------------
//void SpAlgorithmTimetable::set_trace_level(int iCol, int nTraceLevel)
//{
//    m_ColLayouters[iCol]->set_trace_level(nTraceLevel);
//}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::add_shapes_to_box(int iCol, GmoBoxSliceInstr* pSliceInstrBox,
                                             int iInstr)
{
    m_ColLayouters[iCol]->add_shapes_to_box(pSliceInstrBox, iInstr);
}

//---------------------------------------------------------------------------------------
void SpAlgorithmTimetable::delete_shapes(int iCol)
{
    m_ColLayouters[iCol]->delete_shapes();
}



//=======================================================================================
//ColumnLayouter
//=======================================================================================
ColumnLayouter::ColumnLayouter(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                               ColumnStorage* pStorage, SpacingAlgorithm* pSpAlgorithm)
    : m_libraryScope(libraryScope)
    , m_pColStorage(pStorage)
    , m_pScoreMeter(pScoreMeter)
//    , m_fHasSystemBreak(false)
    , m_penalty(1.0f)
    , m_fHasShapes(false)
    , m_yMin(10000000.0f)
    , m_yMax(-10000000.0f)
    , m_pSpAlgorithm(pSpAlgorithm)
    , m_nTraceLevel(k_trace_off)
{
//    reserve_space_for_prolog_clefs_keys( m_pScoreMeter->num_staves() );
}

//---------------------------------------------------------------------------------------
ColumnLayouter::~ColumnLayouter()
{
    delete_line_spacers();
    delete m_pColStorage;
}

////---------------------------------------------------------------------------------------
//void ColumnLayouter::reserve_space_for_prolog_clefs_keys(int numStaves)
//{
//    m_prologClefs.clear();
//    m_prologClefs.reserve(numStaves);
//    m_prologClefs.assign(numStaves, (ColStaffObjsEntry*)NULL);     //GCC complais if NULL not casted
//
//    m_prologKeys.clear();
//    m_prologKeys.reserve(numStaves);
//    m_prologKeys.assign(numStaves, (ColStaffObjsEntry*)NULL);
//}

//---------------------------------------------------------------------------------------
void ColumnLayouter::delete_line_spacers()
{
    LineSpacersIterator it;
    for (it = m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
        delete *it;
    m_LineSpacers.clear();
}

//---------------------------------------------------------------------------------------
bool ColumnLayouter::column_has_barline()
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
bool ColumnLayouter::column_has_visible_barline()
{
    //returns true if there is at least one line containing a barline

    for (LinesIterator it=m_pColStorage->begin(); it != m_pColStorage->end(); ++it)
    {
        if ((*it)->contains_visible_barline())
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::do_spacing(bool fTrace, int UNUSED(level))
{
    //computes the minimum space required by this column

    m_spacingStartTime = microsec_clock::universal_time();

    if (fTrace || m_libraryScope.dump_column_tables())
    {
        dbgLogger << endl << to_simple_string( microsec_clock::local_time() )
                  << " ******************* Before spacing" << endl;
        m_pColStorage->dump_column_storage(dbgLogger);
    }

    compute_spacing();
    compute_penalty_factor();
    m_pColStorage->determine_sizes();

    if (fTrace || m_libraryScope.dump_column_tables())
    {
        ptime now = microsec_clock::universal_time();
        time_duration diff = now - m_spacingStartTime;
        dbgLogger << to_simple_string( microsec_clock::local_time() )
                  << " - (" << diff.total_microseconds()
                  << "µs) ******************* After spacing" << endl;
        m_pColStorage->dump_column_storage(dbgLogger);
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::compute_spacing()
{
    //Spacing algorithm. Returns the resulting minimum column width

    if (there_are_lines())
    {
        create_line_spacers();
        process_non_timed_at_prolog();
        process_barlines_at_current_timepos();
        process_timed_at_current_timepos();
        while (there_are_objects())
        {
            process_non_timed_at_current_timepos();
            process_barlines_at_current_timepos();
            process_timed_at_current_timepos();
        }

        delete_line_spacers();
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::compute_penalty_factor()
{
    int barlines = 0;
    int lines = 0;
    for (LinesIterator it=m_pColStorage->begin(); it != m_pColStorage->end(); ++it)
    {
        lines++;
        if ((*it)->contains_barline())
            barlines++;
    }
    m_penalty = barlines == 0 ? 0.4f : (barlines < lines ? 0.6f : 1.0f);
    //m_penalty = barlines == 0 ? 1000000.0f :  (1.0f - barlines / lines);

}

//---------------------------------------------------------------------------------------
void ColumnLayouter::create_line_spacers()
{
    const LinesIterator itEnd = m_pColStorage->end();
    for (LinesIterator it=m_pColStorage->begin(); it != itEnd; ++it)
	{
        LineSpacer* pLinSpacer = LOMSE_NEW LineSpacer(*it, m_pScoreMeter);
        m_LineSpacers.push_back(pLinSpacer);
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_non_timed_at_prolog()
{
    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "************ Entering ColumnLayouter"
            << "::process_non_timed_at_prolog." << endl;
    }

    LUnits uSpaceAfterProlog =
        m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_AFTER_PROLOG, 0, 0);
    m_rCurrentTime = LOMSE_NO_TIME;           //any impossible high value
    m_uCurrentPos = 0.0f;
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        (*it)->process_non_timed_at_prolog(uSpaceAfterProlog);
        m_rCurrentTime = min(m_rCurrentTime, (*it)->get_next_available_time());
        m_uCurrentPos = max(m_uCurrentPos, (*it)->get_next_position());
    }

    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "After process_non_timed_at_prolog. m_uCurrentPos="
            << m_uCurrentPos << ", m_rCurrentTime=" << m_rCurrentTime << endl;
        dump_column_data(dbgLogger);
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_barlines_at_current_timepos()
{
    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "************ Entering ColumnLayouter"
            << "::process_barlines_at_current_timepos. m_uCurrentPos="
            << m_uCurrentPos << ", m_rCurrentTime=" << m_rCurrentTime << endl;

    }

    LUnits uxPosForNextTime = m_uCurrentPos;        //next position, at least current one

    //determine next valid position
    LUnits uNextPos = m_uCurrentPos;
    bool fBarline = false;
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        if ((*it)->current_time_is(m_rCurrentTime) && (*it)->is_middle_barline())
        {
            fBarline = true;
            uNextPos = max(uNextPos, (*it)->determine_next_feasible_position_after(m_uCurrentPos));
        }
    }

    if (fBarline)
    {
        m_uCurrentPos = uNextPos;

        //position barlines
        for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
        {
            if ((*it)->current_time_is(m_rCurrentTime) && (*it)->is_middle_barline())
            {
                (*it)->process_barline_at_current_timepos(m_uCurrentPos);
                LUnits uxNextPos = (*it)->get_next_position();
                uxPosForNextTime = max(uxPosForNextTime, uxNextPos);
            }
        }

        m_uCurrentPos = uxPosForNextTime;
    }

    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "After process_barlines_at_current_timepos. m_uCurrentPos="
            << m_uCurrentPos << ", m_rCurrentTime=" << m_rCurrentTime << endl;
        dump_column_data(dbgLogger);
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_timed_at_current_timepos()
{
    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "************ Entering ColumnLayouter"
            << "::process_timed_at_current_timepos. m_uCurrentPos="
            << m_uCurrentPos << ", m_rCurrentTime=" << m_rCurrentTime << endl;

    }

    m_fThereAreObjects = false;
    TimeUnits rNextTime = LOMSE_NO_TIME;            //any impossible high value
    LUnits uxPosForNextTime = m_uCurrentPos;        //next position, at least current one

    //determine next valid position
    LUnits uNextPos = m_uCurrentPos;
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        if ((*it)->current_time_is(m_rCurrentTime) && (*it)->are_there_timed_objs())
            uNextPos = max(uNextPos, (*it)->determine_next_feasible_position_after(m_uCurrentPos));
    }
    m_uCurrentPos = uNextPos;

    //position timed objects
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        if ((*it)->current_time_is(m_rCurrentTime) && (*it)->are_there_timed_objs())
        {
            (*it)->process_timed_at_current_timepos(m_uCurrentPos);
            LUnits uxNextPos = (*it)->get_next_position();
            uxPosForNextTime = max(uxPosForNextTime, uxNextPos);
        }
        if ((*it)->are_there_more_objects())
        {
            m_fThereAreObjects = true;
            rNextTime = min(rNextTime, (*it)->get_next_available_time());
        }
    }

    m_rCurrentTime = rNextTime;
    m_uCurrentPos = uxPosForNextTime;

    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "After process_timed_at_current_timepos. m_uCurrentPos="
            << m_uCurrentPos << ", m_rCurrentTime=" << m_rCurrentTime << endl;
        dump_column_data(dbgLogger);
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_non_timed_at_current_timepos()
{
    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "************ Entering ColumnLayouter"
            << "::process_non_timed_at_current_timepos. m_uCurrentPos="
            << m_uCurrentPos << endl;
    }

    LUnits uxPosForNextTime = 0.0f;
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        (*it)->process_non_timed_at_current_timepos(m_uCurrentPos);
        LUnits uxNextPos = (*it)->get_next_position();
        uxPosForNextTime = max(uxPosForNextTime, uxNextPos);
    }
    m_uCurrentPos = uxPosForNextTime;

    if (m_nTraceLevel && k_trace_spacing)
    {
        dbgLogger << "After process_non_timed_at_current_timepos. m_uCurrentPos="
            << m_uCurrentPos << endl;
        dump_column_data(dbgLogger);
    }
}

////---------------------------------------------------------------------------------------
//GmoBoxSliceInstr* ColumnLayouter::create_slice_instr(ImoInstrument* pInstr, LUnits yTop)
//{
//    GmoBoxSliceInstr* pBSI = m_pBoxSlice->add_box_for_instrument(pInstr);
//	pBSI->set_top(yTop);
//	pBSI->set_left( m_pBoxSlice->get_left() );
//	pBSI->set_width( m_pBoxSlice->get_width() );
//    m_sliceInstrBoxes.push_back( pBSI );
//    return pBSI;
//}

//---------------------------------------------------------------------------------------
void ColumnLayouter::add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr)
{
    m_pColStorage->add_shapes(pSliceInstrBox, iInstr);
}
//void ColumnLayouter::add_shapes_to_boxes(ShapesStorage* pStorage)
//{
//    for (int iInstr=0; iInstr < int(m_sliceInstrBoxes.size()); ++iInstr)
//    {
//        m_pColStorage->add_shapes(m_sliceInstrBoxes[iInstr], iInstr);
//        pStorage->add_ready_shapes_to_model( m_sliceInstrBoxes[iInstr] );
//    }
//}

//---------------------------------------------------------------------------------------
void ColumnLayouter::delete_shapes()
{
    m_pColStorage->delete_shapes();
}

////---------------------------------------------------------------------------------------
//void ColumnLayouter::delete_box_and_shapes(ShapesStorage* pStorage)
//{
//    delete_shapes();
//    pStorage->delete_ready_shapes();
//
////   delete m_pBoxSlice;       //box for this column
//}

////---------------------------------------------------------------------------------------
//void ColumnLayouter::set_slice_width(LUnits width)
//{
//    m_pBoxSlice->set_width(width);
//
//    //set instrument slices width
//    std::vector<GmoBoxSliceInstr*>::iterator it;
//    for (it=m_sliceInstrBoxes.begin(); it != m_sliceInstrBoxes.end(); ++it)
//    {
//        (*it)->set_width(width);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void ColumnLayouter::set_slice_final_position(LUnits left, LUnits top)
//{
//    m_pBoxSlice->new_left(left);
//    m_pBoxSlice->new_top(top);
//
//    //Re-position instrument slices
//    std::vector<GmoBoxSliceInstr*>::iterator it;
//    for (it=m_sliceInstrBoxes.begin(); it != m_sliceInstrBoxes.end(); ++it)
//    {
//        (*it)->new_left(left);
//        (*it)->new_top(top);
//        top += (*it)->get_height();
//    }
//}

//---------------------------------------------------------------------------------------
LUnits ColumnLayouter::redistribute_space(LUnits uNewStart, LUnits uNewWidth,
                                          UPoint org)
{
    ColumnResizer oResizer(m_pColStorage, uNewWidth);
	LUnits xPos = oResizer.reposition_shapes(uNewStart, uNewWidth, org);
    if (oResizer.has_shapes())
    {
        m_fHasShapes = true;
        m_yMin = min(m_yMin, oResizer.get_y_min());
        m_yMax = max(m_yMax, oResizer.get_y_max());
    }
    return xPos;
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::start_column_measurements(LUnits uxStart, LUnits fixedSpace)
{
    //prepare to receive data for this column

    m_pColStorage->set_start_position(uxStart);
    m_pColStorage->set_initial_space(fixedSpace);
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::include_object(int iLine, int iInstr, ImoStaffObj* pSO, TimeUnits rTime,
                                   int iStaff, GmoShape* pShape, bool fInProlog)
{
    //caller sends data about one staffobj in current bar, for column iCol [0..n-1]

    LUnits xUserShift =
        m_pScoreMeter->tenths_to_logical(pSO->get_user_location_x(), iInstr, iStaff);
    LUnits yUserShift =
        m_pScoreMeter->tenths_to_logical(pSO->get_user_location_y(), iInstr, iStaff);

    if (m_nTraceLevel && k_trace_entries)
    {
        dbgLogger << "Including object: iLine=" << iLine
            << ", iInstr=" << iInstr
            << ", SO [" << pSO->get_id() << "] type=" << pSO->get_name()
            << ", rTime=" << rTime
            << ", iStaff=" << iStaff
            << ", xUserShift=" << xUserShift
            << ", yUserShift=" << yUserShift
            << endl;
    }

    m_pColStorage->include_object(iLine, iInstr, pSO, rTime, iStaff, pShape, fInProlog,
                                  xUserShift, yUserShift);
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::finish_column_measurements(LUnits xStart)
{
    m_pColStorage->finish_column_measurements(xStart);
    //m_slices.finish_column(xStart);
}

//---------------------------------------------------------------------------------------
LUnits ColumnLayouter::get_start_of_column()
{
    return m_pColStorage->get_start_of_column();
}

//---------------------------------------------------------------------------------------
LineEntry* ColumnLayouter::get_entry_for(ImoId id)
{
    return m_pColStorage->get_entry_for(id);
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::dump_column_data(ostream& outStream)
{
    m_pColStorage->dump_column_storage(outStream);
}

//---------------------------------------------------------------------------------------
bool ColumnLayouter::is_empty_column()
{
    return m_pColStorage->size() == 0;
}

////---------------------------------------------------------------------------------------
//void ColumnLayouter::save_context(int iInstr, int iStaff, ColStaffObjsEntry* pClefEntry,
//                                  ColStaffObjsEntry* pKeyEntry)
//{
//    int idx = m_pScoreMeter->staff_index(iInstr, iStaff);
//    m_prologClefs[idx] = pClefEntry;
//    m_prologKeys[idx] = pKeyEntry;
//}

//---------------------------------------------------------------------------------------
TimeGridTable* ColumnLayouter::create_time_grid_table_for_column()
{
    TimeGridTableBuilder builder(m_pColStorage);
    return builder.build_table();
}


//=====================================================================================
//LineEntry implementation
//=====================================================================================
LineEntry::LineEntry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog, TimeUnits rTime,
                     LUnits xUserShift, LUnits yUserShift)
    : m_fIsBarlineEntry(false)
    , m_pSO(pSO)
    , m_pShape(pShape)
	, m_fProlog(fProlog)
    , m_rTimePos(rTime)
    , m_xLeft(0.0f)
    , m_uxAnchor(0.0f)
    , m_xFinal(0.0f)
    , m_uSize(0.0f)
    , m_uFixedSpace(0.0f)
    , m_uVariableSpace(0.0f)
    , m_xUserShift(xUserShift)
    , m_yUserShift(yUserShift)
{
    //AWARE: At this moment is not possible to use shape information because, as
    //laying out continues and more objects are added to the line, the shape
    //position or its geometry could change (i.e chord engraving). Therefore,
    //information from the shape is obtained later, when starting justification,
    //and method LineEntry::add_shape_info() is invoked.
    m_fShapeInfoLoaded = false;
}

//---------------------------------------------------------------------------------------
//constructor for unit tests
LineEntry::LineEntry(bool fIsBarlineEntry, bool fProlog, TimeUnits rTime, LUnits xAnchor,
                     LUnits xLeft, LUnits uSize, LUnits uFixedSpace,
                     LUnits uVarSpace, LUnits xFinal)
    : m_fIsBarlineEntry(fIsBarlineEntry)
    , m_pSO(NULL)
    , m_pShape(NULL)
	, m_fProlog(fProlog)
    , m_rTimePos(rTime)
    , m_xLeft(xLeft)
    , m_uxAnchor(xAnchor)
    , m_xFinal(xFinal)
    , m_uSize(uSize)
    , m_uFixedSpace(uFixedSpace)
    , m_uVariableSpace(uVarSpace)
    , m_xUserShift(0.0f)
    , m_yUserShift(0.0f)
{
}

//---------------------------------------------------------------------------------------
void LineEntry::add_shape_info()
{
    if (m_pShape)
    {
        m_xLeft = m_pShape->get_left();
        m_uxAnchor = m_pShape->get_anchor_offset();
        m_uSize = m_pShape->get_width();
    }
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
bool LineEntry::has_visible_barline()
{
    return m_pSO && m_pSO->is_barline() && m_pSO->is_visible();
}

//---------------------------------------------------------------------------------------
void LineEntry::reposition_at(LUnits uxNewXLeft)
{
    m_xLeft = uxNewXLeft;
    m_xFinal = m_xLeft + get_total_size();
}

//---------------------------------------------------------------------------------------
void LineEntry::move_shape(UPoint sliceOrg)
{
    if (m_pSO && m_pShape)
    {
        m_pShape->set_origin_and_notify_observers(m_xLeft + m_xUserShift,
                                                  sliceOrg.y + m_yUserShift);
    }
}

//---------------------------------------------------------------------------------------
void LineEntry::dump_header(ostream& outStream)
{
    //              ...+  ..+   ...+ ..+   +  ..........+........+........+........+........+........+........+........+......+
    outStream << "item    Type      ID Prolog   Timepos  xAnchor    xLeft     size  SpFixed    SpVar    Space   xFinal ShpIdx" << endl;
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
void LineEntry::dump(int iEntry, ostream& outStream)
{
    if (!m_fShapeInfoLoaded)
    {
        add_shape_info();
        m_fShapeInfoLoaded = true;
    }

    outStream << setw(4) << iEntry << ": ";      //"%4d: "
//    if (m_fIsBarlineEntry)
//    {
//        outStream << "  Omega";
//        if (m_pSO)
//            outStream << setw(4) << m_pSO->get_obj_type();
//        else
//            outStream << "  - ";
//        outStream << "         ";
//    }
//    else
    {
        string name = m_pSO->get_name();
        name.resize(10, ' ');
		outStream << name << setw(4) << m_pSO->get_id()
                  << (m_fProlog ? "   S  " : "      ");
    }

    outStream << fixed << setprecision(2) << setfill(' ')
                << setw(11) << round_half_up(m_rTimePos)
                << setw(9) << round_half_up(get_anchor())
                << setw(9) << round_half_up(m_xLeft)
                << setw(9) << round_half_up(m_uSize)
                << setw(9) << round_half_up(m_uFixedSpace)
                << setw(9) << round_half_up(m_uVariableSpace)
                << setw(9) << round_half_up(get_total_size())
                << setw(9) << round_half_up(m_xFinal);

    //if (m_pShape)
    //    outStream << "  " << setw(4) << m_pShape->GetOwnerIDX() << "\r\n";
    //else
        outStream << "    --" << endl;
}



//=======================================================================================
//MusicLine:
//  An object to encapsulate positioning data for a line
//=======================================================================================
MusicLine::MusicLine(int line, int nInstr, LUnits uxStart, LUnits fixedSpace)
    : m_line(line)
    , m_nInstr(nInstr)
	, m_nVoice(line+1)
    , m_uxLineStart(uxStart)
    , m_uStartFixedSpace(fixedSpace)
    , m_uxFirstSymbol(0.0f)
    , m_uxFirstAnchor(0.0f)
    , m_uxRightEdge(0.0f)
    , m_uxStartOfEndVarSpace(0.0f)
    , m_barlineType(k_barline_unknown)
{
}

//---------------------------------------------------------------------------------------
MusicLine::~MusicLine()
{
    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
		delete *it;
    m_LineEntries.clear();
}

//---------------------------------------------------------------------------------------
LineEntry* MusicLine::add_entry(ImoStaffObj* pSO, GmoShape* pShape, TimeUnits rTime,
                                bool fInProlog, LUnits xUserShift, LUnits yUserShift)
{
    LineEntry* pEntry = LOMSE_NEW LineEntry(pSO, pShape, fInProlog, rTime,
                                            xUserShift, yUserShift);
    push_back(pEntry);
	return pEntry;
}

//---------------------------------------------------------------------------------------
void MusicLine::add_shapes(GmoBoxSliceInstr* pSliceInstrBox)
{
    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
    {
        GmoShape* pShape = (*it)->get_shape();
        if (pShape)
            pSliceInstrBox->add_shape(pShape, GmoShape::k_layer_notes);
    }
}

//---------------------------------------------------------------------------------------
void MusicLine::delete_shapes()
{
    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
    {
        delete (*it)->get_shape();
    }
}

//---------------------------------------------------------------------------------------
bool MusicLine::contains_barline()
{
    if (m_barlineType == k_barline_unknown)
        determine_barline_type();

    return m_barlineType > k_no_barline;
}

//---------------------------------------------------------------------------------------
bool MusicLine::contains_visible_barline()
{
    if (m_barlineType == k_barline_unknown)
        determine_barline_type();

    return m_barlineType == k_visible_barline;
}

//---------------------------------------------------------------------------------------
void MusicLine::determine_barline_type()
{
    LineEntry* pEntry = get_last_entry();
    if (pEntry->has_barline())
        m_barlineType = (pEntry->has_visible_barline() ?
                            k_visible_barline : k_no_visible_barline);
    else
        m_barlineType = k_no_barline;
}

//---------------------------------------------------------------------------------------
void MusicLine::dump_music_line(ostream& outStream)
{
    outStream << fixed << setprecision(2) << setfill(' ')
                << "Line table dump. Instr=" << get_instrument()
                << ", voice=" << get_voice()
                << ", xStart=" << setw(2) << get_line_start_position()
                << ", FixedSpace=" << setw(2) << get_fixed_space_at_start()
                << endl
                << "============================================================================================================="
                << endl << endl;

    if (size() == 0)
    {
        outStream << "The table is empty." << endl;
        return;
    }

    //headers
    LineEntry::dump_header(outStream);

    //loop to dump table entries
    for (int i = 0; i < (int)size(); i++)
    {
        if (i % 4 == 0) {
            outStream << "-------------------------------------------------------------------------------------------------------------"
                      << endl;
        }
        LineEntry* pTE = item(i);
        pTE->dump(i, outStream);
    }

    outStream << "============================================================================================================="
              << endl   //\r\n"
              << fixed << setprecision(2) << setfill(' ')
              << "VarAfterSpace=" << setw(2) << get_end_hook_width()
              << endl << endl;  //"\r\n\r\n";
}

//---------------------------------------------------------------------------------------
LineEntry* MusicLine::get_entry_for(ImoId id)
{
    for (int i = 0; i < (int)size(); i++)
    {
        LineEntry* pEntry = item(i);
        if (pEntry->get_staffobj()->get_id() == id)
            return pEntry;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
void MusicLine::do_measurements()
{
	if (m_LineEntries.size() <= 0)
        return;

    bool fFirstAnchorFound = false;
    m_rFirstTime = LOMSE_NO_TIME;
    m_uxFirstSymbol = LOMSE_NO_POSITION;
    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
    {
        LUnits xr = (*it)->get_position() + (*it)->get_shape_size();
        m_uxRightEdge = max(m_uxRightEdge, xr);
        m_uxStartOfEndVarSpace = max(m_uxStartOfEndVarSpace, xr + (*it)->get_fixed_space());
        if (!fFirstAnchorFound)
        {
            m_uxFirstAnchor = (*it)->get_position() - (*it)->get_anchor();
            fFirstAnchorFound = (*it)->get_timepos() >= 0.0f;
        }
        if (is_equal_time(m_rFirstTime, LOMSE_NO_TIME)
            && !is_lower_time( (*it)->get_timepos(), 0.0) )
        {
            m_rFirstTime = (*it)->get_timepos();
        }
        if (is_equal_time(m_rFirstTime, (*it)->get_timepos()))
            m_uxFirstSymbol = min(m_uxFirstSymbol, m_uxFirstAnchor + (*it)->get_anchor());
    }
    if (m_rFirstTime == LOMSE_NO_TIME)
    {
        m_uxFirstSymbol = m_uxFirstAnchor;
        m_rFirstTime = -1.0;
    }
}

//---------------------------------------------------------------------------------------
LUnits MusicLine::get_line_width()
{
	//Return the size of the column represented by this line or zero if invalid line

	if (m_LineEntries.size() > 0)
    {
        LUnits start = get_line_start_position() < get_start_of_first_symbol() ?
                            get_line_start_position() : get_start_of_first_symbol();

        return m_LineEntries.back()->get_x_final() - start;
    }
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
LUnits MusicLine::get_end_hook_width()
{
	if (m_LineEntries.size() > 0)
        return m_LineEntries.back()->get_variable_space();
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
LUnits MusicLine::get_fixed_space_at_end()
{
	if (m_LineEntries.size() > 0)
        return m_LineEntries.back()->get_fixed_space();
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
LUnits MusicLine::get_final_pos()
{
	if (m_LineEntries.size() > 0)
        return m_LineEntries.back()->get_x_final();
    else
        return 0.0f;
}


//=======================================================================================
// ColumnStorage implementation: encapsulates the table of lines for a column
//=======================================================================================
ColumnStorage::ColumnStorage()
    : m_uTrimmedSize(0.0f)
    , m_uFixedSpaceForNextColumn(0.0f)
    , m_uEndHookWidth(0.0f)
    , m_uxStart(0.0f)
    , m_uStartFixedSpace(0.0f)
    , m_uxFirstSymbol(0.0f)
    , m_uxFirstAnchor(0.0f)
    , m_uxRightEdge(0.0f)
    , m_uxStartOfEndVarSpace(0.0f)
    , m_uxFinal(0.0f)
    , m_fVisibleBarline(false)
{
}

//---------------------------------------------------------------------------------------
ColumnStorage::~ColumnStorage()
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

//---------------------------------------------------------------------------------------
MusicLine* ColumnStorage::open_new_line(int line, int instr, LUnits uxStart,
                                        LUnits fixedSpace)
{
    MusicLine* pLineTable = LOMSE_NEW MusicLine(line, instr, uxStart, fixedSpace);
    m_Lines.push_back(pLineTable);
    return pLineTable;
}

//---------------------------------------------------------------------------------------
void ColumnStorage::dump_column_storage(ostream& outStream)
{
    outStream << "Start of dump. ColumnStorage" << endl;
	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
	{
        (*it)->dump_music_line(outStream);
    }
}

//---------------------------------------------------------------------------------------
LineEntry* ColumnStorage::get_entry_for(ImoId id)
{
	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
    {
        LineEntry* pEntry = (*it)->get_entry_for(id);
        if (pEntry != NULL)
            return pEntry;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
void ColumnStorage::determine_sizes()
{
    TimeUnits rFirstTime = 1000000000000000.0;
	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
    {
        (*it)->do_measurements();

        TimeUnits rTime = (*it)->get_first_time();
        if (rTime >= 0.0f)
            rFirstTime = min(rFirstTime, rTime);
    }
    if (is_equal_time(rFirstTime, 1000000000000000.0))
        rFirstTime = -1;

    m_uxFinal = 0.0f;
    LUnits minEnd = 0.0f;
    LUnits fixed = 0.0f;
    m_fVisibleBarline = false;
    m_uxFirstSymbol = 10000000.0f;

	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
    {
        LUnits maxX = (*it)->get_final_pos();
        LUnits minX = maxX - (*it)->get_end_hook_width();
        if (fixed == 0.0f && (*it)->contains_visible_barline())
            fixed = (*it)->get_fixed_space_at_end();
        minEnd = max(minEnd, minX);
        m_uxFinal = max(m_uxFinal, maxX);
        m_uxRightEdge = max(m_uxRightEdge, (*it)->get_right_most_edge());
        m_uxStartOfEndVarSpace = max(m_uxStartOfEndVarSpace,
                                     (*it)->get_start_of_final_var_space());
        m_fVisibleBarline |= (*it)->contains_visible_barline();

        if (is_equal_time(rFirstTime, (*it)->get_first_time()))
        {
            m_uxFirstSymbol = min(m_uxFirstSymbol, (*it)->get_start_of_first_symbol());
        }
    }
    m_uEndHookWidth = m_uxFinal - minEnd;
    m_uFixedSpaceForNextColumn = fixed;

    determine_first_anchor_line();
}

//---------------------------------------------------------------------------------------
LUnits ColumnStorage::get_main_width()
{
    LUnits start = get_start_of_column() < get_start_of_first_symbol() ?
                        get_start_of_column() : get_start_of_first_symbol();

    if (m_fVisibleBarline)
        return get_right_most_edge() - start;
    else
        return get_start_of_final_var_space() - start;
}

//---------------------------------------------------------------------------------------
LUnits ColumnStorage::get_start_hook_width()
{
    if (get_fixed_space_at_start() > 0.0f)
        return 0.0f;
    else
        return m_uxFirstAnchor - min(m_uxStart, m_uxFirstSymbol);
}

//---------------------------------------------------------------------------------------
void ColumnStorage::determine_first_anchor_line()
{
    //if (get_fixed_space_at_start() > 0.0f)
    //    m_uxFirstAnchor = get_start_of_column();
    //else
    {
        m_uxFirstAnchor = 10000000.0f;
	    for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
        {
            m_uxFirstAnchor = min(m_uxFirstAnchor, (*it)->get_first_anchor_position());
        }
    }
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

//---------------------------------------------------------------------------------------
void ColumnStorage::delete_shapes()
{
	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
    {
        (*it)->delete_shapes();
    }
}

//---------------------------------------------------------------------------------------
void ColumnStorage::close_all_lines(LUnits UNUSED(xStart))
{
	for (LinesIterator it=m_Lines.begin(); it != m_Lines.end(); ++it)
    {
        LineEntry* pEntry = (*it)->back();
        pEntry->mark_as_barline_entry();
    }
}

//---------------------------------------------------------------------------------------
LinesIterator ColumnStorage::start_line(int line, int instr)
{
    //Start a new line for instrument instr (0..n-1), to be used for voice voice.

    open_new_line(line, instr, m_uxStart, m_uStartFixedSpace);
    return get_last_line();
}

//---------------------------------------------------------------------------------------
void ColumnStorage::finish_column_measurements(LUnits xStart)
{
    close_all_lines(xStart);
}

//---------------------------------------------------------------------------------------
bool ColumnStorage::include_object(int line, int instr, ImoStaffObj* pSO, TimeUnits rTime,
                                  int UNUSED(nStaff), GmoShape* pShape, bool fInProlog,
                                  LUnits xUserShift, LUnits yUserShift)
{
    //if doesn't exist, start it
    LinesIterator it = find_line(line);
    if (is_end_of_table(it))
        it = start_line(line, instr);

    //add new entry for this object
	LineEntry* pCurEntry =
        (*it)->add_entry(pSO, pShape, rTime, fInProlog, xUserShift, yUserShift);

	return pCurEntry->is_prolog_object();
}



//=======================================================================================
// LineResizer implementation:
//      encapsulates the methods to recompute shapes positions so that the column
//      will have the desired width, and to move the shapes to those positions
//=======================================================================================
LineResizer::LineResizer(MusicLine* pLine, LUnits uOldWidth,
                         LUnits uNewWidth, LUnits uNewStart, UPoint sliceOrg)
    : m_pLine(pLine)
    , m_uOldWidth(uOldWidth)
    , m_uNewWidth(uNewWidth)
    , m_uNewStart(uNewStart)
    , m_sliceOrg(sliceOrg)
    , m_fHasShapes(false)
    , m_yMin(10000000.0f)
    , m_yMax(-10000000.0f)
{
}

//---------------------------------------------------------------------------------------
TimeUnits LineResizer::move_prolog_shapes()
{
    //all non-timed entries, at beginning, marked as fProlog must be only re-located
    //returns the first timepos found after the prolog or 0 if no valid timepos

    LUnits uLineStartPos = m_pLine->get_line_start_position();
    LUnits uLineShift = m_uNewStart - uLineStartPos;
    LineEntryIterator it = m_pLine->begin();
    while (it != m_pLine->end() && is_lower_time((*it)->get_timepos(), 0.0))
    {
        if ((*it)->get_shape())
        {
            if ((*it)->is_prolog_object())
            {
                LUnits uNewPos = uLineShift + (*it)->get_position();
                (*it)->reposition_at(uNewPos);
                (*it)->move_shape(m_sliceOrg);

                //get shape limits
                GmoShape* pShape = (*it)->get_shape();
                if (pShape && !pShape->is_shape_invisible())
                {
                    m_yMax = max(m_yMax, pShape->get_bottom());
                    m_yMin = min(m_yMin, pShape->get_top());
                    m_fHasShapes = true;
                }
            }
            else
			    break;
        }
        ++it;
    }
    m_itCurrent = it;

    //return first timepos in this line
    if (it != m_pLine->end())
    {
        if ((*it)->get_timepos() < 0.0)
            return 0.0;
        else
            return (*it)->get_timepos();
    }
    else
        return 0.0;
}

//---------------------------------------------------------------------------------------
LUnits LineResizer::get_time_line_position_for_time(TimeUnits rFirstTime)
{
    if (m_itCurrent != m_pLine->end()
        && is_equal_time((*m_itCurrent)->get_timepos(), rFirstTime) )
    {
        return (*m_itCurrent)->get_position() - (*m_itCurrent)->get_anchor();
    }
    else
        return 0.0;
}

//---------------------------------------------------------------------------------------
void LineResizer::reasign_position_to_all_other_objects(LUnits uFizedSizeAtStart)
{
    if (m_itCurrent == m_pLine->end())
        return;

    //Compute proportion factor
    LUnits uLineStartPos = m_pLine->get_line_start_position();
    LUnits uDiscount = uFizedSizeAtStart - uLineStartPos;
    float rProp = 1.0f;
    if (m_uNewWidth-uDiscount != m_uOldWidth-uDiscount)
        rProp = (m_uNewWidth-uDiscount) / (m_uOldWidth-uDiscount);

	//Reposition the remainder entries
    for (LineEntryIterator it = m_itCurrent; it != m_pLine->end(); ++it)
	{
//        if ((*it)->is_barline_entry())
//        {
//            LUnits uNewPos = m_uNewStart + m_uNewWidth - (*it)->get_shape_size();
//            (*it)->reposition_at(uNewPos);
//            (*it)->move_shape(m_sliceOrg);
//        }
//        else
        {
            LUnits uOldPos = (*it)->get_position() - (*it)->get_anchor();
            LUnits uShift = uDiscount + (m_uNewStart + (uOldPos - uFizedSizeAtStart) * rProp) - uOldPos;
            LUnits uNewPos = uOldPos + uShift + (*it)->get_anchor();;
            (*it)->reposition_at(uNewPos);
            (*it)->move_shape(m_sliceOrg);

            //get shape limits
            GmoShape* pShape = (*it)->get_shape();
            if (pShape && !pShape->is_shape_invisible())
            {
                m_yMax = max(m_yMax, pShape->get_bottom());
                m_yMin = min(m_yMin, pShape->get_top());
                m_fHasShapes = true;
            }
        }
    }
}


//=======================================================================================
//LineSpacer:
//  encapsulates the algorithm to assign spaces and positions to a single line
//=======================================================================================
LineSpacer::LineSpacer(MusicLine* pLineTable, ScoreMeter* pMeter)
    : m_pLine(pLineTable)
    , m_rFactor(pMeter->get_spacing_factor())
    , m_pMeter(pMeter)
    , m_itCur(pLineTable->end())
    , m_rCurTime(0.0)
	, m_uxCurPos(0.0f)
    , m_uxRemovable(0.0f)
{
    add_shapes_info_to_table();
    prepare_for_traversing();
}

//---------------------------------------------------------------------------------------
void LineSpacer::add_shapes_info_to_table()
{
    LineEntryIterator it;
    for (it = m_pLine->begin(); it != m_pLine->end(); ++it)
        (*it)->add_shape_info();
}

//---------------------------------------------------------------------------------------
void LineSpacer::prepare_for_traversing()
{
    //initialize iteration control data, to traverse by timepos

    m_itCur = m_pLine->begin();
    m_rCurTime = get_next_available_time();
    m_uxCurPos = m_pLine->get_line_start_position() + m_pLine->get_fixed_space_at_start();
    m_itNonTimedAtCurPos = m_pLine->end();
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
        LineEntryIterator itPrev = m_itCur;
	    while (is_current_object_non_timed())
        {
            assign_fixed_and_variable_space(*m_itCur);
            (*m_itCur)->reposition_at(uxNextPos);

            uxNextPos += (*m_itCur)->get_total_size();
            itPrev = m_itCur;
            ++m_itCur;
        }
        m_uxCurPos = uxNextPos;

        //add some additional space after prolog
        if ((*itPrev)->is_prolog_object())
        {
            m_uxCurPos += uSpaceAfterProlog;
            m_uxRemovable = uSpaceAfterProlog;
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::determine_next_feasible_position_after(LUnits UNUSED(uxPos))
{
    return m_uxCurPos + compute_shift_to_avoid_overlap_with_previous();
}

//---------------------------------------------------------------------------------------
void LineSpacer::process_barline_at_current_timepos(LUnits uxPos)
{
    //Current position is a barline in this line. Set its position.

    //update current pos with new xPos required for column alignment
    m_uxRemovable += uxPos - m_uxCurPos;
    m_uxCurPos = uxPos;

    //proceed to process this barline
    LUnits uxRequiredPos = m_uxCurPos;  // + compute_shift_to_avoid_overlap_with_previous();

    drag_any_previous_clef_to_place_it_near_this_one();

    //AssignPositionToCurrentEntry();
    LineEntry* pEntry = *m_itCur;
    LUnits xPos = uxRequiredPos + pEntry->get_anchor();
    pEntry->set_position(xPos);

    assign_fixed_and_variable_space(pEntry);

    m_uxCurPos = pEntry->get_x_final();
    m_uxRemovable = pEntry->get_variable_space();

    //AdvanceToNextEntry();
    m_itCur++;
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


    //proceed to process this timepos
    LUnits uxRequiredPos = m_uxCurPos;  // + compute_shift_to_avoid_overlap_with_previous();

    drag_any_previous_clef_to_place_it_near_this_one();
    LUnits uxNextPos = uxRequiredPos;
    LUnits uxMinNextPos = 0.0f;
    LUnits uxMargin = 0.0f;
	while (are_there_timed_objs())
    {
        //AssignPositionToCurrentEntry();
		(*m_itCur)->set_position( uxRequiredPos + (*m_itCur)->get_anchor() );

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
        m_itCur++;
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
bool LineSpacer::is_middle_barline()
{
    return (m_itCur != m_pLine->end())
            && !(*m_itCur)->is_barline_entry()
            && (*m_itCur)->get_staffobj()->is_barline();
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
    const LineEntryIterator itEnd = m_pLine->end();
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
TimeUnits LineSpacer::get_next_available_time()
{
	LineEntryIterator it = m_itCur;
    if (it != m_pLine->end())
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
    if (m_itNonTimedAtCurPos != m_pLine->end() && m_uxCurPos > m_uxNotTimedFinalPos)
    {
        shift_non_timed(m_uxCurPos - m_uxNotTimedFinalPos);
    }
    m_itNonTimedAtCurPos = m_pLine->end();     //no longer needed. Discart value now to avoid problmes at next timepos
}

//---------------------------------------------------------------------------------------
void LineSpacer::assign_fixed_and_variable_space(LineEntry* pEntry)
{
	//assign fixed and variable after spaces to this entry and compute the xFinal pos

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
            int iInstr = m_pLine->get_instrument();
            int iStaff = pSO->get_staff();
//			    LUnits fixed = m_pMeter->tenths_to_logical(LOMSE_EXCEPTIONAL_MIN_SPACE,
//                                                            iInstr, iStaff);
            LUnits fixed = m_pMeter->tenths_to_logical(LOMSE_SPACE_AFTER_SMALL_CLEF,
                                                       iInstr, iStaff);
            pEntry->set_fixed_space(fixed);
            pEntry->set_variable_space(
                m_pMeter->tenths_to_logical(LOMSE_MIN_SPACE, iInstr, iStaff) - fixed );
        }
        else if (pSO->is_spacer())    //TODO || pSO->is_score_anchor())
        {
            pEntry->set_fixed_space(0.0f);
            pEntry->set_variable_space( pEntry->get_shape_size() );
        }
        else if (pSO->is_barline())
        {
            LUnits space = 0.0f;
            if (!pEntry->is_barline_entry())
            {
                int iInstr = m_pLine->get_instrument();
                space = m_pMeter->tenths_to_logical(LOMSE_SPACE_AFTER_BARLINE / 2.0f,
                                                    iInstr, 0);
            }
            pEntry->set_fixed_space(space);
            pEntry->set_variable_space(space);
        }
        else
            assign_no_space(pEntry);
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
    if (m_pMeter->is_proportional_spacing())
        return compute_ideal_distance_proportional(pEntry);
    else
        return compute_ideal_distance_fixed(pEntry);
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::compute_ideal_distance_fixed(LineEntry* pEntry)
{
    int iInstr = m_pLine->get_instrument();
    ImoStaffObj* pSO = pEntry->get_staffobj();
	int iStaff = pSO->get_staff();
    return m_pMeter->tenths_to_logical(m_pMeter->get_spacing_value(), iInstr, iStaff);
}

//---------------------------------------------------------------------------------------
LUnits LineSpacer::compute_ideal_distance_proportional(LineEntry* pEntry)
{
	static const float rLog2 = 0.3010299956640f;		// log(2)
    ImoStaffObj* pSO = pEntry->get_staffobj();
	int iStaff = pSO->get_staff();
    int iInstr = m_pLine->get_instrument();

	//spacing function:   Space(Di) = Smin*[1 + A*log2(Di/Dmin)]
	LUnits uSmin = m_pMeter->tenths_to_logical(LOMSE_MIN_SPACE, iInstr, iStaff);
    ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
    float rVar = log(float(pNR->get_duration()) / LOMSE_DMIN) / rLog2;     //log2(Di/Dmin)
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
    int iInstr = m_pLine->get_instrument();
    ImoStaffObj* pSO = pEntry->get_staffobj();
	int iStaff = pSO->get_staff();
    pEntry->set_fixed_space( m_pMeter->tenths_to_logical(LOMSE_EXCEPTIONAL_MIN_SPACE,
                                                         iInstr, iStaff) );
}


//=======================================================================================
//ColumnResizer: encapsulates the methods to recompute shapes positions so that the
//column will have the desired width, and to move the shapes to those positions
//=======================================================================================
ColumnResizer::ColumnResizer(ColumnStorage* pColStorage, LUnits uNewWidth)
    : m_pColStorage(pColStorage)
    , m_uNewWidth(uNewWidth)
    , m_fHasShapes(false)
    , m_yMin(10000000.0f)
    , m_yMax(-10000000.0f)
{
}

//-------------------------------------------------------------------------------------
LUnits ColumnResizer::reposition_shapes(LUnits uNewStart, LUnits uNewWidth, UPoint org)
{
    m_sliceOrg = org;
    m_uNewStart = uNewStart + org.x;
    m_uOldWidth = m_pColStorage->get_trimmed_width();

    create_line_resizers();
    move_prolog_shapes_and_get_initial_time();
    determine_fixed_size_at_start_of_column();
    reposition_all_other_shapes();
    determine_vertical_limits();
    delete_line_resizers();

    return uNewStart + uNewWidth;
}

//---------------------------------------------------------------------------------------
void ColumnResizer::create_line_resizers()
{
	for (LinesIterator it=m_pColStorage->begin(); it != m_pColStorage->end(); ++it)
	{
        LineResizer* pResizer = LOMSE_NEW LineResizer(*it, m_uOldWidth, m_uNewWidth,
                                                m_uNewStart, m_sliceOrg);
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
void ColumnResizer::determine_vertical_limits()
{
    std::vector<LineResizer*>::iterator itR;
	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
	{
        if ((*itR)->has_shapes())
        {
            m_yMax = max(m_yMax, (*itR)->get_y_max());
            m_yMin = min(m_yMin, (*itR)->get_y_min());
            m_fHasShapes = true;
        }
    }
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
// TimeGridTableBuilder implementation
//=======================================================================================
TimeGridTableBuilder::TimeGridTableBuilder(ColumnStorage* pColStorage)
    : m_pColStorage(pColStorage)
{
}

//---------------------------------------------------------------------------------------
TimeGridTableBuilder::~TimeGridTableBuilder()
{
    m_PosTimes.clear();
}

//---------------------------------------------------------------------------------------
TimeGridTable* TimeGridTableBuilder::build_table()
{
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

    TimeGridTable* table = LOMSE_NEW TimeGridTable();
    table->add_entries(m_PosTimes);
    return table;
}

//---------------------------------------------------------------------------------------
bool TimeGridTableBuilder::there_are_objects()
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
void TimeGridTableBuilder::create_table_entry()
{
    TimeGridTableEntry tPosTime = {m_rCurrentTime, m_rMinDuration, m_uCurPos };
    m_PosTimes.push_back(tPosTime);
}

//---------------------------------------------------------------------------------------
void TimeGridTableBuilder::delete_line_explorers()
{
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
        delete *it;
    m_LineExplorers.clear();
}

//---------------------------------------------------------------------------------------
void TimeGridTableBuilder::create_line_explorers()
{
    const LinesIterator itEnd = m_pColStorage->end();
    for (LinesIterator it=m_pColStorage->begin(); it != itEnd; ++it)
	{
        TimeGridLineExplorer* pLinExplorer = LOMSE_NEW TimeGridLineExplorer(*it);
        m_LineExplorers.push_back(pLinExplorer);
    }
}

//---------------------------------------------------------------------------------------
void TimeGridTableBuilder::skip_non_timed_at_current_timepos()
{
    m_fTimedObjectsFound = false;
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
	{
        m_fTimedObjectsFound |= (*it)->skip_non_timed_at_current_timepos();
    }
}

//---------------------------------------------------------------------------------------
void TimeGridTableBuilder::find_shortest_noterest_at_current_timepos()
{
    get_current_time();
    m_rMinDuration = LOMSE_NO_DURATION;
    m_uCurPos = LOMSE_NO_POSITION;
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
	{
        if (is_equal_time(m_rCurrentTime, (*it)->get_current_time()))
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
void TimeGridTableBuilder::get_current_time()
{
    m_rCurrentTime = LOMSE_NO_TIME;
    std::vector<TimeGridLineExplorer*>::iterator it;
    for (it = m_LineExplorers.begin(); it != m_LineExplorers.end(); ++it)
	{
        m_rCurrentTime = min(m_rCurrentTime, (*it)->get_current_time());
    }
}

//---------------------------------------------------------------------------------------
void TimeGridTableBuilder::interpolate_missing_times()
{
    TimeInserter oInserter(m_PosTimes);
    oInserter.interpolate_missing_times();
}



//=======================================================================================
//TimeInserter
// helper class to interpolate missing entries
//=======================================================================================
TimeInserter::TimeInserter(std::vector<TimeGridTableEntry>& oPosTimes)
    : m_PosTimes(oPosTimes)
{
}

//---------------------------------------------------------------------------------------
void TimeInserter::interpolate_missing_times()
{
    for (int i=0; i < (int)m_PosTimes.size(); ++i)
    {
        TimeUnits rNextTime = m_PosTimes[i].rTimepos + m_PosTimes[i].rDuration;
        if (!is_time_in_table(rNextTime))
        {
            find_insertion_point(rNextTime);
            insert_time_interpolating_position(rNextTime);
        }
    }
}

//---------------------------------------------------------------------------------------
bool TimeInserter::is_time_in_table(TimeUnits rTimepos)
{
    if (m_PosTimes.size() == 0)
        return false;

    std::vector<TimeGridTableEntry>::iterator it;
    for (it=m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
    {
        if (is_equal_time(rTimepos, (*it).rTimepos))
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void TimeInserter::find_insertion_point(TimeUnits rTimepos)
{
    m_uPositionBeforeInsertionPoint = m_PosTimes.front().uxPos;
    m_rTimeBeforeInsertionPoint = m_PosTimes.front().rTimepos;

    std::vector<TimeGridTableEntry>::iterator it;
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
void TimeInserter::insert_time_interpolating_position(TimeUnits rTimepos)
{
    TimeGridTableEntry oItem;
    oItem.rTimepos = rTimepos;
    oItem.rDuration = 0.0;
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
        TimeUnits rTimeGap = (*m_itInsertionPoint).rTimepos - m_rTimeBeforeInsertionPoint;
        LUnits rPosGap = (*m_itInsertionPoint).uxPos - m_uPositionBeforeInsertionPoint;
        TimeUnits rTimeIncrement = rTimepos - m_rTimeBeforeInsertionPoint;
        oItem.uxPos += float(rTimeIncrement * (rPosGap / rTimeGap));
        m_PosTimes.insert(m_itInsertionPoint, oItem);
    }
}


//=======================================================================================
//TimeGridLineExplorer:
//  line traversal algorithm for creating the time-pos table
//=======================================================================================
TimeGridLineExplorer::TimeGridLineExplorer(MusicLine* pLineTable)
    : m_pLine(pLineTable)
{
    m_itCur = m_pLine->begin();
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

	    while (current_object_is_timed()
               && is_equal_time((*m_itCur)->get_timepos(),  m_rCurTime))
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
TimeUnits TimeGridLineExplorer::get_current_time()
{
    if (current_object_is_timed())
        return (*m_itCur)->get_timepos();
    else
        return LOMSE_NO_TIME;
}

//---------------------------------------------------------------------------------------
TimeUnits TimeGridLineExplorer::get_duration_for_found_entry()
{
    return m_rMinDuration;
}

//---------------------------------------------------------------------------------------
LUnits TimeGridLineExplorer::get_position_for_found_entry()
{
    return m_uCurPos + m_uShiftToNoteRestCenter;
}


}  //namespace lomse

