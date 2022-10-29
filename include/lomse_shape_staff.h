//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_STAFF_H__        //to avoid nested includes
#define __LOMSE_SHAPE_STAFF_H__

#include "lomse_shape_base.h"

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class ImoStaffInfo;


//---------------------------------------------------------------------------------------
/** %GmoShapeStaff represents the staff lines for an staff in a system. Its bounding box
    origin is at top-left of upper most staff line. The bottom of its bounding box is
    the lower most staff line.
*/
class GmoShapeStaff : public GmoSimpleShape
{
protected:
    ImoStaffInfo* m_pStaff;
	int m_iStaff;			    //num of staff in the instrument (0..n-1)
    LUnits m_lineThickness;

public:

    ///@cond INTERNALS
    //excluded from public API. Only for internal use.
    GmoShapeStaff(ImoObj* pCreatorImo, ShapeId idx, ImoStaffInfo* m_pStaff, int iStaff,
                  LUnits width, Color color);
    ~GmoShapeStaff();

    ///@endcond


///@cond INTERNALS
//excluded from public API. Only for internal use.

	//implementation of pure virtual methods in base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
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

    //other
    int line_space_at(LUnits yPos);

    //API oriented
    LUnits get_staff_line_spacing();
    int get_staff_num_lines();
    LUnits get_staff_margin();


//protected:
//    //temporary data to be used when mouse tool moving over the staff
//    int         m_nOldSteps;		//to clear leger lines while dragging
//    LUnits    m_uxOldPos;
//    int         m_nPosOnStaff;		//line/space on staff on which this note is placed

///@endcond

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_STAFF_H__

