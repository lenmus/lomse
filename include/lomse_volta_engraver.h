//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_VOLTA_BRACKET_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_VOLTA_BRACKET_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoVoltaBracket;
class ImoBarline;
class GmoShapeVoltaBracket;
class GmoShapeBarline;
class ScoreMeter;
class InstrumentEngraver;
class ImoStyle;

//---------------------------------------------------------------------------------------
class VoltaBracketEngraver : public RelObjEngraver
{
protected:
    int m_numShapes = 0;
    bool m_fFirstShapeAtSystemStart = false;
    ImoVoltaBracket* m_pVolta = nullptr;
    ImoStyle* m_pStyle = nullptr;
    ImoBarline* m_pStartBarline = nullptr;
    ImoBarline* m_pStopBarline = nullptr;
    GmoShapeBarline* m_pStartBarlineShape = nullptr;
    GmoShapeBarline* m_pStopBarlineShape = nullptr;

public:
    VoltaBracketEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);

    void set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    void set_end_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;

    //RelObjEngraver mandatory overrides
    GmoShape* create_first_or_intermediate_shape(const RelObjEngravingContext& ctx) override;
    GmoShape* create_last_shape(const RelObjEngravingContext& ctx) override;

protected:
    inline bool is_end_point_set() { return m_pStopBarline != nullptr; }
    GmoShape* create_single_shape();
    GmoShape* create_first_shape();
    GmoShape* create_intermediate_shape();
    GmoShape* create_final_shape();

    void set_shape_details(GmoShapeVoltaBracket* pShape, EShapeType shapeType);
};


}   //namespace lomse

#endif    // __LOMSE_VOLTA_BRACKET_ENGRAVER_H__

