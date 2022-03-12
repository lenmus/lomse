//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_WEDGE_ENGRAVER_H__        //to avoid nested includes
#define __LOMSE_WEDGE_ENGRAVER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"

namespace lomse
{

//forward declarations
class ImoWedgeData;
class ImoWedge;
class ImoDirection;
class GmoShapeWedge;
class GmoShapeInvisible;
class ScoreMeter;
class InstrumentEngraver;
class VerticalProfile;

//---------------------------------------------------------------------------------------
class WedgeEngraver : public RelObjEngraver
{
protected:
    int m_numShapes = 0;
    ImoWedge* m_pWedge = nullptr;
    bool m_fWedgeAbove = false;
    bool m_fFirstShapeAtSystemStart = false;

    ImoDirection* m_pStartDirection = nullptr;
    ImoDirection* m_pEndDirection = nullptr;
    GmoShapeInvisible* m_pStartDirectionShape = nullptr;
    GmoShapeInvisible* m_pEndDirectionShape = nullptr;

    UPoint m_points[4];    //points for wedge
    LUnits m_yAlignBaseline = 0.0f; //baseline coordinate for aligning with dynamic marks

public:
    WedgeEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter);
    ~WedgeEngraver() {}

    void set_start_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;
    void set_end_staffobj(ImoRelObj* pRO, const AuxObjContext& aoc) override;

    //RelObjEngraver mandatory overrides
    GmoShape* create_first_or_intermediate_shape(const RelObjEngravingContext& ctx) override;
    GmoShape* create_last_shape(const RelObjEngravingContext& ctx) override;

protected:
    void decide_placement();
    GmoShape* create_first_shape();
    GmoShape* create_intermediate_shape();
    GmoShape* create_final_shape();

    void compute_shape_x_position(bool first);
    void compute_first_shape_position();
    void compute_intermediate_or_last_shape_position();

    LUnits determine_default_shape_position_left(bool first) const;
    LUnits determine_shape_position_left(bool first) const;
    LUnits determine_default_shape_position_right() const;
    LUnits determine_shape_position_right() const;
    LUnits determine_center_line_of_shape(LUnits startSpread, LUnits endSpread);
    //void add_user_displacements(int iWedge, UPoint* points);

};


}   //namespace lomse

#endif    // __LOMSE_WEDGE_ENGRAVER_H__

