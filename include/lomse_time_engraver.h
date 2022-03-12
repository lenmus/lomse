//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TIME_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TIME_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class GmoShapeTimeSignature;
class ImoObj;
class GmoShape;
class ScoreMeter;
class ImoTimeSignature;

//---------------------------------------------------------------------------------------
class TimeEngraver : public StaffObjEngraver
{
protected:
    GmoShapeTimeSignature* m_pTimeShape;
    UPoint m_uPos;
    LUnits m_uTopWidth;
    LUnits m_uBottomWidth;
    GmoShape* m_pShapesTop[2];
    GmoShape* m_pShapesBottom[2];
    double m_fontSize;
    ImoTimeSignature* m_pCreatorImo;
    Color m_color;

public:
    TimeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 int iInstr, int iStaff);
    ~TimeEngraver() {}

    GmoShape* create_shape(ImoTimeSignature* pCreatorImo, UPoint uPos,
                           Color color=Color(0,0,0));

protected:
    GmoShape* create_symbol_shape(int iGlyph, ShapeId idx);
    GmoShape* create_shape_normal(UPoint uPos, int beats, int beat_type);

    void create_main_container_shape(UPoint uPos);
    void create_top_digits(UPoint uPos, int beats);
    void create_bottom_digits(UPoint uPos, int beat_type);
    void center_numbers();
    void add_all_shapes_to_container();
    void create_digits(int digits, GmoShape* pShape[]);
    GmoShape* create_digit(int digit);

};


}   //namespace lomse

#endif    // __LOMSE_TIME_ENGRAVER_H__

