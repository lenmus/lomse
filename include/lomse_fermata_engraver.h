//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_FERMATA_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_FERMATA_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoFermata;
class GmoShape;
class ScoreMeter;
class GmoShapeFermata;

//---------------------------------------------------------------------------------------
class FermataEngraver : public AuxObjEngraver
{
protected:
    ImoFermata* m_pFermata;
    int m_placement;
    bool m_fAbove;
    GmoShape* m_pParentShape;
    GmoShapeFermata* m_pFermataShape;

public:
    FermataEngraver(const EngraverContext& ctx);
    ~FermataEngraver() {}

    GmoShapeFermata* create_shape(ImoFermata* pFermata, UPoint pos,
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

#endif    // __LOMSE_FERMATA_ENGRAVER_H__

