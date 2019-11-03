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
TieEngraver::TieEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_uStaffTop(0.0f)
    , m_uStaffLeft(0.0f)
    , m_uStaffRight(0.0f)
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
void TieEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                     int UNUSED(iSystem), int UNUSED(iCol), LUnits xStaffLeft,
                                     LUnits xStaffRight, LUnits yStaffTop,
                                     int idxStaff, VerticalProfile* pVProfile)
{
    m_pTie = dynamic_cast<ImoTie*>( pRO );

    m_pStartNote = dynamic_cast<ImoNote*>(pSO);
    m_pStartNoteShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);
    m_iInstr = iInstr;
    m_iStaff = iStaff;

    m_uStaffLeft = xStaffLeft;
    m_uStaffRight = xStaffRight;
    m_uStaffTop = yStaffTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void TieEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                   GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                   int UNUSED(iStaff), int UNUSED(iSystem), int UNUSED(iCol),
                                   LUnits xStaffLeft, LUnits xStaffRight,
                                   LUnits yStaffTop, int idxStaff,
                                   VerticalProfile* pVProfile)
{
    m_pEndNote = dynamic_cast<ImoNote*>(pSO);
    m_pEndNoteShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffLeft = xStaffLeft;
    m_uStaffRight = xStaffRight;
    m_uStaffTop = yStaffTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
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
GmoShape* TieEngraver::create_first_or_intermediate_shape(Color color)
{
    m_color = color;
    if (m_numShapes == 0)
    {
        decide_placement();
        return create_first_shape();
    }
    else
        return create_intermediate_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* TieEngraver::create_last_shape(Color color)
{
    m_color = color;
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
	m_points[ImoBezierInfo::k_start].x = (m_pStartNoteShape->get_notehead_right() +
                          m_pStartNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths apart from notehead
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

    //y pos: 5 tenths apart from notehead
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    point->y = (m_fTieBelowNote ?
                 m_pEndNoteShape->get_notehead_bottom() + space
               : m_pEndNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void TieEngraver::compute_start_of_staff_point()
{
    m_points[ImoBezierInfo::k_start].x = m_uStaffLeft;
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
