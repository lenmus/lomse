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

#ifndef __LOMSE_SHAPE_TIE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_TIE_H__

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
class GmoShapeSlurTie : public GmoSimpleShape, public VertexSource
{
protected:
    LUnits m_thickness;
    UPoint m_points[4];

    UPoint m_vertices[7];
    int m_nCurVertex;               //index to current vertex
    int m_nContour;                 //current countour

public:
    GmoShapeSlurTie(ImoObj* pCreatorImo, int objtype, ShapeId idx, UPoint points[],
                    LUnits tickness, Color color);
    virtual ~GmoShapeSlurTie();

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //VertexSource
    void rewind(unsigned UNUSED(pathId) = 0) override { m_nCurVertex = 0; }
    unsigned vertex(double* px, double* py) override;

    //support for handlers
    int get_num_handlers() override;
    UPoint get_handler_point(int i) override;
    void on_handler_dragged(int iHandler, UPoint newPos) override;
    void on_end_of_handler_drag(int iHandler, UPoint newPos) override;

protected:
    void save_points(UPoint* points);
    void compute_vertices();
    void compute_bounds();
    void make_points_and_vertices_relative_to_origin();

};


//---------------------------------------------------------------------------------------
class GmoShapeTie : public GmoShapeSlurTie, public VoiceRelatedShape
{
public:
    GmoShapeTie(ImoObj* pCreatorImo, ShapeId idx, UPoint points[], LUnits thickness,
                Color color = Color(0,0,0));
    virtual ~GmoShapeTie() {}
};

//---------------------------------------------------------------------------------------
class GmoShapeSlur : public GmoShapeSlurTie
{
protected:
    std::vector<UPoint> m_dataPoints;       //only for debug: to draw reference points
    Color m_dbgColor;                       //debug: color for the points
    UPoint m_dbgPeak;                       //debug: bezier point at peak point
    LUnits m_xc, m_yc, m_r;                 //debug: arc approximating the bezier

public:
    GmoShapeSlur(ImoObj* pCreatorImo, ShapeId idx, UPoint points[], LUnits thickness,
                 Color color = Color(0,0,0));
    virtual ~GmoShapeSlur() {}

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //methods used only during development, to draw references and points
    void add_data_points(const std::vector<UPoint>& data, UPoint peak,
                         Color color=Color(255,0,0));       //only for debug
    void add_approx_arc(LUnits xc, LUnits yc, LUnits r);    //only for debug


protected:
    void draw_control_points(Drawer* pDrawer);      //only for debug: to draw control points
    void draw_reference_points(Drawer* pDrawer);    //only for debug: to draw reference points
    void draw_approximate_arc(Drawer* pDrawer);     //only for debug: to draw approx. arc
};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_TIE_H__

