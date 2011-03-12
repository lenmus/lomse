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

#include "lomse_time_engraver.h"

#include "lomse_glyphs.h"
#include "lomse_shapes.h"
#include "lomse_score_meter.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// TimeEngraver implementation
//---------------------------------------------------------------------------------------
TimeEngraver::TimeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter)
    : Engraver(libraryScope, pScoreMeter)
{
}

//---------------------------------------------------------------------------------------
GmoShape* TimeEngraver::create_shape_normal(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                            UPoint uPos, int beats, int beat_type)
{
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_fontSize = determine_font_size();
    m_pCreatorImo = pCreatorImo;

    create_top_digits(uPos, beats);
    create_bottom_digits(uPos, beat_type);
    center_numbers();
    create_main_container_shape(uPos);
    add_all_shapes_to_container();

    return m_pTimeShape;
}

//---------------------------------------------------------------------------------------
void TimeEngraver::create_main_container_shape(UPoint uPos)
{
    int idx = 0;
    m_pTimeShape = new GmoShapeTimeSignature(m_pCreatorImo, idx, uPos, Color(0,0,0),
                                             m_libraryScope);
    //m_pTimeShape->SetShapeLevel(lm_eMainShape);
}

//---------------------------------------------------------------------------------------
void TimeEngraver::create_top_digits(UPoint uPos, int beats)
{
    m_uPos = uPos;
    create_digits(beats, m_pShapesTop);
    m_uTopWidth = m_uPos.x - uPos.x;
}

//---------------------------------------------------------------------------------------
void TimeEngraver::create_bottom_digits(UPoint uPos, int beat_type)
{
    m_uPos = uPos;
    m_uPos.y += m_pMeter->tenths_to_logical(20.0f, m_iInstr, m_iStaff);
    create_digits(beat_type, m_pShapesBottom);
    m_uBottomWidth = m_uPos.x - uPos.x;
}

//---------------------------------------------------------------------------------------
void TimeEngraver::create_digits(int digits, GmoShape* pShape[])
{
    if (digits > 9)
    {
        pShape[0] = create_digit(1);
        pShape[1] = create_digit(digits - 10);
    }
    else
    {
        pShape[0] = create_digit(digits);
        pShape[1] = NULL;
    }
}

//---------------------------------------------------------------------------------------
GmoShape* TimeEngraver::create_digit(int digit)
{
    int iGlyph = k_glyph_number_0 + digit;
    Tenths yOffset = glyphs_lmbasic2[iGlyph].GlyphOffset + 40.0f;
    LUnits y = m_uPos.y + m_pMeter->tenths_to_logical(yOffset, m_iInstr, m_iStaff);
    GmoShape* pShape = new GmoShapeDigit(m_pCreatorImo, 0, iGlyph, UPoint(m_uPos.x, y),
                                         Color(0,0,0), m_libraryScope, m_fontSize);
    m_uPos.x += pShape->get_width();
    return pShape;
}

//---------------------------------------------------------------------------------------
double TimeEngraver::determine_font_size()
{
    //TODO
    return 21.0 * m_pMeter->line_spacing_for_instr_staff(m_iInstr, m_iStaff) / 180.0;
}

//---------------------------------------------------------------------------------------
void TimeEngraver::center_numbers()
{
    if (m_uTopWidth > m_uBottomWidth)
    {
        //bottom number wider
        USize shift((m_uBottomWidth - m_uTopWidth) / 2.0f, 0.0f);
        m_pShapesTop[0]->shift_origin(shift);
        if (m_pShapesTop[1])
            m_pShapesTop[1]->shift_origin(shift);
    }
    else
    {
        //top number wider
        USize shift((m_uTopWidth - m_uBottomWidth) / 2.0f, 0.0f);
        m_pShapesBottom[0]->shift_origin(shift);
        if (m_pShapesBottom[1])
            m_pShapesBottom[1]->shift_origin(shift);
    }
}

//---------------------------------------------------------------------------------------
void TimeEngraver::add_all_shapes_to_container()
{
    m_pTimeShape->add(m_pShapesTop[0]);
    if (m_pShapesTop[1])
        m_pTimeShape->add(m_pShapesTop[1]);

    m_pTimeShape->add(m_pShapesBottom[0]);
    if (m_pShapesBottom[1])
        m_pTimeShape->add(m_pShapesBottom[1]);
}

//---------------------------------------------------------------------------------------
GmoShape* TimeEngraver::create_shape_common(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                            UPoint uPos)
{
    //TODO
    m_iInstr = iInstr;
    m_iStaff = iStaff;

    create_main_container_shape(uPos);

//        case eTS_Common:        // a C symbol
//        case eTS_Cut:           // a C/ symbol
//        {
//            int nGlyph = (m_nType==eTS_Common ? GLYPH_COMMON_TIME : GLYPH_CUT_TIME);
//		    LUnits uyPos = uyPosTop
//						    + m_pVStaff->TenthsToLogical(aGlyphsInfo[nGlyph].GlyphOffset, m_nStaffNum );
//            lmShape* pShape = new lmShapeGlyph(this, nShapeIdx, nGlyph, pPaper,
//                                               UPoint(uxPosTop, uyPos),
//									           _T("Time signature"), lmNO_DRAGGABLE,
//                                               color );
//		    uxPosTop += m_pVStaff->TenthsToLogical(aGlyphsInfo[nGlyph].thWidth, m_nStaffNum );
//            return pShape;
//        }
    return m_pTimeShape;
}

//---------------------------------------------------------------------------------------
GmoShape* TimeEngraver::create_shape_cut(ImoObj* pCreatorImo, int iInstr, int iStaff,
                                         UPoint uPos)
{
    //TODO
    m_iInstr = iInstr;
    m_iStaff = iStaff;

    create_main_container_shape(uPos);
    return m_pTimeShape;
}


////---------------------------------------------------------------------------------------
////CODE TO CREATE ALL TIME SIGNATURE SHAPES FOR AN INSTRUMENT
//        //Time signature is common to all lmVStaff staves of the instrument, but the lmStaffObj
//        //representing it is only present in the first staff. Therefore, for renderization, it
//        //is necessary to repeat the shape in each staff of the instrument
//        //So in the following loop we add a time signature shape for each VStaff of the
//        //instrument
//        //At this point uyPosTop is correctly positioned for first staff, so no need to
//        //add StaffDistance for first staff
//        lmStaff* pStaff = m_pVStaff->GetFirstStaff();
//        LUnits yOffset = 0;
//        for (int nStaff=1; pStaff; pStaff = m_pVStaff->GetNextStaff(), nStaff++)
//        {
//            //add top margin if not first staff
//            if (nStaff > 1)
//                yOffset += pStaff->GetStaffDistance();
//
//            //create the shape for time signature
//            lmShape* pShape = CreateShape(nStaff-1, pBox, pPaper, color, sTopGlyphs, uxPosTop,
//                                        uyPosTop + yOffset, sBottomGlyphs, uxPosBottom,
//                                        uyPosBottom + yOffset);
//            pShape->SetShapeLevel(nStaff==1 ? lm_eMainShape : lm_eSecondaryShape);
//	        pBox->AddShape(pShape, GetLayer());
//            StoreShape(pShape);
//
//            //move to next staff
//            yOffset += pStaff->GetHeight();
//        }
//
//    //return total width
//	return GetShape(1)->GetWidth();
//}


}  //namespace lomse
