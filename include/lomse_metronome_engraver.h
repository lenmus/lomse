//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_METRONOME_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_METRONOME_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class GmoShapeMetronomeMark;
class ImoObj;
class GmoShape;
class ScoreMeter;
class ImoMetronomeMark;

//---------------------------------------------------------------------------------------
class MetronomeMarkEngraver : public AuxObjEngraver
{
protected:
    GmoShapeMetronomeMark* m_pMainShape;
    UPoint m_uPos;
    double m_fontSize;
    ImoMetronomeMark* m_pCreatorImo;
    Color m_color;

public:
    MetronomeMarkEngraver(const EngraverContext& ctx);
    ~MetronomeMarkEngraver() {}

    GmoShape* create_shape(ImoMetronomeMark* pImo, UPoint uPos, Color color=Color(0,0,0));

protected:
    void create_main_container_shape();
    GmoShape* create_shape_mm_value();
    GmoShape* create_shape_note_note();
    GmoShape* create_shape_note_value();
    void create_text_shape(const string& text);
    void create_symbol_shape(int noteType, int dots);
    int select_glyph(int noteType);

};


}   //namespace lomse

#endif    // __LOMSE_METRONOME_ENGRAVER_H__

