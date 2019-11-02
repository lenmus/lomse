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
    GmoBoxSystem* get_system_box();
    GmoBoxSliceInstr* get_instr_slice(int iInstr);

    //helpers for layout
    /**  Move boxes and shapes to theirs final 'y' positions. */
    void reposition_slices_and_shapes(const std::vector<LUnits>& yOrgShifts,
                                      std::vector<LUnits>& heights,
                                      vector<LUnits>& barlinesHeight,
                                      SystemLayouter* pSysLayouter);
    GmoBoxSliceStaff* get_slice_staff_for(int iInstr, int iStaff);

};


}   //namespace lomse

#endif    // __LOMSE_BOX_SLICE_H__
