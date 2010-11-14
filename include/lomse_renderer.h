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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_RENDERER_H__        //to avoid nested includes
#define __LOMSE_RENDERER_H__

#include "lomse_basic.h"
#include "lomse_agg_types.h"
#include "lomse_path_attributes.h"


namespace lomse
{

//forward declarations
class GraphicModel;
//class GmoObj;
//struct rgba16;



//---------------------------------------------------------------------------------------
// Renderer: Helper class to render paths and texts
//---------------------------------------------------------------------------------------
class Renderer
{
public:
    Renderer(AttrStorage& attr_storage, AttrStorage& attr_stack, PathStorage& path);
    virtual ~Renderer() {}

    void initialize(RenderingBuffer& buf);
    void render(bool fillColor);
    void render(FontRasterizer& ras, FontScanline& sl);
    void render_gsv_text(double x, double y, const char* str);


    // Make all polygons CCW-oriented
    inline void arrange_orientations() {
        m_path.arrange_orientations_all_paths(path_flags_ccw);
    }

    // Expand all polygons 
    inline void expand(double value) { m_curved_trans_contour.width(value); }

    unsigned operator [](unsigned idx)
    {
        m_transform = m_attr_storage[idx].transform;
        return m_attr_storage[idx].index;
    }

    void get_bounding_rect(double* x1, double* y1, double* x2, double* y2)
    {
        agg::conv_transform<agg::path_storage> trans(m_path, m_transform);
        agg::bounding_rect(trans, *this, 0, m_attr_storage.size(), x1, y1, x2, y2);
    }

    void set_viewport(Pixels x, Pixels y)
    {
        m_vxOrg = double(x);
        m_vyOrg = double(y);
    }

    inline void set_scale(double scale) { m_scale = scale; }
    TransAffine& get_transform() { return set_transformation(); }
    inline double get_scale() { return m_scale; }



protected:
    void push_attr();
    void pop_attr();
    PathAttributes& cur_attr();
    TransAffine& set_transformation();

    //void clear_all(Color c);
    void reset();


    // Rendering. One can specify two additional parameters: 
    // trans_affine and opacity. They can be used to transform the whole
    // image and/or to make it translucent.
    template<class Rasterizer, class Scanline, class Renderer> 
    void render(Rasterizer& ras, 
                Scanline& sl,
                Renderer& ren, 
                const TransAffine& mtx, 
                const rect_i& cb,
                double opacity=1.0)
    {
        unsigned i;

        ras.clip_box(cb.x1, cb.y1, cb.x2, cb.y2);

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

private:

    //renderization parameters
    double m_expand;
    double m_gamma;
    double m_scale;
    double m_rotation;

    //image bounding area (LUnits)
    double m_uxMin; 
    double m_uyMin;
    double m_uxMax;
    double m_uyMax;

    //current viewport origin (pixels)
    double m_vxOrg;      
    double m_vyOrg;



    RenderingBuffer         m_rbuf;
    PixFormat               m_pixFormat;
    RendererBase            m_renBase;
    RendererSolid           m_renSolid;

    AttrStorage&            m_attr_storage;      
    AttrStorage&            m_attr_stack;
    PathStorage&            m_path;

    TransAffine             m_transform;
    TransAffine             m_mtx;
    CurvedConverter         m_curved;
    CurvedStroked           m_curved_stroked;
    CurvedStrokedTrans      m_curved_stroked_trans;
    CurvedTrans             m_curved_trans;
    CurvedTransContour      m_curved_trans_contour;

};




}   //namespace lomse

#endif    // __LOMSE_RENDERER_H__

