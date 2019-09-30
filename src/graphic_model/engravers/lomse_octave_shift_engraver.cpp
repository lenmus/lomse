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

#include "lomse_octave_shift_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_shapes.h"
#include "lomse_shape_octave_shift.h"
#include "lomse_score_meter.h"
#include "lomse_instrument_engraver.h"
#include "lomse_glyphs.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// OctaveShiftEngraver implementation
//---------------------------------------------------------------------------------------
OctaveShiftEngraver::OctaveShiftEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
    , m_uStaffTopStart(0.0f)
    , m_uStaffTopEnd(0.0f)
    , m_numShapes(0)
    , m_pOctaveShift(nullptr)
    , m_uPrologWidth(0.0f)
    , m_pStartDirection(nullptr)
    , m_pEndDirection(nullptr)
    , m_pStartDirectionShape(nullptr)
    , m_pEndDirectionShape(nullptr)
    , m_fUseTwoShapes(false)
    , m_fPlaceAtTop(true)
{
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                       GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                       int iSystem, int iCol, LUnits UNUSED(xRight),
                                       LUnits UNUSED(xLeft), LUnits yTop)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pOctaveShift = static_cast<ImoOctaveShift*>(pRO);

    m_pStartDirection = static_cast<ImoDirection*>(pSO);
    m_pStartDirectionShape = static_cast<GmoShapeInvisible*>(pStaffObjShape);

    m_shapesInfo[0].iCol = iCol;
    m_shapesInfo[0].iInstr = iInstr;
    m_shapesInfo[0].iSystem = iSystem;

    m_points[0][0].x = m_pStartDirectionShape->get_left();
    m_points[0][0].y = yTop;

    m_uStaffTopStart = yTop;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
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

    m_points[0][1].x = m_pEndDirectionShape->get_left();
    m_points[0][1].y = yTop;

    m_uStaffTopEnd = yTop;
}

//---------------------------------------------------------------------------------------
int OctaveShiftEngraver::create_shapes()
{
    return create_shapes( m_pOctaveShift->get_color() );
}
//---------------------------------------------------------------------------------------
int OctaveShiftEngraver::create_shapes(Color color)
{
    m_color = color;
    decide_placement();
    decide_if_one_or_two_shapes();
//    if (two_shapes_needed())
//        create_two_shapes();
//    else
        create_one_shape();
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::create_one_shape()
{
    m_numShapes = 1;

    compute_first_shape_position();
//    //add_user_displacements(0, &m_points[0][0]);

    create_main_container_shape(0);
    add_shape_numeral(0);
    add_line_info(0);

    m_shapesInfo[1].pShape = nullptr;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::create_two_shapes()
{
    m_numShapes = 2;
//    LUnits thickness = tenths_to_logical(LOMSE_WEDGE_LINE_THICKNESS);
//
//    //determine if 'niente' circle should be drawn
//    int niente1 = GmoShapeOctaveShift::k_no_niente;
//    int niente2 = GmoShapeOctaveShift::k_no_niente;
//    LUnits radius = 0.0f;
//    if (m_pOctaveShift->is_niente())
//    {
//        radius = tenths_to_logical(LOMSE_WEDGE_NIENTE_RADIUS);
//        if (m_pOctaveShift->is_crescendo())
//            niente1 = GmoShapeOctaveShift::k_niente_at_start;
//        else
//            niente2 = GmoShapeOctaveShift::k_niente_at_end;
//    }
//
//    //create first shape
//    compute_first_shape_position();
//    //add_user_displacements(0, &m_points[0][0]);
//    m_shapesInfo[0].pShape = LOMSE_NEW GmoShapeOctaveShift(m_pOctaveShift, 0, &m_points[0][0],
//                                                     thickness, m_pOctaveShift->get_color(),
//                                                     niente1, radius);
//    //create second shape
//    compute_second_shape_position();
//    //add_user_displacements(1, &m_points2[0]);
//    m_shapesInfo[1].pShape = LOMSE_NEW GmoShapeOctaveShift(m_pOctaveShift, 0, &m_points2[0],
//                                                     thickness, m_pOctaveShift->get_color(),
//                                                     niente2, radius);
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::create_main_container_shape(int iShape)
{
    ShapeId idx = iShape;
    m_pMainShape[iShape] = LOMSE_NEW GmoShapeOctaveShift(m_pOctaveShift, idx,
                                                    m_pOctaveShift->get_color());
    m_shapesInfo[iShape].pShape = m_pMainShape[iShape];
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::add_shape_numeral(int iShape)
{
    //determine glyph to use
    int steps = m_pOctaveShift->get_shift_steps();
    int iGlyph = find_glyph(steps);

    //adjust position
    Tenths yOffset = m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph) + 10.0f;
    LUnits y = m_points[0][0].y
               + m_pMeter->tenths_to_logical(yOffset, m_iInstr, m_iStaff);

    //determine font size to use
    double fontSize = 16.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;

   //create the shape
    GmoShape* pShape =
        LOMSE_NEW GmoShapeOctaveGlyph(m_pOctaveShift, 0, iGlyph, UPoint(m_points[0][0].x, y),
                                      m_color, m_libraryScope, fontSize);
    m_pShapeNumeral[iShape] = pShape;

    m_pMainShape[iShape]->add(pShape);
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::add_line_info(int iShape)
{
    //start point
    LUnits xStart = m_points[iShape][0].x + m_pShapeNumeral[iShape]->get_width()
                    + tenths_to_logical(LOMSE_OCTAVE_SHIFT_SPACE_TO_LINE);

    LUnits yStart = m_points[iShape][0].y - tenths_to_logical(LOMSE_OCTAVE_SHIFT_LINE_SHIFT);
    if (!octave_shift_at_top())
        yStart += m_pShapeNumeral[iShape]->get_height();

    //locate last applicable note in this instr/staff

    //end point
    LUnits xEnd = m_points[iShape][1].x;
    LUnits yEnd = yStart + (octave_shift_at_top() ? 200.0f : -200.0f);

    LUnits thickness = tenths_to_logical(LOMSE_OCTAVE_SHIFT_LINE_THICKNESS);

    m_pMainShape[iShape]->set_layout_data(xStart, xEnd, yStart, yEnd, thickness);
}

//---------------------------------------------------------------------------------------
int OctaveShiftEngraver::find_glyph(int shift)
{
    // returns the index (over global glyphs table) to the character to use to print
    // the numeral at the start

    if (shift == 14)
        return k_glyph_quindicesimaBassaMb;     //15mb
    else if (shift == 7)
        return k_glyph_ottavaBassaBa;           //8ba
    else if (shift == -7)
        return k_glyph_ottavaAlta;              //8va
    else if (shift == -14)
        return k_glyph_quindicesimaAlta;        //15ma
    else
    {
        LOMSE_LOG_ERROR("Octave shift not defined (%d). '22' numeral used.", shift);
        return k_glyph_ventiduesima;
    }
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::compute_first_shape_position()
{
    //compute xLeft and xRight positions
    if (two_shapes_needed())
    {
        m_points[0][0].x = m_pStartDirectionShape->get_left();    //xLeft at Direction tag
        m_points[0][1].x = m_pInstrEngrv->get_staves_right();     //xRight at end of staff
    }
    else
    {
        m_points[0][0].x = m_pStartDirectionShape->get_left();
        m_points[0][1].x = m_pEndDirectionShape->get_left();
    }

    //determine top line of shape box
    m_points[0][0].y = m_uStaffTopStart;
    if (octave_shift_at_top())
        m_points[0][0].y -= tenths_to_logical(40.0f);   //4 lines above top staff line
    else
        m_points[0][0].y += tenths_to_logical(80.0f);   //4 lines bellow first staff line

    m_points[0][1].y = m_points[0][0].y;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::compute_second_shape_position()
{
//    m_points2[0].x = m_pInstrEngrv->get_staves_left()+ m_uPrologWidth
//                     - tenths_to_logical(10.0f);
//    m_points2[2].x = m_points2[0].x;
//    m_points2[1].x = m_pEndDirectionShape->get_left();
//    m_points2[3].x = m_points2[1].x;
//
//    //determine center line of shape box
//    LUnits yRef = m_uStaffTopEnd;
//    if (m_fOctaveShiftAbove)
//        yRef -= tenths_to_logical(50.0f);   //5 lines above top staff line
//    else
//        yRef += tenths_to_logical(90.0f);   //5 lines bellow first staff line
//
//    //determine spread to apply
//    LUnits endSpread = tenths_to_logical(m_pOctaveShift->get_end_spread()) / 2.0f;
//    LUnits startSpread = tenths_to_logical(m_pOctaveShift->get_start_spread()) / 2.0f;
//    if (m_pOctaveShift->is_crescendo())
//        startSpread = endSpread / 2.0f;
//    else
//        startSpread /= 2.0f;
//
//    //compute points
//    m_points2[0].y = yRef - startSpread;
//    m_points2[2].y = yRef + startSpread;
//
//    m_points2[1].y = yRef - endSpread;
//    m_points2[3].y = yRef + endSpread;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::decide_placement()
{
    m_fPlaceAtTop = m_pOctaveShift->get_shift_steps() < 0;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::decide_if_one_or_two_shapes()
{
    m_fUseTwoShapes = (m_shapesInfo[0].iSystem != m_shapesInfo[1].iSystem);
}

////---------------------------------------------------------------------------------------
//void OctaveShiftEngraver::add_user_displacements(int iOctaveShift, UPoint* points)
//{
//    //ImoBezierInfo* pBezier = (iOctaveShift == 0 ? m_pOctaveShift->get_start_bezier()
//    //                                    : m_pOctaveShift->get_stop_bezier() );
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
int OctaveShiftEngraver::get_num_shapes()
{
    return m_numShapes;
}

//---------------------------------------------------------------------------------------
ShapeBoxInfo* OctaveShiftEngraver::get_shape_box_info(int i)
{
    return &m_shapesInfo[i];
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::set_prolog_width(LUnits width)
{
    m_uPrologWidth += width;
}


}  //namespace lomse
