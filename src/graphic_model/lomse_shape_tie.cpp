//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    LUnits t = m_thickness * 0.5f;
    LUnits a1 = atan2(m_points[ImoBezierInfo::k_ctrol1].y-m_points[ImoBezierInfo::k_start].y,
                      m_points[ImoBezierInfo::k_ctrol1].x-m_points[ImoBezierInfo::k_start].x);
    LUnits dx1 = t*sin(a1);
    LUnits dy1 = max(t*cos(a1), t*0.5f);


    LUnits a2 = atan2(m_points[ImoBezierInfo::k_ctrol2].y-m_points[ImoBezierInfo::k_end].y,
                      m_points[ImoBezierInfo::k_end].x-m_points[ImoBezierInfo::k_ctrol2].x);
    LUnits dx2 = t*sin(a2);
    LUnits dy2 = max(t*cos(a2), t*0.5f);

    m_vertices[0] = m_points[ImoBezierInfo::k_start];
    m_vertices[1].x = m_points[ImoBezierInfo::k_ctrol1].x + dx1;
    m_vertices[1].y = m_points[ImoBezierInfo::k_ctrol1].y - dy1;
    m_vertices[2].x = m_points[ImoBezierInfo::k_ctrol2].x - dx2;
    m_vertices[2].y = m_points[ImoBezierInfo::k_ctrol2].y - dy2;
    m_vertices[3] = m_points[ImoBezierInfo::k_end];
    m_vertices[4].x = m_points[ImoBezierInfo::k_ctrol2].x + dx2;
    m_vertices[4].y = m_points[ImoBezierInfo::k_ctrol2].y + dy2;
    m_vertices[5].x = m_points[ImoBezierInfo::k_ctrol1].x - dx1;
    m_vertices[5].y = m_points[ImoBezierInfo::k_ctrol1].y + dy1;
    m_vertices[6] = m_points[ImoBezierInfo::k_start];
}

//---------------------------------------------------------------------------------------
void GmoShapeSlurTie::compute_bounds()
{
    //TODO: Improve bounds computation.
    //For now, I just take a rectangle based on center point
    LUnits yc = (m_points[0].y + m_points[1].y + 3.0f * (m_points[2].y + m_points[3].y)) / 8.0f;

    m_origin.x = m_points[ImoBezierInfo::k_start].x;
    m_origin.x = min(m_origin.x, m_points[ImoBezierInfo::k_end].x);

    m_origin.y = m_points[ImoBezierInfo::k_start].y;
    m_origin.y = min(m_origin.y, m_points[ImoBezierInfo::k_end].y);
    m_origin.y = min(m_origin.y, yc);

    LUnits max_x = m_points[ImoBezierInfo::k_start].x;
    max_x = max(max_x, m_points[ImoBezierInfo::k_end].x);

    LUnits max_y = m_points[ImoBezierInfo::k_start].y;
    max_y = max(max_y, m_points[ImoBezierInfo::k_end].y);
    max_y = max(max_y, yc);

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

//---------------------------------------------------------------------------------------
void GmoShapeSlur::add_data_points(const std::vector<UPoint>& data, UPoint peak,
                                   Color color)
{
    m_dbgColor = color;
    m_dataPoints = data;
    m_dbgPeak.x = peak.x - m_origin.x;
    m_dbgPeak.y = peak.y - m_origin.y;

    for (unsigned i=0; i < m_dataPoints.size(); ++i)
    {
        m_dataPoints[i].x -= m_origin.x;
        m_dataPoints[i].y -= m_origin.y;
    }
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::add_approx_arc(LUnits xc, LUnits yc, LUnits r)
{
    m_xc = xc - m_origin.x;
    m_yc = yc - m_origin.y;
    m_r = r;
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    if (opt.draw_slur_points)
    {
        draw_control_points(pDrawer);
        draw_reference_points(pDrawer);
        draw_approximate_arc(pDrawer);
    }
    pDrawer->render();

    GmoShapeSlurTie::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::draw_control_points(Drawer* pDrawer)
{
    pDrawer->begin_path();
    pDrawer->fill(m_dbgColor);
//    for (unsigned i=0; i < 4; ++i)
//    {
//        pDrawer->circle(m_points[i].x + m_origin.x,
//                        m_points[i].y + m_origin.y, 60.0f);
//    }

//    //determine cross point for tangent lines
//    LUnits m1 = (m_points[2].y - m_points[0].y) / (m_points[2].x - m_points[0].x);
//    LUnits m2 = (m_points[3].y - m_points[1].y) / (m_points[3].x - m_points[1].x);
//    LUnits xa = (m1 * m_points[0].x - m2 * m_points[1].x + m_points[1].y - m_points[0].y) / (m1 - m2);
//    LUnits ya = m1 * (xa - m_points[0].x) + m_points[0].y;
//    xa += m_origin.x;
//    ya += m_origin.y;
//    pDrawer->circle(xa, ya, 60.0f);
//
//    pDrawer->line(m_points[0].x + m_origin.x, m_points[0].y + m_origin.y, xa, ya, 25.0f);
//    pDrawer->line(m_points[1].x + m_origin.x, m_points[1].y + m_origin.y, xa, ya, 25.0f);

    //compute center point
    LUnits xc = (m_points[0].x + m_points[1].x + 3.0f * (m_points[2].x + m_points[3].x)) / 8.0f;
    LUnits yc = (m_points[0].y + m_points[1].y + 3.0f * (m_points[2].y + m_points[3].y)) / 8.0f;
    xc += m_origin.x;
    yc += m_origin.y;
    pDrawer->circle(xc, yc, 60.0f);

    //draw lines to control points
    pDrawer->line(m_points[0].x + m_origin.x, m_points[0].y + m_origin.y,
                  m_points[2].x + m_origin.x, m_points[2].y + m_origin.y,
                  25.0f);
    pDrawer->line(m_points[1].x + m_origin.x, m_points[1].y + m_origin.y,
                  m_points[3].x + m_origin.x, m_points[3].y + m_origin.y,
                  25.0f);
    //pDrawer->end_path();

    //draw baseline
    //pDrawer->begin_path();
    //pDrawer->fill(Color(0,255,0));
    pDrawer->line(m_points[0].x + m_origin.x, m_points[0].y + m_origin.y,
                  m_points[1].x + m_origin.x, m_points[1].y + m_origin.y,
                  25.0f);
    xc = (m_points[0].x + m_points[1].x) / 2.0f;
    yc = (m_points[0].y + m_points[1].y) / 2.0f;
    xc += m_origin.x;
    yc += m_origin.y;
    pDrawer->circle(xc, yc, 60.0f);

    //draw top line
    pDrawer->line(m_points[2].x + m_origin.x, m_points[2].y + m_origin.y,
                  m_points[3].x + m_origin.x, m_points[3].y + m_origin.y,
                  25.0f);
    xc = (m_points[2].x + m_points[3].x) / 2.0f;
    yc = (m_points[2].y + m_points[3].y) / 2.0f;
    xc += m_origin.x;
    yc += m_origin.y;
    pDrawer->circle(xc, yc, 60.0f);

    pDrawer->end_path();

    //draw peak point
    if (m_dataPoints.size() > 2)
    {
        pDrawer->begin_path();
        pDrawer->fill(Color(0,255,255));    //cyan
        pDrawer->circle(m_dbgPeak.x + m_origin.x, m_dbgPeak.y + m_origin.y, 60.0f);
        pDrawer->end_path();
    }

    //draw aux control points
    pDrawer->begin_path();
    pDrawer->fill(Color(255,0,0));
    pDrawer->circle(m_vertices[1].x + m_origin.x, m_vertices[1].y + m_origin.y, 60.0f);
    pDrawer->circle(m_vertices[2].x + m_origin.x, m_vertices[2].y + m_origin.y, 60.0f);
    pDrawer->end_path();

    pDrawer->begin_path();
    pDrawer->fill(Color(0,0,255));
    pDrawer->circle(m_vertices[4].x + m_origin.x, m_vertices[4].y + m_origin.y, 60.0f);
    pDrawer->circle(m_vertices[5].x + m_origin.x, m_vertices[5].y + m_origin.y, 60.0f);
    pDrawer->end_path();

    pDrawer->begin_path();
    pDrawer->fill(Color(255,255,255));
    pDrawer->circle(m_points[2].x + m_origin.x, m_points[2].y + m_origin.y, 10.0f);
    pDrawer->circle(m_points[3].x + m_origin.x, m_points[3].y + m_origin.y, 10.0f);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::draw_reference_points(Drawer* pDrawer)
{
    pDrawer->begin_path();
    pDrawer->fill(Color(0,0,255));
    for (unsigned i=0; i < m_dataPoints.size(); ++i)
    {
        pDrawer->circle(m_dataPoints[i].x + m_origin.x,
                        m_dataPoints[i].y + m_origin.y, 60.0f);
        if (i > 0)
            pDrawer->line(m_dataPoints[i-1].x + m_origin.x, m_dataPoints[i-1].y + m_origin.y,
                          m_dataPoints[i].x + m_origin.x, m_dataPoints[i].y + m_origin.y,
                          25.0f);
    }

    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoShapeSlur::draw_approximate_arc(Drawer* pDrawer)
{
    if (m_r > 0.0)
    {
        pDrawer->begin_path();
        pDrawer->stroke_width(25.0f);
        pDrawer->stroke(Color(255,128,0));  //orange
        pDrawer->fill_none();
        pDrawer->circle(m_xc + m_origin.x, m_yc + m_origin.y, m_r);
        pDrawer->end_path();
    }
}


}  //namespace lomse
