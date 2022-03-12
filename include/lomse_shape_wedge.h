//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_WEDGE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_WEDGE_H__

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


//---------------------------------------------------------------------------------------
class GmoShapeWedge : public GmoSimpleShape
{
protected:
    LUnits  m_thickness;

    //wedge points, relative to m_origin
    LUnits m_xTopStart;
    LUnits m_yTopStart;
    LUnits m_xTopEnd;
    LUnits m_yTopEnd;
    LUnits m_xBottomStart;
    LUnits m_yBottomStart;
    LUnits m_xBottomEnd;
    LUnits m_yBottomEnd;
    LUnits m_yBaseline;

    int     m_niente;
    LUnits  m_radiusNiente;

public:
    GmoShapeWedge(ImoObj* pCreatorImo, ShapeId idx, UPoint points[], LUnits thickness,
                  Color color, int niente, LUnits radius, LUnits yBaseline);
    virtual ~GmoShapeWedge();

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
    LUnits get_baseline_y() const override;

    //construction
    enum {
        k_niente_at_start = 0,
        k_niente_at_end,
        k_no_niente,
    };

    //support for handlers
    int get_num_handlers() override;
    UPoint get_handler_point(int i) override;
    void on_handler_dragged(int iHandler, UPoint newPos) override;
    void on_end_of_handler_drag(int iHandler, UPoint newPos) override;

protected:
    void save_points_and_compute_bounds(UPoint* points);

};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_WEDGE_H__

