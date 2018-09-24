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
TieEngraver::TieEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                         LUnits uStaffLeft, LUnits uStaffRight)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_uStaffLeft(uStaffLeft)
    , m_uStaffRight(uStaffRight)
    , m_pTie(nullptr)
    , m_numShapes(0)
    , m_pStartNote(nullptr)
    , m_pEndNote(nullptr)
    , m_pStartNoteShape(nullptr)
    , m_pEndNoteShape(nullptr)
    , m_thickness(1.0f)
    , m_fTieBelowNote(false)
    , m_fTwoArches(false)
{
}

//---------------------------------------------------------------------------------------
void TieEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                     int iSystem, int iCol, LUnits UNUSED(xRight),
                                     LUnits UNUSED(xLeft), LUnits UNUSED(yTop))
{
    m_pTie = dynamic_cast<ImoTie*>( pRO );

    m_pStartNote = dynamic_cast<ImoNote*>(pSO);
    m_pStartNoteShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);
    m_iInstr = iInstr;
    m_iStaff = iStaff;

    m_shapesInfo[0].iCol = iCol;
    m_shapesInfo[0].iInstr = iInstr;
    m_shapesInfo[0].iSystem = iSystem;
}

//---------------------------------------------------------------------------------------
void TieEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                   GmoShape* pStaffObjShape, int iInstr,
                                   int UNUSED(iStaff), int iSystem, int iCol,
                                   LUnits UNUSED(xRight), LUnits UNUSED(xLeft),
                                   LUnits UNUSED(yTop))
{
    m_pEndNote = dynamic_cast<ImoNote*>(pSO);
    m_pEndNoteShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);
    m_shapesInfo[1].iCol = iCol;
    m_shapesInfo[1].iInstr = iInstr;
    m_shapesInfo[1].iSystem = iSystem;
}

//---------------------------------------------------------------------------------------
int TieEngraver::create_shapes(Color color)
{
    m_color = color;
    decide_placement();
    decide_if_one_or_two_arches();
    if (two_arches_needed())
        create_two_shapes();
    else
        create_one_shape();
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
void TieEngraver::create_one_shape()
{
    m_numShapes = 1;

    compute_start_point();
    compute_end_point(&m_points1[ImoBezierInfo::k_end]);
    compute_default_control_points(&m_points1[0]);
    add_user_displacements(0, &m_points1[0]);
    m_shapesInfo[0].pShape =
        LOMSE_NEW GmoShapeTie(m_pTie, 0, &m_points1[0], m_thickness, m_color);

    m_shapesInfo[1].pShape = nullptr;
}

//---------------------------------------------------------------------------------------
void TieEngraver::create_two_shapes()
{
    m_numShapes = 2;

    //create first shape
    compute_start_point();
    compute_end_of_staff_point();
    compute_default_control_points(&m_points1[0]);
    add_user_displacements(0, &m_points1[0]);
    GmoShapeTie* pTieShape =
        LOMSE_NEW GmoShapeTie(m_pTie, 0, &m_points1[0], m_thickness, m_color);
    m_shapesInfo[0].pShape = pTieShape;
    add_voice(pTieShape);

    //create second shape
    compute_end_point(&m_points2[ImoBezierInfo::k_end]);
    compute_start_of_staff_point();
    compute_default_control_points(&m_points2[0]);
    add_user_displacements(1, &m_points2[0]);
    pTieShape = LOMSE_NEW GmoShapeTie(m_pTie, 1, &m_points2[0], m_thickness, m_color);
    m_shapesInfo[1].pShape = pTieShape;
    add_voice(pTieShape);
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
    LUnits hc = m_thickness * 3.88f;
    (points+ImoBezierInfo::k_ctrol1)->x = (points+ImoBezierInfo::k_start)->x + d;
    (points+ImoBezierInfo::k_ctrol1)->y = (points+ImoBezierInfo::k_start)->y + (m_fTieBelowNote ? hc : -hc);

    (points+ImoBezierInfo::k_ctrol2)->x = (points+ImoBezierInfo::k_end)->x - d;
    (points+ImoBezierInfo::k_ctrol2)->y = (points+ImoBezierInfo::k_end)->y + (m_fTieBelowNote ? hc : -hc);
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_start_point()
{
	//x pos: center on notehead
	m_points1[ImoBezierInfo::k_start].x = (m_pStartNoteShape->get_notehead_right() +
                          m_pStartNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths apart from notehead
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    m_points1[ImoBezierInfo::k_start].y = (m_fTieBelowNote ?
                          m_pStartNoteShape->get_notehead_bottom() + space
                          : m_pStartNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_end_point(UPoint* point)
{
	//x pos: center on notehead
	point->x = (m_pEndNoteShape->get_notehead_right() +
                m_pEndNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths apart from notehead
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    point->y = (m_fTieBelowNote ?
                 m_pEndNoteShape->get_notehead_bottom() + space
               : m_pEndNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_start_of_staff_point()
{
    m_points2[ImoBezierInfo::k_start].x = m_uStaffLeft;
    m_points2[ImoBezierInfo::k_start].y = m_points2[ImoBezierInfo::k_end].y;
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_end_of_staff_point()
{
    m_points1[ImoBezierInfo::k_end].x = m_uStaffRight;
    m_points1[ImoBezierInfo::k_end].y = m_points1[ImoBezierInfo::k_start].y;
}

//---------------------------------------------------------------------------------------
void TieEngraver::decide_placement()
{
    if (m_pTie->get_orientation() == k_orientation_default)
        m_fTieBelowNote = m_pStartNoteShape->is_up();
    else
        m_fTieBelowNote = m_pTie->get_orientation() == k_orientation_under;
}

//---------------------------------------------------------------------------------------
void TieEngraver::decide_if_one_or_two_arches()
{
    m_fTwoArches = (m_shapesInfo[0].iSystem != m_shapesInfo[1].iSystem);
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

//---------------------------------------------------------------------------------------
int TieEngraver::get_num_shapes()
{
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
ShapeBoxInfo* TieEngraver::get_shape_box_info(int i)
{
    return &m_shapesInfo[i];
}


}  //namespace lomse
