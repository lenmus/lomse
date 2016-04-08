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

#ifndef __LOMSE_RENDERER_H__        //to avoid nested includes
#define __LOMSE_RENDERER_H__

#include "lomse_basic.h"
#include "lomse_agg_types.h"
#include "lomse_path_attributes.h"
#include "lomse_drawer.h"           //enums EBlendMode, EResamplingQuality

#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgb.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_image_filter_rgba.h"

#include "agg_rounded_rect.h"


namespace lomse
{

//forward declarations
class GraphicModel;


typedef agg::pixfmt_gray8       PixFormat_gray8;
typedef agg::pixfmt_gray16      PixFormat_gray16;
typedef agg::pixfmt_rgb555      PixFormat_rgb555;
typedef agg::pixfmt_rgb565      PixFormat_rgb565;
typedef agg::pixfmt_rgbAAA      PixFormat_rgbAAA;
typedef agg::pixfmt_rgbBBA      PixFormat_rgbBBA;
typedef agg::pixfmt_bgrAAA      PixFormat_bgrAAA;
typedef agg::pixfmt_bgrABB      PixFormat_bgrABB;
typedef agg::pixfmt_rgb24       PixFormat_rgb24;
typedef agg::pixfmt_bgr24       PixFormat_bgr24;
typedef agg::pixfmt_rgba32      PixFormat_rgba32;
typedef agg::pixfmt_argb32      PixFormat_argb32;
typedef agg::pixfmt_abgr32      PixFormat_abgr32;
typedef agg::pixfmt_bgra32      PixFormat_bgra32;
typedef agg::pixfmt_rgb48       PixFormat_rgb48;
typedef agg::pixfmt_bgr48       PixFormat_bgr48;
typedef agg::pixfmt_rgba64      PixFormat_rgba64;
typedef agg::pixfmt_argb64      PixFormat_argb64;
typedef agg::pixfmt_abgr64      PixFormat_abgr64;
typedef agg::pixfmt_bgra64      PixFormat_bgra64;


//---------------------------------------------------------------------------------------
class Renderer
{
protected:
    double m_lunitsToPixels;

    //renderization parameters
    double m_expand;
    double m_gamma;

    //affine transformation parameters
    double m_userScale;         //not including LUnits to pixels conversion factor
    double m_rotation;          //degrees: -180.0 to 180.0
    double m_uxShift;           //additional translation (LUnits)
    double m_uyShift;           //additional translation (LUnits)
    double m_vxOrg;             //current viewport origin (pixels)
    double m_vyOrg;

    TransAffine m_transform;    //specific transform for paths
    TransAffine m_mtx;          //global transform

    AttrStorage& m_attr_storage;
    PathStorage& m_path;


public:
    Renderer(double ppi, AttrStorage& attr_storage, PathStorage& path);
    virtual ~Renderer() {}
    virtual void initialize(RenderingBuffer& buf, Color bgcolor) = 0;
    virtual void render() = 0;
    virtual void render(FontRasterizer& ras, FontScanline& sl, Color color) = 0;
    virtual void render_gsv_text(double x, double y, const char* str) = 0;
    virtual void copy_from(RenderingBuffer& img, const AggRectInt* srcRect,
                           int xDest, int yDest) = 0;
    virtual void blend_from(RenderingBuffer& bmap, const AggRectInt* srcRect,
                            int xShift, int yShift, unsigned alpha) = 0;
    virtual void render_bitmap(RenderingBuffer& bmap, bool hasAlpha,
                               double srcX1, double srcY1, double srcX2, double srcY2,
                               double dstX1, double dstY1, double dstX2, double dstY2,
                               EResamplingQuality resamplingMode,
                               double alpha) = 0;

    // Make all polygons CCW-oriented
    inline void arrange_orientations() {
        m_path.arrange_orientations_all_paths(path_flags_ccw);
    }

    // Expand all polygons
    virtual void expand(double value) = 0;

    unsigned operator [](unsigned idx)
    {
        m_transform = m_attr_storage[idx].transform;
        return m_attr_storage[idx].path_index;
    }

    virtual void get_bounding_rect(double* x1, double* y1, double* x2, double* y2) = 0;

    inline void set_viewport(Pixels x, Pixels y)
    {
        m_vxOrg = double(x);
        m_vyOrg = double(y);
    }

    inline void set_scale(double scale) { m_userScale = scale; }
    inline double get_scale() { return m_userScale; }
    inline void set_shift(LUnits x, LUnits y)
    {
        m_uxShift = x;
        m_uyShift = y;
    }
    void remove_shift();
    inline TransAffine& get_transform() { return m_mtx; }
    void set_transform(TransAffine& transform);

protected:
    TransAffine& set_transformation();

    //void clear_all(Color c);
    void reset();
    agg::rgba to_rgba(Color c);

};


//---------------------------------------------------------------------------------------
// RendererTemplate: Helper class to render paths and texts
// Knows how to render bitmaps and paths created by Calligrapher and Drawer objects
//---------------------------------------------------------------------------------------
template <typename PixFormat, typename ColorType>
class RendererTemplate : public Renderer
{
protected:
    //premultiplied pixel format for blending
    //typedef agg::pixel_formats_rgba<BlenderPre, agg::pixel32_type> PixFormatPre;
    typedef agg::pixfmt_bgra32_pre      PixFormatPre;

    //renderers types
    typedef agg::renderer_base<PixFormat>                   RendererBase;
    typedef agg::renderer_scanline_aa_solid<RendererBase>   RendererSolid;
    typedef agg::renderer_base<PixFormatPre>                RendererBasePre;

    //span interpolators for gradients (gradients always in rgba8 colors)
    typedef agg::span_gradient<agg::rgba8,
                               agg::span_interpolator_linear<>,
                               agg::gradient_x,
                               GradientColors>              LinearGradientSpan;
    typedef agg::span_gradient<agg::rgba8,
                               agg::span_interpolator_linear<>,
                               agg::gradient_circle,
                               GradientColors>              RadialGradientSpan;

    RenderingBuffer         m_rbuf;         //the rendering buffer provided by the user
    PixFormat               m_pixFormat;    //pixel accessor to m_rbuf
    PixFormatPre            m_pixFormatPre;    //pixel accessor to m_rbuf
    RendererBase            m_renBase;      //base renderer associated to m_rbuf
    RendererSolid           m_renSolid;     //solid renderer associated to m_rbuf
    RendererBasePre         m_renBasePre;

    CurvedConverter         m_curved;
    CurvedStroked           m_curved_stroked;
    CurvedStrokedTrans      m_curved_stroked_trans;
    CurvedTrans             m_curved_trans;
    CurvedTransContour      m_curved_trans_contour;

public:
    RendererTemplate(double ppi, AttrStorage& attr_storage, PathStorage& path)
        : Renderer(ppi, attr_storage, path)
        , m_rbuf()
        , m_pixFormat(m_rbuf)       //attach the pixel accessor to the rendering buffer
        , m_pixFormatPre(m_rbuf)       //attach the pixel accessor to the rendering buffer
        , m_renBase(m_pixFormat)    //attach the pixel accessor (and the buffer)
        , m_renSolid(m_renBase)     //attach the base renderer (and the buffer)
        , m_renBasePre(m_pixFormatPre)

        , m_curved(m_path)
        , m_curved_stroked(m_curved)
        , m_curved_stroked_trans(m_curved_stroked, m_transform)
        , m_curved_trans(m_curved, m_transform)
        , m_curved_trans_contour(m_curved_trans)
    {
    }

    //-----------------------------------------------------------------------------------
    ~RendererTemplate() {}

    //-----------------------------------------------------------------------------------
    void initialize(RenderingBuffer& buf, Color bgcolor)
    {
        m_rbuf.attach(buf.buf(), buf.width(), buf.height(), buf.stride());
        m_renBase.reset_clipping(true);

        m_renBase.clear( to_rgba(bgcolor) );

        reset();
        set_transformation();
    }

    //-----------------------------------------------------------------------------------
    void render()
    {
        agg::rasterizer_scanline_aa<> ras;
        agg::scanline_p8 sl;

        //set gamma
        ras.gamma(agg::gamma_power(m_gamma));

        //set affine transformation (rotation, scale, translation, skew)
        set_transformation();

        //set expand value for strokes
        expand(m_expand);

        //do renderization. Method doing renderization is a template member, so that
        //it can be created for different Renderer types.
        double alpha = 1.0;
        render(ras, sl, m_renSolid, m_mtx, m_renBase.clip_box(), alpha);

        ////////render controls
        //////ras.gamma(agg::gamma_none());
        //////agg::render_ctrl(ras, sl, m_renBase, m_expand);
        //////agg::render_ctrl(ras, sl, m_renBase, m_gamma);
        //////agg::render_ctrl(ras, sl, m_renBase, m_rotate);

        //clear paths
        reset();
    }

    //-----------------------------------------------------------------------------------
    void render_gsv_text(double x, double y, const char* str)
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

    //-----------------------------------------------------------------------------------
    void render(FontRasterizer& ras, FontScanline& sl, Color color)
    {
        m_renSolid.color( to_rgba(color) );
        agg::render_scanlines(ras, sl, m_renSolid);
    }

    //-----------------------------------------------------------------------------------
    // Expand all polygons
    void expand(double value) { m_curved_trans_contour.width(value); }

    //-----------------------------------------------------------------------------------
    void get_bounding_rect(double* x1, double* y1, double* x2, double* y2)
    {
        agg::conv_transform<agg::path_storage> trans(m_path, m_transform);
        agg::bounding_rect(trans, *this, 0, m_attr_storage.size(), x1, y1, x2, y2);
    }

    //-----------------------------------------------------------------------------------
    void copy_from(RenderingBuffer& bmap, const AggRectInt* srcRect, int xDest, int yDest)
    {
        m_renBase.copy_from(bmap, srcRect, xDest, yDest);
    }

    //-----------------------------------------------------------------------------------
    void blend_from(RenderingBuffer& bmap, const AggRectInt* srcRect, int xShift,
                    int yShift, unsigned alpha)
    {
        typedef agg::pixfmt_rgba32   ImgPixFmt;
        ImgPixFmt img_pixf(bmap);

        m_renBasePre.blend_from(img_pixf, srcRect, xShift, yShift, alpha);
    }

    //-----------------------------------------------------------------------------------
    void render_bitmap(RenderingBuffer& bmap, bool UNUSED(hasAlpha),
                       double srcX1, double srcY1, double srcX2, double srcY2,
                       double dstX1, double dstY1, double dstX2, double dstY2,
                       EResamplingQuality resamplingMode,
                       double alpha)
    {
        //set affine transformation (rotation, scale, translation, skew)
        set_transformation();

        TransAffine mtx;
        if (true)   //hasAlpha)
            render_bitmap<RendererSolid, true>(m_renSolid,
                                bmap, srcX1, srcY1, srcX2, srcY2,
                                dstX1, dstY1, dstX2, dstY2, resamplingMode, mtx, alpha);
        else
            render_bitmap<RendererSolid, false>(m_renSolid,
                                bmap, srcX1, srcY1, srcX2, srcY2,
                                dstX1, dstY1, dstX2, dstY2, resamplingMode, mtx, alpha);
    }


protected:

    //-----------------------------------------------------------------------------------
    // Rendering. You can specify two additional parameters:
    // trans_affine and opacity. They can be used to transform the whole
    // image and/or to make it translucent.
    template<class Rasterizer, class Scanline, class Renderer>
    void render(Rasterizer& ras,
                Scanline& sl,
                Renderer& ren,
                const TransAffine& mtx,
                const AggRectInt& clipBox,
                double opacity=1.0)
    {
        unsigned i;

        ras.clip_box(clipBox.x1, clipBox.y1, clipBox.x2, clipBox.y2);

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

            if (attr.fill_mode == k_fill_solid)
            {
                ras.reset();
                ras.filling_rule(attr.even_odd_flag ? fill_even_odd : fill_non_zero);
                if(fabs(m_curved_trans_contour.width()) < 0.0001)
                {
                    ras.add_path(m_curved_trans, attr.path_index);
                }
                else
                {
                    m_curved_trans_contour.miter_limit(attr.miter_limit);
                    ras.add_path(m_curved_trans_contour, attr.path_index);
                }

                color = to_rgba(attr.fill_color);
                color.opacity(color.opacity() * opacity);
                ren.color(color);
                agg::render_scanlines(ras, sl, ren);
            }

            else if (attr.fill_mode == k_fill_gradient_linear)
            {
                ras.reset();
                ras.filling_rule(attr.even_odd_flag ? fill_even_odd : fill_non_zero);
                if(fabs(m_curved_trans_contour.width()) < 0.0001)
                {
                    ras.add_path(m_curved_trans, attr.path_index);
                }
                else
                {
                    m_curved_trans_contour.miter_limit(attr.miter_limit);
                    ras.add_path(m_curved_trans_contour, attr.path_index);
                }

                //TODO apply 'opacity' received param to gradient colors
                //------------------------------------
                //define a linear interpolator, to interpolate colors
                TransAffine mtx = attr.fill_gradient->transform;
                mtx *= m_transform;
                mtx.invert();
                agg::span_interpolator_linear<> interpolator(mtx);

                //define the gradien function, linear in this case
                agg::gradient_x     gradientFunction;

                //define a span object using the gradient interpolator, the function
                //and the gradient colors
                LinearGradientSpan span(interpolator,
                                        gradientFunction,
                                        attr.fill_gradient->colors,
                                        attr.fill_gradient->d1,
                                        attr.fill_gradient->d2);

                //define a span_allocator. It is responsible for allocating an array of
                //colors for the length of the span
                typedef agg::span_allocator<agg::rgba8>   SpanAllocatorType;
                SpanAllocatorType spanAllocator;

                //define a renderer using the span allocator and the linear gradient span
                typedef agg::renderer_scanline_aa<RendererBase,
                                                  SpanAllocatorType,
                                                  LinearGradientSpan> RendererLinearGradient;

                RendererLinearGradient renderer(m_renBase, spanAllocator, span);

                //procceed to render using defined renderer
                agg::render_scanlines(ras, sl, renderer);
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
                ras.add_path(m_curved_stroked_trans, attr.path_index);
                color = to_rgba(attr.stroke_color);
                color.opacity(color.opacity() * opacity);
                ren.color(color);
                agg::render_scanlines(ras, sl, ren);
            }
        }
    }

    //-----------------------------------------------------------------------------------
    // Render a bitmap.
    template<class Renderer, bool hasAlpha>
    void render_bitmap(Renderer& UNUSED(ren), RenderingBuffer& bmap,
                       double srcX1, double srcY1, double srcX2, double srcY2,
                       double dstX1, double dstY1, double dstX2, double dstY2,
                       EResamplingQuality quality,
                       const TransAffine& UNUSED(mtx),
                       double UNUSED(alpha) =1.0)
    {
        //pipeline:
        // the path to render is a rectangle, and the span generator fills it with
        // the image. During filling, the image is scaled by img_mtx

        //mtx to clip and scale the image
        double parallelogram[6] = { dstX1, dstY1, dstX2, dstY1, dstX2, dstY2 };
        agg::trans_affine img_mtx(srcX1, srcY1, srcX2, srcY2, parallelogram);
        img_mtx.invert();

        //lineal inerpolator, to re-dimension the image
        typedef agg::span_interpolator_linear<agg::trans_affine> InterpolatorType;
        InterpolatorType interpolator(img_mtx);

        //attach the image to a pixel renderer
        typedef agg::pixfmt_rgba32   ImgPixFmt;
        ImgPixFmt img_pixf(bmap);

        //define an accesor to bitmap pixels
        //Alternatives:
        //  image_accessor_no_clip
        //  image_accessor_clip
        //  image_accessor_clone
        typedef agg::image_accessor_no_clip<ImgPixFmt> img_accessor_type;
        img_accessor_type source(img_pixf);

        //define the rasterizer
        agg::rasterizer_scanline_aa<> ras;
        ras.clip_box(dstX1, dstY1, dstX2, dstY2);
        ras.gamma(agg::gamma_power(m_gamma));

        //add rectangle path
        ras.move_to_d(dstX1, dstY1);
        ras.line_to_d(dstX2, dstY1);
        ras.line_to_d(dstX2, dstY2);
        ras.line_to_d(dstX1, dstY2);

        //define the scanline class we are going to use (u8)
        agg::scanline_u8 sl;

        //define an allocator for spanlines
        agg::span_allocator<ColorType> sa;


        #if (1)     //hasAlpha)  //bitmap has alpha channel: use rgba filter
            //define a span generator to fill lines with the image and do renderization
            if (quality == k_quality_low)
            {
                //nearest-neighbor filter
		        typedef agg::span_image_filter_rgba_nn<img_accessor_type,
                                                    InterpolatorType> span_gen_type;
                span_gen_type sg(source, interpolator);
                agg::render_scanlines_aa(ras, sl, m_renBase, sa, sg);
            }

            else if (quality == k_quality_medium)
            {
                //bilinear filter
		        typedef agg::span_image_filter_rgba_bilinear<img_accessor_type,
                                                    InterpolatorType> span_gen_type;
                span_gen_type sg(source, interpolator);
                agg::render_scanlines_aa(ras, sl, m_renBase, sa, sg);
            }

        #else  //bitmap without alpha channel: use rgb filter
            //define a span generator to fill lines with the image and do renderization
            if (quality == k_quality_low)
            {
                //nearest-neighbor filter
		        typedef agg::span_image_filter_rgb_nn<img_accessor_type,
                                                    InterpolatorType> span_gen_type;
                span_gen_type sg(source, interpolator);
                agg::render_scanlines_aa(ras, sl, m_renBase, sa, sg);
            }
            else if (quality == k_quality_medium)
            {
                //bilinear filter
		        typedef agg::span_image_filter_rgb_bilinear<img_accessor_type,
                                                    InterpolatorType> span_gen_type;
                span_gen_type sg(source, interpolator);
                agg::render_scanlines_aa(ras, sl, m_renBase, sa, sg);
            }
        #endif

        ////test to fill rectangle in solid color
        //m_renSolid.color(agg::rgba(0, 0.3, 0.5, 1.0));
        //agg::render_scanlines(ras, sl, m_renSolid);
    }

};


//---------------------------------------------------------------------------------------
// factory class to create Renderers
class RendererFactory
{
public:
    RendererFactory();

    static Renderer* create_renderer(LibraryScope& libraryScope,
                                     AttrStorage& attr_storage,
                                     PathStorage& path);
};



}   //namespace lomse

#endif    // __LOMSE_RENDERER_H__

