//-------------------------------------------------------------------------------------
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
//  Credits:
//  -------------------------
//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev 
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//-------------------------------------------------------------------------------------

#include "lomse_agg_drawer.h"

#include "lomse_exceptions.h"
#include "lomse_screen_renderer.h"      //PathAttributes
//#include "lomse_gm_basic.h"

//using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
AggDrawer::AggDrawer(path_storage& storage, attr_storage& attr_storage)
    : Drawer()
    , m_storage(storage)
    , m_attr_storage(attr_storage)
{
}

//---------------------------------------------------------------------------------------
AggDrawer::~AggDrawer()
{
}

////---------------------------------------------------------------------------------------
//void AggDrawer::remove_all()
//{
//    m_storage.remove_all();
//    m_attr_storage.remove_all();
//    m_attr_stack.remove_all();
//    m_transform.reset();
//}

//---------------------------------------------------------------------------------------
void AggDrawer::begin_path()
{
    push_attr();
    unsigned idx = m_storage.start_new_path();
    m_attr_storage.add(PathAttributes(cur_attr(), idx));
}

//---------------------------------------------------------------------------------------
void AggDrawer::end_path()
{
    if(m_attr_storage.size() == 0) 
    {
        throw exception("end_path : The path was not begun");
    }
    PathAttributes attr = cur_attr();
    unsigned idx = m_attr_storage[m_attr_storage.size() - 1].index;
    attr.index = idx;
    m_attr_storage[m_attr_storage.size() - 1] = attr;
    pop_attr();
}

//---------------------------------------------------------------------------------------
void AggDrawer::move_to(double x, double y)
{
    m_storage.move_to(x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::move_to_rel(double x, double y)
{
    m_storage.rel_to_abs(&x, &y);
    m_storage.move_to(x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::line_to(double x,  double y)
{
    m_storage.line_to(x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::line_to_rel(double x,  double y)
{
    m_storage.rel_to_abs(&x, &y);
    m_storage.line_to(x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::hline_to(double x)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_storage.total_vertices())
    {
        m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
        m_storage.line_to(x, y2);
    }
}

//---------------------------------------------------------------------------------------
void AggDrawer::hline_to_rel(double x)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_storage.total_vertices())
    {
        m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
        x += x2;
        m_storage.line_to(x, y2);
    }
}

//---------------------------------------------------------------------------------------
void AggDrawer::vline_to(double y)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_storage.total_vertices())
    {
        m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
        m_storage.line_to(x2, y);
    }
}

//---------------------------------------------------------------------------------------
void AggDrawer::vline_to_rel(double y)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_storage.total_vertices())
    {
        m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
        y += y2;
        m_storage.line_to(x2, y);
    }
}

//---------------------------------------------------------------------------------------
void AggDrawer::cubic_bezier(double x1, double y1, double x,  double y)
{
    m_storage.curve3(x1, y1, x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::cubic_bezier_rel(double x1, double y1, double x,  double y)
{
    m_storage.rel_to_abs(&x1, &y1);
    m_storage.rel_to_abs(&x,  &y);
    m_storage.curve3(x1, y1, x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::cubic_bezier(double x, double y)
{
    m_storage.curve3(x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::cubic_bezier_rel(double x, double y)
{
    m_storage.curve3_rel(x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::quadratic_bezier(double x1, double y1, double x2, double y2, 
                                 double x,  double y)
{
    m_storage.curve4(x1, y1, x2, y2, x, y);
}
//---------------------------------------------------------------------------------------
void AggDrawer::quadratic_bezier_rel(double x1, double y1, double x2, double y2, 
                                     double x,  double y)
{
    m_storage.rel_to_abs(&x1, &y1);
    m_storage.rel_to_abs(&x2, &y2);
    m_storage.rel_to_abs(&x,  &y);
    m_storage.curve4(x1, y1, x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::quadratic_bezier(double x2, double y2, double x,  double y)
{
    m_storage.curve4(x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::quadratic_bezier_rel(double x2, double y2, double x,  double y)
{
    m_storage.curve4_rel(x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void AggDrawer::close_subpath()
{
    m_storage.end_poly(path_flags_close);
}

//---------------------------------------------------------------------------------------
PathAttributes& AggDrawer::cur_attr()
{
    if(m_attr_stack.size() == 0)
    {
        throw exception("cur_attr : Attribute stack is empty");
    }
    return m_attr_stack[m_attr_stack.size() - 1];
}

//---------------------------------------------------------------------------------------
void AggDrawer::push_attr()
{
    m_attr_stack.add(m_attr_stack.size() ? 
                        m_attr_stack[m_attr_stack.size() - 1] :
                        PathAttributes());
}

//---------------------------------------------------------------------------------------
void AggDrawer::pop_attr()
{
    if(m_attr_stack.size() == 0)
    {
        throw exception("pop_attr : Attribute stack is empty");
    }
    m_attr_stack.remove_last();
}

//---------------------------------------------------------------------------------------
void AggDrawer::fill(const rgba8& f)
{
    PathAttributes& attr = cur_attr();
    attr.fill_color = f;
    attr.fill_flag = true;
}

//---------------------------------------------------------------------------------------
void AggDrawer::stroke(const rgba8& s)
{
    PathAttributes& attr = cur_attr();
    attr.stroke_color = s;
    attr.stroke_flag = true;
}

//---------------------------------------------------------------------------------------
void AggDrawer::even_odd(bool flag)
{
    cur_attr().even_odd_flag = flag;
}

//---------------------------------------------------------------------------------------
void AggDrawer::stroke_width(double w)
{
    cur_attr().stroke_width = w;
}

//---------------------------------------------------------------------------------------
void AggDrawer::fill_none()
{
    cur_attr().fill_flag = false;
}

//---------------------------------------------------------------------------------------
void AggDrawer::stroke_none()
{
    cur_attr().stroke_flag = false;
}

//---------------------------------------------------------------------------------------
void AggDrawer::fill_opacity(double op)
{
    cur_attr().fill_color.opacity(op);
}

//---------------------------------------------------------------------------------------
void AggDrawer::stroke_opacity(double op)
{
    cur_attr().stroke_color.opacity(op);
}

//---------------------------------------------------------------------------------------
void AggDrawer::line_join(line_join_e join)
{
    cur_attr().line_join = join;
}

//---------------------------------------------------------------------------------------
void AggDrawer::line_cap(line_cap_e cap)
{
    cur_attr().line_cap = cap;
}

//---------------------------------------------------------------------------------------
void AggDrawer::miter_limit(double ml)
{
    cur_attr().miter_limit = ml;
}

//---------------------------------------------------------------------------------------
trans_affine& AggDrawer::transform()
{
    return cur_attr().transform;
}

//---------------------------------------------------------------------------------------
void AggDrawer::add_path(VertexSource& vs,  unsigned path_id, bool solid_path)
{
    m_storage.concat_path(vs, path_id);
}




////---------------------------------------------------------------------------------------
//void AggDrawer::flip_text(bool flip)
//{
//    m_fontEngine.flip_y(flip);
//}
//
////---------------------------------------------------------------------------------------
//void AggDrawer::font(const char* fontName, double height, bool bold, bool italic,
//                     FontCacheType ch, double angle)
//{
    //m_textAngle = angle;
    //m_fontHeight = height;
    //m_fontCacheType = ch;

    //m_fontEngine.load_font(fontName, 0,
    //                       (ch == VectorFontCache) ?
    //                            agg::glyph_ren_outline :
    //                            agg::glyph_ren_agg_gray8);
    //m_fontEngine.hinting(m_textHints);
    //m_fontEngine.height((ch == VectorFontCache) ? height : worldToScreen(height));
//}

////---------------------------------------------------------------------------------------
//double AggDrawer::font_height() const
//{
//   return m_fontHeight;
//}
//
////---------------------------------------------------------------------------------------
//void AggDrawer::text_alignment(TextAlignment alignX, TextAlignment alignY)
//{
//   m_textAlignX = alignX;
//   m_textAlignY = alignY;
//}
//
////---------------------------------------------------------------------------------------
//double AggDrawer::text_width(const char* str)
//{
//    double x = 0;
//    double y = 0;
//    bool first = true;
//    while(*str)
//    {
//        const agg::glyph_cache* glyph = m_fontCacheManager.glyph(*str);
//        if(glyph)
//        {
//            if(!first) m_fontCacheManager.add_kerning(&x, &y);
//            x += glyph->advance_x;
//            y += glyph->advance_y;
//            first = false;
//        }
//        ++str;
//    }
//    return (m_fontCacheType == VectorFontCache) ? x : screenToWorld(x);
//}
//
////---------------------------------------------------------------------------------------
//bool AggDrawer::text_hints() const
//{
//   return m_textHints;
//}
//
////---------------------------------------------------------------------------------------
//void AggDrawer::text_hints(bool hints)
//{
//   m_textHints = hints;
//}

//---------------------------------------------------------------------------------------
void AggDrawer::text(double x, double y, const char* str, bool roundOff,
                     double ddx, double ddy)
{
//    double dx = 0.0;
//    double dy = 0.0;
//
//    switch(m_textAlignX)
//    {
//        case AlignCenter:  dx = -textWidth(str) * 0.5; break;
//        case AlignRight:   dx = -textWidth(str);       break;
//        default: break;
//    }
//
//
//    double asc = font_height();
//    const agg::glyph_cache* glyph = m_fontCacheManager.glyph('H');
//    if(glyph)
//    {
//        asc = glyph->bounds.y2 - glyph->bounds.y1;
//    }
//
//    if(m_fontCacheType == RasterFontCache)
//    {
//        asc = screenToWorld(asc);
//    }
//
//    switch(m_textAlignY)
//    {
//        case AlignCenter:  dy = -asc * 0.5; break;
//        case AlignTop:     dy = -asc;       break;
//        default: break;
//    }
//
//    if(m_fontEngine.flip_y()) dy = -dy;
//
//    agg::trans_affine  mtx;
//
//    double start_x = x + dx;
//    double start_y = y + dy;
//
//    if (roundOff)
//    {
//        start_x = int(start_x);
//        start_y = int(start_y);
//    }
//    start_x += ddx;
//    start_y += ddy;
//
//    mtx *= agg::trans_affine_translation(-x, -y);
//    mtx *= agg::trans_affine_rotation(m_textAngle);
//    mtx *= agg::trans_affine_translation(x, y);
//
//    agg::conv_transform<FontCacheManager::path_adaptor_type> tr(m_fontCacheManager.path_adaptor(), mtx);
//
//    if(m_fontCacheType == RasterFontCache)
//    {
//        worldToScreen(start_x, start_y);
//    }
//
//    int i;
//    for (i = 0; str[i]; i++)
//    {
//        glyph = m_fontCacheManager.glyph(str[i]);
//        if(glyph)
//        {
//            if(i) m_fontCacheManager.add_kerning(&start_x, &start_y);
//            m_fontCacheManager.init_embedded_adaptors(glyph, start_x, start_y);
//
//            if(glyph->data_type == agg::glyph_data_outline)
//            {
//                m_path.remove_all();
//                //m_path.add_path(tr, 0, false);
//				m_path.concat_path(tr,0); // JME
//                drawPath();
//            }
//
//            if(glyph->data_type == agg::glyph_data_gray8)
//            {
//                render(m_fontCacheManager.gray8_adaptor(),
//                       m_fontCacheManager.gray8_scanline());
//            }
//            start_x += glyph->advance_x;
//            start_y += glyph->advance_y;
//        }
//    }
}



}  //namespace lomse
