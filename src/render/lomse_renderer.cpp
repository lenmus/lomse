//-------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
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
//-------------------------------------------------------------------------------------

#include "lomse_renderer.h"

using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
// Renderer:
// Knows how to render bitmaps and paths created by Calligrapher and Drawer objects
//---------------------------------------------------------------------------------------
Renderer::Renderer(double ppi, AttrStorage& attr_storage, AttrStorage& attr_stack,
                   PathStorage& path)
    : m_expand(0.0)
    , m_gamma(1.0)
    , m_userScale(1.0)
    , m_rotation(0.0)
    , m_uxShift(0.0)
    , m_uyShift(0.0)
    , m_vxOrg(0.0)
    , m_vyOrg(0.0)

    , m_transform()
    , m_mtx()

    , m_rbuf()
    , m_pixFormat(m_rbuf)
    , m_renBase(m_pixFormat)
    , m_renSolid(m_renBase)

    , m_attr_storage(attr_storage)
    , m_attr_stack(attr_stack)
    , m_path(path)
    , m_curved(m_path)
    , m_curved_stroked(m_curved)
    , m_curved_stroked_trans(m_curved_stroked, m_transform)
    , m_curved_trans(m_curved, m_transform)
    , m_curved_trans_contour(m_curved_trans)
{
    //LUnits to pixels conversion factor
    // device units are pixels. Therefore we must convert from LUnits to pixels:
    //      ppi px/inch = ppi/25.4 px/mm = ppi/2540 px/LU
    // example:
    //      96 px/inch = 96/25.4 px/mm = 96/2540 px/LU = 0.037795275 px/LU
    m_lunitsToPixels = ppi/2540.0;
    set_transformation();
}

void Renderer::initialize(RenderingBuffer& buf)
{
    m_rbuf.attach(buf.buf(), buf.width(), buf.height(), buf.stride());
    m_renBase.reset_clipping(true);

    //////set backgound color
    ////Color bgcolor = m_options.background_color;
    ////m_renBase.clear(agg::rgba(bgcolor.r, bgcolor.b, bgcolor.g, bgcolor.a));
    m_renBase.clear(agg::rgba(0.5, 0.5, 0.5));

    reset();
    set_transformation();
}

//---------------------------------------------------------------------------------------
void Renderer::render(bool fillColor)
{
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;

    //set gamma
    ras.gamma(agg::gamma_power(m_gamma));

    //set affine transformation (rotation, scale, translation, skew)
    set_transformation();

    //set expand value for strokes
    expand(m_expand);

    //do renderization
    render(ras, sl, m_renSolid, m_mtx, m_renBase.clip_box(), 1.0);

    ////////render controls
    //////ras.gamma(agg::gamma_none());
    //////agg::render_ctrl(ras, sl, m_renBase, m_expand);
    //////agg::render_ctrl(ras, sl, m_renBase, m_gamma);
    //////agg::render_ctrl(ras, sl, m_renBase, m_rotate);

    //clear paths
    reset();
}

//---------------------------------------------------------------------------------------
void Renderer::render_gsv_text(double x, double y, const char* str)
{
    agg::gsv_text t;
    t.size(10.0);
    t.flip(true);

    agg::conv_stroke<agg::gsv_text> pt(t);
    pt.width(1.5);

    t.start_point(x, y);
    t.text(str);

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;

    ras.gamma(agg::gamma_power(m_gamma));
    ras.add_path(pt);
    m_renSolid.color(agg::rgba(0,0,0));
    agg::render_scanlines(ras, sl, m_renSolid);
}

//---------------------------------------------------------------------------------------
void Renderer::render(FontRasterizer& ras, FontScanline& sl, Color color)
{
    m_renSolid.color( to_rgba(color) );
    agg::render_scanlines(ras, sl, m_renSolid);
}

//---------------------------------------------------------------------------------------
void Renderer::reset()
{
    m_path.remove_all();
    m_attr_storage.remove_all();
    m_attr_stack.remove_all();
}

//---------------------------------------------------------------------------------------
TransAffine& Renderer::set_transformation()
{
    m_mtx.reset();

    //add shift
    m_mtx *= agg::trans_affine_translation(m_uxShift, m_uyShift);

    //scale LUnits -> pixels
    m_mtx *= agg::trans_affine_scaling(m_userScale * m_lunitsToPixels);

    //move origin to current scroll/drag origin
    m_mtx *= agg::trans_affine_translation(m_vxOrg, m_vyOrg);

    return m_mtx;
}

//---------------------------------------------------------------------------------------
void Renderer::remove_shift()
{
    set_shift(0.0f, 0.0f);
    set_transformation();
}

//---------------------------------------------------------------------------------------
void Renderer::set_transform(TransAffine& transform)
{
    m_uxShift = (transform.tx - m_vxOrg) / m_lunitsToPixels;
    m_uyShift = (transform.ty - m_vyOrg) / m_lunitsToPixels;
    m_userScale = transform.scale();
}

//---------------------------------------------------------------------------------------
agg::rgba Renderer::to_rgba(Color c)
{
    double red = double(c.r)/255.0;
    double green = double(c.g)/255.0;
    double blue = double(c.b)/255.0;
    double opacity = double(c.a)/255.0;
    return agg::rgba(red, green, blue, opacity);
}

//---------------------------------------------------------------------------------------
agg::rgba8 Renderer::to_rgba8(Color c)
{
    return agg::rgba8(c.r, c.g, c.b, c.a);
}


}  //namespace lomse
