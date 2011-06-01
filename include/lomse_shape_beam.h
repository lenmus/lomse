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

#ifndef __LOMSE_SHAPE_BEAM_H__
#define __LOMSE_SHAPE_BEAM_H__

#include "lomse_basic.h"
#include "lomse_shape_base.h"

namespace lomse
{

//forward declarations
class ImoObj;
class Drawer;


//---------------------------------------------------------------------------------------
class GmoShapeBeam : public GmoSimpleShape
{
protected:
    LUnits m_uBeamThickness;
    std::list<LUnits> m_segments;
	UPoint m_outerLeftPoint;
    UPoint m_outerRightPoint;

public:
    GmoShapeBeam(ImoObj* pCreatorImo, LUnits uBeamThickness,
                 Color color = Color(0,0,0));
	~GmoShapeBeam();

    void set_layout_data(std::list<LUnits>& segments, UPoint origin, USize size,
                         UPoint outerLeft, UPoint outerRight);
    void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //provide geometry reference info, for other related shapes
    UPoint get_outer_left_reference_point() { return m_outerLeftPoint; }
    UPoint get_outer_right_reference_point() { return m_outerRightPoint; }


protected:
	void draw_beam_segment(Drawer* pDrawer, LUnits uxStart, LUnits uyStart,
                           LUnits uxEnd, LUnits uyEnd, Color color);


};


}   //namespace lomse

#endif      //__LOMSE_SHAPE_BEAM_H__
