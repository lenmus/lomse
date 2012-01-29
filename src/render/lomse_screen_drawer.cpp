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
//  -------------------------
//  Credits:
//  This file is based on Anti-Grain Geometry version 2.4 examples' code and on
//  ScreenDrawer version 1.0 code.
//
//  Anti-Grain Geometry (AGG) is copyright (C) 2002-2005 Maxim Shemanarev
//  (http://www.antigrain.com). AGG 2.4 is distributed under BSD license.
//
//  ScreenDrawer code is licensed as follows:
//    "Permission to copy, use, modify, sell and distribute this software
//    is granted provided this copyright notice appears in all copies.
//    This software is provided "as is" without express or implied
//    warranty, and with no claim as to its suitability for any purpose."
//
//---------------------------------------------------------------------------------------

#include "lomse_screen_drawer.h"

#include "lomse_renderer.h"
#include "agg_ellipse.h"
#include "agg_rounded_rect.h"

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
    , m_pRenderer( RendererFactory::create_renderer(libraryScope, m_attr_storage, m_path) )
    , m_pCalligrapher( LOMSE_NEW Calligrapher(m_pFonts, m_pRenderer) )
    , m_numPaths(0)
{
}

//---------------------------------------------------------------------------------------
ScreenDrawer::~ScreenDrawer()
{
    delete m_pRenderer;
    delete m_pCalligrapher;
    delete_paths();
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::delete_paths()
{
    //AttrStorage objects are pod_bvector<PathAttributes>
    //and pod_bvector doesn't invoke destructors, just dealloc memory. Therefore, we need to
    //free memory allocated for GradientAttributes, to avoid memory leaks

    for (int i = 0; i < m_numPaths; ++i)
    {
        PathAttributes& attr = m_attr_storage[i];
        delete attr.fill_gradient;
        attr.fill_gradient = NULL;
    }

    m_attr_storage.clear();
    m_numPaths = 0;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::begin_path()
{
    unsigned idx = m_path.start_new_path();
    m_attr_storage.add( m_numPaths==0 ? PathAttributes(idx) : PathAttributes(cur_attr(), idx) );
    m_numPaths++;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::end_path()
{
    if(m_attr_storage.size() == 0)
        throw std::runtime_error("[ScreenDrawer::end_path] The path was not begun");
}

//---------------------------------------------------------------------------------------
PathAttributes& ScreenDrawer::cur_attr()
{
    return m_attr_storage[m_numPaths - 1];
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
void ScreenDrawer::fill(Color color)
{
    PathAttributes& attr = cur_attr();
    attr.fill_color = color;
    attr.fill_mode = k_fill_solid;
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
    cur_attr().fill_mode = k_fill_none;
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
    delete_paths();
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
    render_existing_paths();

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    m_pCalligrapher->draw_glyph(x, y, ch, m_textColor, m_pRenderer->get_scale());
}

//---------------------------------------------------------------------------------------
int ScreenDrawer::draw_text(double x, double y, const std::string& str)
{
    //returns the number of chars drawn

    render_existing_paths();

    TransAffine& mtx = m_pRenderer->get_transform();
    mtx.transform(&x, &y);
    return m_pCalligrapher->draw_text(x, y, str, m_textColor, m_pRenderer->get_scale());
}

////---------------------------------------------------------------------------------------
//void ScreenDrawer::FtSetTextPosition(LUnits uxPos, LUnits uyPos)
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
//        agg::AggRectInt bbox = glyph->bounds;        //AggRectInt is a rectangle with integer values
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
//    //returns glyph bounding box. In LUnits
//
//    VRect vBox = FtGetGlyphBoundsInPixels(nGlyph);
//    return lmURect(DeviceToLogicalX(vBox.x), DeviceToLogicalY(vBox.y),
//                   DeviceToLogicalX(vBox.width), DeviceToLogicalY(vBox.height) );
//}

////---------------------------------------------------------------------------------------
//void ScreenDrawer::FtGetTextExtent(const std::string& sText,
//                                         LUnits* pWidth, LUnits* pHeight,
//                                         LUnits* pDescender, LUnits* pAscender)
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
double ScreenDrawer::Pixels_to_LUnits(Pixels value)
{
    TransAffine& mtx = m_pRenderer->get_transform();
    return double(value) / mtx.scale();
}

//---------------------------------------------------------------------------------------
Pixels ScreenDrawer::LUnits_to_Pixels(double value)
{
    TransAffine& mtx = m_pRenderer->get_transform();
    return Pixels( value * mtx.scale() );
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::reset(RenderingBuffer& buf, Color bgcolor)
{
    m_pRenderer->initialize(buf, bgcolor);
    delete_paths();
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
void ScreenDrawer::render()
{
    m_pRenderer->render();
    delete_paths();
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

//---------------------------------------------------------------------------------------
void ScreenDrawer::circle(LUnits xCenter, LUnits yCenter, LUnits radius)
{
    double x = double(xCenter);
    double y = double(yCenter);
    double r = double(radius);

    agg::ellipse e1;
    e1.init(x, y, r, r, 0);
    m_path.concat_path<agg::ellipse>(e1);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
                        LUnits width, ELineEdge nEdge)
{
    double alpha = atan((y2 - y1) / (x2 - x1));

    switch(nEdge)
    {
        case k_edge_normal:
            // edge line is perpendicular to line
            {
            LUnits uIncrX = (LUnits)( (width * sin(alpha)) / 2.0 );
            LUnits uIncrY = (LUnits)( (width * cos(alpha)) / 2.0 );
            UPoint uPoints[] = {
                UPoint(x1+uIncrX, y1-uIncrY),
                UPoint(x1-uIncrX, y1+uIncrY),
                UPoint(x2-uIncrX, y2+uIncrY),
                UPoint(x2+uIncrX, y2-uIncrY)
            };
            polygon(4, uPoints);
            break;
            }

        case k_edge_vertical:
            // edge is always a vertical line
            {
            LUnits uIncrY = (LUnits)( (width / cos(alpha)) / 2.0 );
            UPoint uPoints[] = {
                UPoint(x1, y1-uIncrY),
                UPoint(x1, y1+uIncrY),
                UPoint(x2, y2+uIncrY),
                UPoint(x2, y2-uIncrY)
            };
            polygon(4, uPoints);
            break;
            }

        case k_edge_horizontal:
            // edge is always a horizontal line
            {
            LUnits uIncrX = (LUnits)( (width / sin(alpha)) / 2.0 );
            UPoint uPoints[] = {
                UPoint(x1+uIncrX, y1),
                UPoint(x1-uIncrX, y1),
                UPoint(x2-uIncrX, y2),
                UPoint(x2+uIncrX, y2)
            };
            polygon(4, uPoints);
            break;
            }
    }
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::polygon(int n, UPoint points[])
{
    move_to(points[0].x, points[0].y);
    int i;
    for (i=1; i < n; i++)
    {
        line_to(points[i].x, points[i].y);
    }
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::rect(UPoint pos, USize size, LUnits radius)
{
    double x1 = double(pos.x);
    double y1 = double(pos.y);
    double x2 = x1 + double(size.width);
    double y2 = y1 + double(size.height);
    double r = double(radius);

    agg::rounded_rect rr(x1, y1, x2, y2, r);
    m_path.concat_path<agg::rounded_rect>(rr);
}

////------------------------------------------------------------------------
//void ScreenDrawer::blendImage(Image& img,
//                       int imgX1, int imgY1, int imgX2, int imgY2,
//                       double dstX, double dstY, unsigned alpha)
//{
//    model_point_to_screen(dstX, dstY);
//    PixFormat pixF(img.renBuf);
//    // JME
//    //agg::rect r(imgX1, imgY1, imgX2, imgY2);
//    AggRectInt r(imgX1, imgY1, imgX2, imgY2);
//    if(m_blendMode == BlendAlpha)
//    {
//        m_renBasePre.blend_from(pixF, &r, int(dstX)-imgX1, int(dstY)-imgY1, alpha);
//    }
//    else
//    {
//        m_renBaseCompPre.blend_from(pixF, &r, int(dstX)-imgX1, int(dstY)-imgY1, alpha);
//    }
//}
//
//
////------------------------------------------------------------------------
//void ScreenDrawer::blendImage(Image& img, double dstX, double dstY, unsigned alpha)
//{
//    model_point_to_screen(dstX, dstY);
//    PixFormat pixF(img.renBuf);
//    m_renBasePre.blend_from(pixF, 0, int(dstX), int(dstY), alpha);
//    if(m_blendMode == BlendAlpha)
//    {
//        m_renBasePre.blend_from(pixF, 0, int(dstX), int(dstY), alpha);
//    }
//    else
//    {
//        m_renBaseCompPre.blend_from(pixF, 0, int(dstX), int(dstY), alpha);
//    }
//}
//
//
////------------------------------------------------------------------------
//void ScreenDrawer::copyImage(RenderingBuffer& img,
//                      VPoint srcOrg, VSize srcSize
                        //int imgX1, int imgY1, int imgX2, int imgY2,
//                      UPoint dest)
//{
    //double x = double(dest.x);
    //double y = double(dest.y);
    //model_point_to_screen(&x, &y);
//    AggRectInt r(imgX1, imgY1, imgX2, imgY2);
//    m_renBase.copy_from(img.renBuf, &r, int(dstX)-imgX1, int(dstY)-imgY1);
//}

//------------------------------------------------------------------------
void ScreenDrawer::render_existing_paths()
{
    if (m_path.total_vertices() > 0)
        render();
}

//------------------------------------------------------------------------
void ScreenDrawer::copy_bitmap(RenderingBuffer& bmap, UPoint dest)
{
    render_existing_paths();

    double x = double(dest.x);
    double y = double(dest.y);
    model_point_to_screen(&x, &y);
    m_pRenderer->copy_from(bmap, NULL, int(x), int(y));
}

//------------------------------------------------------------------------
void ScreenDrawer::copy_bitmap(RenderingBuffer& bmap,
                               Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                               UPoint dest)
{
    render_existing_paths();

    double x = double(dest.x);
    double y = double(dest.y);
    model_point_to_screen(&x, &y);

    AggRectInt r(srcX1, srcY1, srcX2, srcY2);
    m_pRenderer->copy_from(bmap, &r, int(x)-srcX1, int(y)-srcY1);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::draw_bitmap(RenderingBuffer& bmap, bool hasAlpha,
                               Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                               LUnits dstX1, LUnits dstY1, LUnits dstX2, LUnits dstY2,
                               EResamplingQuality resamplingMode,
                               double alpha)
{
    render_existing_paths();

    double x1 = double(dstX1);
    double y1 = double(dstY1);
    double x2 = double(dstX2);
    double y2 = double(dstY2);
    model_point_to_screen(&x1, &y1);
    model_point_to_screen(&x2, &y2);

    m_pRenderer->render_bitmap(bmap, hasAlpha, double(srcX1), double(srcY1),
                               double(srcX2), double(srcY2), x1, y1, x2, y2,
                               resamplingMode, alpha);
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::fill_linear_gradient(LUnits x1, LUnits y1, LUnits x2, LUnits y2)
{
    PathAttributes& attr = cur_attr();
    if (!attr.fill_gradient)
        attr.fill_gradient = LOMSE_NEW GradientAttributes();

    double angle = atan2(double(y2-y1), double(x2-x1));
    attr.fill_gradient->transform.reset();
    attr.fill_gradient->transform *= agg::trans_affine_rotation(angle);
    attr.fill_gradient->transform *= agg::trans_affine_translation(x1, y1);

    attr.fill_gradient->d1 = 0.0;
    attr.fill_gradient->d2 =
        sqrt(double(x2-x1) * double(x2-x1) + double(y2-y1) * double(y2-y1));
    attr.fill_mode = k_fill_gradient_linear;
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::gradient_color(Color c1, Color c2, double start, double stop)
{
    PathAttributes& attr = cur_attr();
    if (!attr.fill_gradient)
        attr.fill_gradient = LOMSE_NEW GradientAttributes();

    int iStart = int(255.0 * start);
    int iStop   = int(255.0 * stop);
    if (iStop <= iStart)
        iStop = iStart + 1;
    double k = 1.0 / double(iStop - iStart);

    GradientColors& colors = attr.fill_gradient->colors;
    for (int i = iStart; i < iStop; i++)
    {
        colors[i] = c1.gradient(c2, double(i - iStart) * k);
    }
}

//---------------------------------------------------------------------------------------
void ScreenDrawer::gradient_color(Color c1, double start, double stop)
{
    PathAttributes& attr = cur_attr();
    if (!attr.fill_gradient)
        attr.fill_gradient = LOMSE_NEW GradientAttributes();

    int iStart = int(255.0 * start);
    int iStop   = int(255.0 * stop);
    if (iStop <= iStart)
        iStop = iStart + 1;

    GradientColors& colors = attr.fill_gradient->colors;
    for (int i = iStart; i < iStop; i++)
    {
        colors[i] = c1;
    }
}




}  //namespace lomse
