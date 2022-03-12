//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_coda_segno_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_score_meter.h"
#include "lomse_shapes.h"
#include "lomse_glyphs.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// CodaSegnoEngraver implementation
//---------------------------------------------------------------------------------------
CodaSegnoEngraver::CodaSegnoEngraver(const EngraverContext& ctx)
    : AuxObjEngraver(ctx)
    , m_pRepetitionMark(nullptr)
    , m_pParentShape(nullptr)
    , m_pCodaSegnoShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
GmoShapeCodaSegno* CodaSegnoEngraver::create_shape(ImoSymbolRepetitionMark* pRepetitionMark,
                                                   UPoint pos, Color color,
                                                   GmoShape* pParentShape)
{
    m_pRepetitionMark = pRepetitionMark;
    m_pParentShape = pParentShape;

    int iGlyph = find_glyph();
    double fontSize = determine_font_size();
    UPoint position = compute_location(pos);
    ShapeId idx = 0;
    m_pCodaSegnoShape =
        LOMSE_NEW GmoShapeCodaSegno(pRepetitionMark, idx, iGlyph, position,
                                    color, m_libraryScope, fontSize);
    center_on_parent();

    return m_pCodaSegnoShape;
}

//---------------------------------------------------------------------------------------
UPoint CodaSegnoEngraver::compute_location(UPoint pos)
{
    pos.y -= tenths_to_logical(20.0f);
	return pos;
}

//---------------------------------------------------------------------------------------
void CodaSegnoEngraver::center_on_parent()
{
    if (!m_pParentShape)
        return;

    //Center coda/segno on parent shape
    LUnits uCenterPos = m_pParentShape->get_left() + m_pParentShape->get_width() / 2.0f;
    LUnits xShift = uCenterPos -
        (m_pCodaSegnoShape->get_left() + m_pCodaSegnoShape->get_width() / 2.0f);

    if (xShift != 0.0f)
    {
        USize shift(xShift, 0.0f);
        m_pCodaSegnoShape->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
int CodaSegnoEngraver::find_glyph()
{
    switch(m_pRepetitionMark->get_symbol())
    {
        case ImoSymbolRepetitionMark::k_coda:
            return k_glyph_coda;
        case ImoSymbolRepetitionMark::k_segno:
            return k_glyph_segno;
        default:
            return k_glyph_coda;   //TODO: what to do if arrives here?
    }
}


}  //namespace lomse
