//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SEGNO_CODA_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_SEGNO_CODA_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoSymbolRepetitionMark;
class GmoShape;
class ScoreMeter;
class GmoShapeCodaSegno;

//---------------------------------------------------------------------------------------
class CodaSegnoEngraver : public AuxObjEngraver
{
protected:
    ImoSymbolRepetitionMark* m_pRepetitionMark;
    GmoShape* m_pParentShape;
    GmoShapeCodaSegno* m_pCodaSegnoShape;

public:
    CodaSegnoEngraver(const EngraverContext& ctx);

    GmoShapeCodaSegno* create_shape(ImoSymbolRepetitionMark* pRepetitionMark, UPoint pos,
                                    Color color=Color(0,0,0),
                                    GmoShape* pParentShape=nullptr);

protected:
    UPoint compute_location(UPoint pos);
    void center_on_parent();
    int find_glyph();

};


}   //namespace lomse

#endif    // __LOMSE_SEGNO_CODA_ENGRAVER_H__

