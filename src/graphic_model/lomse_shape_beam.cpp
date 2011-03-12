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

#include "lomse_shape_beam.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"
#include "lomse_shape_note.h"


namespace lomse
{

//=======================================================================================
// GmoShapeBeam implementation
//=======================================================================================
GmoShapeBeam::GmoShapeBeam(ImoObj* pCreatorImo, LUnits uBeamThickness,
                           LUnits uBeamSpacing, LUnits m_uBeamHookLength,
                           bool fBeamAbove, Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_beam, 0, color)
    , m_uBeamThickness(uBeamThickness)
    , m_uBeamSpacing(uBeamSpacing)
    , m_uBeamHookLength(m_uBeamHookLength)
    , m_fNeedsLayout(true)
    , m_fBeamAbove(fBeamAbove)
{
}

//---------------------------------------------------------------------------------------
GmoShapeBeam::~GmoShapeBeam()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
	adjust_stems_length_if_needed();

    Color color = determine_color_to_use(opt);
    LUnits uxPrev=0, uyPrev=0, uxCur=0, uyCur=0;   // points for previous and current note
    LUnits uyShift = 0;                         // shift, to separate a beam line from the previous one

    bool fForwardPending = false;           //finish a Forward hook in prev note

    //For re-computing bounds
    m_origin.x = 100000000.0f;       //any too big value
    m_origin.y = 100000000.0f;      //any too big value
    m_size.width = 0.0f;
    m_size.height = 0.0f;

    LUnits uHalfBeam = m_uBeamThickness / 2.0f;

    //draw beam segments
    for (int iLevel=0; iLevel < 6; iLevel++)
	{
        LUnits uxStart=0, uxEnd=0, uyStart=0, uyEnd=0; // start and end points for a segment
        bool fStartPointComputed = false;
        bool fEndPointComputed = false;

        std::list<Linkable<USize>*>::iterator it;
        for(it = m_linkedTo.begin(); it != m_linkedTo.end(); ++it)
        {
            GmoShape* pNR = dynamic_cast<GmoShape*>(*it);
            if (pNR->is_shape_note())
            {
                GmoShapeNote* pShapeNote = dynamic_cast<GmoShapeNote*>(pNR);

                uxCur = pShapeNote->get_stem_left();
                uyCur = pShapeNote->get_stem_y_flag() + uyShift;

                //Let's check if we have to finish a forward hook in prev. note
                if (fForwardPending)
                {
                    uxEnd = uxPrev + m_uBeamHookLength;
                    uyEnd = uyPrev + m_uBeamHookLength*(uyCur-uyPrev)/(uxCur-uxPrev);
                    draw_beam_segment(pDrawer, uxStart, uyStart, uxEnd, uyEnd, color);
                    update_bounds(uxStart, uyStart, uxEnd, uyEnd);
                    fForwardPending = false;
                }

                // now we can deal with current note
                switch ( pShapeNote->get_beam_type(iLevel) )
                {
                    case ImoBeam::k_begin:
                        //start of segment. Compute initial point
                        fStartPointComputed = true;
                        uxStart = uxCur;
                        uyStart = uyCur;
                        break;

                    case ImoBeam::k_end:
                        // end of segment. Compute end point
                        fEndPointComputed = true;
                        uxEnd = uxCur;
                        uyEnd = uyCur;
                        break;

                    case ImoBeam::k_forward:
                        // start of segment. A forward hook is pending.
                        // compute its initial point
                        fForwardPending = true;
                        uxStart = uxCur;
                        uyStart = uyCur;
                        break;

                    case ImoBeam::k_backward:
                        // end of segment. compute start and end points
                        uxEnd = uxCur;
                        uyEnd = uyCur;
                        uxStart = uxCur - m_uBeamHookLength;
                        uyStart = uyPrev + (uxCur-uxPrev-m_uBeamHookLength)*(uyCur-uyPrev)/(uxCur-uxPrev);
                        fStartPointComputed = true;      //mark 'segment ready to be drawn'
                        fEndPointComputed = true;
                    break;

                    case ImoBeam::k_continue:
                    case ImoBeam::k_none:
                        // nothing to do.
                        break;
                }

                // if we have data to draw a segment, draw it
                if (fStartPointComputed && fEndPointComputed)
			    {
				    uxEnd += pShapeNote->get_stem_width();
                    draw_beam_segment(pDrawer,  uxStart, uyStart, uxEnd, uyEnd, color);
                    update_bounds(uxStart, uyStart, uxEnd, uyEnd);
                    fStartPointComputed = false;
                    fEndPointComputed = false;
                    if (iLevel == 0)
                    {
                        if (m_fBeamAbove)
                        {
                            m_outerLeftPoint = UPoint(uxStart, uyStart - uHalfBeam);
                            m_outerRightPoint = UPoint(uxEnd, uyEnd - uHalfBeam);
                        }
                        else
                        {
                            m_outerLeftPoint = UPoint(uxStart, uyStart + uHalfBeam);
                            m_outerRightPoint = UPoint(uxEnd, uyEnd + uHalfBeam);
                        }
                    }
                }

                // save position of current note
                uxPrev = uxCur;
                uyPrev = uyCur;
            }
        }

        // displace y coordinate for next beamline
        uyShift += (m_fBeamAbove ? m_uBeamSpacing : - m_uBeamSpacing);
    }

    //take beam thickness into accoun for boundaries
    m_origin.y -= uHalfBeam;
    m_size.height += m_uBeamThickness;

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::adjust_stems_length_if_needed()
{
	if (m_fNeedsLayout)
	{
		m_fNeedsLayout = false;
		trim_stems();
	}
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::draw_beam_segment(Drawer* pDrawer, LUnits uxStart, LUnits uyStart,
                             LUnits uxEnd, LUnits uyEnd, Color color)
{
//    //check to see if the beam segment has to be splitted in two systems
//    //if (pStartNote && pEndNote) {
//    //    lmUPoint paperPosStart = pStartNote->GetReferencePaperPos();
//    //    lmUPoint paperPosEnd = pEndNote->GetReferencePaperPos();
//    //    if (paperPosEnd.y != paperPosStart.y) {
//    //        //if start note paperPos Y is not the same than end note paperPos Y the notes are
//    //        //in different systems. Therefore, the beam must be splitted. Let's do it
//    //        wxLogMessage(_T("***** BEAM SPLIT ***"));
//    //        //TODO
//    //        //LUnits xLeft = pPaper->GetLeftMarginXPos();
//    //        //LUnits xRight = pPaper->GetRightMarginXPos();
//    //        return; //to avoid rendering bad lines across the score. It is less noticeable
//    //    }
//    //}
//
//    //draw the segment
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->line(uxStart, uyStart, uxEnd, uyEnd, m_uBeamThickness, k_edge_vertical);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::update_bounds(LUnits uxStart, LUnits uyStart,
                                 LUnits uxEnd, LUnits uyEnd)
{
    LUnits left = min(uxStart, uxEnd);
    LUnits top = min(uyStart, uyEnd);
    LUnits bottom = max(uyStart, uyEnd);
    LUnits right = max(uxStart, uxEnd);
    m_origin.x = min(m_origin.x, left);
    m_origin.y = min(m_origin.y, top);
    m_size.width = max(m_size.width, right - m_origin.x);
    m_size.height = max(m_size.height, bottom - m_origin.y);
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::set_beam_above(bool value)
{
	m_fBeamAbove = value;
	m_fNeedsLayout = true;
}

//---------------------------------------------------------------------------------------
void GmoShapeBeam::trim_stems()
{
	// In this method the length of note stems in a beamed group is adjusted.
	// It is necessary do do this whenever the x position of a note in the beam changes.

    // At this point all stems have the standard size and the stem start and end points
    // are computed (start point = nearest to notehead). In the following loop we
	// retrieve the start and end 'y' coordinates for each stem,
	// and we store them in the auxiliary arrays yNote and yFlag, respectively.

	int nNumNotes = get_num_links_to();
    std::vector<LUnits> yNote(nNumNotes);
    std::vector<LUnits> yFlag(nNumNotes);
    std::vector<GmoShapeNote*> note(nNumNotes);

	int n = nNumNotes - 1;	// index to last element
    int i = 0;              // index to current element
    LUnits x1, xn;		    // x position of first and last stems, respectively

    std::list<Linkable<USize>*>::iterator it;
    for(it = m_linkedTo.begin(); it != m_linkedTo.end(); ++it, ++i)
    {
        note[i] = NULL;
        GmoShape* pNR = dynamic_cast<GmoShape*>(*it);
        if (pNR->is_shape_note())
        {
            GmoShapeNote* pShapeNote = dynamic_cast<GmoShapeNote*>(pNR);
            note[i] = pShapeNote;
            if (i == 0)
                x1 = pShapeNote->get_stem_left();
            else if (i == n)
                xn = pShapeNote->get_stem_left();

            yNote[i] = pShapeNote->get_stem_y_note();
            yFlag[i] = pShapeNote->get_stem_y_flag();
        }
    }

	// In following loop we compute each stem and update its final position.
    // GmoShapeBeam line position is established by the first and last notes' stems. Now
    // let's adjust the intermediate notes' stem lengths to end up in the beam line.
    // This is just a proportional share based on line slope:
    // If (x1,y1) and (xn,yn) are, respectively, the position of first and last notes of
    // the group, the y position of any intermediate note i can be computed as:
    //     Ay = yn-y1
    //     Ax = xn-x1
    //                Ay
    //     yi = y1 + ---- (xi-x1)
    //                Ax
    //
    // The loop is also used to look for the shortest stem

    LUnits Ay = yFlag[n] - yFlag[0];
    LUnits Ax = xn - x1;
    LUnits uMinStem;
    for(int i=0; i < nNumNotes; i++)
    {
        if (note[i])
		{
            yFlag[i] = yFlag[0] + (Ay * (note[i]->get_stem_left() - x1)) / Ax;

            //compute stem length. For chords we have to substract the stem segment joining
            //all chord notes. This extra length is zero for notes not in chord
            LUnits uStemLength = fabs(yNote[i] - yFlag[i]) - note[i]->get_stem_extra_length();

            //save the shortest stem
            if (i==0)
                uMinStem = uStemLength;
            else
                uMinStem = min(uMinStem, uStemLength);
        }
    }

    // If the pitch of any intermediate note is out of the interval formed by
    // the first note and the last one, then its stem could be too short.
    // For example, consider a beamed group of three notes, the first and the last
    // ones D4 and the middle  one G4; the beam is horizontal, nearly the G4 line;
    // so the midle notehead (G4) would be positioned just on the beam line.
    // To avoid this problem all stems are forced to have a minimum length

    LUnits dyStem = (note[0]->get_stem_height() + note[n]->get_stem_height()) / 2.0f;
    LUnits dyMin = (2.0f * dyStem) / 3.0f;
    bool fAdjust;

    // compare the shortest with this minimun required
    LUnits uyIncr;
    if (uMinStem < dyMin)
    {
        // a stem is smaller than dyMin. Increment all stems.
        uyIncr = dyMin - uMinStem;
        fAdjust = true;
    }
    else if (uMinStem > dyStem)
    {
        // all stems are greater than the standard size. Reduce them.
        //I'm not sure if this case is passible. But it is simple to deal with it
        uyIncr = -(uMinStem - dyStem);
        fAdjust = true;
    }
    else
    {
        fAdjust = false;
    }

    if (fAdjust)
    {
        for (int i = 0; i < nNumNotes; i++)
        {
            if (yNote[i] < yFlag[i])
                yFlag[i] += uyIncr;
             else
                yFlag[i] -= uyIncr;
        }
    }

    // At this point stems' lengths are computed and adjusted.
    // Transfer the computed values to the stem shape
    for(int i=0; i < nNumNotes; i++)
    {
        note[i]->set_stem_length( fabs(yFlag[i] - yNote[i]) );
    }
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
