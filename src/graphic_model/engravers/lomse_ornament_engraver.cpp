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

#include "lomse_ornament_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// OrnamentEngraver implementation
//---------------------------------------------------------------------------------------
OrnamentEngraver::OrnamentEngraver(LibraryScope& libraryScope,
                                           ScoreMeter* pScoreMeter,
                                           int UNUSED(iInstr), int UNUSED(iStaff))
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShapeOrnament* OrnamentEngraver::create_shape(ImoOrnament* pOrnament,
                                                         UPoint pos, Color color,
                                                         GmoShape* pParentShape)
{
    m_pOrnament = pOrnament;
    m_placement = pOrnament->get_placement();
    m_pParentShape = pParentShape;
    m_fAbove = determine_if_above();

    int iGlyph = find_glyph();
    double fontSize = determine_font_size();
    UPoint position = compute_location(pos);
    ShapeId idx = 0;
    m_pOrnamentShape =
        LOMSE_NEW GmoShapeOrnament(pOrnament, idx, iGlyph, position,
                                       color, m_libraryScope, fontSize);
    add_voice();

    if (m_pOrnament->get_ornament_type() != k_ornament_tremolo )
    {
        center_on_parent();
    }

    return m_pOrnamentShape;
}

//---------------------------------------------------------------------------------------
UPoint OrnamentEngraver::compute_location(UPoint pos)
{
//    //TODO: Most shifts are based on glyph size, and this depends on font design.
//    //      Instead, it is necessary to measure glyphs
//
//    int type = m_pOrnament->get_ornament_type();
//
//    if (type == k_ornament_breath_mark)
//    {
//        pos.x = m_pParentShape->get_right() + tenths_to_logical(10.0f);
//        pos.y += tenths_to_logical(5.0f);
//        //TODO: positioning at right of notehead could cause overlaps with other shapes
//    }
//
    //else
    if (m_pOrnament->get_ornament_type() == k_ornament_tremolo )
    {
        if (m_fAbove)
        {
            pos.y = m_pParentShape->get_bottom() - tenths_to_logical(15.0f);
            //pos.x = m_pParentShape->get_width() / 2.0 + m_pParentShape->get_left();
            pos.x = m_pParentShape->get_left();
        }
        else
        {
            pos.y = m_pParentShape->get_top() + tenths_to_logical(15.0f);
            //pos.x = m_pParentShape->get_width() / 2.0 + m_pParentShape->get_left();
            pos.x = m_pParentShape->get_left() + tenths_to_logical(11.0f);
        }
    }

    else
    {
        if (m_fAbove)
            pos.y = min(pos.y, m_pParentShape->get_top()) - tenths_to_logical(5.0f);
        else
        {
            pos.y += tenths_to_logical(40.0f);
            pos.y = max(pos.y, m_pParentShape->get_bottom()) + tenths_to_logical(5.0f);
        }
    }

	return pos;
}

//---------------------------------------------------------------------------------------
void OrnamentEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    LUnits uCenterPos;
    if (m_pParentShape->is_shape_note())
    {
		//it is a note. Center ornament on notehead shape
        GmoShapeNote* pNote = dynamic_cast<GmoShapeNote*>(m_pParentShape);
		uCenterPos = pNote->get_notehead_left() + pNote->get_notehead_width() / 2.0f;
    }
    else
    {
    	//it is not a note (normally it would be a rest).
        //Center ornament on parent shape
    	uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    }
    LUnits xShift = uCenterPos -
        (m_pOrnamentShape->get_left() + m_pOrnamentShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pOrnamentShape->shift_origin(shift);
    }

    //ensure that ornament does not collides with parent shape
    URect overlap = m_pParentShape->get_bounds();
    overlap.intersection( m_pOrnamentShape->get_bounds() );
    LUnits yShift = overlap.get_height();
    if (yShift != 0.0f)
    {
        yShift += tenths_to_logical(5.0f);
        yShift = m_fAbove ? - yShift : yShift;

        USize shift(0.0f, yShift);
        m_pOrnamentShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
void OrnamentEngraver::add_voice()
{
    if (m_pParentShape->is_shape_note() || m_pParentShape->is_shape_rest())
    {
        VoiceRelatedShape* pNR;
        if (m_pParentShape->is_shape_note())
        {
            GmoShapeNote* pNote = static_cast<GmoShapeNote*>(m_pParentShape);
            pNR = static_cast<VoiceRelatedShape*>(pNote);
        }
        else
        {
            GmoShapeRest* pRest = static_cast<GmoShapeRest*>(m_pParentShape);
            pNR = static_cast<VoiceRelatedShape*>(pRest);
        }
        VoiceRelatedShape* pVRS = static_cast<VoiceRelatedShape*>(m_pOrnamentShape);
        pVRS->set_voice(pNR->get_voice());
    }
}

//---------------------------------------------------------------------------------------
bool OrnamentEngraver::determine_if_above()
{
    if (m_placement == k_placement_above)
        return true;

    else if (m_placement == k_placement_below)
        return false;

    else    //k_placement_default
    {
        if (m_pParentShape && m_pParentShape->is_shape_note())
        {
            GmoShapeNote* pNote = dynamic_cast<GmoShapeNote*>(m_pParentShape);
            return !pNote->is_up();
        }
        else
            return true;
    }
}

//---------------------------------------------------------------------------------------
int OrnamentEngraver::find_glyph()
{
    switch( m_pOrnament->get_ornament_type() )
    {
        case k_ornament_delayed_inverted_turn:
            return k_glyph_turn_with_slash;
        case k_ornament_delayed_turn:
            return k_glyph_turn;
        case k_ornament_inverted_turn:
            return k_glyph_inverted_turn;
        case k_ornament_trill_mark:
            return k_glyph_trill;

        case k_ornament_turn:
            return k_glyph_turn;
        case k_ornament_vertical_turn:
            return k_glyph_turn_up;
        case k_ornament_mordent:
            return k_glyph_inverted_mordent;        //glyphs are inverted in SMuFL !
        case k_ornament_inverted_mordent:
            return k_glyph_mordent;
//        case k_ornament
//            return k_glyph_inverted_turn_up;
//        case k_ornament
//            return k_glyph_tremblement;
//        case k_ornament
//            return k_glyph_haydn_ornament;
        case k_ornament_schleifer:
            return k_glyph_schleifer_ornament;
        case k_ornament_wavy_line:
            return k_glyph_trill_wiggle_segment;
        case k_ornament_tremolo:
            return k_glyph_tremolo_3;
        default:
            return k_glyph_turn;        //TODO: what if no symbol?
    }
}


}  //namespace lomse
