//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_dynamics_mark_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// DynamicsMarkEngraver implementation
//---------------------------------------------------------------------------------------
DynamicsMarkEngraver::DynamicsMarkEngraver(LibraryScope& libraryScope,
                                           ScoreMeter* pScoreMeter,
                                           int UNUSED(iInstr), int UNUSED(iStaff))
    : Engraver(libraryScope, pScoreMeter)
    , m_pDynamicsMark(nullptr)
    , m_placement(k_placement_default)
    , m_fAbove(true)
    , m_pParentShape(nullptr)
    , m_pDynamicsMarkShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
GmoShapeDynamicsMark* DynamicsMarkEngraver::create_shape(ImoDynamicsMark* pDynamicsMark,
                                                         UPoint pos, Color color,
                                                         GmoShape* pParentShape)
{
    m_pDynamicsMark = pDynamicsMark;
    m_placement = pDynamicsMark->get_placement();
    m_pParentShape = pParentShape;
    m_fAbove = determine_if_above();

    int iGlyph = find_glyph();
    double fontSize = determine_font_size();
    UPoint position = compute_location(pos);
    ShapeId idx = 0;
    m_pDynamicsMarkShape =
        LOMSE_NEW GmoShapeDynamicsMark(pDynamicsMark, idx, iGlyph, position,
                                       color, m_libraryScope, fontSize);
    add_voice();

//    if (m_pDynamicsMark->get_mark_type() != k_dynamics_mark_breath_mark)
    {
        center_on_parent();
    }

    return m_pDynamicsMarkShape;
}

//---------------------------------------------------------------------------------------
UPoint DynamicsMarkEngraver::compute_location(UPoint pos)
{
    if (m_fAbove)
    {
        pos.y -= tenths_to_logical(30.0f);
    }
    else
    {
        pos.y += tenths_to_logical(80.0f);
    }
	return pos;
}

//---------------------------------------------------------------------------------------
void DynamicsMarkEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    LUnits uCenterPos;
    if (m_pParentShape->is_shape_note())
    {
		//it is a note. Center dynamics_mark on notehead shape
        GmoShapeNote* pNote = static_cast<GmoShapeNote*>(m_pParentShape);
		uCenterPos = pNote->get_notehead_left() + pNote->get_notehead_width() / 2.0f;
    }
    else
    {
    	//it is not a note (normally it would be a rest).
        //Center dynamics_mark on parent shape
    	uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    }
    LUnits xShift = uCenterPos -
        (m_pDynamicsMarkShape->get_left() + m_pDynamicsMarkShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pDynamicsMarkShape->shift_origin(shift);
    }

    //ensure that dynamics_mark does not collides with parent shape
    URect overlap = m_pParentShape->get_bounds();
    overlap.intersection( m_pDynamicsMarkShape->get_bounds() );
    LUnits yShift = overlap.get_height();
    if (yShift != 0.0f)
    {
        yShift += tenths_to_logical(5.0f);
        yShift = m_fAbove ? - yShift : yShift;

        USize shift(0.0f, yShift);
        m_pDynamicsMarkShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
void DynamicsMarkEngraver::add_voice()
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
        VoiceRelatedShape* pVRS = static_cast<VoiceRelatedShape*>(m_pDynamicsMarkShape);
        pVRS->set_voice(pNR->get_voice());
    }
}

//---------------------------------------------------------------------------------------
bool DynamicsMarkEngraver::determine_if_above()
{
    if (m_placement == k_placement_above)
        return true;

    else
        return false;
}

//---------------------------------------------------------------------------------------
int DynamicsMarkEngraver::find_glyph()
{
    string type = m_pDynamicsMark->get_mark_type();
    if (type == "p")
        return k_glyph_dynamic_p;          //Piano
    else if (type == "m")
        return k_glyph_dynamic_m;          //Mezzo
    else if (type == "f")
        return k_glyph_dynamic_f;          //Forte
    else if (type == "r")
        return k_glyph_dynamic_r;          //Rinforzando
    else if (type == "s")
        return k_glyph_dynamic_s;          //Sforzando
    else if (type == "z")
        return k_glyph_dynamic_z;          //Z
    else if (type == "n")
        return k_glyph_dynamic_n;          //Niente
    else if (type == "pppppp")
        return k_glyph_dynamic_6p;         //pppppp
    else if (type == "ppppp")
        return k_glyph_dynamic_5p;         //ppppp
    else if (type == "pppp")
        return k_glyph_dynamic_4p;         //pppp
    else if (type == "ppp")
        return k_glyph_dynamic_3p;         //ppp
    else if (type == "pp")
        return k_glyph_dynamic_2p;         //pp
    else if (type == "mp")
        return k_glyph_dynamic_mp;         //mp
    else if (type == "mf")
        return k_glyph_dynamic_mf;         //mf
    else if (type == "pf")
        return k_glyph_dynamic_pf;         //pf
    else if (type == "ff")
        return k_glyph_dynamic_2f;         //ff
    else if (type == "fff")
        return k_glyph_dynamic_3f;         //fff
    else if (type == "ffff")
        return k_glyph_dynamic_4f;         //ffff
    else if (type == "fffff")
        return k_glyph_dynamic_5f;         //fffff
    else if (type == "ffffff")
        return k_glyph_dynamic_6f;         //ffffff
    else if (type == "fp")
        return k_glyph_dynamic_fp;         //FortePiano
    else if (type == "fz")
        return k_glyph_dynamic_fz;         //Forzando
    else if (type == "sf")
        return k_glyph_dynamic_sf;         //Sforzando1
    else if (type == "sfp")
        return k_glyph_dynamic_sfp;        //SforzandoPiano
    else if (type == "sfpp")
        return k_glyph_dynamic_sfpp;       //SforzandoPianissimo
    else if (type == "sfz")
        return k_glyph_dynamic_sfz;        //Sforzato
    else if (type == "sfzp")
        return k_glyph_dynamic_sfzp;       //SforzatoPiano
    else if (type == "sffz")
        return k_glyph_dynamic_sffz;       //SforzatoFF
    else if (type == "rf")
        return k_glyph_dynamic_rf;         //Rinforzando1
    else if (type == "rfz")
        return k_glyph_dynamic_rfz;        //Rinforzando2

    stringstream s;
    s << "Dynamics string '" << type
      << "' not supported. Replaced by 'p'." << endl;
    LOMSE_LOG_ERROR(s.str());
    return k_glyph_dynamic_p;       //TODO: composite shape?
}


}  //namespace lomse
