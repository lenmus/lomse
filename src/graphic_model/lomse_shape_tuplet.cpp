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
    , m_pShapeText(nullptr)
    , m_pStartNR(nullptr)
    , m_pEndNR(nullptr)
	, m_fAbove(true)
	, m_fDrawBracket(true)
	, m_uBorderLength(0.0f)
    , m_uBracketDistance(0.0f)
    , m_uLineThick(0.0f)
    , m_uSpaceToNumber(0.0f)
    , m_uxStart(0.0f)
    , m_uyStart(0.0f)
    , m_uxEnd(0.0f)
    , m_uyEnd(0.0f)
	, m_yLineStart(0.0f)
    , m_yLineEnd(0.0f)
    , m_yStartBorder(0.0f)
    , m_yEndBorder(0.0f)
    , m_xNumber(0.0f)
    , m_yNumber(0.0f)
    , m_uNumberWidth(0.0f)
{
}

//---------------------------------------------------------------------------------------
GmoShapeTuplet::~GmoShapeTuplet()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::set_layout_data(bool fAbove, bool fDrawBracket, LUnits yStart,
                                     LUnits yEnd, LUnits uBorderLength,
                                     LUnits uBracketDistance, LUnits uLineThick,
                                     LUnits uSpaceToNumber,
                                     GmoShape* pStart, GmoShape* pEnd)
{
	m_fAbove = fAbove;
	m_fDrawBracket = fDrawBracket;
	m_uBorderLength = uBorderLength;
    m_uBracketDistance = uBracketDistance;
    m_uLineThick = uLineThick;
    m_uSpaceToNumber = uSpaceToNumber;

    m_uyStart = yStart;
    m_uyEnd = yEnd;

    m_pStartNR = pStart;
    m_pEndNR = pEnd;

    compute_position();
    compute_bounds();
    make_points_relative_to_origin();
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
    draw_horizontal_line(pDrawer);
    draw_vertical_borders(pDrawer);

    GmoCompositeShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::compute_position()
{
    compute_horizontal_position();

    //determine position for bracket line
    LUnits yShift = (m_fAbove ? -m_uBracketDistance : m_uBracketDistance);
    yShift += (m_fAbove ? -m_uBorderLength : m_uBorderLength);
    m_yLineStart = m_uyStart + yShift;
    m_yLineEnd = m_uyEnd + yShift;

	//measure number
	m_xNumber = 0.0f;
	m_yNumber = 0.0f;
	m_uNumberWidth = 0.0f;
    if (m_pShapeText)
    {
        m_uNumberWidth = m_pShapeText->get_width();
        LUnits uNumberHeight = m_pShapeText->get_height();

        //determine number x position
        m_xNumber = ((m_uxStart + m_uxEnd - m_uNumberWidth) / 2.0f) + m_uSpaceToNumber;

        //determine number y position
        m_yNumber = (m_yLineStart + m_yLineEnd - uNumberHeight) / 2.0f;

        //move number shape to its position
        m_pShapeText->set_origin(m_xNumber, m_yNumber);
    }

    //determine y pos for start/end of bracket vertical lines
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
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::compute_bounds()
{
    if (m_fDrawBracket)
    {
        m_origin.x = m_uxStart;
        m_size.width = m_uxEnd - m_uxStart;

        if (m_pShapeText)
        {
            m_size.height = m_pShapeText->get_height();
            if (m_fAbove)
                m_origin.y = min(m_uyStart, min(m_uyEnd, m_yNumber));
            else
                m_origin.y = min(m_yEndBorder, min(m_yStartBorder, m_yNumber));
        }
        else
        {
            m_size.height = fabs(m_yLineStart - m_yLineEnd) + m_uBorderLength;
            if (m_fAbove)
                m_origin.y = min(m_yLineStart, m_yLineEnd);
            else
                m_origin.y = min(m_yEndBorder, m_yStartBorder);
        }
    }
    else
    {
        m_origin = m_pShapeText->get_origin();
        m_size = m_pShapeText->get_size();
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::make_points_relative_to_origin()
{
    //reference positions
    m_uxStart -= m_origin.x;
    m_uyStart -= m_origin.y;
    m_uxEnd -= m_origin.x;
    m_uyEnd -= m_origin.y;

	m_yLineStart -= m_origin.y;
    m_yLineEnd -= m_origin.y;
    m_yStartBorder -= m_origin.y;
    m_yEndBorder -= m_origin.y;
    m_xNumber -= m_origin.x;
    m_yNumber -= m_origin.y;
}

//---------------------------------------------------------------------------------------
void GmoShapeTuplet::compute_horizontal_position()
{
    //determine x start/end coordinates

    //AWARE: It is simpler to do this here than in the tuplets engraver because
    //when the tuplet is engraved the system is not justified and, therefore,
    //the notes will be moved.

    if (m_pStartNR->is_shape_rest())
        m_uxStart = m_pStartNR->get_left();
    else
    {
        GmoShapeNote* pStartNote = static_cast<GmoShapeNote*>(m_pStartNR);
        m_uxStart = pStartNote->get_notehead_left();
    }

    if (m_pEndNR->is_shape_rest())
        m_uxEnd = m_pEndNR->get_right();
    else
    {
        GmoShapeNote* pEndNote = static_cast<GmoShapeNote*>(m_pEndNR);
        m_uxEnd = pEndNote->get_notehead_right();
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
            LUnits x1 = m_xNumber + m_origin.x - m_uSpaceToNumber;
            LUnits y1 = m_yLineStart + m_origin.y
                        + ((x1 - m_uxStart - m_origin.x) * rTanAlpha);
            LUnits x2 = m_xNumber + m_origin.x + m_uNumberWidth + m_uSpaceToNumber;
            LUnits y2 = m_yLineStart + m_origin.y
                        + ((x2 - m_uxStart - m_origin.x) * rTanAlpha);
            pDrawer->line(m_uxStart + m_origin.x, m_yLineStart + m_origin.y,
                          x1, y1, m_uLineThick, k_edge_vertical);
            pDrawer->line(x2, y2, m_uxEnd + m_origin.x, m_yLineEnd + m_origin.y,
                          m_uLineThick, k_edge_vertical);
        }
        else
        {
            //full line
            pDrawer->line(m_uxStart + m_origin.x, m_yLineStart + m_origin.y,
                          m_uxEnd + m_origin.x, m_yLineEnd + m_origin.y,
                          m_uLineThick, k_edge_vertical);
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
        LUnits x1 = m_uxStart + m_origin.x + m_uLineThick / 2;
        LUnits x2 = m_uxEnd + m_origin.x - m_uLineThick / 2;
        pDrawer->line(x1, m_yLineStart + m_origin.y, x1, m_yStartBorder + m_origin.y,
                      m_uLineThick, k_edge_horizontal);
        pDrawer->line(x2, m_yLineEnd + m_origin.y, x2, m_yEndBorder + m_origin.y,
                      m_uLineThick, k_edge_horizontal);
        pDrawer->end_path();
    }
}


}  //namespace lomse
