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

#include "lomse_slur_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_engraving_options.h"
#include "lomse_shape_tie.h"
#include "lomse_shape_note.h"
#include "lomse_score_meter.h"
#include "lomse_instrument_engraver.h"
#include "lomse_vertical_profile.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// SlurEngraver implementation
//---------------------------------------------------------------------------------------
SlurEngraver::SlurEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           InstrumentEngraver* pInstrEngrv)
    : RelObjEngraver(libraryScope, pScoreMeter)
    , m_pInstrEngrv(pInstrEngrv)
    , m_uStaffTop(0.0f)
    , m_numShapes(0)
    , m_pSlur(nullptr)
    , m_fSlurBelow(false)
    , m_uPrologWidth(0.0f)
    , m_pStartNote(nullptr)
    , m_pEndNote(nullptr)
    , m_pStartNoteShape(nullptr)
    , m_pEndNoteShape(nullptr)
    , m_fTwoArches(false)
    , m_thickness(0.0f)
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
    m_pVProfile = pVProfile;

    //data specific for this class
    m_pSlur = static_cast<ImoSlur*>(pRO);

    m_pStartNote = static_cast<ImoNote*>(pSO);
    m_pStartNoteShape = static_cast<GmoShapeNote*>(pStaffObjShape);

    m_uStaffTop = yTop - m_pStartNoteShape->get_top();     //relative to note top
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

    m_uStaffTop = yTop - m_pEndNoteShape->get_top();     //relative to note top;

    m_idxStaff = idxStaff;
    m_pVProfile = pVProfile;
}

//---------------------------------------------------------------------------------------
void SlurEngraver::decide_placement()
{
    //[Read, p.266] placement: at notehead side when all notes in same direction.
    //When mixed directions, preferred above

    bool fMixed = is_end_point_set() &&
                  ( (m_pStartNoteShape->is_up() && !m_pEndNoteShape->is_up())
                     || (!m_pStartNoteShape->is_up() && m_pEndNoteShape->is_up()) );

    if (fMixed)
        m_fSlurBelow = false;
    else
        m_fSlurBelow = m_pStartNoteShape->is_up() ? true : false;
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_first_or_intermediate_shape(Color color)
{
    m_color = color;
    if (m_numShapes == 0)
    {
        decide_placement();
        return create_first_shape();
    }
    else
        return create_intermediate_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_last_shape(Color color)
{
    m_color = color;
    if (m_numShapes == 0)
    {
        decide_placement();
        return create_single_shape();
    }
    return create_final_shape();
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_first_shape()
{
    //first shape when there are more than one

    compute_start_point();
    compute_end_of_staff_point();
    compute_default_control_points(&m_points[0]);
    //add_user_displacements(0, &m_points[0]);
    m_numShapes = 1;
    return LOMSE_NEW GmoShapeSlur(m_pSlur, 0, &m_points[0], m_thickness, m_color);
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_single_shape()
{
    //the only shape when start and end points are in the same system

    compute_ref_point(m_pStartNoteShape, &m_points[ImoBezierInfo::k_start]);
    compute_ref_point(m_pEndNoteShape, &m_points[ImoBezierInfo::k_end]);
    compute_start_point();
    compute_end_point(&m_points[ImoBezierInfo::k_end]);
    m_dataPoints = find_contour_reference_points();
    #if (0)  //new behaviour
        compute_control_points();
    #else
        compute_default_control_points(&m_points[0]);
    #endif
    //add_user_displacements(0, &m_points[0]);

    GmoShapeSlur* pShape = LOMSE_NEW GmoShapeSlur(m_pSlur, m_numShapes++, &m_points[0],
                                                  m_thickness, m_color);
    pShape->add_data_points(m_dataPoints);

    //if cross-staff slur do not add it to VProfile
    if (m_pStartNote->get_staff() != m_pEndNote->get_staff())
        pShape->set_add_to_vprofile(false);

    return pShape;
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_intermediate_shape()
{
    //intermediate shape spanning the whole system

    ++m_numShapes;
    //TODO
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* SlurEngraver::create_final_shape()
{
    //last shape when there are more than one

    compute_end_point(&m_points[ImoBezierInfo::k_end]);
    compute_start_of_staff_point();
    compute_default_control_points(&m_points[0]);
    //add_user_displacements(1, &m_points[0]);
    return LOMSE_NEW GmoShapeSlur(m_pSlur, m_numShapes++, &m_points[0], m_thickness);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_control_points()
{
    //start and end points
    LUnits x0 = m_points[ImoBezierInfo::k_start].x;
    LUnits y0 = m_points[ImoBezierInfo::k_start].y;
    LUnits x1 = m_points[ImoBezierInfo::k_end].x;
    LUnits y1 = m_points[ImoBezierInfo::k_end].y;

    //determine base line angle and length
    LUnits d = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
    LUnits cosb = (x1-x0)/d;
    LUnits sinb = (y1-y0)/d;

    //normalize ref. points, remove useless ones and find the peak point
    vector<UPoint> refPoint;    //intermediate ref. points, normalized
    size_t iPeak = 0;        //index on refPoint vector, for peak point
    LUnits yPeak = 0.0f;
    vector<UPoint>::iterator it;
    for (it=m_dataPoints.begin(); it < m_dataPoints.end(); ++it)
    {
        //shift point to reference it to origin
        LUnits x = (*it).x - x0;
        LUnits y = (*it).y - y0;

        //rotate it to right, as if base line was horizontal
        //and scale to match the interval [0,1]:
        LUnits xAux = (x*cosb + y*sinb) / d;
        LUnits yAux = (y*cosb - x*sinb) / d;

        //save point and save data to determine peak point.
        //when slur below y is positive
        if ((m_fSlurBelow && y > 0.0f) || (!m_fSlurBelow && y < 0.0f))
        {
            size_t i = refPoint.size();
            refPoint.push_back(UPoint(xAux, yAux));

            if ((i == 0)
                || (m_fSlurBelow && yPeak < yAux)
                || (!m_fSlurBelow && yPeak > yAux) )
            {
                iPeak = i;
                yPeak = yAux;
            }
        }

    }

    //determine peak point
    LUnits minHeight = 0.08f;
    LUnits xm = 0.5f;
    LUnits ym = (m_fSlurBelow ? minHeight : -minHeight);
    if (refPoint.size() > 2 && abs(refPoint[iPeak].y) > minHeight)
    {
        xm = refPoint[iPeak].x;
        ym = refPoint[iPeak].y;
    }


    //lets built the bezier. Move the center of the reference arc to the peak point
    LUnits ax = (8.0f * xm - 1.0f) / 3.0f;
    LUnits x2, x3;
    if (xm < 0.5f)
    {
        x2 = 0.0f;
        x3 = ax;
    }
    else
    {
        x3 = 1.0f;
        x2 = ax - 1.0f;
    }
    LUnits y2 = 4.0f * ym / 3.0f;
    LUnits y3 = y2;

    //scale it to match the base line length
    x2 *= d; y2 *= d;
    x3 *= d; y3 *= d;

    //rotate the coordinates so that they're on the baseline angle
    LUnits xAux = x2 * cosb - y2 * sinb;
    LUnits yAux = x2 * sinb + y2 * cosb;
    x2 = xAux; y2 = yAux;

    xAux = x3 * cosb - y3 * sinb;
    yAux = x3 * sinb + y3 * cosb;
    x3 = xAux; y3 = yAux;

    //translate all the coordinates so that they're in the right spot on the score
    x2 += x0; y2 += y0;
    x3 += x0; y3 += y0;


    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);

    m_points[ImoBezierInfo::k_ctrol1].x = x2;
    m_points[ImoBezierInfo::k_ctrol1].y = y2;

    m_points[ImoBezierInfo::k_ctrol2].x = x3;
    m_points[ImoBezierInfo::k_ctrol2].y = y3;

//    dbgLogger << "Slur " << (m_fSlurBelow ? "below" : "above") << endl
//        << "xm=" << xm << ", ym=" << ym << endl
//        << "p0=(" << x0 << ", " << y0 << ")" << endl
//        << "p1=(" << x1 << ", " << y1 << ")" << endl
//        << "p2=(" << x2 << ", " << y2 << ")" << endl
//        << "p3=(" << x3 << ", " << y3 << ")" << endl
//        << "ax=" << ax << endl << endl;

    //de-normalize ref. points, for drawing them in debug mode
    m_dataPoints.clear();
    for (it=refPoint.begin(); it < refPoint.end(); ++it)
    {
        LUnits x = (*it).x;
        LUnits y = (*it).y;

        //rotate it to left, to original position
        //and scale to match linebase length
        LUnits xAux = (x*cosb - y*sinb) * d;
        LUnits yAux = (y*cosb + x*sinb) * d;

        //shift point to reference it to slur start point, and save it
        m_dataPoints.push_back(UPoint(xAux+x0, yAux+y0));
    }
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_default_control_points(UPoint* points)
{
    LUnits D = (points+ImoBezierInfo::k_end)->x - (points+ImoBezierInfo::k_start)->x;
    LUnits d = D / 5.8f;

    //approx. burda, assuming horizontal base line
    LUnits minHeight = abs((points+ImoBezierInfo::k_start)->y - (points+ImoBezierInfo::k_end)->y );
    LUnits height = minHeight + tenths_to_logical(12.0);

    m_thickness = tenths_to_logical(LOMSE_TIE_MAX_THICKNESS);
    LUnits hc = height; //m_thickness * 4.5f;
    (points+ImoBezierInfo::k_ctrol1)->x = (points+ImoBezierInfo::k_start)->x + d;
    (points+ImoBezierInfo::k_ctrol1)->y = (points+ImoBezierInfo::k_start)->y + (m_fSlurBelow ? hc : -hc);

    (points+ImoBezierInfo::k_ctrol2)->x = (points+ImoBezierInfo::k_end)->x - d;
    (points+ImoBezierInfo::k_ctrol2)->y = (points+ImoBezierInfo::k_end)->y + (m_fSlurBelow ? hc : -hc);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_start_point()
{
    //if start note has stem and stem is on the same position than slur, place slur
    //over stem. Otherwise, place slur over note-head
    bool fOverStem = false;
    if (m_pStartNote->has_stem())
    {
        fOverStem = (m_fSlurBelow && m_pStartNote->is_stem_down())
                    || (!m_fSlurBelow && m_pStartNote->is_stem_up());
    }

	//x pos: center on notehead
	m_points[ImoBezierInfo::k_start].x = (m_pStartNoteShape->get_notehead_right() +
                          m_pStartNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths appart from notehead of from stem
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
    if (fOverStem)
        m_points[ImoBezierInfo::k_start].y = (m_fSlurBelow ?
                              m_pStartNoteShape->get_bottom() + space
                              : m_pStartNoteShape->get_top() - space );
    else
        m_points[ImoBezierInfo::k_start].y = (m_fSlurBelow ?
                              m_pStartNoteShape->get_notehead_bottom() + space
                              : m_pStartNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_point(UPoint* point)
{
    //over stem. Otherwise, place slur over note-head
    bool fOverStem = false;
    if (m_pEndNote->has_stem())
    {
        fOverStem = (m_fSlurBelow && m_pEndNote->is_stem_down())
                    || (!m_fSlurBelow && m_pEndNote->is_stem_up());
    }
	//x pos: center on notehead
	point->x = (m_pEndNoteShape->get_notehead_right() +
                m_pEndNoteShape->get_notehead_left()) / 2.0f;

    //y pos: 5 tenths appart from notehead
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);
    if (fOverStem)
        point->y = (m_fSlurBelow ?
                     m_pEndNoteShape->get_bottom() + space
                   : m_pEndNoteShape->get_top() - space );
    else
        point->y = (m_fSlurBelow ?
                     m_pEndNoteShape->get_notehead_bottom() + space
                   : m_pEndNoteShape->get_notehead_top() - space );
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_start_of_staff_point()
{
    m_points[ImoBezierInfo::k_start].x = m_pInstrEngrv->get_staves_left()
                                          + m_uPrologWidth - tenths_to_logical(10.0f);

    if (m_fSlurBelow)
        m_points[ImoBezierInfo::k_start].y =
            m_uStaffTop + m_pEndNoteShape->get_top() + tenths_to_logical(60.0f);
    else
        m_points[ImoBezierInfo::k_start].y =
            m_uStaffTop + m_pEndNoteShape->get_top() - tenths_to_logical(20.0f);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_end_of_staff_point()
{
    m_points[ImoBezierInfo::k_end].x = m_pInstrEngrv->get_staves_right();
    LUnits yTop = m_uStaffTop + m_pStartNoteShape->get_top();

    if (m_fSlurBelow)
        m_points[ImoBezierInfo::k_end].y = yTop + tenths_to_logical(60.0f);
    else
        m_points[ImoBezierInfo::k_end].y = yTop - tenths_to_logical(20.0f);
}

//---------------------------------------------------------------------------------------
void SlurEngraver::compute_ref_point(GmoShapeNote* pNoteShape, UPoint* point)
{
    bool fOnNoteHead = (m_fSlurBelow && pNoteShape->is_up()) ||
                       (!m_fSlurBelow && !pNoteShape->is_up());

    if (fOnNoteHead)
    {
        //x pos: center on notehead
        point->x = (pNoteShape->get_notehead_right() +
                    pNoteShape->get_notehead_left()) / 2.0f;
    }
    else
    {
        //x. pos: center of stem
        point->x = pNoteShape->get_stem_left() + pNoteShape->get_stem_width() / 2.0f;
    }


    //space = 5 tenths appart from y reference point
    LUnits space = tenths_to_logical(LOMSE_TIE_VERTICAL_SPACE);

    if (fOnNoteHead)
    {
        // y pos: notehead border
        point->y = (m_fSlurBelow ?
                        pNoteShape->get_notehead_bottom() + space
                        : pNoteShape->get_notehead_top() - space );
    }
    else
    {
        //y pos: top/bottom of note shape
        point->y = (pNoteShape->is_up() ?
                        pNoteShape->get_top() - space
                        : pNoteShape->get_bottom() + space );
    }
}

//---------------------------------------------------------------------------------------
vector<UPoint> SlurEngraver::find_contour_reference_points()
{
    vector<UPoint> data = (m_fSlurBelow ?
                    m_pVProfile->get_max_profile_points(m_points[ImoBezierInfo::k_start].x,
                                                    m_points[ImoBezierInfo::k_end].x,
                                                    m_idxStaff)
                    : m_pVProfile->get_min_profile_points(m_points[ImoBezierInfo::k_start].x,
                                                    m_points[ImoBezierInfo::k_end].x,
                                                    m_idxStaff) );

    //remove intermediate no-peak points
    size_t iMax = data.size() + 1;
    while (data.size() > 2 && data.size() < iMax)
    {
        size_t iPrev = 0;
        size_t iCur = 1;
        iMax = data.size();
        for (size_t i=2; i < iMax; ++i)
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

    //add space between reference points and slur points and shift points to center
    //(approximately) on noteheads
    if (data.size() > 0)
    {
        LUnits space = tenths_to_logical(5.0f);
        LUnits shift = m_points[ImoBezierInfo::k_start].x - data.front().x;
        vector<UPoint>::iterator it;
        for (it=data.begin(); it < data.end(); ++it)
        {
            (*it).x += shift;
            (*it).y += (m_fSlurBelow ? space : -space);
        }

        //replace start and end points
        data.front().y = m_points[ImoBezierInfo::k_start].y;
        data.back().x = m_points[ImoBezierInfo::k_end].x;
        data.back().y = m_points[ImoBezierInfo::k_end].y;
    }

    return data;
}

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
    m_uPrologWidth += width;
}


}  //namespace lomse
