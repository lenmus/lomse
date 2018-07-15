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

#include "lomse_caret_positioner.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_caret.h"
#include "lomse_shape_staff.h"
#include "lomse_staffobjs_table.h"
#include "lomse_box_system.h"
#include "lomse_box_slice_instr.h"
#include "lomse_box_slice.h"
#include "lomse_logger.h"
#include "lomse_graphical_model.h"
#include "lomse_timegrid_table.h"


//other
#include <sstream>


namespace lomse
{

//=======================================================================================
// CaretPositioner implementation
//=======================================================================================
CaretPositioner::CaretPositioner()
    : m_pCursor(nullptr)
{
}

//---------------------------------------------------------------------------------------
void CaretPositioner::layout_caret(Caret* pCaret, DocCursor* pCursor,
                                   GraphicModel* pGModel)
{
    m_pCursor = pCursor;
    if (m_pCursor->is_inside_terminal_node())
    {
        ImoObj* pTopLevel = m_pCursor->get_parent_object();
        GmoBox* pBox = pGModel->get_box_for_imo(pCursor->get_parent_id());
        if (!pBox)
        {
            LOMSE_LOG_ERROR("No box for cursor pointed object");
        }
        else
        {
            pCaret->set_top_level_box( pBox->get_bounds() );
            InnerLevelCaretPositioner* p = new_positioner(pTopLevel, pGModel);
            p->layout_caret(pCaret, m_pCursor);
        }
    }
    else
    {
        TopLevelCaretPositioner p(pGModel);
        p.layout_caret(pCaret, m_pCursor);
    }
}

//---------------------------------------------------------------------------------------
InnerLevelCaretPositioner* CaretPositioner::new_positioner(ImoObj* pTopLevel,
                                                           GraphicModel* pGModel)
{
    //factory method to create a positioner of right type

    switch (pTopLevel->get_obj_type())
    {
        //For now create always a score caret positioner
        case k_imo_score:
            return LOMSE_NEW ScoreCaretPositioner(pGModel);
        default:
            return LOMSE_NEW ScoreCaretPositioner(pGModel);
    }
}

//---------------------------------------------------------------------------------------
DocCursorState CaretPositioner::click_point_to_cursor_state(GraphicModel* pGModel,
                                int iPage, LUnits x, LUnits y, ImoObj* pImo, GmoObj* pGmo)
{
    ImoObj* pTopImo = pImo->find_block_level_parent();

    if (pTopImo->is_score())
    {
        InnerLevelCaretPositioner* p = new_positioner(pTopImo, pGModel);
        SpElementCursorState innerState =
                        p->click_point_to_cursor_state(iPage, x, y, pImo, pGmo);
        int topId = (innerState.get() == nullptr ? k_no_imoid : pTopImo->get_id());
        return DocCursorState(topId, innerState);
    }
	else
        return DocCursorState(pTopImo->get_id(), std::shared_ptr<ElementCursorState>());
}


//=======================================================================================
// TopLevelCaretPositioner implementation
//=======================================================================================
TopLevelCaretPositioner::TopLevelCaretPositioner(GraphicModel* pGModel)
    : m_pCursor(nullptr)
    , m_pGModel(pGModel)
{
}

//---------------------------------------------------------------------------------------
void TopLevelCaretPositioner::layout_caret(Caret* pCaret, DocCursor* pCursor)
{
    m_pCursor = pCursor;
    m_state = m_pCursor->get_state();

    ImoId id = m_state.get_parent_level_id();
    GmoBox* pBox = m_pGModel->get_box_for_imo(id);

    URect pos;
    if (pBox)
        pos = pBox->get_bounds();
    else
    {
        //at end of document
        pBox = get_box_for_last_element();
        if (pBox)
        {
            pos.set_top_left( UPoint(pBox->get_left(), pBox->get_bottom()) );
            pos.set_height(1000.0f);
            pos.set_width(1000.0f);
        }
        else
        {
            //empty document
            pos = URect(0.0f, 0.0f, 1000.0f, 1000.0f);
        }
    }

    pCaret->set_type(Caret::k_top_level);
    pCaret->set_top_level_box(pos);
    pCaret->set_position( pos.get_top_left() );
    pCaret->set_size( USize(pos.get_width(), pos.get_height()) );
}

//---------------------------------------------------------------------------------------
GmoBox* TopLevelCaretPositioner::get_box_for_last_element()
{
    DocCursor cursor(m_pCursor);
    cursor.to_last_top_level();
    ImoId id = cursor.get_parent_id();
    return m_pGModel->get_box_for_imo(id);
}


//=======================================================================================
// InnerLevelCaretPositioner implementation
//=======================================================================================
InnerLevelCaretPositioner::InnerLevelCaretPositioner(GraphicModel* pGModel)
    : m_pGModel(pGModel)
{
}


//=======================================================================================
// ScoreCaretPositioner implementation
//=======================================================================================
ScoreCaretPositioner::ScoreCaretPositioner(GraphicModel* pGModel)
    : InnerLevelCaretPositioner(pGModel)
    , m_pDocCursor(nullptr)
    , m_pScoreCursor(nullptr)
    , m_pDoc(nullptr)
{
}

//---------------------------------------------------------------------------------------
ScoreCaretPositioner::~ScoreCaretPositioner()
{
    delete m_pMeter;
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::layout_caret(Caret* pCaret, DocCursor* pCursor)
{
    //save cursor and related data
    m_pDocCursor = pCursor;
    m_pScoreCursor = static_cast<ScoreCursor*>(pCursor->get_inner_cursor());
    m_pDoc = pCursor->get_document();
    m_pBoxSystem = nullptr;

    //get score cursor state
    DocCursorState state = m_pDocCursor->get_state();
    m_spState = SpScoreCursorState( static_pointer_cast<ScoreCursorState>(
                                        state.get_delegate_state() ));

    //get score
    ImoId id = state.get_parent_level_id();
    m_pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(id) );

    //create score meter
    m_pMeter = LOMSE_NEW ScoreMeter(m_pScore);

    if (m_pScoreCursor->is_pointing_object())
        caret_on_pointed_object(pCaret);

    else if (m_pScoreCursor->is_at_empty_place())
        caret_on_empty_timepos(pCaret);

    else if (m_pScoreCursor->is_at_end_of_empty_score())
        caret_at_start_of_score(pCaret);

    else if (m_pScoreCursor->is_at_end_of_staff())
        caret_at_end_of_staff(pCaret);

    else
    {
        LOMSE_LOG_ERROR("[ScoreCaretPositioner::layout_caret] Score cursor is incoherent!");
        caret_at_start_of_score(pCaret);
    }
    pCaret->set_active_system(m_pBoxSystem);
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::caret_on_pointed_object(Caret* pCaret)
{
    URect bounds = get_bounds_for_imo( m_spState->id(), m_spState->staff() );
    bounds.x -= tenths_to_logical(1);
    set_caret_y_pos_and_height(&bounds, m_spState->id(), m_spState->staff());

    pCaret->set_type(Caret::k_line);
    pCaret->set_position( bounds.get_top_left() );
    pCaret->set_size( USize(bounds.get_width(), bounds.get_height()) );
    set_caret_timecode(pCaret);
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::caret_on_empty_timepos(Caret* pCaret)
{
    //cursor is between two staffobjs, on a free time position.
    //Place caret between both staffobjs. Interpolate position based on cursor time

    //get info for current ref. object
    ImoId refId = m_pScoreCursor->staffobj_id_internal();
    int refStaff = m_pScoreCursor->ref_obj_staff();

    //get info for prev object
    SpElementCursorState spState = m_pScoreCursor->get_state();
    m_pScoreCursor->move_prev();
    ImoId prevId = m_pScoreCursor->id();
    int prevStaff = m_pScoreCursor->staff();
    TimeUnits prevTime = m_pScoreCursor->time();
    m_pScoreCursor->restore_state(spState);

    URect boundsPrev = get_bounds_for_imo(prevId, prevStaff);
    URect bounds = get_bounds_for_imo(refId, refStaff);

#if 1
    //interpolate position
    //TODO: linear interpolation is wrong. This has to be changed to use time-grid
    TimeUnits time1 = prevTime;
    TimeUnits time2 = m_spState->time();
    TimeUnits time3 = m_spState->ref_obj_time();
    LUnits xIncr = bounds.x - boundsPrev.x;     // Ax = x3-x1
    bounds.x = boundsPrev.x +
        (xIncr * float((time2 - time1) / (time3 - time1)) );
#else
    //determine x position based on TimeGridTable
    TimeUnits time = m_pScoreCursor->time();
    int iSystem = m_pGModel->get_system_for(m_pScore->get_id(),
                                            m_pScoreCursor->instrument(),
                                            m_pScoreCursor->measure(),
                                            time);
    m_pBoxSystem = m_pGModel->get_system_box(iSystem);
    TimeGridTable* pTimeGrid = m_pBoxSystem->get_time_grid_table();
    bounds.x = pTimeGrid->get_x_for_note_rest_at_time(time);
#endif

    //set caret
    set_caret_y_pos_and_height(&bounds, prevId, prevStaff);

    pCaret->set_type(Caret::k_line);
    pCaret->set_position( bounds.get_top_left() );
    pCaret->set_size( USize(bounds.get_width(), bounds.get_height()) );
    set_caret_timecode(pCaret);
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::caret_at_start_of_score(Caret* pCaret)
{
    //Cursor is at end of score but score is empty.
    //Place cursor at start of first system

    //get shape for first system
    DocCursorState state = m_pDocCursor->get_state();
    ImoId scoreId = state.get_parent_level_id();

    //GmoShapeStaff* pShape = m_pGModel->get_shape_for_first_staff_in_first_system(scoreId);
    GmoBoxScorePage* pBSP =
            static_cast<GmoBoxScorePage*>( m_pGModel->get_box_for_imo(scoreId) );
    m_pBoxSystem = dynamic_cast<GmoBoxSystem*>(pBSP->get_child_box(0));
    GmoShapeStaff* pShape = (m_pBoxSystem ? m_pBoxSystem->get_staff_shape(0) : nullptr);

    URect bounds;
    if (pShape)
    {
        bounds = pShape->get_bounds();
        bounds.x += m_pMeter->tenths_to_logical(20, 0, 0);
    }
    else
    {
        //score totally empty. No staff displayed! Position cursors at start of page
        //TODO
        bounds = URect(0.0f, 0.0f, 20.0f, 700.0f);
//        uPos.y = pBPage->GetYTop();
//        uPos.x = pBPage->GetXLeft() + pScore->tenths_to_logical(20);
    }
    set_caret_y_pos_and_height(&bounds, k_no_imoid, 0);


    pCaret->set_type(Caret::k_line);
    pCaret->set_position( bounds.get_top_left() );
    pCaret->set_size( USize(bounds.get_width(), bounds.get_height()) );
    set_caret_timecode(pCaret);
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::caret_at_end_of_staff(Caret* pCaret)
{
    //cursor is at end of a staff or end of score. Score is not empty.
    //No current staffobj but a previous one must exist.
    //Place cursor 0.8 lines (8 tenths) at the right of last staffobj

    //get info for prev object
    SpElementCursorState spState = m_pScoreCursor->get_state();
    m_pScoreCursor->move_prev();
    ImoId id = m_pScoreCursor->id();
    int staff = m_pScoreCursor->staff();
    m_pScoreCursor->restore_state(spState);

    URect bounds = get_bounds_for_imo(id, staff);

    bounds.x += tenths_to_logical(8);
    set_caret_y_pos_and_height(&bounds, id, staff);

    pCaret->set_type(Caret::k_line);
    pCaret->set_position( UPoint(bounds.right(), bounds.top()) );
    pCaret->set_size( USize(bounds.get_width(), bounds.get_height()) );
    set_caret_timecode(pCaret);
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::set_caret_timecode(Caret* pCaret)
{
    TimeInfo ti = m_pScoreCursor->get_time_info();
    Timecode tc = ti.get_timecode();
    TimeUnits tu = m_spState->time();

    ostringstream ss;
    ss << tc.bar << "." << tc.beat << "." << tc.n16th << "." << tc.ticks << " (" << tu << ")";
    pCaret->set_timecode( ss.str() );
}

//---------------------------------------------------------------------------------------
URect ScoreCaretPositioner::get_bounds_for_imo(ImoId id, int iStaff)
{
    GmoShape* pShape = get_shape_for_imo(id, iStaff);
    if (!pShape)
    {
        LOMSE_LOG_ERROR("[ScoreCaretPositioner::get_bounds_for_imo] No shape for requested object!");
        return URect(0,0,0,0);
    }

    GmoBoxSliceInstr* pBSI = static_cast<GmoBoxSliceInstr*>( pShape->get_owner_box() );
    GmoBoxSlice* pBS = static_cast<GmoBoxSlice*>( pBSI->get_parent_box() );
    m_pBoxSystem = static_cast<GmoBoxSystem*>( pBS->get_parent_box() );

    return pShape->get_bounds();
}

//---------------------------------------------------------------------------------------
GmoShape* ScoreCaretPositioner::get_shape_for_imo(ImoId id, int iStaff)
{
    ImoObj* pImo = m_pDoc->get_pointer_to_imo(id);
    if (pImo && pImo->can_generate_secondary_shapes() && !pImo->is_clef())
    {
        ShapeId idx = GmoShape::generate_main_or_implicity_shape_id(iStaff);
        return m_pGModel->get_shape_for_imo(id, idx);
    }
    else
        return m_pGModel->get_main_shape_for_imo(id);
}

//---------------------------------------------------------------------------------------
LUnits ScoreCaretPositioner::tenths_to_logical(Tenths value)
{
    return m_pMeter->tenths_to_logical(value, m_spState->instrument(), m_spState->staff());
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::set_caret_y_pos_and_height(URect* pBounds, ImoId id,
                                                      int iStaff)
{
    URect staffBounds;

    if (id >= 0)
    {
        GmoShape* pShape = get_shape_for_imo(id, iStaff);
        GmoBox* pBox = pShape->get_owner_box();
        while (pBox && !pBox->is_box_system())
            pBox = pBox->get_owner_box();
        if (!pBox)
        {
            LOMSE_LOG_ERROR("[ScoreCaretPositioner::set_caret_y_pos_and_height] Invalid boxes structure");
            throw runtime_error("[ScoreCaretPositioner::set_caret_y_pos_and_height] Invalid boxes structure");
        }
        m_pBoxSystem = static_cast<GmoBoxSystem*>(pBox);

        int staff = m_spState->staff();
        int instr = m_spState->instrument();
        GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
        staffBounds = pStaff->get_bounds();
    }
    else
    {
        //score is empty
        staffBounds = *pBounds;
    }

    //set position and size
    pBounds->y = staffBounds.y - tenths_to_logical(10);
    pBounds->height = staffBounds.height + tenths_to_logical(20);
}

//---------------------------------------------------------------------------------------
SpElementCursorState ScoreCaretPositioner::click_point_to_cursor_state(int iPage,
                                        LUnits x, LUnits y, ImoObj* pImo, GmoObj* pGmo)
{
    int instr = 0;
    int staff = 0;
    int measure = 0;
    TimeUnits time = 0.0;
    ImoId id = k_no_imoid;

//    stringstream msg;
//    msg << "Click on: Gmo=" << pGmo->get_name() << ", Imo=" << pImo->get_name();
//    LOMSE_LOG_INFO(msg.str());

    //first, filter out clicks on no relevant objects
    //AWARE: For moving cursor, we are only interested on clicks in
    //staves, staff objects and empty space on score.
    //This reduces the complexity to the following cases:
    //1. click on staff: Gmo is shape staff, Imo is instrument
    //2. click out of staff, before last staffobj: Gmo is box slice instr, Imo is instrument
    //3. click out of staff, after last staffobj: Gmo is box system, Imo is score
    //4. click on the score, after final barline: Gmo is box score page, Imo is score
    //5. click on staff object.

    if (!(pGmo->is_shape_staff()          //click on a staff, pImo is instrument
          || pGmo->is_box_slice_instr()   //on the score, in empty place above/below staves, pImo is instrument
          || pGmo->is_box_system()        //on the score, after last staffobj, pImo is score
          || pGmo->is_box_score_page()    //on the score, after final barline, pImo is score
         ))
    {
        //If clicked object is a barline and this case is treated as an staffobj, cursor will
        //be always placed in staff 0. To avoid this, barlines will be ignored
        //and positioning will be based on the GmoBoxSliceInstr under them.
        if (!(pImo->is_staffobj() || pGmo->is_box()) || pImo->is_barline() )
        {
            pGmo = m_pGModel->find_inner_box_at(iPage, x, y);
            pImo = pGmo->get_creator_imo();

    //        stringstream msg;
    //        msg << "Click changed to: Gmo=" << pGmo->get_name() << ", Imo=" << pImo->get_name();
    //        LOMSE_LOG_INFO(msg.str());
        }
    }


    //now process the click
    if (pGmo)
    {
        //click on a staff object
        if (pImo->is_staffobj())
        {
            id = pImo->get_id();
            //AWARE: only id is relevant (id != k_no_imoid)
            return SpElementCursorState(
                LOMSE_NEW ScoreCursorState(instr, staff, measure, time, id, -1, 0.0, -1) );
        }

        //click on staff or score: determine time, instr & staff
        GmoBoxSystem* pBSYS = GModelAlgorithms::get_box_system_for(pGmo, y);
        if (pBSYS)
        {
            //determine time
            TimeGridTable* pTimeGrid = pBSYS->get_time_grid_table();
            //LOMSE_LOG_INFO( pTimeGrid->dump() );
            time = pTimeGrid->get_time_for_position(x);

            //determine instrument & staff
            int absStaff = pBSYS->nearest_staff_to_point(y);
            instr = pBSYS->instr_number_for_staff(absStaff);
            staff = pBSYS->staff_number_for(absStaff, instr);
            //AWARE: only instr, staff & time are relevant (id == k_no_imoid)
            return SpElementCursorState(
                LOMSE_NEW ScoreCursorState(instr, staff, measure, time, id, -1, 0.0, -1) );
        }
    }

    //other cases: ignore click
    return std::shared_ptr<ElementCursorState>();
}


}  //namespace lomse
