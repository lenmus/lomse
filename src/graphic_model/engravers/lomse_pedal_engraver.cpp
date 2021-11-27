//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#include "lomse_pedal_engraver.h"

#include "lomse_aux_shapes_aligner.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_instrument_engraver.h"
#include "lomse_internal_model.h"
#include "lomse_score_meter.h"
#include "lomse_shape_pedal_line.h"
#include "lomse_shape_note.h"
#include "lomse_shape_text.h"
#include "lomse_shapes.h"
#include "lomse_text_engraver.h"
#include "lomse_vertical_profile.h"

#include <limits>


namespace lomse
{

//---------------------------------------------------------------------------------------
// PedalMarkEngraver implementation
//---------------------------------------------------------------------------------------

PedalMarkEngraver::PedalMarkEngraver(const EngraverContext& ctx)
    : AuxObjEngraver(ctx)
{
}

//---------------------------------------------------------------------------------------
GmoShapePedalGlyph* PedalMarkEngraver::create_shape(ImoPedalMark* pPedalMark, UPoint pos,
                                                    const Color& color,
                                                    GmoShape* UNUSED(pParentShape))
{
    const int iGlyph = find_glyph(pPedalMark);

    //determine font size to use
    const double fontSize = determine_font_size();

    //create the shape
    GmoShapePedalGlyph* pShape = LOMSE_NEW GmoShapePedalGlyph(pPedalMark, 0, iGlyph,
                                                              pos, color,
                                                              m_libraryScope, fontSize);

    if (pPedalMark->get_type() == k_pedal_mark_stop)
        pShape->shift_origin(USize(-pShape->get_width(), 0));

    //adjust vertical position
    const bool fAbove = (pPedalMark->get_placement() == k_placement_above);
    const LUnits y = determine_y_pos(pShape->get_left(), pShape->get_right(), pos.y, pShape->get_relative_baseline_y(), fAbove);
    pShape->set_top(y);

    add_to_aux_shapes_aligner(pShape, fAbove);

    return pShape;
}

//---------------------------------------------------------------------------------------
double PedalMarkEngraver::determine_font_size()
{
    return 16.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}

//---------------------------------------------------------------------------------------
int PedalMarkEngraver::find_glyph(const ImoPedalMark* pPedalMark)
{
    switch (pPedalMark->get_type())
    {
        case k_pedal_mark_unknown:
        case k_pedal_mark_start:
            return pPedalMark->is_abbreviated() ? k_glyph_pedal_p : k_glyph_pedal_mark;
        case k_pedal_mark_sostenuto_start:
            return pPedalMark->is_abbreviated() ? k_glyph_pedal_s : k_glyph_pedal_sostenuto;
        case k_pedal_mark_stop:
            return k_glyph_pedal_up_mark;
    }

    return 0;
}

//---------------------------------------------------------------------------------------
LUnits PedalMarkEngraver::determine_y_pos(LUnits xLeft, LUnits xRight, LUnits yRef, LUnits yBaselineOffset, bool fAbove) const
{
    const LUnits yDistanceToStaff = tenths_to_logical(LOMSE_PEDAL_STAFF_DISTANCE);
    const LUnits yDistanceToContent = tenths_to_logical(LOMSE_PEDAL_CONTENT_DISTANCE);
    const LUnits staffHeight = tenths_to_logical(40.0f);

    if (fAbove)
    {
        yRef -= yDistanceToStaff - yBaselineOffset;
        LUnits yMin = m_pVProfile->get_min_for(xLeft, xRight, m_idxStaff).first
                      - yDistanceToContent;
        yRef = min(yRef, yMin);
    }
    else
    {
        yRef += staffHeight + yDistanceToStaff - yBaselineOffset;
        LUnits yMax = m_pVProfile->get_max_for(xLeft, xRight, m_idxStaff).first
                      + yDistanceToContent;
        yRef = max(yRef, yMax);
    }

    return yRef;
}

//---------------------------------------------------------------------------------------
// PedalLineEngraver implementation
//---------------------------------------------------------------------------------------
PedalLineEngraver::PedalLineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : RelObjEngraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc)
{
    m_iInstr = aoc.iInstr;
    m_iStaff = aoc.iStaff;
    m_idxStaff = aoc.idxStaff;

    m_pPedal = static_cast<ImoPedalLine*>(pRO);

    m_pStartDirection = static_cast<ImoDirection*>(aoc.pSO);
    m_pStartDirectionShape = static_cast<GmoShapeInvisible*>(aoc.pStaffObjShape);
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::set_middle_staffobj(ImoRelObj* UNUSED(pRO), const AuxObjContext& aoc)
{
    GmoShapeInvisible* pDirectionShape = static_cast<GmoShapeInvisible*>(aoc.pStaffObjShape);
    m_pedalChangeDirectionShapes.push_back(pDirectionShape);
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), const AuxObjContext& aoc)
{
    m_pEndDirectionShape = static_cast<GmoShapeInvisible*>(aoc.pStaffObjShape);
}

//---------------------------------------------------------------------------------------
double PedalLineEngraver::determine_font_size()
{
    return 16.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::decide_placement()
{
    m_fPedalAbove = (m_pStartDirection->get_placement() == k_placement_above);
}

//---------------------------------------------------------------------------------------
GmoShape* PedalLineEngraver::create_first_or_intermediate_shape(const RelObjEngravingContext& ctx)
{
    save_context_parameters(ctx);
    return create_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* PedalLineEngraver::create_last_shape(const RelObjEngravingContext& ctx)
{
    save_context_parameters(ctx);
    return create_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* PedalLineEngraver::create_shape()
{
    const bool first = (m_numShapes == 0);

    if (first)
        decide_placement();

    GmoShapePedalLine* pShape = LOMSE_NEW GmoShapePedalLine(m_pPedal, m_numShapes++,
                                                            m_pPedal->get_color());

    compute_shape_position(first);

    if (m_xEnd <= m_xStart)
        return nullptr;

    if (!first && m_pPedal->get_draw_continuation_text())
        add_pedal_continuation_text(pShape);

    add_pedal_changes(pShape);
    m_pedalChangeDirectionShapes.clear();

    add_line_info(pShape, m_pPedalStartShape, m_pPedalEndShape, first);
    add_to_aux_shapes_aligner(pShape, m_fPedalAbove);

    return pShape;
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::compute_shape_position(bool first)
{
    m_xStart = determine_shape_position_left(first);
    m_xEnd = determine_shape_position_right();

    if (AuxShapesAligner* aligner = get_aux_shapes_aligner(m_idxStaff, m_fPedalAbove))
    {
        GmoShape* pStartShape = aligner->find_shape(m_xStart);

        if (pStartShape && pStartShape->is_shape_pedal_glyph() && pStartShape->get_creator_imo()->is_pedal_mark())
        {
            const ImoPedalMark* pStartPedalMark = static_cast<const ImoPedalMark*>(pStartShape->get_creator_imo());
            if (pStartPedalMark->is_start_type())
            {
                m_pPedalStartShape = static_cast<const GmoShapePedalGlyph*>(pStartShape);
                m_xStart = m_pPedalStartShape->get_right() + tenths_to_logical(LOMSE_PEDAL_SPACE_TO_LINE);
            }
        }

        GmoShape* pEndShape = aligner->find_shape(m_xEnd);

        if (pEndShape && pEndShape->is_shape_pedal_glyph() && pEndShape->get_creator_imo()->is_pedal_mark())
        {
            const ImoPedalMark* pEndPedalMark = static_cast<const ImoPedalMark*>(pEndShape->get_creator_imo());
            if (pEndPedalMark->is_end_type())
            {
                m_pPedalEndShape = static_cast<const GmoShapePedalGlyph*>(pEndShape);
                m_xEnd = m_pPedalEndShape->get_left() - tenths_to_logical(LOMSE_PEDAL_SPACE_TO_LINE);
            }
        }
    }

    m_lineY = determine_default_base_line_y();
    adjust_vertical_position(m_xStart, m_xEnd, 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits PedalLineEngraver::determine_shape_position_left(bool first) const
{
    if (!first)
        return m_pInstrEngrv->get_staves_left() + m_uPrologWidth - tenths_to_logical(10.0f);

    StaffObjShapeCursor cursor(m_pStartDirectionShape);
    const TimeUnits maxTime = m_pStartDirection->get_time();

    constexpr LUnits xMaxValue = std::numeric_limits<LUnits>::max();
    LUnits xNext = xMaxValue;

    while (cursor.next(maxTime))
    {
        GmoShape* pShape = cursor.get_shape();
        LUnits x;

        if (pShape->is_shape_note())
            x = static_cast<GmoShapeNote*>(pShape)->get_notehead_left();
        else if (pShape->is_shape_rest())
            x = pShape->get_left();
        else
            continue;

        if (x < xNext)
            xNext = x;
    }

    if (xNext < xMaxValue)
        return xNext;

    return m_pStartDirectionShape->get_left() + tenths_to_logical(10.0f);
}

//---------------------------------------------------------------------------------------
LUnits PedalLineEngraver::determine_shape_position_right() const
{
    if (!m_pEndDirectionShape)
        return m_pInstrEngrv->get_staves_right();

    StaffObjShapeCursor cursor(m_pEndDirectionShape);

    LUnits xRight = m_pEndDirectionShape->get_left();
    bool hasNoteRestAfterEnd = false;

    while (cursor.next())
    {
        GmoShape* pShape = cursor.get_shape();

        if (pShape->get_creator_imo()->is_note_rest())
        {
            hasNoteRestAfterEnd = true;
            break;
        }
    }

    if (!hasNoteRestAfterEnd)
        xRight -= tenths_to_logical(10.0f);

    return xRight;
}

//---------------------------------------------------------------------------------------
LUnits PedalLineEngraver::determine_default_base_line_y() const
{
    const LUnits yDistanceToStaff = tenths_to_logical(LOMSE_PEDAL_STAFF_DISTANCE);

    if (m_fPedalAbove)
    {
        return m_uStaffTop - yDistanceToStaff;
    }
    else
    {
        const LUnits staffHeight = tenths_to_logical(40.0f);
        return m_uStaffTop + staffHeight + yDistanceToStaff;
    }
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::adjust_vertical_position(LUnits xLeft, LUnits xRight, LUnits height, GmoShapePedalLine* pMainShape)
{
    const LUnits yDistanceToContent = tenths_to_logical(LOMSE_PEDAL_CONTENT_DISTANCE);
    LUnits yShift = 0.0f;

    if (m_fPedalAbove)
    {
        const LUnits yProfile = m_pVProfile->get_min_for(xLeft, xRight, m_idxStaff).first
                                - yDistanceToContent;
        if (yProfile < m_lineY)
        {
            yShift = yProfile - m_lineY;
            m_lineY = yProfile;
        }
    }
    else
    {
        const LUnits yTop = m_lineY - height;
        const LUnits yProfile = m_pVProfile->get_max_for(xLeft, xRight, m_idxStaff).first
                                + yDistanceToContent;
        if (yProfile > yTop)
        {
            yShift = yProfile - yTop;
            m_lineY += yShift;
        }
    }

    if (pMainShape && yShift)
    {
        pMainShape->shift_origin(USize(0.0f, yShift));
    }
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::add_pedal_changes(GmoShapePedalLine* pMainShape)
{
    if (m_pedalChangeDirectionShapes.empty())
        return;

    const int iGlyph = k_glyph_pedal_up_notch;
    const LUnits overlap = tenths_to_logical(LOMSE_PEDAL_CHANGE_GLYPH_OVERLAP);
    const double fontSize = determine_font_size();

    for (GmoShapeInvisible* pDirectionShape : m_pedalChangeDirectionShapes)
    {
        const int x = pDirectionShape->get_left();

        GmoShapePedalGlyph* pShape = LOMSE_NEW GmoShapePedalGlyph(m_pPedal, 0, iGlyph,
                                                                  UPoint(x, m_lineY),
                                                                  m_color, m_libraryScope, fontSize);
        pMainShape->add(pShape);

        pMainShape->add_line_gap(pShape->get_left() + overlap, pShape->get_right() - overlap);
        adjust_vertical_position(pShape->get_left(), pShape->get_right(), pShape->get_height(), pMainShape);
    }
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::add_pedal_continuation_text(GmoShapePedalLine* pMainShape) {
    ImoStyle* pTextStyle = m_pMeter->get_style_info("Default style");

    TextEngraver engrLeft(m_libraryScope, m_pMeter, "(", "it", pTextStyle);
    GmoShapeText* pLeftBracketShape = engrLeft.create_shape(m_pPedal, m_xStart, m_lineY);
    add_pedal_continuation_part_shape(pMainShape, pLeftBracketShape);

    const int iGlyph = m_pPedal->is_sostenuto() ? k_glyph_pedal_sostenuto : k_glyph_pedal_mark;
    const double fontSize = determine_font_size();
    GmoShape* pGlyphShape = LOMSE_NEW GmoShapePedalGlyph(m_pPedal, 0, iGlyph,
                                                         UPoint(m_xStart, m_lineY),
                                                         m_color, m_libraryScope, fontSize);
    add_pedal_continuation_part_shape(pMainShape, pGlyphShape);

    TextEngraver engrRight(m_libraryScope, m_pMeter, ")", "it", pTextStyle);
    GmoShapeText* pRightBracketShape = engrRight.create_shape(m_pPedal, m_xStart, m_lineY);
    add_pedal_continuation_part_shape(pMainShape, pRightBracketShape);

    const LUnits space = tenths_to_logical(LOMSE_PEDAL_SPACE_TO_LINE);
    m_xStart += space;

    if (m_xStart >= m_xEnd - space)
    {
        //handle the (unlikely) case if the line ends before the "(Ped.)" text ends
        const LUnits xDiff = m_xEnd - 2 * space - m_xStart;
        m_xStart += xDiff;
        pMainShape->shift_origin(USize(xDiff, 0));
    }
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::add_pedal_continuation_part_shape(GmoShapePedalLine* pMainShape, GmoShape* pAddedShape)
{
    const LUnits yShift = pAddedShape->get_baseline_y() - pAddedShape->get_top();
    pAddedShape->set_top(m_lineY - yShift);
    m_xStart = pAddedShape->get_right();
    pMainShape->add(pAddedShape);
    adjust_vertical_position(pAddedShape->get_left(), pAddedShape->get_right(), pAddedShape->get_height(), pMainShape);
}

//---------------------------------------------------------------------------------------
void PedalLineEngraver::add_line_info(GmoShapePedalLine* pMainShape, const GmoShape* pPedalStartShape, const GmoShape* pPedalEndShape, bool start)
{
    const LUnits thickness = tenths_to_logical(LOMSE_PEDAL_LINE_THICKNESS);
    const LUnits cornerHeight = 200.0f;

    const bool fStartCorner = start && m_pPedal->get_draw_start_corner() && !pPedalStartShape;
    const bool fEndCorner = m_pEndDirectionShape && m_pPedal->get_draw_end_corner() && !pPedalEndShape;

    if (fStartCorner || fEndCorner)
    {
        const LUnits edgeWidth = 3.0f * thickness; // can be anything larger than line thickness + some space

        if (fStartCorner)
            adjust_vertical_position(m_xStart, m_xStart + edgeWidth, cornerHeight, pMainShape);

        if (fEndCorner)
            adjust_vertical_position(m_xEnd, m_xEnd - edgeWidth, cornerHeight, pMainShape);
    }

    const LUnits yStart = m_lineY - 0.5f * thickness;
    const LUnits yEnd = yStart + (m_fPedalAbove ? cornerHeight : -cornerHeight);

    pMainShape->set_layout_data(m_xStart, m_xEnd, yStart, yEnd, thickness, fStartCorner, fEndCorner);
}


}  //namespace lomse
