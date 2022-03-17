//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_metronome_engraver.h"

#include "lomse_glyphs.h"
#include "lomse_shapes.h"
#include "lomse_score_meter.h"
#include "lomse_internal_model.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"
#include "lomse_logger.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// MetronomeMarkEngraver implementation
//---------------------------------------------------------------------------------------
MetronomeMarkEngraver::MetronomeMarkEngraver(const EngraverContext& ctx)
    : AuxObjEngraver(ctx)
    , m_pMainShape(nullptr)
    , m_fontSize(0.0)
    , m_pCreatorImo(nullptr)
{
}

//---------------------------------------------------------------------------------------
GmoShape* MetronomeMarkEngraver::create_shape(ImoMetronomeMark* pImo, UPoint uPos,
                                              Color color)
{
    ImoStyle* pStyle = m_pMeter->get_style_info("Metronome marks");
    m_fontSize = pStyle->font_size() * 1.5f;
    m_pCreatorImo = pImo;
    m_uPos = uPos;
    m_uPos.y -= m_pMeter->tenths_to_logical(20.0f, m_iInstr, m_iStaff);
    m_color = color;

    create_main_container_shape();
    int markType = pImo->get_mark_type();
    switch(markType)
    {
        case ImoMetronomeMark::k_note_value:
            return create_shape_note_value();
        case ImoMetronomeMark::k_note_note:
            return create_shape_note_note();
        case ImoMetronomeMark::k_value:
            return create_shape_mm_value();
        default:
        {
            stringstream msg;
            msg << "[MetronomeMarkEngraver::create_shape] invalid mark type " <<
                   markType ;
            LOMSE_LOG_ERROR(msg.str());
            return create_shape_note_value();
        }
    }
}

//---------------------------------------------------------------------------------------
// 'm.m. = value'
GmoShape* MetronomeMarkEngraver::create_shape_mm_value()
{
    int ticksPerMinute = m_pCreatorImo->get_ticks_per_minute();
    bool fParenthesis = m_pCreatorImo->has_parenthesis();

    stringstream msg;
    msg << (fParenthesis ? "(" : "") << "M.M. = " << ticksPerMinute << (fParenthesis ? ")" : "");
    create_text_shape(msg.str());
    return m_pMainShape;
}

//---------------------------------------------------------------------------------------
// 'note_symbol = note_symbol'
GmoShape* MetronomeMarkEngraver::create_shape_note_note()
{
    int leftNoteType = m_pCreatorImo->get_left_note_type();
    int leftDots = m_pCreatorImo->get_left_dots();
    int rightNoteType = m_pCreatorImo->get_right_note_type();
    int rightDots = m_pCreatorImo->get_right_dots();
    bool fParenthesis = m_pCreatorImo->has_parenthesis();

    if (fParenthesis)
        create_text_shape("(");
    create_symbol_shape(leftNoteType, leftDots);
    create_text_shape(" = ");
    create_symbol_shape(rightNoteType, rightDots);
    if (fParenthesis)
        create_text_shape(")");
    return m_pMainShape;
}

//---------------------------------------------------------------------------------------
// 'note_symbol = value'
GmoShape* MetronomeMarkEngraver::create_shape_note_value()
{
    int ticksPerMinute = m_pCreatorImo->get_ticks_per_minute();
    int leftNoteType = m_pCreatorImo->get_left_note_type();
    int leftDots = m_pCreatorImo->get_left_dots();
    bool fParenthesis = m_pCreatorImo->has_parenthesis();

    if (fParenthesis)
        create_text_shape("(");
    create_symbol_shape(leftNoteType, leftDots);
    stringstream msg;
    msg << " = " << ticksPerMinute << (fParenthesis ? ")" : "");
    create_text_shape(msg.str());
    return m_pMainShape;
}

//---------------------------------------------------------------------------------------
void MetronomeMarkEngraver::create_main_container_shape()
{
    ShapeId idx = 0;
    m_pMainShape = LOMSE_NEW GmoShapeMetronomeMark(m_pCreatorImo, idx, m_uPos,
                                                   m_color, m_libraryScope);
}

//---------------------------------------------------------------------------------------
void MetronomeMarkEngraver::create_text_shape(const string& text)
{
    LUnits y = m_uPos.y + m_pMeter->tenths_to_logical(2.0f, m_iInstr, m_iStaff);
    ImoStyle* pStyle = m_pMeter->get_style_info("Metronome marks");
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", pStyle,
                      TextEngraver::k_class_metronome_text);
    GmoShape* pShape = engr.create_shape(m_pCreatorImo, 0, m_uPos.x, y);
	m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width();
}

//---------------------------------------------------------------------------------------
void MetronomeMarkEngraver::create_symbol_shape(int noteType, int dots)
{
    int iGlyph = select_glyph(noteType);
    Tenths yOffset = m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph);
    LUnits y = m_uPos.y + m_pMeter->tenths_to_logical(yOffset, m_iInstr, m_iStaff);
    ShapeId idx = 0;
    GmoShape* pShape = LOMSE_NEW GmoShapeMetronomeGlyph(m_pCreatorImo, idx, iGlyph,
                                               UPoint(m_uPos.x, y), m_color,
                                               m_libraryScope, m_fontSize);
	m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width();

    if (dots > 0)
    {
        LUnits space = m_pMeter->tenths_to_logical(2.0, m_iInstr, m_iStaff);
        for (; dots > 0; dots--)
        {
            m_uPos.x += space;
            pShape = LOMSE_NEW GmoShapeMetronomeGlyph(m_pCreatorImo, idx,
                                               k_glyph_metronome_augmentation_dot,
                                               m_uPos, m_color,
                                               m_libraryScope, m_fontSize);
            m_pMainShape->add(pShape);
            m_uPos.x += pShape->get_width();
        }
    }
}

//---------------------------------------------------------------------------------------
int MetronomeMarkEngraver::select_glyph(int noteType)
{
    switch (noteType)
	{
        case k_longa:
            return k_glyph_small_longa_note;
        case k_whole:
            return k_glyph_small_whole_note;
        case k_half:
            return k_glyph_small_half_note;
        case k_quarter:
            return k_glyph_small_quarter_note;
        case k_eighth:
            return k_glyph_small_eighth_note;
        case k_16th:
            return k_glyph_small_16th_note;
        case k_32nd:
            return k_glyph_small_32nd_note;
        case k_64th:
            return k_glyph_small_64th_note;
        case k_128th:
            return k_glyph_small_128th_note;
        case k_256th:
            return k_glyph_small_256th_note;
        default:
        {
            LOMSE_LOG_ERROR(
                "[MetronomeMarkEngraver::select_glyph] invalid note type %d", noteType);
            return k_glyph_error;
        }
    }
}


}  //namespace lomse
