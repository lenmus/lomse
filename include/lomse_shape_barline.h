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

#ifndef __LOMSE_SHAPE_BARLINE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_BARLINE_H__

#include "lomse_basic.h"
#include "lomse_shape_base.h"

namespace lomse
{

//forward declarations
class ImoBarline;


class GmoShapeBarline : public GmoSimpleShape
{
protected:
    int  m_nBarlineType;

    //geometry
    LUnits  m_uxLeft;
    LUnits  m_uThinLineWidth;
    LUnits  m_uThickLineWidth;
    LUnits  m_uSpacing;            // between lines and lines-dots
    LUnits  m_uRadius;             // for dots

public:
    GmoShapeBarline(ImoObj* pCreatorImo, int idx, int nBarlineType,
                    LUnits xPos, LUnits yTop, LUnits yBottom,
                    LUnits uThinLineWidth, LUnits uThickLineWidth,
                    LUnits uSpacing, LUnits uRadius, Color color,
                    LUnits uMinWidth);
	~GmoShapeBarline();

	//implementation of virtual methods in base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
    void shift_origin(const USize& shift);

	//wxBitmap* OnBeginDrag(double rScale, wxDC* pDC);


	////access to info
	//inline LUnits GetXEnd() const { return m_uxPos + m_uWidth; }

protected:
    void compute_width();
    void draw_thin_line(Drawer* pDrawer, LUnits uxPos, LUnits uyTop, LUnits uyBottom,
                        Color color);
    void draw_thick_line(Drawer* pDrawer, LUnits uxLeft, LUnits uyTop, LUnits uWidth,
                         LUnits uHeight, Color color);
    void draw_two_dots(Drawer* pDrawer, LUnits uxPos, LUnits uyPos);

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_BARLINE_H__

