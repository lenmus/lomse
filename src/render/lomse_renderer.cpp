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

//  This file is based on Anti-Grain Geometry version 2.4 examples' code and on
//  Agg2D version 1.0 code.
//
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed as follows:
//    "Permission to copy, use, modify, sell and distribute this software
//    is granted provided this copyright notice appears in all copies.
//    This software is provided "as is" without express or implied
//    warranty, and with no claim as to its suitability for any purpose."
//
//---------------------------------------------------------------------------------------

#include "lomse_renderer.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// RendererFactory
//---------------------------------------------------------------------------------------
Renderer* RendererFactory::create_renderer(LibraryScope& libraryScope,
                                           AttrStorage& attr_storage,
                                           PathStorage& path)
{
    int pixelFmt = libraryScope.get_pixel_format();
    switch(pixelFmt)
    {
////        case k_pix_format_bw:
////            return LOMSE_NEW RendererTemplate<PixFormat_bw>(libraryScope.get_screen_ppi(),
////                                                         attr_storage, path);
//        case k_pix_format_gray8:
//            return LOMSE_NEW RendererTemplate<PixFormat_gray8>(libraryScope.get_screen_ppi(),
//                                                         attr_storage, path);
//        case k_pix_format_gray16:
//            return LOMSE_NEW RendererTemplate<PixFormat_gray16>(libraryScope.get_screen_ppi(),
//                                                         attr_storage, path);

        case k_pix_format_rgb555:
            return LOMSE_NEW RendererTemplate<PixFormat_rgb555,
                                        PixFormat_rgb555::color_type>
                            (libraryScope.get_screen_ppi(), attr_storage, path);

        case k_pix_format_rgb565:
            return LOMSE_NEW RendererTemplate<PixFormat_rgb565,
                                        PixFormat_rgb565::color_type>
                            (libraryScope.get_screen_ppi(), attr_storage, path);

//        case k_pix_format_rgbAAA:
//            return LOMSE_NEW RendererTemplate<PixFormat_rgbAAA>(libraryScope.get_screen_ppi(),
//                                                         attr_storage, path);
//        case k_pix_format_rgbBBA:
//            return LOMSE_NEW RendererTemplate<PixFormat_rgbBBA>(libraryScope.get_screen_ppi(),
//                                                         attr_storage, path);
//        case k_pix_format_bgrAAA:
//            return LOMSE_NEW RendererTemplate<PixFormat_bgrAAA>(libraryScope.get_screen_ppi(),
//                                                         attr_storage, path);
//        case k_pix_format_bgrABB:
//            return LOMSE_NEW RendererTemplate<PixFormat_bgrABB>(libraryScope.get_screen_ppi(),
//                                                         attr_storage, path);

        case k_pix_format_rgb24:
            return LOMSE_NEW RendererTemplate<PixFormat_rgb24,
                                        PixFormat_rgb24::color_type>
                            (libraryScope.get_screen_ppi(), attr_storage, path);

        //case k_pix_format_bgr24:
        //    return LOMSE_NEW RendererTemplate<PixFormat_bgr24,
        //                                PixFormat_bgr24::color_type>
        //                    (libraryScope.get_screen_ppi(), attr_storage, path);

        case k_pix_format_rgba32:
            return LOMSE_NEW RendererTemplate<agg::pixfmt_rgba32,
                                        agg::pixfmt_rgba32::color_type>
                            (libraryScope.get_screen_ppi(), attr_storage, path);

        case k_pix_format_argb32:
            return LOMSE_NEW RendererTemplate<agg::pixfmt_argb32,
                                        agg::pixfmt_argb32::color_type>
                            (libraryScope.get_screen_ppi(), attr_storage, path);

        //case k_pix_format_abgr32:
        //    return LOMSE_NEW RendererTemplate<PixFormat_abgr32>(libraryScope.get_screen_ppi(),

        case k_pix_format_bgra32:
            return LOMSE_NEW RendererTemplate<PixFormat_bgra32,
                                        PixFormat_bgra32::color_type>
                            (libraryScope.get_screen_ppi(), attr_storage, path);

        //case k_pix_format_rgb48:
        //    return LOMSE_NEW RendererTemplate<PixFormat_rgb48>(libraryScope.get_screen_ppi(),
        //                                                 attr_storage, path);
        //case k_pix_format_bgr48:
        //    return LOMSE_NEW RendererTemplate<PixFormat_bgr48>(libraryScope.get_screen_ppi(),
        //                                                 attr_storage, path);
        //case k_pix_format_rgba64:
        //    return LOMSE_NEW RendererTemplate<PixFormat_rgba64>(libraryScope.get_screen_ppi(),
        //                                                 attr_storage, path);
        //case k_pix_format_argb64:
        //    return LOMSE_NEW RendererTemplate<PixFormat_argb64>(libraryScope.get_screen_ppi(),
        //                                                 attr_storage, path);
        //case k_pix_format_abgr64:
        //    return LOMSE_NEW RendererTemplate<PixFormat_abgr64>(libraryScope.get_screen_ppi(),
        //                                                 attr_storage, path);
        //case k_pix_format_bgra64:
        //    return LOMSE_NEW RendererTemplate<PixFormat_bgra64>(libraryScope.get_screen_ppi(),
        //                                                 attr_storage, path);
        default:
            return nullptr;
    }
}


//---------------------------------------------------------------------------------------
// Renderer
//---------------------------------------------------------------------------------------
Renderer::Renderer(double ppi, AttrStorage& attr_storage, PathStorage& path)
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
    , m_path(path)
{
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
    m_uxShift = (-transform.tx - m_vxOrg) / m_lunitsToPixels;
    m_uyShift = (-transform.ty - m_vyOrg) / m_lunitsToPixels;
    m_userScale = transform.scale();
    set_transformation();
}

//---------------------------------------------------------------------------------------
TransAffine& Renderer::set_transformation()
{
    m_mtx.reset();

    //add shift
    m_mtx *= agg::trans_affine_translation(-m_uxShift, -m_uyShift);

    //scale LUnits -> pixels
    m_mtx *= agg::trans_affine_scaling(m_userScale * m_lunitsToPixels);

    //move origin to current scroll/drag origin
    m_mtx *= agg::trans_affine_translation(-m_vxOrg, -m_vyOrg);

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
void Renderer::reset()
{
    m_path.remove_all();
    m_attr_storage.remove_all();
}


}  //namespace lomse
