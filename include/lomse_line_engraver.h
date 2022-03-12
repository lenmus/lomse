//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_LINE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_LINE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoScoreLine;
class GmoShape;
class ScoreMeter;
class GmoShapeLine;

//---------------------------------------------------------------------------------------
class LineEngraver : public Engraver
{
public:
    LineEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 int iInstr, int iStaff);
    ~LineEngraver() {}

    GmoShapeLine* create_shape(ImoScoreLine* pLine, UPoint pos);

    //dbg
    GmoShapeLine* dbg_create_shape(UPoint start, UPoint end, Color=Color(255,0,0),
                                   LUnits lineWidth=5.0f);
};


}   //namespace lomse

#endif    // __LOMSE_LINE_ENGRAVER_H__

