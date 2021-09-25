//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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
#include "lomse_box_system.h"
#include "lomse_score_layouter.h"
#include "lomse_gm_measures_table.h"
#include "lomse_calligrapher.h"
#include "lomse_graphical_model.h"
#include "lomse_vertical_profile.h"
#include "lomse_shape_note.h"
#include "lomse_noterests_collisions_fixer.h"


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
SpAlgGourlay::SpAlgGourlay(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           ScoreLayouter* pScoreLyt, ImoScore* pScore,
                           EngraversMap& engravers, ShapesCreator* pShapesCreator,
                           PartsEngraver* pPartsEngraver)
    : SpAlgColumn(libraryScope, pScoreMeter, pScoreLyt, pScore, engravers,
                  pShapesCreator, pPartsEngraver)
    , m_pCurSlice(nullptr)
    , m_pLastEntry(nullptr)
    , m_prevType(TimeSlice::k_undefined)
    , m_prevTime(0.0)
    , m_prevAlignTime(0.0)
    , m_numEntries(0)
    , m_pCurColumn(nullptr)
    , m_numSlices(0)
    , m_iPrevColumn(-1)
    //
    , m_lastPrologTime(-1.0)
    //
    , m_maxNoteDur(0.0)
    , m_minNoteDur(LOMSE_NO_DURATION)
    //
	, m_uSmin(0.0f)
    , m_alpha(0.0f)
    , m_dmin(0.0f)
    , m_Fopt(0.0f)
{
    ColStaffObjs* pCol = pScore->get_staffobjs_table();
    m_shapes.reserve(pCol->num_entries());
    m_lastSequence.assign(pCol->num_lines(), SeqData::k_seq_isolated);
    m_lastSlice.assign(pCol->num_lines(), nullptr);
}

//---------------------------------------------------------------------------------------
SpAlgGourlay::~SpAlgGourlay()
{
//    if (int(m_columns.size()) != m_pScoreLyt->get_num_columns())
//        dbgLogger << "ERROR. In SpAlgGourlay: more columns than reserved space. "
//                << "Reserved= " << m_pScoreLyt->get_num_columns()
//                << ", current= " << m_columns.size() << endl;

    if (int(m_shapes.size()) > m_pScore->get_staffobjs_table()->num_entries())
    {
        stringstream ss;
        ss << "Investigate: more shapes than reserved space. "
           << "Reserved= " << m_pScore->get_staffobjs_table()->num_entries()
           << ", current= " << m_shapes.size();
        LOMSE_LOG_ERROR(ss.str());
    }

    vector<ShapeData*>::iterator itD;
    for (itD = m_shapes.begin(); itD != m_shapes.end(); ++itD)
        delete *itD;
    m_shapes.clear();

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

    //uncomment these lines to break sequences at end of each column
    for (size_t i=0; i < m_lastSequence.size(); ++i)
        m_lastSequence[i] = SeqData::k_seq_isolated;

    for (size_t i=0; i < m_lastSlice.size(); ++i)
        m_lastSlice[i] = nullptr;
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::finish_sequences()
{
    for (size_t iLine=0; iLine < m_lastSlice.size(); ++iLine)
    {
        if (m_lastSlice[iLine])
            m_lastSlice[iLine]->finish_sequences(iLine);
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::new_column(TimeSlice* pSlice)
{
    m_pCurColumn = LOMSE_NEW ColumnDataGourlay(pSlice);
    m_columns.push_back(m_pCurColumn);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::include_object(ColStaffObjsEntry* pCurEntry, int iCol, int iInstr,
                                  int iStaff, ImoStaffObj* pSO, GmoShape* pShape,
                                  bool fInProlog)
{
    save_info_for_shape(pShape, iInstr, iStaff);
    int curType = determine_required_slice_type(pSO, fInProlog);
    TimeUnits curTime = pCurEntry->time();
    bool fCreateNewSlice = determine_if_new_slice_needed(pCurEntry, curTime, curType, pSO);

    //include entry in current or new slice
    if (fCreateNewSlice)
    {
//        dbgLogger << ", Start new slice. m_prevType:" << m_prevType
//            << ", m_prevTime:" << m_prevTime << ", curTime:" << curTime
//            << endl;
        //terminate previous slice
        finish_slice(m_pLastEntry, m_numEntries);

        //and start a new slice
        new_slice(pCurEntry, curType, iCol, int(m_shapes.size())-1);

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

        if (pSO->is_grace_note())
        {
            ImoGraceNote* pGrace = static_cast<ImoGraceNote*>(pSO);
            m_prevAlignTime = pGrace->get_align_timepos();
        }
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

//    //save data about full-measure rests
//    if (pSO->is_rest() && static_cast<ImoRest*>(pSO)->is_full_measure())
//        m_pCurColumn->include_full_measure_rest(pShape, pCurEntry);

    //save data for noterests lines and sequences
    if (curType == TimeSlice::k_noterest)
    {
        int iLine = pCurEntry->line();
        int curSeq = m_lastSequence[iLine];
        TimeSliceNoterest* pPrevSlice = m_lastSlice[iLine];
        TimeSliceNoterest* pCurSlice = static_cast<TimeSliceNoterest*>(m_pCurSlice);
        if (pPrevSlice && pPrevSlice != pCurSlice)
        {
            TimeUnits prevDuration = pPrevSlice->get_duration(iLine);
            int prevSeq = pPrevSlice->get_sequence(iLine);
            if (is_equal_time(prevDuration, pCurEntry->duration()) )
            {
//                dbgLogger << "Line " << iLine << ". Equal time. prevSeq=" << prevSeq
//                    << ", curSeq set to 'continue'";
                //both notes have equal duration. curSeq = continue
                curSeq = SeqData::k_seq_continue;
                if (prevSeq == SeqData::k_seq_isolated)
                    pPrevSlice->update_sequence(iLine, SeqData::k_seq_start);
                else if (prevSeq == SeqData::k_seq_end)
                    pPrevSlice->update_sequence(iLine, SeqData::k_seq_end_start);
                else
                {
                    //equal time and prev is continue || start || end_start
                    //nothing to fix in pPrevSlice
                }

//                dbgLogger << ", prevSeq changed to " << prevSeq << endl;
            }
            else
            {
//                dbgLogger << "Line " << iLine << ". Different time. prevSeq=" << prevSeq
//                    << ", curSeq set to 'isolated'";
                //both notes have different duration. curSeq = isolated || end
                if (prevSeq == SeqData::k_seq_continue)
                    curSeq = SeqData::k_seq_end;
                else if (prevSeq == SeqData::k_seq_end)
                    curSeq = SeqData::k_seq_isolated;
                else if (prevSeq == SeqData::k_seq_isolated)
                    curSeq = SeqData::k_seq_isolated;
                else
                {
                    //different time and prev is end_start || start. Impossible!
                    stringstream ss;
                    ss << "Impossible case: different duration and prevSeq=" << prevSeq;
                    LOMSE_LOG_ERROR(ss.str());
                }
//                dbgLogger << ", prevSeq changed to " << prevSeq << endl;
            }
        }
        pCurSlice->save_seq_data(pCurEntry, curSeq);
        m_lastSequence[iLine] = curSeq;
        m_lastSlice[iLine] = pCurSlice;
    }

    m_pLastEntry = pCurEntry;
    ++m_numEntries;
}

//---------------------------------------------------------------------------------------
ShapeData* SpAlgGourlay::save_info_for_shape(GmoShape* pShape, int iInstr, int iStaff)
{
    ShapeData* pShapeData = LOMSE_NEW ShapeData();
    m_shapes.push_back(pShapeData);

    int idxStaff = m_pScoreMeter->staff_index(iInstr, iStaff);
    pShapeData->m_pShape = pShape;
    pShapeData->m_idxStaff = idxStaff;
    pShapeData->m_iStaff = iStaff;

    return pShapeData;
}

//---------------------------------------------------------------------------------------
int SpAlgGourlay::determine_required_slice_type(ImoStaffObj* pSO, bool fInProlog)
{
    //determine slice type for the new object to include

//    dbgLogger << "include_object(). StaffObj type = " << pSO->get_name()
//              << ", id=" << pSO->get_id() << endl;
    if (fInProlog)
        return TimeSlice::k_prolog;
    else
    {
        switch(pSO->get_obj_type())
        {
            case k_imo_clef:
            case k_imo_key_signature:
            case k_imo_time_signature:
            case k_imo_direction:
            case k_imo_metronome_mark:
            case k_imo_go_back_fwd:
            case k_imo_figured_bass:
            case k_imo_sound_change:
            case k_imo_transpose:
            case k_imo_system_break:
                return TimeSlice::k_non_timed;

            case k_imo_note_regular:
            case k_imo_note_cue:
            case k_imo_rest:
                return TimeSlice::k_noterest;

            case k_imo_note_grace:
                return TimeSlice::k_graces;

            case k_imo_barline:
                return TimeSlice::k_barline;

            default:
                stringstream ss;
                ss << "Investigate: un-expected staff object. Name= "
                   << pSO->get_name();
                LOMSE_LOG_ERROR(ss.str());
                return TimeSlice::k_non_timed;
        }
    }
}

//---------------------------------------------------------------------------------------
bool SpAlgGourlay::determine_if_new_slice_needed(ColStaffObjsEntry* pCurEntry,
                                                 TimeUnits curTime, int curType,
                                                 ImoStaffObj* pSO)
{
    bool fCreateNewSlice = false;
    if (curType != m_prevType || !is_equal_time(m_prevTime, curTime) )
        fCreateNewSlice = true;
    else if (pSO->is_grace_note())
    {
        ImoGraceNote* pGrace = static_cast<ImoGraceNote*>(pSO);
        if (!is_equal_time(m_prevAlignTime, pGrace->get_align_timepos()) )
            fCreateNewSlice = true;
    }

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
    return fCreateNewSlice;
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::include_full_measure_rest(GmoShape* pRestShape,
                                             ColStaffObjsEntry* pCurEntry,
                                             GmoShape* pNonTimedShape)
{
    m_pCurColumn->include_full_measure_rest(pRestShape, pCurEntry, pNonTimedShape);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::new_slice(ColStaffObjsEntry* pEntry, int entryType, int iColumn,
                             int iShape)
{
    //create the slice object
    TimeSlice* pSlice;
    switch(entryType)
    {
        case TimeSlice::k_prolog:
            pSlice = LOMSE_NEW TimeSliceProlog(pEntry, iColumn, iShape);
            break;
        case TimeSlice::k_non_timed:
            pSlice = LOMSE_NEW TimeSliceNonTimed(pEntry, iColumn, iShape);
            break;
        case TimeSlice::k_barline:
            pSlice = LOMSE_NEW TimeSliceBarline(pEntry, iColumn, iShape);
            break;
        case TimeSlice::k_noterest:
        {
            int numLines = m_pScoreMeter->num_lines();
            pSlice = LOMSE_NEW TimeSliceNoterest(pEntry, iColumn, iShape, numLines,
                                                 m_pScoreMeter->num_staves());
            break;
        }
        case TimeSlice::k_graces:
            pSlice = LOMSE_NEW TimeSliceGraces(pEntry, iColumn, iShape);
            break;
        default:
            stringstream ss;
            ss << "Error in code/logic. Invalid TimeSlice type " << entryType;
            LOMSE_LOG_ERROR(ss.str());
            throw std::runtime_error(ss.str());
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
        m_pCurSlice->set_final_data(pLastEntry, numEntries, m_maxNoteDur, m_minNoteDur,
                                    m_pScoreMeter);
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::finish_column_measurements(int UNUSED(iCol))
{
    //terminate last slice
    finish_slice(m_pLastEntry, m_numEntries);

    finish_sequences();

    if (m_pCurColumn)
        m_pCurColumn->set_num_entries(m_numSlices);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::do_spacing(int iColumnToTrace)
{
    //when this method is invoked, all columns in the score have been created and the
    //information collected.

    //collect information, mainly by processing slices
    determine_spacing_parameters();
    compute_rods_ds_and_di();
    fix_neighborhood_spacing_problems(iColumnToTrace);
    compute_springs();

    //all information ready. Proceed by columns
    int numInstruments = m_pScoreMeter->num_instruments();
    int iCol = 0;
    vector<ColumnDataGourlay*>::iterator it;
    for (iCol=0, it=m_columns.begin(); it != m_columns.end(); ++it, ++iCol)
    {
        (*it)->order_slices();
        (*it)->collect_barlines_information(numInstruments);
        (*it)->determine_minimum_width();
        (*it)->apply_force(m_Fopt);     //to get an initial estimation for columns width
        (*it)->determine_approx_sff_for(m_Fopt);

        if ((iCol == iColumnToTrace) || m_libraryScope.dump_column_tables())
        {
            dbgLogger << " ****************************** After applying Fopt = "
                << m_Fopt << endl;
            dbgLogger << dump_spacing_parameters();
            (*it)->dump(logger.get_stream());
            dbgLogger << endl;
        }
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::determine_spacing_parameters()
{
    stringstream ss;
    if (m_libraryScope.use_debug_values())
    {
        m_Fopt = m_libraryScope.get_optimum_force();
        m_alpha = m_libraryScope.get_spacing_alpha();
        m_dmin = m_libraryScope.get_spacing_dmin();
        m_uSmin = m_pScoreMeter->tenths_to_logical_max(
                            m_libraryScope.get_spacing_smin() );
        ss << "Using debug";
    }
    else
    {
        m_Fopt = m_pScoreMeter->get_spacing_Fopt();
        m_alpha = m_pScoreMeter->get_spacing_alpha();
        m_dmin = m_pScoreMeter->get_spacing_dmin();
        m_uSmin = m_pScoreMeter->get_spacing_smin();
        ss << "Using default";
    }
    ss << " values for spacing parameters: alpha=" << m_alpha << ", Fopt="
        << m_Fopt << ", Smin =" << m_uSmin;

    //m_dmin==0.0 implies that value must be determined by the spacing algorithm
    if (m_dmin == 0.0f)
    {
        ColStaffObjs* pCol = m_pScore->get_staffobjs_table();
        float h = float(pCol->num_half_noterests());
        float q = float(pCol->num_quarter_noterests());
        float e = float(pCol->num_eighth_noterests());
        float s = float(pCol->num_16th_noterests());
        int dmin = pCol->min_note_duration();
        float total = float(h + q + e + s);

        //m_dmin should be selected based on notes in the score, not a fixed value.
        //But using greater values creates ugly scores, too compressed.
        m_dmin = 16.0;

//        if (total < 100.0)
//            m_dmin = 16.0f;
//        else
//        {
//            m_dmin = min(48.0f, float(m_pScore->get_staffobjs_table()->min_note_duration()));
//            if (s/total > 0.1f)
//                m_dmin = 16.0f;
//            else if (e/total > 0.1f)
//                m_dmin = 32.0f;
//            else
//            {
//                m_dmin = float(m_pScore->get_staffobjs_table()->min_note_duration());
//                if (m_dmin > 48.0f)
//                    m_dmin = 48.0f;
//            }
//        }


        ss << ", h=" << h << ", q=" << q << ", e=" << e << ", s=" << s
            << ", total=" << total << ", dmin=" << dmin << ", Dmin=" << m_dmin;
        LOMSE_LOG_INFO(ss.str());
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::compute_rods_ds_and_di()
{
    TextMeter textMeter(m_libraryScope);
    list<TimeSlice*>::iterator it;
    for (it = m_slices.begin(); it != m_slices.end(); ++it)
    {
        (*it)->assign_spacing_values(m_shapes, m_pScoreMeter, textMeter);
        (*it)->compute_ds_and_di();
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::fix_neighborhood_spacing_problems(int iColumnToTrace)
{
    vector<ColumnDataGourlay*>::iterator it;
    int iCol = 0;
    for (it = m_columns.begin(); it != m_columns.end(); ++it, ++iCol)
    {
        bool fTrace = (iCol == iColumnToTrace) || m_libraryScope.dump_column_tables();
        (*it)->fix_neighborhood_spacing_problems(fTrace);
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::compute_springs()
{
    LUnits dsFixed = m_pScoreMeter->tenths_to_logical_max(
                                m_pScoreMeter->get_spacing_value());
    bool fProportional = m_pScoreMeter->is_proportional_spacing();
    list<TimeSlice*>::iterator it;
    for (it = m_slices.begin(); it != m_slices.end(); ++it)
        (*it)->compute_spring_data(m_uSmin, m_alpha, m_dmin, fProportional, dsFixed);
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
        m_columns[iCol]->move_shapes_to_final_positions(m_shapes, xLeft, yTop + yShift,
                                                        yMin, yMax, m_pScoreMeter,
                                                        m_pVProfile);

        //assign the final width to the boxes
        LUnits colWidth = m_columns[iCol]->get_column_width();
        set_slice_width(iCol, colWidth);

        xLeft += colWidth;
    }
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::reposition_full_measure_rests(int iFirstCol, int iLastCol,
                                                 GmoBoxSystem* pBox)
{
    GraphicModel* pGM = m_pScoreLyt->get_graphic_model();
    GmMeasuresTable* pMeasures = pGM->get_measures_table( m_pScore->get_id() );

    for (int iCol = iFirstCol; iCol < iLastCol; ++iCol)
    {
        m_columns[iCol]->reposition_full_measure_rests(pBox, pMeasures);
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
    float Fprev1 = m_Fopt;
    int round=0;
    float Fmin = 0.0f;              //F must be >= than Fmin
    float Fmax = LOMSE_MAX_FORCE;   //F must be < Fmax

    while (abs(uError) > 10.0f && round < 12)     //10.0f = one tenth of a millimeter
    {
//        dbgLogger << "Iter. " << round << ", Error = " << uError
//            << ", space incr= " << uSpaceIncrement;

        ++round;

        float deltaF = F - Fprev1;
        LUnits deltaS = uSpaceIncrement - uError;
        float Fsave = F;

        if (uError > 0.0f)
        {
            //F is not enough. More stretch needed
            Fmin = max (Fmin, F);
            if (abs(deltaS) < 1.0f)
            {
                //Previous force did nothing. Much more stretch needed
                F *= 2.0f;
//                dbgLogger << ", case=1";
            }
            else
            {
                F += (uError/deltaS) * deltaF;
//                dbgLogger << ", case=2";
            }
            if (F > Fmax)
                F = (Fprev1 + Fmax) / 2.0f;
        }
        else
        {
            //F is excessive. Less stretch needed
            Fmax = min(Fmax, F);
            if (abs(deltaS) < 1.0f)
            {
                //Previous force did nothing. Much less stretch needed
                F *= 0.5f;
//                dbgLogger << ", case=3";
            }
            else
            {
                F += (uError/deltaS) * deltaF;
//                dbgLogger << ", case=4";
            }
            if (F < Fmin)
                F = Fprev1 + (Fprev1 + Fmin) / 2.0f;
        }
        uSpaceIncrement = uError;
        Fprev1 = Fsave;

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
//                  << ", Fprev2= " << Fprev2
//                  << ", Fprev1= " << Fprev1
//                  << ", Fmin= " << Fmin
//                  << ", Fmax= " << Fmax
//                  << endl;
    }
}

//---------------------------------------------------------------------------------------
string SpAlgGourlay::dump_spacing_parameters()
{
    stringstream ss;
    ss << "dmin=" << m_dmin << ", space(dmin)=" << m_uSmin
       << ", Fopt=" << m_Fopt << endl;
    return ss.str();
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
    m_columns[iCol]->add_shapes_to_box(pSliceInstrBox, iInstr, m_shapes);
}

//---------------------------------------------------------------------------------------
void SpAlgGourlay::delete_shapes(int iCol)
{
    m_columns[iCol]->delete_shapes(m_shapes);
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
            dbgLogger << "Determine penalty: minimum width " << minWidth
                      << " is greater than required width " << lineWidth
                      << ". Penalty= 1000" << endl;
        }
        return 1000.0f;
    }

    //determine force to apply to get desired extent
    float F = (lineWidth - fixed) * c;

    //compute penalty:  R(ci, cj) = | sff[cicj](line_width) - fopt |
    float R = fabs(F - m_Fopt);

    if (fTrace)
    {
        dbgLogger << "Determine penalty: lineWidth=" << lineWidth
                  << ", Force="  << F << ", sum=" << sum << ", c=" << c
                  << ", R=" << R << ", iSystem=" << iSystem
                  << endl;
    }


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

    //increase penalty for columns not ended in barline, except when they end in key
    //or time signature after the barline
    if (!m_columns[iLastCol]->all_instr_have_barline_TS_or_KS())
    {
        R *= 4.0f;
        if (!m_columns[iLastCol]->some_instr_have_barline_TS_or_KS())
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
TimeSlice::TimeSlice(ColStaffObjsEntry* pEntry, int entryType, int column, int iShape)
    : m_firstEntry(pEntry)
    , m_lastEntry(pEntry)
    , m_iFirstShape(iShape)
    , m_numEntries(1)
    , m_type(entryType)
    , m_iColumn(column)
    , m_next(nullptr)
    , m_prev(nullptr)
    //
    , m_dxLeft(0.0f)
    , m_dxL(0.0f)
    , m_dxR(0.0f)
    , m_fi(0.0f)
    , m_c(0.0f)
    , m_width(0.0f)
    , m_ds(0.0)
    , m_di(0.0)
    , m_minNote(0.0)
    , m_minNoteNext(0.0)
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
                               TimeUnits maxNextTime, TimeUnits minNote,
                               ScoreMeter* pMeter)
{
    m_lastEntry = pLastEntry;
    m_numEntries = numEntries;
    m_ds = maxNextTime;
    m_minNote = minNote;
    m_width = get_total_rods() + m_dxLeft;

    if (m_type == TimeSlice::k_noterest)
        static_cast<TimeSliceNoterest*>(this)->add_lyrics(pMeter);
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_ds_and_di()
{
    //compute spring duration ds
    TimeUnits nextTime = (m_next ? m_next->get_timepos() : m_ds + get_timepos());
    m_ds = nextTime - get_timepos();

    TimeUnits minNotePrev = (m_prev ? m_prev->get_min_note_still_sounding() : LOMSE_NO_DURATION);
    compute_smallest_duration_di(minNotePrev);

    find_smallest_note_soundig_at(nextTime);
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_spring_data(LUnits uSmin, float alpha, TimeUnits dmin,
                                    bool fProportional, LUnits dsFixed)
{
    compute_spring_constant(uSmin, alpha, dmin, fProportional, dsFixed);
    compute_pre_stretching_force();
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_smallest_duration_di(TimeUnits minNotePrev)
{
//    m_di = 19.2;

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
void TimeSlice::compute_spring_constant(LUnits uSmin, float alpha, TimeUnits dmin,
                                        bool fProportional, LUnits dsFixed)
{
    if (m_ds > 0.0f)
    {
        LUnits space_di;
        if (fProportional)
            space_di = spacing_function(m_di, uSmin, alpha, dmin);
        else
            space_di = dsFixed;
        m_c = float(m_di/m_ds) / space_di;  //* ( 1.0f / space_ds );
    }
    else if (m_type == TimeSlice::k_barline)
        m_c = 0.05f;       //aprox. ten times the hardness of the minimum spaced note
    else
        m_c = 0.0f;
}

//---------------------------------------------------------------------------------------
void TimeSlice::compute_pre_stretching_force()
{
    if (m_type == TimeSlice::k_noterest || m_type == TimeSlice::k_barline)
        m_fi = m_c * get_total_rods();
    else
        m_fi = LOMSE_MAX_FORCE;
}

//---------------------------------------------------------------------------------------
void TimeSlice::apply_force(float F)
{
    if (F > m_fi)
        m_width = F / m_c + m_dxLeft;
    else
        m_width = get_total_rods() + m_dxLeft;
}

//---------------------------------------------------------------------------------------
LUnits TimeSlice::spacing_function(TimeUnits d, LUnits uSmin, float alpha, TimeUnits dmin)
{
	if (d <= dmin)
        return uSmin;

    // This method computes:
    //    space(d) = G(d).space(dmin)       for d = m_ds
    // where
    //    G(d) = 1 + alpha.log2( d / dmin )
    //
    // space(dmin) is parameter uSmin

    return uSmin * (1.0f + alpha * log2(float(d / dmin)));
}

//---------------------------------------------------------------------------------------
int TimeSlice::collect_barlines_information(int numInstruments)
{
    //When any instrument finishes in KS or TS, the last Slice is a
    //TimeSliceNonTimed and barlines are not accesible. As a consequence this method
    //is assuming that when the column ends in a TimeSliceNonTimed and previous slice
    //is a TimeSliceBarlines, all instruments not including objects in the
    //TimeSliceNonTimed have barline. This assumption seems no dangerous but this notice
    //is here just in case.
    //TODO: The fix would be to check that previous slice TimeSliceBarlines has the
    //missing barlines.
    if (m_type != TimeSlice::k_barline && m_type != TimeSlice::k_non_timed)
        return 0;   //neither barlines, key signatures and time signatures

    if (m_type == TimeSlice::k_non_timed
        && (!m_prev || m_prev->get_type() != TimeSlice::k_barline) )
    {
        return 0;   //neither barlines, key signatures and time signatures
    }

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
        else if (pSO->is_key_signature() || pSO->is_time_signature())
            barlines[iInstr] = -2;
        else
            barlines[iInstr] = -1;
    }

    //summarize information
    int info = k_all_instr_have_barline
             | k_all_instr_have_final_barline
             | k_all_instr_have_barline_TS_or_KS;     //assume this

    for (int i=0; i < numInstruments; ++i)
    {
        if (barlines[i] == -1)
        {
            info &= ~(k_all_instr_have_barline | k_all_instr_have_final_barline
                      | k_all_instr_have_barline_TS_or_KS);
        }
        else if (barlines[i] == -2)
        {
            info &= ~(k_all_instr_have_barline | k_all_instr_have_final_barline);
            info |= k_some_instr_have_barline_TS_or_KS;
        }
        else
        {
            info |= (k_some_instr_have_barline | k_some_instr_have_barline_TS_or_KS);
            if (!(barlines[i] == k_barline_end || barlines[i] == k_barline_end_repetition))
                info &= ~k_all_instr_have_final_barline;
        }
    }
    return info;
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
    //       +      +........+........+........+........+........+........+........+........+........+........+...........+........+........+
    ss << "                                                                                                          1000x   1000xc                                                 " << endl;
    ss << "                       num.   ==================================                                  total    min    spring                           min     1st           " << endl;
    ss << "type        timepos  entries  line    dxB     dxNH      dxA  seq nbh  dxLeft      dxL      dxR     rods   force  constant     ds       di          Note   shape  distance" << endl;
    ss << "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
         //noterest       0.00     7 -------------------------------------   1     0.00     0.00   409.42   409.42   735.16     1.80    64.00    64.00       64.00     12     779.69  2227.68
         //                                0    -0.00   200.00     0.00  1
}

//---------------------------------------------------------------------------------------
string TimeSlice::slice_type_to_string(int sliceType)
{
    switch(sliceType)
    {
        case k_prolog:      return "prolog   ";
        case k_non_timed:   return "non-timed";
        case k_noterest:    return "noterest ";
        case k_barline:     return "barline  ";
        case k_graces:      return "graces   ";
        default:
            return "undefined";
    }
}

//---------------------------------------------------------------------------------------
void TimeSlice::dump(ostream& ss)
{
    std::ios_base::fmtflags f( ss.flags() );  //save formating options

    ss << slice_type_to_string(m_type) << " "
       << fixed << setprecision(2) << setfill(' ')
       << setw(9) << get_timepos()
       << setw(6) << m_numEntries
       << " -------------------------------------";

    dump_neighborhood(ss);

    ss << setw(9) << m_dxLeft
       << setw(9) << m_dxL
       << setw(9) << m_dxR
       << setw(9) << get_total_rods();

    if (m_fi == LOMSE_MAX_FORCE)
        ss << setw(9) << "infinite";
    else
        ss << setw(9) << m_fi * 1000;

    ss << setw(9) << m_c * 1000
       << setw(9) << m_ds
       << setw(9) << m_di;

    if (m_minNote == LOMSE_NO_DURATION)
        ss << setw(12) << "no note";
    else
        ss << setw(12) << m_minNote;

    ss << setw(7) << m_iFirstShape;

    //computed data
    ss << setw(11) << m_width;  //distance
    ss << setw(9) << 4.0/m_c;   //achieved extent

    ss << endl;

    dump_lines(ss);
    dump_rods(ss);

    ss.flags( f );  //restore formating options
}

//---------------------------------------------------------------------------------------
void TimeSlice::add_shapes_to_box(GmoBoxSliceInstr* pSliceInstrBox, int iInstr,
                                  vector<ShapeData*>& shapes)
{
    GmoBoxSystem* pBoxSystem = pSliceInstrBox->get_system_box();
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        ShapeData* pData = shapes[i];
        if (pEntry->num_instrument() == iInstr)
        {
            GmoShape* pShape = pData->get_shape();
            if (pShape)
			{
                pSliceInstrBox->add_shape(pShape, GmoShape::k_layer_notes,
                                          pData->m_iStaff);

                //collect barlines info for box system (measures table)
				if (pBoxSystem && pShape->is_shape_barline())
				{
                    ImoStaffObj* pSO = pEntry->imo_object();
				    if (pSO && pSO->is_barline()
                        && !static_cast<ImoBarline*>(pSO)->is_middle())
                    {
                        pBoxSystem->add_barline_info(pEntry->measure(),
                                                     pEntry->num_instrument());
                    }
				}
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSlice::delete_shapes(vector<ShapeData*>& shapes)
{
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i)
    {
        ShapeData* pData = shapes[i];
        delete pData->get_shape();
    }
}

//---------------------------------------------------------------------------------------
void TimeSlice::move_shapes_to_final_positions(vector<ShapeData*>& shapes, LUnits xPos,
                                               LUnits yPos, LUnits* yMin, LUnits* yMax,
                                               ScoreMeter* UNUSED(pMeter),
                                               VerticalProfile* pVProfile)
{
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i)
    {
        ShapeData* pData = shapes[i];
        GmoShape* pShape = pData->get_shape();
        if (pShape)
        {
            //move shape
            LUnits xLeft = xPos + pShape->get_anchor_offset() + m_dxLeft;
            pShape->set_origin_and_notify_observers(xLeft, yPos);

            //save info for vertical profile
            pVProfile->update(pShape, pData->m_idxStaff);

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
TimeSliceProlog::TimeSliceProlog(ColStaffObjsEntry* pEntry, int column, int iShape)
    : TimeSlice(pEntry, k_prolog, column, iShape)
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
void TimeSliceProlog::assign_spacing_values(vector<ShapeData*>& shapes,
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
    m_dxLeft = m_spaceBefore;
    m_dxL = 0.0f;
    m_dxR = 0.0f;

    //loop for computing space for objects in each staff
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = shapes[i]->get_shape();
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

    m_dxLeft += m_clefsWidth + m_keysWidth + m_timesWidth;
    m_dxLeft += pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_PROLOG);
}

//---------------------------------------------------------------------------------------
void TimeSliceProlog::move_shapes_to_final_positions(vector<ShapeData*>& shapes,
                                                     LUnits xPos, LUnits yPos,
                                                     LUnits* yMin, LUnits* yMax,
                                                     ScoreMeter* UNUSED(pMeter),
                                                     VerticalProfile* pVProfile)
{
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        ShapeData* pData = shapes[i];
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
            pShape->set_origin_and_notify_observers(xLeft, yPos);

            //save info for vertical profile
            pVProfile->update(pShape, pData->m_idxStaff);

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
            m_dxLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_PROLOG);
            m_dxLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);
            if (!m_prev)
                m_dxLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceProlog::remove_after_space(ScoreMeter* pMeter)
{
    m_dxLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_PROLOG);
    m_dxLeft -= pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);
}


//=====================================================================================
//TimeSliceNonTimed implementation
//- This slice contains one or more non-timed objects (clef, key signature,
//  time signature, spacers, other) at the same timepos, but not in prolog.
//- This slice represents right rod width for previous slice ==>
//   - this slice is fixed width slice, with a total width = 0
//   - real width of this slice must be transferred to previous slice as m_dxR rod
//- Objects are organized by lines
//=====================================================================================
TimeSliceNonTimed::TimeSliceNonTimed(ColStaffObjsEntry* pEntry, int column, int iShape)
    : TimeSlice(pEntry, k_non_timed, column, iShape)
    , m_numStaves(0)
    , m_interSpace(0.0f)
    , m_fHasWidth(false)
    , m_fSomeVisible(false)
{

}

//---------------------------------------------------------------------------------------
TimeSliceNonTimed::~TimeSliceNonTimed()
{

}

//---------------------------------------------------------------------------------------
void TimeSliceNonTimed::assign_spacing_values(vector<ShapeData*>& shapes,
                                              ScoreMeter* pMeter,
                                              TextMeter& UNUSED(textMeter))
{
	//compute width for this slice. As objects in this slice can be for different
	//instruments/staves we maintain a width value for each staff and transfer to
	//previous slice the maximum of them.
	//All space included as dxLeft. Always dxR=dxL=0

    m_dxLeft = 0.0f;
    m_dxL = 0.0f;
    m_dxR = 0.0f;
    m_interSpace = pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_SMALL_CLEF);

    //vector for widths
    int numStaves = pMeter->num_staves();
    m_numStaves = numStaves;
    m_widths.assign(numStaves, 0.0f);
    m_fHasObjects.assign(numStaves, false);

    //loop for computing widths
    m_fHasWidth = false;            //true if at least one shape has width
    m_fSomeVisible = false;         //true if at least one shape is visible
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = shapes[i]->get_shape();
        m_fSomeVisible |= !pShape->is_shape_invisible();
        int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
        m_fHasObjects[iStaff] = true;
        LUnits width = pShape->get_width();
        if (width > 0.0f)
        {
            m_widths[iStaff] += width;
            m_fHasWidth = true;
            //[NT2] test 00607. CHECK: some space between clef change and note.
            //                        Blue line on note, at center.
            if (!pShape->is_shape_invisible())
                m_widths[iStaff] += m_interSpace;
        }
        m_dxLeft = max(m_dxLeft, m_widths[iStaff]);
    }

    //Add some space before this slice, for separation from previous object.
    //Do not add this extra space for zero width slices or if all shapes are invisible
    //[NT1]
    //  test 00603. CHECK: some space between end of prolog and next clef
    //  test 00621. CHECK: no space added before the 'spacer' shape (invisible) - 1st note
    //                in measures 2 & 3 must be at the same distance from previous barline
    if (m_fHasWidth && m_fSomeVisible)
        m_dxLeft += m_interSpace;   //add space after previous slice


    //joining with previous slices
    if (m_dxLeft > 0.0f)
        join_with_previous(pMeter);
}

//---------------------------------------------------------------------------------------
void TimeSliceNonTimed::join_with_previous(ScoreMeter* pMeter)
{

    //result: transfer (or not) dxLeft. dxL=dxR=0

    //Non-timed after prolog: remove after prolog space when width > 0
    //[NT3]
    //  test 00602. CHECK: (draw anchor objects) spacer width is the separation between
    //                  the TS and the 1st note, no more.
    //  test 00613. CHECK: clefs must be equally spaced
    //  test 00617. CHECK: small space after prolog and clef change
    //  test 00618. CHECK: metronome does not take space
    if (m_dxLeft > 0.0f && m_prev && m_prev->get_type() == TimeSlice::k_prolog)
    {
        TimeSliceProlog* pSlice = static_cast<TimeSliceProlog*>(m_prev);
        pSlice->remove_after_space(pMeter);
    }


    //General rule: transfer space to previous slice if it is note rest
    //Gourlay's spacing algorithm is only for determining the position for timed
    //objects. As non-timed objects are not part of the model, its space will be
    //added *after* the space assigned to the previous note. Therefore, as non-timed
    //slices must not introduce unnecessary extra space, the space needed by non-timed
    //must be transferred to previous noterest slice as right rod so that spacing
    //algorithms substract it from the space it	is going to add after the note.
    if (!m_prev || (m_prev->get_type() != TimeSlice::k_noterest
                    && m_prev->get_type() != TimeSlice::k_graces) )
    {
        //[NT4c] barlines and prolog: space accounted as fixed space at start
        //  test 00608. CHECK: (draw anchor objects) spacer width is the separation
        //                  between barline and 1st note after it, no less.
        //  test 00602. CHECK: (draw anchor objects) spacer width is the separation
        //                  between TS and 1st note, no less.
        ;  //do not transfer
    }
    else    //prev is noterest
    {
        //Space for non-timed should not be added as a full rod
        //in previous slice when there are no objects for the staves in which these
        //non-timed are placed. Instead, transfer only the minimum required for
        //ensuring that previous timed object rods (left+right) is at least equal to the
        //required space for this slice.
        bool fNoOverlap = true;
        ColStaffObjsEntry* pEntry = m_prev->m_firstEntry;
        int iMax = m_prev->m_numEntries;
        for (int i=0; i < iMax; ++i, pEntry = pEntry->get_next())
        {
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            fNoOverlap &= m_widths[iStaff] == 0.0f;
        }

        if (fNoOverlap)
        {
            //transfer only the minimum required, for not introducing extra space in
            //lines not affected
            //[NT4b] test 00615. CHECK: intermediate G clef does not add extra space in
            //                          second staff
            m_prev->merge_with_dxR(m_dxLeft - m_interSpace - m_prev->get_left_rod());
            m_dxLeft = 0.0f;
        }
        else
        {
            //[NT4a]- default case: transfer all space to previous slice
            //  test 00600. CHECK: intermediate clef occupy just the minimum necessary.
            //  test 00612. CHECK: intermediate clefs occupy just the minimum necessary.
            m_prev->merge_with_dxR(m_dxLeft);
            m_dxLeft = 0.0f;
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNonTimed::move_shapes_to_final_positions(vector<ShapeData*>& shapes,
                                                       LUnits xPos, LUnits yPos,
                                                       LUnits* yMin, LUnits* yMax,
                                                       ScoreMeter* pMeter,
                                                       VerticalProfile* pVProfile)
{
    //vector for current position on each staff
    vector<LUnits> positions;
    positions.assign(m_numStaves, 0.0f);
    for (int i=0; i < m_numStaves; ++i)
        positions[i] = xPos - m_widths[i] + m_dxLeft;

    //loop for positioning shapes. They must be right aligned
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        ShapeData* pData = shapes[i];
        GmoShape* pShape = pData->get_shape();
        if (pShape)
        {
            //compute left position for this object
            //int iStaff = pEntry->staff();
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            LUnits xLeft = positions[iStaff];

            //move shape
            pShape->set_origin_and_notify_observers(xLeft, yPos);

            //save info for vertical profile
            pVProfile->update(pShape, pData->m_idxStaff);

            positions[iStaff] += pShape->get_width();
            if (!pShape->is_shape_invisible())
                positions[iStaff] += m_interSpace;

            //update system vertical limits
            *yMax = max(*yMax, pShape->get_bottom());
            *yMin = min(*yMin, pShape->get_top());
        }
    }
}


//=====================================================================================
//TimeSliceGraces implementation
//- This slice contains one or more grace notes at the same timepos.
//=====================================================================================
TimeSliceGraces::TimeSliceGraces(ColStaffObjsEntry* pEntry, int column, int iShape)
    : TimeSlice(pEntry, k_graces, column, iShape)
{
}

//---------------------------------------------------------------------------------------
TimeSliceGraces::~TimeSliceGraces()
{
}

//---------------------------------------------------------------------------------------
void TimeSliceGraces::assign_spacing_values(vector<ShapeData*>& shapes,
                                            ScoreMeter* pMeter,
                                            TextMeter& UNUSED(textMeter))
{
    //For the spacing algorithm, grace notes behave as non-timed objects. All grace
    //notes at the same timepos having the same align time are included in a single slice
	//dxL will be shapes + extra space for separation
	//dxR will be 0
	//dxLeft is accidentals before the shape, to be transferred to previous slice.


	//assign fixed space at start of this slice and compute rods

    vector<LUnits> xAcc;        //space required by accidentals, by staff
    xAcc.assign(pMeter->num_staves(), 0.0f);
    LUnits xPrev = 0.0f;        //space required by accidentals, merged
    m_dxL = 0.0f;
    m_dxR = 0.0f;

    //if this is the first slice (scores without prolog) add some space at start
    //TODO: LDP score for test. [NR2a] 00609. CHECK: there is some space at start of score before the note.
    //      Spacing must be similar to that of first note in second measure
    if (m_iFirstShape == 0)
        m_dxLeft = max(m_dxLeft, pMeter->tenths_to_logical_max(LOMSE_SPACE_BEFORE_PROLOG));

    //if prev slice is a barline slice, add some extra space at start
    if (m_prev && m_prev->get_type() == TimeSlice::k_barline)
    {
        //[GR2b] 00628: enough space between barline and next note accidentals
        m_dxLeft = max(m_dxLeft, pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_BARLINE));
    }
    else if (m_prev && m_prev->get_type() == TimeSlice::k_non_timed
             && m_prev->get_width() == 0.0f)
    {
        //Add some extra space when null width non-timed previous to barline
        //TODO: LDP score for test. [NR2c] test 00611. CHECK: measures 2 and 3 have identical spacings.
        TimeSlice* prev = m_prev->m_prev;
        if (prev && prev->get_type() == TimeSlice::k_barline)
            m_dxLeft = max(m_dxLeft, pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_BARLINE));
    }

    //loop for computing rods. Data is saved by staves
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = shapes[i]->get_shape();
        if (pShape)
        {
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            LUnits xAnchor = pShape->get_anchor_offset();
            m_dxL = max(m_dxL, pShape->get_width() + xAnchor);

            if (xAnchor < 0.0f)
            {
                xAcc[iStaff] = min(xAcc[iStaff], xAnchor);
                xPrev = min(xPrev, xAnchor);
            }
        }
    }

    //add some minimal space after this slice for ensuring some minimal separation
    //between notes
    //[GR1] 00630: some space between a grace and the next one
    m_dxL += pMeter->tenths_to_logical_max(LOMSE_EXCEPTIONAL_MIN_SPACE);

    join_with_previous(pMeter, xAcc, xPrev);
}

//---------------------------------------------------------------------------------------
void TimeSliceGraces::join_with_previous(ScoreMeter* pMeter, vector<LUnits>& xAcc,
                                         LUnits xPrev)
{
    //rules for joining slices (transfer space to previous)
    //As this was copied from TimeSliceNoterest there are many cases not applicable,
    //e.g. when lyrics. This has to be reviewed


    //transfer space at start (accidentals)
    if (m_prev && (m_prev->get_type() == TimeSlice::k_noterest
                   || m_prev->get_type() == TimeSlice::k_graces) )
    {
        m_prev->merge_with_dxR(-xPrev);     //merge accidentals
    }
    else if (m_prev && m_prev->get_type() == TimeSlice::k_non_timed)
    {
        TimeSlice* pPrevPrev = m_prev->m_prev;
        if (pPrevPrev && pPrevPrev->get_type() == TimeSlice::k_noterest)
        {
            //Space for accidentals can be transferred only by staves
            TimeSliceNonTimed* pNonTimed = static_cast<TimeSliceNonTimed*>(m_prev);
            LUnits xTransfer = 0.0f;

            ColStaffObjsEntry* pEntry = m_firstEntry;
            int iMax = m_numEntries;
            for (int i=0; i < iMax; ++i, pEntry = pEntry->get_next())
            {
                int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
                if (xAcc[iStaff] < 0.0f && pNonTimed->is_empty_staff(iStaff))
                {
                    //transfer space for accidentals in this staff
                    xTransfer = min(xTransfer, xAcc[iStaff]);   //AWARE xAcc is negative
                    xAcc[iStaff] = 0.0f;
                }
            }

            if (xTransfer < 0.0f)
            {
                //[NR4b] test 00622. CHECK: accidental in second staff do not interfere
                //                      in clef change spacing.
				// test 00623. CHECK: lyrics in first note do not interfere in clef
				//                      change spacing.
		        // test 00624. CHECK: lyrics and accidentals do not interfere in clef spacing.
                pPrevPrev->merge_with_dxR(-xTransfer);     //dxR is merged. Merge accidentals

                //re-compute remaining xPrev and account it as fixed space in this slice
                xPrev = 0.0f;
                ColStaffObjsEntry* pEntry = m_firstEntry;
                int iMax = m_numEntries;
                for (int i=0; i < iMax; ++i, pEntry = pEntry->get_next())
                {
                    int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
                    xPrev = min(xPrev, xAcc[iStaff]);
                }
                m_dxLeft -= xPrev;   //AWARE: xPrev is always negative
            }
            else
            {
                //do not transfer space for accidental (lyrics already transferred).
                //[NR4c] test 00605. CHECK:enough space for accidental and red line
                //                      before it
                m_dxLeft -= xPrev;   //AWARE: xPrev is always negative
            }
        }
        else if (pPrevPrev)
        {
            //account accidentals as fixed space in this noterest
            m_dxLeft -= xPrev;   //AWARE: xPrev is always negative
        }
        else
        {
            //pPrevPrev does not exist. Do not transfer space
            m_dxLeft -= xPrev;   //AWARE: xPrev is always negative
        }
    }
    else if (m_prev && (m_prev->get_type() == TimeSlice::k_barline))
    {
        //prev is barline or prolog. Accidentals space can never be transferred.

        //do not transfer accidentals:
        //[GR5a] 00628: accidentals don't moved before the barline


        //[NR5b] test 00606. CHECK: equal space between prolog and noteheads in all staves.
        //                  Accidentals do not alter noteheads alignment.
        m_dxLeft -= xPrev;   //AWARE: xPrev is always negative
    }
}


//=====================================================================================
//TimeSliceBarline implementation
//=====================================================================================
TimeSliceBarline::TimeSliceBarline(ColStaffObjsEntry* pEntry, int column, int iShape)
    : TimeSlice(pEntry, k_barline, column, iShape)
{
}

//---------------------------------------------------------------------------------------
TimeSliceBarline::~TimeSliceBarline()
{
}

//---------------------------------------------------------------------------------------
void TimeSliceBarline::assign_spacing_values(vector<ShapeData*>& shapes,
                                            ScoreMeter* pMeter,
                                            TextMeter& UNUSED(textMeter))
{
	//assign fixed space at start of this slice and compute pre-stretching
	//extend (left and right rods)

	//m_dxLeft = 0. Will always be transferred to previous
    m_dxL = 0.0f;   //barline shape
    m_dxR = 0.0f;   //=0

    //assign some space before the barline for separation from very small notes
    //and directions
    //[BL2] test 00619. CHECK: some space between both barlines. It is this minimum space
    m_dxLeft = pMeter->tenths_to_logical_max(LOMSE_EXCEPTIONAL_MIN_SPACE
                                            + LOMSE_MIN_SPACE_BEFORE_BARLINE);

    //loop for computing rods
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i)
    {
        GmoShape* pShape = shapes[i]->get_shape();
        if (pShape)
        {
            LUnits xAnchor = pShape->get_anchor_offset();
            m_dxL = max(m_dxL, pShape->get_width() + xAnchor);
        }
    }

    join_with_previous();
}

//---------------------------------------------------------------------------------------
void TimeSliceBarline::join_with_previous()
{
    //rules for joining slices (transfer space to previous)

    //Fixed space at start (space before the barline) must be transferred as right rod
    //to previous noterest slice so that spacing algorithm substract it from the space
    //it is going to add after the note. By doing this, we avoid to add extra space to
    //normal notes spacing.
    if (m_prev && (m_prev->get_type() == TimeSlice::k_noterest
                   || m_prev->get_type() == TimeSlice::k_graces) )
    {
        //[BL4] test 00604. CHECK: all notes quasi-equally spaced. Barline affects very little.
        m_prev->merge_with_dxR(m_dxLeft);    //dxR is only lyrics. Merge
        m_dxLeft = 0.0f;
    }

    //if previous slice is non-timed and is visible, transfer the space to the
    //previous note so that non-timed is just before the barline (e.g., an intermediate
    //clef). But if non-timed i, suppress barline previous space.
    else if (m_prev && m_prev->get_type() == TimeSlice::k_non_timed)
    {
        if (!static_cast<TimeSliceNonTimed*>(m_prev)->has_width())
        {
            //transfer the space
            //[BL5a] test 00616. CHECK: intermediate clef: more space before the note than
            //                      before the barline
            TimeSlice* prev = m_prev->m_prev;
            if (prev && (prev->get_type() == TimeSlice::k_noterest
                         || m_prev->get_type() == TimeSlice::k_graces) )
            {
                prev->merge_with_dxR(m_dxLeft);    //dxR is already merged. Merge
                m_dxLeft = 0.0f;
            }
        }
        else
        {
            //suppress space
            //[BL5b] test 00620. CHECK: no space between first note and line
            m_dxLeft = 0.0f;
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceBarline::move_shapes_to_final_positions(vector<ShapeData*>& shapes, LUnits xPos,
                                               LUnits yPos, LUnits* yMin, LUnits* yMax,
                                               ScoreMeter* UNUSED(pMeter),
                                               VerticalProfile* pVProfile)
{
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i)
    {
        ShapeData* pData = shapes[i];
        GmoShape* pShape = pData->get_shape();
        if (pShape)
        {
            //move shape
            LUnits xLeft = xPos + m_width - pShape->get_width();
            pShape->set_origin_and_notify_observers(xLeft, yPos);

            //save info for vertical profile
            pVProfile->update(pShape, pData->m_idxStaff);

            //update system vertical limits
            *yMax = max(*yMax, pShape->get_bottom());
            *yMin = min(*yMin, pShape->get_top());
        }
    }
}


//=====================================================================================
//TimeSliceNoterest implementation
//=====================================================================================
TimeSliceNoterest::TimeSliceNoterest(ColStaffObjsEntry* pEntry, int column, int iShape,
                                     int numLines, int numStaves)
    : TimeSlice(pEntry, k_noterest, column, iShape)
    , m_dxRLyrics(0.0f)
    , m_dxRMerged(0.0f)
    , m_neighborhood(SeqData::k_seq_isolated)
{
    m_lines.assign(numLines, nullptr);
    m_rods.assign(numStaves, nullptr);
}

//---------------------------------------------------------------------------------------
TimeSliceNoterest::~TimeSliceNoterest()
{
    for (auto line : m_lines)
        delete line;

    for (auto rod : m_rods)
        delete rod;
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::assign_spacing_values(vector<ShapeData*>& shapes,
                                              ScoreMeter* pMeter,
                                              TextMeter& textMeter)
{
	//assign fixed space at start of this slice and compute pre-stretching
	//extend (left and right rods)

    //dxLeft = not transferred accidentals
    //dxL = notehead width
    //dxR = additional requiered space included in the noteshape (flag, dots)
    //dxRLyrics = after space required by lyrics = half lyrics - half notehead
    //dxRMerged = transferred space from next slices, to be merged with (dxRLyrics + dxR)
    //xPrev = acc. to be transferred to previous or left in dxLeft

    //check and fix possible conflicts between noterests in this slice
    fix_spacing_issues(shapes, pMeter);

    //xPrev is the max. space required by accidentals (positive)
    LUnits xPrev = compute_rods(shapes, pMeter);


    //if this is the first slice (scores without prolog) add some space at start
    //[NR2a] test 00609. CHECK: there is some space at start of score before the note.
    //      Spacing must be similar to that of first note in second measure
    if (m_iFirstShape == 0)
        m_dxLeft += pMeter->tenths_to_logical_max(LOMSE_SPACE_BEFORE_PROLOG);

    //if prev slice is a barline slice, add some extra space at start
    if (m_prev && m_prev->get_type() == TimeSlice::k_barline)
    {
        //[NR2b]
        //  test 00615. CHECK: enough space after barline and 1st note in 2nd measure
        m_dxLeft = max(m_dxLeft, pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_BARLINE));
    }
    else if (m_prev && m_prev->get_type() == TimeSlice::k_non_timed
             && m_prev->get_width() == 0.0f)
    {
        //Add some extra space when null width non-timed previous to barline
        //[NR2c] test 00611. CHECK: measures 2 and 3 have identical spacings.
        TimeSlice* prev = m_prev->m_prev;
        if (prev && prev->get_type() == TimeSlice::k_barline)
            m_dxLeft = max(m_dxLeft, pMeter->tenths_to_logical_max(LOMSE_SPACE_AFTER_BARLINE));
    }

    //take lyrics into account
    LUnits xLyrics = 0.0f;      //space required by lyrics
    vector< pair<ImoLyric*, int> >::iterator it;
    for (it=m_lyrics.begin(); it != m_lyrics.end(); ++it)
    {
        LUnits width = (measure_lyric((*it).first, pMeter, textMeter) - m_dxL) / 2.0f;
        //lyric is centered: half as right rod and half as prev space.
        xLyrics = max(xLyrics, width);
        //TODO: the split must not include the hyphenation
    }
    if (xLyrics > 0.0f)
    {
        //set half of lyrics width as lyrics rod
        increment_dxR(xLyrics);
        //the other half, before the note, is going to be transferred. Discount fixed space
        xLyrics = max(0.0f, xLyrics - m_dxLeft);
    }

    join_with_previous(pMeter, xLyrics, xPrev);
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::join_with_previous(ScoreMeter* pMeter, LUnits xLyrics,
                                           LUnits xPrev)
{
    //rules for joining slices (transfer space to previous)

    LUnits k_min_space = pMeter->tenths_to_logical_max(LOMSE_EXCEPTIONAL_MIN_SPACE);

    //transfer space at start (accidentals and lyrics)
    if (m_prev && (m_prev->get_type() == TimeSlice::k_noterest))
    {
        //transfer all space to previous noterest slice
        TimeSliceNoterest* pPrev = static_cast<TimeSliceNoterest*>(m_prev);
        pPrev->merge_rods_with(m_rods, k_min_space);     //merge accidentals with previous

        if (xLyrics > 0.0f)
            m_prev->increment_dxR(xLyrics + k_min_space);   //increment lyrics
    }
    else if (m_prev && (m_prev->get_type() == TimeSlice::k_graces))
    {
        //transfer all space to previous noterest slice
        //[NR3] test 00614. CHECK: enough space for accidental in 2nd note
        if (xLyrics > 0.0f)
            m_prev->increment_dxR(xLyrics + k_min_space);   //increment lyrics

        m_prev->merge_with_dxR(xPrev);     //merge accidentals
    }
    else if (m_prev && m_prev->get_type() == TimeSlice::k_non_timed)
    {
        TimeSlice* pPrevPrev = m_prev->m_prev;
        if (pPrevPrev && pPrevPrev->get_type() == TimeSlice::k_noterest)
        {
            //space for lyrics can always be transferred
			//[NR4a] test 00625. CHECK: lyrics moved before spacer and clef
            if (xLyrics > 0.0f)
                pPrevPrev->increment_dxR(xLyrics + k_min_space);

            //Space for accidentals can be transferred only by staves
            TimeSliceNonTimed* pNonTimed = static_cast<TimeSliceNonTimed*>(m_prev);
            LUnits xTransfer = 0.0f;

            ColStaffObjsEntry* pEntry = m_firstEntry;
            vector<LUnits> dxAcc;
            dxAcc.assign(pMeter->num_staves(), 0.0f);
            int iMax = m_numEntries;
            for (int i=0; i < iMax; ++i, pEntry = pEntry->get_next())
            {
                int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
                dxAcc[iStaff] = m_rods[iStaff]->m_dxB;
                if (m_rods[iStaff]->m_dxB > 0.0f && pNonTimed->is_empty_staff(iStaff))
                {
                    //transfer space for accidentals in this staff
                    xTransfer = max(xTransfer, m_rods[iStaff]->m_dxB);
                    dxAcc[iStaff] = 0.0f;
                }
            }

            if (xTransfer > 0.0f)
            {
                //[NR4b] test 00622. CHECK: accidental in second staff do not interfere
                //                      in clef change spacing.
				// test 00623. CHECK: lyrics in first note do not interfere in clef
				//                      change spacing.
		        // test 00624. CHECK: lyrics and accidentals do not interfere in clef spacing.
                pPrevPrev->merge_with_dxR(xTransfer);     //dxR is merged. Merge accidentals

                //re-compute remaining xPrev and account it as fixed space in this slice
                xPrev = 0.0f;
                ColStaffObjsEntry* pEntry = m_firstEntry;
                int iMax = m_numEntries;
                for (int i=0; i < iMax; ++i, pEntry = pEntry->get_next())
                {
                    int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
                    xPrev = max(xPrev, dxAcc[iStaff]);
                }
                m_dxLeft += xPrev;
            }
            else
            {
                //do not transfer space for accidental (lyrics already transferred).
                //[NR4c] test 00605. CHECK:enough space for accidental and red line
                //                      before it
                m_dxLeft += xPrev;
            }
        }
        else if (pPrevPrev)
        {
            //Space can never be transferred to barlines because if justification places
            //a system break after the barline, it will not be right justified.
            //[NR4d] test ???. CHECK: first system is justified
            if (pPrevPrev->get_type() != TimeSlice::k_barline)
            {
                //Space for lyrics can be transferred when it is prolog
                //[NR4e] test ???? //TODO
                if (xLyrics > 0.0f)
                {
                    LUnits prevRi = pPrevPrev->get_right_rod();
                    LUnits discount = (prevRi == 0.0f ? pPrevPrev->get_left_rod() : 0.0f);
                    xLyrics = max(0.0f, xLyrics-discount);
                    pPrevPrev->increment_dxR(xLyrics + k_min_space);
                }
            }
            //account accidentals as fixed space in this noterest
            m_dxLeft += xPrev;
        }
        else
        {
            //pPrevPrev does not exist. Do not transfer space
            xPrev = max(xPrev, xLyrics);
            m_dxLeft += xPrev;
        }
    }
    else if (m_prev)
    {
        //prev is barline or prolog. Accidentals space can never be transferred.
        //Lyrics space can be transferred if not barline

        //Lyrics can never be transferred to barlines because if justification places a
        //system break after the barline, it will not be right justified.
        //[NR5a] test 00610. CHECK: enough space for accidental after barline
        //       test 00611. CHECK: enough space for accidental after barline in last measure
        //       test 50021. CHECK: first system is justified
        if (m_prev->get_type() != TimeSlice::k_barline)
        {
            //transfer lyrics:
            //[] test 13b-KeySignatures-ChurchModes
            if (xLyrics > 0.0f)
            {
                LUnits prevRi = m_prev->get_right_rod();
                LUnits discount = (prevRi == 0.0f ? m_prev->get_left_rod() : 0.0f);
                xLyrics = max(0.0f, xLyrics-discount);
                m_prev->increment_dxR(xLyrics + k_min_space);
            }
        }

        //do not transfer accidentals:
        //[NR5a] test 00610. CHECK: enough space between barline and next note accidentals
        //       test 00611. CHECK: enough space for accidental after barline in last measure
        //[NR5b] test 00606. CHECK: equal space between prolog and noteheads in all staves.
        //                  Accidentals do not alter noteheads alignment.
        m_dxLeft += xPrev;
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::merge_rods_with(std::vector<RodsData*>& nextRods,
                                        LUnits minSpace)
{
    //minSpace is a minimum space to add for separation between this note shape and the
    //transferred accidentals from next note or, when no accidentals, for separation
    //with the notehead in another line in the same staff

    m_dxR = 0.0f;
    size_t i = 0;
    bool fCollide = false;
    for (auto rod : m_rods)
    {
        if (rod && nextRods[i])
        {
            fCollide = true;
            LUnits dx = nextRods[i]->m_dxB > 0.0f ? nextRods[i]->m_dxB : 0.0f;
            dx += minSpace;
            m_dxR = max(m_dxR, rod->m_dxA + dx);
        }
        ++i;
    }

    //remove any width due to notehead if no collision with next noterest slice
    if (!fCollide && (m_next->get_type() == TimeSlice::k_noterest))
        m_dxL = 0.0f;
}

//---------------------------------------------------------------------------------------
LUnits TimeSliceNoterest::compute_rods(vector<ShapeData*>& shapes, ScoreMeter* pMeter)
{
    //Loop for computing initial rods. Space required by accidentals is saved by staves
    //in vector m_rods. Returns the max space required by accidentals (positive)

    m_dxLeft = 0.0;
    m_dxL = 0.0f;
    m_dxR = 0.0f;
    m_dxRLyrics = 0.0f;
    m_dxRMerged = 0.0f;
    LUnits xPrev = 0.0f;    //max.space required by accidentals

//    LUnits k_min_space = pMeter->tenths_to_logical_max(LOMSE_EXCEPTIONAL_MIN_SPACE);


    //loop for computing rods. Data is saved by staves in vector m_rods
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = shapes[i]->get_shape();
        if (pShape)
        {
            LUnits dxB = 0.0f;
            LUnits dxNH = 0.0f;
            LUnits dxA = 0.0f;
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            if (pShape->is_shape_note())
            {
                GmoShapeNote* pSN = static_cast<GmoShapeNote*>(pShape);
                dxB = -pSN->get_anchor_offset();
                dxNH = pSN->get_notehead_width();
                dxA = pSN->get_width() - dxB - dxNH;
            }
            else
            {
                dxB = -pShape->get_anchor_offset();
                dxNH = pShape->get_width();
                dxA = 0.0f;
            }

            xPrev = max(xPrev, dxB);
            m_dxL = max(m_dxL, dxNH);
            m_dxR = max(m_dxR, dxA);

            if (m_rods[iStaff])
                m_rods[iStaff]->update_rods(dxB, dxNH, dxA);
            else
                m_rods[iStaff] = LOMSE_NEW RodsData(dxB, dxNH, dxA);
        }
    }
    return xPrev;
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::fix_spacing_issues(vector<ShapeData*>& shapes, ScoreMeter* pMeter)
{
    vector<NoterestsCollisionsFixer*> fixers;
    fixers.assign(pMeter->num_staves(), nullptr);

    //loop for creating fixers and collecting shapes
    ColStaffObjsEntry* pEntry = m_firstEntry;
    int iMax = m_iFirstShape + m_numEntries;
    for (int i=m_iFirstShape; i < iMax; ++i, pEntry = pEntry->get_next())
    {
        GmoShape* pShape = shapes[i]->get_shape();
        if (pShape)
        {
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());

            if (fixers[iStaff])
                fixers[iStaff]->add_noterest(pShape, pEntry);
            else
                fixers[iStaff] = LOMSE_NEW NoterestsCollisionsFixer(pShape, pEntry, pMeter);
        }
    }

    //fix issues and delete fixers
    for (auto fixer : fixers)
    {
        if (fixer)
        {
            fixer->fix_spacing_issues();
            delete fixer;
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits TimeSliceNoterest::measure_lyric(ImoLyric* pLyric, ScoreMeter* pMeter,
                                        TextMeter& textMeter)
{
    //TODO: Move this method to another object. TimeSliceNoterest shoul not have
    //knowledge about the lyrics GM structure. Move perhaps to engraver?

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
        // coverity[var_deref_model]
        totalWidth += measure_text("-", pStyle, "en", textMeter)
                      + pMeter->tenths_to_logical_max(10.0);
    }

    return totalWidth;
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::add_lyrics(ScoreMeter* pMeter)
{
    ColStaffObjsEntry* pEntry = m_firstEntry;
    for (int i=0; i < m_numEntries; ++i, pEntry = pEntry->get_next())
    {
        ImoStaffObj* pSO = pEntry->imo_object();

        if (pSO->is_note() && pSO->get_num_attachments() > 0)
        {
            int iStaff = pMeter->staff_index(pEntry->num_instrument(), pEntry->staff());
            ImoAttachments* pAuxObjs = pSO->get_attachments();
            int size = pAuxObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
                if (pAO->is_lyric())
                {
                    ImoLyric* pLyric = static_cast<ImoLyric*>(pAO);
                    m_lyrics.push_back( make_pair(pLyric, iStaff) );
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::increment_dxR(LUnits value)
{
    m_dxRLyrics += value;
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::merge_with_dxR(LUnits value)
{
    m_dxRMerged = max(m_dxRMerged, value);
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::set_minimum_space(LUnits value)
{
    if (get_total_rods() < value)
    {
        m_dxRLyrics = value;
        m_dxR = max(m_dxRLyrics - m_dxL, m_dxRMerged);
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::dump_lines(std::ostream& ss)
{
    int i = 0;
    for (auto line : m_lines)
    {
        if (line)
            line->dump(i, ss);
        ++i;
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::dump_rods(std::ostream& ss)
{
    int i = 0;
    for (auto rod : m_rods)
    {
        if (rod)
            rod->dump(i, ss);
        ++i;
    }
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::dump_neighborhood(std::ostream& ss)
{
    ss << "   " << m_neighborhood;
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::save_seq_data(ColStaffObjsEntry* pEntry, int sequence)
{
    int iLine = pEntry->line();
    if (m_lines[iLine])
    {
        //note in a chord. it does not affect sequence. ignore it
    }
    else
        m_lines[iLine] = LOMSE_NEW SeqData(pEntry, sequence);
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::update_sequence(int iLine, int sequence)
{
    m_lines[iLine]->update_sequence(sequence);
}

//---------------------------------------------------------------------------------------
void TimeSliceNoterest::finish_sequences(int iLine)
{
    if (m_lines[iLine]->get_sequence() == SeqData::k_seq_continue)
    {
        SeqData* pPrevRod = find_prev_rodsdata_for_line(iLine);
        if (pPrevRod && pPrevRod->get_sequence() == SeqData::k_seq_start)
        {
            pPrevRod->update_sequence(SeqData::k_seq_isolated);
            m_lines[iLine]->update_sequence(SeqData::k_seq_isolated);
        }
        else if (pPrevRod && pPrevRod->get_sequence() == SeqData::k_seq_end_start)
        {
            pPrevRod->update_sequence(SeqData::k_seq_end);
            m_lines[iLine]->update_sequence(SeqData::k_seq_isolated);
        }
        else
            m_lines[iLine]->update_sequence(SeqData::k_seq_end);
    }
}

//---------------------------------------------------------------------------------------
SeqData* TimeSliceNoterest::find_prev_rodsdata_for_line(int iLine)
{
    SeqData* pPrevRod = nullptr;
    TimeSlice* pSlice = m_prev;
    while (pSlice)
    {
        if (pSlice->get_type() == k_noterest)
        {
            pPrevRod = static_cast<TimeSliceNoterest*>(pSlice)->get_rodsdata(iLine);
            if (pPrevRod)
                break;
        }
        pSlice = pSlice->m_prev;
    }
    return pPrevRod;
}

//---------------------------------------------------------------------------------------
TimeUnits TimeSliceNoterest::get_duration(int iLine)
{
    return m_lines[iLine]->get_duration();
}

//---------------------------------------------------------------------------------------
int TimeSliceNoterest::get_sequence(int iLine)
{
    return m_lines[iLine]->get_sequence();
}

//---------------------------------------------------------------------------------------
int TimeSliceNoterest::compute_neighborhood(int prevNeighborhood, int numOpenSeqs)
{
    //A neighborhood starts when a sequence starts and no neighborhood is open.
    //A neighborhood finishes when no already open sequence continues from that point.

    int numEndStart = 0;
    for (size_t i=0; i < m_lines.size(); ++i)
    {
        if (m_lines[i])
        {
            if (m_lines[i]->get_sequence() == SeqData::k_seq_start)
                ++numOpenSeqs;
            else if (m_lines[i]->get_sequence() == SeqData::k_seq_end_start)
                ++numEndStart;
            else if (m_lines[i]->get_sequence() == SeqData::k_seq_end)
                --numOpenSeqs;
        }
    }

    bool fError = false;
    if (numEndStart > 0 && numOpenSeqs - numEndStart == 0)  //end_start
    {
        //as numEndStart is always <= numOpenSeqs this case can only be the end of
        //previous and start a new one: k_seq_end_start
        //a new one
        //e.g.         (......|...)   ...(...).........
        //                            (......|........)
        //numOpenSeqs  11111111       11122221
        //numEndStart  00000001       00000001
        switch (prevNeighborhood)
        {
            case SeqData::k_seq_continue:
                m_neighborhood = SeqData::k_seq_end_start;
                break;
            case SeqData::k_seq_end_start:
                m_neighborhood = SeqData::k_seq_continue;;
                break;
            case SeqData::k_seq_isolated:
            case SeqData::k_seq_end:
                //imposible, as numOpenSeqs is > 0 there is a sequence open. Thus, prev
                //slice can neither be 'isolated' nor 'end'
                fError = true;
                break;
            case SeqData::k_seq_start:
                //imposible, as numOpenSeqs is > 0 there is a sequence open. And as
                //sequence can not contain only one note. Thus, it can not start in
                //previous slice
                fError = true;
                break;
        }
    }
    else if (numOpenSeqs - numEndStart > 0)  //start or continue
    {
        //e.g.         (...............)
        //             (......|........)
        //             (......|........)   (......|........)
        //numOpenSeqs  33333333            1
        //numEndStart  00000002            0
        switch (prevNeighborhood)
        {
            case SeqData::k_seq_isolated:
            case SeqData::k_seq_end:
                m_neighborhood = SeqData::k_seq_start;
                break;
            case SeqData::k_seq_start:
            case SeqData::k_seq_continue:
            case SeqData::k_seq_end_start:
                m_neighborhood = SeqData::k_seq_continue;
                break;
        }
    }
    else if (numOpenSeqs == 0)  //numEndStart==0 ==> end or isolated
    {
        //As numEndStart==0 this case is end or isolated
        //e.g.         (...............)
        //             (......|........)
        //             (......|........)   (......)    .....   (...)...
        //numOpenSeqs  33333333333333330   11111110    000     111100
        //numEndStart  00000002000000000   00000000    000     000000
        switch (prevNeighborhood)
        {
            case SeqData::k_seq_continue:
                m_neighborhood = SeqData::k_seq_end;
                break;
            case SeqData::k_seq_end:
            case SeqData::k_seq_isolated:
                m_neighborhood = SeqData::k_seq_isolated;
                break;
            case SeqData::k_seq_start:
            case SeqData::k_seq_end_start:
                //imposible cases, as numOpenSeqs is 0 previous can neither be start nor
                //end_start
                fError = true;
                break;
        }
    }
    else
    {
        //imposible! implies either numEndStart < 0, numOpenSeqs < 0 or
        //numOpenSeqs - numEndStart < 0
        fError = true;
    }

    if (fError)
    {
        stringstream ss;
        ss << "Impossible case: numOpenSeqs=" << numOpenSeqs
            << ", numEndStart=" << numEndStart << ", prevNeighborhood"
            << prevNeighborhood;
        LOMSE_LOG_ERROR(ss.str());
        m_neighborhood = prevNeighborhood;
    }

    return numOpenSeqs;
}



//=====================================================================================
//ColumnDataGourlay implementation
//=====================================================================================
ColumnDataGourlay::ColumnDataGourlay(TimeSlice* pSlice)
    : m_pFirstSlice(pSlice)
    , m_slope(1.0f)
    , m_minFi(0.0f)
    , m_xFixed(0.0f)
    , m_colWidth(0.0f)
    , m_colMinWidth(0.0f)
    , m_barlinesInfo(0)
    , m_xPos(0.0f)
{
}

//---------------------------------------------------------------------------------------
ColumnDataGourlay::~ColumnDataGourlay()
{
    list<FullMeasureRestData*>::iterator itR;
    for (itR = m_rests.begin(); itR != m_rests.end(); ++itR)
        delete *itR;
    m_rests.clear();
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
void ColumnDataGourlay::include_full_measure_rest(GmoShape* pShape,
                                                  ColStaffObjsEntry* pEntry,
                                                  GmoShape* pNonTimedShape)
{
    FullMeasureRestData* pRest = LOMSE_NEW FullMeasureRestData(pShape, pEntry,pNonTimedShape);
    m_rests.push_back(pRest);
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::set_num_entries(int numSlices)
{
    m_orderedSlices.assign(numSlices, (TimeSlice*)nullptr);   //GCC 4.8.4 complains if nullptr not casted
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
void ColumnDataGourlay::fix_neighborhood_spacing_problems(bool fTrace)
{
    if (fTrace)
    {
        dbgLogger << " ****************************** Before fixing neighborhood problems"
            << endl;
        dump(logger.get_stream());
        dbgLogger << endl;
    }

    int prevNeighborhood = SeqData::k_seq_isolated;
    int numOpenSeqs = 0;
    TimeUnits di_average = 0.0;
    TimeUnits di_min = 0.0;
    TimeUnits di_prev = 0.0;
    int neighborhoodLength = 0;
    bool fFindStart = true;
    bool fSpacingProblem = false;
    TimeSlice* pSlice = m_pFirstSlice;
    TimeSlice* pNbStartSlice = nullptr;
    for (int i=0; i < num_slices(); ++i)
    {
        if (pSlice->get_type() == TimeSlice::k_noterest)
        {
            TimeSliceNoterest* pSliceNR = static_cast<TimeSliceNoterest*>(pSlice);
            numOpenSeqs = pSliceNR->compute_neighborhood(prevNeighborhood, numOpenSeqs);
            prevNeighborhood = pSliceNR->get_neighborhood();
            if (fFindStart && (prevNeighborhood == SeqData::k_seq_start
                               || prevNeighborhood == SeqData::k_seq_end_start) )
            {
                //start found
                di_prev = pSlice->get_shortest_duration();
                di_average = di_prev;
                di_min = di_prev;
                neighborhoodLength = 1;
                fFindStart = false;
                fSpacingProblem = false;
                pNbStartSlice = pSlice;
            }
            else if (prevNeighborhood == SeqData::k_seq_continue)
            {
                //continue found
                TimeUnits di_cur = pSlice->get_shortest_duration();
                if (!is_equal_time(di_prev, di_cur))
                    fSpacingProblem = true;
                di_average += di_cur;
                di_min = min(di_min, di_cur);
                ++neighborhoodLength;
            }
            else if (prevNeighborhood == SeqData::k_seq_end
                     || prevNeighborhood == SeqData::k_seq_end_start)
            {
                //end found
                TimeUnits di_cur = pSlice->get_shortest_duration();
                if (!is_equal_time(di_prev, di_cur))
                    fSpacingProblem = true;
                di_average += di_cur;
                di_min = min(di_min, di_cur);
                ++neighborhoodLength;

                if (fSpacingProblem)
                {
                    //Fix spacing problem
                    di_average /= neighborhoodLength;
                    if (fTrace)
                    {
                        dbgLogger << "Fixing neighborhood spacing problem. di_average="
                           << fixed << setprecision(8) << setfill(' ')
                           << di_average << endl;
                    }
                    while (pNbStartSlice != pSlice)
                    {
                        pNbStartSlice->set_shortest_duration(di_average);
                        pNbStartSlice = pNbStartSlice->m_next;
                    }
                    pNbStartSlice->set_shortest_duration(di_average);
                }

                if (prevNeighborhood == SeqData::k_seq_end_start)
                {
                    //start new sequence
                    di_prev = pSlice->get_shortest_duration();
                    di_average = di_prev;
                    di_min = di_prev;
                    neighborhoodLength = 1;
                    fFindStart = false;
                    fSpacingProblem = false;
                    pNbStartSlice = pSlice;
                }
            }
        }
        pSlice = pSlice->next();
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
                                          vector<ShapeData*>& shapes)
{
    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        pSlice->add_shapes_to_box(pSliceInstrBox, iInstr, shapes);
        pSlice = pSlice->next();
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::delete_shapes(vector<ShapeData*>& shapes)
{
    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        pSlice->delete_shapes(shapes);
        pSlice = pSlice->next();
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::move_shapes_to_final_positions(vector<ShapeData*>& shapes,
                                                       LUnits xPos, LUnits yPos,
                                                       LUnits* yMin, LUnits* yMax,
                                                       ScoreMeter* pMeter,
                                                       VerticalProfile* pVProfile)
{
    m_xPos = xPos;

    TimeSlice* pSlice = m_pFirstSlice;
    for (int i=0; i < num_slices(); ++i)
    {
        pSlice->move_shapes_to_final_positions(shapes, xPos, yPos, yMin, yMax, pMeter,
                                               pVProfile);
        xPos += pSlice->get_width();
        pSlice = pSlice->next();
    }
}

//---------------------------------------------------------------------------------------
void ColumnDataGourlay::reposition_full_measure_rests(GmoBoxSystem* pBox,
                                                      GmMeasuresTable* pMeasures)
{
    list<FullMeasureRestData*>::iterator it;
    for (it = m_rests.begin(); it != m_rests.end(); ++it)
    {
        ColStaffObjsEntry* pEntry = (*it)->get_rest_entry();
        int iInstr = pEntry->num_instrument();
        int iMeasure = pEntry->measure();

        //determine space for centering the rest
        //discount the width of any non-timed after start measure barline
        GmoShape* pShapeNT = (*it)->get_non_timed_shape();
        LUnits xStart = (pShapeNT ? pShapeNT->get_right()
                                  : pMeasures->get_start_barline_right(iInstr, iMeasure, pBox));

        GmoShape* pRestShape = (*it)->get_rest_shape();
        LUnits xEnd = pMeasures->get_end_barline_left(iInstr, iMeasure, pBox);
        LUnits space = xEnd - xStart - pRestShape->get_width();

        //determine new xPos for the rest and move there
        LUnits xNew = space/2.0f + xStart;
        pRestShape->set_origin_and_notify_observers(xNew, 0.0);
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
        extent += pSlice->get_total_rods() + pSlice->m_dxLeft;
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
        s += 1.0f / pSlice->m_c;
        extent += pSlice->m_dxLeft;
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
        xmin += (*it)->get_total_rods();
        xFixed += (*it)->m_dxLeft;
    }

    //if the required extent is lower than the minimum extent the required force is 0
    x -= xFixed;
    if (x <= xmin)
        return 0.0f;

    //compute combined spring constant for all springs that will react to F
    it = m_orderedSlices.begin();
    TimeSlice* pLast = m_orderedSlices.back();
    float F;
    float slope = 1.0f / (*it)->m_c;
    while (true)
    {
        //calculate force required by current spring
        xmin -= (*it)->get_total_rods();
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
        slope += 1.0f / (*it)->m_c;
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
        m_xFixed += (*it)->m_dxLeft;
        if (m_minFi > (*it)->m_fi)
        {
            m_minFi = (*it)->m_fi;
            cFiMin = (*it)->m_c;
        };

        //if the force of this spring is bigger than F do not take this spring into
        //account, only its pre-stretching extent
        if (F <= (*it)->m_fi)
            m_xFixed += (*it)->get_total_rods();
        else
        {
            //Add this spring to the combined spring
            //  c = 1 / ( (1/c) + (1/ci) ), but s = 1/c       ==>
            //  s = s + (1/ci)
            m_slope += 1.0f / (*it)->m_c;
        }

//        dbgLogger << "    Slice: type="<< (*it)->get_type()
//                  << ", 1st data=" << (*it)->dbg_get_first_data()
//                  << ", width=" << (*it)->get_width()
//                  << ", m_fi= " << (*it)->m_fi
//                  << ", m_c=" << (*it)->m_c
//                  << ", xi=" << (*it)->get_total_rods()
//                  << ", m_xFixed=" << m_xFixed
//                  << ", m_dxLeft=" << (*it)->m_dxLeft
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
        LUnits x = xPos + pSlice->get_left_space();
        TimeGridTableEntry entry = { rCurTime, rDuration, x };
        table->add_entry(entry);

        xPos += pSlice->get_width();
        pSlice = pSlice->next();
    }

    return table;
}



//=====================================================================================
//RodsData implementation
//=====================================================================================
void RodsData::update_rods(LUnits dxB, LUnits dxNH, LUnits dxA)
{
    m_dxB = max(m_dxB, dxB);
    m_dxNH = max(m_dxNH, dxNH);
    m_dxA = max(m_dxA, dxA);
}

//---------------------------------------------------------------------------------------
void RodsData::dump(int iStaff, std::ostream& ss)
{
    ss << "                            " << setw(5) << iStaff << fixed << setprecision(2)
        << setw(9) << m_dxB << setw(9) << m_dxNH << setw(9) << m_dxA << endl;
}



//=====================================================================================
//SeqData implementation
//=====================================================================================
SeqData::SeqData(ColStaffObjsEntry* pEntry, int sequence)
    : m_pEntry(pEntry)
    , m_sequence(sequence)
{
}

//---------------------------------------------------------------------------------------
void SeqData::dump(int iLine, std::ostream& ss)
{
    ss << "                            " << setw(5) << iLine
       << setw(5) << m_sequence << endl;
}

//---------------------------------------------------------------------------------------
TimeUnits SeqData::get_duration()
{
    return m_pEntry ? m_pEntry->duration() : 0.0;
}



}  //namespace lomse

