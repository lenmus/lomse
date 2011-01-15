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

#ifndef __LOMSE_SCREEN_DRAWER_H__        //to avoid nested includes
#define __LOMSE_SCREEN_DRAWER_H__

#include "lomse_drawer.h"
#include "lomse_doorway.h"
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
    AttrStorage     m_attr_stack;
    PathStorage     m_path;
    Renderer*       m_pRenderer;
    TextMeter*      m_pTextMeter;
    Calligrapher*   m_pCalligrapher;

public:
    ScreenDrawer(LibraryScope& libraryScope);
    virtual ~ScreenDrawer();

    // SVG path commands
    // http://www.w3.org/TR/SVG/paths.html#PathData
    void begin_path();                                  //SVG: <path>
    void end_path();                                    //SVG: </path>
    void close_subpath();                               //SVG: Z, z
    void move_to(double x, double y);                   //SVG: M
    void move_to_rel(double x, double y);               //SVG: m
    void line_to(double x,  double y);                  //SVG: L
    void line_to_rel(double x,  double y);              //SVG: l
    void hline_to(double x);                            //SVG: H
    void hline_to_rel(double x);                        //SVG: h
    void vline_to(double y);                            //SVG: V
    void vline_to_rel(double y);                        //SVG: v
    void cubic_bezier(double x1, double y1,             //SVG: Q
                      double x, double y);
    void cubic_bezier_rel(double x1, double y1,         //SVG: q
                          double x, double y);
    void cubic_bezier(double x, double y);              //SVG: T
    void cubic_bezier_rel(double x, double y);          //SVG: t
    void quadratic_bezier(double x1, double y1,         //SVG: C
                          double x2, double y2,
                          double x, double y);
    void quadratic_bezier_rel(double x1, double y1,     //SVG: c
                              double x2, double y2,
                              double x, double y);
    void quadratic_bezier(double x2, double y2,         //SVG: S
                          double x, double y);
    void quadratic_bezier_rel(double x2, double y2,     //SVG: s
                              double x, double y);


    // SVG basic shapes commands
    //virtual void rect() = 0;                                      //SVG: <rect>
    void circle(LUnits xCenter, LUnits yCenter, LUnits radius);     //SVG: <circle>
    //virtual void ellipse() = 0;                                   //SVG: <ellipse>
    void line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
              LUnits width, ELineEdge nEdge=k_edge_normal);         //SVG: <line>
    //virtual void polyline() = 0;                                  //SVG: <polyline>
    virtual void polygon(int n, UPoint points[]);                   //SVG: <polygon>

    // not the same but similar to SVG path command
    void add_path(VertexSource& vs, unsigned path_id = 0, bool solid_path = true);

    // Attribute setting functions.
    //-----------------------
    void fill(Color color);
    void stroke(Color color);
    void even_odd(bool flag);
    void stroke_width(double w);
    void fill_none();
    void stroke_none();
    void fill_opacity(unsigned op);
    void stroke_opacity(unsigned op);
    void line_join(line_join_e join);
    void line_cap(line_cap_e cap);
    void miter_limit(double ml);


    // current font
    //-----------------------
    bool select_font(const std::string& fontName, double height,
                     bool fBold=false, bool fItalic=false);
    bool select_raster_font(const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);
    bool select_vector_font(const std::string& fontName, double height,
                            bool fBold=false, bool fItalic=false);

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
    int draw_text(double x, double y, const std::string& str);
    void draw_glyph(double x, double y, unsigned int ch);

    //void FtSetTextPosition(lmLUnits uxPos, lmLUnits uyPos);
    //void FtSetTextPositionPixels(lmPixels vxPos, lmPixels vyPos);
    //void FtGetTextExtent(const wxString& sText, lmLUnits* pWidth, lmLUnits* pHeight,
    //                     lmLUnits* pDescender = NULL, lmLUnits* pAscender = NULL);
    //lmURect FtGetGlyphBounds(unsigned int nGlyph);
    //wxRect FtGetGlyphBoundsInPixels(unsigned int nGlyph);

    // point conversion
    //-----------------------
    void screen_point_to_model(double* x, double* y) const;
    void model_point_to_screen(double* x, double* y) const;

    //units conversion
    double PixelsToLUnits(Pixels value);

    // settings
    //-----------------------
    void reset(RenderingBuffer& buf);
    void set_viewport(Pixels x, Pixels y);
    void set_transform(TransAffine& transform);
    void set_shift(LUnits x, LUnits y);
    void remove_shift();
    void render(bool fillColor);
    void render(FontRasterizer& ras, FontScanline& sl, Color color);


protected:
    void push_attr();
    void pop_attr();
    PathAttributes& cur_attr();

};


}   //namespace lomse

#endif    // __LOMSE_SCREEN_DRAWER_H__

