//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_BARLINE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_BARLINE_H__

#include "lomse_basic.h"
#include "lomse_shape_base.h"

#include <vector>

namespace lomse
{

//forward declarations
class ImoBarline;
class BarlineEngraver;


class GmoShapeBarline : public GmoSimpleShape
{
protected:
    int  m_nBarlineType;

    //geometry
    LUnits  m_uxLeft;
    LUnits  m_uThinLineWidth;
    LUnits  m_uThickLineWidth;
    LUnits  m_uSpacing;             // between lines and lines-dots
    LUnits  m_uRadius;              // for dots

    //lines position
    LUnits  m_xRightLine;           //x right of last right line
    LUnits  m_xLeftLine;            //x left of first left line

    std::vector<LUnits> m_relStaffTopPositions;

    friend class BarlineEngraver;
    GmoShapeBarline(ImoObj* pCreatorImo, ShapeId idx, int nBarlineType,
                    LUnits xPos, LUnits yTop, LUnits yBottom,
                    LUnits uThinLineWidth, LUnits uThickLineWidth,
                    LUnits uSpacing, LUnits uRadius, Color color,
                    LUnits uMinWidth);

public:
	~GmoShapeBarline();

	//implementation of virtual methods in base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;
    void shift_origin(const USize& shift) override;

	//access to info
    inline LUnits get_x_right_line() { return m_xRightLine + m_uxLeft; }
    inline LUnits get_x_left_line() { return m_xLeftLine + m_uxLeft; }

    void set_relative_staff_top_positions(const std::vector<LUnits>& positions) { m_relStaffTopPositions = positions; }
    void set_relative_staff_top_positions(std::vector<LUnits>&& positions) { m_relStaffTopPositions = std::move(positions); }

protected:
    void compute_width();
    void determine_lines_relative_positions();
    void draw_thin_line(Drawer* pDrawer, LUnits uxPos, LUnits uyTop, LUnits uyBottom,
                        Color color);
    void draw_thick_line(Drawer* pDrawer, LUnits uxLeft, LUnits uyTop, LUnits uWidth,
                         LUnits uHeight, Color color);
    void draw_two_dots(Drawer* pDrawer, LUnits uxPos, LUnits uyPos, Color color);
    void draw_repeat_dots_for_all_staves(Drawer* pDrawer, LUnits uxPos, LUnits uyPos, Color color);

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_BARLINE_H__

