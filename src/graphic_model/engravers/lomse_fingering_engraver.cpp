//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_fingering_engraver.h"

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
// FingeringEngraver implementation
//---------------------------------------------------------------------------------------
FingeringEngraver::FingeringEngraver(const EngraverContext& ctx)
    : AuxObjEngraver(ctx)
    , m_pFingering(nullptr)
    , m_placement(k_placement_default)
    , m_fAbove(true)
    , m_pParentShape(nullptr)
    , m_pMainShape(nullptr)
    , m_fontSize(12.0f)         //any value, just to initialize it
{
}

//---------------------------------------------------------------------------------------
GmoShapeFingeringContainer* FingeringEngraver::create_shape(ImoFingering* pFingering,
                                                            UPoint pos, Color UNUSED(color),
                                                            GmoShape* pParentShape)
{
    m_pFingering = pFingering;
    m_placement = pFingering->get_placement();
    m_pParentShape = pParentShape;
    m_fAbove = determine_if_above();
    m_fontSize = determine_font_size();
    m_uPos = pos;

    compute_location();
    create_main_container_shape();
    add_fingering_shapes();
    center_on_parent();
    shift_shape_if_collision();
    add_voice();

    return m_pMainShape;
}

//---------------------------------------------------------------------------------------
void FingeringEngraver::create_main_container_shape()
{
    ShapeId idx = 0;
    m_pMainShape = LOMSE_NEW GmoShapeFingeringContainer(m_pFingering, idx);
}

//---------------------------------------------------------------------------------------
void FingeringEngraver::add_fingering_shapes()
{
    GmoShape* pPrevShape = nullptr;
    const list<FingerData>& fingerings = m_pFingering->get_fingerings();
    std::list<FingerData>::const_iterator it;
    for (it=fingerings.begin(); it != fingerings.end(); ++it)
    {
        const FingerData& data = *it;

        int iGlyph = find_glyph_single_char(data.value);
        if (iGlyph != -1)
        {
            if (pPrevShape && data.is_substitution())
                pPrevShape = add_substitution_shape(pPrevShape, iGlyph);

            else if (pPrevShape && data.is_alternative())
                pPrevShape = add_alternative_shape(iGlyph);

            else
                pPrevShape = add_single_char_shape(pPrevShape, iGlyph);
        }
        else
        {
            //TODO. For now, ignore not supported fingerings
            stringstream ss;
            ss << "Unsupported fingering '" << data.value << "'. Ignored.";
            LOMSE_LOG_ERROR(ss.str());
        }
    }
}

//---------------------------------------------------------------------------------------
GmoShape* FingeringEngraver::add_single_char_shape(GmoShape* pPrevShape, int iGlyph)
{
    ShapeId idx = 0;
    Color color = m_pFingering->get_color();

    LUnits xCenter = compute_position_for_glyph(pPrevShape);

    GmoShape* pShape =  LOMSE_NEW GmoShapeFingering(m_pFingering, idx, iGlyph, m_uPos,
                                                    m_libraryScope, color, m_fontSize);

    if (xCenter != 0.0f)
    {
        LUnits xShift = xCenter - (m_uPos.x + (pShape->get_width() / 2.0f));
        pShape->shift_origin(USize(xShift, 0.0f));
        m_uPos.x += xShift;
    }

    m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width();

    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* FingeringEngraver::add_substitution_shape(GmoShape* pPrevShape, int iGlyph)
{
    int arc = m_fAbove ? k_glyph_fingering_substitution_above
	                   : k_glyph_fingering_substitution_below;
    ShapeId idx = 0;
    Color color = m_pFingering->get_color();
    m_uPos.x -= (pPrevShape->get_width() / 2.0f);
    GmoShape* pShape = LOMSE_NEW GmoShapeFingering(m_pFingering, idx, arc, m_uPos,
                                                   m_libraryScope, color, m_fontSize);

    m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width() - (pPrevShape->get_width() / 2.0f);

    pShape = add_single_char_shape(nullptr, iGlyph);

    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* FingeringEngraver::add_alternative_shape(int iGlyph)
{
    ShapeId idx = 0;
    Color color = m_pFingering->get_color();
    m_uPos.x += tenths_to_logical(LOMSE_FINGERING_AFTER_SPACE);
    GmoShape* pShape =
        LOMSE_NEW GmoShapeFingering(m_pFingering, idx, k_glyph_fingering_left_parenthesis,
                                    m_uPos, m_libraryScope, color, m_fontSize);
    m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width();

    add_single_char_shape(nullptr, iGlyph);

    m_uPos.x += tenths_to_logical(LOMSE_FINGERING_AFTER_SPACE);
    pShape = LOMSE_NEW GmoShapeFingering(m_pFingering, idx, k_glyph_fingering_right_parenthesis,
                                         m_uPos, m_libraryScope, color, m_fontSize);
    m_pMainShape->add(pShape);
    m_uPos.x += pShape->get_width();

    return pShape;
}

//---------------------------------------------------------------------------------------
LUnits FingeringEngraver::compute_position_for_glyph(GmoShape* pPrevShape)
{
    if (pPrevShape)
    {
        if (is_chord())
        {
            m_uPos.x -= pPrevShape->get_width();
            LUnits yShift = pPrevShape->get_height()
                            + tenths_to_logical(LOMSE_FINGERING_VERTICAL_SPACING);
            m_uPos.y += (m_fAbove ? -yShift : yShift);
            return m_uPos.x += pPrevShape->get_width() / 2.0f;
        }
        else
        {
            m_uPos.x += tenths_to_logical(LOMSE_FINGERING_AFTER_SPACE);
            return 0.0f;
        }
    }
    else
        return 0.0f;
}

//---------------------------------------------------------------------------------------
bool FingeringEngraver::is_chord()
{
    if (!m_pParentShape->is_shape_note())
        return false;

    return (static_cast<GmoShapeNote*>(m_pParentShape))->is_chord_note();
}

//---------------------------------------------------------------------------------------
void FingeringEngraver::compute_location()
{
    if (m_fAbove)
        m_uPos.y -= tenths_to_logical(LOMSE_FINGERING_DISTANCE);
    else
        m_uPos.y += tenths_to_logical(50.0f + LOMSE_FINGERING_DISTANCE);
}

//---------------------------------------------------------------------------------------
void FingeringEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    LUnits uCenterPos;
    if (m_pParentShape->is_shape_note())
    {
		//it is a note. Center fingering on notehead shape
        GmoShapeNote* pNote = static_cast<GmoShapeNote*>(m_pParentShape);
		uCenterPos = pNote->get_notehead_left() + pNote->get_notehead_width() / 2.0f;
    }
    else
    {
    	//it is not a note (is this possible?).
        //Center fingering on parent shape
    	uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    }
    LUnits xShift = uCenterPos -
        (m_pMainShape->get_left() + m_pMainShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pMainShape->shift_origin(shift);
    }

    //ensure that fingering does not collides with parent shape
    URect overlap = m_pParentShape->get_bounds();
    overlap.intersection( m_pMainShape->get_bounds() );
    LUnits yShift = overlap.get_height();
    if (yShift != 0.0f)
    {
        yShift += tenths_to_logical(5.0f);
        yShift = m_fAbove ? - yShift : yShift;

        USize shift(0.0f, yShift);
        m_pMainShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
void FingeringEngraver::shift_shape_if_collision()
{
    if (m_fAbove)
    {
        std::pair<LUnits, GmoShape*> minValue =
                        m_pVProfile->get_min_for(m_pMainShape->get_left(),
                                                 m_pMainShape->get_right(),
                                                 m_idxStaff);
		if (minValue.second)
		{
            LUnits yBottom = m_pMainShape->get_bottom();
			if (minValue.first < yBottom)
			{
			    LUnits yShift = yBottom - minValue.first;
			    yShift += (minValue.second->is_shape_fingering()
			                    ? tenths_to_logical(LOMSE_FINGERING_VERTICAL_SPACING)
			                    : tenths_to_logical(LOMSE_FINGERING_SPACING_STACKED_ARTICULATIONS));
                m_pMainShape->shift_origin(USize(0.0f, -yShift));
			}
        }
    }
    else
    {
        std::pair<LUnits, GmoShape*> maxValue =
                        m_pVProfile->get_max_for(m_pMainShape->get_left(),
                                                 m_pMainShape->get_right(),
                                                 m_idxStaff);
        if (maxValue.second)
        {
            LUnits yTop = m_pMainShape->get_top();
			if (maxValue.first > yTop)
			{
			    LUnits yShift = maxValue.first - yTop;
			    yShift += (maxValue.second->is_shape_fingering()
			                    ? tenths_to_logical(LOMSE_FINGERING_VERTICAL_SPACING)
			                    : tenths_to_logical(LOMSE_FINGERING_SPACING_STACKED_ARTICULATIONS));
                m_pMainShape->shift_origin(USize(0.0f, yShift));
			}
        }
    }
}

//---------------------------------------------------------------------------------------
void FingeringEngraver::add_voice()
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
        VoiceRelatedShape* pVRS = static_cast<VoiceRelatedShape*>(m_pMainShape);
        pVRS->set_voice(pNR->get_voice());
    }
}

//---------------------------------------------------------------------------------------
bool FingeringEngraver::determine_if_above()
{
    if (m_placement == k_placement_above)
        return true;

    else if (m_placement == k_placement_below)
        return false;

    else    //k_placement_default
    {
        if (m_iStaff == 0)
            return true;
        else
            return false;
    }
}

//---------------------------------------------------------------------------------------
int FingeringEngraver::find_glyph_single_char(const string& value)
{
    if (value == "1")
        return k_glyph_fingering_1;
    else if (value == "2")
        return k_glyph_fingering_2;
    else if (value == "3")
        return k_glyph_fingering_3;
    else if (value == "4")
        return k_glyph_fingering_4;
    else if (value == "5")
        return k_glyph_fingering_5;
	else if (value == "6")
        return k_glyph_fingering_6;
	else if (value == "7")
        return k_glyph_fingering_7;
	else if (value == "8")
        return k_glyph_fingering_8;
	else if (value == "9")
        return k_glyph_fingering_9;
	else if (value == "0")
        return k_glyph_fingering_0;
	else if (value == "T")
        return k_glyph_fingering_T_upper;
	else if (value == "p")
        return k_glyph_fingering_P_lower;
	else if (value == "t")
        return k_glyph_fingering_T_lower;
	else if (value == "i")
        return k_glyph_fingering_I_lower;
	else if (value == "m")
        return k_glyph_fingering_M_lower;
	else if (value == "a")
        return k_glyph_fingering_A_lower;
	else if (value == "c")
        return k_glyph_fingering_C_lower;
	else if (value == "x")
        return k_glyph_fingering_X_lower;
	else if (value == "e")
        return k_glyph_fingering_E_lower;
	else if (value == "o")
        return k_glyph_fingering_O_lower;
    else
        return -1;
}


}  //namespace lomse
