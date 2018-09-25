//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_shape_tie.h"

#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"
#include "lomse_shape_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
//vertices data. Common for ties and slurs
Vertex m_cmd[] = {
    {   0.0,    0.0, agg::path_cmd_move_to },
    { 110.0,  114.0, agg::path_cmd_curve4 },        //ctrol1
    { 530.0,  114.0, agg::path_cmd_curve4 },        //ctrol2
    { 640.0,    0.0, agg::path_cmd_curve4 },        //on-curve
    { 530.0,   88.0, agg::path_cmd_curve4 },        //ctrol1
    { 110.0,   88.0, agg::path_cmd_curve4 },        //ctrol2
    {   0.0,    0.0, agg::path_cmd_end_poly
                     | agg::path_flags_close
                     | agg::path_flags_ccw },       //close polygon
    {   0.0,    0.0, agg::path_cmd_stop }
};

const int m_nNumVertices = sizeof(m_cmd)/sizeof(Vertex);


//=======================================================================================
// GmoShapeSlurTie implementation
//=======================================================================================
GmoShapeSlurTie::GmoShapeSlurTie(ImoObj* pCreatorImo, int objtype, ShapeId idx,
                                 UPoint* points, LUnits thickness, Color color)
    : GmoSimpleShape(pCreatorImo, objtype, idx, color)
    , VertexSource()
    , m_thickness(thickness)
    , m_nCurVertex(0)
    , m_nContour(0)
{
    save_points(points);
    compute_vertices();
    compute_bounds();
    make_points_and_vertices_relative_to_origin();
}

//---------------------------------------------------------------------------------------
GmoShapeSlurTie::~GmoShapeSlurTie()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::save_points(UPoint* points)
{
    for (int i=0; i < 4; i++)
        m_points[i] = *(points+i);
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::compute_vertices()
{
    LUnits t = m_thickness / 2.0f;
    m_vertices[0] = m_points[ImoBezierInfo::k_start];
    m_vertices[1].x = m_points[ImoBezierInfo::k_ctrol1].x;
    m_vertices[1].y = m_points[ImoBezierInfo::k_ctrol1].y - t;
    m_vertices[2].x = m_points[ImoBezierInfo::k_ctrol2].x;
    m_vertices[2].y = m_points[ImoBezierInfo::k_ctrol2].y - t;
    m_vertices[3] = m_points[ImoBezierInfo::k_end];
    m_vertices[4].x = m_points[ImoBezierInfo::k_ctrol2].x - t;
    m_vertices[4].y = m_points[ImoBezierInfo::k_ctrol2].y + t;
    m_vertices[5].x = m_points[ImoBezierInfo::k_ctrol1].x + t;
    m_vertices[5].y = m_points[ImoBezierInfo::k_ctrol1].y + t;
    m_vertices[6] = m_points[ImoBezierInfo::k_start];
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::compute_bounds()
{
    //TODO: Improve bounds computation.
    //For now, I just take a rectangle based on control points

    m_origin.x = m_points[ImoBezierInfo::k_start].x;
    m_origin.x = min(m_origin.x, m_points[ImoBezierInfo::k_ctrol1].x);
    m_origin.x = min(m_origin.x, m_points[ImoBezierInfo::k_ctrol2].x);
    m_origin.x = min(m_origin.x, m_points[ImoBezierInfo::k_end].x);

    m_origin.y = m_points[ImoBezierInfo::k_start].y;
    m_origin.y = min(m_origin.y, m_points[ImoBezierInfo::k_ctrol1].y);
    m_origin.y = min(m_origin.y, m_points[ImoBezierInfo::k_ctrol2].y);
    m_origin.y = min(m_origin.y, m_points[ImoBezierInfo::k_end].y);

    LUnits max_x = m_points[ImoBezierInfo::k_end].x;
    max_x = max(max_x, m_points[ImoBezierInfo::k_ctrol1].x);
    max_x = max(max_x, m_points[ImoBezierInfo::k_ctrol2].x);
    max_x = max(max_x, m_points[ImoBezierInfo::k_end].x);

    LUnits max_y = m_points[ImoBezierInfo::k_start].y;
    max_y = max(max_y, m_points[ImoBezierInfo::k_ctrol1].y);
    max_y = max(max_y, m_points[ImoBezierInfo::k_ctrol2].y);
    max_y = max(max_y, m_points[ImoBezierInfo::k_end].y);

    m_size.width = max_x - m_origin.x;
    m_size.height = max_y - m_origin.y;
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::make_points_and_vertices_relative_to_origin()
{
    for (int i=0; i < 7; ++i)
        m_vertices[i] -= m_origin;

    for (int i=0; i < 4; i++)
        m_points[i] -= m_origin;
}

//---------------------------------------------------------------------------------------
unsigned GmoShapeSlurTie::vertex(double* px, double* py)
{
	if(m_nCurVertex >= m_nNumVertices)
		return agg::path_cmd_stop;

    if (m_nCurVertex < 7)
    {
	    *px = m_vertices[m_nCurVertex].x + m_origin.x;
	    *py = m_vertices[m_nCurVertex].y + m_origin.y;
    }
    else
    {
	    *px = 0.0;
	    *py = 0.0;
    }

	return m_cmd[m_nCurVertex++].cmd;
}

//---------------------------------------------------------------------------------------
int GmoShapeSlurTie::get_num_handlers()
{
    return 4;
}

//---------------------------------------------------------------------------------------
UPoint GmoShapeSlurTie::get_handler_point(int i)
{
    return m_points[i] + m_origin;
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::on_handler_dragged(int iHandler, UPoint newPos)
{
    m_points[iHandler] = newPos;
    compute_vertices();
    compute_bounds();
    make_points_and_vertices_relative_to_origin();
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::on_end_of_handler_drag(int UNUSED(iHandler), UPoint UNUSED(newPos))
{
    //TODO
}


//=======================================================================================
// GmoShapeTie implementation
//=======================================================================================
GmoShapeTie::GmoShapeTie(ImoObj* pCreatorImo, ShapeId idx, UPoint* points,
                         LUnits thickness, Color color)
    : GmoShapeSlurTie(pCreatorImo, GmoObj::k_shape_tie, idx, points, thickness, color)
    , VoiceRelatedShape()
{
}


//=======================================================================================
// GmoShapeSlur implementation
//=======================================================================================
GmoShapeSlur::GmoShapeSlur(ImoObj* pCreatorImo, ShapeId idx, UPoint* points,
                           LUnits thickness, Color color)
    : GmoShapeSlurTie(pCreatorImo, GmoObj::k_shape_slur, idx, points, thickness, color)
{
}


}  //namespace lomse
