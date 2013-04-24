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

#include "lomse_metronome_engraver.h"

#include "lomse_glyphs.h"
#include "lomse_shapes.h"
#include "lomse_score_meter.h"
#include "lomse_internal_model.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"

//other
#include <boost/format.hpp>

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// MetronomeMarkEngraver implementation
//---------------------------------------------------------------------------------------
MetronomeMarkEngraver::MetronomeMarkEngraver(LibraryScope& libraryScope,
                                             ScoreMeter* pScoreMeter, int iInstr,
                                             int iStaff)
    : Engraver(libraryScope, pScoreMeter, iInstr, iStaff)
{
}

//---------------------------------------------------------------------------------------
GmoShape* MetronomeMarkEngraver::create_shape(ImoMetronomeMark* pImo, UPoint uPos)
{
    m_fontSize = determine_font_size();
    m_pCreatorImo = pImo;
    m_uPos = uPos;
    m_uPos.y -= m_pMeter->tenths_to_logical(20.0f, m_iInstr, m_iStaff);;

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
            string msg = str( boost::format(
                            "[MetronomeMarkEngraver::create_shape] invalid mark type %d")
                            % markType );
            throw std::runtime_error(msg);
        }
    }
}

//---------------------------------------------------------------------------------------
// 'm.m. = value'
GmoShape* MetronomeMarkEngraver::create_shape_mm_value()
{
    int ticksPerMinute = m_pCreatorImo->get_ticks_per_minute();
    bool fParenthesis = m_pCreatorImo->has_parenthesis();

    string msg = str( fParenthesis ? boost::format("(M.M. = %d)") % ticksPerMinute
                                   : boost::format("M.M. = %d") % ticksPerMinute );
    create_text_shape(msg);
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
    create_symbol_shape( select_glyph(leftNoteType, leftDots) );
    create_text_shape(" = ");
    create_symbol_shape( select_glyph(rightNoteType, rightDots) );
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
    create_symbol_shape( select_glyph(leftNoteType, leftDots) );
    string msg = str( fParenthesis ? boost::format(" = %d)") % ticksPerMinute
                                   : boost::format(" = %d") % ticksPerMinute );
    create_text_shape(msg);
    return m_pMainShape;
}

//---------------------------------------------------------------------------------------
void MetronomeMarkEngraver::create_main_container_shape()
{
    ShapeId idx = 0;
    m_pMainShape = LOMSE_NEW GmoShapeMetronomeMark(m_pCreatorImo, idx, m_uPos,
                                                   Color(0,0,0), m_libraryScope);
}

//---------------------------------------------------------------------------------------
void MetronomeMarkEngraver::create_text_shape(const string& text)
{
    LUnits y = m_uPos.y + m_pMeter->tenths_to_logical(10.0f, m_iInstr, m_iStaff);
    ImoStyle* pStyle = m_pMeter->get_metronome_style_info();
    TextEngraver engr(m_libraryScope, m_pMeter, text, "", pStyle);
    GmoShape* pShape = engr.create_shape(m_pCreatorImo, m_uPos.x, y);
	m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width();
}

//---------------------------------------------------------------------------------------
void MetronomeMarkEngraver::create_symbol_shape(int iGlyph)
{
    Tenths yOffset = glyphs_lmbasic2[iGlyph].GlyphOffset + 5.0f;
    LUnits y = m_uPos.y + m_pMeter->tenths_to_logical(yOffset, m_iInstr, m_iStaff);
    GmoShape* pShape = LOMSE_NEW GmoShapeMetronomeGlyph(m_pCreatorImo, 0, iGlyph,
                                               UPoint(m_uPos.x, y), Color(0,0,0),
                                               m_libraryScope, m_fontSize);
	m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width();
}

//---------------------------------------------------------------------------------------
int MetronomeMarkEngraver::select_glyph(int noteType, int dots)
{
    switch (noteType)
	{
        case k_quarter:
            if (dots == 0)
                return k_glyph_small_quarter_note;
            else
                return k_glyph_small_quarter_note_dotted;
            break;
        case k_eighth:
            if (dots == 0)
                return k_glyph_small_eighth_note;
            else
                return k_glyph_small_eighth_note_dotted;
        default:
        {
            string msg = str( boost::format(
                            "[MetronomeMarkEngraver::select_glyph] invalid note type %d")
                            % noteType );
            throw std::runtime_error(msg);
        }
    }
}


}  //namespace lomse
