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

#ifndef __LOMSE_DRAWER_H__        //to avoid nested includes
#define __LOMSE_DRAWER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_vertex_source.h"
#include "agg_color_rgba.h"     //rgba & rgba8
#include "agg_math_stroke.h"    //line_cap_e & line_join_e
#include "agg_trans_affine.h"   //trans_affine
#include "lomse_shape_base.h"   //enums ELineEdge, ...

#include <string>
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
    //for debug: draw a border around boxes
    bool draw_box_doc_page_content_flag;    //draw bounds for page content box
    bool draw_box_container_flag;           //draw bounds for BoxContainer/BoxContent
    bool draw_box_system_flag;              //draw bounds for systems
    bool draw_box_slice_flag;               //draw bounds for slices
    bool draw_box_slice_instr_flag;         //draw bounds for SliceInstr
    bool draw_box_inline_flag;              //draw bounds for InlineBox

    //bool g_fDrawBoundsShapes;           //draw bound rectangles for non boxes
    //bool g_fDrawSelRect;    //draw selection rectangles around staff objects
    //bool g_fDrawBounds;     //draw bounds rectangles around staff objects
    //bool g_fShowMargins;    //draw margins in scores, so user can change them
    //bool g_fDrawAnchors;    //draw anchors, to see them in the score
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
        : draw_box_doc_page_content_flag(false)
        , draw_box_container_flag(false)
        , draw_box_system_flag(false)
        , draw_box_slice_flag(false)
        , draw_box_slice_instr_flag(false)
        , draw_box_inline_flag(false)
        , scale(1.0)
        , background_color(127, 127, 127)   //grey
        , highlighted_color(255,0,0)        //red
        , selected_color(0,0,255)           //blue
        , page_border_flag(true)
        , cast_shadow_flag(true)
    {}

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
        k_align_left,
        k_align_right,
        k_align_center,
        k_align_top = k_align_right,
        k_align_bottom = k_align_left
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
    virtual bool select_font(const std::string& fontName, double height,
                             bool fBold=false, bool fItalic=false) = 0;
    virtual bool select_raster_font(const std::string& fontName, double height,
                                    bool fBold=false, bool fItalic=false) = 0;
    virtual bool select_vector_font(const std::string& fontName, double height,
                                    bool fBold=false, bool fItalic=false) = 0;

    // text attributes
    virtual void set_text_color(Color color);

    // text rederization
    virtual int draw_text(double x, double y, const std::string& str) = 0;
    virtual void draw_glyph(double x, double y, unsigned int ch) = 0;

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


    // settings
    //-----------------------
    virtual void set_shift(LUnits x, LUnits y) = 0;
    virtual void remove_shift() = 0;
    virtual void render(bool fillColor)= 0;




};


}   //namespace lomse

#endif    // __LOMSE_DRAWER_H__

