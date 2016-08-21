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

#include "lomse_system_layouter.h"

#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_box_slice_instr.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_shapes_storage.h"
#include "lomse_logger.h"

#include "lomse_score_layouter.h"
#include "lomse_staffobjs_table.h"
#include "lomse_engraving_options.h"
#include "lomse_barline_engraver.h"
#include "lomse_instrument_engraver.h"
#include "lomse_spacing_algorithm.h"
#include "lomse_timegrid_table.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <math.h>
using namespace std;


namespace lomse
{

#define LOMSE_NO_DURATION   100000000000000.0     //any impossible high value
#define LOMSE_NO_TIME       100000000000000.0     //any impossible high value
#define LOMSE_NO_POSITION   100000000000000.0f    //any impossible high value




//=======================================================================================
// SystemLayouter implementation
//=======================================================================================
SystemLayouter::SystemLayouter(ScoreLayouter* pScoreLyt, LibraryScope& libraryScope,
                               ScoreMeter* pScoreMeter, ImoScore* pScore,
                               ShapesStorage& shapesStorage,
                               ShapesCreator* pShapesCreator,
                               PartsEngraver* pPartsEngraver,
                               SpacingAlgorithm* pSpAlgorithm)
    : m_pScoreLyt(pScoreLyt)
    , m_libraryScope(libraryScope)
    , m_pScoreMeter(pScoreMeter)
    , m_pScore(pScore)
    , m_shapesStorage(shapesStorage)
    , m_pShapesCreator(pShapesCreator)
    , m_pPartsEngraver(pPartsEngraver)
    , m_uPrologWidth(0.0f)
    , m_pBoxSystem(NULL)
    , m_yMin(0.0f)
    , m_yMax(0.0f)
    , m_pSpAlgorithm(pSpAlgorithm)
{
}

//---------------------------------------------------------------------------------------
SystemLayouter::~SystemLayouter()
{
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* SystemLayouter::create_system_box(LUnits left, LUnits top, LUnits width,
                                                LUnits height)
{
    m_pBoxSystem = LOMSE_NEW GmoBoxSystem(m_pScore);
    m_pBoxSystem->set_origin(left, top);

    LUnits leftMargin = 0.0f; //TODO-LOG: m_pScoreLyt->get_system_left_space(iSystem);
    m_pBoxSystem->set_left_margin(leftMargin);

    m_pBoxSystem->set_width(width);
    m_pBoxSystem->set_height(height);
    m_yMin = top;
    m_yMax = top + height;

    return m_pBoxSystem;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_system(LUnits indent, int iFirstCol, int iLastCol,
                                    UPoint pos)
{
    m_iSystem = m_pScoreLyt->m_iCurSystem;
    m_iFirstCol = iFirstCol;
    m_iLastCol = iLastCol;
    m_pagePos = pos;

    reposition_staves(indent);
    fill_current_system_with_columns();
    justify_current_system();
    build_system_timegrid();
    engrave_system_details(m_iSystem);

    if (m_pScoreLyt->is_last_system() && m_pScoreLyt->m_fStopStaffLinesAtFinalBarline)
        truncate_current_system(indent);

    engrave_instrument_details();
    add_instruments_info();

    add_initial_line_joining_all_staves_in_system();
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_staves(LUnits indent)
{
    //For engraving staffobjs, staves where at arbitrary positions (0,0). Now, once
    //the system box is engraved, the good positioning info. is transferred to
    //instrument engravers, as reference for all coming steps.

    UPoint org = m_pBoxSystem->get_origin();
    org.y += m_pScoreLyt->determine_top_space(0);
    org.x = 0.0f;

    m_pPartsEngraver->reposition_staves(indent, org, m_pBoxSystem);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::on_origin_shift(LUnits yShift)
{
    m_yMax += yShift;
    m_yMin += yShift;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::fill_current_system_with_columns()
{
    m_pScoreLyt->m_iCurColumn = 0;
    if (m_pScoreLyt->get_num_systems() == 0)
        return;

    InstrumentEngraver* pEngrv = m_pPartsEngraver->get_engraver_for(0);
    m_pagePos.x = pEngrv->get_staves_left();

    m_uFreeSpace = (m_iSystem == 0 ? m_pScoreLyt->get_first_system_staves_size()
                                   : m_pScoreLyt->get_other_systems_staves_size() );

    m_fFirstColumnInSystem = true;
    for (int iCol = m_iFirstCol; iCol < m_iLastCol; ++iCol)
    {
        m_pScoreLyt->m_iCurColumn = iCol;
        add_system_prolog_if_necessary();
        add_column_to_system(iCol);
        m_fFirstColumnInSystem = false;
    }
    m_pScoreLyt->m_iCurColumn = m_iLastCol;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::justify_current_system()
{
    if (m_pScoreLyt->is_system_empty(m_iSystem))
        return;

    if (system_must_be_justified())
        redistribute_free_space();

    reposition_slices_and_staffobjs();
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_column_to_system(int iCol)
{
    m_pagePos.x = determine_column_start_position(iCol);
    LUnits size = determine_column_size(iCol);

    reposition_and_add_slice_box(iCol, m_pagePos.x, size);
    add_shapes_for_column(iCol, &m_shapesStorage);

    m_uFreeSpace -= size;
    m_pagePos.x += size;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_system_prolog_if_necessary()
{
    if (m_pScoreLyt->m_iCurColumn > 0 && is_first_column_in_system())
	{
	    LUnits uPrologWidth = 0.0f;

	    int numInstruments = m_pScoreMeter->num_instruments();
	    for (int iInstr=0; iInstr < numInstruments; ++iInstr)
	    {
            LUnits width = engrave_prolog(iInstr);
	        uPrologWidth = max(uPrologWidth, width);
	    }

        m_pagePos.x += uPrologWidth;
        m_uFreeSpace -= uPrologWidth;
	}
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::determine_column_start_position(int iCol)
{
    return m_pagePos.x;
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::determine_column_size(int iCol)
{
      return m_pSpAlgorithm->get_column_width(iCol);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_and_add_slice_box(int iCol, LUnits pos,
                                                  LUnits UNUSED(size))
{
    LUnits ySystem = m_pBoxSystem->get_top();
    m_pSpAlgorithm->set_slice_final_position(iCol, pos, ySystem);

    GmoBoxSlice* pSlice = m_pSpAlgorithm->get_slice_box(iCol);
    m_pBoxSystem->add_child_box(pSlice);
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::engrave_prolog(int iInstr)
{
    LUnits uPrologWidth = 0.0f;

    //AWARE when this method is invoked the paper position is at the left marging,
    //at the start of the new system.
    LUnits xStartPos = m_pagePos.x;      //Save x to align all clefs

    //iterate over the collection of staff objects to draw current clef and key signature
    ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);

    GmoBoxSystem* pBox = get_box_system();

    int numStaves = pInstr->get_num_staves();
    InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);
    for (int iStaff=0; iStaff < numStaves; ++iStaff)
    {
        LUnits xPos = xStartPos;
        m_pagePos.y = pInstrEngrv->get_top_line_of_staff(iStaff);
        int iStaffIndex = m_pScoreMeter->staff_index(iInstr, iStaff);
        ColStaffObjsEntry* pClefEntry =
            m_pSpAlgorithm->get_prolog_clef(m_iFirstCol, iStaffIndex);
        ColStaffObjsEntry* pKeyEntry =
            m_pSpAlgorithm->get_prolog_key(m_iFirstCol, iStaffIndex);
        ImoClef* pClef = pClefEntry ? static_cast<ImoClef*>(pClefEntry->imo_object())
                                    : NULL;
        int clefType = pClef ? pClef->get_clef_type() : k_clef_undefined;

        //add clef shape
        if (pClefEntry)
        {
            if (pClef && pClef->is_visible())
            {
                xPos += m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_BEFORE_PROLOG, iInstr, iStaff);
                m_pagePos.x = xPos;
                GmoShape* pShape =
                    m_pShapesCreator->create_staffobj_shape(pClef, iInstr, iStaff,
                                                            m_pagePos, clefType);
                pShape->assign_id_as_prolog_shape(m_iSystem, iStaff, numStaves);
                pBox->add_shape(pShape, GmoShape::k_layer_notes);
                xPos += pShape->get_width();
            }
        }

        //add key signature shape
        if (pKeyEntry)
        {
            ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pKeyEntry->imo_object() );
            if (pKey && pKey->is_visible())
            {
                xPos += m_pScoreMeter->tenths_to_logical(LOMSE_PROLOG_GAP_BEORE_KEY, iInstr, iStaff);
                m_pagePos.x = xPos;
                GmoShape* pShape =
                    m_pShapesCreator->create_staffobj_shape(pKey, iInstr, iStaff,
                                                            m_pagePos, clefType);
                pShape->assign_id_as_prolog_shape(m_iSystem, iStaff, numStaves);
                pBox->add_shape(pShape, GmoShape::k_layer_notes);
                xPos += pShape->get_width();
            }
        }

        xPos += m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_AFTER_PROLOG, iInstr, iStaff);
        uPrologWidth = max(uPrologWidth, xPos - xStartPos);
    }

    m_pagePos.x = xStartPos;     //restore cursor
    set_prolog_width(uPrologWidth);

    return uPrologWidth;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_shapes_for_column(int iCol, ShapesStorage* pStorage)
{
    m_pSpAlgorithm->add_shapes_to_boxes(iCol, pStorage);
}

//---------------------------------------------------------------------------------------
bool SystemLayouter::system_must_be_justified()
{
    //Force justification if it is the last system and free space is negative
    if (m_pScoreLyt->is_last_system() && m_uFreeSpace < 0.0f)
        return true;

    //Otherwise, all systems needs justification except:

    //1. unless justification suppressed, for debugging
    if (!m_libraryScope.justify_systems())
        return false;

    //2. it is the last system and flag "JustifyFinalBarline" is not set,
    //   unless free space is negative
    if (m_pScoreLyt->is_last_system())
    {
        if (m_pScoreLyt->m_fJustifyFinalBarline)
            return true;
        else
            return m_uFreeSpace < 0.0f;
    }

    //3. it is the last system but there is no final barline
    int iLastCol = m_pScoreLyt->get_num_columns();
    if (m_pScoreLyt->is_last_system() && !m_pSpAlgorithm->column_has_barline_at_end(iLastCol))
        return false;



    return true;        //do justification
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_slices_and_staffobjs()
{
    LUnits yShift = m_pScoreLyt->determine_top_space(0);
    m_pSpAlgorithm->reposition_slices_and_staffobjs(m_iFirstCol, m_iLastCol, yShift,
                                                    &m_yMin, &m_yMax);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::redistribute_free_space()
{
//    if (m_uFreeSpace <= 0.0f)
//        return;           //no space to distribute

    m_pSpAlgorithm->justify_system(m_iFirstCol, m_iLastCol, m_uFreeSpace);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_initial_line_joining_all_staves_in_system()
{
    //do not draw if 'hide staff lines' option enabled
    ImoOptionInfo* pOpt = m_pScore->get_option("StaffLines.Hide");
    bool fDrawStafflines = (pOpt == NULL || pOpt->get_bool_value() == false);
    if (!fDrawStafflines)
        return;

    //do not draw if empty score with one instrument with one staff
    if (m_pScore->get_num_instruments() == 1)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        if (pInstr->get_num_staves() == 1 && m_pScoreMeter->is_empty_score())
            return;
    }

    //do not draw if so asked when score meter was created (//TODO: what is this for?)
	if (m_pScoreMeter->must_draw_left_barline())
	{
        InstrumentEngraver* pInstrEngrv = m_pPartsEngraver->get_engraver_for(0);
        ImoObj* pCreator = m_pScore->get_instrument(0);
        LUnits xPos = pInstrEngrv->get_staves_left();
        LUnits yTop = pInstrEngrv->get_staves_top_line();
        int iInstr = m_pScoreMeter->num_instruments() - 1;
        pInstrEngrv = m_pPartsEngraver->get_engraver_for(iInstr);
        LUnits yBottom = pInstrEngrv->get_staves_bottom_line();
        BarlineEngraver engrv(m_libraryScope, m_pScoreMeter);
        Color color = Color(0,0,0); //TODO staff lines color?
        GmoShape* pLine =
            engrv.create_system_barline_shape(pCreator, xPos, yTop, yBottom, color);
        m_pBoxSystem->add_shape(pLine, GmoShape::k_layer_staff);
	}
}

//---------------------------------------------------------------------------------------
void SystemLayouter::truncate_current_system(LUnits indent)
{
    if (m_pScoreMeter->is_empty_score())
        return;

    if (!m_pSpAlgorithm->column_has_barline_at_end(m_iLastCol-1))
        return;

    GmoBoxSlice* pSlice = m_pSpAlgorithm->get_slice_box(m_iLastCol-1);
    m_pBoxSystem->set_width( pSlice->get_right() - m_pBoxSystem->get_left() );
    reposition_staves(indent);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_instrument_details()
{
    ImoOptionInfo* pOpt = m_pScore->get_option("StaffLines.Hide");
    bool fDrawStafflines = (pOpt == NULL || pOpt->get_bool_value() == false);

    m_pPartsEngraver->engrave_names_and_brackets(fDrawStafflines, m_pBoxSystem,
                                                 m_iSystem);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_system_details(int iSystem)
{
    std::list<PendingAuxObjs*>::iterator it;
    for (it = m_pScoreLyt->m_pendingAuxObjs.begin(); it != m_pScoreLyt->m_pendingAuxObjs.end(); )
    {
        int objSystem = m_pScoreLyt->get_system_containing_column( (*it)->m_iCol );
        if (objSystem > iSystem)
            break;
        if (objSystem == iSystem)
        {
            PendingAuxObjs* pPAO = *it;
            engrave_attached_objects((*it)->m_pSO, (*it)->m_pMainShape,
                                     (*it)->m_iInstr, (*it)->m_iStaff, objSystem,
                                     (*it)->m_iCol, (*it)->m_iLine,
                                     (*it)->m_pInstr
                                    );
		    it = m_pScoreLyt->m_pendingAuxObjs.erase(it);
            delete pPAO;
        }
        else
            ++it;
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_attached_objects(ImoStaffObj* pSO, GmoShape* pMainShape,
                                              int iInstr, int iStaff, int iSystem,
                                              int iCol, int iLine,
                                              ImoInstrument* pInstr)
{
    //rel objs
    if (pSO->get_num_relations() > 0)
    {
        ImoRelations* pRelObjs = pSO->get_relations();
        int size = pRelObjs->get_num_items();
	    for (int i=0; i < size; ++i)
	    {
            ImoRelObj* pRO = pRelObjs->get_item(i);

            if (!pRO->is_chord())
            {
		        if (pSO == pRO->get_start_object())
                    m_pShapesCreator->start_engraving_relobj(pRO, pSO, pMainShape,
                                                            iInstr, iStaff, iSystem, iCol,
                                                            iLine, pInstr);
		        else if (pSO == pRO->get_end_object())
		        {
                    SystemLayouter* pSysLyt = m_pScoreLyt->get_system_layouter(iSystem);
                    LUnits prologWidth( pSysLyt->get_prolog_width() );

                    m_pShapesCreator->finish_engraving_relobj(pRO, pSO, pMainShape,
                                                            iInstr, iStaff, iSystem, iCol,
                                                            iLine, prologWidth, pInstr);
                    add_relobjs_shapes_to_model(pRO, GmoShape::k_layer_aux_objs);
		        }
                else
                    m_pShapesCreator->continue_engraving_relobj(pRO, pSO, pMainShape,
                                                                iInstr, iStaff, iSystem,
                                                                iCol, iLine, pInstr);
            }
        }
    }

    //aux objs
    if (pSO->get_num_attachments() > 0)
    {
        ImoAttachments* pAuxObjs = pSO->get_attachments();
        int size = pAuxObjs->get_num_items();
	    for (int i=0; i < size; ++i)
	    {
            ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
            if (pAO->is_lyric())
            {
                if (pSO->is_note())
                {
                    ImoLyric* pLyric = static_cast<ImoLyric*>(pAO);
                    ImoNote* pNote = static_cast<ImoNote*>(pSO);

                    //build hash code from instrument, number & voice.
                    stringstream tag;
                    tag << iInstr << "-" << pLyric->get_number()
                        << "-" << pNote->get_voice();

                    GmoShapeNote* pNoteShape = static_cast<GmoShapeNote*>(pMainShape);
                    if (pLyric->is_start_of_relation())
                        m_pShapesCreator->start_engraving_auxrelobj(pLyric, pSO, tag.str(),
                                                    pNoteShape, iInstr, iStaff, iSystem,
                                                    iCol, iLine, pInstr);
                    else if (pLyric->is_end_of_relation())
                    {
                        SystemLayouter* pSysLyt = m_pScoreLyt->get_system_layouter(iSystem);
                        LUnits prologWidth( pSysLyt->get_prolog_width() );

                        m_pShapesCreator->finish_engraving_auxrelobj(pLyric, pSO, tag.str(),
                                                    pNoteShape, iInstr, iStaff, iSystem,
                                                    iCol, iLine, prologWidth, pInstr);
                        add_relauxobjs_shapes_to_model(tag.str(), GmoShape::k_layer_aux_objs);
                    }
                    else
                        m_pShapesCreator->continue_engraving_auxrelobj(pLyric, pSO, tag.str(),
                                                    pNoteShape, iInstr, iStaff, iSystem,
                                                    iCol, iLine, pInstr);
                }
            }
            else
            {
                GmoShape* pAuxShape =
                            m_pShapesCreator->create_auxobj_shape(pAO, iInstr, iStaff,
                                                                  pMainShape);
    //            pMainShape->accept_link_from(pAuxShape);
                add_aux_shape_to_model(pAuxShape, GmoShape::k_layer_aux_objs, iSystem,
                                       iCol, iInstr);
                m_yMax = max(m_yMax, pAuxShape->get_bottom());
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_relobjs_shapes_to_model(ImoObj* pAO, int layer)
{
    RelObjEngraver* pEngrv
        = dynamic_cast<RelObjEngraver*>(m_shapesStorage.get_engraver(pAO));

    int numShapes = pEngrv->get_num_shapes();
    for (int i=0; i < numShapes; ++i)
    {
        ShapeBoxInfo* pInfo = pEngrv->get_shape_box_info(i);
        GmoShape* pAuxShape = pInfo->pShape;
        if (pAuxShape)
        {
            int iSystem = pInfo->iSystem;
            int iCol = pInfo->iCol;
            int iInstr = pInfo->iInstr;

            add_aux_shape_to_model(pAuxShape, layer, iSystem, iCol, iInstr);
        }
   }

    m_shapesStorage.remove_engraver(pAO);
    delete pEngrv;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_relauxobjs_shapes_to_model(const string& tag, int layer)
{
    AuxRelObjEngraver* pEngrv
        = dynamic_cast<AuxRelObjEngraver*>(m_shapesStorage.get_engraver(tag));

    int numShapes = pEngrv->get_num_shapes();
    for (int i=0; i < numShapes; ++i)
    {
        ShapeBoxInfo* pInfo = pEngrv->get_shape_box_info(i);
        GmoShape* pAuxShape = pInfo->pShape;
        if (pAuxShape)
        {
            int iSystem = pInfo->iSystem;
            int iCol = pInfo->iCol;
            int iInstr = pInfo->iInstr;

            add_aux_shape_to_model(pAuxShape, layer, iSystem, iCol, iInstr);
        }
   }

    m_shapesStorage.remove_engraver(tag);
    delete pEngrv;
}


//---------------------------------------------------------------------------------------
void SystemLayouter::add_aux_shape_to_model(GmoShape* pShape, int layer,
                                            int UNUSED(iSystem),
                                            int iCol, int iInstr)
{
    pShape->set_layer(layer);
    GmoBoxSliceInstr* pBox = m_pSpAlgorithm->get_slice_instr(iCol, iInstr);
    pBox->add_shape(pShape, layer);
    m_yMax = max(m_yMax, pShape->get_bottom());
}

//---------------------------------------------------------------------------------------
void SystemLayouter::build_system_timegrid()
{
    TimeGridTable* pTable = LOMSE_NEW TimeGridTable();

    for (int iCol = m_iFirstCol; iCol < m_iLastCol; ++iCol)
    {
        TimeGridTable* pColTable =
            m_pSpAlgorithm->create_time_grid_table_for_column(iCol);

        //accumulate data
        if (pColTable)
            pTable->add_entries( pColTable->get_entries() );

        delete pColTable;
    }

    m_pBoxSystem->set_time_grid_table(pTable);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_instruments_info()
{
    int maxInstr = m_pScore->get_num_instruments() - 1;
    for (int i = 0; i <= maxInstr; i++)
    {
        ImoInstrument* pInstr = m_pScore->get_instrument(i);
        m_pBoxSystem->add_num_staves_for_instrument(pInstr->get_num_staves());
    }
}



}  //namespace lomse
