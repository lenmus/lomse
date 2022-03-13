//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_rest_engraver.h"

#include "lomse_note_engraver.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"
#include "lomse_engravers_map.h"
#include "lomse_score_meter.h"
#include "lomse_beam_engraver.h"
#include "lomse_shape_beam.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// RestEngraver implementation
//---------------------------------------------------------------------------------------
RestEngraver::RestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           EngraversMap* UNUSED(pEngravers), int iInstr, int iStaff,
                           int clefType, int octaveShift)
    : StaffObjEngraver(libraryScope, pScoreMeter, iInstr, iStaff)
    , m_restType(k_quarter)
    , m_numDots(0)
    , m_clefType(clefType)
    , m_octaveShift(octaveShift)
    , m_pRest(nullptr)
    , m_fontSize(0.0)
    , m_uxLeft(0.0f)
    , m_uyTop(0.0f)
    , m_iGlyph(0)
    , m_pRestShape(nullptr)
    , m_pRestGlyphShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
GmoShapeRest* RestEngraver::create_shape(ImoRest* pRest, UPoint uPos,
                                         Color color)
{
    m_restType = pRest->get_note_type();
    m_numDots = pRest->get_dots();
    m_pRest = pRest;
    m_uxLeft = uPos.x;
    m_uyTop = uPos.y;
    m_fontSize = determine_font_size();
    m_color = color;

    determine_position();
    create_main_shape();
    add_shapes_for_dots_if_required();

    return m_pRestShape;
}

//---------------------------------------------------------------------------------------
GmoShape* RestEngraver::create_tool_dragged_shape(int restType, int dots)
{
    m_restType = restType;
    m_numDots = dots;
    m_pRest = nullptr;
    m_uxLeft = 0.0;
    m_uyTop = 0.0;
    m_fontSize = 21.0;
    m_color = Color(255,0,0);       //TODO: options/configuration

    determine_position();
    create_main_shape();
    add_shapes_for_dots_if_required();

    return m_pRestShape;
}

//---------------------------------------------------------------------------------------
UPoint RestEngraver::get_drag_offset()
{
    //return center of rest shape
    URect total = m_pRestShape->get_bounds();
    URect rest = m_pRestGlyphShape->get_bounds();
    return UPoint(rest.get_x() - total.get_x() + rest.get_width() / 2.0f,
                  rest.get_y() - total.get_y() + rest.get_height() / 2.0f );
}

//---------------------------------------------------------------------------------------
void RestEngraver::determine_position()
{
    m_iGlyph = find_glyph();
    m_uyTop += get_glyph_offset(m_iGlyph);
}

//---------------------------------------------------------------------------------------
void RestEngraver::create_main_shape()
{
    ShapeId idx = 0;
    m_pRestShape = LOMSE_NEW GmoShapeRest(m_pRest, idx++, m_uxLeft, m_uyTop, m_color,
                                    m_libraryScope);
    add_voice(m_pRestShape);

    m_pRestGlyphShape = LOMSE_NEW GmoShapeRestGlyph(m_pRest, idx, m_iGlyph,
                                             UPoint(m_uxLeft, m_uyTop),
                                             m_color, m_libraryScope, m_fontSize);
    add_voice(m_pRestGlyphShape);
    m_pRestShape->add(m_pRestGlyphShape);
    m_pRestShape->set_pos_on_staff( determine_pos_on_staff(m_iGlyph) );
    m_uxLeft += m_pRestGlyphShape->get_width();
}

//---------------------------------------------------------------------------------------
void RestEngraver::add_voice(VoiceRelatedShape* pVRS)
{
    if (m_pRest)
        pVRS->set_voice(m_pRest->get_voice());
}

//---------------------------------------------------------------------------------------
int RestEngraver::find_glyph()
{
    switch (m_restType)
    {
        case k_longa:        return k_glyph_longa_rest;
        case k_breve:        return k_glyph_breve_rest;
        case k_whole:        return k_glyph_whole_rest;
        case k_half:         return k_glyph_half_rest;
        case k_quarter:      return k_glyph_quarter_rest;
        case k_eighth:       return k_glyph_eighth_rest;
        case k_16th:         return k_glyph_16th_rest;
        case k_32nd:         return k_glyph_32nd_rest;
        case k_64th:         return k_glyph_64th_rest;
        case k_128th:        return k_glyph_128th_rest;
        case k_256th:        return k_glyph_256th_rest;
        default:
            //LogMessage(_T("[RestEngraver::find_glyph] Invalid value (%d) for rest type"), nNoteType);
            return k_glyph_quarter_rest;
    }
}

//---------------------------------------------------------------------------------------
int RestEngraver::determine_pos_on_staff(int iGlyph)
{
    // Returns the position on the staff (line/space) referred to the first ledger line of
    // the staff:
    //        0 - on first ledger line (C note in G clef)
    //        1 - on next space (D in G clef)
    //        2 - on first line (E not in G clef)
    //        3 - on first space
    //        4 - on second line
    //        5 - on second space
    //        etc.
    //
    //Default placement is on the center line of a five-line staff,
    //with the exception of the whole note rest, which should hang from the
    //font baseline.
    //
    //if rest placement is defined, it is controlled by step and octave values

    int posOnStaff = 6;     //third line (center)
    if (iGlyph == k_glyph_whole_rest)
        posOnStaff = 8;     //fourth line

    //if rest placement is defined, it is controlled by step and octave values
    if (m_pRest->get_step() != k_step_undefined)
    {
        int pos = NoteEngraver::pitch_to_pos_on_staff(m_pRest, m_clefType, m_octaveShift);
        posOnStaff = (6 - pos);
    }

    return posOnStaff;
}

//---------------------------------------------------------------------------------------
LUnits RestEngraver::get_glyph_offset(int iGlyph)
{
    //default placement: rests are placed on the center line of a five-line staff,
    //with the exception of the whole note rest, which should hang from the
    //font baseline.
    Tenths offset = m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph);
    if (iGlyph == k_glyph_whole_rest)
        offset += 10.0f;
    else
        offset += 20.0f;

    //if rest placement is defined, it is controlled by step and octave values
    if (m_pRest->get_step() != k_step_undefined)
    {
        int pos = NoteEngraver::pitch_to_pos_on_staff(m_pRest, m_clefType, m_octaveShift);
        offset += (6 - pos)*5.0f;
    }
    return tenths_to_logical(offset);
}

//---------------------------------------------------------------------------------------
void RestEngraver::add_shapes_for_dots_if_required()
{
    //AWARE: dots should be placed opposite to the highest hook (Gardner, p.199)

    if (m_numDots > 0)
    {
        LUnits uSpaceBeforeDot = tenths_to_logical(LOMSE_SPACE_BEFORE_DOT);
        LUnits uyPos = m_uyTop + tenths_to_logical( get_offset_for_dot() );
        for (int i = 0; i < m_numDots; i++)
        {
            m_uxLeft += uSpaceBeforeDot;
            m_uxLeft += add_dot_shape(m_uxLeft, uyPos, m_color);
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits RestEngraver::add_dot_shape(LUnits x, LUnits y, Color color)
{
    y += get_glyph_offset(k_glyph_dot);
    GmoShapeDot* pShape = LOMSE_NEW GmoShapeDot(m_pRest, 0, UPoint(x, y), color,
                                                m_libraryScope, m_fontSize);
    add_voice(pShape);
	m_pRestShape->add(pShape);
    return pShape->get_width();
}

//---------------------------------------------------------------------------------------
Tenths RestEngraver::get_offset_for_dot()
{
    switch (m_restType)
    {
        case k_whole:        return -15.0f;
        case k_32nd:         return -35.0f;
        case k_64th:         return -35.0f;
        case k_128th:        return -45.0f;
        case k_256th:        return -45.0f;
        default:
            return -25.0f;
    }
}


}  //namespace lomse
