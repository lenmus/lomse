//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include "lomse_clef_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_engraving_options.h"
#include "lomse_glyphs.h"
#include "lomse_shapes.h"
#include "lomse_box_slice_instr.h"
#include "lomse_font_storage.h"
#include "lomse_score_meter.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// ClefEngraver implementation
//---------------------------------------------------------------------------------------
ClefEngraver::ClefEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShape* ClefEngraver::create_shape(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                     UPoint uPos, int clefType, int symbolSize)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_nClefType = clefType;
    m_symbolSize = symbolSize;
    m_iGlyph = find_glyph();

    // get the shift to the staff on which the clef must be drawn
    LUnits y = uPos.y + m_pMeter->tenths_to_logical(get_glyph_offset(), iInstr, iStaff);

    double fontSize = determine_font_size();

    //create the shape object
    int nIdx = 0;   //single-shape
    GmoShape* pShape = LOMSE_NEW GmoShapeClef(pCreatorImo, nIdx, m_iGlyph, UPoint(uPos.x, y),
                                        Color(0,0,0), m_libraryScope, fontSize);
    return pShape;
}

//---------------------------------------------------------------------------------------
int ClefEngraver::find_glyph()
{
    // returns the index (over global glyphs table) to the character to use to print
    // the clef (LenMus font)

    switch (m_nClefType)
    {
        case k_clef_G2: return k_glyph_g_clef;
        case k_clef_F4: return k_glyph_f_clef;
        case k_clef_F3: return k_glyph_f_clef;
        case k_clef_C1: return k_glyph_c_clef;
        case k_clef_C2: return k_glyph_c_clef;
        case k_clef_C3: return k_glyph_c_clef;
        case k_clef_C4: return k_glyph_c_clef;
        case k_clef_C5: return k_glyph_c_clef;
        case k_clef_F5: return k_glyph_f_clef;
        case k_clef_G1: return k_glyph_g_clef;
        case k_clef_8_G2: return k_glyph_g_clef_ottava_alta;
        case k_clef_G2_8: return k_glyph_g_clef_ottava_bassa;
        case k_clef_8_F4: return k_glyph_f_clef_ottava_alta;
        case k_clef_F4_8: return k_glyph_f_clef_ottava_bassa;
        case k_clef_15_G2: return k_glyph_g_clef_quindicesima_alta;
        case k_clef_G2_15: return k_glyph_g_clef_quindicesima_bassa;
        case k_clef_15_F4: return k_glyph_f_clef_quindicesima_alta;
        case k_clef_F4_15: return k_glyph_f_clef_quindicesima_bassa;
        case k_clef_percussion: return k_glyph_percussion_clef_block;
        default:
            return k_glyph_g_clef;
    }
}

//---------------------------------------------------------------------------------------
double ClefEngraver::determine_font_size()
{
    double fontSize = 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff)
                     / 180.0;
    switch (m_symbolSize)
    {
        case k_size_cue:        return fontSize * 0.80;
        case k_size_large:      return fontSize * 1.34;
        default:                return fontSize;
    }
}

//---------------------------------------------------------------------------------------
Tenths ClefEngraver::get_glyph_offset()
{
    // returns the y-axis offset from paper cursor position so that shape get correctly
    // positioned over a five-lines staff (units: tenths of inter-line space)

    Tenths yOffset = glyphs_lmbasic2[m_iGlyph].GlyphOffset;

    //add offset to move the clef up/down the required lines
    if (m_symbolSize == k_size_cue)
    {
        switch(m_nClefType)
        {
            case k_clef_G2:     return yOffset;
            case k_clef_8_G2:   return yOffset;         //8 above
            case k_clef_G2_8:   return yOffset;         //8 below
            case k_clef_F4:     return yOffset - 7.0f;
            case k_clef_F3:     return yOffset + 3.0f;
            case k_clef_C1:     return yOffset + 16.0f;
            case k_clef_C2:     return yOffset + 6.0f;
            case k_clef_C3:     return yOffset - 4.0f;
            case k_clef_C4:     return yOffset - 14.0f;
            case k_clef_C5:     return yOffset - 24.0f;
            case k_clef_F5:     return yOffset - 17.0f;
            case k_clef_G1:     return yOffset;
            case k_clef_8_F4:   return yOffset - 7.0f;  //8 above
            case k_clef_F4_8:   return yOffset - 7.0f;  //8 below
            case k_clef_15_G2:  return yOffset;         //15 above
            case k_clef_G2_15:  return yOffset;         //15 below
            case k_clef_15_F4:  return yOffset - 7.0f;  //15 above
            case k_clef_F4_15:  return yOffset - 7.0f;  //15 below
            case k_clef_percussion:     return yOffset - 6.0f;
            default:
                return yOffset;
        }
    }
    else
    {
        switch(m_nClefType)
        {
            case k_clef_G2:     return yOffset;
            case k_clef_F4:     return yOffset;
            case k_clef_F3:     return yOffset + 10.0f;
            case k_clef_C1:     return yOffset + 20.0f;
            case k_clef_C2:     return yOffset + 10.0f;
            case k_clef_C3:     return yOffset;
            case k_clef_C4:     return yOffset - 10.0f;
            case k_clef_C5:     return yOffset - 20.0f;
            case k_clef_F5:     return yOffset - 10.0f;
            case k_clef_G1:     return yOffset + 10.0f;
            case k_clef_8_G2:   return yOffset;     //8 above
            case k_clef_G2_8:   return yOffset;     //8 below
            case k_clef_8_F4:   return yOffset;     //8 above
            case k_clef_F4_8:   return yOffset;     //8 below
            case k_clef_15_G2:  return yOffset;     //15 above
            case k_clef_G2_15:  return yOffset;     //15 below
            case k_clef_15_F4:  return yOffset;     //15 above
            case k_clef_F4_15:  return yOffset;     //15 below
            case k_clef_percussion:     return yOffset - 1.0f;
            default:
                return yOffset;
        }
    }
}

////---------------------------------------------------------------------------------------
//UPoint ClefEngraver::ComputeBestLocation(UPoint& uOrg, lmPaper* pPaper)
//{
//	// if no location is specified in LDP source file, this method is invoked from
//	// base class to ask derived object to compute a suitable position to
//	// place itself.
//	// uOrg is the assigned paper position for this object.
//
//	UPoint uPos = uOrg;
//
//	// get the shift to the staff on which the clef must be drawn
//	uPos.y += m_pVStaff->GetStaffOffset(m_nStaffNum);
//
//	return uPos;
//}
//
////---------------------------------------------------------------------------------------
//LUnits ClefEngraver::LayoutObject(GmoBox* pBox, lmPaper* pPaper, UPoint uPos, Color colorC)
//{
//    // This method is invoked by the base class (lmStaffObj). It is responsible for
//    // creating the shape object and adding it to the graphical model.
//    // Paper cursor must be used as the base for positioning.
//
//    if (lmPRESERVE_SHAPES && !IsDirty())
//    {
//        //Not dirty: just add existing shape (main shape) to the Box
//        GmoShape* pOldShape = this->GetShape(1);
//        pBox->AddShape(pOldShape, GetLayer());
//        pOldShape->SetColour(*wxCYAN);//colorC);       //change its colour to new desired colour
//
//        //set shapes index counter so that first prolog shape will have index 1
//        SetShapesIndexCounter(1);
//    }
//    else
//    {
//        //Dirty: create new shapes for this object
//
//        //if not prolog clef its size must be smaller. We know that it is a prolog clef because
//        //there is no previous context
//        bool fSmallClef = (m_pContext->GetPrev() != (lmContext*)NULL);
//
//        //create the shape object
//        GmoShape* pShape = CreateShape(pBox, pPaper, uPos, colorC, fSmallClef);
//        pShape->SetShapeLevel(lm_eMainShape);
//    }
//
//    //return total width
//	return GetShape()->GetWidth();
//}


}  //namespace lomse
