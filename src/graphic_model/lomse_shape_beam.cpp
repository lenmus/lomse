//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_shape_beam.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"
#include "lomse_shape_note.h"
#include "lomse_beam_engraver.h"


namespace lomse
{

//=======================================================================================
// GmoShapeBeam implementation
//=======================================================================================
GmoShapeBeam::GmoShapeBeam(ImoObj* pCreatorImo, LUnits uBeamThickness, Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_beam, 0, color)
    , VoiceRelatedShape()
    , m_uBeamThickness(uBeamThickness)
    , m_BeamFlags(0)
    , m_staff(0)
{
}

//---------------------------------------------------------------------------------------
GmoShapeBeam::~GmoShapeBeam()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::set_layout_data(std::list<LUnits>& segments, UPoint origin, USize size,
                                   bool fCrossStaff, bool fChord, int beamPos, int staff)
{
    m_segments = segments;
    m_origin = origin;
    m_size = size;
    fCrossStaff ? m_BeamFlags |= k_cross_staff : m_BeamFlags &= ~k_cross_staff;
    fChord ? m_BeamFlags |= k_has_chords : m_BeamFlags &= ~k_has_chords;
    m_staff = staff;
    if (beamPos == EComputedBeam::k_beam_double_stemmed)
        m_BeamFlags |= GmoShapeBeam::k_beam_double_stemmed;
    else if (beamPos == EComputedBeam::k_beam_above)
        m_BeamFlags |= GmoShapeBeam::k_beam_above;
    else
        m_BeamFlags |= GmoShapeBeam::k_beam_below;
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);

    if (pDrawer->accepts_id_class())
        pDrawer->start_composite_notation(get_notation_id(), get_notation_class());

    std::list<LUnits>::iterator it = m_segments.begin();
    while (it != m_segments.end())
    {
        LUnits uxStart = *it + m_origin.x;
        ++it;
        LUnits uyStart = *it + m_origin.y;
        ++it;
        LUnits uxEnd = *it + m_origin.x;
        ++it;
        LUnits uyEnd = *it + m_origin.y;
        ++it;

        draw_beam_segment(pDrawer, uxStart, uyStart, uxEnd, uyEnd, color);
    }

    GmoSimpleShape::on_draw(pDrawer, opt);

    if (pDrawer->accepts_id_class())
        pDrawer->end_composite_notation();
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::draw_beam_segment(Drawer* pDrawer, LUnits uxStart, LUnits uyStart,
                             LUnits uxEnd, LUnits uyEnd, Color color)
{
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->line(uxStart, uyStart, uxEnd, uyEnd, m_uBeamThickness, k_edge_vertical);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
UPoint GmoShapeBeam::get_outer_left_reference_point()
{
    std::list<LUnits>::iterator it = m_segments.begin();
    LUnits xStart = *it + m_origin.x;
    ++it;
    LUnits yStart = *it + m_origin.y;
    if (is_beam_above())
        yStart -= m_uBeamThickness / 2.0f;
    else if (is_beam_below())
        yStart += m_uBeamThickness / 2.0f;
    return UPoint(xStart, yStart);
}

//---------------------------------------------------------------------------------------
UPoint GmoShapeBeam::get_outer_right_reference_point()
{
    std::list<LUnits>::iterator it = m_segments.begin();    //points to xStart
    ++it;   //to yStart
    ++it;
    LUnits xEnd = *it + m_origin.x;
    ++it;
    LUnits yEnd = *it + m_origin.y;
    if (is_beam_above())
        yEnd -= m_uBeamThickness / 2.0f;
    else if (is_beam_below())
        yEnd += m_uBeamThickness / 2.0f;
    return UPoint(xEnd, yEnd);
}

////---------------------------------------------------------------------------------------
//bool GmoShapeBeam::BoundsContainsPoint(lmUPoint& uPoint)
//{
//    //check if point is in beam segments
//    if (lmGMObject::BoundsContainsPoint(uPoint))
//        return true;
//
//    //check if point is in any of the stems
//    return GmoCompositeShape::BoundsContainsPoint(uPoint);
//}
//
////---------------------------------------------------------------------------------------
//bool GmoShapeBeam::HitTest(lmUPoint& uPoint)
//{
//    //check if point is in beam segments
//    if (lmGMObject::HitTest(uPoint))
//        return true;
//
//    //check if point is in any of the stems
//    return GmoCompositeShape::HitTest(uPoint);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeBeam::handle_link_event(GmoShape* pShape, int tag, USize shift,
//                                     int nEvent)
//{
//	m_fNeedsLayout = true;
//
//	//identify note moved and move its stem
//	int i = FindNoteShape((GmoShapeNote*)pShape);
//	wxASSERT(i != -1);
//	GmoShapeStem* pStem = GetStem(i);
//	wxASSERT(pStem);
//
//	pStem->Shift(shift);
//}


}  //namespace lomse
