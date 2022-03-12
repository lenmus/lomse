//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_fermata_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"
#include "lomse_glyphs.h"
#include "lomse_shape_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// FermataEngraver implementation
//---------------------------------------------------------------------------------------
FermataEngraver::FermataEngraver(const EngraverContext& ctx)
    : AuxObjEngraver(ctx)
    , m_pFermata(nullptr)
    , m_placement(k_placement_default)
    , m_fAbove(true)
    , m_pParentShape(nullptr)
    , m_pFermataShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
GmoShapeFermata* FermataEngraver::create_shape(ImoFermata* pFermata, UPoint pos,
                                               Color color, GmoShape* pParentShape)
{
    m_pFermata = pFermata;
    m_placement = pFermata->get_placement();
    m_pParentShape = pParentShape;
    m_fAbove = determine_if_above();

    int iGlyph = find_glyph();
    double fontSize = determine_font_size();
    UPoint position = compute_location(pos);
    ShapeId idx = 0;
    m_pFermataShape = LOMSE_NEW GmoShapeFermata(pFermata, idx, iGlyph, position,
                                                color, m_libraryScope, fontSize);
    add_voice();
    center_on_parent();
    return m_pFermataShape;
}

//---------------------------------------------------------------------------------------
UPoint FermataEngraver::compute_location(UPoint pos)
{
	if (m_fAbove)
		pos.y -= tenths_to_logical(5.0f);
	else
		pos.y += tenths_to_logical(45.0f);

	return pos;
}

//---------------------------------------------------------------------------------------
void FermataEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    LUnits uCenterPos;
    if (m_pParentShape->is_shape_note())
    {
		//it is a note. Center fermata on notehead shape
        GmoShapeNote* pNote = static_cast<GmoShapeNote*>(m_pParentShape);
		uCenterPos = pNote->get_notehead_left() + pNote->get_notehead_width() / 2.0f;
    }
    else
    {
    	//it is not a note (normally it would be a rest).
        //Center fermata on parent shape
    	uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    }
    LUnits xShift = uCenterPos -
                    (m_pFermataShape->get_left() + m_pFermataShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pFermataShape->shift_origin(shift);
    }

    //ensure that fermata does not collides with parent shape
    URect overlap = m_pParentShape->get_bounds();
    overlap.intersection( m_pFermataShape->get_bounds() );
    LUnits yShift = overlap.get_height();
    if (yShift != 0.0f)
    {
        yShift += tenths_to_logical(5.0f);
        yShift = m_fAbove ? - yShift : yShift;

        USize shift(0.0f, yShift);
        m_pFermataShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
void FermataEngraver::add_voice()
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
        VoiceRelatedShape* pVRS = static_cast<VoiceRelatedShape*>(m_pFermataShape);
        pVRS->set_voice(pNR->get_voice());
    }
}

//---------------------------------------------------------------------------------------
bool FermataEngraver::determine_if_above()
{
    if (m_placement == k_placement_above)
        return true;

    else if (m_placement == k_placement_below)
        return false;

    else    //k_placement_default
    {
        if (m_pParentShape && m_pParentShape->is_shape_note())
        {
            GmoShapeNote* pNote = static_cast<GmoShapeNote*>(m_pParentShape);
            return !pNote->is_up();
        }
        else
            return true;
    }
}

//---------------------------------------------------------------------------------------
int FermataEngraver::find_glyph()
{
    switch( m_pFermata->get_symbol())
    {
        case ImoFermata::k_short:
            return (m_fAbove ? k_glyph_fermata_above_angle
                             : k_glyph_fermata_below_angle);

        case ImoFermata::k_long:
            return (m_fAbove ? k_glyph_fermata_above_square
                             : k_glyph_fermata_below_square);

        case ImoFermata::k_henze_short:
            return (m_fAbove ? k_glyph_fermata_short_above
                             : k_glyph_fermata_short_below);

        case ImoFermata::k_henze_long:
            return (m_fAbove ? k_glyph_fermata_long_above
                             : k_glyph_fermata_long_below);

        case ImoFermata::k_very_short:
            return (m_fAbove ? k_glyph_fermata_very_short_above
                             : k_glyph_fermata_very_short_below);

        case ImoFermata::k_very_long:
            return (m_fAbove ? k_glyph_fermata_very_long_above
                             : k_glyph_fermata_very_long_below);

        case ImoFermata::k_normal:
        default:
            return (m_fAbove ? k_glyph_fermata_above
                             : k_glyph_fermata_below);
    }
}

}  //namespace lomse
