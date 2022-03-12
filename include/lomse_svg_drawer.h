//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SVG_DRAWER_H__        //to avoid nested includes
#define __LOMSE_SVG_DRAWER_H__

#include "lomse_drawer.h"

//std
#include <sstream>

namespace lomse
{

//---------------------------------------------------------------------------------------
/** %SvgDrawer: a Drawer that renders as svg stream
*/
class LOMSE_EXPORT SvgDrawer : public Drawer
{
private:
    std::ostream&       m_svg;
    bool                m_fStartStream = true;
    std::stringstream   m_attribs;
    std::stringstream   m_path;
    bool                m_fPathEmpty = true;
    TransAffine         m_transform;
    const SvgOptions&   m_options;
    double              m_fontSize = 10;
    std::string         m_fontStyle = "normal";
    std::string         m_fontWeight = "normal";
    std::string         m_fontFamily = "Bravura";

public:
    SvgDrawer(LibraryScope& libraryScope, std::ostream& svgstream, const SvgOptions& opt);
    virtual ~SvgDrawer();

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
    void draw_glyph_rotated(double x, double y, unsigned int ch, double rotation) override;


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


protected:
    std::string to_svg(Color color);
    void add_newline();

};


}   //namespace lomse

#endif    // __LOMSE_SVG_DRAWER_H__

