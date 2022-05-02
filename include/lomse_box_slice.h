//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_BOX_SLICE_H__        //to avoid nested includes
#define __LOMSE_BOX_SLICE_H__

#include "lomse_basic.h"
#include "lomse_gm_basic.h"

namespace lomse
{

//forward declarations
class GmoBoxSystem;
class GmoBoxSliceInstr;
class GmoBoxSliceStaff;
class ImoInstrument;
class SystemLayouter;


//---------------------------------------------------------------------------------------
/**  GmoBoxSlice is a container for all boxes and shapes that compose a system column.
*/
class GmoBoxSlice : public GmoBox
{
protected:

public:
    GmoBoxSlice(int nAbsMeasure, ImoObj* pCreatorImo);
    ~GmoBoxSlice();

    GmoBoxSliceInstr* add_box_for_instrument(ImoInstrument* pInstr, int idxStaff);
    GmoBoxSliceInstr* find_instr_slice_at(LUnits x, LUnits y);
    GmoBoxSystem* get_system_box();
    GmoBoxSliceInstr* get_instr_slice(int iInstr);

    //helpers for layout
    /**  Move boxes and shapes to theirs final 'y' positions. */
    void reposition_slices_and_shapes(const std::vector<LUnits>& yOrgShifts,
                                      const std::vector<LUnits>& heights,
                                      const std::vector<LUnits>& barlinesHeight,
                                      const std::vector<std::vector<LUnits>>& relStaffTopPositions,
                                      LUnits bottomMarginIncr,
                                      SystemLayouter* pSysLayouter);
    GmoBoxSliceStaff* get_slice_staff_for(int iInstr, int iStaff);
    void reduce_last_instrument_height(LUnits space);

};


}   //namespace lomse

#endif    // __LOMSE_BOX_SLICE_H__
