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

    //helpers for layout
    /**  Move boxes and shapes to theirs final 'y' positions. */
    void reposition_slices_and_shapes(const std::vector<LUnits>& yOrgShifts,
                                      std::vector<LUnits>& heights,
                                      LUnits barlinesHeight,
                                      SystemLayouter* pSysLayouter);
    GmoBoxSliceStaff* get_slice_staff_for(int iStaff);

};


//---------------------------------------------------------------------------------------
/** Class GmoBoxSliceStaff represents one staff in a SliceInstr. It is a container
    for the shapes associated to a staff, to simplify tasks requiring to access the
    shapes for one staff, such as moving them when thet staff is moved.

    The bounding box is not relevant for any task, so it is not computed.
*/
class GmoBoxSliceStaff : public GmoBox
{
private:
    int m_idxStaff;

public:
    GmoBoxSliceStaff(ImoInstrument* pInstr, int idxStaff);
    ~GmoBoxSliceStaff();

    //helpers for layout
    /**  Move shapes to theirs final 'y' positions and increment barlines height. */
    void reposition_shapes(const vector<LUnits>& yShifts, LUnits barlinesHeight,
                           SystemLayouter* pSysLayouter);

protected:
    void dump(ostream& outStream, int level) override;

};


}   //namespace lomse

#endif      //__LOMSE_BOX_SLICE_INSTR_H__
