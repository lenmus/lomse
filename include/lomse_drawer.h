//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_DRAWER_H__        //to avoid nested includes
#define __LOMSE_DRAWER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_vertex_source.h"
#include "agg_color_rgba.h"     //rgba & rgba8
#include "agg_math_stroke.h"    //line_cap_e & line_join_e
#include "agg_trans_affine.h"   //trans_affine
#include "lomse_shape_base.h"   //enums ELineEdge, ...
#include "lomse_agg_types.h"    //RenderingBuffer
#include "lomse_gm_basic.h"     //for GmoObj::k_max
#include "lomse_path_attributes.h"

#include <string>
#include <bitset>
using namespace std;

using namespace agg;

namespace lomse
{

//forward declarations
class FontStorage;

//---------------------------------------------------------------------------------------
//rendering options. Mainly for debugging
enum ERenderOptions
{
    k_option_draw_box_doc_page_content=0,
    k_option_draw_box_container,
    k_option_draw_box_system,
    k_option_draw_box_slice,
    k_option_draw_box_slice_instr,
    k_option_draw_box_inline_flag,
};


//---------------------------------------------------------------------------------------
// RenderOptions: struct to contain renderization options for a document
//---------------------------------------------------------------------------------------
struct RenderOptions
{
    //for debug: draw a box around boxes of selected types
    bitset<GmoObj::k_max> boxes;

    //bool g_fDrawBoundsShapes;           //draw bound rectangles for non boxes
    //bool g_fDrawSelRect;    //draw selection rectangles around staff objects
    //bool g_fDrawBounds;     //draw bounds rectangles around staff objects
    //bool g_fShowMargins;    //draw margins in scores, so user can change them
    bool draw_anchors;      //draw anchors, to see them in the score
    //bool g_fFreeMove;		//the shapes can be dragged without restrictions


    //-------------------------------------------------------------------------
    // scaling factor
    // To be transferred to the View
    double  scale;

    //colors
    Color background_color;
    Color highlighted_color;
    Color selected_color;

    //document page appearance
    bool page_border_flag;
    bool cast_shadow_flag;

    RenderOptions()
        : draw_anchors(false)
        , scale(1.0)
        , background_color(127, 127, 127)   //grey
        , highlighted_color(255,0,0)        //red
        , selected_color(0,0,255)           //blue
        , page_border_flag(true)
        , cast_shadow_flag(true)
    {
        boxes.reset();
    }

    void reset_boxes_to_draw()
    {
        boxes.reset();
    }

    void draw_box_for(int type)
    {
        boxes[type] = true;
    }

    bool must_draw_box_for(int type)
    {
        return boxes[type];
    }


};


// Values for different attributes
//---------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    enum ELineJoin
    {
        k_miter_join = agg::miter_join,
        k_round_join = agg::round_join,
        k_bevel_join = agg::bevel_join
    };

    //-------------------------------------------------------------------------
    enum ETextAlignment
    {
        k_align_left = 0,
        k_align_right,
        k_align_center,
        k_align_top = k_align_right,
        k_align_bottom = k_align_left
    };

    //-------------------------------------------------------------------------
    enum EBlendMode
    {
        k_blend_alpha       = agg::end_of_comp_op_e,
        k_blend_clear       = agg::comp_op_clear,
        k_blend_src         = agg::comp_op_src,
        k_blend_dst         = agg::comp_op_dst,
        k_blend_src_over    = agg::comp_op_src_over,
        k_blend_dst_over    = agg::comp_op_dst_over,
        k_blend_src_in      = agg::comp_op_src_in,
        k_blend_dst_in      = agg::comp_op_dst_in,
        k_blend_src_out     = agg::comp_op_src_out,
        k_blend_dst_out     = agg::comp_op_dst_out,
        k_blend_src_atop    = agg::comp_op_src_atop,
        k_blend_dst_atop    = agg::comp_op_dst_atop,
        k_blend_xor         = agg::comp_op_xor,
        k_blend_plus        = agg::comp_op_plus,
        k_blend_minus       = agg::comp_op_minus,
        k_blend_multiply    = agg::comp_op_multiply,
        k_blend_screen      = agg::comp_op_screen,
        k_blend_overlay     = agg::comp_op_overlay,
        k_blend_darken      = agg::comp_op_darken,
        k_blend_lighten     = agg::comp_op_lighten,
        k_blend_color_dodge = agg::comp_op_color_dodge,
        k_blend_color_burn  = agg::comp_op_color_burn,
        k_blend_hard_light  = agg::comp_op_hard_light,
        k_blend_soft_light  = agg::comp_op_soft_light,
        k_blend_difference  = agg::comp_op_difference,
        k_blend_exclusion   = agg::comp_op_exclusion,
        k_blend_contrast    = agg::comp_op_contrast,
    };

    //-------------------------------------------------------------------------
    enum EResamplingQuality
    {
        k_quality_low = 0,  ///uses a nearest-neighbour algorithm
        k_quality_medium,   //bilinear, for upsampling and area-averaging for downsampling
        k_quality_high,     //bicubic, for upsampling and area-averaging for downsampling
    };


//---------------------------------------------------------------------------------------
// Drawer: Abstract class for drawers
// A drawer is responsible for transforming drawing commands into a final
// product (bitmap, svg file, paths, etc.)
//
// Implementation is in file lomse_screen_drawer.cpp
class Drawer
{
protected:
    LibraryScope& m_libraryScope;
    FontStorage* m_pFonts;
    Color m_textColor;

public:
    Drawer(LibraryScope& libraryScope);
    virtual ~Drawer() {}

    // SVG path commands
    // http://www.w3.org/TR/SVG/paths.html#PathData
    virtual void begin_path() = 0;                                  //SVG: <path>
    virtual void end_path() = 0;                                    //SVG: </path>
    virtual void close_subpath() = 0;                               //SVG: Z, z
    virtual void move_to(double x, double y) = 0;                   //SVG: M
    virtual void move_to_rel(double x, double y) = 0;               //SVG: m
    virtual void line_to(double x,  double y) = 0;                  //SVG: L
    virtual void line_to_rel(double x,  double y) = 0;              //SVG: l
    virtual void hline_to(double x) = 0;                            //SVG: H
    virtual void hline_to_rel(double x) = 0;                        //SVG: h
    virtual void vline_to(double y) = 0;                            //SVG: V
    virtual void vline_to_rel(double y) = 0;                        //SVG: v
    virtual void cubic_bezier(double x1, double y1,                 //SVG: Q
                              double x, double y) = 0;
    virtual void cubic_bezier_rel(double x1, double y1,             //SVG: q
                                  double x, double y) = 0;
    virtual void cubic_bezier(double x, double y) = 0;              //SVG: T
    virtual void cubic_bezier_rel(double x, double y) = 0;          //SVG: t
    virtual void quadratic_bezier(double x1, double y1,             //SVG: C
                                  double x2, double y2,
                                  double x, double y) = 0;
    virtual void quadratic_bezier_rel(double x1, double y1,         //SVG: c
                                      double x2, double y2,
                                      double x, double y) = 0;
    virtual void quadratic_bezier(double x2, double y2,             //SVG: S
                                  double x, double y) = 0;
    virtual void quadratic_bezier_rel(double x2, double y2,         //SVG: s
                                      double x, double y) = 0;

    //curve?: elliptical_arc (A and a)

    // SVG basic shapes commands
    virtual void rect(UPoint pos, USize size, LUnits radius) = 0;           //SVG: <rect>                                      //SVG: <rect>
    virtual void circle(LUnits xCenter, LUnits yCenter, LUnits radius) = 0; //SVG: <circle>
    //virtual void ellipse() = 0;                                           //SVG: <ellipse>
    virtual void line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
                      LUnits width, ELineEdge nEdge=k_edge_normal) = 0;     //SVG: <line>
    //virtual void polyline() = 0;                                          //SVG: <polyline>
    virtual void polygon(int n, UPoint points[]) = 0;                       //SVG: <polygon>

    // not the same but similar to SVG path command
    virtual void add_path(VertexSource& vs, unsigned path_id = 0, bool solid_path = true) = 0;

    // current font
    virtual bool select_font(const std::string& language,
                             const std::string& fontFile,
                             const std::string& fontName, double height,
                             bool fBold=false, bool fItalic=false) = 0;
    virtual bool select_raster_font(const std::string& language,
                                    const std::string& fontFile,
                                    const std::string& fontName, double height,
                                    bool fBold=false, bool fItalic=false) = 0;
    virtual bool select_vector_font(const std::string& language,
                                    const std::string& fontFile,
                                    const std::string& fontName, double height,
                                    bool fBold=false, bool fItalic=false) = 0;

    // text attributes
    virtual void set_text_color(Color color);

    // text rederization
    virtual int draw_text(double x, double y, const std::string& str) = 0;
    virtual int draw_text(double x, double y, const wstring& str) = 0;
    virtual void draw_glyph(double x, double y, unsigned int ch) = 0;

    //copy/blend a bitmap
    virtual void copy_bitmap(RenderingBuffer& img, UPoint pos) = 0;
    virtual void copy_bitmap(RenderingBuffer& bmap,
                             Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                             UPoint dest) = 0;
    virtual void draw_bitmap(RenderingBuffer& bmap, bool hasAlpha,
                             Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                             LUnits dstX1, LUnits dstY1, LUnits dstX2, LUnits dstY2,
                             EResamplingQuality resamplingMode,
                             double alpha=1.0) = 0;

    //SVG line with start/end markers
    virtual void line_with_markers(UPoint start, UPoint end, LUnits width,
                                   ELineCap startCap, ELineCap endCap) = 0;


    // Attribute setting functions.
    virtual void fill(Color color) = 0;
    virtual void stroke(Color color) = 0;
    virtual void even_odd(bool flag) = 0;
    virtual void stroke_width(double w) = 0;
    virtual void fill_none() = 0;
    virtual void stroke_none() = 0;
    virtual void fill_opacity(unsigned op) = 0;
    virtual void stroke_opacity(unsigned op) = 0;
    virtual void line_join(line_join_e join) = 0;
    virtual void line_cap(line_cap_e cap) = 0;
    virtual void miter_limit(double ml) = 0;
    virtual void fill_linear_gradient(LUnits x1, LUnits y1, LUnits x2, LUnits y2) = 0;
    virtual void gradient_color(Color c1, Color c2, double start, double stop) = 0;
    virtual void gradient_color(Color c1, double start, double stop) = 0;


    // settings
    //-----------------------
    virtual void set_shift(LUnits x, LUnits y) = 0;
    virtual void remove_shift() = 0;
    virtual void render()= 0;


};


}   //namespace lomse

#endif    // __LOMSE_DRAWER_H__

