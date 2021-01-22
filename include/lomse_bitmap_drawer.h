//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2021. All rights reserved.
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

#ifndef __LOMSE_SCREEN_DRAWER_H__        //to avoid nested includes
#define __LOMSE_SCREEN_DRAWER_H__

#include "lomse_drawer.h"
#include "lomse_pixel_formats.h"
#include "lomse_agg_types.h"
#include "lomse_path_attributes.h"
#include "lomse_font_storage.h"
#include "lomse_calligrapher.h"

using namespace agg;

namespace lomse
{

//forward declarations
class Renderer;


//---------------------------------------------------------------------------------------
/** %BitmapDrawer: a Drawer that renders on a bitmap using agg
*/
class LOMSE_EXPORT BitmapDrawer : public Drawer
{
private:
    AttrStorage     m_attr_storage;
    PathStorage     m_path;
    Renderer*       m_pRenderer;
    TextMeter*      m_pTextMeter;
    Calligrapher*   m_pCalligrapher;
    int             m_numPaths;
    RenderingBuffer m_rbuf;
    unsigned char*  m_pBuf;         //the memory for the bitmap. Owned by user app.
    unsigned        m_bufWidth;
    unsigned        m_bufHeight;

public:
    BitmapDrawer(LibraryScope& libraryScope);
    virtual ~BitmapDrawer();


    //===================================================================
    // Implementation of pure virtual methods in Drawer base class
    //===================================================================

    // SVG path commands
    // http://www.w3.org/TR/SVG/paths.html#PathData
    void begin_path() override;                                  //SVG: <path>
    void end_path() override;                                    //SVG: </path>
    void close_path() override;                               //SVG: Z, z
    // not the same but similar to SVG path element
    void add_path(VertexSource& vs, unsigned path_id = 0, bool solid_path = true) override;
    void move_to(double x, double y) override;                   //SVG: M
    void move_to_rel(double x, double y) override;               //SVG: m
    void line_to(double x,  double y) override;                  //SVG: L
    void line_to_rel(double x,  double y) override;              //SVG: l
    void hline_to(double x) override;                            //SVG: H
    void hline_to_rel(double x) override;                        //SVG: h
    void vline_to(double y) override;                            //SVG: V
    void vline_to_rel(double y) override;                        //SVG: v


    void quadratic_bezier(double x1, double y1,                  //SVG: Q  quadratic Bézier, abs (x1 y1 x y)
                          double x, double y) override;

    void quadratic_bezier_rel(double x1, double y1,              //SVG: q  quadratic Bézier, rel (x1 y1 x y)
                              double x, double y) override;

    void quadratic_bezier(double x, double y) override;          //SVG: T  smooth quadratic Bézier, abs (x y)

    void quadratic_bezier_rel(double x, double y) override;      //SVG: t  smooth quadratic Bézier, rel (x y)

    /** Draws a cubic Bézier curve from the current point to (x,y) using (x1,y1) as
        the control point at the beginning of the curve and (x2,y2) as the control
        point at the end of the curve. All points in absolute coordinates.
        It is equivalent to the C svg command.
    */
    void cubic_bezier(double x1, double y1,                      //SVG: C  cubic Bézier, abs (x1 y1 x2 y2 x y)
                      double x2, double y2,
                      double x, double y) override;

    /** Draws a cubic Bézier curve from the current point to (x,y) using (x1,y1) as
        the control point at the beginning of the curve and (x2,y2) as the control
        point at the end of the curve. All points in absolute coordinates.
        It is equivalent to the c svg command.
    */
    void cubic_bezier_rel(double x1, double y1,                  //SVG: c  cubic Bézier, rel (x1 y1 x2 y2 x y)
                          double x2, double y2,
                          double x, double y) override;

    /** Draws a cubic Bézier curve from the current point to (x,y). The first control
        point is assumed to be the reflection of the second control point on the previous
        command relative to the current point. (If there is no previous command or if
        the previous command was not an C, c, S or s, assume the first control point
        is coincident with the current point.) (x2,y2) is the second control point
        (i.e., the control point at the end of the curve).
        All points in absolute coordinates.
        It is equivalent to the S svg command.
    */
    void cubic_bezier(double x2, double y2,                      //SVG: S  smooth cubic Bézier, abs (x2 y2 x y)
                      double x, double y) override;

    /** Draws a cubic Bézier curve from the current point to (x,y). The first control
        point is assumed to be the reflection of the second control point on the previous
        command relative to the current point. (If there is no previous command or if
        the previous command was not an C, c, S or s, assume the first control point
        is coincident with the current point.) (x2,y2) is the second control point
        (i.e., the control point at the end of the curve).
        All points in relative coordinates.
        It is equivalent to the s svg command.
    */
    void cubic_bezier_rel(double x2, double y2,                  //SVG: s  smooth cubic Bézier, rel (x2 y2 x y)
                          double x, double y) override;


    // SVG basic shapes commands
    void rect(UPoint pos, USize size, LUnits radius) override;               //SVG: <rect>                                      //SVG: <rect>
    void circle(LUnits xCenter, LUnits yCenter, LUnits radius) override;     //SVG: <circle>
    //virtual void ellipse() = 0;                                            //SVG: <ellipse>
    void line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
              LUnits width, ELineEdge nEdge=k_edge_normal) override;         //SVG: <line>
    //virtual void polyline() = 0;                                           //SVG: <polyline>
    void polygon(int n, UPoint points[]) override;                           //SVG: <polygon>
    //SVG line with start/end markers
    void line_with_markers(UPoint start, UPoint end, LUnits width,
                           ELineCap startCap, ELineCap endCap) override;


    // Attribute setting functions.
    void fill(Color color) override;
    void fill_none() override;
    void stroke(Color color) override;
    void stroke_none() override;
    void stroke_width(double w) override;
    void gradient_color(Color c1, Color c2, double start, double stop) override;
    void gradient_color(Color c1, double start, double stop) override;
    void fill_linear_gradient(LUnits x1, LUnits y1, LUnits x2, LUnits y2) override;
    #if (0)
    void even_odd(bool flag) override;
    void fill_opacity(unsigned op) override;
    void stroke_opacity(unsigned op) override;
    void line_join(line_join_e join) override;
    void line_cap(line_cap_e cap) override;
    void miter_limit(double ml) override;
    #endif


    // text rederization
    //-----------------------
    bool select_font(const std::string& language,
                     const std::string& fontFile,
                     const std::string& fontName, double height,
                     bool fBold=false, bool fItalic=false) override;


    int draw_text(double x, double y, const std::string& str) override;
    int draw_text(double x, double y, const wstring& str) override;
    void draw_glyph(double x, double y, unsigned int ch) override;


    //copy/blend a bitmap
    //-----------------------
    #if (0)
    void copy_bitmap(RenderingBuffer& bmap, UPoint pos) override;
    void copy_bitmap(RenderingBuffer& bmap,
                     Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                     UPoint dest) override;
    #endif
    void draw_bitmap(RenderingBuffer& bmap, bool hasAlpha,
                     Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                     LUnits dstX1, LUnits dstY1, LUnits dstX2, LUnits dstY2,
                     EResamplingQuality resamplingMode,
                     double alpha=1.0) override;



    // settings
    //-----------------------
    void set_shift(LUnits x, LUnits y) override;
    void remove_shift() override;
    void render() override;
    void set_affine_transformation(TransAffine& transform) override;

    /** Set the background color and prepare to render a new image.  */
    void reset(Color bgcolor) override;


    // device - model units conversion
    //-------------------------------------
    void device_point_to_model(double* x, double* y) const override;
    void model_point_to_device(double* x, double* y) const override;
    LUnits device_units_to_model(double value) const override;
    double model_to_device_units(LUnits value) const override;


    //info
    //---------------------------------------
    bool is_ready() const override;


    //Viewport info
    //---------------------------------------
    //coordinates in device units (e.g. Pixel)
    void new_viewport_origin(double x, double y) override;
    void new_viewport_size(double x, double y) override;


    //===================================================================
    // Specific methods not in Drawer base class
    //===================================================================

    //Inform about the RenderingBuffer to use and clear it with the desired color.
    void set_rendering_buffer(unsigned char* buf, unsigned width, unsigned height,
                              Color bgcolor=Color(255,255,255));

    //The view area is the region to which drawing is restricted. View area must be
    //given in device coordinates (e.g. Pixel).
    void set_view_area(unsigned width, unsigned height, unsigned xShift, unsigned yShift);

    unsigned char* get_rendering_buffer() { return m_pBuf; };
    unsigned get_rendering_buffer_width() const { return m_bufWidth; };
    unsigned get_rendering_buffer_height() const { return m_bufHeight; };


protected:
    void push_attr();
    void pop_attr();
    PathAttributes& cur_attr();
    void render_existing_paths();
    void delete_paths();

};


}   //namespace lomse

#endif    // __LOMSE_SCREEN_DRAWER_H__

