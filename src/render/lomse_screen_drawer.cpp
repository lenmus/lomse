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

#include "lomse_screen_drawer.h"

#include "lomse_renderer.h"

//#include "lomse_gm_basic.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// Drawer implementation
//---------------------------------------------------------------------------------------
Drawer::Drawer(LibraryScope& libraryScope)
    : m_libraryScope(libraryScope)
{
    m_pFonts = libraryScope.font_storage();
}



//---------------------------------------------------------------------------------------
// ScreenDrawer implementation
//---------------------------------------------------------------------------------------

ScreenDrawer::ScreenDrawer(LibraryScope& libraryScope)
    : Drawer(libraryScope)
    , m_pRenderer( new Renderer(m_attr_storage, m_attr_stack, m_path) )
    , m_pCalligrapher( new Calligrapher(m_pFonts, m_pRenderer) )
{
}

//---------------------------------------------------------------------------------------
ScreenDrawer::~ScreenDrawer()
{
    delete m_pRenderer;
    delete m_pCalligrapher;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::begin_path()
{
    push_attr();
    unsigned idx = m_path.start_new_path();
    m_attr_storage.add(PathAttributes(cur_attr(), idx));
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::end_path()
{
    if(m_attr_storage.size() == 0)
    {
        throw "end_path : The path was not begun";
    }
    PathAttributes attr = cur_attr();
    unsigned idx = m_attr_storage[m_attr_storage.size() - 1].index;
    attr.index = idx;
    m_attr_storage[m_attr_storage.size() - 1] = attr;
    pop_attr();
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::move_to(double x, double y)
{
    m_path.move_to(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::move_to_rel(double x, double y)
{
    m_path.rel_to_abs(&x, &y);
    m_path.move_to(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::line_to(double x,  double y)
{
    m_path.line_to(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::line_to_rel(double x,  double y)
{
    m_path.rel_to_abs(&x, &y);
    m_path.line_to(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::hline_to(double x)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        m_path.line_to(x, y2);
    }
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::hline_to_rel(double x)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        x += x2;
        m_path.line_to(x, y2);
    }
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::vline_to(double y)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        m_path.line_to(x2, y);
    }
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::vline_to_rel(double y)
{
    double x2 = 0.0;
    double y2 = 0.0;
    if(m_path.total_vertices())
    {
        m_path.vertex(m_path.total_vertices() - 1, &x2, &y2);
        y += y2;
        m_path.line_to(x2, y);
    }
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::cubic_bezier(double x1, double y1, double x,  double y)
{
    m_path.curve3(x1, y1, x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::cubic_bezier_rel(double x1, double y1, double x,  double y)
{
    m_path.rel_to_abs(&x1, &y1);
    m_path.rel_to_abs(&x,  &y);
    m_path.curve3(x1, y1, x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::cubic_bezier(double x, double y)
{
    m_path.curve3(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::cubic_bezier_rel(double x, double y)
{
    m_path.curve3_rel(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::quadratic_bezier(double x1, double y1, double x2, double y2,
                                 double x,  double y)
{
    m_path.curve4(x1, y1, x2, y2, x, y);
}
//---------------------------------------------------------------------------------------
void ScreenDrawer::quadratic_bezier_rel(double x1, double y1, double x2, double y2,
                                     double x,  double y)
{
    m_path.rel_to_abs(&x1, &y1);
    m_path.rel_to_abs(&x2, &y2);
    m_path.rel_to_abs(&x,  &y);
    m_path.curve4(x1, y1, x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::quadratic_bezier(double x2, double y2, double x,  double y)
{
    m_path.curve4(x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::quadratic_bezier_rel(double x2, double y2, double x,  double y)
{
    m_path.curve4_rel(x2, y2, x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::close_subpath()
{
    m_path.end_poly(path_flags_close);
}

//---------------------------------------------------------------------------------------
PathAttributes& ScreenDrawer::cur_attr()
{
    if(m_attr_stack.size() == 0)
    {
        throw "cur_attr : Attribute stack is empty";
    }
    return m_attr_stack[m_attr_stack.size() - 1];
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::push_attr()
{
    m_attr_stack.add(m_attr_stack.size() ?
                        m_attr_stack[m_attr_stack.size() - 1] :
                        PathAttributes());
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::pop_attr()
{
    if(m_attr_stack.size() == 0)
    {
        throw "pop_attr : Attribute stack is empty";
    }
    m_attr_stack.remove_last();
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::fill(const rgba8& f)
{
    PathAttributes& attr = cur_attr();
    attr.fill_color = f;
    attr.fill_flag = true;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::stroke(const rgba8& s)
{
    PathAttributes& attr = cur_attr();
    attr.stroke_color = s;
    attr.stroke_flag = true;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::even_odd(bool flag)
{
    cur_attr().even_odd_flag = flag;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::stroke_width(double w)
{
    cur_attr().stroke_width = w;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::fill_none()
{
    cur_attr().fill_flag = false;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::stroke_none()
{
    cur_attr().stroke_flag = false;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::fill_opacity(double op)
{
    cur_attr().fill_color.opacity(op);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::stroke_opacity(double op)
{
    cur_attr().stroke_color.opacity(op);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::line_join(line_join_e join)
{
    cur_attr().line_join = join;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::line_cap(line_cap_e cap)
{
    cur_attr().line_cap = cap;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::miter_limit(double ml)
{
    cur_attr().miter_limit = ml;
}

//---------------------------------------------------------------------------------------
TransAffine& ScreenDrawer::transform()
{
    return cur_attr().transform;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::add_path(VertexSource& vs,  unsigned path_id, bool solid_path)
{
    begin_path();
    m_path.concat_path(vs, path_id);
}




////---------------------------------------------------------------------------------------
////text. Agg2D font methods
////---------------------------------------------------------------------------------------
//
////---------------------------------------------------------------------------------------
//void ScreenDrawer::set_font(const char* fontFullName, double height,
//                            EFontCacheType type, double angle)
//{
//    m_textAngle = angle;
//    m_fontHeight = height;
//    m_fontCacheType = type;
//
//    if (type == k_vector_font_cache)
//    {
//        m_fontEngine.load_font(fontFullName, 0, agg::glyph_ren_outline);
//        m_fontEngine.height(height);
//    }
//    else
//    {
//        m_fontEngine.load_font(fontFullName, 0, agg::glyph_ren_agg_gray8);
//        m_fontEngine.height( worldToScreen(height) );
//    }
//
//    m_fontEngine.hinting(m_textHints);
//}
//
////---------------------------------------------------------------------------------------
//double ScreenDrawer::get_font_height() const
//{
//   return m_fontHeight;
//}
//
////---------------------------------------------------------------------------------------
//void ScreenDrawer::set_text_alignment(ETextAlignment align_x, ETextAlignment align_y)
//{
//   m_textAlignX = align_x;
//   m_textAlignY = align_y;
//}
//
////---------------------------------------------------------------------------------------
//double ScreenDrawer::get_text_width(const char* str)
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
//    return (m_fontCacheType == k_vector_font_cache) ? x : screenToWorld(x);
//}
//
////---------------------------------------------------------------------------------------
//bool ScreenDrawer::get_text_hints() const
//{
//   return m_textHints;
//}
//
////---------------------------------------------------------------------------------------
//void ScreenDrawer::set_text_hints(bool hints)
//{
//   m_textHints = hints;
//}
//
//
//
////---------------------------------------------------------------------------------------
//void ScreenDrawer::text(double x, double y, const char* str, bool roundOff,
//                        double ddx, double ddy)
//{
//    double dx = 0.0;
//    double dy = 0.0;
//
//    switch(m_textAlignX)
//    {
//        case k_align_center:  dx = -get_text_width(str) * 0.5;   break;
//        case k_align_right:   dx = -get_text_width(str);         break;
//        default:
//            break;
//    }
//
//
//    double asc = get_font_height();
//    const agg::glyph_cache* glyph = m_fontCacheManager.glyph('H');
//    if(glyph)
//    {
//        asc = glyph->bounds.y2 - glyph->bounds.y1;
//    }
//
//    if(m_fontCacheType == k_raster_font_cache)
//    {
//        asc = screenToWorld(asc);
//    }
//
//    switch(m_textAlignY)
//    {
//        case k_align_center:  dy = -asc * 0.5; break;
//        case k_align_top:     dy = -asc;       break;
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
//    if(m_fontCacheType == k_raster_font_cache)
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
//				m_path.concat_path(tr,0);
//                render(true);
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
//}

//---------------------------------------------------------------------------------------
void ScreenDrawer::render(FontRasterizer& ras, FontScanline& sl)
{
    //if(m_blendMode == BlendAlpha)
        m_pRenderer->render(ras, sl);
    //else
    //    m_pRenderer->render(*this, m_renBaseComp, m_renSolidComp, ras, sl);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::gsv_text(double x, double y, const char* str)
{
    m_pRenderer->render_gsv_text(x, y, str);
}

//---------------------------------------------------------------------------------------
//text. FreeType with AGG rederization using my font manager
//---------------------------------------------------------------------------------------

bool ScreenDrawer::set_font(const char* fontFullName, double height, EFontCacheType type)
{
    //Returns true if any error

    return m_pFonts->set_font(fontFullName, height, type);
}


////---------------------------------------------------------------------------------------
//int ScreenDrawer::FtDrawText(const char* sText)
//{
//    //render text (FreeType) at current position, using current settings for font.
//    //Returns the number of chars drawn
//
//    if (!m_pFonts->is_font_valid()) return 0;
//
//    ////convert text to utf-32
//    //size_t nLength = sText.Length();
//    //wxMBConvUTF32 oConv32;
//    //wxCharBuffer s32Text = sText.mb_str(oConv32);
//
//    return FtDrawText(sText, nLength);
//}

////---------------------------------------------------------------------------------------
//int ScreenDrawer::FtDrawChar(unsigned int nChar)
//{
//    //render char (FreeType) at current position, using current settings for font
//    //Returns 0 if error. 1 if ok
//
//    return FtDrawText(&nChar, 1);
//}

//---------------------------------------------------------------------------------------
int ScreenDrawer::draw_text(double x, double y, const std::string& str)
{
    //returns the number of chars drawn

    if (m_path.total_vertices() > 0)
        render(true);

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    //return m_pFonts->draw_text(x, y, str, m_pRenderer->get_scale());
    return m_pCalligrapher->draw_text(x, y, str, m_pRenderer->get_scale());
}

////---------------------------------------------------------------------------------------
//void ScreenDrawer::FtSetTextPosition(lmLUnits uxPos, lmLUnits uyPos)
//{
//    m_vCursorX = WorldToDeviceX(uxPos);
//    m_vCursorY = WorldToDeviceY(uyPos);
//}
//
////---------------------------------------------------------------------------------------
//void ScreenDrawer::FtSetTextPositionPixels(lmPixels vxPos, lmPixels vyPos)
//{
//    m_vCursorX = (double)vxPos;
//    m_vCursorY = (double)vyPos;
//}

////---------------------------------------------------------------------------------------
//VRect ScreenDrawer::FtGetGlyphBoundsInPixels(unsigned int nGlyph)
//{
//    //returns glyph bounding box. In pixels
//
//    VRect boxRect;
//    if (!m_pFonts->is_font_valid()) return boxRect;
//
//    const agg::glyph_cache* glyph = m_pFonts->get_glyph_cache(nGlyph);
//    if(glyph)
//    {
//        //m_pFonts->init_adaptors(glyph, x, y);
//        agg::rect_i bbox = glyph->bounds;        //rect_i is a rectangle with integer values
//        boxRect.x = bbox.x1;
//        boxRect.y = bbox.y1;
//        boxRect.width = bbox.x2-bbox.x1;
//        boxRect.height = bbox.y2-bbox.y1;
//    }
//    return boxRect;
//}

////---------------------------------------------------------------------------------------
//URect ScreenDrawer::FtGetGlyphBounds(unsigned int nGlyph)
//{
//    //returns glyph bounding box. In lmLUnits
//
//    VRect vBox = FtGetGlyphBoundsInPixels(nGlyph);
//    return lmURect(DeviceToLogicalX(vBox.x), DeviceToLogicalY(vBox.y),
//                   DeviceToLogicalX(vBox.width), DeviceToLogicalY(vBox.height) );
//}

////---------------------------------------------------------------------------------------
//void ScreenDrawer::FtGetTextExtent(const std::string& sText,
//                                         lmLUnits* pWidth, lmLUnits* pHeight,
//                                         lmLUnits* pDescender, lmLUnits* pAscender)
//{
//    //Gets the dimensions of the string using the currently selected font.
//    //Parameters:
//    //  sText is the text string to measure,
//    //  descent is the dimension from the baseline of the font to the bottom of
//    //          the descender,
//    //  externalLeading is any extra vertical space added to the font by the
//    //          font designer (usually is zero).
//    //
//    //The text extent is returned in w and h pointers.
//    //
//    //The currently selected font is used to compute dimensions.
//    //Note that this function only works with single-line strings.
//
//    //convert text to utf-32
//    size_t nLength = sText.Length();
//    wxMBConvUTF32 oConv32;
//    wxCharBuffer s32Text = sText.mb_str(oConv32);
//
//    double x  = 0.0;
//    double y  = m_pFonts->get_font_heught();
//    const unsigned int* p = (unsigned int*)s32Text.data();
//
//    while(*p && nLength--)
//    {
//        const agg::glyph_cache* glyph = m_pFonts->get_glyph_cache(*p);
//        if(glyph)
//        {
//            if(m_fKerning)
//                m_pFonts->add_kerning(&x, &y);
//
//            x += glyph->advance_x;
//        }
//        ++p;
//    }
//
//    //return results
//    *pWidth = DeviceToWorldX(x);
//    *pHeight = DeviceToWorldY(y);
//
//    if (pAscender)
//        *pAscender = DeviceToWorldY(m_pFonts->get_ascender());
//
//    if (pDescender)
//        *pDescender = DeviceToWorldY(m_pFonts->get_descender());
//}

//---------------------------------------------------------------------------------------
void ScreenDrawer::LUnitsToPixels(double& x, double& y) const
{
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::PixelsToLUnits(double& x, double& y) const
{
}

//---------------------------------------------------------------------------------------
double ScreenDrawer::LUnitsToPixels(double scalar) const
{
    return scalar;
}

//---------------------------------------------------------------------------------------

double ScreenDrawer::PixelsToLUnits(double scalar) const
{
    return scalar;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::reset(RenderingBuffer& buf)
{
    m_pRenderer->initialize(buf);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::set_viewport(Pixels x, Pixels y)
{
    m_pRenderer->set_viewport(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::set_scale(double scale)
{
    m_pRenderer->set_scale(scale);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::render(bool fillColor)
{
    m_pRenderer->render(fillColor);
}


}  //namespace lomse
