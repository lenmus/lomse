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

#include "lomse_spacing_algorithm.h"

#include "lomse_staffobjs_table.h"
#include "lomse_score_iterator.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_box_slice_instr.h"
#include "lomse_score_iterator.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_instrument_engraver.h"
#include "lomse_score_layouter.h"
#include "lomse_box_slice.h"
#include "lomse_shape_barline.h"


namespace lomse
{


//=====================================================================================
//SpacingAlgorithm implementation
//=====================================================================================
SpacingAlgorithm::SpacingAlgorithm(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                                   ScoreLayouter* pScoreLyt, ImoScore* pScore,
                                   EngraversMap& engravers,
                                   ShapesCreator* pShapesCreator,
                                   PartsEngraver* pPartsEngraver)
    : m_libraryScope(libraryScope)
    , m_pScoreMeter(pScoreMeter)
    , m_pScoreLyt(pScoreLyt)
    , m_pScore(pScore)
    , m_engravers(engravers)
    , m_pShapesCreator(pShapesCreator)
    , m_pPartsEngraver(pPartsEngraver)
{
}

//---------------------------------------------------------------------------------------
SpacingAlgorithm::~SpacingAlgorithm()
{
}


//=====================================================================================
//SpAlgColumn implementation
//=====================================================================================
SpAlgColumn::SpAlgColumn(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                         ScoreLayouter* pScoreLyt, ImoScore* pScore,
                         EngraversMap& engravers,
                         ShapesCreator* pShapesCreator,
                         PartsEngraver* pPartsEngraver)
    :SpacingAlgorithm(libraryScope, pScoreMeter, pScoreLyt, pScore, engravers,
                      pShapesCreator, pPartsEngraver)
    , m_pColsBuilder(nullptr)
{
    m_pColsBuilder = LOMSE_NEW ColumnsBuilder(m_pScoreMeter, m_colsData,
                     m_pScoreLyt, m_pScore, m_engravers,
                     m_pShapesCreator,
                     m_pPartsEngraver, this);
}

//---------------------------------------------------------------------------------------
SpAlgColumn::~SpAlgColumn()
{
    delete m_pColsBuilder;
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::split_content_in_columns()
{
    m_pColsBuilder->create_columns();
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::do_spacing_algorithm()
{
    m_pColsBuilder->do_spacing_algorithm();
}

//---------------------------------------------------------------------------------------
LUnits SpAlgColumn::get_staves_height()
{
    return m_pColsBuilder->get_staves_height();
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::add_shapes_to_boxes(int iCol, VerticalProfile* pVProfile)
{
    m_pVProfile = pVProfile;
    m_pColsBuilder->add_shapes_to_boxes(iCol);
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr* SpAlgColumn::create_slice_instr(int iCol, ImoInstrument* pInstr,
                                                  int idxStaff, LUnits yTop)
{
    return m_colsData[iCol]->create_slice_instr(pInstr, idxStaff, yTop);
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr* SpAlgColumn::get_slice_instr(int iCol, int iInstr)
{
    return m_colsData[iCol]->get_slice_instr(iInstr);
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::set_slice_width(int iCol, LUnits width)
{
    m_colsData[iCol]->set_slice_width(width);
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::create_boxes_for_column(int iCol, LUnits left, LUnits top)
{
    m_pColsBuilder->create_boxes_for_column(iCol, left, top);
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::set_slice_final_position(int iCol, LUnits left, LUnits top)
{
    m_colsData[iCol]->set_slice_final_position(left, top);
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::use_this_slice_box(int iCol, GmoBoxSlice* pBoxSlice)
{
    m_colsData[iCol]->use_this_slice_box(pBoxSlice);
}

//---------------------------------------------------------------------------------------
GmoBoxSlice* SpAlgColumn::get_slice_box(int iCol)
{
    return m_colsData[iCol]->get_slice_box();
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::save_context(int iCol, int iInstr, int iStaff,
                               ColStaffObjsEntry* pClefEntry,
                               ColStaffObjsEntry* pKeyEntry)

{
    m_colsData[iCol]->save_context(iInstr, iStaff, pClefEntry, pKeyEntry);
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* SpAlgColumn::get_prolog_clef(int iCol, ShapeId idx)
{
    return m_colsData[iCol]->get_prolog_clef(idx);
}

//---------------------------------------------------------------------------------------
ColStaffObjsEntry* SpAlgColumn::get_prolog_key(int iCol, ShapeId idx)
{
    return m_colsData[iCol]->get_prolog_key(idx);
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::set_system_break(int iCol, bool value)
{
    m_colsData[iCol]->set_system_break(value);
}

//---------------------------------------------------------------------------------------
bool SpAlgColumn::has_system_break(int iCol)
{
    return m_colsData[iCol]->has_system_break();
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::delete_box_and_shapes(int iCol)
{
    m_colsData[iCol]->delete_box_and_shapes(iCol);
}

//---------------------------------------------------------------------------------------
int SpAlgColumn::get_num_columns()
{
    return int(m_colsData.size());
}

//---------------------------------------------------------------------------------------
ColumnData* SpAlgColumn::get_column(int iCol)
{
    return (iCol < get_num_columns() ? m_colsData[iCol] : nullptr);
}

//---------------------------------------------------------------------------------------
TypeMeasureInfo* SpAlgColumn::get_measure_info_for_column(int iCol)
{
    ColumnData* pCol = get_column(iCol);
    if (pCol && pCol->is_start_of_measure())
        return pCol->get_measure_info();

    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShapeBarline* SpAlgColumn::get_start_barline_shape_for_column(int iCol)
{
    ColumnData* pCol = get_column(iCol);
    if (pCol && pCol->is_start_of_measure())
        return pCol->get_shape_for_start_barline();

    return nullptr;
}

//---------------------------------------------------------------------------------------
void SpAlgColumn::set_trace_level(int iColumnToTrace, int nTraceLevel)
{
    m_pColsBuilder->set_debug_options(iColumnToTrace, nTraceLevel);
}


//=======================================================================================
// ColumnsBuilder implementation
//=======================================================================================
ColumnsBuilder::ColumnsBuilder(ScoreMeter* pScoreMeter, vector<ColumnData*>& colsData,
                               ScoreLayouter* pScoreLyt, ImoScore* m_pScore,
                               EngraversMap& engravers,
                               ShapesCreator* pShapesCreator,
                               PartsEngraver* pPartsEngraver,
                               SpAlgColumn* pSpAlgorithm)
    : m_pScoreMeter(pScoreMeter)
    , m_pScoreLyt(pScoreLyt)
    , m_pScore(m_pScore)
    , m_engravers(engravers)
    , m_pShapesCreator(pShapesCreator)
    , m_pPartsEngraver(pPartsEngraver)
    , m_pSysCursor( LOMSE_NEW StaffObjsCursor(m_pScore) )
    , m_pBreaker( LOMSE_NEW ColumnBreaker(m_pScoreMeter->num_instruments(),
                                          m_pSysCursor) )
    , m_stavesHeight(0.0f)
    , m_iColumn(0)
    , m_iColStartMeasure(0)
    , m_pStartBarlineShape(nullptr)
    , m_iColumnToTrace(-1)
    , m_nTraceLevel(k_trace_off)
    , m_pSpAlgorithm(pSpAlgorithm)
    , m_maxColumn(0)
    , m_colsData(colsData)
{
}

//---------------------------------------------------------------------------------------
ColumnsBuilder::~ColumnsBuilder()
{
    delete m_pSysCursor;
    delete m_pBreaker;

    vector<ColumnData*>::iterator it;
    for (it = m_colsData.begin(); it != m_colsData.end(); ++it)
        delete *it;
    m_colsData.clear();

}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::create_columns()
{
    m_iColumn = -1;
    m_iColStartMeasure = 0;
    m_pStartBarlineShape = nullptr;
    m_fNoSignatures.assign(m_pScore->get_num_instruments(), true);
    m_fClefFound.assign(m_pSysCursor->get_num_staves(), false);

    determine_staves_vertical_position();
    while(!m_pSysCursor->is_end())
    {
        m_iColumn++;
        prepare_for_new_column();
        m_colsData.push_back( LOMSE_NEW ColumnData(m_pScoreMeter, m_pSpAlgorithm) );
        find_and_save_context_info_for_this_column();
        collect_content_for_this_column();
    }
    m_maxColumn = m_iColumn;
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::do_spacing_algorithm()
{
    for (m_iColumn=0; m_iColumn <= m_maxColumn; ++m_iColumn)
        layout_column();
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::collect_content_for_this_column()
{
    //ask system layouter to prepare for receiving data for objects in this column
    m_pSpAlgorithm->start_column_measurements(m_iColumn);

    //loop to process all StaffObjs until this column is completed
    ImoStaffObj* pSO = nullptr;
    ImoStaffObj* pPrevSO = nullptr;
    GmoShapeBarline* pPrevBarlineShape = nullptr;
    GmoShape* pShape = nullptr;

    bool fSaveNonTimed = (m_iColumn == 0);
    vector<GmoShape*> nonTimed;     //last non-timed shape at start or after a barline
    nonTimed.assign(m_pScoreMeter->num_instruments(), nullptr);

    while(!m_pSysCursor->is_end() )
    {
        pPrevSO = pSO;
        if (pPrevSO && pPrevSO->is_barline())
            pPrevBarlineShape = static_cast<GmoShapeBarline*>(pShape);

        pSO = m_pSysCursor->get_staffobj();
        int iInstr = m_pSysCursor->num_instrument();
        int iStaff = m_pSysCursor->staff();
        int iLine = m_pSysCursor->line();
        int idxStaff = m_pScoreMeter->staff_index(iInstr, iStaff);
        TimeUnits rTime = m_pSysCursor->time();
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        InstrumentEngraver* pIE = m_pPartsEngraver->get_engraver_for(iInstr);
        UPoint pagePos;           //to track current position
        pagePos.x = 0.0f;
        pagePos.y = pIE->get_top_line_of_staff(iStaff);

        //if feasible column break, exit loop and finish column
        if ( m_pBreaker->feasible_break_before_this_obj(pSO, rTime, iInstr, iLine) )
            break;


        if (pSO->is_system_break())
        {
            m_pSpAlgorithm->set_system_break(m_iColumn, true);
        }
        else
        {
            if (pSO->is_clef())
            {
                ImoClef* pClef = static_cast<ImoClef*>(pSO);
                int idx = m_pSysCursor->staff_index();
                bool fInProlog = determine_if_is_in_prolog(pSO, rTime, iInstr, idx);
                unsigned flags = fInProlog ? 0 : ShapesCreator::k_flag_small_clef;
                int clefType = pClef->get_clef_type();
                pShape = m_pShapesCreator->create_staffobj_shape(pSO, iInstr, iStaff,
                         pagePos, clefType, 0, flags);
                pShape->assign_id_as_main_shape();
                m_pSpAlgorithm->include_object(m_pSysCursor->cur_entry(), m_iColumn,
                                               iInstr, iStaff, pSO, pShape, fInProlog);
            }

            else if (pSO->is_key_signature() || pSO->is_time_signature())
            {
                unsigned flags = 0;
                int idx = m_pSysCursor->staff_index();
                bool fInProlog = determine_if_is_in_prolog(pSO, rTime, iInstr, idx);
                int clefType = m_pSysCursor->get_applicable_clef_type();
                pShape = m_pShapesCreator->create_staffobj_shape(pSO, iInstr, iStaff,
                         pagePos, clefType, 0, flags);
                pShape->assign_id_as_main_or_implicit_shape(iStaff);
                m_pSpAlgorithm->include_object(m_pSysCursor->cur_entry(), m_iColumn,
                                               iInstr, iStaff, pSO, pShape, fInProlog);
            }

            else
            {
                int clefType = m_pSysCursor->get_applicable_clef_type();
                int octaveShift = m_pSysCursor->get_applicable_octave_shift();
                pShape = m_pShapesCreator->create_staffobj_shape(pSO, iInstr, iStaff,
                         pagePos, clefType, octaveShift);
                //TimeUnits time = (pSO->is_spacer() ? -1.0f : rTime);
                m_pSpAlgorithm->include_object(m_pSysCursor->cur_entry(), m_iColumn,
                                               iInstr, iStaff, pSO, pShape);

                //save data about full-measure rests
                if (pSO->is_rest() && static_cast<ImoRest*>(pSO)->is_full_measure())
                {
                    m_pSpAlgorithm->include_full_measure_rest(pShape, m_pSysCursor->cur_entry(),
                                                              nonTimed[iInstr]);
                }
            }

            store_info_about_attached_objects(pSO, pShape, iInstr, iStaff,
                                              m_iColumn, iLine, pInstr, idxStaff);

            //save shapes for non-timed after barline
            if (fSaveNonTimed &&
                (pSO->is_clef() || pSO->is_key_signature() || pSO->is_time_signature()))
            {
                nonTimed[iInstr] = pShape;
            }

            //save data for building the GmMeasuresTable and reset non-timed table
            if (pSO->is_barline() && !static_cast<ImoBarline*>(pSO)->is_middle())
            {
                m_pScoreLyt->finish_measure(iInstr, static_cast<GmoShapeBarline*>(pShape));

                fSaveNonTimed = false;
                nonTimed.assign(nonTimed.size(), nullptr);
            }

        }

        m_pSysCursor->move_next();
    }

    //The loop is exited:
    //a) on the first SO after a barline: pSO=normally a note/rest, pPrevSO=barline
    //   pPrevBarlineShape= ptr.to barline for pPrevSO
    //b) or at end of SysCursor: pSO=last barline, last pSO, or nullptr if score empty.

    //save data for laying out measure attributes
    if ((pPrevSO && pPrevSO->is_barline()) || (m_pSysCursor->is_end() && pSO))
    {
        //measure that starts in column m_iColStartMeasure finishes in current column.
        //Its start barline shape is in m_pStartBarlineShape

        ColumnData* pStartColumn = m_colsData[m_iColStartMeasure];
        TypeMeasureInfo* pInfo = nullptr;

        if (m_pSysCursor->is_end())
        {
            if (pSO->is_barline())
            {
                ImoBarline* pBarline = static_cast<ImoBarline*>(pSO);
                pInfo = pBarline->get_measure_info();
            }
            else
            {
                ImoInstrument* pInstr = pSO->get_instrument();
                pInfo = pInstr->get_last_measure_info();
            }
        }
        else
        {
            ImoBarline* pBarline = static_cast<ImoBarline*>(pPrevSO);
            pInfo = pBarline->get_measure_info();
        }
        pStartColumn->mark_as_start_of_measure(pInfo, m_pStartBarlineShape);

        m_iColStartMeasure = m_iColumn + 1;
        m_pStartBarlineShape = pPrevBarlineShape;
    }

    m_pSpAlgorithm->finish_column_measurements(m_iColumn);
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::find_and_save_context_info_for_this_column()
{
    int numInstr = m_pScore->get_num_instruments();
    for (int iInstr=0; iInstr < numInstr; ++iInstr)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        for (int iStaff=0; iStaff < pInstr->get_num_staves(); ++iStaff)
        {
            ColStaffObjsEntry* pClefEntry =
                m_pSysCursor->get_clef_entry_for_instr_staff(iInstr, iStaff);
            ColStaffObjsEntry* pKeyEntry =
                m_pSysCursor->get_key_entry_for_instr_staff(iInstr, iStaff);
            m_pSpAlgorithm->save_context(m_iColumn, iInstr, iStaff, pClefEntry, pKeyEntry);
        }
    }
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::store_info_about_attached_objects(ImoStaffObj* pSO,
        GmoShape* pMainShape, int iInstr, int iStaff,
        int iCol, int iLine, ImoInstrument* pInstr, int idxStaff)
{
    ImoAttachments* pAuxObjs = pSO->get_attachments();
    ImoRelations* pRelObjs = pSO->get_relations();
    if (!pAuxObjs && !pRelObjs)
        return;

    PendingAuxObjs* data = LOMSE_NEW PendingAuxObjs(pSO, pMainShape, iInstr, iStaff,
                           iCol, iLine, pInstr, idxStaff);
    m_pScoreLyt->m_pendingAuxObjs.push_back(data);
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::layout_column()
{
    bool fTrace = (m_iColumnToTrace == m_iColumn);
    m_pSpAlgorithm->do_spacing(m_iColumn, fTrace);
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::determine_staves_vertical_position()
{
    int numInstrs = m_pScore->get_num_instruments();

    m_SliceInstrHeights.resize(numInstrs);

    LUnits yPos = 0.0f;
    for (int iInstr = 0; iInstr < numInstrs; iInstr++)
    {
        LUnits yTop = yPos;
        InstrumentEngraver* engrv = m_pPartsEngraver->get_engraver_for(iInstr);

        if (iInstr > 0)
        {
            LUnits uMargin = m_pScoreLyt->determine_top_space(iInstr);
            yPos += uMargin;
        }

        engrv->set_staves_vertical_position(yPos);
        yPos = engrv->get_staves_bottom();

        if (iInstr != numInstrs-1)
            yPos += m_pScoreLyt->determine_top_space(iInstr+1);

        m_SliceInstrHeights[iInstr] = yPos - yTop;
    }
    m_stavesHeight = yPos;
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::create_boxes_for_column(int iCol, LUnits xLeft, LUnits yTop)
{
    GmoBoxSlice* pSlice = LOMSE_NEW GmoBoxSlice(iCol, m_pScore);
    pSlice->set_left(xLeft);
    pSlice->set_top(yTop);

    m_pSpAlgorithm->use_this_slice_box(iCol, pSlice);

    //create instrument slice boxes
    int numInstrs = m_pScore->get_num_instruments();

    int idxStaff = 0;
    for (int iInstr = 0; iInstr < numInstrs; iInstr++)
    {
        //create slice instr box
        ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
        GmoBoxSliceInstr* pCurBSI =
                m_pSpAlgorithm->create_slice_instr(iCol, pInstr, idxStaff, yTop);

        //set box height
        LUnits height = m_SliceInstrHeights[iInstr];
        if (iInstr==0)
        {
            //add top margin
            height += m_pScoreLyt->determine_top_space(0);
        }
        if (iInstr==numInstrs-1)
        {
            //add bottom margin
            ImoSystemInfo* pInfo = m_pScore->get_other_system_info();
            height += pInfo->get_system_distance() / 2.0f;
        }
        yTop += height;
        pCurBSI->set_height(height);
        idxStaff += pInstr->get_num_staves();
    }

    //set slice and system height
    LUnits uTotalHeight = yTop - pSlice->get_top();
    pSlice->set_height(uTotalHeight);
}

//---------------------------------------------------------------------------------------
GmoBoxSlice* ColumnsBuilder::create_slice_box()
{
    GmoBoxSlice* pSlice = LOMSE_NEW GmoBoxSlice(m_iColumn, m_pScore);
    pSlice->set_left(0.0f);
    pSlice->set_top(0.0f);

    m_pSpAlgorithm->use_this_slice_box(m_iColumn, pSlice);

    return pSlice;
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::prepare_for_new_column()
{
    m_pSpAlgorithm->prepare_for_new_column(m_iColumn);
}

//---------------------------------------------------------------------------------------
bool ColumnsBuilder::determine_if_is_in_prolog(ImoStaffObj* pSO, TimeUnits rTime,
                                               int iInstr, int idx)
{
    // In prolog only any clef, key & time signature at start of score. And only the
    // first clef before key/time signature. Any other clef will be considered a
    // clef change.
    // AWARE: Take into account that when the instrument has more than one staff
    // (e.g. piano) there are more than one clef per instrument. Also, take into
    // account that clef for second instrument could be defined after key and time
    // signatures for first instrument.

    if (!is_equal_time(rTime, 0.0))
        return false;

    if (pSO->is_clef())
    {
        bool fFirstClef = !m_fClefFound[idx];
        m_fClefFound[idx] = true;
        return fFirstClef || m_fNoSignatures[iInstr];
    }
    else if (pSO->is_key_signature() || pSO->is_time_signature())
    {
        m_fNoSignatures[iInstr] = false;
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::add_shapes_to_boxes(int iCol)
{
    m_colsData[iCol]->add_shapes_to_boxes(iCol);
}

//---------------------------------------------------------------------------------------
void ColumnsBuilder::delete_shapes(int iCol)
{
    m_colsData[iCol]->delete_shapes(iCol);
}



//=======================================================================================
//ColumnData implementation
//=======================================================================================
ColumnData::ColumnData(ScoreMeter* pScoreMeter, SpAlgColumn* pSpAlgorithm)
    : m_pScoreMeter(pScoreMeter)
    , m_fHasSystemBreak(false)
    , m_pBoxSlice(nullptr)
    , m_pSpAlgorithm(pSpAlgorithm)
    , m_fMeasureStart(false)
    , m_pMeasureInfo(nullptr)
    , m_pShapeBarline(nullptr)
    , m_nTraceLevel(k_trace_off)
{
    reserve_space_for_prolog_clefs_keys( m_pScoreMeter->num_staves() );
}

//---------------------------------------------------------------------------------------
ColumnData::~ColumnData()
{
}

//---------------------------------------------------------------------------------------
void ColumnData::reserve_space_for_prolog_clefs_keys(int numStaves)
{
    m_prologClefs.assign(numStaves, (ColStaffObjsEntry*)nullptr);     //GCC complains if nullptr not casted

    m_prologKeys.assign(numStaves, (ColStaffObjsEntry*)nullptr);
}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr* ColumnData::create_slice_instr(ImoInstrument* pInstr, int idxStaff,
                                                 LUnits yTop)
{
    GmoBoxSliceInstr* pBSI = m_pBoxSlice->add_box_for_instrument(pInstr, idxStaff);
	pBSI->set_top(yTop);
	pBSI->set_left( m_pBoxSlice->get_left() );
	pBSI->set_width( m_pBoxSlice->get_width() );
    m_sliceInstrBoxes.push_back( pBSI );
    return pBSI;
}

//---------------------------------------------------------------------------------------
void ColumnData::add_shapes_to_boxes(int iCol)
{
    for (int iInstr=0; iInstr < int(m_sliceInstrBoxes.size()); ++iInstr)
    {
        m_pSpAlgorithm->add_shapes_to_box(iCol, m_sliceInstrBoxes[iInstr], iInstr);
    }
}

//---------------------------------------------------------------------------------------
void ColumnData::delete_shapes(int iCol)
{
    m_pSpAlgorithm->delete_shapes(iCol);
}

//---------------------------------------------------------------------------------------
void ColumnData::delete_box_and_shapes(int iCol)
{
    delete_shapes(iCol);
    delete m_pBoxSlice;       //box for this column
}

//---------------------------------------------------------------------------------------
void ColumnData::set_slice_width(LUnits width)
{
    m_pBoxSlice->set_width(width);

    //set instrument slices width
    std::vector<GmoBoxSliceInstr*>::iterator it;
    for (it=m_sliceInstrBoxes.begin(); it != m_sliceInstrBoxes.end(); ++it)
    {
        (*it)->set_width(width);
    }
}

//---------------------------------------------------------------------------------------
void ColumnData::set_slice_final_position(LUnits left, LUnits top)
{
    m_pBoxSlice->new_left(left);
    m_pBoxSlice->new_top(top);

    //Re-position instrument slices
    std::vector<GmoBoxSliceInstr*>::iterator it;
    for (it=m_sliceInstrBoxes.begin(); it != m_sliceInstrBoxes.end(); ++it)
    {
        (*it)->new_left(left);
        (*it)->new_top(top);
        top += (*it)->get_height();
    }
}

//---------------------------------------------------------------------------------------
void ColumnData::save_context(int iInstr, int iStaff, ColStaffObjsEntry* pClefEntry,
                                  ColStaffObjsEntry* pKeyEntry)
{
    int idx = m_pScoreMeter->staff_index(iInstr, iStaff);
    m_prologClefs[idx] = pClefEntry;
    m_prologKeys[idx] = pKeyEntry;
}


}  //namespace lomse
