//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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


//---------------------------------------------------------------------------------------
class GmoShapeTuplet : public GmoCompositeShape
{
protected:
    int m_design;
    bool m_fNeedsLayout;
    GmoShapeText* m_pShapeText;

    //geometry
	bool m_fAbove;
	bool m_fDrawBracket;
	LUnits m_uBorderLength;
    LUnits m_uBracketDistance;
    LUnits m_uLineThick;
    LUnits m_uNumberDistance;
    LUnits m_uSpaceToNumber;

public:
    GmoShapeTuplet(ImoObj* pCreatorImo, Color color = Color(0,0,0),
                   int design = ImoTuplet::k_straight);
    ~GmoShapeTuplet();

    void set_layout_data(bool fAbove, bool fDrawBracket, LUnits yStart, LUnits yEnd,
                         LUnits uBorderLength,
                         LUnits uBracketDistance, LUnits uLineThick,
                         LUnits uNumberDistance, LUnits uSpaceToNumber);

    void add_label(GmoShapeText* pShape);

    void on_draw(Drawer* pDrawer, RenderOptions& opt);

protected:
    void get_noterests_positions();
    void compute_position();
    void draw_horizontal_line(Drawer* pDrawer);
    void draw_vertical_borders(Drawer* pDrawer);
    void compute_bounds();

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

