//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_TUPLET_H__        //to avoid nested includes
#define __LOMSE_SHAPE_TUPLET_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_shape_base.h"
#include "lomse_shape_text.h"
#include "lomse_internal_model.h"

namespace lomse
{

//forward declarations
class ImoNoteRest;
class ImoObj;
class GmoShapeText;
class GmoShape;


//---------------------------------------------------------------------------------------
class GmoShapeTuplet : public GmoCompositeShape
{
protected:
    int m_design;
    GmoShapeText* m_pShapeText;
    GmoShape* m_pStartNR;
    GmoShape* m_pEndNR;

    //geometry
	bool m_fAbove;
	bool m_fDrawBracket;
	LUnits m_uBorderLength;
    LUnits m_uBracketDistance;
    LUnits m_uLineThick;
    LUnits m_uSpaceToNumber;

public:
    GmoShapeTuplet(ImoObj* pCreatorImo, Color color = Color(0,0,0),
                   int design = ImoTuplet::k_straight);
    ~GmoShapeTuplet();

    void set_layout_data(bool fAbove, bool fDrawBracket, LUnits yStart, LUnits yEnd,
                         LUnits uBorderLength,
                         LUnits uBracketDistance, LUnits uLineThick,
                         LUnits uSpaceToNumber,
                         GmoShape* pStart, GmoShape* pEnd);

    void add_label(GmoShapeText* pShape);

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

protected:
    void compute_horizontal_position();
    void compute_position();
    void draw_horizontal_line(Drawer* pDrawer);
    void draw_vertical_borders(Drawer* pDrawer);
    void compute_bounds();
    void make_points_relative_to_origin();

    //reference positions
    LUnits m_uxStart, m_uyStart;
    LUnits m_uxEnd, m_uyEnd;

	LUnits m_yLineStart;
    LUnits m_yLineEnd;
    LUnits m_yStartBorder;
    LUnits m_yEndBorder;
    LUnits m_xNumber;
    LUnits m_yNumber;
    LUnits m_uNumberWidth;
};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_TUPLET_H__

