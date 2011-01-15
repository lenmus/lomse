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

#include "lomse_score_layouter.h"

#include "lomse_basic_model.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_box_slice_instr.h"
#include "lomse_shape_staff.h"
#include "lomse_shapes.h"
#include "lomse_instrument_engraver.h"
#include "lomse_engraving_options.h"
#include "lomse_system_layouter.h"
#include "lomse_barline_engraver.h"
#include "lomse_clef_engraver.h"
#include "lomse_note_engraver.h"
#include "lomse_system_cursor.h"
#include "lomse_shape_barline.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// ScoreLayouter implementation
//---------------------------------------------------------------------------------------

ScoreLayouter::ScoreLayouter(ImoDocObj* pImo, GraphicModel* pGModel,
                             LibraryScope& libraryScope)
    : ContentLayouter(pImo, pGModel)
    , m_libraryScope(libraryScope)
    , m_pSysCursor(NULL)
    , m_pScoreMeter( new ScoreMeter(get_imo_score()) )
    , m_pStubScore(NULL)
    , m_pCurBoxPage(NULL)
    , m_pCurBoxSystem(NULL)
{
}

//---------------------------------------------------------------------------------------
ScoreLayouter::~ScoreLayouter()
{
    delete_system_cursor();
    delete_instrument_engravers();
    delete_system_layouters();
    delete m_pScoreMeter;
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
    create_system_cursor();

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
void ScoreLayouter::delete_instrument_engravers()
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
    //layout_in_page() method must always continue layouting from current state.

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
    GmoBox* pBox = new GmoBoxScorePage(m_pStubScore);
    pParentBox->add_child_box(pBox);
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
    create_system_box();
    fill_current_system_with_columns();
    justify_current_system();

    if (!more_systems_to_add() && m_fStopStaffLinesAtFinalBarline)
        truncate_current_system();
    add_initial_line_joining_all_staves_in_system();
    update_box_slices_sizes();
    set_system_height_and_advance_paper_cursor();
    is_first_system_in_page(false);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_system_cursor()
{
    delete_system_cursor();
    m_pSysCursor = new SystemCursor( get_imo_score() );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::delete_system_cursor()
{
    delete m_pSysCursor;
}

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
    m_sysLayouters.push_back( new SystemLayouter(m_pScoreMeter) );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_system_box()
{
    m_nColumnsInSystem = 0;
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
    while(!m_pSysCursor->is_end() && !must_terminate_system());

    more_systems_to_add( !m_pSysCursor->is_end() );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::justify_current_system()
{
    if (system_must_be_justified())
        redistribute_free_space();  //!more_systems_to_add());

    reposition_staffobjs();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_column_and_add_it_to_current_system()
{
    //Creates a column and adds it to current system, if enough space.
    //The column is sized and this space discunted from available line space.
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

    //check if there is enough space to add this column to current system
    if(m_uFreeSpace < m_sysLayouters[m_nCurSystem-1]->get_minimum_size(m_nRelColumn))
	{
        //there is no enough space for this column.

        //restore cursors to re-process this column
        m_pSysCursor->go_back_to_saved_position();

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

        //if at least one column in current system, the system is finished
        if (m_nColumnsInSystem > 0)
        {
            must_terminate_system(true);
            return;
        }
    }
    else
    {
        //there is enough space for this column. Add it to system
        add_column_to_system();
    }
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::get_available_space_in_system()
{
    InstrumentEngraver* engraver = get_instrument_engraver(0);
    return engraver->get_staves_width() - space_used_by_prolog();
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::space_used_by_prolog()
{
    return m_sysLayouters[m_nCurSystem-1]->get_prolog_width();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_column_boxes()
{
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

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_slice_box()
{
    m_pCurSlice = m_pCurBoxSystem->add_slice(m_nAbsColumn);

	m_pCurSlice->set_left(m_pCurBoxSystem->get_content_left());
	m_pCurSlice->set_top(m_pCurBoxSystem->get_top());
	m_pCurSlice->set_width(m_pCurBoxSystem->get_content_width());

    m_pageCursor.y = m_pCurSlice->get_top();

    m_sysLayouters[m_nCurSystem-1]->prepare_for_new_column(m_pCurSlice);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::measure_this_bar()
{

        //    //restore x cursor to paper left margin
//    m_pageCursor.x = m_pScore->GetPageLeftMargin();
//
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
    m_pCurBSI = m_sysLayouters[m_nCurSystem-1]->create_slice_instr(m_nRelColumn,
                                                                   pInstr,
                                                                   m_pageCursor.y);
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

    //TODO
    if (!more_systems_to_add() && m_fStopStaffLinesAtFinalBarline)
    {
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
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::update_box_slices_sizes()
{
//    //update BoxSlices with the final measures sizes, except for last
//    //measure, as its length has been already set up
//
    //TODO
//    LUnits xEnd = m_sysLayouters[m_nCurSystem-1]->GetStartPositionForColumn(0);
//    for (int iRel=0; iRel < m_nColumnsInSystem; iRel++)
//    {
//        LUnits xStart = xEnd;
//        xEnd = xStart + m_sysLayouters[m_nCurSystem-1]->get_minimum_size(iRel);
//        GmoBoxSlice* pBoxSlice = m_pCurBoxSystem->get_slice(iRel);
//		pBoxSlice->UpdateXLeft(xStart);
//        if (iRel < m_nColumnsInSystem)
//			pBoxSlice->UpdateXRight(xEnd);
//    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::collect_content_for_this_bar()
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
    std::vector<bool> fAddingProlog;
	ImoScore* pScore = get_imo_score();
	int numInstruments = pScore->get_num_instruments();
    fAddingProlog.reserve(numInstruments);
    fAddingProlog.assign(numInstruments, is_first_column_in_system());
    if (m_nAbsColumn != 1 && is_first_column_in_system())
	{
	    for (int iInstr=0; iInstr < numInstruments; ++iInstr)
	    {
            add_prolog(iInstr); //, m_pCurBSI[iInstr]);
            fAddingProlog[iInstr] = false;
	    }
	}

    //loop to process all StaffObjs in this measure
    bool fNewSystemTagFound = false;
    bool fEndOfBarFound = false;
    ImoStaffObj* pSO = NULL;
////    m_pSysCursor->ResetFlags();
    while(!m_pSysCursor->is_end() )   //&& !m_pSysCursor->change_of_measure())
    {
        pSO = m_pSysCursor->get_staffobj();
        int iInstr = m_pSysCursor->num_instrument();
        int iStaff = m_pSysCursor->staff();
        int iLine = m_pSysCursor->line();
        float rTime = m_pSysCursor->time();
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        m_pageCursor.y = m_instrEngravers[iInstr]->get_top_of_staff(iStaff);
//        LUnits lineSpacing = pInstr->get_line_spacing_for_staff(iStaff);
        LUnits lineSpacing = m_pScoreMeter->line_spacing_for_instr_staff(iInstr, iStaff);

        if (!pSO->is_barline() && fEndOfBarFound)
            break;

        if (pSO->is_barline())  //TODO: || IsHigherTime(pSO->GetTimePos(), m_pSysCursor->GetBreakTime()) )
        {
            GmoShape* pShape = create_staffobj_shape(pSO, lineSpacing, iInstr);
            pSysLayouter->include_barline_and_terminate_bar_measurements(m_nRelColumn,
                                pSO, pShape, uxStart, rTime);
            fEndOfBarFound = true;
        }

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
        else if (pSO->is_clef())
		{
            GmoShape* pShape = create_staffobj_shape(pSO, lineSpacing, iInstr);
            pSysLayouter->include_object(m_nRelColumn, iLine, iInstr, pInstr, pSO,
                                         -1.0f, fAddingProlog[iInstr], iStaff, pShape);
		}

        else if (pSO->is_key_signature())
		{
			add_key_signature(pSO, lineSpacing, iInstr, fAddingProlog[iInstr]);
        }

        else if (pSO->is_time_signature())
		{
			add_time_signature(pSO, lineSpacing, iInstr, fAddingProlog[iInstr]);
		}

		else
		{
            //it is neither clef, key signature nor time signature. Finish prologue
            //for current instrument
            fAddingProlog[iInstr] = false;

            GmoShape* pShape = create_staffobj_shape(pSO, lineSpacing, iInstr);
            pSysLayouter->include_object(m_nRelColumn, iLine, iInstr, pInstr, pSO,
                                         rTime, false, iStaff, pShape);
        }

        m_pSysCursor->move_next();
    }

    if (!fEndOfBarFound)
    {
        //The loop is exited because the end of the score is
        //reached without a barline or because line break found.
        //We have to close the last line
        pSysLayouter->terminate_bar_measurements_without_barline(m_nRelColumn, uxStart, -1.0f);

//        //force new system if a break point reached
//        if (pSO && IsHigherTime(pSO->GetTimePos(), m_pSysCursor->GetBreakTime()))
//            fNewSystemTagFound = true;
    }

    must_terminate_system( fNewSystemTagFound );
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::determine_initial_space()
{
    //AWARE: Thie initial space is controlled by first staff in system. It cannot
    //be independent for ecah staff, because then objects will not be aligned.

    if (is_first_column_in_system())
    {
    	return m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_BEFORE_PROLOG, 0, 0);
    }
    else
    {
        ImoBarline* pBar = m_pSysCursor->get_previous_barline();
        if (pBar && pBar->is_visible())
            return m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_AFTER_BARLINE, 0, 0);
        else
            return 0.0f;
    }
}

//---------------------------------------------------------------------------------------
GmoShape* ScoreLayouter::create_staffobj_shape(ImoStaffObj* pSO, LUnits lineSpacing,
                                               int iInstr)
{
    //factory method to create shapes for staffobjs

    switch (pSO->get_obj_type())
    {
        case ImoObj::k_barline:
        {
            ImoBarline* pImo = dynamic_cast<ImoBarline*>(pSO);
            InstrumentEngraver* pInstrEngrv = get_instrument_engraver(iInstr);
            LUnits yTop = pInstrEngrv->get_staves_top();
            LUnits yBottom = pInstrEngrv->get_staves_bottom();
            BarlineEngraver engrv(m_libraryScope);
            return engrv.create_shape(pImo, m_pageCursor.x, yTop, yBottom, lineSpacing);
        }
        case ImoObj::k_clef:
        {
            ImoClef* pImo = dynamic_cast<ImoClef*>(pSO);
            ClefEngraver engrv(m_libraryScope);
            return engrv.create_shape(pImo, m_pCurBSI, m_pageCursor, lineSpacing);
        }
        //case ImoObj::k_key_signature:
        //{
        //    ImoKeySignature* pImo = dynamic_cast<ImoKeySignature*>(pSO);
        //    KeyEngraver engrv(m_libraryScope);
        //    return engrv.create_shape(pImo, m_pCurBSI, m_pageCursor, lineSpacing);
        //}
        case ImoObj::k_note:
        {
            ImoNote* pImo = dynamic_cast<ImoNote*>(pSO);
            NoteEngraver engrv(m_libraryScope);
            int clefType = m_pSysCursor->get_applicable_clef_type();
            return engrv.create_shape(pImo, clefType, m_pageCursor, lineSpacing);    //m_pCurBSI,
        }
        default:
            return new GmoShapeInvisible(0, m_pageCursor, USize(0.0, 0.0));
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_column_to_system()
{
    //Add column to current system and discount the space that the measure will take
    add_shapes_for_score_objs(m_nRelColumn);
    m_uFreeSpace -= m_sysLayouters[m_nCurSystem-1]->get_minimum_size(m_nRelColumn);
    m_nColumnsInSystem++;
    m_pSysCursor->save_position();

//
//    //mark all objects in column as 'non dirty'
//    m_sysLayouters[m_nCurSystem-1]->ClearDirtyFlags(m_nRelColumn);

    //prepare to create a new column
    m_nRelColumn++;
    m_nAbsColumn++;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_shapes_for_score_objs(int iCol)
{
    m_sysLayouters[m_nCurSystem-1]->add_shapes_to_column(iCol); //, m_sliceInstrBoxes[iCol]);
}

//---------------------------------------------------------------------------------------
bool ScoreLayouter::system_must_be_justified()
{
    //all systems needs justification except:

    //1. it is the last system and flag "JustifyFinalBarline" is not set
    if (!more_systems_to_add() && !m_fJustifyFinalBarline)
        return false;

    //2. it is the last system but there is no final barline
    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
    if (!more_systems_to_add() && !pSysLayouter->column_has_barline(m_nColumnsInSystem-1))
            return false;

    return true;        //do justification
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::redistribute_free_space()
{
    //Space is redistributed to try to have all columns with equal witdh.

    if (m_uFreeSpace <= 0.0f)
        return;           //no space to distribute

   //compute average column size and total occupied
    LUnits uTotal = 0.0f;
    for (int i = 0; i < m_nColumnsInSystem; i++)
    {
         uTotal += m_sysLayouters[m_nCurSystem-1]->get_minimum_size(i);
    }
    LUnits uAverage = (uTotal + m_uFreeSpace) / m_nColumnsInSystem;

    //For each column, compute the diference between its size and the average target size.
    //Sum up all the diferences in uDifTotal
    std::vector<LUnits> uDif(m_nColumnsInSystem, 0.0f);
    LUnits uDifTotal = 0;
    int nNumSmallerColumns = 0;      //num of columns smaller than average
    for (int i = 0; i < m_nColumnsInSystem; i++)
    {
        uDif[i] = uAverage - m_sysLayouters[m_nCurSystem-1]->get_minimum_size(i);
        if (uDif[i] > 0.0f)
        {
            uDifTotal += uDif[i];
            nNumSmallerColumns++;
        }
    }

    //distribute space
    if (uDifTotal > m_uFreeSpace)
    {
        //not enough space to make all equal
        LUnits uReduce = (uDifTotal - m_uFreeSpace) / nNumSmallerColumns;
        for (int i = 0; i < m_nColumnsInSystem; i++)
        {
            if (uDif[i] > 0.0f)
            {
                uDif[i] -= uReduce;
                m_sysLayouters[m_nCurSystem-1]->increment_column_size(i, uDif[i]);
            }
        }
    }
    else
    {
        //enough space to make all columns equal size
        for (int i = 0; i < m_nColumnsInSystem; i++)
        {
            if (uDif[i] > 0.0f)
            {
                m_sysLayouters[m_nCurSystem-1]->increment_column_size(i, uDif[i]);
            }
        }
    }

}

//---------------------------------------------------------------------------------------
void ScoreLayouter::reposition_staffobjs()
{
    //SystemLayouter stores the final size that must have each column
    //of this system. This method changes StaffObjs locations so that they are evenly
    //distributed across the the bar.

    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
    if (!pSysLayouter->has_content())
        return;

    LUnits uxStartOfMeasure = pSysLayouter->get_start_position_for_column(0);
    for (int i=0; i < m_nColumnsInSystem; i++)
    {
//        GmoBoxSlice* pBSlice = (GmoBoxSlice*)m_pCurBoxSystem->GetChildBox(i);
        uxStartOfMeasure = pSysLayouter->redistribute_space(i, uxStartOfMeasure);
//        pSysLayouter->AddTimeGridToBoxSlice(i, pBSlice);
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_initial_line_joining_all_staves_in_system()
{
    //TODO: In current code, the decision about joining staves depends only on first
    //instrument. This should be changed and the line should go from first visible
    //staff to last visible one.

    if (m_pSysCursor->is_empty_score())
        return;

    //TODO
//	//Add the shape for the initial barline that joins all staves in a system
//    lmVStaff* pVStaff = m_pScore->GetFirstInstrument()->GetVStaff();
	if (m_pScoreMeter->must_draw_left_barline())    //&& !pVStaff->HideStaffLines() )
	{
        LUnits xPos = m_instrEngravers[0]->get_staves_left();
        LUnits yTop = m_instrEngravers[0]->get_staves_top();
        int iInstr = m_pScoreMeter->num_instruments() - 1;
        LUnits yBottom = m_instrEngravers[iInstr]->get_staves_bottom();
        LUnits uLineThickness =
            m_pScoreMeter->tenths_to_logical(LOMSE_THIN_LINE_WIDTH, 0, 0);
        GmoShape* pLine = new GmoShapeBarline(0, ImoBarline::k_simple, xPos, yTop,
                                            yBottom, uLineThickness, uLineThickness,
                                            0.0f, 0.0f, Color(0,0,0), uLineThickness);
        m_pCurBoxSystem->add_shape(pLine, GmoShape::k_layer_staff);
	}
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_key_signature(ImoStaffObj* pSO, LUnits lineSpacing,
                                      int iInstr, bool fProlog)
{
//    // This method is responsible for creating the key signature shapes for
//    // all staves of this instrument. And also, of adding them to the graphical
//    // model and to the Timepos table
//
//    //create the shapes
//    pKey->Layout(pBox, m_pPaper);
//
//	//add the shapes to the timepos table
//    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
//	GmoShape* pMainShape = ((ImoStaffObj*)pKey)->GetShape();          //cast forced because otherwise the compiler complains
//    for (int nStaff=1; nStaff <= pVStaff->GetNumStaves(); nStaff++)
//    {
//        GmoShape* pShape = pKey->GetShape(nStaff);
//        pSysLayouter->IncludeObject(m_nRelColumn, nInstr, pKey, pShape, fProlog, nStaff);
//    }
//            GmoShape* pShape = create_staffobj_shape(pSO, lineSpacing, iInstr);
//            pSysLayouter->include_object(m_nRelColumn, iLine, iInstr, pInstr, pSO,
//                                         rTime, false, iStaff, pShape);

}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_time_signature(ImoStaffObj* pSO, LUnits lineSpacing,
                                       int iInstr, bool fProlog)
{
//    // This method is responsible for creating the time signature shapes for
//    // all staves of this instrument. And also, of adding them to the graphical
//    // model and to the Timepos table
//
//    //create the shapes
//    pTime->Layout(pBox, m_pPaper);
//
//	//add the shapes to the timepos table
//    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
//	GmoShape* pMainShape = ((ImoStaffObj*)pTime)->GetShape();          //cast forced because otherwise the compiler complains
//    for (int nStaff=1; nStaff <= pVStaff->GetNumStaves(); nStaff++)
//    {
//        GmoShape* pShape = pTime->GetShape(nStaff);
//        pSysLayouter->IncludeObject(m_nRelColumn, nInstr, pTime, pShape, fProlog, nStaff);
//    }
//            GmoShape* pShape = create_staffobj_shape(pSO, lineSpacing, iInstr);
//            pSysLayouter->include_object(m_nRelColumn, iLine, iInstr, pInstr, pSO,
//                                         rTime, false, iStaff, pShape);

}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_prolog(int iInstr)
{
    // The prolog (clef and key signature) must be rendered on each system,
    // but the matching StaffObjs only exist in the first system. Therefore, in the
    // normal staffobj rendering process, the prolog would be rendered only in
    // the first system.
    // So, for the other systems it is necessary to force the rendering
    // of the prolog because there are no StaffObjs representing it.
    // This method does it.


    LUnits uPrologWidth = 0.0f;
//    ImoKeySignature* pKey = NULL;
//    ImoTimeSignature* pTime = NULL;
//
//    //AWARE when this method is invoked the paper position must be at the left marging,
//    //at the start of a new system.
    LUnits xStartPos = m_pageCursor.x;      //Save x to align all clefs
//    LUnits yStartPos = m_pageCursor.y;

    //iterate over the collection of lmStaff objects to draw current clef and key signature
    ImoScore* pScore = get_imo_score();
    ImoInstrument* pInstr = pScore->get_instrument(iInstr);
//    lmStaff* pStaff = pVStaff->GetFirstStaff();
//    LUnits uyOffset = 0.0f;
    LUnits xPos = 0.0f;

    SystemLayouter* pSysLayouter = m_sysLayouters[m_nCurSystem-1];
    for (int iStaff=0; iStaff < pInstr->get_num_staves(); ++iStaff)    //pStaff = pVStaff->GetNextStaff(), iStaff++)
    {
        xPos = xStartPos;
//        if (iStaff > 1)
//            uyOffset += pStaff->GetStaffDistance();
//
        ColStaffObjsEntry* pClefEntry =
            m_pSysCursor->get_clef_entry_for_instr_staff(iInstr, iStaff);
//            pKey = pContext->GetKey();
//            pTime = pContext->GetTime();
//
        //add clef shape
        if (pClefEntry)
        {
            ImoClef* pClef = dynamic_cast<ImoClef*>( pClefEntry->imo_object() );
            if (pClef && pClef->is_visible())
            {
                int iLine = pClefEntry->line();
                m_pageCursor.x = xPos;
                m_pageCursor.y = m_instrEngravers[iInstr]->get_top_of_staff(iStaff);
                LUnits lineSpacing =
                    m_pScoreMeter->line_spacing_for_instr_staff(iInstr, iStaff);
                GmoShape* pShape = create_staffobj_shape(pClef, lineSpacing, iInstr);
                pSysLayouter->include_object(m_nRelColumn, iLine, iInstr, pInstr, pClef,
                                             -1.0f, true, iStaff, pShape);
//                    pShape->SetShapeLevel(lm_ePrologShape);
                xPos += pShape->get_width();
            }
        }
//
//            //add key signature shape
//            if (pKey && pKey->is_visible())
//            {
//                UPoint uPos = UPoint(xPos, yStartPos+uyOffset);        //absolute position
//                GmoShape* pShape = pKey->CreateShape(pBSI, m_pPaper, uPos, clefType, pStaff);
//                pShape->SetShapeLevel(lm_ePrologShape);
//				xPos += pShape->GetWidth();
//                pSysLayouter->IncludeObject(m_nRelColumn, nInstr, pKey, pShape, true, iStaff);
//            }

        uPrologWidth = max(uPrologWidth, xPos - xStartPos);
//
//        //compute vertical displacement for next staff
//        uyOffset += pStaff->GetHeight();
//
    }
//
//    // update paper cursor position
//    m_pageCursor.x = xStartPos + uPrologWidth;
    pSysLayouter->set_prolog_width(uPrologWidth);
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
//
////=========================================================================================
//// Methods to deal with measures
////=========================================================================================
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
// any gain by coding previous method? You will have to write more or less the same:
//      m_sysLayouters[m_nCurSystem-1]
//      get_system_layouter(m_nCurSystem-1)
//      get_current_system_layouter()

}  //namespace lomse
