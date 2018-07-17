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

#include "lomse_spacing_algorithm_gourlay.h"

#include "lomse_score_iterator.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_box_slice_instr.h"
#include "lomse_box_slice.h"
#include "lomse_score_layouter.h"

#include "lomse_calligrapher.h"

#include <vector>
#include <cmath>   //abs
using namespace std;


namespace lomse
{

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
    , m_pCurSlice(nullptr)
    , m_pLastEntry(nullptr)
    , m_prevType(TimeSlice::k_undefined)
    , m_prevTime(0.0f)
    , m_numEntries(0)
    , m_pCurColumn(nullptr)
    , m_numSlices(0)
    , m_iPrevColumn(-1)
    //
    , m_lastPrologTime(-1.0f)
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

    if (int(m_data.size()) > m_pScore->get_staffobjs_table()->num_entries())
    {
        stringstream ss;
        ss << "Investigate: more data than reserved space. "
           << "Reserved= " << m_pScore->get_staffobjs_table()->num_entries()
           << ", current= " << m_data.size();
        LOMSE_LOG_ERROR(ss.str());
    }

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
void SpAlgGourlay::start_column_measurements(int UNUSED(iCol))
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
                                  int UNUSED(iInstr), ImoStaffObj* pSO,
                                  TimeUnits UNUSED(rTime),
                                  int UNUSED(nStaff), GmoShape* pShape, bool fInProlog)
{
    StaffObjData* pData = LOMSE_NEW StaffObjData();
    m_data.push_back(pData);

    pData->m_pShape = pShape;

    //determine slice type for the new object to include
//    dbgLogger << "include_object(). StaffObj type = " << pSO->get_name()
//              << ", id=" << pSO->get_id() << endl;
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
            case k_imo_direction:
            case k_imo_metronome_mark:
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
                stringstream ss;
                ss << "Investigate: un-expected staff object. Type= "
                   << pSO->get_obj_type();
                LOMSE_LOG_ERROR(ss.str());
                curType = TimeSlice::k_non_timed;
        }
    }

    //determine if a new slice should be created
    TimeUnits curTime = pCurEntry->time();
    bool fCreateNewSlice = false;
    if (curType != m_prevType || !is_equal_time(m_prevTime, curTime) )
        fCreateNewSlice = true;

    //avoid re-using current prolog slice if object is not clef, key or time
    //signature or already exists for this staff.
    if (!fCreateNewSlice && curType == TimeSlice::k_prolog && m_pCurSlice)
    {
        if (!accept_for_prolog_slice(pCurEntry))
        {
            fCreateNewSlice = true;
            curType = TimeSlice::k_non_timed;
        }
    }

    //avoid starting a new prolog slice when non-timed after prolog
    if (fCreateNewSlice && curType == TimeSlice::k_prolog)
    {
        if (is_equal_time(m_lastPrologTime, curTime) && m_prevType == TimeSlice::k_non_timed)
        {
            fCreateNewSlice = !m_pCurSlice;
            curType = TimeSlice::k_non_timed;
        }
    }
//    dbgLogger << ", slice type= " << curType;

    //include entry in current or new slice
    if (fCreateNewSlice)
    {
//        dbgLogger << ", Start new slice. m_prevType:" << m_prevType
//            << ", m_prevTime:" << m_prevTime << ", curTime:" << curTime
//            << endl;
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
//        dbgLogger << ", Use previous slice." << endl;
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
                             int iData, bool UNUSED(fInProlog))
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
            pSlice = LOMSE_NEW TimeSlice(pEntry, entryType, iColumn, iData);
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

    //if prolog slice, prepare for accounting included objects
    if (entryType == TimeSlice::k_prolog)
    {
        int numStaves = m_pScoreMeter->num_staves();
        m_prologClefs.assign(numStaves, false);
        m_prologKeys.assign(numStaves, false);
        m_prologTimes.assign(numStaves, false);

        m_lastPrologTime = pEntry->time();
        accept_for_prolog_slice(pEntry);
    }
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::accept_for_prolog_slice(ColStaffObjsEntry* pEntry)
{
    //returns true if the entry is acceptable for current prolog slice

    ImoStaffObj* pSO = pEntry->imo_object();
    int iStaff = m_pScoreMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
    if (pSO->is_clef())
    {
        if (m_prologClefs[iStaff])
            return false;
        m_prologClefs[iStaff] = true;
        return true;
    }
    else if (pSO->is_key_signature())
    {
        if (m_prologKeys[iStaff])
            return false;
        m_prologKeys[iStaff] = true;
        return true;
    }
    else if (pSO->is_time_signature())
    {
        if (m_prologTimes[iStaff])
            return false;
        m_prologTimes[iStaff] = true;
        return true;
    }
    else
        return false;
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
void SpAlgGourlay::finish_column_measurements(int UNUSED(iCol))
{
    //terminate last slice
    finish_slice(m_pLastEntry, m_numEntries);

    if (m_pCurColumn)
        m_pCurColumn->set_num_entries(m_numSlices);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::do_spacing(int iCol, bool fTrace)
{
    determine_spacing_parameters();
    compute_springs();
    order_slices_in_columns();

    int numInstruments = m_pScoreMeter->num_instruments();
    m_columns[iCol]->collect_barlines_information(numInstruments);
    m_columns[iCol]->determine_minimum_width();

    if (fTrace || m_libraryScope.dump_column_tables())
    {
        dbgLogger << endl << to_simple_string(chrono::system_clock::now(), true)
                  << " ******************* Before spacing" << endl;
        m_columns[iCol]->dump(dbgLogger);
    }

    //apply optimum force to get an initial estimation for columns width
    apply_force(m_Fopt);

    //determine column spacing function slope in the neighborhood of Fopt
    m_columns[iCol]->determine_approx_sff_for(m_Fopt);

    if (fTrace || m_libraryScope.dump_column_tables())
    {
        dbgLogger << "Column " << iCol << ". Slope for Fopt= " << m_Fopt
                  << " is slope= " << m_columns[iCol]->m_slope << endl;
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::determine_spacing_parameters()
{
    if (m_libraryScope.use_debug_values())
    {
        m_Fopt = m_libraryScope.get_optimum_force();
        m_alpha = m_libraryScope.get_spacing_alpha();
        m_dmin = m_libraryScope.get_spacing_dmin();
        m_uSmin = m_pScoreMeter->tenths_to_logical_max(
                            m_libraryScope.get_spacing_smin() );
    }
    else
    {
        m_Fopt = m_pScoreMeter->get_spacing_Fopt();
        m_alpha = m_pScoreMeter->get_spacing_alpha();
        if (m_pScoreMeter->get_render_spacing_opts() & k_render_opt_dmin_global)
            m_dmin = float(m_pScore->get_staffobjs_table()->min_note_duration());
        else //k_render_opt_dmin_fixed
            m_dmin = m_pScoreMeter->get_spacing_dmin();
        m_uSmin = m_pScoreMeter->get_spacing_smin();
    }

	static const float rLog2 = 0.3010299956640f;    //log(2)
    m_log2dmin = log(m_dmin) / rLog2;             //compute log2(dmin)



//    //choose Fopt as a function of Dmin
//    if (m_dmin <= 8.0f)
//        m_Fopt *= 0.8f;
//    else if (m_dmin <= 16.0f)
//        m_Fopt *= 1.0f;
//    else if (m_dmin <= 32.0f)
//        m_Fopt *= 2.0f;
//    else
//        m_Fopt *= 3.0f;
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::compute_springs()
{
    TextMeter textMeter(m_libraryScope);

    list<TimeSlice*>::iterator it;
    for (it = m_slices.begin(); it != m_slices.end(); ++it)
        (*it)->assign_spacing_values(m_data, m_pScoreMeter, textMeter);

    //AWARE: Can not be included in previous loop because slice i+1 also
    //       sets data for slice i
    LUnits dsFixed = m_pScoreMeter->tenths_to_logical_max(
                                m_pScoreMeter->get_spacing_value());
    bool fProportional = m_pScoreMeter->is_proportional_spacing();
    for (it = m_slices.begin(); it != m_slices.end(); ++it)
        (*it)->compute_spring_data(m_uSmin, m_alpha, m_log2dmin, m_dmin,
                                   fProportional, dsFixed);
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
        m_columns[iCol]->move_shapes_to_final_positions(m_data, xLeft, yTop + yShift,
                                                        yMin, yMax, m_pScoreMeter);

        //assign the final width to the boxes
        LUnits colWidth = m_columns[iCol]->get_column_width();
        set_slice_width(iCol, colWidth);

        xLeft += colWidth;
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement)
{
    //Use approximate spacing function sff[cicj] for computing the required force.
    //If required force is greater than 20% Fopt the errors are noticeable. In this
    //case, a loop of succesive approximations is started (usually a single
    //pass is enough).


    //determine required line width
    LUnits required = uSpaceIncrement;
    LUnits lineWidth = uSpaceIncrement;
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
        required  += m_columns[i]->get_column_width();
        lineWidth += m_columns[i]->get_column_width();
        lineWidth -= m_columns[i]->m_xFixed;
    }

    //determine composite spacing function sff[cicj]
    //                       j                          j
    //    sff[cicj] = 1 / ( SUM ( 1/Cappn ) )  = 1 / ( SUM ( slope.n ) )
    //                      n=i                        n=i
    float sum = 0.0f;
    float minF = m_Fopt;
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
        sum += m_columns[i]->m_slope;
        minF = max(minF, m_columns[i]->m_minFi);
    }
    float c = 1.0f / sum;

    //determine force to apply to get desired extent
    float F = lineWidth * c;

    //if columns must be stretched but F < FOpt implies that columns
    //at Fopt are not yet stretched. Something greater than minF is needed
    if (uSpaceIncrement > 0.0f && F < m_Fopt)
        F = max(2.0f * m_Fopt, 1.1f * minF);

    //apply this force to columns
    LUnits achieved = 0.0f;
    for (int i = iFirstCol; i < iLastCol; ++i)
    {
        m_columns[i]->apply_force(F);
        achieved += m_columns[i]->get_column_width();
    }

//    dbgLogger << "--------------------------------------------------------------" << endl
//              << "Justifying cols " << iFirstCol << ", " << iLastCol
//              << ", space incr= " << fixed << setprecision(2) << uSpaceIncrement
//              << ", line width= " << lineWidth
//              << ", c= " << fixed << setprecision(6) << c
//              << ", Fopt= " << m_Fopt
//              << ", alpha= " << m_alpha
//              << ", new force= " << F
//              << ", required width= " << setprecision(2) << required
//              << ", achieved= " << achieved
//              << ", Dmin= " << m_dmin
//              << endl;

    //Errors are noticeable when required force is > 20% of Fopt.
    //If error is greater than one tenth of a millimeter, a loop of
    //succesive approximations is started.

    LUnits uError = required - achieved;
    float Fprev = m_Fopt;
    int round=0;
    while (abs(uError) > 10.0f && round < 8)     //10.0f = one tenth of a millimeter
    {
//        dbgLogger << "Error = " << uError << ". Iter. "
//                  << round;
        ++round;

        //compute an increment for F
        float deltaF = F - Fprev;
        Fprev = F;
        LUnits deltaS = uSpaceIncrement - uError;
        uSpaceIncrement = uError;
        F += (uError/deltaS) * deltaF;

        //apply then new force to columns
        LUnits achieved = 0.0f;
        for (int i = iFirstCol; i < iLastCol; ++i)
        {
            m_columns[i]->apply_force(F);
            achieved += m_columns[i]->get_column_width();
        }

        uError = required - achieved;

//        dbgLogger << ". deltaF= "  << setprecision(6) << deltaF
//                  << ", deltaS= " << deltaS
//                  << ", new F= " << setprecision(6) << F
//                  << ", achieved= " << setprecision(2) << achieved
//                  << ", error= " << uError
//                  << endl;
    }
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::is_empty_column(int iCol)
{
    return m_columns[iCol]->is_empty_column();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgGourlay::get_column_width(int iCol)
{
    return m_columns[iCol]->get_column_width();
}

//---------------------------------------------------------------------------------------
int SpAlgGourlay::get_column_barlines_information(int iCol)
{
    return m_columns[iCol]->get_barlines_information();
}

//---------------------------------------------------------------------------------------
TimeGridTable* SpAlgGourlay::create_time_grid_table_for_column(int iCol)
{
    return m_columns[iCol]->create_time_grid_table();
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::dump_column_data(int iCol, ostream& outStream)
{
    m_columns[iCol]->dump(outStream);
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

    bool fTrace = (m_libraryScope.get_trace_level_for_lines_breaker()
                       & k_trace_breaks_penalties) != 0;

    //force break when so required
    if (m_pScoreLyt->column_has_system_break(iLastCol))
    {
        if (fTrace)
        {
            dbgLogger << "Determine penalty: column has break. Penalty= 0" << endl;
        }
        return 0.0f;
    }

//    //do not justify last system ==> no penalty for any combination for last system
//    if (iLastCol == m_pScoreLyt->get_num_columns() - 1)
//    {
//        if (fTrace)
//        {
//            dbgLogger << "Determine penalty: do not justify last system. Penalty= -1" << endl;
//        }
//        return -1.0f;
//    }

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
    if (minWidth > lineWidth && iFirstCol != iLastCol)
    {
        if (fTrace)
        {
            dbgLogger << "Determine penalty: minimum width is greater than "
                      << "required width. Penalty= 1000" << endl;
        }
        return 1000.0f;
    }

    //determine force to apply to get desired extent
    float F = (lineWidth - fixed) * c;

    if (fTrace)
    {
        dbgLogger << "Determine penalty: lineWidth= " << lineWidth
                  << ", Force= " << F << ", sum= " << sum << ", c= " << c
                  << ", iSystem" << iSystem
                  << endl;
    }

    //compute penalty:  R(ci, cj) = | sff[cicj](line_width) - fopt |
    float R = fabs(F - m_Fopt);

    //do not accept shrinking, if required
	if (m_pScoreMeter->get_render_spacing_opts() & k_render_opt_breaker_no_shrink)
    {
        if (F < m_Fopt)
            return 1000.0f;
    }

    //increase penalty if requiring high shrink
    if (F < 0.7f * m_Fopt)
    {
        //R *= 1.0f + (5.0f * (0.7f * m_Fopt - F));
        R *= 2.0f;
    }

    //increase penalty for columns not ended in barline
    if (!m_columns[iLastCol]->all_instr_have_barline())
    {
        R *= 4.0f;
        if (!m_columns[iLastCol]->some_instr_have_barline())
            R *= 4.0f;
    }
    //m_penalty = barlines == 0 ? 0.4f : (barlines < lines ? 0.6f : 1.0f);


    return R;
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::is_better_option(float prevPenalty, float newPenalty,
                                    float nextPenalty, int UNUSED(i), int UNUSED(j))
{
    return (newPenalty + prevPenalty < nextPenalty);
}


//=======================================================================================
// TimeSlice implementation
//=======================================================================================
TimeSlice::TimeSlice(ColStaffObjsEntry* pEntry, int entryType, int column, int iData)
    : m_firstEntry(pEntry)
    , m_lastEntry(pEntry)
    , m_iFirstData(iData)
    , m_numEntries(1)
    , m_type(entryType)
    , m_iColumn(column)
    , m_next(nullptr)
    , m_prev(nullptr)
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
    m_lyrics.clear();
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

    add_lyrics();
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
        TimeUnits durLimit = nextTime - get_timepos() + 0.1f * k_duration_256th;
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
        m_ci = float(m_di/m_ds) * ( 1.0f / space_ds );
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

    return uSmin * (1.0f + alpha * (log(float(m_ds)) / 0.3010299956640f - log2dmin));
}

//---------------------------------------------------------------------------------------
void TimeSlice::assign_spacing_values(vector<StaffObjData*>& data, ScoreMeter* pMeter,
                                      TextMeter& textMeter)
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
            xPrev -= pMeter->tenths_to_logical_max(LOMSE_MIN_SPACE_BEFORE_BARLINE);
            break;

        case TimeSlice::k_noterest:
            m_xLeft = k_EXCEPTIONAL_MIN_SPACE;
            break;

        default:
            stringstream ss;
            ss << "Investigate: un-expected TimeSlice type. Type= " << get_type();
            LOMSE_LOG_ERROR(ss.str());
    }

    //[SP001] if this is the first slice (scores without prolog) add some space at start
    //test 00046: some space between start of score and first note
    if (m_iFirstData == 0)
        m_xLeft += pMeter->tenths_to_logical_max(LOMSE_SPACE_BEFORE_PROLOG);

    //[SP002] if prev slice is a barline slice, add some extra space at start
    //test 00136: some space after barline and 1st note in 2nd measure
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

//    //for barlines, nothing else to do
//    if (get_type() == TimeSlice::k_barline)
//        return;

    //for notes/rests take lyrics into account
    vector<ImoLyric*>::iterator it;
    for (it=m_lyrics.begin(); it != m_lyrics.end(); ++it)
    {
        LUnits width = measure_lyric(*it, pMeter, textMeter) / 2.0f;
        m_xLi = max(m_xLi, width);
        xPrev = min(xPrev, -width);
        //TODO: the split must not include the hyphenation
    }


    //transfer space at start (accidentals and other space at start) to the previous
    //note (timed slice) as right rod space. This is necessary for timepos vertical
    //alignment. Otherwise, accidentals will shift the note to right and will not be
    //vertically aligned. There are some exceptions to this rule.


    //[SP041] test 01a
    //if previous slice is barline, space must not be transferred to it
    //because if it is transferred and justification decides to place a new system
    //after the barline, the barline will not be right justified (there will be some
    //space after it). Therefore, if previous slice is barline, the space
    //should be accounted as fixed space at start of this slice
    if (!m_prev || m_prev->get_type() == TimeSlice::k_barline)
        m_xLeft -= xPrev;
    else
    {
        //[SP??] test 00138 (first staff) - intermediate clef must go near next note
        //       test 50036 - no space between note and direction (dynamics)
        //there must not be extra space between previous non-timed slice and this
        //one. Therefore, transfer of accidentals and other space at start must skip
        //any previous non-timed (i.e. clef, directions), and the space must be only
        //transferred to previous one if it is not a barline. Otherwise, the space
        //must be transferred to the non-timed as left-rod space.
        if (m_prev->get_type() == TimeSlice::k_non_timed)
        {
            TimeSlice* prev = m_prev->m_prev;   //must always exist. Error in code/logic if not!
            if (prev == nullptr)
            {
                //TODO: check this is true. Score without prolog starting with (space)
                LOMSE_LOG_ERROR("Error in code/logic. No previous slice exists for non-timed slice!");
                m_xLeft -= xPrev;
            }
            else
            {
                if (prev->get_type() != TimeSlice::k_barline)
                {
                    //[SP???] test ??????
                    //transfer accidentals and other space at start to previous noterest
                    prev->m_xRi -= xPrev;
                }
                else if (prev->get_type() == TimeSlice::k_non_timed)
                {
                    //[SP???] test ??????
                    //add space as left-rod in the previous non-timed slice
                    //except if prolog slice.
                    prev->m_xLi += xPrev;
                }
                else
                {
                    //If it is prolog do nothing for now. Lets assume that after prolog
                    //space will be enough
                    //test: 22a first system second page)
                }
            }
        }
        else
            //[SP040] test 00134, 00050
            //default case: transfer accidentals and other space at start to previous
            //non-timed slice
            m_prev->m_xRi -= xPrev;
    }
}

//---------------------------------------------------------------------------------------
int TimeSlice::collect_barlines_information(int numInstruments)
{
    int info = 0;

    if (m_type != TimeSlice::k_barline)
        return info;

    //vector: one entry per instrument. Value = barline type or -1 if no barline
    vector<int> barlines;
    barlines.resize(numInstruments);

    ColStaffObjsEntry* pEntry = m_firstEntry;
    for (int i=0; i < m_numEntries; ++i, pEntry = pEntry->get_next())
    {
        ImoStaffObj* pSO = pEntry->imo_object();
        int iInstr = pEntry->num_instrument();
        if (pSO->is_barline())
        {
            ImoBarline* pBarline = static_cast<ImoBarline*>(pSO);
            barlines[iInstr] = pBarline->get_type();
        }
        else
            barlines[iInstr] = -1;
    }

    //summarize information
    info = k_all_instr_have_barline | k_all_instr_have_final_barline;   //assume this

    for (int i=0; i < numInstruments; ++i)
    {
        if (barlines[i] == -1)
            info &= ~(k_all_instr_have_barline | k_all_instr_have_final_barline);
        else
        {
            info |= k_some_instr_have_barline;
            if (!(barlines[i] == k_barline_end || barlines[i] == k_barline_end_repetition))
                info &= ~k_all_instr_have_final_barline;
        }
    }
    return info;
}

//---------------------------------------------------------------------------------------
void TimeSlice::add_lyrics()
{
    if (m_type != TimeSlice::k_noterest)
        return;

    ColStaffObjsEntry* pEntry = m_firstEntry;
    for (int i=0; i < m_numEntries; ++i, pEntry = pEntry->get_next())
    {
        ImoStaffObj* pSO = pEntry->imo_object();

        if (pSO->is_note() && pSO->get_num_attachments() > 0)
        {
            ImoAttachments* pAuxObjs = pSO->get_attachments();
            int size = pAuxObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
                if (pAO->is_lyric())
                {
                    ImoLyric* pLyric = static_cast<ImoLyric*>(pAO);
                    m_lyrics.push_back(pLyric);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits TimeSlice::measure_lyric(ImoLyric* pLyric, ScoreMeter* pMeter,
                                TextMeter& textMeter)
{
    //TODO: Move this method to another object. TimeSlice shoul not have knowledge
    //about the lyrics GM structure. Move perhaps to engraver?

    //TODO tenths to logical: must use instrument & staff for pSO

    LUnits totalWidth = 0;
    ImoStyle* pStyle = nullptr;
    int numSyllables = pLyric->get_num_text_items();
    for (int i=0; i < numSyllables; ++i)
    {
        //get text for syllable
        ImoLyricsTextInfo* pText = pLyric->get_text_item(i);
        const string& text = pText->get_syllable_text();
        const string& language = pText->get_syllable_language();
        pStyle = pText->get_syllable_style();
        if (pStyle == nullptr)
            pStyle = pMeter->get_style_info("Lyrics");

        //measure this syllable
        totalWidth += measure_text(text, pStyle, language, textMeter);

        //elision symbol
        if (pText->has_elision())
        {
            const string& elision = pText->get_elision_text();
            totalWidth += measure_text(elision, pStyle, "en", textMeter)
                          + pMeter->tenths_to_logical_max(2.0);
        }
    }
    totalWidth += pMeter->tenths_to_logical_max(10.0);

    //hyphenation, if needed
    if (pLyric->has_hyphenation() && !pLyric->has_melisma())
    {
        totalWidth += measure_text("-", pStyle, "en", textMeter)
                      + pMeter->tenths_to_logical_max(10.0);
    }

    return totalWidth;
}

//---------------------------------------------------------------------------------------
LUnits TimeSlice::measure_text(const string& text, ImoStyle* pStyle,
                               const string& language, TextMeter& meter)
{
    meter.select_font(language,
                      pStyle->font_file(),
                      pStyle->font_name(),
                      pStyle->font_size(),
                      pStyle->is_bold(),
                      pStyle->is_italic() );
    return meter.measure_width(text);
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
void TimeSlice::move_shapes_to_final_positions(vector<StaffObjData*>& data, LUnits xPos,
                                               LUnits yPos, LUnits* yMin, LUnits* yMax,
                                               ScoreMeter* UNUSED(pMeter))
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

            //update system vertical limits
            *yMax = max(*yMax, pShape->get_bottom());
            *yMin = min(*yMin, pShape->get_top());
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
    : TimeSlice(pEntry, entryType, column, iData)
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
                                            ScoreMeter* pMeter,
                                            TextMeter& UNUSED(textMeter))
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
                                                     LUnits xPos, LUnits yPos,
                                                     LUnits* yMin, LUnits* yMax,
                                                     ScoreMeter* UNUSED(pMeter))
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

            //update system vertical limits
            *yMax = max(*yMax, pShape->get_bottom());
            *yMin = min(*yMin, pShape->get_top());
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceProlog::remove_after_space_if_not_full(ScoreMeter* pMeter, int SOtype)
{
    //if only one staff and only one object, remove after space unless
    //next object is of different type than this one.
    //This is a trick for non-valid scores used as notation examples, such
    //as an staff with all clef types

    if (m_numEntries == 1)
    {
        ImoStaffObj* pSO = m_lastEntry->imo_object();
        if (SOtype == pSO->get_obj_type())
        {
            m_xLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_PROLOG);
            m_xLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);
            if (!m_prev)
                m_xLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);
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
    : TimeSlice(pEntry, entryType, column, iData)
{

}

//---------------------------------------------------------------------------------------
TimeSliceNonTimed::~TimeSliceNonTimed()
{

}

//---------------------------------------------------------------------------------------
void TimeSliceNonTimed::assign_spacing_values(vector<StaffObjData*>& data,
                                              ScoreMeter* pMeter,
                                              TextMeter& UNUSED(textMeter))
{
	//compute width for this slice. As objects in this slice can be for different
	//instruments/staves we maintain a width value for each staff and transfer to
	//previous slice the maximum of them.

    m_xLeft = 0.0f;
    m_xLi = 0.0f;
    m_xRi = 0.0f;
    m_interSpace = pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);

    //vector for widths
    int numStaves = pMeter->num_staves();
    m_numStaves = numStaves;
    m_widths.assign(numStaves, 0.0f);

    //loop for computing widths
    bool fHasWidth = false;            //true if at least one shape has width
    LUnits maxWidth = 0.0f;
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = data[i]->get_shape();
        if (pShape)
        {
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            LUnits width = pShape->get_width();
            if (width > 0.0f)
            {
                m_widths[iStaff] += m_interSpace + width;
                fHasWidth = true;
            }
            maxWidth = max(maxWidth, m_widths[iStaff]);
        }
    }

    //[SP???] Do not add extra space for zero with slices
    if (fHasWidth)
        maxWidth += m_interSpace;   //add space before next slice symbol


    //transfer width to previous timed slice
    //For timepos vertical alignment, all extra space before notes must be
    //transferred to previous timed object as right rod space.

    //[SP061] test 12a
    //if previous slice is barline, space must not be transferred to it (as right rod
    //space) because if it is transferred and justification
	//decides to place a new system after the barline, the barline will
	//not be right justified. Therefore, if previous slice is barline, the space
	//should be accounted as fixed space at start of this slice
    if (!m_prev || m_prev->get_type() == TimeSlice::k_barline)
    {
        m_realWidth = 0.0f;
        m_xLeft = maxWidth;

        //[SP???] test 50035: distance from barline to 1st note in 2nd measure
        //                    must be equal to that in 3rd measure
        //if null width add some extra space for separation from barline
        if (maxWidth == 0.0f)
            m_xLeft += pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_BARLINE / 2.0f);
    }
    else
    {
        m_realWidth = maxWidth;

        //[SP011] test 00136 (intermediate G clef should not take extra space)
        //Space for non-timed should not be added as a full rod
        //in previous slice when there are no objects for the staves in which these
        //non-timed are placed. Instead, transfer only the minimum required for
        //ensuring that previous xRi rod is at least equal to the required space
        //for this slice.
        bool fNoOverlap = true;
        ColStaffObjsEntry* pEntry = m_prev->m_firstEntry;
        int iMax = m_prev->m_numEntries;
        for (int i=0; i < iMax; ++i, pEntry = pEntry->get_next())
        {
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            fNoOverlap &= m_widths[iStaff] == 0.0f;
        }

        if (fNoOverlap)
            m_prev->set_minimum_xi(maxWidth - m_interSpace);
        else
            //[SP060] test
            //default case: transfer width to previous slice
            m_prev->increment_xRi(maxWidth);
    }

    //Fix for snippet scores (i.e. 00100)
    //The score has only one staff and starts with non-timed after prolog.
    //If this initial non-timed is clef, key or time signature and the
    //the prolog is missing any (clef, key or time) remove afterspace in the prolog.
    if (numStaves == 1 && m_prev && m_prev->get_type() == TimeSlice::k_prolog)
    {
        ImoStaffObj* pSO = m_firstEntry->imo_object();
        int type = pSO->get_obj_type();
        switch(type)
        {
            case k_imo_clef:
            case k_imo_key_signature:
            case k_imo_time_signature:
            {
                TimeSliceProlog* pSlice = static_cast<TimeSliceProlog*>(m_prev);
                pSlice->remove_after_space_if_not_full(pMeter, type);
            }
            default:
                break;    //break?  what!!!
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNonTimed::move_shapes_to_final_positions(vector<StaffObjData*>& data,
                                                       LUnits xPos, LUnits yPos,
                                                       LUnits* yMin, LUnits* yMax,
                                                       ScoreMeter* pMeter)
{
    //vector for current position on each staff
    vector<LUnits> positions;
    positions.assign(m_numStaves, 0.0f);
    for (int i=0; i < m_numStaves; ++i)
        positions[i] = xPos - m_widths[i] + m_xLeft;

    //loop for positioning shapes. They must be right aligned
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstData + m_numEntries;
    for (int i=m_iFirstData; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        StaffObjData* pData = data[i];
        GmoShape* pShape = pData->get_shape();
        if (pShape)
        {
            //compute left position for this object
            //int iStaff = pEntry->staff();
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            LUnits xLeft = positions[iStaff];

            //move shape
            pShape->set_origin_and_notify_observers(xLeft + pData->m_xUserShift,
                                                    yPos + pData->m_yUserShift);

            positions[iStaff] += pShape->get_width() + m_interSpace;

            //update system vertical limits
            *yMax = max(*yMax, pShape->get_bottom());
            *yMin = min(*yMin, pShape->get_top());
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
    , m_xPos(0.0f)
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
    m_orderedSlices.assign(numSlices, (TimeSlice*)nullptr);   //GCC 4.8.4 complains if nullptr not casted
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::order_slices()
{
    //load vector and take the opportunity for collecting some data
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
void ColumnDataGourlay::collect_barlines_information(int numInstruments)
{
    //get last slice
    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices() - 1; ++i)
        pSlice = pSlice->next();

    //and collect info
    m_barlinesInfo = pSlice->collect_barlines_information(numInstruments);
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
                                                       LUnits xPos, LUnits yPos,
                                                       LUnits* yMin, LUnits* yMax,
                                                       ScoreMeter* pMeter)
{
    m_xPos = xPos;

    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        pSlice->move_shapes_to_final_positions(data, xPos, yPos, yMin, yMax, pMeter);
        xPos += pSlice->get_width();
        pSlice = pSlice->next();
    }
}

//---------------------------------------------------------------------------------------
bool ColumnDataGourlay::is_empty_column()
{
    return m_pFirstSlice == nullptr;
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
    //function sff(x) : Determining the required force for a given extent x
    //The result is exact. The returned force achieves the required extent.

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

    //dbgLogger << "determining approx. sff for column. F=" << F << endl;

    m_slope = 0.0f;
    m_xFixed = 0.0f;
    m_minFi = LOMSE_MAX_FORCE;
    float cFiMin = 0.0f;
    vector<TimeSlice*>::iterator it;
    for (it = m_orderedSlices.begin(); it != m_orderedSlices.end(); ++it)
    {
        m_xFixed += (*it)->m_xLeft;
        if (m_minFi > (*it)->m_fi)
        {
            m_minFi = (*it)->m_fi;
            cFiMin = (*it)->m_ci;
        };

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

//        dbgLogger << "    Slice: type="<< (*it)->get_type()
//                  << ", 1st data=" << (*it)->dbg_get_first_data()
//                  << ", width=" << (*it)->get_width()
//                  << ", m_fi= " << (*it)->m_fi
//                  << ", m_ci=" << (*it)->m_ci
//                  << ", xi=" << (*it)->get_xi()
//                  << ", m_xFixed=" << m_xFixed
//                  << ", m_xLeft=" << (*it)->m_xLeft
//                  << ", slope=" << m_slope << endl;
    }

    //if F is lower than minimum force for all springs, use the
    //slope for minimum force at which the column will react
    if (m_slope == 0.0f)
        m_slope = 1.0f / cFiMin;

//    dbgLogger << "Final slope=" << m_slope
//              << ", xFixed=" << m_xFixed
//              << endl;
}

//---------------------------------------------------------------------------------------
TimeGridTable* ColumnDataGourlay::create_time_grid_table()
{

    TimeGridTable* table = LOMSE_NEW TimeGridTable();

    TimeSlice* pSlice = m_pFirstSlice;
    LUnits xPos = get_position();

    for (int i=0; i < num_slices(); ++i)
    {
        TimeUnits rCurTime = pSlice->get_timepos();
        TimeUnits rDuration = pSlice->get_spring_duration();
        LUnits x = xPos + pSlice->get_fixed_extent();
        TimeGridTableEntry entry = { rCurTime, rDuration, x };
        table->add_entry(entry);

        xPos += pSlice->get_width();
        pSlice = pSlice->next();
    }

    return table;
}


//=====================================================================================
//StaffObjData implementation
//=====================================================================================
StaffObjData::StaffObjData()
    : m_pShape(nullptr)
    , m_xUserShift(0.0f)
    , m_yUserShift(0.0f)
{
}

//---------------------------------------------------------------------------------------
StaffObjData::~StaffObjData()
{
}


}  //namespace lomse

