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

#include "lomse_vertical_profile.h"

#include "lomse_gm_basic.h"

//for generating a debug shape
#include "lomse_shapes.h"
#include "lomse_shape_note.h"
#include "lomse_vertex_source.h"
#include "lomse_logger.h"

#include <sstream>
using namespace std;


namespace lomse
{

//=======================================================================================
// VerticalProfile implementation
//=======================================================================================
VerticalProfile::VerticalProfile(LUnits xStart, LUnits xEnd, int numStaves)
    : m_numStaves(numStaves)
    , m_xStart(xStart)
    , m_xEnd(xEnd)
{
	m_yMin.resize(m_numStaves, LOMSE_PAPER_UPPER_LIMIT);
	m_yMax.resize(m_numStaves, LOMSE_PAPER_LOWER_LIMIT);

	m_yStaffTop.resize(m_numStaves, 0.0f);
	m_yStaffBottom.resize(m_numStaves, 0.0f);

    m_xMax.resize(m_numStaves, nullptr);
    m_xMin.resize(m_numStaves, nullptr);
}

//---------------------------------------------------------------------------------------
VerticalProfile::~VerticalProfile()
{
    for (int idxStaff=0; idxStaff < m_numStaves; ++idxStaff)
    {
        delete m_xMax[idxStaff];
        delete m_xMin[idxStaff];
    }
}

//---------------------------------------------------------------------------------------
void VerticalProfile::initialize(int idxStaff, LUnits yStaffTop, LUnits yStaffBottom)
{
    m_yMin[idxStaff] = LOMSE_PAPER_UPPER_LIMIT;
    m_yMax[idxStaff] = LOMSE_PAPER_LOWER_LIMIT;

    m_yStaffTop[idxStaff] = yStaffTop;
    m_yStaffBottom[idxStaff] = yStaffBottom;

    PointsRow* pPoints = LOMSE_NEW PointsRow;
    m_xMax[idxStaff] = pPoints;
    pPoints->push_back( {m_xStart, LOMSE_PAPER_LOWER_LIMIT, nullptr} );
    pPoints->push_back( {m_xEnd, LOMSE_PAPER_LOWER_LIMIT, nullptr} );

    pPoints = LOMSE_NEW PointsRow;
    m_xMin[idxStaff] = pPoints;
    pPoints->push_back( {m_xStart, LOMSE_PAPER_UPPER_LIMIT, nullptr} );
    pPoints->push_back( {m_xEnd, LOMSE_PAPER_UPPER_LIMIT, nullptr} );
}

//---------------------------------------------------------------------------------------
void VerticalProfile::update(GmoShape* pShape, int idxStaff)
{
    //barlines are excluded because they could join two or more staves and this will
    //hide the true boundaries of each staff. also, barlines have no effect of
    //preventing collisions between auxiliary notations.
    if (pShape->is_shape_barline())
        return;

    //for notes, only notehead, stem and accidentals
    if (pShape->is_shape_note())
    {
        GmoShapeNote* pNote = dynamic_cast<GmoShapeNote*>(pShape);
        GmoShape* pNotehead = pNote->get_notehead_shape();
        update_shape(pNotehead, idxStaff);
        update_shape(pNote->get_accidentals_shape(), idxStaff);

        //for isolated notes, add stem and flag
        if (!pNote->is_in_chord() && !pNote->has_beam())
        {
            update_shape(pNote->get_stem_shape(), idxStaff);
            update_shape(pNote->get_flag_shape(), idxStaff);
            return;
        }

        //for chords accross two staves, only the fixed segment stem must be added.
        if (pNote->is_in_chord() && pNote->is_shape_chord_base_note())
        {
            GmoShapeNote* pFlagNote = static_cast<GmoShapeChordBaseNote*>(pNote)->get_flag_note();
            update_shape(pFlagNote->get_stem_shape(), idxStaff);
            update_shape(pFlagNote->get_flag_shape(), idxStaff);
            return;
        }

        //if arrives here, it is a beamed note.
        //for beamed groups, the stem does not contribute to VProfile. So do not add it.
        return;
    }

    //For beams, the beam is not added when cross-staff beam
    if (!pShape->add_to_vprofile())
        return;

    GmoCompositeShape* pCS = dynamic_cast<GmoCompositeShape*>(pShape);
    if (pCS)
    {
        list<GmoShape*>& shapes = pCS->get_components();
        list<GmoShape*>::iterator it;
        for (it=shapes.begin(); it != shapes.end(); ++it)
            update_shape(*it, idxStaff);
    }
    else
        update_shape(pShape, idxStaff);
}

//---------------------------------------------------------------------------------------
void VerticalProfile::update_shape(GmoShape* pShape, int idxStaff)
{
    if (pShape == nullptr || pShape->is_shape_invisible())
        return;

    LUnits xLeft = pShape->get_left();
    LUnits xRight = pShape->get_right();
    LUnits yTop = pShape->get_top();
    LUnits yBottom = pShape->get_bottom();

    if (xLeft < m_xStart || xRight > m_xEnd)
        return;

//    dbgLogger << "Update shape " << pShape->get_name() << ", yTop=" << yTop
//        << ", yBottom=" << yBottom << ", staff=" << idxStaff << endl;

    //update limits
    if (m_yMin[idxStaff] == LOMSE_PAPER_LOWER_LIMIT)
        m_yMin[idxStaff] = yTop;
    else
        m_yMin[idxStaff] = min(m_yMin[idxStaff], yTop);

    if (m_yMax[idxStaff] == LOMSE_PAPER_LOWER_LIMIT)
        m_yMax[idxStaff] = yBottom;
    else
        m_yMax[idxStaff] = max(m_yMax[idxStaff], yBottom);


    //update xPos and shapes, minimum profile
    list<VProfilePoint>* pPointsMin = m_xMin[idxStaff];
    update_profile(pPointsMin, yTop, false, xLeft, xRight, pShape);    //false -> minimum profile

    //update xPos and shapes, maximum profile
    list<VProfilePoint>* pPointsMax = m_xMax[idxStaff];
    update_profile(pPointsMax, yBottom, true, xLeft, xRight, pShape);  //true -> maximum profile
}

//---------------------------------------------------------------------------------------
void VerticalProfile::update_profile(list<VProfilePoint>* pPoints, LUnits yPos, bool fMax,
                                     LUnits xLeft, LUnits xRight, GmoShape* pShape)
{
//    cout << "Update " << (fMax ? "Max" : "Min") << " ----------------------------------------------------" << endl;

    PointsIterator itLeft = locate_insertion_point(pPoints, xLeft);
    PointsIterator itPrevLeft = (itLeft != pPoints->begin() ? std::prev(itLeft) : itLeft);
    VProfilePoint ptPrevLeft = *itPrevLeft;    //Data defining current level

    PointsIterator itRight = locate_insertion_point(pPoints, xRight);
    PointsIterator itPrevRight = (itRight != pPoints->begin() ? std::prev(itRight) : itRight);
    VProfilePoint ptPrevRight = *itPrevRight;


    //Insert/update point for left border of added shape
    if ((fMax && (yPos > ptPrevLeft.y)) || (!fMax && (yPos < ptPrevLeft.y)))
    {
        update_point(pPoints, xLeft, yPos, pShape, itLeft);
    }


    //remove or update intermediate points if necessary
    VProfilePoint ptRef = {xRight, yPos, pShape};   //left border of new added shape
//    cout << "Ref.point: xPos=" << ptRef.x << ", ptRef.y=" << ptRef.y
//         << ", ptRef.shape=" << (void*)(ptRef.shape) << endl;
    LUnits yPrev = (*std::prev(itLeft)).y;
    GmoShape* pPrevShape = (*std::prev(itLeft)).shape;
//    cout << "itLeft: xPos=" << (*itLeft).x << ", itRight: xPos=" << (*itRight).x << endl;
    while (itLeft != itRight)
    {
        VProfilePoint ptCur = *itLeft;
//        cout << "Cur.point: xPos=" << ptCur.x << ", ptCur.y=" << ptCur.y
//             << ", yPrev=" << yPrev << "--> ptRef.y=" << ptRef.y << endl;
        if ( (!fMax && (ptCur.y > ptRef.y)) || (fMax && (ptCur.y < ptRef.y)) )
        {
            if (yPrev == ptRef.y && pPrevShape == ptRef.shape)
            {
                //remove point
                itLeft = pPoints->erase(itLeft);
//                cout << "    Point removed. New itLeft: xPos=" << (*itLeft).x << endl;
            }
            else
            {
                //update point
                (*itLeft).y = ptRef.y;
                (*itLeft).shape = ptRef.shape;
//                cout << "    Point updated: xPos=" << ptCur.x << ", yPos=" << ptRef.y
//                     << ", shape=" << (void*)(ptRef.shape) << endl;
//
                yPrev = ptRef.y;
                pPrevShape = ptRef.shape;
                ++itLeft;
            }
        }
        else
        {
            //keep point as is
//            cout << "    Point kept" << endl;
            yPrev = (*itLeft).y;
            pPrevShape = (*itLeft).shape;
            ++itLeft;
        }
    }
//    cout << endl;

    //Insert/update point for right border of added shape
    if ((fMax && (yPos > ptPrevRight.y)) || (!fMax && (yPos < ptPrevRight.y)))
    {
        update_point(pPoints, xRight, ptPrevRight.y, ptPrevRight.shape, itRight);
    }
}

//---------------------------------------------------------------------------------------
void VerticalProfile::update_point(list<VProfilePoint>* pPoints, LUnits xPos,
                                   LUnits yPos, GmoShape* pShape, PointsIterator itNext)
{
    VProfilePoint ptNext = *itNext;    //Data for point greater or equal to xLeft

    //update minimum profile
    if (xPos == ptNext.x)
    {
        //replace point. But nothing to do as existing point either:
        //- is valid (this is the case for the right border of the new shape, or
        //- will be upated when dealing with intermediate points (left border of new shape)
    }
    else    //xPos < ptNext.x
    {
        //insert point
        pPoints->insert(itNext, VProfilePoint(xPos, yPos, pShape));
    }
}

//---------------------------------------------------------------------------------------
PointsIterator VerticalProfile::locate_insertion_point(list<VProfilePoint>* pPoints,
                                                       LUnits xPos)
{
    list<VProfilePoint>::iterator it;
    for (it = pPoints->begin(); it != pPoints->end(); ++it)
    {
        if ((*it).x >= xPos)
            break;
    }
    return it;
}

//---------------------------------------------------------------------------------------
std::pair<LUnits, GmoShape*> VerticalProfile::get_max_for(LUnits xStart, LUnits xEnd, int idxStaff)
{
    list<VProfilePoint>* pPoints = m_xMax[idxStaff];
    PointsIterator it = locate_insertion_point(pPoints, xStart);
    if (it != pPoints->begin())
        --it;
    LUnits yMax = (*it).y;
    GmoShape* pShape = (*it).shape;
    for (; it != pPoints->end() && (*it).x <= xEnd; ++it)
    {
        if (yMax <= (*it).y)
        {
            yMax = (*it).y;
            pShape = (*it).shape;
        }
    }
    return make_pair(yMax, pShape);
}

//---------------------------------------------------------------------------------------
std::pair<LUnits, GmoShape*> VerticalProfile::get_min_for(LUnits xStart, LUnits xEnd,
                                                          int idxStaff)
{
    list<VProfilePoint>* pPoints = m_xMin[idxStaff];
    PointsIterator it = locate_insertion_point(pPoints, xStart);
    if (it != pPoints->begin())
        --it;
    LUnits yMin = (*it).y;
    GmoShape* pShape = (*it).shape;
    for (; it != pPoints->end() && (*it).x <= xEnd; ++it)
    {
        if (yMin >= (*it).y)
        {
            yMin = (*it).y;
            pShape = (*it).shape;
        }
    }
    return make_pair(yMin, pShape);
}

//---------------------------------------------------------------------------------------
void VerticalProfile::dbg_add_vertical_profile_shapes(GmoBox* pBoxSystem)
{
    for (int idxStaff=0; idxStaff < m_numStaves; ++idxStaff)
    {
        pBoxSystem->add_shape( dbg_generate_shape(true, idxStaff), 0 );
        pBoxSystem->add_shape( dbg_generate_shape(false, idxStaff), 0 );
    }
}

//---------------------------------------------------------------------------------------
GmoShape* VerticalProfile::dbg_generate_shape(bool fMax, int idxStaff)
{
    GmoShapeDebug* pShape = LOMSE_NEW GmoShapeDebug(fMax ? Color(0,255,0,128)
                                                         : Color(255,0,0,128));

    LUnits yBase = (fMax ? get_max_limit(idxStaff) + 100.0f
                         : get_min_limit(idxStaff) - 100.0f);
    LUnits yInfinite = (fMax ? LOMSE_PAPER_LOWER_LIMIT : LOMSE_PAPER_UPPER_LIMIT);
    LUnits yFloor = (fMax ? yBase + 100.0f : yBase - 100.0f);

    LUnits xStart = m_xStart;
    LUnits yStart = yFloor;
    pShape->add_vertex('M', xStart, yStart);

    LUnits xLast = xStart;
    LUnits yLast = yStart;

    list<VProfilePoint>* pPoints = (fMax ? m_xMax[idxStaff] : m_xMin[idxStaff]);
    PointsIterator it;
    for (it=pPoints->begin(); it != pPoints->end(); ++it)
    {
        xLast = (*it).x;
        pShape->add_vertex('L', xLast, yLast);
        yLast = ((*it).y == yInfinite ? yBase : (*it).y);
        pShape->add_vertex('L', xLast, yLast);
    }
    pShape->add_vertex('L', xLast, yStart);
    pShape->add_vertex('Z',    0.0f,    0.0f);
    pShape->close_vertex_list();

    return pShape;
}

//---------------------------------------------------------------------------------------
string VerticalProfile::dump_min(int idxStaff)
{
    return dump(m_xMin[idxStaff]);
}

//---------------------------------------------------------------------------------------
string VerticalProfile::dump_max(int idxStaff)
{
    return dump(m_xMax[idxStaff]);
}

//---------------------------------------------------------------------------------------
string VerticalProfile::dump(list<VProfilePoint>* pPoints)
{
    stringstream msg;
    PointsIterator it;
    for (it=pPoints->begin(); it != pPoints->end(); ++it)
    {
        msg << "(" << (*it).x << ", " << (*it).y << "),";
    }
    return msg.str();
}

//---------------------------------------------------------------------------------------
LUnits VerticalProfile::get_staves_distance(int idxStaff)
{
    int idxPrev = idxStaff - 1;

    list<VProfilePoint>* pPointsPrev = m_xMax[idxPrev];
    list<VProfilePoint>* pPointsCur = m_xMin[idxStaff];
    PointsIterator itPrev = pPointsPrev->begin();
	LUnits xPrev = (*itPrev).x;
    LUnits yPrev = ((*itPrev).y == LOMSE_PAPER_LOWER_LIMIT ? m_yStaffBottom[idxPrev]
                                                           : (*itPrev).y);

    PointsIterator itCur = pPointsCur->begin();
	LUnits xCur = (*itCur).x;
    LUnits yCur = ((*itCur).y == LOMSE_PAPER_UPPER_LIMIT ? m_yStaffTop[idxStaff]
                                                         : (*itCur).y);
	LUnits distance = yCur - yPrev;

	while (itPrev != pPointsPrev->end() && itCur != pPointsCur->end())
	{
	    if (itPrev != pPointsPrev->end() && xPrev <= xCur)
        {
            yPrev = ((*itPrev).y == LOMSE_PAPER_LOWER_LIMIT ? m_yStaffBottom[idxPrev]
                                                            : (*itPrev).y);
	        ++itPrev;
	        if (itPrev != pPointsPrev->end())
                xPrev = (*itPrev).x;
        }
        else if (itCur != pPointsCur->end())
        {
            yCur = ((*itCur).y == LOMSE_PAPER_UPPER_LIMIT ? m_yStaffTop[idxStaff]
                                                          : (*itCur).y);
            ++itCur;
	        if (itCur != pPointsCur->end())
                xCur = (*itCur).x;
        }
        else
        {
            LOMSE_LOG_ERROR("Impossible case!");
            break;
        }
	    distance = min(distance, yCur - yPrev);
	}

    return distance;
}


}  //namespace lomse
