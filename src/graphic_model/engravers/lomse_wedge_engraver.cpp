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


namespace lomse
{

//---------------------------------------------------------------------------------------
// WedgeEngraver implementation
//---------------------------------------------------------------------------------------
WedgeEngraver::WedgeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
    , m_uStaffTopStart(0.0f)
    , m_uStaffTopEnd(0.0f)
    , m_numShapes(0)
    , m_pWedge(nullptr)
    , m_fWedgeAbove(false)
    , m_uPrologWidth(0.0f)
    , m_pStartDirection(nullptr)
    , m_pEndDirection(nullptr)
    , m_pStartDirectionShape(nullptr)
    , m_pEndDirectionShape(nullptr)
    , m_fTwoWedges(false)
{
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                       GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                       int iSystem, int iCol, LUnits UNUSED(xRight),
                                       LUnits UNUSED(xLeft), LUnits yTop)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pWedge = static_cast<ImoWedge*>(pRO);

    m_pStartDirection = static_cast<ImoDirection*>(pSO);
    m_pStartDirectionShape = static_cast<GmoShapeInvisible*>(pStaffObjShape);

    m_shapesInfo[0].iCol = iCol;
    m_shapesInfo[0].iInstr = iInstr;
    m_shapesInfo[0].iSystem = iSystem;

    m_points1[0].x = m_pStartDirectionShape->get_left();
    m_points1[0].y = yTop;

    m_uStaffTopStart = yTop;
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int iInstr,
                                     int UNUSED(iStaff), int iSystem, int iCol,
                                     LUnits UNUSED(xRight), LUnits UNUSED(xLeft),
                                     LUnits yTop)
{
    m_pEndDirection = static_cast<ImoDirection*>(pSO);
    m_pEndDirectionShape = static_cast<GmoShapeInvisible*>(pStaffObjShape);

    m_shapesInfo[1].iCol = iCol;
    m_shapesInfo[1].iInstr = iInstr;
    m_shapesInfo[1].iSystem = iSystem;

    m_points1[1].x = m_pEndDirectionShape->get_left();
    m_points1[1].y = yTop;

    m_uStaffTopEnd = yTop;
}

//---------------------------------------------------------------------------------------
int WedgeEngraver::create_shapes()
{
    return create_shapes( m_pWedge->get_color() );
}
//---------------------------------------------------------------------------------------
int WedgeEngraver::create_shapes(Color color)
{
    m_color = color;
    decide_placement();
    decide_if_one_or_two_wedges();
    if (two_wedges_needed())
        create_two_shapes();
    else
        create_one_shape();
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::create_one_shape()
{
    m_numShapes = 1;
    LUnits thickness = tenths_to_logical(LOMSE_WEDGE_LINE_THICKNESS);

    compute_first_shape_position();
    //add_user_displacements(0, &m_points1[0]);

    //determine if 'niente' circle should be drawn
    int niente = GmoShapeWedge::k_no_niente;
    LUnits radius = 0.0f;
    if (m_pWedge->is_niente())
    {
        niente = (m_pWedge->is_crescendo() ? GmoShapeWedge::k_niente_at_start
                                           : GmoShapeWedge::k_niente_at_end);
        radius = tenths_to_logical(LOMSE_WEDGE_NIENTE_RADIUS);
    }

    m_shapesInfo[0].pShape = LOMSE_NEW GmoShapeWedge(m_pWedge, 0, &m_points1[0],
                                                     thickness, m_pWedge->get_color(),
                                                     niente, radius);
    m_shapesInfo[1].pShape = nullptr;
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::create_two_shapes()
{
    m_numShapes = 2;
    LUnits thickness = tenths_to_logical(LOMSE_WEDGE_LINE_THICKNESS);

    //determine if 'niente' circle should be drawn
    int niente1 = GmoShapeWedge::k_no_niente;
    int niente2 = GmoShapeWedge::k_no_niente;
    LUnits radius = 0.0f;
    if (m_pWedge->is_niente())
    {
        radius = tenths_to_logical(LOMSE_WEDGE_NIENTE_RADIUS);
        if (m_pWedge->is_crescendo())
            niente1 = GmoShapeWedge::k_niente_at_start;
        else
            niente2 = GmoShapeWedge::k_niente_at_end;
    }

    //create first shape
    compute_first_shape_position();
    //add_user_displacements(0, &m_points1[0]);
    m_shapesInfo[0].pShape = LOMSE_NEW GmoShapeWedge(m_pWedge, 0, &m_points1[0],
                                                     thickness, m_pWedge->get_color(),
                                                     niente1, radius);
    //create second shape
    compute_second_shape_position();
    //add_user_displacements(1, &m_points2[0]);
    m_shapesInfo[1].pShape = LOMSE_NEW GmoShapeWedge(m_pWedge, 0, &m_points2[0],
                                                     thickness, m_pWedge->get_color(),
                                                     niente2, radius);
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::compute_second_shape_position()
{
    m_points2[0].x = m_pInstrEngrv->get_staves_left()+ m_uPrologWidth
                     - tenths_to_logical(10.0f);
    m_points2[2].x = m_points2[0].x;
    m_points2[1].x = m_pEndDirectionShape->get_left();
    m_points2[3].x = m_points2[1].x;

    //determine center line of shape box
    LUnits yRef = m_uStaffTopEnd;
    if (m_fWedgeAbove)
        yRef -= tenths_to_logical(50.0f);   //5 lines above top staff line
    else
        yRef += tenths_to_logical(90.0f);   //5 lines bellow first staff line

    //determine spread to apply
    LUnits endSpread = tenths_to_logical(m_pWedge->get_end_spread()) / 2.0f;
    LUnits startSpread = tenths_to_logical(m_pWedge->get_start_spread()) / 2.0f;
    if (m_pWedge->is_crescendo())
        startSpread = endSpread / 2.0f;
    else
        startSpread /= 2.0f;

    //compute points
    m_points2[0].y = yRef - startSpread;
    m_points2[2].y = yRef + startSpread;

    m_points2[1].y = yRef - endSpread;
    m_points2[3].y = yRef + endSpread;
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::compute_first_shape_position()
{
    //compute xLeft and xRight positions
    if (two_wedges_needed())
    {
        m_points1[0].x = m_pStartDirectionShape->get_left();    //xLeft at Direction tag
        m_points1[1].x = m_pInstrEngrv->get_staves_right();     //xRight at end of staff
    }
    else
    {
        m_points1[0].x = m_pStartDirectionShape->get_left();
        m_points1[1].x = m_pEndDirectionShape->get_left();
    }
    m_points1[2].x = m_points1[0].x;
    m_points1[3].x = m_points1[1].x;

    //determine spread to apply
    LUnits startSpread = tenths_to_logical(m_pWedge->get_start_spread()) / 2.0f;
    LUnits endSpread = tenths_to_logical(m_pWedge->get_end_spread()) / 2.0f;
    if (two_wedges_needed())
    {
        if (m_pWedge->is_crescendo())
            endSpread /= 2.0f;
        else
            endSpread = startSpread / 2.0f;
    }

    //determine center line of shape box. yRef is referred to first staff
    LUnits yRef = m_uStaffTopStart;
    if (m_fWedgeAbove)
        yRef -= tenths_to_logical(50.0f);   //5 lines above top staff line
    else
        yRef += tenths_to_logical(90.0f);   //5 lines bellow first staff line

    //compute points
    m_points1[0].y = yRef - startSpread;
    m_points1[2].y = yRef + startSpread;

    m_points1[1].y = yRef - endSpread;
    m_points1[3].y = yRef + endSpread;
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::decide_placement()
{
    //default is bellow
    m_fWedgeAbove = (m_pStartDirection->get_placement() == k_placement_above);
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::decide_if_one_or_two_wedges()
{
    m_fTwoWedges = (m_shapesInfo[0].iSystem != m_shapesInfo[1].iSystem);
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
int WedgeEngraver::get_num_shapes()
{
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
ShapeBoxInfo* WedgeEngraver::get_shape_box_info(int i)
{
    return &m_shapesInfo[i];
}

//---------------------------------------------------------------------------------------
void WedgeEngraver::set_prolog_width(LUnits width)
{
    m_uPrologWidth += width;
}


}  //namespace lomse
