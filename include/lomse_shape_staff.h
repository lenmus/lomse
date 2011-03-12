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

#ifndef __LOMSE_SHAPE_STAFF_H__        //to avoid nested includes
#define __LOMSE_SHAPE_STAFF_H__

#include "lomse_shape_base.h"

namespace lomse
{

//forward declarations
class ImoStaffInfo;


//---------------------------------------------------------------------------------------
class GmoShapeStaff : public GmoSimpleShape
{
protected:
    ImoStaffInfo* m_pStaff;
	int m_iStaff;			    //num of staff in the instrument (0..n-1)
    LUnits m_lineThickness;

public:
    GmoShapeStaff(ImoObj* pCreatorImo, int idx, ImoStaffInfo* m_pStaff, int iStaff,
                  LUnits width, Color color);
	~GmoShapeStaff();

//	//implementation of pure virtual methods in base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
//    void Shift(LUnits xIncr, LUnits yIncr);

	//ownership and related info
	inline int get_num_staff() { return m_iStaff; }

    //info
    inline LUnits get_line_thickness() { return m_lineThickness; }

//    //adding notes/rest with mouse
//    UPoint OnMouseStartMoving(lmPaper* pPaper, const UPoint& uPos);
//    UPoint OnMouseMoving(lmPaper* pPaper, const UPoint& uPos);
//	void OnMouseEndMoving(lmPaper* pPaper, UPoint uPagePos);
//
//
//    //other
//    int GetLineSpace(LUnits uyPos);
//    lmVStaff* GetOwnerVStaff();
//
//protected:
//    //temporary data to be used when mouse tool moving over the staff
//    int         m_nOldSteps;		//to clear leger lines while dragging
//    LUnits    m_uxOldPos;
//    int         m_nPosOnStaff;		//line/space on staff on which this note is placed

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_STAFF_H__

