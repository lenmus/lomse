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

#include "lomse_score_meter.h"
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
#include "lomse_shape_note.h"
#include "lomse_engraving_options.h"
#include "lomse_system_layouter.h"
#include "lomse_system_cursor.h"
#include "lomse_shape_barline.h"
#include "lomse_barline_engraver.h"
#include "lomse_clef_engraver.h"
#include "lomse_fermata_engraver.h"
#include "lomse_instrument_engraver.h"
#include "lomse_key_engraver.h"
#include "lomse_note_engraver.h"
#include "lomse_rest_engraver.h"
#include "lomse_tie_engraver.h"
#include "lomse_time_engraver.h"


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
//    , m_pColsBuilder( new ColumnsBuilder(this) )
    , m_pStubScore(NULL)
    , m_pCurBoxPage(NULL)
    , m_pCurBoxSystem(NULL)
{
    //ImoScore* pScore = get_imo_score();
    //ColStaffObjs* pCol = pScore->get_staffobjs_table();
    //pCol->dump();
}

//---------------------------------------------------------------------------------------
ScoreLayouter::~ScoreLayouter()
{
    delete_system_cursor();
    delete_instrument_engravers();
    delete_system_layouters();
    delete_column_layouters();
    delete m_pScoreMeter;
//    delete m_pColsBuilder;
}

//---------------------------------------------------------------------------------------
ImoScore* ScoreLayouter::get_imo_score()
{
    return dynamic_cast<ImoScore*>(m_pImo);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::prepare_to_start_layout()
{
    initialize();
    create_columns();
    m_iCurColumn = -1;
    m_iCurSystem = -1;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::initialize()
{
    ContentLayouter::prepare_to_start_layout();

    create_stub_for_score();
    create_instrument_engravers();
    get_score_renderization_options();
    decide_systems_indentation();

	m_iCurPage = -1;
//
//    //create_column_layouters();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_columns()
{
    create_system_cursor();
    m_iCurColumn = -1;
	determine_staves_vertical_position();
    while(!m_pSysCursor->is_end())
    {
        create_column();
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_column()
{
    m_iCurColumn++;
    create_column_boxes();
    collect_content_for_this_column();
    measure_this_column();
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
        m_instrEngravers.push_back( new InstrumentEngraver(m_libraryScope, m_pScoreMeter,
                                                           pInstr, pScore) );
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
    if (is_first_page())
    {
        decide_line_breaks();
        add_score_titles();
        move_cursor_after_headers();
    }

    while(enough_space_in_page() && m_iCurColumn < get_num_columns())
    {
        create_system();
        add_system_to_page();
    }

    set_layout_is_finished( m_iCurColumn >= get_num_columns() );
    //set_layout_is_finished(true);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::page_initializations(GmoBox* pContainerBox)
{
    m_iCurPage++;
    m_pCurBoxPage = dynamic_cast<GmoBoxScorePage*>( pContainerBox );
    is_first_system_in_page(true);
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::get_first_system_staves_size()
{
    ImoScore* pScore = get_imo_score();
    ImoSystemInfo* pInfo = pScore->get_first_system_info();
    LUnits leftMargin = pInfo->get_left_margin();
    LUnits rightMargin = pInfo->get_right_margin();
    return m_pCurBoxPage->get_width() - leftMargin - rightMargin - m_uFirstSystemIndent;
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::get_other_systems_staves_size()
{
    ImoScore* pScore = get_imo_score();
    ImoSystemInfo* pInfo = pScore->get_other_system_info();
    LUnits leftMargin = pInfo->get_left_margin();
    LUnits rightMargin = pInfo->get_right_margin();
    return m_pCurBoxPage->get_width() - leftMargin - rightMargin - m_uOtherSystemIndent;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::decide_line_breaks()
{
    int numCols = int( m_ColStorage.size() );
    int iSystem = 0;

    if (numCols == 0)
        return;

    //start first system
    m_breaks.push_back(0);
    LUnits space = get_available_space_in_system(0) - get_minimum_size(0);

    for (int iCol=1; iCol < numCols; ++iCol)
    {
        if (space >= get_minimum_size(iCol) && !m_ColLayouters[iCol]->has_system_break())
            space -= get_minimum_size(iCol);
        else
        {
            //start new system
            iSystem++;
            m_breaks.push_back(iCol);
            space = get_available_space_in_system(iSystem) - get_minimum_size(iCol);
        }
    }
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
void ScoreLayouter::create_system()
{
    SystemLayouter* pSysLyt = create_system_layouter();
    create_system_box(pSysLyt);

    //REFACTOR: reposition_staves()
    UPoint org = m_pCurBoxSystem->get_origin();
    org.y += determine_top_space(0);
    org.x = 0.0f;

    ImoScore* pScore = get_imo_score();
    int maxInstr = pScore->get_num_instruments() - 1;
    for (int iInstr = 0; iInstr <= maxInstr; iInstr++)
    {
        InstrumentEngraver* engrv = get_instrument_engraver(iInstr);
        //determine_staff_lines_horizontal_position(iInstr);
        LUnits indent = get_system_indent();
        LUnits width = m_pCurBoxSystem->get_content_width();
        LUnits left = m_pCurBoxSystem->get_left();
        engrv->set_staves_horizontal_position(left, width, indent);

        engrv->set_slice_instr_origin(org);
    }

    fill_current_system_with_columns();
    justify_current_system();
    engrave_system_details();
    engrave_instrument_details();

    if (is_last_system() && m_fStopStaffLinesAtFinalBarline)
        truncate_current_system();
    add_initial_line_joining_all_staves_in_system();
}

//---------------------------------------------------------------------------------------
SystemLayouter* ScoreLayouter::create_system_layouter()
{
    SystemLayouter* pSysLyt = new SystemLayouter(this, m_pScoreMeter);
    m_sysLayouters.push_back(pSysLyt);
    return pSysLyt;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_system_box(SystemLayouter* pSysLyt)
{
    m_iCurSystem++;
    m_nColumnsInSystem = 0;
    LUnits width = m_pCurBoxPage->get_width();
    LUnits top = m_pageCursor.y
                 + distance_to_top_of_system(m_iCurSystem, m_fFirstSystemInPage);
    LUnits left = m_pageCursor.x;
    m_pCurBoxSystem = pSysLyt->create_system_box(left, top, width);

    //set system heigt
    LUnits height = m_stavesHeight;
    height += determine_top_space(0);   //add top margin
    //add bottom margin
     ImoScore* pScore = get_imo_score();
    ImoSystemInfo* pInfo = pScore->get_other_system_info();
    height += pInfo->get_system_distance() / 2.0f;
    m_pCurBoxSystem->set_height(height);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_system_to_page()
{
    m_pCurBoxPage->add_system(m_pCurBoxSystem, m_iCurSystem);
    m_pCurBoxSystem->store_shapes_in_page();

    advance_paper_cursor_to_bottom_of_added_system();
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

    LUnits height = m_stavesHeight;
    height += distance_to_top_of_system(m_iCurSystem+1, m_fFirstSystemInPage);
    height += determine_top_space(0);
    return remaining_height() >= height;
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
void ScoreLayouter::advance_paper_cursor_to_bottom_of_added_system()
{
    m_pageCursor.x = m_pCurBoxPage->get_left();
    m_pageCursor.y = m_pCurBoxSystem->get_bottom();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::fill_current_system_with_columns()
{
    m_iCurColumn = 0;
    if (get_num_systems() == 0)
        return;

    InstrumentEngraver* pEngrv = get_instrument_engraver(0);
    m_pageCursor.x = pEngrv->get_staves_left();

    int iFirstCol = m_breaks[m_iCurSystem];
    int iLastCol = (m_iCurSystem == get_num_systems() - 1 ?
                        get_num_columns() : m_breaks[m_iCurSystem + 1] );
    m_uFreeSpace = (m_iCurSystem == 0 ? get_first_system_staves_size()
                                      : get_other_systems_staves_size() );

    m_fFirstColumnInSystem = true;
    for (m_iCurColumn = iFirstCol; m_iCurColumn < iLastCol; ++m_iCurColumn)
    {
        add_column_to_system();
        m_fFirstColumnInSystem = false;
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::justify_current_system()
{
    if (is_system_empty(m_iCurSystem))
        return;
    
    //AWARE: To create score without system justification just comment out next two lines
    if (system_must_be_justified())
        redistribute_free_space();

    reposition_slices_and_staffobjs();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::reposition_slices_and_staffobjs()
{
    int iFirstCol = m_breaks[m_iCurSystem];
    int iLastCol = (m_iCurSystem == get_num_systems() - 1 ?
                        get_num_columns() : m_breaks[m_iCurSystem + 1] );

    GmoBoxSlice* pFirstSlice = get_slice_box_for_column(iFirstCol);
    LUnits xLeft = pFirstSlice->get_left();
    LUnits yTop = pFirstSlice->get_top();
    LUnits xStartPos = get_start_position_for_column(iFirstCol);

    for (int iCol = iFirstCol; iCol < iLastCol; ++iCol)
    {
        //reposition boxes
        m_ColLayouters[iCol]->set_slice_final_position(xLeft, yTop);
        xLeft += m_ColLayouters[iCol]->get_minimum_size();

        //reposition staffobjs
        LUnits xEndPos = redistribute_space(iCol, xStartPos);

        //assign justified width to boxes
        set_slice_box_size(iCol, xEndPos - xStartPos);
    }
}

//---------------------------------------------------------------------------------------
bool ScoreLayouter::enough_space_in_system()
{
    if (is_first_column_in_system())
        m_uFreeSpace = get_available_space_in_system(m_iCurSystem);

    //check if there is enough space to add current column to current system
    if(m_uFreeSpace < get_minimum_size(m_iCurColumn))
        return false;

    return true;
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::get_available_space_in_system(int iSystem)
{
    LUnits width = (iSystem == 0 ? get_first_system_staves_size()
                                 : get_other_systems_staves_size() );
    if (iSystem > 0)
        width -= space_used_by_prolog(iSystem);
    return width;
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::space_used_by_prolog(int iSystem)
{
    //TODO. Prolog width should be computed on each column.
    //For now, an estimation: the height of ten lines (two staff)
    return m_pScoreMeter->tenths_to_logical(10.0f, 0, 0);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_column_boxes()
{
    create_slice_box();

    //create instrument slice boxes
    ImoScore* pScore = get_imo_score();
    m_pCurBSI = NULL;
    int numInstrs = pScore->get_num_instruments();

    LUnits yTop = m_pCurSlice->get_top();
    for (int iInstr = 0; iInstr < numInstrs; iInstr++)
    {
        m_pageCursor.x = m_pCurSlice->get_left();    //align start of all staves

        //create slice instr box
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        m_pCurBSI = m_ColLayouters[m_iCurColumn]->create_slice_instr(pInstr, yTop);

        //set box height
        LUnits height = m_SliceInstrHeights[iInstr];
        if (iInstr==0)
        {
            //add top margin
            height += determine_top_space(0);
        }
        if (iInstr==numInstrs-1)
        {
            //add bottom margin
            ImoSystemInfo* pInfo = pScore->get_other_system_info();
            height += pInfo->get_system_distance() / 2.0f;
        }
        yTop += height;
        m_pCurBSI->set_height(height);
    }
    m_pageCursor.y = yTop;

    //set slice and system height
    LUnits uTotalHeight = m_pageCursor.y - m_pCurSlice->get_top();
    m_pCurSlice->set_height(uTotalHeight);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::determine_staves_vertical_position()
{
    ImoScore* pScore = get_imo_score();
    int numInstrs = pScore->get_num_instruments();

    m_SliceInstrHeights.reserve(numInstrs);

    LUnits yPos = 0.0f;
    for (int iInstr = 0; iInstr < numInstrs; iInstr++)
    {
        LUnits yTop = yPos;
        InstrumentEngraver* engrv = get_instrument_engraver(iInstr);

        if (iInstr > 0)
        {
            LUnits uMargin = determine_top_space(iInstr);
            yPos += uMargin;
        }

        engrv->set_staves_vertical_position(yPos);
        yPos = engrv->get_staves_bottom();

        if (iInstr != numInstrs-1)
            yPos += determine_top_space(iInstr+1);

        m_SliceInstrHeights[iInstr] = yPos - yTop;
    }
    m_stavesHeight = yPos;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::create_slice_box()
{
    m_pCurSlice = new GmoBoxSlice(m_iCurColumn);
	m_pCurSlice->set_left(0.0f);
	m_pCurSlice->set_top(0.0f);

    prepare_for_new_column(m_pCurSlice);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::measure_this_column()
{
    bool fTrace = false;

//    m_sysLayouters[m_iCurSystem]->end_of_system_measurements();
    do_column_spacing(m_iCurColumn, fTrace);
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::determine_top_space(int iInstr)
{
    ImoScore* pScore = get_imo_score();
    if (iInstr == 0)
    {
        //distance_to_first_staff
        if (is_first_system_in_score())
        {
            ImoSystemInfo* pInfo = pScore->get_first_system_info();
            return pInfo->get_system_distance() / 2.0f;
        }
        else if (is_first_system_in_page())
        {
            ImoSystemInfo* pInfo = pScore->get_other_system_info();
            return pInfo->get_system_distance() / 2.0f;
        }
        else
        {
            ImoSystemInfo* pInfo = pScore->get_other_system_info();
            return pInfo->get_system_distance() / 2.0f;
        }
    }
    else
    {
        ImoInstrument* pInstr = pScore->get_instrument(iInstr);
        return pInstr->get_staff(0)->get_staff_margin() / 2.0f;
    }
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::distance_to_top_of_system(int iSystem, bool fFirstInPage)
{
    if (iSystem == 0)   //is_first_system_in_score())
    {
        ImoScore* pScore = get_imo_score();
        ImoSystemInfo* pInfo = pScore->get_first_system_info();
        return pInfo->get_top_system_distance() - pInfo->get_system_distance() / 2.0f;
    }
    else if (fFirstInPage)  //is_first_system_in_page())
    {
        ImoScore* pScore = get_imo_score();
        ImoSystemInfo* pInfo = pScore->get_other_system_info();
        return pInfo->get_top_system_distance() - pInfo->get_system_distance() / 2.0f;
    }
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::determine_staff_lines_horizontal_position(int iInstr)
{
    LUnits indent = get_system_indent();
    LUnits width = m_pCurBoxSystem->get_content_width();
    InstrumentEngraver* engrv = get_instrument_engraver(iInstr);
    engrv->set_staves_horizontal_position(m_pageCursor.x, width, indent);
    m_pageCursor.x += indent;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::engrave_instrument_details()
{
    ImoScore* pScore = get_imo_score();
    int maxInstr = pScore->get_num_instruments() - 1;
    for (int iInstr = 0; iInstr <= maxInstr; iInstr++)
    {
        InstrumentEngraver* engrv = get_instrument_engraver(iInstr);
        engrv->add_staff_lines(m_pCurBoxSystem);
        engrv->add_name_abbrev(m_pCurBoxSystem, m_iCurSystem);
        engrv->add_brace_bracket(m_pCurBoxSystem);
    }
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
    //to right margin. Set here the applicable length.

    //TODO
    ////this is the last system and it has been requested to stop staff lines
    ////in last measure. So, set final x so staff lines go to final bar line
    //LUnits xFinalPos = 0.0f;
    //LUnits yFinalPos = 0.0f;
    //ImoInstrument *pI;
    //for (pI = m_pScore->GetFirstInstrument(); pI; pI=m_pScore->GetNextInstrument())
    //{
    //    LUnits xPos, yPos;
    //    pI->GetVStaff()->GetBarlineOfLastNonEmptyMeasure(&xPos, &yPos);
    //    if (yPos > yFinalPos)
    //    {
    //        yFinalPos = yPos;
    //        xFinalPos = xPos;
    //    }
    //}
    //if (xFinalPos > 0.0f)
    //    m_pCurBoxSystem->UpdateXRight( xFinalPos - 1 );
    //else
    //    m_pCurBoxSystem->UpdateXRight( m_pScore->GetRightMarginXPos() );
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_system_prolog()
{
    if (m_iCurColumn > 0 && is_first_column_in_system())
	{
	    LUnits uPrologWidth = 0.0f;

	    int numInstruments = m_pScoreMeter->num_instruments();
	    for (int iInstr=0; iInstr < numInstruments; ++iInstr)
	    {
            LUnits width = engrave_prolog(iInstr);
	        uPrologWidth = max(uPrologWidth, width);
	    }

        m_pageCursor.x += uPrologWidth;
        m_uFreeSpace -= uPrologWidth;
	}
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::collect_content_for_this_column()
{
    LUnits uInitialSpace = determine_initial_space();
	ImoScore* pScore = get_imo_score();
	int numInstruments = m_pScoreMeter->num_instruments();

	//variables to control barlines found on each instrument
    std::vector<bool> fBarlineFound;
    fBarlineFound.reserve(numInstruments);
    fBarlineFound.assign(numInstruments, false);
    bool fAtLeastABarlineFound = false;

    //ask system layouter to prepare to receive data for this instrument objects in
    //this column
    LUnits uxStart = m_pageCursor.x;
    start_bar_measurements(m_iCurColumn, uxStart, uInitialSpace);

    //loop to process all StaffObjs in this measure
    bool fNewSystemTagFound = false;
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
        m_pageCursor.y = m_instrEngravers[iInstr]->get_top_line_of_staff(iStaff);
        GmoShape* pShape = NULL;

        if (!pSO->is_barline() && fAtLeastABarlineFound)
            break;

        if (pSO->is_barline())  //TODO: || IsHigherTime(pSO->GetTimePos(), m_pSysCursor->GetBreakTime()) )
        {
            pShape = create_staffobj_shape(pSO, iInstr, iStaff);
            include_barline_and_finish_bar_measurements(m_iCurColumn, iLine, pSO,
                                                        pShape, uxStart, rTime);
            fBarlineFound[iInstr] = true;
            fAtLeastABarlineFound = true;
        }

        else if (pSO->is_control())
        {
            fNewSystemTagFound = true;
            m_ColLayouters[m_iCurColumn]->set_system_break(true);
        }

        else if (pSO->is_clef() || pSO->is_key_signature() || pSO->is_time_signature())
		{
            unsigned flags = 0; //fAddingProlog[iAbsStaff] ? 0 : k_flag_small_clef;
            pShape = create_staffobj_shape(pSO, iInstr, iStaff, flags);
            include_object(m_iCurColumn, iLine, iInstr, pInstr, pSO,
                           -1.0f, iStaff, pShape);
        }

		else
		{
            pShape = create_staffobj_shape(pSO, iInstr, iStaff);
            include_object(m_iCurColumn, iLine, iInstr, pInstr, pSO,
                           rTime, iStaff, pShape);
        }

        store_info_about_attached_objects(pSO, pShape, iInstr, iStaff,
                                          m_iCurColumn, iLine, pInstr);

        m_pSysCursor->move_next();
    }

    finish_bar_measurements(m_iCurColumn, uxStart);

//        //force new system if a break point reached
//        if (pSO && IsHigherTime(pSO->GetTimePos(), m_pSysCursor->GetBreakTime()))
//            fNewSystemTagFound = true;

    must_finish_system( fNewSystemTagFound );
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
GmoShape* ScoreLayouter::create_staffobj_shape(ImoStaffObj* pSO, int iInstr, int iStaff,
                                               unsigned flags)
{
    //factory method to create shapes for staffobjs

    switch (pSO->get_obj_type())
    {
        case ImoObj::k_barline:
        {
            ImoBarline* pImo = dynamic_cast<ImoBarline*>(pSO);
            InstrumentEngraver* pInstrEngrv = get_instrument_engraver(iInstr);
            LUnits yTop = pInstrEngrv->get_staves_top_line();
            LUnits yBottom = pInstrEngrv->get_staves_bottom_line();
            BarlineEngraver engrv(m_libraryScope, m_pScoreMeter);
            return engrv.create_shape(pImo, iInstr, m_pageCursor.x, yTop, yBottom);
        }
        case ImoObj::k_clef:
        {
            bool fSmallClef = flags && k_flag_small_clef;
            ImoClef* pClef = dynamic_cast<ImoClef*>(pSO);
            int clefSize = pClef->get_symbol_size();
            if (clefSize == k_size_default)
                clefSize = fSmallClef ? k_size_cue : k_size_full;
            ClefEngraver engrv(m_libraryScope, m_pScoreMeter);
            return engrv.create_shape(pClef, iInstr, iStaff, m_pageCursor,
                                      pClef->get_clef_type(), clefSize);
        }
        case ImoObj::k_key_signature:
        {
            ImoKeySignature* pImo = dynamic_cast<ImoKeySignature*>(pSO);
            int clefType = m_pSysCursor->get_clef_type_for_instr_staff(iInstr, iStaff);
            KeyEngraver engrv(m_libraryScope, m_pScoreMeter);
            return engrv.create_shape(pImo, iInstr, iStaff, clefType, m_pageCursor);
        }
        case ImoObj::k_note:
        {
            ImoNote* pImo = dynamic_cast<ImoNote*>(pSO);
            NoteEngraver engrv(m_libraryScope, m_pScoreMeter, &m_shapesStorage);
            int clefType = m_pSysCursor->get_applicable_clef_type();
            return engrv.create_shape(pImo, iInstr, iStaff, clefType, m_pageCursor);
        }
        case ImoObj::k_rest:
        {
            ImoRest* pImo = dynamic_cast<ImoRest*>(pSO);
            RestEngraver engrv(m_libraryScope, m_pScoreMeter, &m_shapesStorage);
            int type = pImo->get_note_type();
            int dots = pImo->get_dots();
            return engrv.create_shape(pImo, iInstr, iStaff, m_pageCursor, type, dots, pImo);
        }
        case ImoObj::k_time_signature:
        {
            ImoTimeSignature* pImo = dynamic_cast<ImoTimeSignature*>(pSO);
            int beats = pImo->get_beats();
            int beat_type = pImo->get_beat_type();
            TimeEngraver engrv(m_libraryScope, m_pScoreMeter);
            return engrv.create_shape_normal(pImo, iInstr, iStaff, m_pageCursor,
                                             beats, beat_type);
        }
        default:
            return new GmoShapeInvisible(pSO, 0, m_pageCursor, USize(0.0, 0.0));
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::store_info_about_attached_objects(ImoStaffObj* pSO,
                                    GmoShape* pMainShape, int iInstr, int iStaff,
                                    int iCol, int iLine, ImoInstrument* pInstr)
{
    ImoAttachments* pAuxObjs = pSO->get_attachments();
    if (!pAuxObjs)
        return;

    PendingAuxObjs* data = new PendingAuxObjs(pSO, pMainShape, iInstr, iStaff,
                                              iCol, iLine, pInstr);
    m_pendingAuxObjs.push_back(data);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::engrave_system_details()
{
    std::list<PendingAuxObjs*>::iterator it;
    for (it = m_pendingAuxObjs.begin(); it != m_pendingAuxObjs.end(); ++it)
    {
        engrave_attached_objects((*it)->m_pSO, (*it)->m_pMainShape,
                                             (*it)->m_iInstr, (*it)->m_iStaff,
                                             get_system_including_col((*it)->m_iCol),
                                             (*it)->m_iCol, (*it)->m_iLine,
                                             (*it)->m_pInstr );
        delete *it;
    }
    m_pendingAuxObjs.clear();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::engrave_attached_objects(ImoStaffObj* pSO, GmoShape* pMainShape,
                                             int iInstr, int iStaff, int iSystem,
                                             int iCol, int iLine,
                                             ImoInstrument* pInstr)
{
    ImoAttachments* pAuxObjs = pSO->get_attachments();
    int size = pAuxObjs->get_num_items();
	for (int i=0; i < size; ++i)
	{
        ImoAuxObj* pAO = dynamic_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );

        GmoShape* pAuxShape = NULL;
        if (!pAO->is_relobj())
        {
            //1-R AuxObjs. Engrave it
            pAuxShape = create_auxobj_shape(pAO, iInstr, iStaff, pMainShape);
            pMainShape->accept_link_from(pAuxShape);
            add_auxobj_shape_to_model(pAuxShape, GmoShape::k_layer_aux_objs, iSystem,
                               iCol, iInstr);
        }
        else    // pAO->is_binary_relobj() || pAO->is_multi_relobj()
        {
            //2-R and n-R AuxObjs
            ImoRelObj* pRO;
            if (pAO->is_binary_relobj())
                pRO = dynamic_cast<ImoBinaryRelObj*>(pAO);
            else
                pRO = dynamic_cast<ImoMultiRelObj*>(pAO);

		    if (pSO == pRO->get_start_object())
                start_engraving_auxobj(pAO, pSO, pMainShape, iInstr, iStaff,
                                       iSystem, iCol, iLine, pInstr);
		    else if (pSO == pRO->get_end_object())
		    {
                finish_engraving_auxobj(pAO, pSO, pMainShape, iInstr, iStaff, iSystem,
                                        iCol, iLine, pInstr);
                add_auxobjs_shapes_to_model(pAO, pMainShape, GmoShape::k_layer_aux_objs);
		    }
            else
                continue_engraving_auxobj(pAO, pSO, pMainShape, iInstr, iStaff,
                                          iSystem, iCol, iLine, pInstr);
        }

    }
}
//---------------------------------------------------------------------------------------
void ScoreLayouter::start_engraving_auxobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                           GmoShape* pStaffObjShape,
                                           int iInstr, int iStaff, int iSystem,
                                           int iCol, int iLine,
                                           ImoInstrument* pInstr)
{
    //factory method to create the engraver for relation auxobjs

    RelAuxObjEngraver* pEngrv = NULL;
    switch (pAO->get_obj_type())
    {
        case ImoObj::k_tie:
        {
            InstrumentEngraver* pInstrEngrv = get_instrument_engraver(iInstr);
            LUnits xRight = pInstrEngrv->get_staves_right();
            LUnits xLeft = pInstrEngrv->get_staves_left();
            pEngrv = new TieEngraver(m_libraryScope, m_pScoreMeter, xLeft, xRight);
            break;
        }

        default:
            return;
    }

    if (pEngrv)
    {
        pEngrv->set_start_staffobj(pAO, pSO, pStaffObjShape, iInstr, iStaff,
                                   iSystem, iCol, m_pageCursor);
        m_shapesStorage.save_engraver(pEngrv, pAO);
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::continue_engraving_auxobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                              GmoShape* pStaffObjShape,
                                              int iInstr, int iStaff, int iSystem,
                                              int iCol, int iLine,
                                              ImoInstrument* pInstr)
{
    RelAuxObjEngraver* pEngrv
        = dynamic_cast<RelAuxObjEngraver*>(m_shapesStorage.get_engraver(pAO));
    pEngrv->set_middle_staffobj(pAO, pSO, pStaffObjShape, iInstr, iStaff, iSystem, iCol);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::finish_engraving_auxobj(ImoAuxObj* pAO, ImoStaffObj* pSO,
                                                 GmoShape* pStaffObjShape,
                                                 int iInstr, int iStaff, int iSystem,
                                                 int iCol, int iLine,
                                                 ImoInstrument* pInstr)
{
    RelAuxObjEngraver* pEngrv
        = dynamic_cast<RelAuxObjEngraver*>(m_shapesStorage.get_engraver(pAO));
    pEngrv->set_end_staffobj(pAO, pSO, pStaffObjShape, iInstr, iStaff, iSystem, iCol);

    SystemLayouter* pSysLyt = get_system_layouter(iSystem);
    pEngrv->set_prolog_width( pSysLyt->get_prolog_width() );

    pEngrv->create_shapes();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_auxobjs_shapes_to_model(ImoAuxObj* pAO, GmoShape* pStaffObjShape,
                                                int layer)
{
    RelAuxObjEngraver* pEngrv
        = dynamic_cast<RelAuxObjEngraver*>(m_shapesStorage.get_engraver(pAO));

    int numShapes = pEngrv->get_num_shapes();
    for (int i=0; i < numShapes; ++i)
    {
        ShapeBoxInfo* pInfo = pEngrv->get_shape_box_info(i);
        GmoShape* pAuxShape = pInfo->pShape;
        int iSystem = pInfo->iSystem;
        int iCol = pInfo->iCol;
        int iInstr = pInfo->iInstr;

        pStaffObjShape->accept_link_from(pAuxShape);
        add_auxobj_shape_to_model(pAuxShape, layer, iSystem, iCol, iInstr);
    }

    m_shapesStorage.remove_engraver(pAO);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_auxobj_shape_to_model(GmoShape* pShape, int layer, int iSystem,
                                              int iCol, int iInstr)
{
    pShape->set_layer(layer);
    GmoBoxSliceInstr* pSliceInstrBox = get_slice_instr_box_for(iCol, iInstr);
    pSliceInstrBox->add_shape(pShape, layer);
}

//---------------------------------------------------------------------------------------
GmoShape* ScoreLayouter::create_auxobj_shape(ImoAuxObj* pAO, int iInstr, int iStaff,
                                             GmoShape* pParentShape)
{
    //factory method to create shapes for auxobjs

    switch (pAO->get_obj_type())
    {
        case ImoObj::k_fermata:
        {
            ImoFermata* pImo = dynamic_cast<ImoFermata*>(pAO);
            FermataEngraver engrv(m_libraryScope, m_pScoreMeter);
            int placement = pImo->get_placement();
            return engrv.create_shape(pImo, iInstr, iStaff, m_pageCursor, placement,
                                      pParentShape);
        }
        default:
            return new GmoShapeInvisible(pAO, 0, m_pageCursor, USize(0.0, 0.0));
    }
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_column_to_system()
{
    add_system_prolog();

    //REFACTOR: reposition slice and slice instrument boxes.
    //step 2: map slice without top/bottom margins
    m_pCurSlice = m_ColLayouters[m_iCurColumn]->get_slice_box();
    LUnits ySystem = m_pCurBoxSystem->get_top();
    m_ColLayouters[m_iCurColumn]->set_slice_final_position(m_pageCursor.x, ySystem);

    m_pCurBoxSystem->add_child_box(m_pCurSlice);

    //Add column to current system and discount the space that the measure will take
    add_shapes_for_column(m_iCurColumn, &m_shapesStorage);
    LUnits size = get_minimum_size(m_iCurColumn);
    m_uFreeSpace -= size;
    m_pageCursor.x += size;
    m_nColumnsInSystem++;
    m_pSysCursor->save_position();

//    //mark all objects in column as 'non dirty'
//    m_sysLayouters[m_iCurSystem]->ClearDirtyFlags(m_nRelColumn);

    //prepare to create a new column
    m_pCurSlice = NULL;
}

//---------------------------------------------------------------------------------------
int ScoreLayouter::get_system_including_col(int iCol)
{
    if (iCol > 0)
    {
        
        int maxSystem = get_num_systems() - 1;
        for (int iSys = 0; iSys < maxSystem; ++iSys)
        {
            if (iCol >= m_breaks[iSys] && iCol < m_breaks[iSys+1])
                return iSys;
        }
        return maxSystem;
    }
    else
        return 0;
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_shapes_for_column(int iCol, ShapesStorage* pStorage)
{
    add_shapes_to_column(iCol, pStorage);
}

//---------------------------------------------------------------------------------------
bool ScoreLayouter::system_must_be_justified()
{
    //all systems needs justification except:

    //1. it is the last system and flag "JustifyFinalBarline" is not set
    if (is_last_system() && !m_fJustifyFinalBarline)
        return false;

    //2. it is the last system but there is no final barline
    int iLastCol = get_num_columns();
    if (is_last_system() && !column_has_barline(iLastCol))
            return false;

    return true;        //do justification
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::redistribute_free_space()
{
    //Space is redistributed to try to have all columns with equal witdh.

    if (m_uFreeSpace <= 0.0f)
        return;           //no space to distribute

    int iFirstCol = m_breaks[m_iCurSystem];

   //compute average column size and total occupied
    LUnits uTotal = 0.0f;
    for (int i = 0; i < m_nColumnsInSystem; i++)
    {
         uTotal += get_minimum_size(i + iFirstCol);
    }
    LUnits uAverage = (uTotal + m_uFreeSpace) / m_nColumnsInSystem;

    //For each column, compute the diference between its size and the average target size.
    //Sum up all the diferences in uDifTotal
    std::vector<LUnits> uDif(m_nColumnsInSystem, 0.0f);
    LUnits uDifTotal = 0;
    int nNumSmallerColumns = 0;      //num of columns smaller than average
    for (int i = 0; i < m_nColumnsInSystem; i++)
    {
        uDif[i] = uAverage - get_minimum_size(i + iFirstCol);
        if (uDif[i] > 0.0f)
        {
            uDifTotal += uDif[i];
            nNumSmallerColumns++;
        }
    }

    //distribute space
    if (uDifTotal > m_uFreeSpace)
    {
        //not enough space to make all columns equal size
        LUnits uReduce = (uDifTotal - m_uFreeSpace) / nNumSmallerColumns;
        for (int i = 0; i < m_nColumnsInSystem; i++)
        {
            if (uDif[i] > 0.0f)
            {
                uDif[i] -= uReduce;
                increment_column_size(i+iFirstCol, uDif[i]);
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
                increment_column_size(i+iFirstCol, uDif[i]);
            }
        }
    }

}

//---------------------------------------------------------------------------------------
void ScoreLayouter::set_slice_box_size(int iCol, LUnits width)
{
    m_ColLayouters[iCol]->set_slice_width(width);
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
        ImoObj* pCreator = get_imo_score()->get_instrument(0);
        LUnits xPos = m_instrEngravers[0]->get_staves_left();
        LUnits yTop = m_instrEngravers[0]->get_staves_top_line();
        int iInstr = m_pScoreMeter->num_instruments() - 1;
        LUnits yBottom = m_instrEngravers[iInstr]->get_staves_bottom_line();
        LUnits uLineThickness =
            m_pScoreMeter->tenths_to_logical(LOMSE_THIN_LINE_WIDTH, 0, 0);
        GmoShape* pLine = new GmoShapeBarline(pCreator, 0, ImoBarline::k_simple,
                                              xPos, yTop, yBottom,
                                              uLineThickness, uLineThickness,
                                              0.0f, 0.0f, Color(0,0,0), uLineThickness);
        m_pCurBoxSystem->add_shape(pLine, GmoShape::k_layer_staff);
	}
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::engrave_prolog(int iInstr)
{
    LUnits uPrologWidth = 0.0f;

    //AWARE when this method is invoked the paper position must be at the left marging,
    //at the start of a new system.
    LUnits xStartPos = m_pageCursor.x;      //Save x to align all clefs

    //iterate over the collection of lmStaff objects to draw current clef and key signature
    ImoScore* pScore = get_imo_score();
    ImoInstrument* pInstr = pScore->get_instrument(iInstr);

    SystemLayouter* pSysLyt = m_sysLayouters[m_iCurSystem];
    GmoBoxSystem* pBox = pSysLyt->get_box_system();

    for (int iStaff=0; iStaff < pInstr->get_num_staves(); ++iStaff)
    {
        LUnits xPos = xStartPos;
        m_pageCursor.y = m_instrEngravers[iInstr]->get_top_line_of_staff(iStaff);
        ColStaffObjsEntry* pClefEntry =
            m_pSysCursor->get_clef_entry_for_instr_staff(iInstr, iStaff);
        ColStaffObjsEntry* pKeyEntry =
            m_pSysCursor->get_key_entry_for_instr_staff(iInstr, iStaff);

        //add clef shape
        if (pClefEntry)
        {
            ImoClef* pClef = dynamic_cast<ImoClef*>( pClefEntry->imo_object() );
            if (pClef && pClef->is_visible())
            {
                xPos += m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_BEFORE_PROLOG, iInstr, iStaff);
                m_pageCursor.x = xPos;
                GmoShape* pShape = create_staffobj_shape(pClef, iInstr, iStaff);
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
                m_pageCursor.x = xPos;
                GmoShape* pShape = create_staffobj_shape(pKey, iInstr, iStaff);
                pBox->add_shape(pShape, GmoShape::k_layer_notes);
                xPos += pShape->get_width();
            }
        }

        xPos += m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_AFTER_PROLOG, iInstr, iStaff);
        uPrologWidth = max(uPrologWidth, xPos - xStartPos);
    }

    m_pageCursor.x = xStartPos;     //restore cursor
    pSysLyt->set_prolog_width(uPrologWidth);

    return uPrologWidth;
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
//    if (m_sysLayouters[m_iCurSystem]->GetOptimumBreakPoint(m_nRelColumn, uAvailable, &rTime, &uWidth))
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
//    m_pageCursor.y +=  m_pScore->GetSystemDistance(m_iCurSystem, false) / 2.0 ;
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
//            m_pCurBoxPage->add_system(m_iCurSystem, m_pageCursor.x,
//                                         m_uStartOfCurrentSystem, fFirstSystemInPage);
//        m_pCurBoxSystem->SetFirstMeasure(m_iCurColumn);
//        m_pageCursor.x += m_pCurBoxSystem->get_left_margin();
//        m_pCurBoxSystem->SetIndent(get_system_indent());
//
//
//        m_nRelColumn = 0;          //first column of this system
//        m_iCurColumn++;
//        m_uLastSystemHeight = AddEmptySystem(m_iCurSystem, m_pCurBoxSystem);     //Add the staff lines
//
//        //staff lines go to the rigth margin
//        m_pCurBoxSystem->UpdateXRight( m_pScore->GetRightMarginXPos() );
//
//        // compute system bottom space
//        //AWARE:
//        //  In GetSystemDistance() we are using m_iCurSystem instead of m_iCurSystem.
//        //  This is to get the system distance between this system and next one.
//        LUnits uSystemBottomSpace = m_pScore->GetSystemDistance(m_iCurSystem, false) / 2.0;
//        m_pCurBoxSystem->SetBottomSpace(uSystemBottomSpace);
//
//        //advance paper in system bottom space
//        m_pageCursor.y += uSystemBottomSpace;
//
//        //increment loop information
//        m_iCurSystem++;
//    }
//}
//
////=========================================================================================
//// methods coded only for Unit Tests
////=========================================================================================

//int ScoreLayouter::GetNumSystemLayouters() { return (int)m_sysLayouters.size(); }
//int ScoreLayouter::get_num_columns(int iSys) { return m_sysLayouters[iSys]->get_num_columns(); }
//int ScoreLayouter::GetNumLines(int iSys, int iCol)
//        { return m_sysLayouters[iSys]->GetNumLinesInColumn(iCol); }
//
//SystemLayouter* ScoreLayouter::get_system_layouter(int iSys)
//{
//    //iSys=[0..n-1]
//    return m_sysLayouters[iSys];
//}
// any gain by coding previous method? You will have to write more or less the same:
//      m_sysLayouters[m_iCurSystem]
//      get_system_layouter(m_iCurSystem)
//      get_current_system_layouter()



//=======================================================================================
// ColumnsBuilder implementation
//=======================================================================================


//---------------------------------------------------------------------------------------
void ScoreLayouter::delete_column_layouters()
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
void ScoreLayouter::prepare_for_new_column(GmoBoxSlice* pBoxSlice)
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

//////---------------------------------------------------------------------------------------
////void ScoreLayouter::end_of_system_measurements()
////{
////    //caller informs that all data for this system has been suplied.
////    //This is the right place to do any preparatory work, not to be repeated if re-spacing.
////
////    //Nothing to do for current implementation
////}

//---------------------------------------------------------------------------------------
void ScoreLayouter::start_bar_measurements(int iCol, LUnits uxStart, LUnits uSpace)
{
    //prepare to receive data for a new bar in column iCol [0..n-1].

    LinesBuilder* pLB = m_LinesBuilder[iCol];
    pLB->set_start_position(uxStart);
    pLB->set_initial_space(uSpace);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::include_object(int iCol, int iLine, int iInstr, ImoInstrument* pInstr,
                                   ImoStaffObj* pSO, float rTime,
                                   int nStaff, GmoShape* pShape)
{
    //caller sends data about one staffobj in current bar, for column iCol [0..n-1]

    m_LinesBuilder[iCol]->include_object(iLine, iInstr, pInstr, pSO, rTime,
                                         nStaff, pShape);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::include_barline_and_finish_bar_measurements(int iCol, int iLine,
                        ImoStaffObj* pSO, GmoShape* pShape, LUnits xStart, float rTime)
{
    //caller sends lasts object to store in current bar, for column iCol [0..n-1].

    m_LinesBuilder[iCol]->close_line(iLine, pSO, pShape, xStart, rTime);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::finish_bar_measurements(int iCol, LUnits xStart)
{
    m_LinesBuilder[iCol]->finish_bar_measurements(xStart);
}

//////---------------------------------------------------------------------------------------
////void ScoreLayouter::discard_data_for_current_column(ShapesStorage* pStorage)
////{
////    //caller request to ignore measurements for column iCol [0..n-1]
////
////    //m_ColStorage[iCol]->initialize();
////    //m_ColLayouters[iCol]->initialize();
////    ////m_LinesBuilder[iCol]->initialize();
////
////    //delete shapes
////    ColumnLayouter* pColLayouter = m_ColLayouters.back();
////    pColLayouter->delete_shapes(pStorage);
////
////    //delete helper objects
////    m_ColStorage.pop_back();
////    m_LinesBuilder.pop_back();
////    m_ColLayouters.pop_back();
////
////
////}

//---------------------------------------------------------------------------------------
void ScoreLayouter::do_column_spacing(int iCol, bool fTrace)
{
    m_ColLayouters[iCol]->do_spacing(fTrace);
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::redistribute_space(int iCol, LUnits uNewStart)
{
    LUnits uNewBarSize = m_ColLayouters[iCol]->get_minimum_size();
    GmoBoxSlice* pBox = m_ColLayouters[iCol]->get_slice_box();

    //step 3: map to system
    UPoint org = pBox->get_origin();
    org.y += determine_top_space(0);

    ColumnResizer oResizer(m_ColStorage[iCol], uNewBarSize);
	LUnits uBarFinalPosition = oResizer.reposition_shapes(uNewStart, uNewBarSize, org);

    return uBarFinalPosition;
}

////////---------------------------------------------------------------------------------------
//////void ScoreLayouter::AddTimeGridToBoxSlice(int iCol, GmoBoxSlice* pBSlice)
//////{
//////    //create the time-grid table and transfer it (and its ownership) to GmoBoxSlice
//////    pBSlice->SetTimeGridTable( new TimeGridTable(m_ColStorage[iCol]) );
//////}

//---------------------------------------------------------------------------------------
void ScoreLayouter::increment_column_size(int iCol, LUnits uIncr)
{
    m_ColLayouters[iCol]->increment_column_size(uIncr);
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::add_shapes_to_column(int iCol, ShapesStorage* pStorage)
{
    m_ColLayouters[iCol]->add_shapes_to_boxes(pStorage);
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::get_start_position_for_column(int iCol)
{
    return m_ColStorage[iCol]->get_start_of_bar_position();
}

//---------------------------------------------------------------------------------------
LUnits ScoreLayouter::get_minimum_size(int iCol)
{
    return m_ColLayouters[iCol]->get_minimum_size();
}

//---------------------------------------------------------------------------------------
bool ScoreLayouter::is_system_empty(int iSystem)
{
    if (get_num_systems() == 0)
        return true;

    int iFirtsCol = m_breaks[iSystem];
    return m_ColStorage[iFirtsCol]->size() == 0;
}

//////---------------------------------------------------------------------------------------
////bool ScoreLayouter::get_optimum_break_point(int iCol, LUnits uAvailable,
////                                        float* prTime, LUnits* puWidth)
////{
////    //return m_ColLayouters[iCol]->get_optimum_break_point(uAvailable, prTime, puWidth);
////    BreakPoints oBreakPoints(m_ColStorage[iCol]);
////    if (oBreakPoints.find_optimum_break_point_for_space(uAvailable))
////    {
////        *prTime = oBreakPoints.get_optimum_time_for_found_break_point();
////        *puWidth = oBreakPoints.get_optimum_position_for_break_point();
////        return false;
////    }
////    else
////        return true;
////}

//---------------------------------------------------------------------------------------
bool ScoreLayouter::column_has_barline(int iCol)
{
    return m_ColLayouters[iCol]->is_there_barline();
}

////////---------------------------------------------------------------------------------------
//////void ScoreLayouter::ClearDirtyFlags(int iCol)
//////{
//////    m_ColStorage[iCol]->ClearDirtyFlags();
//////}
////
//////---------------------------------------------------------------------------------------
////void ScoreLayouter::dump_column_data(int iCol, ostream& outStream)
////{
////    m_ColStorage[iCol]->dump_column_storage(outStream);
////}

//---------------------------------------------------------------------------------------
GmoBoxSliceInstr* ScoreLayouter::get_slice_instr_box_for(int iCol, int iInstr)
{
    return m_ColLayouters[iCol]->get_slice_instr(iInstr);
}

//---------------------------------------------------------------------------------------
GmoBoxSlice* ScoreLayouter::get_slice_box_for_column(int iCol)
{
    return m_ColLayouters[iCol]->get_slice_box();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::discard_data_for_current_column(ShapesStorage* pStorage)
{
    //caller request to ignore measurements for last column

    //delete shapes
    ColumnLayouter* pColLayouter = m_ColLayouters.back();
    pColLayouter->delete_shapes(pStorage);

    //delete helper objects
    m_ColStorage.pop_back();
    m_LinesBuilder.pop_back();
    m_ColLayouters.pop_back();
}

//---------------------------------------------------------------------------------------
int ScoreLayouter::get_num_columns()
{
    return int(m_ColLayouters.size());
}

//---------------------------------------------------------------------------------------
int ScoreLayouter::get_num_lines_in_column(int iCol)
{
    return m_ColLayouters[iCol]->get_num_lines();
}

//---------------------------------------------------------------------------------------
void ScoreLayouter::dump_column_data(int iCol, ostream& outStream)
{
    m_ColStorage[iCol]->dump_column_storage(outStream);
}


////////------------------------------------------------
//////// Debug build: methods coded only for Unit Tests
////////------------------------------------------------
//////int ScoreLayouter::get_num_objects_in_column_line(int iCol, int iLine)
//////{
//////    //iCol, iLine = [0..n-1]
//////    return m_ColStorage[iCol]->get_num_objects_in_line(iLine);
//////}



}  //namespace lomse
