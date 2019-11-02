//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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
{
}

//---------------------------------------------------------------------------------------
GmoShapeBeam::~GmoShapeBeam()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::set_layout_data(std::list<LUnits>& segments, UPoint origin,
                                   USize size, UPoint outerLeft, UPoint outerRight)
{
    m_segments = segments;
    m_origin = origin;
    m_size = size;
    m_outerLeftPoint = outerLeft;
    m_outerRightPoint = outerRight;
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);

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
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::draw_beam_segment(Drawer* pDrawer, LUnits uxStart, LUnits uyStart,
                             LUnits uxEnd, LUnits uyEnd, Color color)
{
//    //check to see if the beam segment has to be split in two systems
//    //if (pStartNote && pEndNote) {
//    //    lmUPoint paperPosStart = pStartNote->GetReferencePaperPos();
//    //    lmUPoint paperPosEnd = pEndNote->GetReferencePaperPos();
//    //    if (paperPosEnd.y != paperPosStart.y) {
//    //        //if start note paperPos Y is not the same than end note paperPos Y the notes are
//    //        //in different systems. Therefore, the beam must be splitted. Let's do it
//    //        wxLogMessage(_T("***** BEAM SPLIT ***"));
//    //        //TODO: split beam in two systems
//    //        //LUnits xLeft = pPaper->GetLeftMarginXPos();
//    //        //LUnits xRight = pPaper->GetRightMarginXPos();
//    //        return; //to avoid rendering bad lines across the score. It is less noticeable
//    //    }
//    //}

    //draw the segment
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->line(uxStart, uyStart, uxEnd, uyEnd, m_uBeamThickness, k_edge_vertical);
    pDrawer->end_path();
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
