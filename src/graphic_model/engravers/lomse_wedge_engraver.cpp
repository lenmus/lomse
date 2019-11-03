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

#include "lomse_wedge_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_wedge.h"
#include "lomse_score_meter.h"
#include "lomse_instrument_engraver.h"
#include "lomse_vertical_profile.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// WedgeEngraver implementation
//---------------------------------------------------------------------------------------
WedgeEngraver::WedgeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                             InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
    , m_uStaffTop(0.0f)
    , m_numShapes(0)
    , m_pWedge(nullptr)
    , m_fWedgeAbove(false)
    , m_uPrologWidth(0.0f)
    , m_pStartDirection(nullptr)
    , m_pEndDirection(nullptr)
    , m_pStartDirectionShape(nullptr)
    , m_pEndDirectionShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                       GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                       int UNUSED(iSystem), int UNUSED(iCol),
                                       LUnits UNUSED(xStaffLeft),
                                       LUnits UNUSED(xStaffRight), LUnits yTop,
                                       int idxStaff, VerticalProfile* pVProfile)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pWedge = static_cast<ImoWedge*>(pRO);

    m_pStartDirection = static_cast<ImoDirection*>(pSO);
    m_pStartDirectionShape = static_cast<GmoShapeInvisible*>(pStaffObjShape);

    m_uStaffTop = yTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;

}

//---------------------------------------------------------------------------------------
void WedgeEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                     int UNUSED(iStaff), int UNUSED(iSystem), int UNUSED(iCol),
                                     LUnits UNUSED(xStaffLeft), LUnits UNUSED(xStaffRight),
                                     LUnits yTop, int idxStaff, VerticalProfile* pVProfile)
{
    m_pEndDirection = static_cast<ImoDirection*>(pSO);
    m_pEndDirectionShape = static_cast<GmoShapeInvisible*>(pStaffObjShape);

    m_uStaffTop = yTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::decide_placement()
{
    //default is bellow
    m_fWedgeAbove = (m_pStartDirection->get_placement() == k_placement_above);
}

//---------------------------------------------------------------------------------------
GmoShape* WedgeEngraver::create_first_or_intermediate_shape(Color color)
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
GmoShape* WedgeEngraver::create_last_shape(Color color)
{
    m_color = color;
    if (m_numShapes == 0)
    {
        decide_placement();
        return create_first_shape();
    }
    return create_final_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* WedgeEngraver::create_intermediate_shape()
{
    //intermediate shape spanning the whole system

    ++m_numShapes;

    LUnits thickness = tenths_to_logical(LOMSE_WEDGE_LINE_THICKNESS);
    int niente = GmoShapeWedge::k_no_niente;
    LUnits radius = 0.0f;

    compute_intermediate_shape_position();
    //add_user_displacements(1, &m_points[0]);
    return LOMSE_NEW GmoShapeWedge(m_pWedge, 0, &m_points[0], thickness,
                                   m_pWedge->get_color(), niente, radius);
}

//---------------------------------------------------------------------------------------
GmoShape* WedgeEngraver::create_first_shape()
{
    //first shape when there are more than one, or single shape

    LUnits thickness = tenths_to_logical(LOMSE_WEDGE_LINE_THICKNESS);

    compute_first_shape_position();
    //add_user_displacements(0, &m_points[0]);

    //determine if 'niente' circle should be drawn
    int niente = GmoShapeWedge::k_no_niente;
    LUnits radius = 0.0f;
    if (m_pWedge->is_niente())
    {
        niente = (m_pWedge->is_crescendo() ? GmoShapeWedge::k_niente_at_start
                                           : GmoShapeWedge::k_niente_at_end);
        radius = tenths_to_logical(LOMSE_WEDGE_NIENTE_RADIUS);
    }

    return LOMSE_NEW GmoShapeWedge(m_pWedge, m_numShapes++, &m_points[0], thickness,
                                   m_pWedge->get_color(), niente, radius);
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::compute_first_shape_position()
{
    //compute xLeft and xRight positions
    if (m_pEndDirectionShape == nullptr)
    {
        m_points[0].x = m_pStartDirectionShape->get_left();    //xLeft at Direction tag
        m_points[1].x = m_pInstrEngrv->get_staves_right();     //xRight at end of staff
    }
    else
    {
        m_points[0].x = m_pStartDirectionShape->get_left();
        m_points[1].x = m_pEndDirectionShape->get_left();
    }
    m_points[2].x = m_points[0].x;
    m_points[3].x = m_points[1].x;

    //determine wedge spread
    LUnits startSpread = tenths_to_logical(m_pWedge->get_start_spread()) / 2.0f;
    LUnits endSpread = tenths_to_logical(m_pWedge->get_end_spread()) / 2.0f;
    if (m_pEndDirectionShape == nullptr)
    {
        if (m_pWedge->is_crescendo())
            endSpread /= 2.0f;
        else
            endSpread = startSpread / 2.0f;
    }

    //determine center line of shape box
    LUnits yRef = determine_center_line_of_shape(startSpread, endSpread);

    //compute points
    m_points[0].y = yRef - startSpread;
    m_points[2].y = yRef + startSpread;

    m_points[1].y = yRef - endSpread;
    m_points[3].y = yRef + endSpread;
}

//---------------------------------------------------------------------------------------
GmoShape* WedgeEngraver::create_final_shape()
{
    //last shape when there are more than one

    LUnits thickness = tenths_to_logical(LOMSE_WEDGE_LINE_THICKNESS);

    //determine if 'niente' circle should be drawn
    int niente = GmoShapeWedge::k_no_niente;
    LUnits radius = 0.0f;
    if (m_pWedge->is_niente())
    {
        radius = tenths_to_logical(LOMSE_WEDGE_NIENTE_RADIUS);
        if (!m_pWedge->is_crescendo())
            niente = GmoShapeWedge::k_niente_at_end;
    }

    compute_last_shape_position();
    //add_user_displacements(1, &m_points[0]);
    return LOMSE_NEW GmoShapeWedge(m_pWedge, m_numShapes++, &m_points[0], thickness,
                                   m_pWedge->get_color(), niente, radius);
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::compute_last_shape_position()
{
    m_points[0].x = m_pInstrEngrv->get_staves_left()+ m_uPrologWidth
                     - tenths_to_logical(10.0f);
    m_points[2].x = m_points[0].x;
    m_points[1].x = m_pEndDirectionShape->get_left();
    m_points[3].x = m_points[1].x;

    //determine wedge spread
    LUnits endSpread = tenths_to_logical(m_pWedge->get_end_spread()) / 2.0f;
    LUnits startSpread = tenths_to_logical(m_pWedge->get_start_spread()) / 2.0f;
    if (m_pWedge->is_crescendo())
        startSpread = endSpread / 2.0f;
    else
        startSpread /= 2.0f;

    //determine center line of shape box
    LUnits yRef = determine_center_line_of_shape(startSpread, endSpread);

    //compute points
    m_points[0].y = yRef - startSpread;
    m_points[2].y = yRef + startSpread;

    m_points[1].y = yRef - endSpread;
    m_points[3].y = yRef + endSpread;
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::compute_intermediate_shape_position()
{
    m_points[0].x = m_pInstrEngrv->get_staves_left()+ m_uPrologWidth
                     - tenths_to_logical(10.0f);
    m_points[2].x = m_points[0].x;
    m_points[1].x = m_pInstrEngrv->get_staves_right();     //xRight at end of staff
    m_points[3].x = m_points[1].x;

    //determine wedge spread
    LUnits endSpread = tenths_to_logical(m_pWedge->get_end_spread()) / 2.0f;
    LUnits startSpread = tenths_to_logical(m_pWedge->get_start_spread()) / 2.0f;
    if (m_pWedge->is_crescendo())
        startSpread = endSpread / 2.0f;
    else
        startSpread /= 2.0f;

    //determine center line of shape box
    LUnits yRef = determine_center_line_of_shape(startSpread, endSpread);

    //compute points
    m_points[0].y = yRef - startSpread;
    m_points[2].y = yRef + startSpread;

    m_points[1].y = yRef - endSpread;
    m_points[3].y = yRef + endSpread;
}

//---------------------------------------------------------------------------------------
LUnits WedgeEngraver::determine_center_line_of_shape(LUnits startSpread, LUnits endSpread)
{
    LUnits yRef = m_uStaffTop;
    LUnits oneLine = tenths_to_logical(10.0f);
    LUnits maxSpread = max(startSpread, endSpread);
    if (m_fWedgeAbove)
    {
        yRef -= oneLine;   //1 line above top staff line
        yRef -= maxSpread;
        LUnits yMin = m_pVProfile->get_min_for(m_points[0].x, m_points[1].x, m_idxStaff).first
                      - maxSpread - oneLine;
        yRef = min(yRef, yMin);
    }
    else
    {
        yRef += tenths_to_logical(50.0f);   //1 lines bellow first staff line
        yRef += maxSpread;
        LUnits yMax = m_pVProfile->get_max_for(m_points[0].x, m_points[1].x, m_idxStaff).first
                      + maxSpread + oneLine;
        yRef = max(yRef, yMax);
    }

    return yRef;
}

////---------------------------------------------------------------------------------------
//void WedgeEngraver::add_user_displacements(int iWedge, UPoint* points)
//{
//    //ImoBezierInfo* pBezier = (iWedge == 0 ? m_pWedge->get_start_bezier()
//    //                                    : m_pWedge->get_stop_bezier() );
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
void WedgeEngraver::set_prolog_width(LUnits width)
{
    m_uPrologWidth += width;
}


}  //namespace lomse
