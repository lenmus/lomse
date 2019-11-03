//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_BARLINE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_BARLINE_H__

#include "lomse_basic.h"
#include "lomse_shape_base.h"

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

protected:
    void compute_width();
    void determine_lines_relative_positions();
    void draw_thin_line(Drawer* pDrawer, LUnits uxPos, LUnits uyTop, LUnits uyBottom,
                        Color color);
    void draw_thick_line(Drawer* pDrawer, LUnits uxLeft, LUnits uyTop, LUnits uWidth,
                         LUnits uHeight, Color color);
    void draw_two_dots(Drawer* pDrawer, LUnits uxPos, LUnits uyPos);

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_BARLINE_H__

