//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ARTICULATION_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ARTICULATION_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoArticulation;
class GmoShape;
class ScoreMeter;
class GmoShapeArticulation;
class VerticalProfile;

//---------------------------------------------------------------------------------------
class ArticulationEngraver : public AuxObjEngraver
{
protected:
    ImoArticulation* m_pArticulation;
    int m_placement;
    bool m_fAbove;
    bool m_fEnableShiftWhenCollision;
    GmoShape* m_pParentShape;
    GmoShapeArticulation* m_pArticulationShape;

public:
    ArticulationEngraver(const EngraverContext& ctx);
    ~ArticulationEngraver() {}

    GmoShapeArticulation* create_shape(ImoArticulation* pArticulation, UPoint pos,
                                       Color color=Color(0,0,0),
                                       GmoShape* pParentShape=nullptr);

protected:
    bool determine_if_above();
    UPoint compute_location(UPoint pos);
    void center_on_parent();
    void add_voice();
    int find_glyph();
    bool must_be_placed_outside_staff();
    bool is_accent_articulation();
    void shift_shape_if_collision();

};


}   //namespace lomse

#endif    // __LOMSE_ARTICULATION_ENGRAVER_H__

