//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//  
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//  Credits:
//  -------------------------
//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev 
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SCREEN_RENDERER_H__
#define __LOMSE_SCREEN_RENDERER_H__

#include "agg_path_storage.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_color_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_bounding_rect.h"
#include "agg_rasterizer_scanline_aa.h"

#include "lomse_renderer.h"


using namespace agg;

namespace lomse
{

//forward declarations
class GraphicModel;
struct RenderOptions;
class Drawer;
class AggDrawer;



// Conversion pipeline: a class just to count the number of vertices
//---------------------------------------------------------------------------------------
template<class VertexSource> class conv_count
{
public:
    conv_count(VertexSource& vs) : m_source(&vs), m_count(0) {}

    void count(unsigned n) { m_count = n; }
    unsigned count() const { return m_count; }

    void rewind(unsigned path_id) { m_source->rewind(path_id); }
    unsigned vertex(double* x, double* y) 
    { 
        ++m_count; 
        return m_source->vertex(x, y); 
    }

private:
    VertexSource* m_source;
    unsigned m_count;
};


// PathAttributes: struct to contain attributes for a path
//---------------------------------------------------------------------------------------
struct PathAttributes
{
    unsigned     index;
    rgba8        fill_color;
    rgba8        stroke_color;
    bool         fill_flag;
    bool         stroke_flag;
    bool         even_odd_flag;
    line_join_e  line_join;
    line_cap_e   line_cap;
    double       miter_limit;
    double       stroke_width;
    trans_affine transform;

    // Empty constructor
    PathAttributes()
        : index(0)
        , fill_color(rgba(0,0,0))
        , stroke_color(rgba(0,0,0))
        , fill_flag(true)
        , stroke_flag(false)
        , even_odd_flag(false)
        , line_join(miter_join)
        , line_cap(butt_cap)
        , miter_limit(4.0)
        , stroke_width(1.0)
        , transform()
    {
    }

    // Copy constructor
    PathAttributes(const PathAttributes& attr)
        : index(attr.index)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , fill_flag(attr.fill_flag)
        , stroke_flag(attr.stroke_flag)
        , even_odd_flag(attr.even_odd_flag)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , transform(attr.transform)
    {
    }

    // Copy constructor with new index value
    PathAttributes(const PathAttributes& attr, unsigned idx)
        : index(idx)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , fill_flag(attr.fill_flag)
        , stroke_flag(attr.stroke_flag)
        , even_odd_flag(attr.even_odd_flag)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , transform(attr.transform)
    {
    }
};


// Path container and renderer. 
//---------------------------------------------------------------------------------------
class ScreenRenderer : public Renderer
{
public:


    //the conversion pipeline
    //path storage: raw points for curves (in-line/off-line)
    //step 1. cumpute curves, transforming control points into a move_to/line_to sequence. 
    typedef conv_curve<path_storage>       curved;
    //step 2. count the total number of vertices in the path
    typedef conv_count<curved>             curved_count;
    //step 3. add aditional points to lines for line-caps, line-ends and line-joins.
    //        These additional points will not be counted
    typedef conv_stroke<curved_count>      curved_stroked;
    //step 4. apply affine transformation to points
    typedef conv_transform<curved_stroked> curved_stroked_trans;

    typedef conv_transform<curved_count>   curved_trans;
    typedef conv_contour<curved_trans>     curved_trans_contour;

    typedef pod_bvector<PathAttributes>   attr_storage;

    ////font renderization with FreeType
    //typedef agg::font_engine_freetype_int32       FontEngine;
    //typedef agg::font_cache_manager<FontEngine>   FontCacheManager;
    //typedef FontCacheManager::gray8_adaptor_type  FontRasterizer;
    //typedef FontCacheManager::gray8_scanline_type FontScanline;

private:
    path_storage   m_storage;
    attr_storage   m_attr_storage;
    attr_storage   m_attr_stack;
    trans_affine   m_transform;

    curved                       m_curved;
    curved_count                 m_curved_count;

    curved_stroked               m_curved_stroked;
    curved_stroked_trans         m_curved_stroked_trans;

    curved_trans                 m_curved_trans;
    curved_trans_contour         m_curved_trans_contour;

public:
    ScreenRenderer();
    ~ScreenRenderer();

    void reset();
    //path_storage& get_path_storage() { return m_storage; }
    //attr_storage& get_attr_storage() { return m_attr_storage; }

    unsigned vertex_count() const { return m_curved_count.count(); }

    Drawer* paths_generation_start();
    void paths_generation_end();

    // Make all polygons CCW-oriented
    inline void arrange_orientations() {
        m_storage.arrange_orientations_all_paths(path_flags_ccw);
    }

    // Expand all polygons 
    inline void expand(double value) { m_curved_trans_contour.width(value); }

    unsigned operator [](unsigned idx) {
        m_transform = m_attr_storage[idx].transform;
        return m_attr_storage[idx].index;
    }

    void get_bounding_rect(double* x1, double* y1, double* x2, double* y2) {
        agg::conv_transform<agg::path_storage> trans(m_storage, m_transform);
        agg::bounding_rect(trans, *this, 0, m_attr_storage.size(), x1, y1, x2, y2);
    }

    // Rendering. One can specify two additional parameters: 
    // trans_affine and opacity. They can be used to transform the whole
    // image and/or to make it translucent.
    template<class Rasterizer, class Scanline, class Renderer> 
    void render(Rasterizer& ras, 
                Scanline& sl,
                Renderer& ren, 
                const trans_affine& mtx, 
                const rect_i& cb,
                double opacity=1.0)
    {
        unsigned i;

        ras.clip_box(cb.x1, cb.y1, cb.x2, cb.y2);
        m_curved_count.count(0);

        for(i = 0; i < m_attr_storage.size(); i++)
        {
            const PathAttributes& attr = m_attr_storage[i];
            m_transform = attr.transform;
            m_transform *= mtx;
            double scl = m_transform.scale();
            //m_curved.approximation_method(curve_inc);
            m_curved.approximation_scale(scl);
            m_curved.angle_tolerance(0.0);

            rgba8 color;

            if(attr.fill_flag)
            {
                ras.reset();
                ras.filling_rule(attr.even_odd_flag ? fill_even_odd : fill_non_zero);
                if(fabs(m_curved_trans_contour.width()) < 0.0001)
                {
                    ras.add_path(m_curved_trans, attr.index);
                }
                else
                {
                    m_curved_trans_contour.miter_limit(attr.miter_limit);
                    ras.add_path(m_curved_trans_contour, attr.index);
                }

                color = attr.fill_color;
                color.opacity(color.opacity() * opacity);
                ren.color(color);
                agg::render_scanlines(ras, sl, ren);
            }

            if(attr.stroke_flag)
            {
                m_curved_stroked.width(attr.stroke_width);
                //m_curved_stroked.line_join((attr.line_join == miter_join) ? miter_join_round : attr.line_join);
                m_curved_stroked.line_join(attr.line_join);
                m_curved_stroked.line_cap(attr.line_cap);
                m_curved_stroked.miter_limit(attr.miter_limit);
                m_curved_stroked.inner_join(inner_round);
                m_curved_stroked.approximation_scale(scl);

                // If the *visual* line width is considerable we 
                // turn on processing of curve cusps.
                //---------------------
                if(attr.stroke_width * scl > 1.0)
                {
                    m_curved.angle_tolerance(0.2);
                }
                ras.reset();
                ras.filling_rule(fill_non_zero);
                ras.add_path(m_curved_stroked_trans, attr.index);
                color = attr.stroke_color;
                color.opacity(color.opacity() * opacity);
                ren.color(color);
                agg::render_scanlines(ras, sl, ren);
            }
        }
    }

protected:
    AggDrawer* m_drawer;

    void delete_drawer();

};


}   //namespace lomse

#endif
