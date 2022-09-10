//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_shape_note.h"

#include "lomse_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_calligrapher.h"
#include "lomse_time.h"


namespace lomse
{

//=======================================================================================
// GmoShapeNote implementation
//=======================================================================================
GmoShapeNote::GmoShapeNote(ImoObj* pCreatorImo,
                           LUnits UNUSED(x), LUnits UNUSED(y), Color color,
                           LibraryScope& UNUSED(libraryScope))
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_note, 0, color)
    , VoiceRelatedShape()
    , m_pNoteheadShape(nullptr)
	, m_pStemShape(nullptr)
    , m_pAccidentalsShape(nullptr)
    , m_pFlagShape(nullptr)
    , m_uAnchorOffset(0.0f)
    , m_fUpOriented(true)
    , m_nPosOnStaff(1)
    , m_nTopPosOnStaff(1)
    , m_nBottomPosOnStaff(1)
    , m_uyStaffTopLine(0)
    , m_uLineOutgoing(0)
    , m_uLineThickness(0)
    , m_lineSpacing(0)
    , m_chordNoteType(k_chord_note_no)
    , m_pBaseNoteShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation(get_notation_id(), get_notation_class());

    if (opt.draw_anchor_lines)
    {
        pDrawer->begin_path();
        pDrawer->fill(Color(0, 0, 0, 0));
        pDrawer->stroke(Color(255, 0, 255));
        pDrawer->stroke_width(15.0);
        pDrawer->move_to(m_origin.x - get_anchor_offset(), m_origin.y);
        pDrawer->vline_to(m_origin.y + m_size.height);
        pDrawer->end_path();
    }

    draw_leger_lines(pDrawer);

    if (opt.draw_chords_coloured)
    {
        Color save = m_pNoteheadShape->get_normal_color();
        Color dbgColor = save;
        if (m_chordNoteType == k_chord_note_flag)
            dbgColor = Color(51,153,51);    //green
        else if (m_chordNoteType == k_chord_note_link)
            dbgColor = Color(255,105,180);  //magenta
        else if (m_chordNoteType == k_chord_note_start)
            dbgColor = Color(150,200,250);  //cyan

        m_pNoteheadShape->set_color(dbgColor);
        if (m_pStemShape)
            m_pStemShape->set_color(dbgColor);
        if (m_pAccidentalsShape)
            m_pAccidentalsShape->set_color(dbgColor);
        if (m_pFlagShape)
            m_pFlagShape->set_color(dbgColor);

        GmoCompositeShape::on_draw(pDrawer, opt);

        m_pNoteheadShape->set_color(save);
        if (m_pStemShape)
            m_pStemShape->set_color(save);
        if (m_pAccidentalsShape)
            m_pAccidentalsShape->set_color(save);
        if (m_pFlagShape)
            m_pFlagShape->set_color(save);
    }
    else
        GmoCompositeShape::on_draw(pDrawer, opt);

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::draw_leger_lines(Drawer* pDrawer)
{
    //if note is on staff, nothing to draw
    if (m_nPosOnStaff > m_nBottomPosOnStaff && m_nPosOnStaff < m_nTopPosOnStaff)
        return;

    if (pDrawer->accepts_id_class())
        pDrawer->start_simple_notation("", "ledger-line");

    pDrawer->begin_path();
    pDrawer->fill(Color(0, 0, 0, 0));
    pDrawer->stroke(Color(0, 0, 0));
    pDrawer->stroke_width(m_uLineThickness);

    LUnits xPos = get_notehead_left() - m_uLineOutgoing;
    LUnits lineLength = get_notehead_width() + 2.0f * m_uLineOutgoing;

    if (m_nPosOnStaff >= m_nTopPosOnStaff)     //lines at top
	{
        //AWARE: m_uyStaffTopLine refers to the fifth line of a five lines staff. It has
        //to be corrected when m_nTopPosOnStaff < 12 (less than 5 lines)
        LUnits yPos = m_uyStaffTopLine + get_notehead_top()
                      + m_lineSpacing * float((10 - m_nTopPosOnStaff)/2);

        for (int i=m_nTopPosOnStaff; i <= m_nPosOnStaff; i+=2)
        {
            pDrawer->move_to(xPos, yPos);
            pDrawer->hline_to(xPos + lineLength);
            yPos -= m_lineSpacing;
        }
    }
	else    //lines at bottom
	{
        //AWARE: m_uyStaffTopLine refers to the fifth line of a five lines staff. It has
        //to be corrected when m_nBottomPosOnStaff != 0 (no 5 lines)
        LUnits yPos = m_uyStaffTopLine + get_notehead_top()
                      + m_lineSpacing * float((10 - m_nBottomPosOnStaff)/2);

        for (int i=m_nBottomPosOnStaff; i >= m_nPosOnStaff; i-=2)
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
void GmoShapeNote::add_leger_lines_info(int posOnStaff, int topPosOnStaff,
                                        int bottomPosOnStaff, LUnits yStaffTopLine,
                                        LUnits lineOutgoing, LUnits lineThickness,
                                        LUnits lineSpacing)
{
	m_nPosOnStaff = posOnStaff;
    m_nTopPosOnStaff = topPosOnStaff;
    m_nBottomPosOnStaff = bottomPosOnStaff;
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

    //recompute bounding box
    recompute_bounds();
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::set_stem_length(LUnits length)
{
    if (m_pStemShape)
        m_pStemShape->change_length(length);
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::increment_stem_length(LUnits yIncrement)
{
    if (m_pStemShape)
    {
        LUnits length = m_pStemShape->get_height() + yIncrement;
        m_pStemShape->change_length(length);
    }
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
LUnits GmoShapeNote::get_stem_right() const
{
    return (m_pStemShape ? m_pStemShape->get_right() : 0.0f);
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
bool GmoShapeNote::has_beam()
{
    ImoNote* pNote = dynamic_cast<ImoNote*>(m_pCreatorImo);
    if (pNote)
        return pNote->has_beam();
    return false;
}

//---------------------------------------------------------------------------------------
bool GmoShapeNote::is_in_chord()
{
    ImoNote* pNote = dynamic_cast<ImoNote*>(m_pCreatorImo);
    if (pNote)
        return pNote->is_in_chord();
    return false;
}

//---------------------------------------------------------------------------------------
bool GmoShapeNote::is_cross_staff_chord()
{
    ImoNote* pNote = dynamic_cast<ImoNote*>(m_pCreatorImo);
    if (pNote)
        return pNote->is_cross_staff_chord();
    return false;
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::set_notehead_color(Color color)
{
    m_pNoteheadShape->set_color(color);
}

//---------------------------------------------------------------------------------------
void GmoShapeNote::dump(ostream& outStream, int level)
{
    std::ios_base::fmtflags f( outStream.flags() );  //save formating options

    outStream << setw(level*3) << level << " [" << setw(3) << m_objtype << "] "
              << "(" << get_pos_on_staff() << ")"
              << get_name(m_objtype)
              << "[" << m_idx << "]"
              << fixed << setprecision(2) << setfill(' ')
              << setw(10) << round_half_up(m_origin.x) << ", "
              << setw(10) << round_half_up(m_origin.y) << ", "
              << setw(10) << round_half_up(m_size.width) << ", "
              << setw(10) << round_half_up(m_size.height) << ", s=" << get_stem_height() << endl;

    outStream.flags( f );  //restore formating options
}


//=======================================================================================
// GmoShapeChordBaseNote implementation
//=======================================================================================
void GmoShapeChordBaseNote::set_flag_note(GmoShapeNote* pNote)
{
    m_pFlagNote = pNote;
    pNote->set_chord_note_type(k_chord_note_flag);
    //pNote->set_color( Color(255,0,0) );
}

//---------------------------------------------------------------------------------------
void GmoShapeChordBaseNote::set_link_note(GmoShapeNote* pNote)
{
    m_pLinkNote = pNote;
    pNote->set_chord_note_type(k_chord_note_link);
}

//---------------------------------------------------------------------------------------
void GmoShapeChordBaseNote::set_start_note(GmoShapeNote* pNote)
{
    m_pStartNote = pNote;
    pNote->set_chord_note_type(k_chord_note_start);
}

//---------------------------------------------------------------------------------------
GmoShapeNote* GmoShapeChordBaseNote::get_top_note()
{
    //stem up: flag note
    //stem down: link note (single staff chords) or start note (cross-staff chords)
    //no stem: find, based on position

    if (has_stem())
        return is_up() ? m_pFlagNote
                       : (m_pStartNote != nullptr ? m_pStartNote : m_pLinkNote);

    //notes without stem: decide based on noteheads position (AWARE: y axis is reversed)
    GmoShapeNote* pNonFlagNote = (m_pStartNote != nullptr ? m_pStartNote : m_pLinkNote);
    return pNonFlagNote->m_pNoteheadShape->get_top() <  m_pFlagNote->m_pNoteheadShape->get_top() ?
                    pNonFlagNote : m_pFlagNote;
}

//---------------------------------------------------------------------------------------
GmoShapeNote* GmoShapeChordBaseNote::get_bottom_note()
{
    //stem up: link note (single staff chords) or start note (cross-staff chords)
    //stem down: flag note
    //no stem: find, based on position

    if (has_stem())
        return is_up() ? (m_pStartNote != nullptr ? m_pStartNote : m_pLinkNote)
                       : m_pFlagNote;

    //notes without stem: decide based on noteheads position (AWARE: y axis is reversed)
    GmoShapeNote* pNonFlagNote = (m_pStartNote != nullptr ? m_pStartNote : m_pLinkNote);
    return pNonFlagNote->m_pNoteheadShape->get_top() <  m_pFlagNote->m_pNoteheadShape->get_top() ?
                    m_pFlagNote : pNonFlagNote;
}


//=======================================================================================
// GmoShapeRest implementation
//=======================================================================================
GmoShapeRest::GmoShapeRest(ImoObj* pCreatorImo, ShapeId idx,
                           LUnits UNUSED(x), LUnits UNUSED(y), Color color,
                           LibraryScope& UNUSED(libraryScope))
    : GmoCompositeShape(pCreatorImo, GmoObj::k_shape_rest, idx, color)
    , VoiceRelatedShape()
	, m_pBeamShape(nullptr)
{
}

//---------------------------------------------------------------------------------------
void GmoShapeRest::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation(get_notation_id(), get_notation_class());

    GmoCompositeShape::on_draw(pDrawer, opt);

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}


//=======================================================================================
// GmoShapeNotehead implementation
//=======================================================================================
void GmoShapeNotehead::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (pDrawer->accepts_id_class())
        pDrawer->start_simple_notation("", get_name());

    GmoShapeGlyph::on_draw(pDrawer, opt);
}


//=======================================================================================
// GmoShapeFret implementation
//=======================================================================================
void GmoShapeFret::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation("", "fret");

    Color color(255,255,255);
    LUnits hair = m_size.width / 8.0f;      //small gap at left and right
    LUnits x = m_origin.x - hair;
    pDrawer->fill(color);
    pDrawer->begin_path();
    pDrawer->move_to(x, m_origin.y);
    pDrawer->hline_to(x + m_size.width + hair + hair);
    pDrawer->vline_to(m_origin.y + m_size.height);
    pDrawer->hline_to(x);
    pDrawer->vline_to(m_origin.y);
    pDrawer->end_path();

    GmoShapeGlyph::on_draw(pDrawer, opt);

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}


//=======================================================================================
// GmoShapeDot implementation
//=======================================================================================
void GmoShapeDot::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (pDrawer->accepts_id_class())
        pDrawer->start_simple_notation("", get_name());

    GmoShapeGlyph::on_draw(pDrawer, opt);
}


//=======================================================================================
// GmoShapeFlag implementation
//=======================================================================================
void GmoShapeFlag::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (pDrawer->accepts_id_class())
        pDrawer->start_simple_notation("", get_name());

    GmoShapeGlyph::on_draw(pDrawer, opt);
}


//=======================================================================================
// GmoShapeRestGlyph implementation
//=======================================================================================
void GmoShapeRestGlyph::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (pDrawer->accepts_id_class())
        pDrawer->start_simple_notation("", get_name());

    GmoShapeGlyph::on_draw(pDrawer, opt);
}



}  //namespace lomse
