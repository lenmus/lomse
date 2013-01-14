//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

//some macros to improve code reading
#define START       ImoBezierInfo::k_start
#define END         ImoBezierInfo::k_end
#define CTROL1      ImoBezierInfo::k_ctrol1
#define CTROL2      ImoBezierInfo::k_ctrol2


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
// GmoShapeTie implementation
//=======================================================================================
GmoShapeTie::GmoShapeTie(ImoObj* pCreatorImo, int idx, UPoint* points, LUnits tickness,
                         Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_tie, idx, color)
    , m_thickness(tickness)
{
    save_points(points);
    compute_vertices();
}

//---------------------------------------------------------------------------------------
GmoShapeTie::~GmoShapeTie()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeTie::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);

    //Test code, just to see where the points are located
#if (0)
    pDrawer->begin_path();
    pDrawer->fill(Color(0,0,0,0));          //transparent white
    pDrawer->stroke(Color(255,0,0));
    LUnits uWidth = 20.0f;
    pDrawer->stroke_width(uWidth);
    pDrawer->move_to(m_points[START].x + uWidth / 2.0f, m_points[START].y);
    pDrawer->line_to(m_points[CTROL1].x + uWidth / 2.0f, m_points[CTROL1].y);
    pDrawer->line_to(m_points[CTROL2].x + uWidth / 2.0f, m_points[CTROL2].y);
    pDrawer->line_to(m_points[END].x + uWidth / 2.0f, m_points[END].y);
    pDrawer->end_path();
    pDrawer->render();
#endif
}

//---------------------------------------------------------------------------------------
void GmoShapeTie::save_points(UPoint* points)
{
    for (int i=0; i < 4; i++)
        m_points[i] = *(points+i);
}

//---------------------------------------------------------------------------------------
void GmoShapeTie::compute_vertices()
{
    LUnits t = m_thickness / 2.0f;
    m_vertices[0] = m_points[START];
    m_vertices[1].x = m_points[CTROL1].x;
    m_vertices[1].y = m_points[CTROL1].y - t;
    m_vertices[2].x = m_points[CTROL2].x;
    m_vertices[2].y = m_points[CTROL2].y - t;
    m_vertices[3] = m_points[END];
    m_vertices[4].x = m_points[CTROL2].x - t;
    m_vertices[4].y = m_points[CTROL2].y + t;
    m_vertices[5].x = m_points[CTROL1].x + t;
    m_vertices[5].y = m_points[CTROL1].y + t;
    m_vertices[6] = m_points[START];
}

//---------------------------------------------------------------------------------------
unsigned GmoShapeTie::vertex(double* px, double* py)
{
	if(m_nCurVertex >= m_nNumVertices)
		return agg::path_cmd_stop;

    if (m_nCurVertex < 7)
    {
	    *px = m_vertices[m_nCurVertex].x;   //.ux_coord;
	    *py = m_vertices[m_nCurVertex].y;   //.uy_coord;
    }
    else
    {
	    *px = 0.0;
	    *py = 0.0;
    }

	return m_cmd[m_nCurVertex++].cmd;
}


//=======================================================================================
// GmoShapeSlur implementation
//=======================================================================================
GmoShapeSlur::GmoShapeSlur(ImoObj* pCreatorImo, int idx, UPoint* points, LUnits tickness,
                         Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_slur, idx, color)
    , m_thickness(tickness)
{
    save_points(points);
    compute_vertices();
}

//---------------------------------------------------------------------------------------
GmoShapeSlur::~GmoShapeSlur()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);

////    //Test code, just to see where the points are located
//    pDrawer->begin_path();
//    pDrawer->fill(Color(0,0,0,0));          //transparent white
//    pDrawer->stroke(Color(255,0,0));
//    LUnits uWidth = 20.0f;
//    pDrawer->stroke_width(uWidth);
//    pDrawer->move_to(m_points[START].x + uWidth / 2.0f, m_points[START].y);
//    pDrawer->line_to(m_points[CTROL1].x + uWidth / 2.0f, m_points[CTROL1].y);
//    pDrawer->line_to(m_points[CTROL2].x + uWidth / 2.0f, m_points[CTROL2].y);
//    pDrawer->line_to(m_points[END].x + uWidth / 2.0f, m_points[END].y);
//    pDrawer->end_path();
//    pDrawer->render();
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::save_points(UPoint* points)
{
    for (int i=0; i < 4; i++)
        m_points[i] = *(points+i);
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::compute_vertices()
{
    LUnits t = m_thickness / 2.0f;
    m_vertices[0] = m_points[START];
    m_vertices[1].x = m_points[CTROL1].x;
    m_vertices[1].y = m_points[CTROL1].y - t;
    m_vertices[2].x = m_points[CTROL2].x;
    m_vertices[2].y = m_points[CTROL2].y - t;
    m_vertices[3] = m_points[END];
    m_vertices[4].x = m_points[CTROL2].x - t;
    m_vertices[4].y = m_points[CTROL2].y + t;
    m_vertices[5].x = m_points[CTROL1].x + t;
    m_vertices[5].y = m_points[CTROL1].y + t;
    m_vertices[6] = m_points[START];
}

//---------------------------------------------------------------------------------------
unsigned GmoShapeSlur::vertex(double* px, double* py)
{
	if(m_nCurVertex >= m_nNumVertices)
		return agg::path_cmd_stop;

    if (m_nCurVertex < 7)
    {
	    *px = m_vertices[m_nCurVertex].x;   //.ux_coord;
	    *py = m_vertices[m_nCurVertex].y;   //.uy_coord;
    }
    else
    {
	    *px = 0.0;
	    *py = 0.0;
    }

	return m_cmd[m_nCurVertex++].cmd;
}


}  //namespace lomse
