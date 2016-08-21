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

#include "lomse_spacing_algorithm_gourlay.h"

#include "lomse_score_iterator.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_box_slice_instr.h"
#include "lomse_box_slice.h"
#include "lomse_score_layouter.h"


namespace lomse
{

#define LOMSE_NO_DURATION   100000000000000.0f  //any too high value for a note duration
#define LOMSE_NO_TIME       100000000000000.0   //any impossible high value fir a timepos
#define LOMSE_NO_POSITION   100000000000000.0f  //any impossible high value
#define LOMSE_MAX_FORCE     100000000000000.0f  //any impossible high value

//=====================================================================================
//SpAlgGourlay implementation
//=====================================================================================
SpAlgGourlay::SpAlgGourlay(LibraryScope& libraryScope,
                                       ScoreMeter* pScoreMeter,
                                       ScoreLayouter* pScoreLyt, ImoScore* pScore,
                                       ShapesStorage& shapesStorage,
                                       ShapesCreator* pShapesCreator,
                                       PartsEngraver* pPartsEngraver)
    : SpAlgColumn(libraryScope, pScoreMeter, pScoreLyt, pScore, shapesStorage,
                  pShapesCreator, pPartsEngraver)
    , m_libraryScope(libraryScope)
    , m_pScoreMeter(pScoreMeter)
    , m_pScoreLyt(pScoreLyt)
    , m_pScore(pScore)
    , m_shapesStorage(shapesStorage)
    , m_pShapesCreator(pShapesCreator)
    , m_pPartsEngraver(pPartsEngraver)
    //
    , m_pCurSlice(NULL)
    , m_pLastEntry(NULL)
    , m_prevType(TimeSlice::k_undefined)
    , m_prevTime(0.0f)
    , m_numEntries(0)
    , m_pCurColumn(NULL)
    , m_numSlices(0)
    , m_iPrevColumn(-1)
    //
    , m_maxNoteDur(0.0f)
    , m_minNoteDur(LOMSE_NO_DURATION)
{
//    m_columns.reserve(pScoreLyt->get_num_columns());
    m_data.reserve(pScore->get_staffobjs_table()->num_entries());
}

//---------------------------------------------------------------------------------------
SpAlgGourlay::~SpAlgGourlay()
{
//    if (int(m_columns.size()) != m_pScoreLyt->get_num_columns())
//        dbgLogger << "ERROR. In SpAlgGourlay: more columns than reserved space. "
//                << "Reserved= " << m_pScoreLyt->get_num_columns()
//                << ", current= " << m_columns.size() << endl;

    if (int(m_data.size()) != m_pScore->get_staffobjs_table()->num_entries())
        dbgLogger << "ERROR. In SpAlgGourlay: more data than reserved space. "
                << "Reserved= " << m_pScore->get_staffobjs_table()->num_entries()
                << ", current= " << m_data.size() << endl;

    vector<StaffObjData*>::iterator itD;
    for (itD = m_data.begin(); itD != m_data.end(); ++itD)
        delete *itD;
    m_data.clear();

    vector<ColumnDataGourlay*>::iterator itC;
    for (itC = m_columns.begin(); itC != m_columns.end(); ++itC)
        delete *itC;
    m_columns.clear();

    list<TimeSlice*>::iterator itS;
    for (itS = m_slices.begin(); itS != m_slices.end(); ++itS)
        delete *itS;
    m_slices.clear();
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::start_column_measurements(int UNUSED(iCol), LUnits UNUSED(uxStart),
                                             LUnits UNUSED(fixedSpace))
{
    m_numSlices = 0;
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::new_column(TimeSlice* pSlice)
{
    m_pCurColumn = LOMSE_NEW ColumnDataGourlay(pSlice);
    m_columns.push_back(m_pCurColumn);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::include_object(ColStaffObjsEntry* pCurEntry, int iCol, int UNUSED(iLine),
                                  int UNUSED(iInstr), ImoStaffObj* pSO, TimeUnits rTime,
                                  int UNUSED(nStaff), GmoShape* pShape, bool fInProlog)
{
    StaffObjData* pData = LOMSE_NEW StaffObjData();
    m_data.push_back(pData);

    pData->m_pShape = pShape;

    //determine slice type for the new object to include
    int curType = TimeSlice::k_undefined;
    if (fInProlog)
        curType = TimeSlice::k_prolog;
    else
    {
        switch(pSO->get_obj_type())
        {
            case k_imo_clef:
            case k_imo_key_signature:
            case k_imo_time_signature:
            case k_imo_spacer:
            case k_imo_go_back_fwd:
                curType = TimeSlice::k_non_timed;
                break;

            case k_imo_note:
            case k_imo_rest:
                curType = TimeSlice::k_noterest;
                break;

            case k_imo_barline:
                curType = TimeSlice::k_barline;
                break;

            default:
                LOMSE_LOG_ERROR("????????????????????????");
        }
    }

    //determine if a new slice must be created
    TimeUnits curTime = rTime;
    if (curType != m_prevType || !is_equal(m_prevTime, curTime) )
    {
        //terminate previous slice
        finish_slice(m_pLastEntry, m_numEntries);

        //and start a new slice
        new_slice(pCurEntry, curType, iCol, int(m_data.size())-1, fInProlog);

        //save data from object to include
        if (pSO->is_note_rest())
        {
            m_maxNoteDur = pSO->get_duration();
            m_minNoteDur = m_maxNoteDur;
        }
        else
        {
            m_maxNoteDur = 0.0f;
            m_minNoteDur = LOMSE_NO_DURATION;
        }

        //create new column if necessary
        if (m_iPrevColumn != iCol)
        {
            new_column(m_pCurSlice);
            m_numSlices = 0;
            m_iPrevColumn = iCol;
        }

        ++m_numSlices;

        m_prevType = curType;
        m_prevTime = curTime;
    }
    else
    {
        if (pSO->is_note_rest())
        {
            m_maxNoteDur = max(m_maxNoteDur, pSO->get_duration());
            m_minNoteDur = min(m_minNoteDur, pSO->get_duration());
        }
    }

    m_pLastEntry = pCurEntry;
    ++m_numEntries;
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::new_slice(ColStaffObjsEntry* pEntry, int entryType, int iColumn,
                             int iData, bool fInProlog)
{
    //create the slice object
    TimeSlice* pSlice;
    switch(entryType)
    {
        case TimeSlice::k_prolog:
            pSlice = LOMSE_NEW TimeSliceProlog(pEntry, entryType, iColumn, iData);
            break;
        case TimeSlice::k_non_timed:
            pSlice = LOMSE_NEW TimeSliceNonTimed(pEntry, entryType, iColumn, iData);
            break;
        default:
            pSlice = LOMSE_NEW TimeSlice(pEntry, entryType, iColumn, iData, fInProlog);
    }

    //link slices, creating a list
    if (m_pCurSlice)
    {
        m_pCurSlice->set_next(pSlice);
        pSlice->m_prev = m_pCurSlice;
    }
    m_slices.push_back(pSlice);

    //update variables
    m_pCurSlice = pSlice;
    m_numEntries = 0;
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::finish_slice(ColStaffObjsEntry* pLastEntry, int numEntries)
{
    if (m_pCurSlice)
    {
        m_pCurSlice->set_final_data(pLastEntry, numEntries, m_maxNoteDur, m_minNoteDur);
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::finish_column_measurements(int UNUSED(iCol), LUnits UNUSED(xStart))
{
    //terminate last slice
    finish_slice(m_pLastEntry, m_numEntries);

    if (m_pCurColumn)
        m_pCurColumn->set_num_entries(m_numSlices);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::do_spacing(int iCol, bool fTrace, int level)
{
    compute_springs();
    order_slices_in_columns();
    m_columns[iCol]->determine_minimum_width();

    if (fTrace || m_libraryScope.dump_column_tables())
    {
        dbgLogger << endl << to_simple_string( microsec_clock::local_time() )
                  << " ******************* Before spacing" << endl;
        m_columns[iCol]->dump(dbgLogger);
    }

    //apply optimum force to get an initial estimation for columns width
    float Fopt = m_libraryScope.get_optimum_force();
    apply_force(Fopt);

    //determine column spacing function slope in the neighborhood of Fopt
    m_columns[iCol]->determine_approx_sff_for(Fopt);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::compute_springs()
{
    #define LOMSE_MIN_SPACE     10.0f   //Smin: space for Dmin
	LUnits uSmin = m_pScoreMeter->tenths_to_logical_max(LOMSE_MIN_SPACE);
    float alpha = m_libraryScope.get_spacing_alpha();

	static const float rLog2 = 0.3010299956640f;    //log(2)
    float dmin = m_libraryScope.get_spacing_dmin(); //LOMSE_DMIN
    float log2dmin = log(dmin) / rLog2;             //compute log2(dmin)

    list<TimeSlice*>::iterator it;
    for (it = m_slices.begin(); it != m_slices.end(); ++it)
        (*it)->assign_spacing_values(m_data, m_pScoreMeter);

    //AWARE: Can not be included in previous loop because slice i+1 also
    //       sets data for slice i
    LUnits dsFixed = m_pScoreMeter->tenths_to_logical_max(
                                m_pScoreMeter->get_spacing_value());
    bool fProportional = m_pScoreMeter->is_proportional_spacing();
    for (it = m_slices.begin(); it != m_slices.end(); ++it)
        (*it)->compute_spring_data(uSmin, alpha, log2dmin, dmin, fProportional, dsFixed);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::order_slices_in_columns()
{
    vector<ColumnDataGourlay*>::iterator it;
    for (it = m_columns.begin(); it != m_columns.end(); ++it)
        (*it)->order_slices();
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::apply_force(float F)
{
    //apply force to all columns
    vector<ColumnDataGourlay*>::iterator it;
    for (it = m_columns.begin(); it != m_columns.end(); ++it)
        (*it)->apply_force(F);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::assign_width_to_column(int iCol)
{
}


//---------------------------------------------------------------------------------------
LUnits SpAlgGourlay::aditional_space_before_adding_column(int iCol)
{
    return 0.0f;
}

//---------------------------------------------------------------------------------------
LUnits SpAlgGourlay::get_column_width(int iCol, bool fFirstColumnInSystem)
{
    //Returns the width for column iCol. Flag fFirstColumnInSystem could be
    //used in case the spacing algorithm would like to remove space from start
    //of column when the column is going to be placed as first column of a system

    return m_columns[iCol]->get_column_width();
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
                                                   LUnits yShift,
                                                   LUnits* yMin, LUnits* yMax)
{
    // A system is ready. It is formed by columns iFirstCol and iLastCol, both included.
    //
    // The spacing algorithm has already finished, and positions for these columns
    // are already computed.
    //
    // Also the justification process (adjust columns' width for justifying systems) is
    // already done.
    //
    // Now this method must:
    //
    // - move slice boxes to final columns' positions and set their widths to be equal
    //   to the width of the corresponding column. For each slice box its x position is
    //   that of the associated column. Its y position is valid and must not be changed.
    //
    // - move staff objs. shapes to their final positions. Each object x position is
    //   determined by the spacing algorithm and its y position is valid but
    //   must be shifted by the amount indicated by parameter yShift.
    //
    // - collect information about system vertical limits.


    GmoBoxSlice* pFirstSlice = get_slice_box(iFirstCol);
    LUnits xLeft = pFirstSlice->get_left();
    LUnits yTop = pFirstSlice->get_top();

    for (int iCol = iFirstCol; iCol < iLastCol; ++iCol)
    {
        //reposition boxes
        set_slice_final_position(iCol, xLeft, yTop);

        //reposition staffobjs
        m_columns[iCol]->move_shapes_to_final_positions(m_data, xLeft, yTop + yShift);

        //collect information about system vertical limits
        if (m_columns[iCol]->has_shapes())
        {
            *yMin = min(*yMin, m_columns[iCol]->get_y_min());
            *yMax = max(*yMax, m_columns[iCol]->get_y_max());
        }

        //assign the final width to the boxes
        LUnits colWidth = m_columns[iCol]->get_column_width();
        set_slice_width(iCol, colWidth);

        xLeft += colWidth;
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement)
{
#if 1
    //method 2: proportional division of the space and specific force for each column.
    //          Exact. No approximations used. It woks OK.

    vector<LUnits> colWidths;
    int numCols = iLastCol - iFirstCol;
    colWidths.reserve(numCols);

    LUnits stretchableWidth = 0.0f;
    int i=0;
    for (int iCol = iFirstCol; iCol < iLastCol; ++i, ++iCol)
    {
        colWidths[i] = m_columns[iCol]->get_column_width() - m_columns[iCol]->m_xFixed;
        stretchableWidth += colWidths[i];
    }

    i=0;
    for (int iCol = iFirstCol; iCol < iLastCol; ++i, ++iCol)
    {
        LUnits extraWidth = stretchableWidth == 0.0f ?
                uSpaceIncrement / (iLastCol - iFirstCol)
                : uSpaceIncrement * colWidths[i] / stretchableWidth;
        LUnits colWidth = m_columns[iCol]->get_column_width();
        float F = m_columns[iCol]->determine_force_for(colWidth + extraWidth);
        m_columns[iCol]->apply_force(F);

        dbgLogger << "Justifying system. Col " << iCol
                  << ": extraWidth= " << extraWidth
                  << ", stretchableWidth= " << stretchableWidth
                  << ", Force= " << F
                  << ", target width= " << (colWidth + extraWidth)
                  << ", achieved= " << m_columns[iCol]->get_column_width()
                  << endl;
    }

#else
    //Method 1: use approximate spacing function sff[cicj]
    //          Not good. Errors are noticeable when required force is > 20% of Fopt

    //determine required line width
    /*dbg*/ LUnits required = uSpaceIncrement;
    LUnits lineWidth = uSpaceIncrement;
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
        /*dbg*/ required  += m_columns[i]->get_column_width();
        lineWidth += m_columns[i]->get_column_width();
        lineWidth -= m_columns[i]->m_xFixed;
    }

    //determine composite spacing function sff[cicj]
    //                       j                          j
    //    sff[cicj] = 1 / ( SUM ( 1/Cappn ) )  = 1 / ( SUM ( slope.n ) )
    //                      n=i                        n=i
    float sum = 0.0f;
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
        sum += m_columns[i]->m_slope;
    }
    float c = 1.0f / sum;

    //determine force to apply to get desired extent
    float F = lineWidth * c;

    //apply this force to columns
    /*dbg*/ LUnits achieved = 0.0f;
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
        m_columns[i]->apply_force(F);
        /*dbg*/ achieved += m_columns[i]->get_column_width();
    }

    dbgLogger << "Justifing system: cols " << iFirstCol << ", " << iLastCol
              << ", line width " << lineWidth
              << ", c= " << c << ", new force: " << F
              << ", required width= " << required
              << ", achieved= " << achieved
              << endl;
#endif
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::is_empty_column(int iCol)
{
    return m_columns[iCol]->is_empty_column();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgGourlay::get_trimmed_width(int iCol)
{
    return m_columns[iCol]->get_column_width();
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::column_has_barline(int iCol)
{
    return true;    //TODO
}

//---------------------------------------------------------------------------------------
TimeGridTable* SpAlgGourlay::create_time_grid_table_for_column(int iCol)
{
    return NULL;    //TODO
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::dump_column_data(int iCol, ostream& outStream)
{
    m_columns[iCol]->dump(outStream);
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::column_has_visible_barline(int iCol)
{
    return true;    //TODO
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::add_shapes_to_box(int iCol, GmoBoxSliceInstr* pSliceInstrBox,
                                   int iInstr)
{
    m_columns[iCol]->add_shapes_to_box(pSliceInstrBox, iInstr, m_data);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::delete_shapes(int iCol)
{
    m_columns[iCol]->delete_shapes(m_data);
}

//---------------------------------------------------------------------------------------
float SpAlgGourlay::determine_penalty_for_line(int iSystem, int iFirstCol, int iLastCol)
{
    //penalty function PF(i,j) for line {ci, ..., cj}

    //The penalty function R(ci, cj) is calculated as follows:
    //
    //    R(ci, cj) = | sff[cicj](line_width) - fopt |
    //
    //where:
    //    sff[cicj]()  is just the merge of all sff’s from column ci up to column cj
    //    line_width   is the desired width of for the line
    //    fopt  is a constant value set by the user (and dependent on personal taste).

    //force break when so required
    if (m_pScoreLyt->column_has_system_break(iLastCol))
        return 0.0f;

    LUnits lineWidth = m_pScoreLyt->get_target_size_for_system(iSystem);
    if (iSystem > 0)
        lineWidth -= 1000.0f; //m_pScoreLyt->get_prolog_width_for_system(iSystem);

    //determine composite spacing function sff[cicj]
    //                       j                          j
    //    sff[cicj] = 1 / ( SUM ( 1/Cappn ) )  = 1 / ( SUM ( slope.n ) )
    //                      n=i                        n=i
    float sum = 0.0f;
    LUnits fixed = 0.0f;
    LUnits minWidth = 0.0f;
    for (int i = iFirstCol; i <= iLastCol; ++i)
    {
        sum += m_columns[i]->m_slope;
        fixed += m_columns[i]->m_xFixed;
        minWidth += m_columns[i]->get_minimum_width();
    }
    float c = 1.0f / sum;

    //if minimum width is greater than required width, it is impossible to achieve
    //the requiered width. Return a too high penalty
    if (minWidth > lineWidth)
        return 1000.0f;

    //determine force to apply to get desired extent
    float F = (lineWidth - fixed) * c;

//    dbgLogger << "Determine penalty: lineWidth= " << lineWidth
//              << ", Force= " << F << ", sum= " << sum << ", c= " << c << endl;

    //compute penalty:  R(ci, cj) = | sff[cicj](line_width) - fopt |
    float Fopt = m_libraryScope.get_optimum_force();
    float R = fabs(F - Fopt);

    return R;
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::is_better_option(float prevPenalty, float newPenalty,
                                    float nextPenalty, int i, int j)
{
    return (newPenalty + prevPenalty < nextPenalty);
}


//=======================================================================================
// TimeSlice implementation
//=======================================================================================
TimeSlice::TimeSlice(ColStaffObjsEntry* pEntry, int entryType, int column,
                                   int iData, bool fInProlog)
    : m_firstEntry(pEntry)
    , m_lastEntry(pEntry)
    , m_iFirstData(iData)
    , m_numEntries(1)
    , m_type(entryType)
    , m_iColumn(column)
    , m_fInProlog(fInProlog)
    , m_next(NULL)
    , m_prev(NULL)
    //
    , m_xLi(0.0f)
    , m_xRi(0.0f)
    , m_fi(0.0f)
    , m_ci(0.0f)
    , m_width(0.0f)
    , m_xLeft(0.0f)
    , m_ds(0.0f)
    , m_di(0.0f)
    , m_minNote(0.0f)
{
}

//---------------------------------------------------------------------------------------
TimeSlice::~TimeSlice()
{
}

//---------------------------------------------------------------------------------------
TimeUnits TimeSlice::get_timepos()
{
    return m_firstEntry->time();
}

//---------------------------------------------------------------------------------------
void TimeSlice::set_final_data(ColStaffObjsEntry* pLastEntry, int numEntries,
                                      TimeUnits maxNextTime, TimeUnits minNote)
{
    m_lastEntry = pLastEntry;
    m_numEntries = numEntries;
    m_ds = maxNextTime;
    m_minNote = minNote;
    m_width = get_xi() + m_xLeft;
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_spring_data(LUnits uSmin, float alpha, float log2dmin,
                                    TimeUnits dmin, bool fProportional, LUnits dsFixed)
{
    //compute spring duration ds
    TimeUnits nextTime = (m_next ? m_next->get_timepos() : m_ds + get_timepos());
    m_ds = nextTime - get_timepos();

    TimeUnits minNotePrev = (m_prev ? m_prev->get_min_note_still_sounding() : LOMSE_NO_DURATION);
    compute_smallest_duration_di(minNotePrev);

    find_smallest_note_soundig_at(nextTime);
    compute_spring_constant(uSmin, alpha, log2dmin, dmin, fProportional, dsFixed);
    compute_pre_stretching_force();
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_smallest_duration_di(TimeUnits minNotePrev)
{
    if (m_ds > 0.0f)
        m_di = min(minNotePrev, m_minNote);
    else
        m_di = m_ds;
}

//---------------------------------------------------------------------------------------
void TimeSlice::find_smallest_note_soundig_at(TimeUnits nextTime)
{
    //returns LOMSE_NO_DURATION if no note still sounding in next segment

    if (m_type == TimeSlice::k_noterest)
    {
        TimeUnits durLimit = nextTime - get_timepos();
        m_minNoteNext = LOMSE_NO_DURATION;      //too high value
        ColStaffObjsEntry* pEntry = m_firstEntry;
        for (int i=0; i < m_numEntries; ++i, pEntry = pEntry->get_next())
        {
            TimeUnits duration = pEntry->duration();
            if (duration > durLimit)
                m_minNoteNext = min(m_minNoteNext, duration);
        }
    }
    else if (m_prev)
        m_minNoteNext = m_prev->get_min_note_still_sounding();
    else
        m_minNoteNext = LOMSE_NO_DURATION;
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_spring_constant(LUnits uSmin, float alpha, float log2dmin,
                                        TimeUnits dmin, bool fProportional,
                                        LUnits dsFixed)
{
    if (m_ds > 0.0f)
    {
        LUnits space_ds;
        if (fProportional)
            space_ds = spacing_function(uSmin, alpha, log2dmin, dmin);
        else
            space_ds = dsFixed;
        m_ci = (m_di/m_ds) * ( 1.0f / space_ds );
    }
    else
        m_ci = 0.0f;
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_pre_stretching_force()
{
    if (m_type == TimeSlice::k_noterest)
        m_fi = m_ci * get_xi();
    else
        m_fi = LOMSE_MAX_FORCE;
}

//---------------------------------------------------------------------------------------
void TimeSlice::apply_force(float F)
{
    if (F > m_fi)
        m_width = F / m_ci + m_xLeft;
    else
        m_width = get_xi() + m_xLeft;
}

//---------------------------------------------------------------------------------------
LUnits TimeSlice::spacing_function(LUnits uSmin, float alpha, float log2dmin,
                                          TimeUnits dmin)
{
	if (m_ds <= dmin)
        return uSmin;

    // This method computes:
    //    space(d) = G(d).space(dmin)       for d = m_ds
    // where
    //    G(d) = 1 + alpha.log2( d / dmin )
    //
    // space(dmin) is parameter uSmin
    // parameter log2dmin is log2(dmin)
    //
    // As log2(x) = log(x) / log(2), then log2(d/dmin) = log2(d) - log2(dmin).
    // As log2(dmin) is constant we replace a division by a substraction.
	// log2(d/dmin) is always > 0 as it has been checked that d > dmin.
	// log(2) = 0.3010299956640f

    return uSmin * (1.0f + alpha * (log(m_ds) / 0.3010299956640f - log2dmin));
}

//---------------------------------------------------------------------------------------
void TimeSlice::assign_spacing_values(vector<StaffObjData*>& data,
                                             ScoreMeter* pMeter)
{
	//assign fixed space at start of this slice and compute pre-stretching
	//extend (left and right rods)

    LUnits xPrev = 0.0f;
    m_xLi = 0.0f;
    m_xRi = 0.0f;

    LUnits k_EXCEPTIONAL_MIN_SPACE = pMeter->tenths_to_logical_max(LOMSE_EXCEPTIONAL_MIN_SPACE);


    //assign fixed space at start of this slice
    m_xLeft = 0.0f;
    switch (get_type())
    {
        case TimeSlice::k_barline:
            m_xLeft = k_EXCEPTIONAL_MIN_SPACE;
            break;

        case TimeSlice::k_noterest:
            m_xLeft = k_EXCEPTIONAL_MIN_SPACE;
            break;

        default:
            LOMSE_LOG_ERROR("????????????????????????");
    }

    //if this is the first slice (the first data element is element 0) add some space
    //at start (AWARE: for scores without prolog)
    if (m_iFirstData == 0)
        m_xLeft += pMeter->tenths_to_logical_max(LOMSE_SPACE_BEFORE_PROLOG);

    //if prev slice is a barline slice, add some extra space at start
    if (m_prev && m_prev->get_type() == TimeSlice::k_barline)
        m_xLeft += pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_BARLINE / 2.0f);

    //loop for computing rods
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i)
    {
        GmoShape* pShape = data[i]->get_shape();
        if (pShape)
        {
            LUnits xAnchor = pShape->get_anchor_offset();
            m_xLi = max(m_xLi, pShape->get_width() + xAnchor);
            if (xAnchor < 0.0f)
                xPrev = min(xPrev, xAnchor);
        }
    }

//    //add some extra after space if this is the last slice in the prolog
//    if (is_last_slice_in_prolog())
//        m_xRi += pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_PROLOG);

    //if previous slice is barline, accidentals and other space at start must not be
    //transferred to previous slice (as right rod space). Instead should be accounted
    //as fixed space at start of this slice
    if (!m_prev || m_prev->get_type() == TimeSlice::k_barline)
        m_xLeft -= xPrev;
    else
    {
        if (m_prev->get_type() == TimeSlice::k_non_timed)
        {
            TimeSlice* prev = m_prev->m_prev;
            //m_prev->m_prev must always exist. Error in code/logic if not!
            if (prev == NULL)
            {
                LOMSE_LOG_ERROR("Error in code/logic. No previous slice exists for non-timed slice!");
                m_xLeft -= xPrev;
            }
            else
                prev->m_xRi -= xPrev;
        }
        else
            m_prev->m_xRi -= xPrev;
    }
}

//---------------------------------------------------------------------------------------
bool TimeSlice::is_last_slice_in_prolog()
{
    return m_fInProlog && m_next && !(m_next->m_fInProlog);
}

//---------------------------------------------------------------------------------------
void TimeSlice::dump_header(ostream& ss)
{
    //       +   +........+........+........+........+........+........+........+........+........+........+...........+........+
    ss << "type    timepos  entries   xStart       xi      xLi      xRi       fi       ci       ds       di     minNote 1st data" << endl;
}
//---------------------------------------------------------------------------------------
void TimeSlice::dump(ostream& ss)
{
    ss << "  " << m_type << "   "
       << fixed << setprecision(2) << setfill(' ')
       << setw(9) << get_timepos()
       << setw(9) << m_numEntries
       << setw(9) << m_xLeft
       << setw(9) << get_xi()
       << setw(9) << m_xLi
       << setw(9) << m_xRi;

    if (m_fi == LOMSE_MAX_FORCE)
        ss << setw(9) << "infinite";
    else
        ss << setw(9) << m_fi * 1000;

    ss << setw(9) << m_ci * 1000
       << setw(9) << m_ds
       << setw(9) << m_di;

    if (m_minNote == LOMSE_NO_DURATION)
        ss << setw(12) << "no note";
    else
        ss << setw(12) << m_minNote;

    ss << setw(9) << m_iFirstData;

    ss << endl;
}

//---------------------------------------------------------------------------------------
void TimeSlice::add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                                         vector<StaffObjData*>& data)
{
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        StaffObjData* pData = data[i];
        if (pEntry->num_instrument() == iInstr)
        {
            GmoShape* pShape = pData->get_shape();
            if (pShape)
                pSliceInstrBox->add_shape(pShape, GmoShape::k_layer_notes);
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSlice::delete_shapes(vector<StaffObjData*>& data)
{
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i)
    {
        StaffObjData* pData = data[i];
        delete pData->get_shape();
    }
}

//---------------------------------------------------------------------------------------
void TimeSlice::move_shapes_to_final_positions(vector<StaffObjData*>& data,
                                                      LUnits xPos, LUnits yPos)
{
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i)
    {
        StaffObjData* pData = data[i];
        GmoShape* pShape = pData->get_shape();
        if (pShape)
        {
            //move shape
            LUnits xLeft = xPos + pShape->get_anchor_offset() + m_xLeft;
            pShape->set_origin_and_notify_observers(xLeft + pData->m_xUserShift,
                                                    yPos + pData->m_yUserShift);

//            //get shape limits
//            m_yMax = max(m_yMax, pShape->get_bottom());
//            m_yMin = min(m_yMin, pShape->get_top());
//            m_fHasShapes = true;
        }
    }
}


//=====================================================================================
//TimeSliceProlog implementation
// This slice contains only clef, key signature and time signature objects, for all
// intruments/staves at score prolog
//=====================================================================================
TimeSliceProlog::TimeSliceProlog(ColStaffObjsEntry* pEntry, int entryType, int column,
                                 int iData)
    : TimeSlice(pEntry, entryType, column, iData, true)
    , m_clefsWidth(0.0f)
    , m_timesWidth(0.0f)
    , m_keysWidth(0.0f)
    , m_spaceBefore(0.0f)
{

}

//---------------------------------------------------------------------------------------
TimeSliceProlog::~TimeSliceProlog()
{

}

//---------------------------------------------------------------------------------------
void TimeSliceProlog::assign_spacing_values(vector<StaffObjData*>& data,
                                            ScoreMeter* pMeter)
{
	//compute width for this slice

    //initialize widths
    m_spaceBefore =  pMeter->tenths_to_logical_max(LOMSE_SPACE_BEFORE_PROLOG);
    m_clefsWidth = 0.0f;
    m_timesWidth = 0.0f;
    m_keysWidth = 0.0f;

    //assign fixed space at start of this slice and rods space
    m_xLeft = m_spaceBefore;
    m_xLi = 0.0f;
    m_xRi = 0.0f;

    //loop for computing space for objects in each staff
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = data[i]->get_shape();
        if (pShape)
        {
            LUnits width = pShape->get_width()
                           + pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);

            if (pEntry->imo_object()->is_clef())
                m_clefsWidth = max(m_clefsWidth, width);
            else if (pEntry->imo_object()->is_time_signature())
                m_timesWidth = max(m_timesWidth, width);
            else
                m_keysWidth = max(m_keysWidth, width);
        }
    }

    m_xLeft += m_clefsWidth + m_keysWidth + m_timesWidth;
    m_xLeft += pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_PROLOG);
}

//---------------------------------------------------------------------------------------
void TimeSliceProlog::move_shapes_to_final_positions(vector<StaffObjData*>& data,
                                                     LUnits xPos, LUnits yPos)
{
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        StaffObjData* pData = data[i];
        GmoShape* pShape = pData->get_shape();
        if (pShape)
        {
            //compute left position for this object
            LUnits xLeft = xPos + m_spaceBefore;
            if (pEntry->imo_object()->is_clef())
                ;
            else if (pEntry->imo_object()->is_time_signature())
                xLeft += m_clefsWidth + m_keysWidth;
            else
                xLeft += m_clefsWidth;

            //move shape
            pShape->set_origin_and_notify_observers(xLeft + pData->m_xUserShift,
                                                    yPos + pData->m_yUserShift);

//            //get shape limits
//            m_yMax = max(m_yMax, pShape->get_bottom());
//            m_yMin = min(m_yMin, pShape->get_top());
//            m_fHasShapes = true;
        }
    }
}


//=====================================================================================
//TimeSliceNonTimed implementation
//- This slice contains one or more non-timed objects (clef, key signature,
//  time signature, spacers, other) at the same timepos, but not in prolog.
//- This slice represents right rod width for previous slice ==>
//   - this slice is fixed width slice, with a total width = 0
//   - real width of this slice must be transferred to previous slice as m_xRi rod
//- Objects are organized by lines
//=====================================================================================
TimeSliceNonTimed::TimeSliceNonTimed(ColStaffObjsEntry* pEntry, int entryType,
                                     int column, int iData)
    : TimeSlice(pEntry, entryType, column, iData, true)
{

}

//---------------------------------------------------------------------------------------
TimeSliceNonTimed::~TimeSliceNonTimed()
{

}

//---------------------------------------------------------------------------------------
void TimeSliceNonTimed::assign_spacing_values(vector<StaffObjData*>& data,
                                              ScoreMeter* pMeter)
{
	//compute width for this slice. As objects in this slice can be for different
	//instruments/staves we maintain a width value for each staff and transfer to
	//previous slice the maximum of them.

    m_xLeft = 0.0f;
    m_xLi = 0.0f;
    m_xRi = 0.0f;
    m_interSpace = pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);

    //vector for widths
    vector<LUnits> widths;
    int numStaves = pMeter->num_staves();
    m_numStaves = numStaves;
    widths.reserve(numStaves);
    widths.assign(numStaves, 0.0f);

    //loop for computing widths
    LUnits maxWidth = 0.0f;
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = data[i]->get_shape();
        if (pShape)
        {
            int iStaff = pEntry->staff();
            widths[iStaff] += m_interSpace + pShape->get_width();
            maxWidth = max(maxWidth, widths[iStaff]);
        }
    }
    maxWidth += m_interSpace;   //add space before next slice symbol

    //if previous slice is barline, space must not be
    //transferred to previous slice (as right rod space). Instead should be accounted
    //as fixed space at start of this slice
    if (!m_prev || m_prev->get_type() == TimeSlice::k_barline)
    {
        m_realWidth = 0.0f;
        m_xLeft = maxWidth;
    }
    else
    {
        m_realWidth = maxWidth;

        //Improvement: test 136
        //Space for these non-timed should not be added as a full rod
        //in previous slice when there are no objects for the staves in which these
        //non-timed are placed. Instead, transfer only the minimum required for
        //ensuring that previous xRi rod is at least equal to the required space
        //for this slice.
        bool fNoOverlap = true;
        ColStaffObjsEntry* pEntry = m_prev->m_firstEntry;
        int iMax = m_prev->m_numEntries;
        for (int i=0; i < iMax; ++i, pEntry = pEntry->get_next())
        {
            int iStaff = pEntry->staff();
            fNoOverlap &= widths[iStaff] == 0.0f;
        }

        if (fNoOverlap)
            m_prev->set_minimum_xi(maxWidth - m_interSpace);
        else
            m_prev->increment_xRi(maxWidth);
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNonTimed::move_shapes_to_final_positions(vector<StaffObjData*>& data,
                                                       LUnits xPos, LUnits yPos)
{
    //vector for positions on each staff
    vector<LUnits> positions;
    positions.reserve(m_numStaves);
    positions.assign(m_numStaves, 0.0f);

    //loop for positioning shapes
    LUnits xStart = xPos - m_realWidth + m_interSpace;
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        StaffObjData* pData = data[i];
        GmoShape* pShape = pData->get_shape();
        if (pShape)
        {
            //compute left position for this object
            int iStaff = pEntry->staff();
            LUnits xLeft = xStart + positions[iStaff];

            //move shape
            pShape->set_origin_and_notify_observers(xLeft + pData->m_xUserShift,
                                                    yPos + pData->m_yUserShift);

            positions[iStaff] += pShape->get_width() + m_interSpace;

//            //get shape limits
//            m_yMax = max(m_yMax, pShape->get_bottom());
//            m_yMin = min(m_yMin, pShape->get_top());
//            m_fHasShapes = true;
        }
    }
}



//=====================================================================================
//ColumnDataGourlay implementation
//=====================================================================================
ColumnDataGourlay::ColumnDataGourlay(TimeSlice* pSlice)
    : m_pFirstSlice(pSlice)
    , m_slope(1.0f)
    , m_xFixed(0.0f)
    , m_colWidth(0.0f)
{
}

//---------------------------------------------------------------------------------------
ColumnDataGourlay::~ColumnDataGourlay()
{
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::dump(ostream& outStream, bool fOrdered)
{
    outStream << "---------------------------------------------------------------------"
              << endl;

    int numSlices = num_slices();
    outStream << "Column " << m_pFirstSlice->m_iColumn
              << ": Num.slices= " << numSlices
              << ", slope= " << m_slope
              << ", min.width= " << m_colMinWidth
              << endl;

    if (numSlices > 0)
    {
        TimeSlice::dump_header(outStream);
        if (fOrdered)
        {
            vector<TimeSlice*>::iterator it;
            for (it = m_orderedSlices.begin(); it != m_orderedSlices.end(); ++it)
                (*it)->dump(outStream);
        }
        else
        {
            TimeSlice* pSlice = m_pFirstSlice;
            for (int i=0; i < numSlices; ++i)
            {
                pSlice->dump(outStream);
                pSlice = pSlice->next();
            }
        }
        outStream << endl;
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::set_num_entries(int numSlices)
{
    m_orderedSlices.reserve(numSlices);
    m_orderedSlices.assign(numSlices, NULL);
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::order_slices()
{
    //load vector
    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        m_orderedSlices[i] = pSlice;
        pSlice = pSlice->next();
    }

    //sort vector by pre-stretching force, so that fi <= fi+1
    //uses bubble algorithm

    int j, k;
    int numSlices = num_slices();
    bool fChanges;
    for (int i = 0; i < numSlices; ++i)
    {
        fChanges = false;
        j = numSlices - 1;

        while ( j != i )
        {
            k = j - 1;
            if (m_orderedSlices[j]->m_fi < m_orderedSlices[k]->m_fi)
            {
                //interchange elements
                pSlice = m_orderedSlices[j];
                m_orderedSlices[j] = m_orderedSlices[k];
                m_orderedSlices[k] = pSlice;
                fChanges = true;
            }
            j = k;
        }

        //If there were no changes in this loop step this implies that the vector is ordered;
        //in this case exit loop to save time
        if (!fChanges) break;
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::determine_minimum_width()
{
    //Computes the minimum column extent as the sum of all slices fixed space and their
    //pre-stretching extent

    m_colMinWidth = 0.0f;
    vector<TimeSlice*>::iterator it;
    for (it = m_orderedSlices.begin(); it != m_orderedSlices.end(); ++it)
    {
        m_colMinWidth += (*it)->get_minimum_extent();
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                                          vector<StaffObjData*>& data)
{
    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        pSlice->add_shapes_to_box(pSliceInstrBox, iInstr, data);
        pSlice = pSlice->next();
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::delete_shapes(vector<StaffObjData*>& data)
{
    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        pSlice->delete_shapes(data);
        pSlice = pSlice->next();
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::move_shapes_to_final_positions(vector<StaffObjData*>& data,
                                                       LUnits xPos, LUnits yPos)
{
    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        pSlice->move_shapes_to_final_positions(data, xPos, yPos);
        xPos += pSlice->get_width();
        pSlice = pSlice->next();
    }
}

//---------------------------------------------------------------------------------------
bool ColumnDataGourlay::is_empty_column()
{
    return m_pFirstSlice == NULL;
}

//---------------------------------------------------------------------------------------
bool ColumnDataGourlay::has_shapes()
{
    return true;    //TODO
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::apply_force(float F)
{
    //modify slices by applying force F to them

    m_colWidth = 0.0f;
    vector<TimeSlice*>::iterator it;
    for (it = m_orderedSlices.begin(); it != m_orderedSlices.end(); ++it)
    {
        (*it)->apply_force(F);
        m_colWidth += (*it)->get_width();
    }
}

//---------------------------------------------------------------------------------------
LUnits ColumnDataGourlay::determine_extent_for(float F)
{
    //Alorithm C2: determine column extent for a given force.
    //             But force is not applied to slices, so that no data is changed
    //Param F   force to apply to the column

    LUnits extent = 0.0f;
    int i;

    //find first slice whose pre-stretching force is lower than F
    for (i = num_slices()-1; i >= 0; --i)
    {
        TimeSlice* pSlice = m_orderedSlices[i];
        if (F > pSlice->m_fi)
            break;      //the first slice that will react to F is found

        //pre-stretching force for current slice is greater than F. The slice extent
        //will be the pre-stretching extent xi
        extent += pSlice->get_xi() + pSlice->m_xLeft;
    }

    //if F is lower than all slices pre-stretching force, the column extent
    //is just the column pre-stretching extent
    if (i == 0)
        return extent;

    //slice i is the first on to react to force F. Compute the composite
    //spring from slices i to n
    float s = 0.0f;
    for (int j = i-1; j > 0; --j)
    {
        TimeSlice* pSlice = m_orderedSlices[j];
        s += 1.0f / pSlice->m_ci;
        extent += pSlice->m_xLeft;
    }
    //composite spring constant is c = 1 / s

    //now apply the force F to this combined spring
    extent += F / s;

    return extent;
}

//---------------------------------------------------------------------------------------
float ColumnDataGourlay::determine_force_for(LUnits x)
{
    // OK
    //-----------------------------------------------------------------------------------
    //function sff(x) : Determining the required force for a given extent x
    //-----------------------------------------------------------------------------------

    //Pre-calculate the minimum extent
    LUnits xmin = 0.0f;
    LUnits xFixed = 0.0f;
    vector<TimeSlice*>::iterator it;
    for (it = m_orderedSlices.begin(); it != m_orderedSlices.end(); ++it)
    {
        xmin += (*it)->get_xi();
        xFixed += (*it)->m_xLeft;
    }

    //if the required extent is lower than the minimum extent the required force is 0
    x -= xFixed;
    if (x <= xmin)
        return 0.0f;

    //compute combined spring constant for all springs that will react to F
    it = m_orderedSlices.begin();
    TimeSlice* pLast = m_orderedSlices.back();
    float F;
    float slope = 1.0f / (*it)->m_ci;
    while (true)
    {
        //calculate force required by current spring
        xmin -= (*it)->get_xi();
        F = (x - xmin) / slope;

        //if we are at the very last spring return the computed force
        if (*it == pLast)
            return F;

        ++it;
        //the next spring is bigger than current force
        //return the computed force
        if (F <= (*it)->m_fi)
            return F;

        //Adjust the combined spring constant
        slope += 1.0f / (*it)->m_ci;
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::determine_approx_sff_for(float F)
{
    //compute combined spring constant for all springs that will react to F

#if 0
    //method 2: direct computation of the slope
    //          produces more or less the same results than method 1 but requires
    //          more computations.

    // extent1 = apply optimum force fopt to sff  (inverse of Algorithm 1)
    // extent2 = apply optimum force fopt * 1.15 to sff  (inverse of Algorithm 1)    fopt + 15%
    // slope = (extent1 - extent2) / (0.15 * fopt)

    LUnits e1 = get_column_width();
    apply_force(F * 1.15f);
    LUnits e2 = get_column_width();
    m_slope = (e2 - e1) / (F * 0.15f);

    m_xFixed = 0.0f;
    vector<TimeSlice*>::iterator it;
    for (it = m_orderedSlices.begin(); it != m_orderedSlices.end(); ++it)
    {
        m_xFixed += (*it)->m_xLeft;

        //if the force of this spring is bigger than F do not take this spring into
        //account, only its pre-stretching extent
        if (F <= (*it)->m_fi)
            m_xFixed += (*it)->get_xi();
    }

#else
    //method 1: compute combined spring for active springs at force F

    m_slope = 0.0f;
    m_xFixed = 0.0f;
    vector<TimeSlice*>::iterator it;
    for (it = m_orderedSlices.begin(); it != m_orderedSlices.end(); ++it)
    {
        m_xFixed += (*it)->m_xLeft;

        //if the force of this spring is bigger than F do not take this spring into
        //account, only its pre-stretching extent
        if (F <= (*it)->m_fi)
            m_xFixed += (*it)->get_xi();
        else
        {
            //Add this spring to the combined spring
            //  c = 1 / ( (1/c) + (1/ci) ), but s = 1/c       ==>
            //  s = s + (1/ci)
            m_slope += 1.0f / (*it)->m_ci;
        }

//        dbgLogger << "    determining approx. sff for column. F=" << F
//                  << ", m_fi= " << (*it)->m_fi
//                  << ", m_ci=" << (*it)->m_ci
//                  << ", slope=" << m_slope << endl;
    }

    //if F is lower than minimum force for all springs, use the first one
    if (m_slope == 0.0f)
        m_slope = 1.0f / m_orderedSlices[0]->m_ci;
#endif
    dbgLogger << "Final slope=" << m_slope
              << ", xFixed=" << m_xFixed
              << endl;
}

//=====================================================================================
//StaffObjData implementation
//=====================================================================================
StaffObjData::StaffObjData()
    : m_pShape(NULL)
    , m_xUserShift(0.0f)
    , m_yUserShift(0.0f)
{
}

//---------------------------------------------------------------------------------------
StaffObjData::~StaffObjData()
{
}


}  //namespace lomse

