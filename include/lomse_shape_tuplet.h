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

    void on_draw(Drawer* pDrawer, RenderOptions& opt);

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

