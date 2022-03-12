//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

