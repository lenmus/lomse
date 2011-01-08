//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_AGG_TYPES_H__        //to avoid nested includes
#define __LOMSE_AGG_TYPES_H__

#include "lomse_doorway.h"
#include "agg_color_rgba.h"     //rgba & rgba8
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

#include "lomse_font_cache_manager.h"     //font renderization using FreeType
#include "lomse_font_freetype.h"

using namespace agg;

namespace lomse
{

//---------------------------------------------------------------------------------------
typedef agg::trans_affine       TransAffine;

typedef agg::pixfmt_bgra32                              PixFormat;
typedef agg::renderer_base<PixFormat>                   RendererBase;
typedef agg::renderer_scanline_aa_solid<RendererBase>   RendererSolid;

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



}   //namespace lomse

#endif    // __LOMSE_AGG_TYPES_H__

