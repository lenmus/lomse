//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_BOX_SLICE_INSTR_H__
#define __LOMSE_BOX_SLICE_INSTR_H__

#include "lomse_gm_basic.h"

namespace lomse
{

//forward declarations
class ImoInstrument;
class GmoBoxSliceStaff;
class SystemLayouter;


//---------------------------------------------------------------------------------------
/** Class GmoBoxSliceInstr represents a column (measure) of an instrument. It is
    a container for the GmoBoxSliceStaff objects that contain the shapes for the slice.
*/
class GmoBoxSliceInstr : public GmoBox
{
private:
    int m_idxStaff;     //for first staff in this instrument

public:
    GmoBoxSliceInstr(ImoInstrument* pInstr, int idxStaff);
    ~GmoBoxSliceInstr();

    GmoBoxSystem* get_system_box();

    void add_shape(GmoShape* pShape, int layer, int iStaff);
    GmoShape* find_staffobj_shape_before(LUnits x);
    GmoShape* find_staffobj_shape_after(LUnits x);

    //helpers for layout
    /**  Move boxes and shapes to theirs final 'y' positions. */
    void reposition_slices_and_shapes(const std::vector<LUnits>& yOrgShifts,
                                      const std::vector<LUnits>& heights,
                                      LUnits barlinesHeight,
                                      const std::vector<LUnits>& relStaffTopPositions,
                                      SystemLayouter* pSysLayouter);
    GmoBoxSliceStaff* get_slice_staff_for(int iStaff);

};


//---------------------------------------------------------------------------------------
/** Class GmoBoxSliceStaff represents one staff in a SliceInstr. It is a container
    for the shapes associated to a staff, to simplify tasks requiring to access the
    shapes for one staff, such as moving them when the staff is moved.

    The bounding box is not relevant for any task, so it is not computed.
*/
class GmoBoxSliceStaff : public GmoBox
{
private:
    int m_idxStaff;

public:
    GmoBoxSliceStaff(ImoInstrument* pInstr, int idxStaff);
    ~GmoBoxSliceStaff();

    GmoShape* find_staffobj_shape_before(LUnits x);
    GmoShape* find_staffobj_shape_after(LUnits x);

    //helpers for layout
    /**  Move shapes to theirs final 'y' positions and increment barlines height. */
    void reposition_shapes(const vector<LUnits>& yShifts, LUnits barlinesHeight,
                           const std::vector<LUnits>& relStaffTopPositions,
                           SystemLayouter* pSysLayouter, int staff);

protected:
    void dump(ostream& outStream, int level) override;

};


}   //namespace lomse

#endif      //__LOMSE_BOX_SLICE_INSTR_H__
