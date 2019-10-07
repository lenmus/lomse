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

namespace lomse
{

//forward declarations
class GmoShape;
class GmoBox;


/*---------------------------------------------------------------------------------------
	VerticalProfile is responsible for maintaining and managing the information about
	current max & min vertical positions ocupied in each staff of a system, as the
	system is being engraved.

	The profile is, basically, two vectors per staff containing, respectively, the max.
	and min. vertical positions per horizontal resolution unit (cell).

	Vertical positions are referred to the top line of each staff.

	Staff lines are never part of the profile, so that they can be taken or not into
	account, depending on the needs.
*/
class VerticalProfile
{
protected:
    int m_numStaves;        //in system 0..n-1
    int m_numCells;         //all staves have the same number of cells
    LUnits m_xStart;        //sytem left side
    LUnits m_xEnd;          //system right side
    LUnits m_cellWidth;

    typedef std::vector<LUnits> CellsRow;   //cells for one staff
    std::vector<CellsRow*> m_yMax;          //ptrs. to max cells vector for each staff
    std::vector<CellsRow*> m_yMin;          //ptrs. to min cells vector for each staff

	std::vector<LUnits> m_yStaffTop;        //top line position for each staff
	std::vector<LUnits> m_yStaffBottom;     //bottom line position for each staff

public:
    VerticalProfile(LUnits xStart, LUnits xEnd, LUnits cellWidth, int numStaves);
    virtual ~VerticalProfile();

    void initialize(int idxStaff, LUnits yTop, LUnits yBottom);
    LUnits get_max_cell(int iCell, int idxStaff);
    LUnits get_min_cell(int iCell, int idxStaff);
    void update(GmoShape* pShape, int idxStaff);
    LUnits get_max_for(LUnits xStart, LUnits xEnd, int idxStaff);
    LUnits get_min_for(LUnits xStart, LUnits xEnd, int idxStaff);

    //debug
    void dbg_add_vertical_profile_shapes(GmoBox* pBoxSystem);

protected:
    int cell_index(LUnits xPos);       //floor
    void update_shape(GmoShape* pShape, int idxStaff);

    //debug
    GmoShape* dbg_generate_shape(bool fMax, int idxStaff);

};


}   //namespace lomse

#endif      //__LOMSE_VERTICAL_PROFILE_H__
