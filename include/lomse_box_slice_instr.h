//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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


//---------------------------------------------------------------------------------------
// Class GmoBoxSliceInstr represents a part (column, measure) of an instrument.
class GmoBoxSliceInstr : public GmoBox
{
private:

public:
    GmoBoxSliceInstr(ImoInstrument* pInstr);
    ~GmoBoxSliceInstr();
};


//---------------------------------------------------------------------------------------
// Class GmoBoxSliceStaff represents one staff in a SliceInstr
class GmoBoxSliceStaff : public GmoBox
{
private:

public:
    GmoBoxSliceStaff(ImoInstrument* pInstr);
    ~GmoBoxSliceStaff();
};


}   //namespace lomse

#endif      //__LOMSE_BOX_SLICE_INSTR_H__
