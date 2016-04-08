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

#ifndef __LOMSE_AGG_TYPES_H__        //to avoid nested includes
#define __LOMSE_AGG_TYPES_H__

#include "lomse_pixel_formats.h"
#include "agg_color_rgba.h"     //rgba & rgba8 formats
#include "agg_pixfmt_rgb.h"     //rgb pixel formats
#include "agg_pixfmt_rgba.h"    //rgba pixel formats
#include "agg_pixfmt_gray.h"    //gray pixel formats
#include "agg_pixfmt_rgb_packed.h"
#include "agg_basics.h"         //rect_i, rect_d

#include "agg_math_stroke.h"    //line_cap_e & line_join_e
#include "agg_trans_affine.h"   //trans_affine
#include "agg_path_storage.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_color_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_bounding_rect.h"
#include "agg_gsv_text.h"
#include "agg_span_gradient.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_allocator.h"

#include "lomse_font_cache_manager.h"     //font renderization using FreeType
#include "lomse_font_freetype.h"

using namespace agg;

namespace lomse
{

//---------------------------------------------------------------------------------------
typedef agg::trans_affine       TransAffine;

//typdefs for the conversion pipeline

//path storage: raw points for curves (in-line/off-line)
typedef agg::path_storage                   PathStorage;

//compute curves, transforming control points into a move_to/line_to sequence.
typedef conv_curve<PathStorage>             CurvedConverter;

//add aditional points to lines for line-caps, line-ends and line-joins.
typedef conv_stroke<CurvedConverter>        CurvedStroked;

//apply affine transformation to points
typedef conv_transform<CurvedStroked>       CurvedStrokedTrans;

//apply affine transformation to points
typedef conv_transform<CurvedConverter>     CurvedTrans;

//generate contours instead of strokes
typedef conv_contour<CurvedTrans>           CurvedTransContour;

//to render fonts
typedef lomse::font_engine_freetype_int32           FontEngine;
typedef lomse::font_cache_manager<FontEngine>       FontCacheManager;
typedef FontCacheManager::gray8_adaptor_type        FontRasterizer;
typedef FontCacheManager::gray8_scanline_type       FontScanline;

typedef agg::rendering_buffer  RenderingBuffer;

//rectangles, integer and double
typedef agg::rect_i       AggRectInt;          //rectangle, int
typedef agg::rect_d       AggRectDouble;       //rectangle, double

//array of colors defining a gradient
typedef agg::pod_auto_array<agg::rgba8, 256> GradientColors;


}   //namespace lomse

#endif    // __LOMSE_AGG_TYPES_H__

