//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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


// ScreenDrawer: a Drawer that renders on screen using agg
//---------------------------------------------------------------------------------------
class LOMSE_EXPORT ScreenDrawer : public Drawer
{
private:
    AttrStorage     m_attr_storage;
    PathStorage     m_path;
    Renderer*       m_pRenderer;
    TextMeter*      m_pTextMeter;
    Calligrapher*   m_pCalligrapher;
    int             m_numPaths;

public:
    ScreenDrawer(LibraryScope& libraryScope);
    virtual ~ScreenDrawer();

    // SVG path commands
    // http://www.w3.org/TR/SVG/paths.html#PathData
    void begin_path() override;                                  //SVG: <path>
    void end_path() override;                                    //SVG: </path>
    void close_subpath() override;                               //SVG: Z, z
    void move_to(double x, double y) override;                   //SVG: M
    void move_to_rel(double x, double y) override;               //SVG: m
    void line_to(double x,  double y) override;                  //SVG: L
    void line_to_rel(double x,  double y) override;              //SVG: l
    void hline_to(double x) override;                            //SVG: H
    void hline_to_rel(double x) override;                        //SVG: h
    void vline_to(double y) override;                            //SVG: V
    void vline_to_rel(double y) override;                        //SVG: v
    void cubic_bezier(double x1, double y1,                      //SVG: Q
                      double x, double y) override;
    void cubic_bezier_rel(double x1, double y1,                  //SVG: q
                          double x, double y) override;
    void cubic_bezier(double x, double y) override;              //SVG: T
    void cubic_bezier_rel(double x, double y) override;          //SVG: t
    void quadratic_bezier(double x1, double y1,                  //SVG: C
                          double x2, double y2,
                          double x, double y) override;
    void quadratic_bezier_rel(double x1, double y1,              //SVG: c
                              double x2, double y2,
                              double x, double y) override;
    void quadratic_bezier(double x2, double y2,                  //SVG: S
                          double x, double y) override;
    void quadratic_bezier_rel(double x2, double y2,              //SVG: s
                              double x, double y) override;


    // SVG basic shapes commands
    void rect(UPoint pos, USize size, LUnits radius) override;               //SVG: <rect>                                      //SVG: <rect>
    void circle(LUnits xCenter, LUnits yCenter, LUnits radius) override;     //SVG: <circle>
    //virtual void ellipse() = 0;                                   //SVG: <ellipse>
    void line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
              LUnits width, ELineEdge nEdge=k_edge_normal) override;         //SVG: <line>
    //virtual void polyline() = 0;                                  //SVG: <polyline>
    virtual void polygon(int n, UPoint points[]) override;                   //SVG: <polygon>

    // not the same but similar to SVG path command
    void add_path(VertexSource& vs, unsigned path_id = 0, bool solid_path = true) override;


    // Attribute setting functions.
    void fill(Color color) override;
    void stroke(Color color) override;
    void even_odd(bool flag) override;
    void stroke_width(double w) override;
    void fill_none() override;
    void stroke_none() override;
    void fill_opacity(unsigned op) override;
    void stroke_opacity(unsigned op) override;
    void line_join(line_join_e join) override;
    void line_cap(line_cap_e cap) override;
    void miter_limit(double ml) override;
    void fill_linear_gradient(LUnits x1, LUnits y1, LUnits x2, LUnits y2) override;
    void gradient_color(Color c1, Color c2, double start, double stop) override;
    void gradient_color(Color c1, double start, double stop) override;


    // current font
    bool select_font(const std::string& language,
                     const std::string& fontFile,
                     const std::string& fontName, double height,
                     bool fBold=false, bool fItalic=false) override;
    bool select_raster_font(const std::string& language,
                            const std::string& fontFile,
                            const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false) override;
    bool select_vector_font(const std::string& language,
                            const std::string& fontFile,
                            const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false) override;

    //inline void FtSetFontSize(double rPoints) { return m_pMFM->SetFontSize(rPoints); }
    //inline void FtSetFontHeight(double rPoints) { return m_pMFM->SetFontHeight(rPoints); }
    //inline void FtSetFontWidth(double rPoints) { return m_pMFM->SetFontWidth(rPoints); }
    //void font_family();
    //void font_size();
    //void font_style();  // normal | italic
    //void font_weight(); // normal | bold

    //// text settings
    ////-----------------------
    ////void flip_text(bool flip);
    //void set_text_alignment(ETextAlignment align_x, ETextAlignment align_y);
    //bool get_text_hints() const;
    //void set_text_hints(bool hints);
    //double get_text_width(const char* str);

    // text attributes

    // text rederization
    void gsv_text(double x, double y, const char* str);
    int draw_text(double x, double y, const std::string& str) override;
    int draw_text(double x, double y, const wstring& str) override;
    void draw_glyph(double x, double y, unsigned int ch) override;

    //void FtSetTextPosition(lmLUnits uxPos, lmLUnits uyPos);
    //void FtSetTextPositionPixels(lmPixels vxPos, lmPixels vyPos);
    //void FtGetTextExtent(const wxString& sText, lmLUnits* pWidth, lmLUnits* pHeight,
    //                     lmLUnits* pDescender = nullptr, lmLUnits* pAscender = nullptr);
    //lmURect FtGetGlyphBounds(unsigned int nGlyph);
    //wxRect FtGetGlyphBoundsInPixels(unsigned int nGlyph);

    //copy/blend a bitmap
    //-----------------------
    void copy_bitmap(RenderingBuffer& bmap, UPoint pos) override;
    void copy_bitmap(RenderingBuffer& bmap,
                     Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                     UPoint dest) override;
    void draw_bitmap(RenderingBuffer& bmap, bool hasAlpha,
                     Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                     LUnits dstX1, LUnits dstY1, LUnits dstX2, LUnits dstY2,
                     EResamplingQuality resamplingMode,
                     double alpha=1.0) override;


    //SVG line with start/end markers
    //-----------------------
    void line_with_markers(UPoint start, UPoint end, LUnits width,
                           ELineCap startCap, ELineCap endCap) override;


    // point conversion
    //-----------------------
    void screen_point_to_model(double* x, double* y) const;
    void model_point_to_screen(double* x, double* y) const;

    //units conversion
    LUnits Pixels_to_LUnits(Pixels value);
    Pixels LUnits_to_Pixels(double value);

    // settings
    //-----------------------
    void reset(RenderingBuffer& buf, Color bgcolor);
    void set_viewport(Pixels x, Pixels y);
    void set_transform(TransAffine& transform);
    void set_shift(LUnits x, LUnits y) override;
    void remove_shift() override;
    void render() override;
    void render(FontRasterizer& ras, FontScanline& sl, Color color);


protected:
    void push_attr();
    void pop_attr();
    PathAttributes& cur_attr();
    void render_existing_paths();
    void delete_paths();

};


}   //namespace lomse

#endif    // __LOMSE_SCREEN_DRAWER_H__

