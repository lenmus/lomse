//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
    : Engraver(libraryScope, pScoreMeter, iInstr, iStaff)
{
}

//---------------------------------------------------------------------------------------
GmoShapeAccidentals* AccidentalsEngraver::create_shape(ImoNote* pNote,
                                                       UPoint uPos,
                                                       EAccidentals accidentals,
                                                       bool fCautionary,
                                                       Color color)
{
    m_accidentals = accidentals;
    m_fCautionary = fCautionary;
    m_fontSize = determine_font_size();
    m_pNote = pNote;
    m_color = color;

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

    switch(m_accidentals)
    {
        case k_natural:
            m_glyphs[i++] = k_glyph_natural_accidental;
            break;
        case k_sharp:
            m_glyphs[i++] = k_glyph_sharp_accidental;
            break;
        case k_flat:
            m_glyphs[i++] = k_glyph_flat_accidental;
            break;
        case k_flat_flat:
            m_glyphs[i++] = k_glyph_double_flat_accidental;
            break;
        case k_double_sharp:
            m_glyphs[i++] = k_glyph_double_sharp_accidental;
            break;
        case k_natural_flat:
            m_glyphs[i++] = k_glyph_natural_accidental;
            m_glyphs[i++] = k_glyph_flat_accidental;
            break;
        case k_natural_sharp:
            m_glyphs[i++] = k_glyph_natural_accidental;
            m_glyphs[i++] = k_glyph_sharp_accidental;
            break;
        case k_sharp_sharp:
            m_glyphs[i++] = k_glyph_sharp_accidental;
            m_glyphs[i++] = k_glyph_sharp_accidental;
            break;
        //case lm_eQuarterFlat:
        //    m_glyphs[i++] = k_glyph_natural_accidental;
        //    break;
        //case lm_eQuarterSharp:
        //    m_glyphs[i++] = k_glyph_natural_accidental;
        //    break;
        //case lm_eThreeQuartersFlat:
        //    m_glyphs[i++] = k_glyph_natural_accidental;
        //    break;
        //case lm_eThreeQuartersSharp:
        //    m_glyphs[i++] = k_glyph_natural_accidental;
        //    break;
        default:
            m_glyphs[i++] = k_glyph_natural_accidental;
            //LogMessage
    }

    if (m_fCautionary)
        m_glyphs[i++] = k_glyph_close_cautionary_accidental;

    m_numGlyphs = i;
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
        int iGlyph = m_glyphs[i];
        LUnits y = pos.y + glyph_offset(iGlyph);
        GmoShapeAccidental* pShape =
            LOMSE_NEW GmoShapeAccidental(m_pNote, 0, iGlyph, UPoint(x, y),
                                         m_color, m_libraryScope, m_fontSize);
        add_voice(pShape);
        m_pContainer->add(pShape);
        x += (pShape->get_left() - x) + pShape->get_width();
        x += m_pMeter->tenths_to_logical(LOMSE_SPACE_BETWEEN_ACCIDENTALS,
                                         m_iInstr, m_iStaff);
    }
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
