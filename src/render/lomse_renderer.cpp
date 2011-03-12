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
// RendererFactory
//---------------------------------------------------------------------------------------
Renderer* RendererFactory::create_renderer(LibraryScope& libraryScope,
                                           AttrStorage& attr_storage,
                                           AttrStorage& attr_stack,
                                           PathStorage& path)
{
    int pixelFmt = libraryScope.get_pixel_format();
    switch(pixelFmt)
    {
//        case k_pix_format_bw:
//            return new RendererTemplate<PixFormat_bw>(libraryScope.get_screen_ppi(),
//                                                         attr_storage, attr_stack, path);
        case k_pix_format_gray8:
            return new RendererTemplate<PixFormat_gray8>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_gray16:
            return new RendererTemplate<PixFormat_gray16>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgb555:
            return new RendererTemplate<PixFormat_rgb555>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgb565:
            return new RendererTemplate<PixFormat_rgb565>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgbAAA:
            return new RendererTemplate<PixFormat_rgbAAA>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgbBBA:
            return new RendererTemplate<PixFormat_rgbBBA>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_bgrAAA:
            return new RendererTemplate<PixFormat_bgrAAA>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_bgrABB:
            return new RendererTemplate<PixFormat_bgrABB>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgb24:
            return new RendererTemplate<PixFormat_rgb24>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_bgr24:
            return new RendererTemplate<PixFormat_bgr24>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgba32:
            return new RendererTemplate<PixFormat_rgba32>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_argb32:
            return new RendererTemplate<PixFormat_argb32>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_abgr32:
            return new RendererTemplate<PixFormat_abgr32>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_bgra32:
            return new RendererTemplate<PixFormat_bgra32>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgb48:
            return new RendererTemplate<PixFormat_rgb48>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_bgr48:
            return new RendererTemplate<PixFormat_bgr48>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_rgba64:
            return new RendererTemplate<PixFormat_rgba64>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_argb64:
            return new RendererTemplate<PixFormat_argb64>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_abgr64:
            return new RendererTemplate<PixFormat_abgr64>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        case k_pix_format_bgra64:
            return new RendererTemplate<PixFormat_bgra64>(libraryScope.get_screen_ppi(),
                                                         attr_storage, attr_stack, path);
        default:
            return NULL;
    }
}


//---------------------------------------------------------------------------------------
// Renderer
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

    , m_attr_storage(attr_storage)
    , m_attr_stack(attr_stack)
    , m_path(path)
{
    //LUnits to pixels conversion factor
    // device units are pixels. Therefore we must convert from LUnits to pixels:
    //      ppi px/inch = ppi/25.4 px/mm = ppi/2540 px/LU
    // example:
    //      96 px/inch = 96/25.4 px/mm = 96/2540 px/LU = 0.037795275 px/LU
    m_lunitsToPixels = ppi/2540.0;
    set_transformation();
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

//---------------------------------------------------------------------------------------
void Renderer::reset()
{
    m_path.remove_all();
    m_attr_storage.remove_all();
    m_attr_stack.remove_all();
}





}  //namespace lomse
