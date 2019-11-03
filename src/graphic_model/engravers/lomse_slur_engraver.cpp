//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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
#include "lomse_vertical_profile.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// SlurEngraver implementation
//---------------------------------------------------------------------------------------
SlurEngraver::SlurEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
    , m_uStaffTop(0.0f)
    , m_numShapes(0)
    , m_pSlur(nullptr)
    , m_fSlurBelow(false)
    , m_uPrologWidth(0.0f)
    , m_pStartNote(nullptr)
    , m_pEndNote(nullptr)
    , m_pStartNoteShape(nullptr)
    , m_pEndNoteShape(nullptr)
    , m_fTwoArches(false)
    , m_thickness(0.0f)
{
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int UNUSED(iSystem), int UNUSED(iCol), LUnits UNUSED(xStaffLeft),
                                      LUnits UNUSED(xStaffRight), LUnits yTop,
                                      int idxStaff, VerticalProfile* pVProfile)
{
    //data stored in base class
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;

    //data specific for this class
    m_pSlur = static_cast<ImoSlur*>(pRO);

    m_pStartNote = static_cast<ImoNote*>(pSO);
    m_pStartNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffTop = yTop - m_pStartNoteShape->get_top();     //relative to note top
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                    int UNUSED(iStaff), int UNUSED(iSystem), int UNUSED(iCol),
                                    LUnits UNUSED(xStaffLeft), LUnits UNUSED(xStaffRight),
                                    LUnits yTop, int idxStaff, VerticalProfile* pVProfile)
{
    m_pEndNote = static_cast<ImoNote*>(pSO);
    m_pEndNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffTop = yTop - m_pEndNoteShape->get_top();     //relative to note top;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::decide_placement()
{
    //[Read, p.266] placement: at notehead side when all notes in same direction.
    //When mixed directions, preferred above

    bool fMixed = is_end_point_set() &&
                  ( (m_pStartNoteShape->is_up() && !m_pEndNoteShape->is_up())
                     || (!m_pStartNoteShape->is_up() && m_pEndNoteShape->is_up()) );

    if (fMixed)
        m_fSlurBelow = false;
    else
        m_fSlurBelow = m_pStartNoteShape->is_up() ? true : false;
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_first_or_intermediate_shape(Color color)
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
GmoShape* SlurEngraver::create_last_shape(Color color)
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
GmoShape* SlurEngraver::create_first_shape()
{
    //first shape when there are more than one

    compute_start_point();
    compute_end_of_staff_point();
    compute_default_control_points(&m_points[0]);
    //add_user_displacements(0, &m_points[0]);
    m_numShapes = 1;
    return LOMSE_NEW GmoShapeSlur(m_pSlur, 0, &m_points[0], m_thickness, m_color);
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_single_shape()
{
    //the only shape when start and end points are in the same system

    compute_ref_point(m_pStartNoteShape, &m_points[ImoBezierInfo::k_start]);
    compute_ref_point(m_pEndNoteShape, &m_points[ImoBezierInfo::k_end]);
    compute_start_point();
    compute_end_point(&m_points[ImoBezierInfo::k_end]);
    compute_default_control_points(&m_points[0]);
    //add_user_displacements(0, &m_points[0]);

    GmoShape* pShape = LOMSE_NEW GmoShapeSlur(m_pSlur, m_numShapes++, &m_points[0],
                                              m_thickness, m_color);

    if (m_pStartNote->get_staff() != m_pEndNote->get_staff())
        pShape->set_add_to_vprofile(false);

    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_intermediate_shape()
{
    //intermediate shape spanning the whole system

    ++m_numShapes;
    //TODO
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_final_shape()
{
    //last shape when there are more than one

    compute_end_point(&m_points[ImoBezierInfo::k_end]);
    compute_start_of_staff_point();
    compute_default_control_points(&m_points[0]);
    //add_user_displacements(1, &m_points[0]);
    return LOMSE_NEW GmoShapeSlur(m_pSlur, m_numShapes++, &m_points[0], m_thickness);
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
	m_points[ImoBezierInfo::k_start].x = (m_pStartNoteShape->get_notehead_right() +
                          m_pStartNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths appart from notehead of from stem
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
    if (fOverStem)
        m_points[ImoBezierInfo::k_start].y = (m_fSlurBelow ?
                              m_pStartNoteShape->get_bottom() + space
                              : m_pStartNoteShape->get_top() - space );
    else
        m_points[ImoBezierInfo::k_start].y = (m_fSlurBelow ?
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
    m_points[ImoBezierInfo::k_start].x = m_pInstrEngrv->get_staves_left()
                                          + m_uPrologWidth - tenths_to_logical(10.0f);

    if (m_fSlurBelow)
        m_points[ImoBezierInfo::k_start].y =
            m_uStaffTop + m_pEndNoteShape->get_top() + tenths_to_logical(60.0f);
    else
        m_points[ImoBezierInfo::k_start].y =
            m_uStaffTop + m_pEndNoteShape->get_top() - tenths_to_logical(20.0f);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_of_staff_point()
{
    m_points[ImoBezierInfo::k_end].x = m_pInstrEngrv->get_staves_right();
    LUnits yTop = m_uStaffTop + m_pStartNoteShape->get_top();

    if (m_fSlurBelow)
        m_points[ImoBezierInfo::k_end].y = yTop + tenths_to_logical(60.0f);
    else
        m_points[ImoBezierInfo::k_end].y = yTop - tenths_to_logical(20.0f);
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
void SlurEngraver::set_prolog_width(LUnits width)
{
    m_uPrologWidth += width;
}


}  //namespace lomse
