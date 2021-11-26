//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#ifndef __LOMSE_CLEF_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_CLEF_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_score_enums.h"

namespace lomse
{

//forward declarations
class ImoClef;
class GmoBoxSliceInstr;
class GmoShape;
class ScoreMeter;

//---------------------------------------------------------------------------------------
class ClefEngraver : public StaffObjEngraver
{
protected:
    int m_nClefType;
    int m_symbolSize;
    int m_iGlyph;

public:
    ClefEngraver(LibraryScope& libraryScope);
    ClefEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter, int iInstr,
                 int iStaff);
    ~ClefEngraver() {}

    GmoShape* create_shape(ImoClef* pCreatorImo, UPoint uPos, int clefType,
                           int m_symbolSize=k_size_full, Color color=Color(0,0,0));

    GmoShape* create_tool_dragged_shape(int clefType);
    UPoint get_drag_offset();


protected:
    int find_glyph(int clefType);
    double determine_font_size() override;
    Tenths get_glyph_offset();

    GmoShape* m_pClefShape;
};


}   //namespace lomse

#endif    // __LOMSE_CLEF_ENGRAVER_H__

