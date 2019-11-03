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

#ifndef __LOMSE_VERTICAL_PROFILE_H__
#define __LOMSE_VERTICAL_PROFILE_H__

#include "lomse_basic.h"

#include <vector>
#include <list>

namespace lomse
{

//forward declarations
class GmoShape;
class GmoBox;

#define LOMSE_PAPER_LOWER_LIMIT   -100000000000000.0f  //any impossible low value
#define LOMSE_PAPER_UPPER_LIMIT    100000000000000.0f  //any impossible high value

//---------------------------------------------------------------------------------------
/**	Helper class to store data for a point in the vertical profile
*/
class VProfilePoint
{
public:
    LUnits x;
    LUnits y;
    GmoShape* shape;

    VProfilePoint(LUnits xp, LUnits yp, GmoShape* sp) : x(xp), y(yp), shape(sp) {}
    bool operator ==(const VProfilePoint &p) const { return x==p.x && y==p.y && shape==p.shape; }
};

//to simplify writing code
typedef list<VProfilePoint>::iterator  PointsIterator;

//---------------------------------------------------------------------------------------
/**	VerticalProfile is responsible for maintaining and managing the information about
	current max & min vertical positions ocupied in each staff of a system, as the
	system is being engraved.

	The profile is, basically, two vectors per staff containing, respectively, the shapes
	that define the max. and min. vertical positions along the x axis.

	Wen no shape occupies the space, the profile assigns to this space as max and min
	values an upper and lower value of ten time the staff height. This is, the profile
	height is 21 times the staff height.

	Staff lines are never part of the profile. But %VerticalProfile stores information
	about the staff to facilite measurements to Engraver and Layouter objects.
*/
class VerticalProfile
{
protected:
    int m_numStaves;        //in system 0..n-1
    LUnits m_xStart;        //sytem left side
    LUnits m_xEnd;          //system right side

    std::vector<LUnits> m_yMin; //minimum reached value, for each staff
    std::vector<LUnits> m_yMax; //maximum reached value, for each staff

	std::vector<LUnits> m_yStaffTop;        //top line position for each staff
	std::vector<LUnits> m_yStaffBottom;     //bottom line position for each staff

    typedef std::list<VProfilePoint> PointsRow;  //data for a profile change, for one staff
	std::vector<PointsRow*> m_xMax;         //ptrs. to max x pos vector for each staff
	std::vector<PointsRow*> m_xMin;         //ptrs. to min x pos vector for each staff

public:
    VerticalProfile(LUnits xStart, LUnits xEnd, int numStaves);
    virtual ~VerticalProfile();

    void initialize(int idxStaff, LUnits yStaffTop, LUnits yStaffBottom);
    void update(GmoShape* pShape, int idxStaff);

    /** Return max/min reached value for staff idxStaff. */
    LUnits get_min_limit(int idxStaff) { return m_yMin[idxStaff]; }
    LUnits get_max_limit(int idxStaff) { return m_yMax[idxStaff]; }

    /** Return max/min reached value for staff idxStaff in the
        interval [xStart, xEnd], and the shape responsible for that value. */
    std::pair<LUnits, GmoShape*> get_max_for(LUnits xStart, LUnits xEnd, int idxStaff);
    std::pair<LUnits, GmoShape*> get_min_for(LUnits xStart, LUnits xEnd, int idxStaff);

    /** Return minimun distance between max profile for staff idxStaff-1 and
        min profile for staff idxStaff. A negative distance means that
        the profiles overlap. */
    LUnits get_staves_distance(int idxStaff);


    //debug
    void dbg_add_vertical_profile_shapes(GmoBox* pBoxSystem);
    std::string dump_max(int idxStaff);
    std::string dump_min(int idxStaff);

protected:
    void update_profile(list<VProfilePoint>* pPoints, LUnits yPos, bool fMax,
                        LUnits xLeft, LUnits xRight, GmoShape* pShape);
    PointsIterator locate_insertion_point(std::list<VProfilePoint>* pPoints, LUnits xLeft);
    void update_point(std::list<VProfilePoint>* pPointsMin, LUnits xPos,
                      LUnits yPos, GmoShape* pShape, PointsIterator itNext);


    void update_shape(GmoShape* pShape, int idxStaff);

    //debug
    GmoShape* dbg_generate_shape(bool fMax, int idxStaff);
    std::string dump(list<VProfilePoint>* pPoints);

};


}   //namespace lomse

#endif      //__LOMSE_VERTICAL_PROFILE_H__
