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

#include "lomse_slur_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_tie.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_instrument_engraver.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// SlurEngraver implementation
//---------------------------------------------------------------------------------------
SlurEngraver::SlurEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
    , m_numShapes(0)
    , m_uPrologWidth(0.0f)
{
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int iSystem, int iCol, LUnits UNUSED(xRight),
                                      LUnits UNUSED(xLeft), LUnits yTop)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pSlur = dynamic_cast<ImoSlur*>(pRO);

    m_pStartNote = dynamic_cast<ImoNote*>(pSO);
    m_pStartNoteShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);

    m_shapesInfo[0].iCol = iCol;
    m_shapesInfo[0].iInstr = iInstr;
    m_shapesInfo[0].iSystem = iSystem;

    m_uStaffTopStart = yTop - m_pStartNoteShape->get_top();     //relative to note top
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int iInstr,
                                    int UNUSED(iStaff), int iSystem, int iCol,
                                    LUnits UNUSED(xRight), LUnits UNUSED(xLeft),
                                    LUnits yTop)
{
    m_pEndNote = dynamic_cast<ImoNote*>(pSO);
    m_pEndNoteShape = dynamic_cast<GmoShapeNote*>(pStaffObjShape);

    m_shapesInfo[1].iCol = iCol;
    m_shapesInfo[1].iInstr = iInstr;
    m_shapesInfo[1].iSystem = iSystem;

    m_uStaffTopEnd = yTop - m_pEndNoteShape->get_top();     //relative to note top;
}

//---------------------------------------------------------------------------------------
int SlurEngraver::create_shapes(Color color)
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
void SlurEngraver::create_one_shape()
{
    m_numShapes = 1;

    compute_ref_point(m_pStartNoteShape, &m_points1[ImoBezierInfo::k_start]);
    compute_ref_point(m_pEndNoteShape, &m_points1[ImoBezierInfo::k_end]);
    compute_start_point();
    compute_end_point(&m_points1[ImoBezierInfo::k_end]);
    compute_default_control_points(&m_points1[0]);
    //add_user_displacements(0, &m_points1[0]);
    m_shapesInfo[0].pShape =
        LOMSE_NEW GmoShapeSlur(m_pSlur, 0, &m_points1[0], m_thickness, m_color);

    m_shapesInfo[1].pShape = nullptr;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::create_two_shapes()
{
    m_numShapes = 2;

    //create first shape
    compute_start_point();
    compute_end_of_staff_point();
    compute_default_control_points(&m_points1[0]);
    //add_user_displacements(0, &m_points1[0]);
    m_shapesInfo[0].pShape = LOMSE_NEW GmoShapeSlur(m_pSlur, 0, &m_points1[0], m_thickness);

    //create second shape
    compute_end_point(&m_points2[ImoBezierInfo::k_end]);
    compute_start_of_staff_point();
    compute_default_control_points(&m_points2[0]);
    //add_user_displacements(1, &m_points2[0]);
    m_shapesInfo[1].pShape = LOMSE_NEW GmoShapeSlur(m_pSlur, 0, &m_points2[0], m_thickness);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_default_control_points(UPoint* points)
{
    LUnits D = (points+ImoBezierInfo::k_end)->x - (points+ImoBezierInfo::k_start)->x;
    LUnits d = D / 5.8f;
    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
    LUnits hc = m_thickness * 4.5f; //3.88f;
    (points+ImoBezierInfo::k_ctrol1)->x = (points+ImoBezierInfo::k_start)->x + d;
    (points+ImoBezierInfo::k_ctrol1)->y = (points+ImoBezierInfo::k_start)->y + (m_fSlurBelow ? hc : -hc);

    (points+ImoBezierInfo::k_ctrol2)->x = (points+ImoBezierInfo::k_end)->x - d;
    (points+ImoBezierInfo::k_ctrol2)->y = (points+ImoBezierInfo::k_end)->y + (m_fSlurBelow ? hc : -hc);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_start_point()
{
    //if start note has stem and stem is on the same position than slur, place slur
    //over stem. Otherwise, place slur over note-head
    bool fOverStem = false;
    if (m_pStartNote->has_stem())
    {
        fOverStem = (m_fSlurBelow && m_pStartNote->is_stem_down())
                    || (!m_fSlurBelow && m_pStartNote->is_stem_up());
    }

	//x pos: center on notehead
	m_points1[ImoBezierInfo::k_start].x = (m_pStartNoteShape->get_notehead_right() +
                          m_pStartNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths appart from notehead of from stem
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
    if (fOverStem)
        m_points1[ImoBezierInfo::k_start].y = (m_fSlurBelow ?
                              m_pStartNoteShape->get_bottom() + space
                              : m_pStartNoteShape->get_top() - space );
    else
        m_points1[ImoBezierInfo::k_start].y = (m_fSlurBelow ?
                              m_pStartNoteShape->get_notehead_bottom() + space
                              : m_pStartNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_point(UPoint* point)
{
    //over stem. Otherwise, place slur over note-head
    bool fOverStem = false;
    if (m_pEndNote->has_stem())
    {
        fOverStem = (m_fSlurBelow && m_pEndNote->is_stem_down())
                    || (!m_fSlurBelow && m_pEndNote->is_stem_up());
    }
	//x pos: center on notehead
	point->x = (m_pEndNoteShape->get_notehead_right() +
                m_pEndNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths appart from notehead
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
    if (fOverStem)
        point->y = (m_fSlurBelow ?
                     m_pEndNoteShape->get_bottom() + space
                   : m_pEndNoteShape->get_top() - space );
    else
        point->y = (m_fSlurBelow ?
                     m_pEndNoteShape->get_notehead_bottom() + space
                   : m_pEndNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_start_of_staff_point()
{
    m_points2[ImoBezierInfo::k_start].x = m_pInstrEngrv->get_staves_left()
                                          + m_uPrologWidth - tenths_to_logical(10.0f);

    if (m_fSlurBelow)
        m_points2[ImoBezierInfo::k_start].y =
            m_uStaffTopEnd + m_pEndNoteShape->get_top() + tenths_to_logical(60.0f);
    else
        m_points2[ImoBezierInfo::k_start].y =
            m_uStaffTopEnd + m_pEndNoteShape->get_top() - tenths_to_logical(20.0f);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_of_staff_point()
{
    m_points1[ImoBezierInfo::k_end].x = m_pInstrEngrv->get_staves_right();
    LUnits yTop = m_uStaffTopStart + m_pStartNoteShape->get_top();

    if (m_fSlurBelow)
        m_points1[ImoBezierInfo::k_end].y = yTop + tenths_to_logical(60.0f);
    else
        m_points1[ImoBezierInfo::k_end].y = yTop - tenths_to_logical(20.0f);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::decide_placement()
{
    //[Read, p.266] placement: at notehead side when all notes in same direction.
    //When mixed directions, preferred above

    bool fMixed = (m_pStartNoteShape->is_up() && !m_pEndNoteShape->is_up())
                  || (!m_pStartNoteShape->is_up() && m_pEndNoteShape->is_up());

    if (fMixed)
        m_fSlurBelow = false;
    else
        m_fSlurBelow = m_pStartNoteShape->is_up() ? true : false;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::decide_if_one_or_two_arches()
{
    m_fTwoArches = (m_shapesInfo[0].iSystem != m_shapesInfo[1].iSystem);
}


//---------------------------------------------------------------------------------------
void SlurEngraver::compute_ref_point(GmoShapeNote* pNoteShape, UPoint* point)
{
    bool fOnNoteHead = (m_fSlurBelow && pNoteShape->is_up()) ||
                       (!m_fSlurBelow && !pNoteShape->is_up());

    if (fOnNoteHead)
    {
        //x pos: center on notehead
        point->x = (pNoteShape->get_notehead_right() +
                    pNoteShape->get_notehead_left()) / 2.0f;
    }
    else
    {
        //x. pos: center of stem
        point->x = pNoteShape->get_stem_left() + pNoteShape->get_stem_width() / 2.0f;
    }


    //space = 5 tenths appart from y reference point
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    if (fOnNoteHead)
    {
        // y pos: notehead border
        point->y = (m_fSlurBelow ?
                        pNoteShape->get_notehead_bottom() + space
                        : pNoteShape->get_notehead_top() - space );
    }
    else
    {
        //y pos: top/bottom of note shape
        point->y = (pNoteShape->is_up() ?
                        pNoteShape->get_top() - space
                        : pNoteShape->get_bottom() + space );
    }
}

////---------------------------------------------------------------------------------------
//void SlurEngraver::add_user_displacements(int iSlur, UPoint* points)
//{
//    //ImoBezierInfo* pBezier = (iSlur == 0 ? m_pSlur->get_start_bezier()
//    //                                    : m_pSlur->get_stop_bezier() );
//    //if (pBezier)
//    //{
//    //    for (int i=0; i < 4; i++)
//    //    {
//    //        (points+i)->x += tenths_to_logical(pBezier->get_point(i).x);
//    //        (points+i)->y += tenths_to_logical(pBezier->get_point(i).y);
//    //    }
//    //}
//}

//---------------------------------------------------------------------------------------
int SlurEngraver::get_num_shapes()
{
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
ShapeBoxInfo* SlurEngraver::get_shape_box_info(int i)
{
    return &m_shapesInfo[i];
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_prolog_width(LUnits width)
{
    m_uPrologWidth += width;
}


}  //namespace lomse
