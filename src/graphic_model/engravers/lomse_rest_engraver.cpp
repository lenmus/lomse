//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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
                           ShapesStorage* pShapesStorage)
    : NoterestEngraver(libraryScope, pScoreMeter, pShapesStorage)
    , m_pRest(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShapeRest* RestEngraver::create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                         UPoint uPos, int restType, int numDots,
                                         ImoRest* pRest)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_restType = restType;
    m_numDots = numDots;
    m_pRest = pRest;
    m_pNoteRest = pRest;
    m_uxLeft = uPos.x;
    m_uyTop = uPos.y;
    m_fontSize = determine_font_size();
    m_pCreatorImo = pCreatorImo;

    determine_position();
    create_main_shape();
    add_shapes_for_dots_if_required();

    //add the rest to relation objects
    add_to_beam_if_beamed();
    add_to_tuplet_if_in_tuplet(NULL);

    //if this is the last rest of a relation, finish relation

    // tuplet bracket
    if (is_last_noterest_of_tuplet())
        layout_tuplet();

    return m_pRestShape;
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
    int nIdx = 0;   //single-shape
    m_pRestShape = new GmoShapeRest(m_pCreatorImo, nIdx, m_uxLeft, m_uyTop, Color(0,0,0),
                                    m_libraryScope);
    m_pNoteRestShape = m_pRestShape;

    GmoShape* pGlyph = new GmoShapeRestGlyph(m_pCreatorImo, nIdx, m_iGlyph,
                                             UPoint(m_uxLeft, m_uyTop),
                                             Color(0,0,0), m_libraryScope, m_fontSize);
    m_pRestShape->add(pGlyph);
    m_uxLeft += pGlyph->get_width();
}

//---------------------------------------------------------------------------------------
int RestEngraver::find_glyph()
{
    switch (m_restType)
    {
        case ImoNoteRest::k_longa:        return k_glyph_longa_rest;
        case ImoNoteRest::k_breve:        return k_glyph_breve_rest;
        case ImoNoteRest::k_whole:        return k_glyph_whole_rest;
        case ImoNoteRest::k_half:         return k_glyph_half_rest;
        case ImoNoteRest::k_quarter:      return k_glyph_quarter_rest;
        case ImoNoteRest::k_eighth:       return k_glyph_eighth_rest;
        case ImoNoteRest::k_16th:         return k_glyph_16th_rest;
        case ImoNoteRest::k_32th:         return k_glyph_32nd_rest;
        case ImoNoteRest::k_64th:         return k_glyph_64th_rest;
        case ImoNoteRest::k_128th:        return k_glyph_128th_rest;
        case ImoNoteRest::k_256th:        return k_glyph_256th_rest;
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
            m_uxLeft += add_dot_shape(m_uxLeft, uyPos, Color(0,0,0));
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits RestEngraver::add_dot_shape(LUnits x, LUnits y, Color color)
{
    y += get_glyph_offset(k_glyph_dot) + tenths_to_logical(-75.0);
    GmoShapeDot* pShape = new GmoShapeDot(m_pCreatorImo, 0, k_glyph_dot, UPoint(x, y),
                                          color, m_libraryScope, m_fontSize);
	m_pRestShape->add(pShape);
    return pShape->get_width();
}

//---------------------------------------------------------------------------------------
void RestEngraver::add_to_beam_if_beamed()
{
    if (m_pRest && m_pRest->is_beamed())
    {
        ImoBeam* pBeam = m_pRest->get_beam();
        BeamEngraver* pEngrv
            = dynamic_cast<BeamEngraver*>(m_pShapesStorage->get_engraver(pBeam));
        GmoShapeBeam* pBeamShape = pEngrv->get_beam_shape();
        m_pRestShape->accept_link_from(pBeamShape, k_link_middle);
        pEngrv->add_note_rest(m_pRest, m_pRestShape);
    }
}

//---------------------------------------------------------------------------------------
LUnits RestEngraver::tenths_to_logical(Tenths tenths)
{
    return m_pMeter->tenths_to_logical(tenths, m_iInstr, m_iStaff);
}



}  //namespace lomse
