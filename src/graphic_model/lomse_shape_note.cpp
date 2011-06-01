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

#include "lomse_shape_note.h"

#include "lomse_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"


namespace lomse
{

//=======================================================================================
// GmoShapeNote implementation
//=======================================================================================
GmoShapeNote::GmoShapeNote(ImoObj* pCreatorImo, LUnits x, LUnits y, Color color,
                           LibraryScope& libraryScope)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_note, 0, color)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_libraryScope(libraryScope)
    , m_pNoteheadShape(NULL)
	, m_pStemShape(NULL)
    , m_pAccidentalsShape(NULL)
    , m_pFlagShape(NULL)
    , m_uAnchorOffset(0.0f)
{
}

//---------------------------------------------------------------------------------------
GmoShapeNote::~GmoShapeNote()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    ////DBG: Draw anchor line
    //pDrawer->begin_path();
    //pDrawer->fill(Color(0, 0, 0, 0));
    //pDrawer->stroke(Color(255, 0, 255));
    //pDrawer->stroke_width(15.0);
    //pDrawer->move_to(m_origin.x - get_anchor_offset(), m_origin.y);
    //pDrawer->vline_to(m_origin.y + m_size.height);
    //pDrawer->end_path();

    draw_leger_lines(pDrawer);
    GmoCompositeShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::draw_leger_lines(Drawer* pDrawer)
{
    //if note is on staff, nothing to draw
    if (m_nPosOnStaff > 0 && m_nPosOnStaff < 12)
        return;

    pDrawer->begin_path();
    pDrawer->fill(Color(0, 0, 0, 0));
    pDrawer->stroke(Color(0, 0, 0));
    pDrawer->stroke_width(m_uLineThickness);

    LUnits xPos = get_notehead_left() - m_uLineOutgoing;
    LUnits lineLength = get_notehead_width() + 2.0f * m_uLineOutgoing;

    if (m_nPosOnStaff > 11)     //lines at top
	{
        LUnits yPos = m_uyStaffTopLine + get_notehead_top() - m_lineSpacing;
        for (int i=12; i <= m_nPosOnStaff; i+=2)
        {
            pDrawer->move_to(xPos, yPos);
            pDrawer->hline_to(xPos + lineLength);
            yPos -= m_lineSpacing;
        }
    }
	else    //lines at bottom
	{
        LUnits yPos = m_uyStaffTopLine + get_notehead_top() + m_lineSpacing * 5.0f;
        for (int i=0; i >= m_nPosOnStaff; i-=2)
        {
            pDrawer->move_to(xPos, yPos);
            pDrawer->hline_to(xPos + lineLength);
            yPos += m_lineSpacing;
        }
    }

    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_stem(GmoShapeStem* pShape)
{
	add(pShape);
	m_pStemShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_notehead(GmoShapeNotehead* pShape)
{
	add(pShape);
	m_pNoteheadShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_flag(GmoShapeFlag* pShape)
{
	add(pShape);
    m_pFlagShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_accidentals(GmoShapeAccidentals* pShape)
{
	add(pShape);
    m_pAccidentalsShape = pShape;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_leger_lines_info(int posOnStaff, LUnits yStaffTopLine,
                                        LUnits lineOutgoing, LUnits lineThickness,
                                        LUnits lineSpacing)
{
	m_nPosOnStaff = posOnStaff;
	m_uyStaffTopLine = yStaffTopLine;   //relative to notehead top
    m_uLineOutgoing = lineOutgoing;
    m_uLineThickness = lineThickness;
    m_lineSpacing = lineSpacing;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::add_note_in_block(GmoShape* pShape)
{
	add(pShape);
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::set_stem_down(bool down)
{
    set_up_oriented(!down);

    if (!m_pStemShape)
        return;

    if (down && !m_pStemShape->is_stem_down())
    {
        LUnits xLeft = get_notehead_left();
        LUnits yNote = get_notehead_bottom() - m_pStemShape->get_y_note()
                       + get_notehead_top();
        m_pStemShape->set_stem_down(xLeft, yNote);
    }
    else if (!down && m_pStemShape->is_stem_down())
    {
        LUnits xRight = get_notehead_right();
        LUnits yNote = get_notehead_bottom() - m_pStemShape->get_y_note()
                       + get_notehead_top();
        m_pStemShape->set_stem_up(xRight, yNote);
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::set_stem_length(LUnits length)
{
    if (m_pStemShape)
        m_pStemShape->change_length(length);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_width() const
{
    return m_pNoteheadShape->get_width();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_left() const
{
    return m_pNoteheadShape->get_left();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_right() const
{
    return m_pNoteheadShape->get_right();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_height() const
{
    return m_pNoteheadShape->get_height();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_top() const
{
    return m_pNoteheadShape->get_top();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_notehead_bottom() const
{
    return m_pNoteheadShape->get_bottom();
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_width() const
{
    return (m_pStemShape ? m_pStemShape->get_width() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_height() const
{
    return (m_pStemShape ? m_pStemShape->get_height() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_left() const
{
    return (m_pStemShape ? m_pStemShape->get_left() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_y_flag() const
{
    return (m_pStemShape ? m_pStemShape->get_y_flag() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_y_note() const
{
    return (m_pStemShape ? m_pStemShape->get_y_note() : 0.0f);
}

//---------------------------------------------------------------------------------------
LUnits GmoShapeNote::get_stem_extra_length() const
{
    return (m_pStemShape ? m_pStemShape->get_extra_length() : 0.0f);
}



//=======================================================================================
// GmoShapeNote implementation
//=======================================================================================
GmoShapeRest::GmoShapeRest(ImoObj* pCreatorImo, int idx, LUnits x, LUnits y, Color color,
                           LibraryScope& libraryScope)
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_rest, idx, color)
    , m_libraryScope(libraryScope)
	, m_pBeamShape(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoShapeRest::~GmoShapeRest()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeRest::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    GmoCompositeShape::on_draw(pDrawer, opt);
}


}  //namespace lomse
