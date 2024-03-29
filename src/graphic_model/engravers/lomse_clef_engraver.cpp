//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_clef_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shapes.h"
#include "lomse_box_slice_instr.h"
#include "lomse_font_storage.h"
#include "lomse_score_meter.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// ClefEngraver implementation
//---------------------------------------------------------------------------------------
ClefEngraver::ClefEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           int iInstr, int iStaff)
    : StaffObjEngraver(libraryScope, pScoreMeter, iInstr, iStaff)
    , m_nClefType(0)
    , m_symbolSize(0)
    , m_iGlyph(0)
    , m_pClefShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
ClefEngraver::ClefEngraver(LibraryScope& libraryScope)
    : StaffObjEngraver(libraryScope, nullptr, 0, 0)
    , m_nClefType(0)
    , m_symbolSize(0)
    , m_iGlyph(0)
    , m_pClefShape(nullptr)
{
    //constructor for dragged images
}

//---------------------------------------------------------------------------------------
GmoShape* ClefEngraver::create_shape(ImoClef* pCreatorImo, UPoint uPos, int clefType,
                                     int symbolSize, Color color)
{
    m_nClefType = clefType;
    m_symbolSize = symbolSize;
    m_iGlyph = find_glyph(clefType);

    // get the shift to the staff on which the clef must be drawn
    LUnits y = uPos.y + m_pMeter->tenths_to_logical(get_glyph_offset(), m_iInstr, m_iStaff);

//    //fix shift for TAB clef
//    if (m_nClefType == k_clef_TAB)
//    {
//        y = uPos.y + m_pMeter->tenths_to_logical(get_glyph_offset(), m_iInstr, m_iStaff);
//    }

    //Add minimum space before clef change
    LUnits x = uPos.x;
    if (symbolSize == k_size_cue)
        x += m_pMeter->tenths_to_logical(20.0f, m_iInstr, m_iStaff);

    double fontSize = determine_font_size();

    //create the shape object
    ShapeId idx = 0;
    m_pClefShape = LOMSE_NEW GmoShapeClef(pCreatorImo, idx, m_iGlyph, UPoint(x, y),
                                          color, m_libraryScope, fontSize);
    return m_pClefShape;
}

//---------------------------------------------------------------------------------------
GmoShape* ClefEngraver::create_tool_dragged_shape(int clefType)
{
    Color color(255,0,0);       //TODO: options/configuration
    double fontSize = 21.0;
    ShapeId idx = 0;
    int iGlyph = find_glyph(clefType);
    UPoint pos(0.0, 0.0);

    m_pClefShape = LOMSE_NEW GmoShapeClef(nullptr, idx, iGlyph, pos, color, m_libraryScope,
                                          fontSize);
    return m_pClefShape;
}

//---------------------------------------------------------------------------------------
UPoint ClefEngraver::get_drag_offset()
{
    //return center of clef
    URect bounds = m_pClefShape->get_bounds();
    return UPoint(bounds.get_width() / 2.0f,
                  bounds.get_height() / 2.0f );
}

//---------------------------------------------------------------------------------------
int ClefEngraver::find_glyph(int clefType)
{
    // returns the index (over global glyphs table) to the character to use to print
    // the clef (LenMus font)

    switch (clefType)
    {
        case k_clef_G2: return k_glyph_g_clef;
        case k_clef_F4: return k_glyph_f_clef;
        case k_clef_F3: return k_glyph_f_clef;
        case k_clef_C1: return k_glyph_c_clef;
        case k_clef_C2: return k_glyph_c_clef;
        case k_clef_C3: return k_glyph_c_clef;
        case k_clef_C4: return k_glyph_c_clef;
        case k_clef_C5: return k_glyph_c_clef;
        case k_clef_F5: return k_glyph_f_clef;
        case k_clef_G1: return k_glyph_g_clef;
        case k_clef_8_G2: return k_glyph_g_clef_ottava_alta;
        case k_clef_G2_8: return k_glyph_g_clef_ottava_bassa;
        case k_clef_8_F4: return k_glyph_f_clef_ottava_alta;
        case k_clef_F4_8: return k_glyph_f_clef_ottava_bassa;
        case k_clef_15_G2: return k_glyph_g_clef_quindicesima_alta;
        case k_clef_G2_15: return k_glyph_g_clef_quindicesima_bassa;
        case k_clef_15_F4: return k_glyph_f_clef_quindicesima_alta;
        case k_clef_F4_15: return k_glyph_f_clef_quindicesima_bassa;
        case k_clef_percussion: return k_glyph_percussion_clef_block;
        case k_clef_TAB: return k_glyph_TAB_clef;
        case k_clef_none: return k_glyph_g_clef;
        default:
        {
            LOMSE_LOG_ERROR("No glyph defined for clef type %d.", clefType);
            return k_glyph_g_clef;
        }
    }
}

//---------------------------------------------------------------------------------------
double ClefEngraver::determine_font_size()
{
    double fontSize = StaffSymbolEngraver::determine_font_size();

    switch (m_symbolSize)
    {
        case k_size_cue:        return fontSize * 0.80;
        case k_size_large:      return fontSize * 1.34;
        default:                return fontSize;
    }
}

//---------------------------------------------------------------------------------------
Tenths ClefEngraver::get_glyph_offset()
{
    // returns the y-axis offset from paper cursor position so that shape get correctly
    // positioned over a five-lines staff (units: tenths of inter-line space)

    //AWARE: Clef registration is as follows:
    // * The baseline is on the pitch the clef refers to (e.g. the F clef is placed such
    //   that the upper dot is above and the lower dot below the baseline).
    // * If a clef does not refer specifically to a pitch, its y=0 should coincide with
    //   the center staff line on a five-line staff, or the visual center for staves with more or fewer than five lines (e.g. tablature staves).
    // * The leftmost point coincides with x = 0.

    Tenths yOffset = m_libraryScope.get_glyphs_table()->glyph_offset(m_iGlyph);

    //add offset to move the clef up/down the required lines
    if (m_symbolSize == k_size_cue)
    {
        switch(m_nClefType)
        {
            case k_clef_G2:     return yOffset + 30.0f;
            case k_clef_8_G2:   return yOffset + 30.0f;     //8 above
            case k_clef_G2_8:   return yOffset + 30.0f;     //8 below
            case k_clef_15_G2:  return yOffset + 30.0f;     //15 above
            case k_clef_G2_15:  return yOffset + 30.0f;     //15 below
            case k_clef_G1:     return yOffset + 40.0f;

            case k_clef_F5:     return yOffset;
            case k_clef_F4:     return yOffset + 9.0f;
            case k_clef_8_F4:   return yOffset + 9.0f;      //8 above
            case k_clef_F4_8:   return yOffset + 9.0f;      //8 below
            case k_clef_15_F4:  return yOffset + 9.0f;      //15 above
            case k_clef_F4_15:  return yOffset + 9.0f;      //15 below
            case k_clef_F3:     return yOffset + 19.0f;

            case k_clef_C1:     return yOffset + 39.0f;
            case k_clef_C2:     return yOffset + 29.0f;
            case k_clef_C3:     return yOffset + 19.0f;
            case k_clef_C4:     return yOffset + 9.0f;
            case k_clef_C5:     return yOffset - 1.0f;

            case k_clef_percussion:     return yOffset + 20.0f;
            case k_clef_TAB:            return yOffset + 25.0f;     //assumes 6 lines
            case k_clef_none:           return yOffset + 30.0f;
            default:
            {
                LOMSE_LOG_ERROR("No offset defined for clef type %d.", m_nClefType);
                return yOffset;
            }
        }
    }
    else
    {
        switch(m_nClefType)
        {
            case k_clef_G2:     return yOffset + 30.0f;
            case k_clef_8_G2:   return yOffset + 30.0f;     //8 above
            case k_clef_G2_8:   return yOffset + 30.0f;     //8 below
            case k_clef_15_G2:  return yOffset + 30.0f;     //15 above
            case k_clef_G2_15:  return yOffset + 30.0f;     //15 below
            case k_clef_G1:     return yOffset + 40.0f;

            case k_clef_F5:     return yOffset;
            case k_clef_F4:     return yOffset + 10.0f;
            case k_clef_8_F4:   return yOffset + 10.0f;     //8 above
            case k_clef_F4_8:   return yOffset + 10.0f;     //8 below
            case k_clef_15_F4:  return yOffset + 10.0f;     //15 above
            case k_clef_F4_15:  return yOffset + 10.0f;     //15 below
            case k_clef_F3:     return yOffset + 20.0f;

            case k_clef_C1:     return yOffset + 40.0f;
            case k_clef_C2:     return yOffset + 30.0f;
            case k_clef_C3:     return yOffset + 20.0f;
            case k_clef_C4:     return yOffset + 10.0f;
            case k_clef_C5:     return yOffset;

            case k_clef_percussion:     return yOffset + 20.0f;
            case k_clef_TAB:            return yOffset + 25.0f;     //assumes 6 lines
            case k_clef_none:           return yOffset + 30.0f;

            default:
            {
                LOMSE_LOG_ERROR("No offset defined for clef type %d.", m_nClefType);
                return yOffset;
            }
        }
    }
}


}  //namespace lomse
