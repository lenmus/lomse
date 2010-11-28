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

#include "lomse_score_layouter.h"

#include "lomse_basic_model.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_box_slice_instr.h"
#include "lomse_shape_staff.h"
#include "lomse_instrument_engraver.h"
#include "lomse_system_layouter.h"
#include "lomse_clef_engraver.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// ScoreLayouter implementation
//---------------------------------------------------------------------------------------

ScoreLayouter::ScoreLayouter(ImoDocObj* pImo, GraphicModel* pGModel,
                             LibraryScope& libraryScope)
    : ContentLayouter(pImo, pGModel)
    , m_libraryScope(libraryScope)
    , m_scoreIt( get_imo_score()->get_staffobjs_table() )
//    , m_pSysCursor((SystemCursor*)NULL)
    , m_pStubScore(NULL)
    , m_pCurBoxPage(NULL)
    , m_pCurBoxSystem(NULL)
{
}

//---------------------------------------------------------------------------------------
ScoreLayouter::~ScoreLayouter()
{
    //delete_system_cursor();
    delete_instrument_layouters();
    delete_system_layouters();
}

//---------------------------------------------------------------------------------------
ImoScore* ScoreLayouter::get_imo_score()
{
    return dynamic_cast<ImoScore*>(m_pImo);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::prepare_to_start_layout()
{
    ContentLayouter::prepare_to_start_layout();

    create_stub_for_score();
    create_instrument_engravers();
    get_score_renderization_options();
 //   PrepareFontsThatMatchesStavesSizes();
    decide_systems_indentation();

	m_nCurrentPageNumber = 0;
    m_nCurSystem = 0;
    m_uLastSystemHeight = 0.0f;
    m_nAbsColumn = 0;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_stub_for_score()
{
    m_pStubScore = new GmoStubScore( get_imo_score() );
    m_pGModel->add_stub(m_pStubScore);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_instrument_engravers()
{
    ImoScore* pScore = get_imo_score();
    for (int iInstr = 0; iInstr < pScore->get_num_instruments(); iInstr++)
    {
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        m_instrEngravers.push_back( new InstrumentEngraver(pInstr, pScore, m_libraryScope) );
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::delete_instrument_layouters()
{
    std::vector<InstrumentEngraver*>::iterator it;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it)
        delete *it;
    m_instrEngravers.clear();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::get_score_renderization_options()
{
    ImoScore* pScore = get_imo_score();

    ImoOptionInfo* pOpt = pScore->get_option("StaffLines.StopAtFinalBarline");
    m_fStopStaffLinesAtFinalBarline = pOpt->get_bool_value();

    pOpt = pScore->get_option("Score.JustifyFinalBarline");
    m_fJustifyFinalBarline = pOpt->get_bool_value();

    pOpt = pScore->get_option("Render.SpacingFactor");
    m_rSpacingFactor = pOpt->get_float_value();

    pOpt = pScore->get_option("Render.SpacingMethod");
    m_nSpacingMethod = static_cast<ESpacingMethod>( pOpt->get_long_value() );

    pOpt = pScore->get_option("Render.SpacingValue");
    m_nSpacingValue = static_cast<Tenths>( pOpt->get_long_value() );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::delete_system_layouters()
{
    std::vector<SystemLayouter*>::iterator it;
    for (it = m_sysLayouters.begin(); it != m_sysLayouters.end(); ++it)
        delete *it;
    m_sysLayouters.clear();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::decide_systems_indentation()
{
    m_uFirstSystemIndent = 0.0f;
    m_uOtherSystemIndent = 0.0f;
    std::vector<InstrumentEngraver*>::iterator it;
    for (it = m_instrEngravers.begin(); it != m_instrEngravers.end(); ++it)
    {
        (*it)->measure_indents();
        m_uFirstSystemIndent = max(m_uFirstSystemIndent, (*it)->get_indent_first());
        m_uOtherSystemIndent = max(m_uOtherSystemIndent, (*it)->get_indent_other());
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::layout_in_page(GmoBox* pContainerBox)
{
    //AWARE: This method is invoked to layout a page. If there are more pages to
    //layout, it will be invoked more times. Therefore, this method must not initialize
    //anything. All initializations must be done in 'prepare_to_start_layout()'.
    //layout_in_page() method must allways continue layouting from current state.

    page_initializations(pContainerBox);
    move_cursor_to_top_left_corner();
    add_titles_if_first_page();

    while(more_systems_to_add() && enough_space_in_page())
    {
        add_next_system();
    }

    set_layout_is_finished( !more_systems_to_add() );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::page_initializations(GmoBox* pContainerBox)
{
    m_pCurBoxPage = dynamic_cast<GmoBoxScorePage*>( pContainerBox );
    is_first_system_in_page(true);
    more_systems_to_add(true);
}

//---------------------------------------------------------------------------------------
GmoBox* ScoreLayouter::create_pagebox(GmoBox* pParentBox)
{
    GmoBox* pBox = new GmoBoxScorePage(m_pStubScore, pParentBox);
    pBox->set_left( pParentBox->get_left() );
    pBox->set_top( pParentBox->get_top() );
    return pBox;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_titles_if_first_page()
{
    if (is_first_page())
    {
        add_score_titles();
        move_cursor_after_headers();
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_next_system()
{
    create_system_layouter();
    //create_system_cursor();
    create_system_box();
    fill_current_system_with_columns();
    justify_current_system();

    if (!more_systems_to_add() && m_fStopStaffLinesAtFinalBarline)
        truncate_current_system();
    //AddInitialLineJoiningAllStavesInSystem();
    //UpdateBoxSlicesSizes();
    set_system_height_and_advance_paper_cursor();
    is_first_system_in_page(false);
}

////---------------------------------------------------------------------------------------
//void ScoreLayouter::create_system_cursor()
//{
//    delete_system_cursor();
//    m_pSysCursor = new SystemCursor(m_pScore);
//}

////---------------------------------------------------------------------------------------
//void ScoreLayouter::delete_system_cursor()
//{
//    if (m_pSysCursor)
//        delete m_pSysCursor;
//}

//---------------------------------------------------------------------------------------
bool ScoreLayouter::enough_space_in_page()
{
    //TODO ************
    //  If paper height is smaller than system height it is impossible to fit
    //  one system in a page. We have to split system horizontally (some staves in
    //  one page and the others in next page).

    //TODO ************
    //By using m_uLastSystemHeight to determine if there is enough space we are
    //assuming that next system height will be equal to last finished system.

    return remaining_height() >= m_uLastSystemHeight;
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::remaining_height()
{
    return m_pCurBoxPage->get_height() - m_pageCursor.y;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::move_cursor_to_top_left_corner()
{
    m_pageCursor.x = m_pCurBoxPage->get_left();
    m_pageCursor.y = m_pCurBoxPage->get_top();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_score_titles()
{
    //TODO
    //m_pScore->LayoutAttachedObjects(m_pStubScore->GetCurrentPage(), m_pPaper);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::move_cursor_after_headers()
{
    //TODO
    move_cursor_to_top_left_corner();
    //m_pageCursor.y += m_pScore->GetHeadersHeight();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_system_layouter()
{
    m_sysLayouters.push_back(
        new SystemLayouter(m_rSpacingFactor, m_nSpacingMethod, m_nSpacingValue) );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_system_box()
{
    m_nColumnsInSystem = 0;

    //create system box
    m_pCurBoxSystem = m_pCurBoxPage->add_system(m_nCurSystem++);
    //m_pCurBoxSystem->SetFirstMeasure(m_nAbsColumn);

    m_pCurBoxSystem->set_origin(m_pageCursor.x, m_pageCursor.y);

    LUnits left = 0.0f; //TODO-LOG: pScore->get_system_left_space(iSystem);
    m_pCurBoxSystem->set_left_margin(left);

    m_pCurBoxSystem->set_width( m_pCurBoxPage->get_width() );

    //move x cursor to system left marging
    m_pageCursor.x += left;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::set_system_height_and_advance_paper_cursor()
{
    //TODO
//    //A system has been layouted. Update last system heigh record (without bottom space)
//    //and advance paper cursor to next system position
//
//    //AWARE: In GetSystemDistance() we are using m_nCurSystem instead of
//    //m_nCurSystem-1. This is to get the system distance between this system
//    //and next one.
//    LUnits uSystemBottomSpace = m_pScore->GetSystemDistance(m_nCurSystem, false) / 2.0;
//    m_pCurBoxSystem->SetBottomSpace(uSystemBottomSpace);
      m_uLastSystemHeight = m_pCurBoxSystem->get_height();

    //advance paper in system bottom space
    m_pageCursor.x = m_pCurBoxPage->get_left();
    m_pageCursor.y = m_pCurBoxSystem->get_bottom();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::fill_current_system_with_columns()
{
    m_nRelColumn = 0;
    do
    {
        create_column_and_add_it_to_current_system();
    }
    while(!m_scoreIt.is_end() && !must_terminate_system());

    more_systems_to_add( !m_scoreIt.is_end() );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::justify_current_system()
{
    compute_bar_sizes_to_justify_current_system();  //fThisIsLastSystem);
    reposition_staffobjs();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_column_and_add_it_to_current_system()
{
    //Creates a column and adds it to current system, if enough space.
    //The column is sized and this space discunted from available line space.
    //Returns true if either:
    //  - current system is completed,
    //  - there is not enough space tor include this column in current system, or
    //  - if a newSystem tag is found.
    //If not enough space for adding the column, SystemCursor is repositined again at
    //start of this column and nothing is added to current system.

//    //reposition paper vertically at the start of the system. It has been advanced
//    //when sizing the previous column
//    m_pageCursor.y = m_uStartOfCurrentSystem;
//

    create_column_boxes();
    must_terminate_system(false);
    collect_content_for_this_bar();
    measure_this_bar();

    if (is_first_column_in_system())
        m_uFreeSpace = get_available_space_in_system();

//    //check if there is enough space to add this column to current system
//    if(m_uFreeSpace < m_sysLayouters[m_nCurSystem-1]->GetMinimumSize(m_nRelColumn))
//	{
//        //there is no enough space for this column.
//
//        //restore cursors to re-process this column
//        m_pSysCursor->GoBackPrevPosition();
//
//        //if no column added to system, the line width is not enough for drawing
//        //just one measure or no measures in score (i.e. no time signature).
//        //We have to split the current column and reprocess it
//        if (m_nColumnsInSystem == 0)
//        {
//            //determine break time to split this column
//            SplitColumn(m_uFreeSpace);
//        }
//
//        //discard measurements for current column
//        m_sysLayouters[m_nCurSystem-1]->DiscardMeasurementsForColumn(m_nRelColumn);
//        m_pCurBoxSystem->DeleteLastSlice();
//
//        //if at least one column in current system, the system is finished
//        if (m_nColumnsInSystem > 0)
//            return true;    //terminate system
//    }
//    else
    {
        //there is enough space for this column. Add it to system
        add_column_to_system();
    }
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::get_available_space_in_system()
{
    //if this is the first column compute the space available in
    //this system. The method is a little tricky. The total space available
    //is (pScore->GetPageRightMargin() - pScore->GetCursorX()). But we have
    //to take into account the space that will be used by the prolog. As the
    //left position of the first column has taken all this into account,
    //it is posible to use that value by just doing:

    //ImoScore* pScore = get_imo_score();
    //return pScore->GetRightMarginXPos()
//                       - m_pScore->GetSystemLeftSpace(m_nCurSystem - 1)
//                       - m_sysLayouters[m_nCurSystem-1]->get_start_position_for_column(m_nRelColumn);
    return 16000.0f;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_slice_box()
{
    m_pCurSlice = m_pCurBoxSystem->add_slice(m_nAbsColumn);

	m_pCurSlice->set_left(m_pCurBoxSystem->get_content_left());
	m_pCurSlice->set_top(m_pCurBoxSystem->get_top());
	m_pCurSlice->set_width(m_pCurBoxSystem->get_content_width());

    m_pageCursor.y = m_pCurSlice->get_top();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_column_boxes()
{
    m_sliceInstrBoxes.clear();
    add_slice_box();
    ImoScore* pScore = get_imo_score();
    m_pCurBSI = NULL;
    ImoInstrument* pInstr = NULL;
    int maxInstr = pScore->get_num_instruments() - 1;

    for (int iInstr = 0; iInstr <= maxInstr; iInstr++)
    {
        m_pageCursor.x = m_pCurSlice->get_left();    //align start of all staves
        pInstr = pScore->get_instrument(iInstr);
        LUnits uMargin = determine_top_space(iInstr, pInstr);

        if (iInstr > 0)
            terminate_slice_instr(iInstr-1, uMargin);

        start_slice_instr(pInstr, iInstr, uMargin);
    }

    //set last SliceInstr height
    LUnits uBottomMargin = pInstr->get_staff(0)->get_staff_margin() / 2.0f;
    terminate_slice_instr(maxInstr, uBottomMargin);

    //set slice and system height
    LUnits uTotalHeight = m_pageCursor.y - m_pCurSlice->get_top();
    m_pCurSlice->set_height(uTotalHeight);
    if (is_first_column_in_system())
        m_pCurBoxSystem->set_height(uTotalHeight);
}

void ScoreLayouter::measure_this_bar()
{

        //    //restore x cursor to paper left margin
//    m_pageCursor.x = m_pScore->GetPageLeftMargin();
//
//    //all measures in column number m_nAbsColumn have been sized. The information is stored in
//    //object SystemLayouter. Now proced to re-position the StaffObjs so that all StaffObjs
//    //sounding at the same time will have the same x coordinate.
    bool fTrace = false;    //m_fDebugMode && (m_nTraceMeasure == 0  || m_nTraceMeasure == m_nAbsColumn);

    m_sysLayouters[m_nCurSystem-1]->end_of_system_measurements();
    m_sysLayouters[m_nCurSystem-1]->do_column_spacing(m_nRelColumn, fTrace);
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::determine_top_space(int nInstr, ImoInstrument* pInstr)
{
    if (nInstr == 0)
    {
        ImoScore* pScore = get_imo_score();
        if (is_first_system_in_page())
        {
            ImoSystemInfo* pInfo = pScore->get_first_system_info();
            return pInfo->get_top_system_distance();
        }
        else
        {
            ImoSystemInfo* pInfo = pScore->get_other_system_info();
            return pInfo->get_system_distance() / 2.0f;
        }
    }
    else
        return pInstr->get_staff(0)->get_staff_margin() / 2.0f;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::start_slice_instr(ImoInstrument* pInstr, int iInstr, LUnits uTopMargin)
{
    m_pCurBSI = m_pCurSlice->add_box_for_instrument(pInstr);
    m_sliceInstrBoxes.push_back( m_pCurBSI );
	m_pCurBSI->set_top( m_pageCursor.y );
	m_pCurBSI->set_left( m_pCurSlice->get_left() );
	m_pCurBSI->set_width( m_pCurSlice->get_width() );

    m_pageCursor.y += uTopMargin;

	if (is_first_column_in_system())
        add_staff_lines_name_and_bracket(iInstr, uTopMargin);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::terminate_slice_instr(int iInstr, LUnits uBottomMargin)
{
    m_pageCursor.y += uBottomMargin;
	m_pCurBSI->set_height( m_pageCursor.y - m_pCurBSI->get_top() );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_staff_lines_name_and_bracket(int iInstr, LUnits uTopMargin)
{
    LUnits indent = get_system_indent();
    InstrumentEngraver* engraver = get_instrument_engraver(iInstr);
    engraver->add_staff_lines(m_pCurBoxSystem, m_pageCursor.x, m_pageCursor.y, indent);
    engraver->add_name_abbrev(m_pCurBSI, m_nCurSystem);
    engraver->add_brace_bracket(m_pCurBSI);

    m_pageCursor.x += indent;
    m_pageCursor.y = engraver->get_staves_bottom();
}

//---------------------------------------------------------------------------------------
InstrumentEngraver* ScoreLayouter::get_instrument_engraver(int iInstr)
{
    std::vector<InstrumentEngraver*>::iterator it = m_instrEngravers.begin();
    for (; it != m_instrEngravers.end() && iInstr > 0; ++it, --iInstr);
    return *it;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::truncate_current_system()
{
    //system must be truncated at final barline if requested. Otherwise it must go
    //to right margin. Set here the applicable lenght.

//    if (fThisIsLastSystem && m_fStopStaffLinesAtFinalBarline)
//    {
//        //this is the last system and it has been requested to stop staff lines
//        //in last measure. So, set final x so staff lines go to final bar line
//        LUnits xFinalPos = 0.0f;
//        LUnits yFinalPos = 0.0f;
//        ImoInstrument *pI;
//        for (pI = m_pScore->GetFirstInstrument(); pI; pI=m_pScore->GetNextInstrument())
//        {
//            LUnits xPos, yPos;
//            pI->GetVStaff()->GetBarlineOfLastNonEmptyMeasure(&xPos, &yPos);
//            if (yPos > yFinalPos)
//            {
//                yFinalPos = yPos;
//                xFinalPos = xPos;
//            }
//        }
//        if (xFinalPos > 0.0f)
//            m_pCurBoxSystem->UpdateXRight( xFinalPos - 1 );
//        else
//            m_pCurBoxSystem->UpdateXRight( m_pScore->GetRightMarginXPos() );
//    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::collect_content_for_this_bar()  //ImoInstrument* pInstr, int iInstr)
{
    LUnits uInitialSpace = determine_initial_space();

    //ask system formatter to prepare to receive data for this instrument objects in this column
    LUnits uxStart = m_pageCursor.x;
    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
    pSysLayouter->start_bar_measurements(m_nRelColumn, uxStart, uInitialSpace);

    //The prolog (clef and key signature) must be rendered on each system, but the
    //matching StaffObjs only exist in the first system. In the first system the prolog
	//is rendered as part as the normal StaffObj rendering process, so there is nothig
	//special to do to render the prolog. But for the other systems we must force the
	//rendering of the prolog because there are no StaffObjs representing the prolog.
    bool fProlog = is_first_column_in_system();        //fProlog -> we are adding prolog objects
//    if (m_nAbsColumn != 1 && fProlog)
//	{
//		AddProlog(m_pCurBSI, false, pVStaff, nInstr);
//        fProlog = false;                    //prolog added
//	}

    //Pre-allocate common engravers
    ClefEngraver clefEngrv(m_libraryScope);


    //loop to process all StaffObjs in this measure
    bool fNewSystemTagFound = false;                //newSystem tag found
    ImoStaffObj* pSO = NULL;
    ImoScore* pScore = get_imo_score();
    //ScoreIterator sit = m_pSysCursor->get_iterator(iInstr);
////    m_scoreIt.ResetFlags();
    while(!m_scoreIt.is_end() )   //&& !m_scoreIt.change_of_measure())
    {
        pSO = dynamic_cast<ImoStaffObj*>( (*m_scoreIt)->imo_object() );
        int iInstr = (*m_scoreIt)->num_instrument();
        int iStaff = (*m_scoreIt)->staff();
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        m_pageCursor.y = m_instrEngravers[iInstr]->get_top_of_staff(iStaff);
        LUnits lineSpacing = pInstr->get_line_spacing_for_staff(iStaff);
        int iLine = (*m_scoreIt)->line();
        float rTime = (*m_scoreIt)->time();

//        if (pSO->IsBarline() || IsHigherTime(pSO->GetTimePos(), m_pSysCursor->GetBreakTime()) )
//        {
//             break;         //End of measure: exit loop.
//        }
//
//        else if (pSO->IsControl())
//        {
//            ESOCtrolType nCtrolType = ((lmSOControl*)pSO)->GetCtrolType();
//            if(lmNEW_SYSTEM == nCtrolType)
//            {
//                //new system tag found in this measure
//                fNewSystemTagFound = true;
//            }
//			else {
//				wxLogMessage(_T("ScoreLayouter::collect_content_for_this_bar] Bad SOControl type"));
//				wxASSERT(false);
//			}
//        }
//
        /*else*/ if (pSO->is_clef())
		{
			m_pageCursor.x = uxStart;
            ImoClef* pClef = dynamic_cast<ImoClef*>(pSO);
            GmoShape* pShape = clefEngrv.create_shape(pClef, m_pCurBSI, m_pageCursor,
                                                      lineSpacing);
            pSysLayouter->include_object(m_nRelColumn, iLine, iInstr, pInstr, pSO,
                                         -1.0f, fProlog, iStaff, pShape);
		}
//        else if (pSO->IsKeySignature())
//		{
//			m_pageCursor.x =uxStart);
//			AddKey((lmKeySignature*)pSO, m_pCurBSI, pVStaff, nInstr, fProlog);
//        }
//
//        else if (pSO->IsTimeSignature())
//		{
//			m_pageCursor.x =uxStart);
//			AddTime((TimeSignature*)pSO, m_pCurBSI, pVStaff, nInstr, fProlog);
//		}

		else
		{
//            //it is neither clef, key signature nor time signature. Finish prologue
//            fProlog = false;
//
//			//create this lmStaffObj shape and add to table
//			m_pageCursor.x =uxStart;
//			pSO->Layout(m_pCurBSI, m_pPaper);
//			GmoShape* pShape = pSO->GetShape();
//            pSysLayouter->IncludeObject(m_nRelColumn, iLine, nInstr, pSO, pShape, fProlog);
        }

        ++m_scoreIt;
    }

//    //The barline lmStaffObj is not included in the loop as it might not exist in the last
//    //bar of a score. In theses cases, the loop is exited because the end of the score is
//    //reached. In any case we have to close the line
//    if (pSO && pSO->IsBarline())
//    {
//        ++sit;    //leave cursor pointing to next measure
//
//        m_pageCursor.x =uxStart);
//        pSO->Layout(m_pCurBSI, m_pPaper);
//        GmoShape* pShape = pSO->GetShape();
//        pSysLayouter->include_barline_and_terminate_bar_measurements(m_nRelColumn, pSO, uxStart);
//    }
//    else
//	{
//        // no barline at the end of the measure.
        pSysLayouter->terminate_bar_measurements_without_barline(m_nRelColumn, uxStart, -1.0f);

//        //force new system if a break point reached
//        if (pSO && IsHigherTime(pSO->GetTimePos(), m_pSysCursor->GetBreakTime()))
//            fNewSystemTagFound = true;
//    }

    must_terminate_system( fNewSystemTagFound );
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::determine_initial_space()
{
    //LUnits uInitialSpace = 0.0f;
    //if (is_first_column_in_system())
    //{
    //    //TODO. Now a fixed value of 7.5 tenths is used. User options ?
    //
    //    Tenths rSpaceBeforeProlog = 7.5f;
    //	ImoInstrument* pInstr = m_pScore->GetFirstInstrument();
    //	lmVStaff* pVStaff = pInstr->GetVStaff();
    //	uInitialSpace = pVStaff->TenthsToLogical(rSpaceBeforeProlog, 1);
    //}
    //else
    //{
    //    //Not first measure of system. Get the previous barline and add some space if
    //    //the previous barline is visible.
    //    ImoBarline* pBar = m_pSysCursor->GetPreviousBarline(nInstr);
    //    if (pBar)
    //    {
    //        if (pBar->IsVisible())
    //            uInitialSpace = pVStaff->TenthsToLogical(20.0f);    // TODO: user options
    //    }
    //}
    //return uInitialSpace;
   return 200.0f;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_column_to_system()
{
    //Add column to current system and discount the space that the measure will take
    add_shapes_for_score_objs();
    m_uFreeSpace -= m_sysLayouters[m_nCurSystem-1]->get_minimum_size(m_nRelColumn);
    m_nColumnsInSystem++;
//    m_pSysCursor->CommitCursors();

//
//    //mark all objects in column as 'non dirty'
//    m_sysLayouters[m_nCurSystem-1]->ClearDirtyFlags(m_nRelColumn);

    //prepare to create a new column
    m_nRelColumn++;
    m_nAbsColumn++;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_shapes_for_score_objs()
{
    m_sysLayouters[m_nCurSystem-1]->add_shapes(m_sliceInstrBoxes);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::reposition_staffobjs()
{
    //SystemLayouter stores the final size that must have each column
    //of this system. This method changes StaffObjs locations so that they are evenly
    //distributed across the the bar.

    if (!m_sysLayouters[m_nCurSystem-1]->has_content())
        return;

    LUnits uxStartOfMeasure = m_sysLayouters[m_nCurSystem-1]->get_start_position_for_column(0);
    for (int i=0; i < m_nColumnsInSystem; i++)
    {
//        GmoBoxSlice* pBSlice = (GmoBoxSlice*)m_pCurBoxSystem->GetChildBox(i);
        uxStartOfMeasure = m_sysLayouters[m_nCurSystem-1]->redistribute_space(i, uxStartOfMeasure);
//        m_sysLayouters[m_nCurSystem-1]->AddTimeGridToBoxSlice(i, pBSlice);
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::compute_bar_sizes_to_justify_current_system()   //bool fThisIsLastSystem)
{
    //To compute_bar_sizes_to_justify_current_system divide up the remaining space
    //between all bars, except if current system is the last one and flag
    //"JustifyFinalBarline" is not set or there is no final barline.

//    if (!fThisIsLastSystem || (fThisIsLastSystem && m_fJustifyFinalBarline))
//        RedistributeFreeSpace(m_uFreeSpace, fThisIsLastSystem);
}







////-----------------------------------------------------------------------------------------
//// ScoreLayouter implementation OLD CODE TO REVIEW
////-----------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::SplitColumn(LUnits uAvailable)
//{
//    //We have measured a column and it doesn't fit in current system. Split it.
//    //All information about column to split is stored in the SystemLayouter.
//    //Parameter uAvailable is the available space in current system
//
//    float rTime;
//    LUnits uWidth;
//    if (m_sysLayouters[m_nCurSystem-1]->GetOptimumBreakPoint(m_nRelColumn, uAvailable, &rTime, &uWidth))
//    {
//        wxString sMsg = _("Program failure: not enough space for drawing just one bar.");
//        ::wxLogFatalError(sMsg);
//    }
//
//    m_pSysCursor->SetBreakTime(rTime);
//}
//
//---------------------------------------------------------------------------------------
//LUnits ScoreLayouter::AddEmptySystem(int nSystem, GmoBoxSystem* pBoxSystem)
//{
//    //Add staff lines to received GmoBoxSystem. Returns system height including bottom margin.
//    //Paper is left positioned at next staff position
//
//    LUnits uSystemHeight = - m_pageCursor.y;
//
//    // explore all instruments in the score
//    LUnits xStartPos = m_pageCursor.x;
//    LUnits uyBottom;
//    int nInstr = 0;
//    ImoInstrument* pInstr = m_pScore->GetFirstInstrument();
//    while (pInstr)
//    {
//        m_pageCursor.x = xStartPos );    //align start of staves in this system
//
//        LUnits yPaperPos = m_pageCursor.y;
//
//        lmVStaff* pVStaff = pInstr->GetVStaff();
//
//        //Top space
//        LUnits uTopMargin;
//        if (nInstr == 0)
//        {
//            //First instrument of this system
//            uTopMargin = pBoxSystem->get_top_space();
//
//        }
//        else
//        {
//            //Not first instrument of system. Split distance: half for each instrument
//            uTopMargin = pVStaff->GetFirstStaff()->GetStaffDistance() / 2.0f;
//
//            //advance paper to top limit of current instrument
//            m_pageCursor.y += uTopMargin;
//            yPaperPos = m_pageCursor.y;
//        }
//
//        //Here paper is positioned at top limit of current instrument
//
//        //advance paper to top line of first staff (instrument top bounds)
//        m_pageCursor.y += uTopMargin;
//        yPaperPos = m_pageCursor.y;
//
//        //Add staff lines for current instrument. As final xPos is yet unknown, so I use zero.
//		//It will be updated when the system is completed
//		uyBottom = pVStaff->LayoutStaffLines(pBoxSystem, pInstr, xStartPos,
//                                             0.0f, m_pageCursor.y);
//
//        //advance paper in height of this lmVStaff
//        m_pageCursor.y =uyBottom );
//
//        //proceed with next instrument
//        pInstr = m_pScore->GetNextInstrument();
//        ++nInstr;
//    }
//
//    //restore x cursor to paper left margin
//    m_pageCursor.x = m_pScore->GetPageLeftMargin() );
//
//    //set system bottom position
//    pBoxSystem->SetYBottom(uyBottom);
//
//
//    uSystemHeight += m_pageCursor.y;
//    return uSystemHeight;
//}
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::RedistributeFreeSpace(LUnits uAvailable, bool fLastSystem)
//{
//    //Step 3: Justify bars (distribute remainnig space across all bars in system)
//    //-------------------------------------------------------------------------------
//    //Redistributes the space to try to have all columns with equal witdh.
//    //
//    //on entering in this function:
//    // - object SystemLayouter stores the minimum size for each column for
//    //   the current system.
//    // - uAvailable stores the free space remaining at the end of this system
//    //
//    //on exit:
//    // - the values stored in SystemLayouter are modified to reflect the new size
//    //   for the bar columns, so that the line get justified.
//    //
//    //-------------------------------------------------------------------------------------
//
//    if (uAvailable <= 0.0f) return;       //no space to distribute
//
//    //The system must not be justified if this is the last system and there is no barline
//    //in the last bar. Check this.
//    SystemLayouter* pSysFmt = m_sysLayouters[m_nCurSystem-1];
//    if (fLastSystem && !pSysFmt->ColumnHasBarline(m_nColumnsInSystem-1))
//            return;     //no need to justify
//
//   //compute average column size and total occupied
//    LUnits uTotal = 0.0f;
//    for (int i = 0; i < m_nColumnsInSystem; i++)
//    {
//         uTotal += pSysFmt->GetMinimumSize(i);
//    }
//    LUnits uAverage = (uTotal + uAvailable) / m_nColumnsInSystem;
//
//    //for each column, compute the diference between its size and the average target size
//    //sum up all the diferences in uDifTotal
//    std::vector<LUnits> uDif(m_nColumnsInSystem, 0.0f);
//    LUnits uDifTotal = 0;
//    int nNumSmallerColumns = 0;      //num of columns smaller than average
//    for (int i = 0; i < m_nColumnsInSystem; i++)
//    {
//        uDif[i] = uAverage - pSysFmt->GetMinimumSize(i);
//        if (uDif[i] > 0.0f)
//        {
//            uDifTotal += uDif[i];
//            nNumSmallerColumns++;
//        }
//    }
//
//    //distribute space
//    if (uDifTotal > uAvailable)
//    {
//        //not enough space to make all equal
//        LUnits uReduce = (uDifTotal - uAvailable) / nNumSmallerColumns;
//        for (int i = 0; i < m_nColumnsInSystem; i++)
//        {
//            if (uDif[i] > 0.0f)
//            {
//                uDif[i] -= uReduce;
//                pSysFmt->IncrementColumnSize(i, uDif[i]);
//            }
//        }
//    }
//    else
//    {
//        //enough space to make all columns equal size
//        for (int i = 0; i < m_nColumnsInSystem; i++)
//        {
//            if (uDif[i] > 0.0f)
//            {
//                pSysFmt->IncrementColumnSize(i, uDif[i]);
//            }
//        }
//    }
//
//}
//
//
////=========================================================================================
//// Methods to deal with measures
////=========================================================================================
//---------------------------------------------------------------------------------------
//void ScoreLayouter::AddProlog(GmoBoxSliceInstr* pBSI, bool fDrawTimekey, lmVStaff* pVStaff,
//                             int nInstr)
//{
//    // The prolog (clef and key signature) must be rendered on each system,
//    // but the matching StaffObjs only exist in the first system. Therefore, in the
//    // normal staffobj rendering process, the prolog would be rendered only in
//    // the first system.
//    // So, for the other systems it is necessary to force the rendering
//    // of the prolog because there are no StaffObjs representing it.
//    // This method does it.
//    //
//    // To know what clef, key and time signature to draw we take this information from the
//    // context associated to first note of the measure on each staff. If there are no notes,
//    // the context is taken from the barline. If, finally, no context is found, no prolog
//    // is drawn.
//
//    LUnits uPrologWidth = 0.0f;
//    lmClef* pClef = (lmClef*)NULL;
//    lmEClefType nClef = lmE_Undefined;
//    lmKeySignature* pKey = (lmKeySignature*)NULL;
//    TimeSignature* pTime = (TimeSignature*)NULL;
//
//    //AWARE when this method is invoked the paper position must be at the left marging,
//    //at the start of a new system.
//    LUnits xStartPos = m_pageCursor.x;      //Save x to align all clefs
//    LUnits yStartPos = m_pageCursor.y;
//
//    //iterate over the collection of lmStaff objects to draw current clef and key signature
//
//    lmStaff* pStaff = pVStaff->GetFirstStaff();
//    LUnits uyOffset = 0.0f;
//    LUnits xPos = 0.0f;
//
//    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
//    lmContext* pContext = (lmContext*)NULL;
//    for (int nStaff=1; nStaff <= pVStaff->GetNumStaves(); pStaff = pVStaff->GetNextStaff(), nStaff++)
//    {
//        xPos = xStartPos;
//        if (nStaff > 1)
//            uyOffset += pStaff->GetStaffDistance();
//
//            //locate context for first note in this staff
//        pContext = m_pSysCursor->GetStartOfColumnContext(nInstr, nStaff);
//
//        if (pContext)
//        {
//            pClef = pContext->GetClef();
//            pKey = pContext->GetKey();
//            pTime = pContext->GetTime();
//
//            //render clef
//            if (pClef)
//            {
//                nClef = pClef->GetClefType();
//				if (pClef->IsVisible())
//                {
//					lmUPoint uPos = lmUPoint(xPos, yStartPos+uyOffset);        //absolute position
//					GmoShape* pShape = pClef->CreateShape(pBSI, m_pPaper, uPos);
//                    pShape->SetShapeLevel(lm_ePrologShape);
//					xPos += pShape->GetWidth();
//                    pSysLayouter->IncludeObject(m_nRelColumn, nInstr, pClef, pShape, true);
//				}
//            }
//
//            //render key signature
//            if (pKey && pKey->IsVisible())
//            {
//                lmUPoint uPos = lmUPoint(xPos, yStartPos+uyOffset);        //absolute position
//                GmoShape* pShape = pKey->CreateShape(pBSI, m_pPaper, uPos, nClef, pStaff);
//                pShape->SetShapeLevel(lm_ePrologShape);
//				xPos += pShape->GetWidth();
//                pSysLayouter->IncludeObject(m_nRelColumn, nInstr, pKey, pShape, true, nStaff);
//            }
//
//        }
//
//        //compute prolog width
//        uPrologWidth = wxMax(uPrologWidth, xPos - xStartPos);
//
//        //compute vertical displacement for next staff
//        uyOffset += pStaff->GetHeight();
//
//    }
//
//    // update paper cursor position
//    m_pageCursor.x =xStartPos + uPrologWidth);
//
//}
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::AddKey(lmKeySignature* pKey, Box* pBox,
//						  lmVStaff* pVStaff, int nInstr, bool fProlog)
//{
//    // This method is responsible for creating the key signature shapes for
//    // all staves of this instrument. And also, of adding them to the graphical
//    // model and to the Timepos table
//
//    //create the shapes
//    pKey->Layout(pBox, m_pPaper);
//
//	//add the shapes to the timepos table
//    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
//	GmoShape* pMainShape = ((lmStaffObj*)pKey)->GetShape();          //cast forced because otherwise the compiler complains
//    for (int nStaff=1; nStaff <= pVStaff->GetNumStaves(); nStaff++)
//    {
//        GmoShape* pShape = pKey->GetShape(nStaff);
//        pSysLayouter->IncludeObject(m_nRelColumn, nInstr, pKey, pShape, fProlog, nStaff);
//    }
//
//}
//
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::AddTime(TimeSignature* pTime, Box* pBox,
//						   lmVStaff* pVStaff, int nInstr, bool fProlog)
//{
//    // This method is responsible for creating the time signature shapes for
//    // all staves of this instrument. And also, of adding them to the graphical
//    // model and to the Timepos table
//
//    //create the shapes
//    pTime->Layout(pBox, m_pPaper);
//
//	//add the shapes to the timepos table
//    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
//	GmoShape* pMainShape = ((lmStaffObj*)pTime)->GetShape();          //cast forced because otherwise the compiler complains
//    for (int nStaff=1; nStaff <= pVStaff->GetNumStaves(); nStaff++)
//    {
//        GmoShape* pShape = pTime->GetShape(nStaff);
//        pSysLayouter->IncludeObject(m_nRelColumn, nInstr, pTime, pShape, fProlog, nStaff);
//    }
//
//}
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::PrepareFontsThatMatchesStavesSizes()
//{
//    //for each staff size, setup fonts of right point size for that staff size
//    ImoInstrument *pInstr;
//    for (pInstr = m_pScore->GetFirstInstrument(); pInstr; pInstr=m_pScore->GetNextInstrument())
//    {
//        pInstr->GetVStaff()->SetUpFonts(m_pPaper);
//    }
//}
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::AddInitialLineJoiningAllStavesInSystem()
//{
//    //TODO: In current code, the decision about joining staves depends only on first
//    //instrument. This should be changed and the line should go from first visible
//    //staff to last visible one.
//
//    //TODO: For empty scores (no staffobj on any instrument) this initial line should
//    //not be drawn
//
//	//Add the shape for the initial barline that joins all staves in a system
//    lmVStaff* pVStaff = m_pScore->GetFirstInstrument()->GetVStaff();
//	if (m_pScore->GetOptionBool(_T("Staff.DrawLeftBarline")) && !pVStaff->HideStaffLines() )
//	{
//		LUnits uxPos = m_pCurBoxSystem->get_left() + get_system_indent();
//		LUnits uLineThickness = lmToLogicalUnits(0.2, lmMILLIMETERS);        // thin line width will be 0.2 mm TODO user options
//        GmoShapeSimpleLine* pLine =
//            new GmoShapeSimpleLine(pVStaff, uxPos, m_pCurBoxSystem->GetYTopFirstStaff(),
//						uxPos, m_pCurBoxSystem->GetYBottom(),
//						uLineThickness, 0.0, *wxBLACK, _T("System joining line"),
//						lm_eEdgeHorizontal);
//	    m_pCurBoxSystem->AddShape(pLine, lm_eLayerStaff);
//	}
//}
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::UpdateBoxSlicesSizes()
//{
//    //update BoxSlices with the final measures sizes, except for last
//    //measure, as its length has been already set up
//
//    LUnits xEnd = m_sysLayouters[m_nCurSystem-1]->GetStartPositionForColumn(0);
//    for (int iRel=0; iRel < m_nColumnsInSystem; iRel++)
//    {
//        LUnits xStart = xEnd;
//        xEnd = xStart + m_sysLayouters[m_nCurSystem-1]->GetMinimumSize(iRel);
//        GmoBoxSlice* pBoxSlice = m_pCurBoxSystem->get_slice(iRel);
//		pBoxSlice->UpdateXLeft(xStart);
//        if (iRel < m_nColumnsInSystem)
//			pBoxSlice->UpdateXRight(xEnd);
//    }
//}
//
//---------------------------------------------------------------------------------------
//bool ScoreLayouter::RequestedToFillScoreWithEmptyStaves()
//{
//    return (!m_fStopStaffLinesAtFinalBarline
//            && m_pScore->GetOptionBool(_T("Score.FillPageWithEmptyStaves")) );
//}
//
//---------------------------------------------------------------------------------------
//void ScoreLayouter::FillPageWithEmptyStaves()
//{
//    //First system has been always added before arriving here. Fill the remaining
//    //page space with empty staves
//
//    //advance vertically the previous system bottom space
//    m_pageCursor.y +=  m_pScore->GetSystemDistance(m_nCurSystem, false) / 2.0 ;
//
//    while (true)      //loop is exited when reaching end of page
//    {
//        //Here Paper is positioned at the start of the new current system.
//
//        bool fFirstSystemInPage = false;
//
//        //TODO ************
//        //  By using nSystemHeight we are assuming that next system height is going
//        //  to be equal to last finished system. In this test it is necessary
//        //  to compute and use next system height
//        LUnits nNextSystemHeight = m_uLastSystemHeight;
//        LUnits yNew = m_pageCursor.y + nNextSystemHeight;
//        if ( yNew > m_pScore->GetMaximumY() )
//            break;        //exit loop
//
//        //create the system container
//        m_uStartOfCurrentSystem = m_pageCursor.y;      //save start of system position
//        m_pCurBoxSystem =
//            m_pCurBoxPage->add_system(m_nCurSystem, m_pageCursor.x,
//                                         m_uStartOfCurrentSystem, fFirstSystemInPage);
//        m_pCurBoxSystem->SetFirstMeasure(m_nAbsColumn);
//        m_pageCursor.x += m_pCurBoxSystem->get_left_margin();
//        m_pCurBoxSystem->SetIndent(get_system_indent());
//
//
//        m_nRelColumn = 0;          //first column of this system
//        m_nAbsColumn++;
//        m_uLastSystemHeight = AddEmptySystem(m_nCurSystem, m_pCurBoxSystem);     //Add the staff lines
//
//        //staff lines go to the rigth margin
//        m_pCurBoxSystem->UpdateXRight( m_pScore->GetRightMarginXPos() );
//
//        // compute system bottom space
//        //AWARE:
//        //  In GetSystemDistance() we are using m_nCurSystem instead of m_nCurSystem-1.
//        //  This is to get the system distance between this system and next one.
//        LUnits uSystemBottomSpace = m_pScore->GetSystemDistance(m_nCurSystem, false) / 2.0;
//        m_pCurBoxSystem->SetBottomSpace(uSystemBottomSpace);
//
//        //advance paper in system bottom space
//        m_pageCursor.y += uSystemBottomSpace;
//
//        //increment loop information
//        m_nCurSystem++;
//    }
//}
//
////=========================================================================================
//// methods coded only for Unit Tests
////=========================================================================================

//int ScoreLayouter::GetNumSystemLayouters() { return (int)m_sysLayouters.size(); }
//int ScoreLayouter::GetNumColumns(int iSys) { return m_sysLayouters[iSys]->GetNumColumns(); }
//int ScoreLayouter::GetNumLines(int iSys, int iCol)
//        { return m_sysLayouters[iSys]->GetNumLinesInColumn(iCol); }
//
//SystemLayouter* ScoreLayouter::get_system_layouter(int iSys)
//{
//    //iSys=[0..n-1]
//    return m_sysLayouters[iSys];
//}
// any gain?
//m_sysLayouters[m_nCurSystem-1]
//get_system_layouter(m_nCurSystem-1)
//get_current_system_layouter()

}  //namespace lomse
