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
#include "lomse_shape_note.h"
#include "lomse_shape_octave_shift.h"
#include "lomse_score_meter.h"
#include "lomse_instrument_engraver.h"
#include "lomse_glyphs.h"
#include "lomse_vertical_profile.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// OctaveShiftEngraver implementation
//---------------------------------------------------------------------------------------
OctaveShiftEngraver::OctaveShiftEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                                         InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
    , m_uStaffTop(0.0f)
    , m_numShapes(0)
    , m_pOctaveShift(nullptr)
    , m_pShapeNumeral(nullptr)
    , m_pMainShape(nullptr)
    , m_uPrologWidth(0.0f)
    , m_pStartNote(nullptr)
    , m_pEndNote(nullptr)
    , m_pStartNoteShape(nullptr)
    , m_pEndNoteShape(nullptr)
    , m_fPlaceAtTop(true)
{
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                       GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                       int UNUSED(iSystem), int UNUSED(iCol), LUnits UNUSED(xStaffLeft),
                                       LUnits UNUSED(xStaffRight), LUnits yTop,
                                       int idxStaff, VerticalProfile* pVProfile)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_pOctaveShift = static_cast<ImoOctaveShift*>(pRO);

    m_pStartNote = static_cast<ImoNote*>(pSO);
    m_pStartNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffTop = yTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                     GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                     int UNUSED(iStaff), int UNUSED(iSystem), int UNUSED(iCol),
                                     LUnits UNUSED(xStaffLeft), LUnits UNUSED(xStaffRight),
                                     LUnits yTop, int idxStaff, VerticalProfile* pVProfile)
{
    m_pEndNote = static_cast<ImoNote*>(pSO);
    m_pEndNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffTop = yTop;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
GmoShape* OctaveShiftEngraver::create_first_or_intermediate_shape(Color color)
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
GmoShape* OctaveShiftEngraver::create_last_shape(Color color)
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
GmoShape* OctaveShiftEngraver::create_intermediate_shape()
{
    //intermediate shape spanning the whole system

    ++m_numShapes;
    //TODO
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* OctaveShiftEngraver::create_first_shape()
{
    //first shape when there are more than one, or single shape

    compute_first_shape_position();
    //add_user_displacements(0, &m_points[0]);
    create_main_container_shape();
    add_shape_numeral();
    add_line_info();

    m_numShapes++;
    return m_pMainShape;
}

//---------------------------------------------------------------------------------------
GmoShape* OctaveShiftEngraver::create_final_shape()
{
    //last shape when there are more than one

    compute_second_shape_position();
    //add_user_displacements(1, &m_points[0]);
    create_main_container_shape();
    add_shape_numeral();
    add_line_info();

    m_numShapes++;
    return m_pMainShape;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::create_main_container_shape()
{
    ShapeId idx = m_numShapes;
    m_pMainShape = LOMSE_NEW GmoShapeOctaveShift(m_pOctaveShift, idx,
                                                 m_pOctaveShift->get_color());
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::add_shape_numeral()
{
    //determine glyph to use
    int steps = m_pOctaveShift->get_shift_steps();
    int iGlyph = find_glyph(steps);

    //adjust position
    Tenths yOffset = m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph) + 10.0f;
    LUnits y = m_points[0].y
               + m_pMeter->tenths_to_logical(yOffset, m_iInstr, m_iStaff);

    //determine font size to use
    double fontSize = 16.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;

   //create the shape
    GmoShape* pShape = LOMSE_NEW GmoShapeOctaveGlyph(m_pOctaveShift, 0, iGlyph,
                                                     UPoint(m_points[0].x, y),
                                                     m_color, m_libraryScope, fontSize);
    m_pShapeNumeral = pShape;

    m_pMainShape->add(pShape);
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::add_line_info()
{
    //start point
    LUnits xStart = m_points[0].x + m_pShapeNumeral->get_width()
                    + tenths_to_logical(LOMSE_OCTAVE_SHIFT_SPACE_TO_LINE);

    LUnits yShift = tenths_to_logical(LOMSE_OCTAVE_SHIFT_LINE_SHIFT);
    LUnits yStart = m_points[0].y - yShift;
    if (!octave_shift_at_top())
        yStart += (m_pShapeNumeral->get_height() - yShift);

    //locate last applicable note in this instr/staff

    //end point
    LUnits xEnd = m_points[1].x;
    LUnits yEnd = yStart + (octave_shift_at_top() ? 200.0f : -200.0f);

    LUnits thickness = tenths_to_logical(LOMSE_OCTAVE_SHIFT_LINE_THICKNESS);
    bool fEndCorner = is_end_point_set();

    m_pMainShape->set_layout_data(xStart, xEnd, yStart, yEnd, thickness, fEndCorner);
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
    if (is_end_point_set())
    {
        m_points[0].x = m_pStartNoteShape->get_left();
        m_points[1].x = m_pEndNoteShape->get_right();
    }
    else
    {
        m_points[0].x = m_pStartNoteShape->get_left();    //xLeft at Note tag
        m_points[1].x = m_pInstrEngrv->get_staves_right();     //xRight at end of staff
    }

    //determine yTop
    m_points[0].y = determine_top_line_of_shape();
    m_points[1].y = m_points[0].y;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::compute_second_shape_position()
{
    //compute xLeft and xRight positions
    m_points[0].x = m_pInstrEngrv->get_staves_left()+ m_uPrologWidth
                     - tenths_to_logical(10.0f);
    m_points[1].x = m_pEndNoteShape->get_right();

    //determine yTop
    m_points[0].y = determine_top_line_of_shape();
    m_points[1].y = m_points[0].y;
}

//---------------------------------------------------------------------------------------
LUnits OctaveShiftEngraver::determine_top_line_of_shape()
{
    LUnits yRef = m_uStaffTop;
    LUnits twoLines = tenths_to_logical(20.0f);
    if (octave_shift_at_top())
    {
        yRef -= twoLines;   //2 lines above top staff line
        LUnits yMin = m_pVProfile->get_min_for(m_points[0].x, m_points[1].x, m_idxStaff).first
                      - twoLines;
        yRef = min(yRef, yMin);
    }
    else
    {
        yRef += tenths_to_logical(50.0f);   //1 lines bellow first staff line
        LUnits yMax = m_pVProfile->get_max_for(m_points[0].x, m_points[1].x, m_idxStaff).first
                      + tenths_to_logical(5.0f);
        yRef = max(yRef, yMax);
    }

    return yRef;
}

//---------------------------------------------------------------------------------------
void OctaveShiftEngraver::decide_placement()
{
    m_fPlaceAtTop = m_pOctaveShift->get_shift_steps() < 0;
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
void OctaveShiftEngraver::set_prolog_width(LUnits width)
{
    m_uPrologWidth += width;
}


}  //namespace lomse
