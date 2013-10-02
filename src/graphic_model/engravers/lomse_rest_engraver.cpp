//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#include "lomse_rest_engraver.h"

#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"
#include "lomse_shapes_storage.h"
#include "lomse_score_meter.h"
#include "lomse_beam_engraver.h"
#include "lomse_shape_beam.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// RestEngraver implementation
//---------------------------------------------------------------------------------------
RestEngraver::RestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           ShapesStorage* pShapesStorage, int iInstr, int iStaff)
    : Engraver(libraryScope, pScoreMeter, iInstr, iStaff)
    , m_pRest(NULL)
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
    m_pRest = NULL;
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
    return UPoint(rest.get_x() - total.get_x() + rest.get_width() / 2.0,
                  rest.get_y() - total.get_y() + rest.get_height() / 2.0 );
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
    m_pRestShape = LOMSE_NEW GmoShapeRest(m_pRest, idx, m_uxLeft, m_uyTop, m_color,
                                    m_libraryScope);
    add_voice(m_pRestShape);

    m_pRestGlyphShape = LOMSE_NEW GmoShapeRestGlyph(m_pRest, idx, m_iGlyph,
                                             UPoint(m_uxLeft, m_uyTop),
                                             m_color, m_libraryScope, m_fontSize);
    add_voice(m_pRestGlyphShape);
    m_pRestShape->add(m_pRestGlyphShape);
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
        case k_32th:         return k_glyph_32nd_rest;
        case k_64th:         return k_glyph_64th_rest;
        case k_128th:        return k_glyph_128th_rest;
        case k_256th:        return k_glyph_256th_rest;
        default:
            //LogMessage(_T("[RestEngraver::find_glyph] Invalid value (%d) for rest type"), nNoteType);
            return k_glyph_quarter_rest;
    }
}

//---------------------------------------------------------------------------------------
LUnits RestEngraver::get_glyph_offset(int iGlyph)
{
    return tenths_to_logical( glyphs_lmbasic2[iGlyph].GlyphOffset );
}

//---------------------------------------------------------------------------------------
void RestEngraver::add_shapes_for_dots_if_required()
{
    if (m_numDots > 0)
    {
        LUnits uSpaceBeforeDot = tenths_to_logical(LOMSE_SPACE_BEFORE_DOT);
        LUnits uyPos = m_uyTop;
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
    y += get_glyph_offset(k_glyph_dot) + tenths_to_logical(-75.0);
    GmoShapeDot* pShape = LOMSE_NEW GmoShapeDot(m_pRest, 0, k_glyph_dot, UPoint(x, y),
                                                color, m_libraryScope, m_fontSize);
    add_voice(pShape);
	m_pRestShape->add(pShape);
    return pShape->get_width();
}



}  //namespace lomse
