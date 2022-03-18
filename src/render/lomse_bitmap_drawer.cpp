//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

//  This file is based on Anti-Grain Geometry version 2.4 examples' code and on
//  BitmapDrawer version 1.0 code.
//
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed as follows:
//    "Permission to copy, use, modify, sell and distribute this software
//    is granted provided this copyright notice appears in all copies.
//    This software is provided "as is" without express or implied
//    warranty, and with no claim as to its suitability for any purpose."
//
//---------------------------------------------------------------------------------------

#include "lomse_bitmap_drawer.h"

#include "lomse_logger.h"
#include "lomse_renderer.h"
#include "agg_rounded_rect.h"

#include "agg_path_storage.h"



using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
MarkerVertexSource::MarkerVertexSource()
    : VertexSource()
    , m_head_d1(1.0)
    , m_head_d2(1.0)
    , m_head_d3(1.0)
    , m_head_d4(0.0)
    , m_tail_d1(1.0)
    , m_tail_d2(1.0)
    , m_tail_d3(1.0)
    , m_tail_d4(0.0)
    , m_head_type(k_none)
    , m_tail_type(k_none)
    , m_curr_id(0)
    , m_curr_coord(0)
    , m_status(stop)
    , m_radius(1.0)
{
}

//---------------------------------------------------------------------------------------
void MarkerVertexSource::rewind(unsigned path_id)
{
    m_status = stop;
    m_curr_id = path_id;
    m_curr_coord = 0;

    if(path_id == 0)    //head
    {
        switch(m_head_type)
        {
        case k_none:
            m_cmd[0] = agg::path_cmd_stop;
            return;

        case k_arrowhead:
			m_status = points;
            m_coord[0]  = -m_head_d1;            m_coord[1]  = 0.0;
            m_coord[2]  = m_head_d2 + m_head_d4; m_coord[3]  = -m_head_d3;
            m_coord[4]  = m_head_d2;             m_coord[5]  = 0.0;
            m_coord[6]  = m_head_d2 + m_head_d4; m_coord[7]  = m_head_d3;

            m_cmd[0] = agg::path_cmd_move_to;
            m_cmd[1] = agg::path_cmd_line_to;
            m_cmd[2] = agg::path_cmd_line_to;
            m_cmd[3] = agg::path_cmd_line_to;
            m_cmd[4] = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
            m_cmd[5] = agg::path_cmd_stop;
			return;

        case k_arrowtail:
			m_status = points;
			m_coord[0]  =  m_head_d4;                   m_coord[1]  =  0.0;
			m_coord[2]  =  m_coord[0] - m_head_d2;      m_coord[3]  =  m_head_d3;
			m_coord[4]  =  m_coord[2] - m_head_d1;      m_coord[5]  =  m_head_d3;
			m_coord[6]  =  m_coord[0] - m_head_d1;      m_coord[7]  =  0.0;
			m_coord[8]  =  m_coord[2] - m_head_d1;      m_coord[9]  = -m_head_d3;
			m_coord[10] =  m_coord[0] - m_head_d2;      m_coord[11] = -m_head_d3;

			m_cmd[0] = agg::path_cmd_move_to;
			m_cmd[1] = agg::path_cmd_line_to;
			m_cmd[2] = agg::path_cmd_line_to;
			m_cmd[3] = agg::path_cmd_line_to;
			m_cmd[4] = agg::path_cmd_line_to;
			m_cmd[5] = agg::path_cmd_line_to;
			m_cmd[6] = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
			m_cmd[7] = agg::path_cmd_stop;
			return;

        case k_circle:
            m_radius = m_head_d1;
            m_status = circle_start;
            return;

        case k_square:
			m_status = points;
			m_coord[0]  =  m_head_d2;                   m_coord[1]  =  m_head_d3;
			m_coord[2]  =  m_coord[0] - m_head_d1;      m_coord[3]  =  m_head_d3;
			m_coord[4]  =  m_coord[2];                  m_coord[5]  = -m_head_d3;
			m_coord[6]  =  m_coord[0];                  m_coord[7]  = -m_head_d3;

			m_cmd[0] = agg::path_cmd_move_to;
			m_cmd[1] = agg::path_cmd_line_to;
			m_cmd[2] = agg::path_cmd_line_to;
			m_cmd[3] = agg::path_cmd_line_to;
			m_cmd[4] = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
			m_cmd[5] = agg::path_cmd_stop;
			return;
        }
        return;
    }

    if(path_id == 1)    //tail
    {
        switch(m_tail_type)
        {
        case k_none:
            m_cmd[0] = agg::path_cmd_stop;
            return;

        case k_arrowhead:
			m_status = points;
            m_coord[0]  = -m_tail_d1;            m_coord[1]  = 0.0;
            m_coord[2]  = m_tail_d2 + m_tail_d4; m_coord[3]  = -m_tail_d3;
            m_coord[4]  = m_tail_d2;             m_coord[5]  = 0.0;
            m_coord[6]  = m_tail_d2 + m_tail_d4; m_coord[7]  = m_tail_d3;

            m_cmd[0] = agg::path_cmd_move_to;
            m_cmd[1] = agg::path_cmd_line_to;
            m_cmd[2] = agg::path_cmd_line_to;
            m_cmd[3] = agg::path_cmd_line_to;
            m_cmd[4] = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
            m_cmd[5] = agg::path_cmd_stop;
			return;

        case k_arrowtail:
			m_status = points;
			m_coord[0]  =  m_tail_d4;                   m_coord[1]  =  0.0;
			m_coord[2]  =  m_coord[0] - m_tail_d2;      m_coord[3]  =  m_tail_d3;
			m_coord[4]  =  m_coord[2] - m_tail_d1;      m_coord[5]  =  m_tail_d3;
			m_coord[6]  =  m_coord[0] - m_tail_d1;      m_coord[7]  =  0.0;
			m_coord[8]  =  m_coord[2] - m_tail_d1;      m_coord[9]  = -m_tail_d3;
			m_coord[10] =  m_coord[0] - m_tail_d2;      m_coord[11] = -m_tail_d3;

			m_cmd[0] = agg::path_cmd_move_to;
			m_cmd[1] = agg::path_cmd_line_to;
			m_cmd[2] = agg::path_cmd_line_to;
			m_cmd[3] = agg::path_cmd_line_to;
			m_cmd[4] = agg::path_cmd_line_to;
			m_cmd[5] = agg::path_cmd_line_to;
			m_cmd[6] = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
			m_cmd[7] = agg::path_cmd_stop;
			return;

        case k_circle:
            m_radius = m_tail_d1;
            m_status = circle_start;
            return;

        case k_square:
			m_status = points;
			m_coord[0]  =  m_tail_d2;                   m_coord[1]  =  m_tail_d3;
			m_coord[2]  =  m_coord[0] - m_tail_d1;      m_coord[3]  =  m_tail_d3;
			m_coord[4]  =  m_coord[2];                  m_coord[5]  = -m_tail_d3;
			m_coord[6]  =  m_coord[0];                  m_coord[7]  = -m_tail_d3;

			m_cmd[0] = agg::path_cmd_move_to;
			m_cmd[1] = agg::path_cmd_line_to;
			m_cmd[2] = agg::path_cmd_line_to;
			m_cmd[3] = agg::path_cmd_line_to;
			m_cmd[4] = agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw;
			m_cmd[5] = agg::path_cmd_stop;
			return;
        }
        return;
    }
}

//---------------------------------------------------------------------------------------
unsigned MarkerVertexSource::vertex(double* x, double* y)
{
    unsigned cmd = agg::path_cmd_stop;
    switch(m_status)
    {
        // coverity[unterminated_case]
        case circle_start:
            m_circle.init(0.0, 0.0, m_radius, m_radius);
            m_circle.rewind(0);
            m_status = circle_point;
            [[fallthrough]];    //continues in next case code

        // coverity[unterminated_case]
        case circle_point:
            cmd = m_circle.vertex(x, y);
            if(agg::is_stop(cmd)) m_status = stop;
            else return cmd;
            [[fallthrough]];    //to avoid stupid warning

        case points:
            if(m_curr_id < 2)
            {
                unsigned curr_idx = m_curr_coord * 2;
                *x = m_coord[curr_idx];
                *y = m_coord[curr_idx + 1];
                return m_cmd[m_curr_coord++];
            }
            return agg::path_cmd_stop;

        case stop:
    	    return agg::path_cmd_stop;
    }

    return cmd;
}


//=======================================================================================
// Drawer implementation
//=======================================================================================
Drawer::Drawer(LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
    , m_textColor(Color(0,0,0))
{
    m_pFonts = libraryScope.font_storage();
}

//---------------------------------------------------------------------------------------
void Drawer::set_text_color(Color color)
{
    m_textColor = color;
}

//---------------------------------------------------------------------------------------
void Drawer::new_viewport_origin(double x, double y)
{
    //coordinates in device units (e.g. Pixel)
    m_viewportOrg.x = x;
    m_viewportOrg.y = y;
}

//---------------------------------------------------------------------------------------
void Drawer::new_viewport_size(double x, double y)
{
    //in device units (e.g. Pixel)
    m_viewportSize.width = x;
    m_viewportSize.height = y;
}



//=======================================================================================
// BitmapDrawer implementation
//=======================================================================================
BitmapDrawer::BitmapDrawer(LibraryScope& libraryScope)
    : Drawer(libraryScope)
    , m_pRenderer( RendererFactory::create_renderer(libraryScope, m_attr_storage, m_path) )
//    , m_pTextMeter(nullptr)
    , m_pCalligrapher( LOMSE_NEW Calligrapher(m_pFonts, m_pRenderer) )
    , m_numPaths(0)
    , m_rbuf(nullptr, 0, 0, 0)
    , m_pBuf(nullptr)
{
}

//---------------------------------------------------------------------------------------
BitmapDrawer::~BitmapDrawer()
{
    delete m_pRenderer;
    delete m_pCalligrapher;
    delete_paths();
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::delete_paths()
{
    //AttrStorage objects are typedef for pod_bvector<PathAttributes>
    //and pod_bvector doesn't invoke destructors, just dealloc memory. Therefore, it
    //is necessary to ensure that memory allocated for GradientAttributes is freed,
    //to avoid memory leaks. Not sure if this is needed but just in case.

    for (int i = 0; i < m_numPaths; ++i)
    {
        PathAttributes& attr = m_attr_storage[i];
        delete attr.fill_gradient;
        attr.fill_gradient = nullptr;
    }

    m_attr_storage.clear();
    m_numPaths = 0;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::begin_path()
{
    unsigned idx = m_path.start_new_path();
    m_attr_storage.add( m_numPaths==0 ? PathAttributes(idx) : PathAttributes(cur_attr(), idx) );
    m_numPaths++;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::end_path()
{
    if(m_attr_storage.size() == 0)
    {
        LOMSE_LOG_ERROR("[BitmapDrawer::end_path] The path was not begun!");
        throw runtime_error("[BitmapDrawer::end_path] The path was not begun!");
    }
}

//---------------------------------------------------------------------------------------
PathAttributes& BitmapDrawer::cur_attr()
{
    return m_attr_storage[m_numPaths - 1];
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::move_to(double x, double y)
{
    m_path.move_to(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::move_to_rel(double x, double y)
{
    m_path.rel_to_abs(&x, &y);
    m_path.move_to(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::line_to(double x,  double y)
{
    m_path.line_to(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::line_to_rel(double x,  double y)
{
    m_path.rel_to_abs(&x, &y);
    m_path.line_to(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::hline_to(double x)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        m_path.line_to(x, y2);
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::hline_to_rel(double x)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        x += x2;
        m_path.line_to(x, y2);
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::vline_to(double y)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        m_path.line_to(x2, y);
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::vline_to_rel(double y)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        y += y2;
        m_path.line_to(x2, y);
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::quadratic_bezier(double x1, double y1, double x,  double y)
{
    m_path.curve3(x1, y1, x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::quadratic_bezier_rel(double x1, double y1, double x,  double y)
{
    m_path.rel_to_abs(&x1, &y1);
    m_path.rel_to_abs(&x,  &y);
    m_path.curve3(x1, y1, x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::quadratic_bezier(double x, double y)
{
    m_path.curve3(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::quadratic_bezier_rel(double x, double y)
{
    m_path.curve3_rel(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::cubic_bezier(double x1, double y1, double x2, double y2,
                                 double x,  double y)
{
    m_path.curve4(x1, y1, x2, y2, x, y);
}
//---------------------------------------------------------------------------------------
void BitmapDrawer::cubic_bezier_rel(double x1, double y1, double x2, double y2,
                                     double x,  double y)
{
    m_path.rel_to_abs(&x1, &y1);
    m_path.rel_to_abs(&x2, &y2);
    m_path.rel_to_abs(&x,  &y);
    m_path.curve4(x1, y1, x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::cubic_bezier(double x2, double y2, double x,  double y)
{
    m_path.curve4(x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::cubic_bezier_rel(double x2, double y2, double x,  double y)
{
    m_path.curve4_rel(x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::close_path()
{
    m_path.end_poly(path_flags_close);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::fill(Color color)
{
    PathAttributes& attr = cur_attr();
    attr.fill_color = color;
    attr.fill_mode = k_fill_solid;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::stroke(Color color)
{
    PathAttributes& attr = cur_attr();
    attr.stroke_color = color;
    attr.stroke_flag = true;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::stroke_width(double w)
{
    cur_attr().stroke_width = w;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::fill_none()
{
    cur_attr().fill_mode = k_fill_none;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::stroke_none()
{
    cur_attr().stroke_flag = false;
}

#if (0)
//---------------------------------------------------------------------------------------
void BitmapDrawer::even_odd(bool flag)
{
    cur_attr().even_odd_flag = flag;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::fill_opacity(unsigned op)
{
    cur_attr().fill_color.opacity(op);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::stroke_opacity(unsigned op)
{
    cur_attr().stroke_color.opacity(op);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::line_join(line_join_e join)
{
    cur_attr().line_join = join;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::line_cap(line_cap_e cap)
{
    cur_attr().line_cap = cap;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::miter_limit(double ml)
{
    cur_attr().miter_limit = ml;
}
#endif

//---------------------------------------------------------------------------------------
void BitmapDrawer::add_path(VertexSource& vs,  unsigned path_id,
                            bool UNUSED(solid_path))
{
    m_path.concat_path(vs, path_id);
}

//---------------------------------------------------------------------------------------
bool BitmapDrawer::select_font(const std::string& language,
                               const std::string& fontFile,
                               const std::string& fontName, double height,
                               bool fBold, bool fItalic)
{
    return m_pFonts->select_font(language, fontFile, fontName, height, fBold, fItalic);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::draw_glyph(double x, double y, unsigned int ch)
{
    render_existing_paths();

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    m_pCalligrapher->draw_glyph(x, y, ch, m_textColor, m_pRenderer->get_scale());
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::draw_glyph_rotated(double x, double y, unsigned int ch, double rotation)
{
    render_existing_paths();

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    m_pCalligrapher->draw_glyph_rotated(x, y, ch, m_textColor, m_pRenderer->get_scale(), rotation);
}

//---------------------------------------------------------------------------------------
int BitmapDrawer::draw_text(double x, double y, const std::string& str)
{
    //returns the number of chars drawn

    render_existing_paths();

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    return m_pCalligrapher->draw_text(x, y, str, m_textColor, m_pRenderer->get_scale());
}

//---------------------------------------------------------------------------------------
int BitmapDrawer::draw_text(double x, double y, const wstring& str)
{
    //returns the number of chars drawn

    render_existing_paths();

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    return m_pCalligrapher->draw_text(x, y, str, m_textColor, m_pRenderer->get_scale());
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::device_point_to_model(double* x, double* y) const
{
    //e.g. Pixels to LUnits
    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.inverse_transform(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::model_point_to_device(double* x, double* y) const
{
    //e.g. LUnits to Pixels
    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(x, y);
}

//---------------------------------------------------------------------------------------
LUnits BitmapDrawer::device_units_to_model(double value) const
{
    //e.g. Pixels to LUnits
    TransAffine& mtx = m_pRenderer->get_transform();
    return value / mtx.scale();
}

//---------------------------------------------------------------------------------------
double BitmapDrawer::model_to_device_units(LUnits value) const
{
    //e.g. LUnits to Pixels
    TransAffine& mtx = m_pRenderer->get_transform();
    return value * mtx.scale();
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::reset(Color bgcolor)
{
    m_pRenderer->initialize(m_rbuf, bgcolor);
    delete_paths();
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::set_rendering_buffer(unsigned char* buf, unsigned width,
                                        unsigned height, Color bgcolor)
{
    if (buf && width > 0 && height > 0)
    {
        int pixFmt = m_libraryScope.get_pixel_format();
        int stride = Renderer::bytesPerPixel(pixFmt) * width;
        m_rbuf.attach(buf, width, height, stride);
        m_pBuf = buf;
        m_bufWidth = width;
        m_bufHeight = height;

        reset(bgcolor);
    }
    else
    {
        m_rbuf.attach(nullptr, 0, 0, 0);
        m_pBuf = nullptr;
        m_bufWidth = 0;
        m_bufHeight = 0;
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::set_view_area(unsigned width, unsigned height, unsigned xShift,
                                 unsigned yShift)
{
    if (width > (m_bufWidth - xShift) || height > (m_bufHeight - yShift))
    {
        LOMSE_LOG_ERROR("Invalid view area. Too big. Ignored.");
        return;
    }

    int bytesPerPixel = Renderer::bytesPerPixel( m_libraryScope.get_pixel_format() );

    unsigned shift = yShift * m_bufWidth + xShift;
    unsigned char* start = m_pBuf + shift * bytesPerPixel;
    int stride = m_rbuf.stride();
    m_rbuf.attach(start, width, height, stride);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::new_viewport_origin(double x, double y)
{
    //coordinates in device units (e.g. Pixel)
    Drawer::new_viewport_origin(x, y);
    m_pRenderer->set_viewport(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::new_viewport_size(double x, double y)
{
    //in device units (e.g. Pixel)
    Drawer::new_viewport_size(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::set_affine_transformation(TransAffine& transform)
{
    m_pRenderer->set_transform(transform);
    //m_pRenderer->set_scale(transform.scale());
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::render()
{
    m_pRenderer->render();
    delete_paths();
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::set_shift(LUnits x, LUnits y)
{
    m_pRenderer->set_shift(x, y);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::remove_shift()
{
    m_pRenderer->remove_shift();
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::circle(LUnits xCenter, LUnits yCenter, LUnits radius)
{
    double x = double(xCenter);
    double y = double(yCenter);
    double r = double(radius);

    agg::ellipse e1;
    e1.init(x, y, r, r, 0);
    m_path.concat_path<agg::ellipse>(e1);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
                        LUnits width, ELineEdge nEdge)
{
    double alpha = atan((y2 - y1) / (x2 - x1));

    switch(nEdge)
    {
        case k_edge_normal:
            // edge line is perpendicular to line
            {
            LUnits uIncrX = (LUnits)( (width * sin(alpha)) / 2.0 );
            LUnits uIncrY = (LUnits)( (width * cos(alpha)) / 2.0 );
            UPoint uPoints[] = {
                UPoint(x1+uIncrX, y1-uIncrY),
                UPoint(x1-uIncrX, y1+uIncrY),
                UPoint(x2-uIncrX, y2+uIncrY),
                UPoint(x2+uIncrX, y2-uIncrY)
            };
            polygon(4, uPoints);
            break;
            }

        case k_edge_vertical:
            // edge is always a vertical line
            {
            LUnits uIncrY = (LUnits)( (width / cos(alpha)) / 2.0 );
            UPoint uPoints[] = {
                UPoint(x1, y1-uIncrY),
                UPoint(x1, y1+uIncrY),
                UPoint(x2, y2+uIncrY),
                UPoint(x2, y2-uIncrY)
            };
            polygon(4, uPoints);
            break;
            }

        case k_edge_horizontal:
            // edge is always a horizontal line
            {
            LUnits uIncrX = (LUnits)( (width / sin(alpha)) / 2.0 );
            UPoint uPoints[] = {
                UPoint(x1+uIncrX, y1),
                UPoint(x1-uIncrX, y1),
                UPoint(x2-uIncrX, y2),
                UPoint(x2+uIncrX, y2)
            };
            polygon(4, uPoints);
            break;
            }
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::polygon(int n, UPoint points[])
{
    move_to(points[0].x, points[0].y);
    int i;
    for (i=1; i < n; i++)
    {
        line_to(points[i].x, points[i].y);
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::rect(UPoint pos, USize size, LUnits radius)
{
    double x1 = double(pos.x);
    double y1 = double(pos.y);
    double x2 = x1 + double(size.width);
    double y2 = y1 + double(size.height);
    double r = double(radius);

    agg::rounded_rect rr(x1, y1, x2, y2, r);
    m_path.concat_path<agg::rounded_rect>(rr);
}

//------------------------------------------------------------------------
void BitmapDrawer::render_existing_paths()
{
    if (m_path.total_vertices() > 0)
        render();
}

#if (0)
//------------------------------------------------------------------------
void BitmapDrawer::copy_bitmap(RenderingBuffer& bmap, UPoint dest)
{
    render_existing_paths();

    double x = double(dest.x);
    double y = double(dest.y);
    model_point_to_device(&x, &y);
    m_pRenderer->copy_from(bmap, nullptr, int(x), int(y));
}

//------------------------------------------------------------------------
void BitmapDrawer::copy_bitmap(RenderingBuffer& bmap,
                               Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                               UPoint dest)
{
    render_existing_paths();

    double x = double(dest.x);
    double y = double(dest.y);
    model_point_to_device(&x, &y);

    AggRectInt r(srcX1, srcY1, srcX2, srcY2);
    m_pRenderer->copy_from(bmap, &r, int(x)-srcX1, int(y)-srcY1);
}
#endif

//---------------------------------------------------------------------------------------
void BitmapDrawer::draw_bitmap(RenderingBuffer& bmap, bool hasAlpha,
                               Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                               LUnits dstX1, LUnits dstY1, LUnits dstX2, LUnits dstY2,
                               EResamplingQuality resamplingMode,
                               double alpha)
{
    render_existing_paths();

    double x1 = double(dstX1);
    double y1 = double(dstY1);
    double x2 = double(dstX2);
    double y2 = double(dstY2);
    model_point_to_device(&x1, &y1);
    model_point_to_device(&x2, &y2);

    m_pRenderer->render_bitmap(bmap, hasAlpha, double(srcX1), double(srcY1),
                               double(srcX2), double(srcY2), x1, y1, x2, y2,
                               resamplingMode, alpha);
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::fill_linear_gradient(LUnits x1, LUnits y1, LUnits x2, LUnits y2)
{
    PathAttributes& attr = cur_attr();
    if (!attr.fill_gradient)
        attr.fill_gradient = LOMSE_NEW GradientAttributes();

    double angle = atan2(double(y2-y1), double(x2-x1));
    attr.fill_gradient->transform.reset();
    attr.fill_gradient->transform *= agg::trans_affine_rotation(angle);
    attr.fill_gradient->transform *= agg::trans_affine_translation(x1, y1);

    attr.fill_gradient->d1 = 0.0;
    attr.fill_gradient->d2 =
        sqrt(double(x2-x1) * double(x2-x1) + double(y2-y1) * double(y2-y1));
    attr.fill_mode = k_fill_gradient_linear;
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::gradient_color(Color c1, Color c2, double start, double stop)
{
    PathAttributes& attr = cur_attr();
    if (!attr.fill_gradient)
        attr.fill_gradient = LOMSE_NEW GradientAttributes();

    int iStart = int(255.0 * start);
    int iStop   = int(255.0 * stop);
    if (iStop <= iStart)
        iStop = iStart + 1;
    double k = 1.0 / double(iStop - iStart);

    GradientColors& colors = attr.fill_gradient->colors;
    for (int i = iStart; i < iStop; i++)
    {
        colors[i] = c1.gradient(c2, double(i - iStart) * k);
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::gradient_color(Color c1, double start, double stop)
{
    PathAttributes& attr = cur_attr();
    if (!attr.fill_gradient)
        attr.fill_gradient = LOMSE_NEW GradientAttributes();

    int iStart = int(255.0 * start);
    int iStop   = int(255.0 * stop);
    if (iStop <= iStart)
        iStop = iStart + 1;

    GradientColors& colors = attr.fill_gradient->colors;
    for (int i = iStart; i < iStop; i++)
    {
        colors[i] = c1;
    }
}

//---------------------------------------------------------------------------------------
void BitmapDrawer::line_with_markers(UPoint start, UPoint end, LUnits width,
                                     ELineCap startCap, ELineCap endCap)
{
    //add paths for antialiased line with head/tail markers

    LineVertexSource line(double(start.x), double(start.y),
                          double(end.x), double(end.y) );

    typedef LineCapsConverter<LineVertexSource> MyConverter;

    //width = 100.0f;       //100 = 1 mm
    MyConverter converter(line, double(width), startCap, endCap);
    m_path.concat_path<MyConverter>(converter);
}

//---------------------------------------------------------------------------------------
bool BitmapDrawer::is_ready() const
{
    return (m_pBuf != nullptr) && (m_viewportSize.width > 0.0)
           && (m_viewportSize.height > 0.0);
}





}  //namespace lomse
