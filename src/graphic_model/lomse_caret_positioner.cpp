//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
#include "lomse_logger.h"

//other
#include <sstream>


namespace lomse
{

//=======================================================================================
// CaretPositioner implementation
//=======================================================================================
CaretPositioner::CaretPositioner(DocCursor* pCursor)
    : m_pCursor(pCursor)
{
}

//---------------------------------------------------------------------------------------
void CaretPositioner::prepare_caret(Caret* pCaret, GraphicModel* pGModel)
{
    if (m_pCursor->is_delegating())
    {
        InnerLevelCaretPositioner* p = new_positioner(m_pCursor, pGModel);
        p->prepare_caret(pCaret);
    }
    else
    {
        TopLevelCaretPositioner p(m_pCursor, pGModel);
        p.prepare_caret(pCaret);
    }
}

//---------------------------------------------------------------------------------------
InnerLevelCaretPositioner* CaretPositioner::new_positioner(DocCursor* pCursor,
                                                           GraphicModel* pGModel)
{
    //factory method to create a positioner of right type
    return LOMSE_NEW ScoreCaretPositioner(pCursor, pGModel);
}


//=======================================================================================
// TopLevelCaretPositioner implementation
//=======================================================================================
TopLevelCaretPositioner::TopLevelCaretPositioner(DocCursor* pCursor,
                                                 GraphicModel* pGModel)
    : m_pCursor(pCursor)
    , m_pGModel(pGModel)
{
    m_state = m_pCursor->get_state();
}

//---------------------------------------------------------------------------------------
void TopLevelCaretPositioner::prepare_caret(Caret* pCaret)
{
    ImoId id = m_state.get_top_level_id();
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
    ImoId id = cursor.get_top_id();
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
ScoreCaretPositioner::ScoreCaretPositioner(DocCursor* pCursor,
                                           GraphicModel* pGModel)
    : InnerLevelCaretPositioner(pGModel)
    , m_pDocCursor(pCursor)
    , m_pScoreCursor( static_cast<ScoreCursor*>(pCursor->get_inner_cursor()) )
    , m_pDoc( pCursor->get_document() )

{
    //get score cursor state
    DocCursorState state = m_pDocCursor->get_state();
    m_pState = static_cast<ScoreCursorState*>( state.get_delegate_state() );

    //get score
    ImoId id = state.get_top_level_id();
    m_pScore = static_cast<ImoScore*>( m_pDoc->get_pointer_to_imo(id) );

    //create score meter
    m_pMeter = LOMSE_NEW ScoreMeter(m_pScore);
}

//---------------------------------------------------------------------------------------
ScoreCaretPositioner::~ScoreCaretPositioner()
{
    delete m_pMeter;
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::prepare_caret(Caret* pCaret)
{
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
        LOMSE_LOG_ERROR("[ScoreCaretPositioner::prepare_caret] Score cursor is incoherent!");
        throw runtime_error("[ScoreCaretPositioner::prepare_caret] Score cursor is incoherent!");
    }
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::caret_on_pointed_object(Caret* pCaret)
{
    URect bounds = get_bounds_for_imo( m_pState->id() );
    bounds.x -= tenths_to_logical(3);
    set_caret_y_pos_and_height(&bounds, m_pState->id());

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

    ColStaffObjsEntry* pEntry = m_pScoreCursor->find_previous_imo();
    ImoId prevId = pEntry->element_id();
    ImoId refId = m_pState->ref_obj_id();
//    int instr = m_pState->instrument();
//    int staff = m_pState->staff();

    URect boundsPrev = get_bounds_for_imo(prevId);
    URect bounds = get_bounds_for_imo(refId);

    //interpolate position
    //TODO: linear interpolation is wrong. This has to be changed to use time-grid
    TimeUnits time1 = pEntry->time();
    TimeUnits time2 = m_pState->time();
    TimeUnits time3 = m_pState->ref_obj_time();
    LUnits xIncr = bounds.x - boundsPrev.x;     // Ax = x3-x1
    bounds.x = boundsPrev.x +
        (xIncr * float((time2 - time1) / (time3 - time1)) );
    set_caret_y_pos_and_height(&bounds, prevId);

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
    ImoId scoreId = state.get_top_level_id();
    GmoShapeStaff* pShape = m_pGModel->get_shape_for_first_staff_in_first_system(scoreId);

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
    set_caret_y_pos_and_height(&bounds, k_no_imoid);


    pCaret->set_type(Caret::k_line);
    pCaret->set_position( bounds.get_top_left() );
    pCaret->set_size( USize(bounds.get_width(), bounds.get_height()) );
    set_caret_timecode(pCaret);
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::caret_at_end_of_staff(Caret* pCaret)
{
    //cursor is at end of a staff or end of score. Score is not empty.
    //No current staffobj but previous one must exist.
    //Place cursor two lines (20 tenths) at the right of last staffobj

    ImoId id = m_pScoreCursor->prev_pos_id();
    URect bounds = get_bounds_for_imo(id);

    bounds.x += tenths_to_logical(20);
    set_caret_y_pos_and_height(&bounds, id);

    pCaret->set_type(Caret::k_line);
    pCaret->set_position( bounds.get_top_left() );
    pCaret->set_size( USize(bounds.get_width(), bounds.get_height()) );
    set_caret_timecode(pCaret);
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::set_caret_timecode(Caret* pCaret)
{
    ostringstream ss;
    ss << (m_pState->measure() + 1) << ".0.0." << m_pState->time();
    pCaret->set_timecode( ss.str() );
}

//---------------------------------------------------------------------------------------
URect ScoreCaretPositioner::get_bounds_for_imo(ImoId id)
{
    GmoShape* pShape = m_pGModel->get_shape_for_imo(id, 0);
    if (!pShape)
    {
        LOMSE_LOG_ERROR("[ScoreCaretPositioner::get_bounds_for_imo] No shape for requested object!");
        throw runtime_error("[ScoreCaretPositioner::get_bounds_for_imo] No shape for requested object!");
    }

    return pShape->get_bounds();
}

//---------------------------------------------------------------------------------------
LUnits ScoreCaretPositioner::tenths_to_logical(Tenths value)
{
    return m_pMeter->tenths_to_logical(value, m_pState->instrument(), m_pState->staff());
}

//---------------------------------------------------------------------------------------
void ScoreCaretPositioner::set_caret_y_pos_and_height(URect* pBounds, ImoId id)
{
    URect staffBounds;

    if (id != k_no_imoid)
    {
        GmoShape* pShape = m_pGModel->get_shape_for_imo(id, 0);
        GmoBox* pBox = pShape->get_owner_box();
        while (pBox && !pBox->is_box_system())
            pBox = pBox->get_owner_box();
        if (!pBox)
        {
            LOMSE_LOG_ERROR("[ScoreCaretPositioner::set_caret_y_pos_and_height] Invalid boxes structure");
            throw runtime_error("[ScoreCaretPositioner::set_caret_y_pos_and_height] Invalid boxes structure");
        }
        GmoBoxSystem* pSystem = static_cast<GmoBoxSystem*>(pBox);

        //get_staff_shape() requires as parameter the staff number, relative to the
        //total number of staves in the system. But we have staff number relative to
        //staves in current instrument. So we have to determine how many instruments
        //there are, and transform staff number.
        int relStaff = m_pState->staff();
        int instr = m_pState->instrument();
        if (instr > 0)
        {
            relStaff += m_pScore->get_instrument(0)->get_num_staves();
            for (int i=1; i < instr; i++)
            {
                relStaff += m_pScore->get_instrument(i)->get_num_staves();
            }
        }
        //here we have the staff number relative to total staves in system

        GmoShapeStaff* pStaff = pSystem->get_staff_shape(relStaff);
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


}  //namespace lomse
