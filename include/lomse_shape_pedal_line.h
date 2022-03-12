//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_PEDAL_H__
#define __LOMSE_SHAPE_PEDAL_H__

#include "lomse_basic.h"
#include "lomse_drawer.h"
#include "lomse_shape_base.h"

#include <vector>

namespace lomse
{

//forward declarations
class ImoObj;


//---------------------------------------------------------------------------------------
class GmoShapePedalLine : public GmoCompositeShape
{
protected:
    struct LineGap {
        LUnits xStart;
        LUnits xEnd;
    };

    std::vector<LineGap> m_lineGaps;

    LUnits m_xLineStart;
    LUnits m_yLineStart;
    LUnits m_yLineEnd;
    LUnits m_uLineThick;
    bool m_fStartCorner;
    bool m_fEndCorner;

public:
    GmoShapePedalLine(ImoObj* pCreatorImo, ShapeId idx, Color color);

    void set_layout_data(LUnits xStart, LUnits xEnd, LUnits yStart, LUnits yEnd,
                         LUnits uLineThick, bool fStartCorner, bool fEndCorner);
    void add_line_gap(LUnits xStart, LUnits xEnd);

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_PEDAL_H__

