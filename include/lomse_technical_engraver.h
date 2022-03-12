//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TECHNICAL_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_TECHNICAL_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoTechnical;
class GmoShape;
class ScoreMeter;
class GmoShapeTechnical;

//---------------------------------------------------------------------------------------
class TechnicalEngraver : public AuxObjEngraver
{
protected:
    ImoTechnical* m_pTechnical;
    int m_placement;
    bool m_fAbove;
    GmoShape* m_pParentShape;
    GmoShapeTechnical* m_pTechnicalShape;

public:
    TechnicalEngraver(const EngraverContext& ctx);

    GmoShapeTechnical* create_shape(ImoTechnical* pTechnical, UPoint pos,
                                    Color color=Color(0,0,0),
                                    GmoShape* pParentShape=nullptr);

protected:
    bool determine_if_above();
    UPoint compute_location(UPoint pos);
    void center_on_parent();
    void add_voice();
    int find_glyph();
    bool must_be_placed_outside_staff();

};


}   //namespace lomse

#endif    // __LOMSE_TECHNICAL_ENGRAVER_H__

