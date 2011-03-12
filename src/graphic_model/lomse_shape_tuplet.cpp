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

#include "lomse_shape_tuplet.h"

#include "lomse_shape_tuplet.h"
#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"
#include "lomse_shape_beam.h"
#include "lomse_shape_note.h"


namespace lomse
{


//=======================================================================================
// GmoShapeTuplet implementation
//=======================================================================================
GmoShapeTuplet::GmoShapeTuplet(ImoObj* pCreatorImo, Color color, int design)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_tuplet, 0, color)
    , m_design(design)
    , m_fNeedsLayout(true)
    , m_pShapeText(NULL)
{
//	//compute positions and bounds
//	OnAttachmentPointMoved(pStartNR->GetShape(), lm_eGMA_StartObj, 0.0, 0.0, lmMOVE_EVENT);
//	OnAttachmentPointMoved(pEndNR->GetShape(), lm_eGMA_EndObj, 0.0, 0.0, lmMOVE_EVENT);
}

//---------------------------------------------------------------------------------------
GmoShapeTuplet::~GmoShapeTuplet()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::set_layout_data(bool fAbove, bool fDrawBracket, LUnits yStart,
                                     LUnits yEnd, LUnits uBorderLength,
                                     LUnits uBracketDistance, LUnits uLineThick,
                                     LUnits uNumberDistance, LUnits uSpaceToNumber)
{
	m_fAbove = fAbove;
	m_fDrawBracket = fDrawBracket;
	m_uBorderLength = uBorderLength;
    m_uBracketDistance = uBracketDistance;
    m_uLineThick = uLineThick;
    m_uNumberDistance = uNumberDistance;
    m_uSpaceToNumber = uSpaceToNumber;

    m_uyStart = yStart;
    m_uyEnd = yEnd;
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::add_label(GmoShapeText* pShape)
{
    m_pShapeText = pShape;
    add(pShape);
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    compute_position();
    draw_horizontal_line(pDrawer);
    draw_vertical_borders(pDrawer);

    GmoCompositeShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::compute_position()
{
    get_noterests_positions();

	//measure number
    m_uNumberWidth = m_pShapeText->get_width();
    LUnits uNumberHeight = m_pShapeText->get_height();

    //determine number x position
    m_xNumber = (m_uxStart + m_uxEnd - m_uNumberWidth)/2.0f;

    //determine number y position
    m_yNumber = (m_uyStart + m_uyEnd) / 2.0f;
    LUnits yShift = (m_fAbove ? -m_uBracketDistance : m_uBracketDistance);
    yShift += (m_fAbove ? -uNumberHeight : 0.0f);
    m_yNumber += yShift;

    //move nomber shape to its position
    m_pShapeText->set_origin(m_xNumber, m_yNumber);

    //determine y pos for start/end of bracket horizontal line
    yShift += uNumberHeight / 2.0f;
    m_yLineStart = m_uyStart + yShift;
    m_yLineEnd = m_uyEnd + yShift;

    if (m_fAbove)
    {
        m_yStartBorder = m_yLineStart + m_uBorderLength;
        m_yEndBorder = m_yLineEnd + m_uBorderLength;
    }
    else
    {
        m_yStartBorder = m_yLineStart - m_uBorderLength;
        m_yEndBorder = m_yLineEnd - m_uBorderLength;
    }

    compute_bounds();
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::compute_bounds()
{
    if (m_fDrawBracket)
    {
        m_origin.x = m_uxStart;
        m_size.width = m_uxEnd - m_uxStart;

        if (m_fAbove)
            m_origin.y = min(m_uyStart, min(m_uyEnd, m_yNumber));
        else
            m_origin.y = min(m_yEndBorder, min(m_yStartBorder, m_yNumber));

        m_size.height = m_pShapeText->get_height();
    }
    else
    {
        m_origin = m_pShapeText->get_origin();
        m_size = m_pShapeText->get_size();
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::get_noterests_positions()
{
	if (!m_fNeedsLayout)
        return;

    m_fNeedsLayout = false;
    GmoShape* pStart = dynamic_cast<GmoShape*>(m_linkedTo.front());
    GmoShape* pEnd = dynamic_cast<GmoShape*>(m_linkedTo.back());

    bool fUp = true;
    if (pStart->is_shape_note())
    {
        GmoShapeNote* pNote = dynamic_cast<GmoShapeNote*>(pStart);
        fUp = pNote->is_up();
    }

    //determine x start/end coordinates
    if (m_fDrawBracket)
    {
        m_uxStart = pStart->get_left();
        m_uxEnd = pEnd->get_right();
    }
    else
    {
        if (fUp)
        {
            m_uxStart = pStart->get_right();
            m_uxEnd = pEnd->get_right();
        }
        else
        {
            m_uxStart = pStart->get_left();
            m_uxEnd = pEnd->get_left();
        }
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::draw_horizontal_line(Drawer* pDrawer)
{
    if (m_fDrawBracket)
    {
        pDrawer->begin_path();
        pDrawer->fill(m_color);
        if (m_pShapeText)
        {
            //broken line
            float rTanAlpha = (m_yLineEnd - m_yLineStart) / (m_uxEnd - m_uxStart);
            LUnits x1 = m_xNumber - m_uSpaceToNumber;
            LUnits y1 = m_yLineStart + (x1 - m_uxStart) * rTanAlpha;
            LUnits x2 = m_xNumber + m_uNumberWidth + m_uSpaceToNumber;
            LUnits y2 = m_yLineStart + (x2 - m_uxStart) * rTanAlpha;
            pDrawer->line(m_uxStart, m_yLineStart, x1, y1, m_uLineThick, k_edge_vertical);
            pDrawer->line(x2, y2, m_uxEnd, m_yLineEnd, m_uLineThick, k_edge_vertical);
        }
        else
        {
            //full line
            pDrawer->line(m_uxStart, m_yLineStart, m_uxEnd, m_yLineEnd, m_uLineThick, k_edge_vertical);
        }
        pDrawer->end_path();
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::draw_vertical_borders(Drawer* pDrawer)
{
    if (m_fDrawBracket)
    {
        pDrawer->begin_path();
        pDrawer->fill(m_color);
        LUnits x1 = m_uxStart + m_uLineThick / 2;
        LUnits x2 = m_uxEnd - m_uLineThick / 2;
        pDrawer->line(x1, m_yLineStart, x1, m_yStartBorder, m_uLineThick, k_edge_horizontal);
        pDrawer->line(x2, m_yLineEnd, x2, m_yEndBorder, m_uLineThick, k_edge_horizontal);
        pDrawer->end_path();
    }
}

////---------------------------------------------------------------------------------------
//void GmoShapeTuplet::OnAttachmentPointMoved(GmoShape* pShape, lmEAttachType nTag,
//										   LUnits ux, LUnits uy, lmEParentEvent nEvent)
//{
//	WXUNUSED(ux);
//	WXUNUSED(uy);
//	WXUNUSED(nEvent);
//
//	//if intermediate note moved, nothing to do
//	if (!(nTag == lm_eGMA_StartObj || nTag == lm_eGMA_EndObj)) return;
//
//	//computhe half notehead width
//	GmoShapeNote* pNoteShape = (GmoShapeNote*)pShape;
//	GmoShape* pSNH = pNoteShape->GetNoteHead();
//	wxASSERT(pSNH);
//	LUnits uHalfNH = (pSNH->GetXRight() - pSNH->GetXLeft()) / 2.0;
//
//	if (nTag == lm_eGMA_StartObj)
//	{
//		//start note moved. Recompute start of shape
//		//Placed on center of notehead if above, or half notehead before if below
//		GmoShapeStem* pStem = pNoteShape->GetStem();
//		if (m_fAbove)
//		{
//			m_uxStart = pSNH->GetXLeft() + uHalfNH;
//			if (pStem)
//				m_uyStart = pStem->GetYEndStem();
//			else
//				m_uyStart = pShape->GetYTop();
//		}
//		else
//		{
//			m_uxStart = pSNH->GetXLeft() - uHalfNH;
//			if (pStem)
//				m_uyStart = pStem->GetYEndStem();
//			else
//				m_uyStart = pShape->GetYBottom();
//		}
//	}
//
//	else if (nTag == lm_eGMA_EndObj)
//	{
//		//end note moved. Recompute end of shape
//		//Placed half notehead appart if above, or on center of notehead if below
//		GmoShapeStem* pStem = pNoteShape->GetStem();
//		if (m_fAbove)
//		{
//			m_uxEnd = pSNH->GetXRight() + uHalfNH;
//			if (pStem)
//				m_uyEnd = pStem->GetYEndStem();
//			else
//				m_uyEnd = pShape->GetYTop();
//		}
//		else
//		{
//			m_uxEnd = pSNH->GetXRight() - uHalfNH;
//			if (pStem)
//				m_uyEnd = pStem->GetYEndStem();
//			else
//				m_uyEnd = pShape->GetYBottom();
//		}
//	}
//
//    // Recompute boundling rectangle
//
//	LUnits m_uBorderLength = ((lmStaffObj*)m_pOwner)->TenthsToLogical(10.0);
//    LUnits m_uBracketDistance = ((lmStaffObj*)m_pOwner)->TenthsToLogical(10.0);
//
//	LUnits m_yLineStart;
//    LUnits m_yLineEnd;
//    LUnits m_yStartBorder;
//    LUnits m_yEndBorder;
//
//    if (m_fAbove) {
//        m_yLineStart = m_uyStart - m_uBracketDistance;
//        m_yLineEnd = m_uyEnd - m_uBracketDistance;
//        m_yStartBorder = m_yLineStart + m_uBorderLength;
//        m_yEndBorder = m_yLineEnd + m_uBorderLength;
//    } else {
//        m_yLineStart = m_uyStart + m_uBracketDistance;
//        m_yLineEnd = m_uyEnd + m_uBracketDistance;
//        m_yStartBorder = m_yLineStart - m_uBorderLength;
//        m_yEndBorder = m_yLineEnd - m_uBorderLength;
//    }
//
//    //TODO:
//    // Above code is duplicated in method Render(). Share it !!!
//    //
//    // Center of control points are in (m_uxStart, m_yStartBorder) (m_uxStart, m_yLineStart)
//    // (m_uxEnd, m_yLineEnd) and (m_uxEnd, m_yEndBorder)
//
//	SetXLeft(m_uxStart);
//	SetXRight(m_uxEnd);
//	SetYTop( wxMin( wxMin(m_yLineStart, m_yLineEnd), wxMin(m_yStartBorder, m_yEndBorder)) );
//	SetYBottom( wxMax( wxMax(m_yLineStart, m_yLineEnd), wxMax(m_yStartBorder, m_yEndBorder)) );
//
//    NormaliceBoundsRectangle();
//}

////---------------------------------------------------------------------------------------
//void GmoShapeTuplet::DrawControlPoints(lmPaper* pPaper)
//{
//    //DBG
//    DrawBounds(pPaper, *wxGREEN);
//}





}  //namespace lomse
