//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
#include "lomse_shape_barline.h"
#include "lomse_staffobjs_table.h"
#include "lomse_engraving_options.h"
#include "lomse_instrument_engraver.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <math.h>
using namespace std;


namespace lomse
{

#define LOMSE_NO_DURATION   100000000000.0f     //any impossible high value
#define LOMSE_NO_TIME       100000000000.0f     //any impossible high value
#define LOMSE_NO_POSITION   100000000000.0f     //any impossible high value



//=====================================================================================
//LineEntry implementation
//=====================================================================================
LineEntry::LineEntry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog, float rTime)
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
{
    //AWARE: At this moment is not possible to use shape information because, as
    //layouting continues and more objects are added to the line, the shape
    //position or its geometry could change (i.e chord engraving). Therefore,
    //information from the shape is obtained later, when starting justification,
    //and method LineEntry::add_shape_info() is invoked.
    m_fShapeInfoLoaded = false;
}

//---------------------------------------------------------------------------------------
//constructor for unit tests
LineEntry::LineEntry(bool fIsBarlineEntry, bool fProlog, float rTime, LUnits xAnchor,
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
        //TODO
        //LUnits uShift = m_xLeft - m_pShape->get_left();
//        m_pSO->StoreOriginAndShiftShapes( uShift, m_pShape->GetOwnerIDX() );
        m_pShape->set_origin_and_notify_observers(m_xLeft, sliceOrg.y);
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
		outStream << "  pSO "
					<< setw(4) << m_pSO->get_obj_type()
					<< setw(4) << m_pSO->get_id()
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
        outStream << "    --" << endl;  //\r\n";
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
LineEntry* MusicLine::add_entry(ImoStaffObj* pSO, GmoShape* pShape, float rTime,
                                bool fInProlog)
{
    LineEntry* pEntry = LOMSE_NEW LineEntry(pSO, pShape, fInProlog, rTime);
    push_back(pEntry);
	return pEntry;
}

//---------------------------------------------------------------------------------------
void MusicLine::add_shapes(GmoBoxSliceInstr* pSliceInstrBox)
{
    for (LineEntryIterator it = m_LineEntries.begin(); it != m_LineEntries.end(); ++it)
    {
        if ((*it)->get_shape())
            pSliceInstrBox->add_shape((*it)->get_shape(), GmoShape::k_layer_notes);
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
                << endl //"\r\n"
                << "============================================================================================================="
                << endl << endl;    //\r\n\r\n";

    if (size() == 0)
    {
        outStream << "The table is empty." << endl; //\r\n";
        return;
    }

    //headers
    LineEntry::dump_header(outStream);

    //loop to dump table entries
    for (int i = 0; i < (int)size(); i++)
    {
        if (i % 4 == 0) {
            outStream << "-------------------------------------------------------------------------------------------------------------"
                      << endl;  //\r\n";
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
        if (m_rFirstTime == LOMSE_NO_TIME && (*it)->get_timepos() >= 0.0f)
            m_rFirstTime = (*it)->get_timepos();

        if (is_equal_time(m_rFirstTime, (*it)->get_timepos()))
            m_uxFirstSymbol = min(m_uxFirstSymbol, m_uxFirstAnchor + (*it)->get_anchor());
    }
    if (m_rFirstTime == LOMSE_NO_TIME)
    {
        m_uxFirstSymbol = m_uxFirstAnchor;
        m_rFirstTime = -1.0f;
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
//ColumnLayouter
//=======================================================================================
ColumnLayouter::ColumnLayouter(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                               ColumnStorage* pStorage)
    : m_libraryScope(libraryScope)
    , m_pColStorage(pStorage)
    , m_pScoreMeter(pScoreMeter)
    , m_fHasSystemBreak(false)
    , m_penalty(1.0f)
{
    reserve_space_for_prolog_clefs_keys( m_pScoreMeter->num_staves() );
}

//---------------------------------------------------------------------------------------
ColumnLayouter::~ColumnLayouter()
{
    delete_line_spacers();
    delete m_pColStorage;
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::reserve_space_for_prolog_clefs_keys(int numStaves)
{
    m_prologClefs.clear();
    m_prologClefs.reserve(numStaves);
    m_prologClefs.assign(numStaves, (ColStaffObjsEntry*)NULL);     //GCC complais if NULL not casted

    m_prologKeys.clear();
    m_prologKeys.reserve(numStaves);
    m_prologKeys.assign(numStaves, (ColStaffObjsEntry*)NULL);
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
void ColumnLayouter::do_spacing(bool fTrace)
{
    //computes the minimum space required by this column

    if (fTrace || m_libraryScope.dump_column_tables())
    {
        dbgLogger << "******************* Before spacing" << endl;  //\r\n";
        m_pColStorage->dump_column_storage(dbgLogger);
    }

    compute_spacing();
    compute_penalty_factor();
    m_pColStorage->determine_sizes();

    if (fTrace || m_libraryScope.dump_column_tables())
    {
        dbgLogger << "******************* After spacing" << endl;  //\r\n";
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
        process_timed_at_current_timepos();
        while (there_are_objects())
        {
            process_non_timed_at_current_timepos();
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
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_timed_at_current_timepos()
{
    m_fThereAreObjects = false;
    float rNextTime = LOMSE_NO_TIME;           //any impossible high value
    LUnits uxPosForNextTime = LOMSE_NO_POSITION;    //any impossible high value

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
        m_uCurrentPos = uxPosForNextTime;
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::process_non_timed_at_current_timepos()
{
    LUnits uxPosForNextTime = 0.0f;
    for (LineSpacersIterator it=m_LineSpacers.begin(); it != m_LineSpacers.end(); ++it)
	{
        (*it)->process_non_timed_at_current_timepos(m_uCurrentPos);
        LUnits uxNextPos = (*it)->get_next_position();
        uxPosForNextTime = max(uxPosForNextTime, uxNextPos);
    }
    m_uCurrentPos = uxPosForNextTime;
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
void ColumnLayouter::add_shapes_to_boxes(ShapesStorage* pStorage)
{
    for (int iInstr=0; iInstr < int(m_sliceInstrBoxes.size()); ++iInstr)
    {
        m_pColStorage->add_shapes(m_sliceInstrBoxes[iInstr], iInstr);
        pStorage->add_ready_shapes_to_model( m_sliceInstrBoxes[iInstr] );
    }
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::delete_box_and_shapes(ShapesStorage* pStorage)
{
    m_pColStorage->delete_shapes();
    pStorage->delete_ready_shapes();

   delete m_pBoxSlice;       //box for this column
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::set_slice_width(LUnits width)
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
void ColumnLayouter::set_slice_final_position(LUnits left, LUnits top)
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
LUnits ColumnLayouter::redistribute_space(LUnits uNewStart, LUnits uNewWidth,
                                          UPoint org)
{
    ColumnResizer oResizer(m_pColStorage, uNewWidth);
	return oResizer.reposition_shapes(uNewStart, uNewWidth, org);
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::start_column_measurements(LUnits uxStart, LUnits fixedSpace)
{
    //prepare to receive data for this column

    m_pColStorage->set_start_position(uxStart);
    m_pColStorage->set_initial_space(fixedSpace);
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::include_object(int iLine, int iInstr, ImoStaffObj* pSO, float rTime,
                                   int nStaff, GmoShape* pShape, bool fInProlog)
{
    //caller sends data about one staffobj in current bar, for column iCol [0..n-1]

    m_pColStorage->include_object(iLine, iInstr, pSO, rTime, nStaff, pShape, fInProlog);
}

//---------------------------------------------------------------------------------------
void ColumnLayouter::finish_column_measurements(LUnits xStart)
{
    m_pColStorage->finish_column_measurements(xStart);
}

//---------------------------------------------------------------------------------------
LUnits ColumnLayouter::get_start_of_column()
{
    return m_pColStorage->get_start_of_column();
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

//---------------------------------------------------------------------------------------
void ColumnLayouter::save_context(int iInstr, int iStaff, ColStaffObjsEntry* pClefEntry,
                                  ColStaffObjsEntry* pKeyEntry)
{
    int idx = m_pScoreMeter->staff_index(iInstr, iStaff);
    m_prologClefs[idx] = pClefEntry;
    m_prologKeys[idx] = pKeyEntry;
}



//=======================================================================================
// SystemLayouter implementation
//=======================================================================================
SystemLayouter::SystemLayouter(ScoreLayouter* pScoreLyt, LibraryScope& libraryScope,
                               ScoreMeter* pScoreMeter, ImoScore* pScore,
                               ShapesStorage& shapesStorage,
                               ShapesCreator* pShapesCreator,
                               std::vector<ColumnLayouter*>& colLayouters,
                               std::vector<InstrumentEngraver*>& instrEngravers)
    : m_pScoreLyt(pScoreLyt)
    , m_libraryScope(libraryScope)
    , m_pScoreMeter(pScoreMeter)
    , m_pScore(pScore)
    , m_shapesStorage(shapesStorage)
    , m_pShapesCreator(pShapesCreator)
    , m_ColLayouters(colLayouters)
    , m_instrEngravers(instrEngravers)
    , m_uPrologWidth(0.0f)
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
    engrave_system_details(m_iSystem);

    if (m_pScoreLyt->is_last_system() && m_pScoreLyt->m_fStopStaffLinesAtFinalBarline)
        truncate_current_system(indent);

    engrave_instrument_details();

    if (!m_pScoreLyt->is_system_empty(m_iSystem))
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

    LUnits width = m_pBoxSystem->get_content_width_old();
    LUnits left = m_pBoxSystem->get_left();
    int maxInstr = m_pScore->get_num_instruments() - 1;
    for (int iInstr = 0; iInstr <= maxInstr; iInstr++)
    {
        InstrumentEngraver* engrv = m_instrEngravers[iInstr];
        engrv->set_staves_horizontal_position(left, width, indent);
        engrv->set_slice_instr_origin(org);
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::fill_current_system_with_columns()
{
    m_pScoreLyt->m_iCurColumn = 0;
    if (m_pScoreLyt->get_num_systems() == 0)
        return;

    InstrumentEngraver* pEngrv = m_instrEngravers[0];
    m_pagePos.x = pEngrv->get_staves_left();

    m_uFreeSpace = (m_iSystem == 0 ? m_pScoreLyt->get_first_system_staves_size()
                                   : m_pScoreLyt->get_other_systems_staves_size() );

    m_fFirstColumnInSystem = true;
    for (int iCol = m_iFirstCol; iCol < m_iLastCol; ++iCol)
    {
        m_pScoreLyt->m_iCurColumn = iCol;
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
    add_system_prolog_if_necessary();

    m_pagePos.x = determine_column_start_position(iCol);
    LUnits size = determine_column_size(iCol);

    reposition_and_add_slice_boxes(iCol, m_pagePos.x, size);
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
    if (is_first_column_in_system())
        return m_pagePos.x;
    else
    {
        LUnits uEndHookWidth = m_ColLayouters[iCol-1]->get_end_hook_width();
        LUnits uStartHookWidth = m_ColLayouters[iCol]->get_start_hook_width();
        LUnits xEndPrev = m_pagePos.x;
        if (uEndHookWidth > uStartHookWidth)
            return xEndPrev + (uEndHookWidth - uStartHookWidth);
        else
            return xEndPrev;
    }
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::determine_column_size(int iCol)
{
    LUnits size = m_ColLayouters[iCol]->get_main_width();
    if (!is_first_column_in_system())
    {
        LUnits uEndHook = m_ColLayouters[iCol-1]->get_end_hook_width();
        LUnits uStartHook = m_ColLayouters[iCol]->get_start_hook_width();
        if (uEndHook > uStartHook)
            size += (uEndHook - uStartHook);
    }
    return size;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_and_add_slice_boxes(int iCol, LUnits pos, LUnits size)
{
    LUnits ySystem = m_pBoxSystem->get_top();
    m_ColLayouters[iCol]->set_slice_final_position(pos, ySystem);

    GmoBoxSlice* pSlice = m_ColLayouters[iCol]->get_slice_box();
    m_pBoxSystem->add_child_box(pSlice);
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::engrave_prolog(int iInstr)
{
    LUnits uPrologWidth = 0.0f;

    //AWARE when this method is invoked the paper position is at the left marging,
    //at the start of the new system.
    LUnits xStartPos = m_pagePos.x;      //Save x to align all clefs

    //iterate over the collection of lmStaff objects to draw current clef and key signature
    ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);

    GmoBoxSystem* pBox = get_box_system();

    for (int iStaff=0; iStaff < pInstr->get_num_staves(); ++iStaff)
    {
        LUnits xPos = xStartPos;
        m_pagePos.y = m_instrEngravers[iInstr]->get_top_line_of_staff(iStaff);
        int iStaffIndex = m_pScoreMeter->staff_index(iInstr, iStaff);
        ColStaffObjsEntry* pClefEntry =
            m_ColLayouters[m_iFirstCol]->get_prolog_clef(iStaffIndex);
        ColStaffObjsEntry* pKeyEntry =
            m_ColLayouters[m_iFirstCol]->get_prolog_key(iStaffIndex);
        ImoClef* pClef = pClefEntry ? dynamic_cast<ImoClef*>(pClefEntry->imo_object())
                                    : NULL;
        int clefType = pClef ? pClef->get_clef_type() : k_clef_undefined;

        //add clef shape
        if (pClefEntry)
        {
            ImoClef* pClef = dynamic_cast<ImoClef*>( pClefEntry->imo_object() );
            if (pClef && pClef->is_visible())
            {
                xPos += m_pScoreMeter->tenths_to_logical(LOMSE_SPACE_BEFORE_PROLOG, iInstr, iStaff);
                m_pagePos.x = xPos;
                GmoShape* pShape =
                    m_pShapesCreator->create_staffobj_shape(pClef, iInstr, iStaff,
                                                            m_pagePos);
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
    m_ColLayouters[iCol]->add_shapes_to_boxes(pStorage);
}

//---------------------------------------------------------------------------------------
bool SystemLayouter::system_must_be_justified()
{
    //all systems needs justification except:

    //1. unless justification supressed, for debugging
    if (!m_libraryScope.justify_systems())
        return false;

    //2. it is the last system and flag "JustifyFinalBarline" is not set
    if (m_pScoreLyt->is_last_system() && !m_pScoreLyt->m_fJustifyFinalBarline)
        return false;

    //3. it is the last system but there is no final barline
    int iLastCol = m_pScoreLyt->get_num_columns();
    if (m_pScoreLyt->is_last_system() && !m_ColLayouters[iLastCol]->column_has_barline())
        return false;

    return true;        //do justification
}

//---------------------------------------------------------------------------------------
void SystemLayouter::reposition_slices_and_staffobjs()
{
    GmoBoxSlice* pFirstSlice = m_ColLayouters[m_iFirstCol]->get_slice_box();
    LUnits xLeft = pFirstSlice->get_left();
    LUnits yTop = pFirstSlice->get_top();
    LUnits xStartPos = m_ColLayouters[m_iFirstCol]->get_start_of_column();

    for (int iCol = m_iFirstCol; iCol < m_iLastCol; ++iCol)
    {
        //reposition boxes
        m_ColLayouters[iCol]->set_slice_final_position(xLeft, yTop);
        xLeft += m_ColLayouters[iCol]->get_justified_width();

        //reposition staffobjs
        LUnits xEndPos = redistribute_space(iCol, xStartPos);

        //assign justified width to boxes
        m_ColLayouters[iCol]->set_slice_width(xEndPos - xStartPos);
    }
}

//---------------------------------------------------------------------------------------
LUnits SystemLayouter::redistribute_space(int iCol, LUnits uNewStart)
{
    LUnits uNewWidth = m_ColLayouters[iCol]->get_justified_width();
    GmoBoxSlice* pBox = m_ColLayouters[iCol]->get_slice_box();

    //step 3: map to system
    UPoint org = pBox->get_origin();
    org.y += m_pScoreLyt->determine_top_space(0);

    return m_ColLayouters[iCol]->redistribute_space(uNewStart, uNewWidth, org);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::redistribute_free_space()
{
    if (m_uFreeSpace <= 0.0f)
        return;           //no space to distribute

    //Space is redistributed proportionally to actual width

    //compute total occupied
    LUnits uTotal = 0.0f;
    for (int i = m_iFirstCol; i < m_iLastCol; ++i)
    {
         uTotal += m_ColLayouters[i]->get_trimmed_width();
    }

    //proportion factor
    float alpha = (m_uFreeSpace + uTotal) / uTotal;

    //assign new sizt to columns
    for (int i = m_iFirstCol; i < m_iLastCol; ++i)
    {
        float newSize = alpha * m_ColLayouters[i]->get_trimmed_width();
        m_ColLayouters[i]->set_justified_width(newSize);
    }


#if 0
    //Space is redistributed to try to have all columns with equal witdh.

    //compute average column size and total occupied
    LUnits uTotal = 0.0f;
    int nColumnsInSystem = m_iLastCol - m_iFirstCol;
    for (int i = m_iFirstCol; i < m_iLastCol; ++i)
    {
         uTotal += m_ColLayouters[i]->get_trimmed_width();
    }
    LUnits uAverage = (uTotal + m_uFreeSpace) / nColumnsInSystem;

    //For each column, compute the diference between its size and the average target size.
    //Sum up all the diferences in uDifTotal
    std::vector<LUnits> uDif(nColumnsInSystem, 0.0f);
    LUnits uDifTotal = 0;
    int nNumSmallerColumns = 0;      //num of columns smaller than average
    for (int i = 0; i < nColumnsInSystem; i++)
    {
        uDif[i] = uAverage - m_ColLayouters[i + m_iFirstCol]->get_trimmed_width();
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
        for (int i = 0; i < nColumnsInSystem; i++)
        {
            if (uDif[i] > 0.0f)
            {
                uDif[i] -= uReduce;
                m_ColLayouters[i+m_iFirstCol]->increment_justified_width(uDif[i]);
            }
        }
    }
    else
    {
        //enough space to make all columns equal size
        for (int i = 0; i < nColumnsInSystem; i++)
        {
            if (uDif[i] > 0.0f)
                m_ColLayouters[i+m_iFirstCol]->increment_justified_width(uDif[i]);
        }
    }
#endif
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_initial_line_joining_all_staves_in_system()
{
    //TODO: In current code, the decision about joining staves depends only on first
    //instrument. This should be changed and the line should go from first visible
    //staff to last visible one.

    if (m_pScoreMeter->is_empty_score())
        return;

    //TODO: HideStaffLines option
//    lmVStaff* pVStaff = m_pScore->GetFirstInstrument()->GetVStaff();
	if (m_pScoreMeter->must_draw_left_barline())    //&& !pVStaff->HideStaffLines() )
	{
        ImoObj* pCreator = m_pScore->get_instrument(0);
        LUnits xPos = m_instrEngravers[0]->get_staves_left();
        LUnits yTop = m_instrEngravers[0]->get_staves_top_line();
        int iInstr = m_pScoreMeter->num_instruments() - 1;
        LUnits yBottom = m_instrEngravers[iInstr]->get_staves_bottom_line();
        LUnits uLineThickness =
            m_pScoreMeter->tenths_to_logical(LOMSE_THIN_LINE_WIDTH, 0, 0);
        GmoShape* pLine = LOMSE_NEW GmoShapeBarline(pCreator, 0, ImoBarline::k_simple,
                                              xPos, yTop, yBottom,
                                              uLineThickness, uLineThickness,
                                              0.0f, 0.0f, Color(0,0,0), uLineThickness);
        m_pBoxSystem->add_shape(pLine, GmoShape::k_layer_staff);
	}
}

//---------------------------------------------------------------------------------------
void SystemLayouter::truncate_current_system(LUnits indent)
{
    if (m_pScoreMeter->is_empty_score())
        return;

    if (!m_ColLayouters[m_iLastCol-1]->column_has_barline())
        return;

    GmoBoxSlice* pSlice = m_ColLayouters[m_iLastCol-1]->get_slice_box();
    m_pBoxSystem->set_width( pSlice->get_right() - m_pBoxSystem->get_left() );
    reposition_staves(indent);
}

//---------------------------------------------------------------------------------------
void SystemLayouter::engrave_instrument_details()
{
    int maxInstr = m_pScore->get_num_instruments() - 1;
    for (int iInstr = 0; iInstr <= maxInstr; iInstr++)
    {
        InstrumentEngraver* engrv = m_instrEngravers[iInstr];
        engrv->add_staff_lines(m_pBoxSystem);
        engrv->add_name_abbrev(m_pBoxSystem, m_iSystem);
        engrv->add_brace_bracket(m_pBoxSystem);
    }
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
                                     (*it)->m_iInstr, (*it)->m_iStaff,
                                     objSystem,
                                     (*it)->m_iCol, (*it)->m_iLine,
                                     (*it)->m_pInstr );
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
                                                            iLine, pInstr, m_pagePos);
		        else if (pSO == pRO->get_end_object())
		        {
                    SystemLayouter* pSysLyt = m_pScoreLyt->get_system_layouter(iSystem);
                    LUnits prologWidth( pSysLyt->get_prolog_width() );

                    m_pShapesCreator->finish_engraving_relobj(pRO, pSO, pMainShape,
                                                            iInstr, iStaff, iSystem, iCol,
                                                            iLine, prologWidth, pInstr);
                    add_auxobjs_shapes_to_model(pRO, pMainShape, GmoShape::k_layer_aux_objs);
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

            GmoShape* pAuxShape =
                        m_pShapesCreator->create_auxobj_shape(pAO, iInstr, iStaff,
                                                            pMainShape, m_pagePos);
            pMainShape->accept_link_from(pAuxShape);
            add_auxobj_shape_to_model(pAuxShape, GmoShape::k_layer_aux_objs, iSystem,
                                        iCol, iInstr);
        }
    }
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_auxobjs_shapes_to_model(ImoObj* pAO, GmoShape* pStaffObjShape,
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

        //pStaffObjShape->accept_link_from(pAuxShape);
        add_auxobj_shape_to_model(pAuxShape, layer, iSystem, iCol, iInstr);
    }

    m_shapesStorage.remove_engraver(pAO);
    delete pEngrv;
}

//---------------------------------------------------------------------------------------
void SystemLayouter::add_auxobj_shape_to_model(GmoShape* pShape, int layer, int iSystem,
                                              int iCol, int iInstr)
{
    pShape->set_layer(layer);
    GmoBoxSliceInstr* pBox = m_ColLayouters[iCol]->get_slice_instr(iInstr);
    pBox->add_shape(pShape, layer);
}




//////---------------------------------------------------------------------------------------
////void SystemLayouter::AddTimeGridToBoxSlice(int iCol, GmoBoxSlice* pBSlice)
////{
////    //create the time-grid table and transfer it (and its ownership) to GmoBoxSlice
////    pBSlice->SetTimeGridTable( new TimeGridTable(m_ColStorage[iCol]) );
////}





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
    outStream << "Start of dump. ColumnStorage" << endl;    //\r\n";
	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
	{
        (*it)->dump_music_line(outStream);
    }
}

//---------------------------------------------------------------------------------------
void ColumnStorage::determine_sizes()
{
    float rFirstTime = 10000000.0f;
	for (LinesIterator it = m_Lines.begin(); it != m_Lines.end(); ++it)
    {
        (*it)->do_measurements();

        float rTime = (*it)->get_first_time();
        if (rTime >= 0.0f)
            rFirstTime = min(rFirstTime, rTime);
    }
    if (rFirstTime == 10000000.0f)
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
void ColumnStorage::close_all_lines(LUnits xStart)
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
bool ColumnStorage::include_object(int line, int instr, ImoStaffObj* pSO, float rTime,
                                  int nStaff, GmoShape* pShape, bool fInProlog)
{
    //if doesn't exist, start it
    LinesIterator it = find_line(line);
    if (is_end_of_table(it))
        it = start_line(line, instr);

    //add new entry for this object
	LineEntry* pCurEntry = (*it)->add_entry(pSO, pShape, rTime, fInProlog);

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
{
}

//---------------------------------------------------------------------------------------
float LineResizer::move_prolog_shapes()
{
    //all non-timed entries, at beginning, marked as fProlog must be only re-located
    //returns the first timepos found after the prolog or 0 if no valid timepos

    LUnits uLineStartPos = m_pLine->get_line_start_position();
    LUnits uLineShift = m_uNewStart - uLineStartPos;
    LineEntryIterator it = m_pLine->begin();
    while (it != m_pLine->end() && (*it)->get_timepos() < 0.0f)
    {
        if ((*it)->get_shape())
        {
            if ((*it)->is_prolog_object())
            {
                LUnits uNewPos = uLineShift + (*it)->get_position();
                (*it)->reposition_at(uNewPos);
                (*it)->move_shape(m_sliceOrg);
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
    if (m_itCurrent != m_pLine->end()
        && is_equal_time((*m_itCurrent)->get_timepos(), rFirstTime) )
    {
        return (*m_itCurrent)->get_position() - (*m_itCurrent)->get_anchor();
    }
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
void LineResizer::reasign_position_to_all_other_objects(LUnits uFizedSizeAtStart)
{
    if (m_itCurrent == m_pLine->end())
        return;

    //Compute proportion factor
    LUnits uLineStartPos = m_pLine->get_line_start_position();
    LUnits uDiscount = uFizedSizeAtStart - uLineStartPos;
    float rProp = (m_uNewWidth-uDiscount) / (m_uOldWidth-uDiscount);

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
    , m_rCurTime(0.0f)
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
LUnits LineSpacer::determine_next_feasible_position_after(LUnits uxPos)
{
    return m_uxCurPos + compute_shift_to_avoid_overlap_with_previous();
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
float LineSpacer::get_next_available_time()
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
            pEntry->set_fixed_space(0.0f);
            pEntry->set_variable_space(0.0f);
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
    int iInstr = m_pLine->get_instrument();
    ImoStaffObj* pSO = pEntry->get_staffobj();
	int iStaff = pSO->get_staff();
    pEntry->set_fixed_space( m_pMeter->tenths_to_logical(LOMSE_EXCEPTIONAL_MIN_SPACE,
                                                         iInstr, iStaff) );
}



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
        TimeGridLineExplorer* pLinExplorer = LOMSE_NEW TimeGridLineExplorer(*it);
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
//                      //   .......+.......+.......+
//    std::string sDump = _T("\n timepos     Dur     Pos\n");
//    std::vector<PosTimeItem>::iterator it;
//    for (it = m_PosTimes.begin(); it != m_PosTimes.end(); ++it)
//    {
//        sDump += std::string::Format(_T("%8.2f %8.2f %8.2f\n"),
//                                (*it).rTimepos, (*it).rDuration, (*it).uxPos );
//    }
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
ColumnResizer::ColumnResizer(ColumnStorage* pColStorage, LUnits uNewWidth)
    : m_pColStorage(pColStorage)
    , m_uNewWidth(uNewWidth)
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
void ColumnResizer::delete_line_resizers()
{
    std::vector<LineResizer*>::iterator itR;
	for (itR = m_LineResizers.begin(); itR != m_LineResizers.end(); ++itR)
		delete *itR;
    m_LineResizers.clear();
}


}  //namespace lomse
