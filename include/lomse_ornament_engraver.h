//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ORNAMENT_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_ORNAMENT_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoOrnament;
class GmoShape;
class ScoreMeter;
class GmoShapeOrnament;

//---------------------------------------------------------------------------------------
class OrnamentEngraver : public AuxObjEngraver
{
protected:
    ImoOrnament* m_pOrnament;
    int m_placement;
    bool m_fAbove;
    GmoShape* m_pParentShape;
    GmoShapeOrnament* m_pOrnamentShape;

public:
    OrnamentEngraver(const EngraverContext& ctx);
    ~OrnamentEngraver() {}

    GmoShapeOrnament* create_shape(ImoOrnament* pOrnament, UPoint pos,
                                   Color color=Color(0,0,0),
                                   GmoShape* pParentShape=nullptr);

protected:
    bool determine_if_above();
    UPoint compute_location(UPoint pos);
    void center_on_parent();
    void add_voice();
    int find_glyph();

};


}   //namespace lomse

#endif    // __LOMSE_ORNAMENT_ENGRAVER_H__

