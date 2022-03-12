//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_DYNAMICS_MARK_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_DYNAMICS_MARK_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoDynamicsMark;
class GmoShape;
class ScoreMeter;
class GmoShapeDynamicsMark;
class VerticalProfile;

//---------------------------------------------------------------------------------------
class DynamicsMarkEngraver : public AuxObjEngraver
{
protected:
    ImoDynamicsMark* m_pDynamicsMark;
    int m_placement;
    bool m_fAbove;
    GmoShape* m_pParentShape;
    GmoShapeDynamicsMark* m_pDynamicsMarkShape;

public:
    DynamicsMarkEngraver(const EngraverContext& ctx);
    ~DynamicsMarkEngraver() {}

    GmoShapeDynamicsMark* create_shape(ImoDynamicsMark* pDynamicsMark, UPoint pos,
                                       Color color=Color(0,0,0),
                                       GmoShape* pParentShape=nullptr);

protected:
    bool determine_if_above();
    UPoint compute_location(UPoint pos);
    void center_on_parent();
    void add_voice();
    int find_glyph();
    void shift_shape_if_collision();

};


}   //namespace lomse

#endif    // __LOMSE_DYNAMICS_MARK_ENGRAVER_H__

