//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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

#ifndef __LOMSE_AGG_DRAWER_H__        //to avoid nested includes
#define __LOMSE_AGG_DRAWER_H__

#include "lomse_drawer.h"

#include "agg_basics.h"
#include "agg_path_storage.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_color_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_bounding_rect.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_array.h"

using namespace agg;

namespace lomse
{

//forward declarations
struct PathAttributes;


// AggDrawer: a Drawer that creates agg paths to be rendered with agg
//---------------------------------------------------------------------------------------
class AggDrawer : public Drawer
{
public:
    typedef pod_bvector<PathAttributes>   attr_storage;

private:
    path_storage&   m_storage;          //raw path points
    attr_storage&   m_attr_storage;      
    attr_storage    m_attr_stack;

public:
    AggDrawer(path_storage& storage, attr_storage& attr_storage);
    virtual ~AggDrawer();

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

    // not the same but similar to SVG path command 
    void add_path(VertexSource& vs, unsigned path_id = 0, bool solid_path = true);

    // Attribute setting functions.
    void fill(const rgba8& f);
    void stroke(const rgba8& s);
    void even_odd(bool flag);
    void stroke_width(double w);
    void fill_none();
    void stroke_none();
    void fill_opacity(double op);
    void stroke_opacity(double op);
    void line_join(line_join_e join);
    void line_cap(line_cap_e cap);
    void miter_limit(double ml);
    trans_affine& transform();


    // Text
    //-----------------------
    //void flip_text(bool flip);
    //void font(const char* fileName, double height, bool bold = false, bool italic = false,
    //          FontCacheType ch = RasterFontCache, double angle = 0.0);
    //double font_height() const;
    //void text_alignment(TextAlignment alignX, TextAlignment alignY);
    //bool text_hints() const;
    //void text_hints(bool hints);
    //double text_width(const char* str);
    void text(double x, double y, const char* str, bool roundOff=false, double dx=0.0,
              double dy=0.0);

    ////Font attributes
    //void font_family();
    //void font_size();
    ////void font_size_adjust();
    ////void font_stretch();
    //void font_style();  // normal | italic
    ////void font_variant();
    //void font_weight(); // normal | bold 




protected:
    void push_attr();
    void pop_attr();
    PathAttributes& cur_attr();


    //text
    double m_textAngle;
    ETextAlignment m_textAlignX;
    ETextAlignment m_textAlignY;
    bool m_textHints;
    double m_fontHeight;
    double m_fontAscent;
    double m_fontDescent;
    //FontCacheType m_fontCacheType;

};


}   //namespace lomse

#endif    // __LOMSE_AGG_DRAWER_H__

