//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_SHAPE_VOLTA_BRACKET_H__        //to avoid nested includes
#define __LOMSE_SHAPE_VOLTA_BRACKET_H__

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
class GmoShapeVoltaBracket : public GmoCompositeShape
{
protected:
    GmoShapeBarline* m_pStopBarlineShape;
    GmoShapeText* m_pShapeText;
    bool m_fTwoBrackets;
    bool m_fStopJog;
	LUnits m_uJogLength;
    LUnits m_uLineThick;
    LUnits m_uStaffLeft;
    LUnits m_uStaffRight;
    LUnits m_uBracketDistance;

public:
    GmoShapeVoltaBracket(ImoObj* pCreatorImo, ShapeId idx, Color color);
    virtual ~GmoShapeVoltaBracket();

    void on_draw(Drawer* pDrawer, RenderOptions& opt);


    void set_layout_data(LUnits xStart, LUnits xEnd, LUnits yPos, LUnits uBracketDistance,
                         LUnits uJogLength, LUnits uLineThick, LUnits uLeftSpaceToText,
                         LUnits uTopSpaceToText, LUnits xStaffLeft, LUnits xStaffRight);

    void add_label(GmoShapeText* pShape);
    inline void enable_final_jog(bool value) { m_fStopJog = value; }
    inline void set_two_brackets() { m_fTwoBrackets = true; }

    //support for handlers
    int get_num_handlers();
    UPoint get_handler_point(int i);
    void on_handler_dragged(int iHandler, UPoint newPos);
    void on_end_of_handler_drag(int iHandler, UPoint newPos);

protected:
    void compute_bounds(LUnits xStart, LUnits xEnd, LUnits yPos);

};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_VOLTA_BRACKET_H__

