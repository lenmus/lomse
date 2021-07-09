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

#ifndef __LOMSE_BARLINE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_BARLINE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoBarline;
class InstrumentEngraver;
class GmoBoxSliceInstr;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class BarlineEngraver : public Engraver
{
public:
    BarlineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                    int iInstr=0, InstrumentEngraver* pInstrEngrv = nullptr);
    BarlineEngraver(LibraryScope& libraryScope);
    ~BarlineEngraver() {}

    GmoShape* create_shape(ImoBarline* pBarline, LUnits xPos, LUnits yTop,
                           LUnits yBottom, Color color=Color(0,0,0));
    GmoShape* create_system_barline_shape(ImoObj* pCreatorImo, LUnits xPos,
                                          LUnits yTop, LUnits yBottom,
                                          Color color=Color(0,0,0));
    GmoShape* create_tool_dragged_shape(int barType);
    UPoint get_drag_offset();

protected:
    GmoShape* m_pBarlineShape;
    InstrumentEngraver* m_pInstrEngrv;

};


}   //namespace lomse

#endif    // __LOMSE_BARLINE_ENGRAVER_H__

