//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_OCTAVE_SHIFT_H__        //to avoid nested includes
#define __LOMSE_SHAPE_OCTAVE_SHIFT_H__

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
class GmoShapeOctaveShift : public GmoCompositeShape
{
protected:
    bool m_fTwoLines;
    bool m_fEndCorner;
    LUnits m_xLineStart;
    LUnits m_yLineStart;
    LUnits m_yLineEnd;
    LUnits m_uLineThick;

public:
    GmoShapeOctaveShift(ImoObj* pCreatorImo, ShapeId idx, Color color);
    virtual ~GmoShapeOctaveShift();

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    void set_layout_data(LUnits xStart, LUnits xEnd, LUnits yStart, LUnits yEnd,
                         LUnits uLineThick, bool fEndCorner);

    //support for handlers
    int get_num_handlers() override;
    UPoint get_handler_point(int i) override;
    void on_handler_dragged(int iHandler, UPoint newPos) override;
    void on_end_of_handler_drag(int iHandler, UPoint newPos) override;

protected:

};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_OCTAVE_SHIFT_H__

