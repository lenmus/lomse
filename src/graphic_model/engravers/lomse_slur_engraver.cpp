//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#include "lomse_slur_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_tie.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_instrument_engraver.h"
#include "lomse_vertical_profile.h"


#include <vector>
using namespace std;


namespace lomse
{

//---------------------------------------------------------------------------------------
// SlurEngraver implementation
//---------------------------------------------------------------------------------------
SlurEngraver::SlurEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
{
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_start_staffobj(ImoRelObj* pRO, ImoStaffObj* pSO,
                                      GmoShape* pStaffObjShape, int iInstr, int iStaff,
                                      int UNUSED(iSystem), int UNUSED(iCol), LUnits UNUSED(xStaffLeft),
                                      LUnits UNUSED(xStaffRight), LUnits yTop,
                                      int idxStaff, VerticalProfile* pVProfile)
{
    //data stored in base class
    m_iInstr = iInstr;
    m_iStaff = iStaff;
    m_idxStaff = idxStaff;
    m_idxStaffStart = idxStaff;
    m_pVProfile = pVProfile;

    //data specific for this class
    m_pSlur = static_cast<ImoSlur*>(pRO);

    m_pStartNote = static_cast<ImoNote*>(pSO);
    m_pStartNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffTop = yTop;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_end_staffobj(ImoRelObj* UNUSED(pRO), ImoStaffObj* pSO,
                                    GmoShape* pStaffObjShape, int UNUSED(iInstr),
                                    int UNUSED(iStaff), int UNUSED(iSystem), int UNUSED(iCol),
                                    LUnits UNUSED(xStaffLeft), LUnits UNUSED(xStaffRight),
                                    LUnits yTop, int idxStaff, VerticalProfile* pVProfile)
{
    m_pEndNote = static_cast<ImoNote*>(pSO);
    m_pEndNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffTop = yTop;

    m_fSlurForGraces = m_pStartNote->is_grace_note() || m_pEndNote->is_grace_note();

    m_idxStaff = idxStaff;
    m_idxStaffEnd = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::decide_placement()
{
    if (m_pSlur->get_orientation() != k_orientation_default)
    {
        m_fSlurBelow = (m_pSlur->get_orientation() == k_orientation_under);
    }
    else
    {
        //TODO: To decide placement only start and end notes are considered. This is
        //      wrong. It is necessary to consider all notes

        //[Read, p.266] placement: at notehead side when all notes in same direction.
        //When mixed directions, preferred above. Except when grace notes: at grace notehead
        //side

        //Rule  : When all stems in the same direction start-end points are placed near
        //to noteheads

        bool fMixed = is_end_point_set() &&
                      ( (m_pStartNoteShape->is_up() && !m_pEndNoteShape->is_up())
                         || (!m_pStartNoteShape->is_up() && m_pEndNoteShape->is_up()) );

        if (fMixed && !m_fSlurForGraces)
            m_fSlurBelow = false;
        else
            m_fSlurBelow = m_pStartNoteShape->is_up() ? true : false;
    }

    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_first_or_intermediate_shape(LUnits UNUSED(xStaffLeft),
                                    LUnits UNUSED(xStaffRight), LUnits yStaffTop,
                                    LUnits prologWidth, VerticalProfile* pVProfile,
                                    Color color)
{
    m_color = color;
    m_uStaffTop = yStaffTop;
    m_pVProfile = pVProfile;
    m_uPrologWidth = prologWidth;

    if (m_numShapes == 0)
    {
        decide_placement();
        return create_shape(k_system_end);      //first shape segment, starting the slur
    }
    else
        return create_shape(k_system_full);     //continuation shape, spanning the whole system
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_last_shape(Color color)
{
    m_color = color;
    if (m_numShapes == 0)
    {
        decide_placement();
        return create_shape(k_single_shape);    //single shape
    }
    return create_shape(k_system_start);        //last shape finishing the slur
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_shape(int type)
{
    compute_start_end_points(type);
    apply_rules_for_start_end_points(type);
    find_contour_reference_points();
    compute_baseline();
    compute_control_points(type);
    //add_user_displacements(0, &m_points[0]);

    ++m_numShapes;
    GmoShapeSlur* pShape = LOMSE_NEW GmoShapeSlur(m_pSlur, 0, &m_points[0],
                                                  m_thickness, m_color);

    pShape->add_data_points(m_dataPoints, m_dbgPeak, m_dbgColor);  //only for debug


    //dbg: add approximate circle to the shape, to display for debug
    if (type == k_single_shape)
        pShape->add_approx_arc(xc, yc, r);  //only for debug


    //adding slurs to VProfile is problematic as very height slurs or slurs with
    //big baseline angle takes unreasonable space. Tentatively, slurs with height
    //greater than 4 lines will not be added.
    if (pShape->get_bounds().get_height() > tenths_to_logical(40.0f))
        pShape->set_add_to_vprofile(false);

    return pShape;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_start_end_points(int type)
{
    //determine initial start/end points for the slur. They could be later moved
    //up/down for better layout

    switch (type)
    {
        case k_system_start:
            //slur starts at system start. It is the last shape when more than one
            compute_start_of_staff_point();
            compute_end_point();
            break;

        case k_system_end:
            //slur end at systme right. It is the first shape when the slur does not
            //finish in current system
            compute_start_point();
            compute_end_of_staff_point();
            break;

        case k_system_full:
            //intermediate shape spanning the whole system
            compute_start_of_staff_point();
            compute_end_of_staff_point();
            break;

        case k_single_shape:
            //the only shape when start and end notes are in the same system
            compute_start_point();
            compute_end_point();
            break;
    }
}

//---------------------------------------------------------------------------------------
void SlurEngraver::apply_rules_for_start_end_points(int type)
{
    //apply layout rules to start/end points, only for single shape slurs

    if (type != k_single_shape)
        return;

    //Rule: When outer notes have opposite stems directions, the base line is
    //horizontal placed on notehead and the slur attachment point moved to the
    //left/right of the stem.
    //It is equivalent to this rule:
    //Rule: When the slur tilt contrary to the outer notes pitches, move the slur at the
    //      stem end towards the noteheads, unless the note is beamed

    //determine angle of baseline: y = m.x + b
    LUnits m = (y3-y0) / (x3-x0);

    //determine angle of line between start/end noteheads
    LUnits mn = (m_pEndNoteShape->get_notehead_top() - m_pStartNoteShape->get_notehead_top())
                / (m_pEndNoteShape->get_notehead_left() - m_pStartNoteShape->get_notehead_left());

    //if angles have different sign the slur goes in bad direction. The only possibility
    //for this scenario is that one of the start/end points is attached to a stem and
    //the other one to a note. In this cases, prefer horizontal slur, unless the
    //stem attached point is a beamed note.
    if (!is_equal_float(m, 0.0f) && ((m > 0.0f && mn < 0.0f) || (m < 0.0f && mn > 0.0f)) )
    {
        if (m_fStartOverStem && !m_pStartNote->is_beamed())
        {
            //end point is below (remeber: y axis is reversed)
            //move down start point (stem) and shift it to right of stem
            y0 = y3;
            x0 = m_pStartNoteShape->get_stem_right() + tenths_to_logical(5.0f);
        }
       else if (m_fEndOverStem && !m_pEndNote->is_beamed())
        {
            //start point is below (remeber: y axis is reversed)
            //move down end point (stem) and shift it to left of stem
            y3 = y0;
            x3 = m_pEndNoteShape->get_stem_left() - tenths_to_logical(5.0f);
        }
        else
        {
            //in chords, this scenario is possible: slur over noteheads but chord base
            //notes line has oppoiste angle.
            LOMSE_LOG_ERROR("Undefined case. How is this possible?");
        }
    }

}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_baseline()
{
    uD = sqrt((x3-x0)*(x3-x0) + (y3-y0)*(y3-y0));
    D = uD / tenths_to_logical(1.0f);
    cosb = (x3-x0)/uD;
    sinb = (y3-y0)/uD;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_start_point()
{
    //if start note has stem and stem is on the same position than slur, place slur
    //over stem. Otherwise, place slur over note-head
    m_fStartOverStem = false;
    if (m_pStartNote->has_stem())
    {
        m_fStartOverStem = (m_fSlurBelow && !m_pStartNoteShape->is_up())
                           || (!m_fSlurBelow && m_pStartNoteShape->is_up());
    }

    //for chords, the received start note is the base note, but computations
    //it is necessary to use the top or the bottom note, not the base note
    GmoShapeNote* pNoteShape = get_relevant_note_for_chords(m_pStartNoteShape);

    //x pos: centered on the stem or on the nothead
    if (m_fStartOverStem)
    {
        //x pos: center on stem
        x0 = (m_fSlurBelow ? pNoteShape->get_notehead_left()
                           : pNoteShape->get_notehead_right());
    }
    else
    {
        //x pos: center on notehead
        x0 = (pNoteShape->get_notehead_right()
              + pNoteShape->get_notehead_left()) / 2.0f;
    }

    //y pos: 5 tenths appart from notehead or from stem
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
    if (m_fStartOverStem)
    {
        if (m_pStartNote->is_beamed())
        {
            y0 = (m_fSlurBelow ? pNoteShape->get_stem_y_flag() + space
                               : pNoteShape->get_stem_y_flag() - space );
        }
        else
            y0 = (m_fSlurBelow ? pNoteShape->get_bottom() + space
                               : pNoteShape->get_top() - space );
    }
    else
        y0 = (m_fSlurBelow ? pNoteShape->get_notehead_bottom() + space
                           : pNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_point()
{
    if (m_fSlurForGraces)
    {
        compute_end_point_for_graces();
        return;
    }

    //if end note has stem and stem is on the same position than slur, place slur
    //over stem. Otherwise, place slur over note-head
    m_fEndOverStem = false;
    if (m_pEndNote->has_stem())
    {
        m_fEndOverStem = (m_fSlurBelow && !m_pEndNoteShape->is_up())
                         || (!m_fSlurBelow && m_pEndNoteShape->is_up());
    }

    //for chords, the received end note is the base note, but computations
    //it is necessary to use the top or the bottom note, not the base note
    GmoShapeNote* pNoteShape = get_relevant_note_for_chords(m_pEndNoteShape);

    //x pos: centered on the stem or on the nothead
    if (m_fEndOverStem)
    {
        //x pos: center on stem
        x3 = (m_fSlurBelow ? pNoteShape->get_notehead_left()
                           : pNoteShape->get_notehead_right());
    }
    else
    {
        //x pos: center on notehead
        x3 = (pNoteShape->get_notehead_right()
              + pNoteShape->get_notehead_left()) / 2.0f;
    }

    //y pos: 5 tenths appart from notehead or from stem
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
    if (m_fEndOverStem)
    {
        if (m_pEndNote->is_beamed())
        {
            y3 = (m_fSlurBelow ? pNoteShape->get_stem_y_flag() + space
                               : pNoteShape->get_stem_y_flag() - space );
        }
        else
            y3 = (m_fSlurBelow ? pNoteShape->get_bottom() + space
                               : pNoteShape->get_top() - space );
    }
    else
    {
        y3 = (m_fSlurBelow ? pNoteShape->get_notehead_bottom() + space
                           : pNoteShape->get_notehead_top() - space );
    }
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_point_for_graces()
{
    m_fEndOverStem = false;

    //for chords, the received end note is the base note, but computations
    //it is necessary to use the top or the bottom note, not the base note
    GmoShapeNote* pNoteShape = get_relevant_note_for_chords(m_pEndNoteShape);


    //x pos: when the end note has stem, the slur position has to be
    //moved to the left of the notehead. Otherwise, centered on the on the nothead
    bool fLeftCenter = (m_pEndNote->has_stem()
                        && ((m_fSlurBelow && !m_pEndNoteShape->is_up())
                             || (!m_fSlurBelow && m_pEndNoteShape->is_up()) ));
    if (fLeftCenter)
    {
        //x pos: at left of notehead, centered
        x3 = pNoteShape->get_notehead_left() - tenths_to_logical(3.0f);
    }
    else
    {
        //x pos: center on notehead
        x3 = (pNoteShape->get_notehead_right() + pNoteShape->get_notehead_left()) / 2.0f;
    }

    //y pos
    if (fLeftCenter)
    {
        //y pos: aligned with notehead top/bottom
        y3 = (m_fSlurBelow ? pNoteShape->get_notehead_bottom()
                           : pNoteShape->get_notehead_top());
    }
    else
    {
        //y pos: 5 tenths appart from notehead
        LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
        y3 = (m_fSlurBelow ? pNoteShape->get_notehead_bottom() + space
                           : pNoteShape->get_notehead_top() - space );
    }
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_start_of_staff_point()
{
    x0 = m_pInstrEngrv->get_staves_left() + m_uPrologWidth - tenths_to_logical(10.0f);

    if (m_fSlurBelow)
        y0 = m_uStaffTop + tenths_to_logical(60.0f);
    else
        y0 = m_uStaffTop - tenths_to_logical(20.0f);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_of_staff_point()
{
    x3 = m_pInstrEngrv->get_staves_right();

    //if current instrument has more than one staff it is necessary to place the end
    //of the slur at the same staff than slur end note
    int iStaff = m_idxStaffStart;
    if (m_idxStaffEnd != m_idxStaffStart)
        iStaff = m_pSlur->get_end_note()->get_staff();

    if (m_fSlurBelow)
        y3 = m_pInstrEngrv->get_bottom_line_of_staff(iStaff) + tenths_to_logical(60.0f);
    else
        y3 = m_pInstrEngrv->get_top_line_of_staff(iStaff) - tenths_to_logical(20.0f);


//    if (m_fSlurBelow)
//        y3 = m_uStaffTop + tenths_to_logical(60.0f);
//    else
//        y3 = m_uStaffTop - tenths_to_logical(20.0f);
}

//---------------------------------------------------------------------------------------
GmoShapeNote* SlurEngraver::get_relevant_note_for_chords(GmoShapeNote* pNoteShape)
{
    if (pNoteShape->is_in_chord() && pNoteShape->is_shape_chord_base_note())
    {
        GmoShapeChordBaseNote* pBase = static_cast<GmoShapeChordBaseNote*>(pNoteShape);
        return m_fSlurBelow ? pBase->get_bottom_note() : pBase->get_top_note();
    }

    return pNoteShape;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_control_points(int type)
{
//    dbgLogger << "Slur " << (m_fSlurBelow ? "below" : "above")
//        << ", num.data points=" << m_dataPoints.size();

    if (m_dataPoints.size() <= 2 || m_fSlurForGraces)
        compute_two_points_slur(type);      //one point in case of system start/end
    else
        compute_many_points_slur(type);

    //slur computed. Transfer final points
    m_points[ImoBezierInfo::k_start].x = x0;
    m_points[ImoBezierInfo::k_start].y = y0;
    m_points[ImoBezierInfo::k_ctrol1].x = x1;
    m_points[ImoBezierInfo::k_ctrol1].y = y1;
    m_points[ImoBezierInfo::k_ctrol2].x = x2;
    m_points[ImoBezierInfo::k_ctrol2].y = y2;
    m_points[ImoBezierInfo::k_end].x = x3;
    m_points[ImoBezierInfo::k_end].y = y3;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_many_points_slur(int UNUSED(type))
{
    compute_default_slur();

    LUnits dist = check_if_slur_exceeds_all_reference_points();

    //if the slur has insuficient height, do something to improve it
    if (dist > 0.0f)
        improve_low_slur(dist);
}

//---------------------------------------------------------------------------------------
LUnits SlurEngraver::check_if_slur_exceeds_all_reference_points()
{
    //check that all reference points are above/below the slur.
    //Returs 0.0 if the slur fits perfectly. Otherwise, retuns the required distance
    //to shitf the slur so that it overpasses all reference points.

    //finding the distance between a reference point and the Bezier curve requires many
    //computations: solving a cubic equation to find t, then use t to find the point on
    //the Bezier and, finally, compute the distance between the two points.
    //To save time, the slur will be approximated by a circular arc. This is
    //safe as the arc is always below the bezier (when slur up) or above it (slur down).
    //And the distance from the arc to the ref point is a cheap computation.
    compute_approximate_arc();

    //now, check if distance from ref.points to circle center is greater than radius
    LUnits r2 = r*r;
    LUnits maxDist = 0.0f;
    UPoint peak(0.0f, 0.0f);
    for (size_t i=1; i < m_dataPoints.size()-1; ++i)
    {
        LUnits x = m_dataPoints[i].x;
        LUnits y = m_dataPoints[i].y;
        LUnits dx2 = (xc-x)*(xc-x);
        LUnits dist2 = dx2 + (yc-y)*(yc-y);
        if (dist2 > r2)
        {
            LUnits dy = abs(yc-y) - sqrt(r2 - dx2);
            if (dy > maxDist)
            {
                peak.x = x;
                peak.y = y;
                maxDist = dy;
            }
        }
    }
    m_dbgPeak.y = peak.y;
    m_dbgPeak.x = peak.x;

    return maxDist;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_approximate_arc()
{
    //for a symmetrical horizontal bezier:
    xc = xm;
    yc = ((x0-xc)*(x0-xc) + (y0+ym)*(y0-ym)) / (2.0f*(y0-ym));
    r = abs(ym - yc);

    //if the bezier is not horizontal, rotate circle center around P0
    //new circle center (sinb reversed, as y axis is reversed)
    LUnits dx = xc-x0;
    LUnits dy = abs(yc-y0);
    if (m_fSlurBelow)
    {
        xc = x0 + dx * cosb + dy * sinb;;
        yc = y0 - dy * cosb + dx * sinb;
    }
    else
    {
        xc = x0 + dx * cosb - dy * sinb;
        yc = y0 + dy * cosb + dx * sinb;
    }
}

//---------------------------------------------------------------------------------------
void SlurEngraver::improve_low_slur(LUnits dist)
{
    //the slur does not overpass some ref. points. 'dist' is the required distance
    //to overpass all points. Something must be done!

    //If distance is less than 3 lines just shift the slur
    if (dist < tenths_to_logical(30.0f))
    {
        shift_bezier_up_down(dist);
    }
    else
    {
        //too much distance to solve by shifting the slur
        //Let's try an asymmetrical bezier with more curvature
        height += dist;
        method_asymmetrical_slur(height);
        //TODO: Now distance to ref.points from this new  slur should be measured and
        // if not overpassing all points, shift the slur. For now, apply a fix shift.
        shift_bezier_up_down( tenths_to_logical(10.0f) );
    }
}

//---------------------------------------------------------------------------------------
void SlurEngraver::shift_bezier_up_down(LUnits dist)
{
    if (!m_fSlurBelow)
        dist = -dist;

    y0 += dist;
    y1 += dist;
    y2 += dist;
    y3 += dist;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::method_asymmetrical_slur(LUnits height)
{
    //Method: Compute angles to intermediate points, keep the greatest angles and
    //double them

//    dbgLogger << ". Asymmetrical slur method." << endl;
    m_dbgColor = Color(127,0,255);  //dbg: violet
    r = 0.0f;   //dbg: to avoid drawing the approx. circle

    //some constants for angles
    const float PI = 3.141592654f;
    const float a90 = PI/2.0f;


    //------------------------------------------------------------------------
    //Determine max. angles a1 & a2 (referred to the horizontal)


    //get max angle from the start point (referred to the horizontal)
    LUnits a1 = 0.0f;       //angles (in radians) for the control points
    LUnits a2 = 0.0f;
    size_t numPoints = m_dataPoints.size() - 2;
    for(size_t i = 1; i <= numPoints; ++i)
    {
        LUnits dx = m_dataPoints[i].x - x0;
        LUnits dy = abs(y0 - m_dataPoints[i].y);
        LUnits a = atan2(dy, dx);               //get angle

//        dbgLogger << "     Selecting start angle: i=" << i
//            << ", a=" << a << " (" << a*180.0f/PI << "º)"
//            << ", a1=" << a1 << " (" << a1*180.0f/PI << "º)" << endl;

        if (a > a1)     //save data for greatest angle
            a1 = a;
    }
//    dbgLogger << "     Maximum a1= " << a1 << " (" << a1*180.0f/PI << "º)" << endl;

    //get max angle from the end point
    for(size_t i=1; i <= numPoints; ++i)
    {
        LUnits dx = x3 - m_dataPoints[i].x;
        LUnits dy = abs(y3 - m_dataPoints[i].y);
        //LUnits tana = dy/dx;
        LUnits a = atan2(dy, dx);               //get angle

//        dbgLogger << "     Selecting end angle: i=" << i
//            << ", a=" << a << " (" << a*180.0f/PI << "º)"
//            << ", a2=" << a2 << " (" << a2*180.0f/PI << "º)" << endl;

        if (a2 < a)     //save data for greatest angle
            a2 = a;
    }
//    dbgLogger << "     Maximum a2= " << a2 << " (" << a2*180.0f/PI << "º)" << endl;

    //------------------------------------------------------------------------
    //double the angles
    a1 += a1;
    a2 += a2;

    //limit angles to 90º
    if (a1 > a90)
        a1 = a90;
    if (a2 > a90)
        a2 = a90;
//    dbgLogger << "     After limiting angles: a1=" << a1 << " (" << a1*180.0f/PI << "º)"
//        << ", a2=" << a2 << " (" << a2*180.0f/PI << "º)" << endl;

    //------------------------------------------------------------------------
    //Compute slur (p0, p3, a1, a2, height) a1,a2 referred to the horizontal

    //compute bezier tangents' lenght
    LUnits b = atan((y0-y3)/(x3-x0));   //y axis reversed
    LUnits t1 = height / sin(a1+b);
    LUnits t2 = height / sin(a2+b);

//    dbgLogger << "     "
//        << ", cos(a1)=" << cos(a1) << ", cos(a2)=" << cos(a2)
//        << ", sin(a1)=" << sin(a1) << ", sin(a2)=" << sin(a2) << endl;
//
//    dbgLogger << "     t1=" << t1 << ", a1=" << a1
//        << ", t2=" << t2 << ", a2=" << a2 << endl << endl;


    //calculate the control points
    if (m_fSlurBelow)
    {
        x1 = x0 + t2 * cos(a1);
        y1 = y0 + t2 * sin(a1);
        x2 = x3 - t1 * cos(a2);
        y2 = y3 + t1 * sin(a2);
    }
    else
    {
        x1 = x0 + t1 * cos(a1);
        y1 = y0 - t1 * sin(a1);
        x2 = x3 - t2 * cos(a2);
        y2 = y3 - t2 * sin(a2);
    }
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_default_slur()
{
    //Method:
    //A quite flat one, and symmetrical.
    //- The height h is incremented (to a maximum of 3.5 lines) as D increases, add more
    //  curvature.
    //- The proportion d/D increases as D increases to increase the start/end points
    //  curvature.

    m_dbgColor = Color(175, 165, 21);  //dbg: olive green

    //determine height, in tenths
    LUnits h = 8.0f;
    if (D < 25.0f)
        h = 8.0f;
    else if (D < 50.0f)
        h = 14.0f;
    else if (D < 100.0f)
        h = 14.0f + (D - 50.0f) * 0.12f;
    else if (D < 300.0f)
        h = 20.0f + (D - 100.0f) * 0.045f;
    else if (D < 600.0f)
        h = 29.0f + (D - 300.0f) * 0.016666666f;
    else
        h = 35.0f;

    // determine d
    LUnits d = D;
    if (D < 55.0f)
        d = 0.5f * D;
    else if (D < 200.0f)
        d = 0.6 * D;
    else
        d = 0.7 * D;

    //final height, in LUnits
    LUnits hu = tenths_to_logical(h);
    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
    height = (m_fSlurForGraces ? m_thickness * 2.5f : hu);

    //determine tangents' length
    LUnits d1 = tenths_to_logical((D - d)/2.0f);

    //peak point when this bezier is horizontal
    xm = x0 + uD/2.0f;
    ym = y0 + 0.75f*(m_fSlurBelow ? height : -height);

    //shifts from start/end points for control points
    //as y axis is reversed, sinb has the sign changed
    LUnits dx1 = d1 * cosb + height * sinb;
    LUnits dx2 = d1 * cosb - height * sinb;
    LUnits dy1 = height * cosb - d1 * sinb;
    LUnits dy2 = height * cosb + d1 * sinb;

    //control points
    x1 = x0 + (m_fSlurBelow ? dx2 : dx1);
    y1 = y0 + (m_fSlurBelow ? dy2 : -dy1);
    x2 = x3 - (m_fSlurBelow ? dx1 : dx2);
    y2 = y3 + (m_fSlurBelow ? dy1 : -dy2);
}

//---------------------------------------------------------------------------------------
UPoint SlurEngraver::determine_peak_point()
{
    if (m_dataPoints.size() < 3)
        return UPoint(-1.0f, 0.0f);

    //determine base line: y = b.x + m;  b.x - y + m = 0
    LUnits m = (y3-y0)/(x3-x0);
    LUnits b = y0 - m*x0;

    //determine distance between baseline and data points. Take the maximum
    //distance (x0,y0) = abs(m.x0 - y0 + b) / sqrt(m*m + 1)
    float q = sqrt(m*m + 1.0f);
    float maxDist = 0.0f;
    UPoint peak(0.0f, 0.0f);
    for (size_t i=1; i < m_dataPoints.size()-1; ++i)
    {
        LUnits x = m_dataPoints[i].x;
        LUnits y = m_dataPoints[i].y;
        float distance = abs(m*x - y + b) / q;
        if (maxDist < distance)
        {
            maxDist = distance;
            peak.x = x;
            peak.y = y;
        }
    }

    return peak;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_two_points_slur(int type)
{
    if (type == k_single_shape)
        method_for_two_points();
    else
        compute_default_slur();
}

//---------------------------------------------------------------------------------------
void SlurEngraver::find_contour_reference_points()
{
    //invoked from create_single_shape() to determine the curve profile.

    //get profile points between bezier start and end points
    int idxStaff = (m_fSlurBelow ? max(m_idxStaffEnd, m_idxStaffStart)
                                 : min(m_idxStaffEnd, m_idxStaffStart));

    vector<UPoint> data = (m_fSlurBelow ? m_pVProfile->get_max_profile_points(x0, x3, idxStaff)
                                        : m_pVProfile->get_min_profile_points(x0, x3, idxStaff) );

    //fix x coordinates for start and end points
    if (data.size() > 1)
    {
        if (data[0].x < x0)
            data[0].x = x0;
        else if (data[0].x > x0)
            data.insert(data.begin(), UPoint(x0, y0));

        if (data.back().x + tenths_to_logical(10.0f) < x3)
            data.push_back(UPoint(x3, y3));
        else
            data.back() = UPoint(x3, y3);
    }

    //remove intermediate no-peak points
    size_t iMax = data.size() + 1;
    while (data.size() > 2 && data.size() < iMax)
    {
        size_t iPrev = 0;
        size_t iCur = 1;
        iMax = data.size();
        for (size_t i=2; i < data.size(); ++i)
        {
            if (m_fSlurBelow)
            {
                if (data[iCur].y < data[iPrev].y && data[iCur].y < data[i].y)
                {
                    //remove itCur
                    data.erase(data.begin() + iCur);
                    i = iCur;
                }
                else
                {
                    iPrev = iCur;
                    iCur = i;
                }
            }
            else
            {
                if (data[iCur].y > data[iPrev].y && data[iCur].y > data[i].y)
                {
                    //remove itCur
                    data.erase(data.begin() + iCur);
                    i = iCur;
                }
                else
                {
                    iPrev = iCur;
                    iCur = i;
                }
            }
        }
    }

    //compute baseline
    if (data.size() > 2)
    {
        //determine base line: y = m.x + b
        LUnits m = (y3-y0)/(x3-x0);
        LUnits b = y0 - m*x0;

        //remove points not between slur and base line
        size_t i = 1;
        while (i < data.size()-1)
        {
            //compute base line point for current x position
            LUnits x = data[i].x;
            LUnits y = m * x + b;

            //remove point if not between slur and base line
            if (   (m_fSlurBelow && (data[i].y <= y))
                || (!m_fSlurBelow && (data[i].y >= y)) )
            {
                data.erase(data.begin() + i);
            }
            else
                ++i;
        }
    }

    //add space between reference points and slur points
    if (data.size() > 0)
    {
        LUnits space = tenths_to_logical(5.0f);
        vector<UPoint>::iterator it;
        for (it=data.begin(); it < data.end(); ++it)
        {
            (*it).y += (m_fSlurBelow ? space : -space);
        }
    }

    m_dataPoints = data;
}

////---------------------------------------------------------------------------------------
//LUnits SlurEngraver::determine_beam_height(ImoNote* pNote)
//{
//    //approx beam height: beamThickness * (3*numBeams - 1) / 2
//
//    LUnits beamThickness = tenths_to_logical(LOMSE_BEAM_THICKNESS);
//    int numBeams = 0;
//    for (; numBeams < 6; numBeams++)
//    {
//        if (pNote->get_beam_type(numBeams) == ImoBeam::k_none)
//            break;
//    }
//    return beamThickness * float(3 * numBeams - 1) / 2.0f;
//}

////---------------------------------------------------------------------------------------
//void SlurEngraver::add_user_displacements(int iSlur, UPoint* points)
//{
//    //ImoBezierInfo* pBezier = (iSlur == 0 ? m_pSlur->get_start_bezier()
//    //                                    : m_pSlur->get_stop_bezier() );
//    //if (pBezier)
//    //{
//    //    for (int i=0; i < 4; i++)
//    //    {
//    //        (points+i)->x += tenths_to_logical(pBezier->get_point(i).x);
//    //        (points+i)->y += tenths_to_logical(pBezier->get_point(i).y);
//    //    }
//    //}
//}

//---------------------------------------------------------------------------------------
void SlurEngraver::set_prolog_width(LUnits width)
{
    m_uPrologWidth = width;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::method_for_two_points()
{
    //method: curvature increases with height and center is moved toward peak point
    //when great height vertical/horizontal


    //AWARE: all formulas marked as "y axis reversed" have a sign changed to take
    //       into account the fact that y axis is reversed

    //Determine base line angle
    LUnits b = atan((y0-y3)/(x3-x0));   //y axis reversed

    //some constants for angles
    const float PI = 3.141592654f;
    const float a30 = PI/8.0f;
    const float a45 = PI/4.0f;
    const float a90 = PI/2.0f;

//    dbgLogger << ". Two points case. angle b=" << b << " (" << (b*180.0f/PI) << "º), D=" << D;
//    m_dbgColor = (Color(255,0,0));    //red

    //For short slurs or slurs with small angles use the default slur.
    //it has low height, is symmetrical, and requires less computations
    if ( abs(b) <= a30 || D < 50.0f)
    {
        compute_default_slur();
    }
    else
    {
        //Decide on angles for control lines (referred to horizontal line)
        //empirical formulas that provide good results
        LUnits a1 = 0.0f;
        LUnits a2 = 0.0f;
        if (m_fSlurBelow)
        {
            if (b > 0.0f)
            {
                a2 = min(a90, a45+b);
                a1 = (b >= a45 ? a90-b : a45);
            }
            else
            {
                a1 = min(a90, a45-b);
                a2 = (b <= -a45 ? a90+b : a45);
            }
        }
        else
        {
            if (b > 0.0f)
            {
                a1 = min(a90, a45+b);
                a2 = (b >= a45 ? a90-b : a45);
            }
            else
            {
                a2 = min(a90, a45-b);
                a1 = (b <= -a45 ? a90+b : a45);
            }
        }

        //Decide on Bezier height. It will be 1/3 of the distance, with a maximum
        //of 3.0 lines and a minimum of 1.5 lines
        LUnits d = sqrt((x3-x0)*(x3-x0) + (y3-y0)*(y3-y0));
        LUnits q = min(tenths_to_logical(30.0f), d*0.33f);
        q = max(q, tenths_to_logical(15.0f));

        //reduce height by half if one angle is horizontal (less than 3º)
        if (a1 < 0.05f || a2 < 0.05f)
            q *= 0.5f;

        //bezier tangents lenghts
        LUnits b1 = 0.0f;
        LUnits b2 = 0.0f;
        if (m_fSlurBelow)
        {
            b1 = a90 - a1 - b;
            b2 = a90 - a2 + b;
        }
        else
        {
            b1 = a90 - a1 + b;
            b2 = a90 - a2 - b;
        }
        LUnits t1 = abs(q / cos(b1));
        LUnits t2 = abs(q / cos(b2));

        //Compute Bezier points:
        if (m_fSlurBelow)
        {
            x1 = x0 + t1 * cos(a1);
            y1 = y0 + t1 * sin(a1);  //y axis reversed

            x2 = x3 - t2 * cos(a2);
            y2 = y3 + t2 * sin(a2);  //y axis reversed
        }
        else
        {
            x1 = x0 + t1 * cos(a1);
            y1 = y0 - t1 * sin(a1);  //y axis reversed

            x2 = x3 - t2 * cos(a2);
            y2 = y3 - t2 * sin(a2);  //y axis reversed
        }

//        dbgLogger << ", a1=" << a1 << " (" << (a1*180.0f/PI) << "º)"
//            << ", a2=" << a2 << " (" << (a2*180.0f/PI) << "º)"
//            << ", q=" << q << ", t1=" << t1 << ", t2=" << t2 << endl
//            << "     angle b1=" << b1 << " (" << (b1*180.0f/PI) << "º)"
//            << ", angle b2=" << b2 << " (" << (b2*180.0f/PI) << "º)" << endl
//            << "     d=" << d << ", d/3=" << d*0.33f
//            << ", 15T=" << tenths_to_logical(15.0f)
//            << ", 30T=" << tenths_to_logical(30.0f)
//            << ", q=" << q << endl;

    }

//    dbgLogger << endl;
}

////======================================================================================
//// UNUSED
//// Letft here until slur layout is more advanced, just in case some of
//// these methods could still be needed. Some of them took time to write
////======================================================================================
//
////---------------------------------------------------------------------------------------
//void SlurEngraver::find_peak_point_and_shift()
//{
//    //find bezier point at peak point
//    UPoint peak = determine_peak_point();
//
//    if (peak.x >= 0.0f)     //peak.x is negative for two point slurs
//    {
//        float a = -x0 + 3.0f*x1 - 3.0f*x2 + x3;
//        float b = 3.0f*x0 - 6.0f*x1 + 3.0f*x2;
//        float c = 3.0f*(x1 - x0);
//        float d = x0 - peak.x;
//        float tPeak = solve_cubic(a, b, c, d);
//        if (tPeak >= 0.0f)
//        {
//            LUnits yBezier = bezier(y0, y1, y2, y3, tPeak);
//            m_dbgPeak.y = yBezier;
//            m_dbgPeak.x = peak.x;
//
//            //check distance between peak point and bezier
//            if ( (m_fSlurBelow && (yBezier < peak.y))
//                 || (!m_fSlurBelow && (yBezier > peak.y)) )
//            {
//                LUnits dist = abs(yBezier - peak.y);
//                shift_bezier_up_down(dist);
//            }
//        }
//    }
//}
//
////---------------------------------------------------------------------------------------
//float SlurEngraver::compute_bezier(float x0, float x1, float x2, float x3, float t)
//{
//    float a = -x0 + 3.0f*x1 - 3.0f*x2 + x3;
//    float b = 3.0f*x0 - 6.0f*x1 + 3.0f*x2;
//    float c = 3.0f*(x1 - x0);
//    float d = x0;
//
//    return a*t*t*t + b*t*t + c*t + d;
//}
//
////---------------------------------------------------------------------------------------
//void SlurEngraver::increment_bezier_height(LUnits dist)
//{
//    //As if moving vertically the handler in top line center
//
//    //determine baseline angle
//    LUnits d = sqrt((x3-x0)*(x3-x0) + (y3-y0)*(y3-y0));
//    LUnits cosb = (x3-x0)/d;
//    LUnits sinb = (y0-y3)/d;    //y axis reversed
//
//    //compute shifts
//    LUnits dy = dist + tenths_to_logical(10.0f);
//    LUnits dx = (cosb < 0.0001f ? 0.0f : dist*sinb/cosb);
//
//    //add shifts
//    x1 -= dx;
//    y1 += (m_fSlurBelow ? dy : -dy);
//    x2 -= dx;
//    y2 += (m_fSlurBelow ? dy : -dy);
//}
//
////---------------------------------------------------------------------------------------
//void SlurEngraver::method_double_max_angles(int type)
//{
//    //Method: Compute angles to intermediate points, keep the greatest angles and
//    //double them
//
//    dbgLogger << ". Double max. angle method." << endl;
//    m_dbgColor = Color(127,0,255);  //violet
//    r = 0.0f;   //dbg: to avoid drawing the approx. circle
//
//
//    //some constants for angles
//    const float PI = 3.141592654f;
//    const float a15 = PI/12.0f;
//    const float a45 = PI/4.0f;
//    const float a90 = PI/2.0f;
//
//    //get max angle from the start point (referred to the horizontal)
//    LUnits yPeak = m_dataPoints[1].y;      //y for peak point
//    LUnits iPeak = 1;       //index for peak point
//    LUnits a1 = -a90;       //angles (in radians) for the control points
//    LUnits a2 = 0.0f;
//    size_t numPoints = m_dataPoints.size() - 2;
//    for(size_t i = 1; i <= numPoints; ++i)
//    {
//        LUnits dx = m_dataPoints[i].x - x0;
//        LUnits dy = y0 - m_dataPoints[i].y;     //y axis reversed
//        LUnits a = atan2(dy, dx);               //get angle
//
//        dbgLogger << "     Selecting start angle: i=" << i
//            << ", a=" << a << " (" << a*180.0f/PI << "º)"
//            << ", a1=" << a1 << " (" << a1*180.0f/PI << "º)"
//            << ", yPeak=" << yPeak << ", iPeak=" << iPeak << endl;
//        if (i == 1)
//        {
//            a1 = a;
//        }
//        else if (m_fSlurBelow)
//        {
//            if (a < a1)     //save data for lowest angle
//                a1 = a;
//
//            if (m_dataPoints[i].y > yPeak)      //y axis reversed
//            {
//                yPeak = m_dataPoints[i].y;
//                iPeak = i;
//            }
//        }
//        else
//        {
//            //start angle is -90º < a < 90º. Take greatest value
//            if (a > a1)     //save data for greatest angle
//                a1 = a;
//
//            if (m_dataPoints[i].y < yPeak)      //y axis reversed
//            {
//                yPeak = m_dataPoints[i].y;
//                iPeak = i;
//            }
//        }
//    }
//    dbgLogger << "     Maximum a1= " << a1 << " (" << a1*180.0f/PI << "º)"
//        << ", yPeak=" << yPeak << ", iPeak=" << iPeak << endl;
//
//    //get max angle from the end point
//    for(size_t i=1; i <= numPoints; ++i)
//    {
//        LUnits dx = m_dataPoints[i].x - x3;
//        LUnits dy = y3 - m_dataPoints[i].y;     //y axis reversed
//        LUnits a = atan2(dy, dx);               //get angle
//
//        dbgLogger << "     Selecting end angle: i=" << i
//            << ", a=" << a << " (" << a*180.0f/PI << "º)"
//            << ", a2=" << a2 << " (" << a2*180.0f/PI << "º)" << endl;
//        if (i == 1)
//        {
//            a2 = a;
//        }
//        else if (m_fSlurBelow)
//        {
//            if (a < a2)     //save data for lowest angle
//                a2 = a;
//        }
//        else
//        {
//            //end angle is > 90º (+) and < 270º (-)
//            //take the nearest one to 90º
//            if (a > a2)
//                a2 = a;
//        }
//    }
//    a2 = (a2 > 0.0f ? PI - a2 : -(PI + a2));   //take inner angle
//    dbgLogger << "     Maximum a2= " << a2 << " (" << a2*180.0f/PI << "º)" << endl;
//
//
//    //computed angles a1, a2 are referred to the horizontal line. Refer them to the
//    //base line
//    LUnits b = atan((y0-y3)/(x3-x0));   //y axis reversed
//    if (b > 0)
//    {
//        a1 -= b;
//        a2 += b;
//    }
//    else
//    {
//        a1 -= b;
//        a2 += b;
//    }
//    dbgLogger << "     Ref.to baseline. b=" << b << " (" << b*180.0f/PI << "º)"
//        << ", a1=" << a1 << " (" << a1*180.0f/PI << "º)"
//        << ", a2=" << a2 << " (" << a2*180.0f/PI << "º)" << endl;
//
//    //The symmetry of the curve is determined by the angles. The more different the
//    //the angles the more asymmetrical the curve. Angles also influences curvature:
//    //90º angles creates a "half circle". As angles get lower the curve is flattened.
//    //
//    //Tangents length determines the "squaredness" and thus, also the height of the curve
//
//
//    //increment angles, to ensure overpass the higest point. Increment will depend
//    //on assimetry: angles difference, (=distance to central point)
//    //Double the angles
////    LUnits da1 = abs(a1);                            // get angle diff
////    LUnits da2 = a2 < 0 ? PI + a2 : abs(a2);
////    LUnits da1 = abs(a1);
////    LUnits da2 = abs(a2);
////    a1 -= da1 * 2.0f;       // double the diff
////    a2 += da2 * 2.0f;
////    float adiff = abs(da1 - da2);
////    float factor = 1.0;
////    if (adiff < a90 * 0.1f)
////        factor = 1.5f;
////    else if (adiff < a90 * 0.2f)
////        factor = 1.6f;
////    else if (adiff < a90 * 0.3f)
////        factor = 1.7f;
////    else if (adiff < a90 * 0.4f)
////        factor = 1.8f;
////    else // > 45º (0.5)
////        factor = 2.0f;
//
////    dbgLogger << "     a1=" << a1 << "(" << a1*180.0f/PI << "º), a2="
////        << a2 << ", adiff=" << adiff << ", factor=" << factor << endl;
//
//    LUnits d1 = m_dataPoints[iPeak].x - x0;
//    LUnits d2 = x3 - m_dataPoints[iPeak].x;
//
//    float f1 = d2 / (d1 + d2);
//    float f2 = d1 / (d1 + d2);
//    a1 *= (2.4f + f1);
//    a2 *= (2.4f + f2);
//    dbgLogger << "     After incrementing angles: f1=" << f1 << ", f2=" << f2
//        << ", a1=" << a1 << " (" << a1*180.0f/PI << "º)"
//        << ", a2=" << a2 << " (" << a2*180.0f/PI << "º)" << endl;
//
//    //limit angles to 90º
//    if (a1 > a90)
//        a1 = a90;
//    if (a2 > a90)
//        a2 = a90;
//    dbgLogger << "     After limiting angles: a1=" << a1 << " (" << a1*180.0f/PI << "º)"
//        << ", a2=" << a2 << " (" << a2*180.0f/PI << "º)" << endl;
//
//
//    //Refer back the angles to the horizontal line
//    if (b > 0)
//    {
//        a1 += b;
//        a2 -= b;
//    }
//    else
//    {
//        a1 += b;
//        a2 -= b;
//    }
//    dbgLogger << "     Ref.to horizontal. b=" << b << " (" << b*180.0f/PI << "º)"
//        << ", a1=" << a1 << " (" << a1*180.0f/PI << "º)"
//        << ", a2=" << a2 << " (" << a2*180.0f/PI << "º)" << endl;
//
//
//
//    dbgLogger << "     "
//        << ", cos(a1)=" << cos(a1) << ", cos(a2)=" << cos(a2)
//        << ", sin(a1)=" << sin(a1) << ", sin(a2)=" << sin(a2) << endl;
//
//    dbgLogger << "     d1=" << d1 << ", a1=" << a1
//        << ", d2=" << d2 << ", a2=" << a2 << endl << endl;
//
//
//    //bezier tangents lenghts
//    //As incrementing the lenght of the tangent raises the curve, the assimetry factor
//    //will be used to control the tangents length
//    LUnits t1 = d1; // * (0.8f + f2);
//    LUnits t2 = d2; // * (0.8f + f1);
//
//    //calculate the control points
//    if (m_fSlurBelow)
//    {
//        x1 = x0 + t1 * cos(a1);
//        y1 = y0 - t1 * sin(a1);    //y axis reversed
//        x2 = x3 - t2 * cos(a2);
//        y2 = y3 - t2 * sin(a2);    //y axis reversed
//    }
//    else
//    {
//        x1 = x0 + t1 * cos(a1);
//        y1 = y0 - t1 * sin(a1);    //y axis reversed
//        x2 = x3 - t2 * cos(a2);
//        y2 = y3 - t2 * sin(a2);    //y axis reversed
//    }
//}
//
////---------------------------------------------------------------------------------------
//void SlurEngraver::method_old(int type)
//{
//    //Method:
//    //Use a fixed symmetrical curve with variable height:
//    //heigh 1,2 lines + abs. difference between baseline start/end points ==>
//    //  ==> low height when horizontal, greater heigth as more slope
//    //
//    //Both tangents with the same length (symmetrical bezier). Length of tangents is
//    //proportional to star/end distance, but always the same proportion.
//    //
//    //Control poins are computed assuming horizontal baseline. This implies that
//    //the peak point moves toward the higest end when it is not horizontal
//    //-----------------------------------------------------------------------------------
//
//    dbgLogger << ". Old method." << endl;
//    m_dbgColor = Color(255,128,0);  //orange
//
//
//    LUnits D = m_points[ImoBezierInfo::k_end].x - x0;
//    LUnits d = D / 5.8f;
//
//    //rough approx., assuming horizontal base line
//    LUnits minHeight = abs(y0-y3);
//    LUnits height = minHeight + tenths_to_logical(12.0);
//
//    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
//    LUnits hc = (m_fSlurForGraces ? m_thickness * 2.5f : height);
//    x1 = x0 + d;
//    y1 = y0 + (m_fSlurBelow ? hc : -hc);
//
//    x2 = x3 - d;
//    y2 = y3 + (m_fSlurBelow ? hc : -hc);
//}
//
////---------------------------------------------------------------------------------------
//void SlurEngraver::method_symmetrical_flat(int type)
//{
//    //Method:
//    //Use a fixed symmetrical curve with a fixed height of 1,2 lines.
////    //heigh 1,2 lines + abs. difference between baseline start/end points ==>
////    //  ==> low height when horizontal, greater heigth as more slope
//    //Both tangents with the same length (symmetrical bezier). Length of tangents is
//    //proportional to star/end distance, but always the same proportion
//
//    dbgLogger << ". Symmetrical flat method." << endl;
//    m_dbgColor = Color(80,162,74);  //dark green
//
//    //determine height
//    //LUnits minHeight = abs(y3 - y0);
//    LUnits height = tenths_to_logical(12.0);
//
//    //determine baseline angle
//    LUnits d = sqrt((x3-x0)*(x3-x0) + (y3-y0)*(y3-y0));
//    LUnits cosb = (x3-x0)/d;
//    LUnits sinb = (y0-y3)/d;    //y axis reversed
//
//    //compute control points for an horizontal bezier
//    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
//    LUnits hc = (m_fSlurForGraces ? m_thickness * 2.5f : height);
//    LUnits D = (x3 - x0) / 5.8f;
//
//    LUnits dx1 = D * cosb - hc * sinb;
//    LUnits dx2 = D * cosb + hc * sinb;
//    LUnits dy1 = hc * cosb + D * sinb;
//    LUnits dy2 = hc * cosb - D * sinb;
//
//    x1 = x0 + (m_fSlurBelow ? dx2 : dx1);
//    y1 = y0 + (m_fSlurBelow ? dy2 : -dy1);
//    x2 = x3 - (m_fSlurBelow ? dx1 : dx2);
//    y2 = y3 + (m_fSlurBelow ? dy1 : -dy2);
//}
//
////---------------------------------------------------------------------------------------
//float SlurEngraver::solve_cubic(float a, float b, float c, float d)
//{
//    //returns the only solution that is real and also is between 0 and 1
//    //can return -1.0 if something fails
//
//    //the three solutions:
//    float x1Re; //, x1Im;   //real and imaginary parts
//    float x2Re; //, x2Im;   //real and imaginary parts
//    float x3Re; //, x3Im;   //real and imaginary parts
//    const float PI = 3.141592654f;
//
//    if (a == 0.0f)
//    {
//        //need to solve b.t2 + c.t + d = 0
//        return -1.0f;
//    }
//
//    if (d == 0.0f)
//    {
//        //One root is 0. the other two are found by solving a.t2 + b.t + c = 0
//        return -1.0f;
//    }
//
//    b /= a;
//    c /= a;
//    d /= a;
//
//    float q = (3.0f * c - (b*b))/9.0f;
//    float r = (-27.0f * d + b*(9.0f*c - 2.0f*(b*b))) / 54.0f;
//    float disc = q*q*q + r*r;
//    //x1Im = 0.0f;    //The first root is always real.
//    float term1 = (b/3.0f);
//
//    if (disc > 0.0f)
//    {
//        // one root real, two are complex
//        float s = r + sqrt(disc);
//        s = ((s < 0.0f) ? -pow(-s, (1.0f/3.0f)) : pow(s, (1.0f/3.0f)));
//        float t = r - sqrt(disc);
//        t = ((t < 0.0f) ? -pow(-t, (1.0f/3.0f)) : pow(t, (1.0f/3.0f)));
//        x1Re = -term1 + s + t;
//        //not interested in imaginary roots
////        term1 += (s + t)/2.0f;
////        x3Re = x2Re = -term1;
////        term1 = sqrt(3.0f)*(-t + s)/2.0f;
////        x2Im = term1;
////        x3Im = -term1;
//
//        //return the one that is between 0 and 1
//        if (x1Re >= 0.0f && x1Re <= 1.0f)
//            return x1Re;
//
//        return -1.0f;
//    }
//
//    // The remaining options are all real
//    //x3Im = x2Im = 0.0f;
//    if (disc == 0)
//    {
//        //All roots real, at least two are equal.
//        //return the one that is between 0 and 1
//        float r13 = ((r < 0.0f) ? -pow(-r,(1.0f/3.0f)) : pow(r,(1.0f/3.0f)));
//        x1Re = -term1 + 2.0f*r13;
//        if (x1Re >= 0.0f && x1Re <= 1.0f)
//            return x1Re;
//
//        x2Re = -(r13 + term1);
//        if (x2Re >= 0.0f && x2Re <= 1.0f)
//            return x2Re;
//
//        //x3Re = x2Re;
//
//        return -1.0f;
//    }
//
//    //q < 0. Only option left is that all roots are real and unequal
//    //return the one that is between 0 and 1
//    q = -q;
//    float dum1 = acos(r/sqrt(q*q*q));
//    float r13 = 2.0f*sqrt(q);
//
//    x1Re = -term1 + r13 * cos(dum1/3.0f);
//    if (x1Re >= 0.0f && x1Re <= 1.0f)
//        return x1Re;
//
//    x2Re = -term1 + r13 * cos((dum1 + 2.0f* PI)/3.0f);
//    if (x2Re >= 0.0f && x2Re <= 1.0f)
//        return x2Re;
//
//    x3Re = -term1 + r13 * cos((dum1 + 4.0f* PI)/3.0f);
//    if (x3Re >= 0.0f && x3Re <= 1.0f)
//        return x3Re;
//
//    return -1.0f;
//}

}  //namespace lomse
