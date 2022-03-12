//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_accidentals_engraver.h"

#include "lomse_engraving_options.h"
#include "lomse_im_note.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"
#include "lomse_shapes.h"
#include "lomse_score_meter.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// AccidentalsEngraver implementation
//---------------------------------------------------------------------------------------
AccidentalsEngraver::AccidentalsEngraver(LibraryScope& libraryScope,
                                         ScoreMeter* pScoreMeter, int iInstr, int iStaff)
    : StaffSymbolEngraver(libraryScope, pScoreMeter, iInstr, iStaff)
    , m_accidentals(k_no_accidentals)
    , m_fCautionary(false)
    , m_pContainer(nullptr)
    , m_fontSize(12.0)
    , m_pNote(nullptr)
    , m_numGlyphs(0)
{
}

//---------------------------------------------------------------------------------------
GmoShapeAccidentals* AccidentalsEngraver::create_shape(ImoNote* pNote,
                                                       UPoint uPos,
                                                       EAccidentals accidentals,
                                                       double fontSize,
                                                       bool fCautionary,
                                                       Color color)
{
    m_accidentals = accidentals;
    m_fCautionary = fCautionary;
    m_pNote = pNote;
    m_color = color;
    m_fontSize = fontSize;

    find_glyphs();
    create_container_shape(uPos);
    add_glyphs_to_container_shape(uPos);

    return m_pContainer;
}

//---------------------------------------------------------------------------------------
void AccidentalsEngraver::add_voice(VoiceRelatedShape* pVRS)
{
    if (m_pNote)
        pVRS->set_voice(m_pNote->get_voice());
}

//---------------------------------------------------------------------------------------
void AccidentalsEngraver::find_glyphs()
{
    int i = 0;

    if (m_fCautionary)
        m_glyphs[i++] = k_glyph_open_cautionary_accidental;

    m_glyphs[i++] = get_glyph_for(m_accidentals);

    if (m_fCautionary)
        m_glyphs[i++] = k_glyph_close_cautionary_accidental;

    m_numGlyphs = i;
}

//---------------------------------------------------------------------------------------
int AccidentalsEngraver::get_glyph_for(int accidental)
{
    switch(accidental)
    {
        //standard accidentals
        case k_natural:             return k_glyph_natural_accidental;
        case k_sharp:               return k_glyph_sharp_accidental;
        case k_flat:                return k_glyph_flat_accidental;
        case k_flat_flat:           return k_glyph_double_flat_accidental;
        case k_double_sharp:        return k_glyph_double_sharp_accidental;
        case k_natural_flat:        return k_glyph_accidentalNaturalFlat;
        case k_natural_sharp:       return k_glyph_accidentalNaturalSharp;
        case k_sharp_sharp:         return k_glyph_accidentalSharpSharp;
        case k_acc_triple_sharp:    return k_glyph_accidentalTripleSharp;
        case k_acc_triple_flat:     return k_glyph_accidentalTripleFlat;

        //microtonal accidentals: Tartini-style quarter-tone accidentals
        case k_acc_quarter_flat:            return k_glyph_accidentalQuarterToneFlatStein;
        case k_acc_quarter_sharp:           return k_glyph_accidentalQuarterToneSharpStein;
        case k_acc_three_quarters_flat:     return k_glyph_accidentalThreeQuarterTonesFlatCouper;
        case k_acc_three_quarters_sharp:    return k_glyph_accidentalThreeQuarterTonesSharpStein;

        //microtonal accidentals: quarter-tone accidentals that include arrows pointing down or up
        case k_acc_sharp_down:          return k_glyph_accidentalQuarterToneSharpArrowDown;
        case k_acc_sharp_up:            return k_glyph_accidentalThreeQuarterTonesSharpArrowUp;
        case k_acc_natural_down:        return k_glyph_accidentalQuarterToneFlatNaturalArrowDown;
        case k_acc_natural_up:          return k_glyph_accidentalQuarterToneSharpNaturalArrowUp;
        case k_acc_flat_down:           return k_glyph_accidentalThreeQuarterTonesFlatArrowDown;
        case k_acc_flat_up:             return k_glyph_accidentalQuarterToneFlatArrowUp;
        case k_acc_double_sharp_down:   return k_glyph_accidentalThreeQuarterTonesSharpArrowDown;
        case k_acc_double_sharp_up:     return k_glyph_accidentalFiveQuarterTonesSharpArrowUp;
        case k_acc_flat_flat_down:      return k_glyph_accidentalFiveQuarterTonesFlatArrowDown;
        case k_acc_flat_flat_up:        return k_glyph_accidentalThreeQuarterTonesFlatArrowUp;
        case k_acc_arrow_down:          return k_glyph_accidentalArrowDown;
        case k_acc_arrow_up:            return k_glyph_accidentalArrowUp;

        //accidentals used in Turkish classical music
        case k_acc_slash_quarter_sharp: return k_glyph_accidentalKucukMucennebSharp;
        case k_acc_slash_sharp:         return k_glyph_accidentalBuyukMucennebSharp;
        case k_acc_slash_flat:          return k_glyph_accidentalBakiyeFlat;
        case k_acc_double_slash_flat:   return k_glyph_accidentalBuyukMucennebFlat;

    	//superscripted versions of standard accidental signs, used in Turkish folk music
        case k_acc_sharp_1:     return k_glyph_accidental1CommaSharp;
        case k_acc_sharp_2:     return k_glyph_accidental2CommaSharp;
        case k_acc_sharp_3:     return k_glyph_accidental3CommaSharp;
        case k_acc_sharp_5:     return k_glyph_accidental5CommaSharp;
        case k_acc_flat_1:      return k_glyph_accidental1CommaFlat;
        case k_acc_flat_2:      return k_glyph_accidental2CommaFlat;
        case k_acc_flat_3:      return k_glyph_accidental3CommaFlat;
        case k_acc_flat_4:      return k_glyph_accidental4CommaFlat;

        //microtonal flat and sharp accidentals used in Iranian and Persian music
        case k_acc_sori:    return k_glyph_accidentalSori;
        case k_acc_koron:   return k_glyph_accidentalKoron;

	    //other, usually used in combination with the SMuFl glyph
        case k_acc_other:   return k_glyph_natural_accidental;
            //TODO: use SMuFl glyph if specified

        default:
        {
            LOMSE_LOG_ERROR("Program maintenance error?: Missing case for accidental %d. Natural accidental used.",
                            accidental);
            return k_glyph_natural_accidental;
        }
    }
}

//---------------------------------------------------------------------------------------
void AccidentalsEngraver::create_container_shape(UPoint pos)
{
    ShapeId idx = 0;
    m_pContainer = LOMSE_NEW GmoShapeAccidentals(m_pNote, idx, pos, m_color);
    add_voice(m_pContainer);
}

//---------------------------------------------------------------------------------------
void AccidentalsEngraver::add_glyphs_to_container_shape(UPoint pos)
{
    LUnits x = pos.x;
    for (int i=0; i < m_numGlyphs; ++i)
    {
        GmoShapeAccidental* pShape =
                create_shape_for_glyph(m_glyphs[i], pos, m_color, m_fontSize, m_pNote, 0);

        add_voice(pShape);
        m_pContainer->add(pShape);
        x += (pShape->get_left() - x) + pShape->get_width();
        x += tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS);
    }
}

//---------------------------------------------------------------------------------------
GmoShapeAccidental* AccidentalsEngraver::create_shape_for_glyph(int iGlyph, UPoint pos,
                                                                Color color,
                                                                double fontSize,
                                                                ImoObj* pCreatorImo,
                                                                ShapeId idx)
{
    pos.y += glyph_offset(iGlyph);

    return LOMSE_NEW GmoShapeAccidental(pCreatorImo, idx, iGlyph, pos, color,
                                        m_libraryScope, fontSize);
}

//---------------------------------------------------------------------------------------
LUnits AccidentalsEngraver::glyph_offset(int iGlyph)
{
    //AWARE: Accidentals registration is as follows:
    // * Vertically centered on the baseline.
    // * They are positioned as if they apply to a notehead on the bottom line of the staff.
    // * the leftmost point coincides with x = 0.

    Tenths offset = m_libraryScope.get_glyphs_table()->glyph_offset(iGlyph) + 50.0f;
    return m_pMeter->tenths_to_logical(offset, m_iInstr, m_iStaff);
}


}  //namespace lomse
