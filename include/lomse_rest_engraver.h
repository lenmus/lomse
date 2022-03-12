//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_REST_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_REST_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoRest;
class GmoShapeRest;
class GmoShapeRestGlyph;
class ScoreMeter;
class EngraversMap;
class GmoShapeBeam;
class VoiceRelatedShape;

//---------------------------------------------------------------------------------------
class RestEngraver : public StaffObjEngraver
{
protected:
    int m_restType;
    int m_numDots;
    int m_clefType;
    int m_octaveShift;
    ImoRest* m_pRest;
    Color m_color;
    double m_fontSize;

public:
    RestEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                 EngraversMap* pEngravers, int iInstr, int iStaff, int clefType,
                 int octaveShift);
    ~RestEngraver() {}

    GmoShapeRest* create_shape(ImoRest* pRest, UPoint uPos, Color color=Color(0,0,0));
    GmoShape* create_tool_dragged_shape(int restType, int dots);
    UPoint get_drag_offset();

protected:
    void determine_position();
    void create_main_shape();
    void add_shapes_for_dots_if_required();

    void add_voice(VoiceRelatedShape* pVRS);
    int find_glyph();
    LUnits get_glyph_offset(int iGlyph);
    LUnits add_dot_shape(LUnits x, LUnits y, Color color);
    Tenths get_offset_for_dot();
    int determine_pos_on_staff(int iGlyph);

    LUnits m_uxLeft, m_uyTop;       //current position
    int m_iGlyph;
    GmoShapeRest* m_pRestShape;
    GmoShapeRestGlyph* m_pRestGlyphShape;
};


}   //namespace lomse

#endif    // __LOMSE_REST_ENGRAVER_H__

