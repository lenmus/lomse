//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
class BarlineEngraver : public StaffObjEngraver
{
public:
    BarlineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                    int iInstr=0, InstrumentEngraver* pInstrEngrv = nullptr);
    BarlineEngraver(LibraryScope& libraryScope);
    ~BarlineEngraver() {}

    GmoShape* create_shape(ImoBarline* pBarline, LUnits xPos, LUnits yTop,
                           LUnits yBottom, Color color=Color(0,0,0));
    GmoShape* create_systemic_barline_shape(ImoObj* pCreatorImo, ShapeId idx,
                                            LUnits xPos, LUnits yTop, LUnits yBottom,
                                            Color color=Color(0,0,0));
    GmoShape* create_tool_dragged_shape(int barType);
    UPoint get_drag_offset();

protected:
    GmoShape* m_pBarlineShape;
    InstrumentEngraver* m_pInstrEngrv;

};


}   //namespace lomse

#endif    // __LOMSE_BARLINE_ENGRAVER_H__

