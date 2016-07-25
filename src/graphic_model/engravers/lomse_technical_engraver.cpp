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

#include "lomse_technical_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// TechnicalEngraver implementation
//---------------------------------------------------------------------------------------
TechnicalEngraver::TechnicalEngraver(LibraryScope& libraryScope,
                                           ScoreMeter* pScoreMeter,
                                           int UNUSED(iInstr), int UNUSED(iStaff))
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShapeTechnical* TechnicalEngraver::create_shape(ImoTechnical* pTechnical,
                                                         UPoint pos, Color color,
                                                         GmoShape* pParentShape)
{
    m_pTechnical = pTechnical;
    m_placement = pTechnical->get_placement();
    m_pParentShape = pParentShape;
    m_fAbove = determine_if_above();

    int iGlyph = find_glyph();
    double fontSize = determine_font_size();
    UPoint position = compute_location(pos);
    ShapeId idx = 0;
    m_pTechnicalShape =
        LOMSE_NEW GmoShapeTechnical(pTechnical, idx, iGlyph, position,
                                       color, m_libraryScope, fontSize);
    add_voice();

    //if (m_pTechnical->get_technical_type() != k_technical_breath_mark)
    {
        center_on_parent();
    }

    return m_pTechnicalShape;
}

//---------------------------------------------------------------------------------------
UPoint TechnicalEngraver::compute_location(UPoint pos)
{
//    //TODO: Most shifts are based on glyph size, and this depends on font design.
//    //      Instead, it is necessary to measure glyphs
//
//    int type = m_pTechnical->get_technical_type();
//
//    if (type == k_technical_breath_mark)
//    {
//        pos.x = m_pParentShape->get_right() + tenths_to_logical(10.0f);
//        pos.y += tenths_to_logical(5.0f);
//        //TODO: positioning at right of notehead could cause overlaps with other shapes
//    }
//
//    else if (m_pTechnical->is_technical_line())
//    {
//        pos.y = (m_pParentShape->get_top() + m_pParentShape->get_bottom() ) / 2.0;
//        if (type == k_technical_plop || type == k_technical_scoop)
//        {
//            pos.x = m_pParentShape->get_left() - tenths_to_logical(16.0f);
//            if (type == k_technical_plop)
//                pos.y -= tenths_to_logical(15.0f);
//            else //scoop
//                pos.y += tenths_to_logical(5.0f);
//        }
//        else
//        {
//            pos.x = m_pParentShape->get_right() + tenths_to_logical(3.0f);
//            if (type == k_technical_doit)
//                pos.y -= tenths_to_logical(15.0f);
//        }
//    }

    //else
    if ( must_be_placed_outside_staff() )
    {
        if (m_fAbove)
            pos.y -= tenths_to_logical(5.0f);
        else
            pos.y += tenths_to_logical(45.0f);
    }

    else
    {
        if (m_fAbove)
        {
            pos.y = m_pParentShape->get_top();
            pos.y -= tenths_to_logical(5.0f);
        }
        else
        {
            pos.y = m_pParentShape->get_bottom();
            pos.y += tenths_to_logical(5.0f);
        }
    }

	return pos;
}

//---------------------------------------------------------------------------------------
void TechnicalEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    LUnits uCenterPos;
    if (m_pParentShape->is_shape_note())
    {
		//it is a note. Center technical on notehead shape
        GmoShapeNote* pNote = dynamic_cast<GmoShapeNote*>(m_pParentShape);
		uCenterPos = pNote->get_notehead_left() + pNote->get_notehead_width() / 2.0f;
    }
    else
    {
    	//it is not a note (normally it would be a rest).
        //Center technical on parent shape
    	uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    }
    LUnits xShift = uCenterPos -
        (m_pTechnicalShape->get_left() + m_pTechnicalShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pTechnicalShape->shift_origin(shift);
    }

    //ensure that technical does not collides with parent shape
    URect overlap = m_pParentShape->get_bounds();
    overlap.intersection( m_pTechnicalShape->get_bounds() );
    LUnits yShift = overlap.get_height();
    if (yShift != 0.0f)
    {
        yShift += tenths_to_logical(5.0f);
        yShift = m_fAbove ? - yShift : yShift;

        USize shift(0.0f, yShift);
        m_pTechnicalShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
void TechnicalEngraver::add_voice()
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
        VoiceRelatedShape* pVRS = static_cast<VoiceRelatedShape*>(m_pTechnicalShape);
        pVRS->set_voice(pNR->get_voice());
    }
}

//---------------------------------------------------------------------------------------
bool TechnicalEngraver::determine_if_above()
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
int TechnicalEngraver::find_glyph()
{
    switch(m_pTechnical->get_technical_type())
    {
        case k_technical_up_bow:
            return k_glyph_string_up_bow;
        case k_technical_down_bow:
            return k_glyph_string_down_bow;
        case k_technical_harmonic:
            return k_glyph_string_harmonic;
//        case k_technical_fingering:
//            return
        case k_technical_double_tongue:
            return (m_fAbove ? k_glyph_double_tongue_above : k_glyph_double_tongue_below);
        case k_technical_triple_tongue:
            return (m_fAbove ? k_glyph_triple_tongue_above : k_glyph_triple_tongue_below);
        case k_technical_hole:
            return k_glyph_wind_open_hole;
        case k_technical_handbell:
            return k_glyph_bells_martellato;
        default:
            return k_glyph_wind_three_quarters_closed_hole;   //TODO: what to do if arrives here?
    }

//    k_glyph_wind_closed_hole,               // Closed hole
//    k_glyph_wind_three_quarters_close_hole, // Three-quarters closed hole
//    k_glyph_wind_half_closed_hole,          // Half-closed hole
//    k_glyph_wind_half_closed_hole_2,        // Half-closed hole 2
//    k_glyph_wind_half_open_hole,            // Half-open hole
//    k_glyph_wind_open_hole,                 // Open hole
//
//    //String techniques (U+E610 - U+E62F)
//    ,        // Down bow
//    ,          // Up bow
//    k_glyph_string_harmonic,        // Harmonic
//    k_glyph_string_half_harmonic,   // Half-harmonic
//
//    //Keyboard techniques (U+E650 - U+E67F)
//    k_glyph_pedal_mark,         // Pedal mark
//    k_glyph_pedal_up_mark,      // Pedal up mark
//    k_glyph_half_pedal_mark,    // Half-pedal mark
//
//    //Handbells (U+E810 - U+E82F)
//    k_glyph_bells_martellato,               // Martellato
//    k_glyph_bells_martellato_lift,          // Martellato lift
//    k_glyph_bells_hand_martellato,          // Hand martellato
//    k_glyph_bells_muted_martellato,         // Muted martellato
//    k_glyph_mallet_bell_suspended,          // Mallet, bell suspended
//    k_glyph_mallet_bell_on_table,           // Mallet, bell on table
//    k_glyph_mallet_lift,                    // Mallet lift
//    k_glyph_pluck_lift,                     // Pluck lift
//    k_glyph_swing_up,                       // Swing up
//    k_glyph_swing_down,                     // Swing down
//    k_glyph_swing,                          // Swing
//    k_glyph_echo,                           // Echo
//    k_glyph_echo_2,                         // Echo 2
//    k_glyph_gyro,                           // Gyro
//    k_glyph_damp_3,                         // Damp 3
//    k_glyph_belltree,                       // Belltree
//    k_glyph_table_single_handbell,          // Table single handbell
//    k_glyph_table_pair_of_handbells,        // Table pair of handbells
}

//---------------------------------------------------------------------------------------
bool TechnicalEngraver::must_be_placed_outside_staff()
{
//    switch( m_pTechnical->get_technical_type() )
//    {
//        //a) The following signs go  always outside the staff:
//        //unaccented
//        //strong beat
//        //strong accent
//        //strong tenuto
//        //accented tenuto (but the tenuto line is over the notehead)
//        case k_technical_unstress:
//        case k_technical_strong_accent:
//        case k_technical_breath_mark:
//        case k_technical_caesura:
//            return true;
//
//        //b) outside the staff or not, depending on stem position:
//        //staccato
//        //staccatissimo
//        //tenuto
//        //non legato
//        case k_technical_staccato:
//        case k_technical_staccatissimo:
//        case k_technical_tenuto:
//        case k_technical_tenuto_staccato:
//        case k_technical_stress:
//            return true;    //TODO
//
//        //c) never outside the staff
//        case k_technical_accent:
//        case k_technical_spiccato:
//        case k_technical_scoop:
//        case k_technical_plop:
//        case k_technical_doit:
//        case k_technical_falloff:
//            return false;
//
//        default:
//            return true;
//    }
    return true;
}


}  //namespace lomse
