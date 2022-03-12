//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_FINGERING_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_FINGERING_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoFingering;
class GmoShape;
class ScoreMeter;
class GmoShapeFingeringContainer;

//---------------------------------------------------------------------------------------
class FingeringEngraver : public AuxObjEngraver
{
protected:
    ImoFingering* m_pFingering;
    int m_placement;
    bool m_fAbove;
    GmoShape* m_pParentShape;
    GmoShapeFingeringContainer* m_pMainShape;
    UPoint m_uPos;
    double m_fontSize;

public:
    FingeringEngraver(const EngraverContext& ctx);

    GmoShapeFingeringContainer* create_shape(ImoFingering* pFingering, UPoint pos,
                                             Color color=Color(0,0,0),
                                             GmoShape* pParentShape=nullptr);

protected:
    bool determine_if_above();
    void create_main_container_shape();
    void add_fingering_shapes();
    void compute_location();
    void center_on_parent();
    void add_voice();
    int find_glyph_single_char(const std::string& value);
    GmoShape* add_single_char_shape(GmoShape* pPrevShape, int iGlyph);
    GmoShape* add_substitution_shape(GmoShape* pPrevShape, int iGlyph);
    GmoShape* add_alternative_shape(int iGlyph);
    LUnits compute_position_for_glyph(GmoShape* pPrevShape);
    bool is_chord();
    void shift_shape_if_collision();

};


}   //namespace lomse

#endif    // __LOMSE_FINGERING_ENGRAVER_H__

