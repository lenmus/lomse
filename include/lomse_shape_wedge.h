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

    int     m_niente;
    LUnits  m_radiusNiente;

public:
    GmoShapeWedge(ImoObj* pCreatorImo, ShapeId idx, UPoint points[], LUnits thickness,
                  Color color, int niente, LUnits radius);
    virtual ~GmoShapeWedge();

    void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //construction
    enum {
        k_niente_at_start = 0,
        k_niente_at_end,
        k_no_niente,
    };

    //support for handlers
    int get_num_handlers();
    UPoint get_handler_point(int i);
    void on_handler_dragged(int iHandler, UPoint newPos);
    void on_end_of_handler_drag(int iHandler, UPoint newPos);

protected:
    void save_points_and_compute_bounds(UPoint* points);

};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_WEDGE_H__

