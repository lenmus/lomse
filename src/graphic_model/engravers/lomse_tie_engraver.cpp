//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_tie_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_tie.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// TieEngraver implementation
//---------------------------------------------------------------------------------------
TieEngraver::TieEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pTie(nullptr)
    , m_numShapes(0)
    , m_pStartNote(nullptr)
    , m_pEndNote(nullptr)
    , m_pStartNoteShape(nullptr)
    , m_pEndNoteShape(nullptr)
    , m_thickness(1.0f)
    , m_fTieBelowNote(false)
{
}

//---------------------------------------------------------------------------------------
void TieEngraver::set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc)
{
    m_iInstr = aoc.iInstr;
    m_iStaff = aoc.iStaff;
    m_idxStaff = aoc.idxStaff;

    m_pTie = dynamic_cast<ImoTie*>( pRO );

    m_pStartNote = dynamic_cast<ImoNote*>(aoc.pSO);
    m_pStartNoteShape = dynamic_cast<GmoShapeNote*>(aoc.pStaffObjShape);
}

//---------------------------------------------------------------------------------------
void TieEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), const AuxObjContext& aoc)
{
    m_pEndNote = dynamic_cast<ImoNote*>(aoc.pSO);
    m_pEndNoteShape = dynamic_cast<GmoShapeNote*>(aoc.pStaffObjShape);
}

//---------------------------------------------------------------------------------------
void TieEngraver::decide_placement()
{
    if (m_pTie->get_orientation() == k_orientation_default)
    {
        if (m_pStartNoteShape->has_stem())
            m_fTieBelowNote = m_pStartNoteShape->is_up();
        else if (m_pEndNoteShape && m_pEndNoteShape->has_stem())
            m_fTieBelowNote = m_pEndNoteShape->is_up();
        else
            m_fTieBelowNote = true;     //can go either up or down. I prefer below
    }
    else
        m_fTieBelowNote = m_pTie->get_orientation() == k_orientation_under;
}

//---------------------------------------------------------------------------------------
GmoShape* TieEngraver::create_first_or_intermediate_shape(const RelObjEngravingContext& ctx)
{
    save_context_parameters(ctx);

    if (m_numShapes == 0)
    {
        decide_placement();
        return create_first_shape();
    }
    else
        return create_intermediate_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* TieEngraver::create_last_shape(const RelObjEngravingContext& ctx)
{
    save_context_parameters(ctx);

    if (m_numShapes == 0)
    {
        decide_placement();
        return create_single_shape();
    }
    return create_final_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* TieEngraver::create_intermediate_shape()
{
    //intermediate shape spanning the whole system

    //TODO: Is it possible to have a tie spanning a whole system ?

    ++m_numShapes;
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* TieEngraver::create_single_shape()
{
    //the only shape when start and end points are in the same system

    compute_start_point();
    compute_end_point(&m_points[ImoBezierInfo::k_end]);
    compute_default_control_points(&m_points[0]);
    add_user_displacements(0, &m_points[0]);
    GmoShapeTie* pShape = LOMSE_NEW GmoShapeTie(m_pTie, m_numShapes, &m_points[0],
                                                m_thickness, m_color);

    m_numShapes++;
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* TieEngraver::create_first_shape()
{
    //first shape when there are more than one

    compute_start_point();
    compute_end_of_staff_point();
    compute_default_control_points(&m_points[0]);
    add_user_displacements(0, &m_points[0]);
    GmoShapeTie* pShape =
        LOMSE_NEW GmoShapeTie(m_pTie, m_numShapes, &m_points[0], m_thickness, m_color);
    add_voice(pShape);

    m_numShapes++;
    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* TieEngraver::create_final_shape()
{
    //last shape when there are more than one

    compute_end_point(&m_points[ImoBezierInfo::k_end]);
    compute_start_of_staff_point();
    compute_default_control_points(&m_points[0]);
    add_user_displacements(1, &m_points[0]);
    GmoShapeTie* pShape =
        LOMSE_NEW GmoShapeTie(m_pTie, m_numShapes, &m_points[0], m_thickness, m_color);
    add_voice(pShape);

    m_numShapes++;
    return pShape;
}

//---------------------------------------------------------------------------------------
void TieEngraver::add_voice(VoiceRelatedShape* pVRS)
{
    if (m_pStartNote)
        pVRS->set_voice(m_pStartNote->get_voice());
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_default_control_points(UPoint* points)
{
    LUnits D = (points+ImoBezierInfo::k_end)->x - (points+ImoBezierInfo::k_start)->x;
    LUnits d = D / 5.8f;
    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
    LUnits hc = m_thickness * 2.5f; //3.88f;
    (points+ImoBezierInfo::k_ctrol1)->x = (points+ImoBezierInfo::k_start)->x + d;
    (points+ImoBezierInfo::k_ctrol1)->y = (points+ImoBezierInfo::k_start)->y + (m_fTieBelowNote ? hc : -hc);

    (points+ImoBezierInfo::k_ctrol2)->x = (points+ImoBezierInfo::k_end)->x - d;
    (points+ImoBezierInfo::k_ctrol2)->y = (points+ImoBezierInfo::k_end)->y + (m_fTieBelowNote ? hc : -hc);
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_start_point()
{
	//x pos: center on notehead
	m_points[ImoBezierInfo::k_start].x = (m_pStartNoteShape->get_notehead_right() +
                          m_pStartNoteShape->get_notehead_left()) / 2.0f;

    //y pos: some distance apart from notehead
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    m_points[ImoBezierInfo::k_start].y = (m_fTieBelowNote ?
                          m_pStartNoteShape->get_notehead_bottom() + space
                          : m_pStartNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_end_point(UPoint* point)
{
	//x pos: center on notehead
	point->x = (m_pEndNoteShape->get_notehead_right() +
                m_pEndNoteShape->get_notehead_left()) / 2.0f;

    //y pos: some distance apart from notehead
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    point->y = (m_fTieBelowNote ?
                 m_pEndNoteShape->get_notehead_bottom() + space
               : m_pEndNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_start_of_staff_point()
{
    m_points[ImoBezierInfo::k_start].x = m_uStaffLeft + m_uPrologWidth;
    m_points[ImoBezierInfo::k_start].y = m_points[ImoBezierInfo::k_end].y;
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_end_of_staff_point()
{
    m_points[ImoBezierInfo::k_end].x = m_uStaffRight;
    m_points[ImoBezierInfo::k_end].y = m_points[ImoBezierInfo::k_start].y;
}

//---------------------------------------------------------------------------------------
void TieEngraver::add_user_displacements(int iTie, UPoint* points)
{
    ImoBezierInfo* pBezier = (iTie == 0 ? m_pTie->get_start_bezier()
                                        : m_pTie->get_stop_bezier() );
    if (pBezier)
    {
        for (int i=0; i < 4; i++)
        {
            (points+i)->x += tenths_to_logical(pBezier->get_point(i).x);
            (points+i)->y += tenths_to_logical(pBezier->get_point(i).y);
        }
    }
}


}  //namespace lomse
