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
CodaSegnoEngraver::CodaSegnoEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                                     int UNUSED(iInstr), int UNUSED(iStaff))
    : Engraver(libraryScope, pScoreMeter)
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
