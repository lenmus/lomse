//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#include "lomse_articulation_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"
#include "lomse_vertical_profile.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// ArticulationEngraver implementation
//---------------------------------------------------------------------------------------
ArticulationEngraver::ArticulationEngraver(LibraryScope& libraryScope,
                                           ScoreMeter* pScoreMeter,
                                           int UNUSED(iInstr), int UNUSED(iStaff),
                                           int idxStaff, VerticalProfile* pVProfile)
    : Engraver(libraryScope, pScoreMeter)
    , m_pArticulation(nullptr)
    , m_placement(k_placement_default)
    , m_fAbove(true)
    , m_fEnableShiftWhenCollision(false)
    , m_pParentShape(nullptr)
    , m_pArticulationShape(nullptr)
    , m_idxStaff(idxStaff)
    , m_pVProfile(pVProfile)
{
}

//---------------------------------------------------------------------------------------
GmoShapeArticulation* ArticulationEngraver::create_shape(ImoArticulation* pArticulation,
                                                         UPoint pos, Color color,
                                                         GmoShape* pParentShape)
{
    m_pArticulation = pArticulation;
    m_placement = pArticulation->get_placement();
    m_pParentShape = pParentShape;
    m_fAbove = determine_if_above();

    int iGlyph = find_glyph();
    double fontSize = determine_font_size();
    UPoint position = compute_location(pos);
    ShapeId idx = 0;
    m_pArticulationShape =
        LOMSE_NEW GmoShapeArticulation(pArticulation, idx, iGlyph, position,
                                       color, m_libraryScope, fontSize);
    add_voice();

    if (m_pArticulation->is_articulation_symbol()
        && m_pArticulation->get_articulation_type() != k_articulation_breath_mark
        && m_pArticulation->get_articulation_type() != k_articulation_caesura
       )
    {
        center_on_parent();
    }

    if (m_fEnableShiftWhenCollision)
        shift_shape_if_collision();

    return m_pArticulationShape;
}

//---------------------------------------------------------------------------------------
UPoint ArticulationEngraver::compute_location(UPoint pos)
{
    //TODO: Most shifts are based on glyph size, and this depends on font design.
    //      Instead, it is necessary to measure glyphs

    int type = m_pArticulation->get_articulation_type();
    LUnits spaceNotehead = tenths_to_logical(LOMSE_SPACING_NOTEHEAD_ARTICULATION);

    if (type == k_articulation_breath_mark)
    {
        //breath mark is displayed over staff before the note to which it is attached
        //(G.Read p.105)
        pos.x = m_pParentShape->get_right() - tenths_to_logical(25.0f);
        pos.y += tenths_to_logical(-2.0f);
        //TODO: positioning at left of notehead could cause overlaps with other shapes
    }

    else if (type == k_articulation_caesura)
    {
        //breath mark is displayed over staff, on line 4th, after the note to which
        //it is attached(G.Read p.295, G.Read p.105)
        pos.x = m_pParentShape->get_right() + tenths_to_logical(10.0f);
        pos.y += tenths_to_logical(10.0f);
        //TODO: positioning at right of notehead could cause overlaps with other shapes
    }

    else if (m_pArticulation->is_articulation_line())
    {
        pos.y = (m_pParentShape->get_top() + m_pParentShape->get_bottom() ) / 2.0f;
        if (type == k_articulation_plop || type == k_articulation_scoop)
        {
            pos.x = m_pParentShape->get_left() - tenths_to_logical(16.0f);
            if (type == k_articulation_plop)
                pos.y -= tenths_to_logical(10.0f + LOMSE_SPACING_NOTEHEAD_ARTICULATION);
            else //scoop
                pos.y += spaceNotehead;
        }
        else
        {
            pos.x = m_pParentShape->get_right() + tenths_to_logical(3.0f);
            if (type == k_articulation_doit)
                pos.y -= tenths_to_logical(10.0f + LOMSE_SPACING_NOTEHEAD_ARTICULATION);
        }
    }

    else if ( must_be_placed_outside_staff() )
    {
        m_fEnableShiftWhenCollision = true;
        if (m_fAbove)
            pos.y -= spaceNotehead;
        else
            pos.y += tenths_to_logical(40.0f + LOMSE_SPACING_NOTEHEAD_ARTICULATION);
    }

    else if (is_accent_articulation() && m_pParentShape->is_shape_note())
    {
        m_fEnableShiftWhenCollision = true;
        GmoShapeNote* pNote = static_cast<GmoShapeNote*>(m_pParentShape);
        GmoShape* pShape = pNote->get_notehead_shape();

//        //articulation must be centered of next staff space
//        LUnits shift = pNote->is_on_staff_line() ? tenths_to_logical(15.0f)
//                                                 : tenths_to_logical(5.0f);
        if (m_fAbove)
        {
            pos.y = pShape->get_top();
            pos.y -= spaceNotehead;
        }
        else
        {
            pos.y = pShape->get_bottom();
            pos.y += spaceNotehead;
        }
    }

    else
    {
        m_fEnableShiftWhenCollision = true;
        if (m_fAbove)
        {
            pos.y = m_pParentShape->get_top();
            pos.y -= spaceNotehead;
        }
        else
        {
            pos.y = m_pParentShape->get_bottom();
            pos.y += spaceNotehead;
        }
    }

	return pos;
}

//---------------------------------------------------------------------------------------
void ArticulationEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    LUnits uCenterPos;
    if (m_pParentShape->is_shape_note())
    {
		//it is a note. Center articulation on notehead shape
        GmoShapeNote* pNote = static_cast<GmoShapeNote*>(m_pParentShape);
		uCenterPos = pNote->get_notehead_left() + pNote->get_notehead_width() / 2.0f;
    }
    else
    {
    	//it is not a note (normally it would be a rest).
        //Center articulation on parent shape
    	uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    }
    LUnits xShift = uCenterPos -
        (m_pArticulationShape->get_left() + m_pArticulationShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pArticulationShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
void ArticulationEngraver::add_voice()
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
        VoiceRelatedShape* pVRS = static_cast<VoiceRelatedShape*>(m_pArticulationShape);
        pVRS->set_voice(pNR->get_voice());
    }
}

//---------------------------------------------------------------------------------------
void ArticulationEngraver::shift_shape_if_collision()
{
    LUnits yCurPos = m_pArticulationShape->get_top();
    LUnits spaceNotehead = tenths_to_logical(LOMSE_SPACING_NOTEHEAD_ARTICULATION);
    LUnits spaceOther = tenths_to_logical(LOMSE_SPACING_STACKED_ARTICULATIONS);

    if (m_fAbove)
    {
        std::pair<LUnits, GmoShape*> minValue =
                        m_pVProfile->get_min_for(m_pArticulationShape->get_left(),
                                                 m_pArticulationShape->get_right(),
                                                 m_idxStaff);
        LUnits yMin = minValue.first;
        yMin -= m_pArticulationShape->get_height();
        yMin -= (minValue.second->is_shape_notehead() ? spaceNotehead : spaceOther);
        m_pArticulationShape->set_top( min(yCurPos, yMin) );
    }
    else
    {
        std::pair<LUnits, GmoShape*> maxValue =
                        m_pVProfile->get_max_for(m_pArticulationShape->get_left(),
                                                 m_pArticulationShape->get_right(),
                                                 m_idxStaff);
        LUnits yMax = maxValue.first;
        yMax += (maxValue.second->is_shape_notehead() ? spaceNotehead : spaceOther);
        m_pArticulationShape->set_top( max(yCurPos, yMax) );
    }
}

//---------------------------------------------------------------------------------------
bool ArticulationEngraver::determine_if_above()
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
            return pNote && !pNote->is_up();
        }
        else
            return true;
    }
}

//---------------------------------------------------------------------------------------
int ArticulationEngraver::find_glyph()
{
    int type = m_pArticulation->get_articulation_type();
    switch( type )
    {

        //accents
        case k_articulation_accent:
            return (m_fAbove ? k_glyph_accent_above : k_glyph_accent_below);
        case k_articulation_marccato:
            return (m_fAbove ? k_glyph_marcato_above : k_glyph_marcato_below);
        case k_articulation_staccato:
            return (m_fAbove ? k_glyph_staccato_above : k_glyph_staccato_below);
        case k_articulation_tenuto:
            return (m_fAbove ? k_glyph_tenuto_above : k_glyph_tenuto_below);
        case k_articulation_mezzo_staccato:
            return (m_fAbove ? k_glyph_tenuto_staccato_above : k_glyph_tenuto_staccato_below);
        case k_articulation_staccatissimo:
            return (m_fAbove ? k_glyph_staccatissimo_above : k_glyph_staccatissimo_below);

        case k_articulation_legato_duro:
            return (m_fAbove ? k_glyph_marcato_tenuto_above : k_glyph_marcato_tenuto_below);
        case k_articulation_marccato_legato:
            return (m_fAbove ? k_glyph_tenuto_accent_above : k_glyph_tenuto_accent_below);
        case k_articulation_marccato_staccato:
            return (m_fAbove ? k_glyph_accent_staccato_above : k_glyph_accent_staccato_below);
        case k_articulation_staccato_duro:
            return (m_fAbove ? k_glyph_marcato_staccato_above : k_glyph_marcato_staccato_below);

        //TODO: No glyphs for these
        case k_articulation_marccato_staccatissimo:
            // symbol > with black triangle under it
        case k_articulation_mezzo_staccatissimo:
            // symbol - with black triangle under it
        case k_articulation_staccatissimo_duro:
            // symbol ^ with black triangle under it
        {
            stringstream s;
            s << "Incomplete code: No glyph for articulation " << type
              << " in ArticulationEngraver." << endl;
            LOMSE_LOG_ERROR(s.str());
            return (m_fAbove ? k_glyph_accent_above : k_glyph_accent_below);
        }

        //TODO: There are more glyphs for articulations:
//            return (m_fAbove ? k_glyph_staccatissimo_wedge_above : k_glyph_staccatissimo_wedge_below);
//            return (m_fAbove ? k_glyph_staccatissimo_stroke_above : k_glyph_staccatissimo_stroke_below);
//            return (m_fAbove ? k_glyph_stress_above : k_glyph_stress_below);
//            return (m_fAbove ? k_glyph_unstress_above : k_glyph_unstress_below);
//            return (m_fAbove ? k_glyph_laissez_vibrer_above : k_glyph_laissez_vibrer_below);

        //jazz pitch articulations
        case k_articulation_scoop:
            return k_glyph_brass_scoop;
        case k_articulation_plop:
            return k_glyph_brass_plop;
        case k_articulation_doit:
            return k_glyph_brass_doit_medium;
        case k_articulation_falloff:
            return k_glyph_brass_fall_lip_medium;

        //breath marks
        case k_articulation_breath_mark:
            switch (static_cast<ImoArticulationSymbol*>(m_pArticulation)->get_symbol())
            {
                case ImoArticulationSymbol::k_breath_tick:
                    return k_glyph_breath_mark_tick;
                case ImoArticulationSymbol::k_breath_comma:
                    return k_glyph_breath_mark_comma;
                case ImoArticulationSymbol::k_breath_v:
                    return k_glyph_breath_mark_v;
                case ImoArticulationSymbol::k_breath_salzedo:
                    return k_glyph_breath_mark_salzedo;
                default:
                    return k_glyph_breath_mark_comma;
            }

        case k_articulation_caesura:
            switch (static_cast<ImoArticulationSymbol*>(m_pArticulation)->get_symbol())
            {
                case ImoArticulationSymbol::k_caesura_normal:
                    return k_glyph_caesura;
                case ImoArticulationSymbol::k_caesura_thick:
                    return k_glyph_caesura_thick;
                case ImoArticulationSymbol::k_caesura_short:
                    return k_glyph_caesura_short;
                case ImoArticulationSymbol::k_caesura_curved:
                    return k_glyph_caesura_curved;
                default:
                    return k_glyph_caesura;
            }

        //stress accents
        case k_articulation_stress:
            return (m_fAbove ? k_glyph_stress_above : k_glyph_stress_below);
        case k_articulation_unstress:
            return (m_fAbove ? k_glyph_unstress_above : k_glyph_unstress_below);

        //other in MusicXML
        case k_articulation_spiccato:
            //The dot is an alternate sign for spiccato
            return (m_fAbove ? k_glyph_staccato_above : k_glyph_staccato_below);

        //unexpected types: code maintenance problem
        default:
            stringstream s;
            s << "Code incoherence: articulation " << type
              << " not expected in ArticulationEngraver." << endl;
            LOMSE_LOG_ERROR(s.str());
            return (m_fAbove ? k_glyph_accent_above : k_glyph_accent_below);
    }
}

//---------------------------------------------------------------------------------------
bool ArticulationEngraver::must_be_placed_outside_staff()
{
    switch( m_pArticulation->get_articulation_type() )
    {
        //a) The following signs go  always outside the staff:
        //unaccented
        //strong beat
        //strong accent
        //strong tenuto
        //accented tenuto (but the tenuto line is over the notehead)
        case k_articulation_unstress:
        case k_articulation_marccato:
        case k_articulation_breath_mark:
        case k_articulation_caesura:
            return true;

        //b) outside the staff or not, depending on stem position:
        //staccato
        //staccatissimo
        //tenuto
        //non legato
        case k_articulation_staccatissimo:
        case k_articulation_tenuto:
        case k_articulation_mezzo_staccato:
        case k_articulation_stress:
            return true;    //TODO

        //c) never outside the staff
        case k_articulation_accent:
        case k_articulation_staccato:
        case k_articulation_spiccato:
        case k_articulation_scoop:
        case k_articulation_plop:
        case k_articulation_doit:
        case k_articulation_falloff:
            return false;

        default:
            return true;
    }

}

//---------------------------------------------------------------------------------------
bool ArticulationEngraver::is_accent_articulation()
{
    int type = m_pArticulation->get_articulation_type();

    return type == k_articulation_accent
           || type == k_articulation_marccato
           || type == k_articulation_staccato
           || type == k_articulation_tenuto
           || type == k_articulation_mezzo_staccato
           || type == k_articulation_staccatissimo
           || type == k_articulation_legato_duro
           || type == k_articulation_marccato_legato
           || type == k_articulation_marccato_staccato
           || type == k_articulation_staccato_duro
           || type == k_articulation_marccato_staccatissimo
           || type == k_articulation_mezzo_staccatissimo
           || type == k_articulation_staccatissimo_duro
        //TODO There are more glyphs for articulations
           || type == k_glyph_staccatissimo_wedge_above
           || type == k_glyph_staccatissimo_stroke_above
           || type == k_glyph_stress_above
           || type == k_glyph_unstress_above
           || type == k_glyph_laissez_vibrer_above
           ;
}


}  //namespace lomse
