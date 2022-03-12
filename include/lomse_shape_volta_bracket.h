//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_VOLTA_BRACKET_H__        //to avoid nested includes
#define __LOMSE_SHAPE_VOLTA_BRACKET_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_shape_base.h"
#include "lomse_shape_text.h"
#include "lomse_internal_model.h"

#include "lomse_shape_base.h"
#include "lomse_vertex_source.h"
#include "agg_trans_affine.h"

namespace lomse
{

//forward declarations
class ImoObj;
class GmoShapeBarline;
class GmoShapeText;


//---------------------------------------------------------------------------------------
class GmoShapeVoltaBracket : public GmoCompositeShape
{
protected:
    GmoShapeBarline* m_pStopBarlineShape;
    GmoShapeText* m_pShapeText;
    bool m_fTwoBrackets;
    bool m_fStopJog;
	LUnits m_uJogLength;
    LUnits m_uLineThick;
    LUnits m_uStaffLeft;
    LUnits m_uStaffRight;
    LUnits m_uBracketDistance;

public:
    GmoShapeVoltaBracket(ImoObj* pCreatorImo, ShapeId idx, Color color);
    virtual ~GmoShapeVoltaBracket();

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;


    void set_layout_data(LUnits xStart, LUnits xEnd, LUnits yPos, LUnits uBracketDistance,
                         LUnits uJogLength, LUnits uLineThick, LUnits uLeftSpaceToText,
                         LUnits uTopSpaceToText, LUnits xStaffLeft, LUnits xStaffRight);

    void add_label(GmoShapeText* pShape);
    inline void enable_final_jog(bool value) { m_fStopJog = value; }
    inline void set_two_brackets() { m_fTwoBrackets = true; }

    //support for handlers
    int get_num_handlers() override;
    UPoint get_handler_point(int i) override;
    void on_handler_dragged(int iHandler, UPoint newPos) override;
    void on_end_of_handler_drag(int iHandler, UPoint newPos) override;

protected:
    void compute_bounds(LUnits xStart, LUnits xEnd, LUnits yPos);

};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_VOLTA_BRACKET_H__

