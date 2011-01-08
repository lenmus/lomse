//-------------------------------------------------------------------------------------
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
//  -------------------------
//  Credits:
//  This file is based on Anti-Grain Geometry version 2.4 examples' code.
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//-------------------------------------------------------------------------------------

#include "lomse_screen_drawer.h"

#include "lomse_renderer.h"

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
void Drawer::set_text_color(Color color)
{
    m_textColor = color;
}



//---------------------------------------------------------------------------------------
// ScreenDrawer implementation
//---------------------------------------------------------------------------------------

ScreenDrawer::ScreenDrawer(LibraryScope& libraryScope)
    : Drawer(libraryScope)
    , m_pRenderer( new Renderer(libraryScope.get_screen_ppi(), m_attr_storage,
                                m_attr_stack, m_path) )
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
void ScreenDrawer::fill(Color color)
{
    PathAttributes& attr = cur_attr();
    attr.fill_color = color;
    attr.fill_flag = true;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::stroke(Color color)
{
    PathAttributes& attr = cur_attr();
    attr.stroke_color = color;
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
void ScreenDrawer::fill_opacity(unsigned op)
{
    cur_attr().fill_color.opacity(op);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::stroke_opacity(unsigned op)
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
void ScreenDrawer::add_path(VertexSource& vs,  unsigned path_id, bool solid_path)
{
    m_path.concat_path(vs, path_id);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::render(FontRasterizer& ras, FontScanline& sl, Color color)
{
    //if(m_blendMode == BlendAlpha)
        m_pRenderer->render(ras, sl, color);
    //else
    //    m_pRenderer->render(*this, m_renBaseComp, m_renSolidComp, ras, sl);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::gsv_text(double x, double y, const char* str)
{
    m_pRenderer->render_gsv_text(x, y, str);
}

//---------------------------------------------------------------------------------------
bool ScreenDrawer::select_font(const std::string& fontName, double height,
                               bool fBold, bool fItalic)
{
    return m_pFonts->select_font(fontName, height, fBold, fItalic);
}

//---------------------------------------------------------------------------------------
bool ScreenDrawer::select_raster_font(const std::string& fontName, double height,
                                      bool fBold, bool fItalic)
{
    return m_pFonts->select_raster_font(fontName, height, fBold, fItalic);
}

//---------------------------------------------------------------------------------------
bool ScreenDrawer::select_vector_font(const std::string& fontName, double height,
                                      bool fBold, bool fItalic)
{
    return m_pFonts->select_vector_font(fontName, height, fBold, fItalic);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::draw_glyph(double x, double y, unsigned int ch)
{
    if (m_path.total_vertices() > 0)
        render(true);

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    m_pCalligrapher->draw_glyph(x, y, ch, m_textColor, m_pRenderer->get_scale());
}

//---------------------------------------------------------------------------------------
int ScreenDrawer::draw_text(double x, double y, const std::string& str)
{
    //returns the number of chars drawn

    if (m_path.total_vertices() > 0)
        render(true);

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    return m_pCalligrapher->draw_text(x, y, str, m_textColor, m_pRenderer->get_scale());
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
void ScreenDrawer::screen_point_to_model(double* x, double* y) const
{
    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.inverse_transform(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::model_point_to_screen(double* x, double* y) const
{
    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(x, y);
}

//---------------------------------------------------------------------------------------
double ScreenDrawer::PixelsToLUnits(Pixels value)
{
    TransAffine& mtx = m_pRenderer->get_transform();
    return double(value) / mtx.scale();
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
void ScreenDrawer::set_transform(TransAffine& transform)
{
    m_pRenderer->set_transform(transform);
    //m_pRenderer->set_scale(transform.scale());
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::render(bool fillColor)
{
    m_pRenderer->render(fillColor);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::set_shift(LUnits x, LUnits y)
{
    m_pRenderer->set_shift(x, y);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::remove_shift()
{
    m_pRenderer->remove_shift();
}


}  //namespace lomse
