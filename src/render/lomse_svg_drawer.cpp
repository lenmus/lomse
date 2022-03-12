//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_svg_drawer.h"

#include "lomse_logger.h"
#include "lomse_font_storage.h"
#include "agg_basics.h"

//std
#include <locale>
#include <codecvt>
using namespace std;


namespace lomse
{

//=======================================================================================
// SvgDrawer implementation
//=======================================================================================
SvgDrawer::SvgDrawer(LibraryScope& libraryScope, ostream& svgstream,
                     const SvgOptions& opt)
    : Drawer(libraryScope)
    , m_svg(svgstream)
    , m_options(opt)
{
}

//---------------------------------------------------------------------------------------
SvgDrawer::~SvgDrawer()
{
}

//---------------------------------------------------------------------------------------
void SvgDrawer::reset(Color UNUSED(bgcolor))
{
    m_path.str("");
    m_path.clear();
    m_attribs.str("");
    m_attribs.clear();
}

//---------------------------------------------------------------------------------------
void SvgDrawer::add_newline()
{
    if (m_options.add_newlines)
        m_svg << endl;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::begin_path()
{
    if (m_path.tellp() != std::streampos(0))
    {
        LOMSE_LOG_ERROR("Path already open: [" + m_path.str() + "]");
        m_path.str("");
        m_path.clear();
    }
    m_path << "<path d='";
}

//---------------------------------------------------------------------------------------
void SvgDrawer::end_path()
{
    if (m_path.tellp() == std::streampos(0))
    {
        LOMSE_LOG_ERROR("The path was not begun!");
    }
    else
    {
        if (m_path.str() != "<path d='")
        {
            m_svg << m_path.str() << "'" << m_attribs.str() << "/>";
            add_newline();
        }
    }

    m_path.str("");
    m_path.clear();
    m_attribs.str("");
    m_attribs.clear();
}

//---------------------------------------------------------------------------------------
void SvgDrawer::move_to(double x, double y)
{
    m_path << " M " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::move_to_rel(double x, double y)
{
    m_path << " m " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::line_to(double x,  double y)
{
    m_path << " L " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::line_to_rel(double x,  double y)
{
    m_path << " l " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::hline_to(double x)
{
    m_path << " H " << x;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::hline_to_rel(double x)
{
    m_path << " h " << x;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::vline_to(double y)
{
    m_path << " V " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::vline_to_rel(double y)
{
    m_path << " v " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::quadratic_bezier(double x1, double y1, double x,  double y)
{
    m_path << " Q " << x1 << " " << y1 << " " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::quadratic_bezier_rel(double x1, double y1, double x,  double y)
{
    m_path << " q " << x1 << " " << y1 << " " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::quadratic_bezier(double x, double y)
{
    m_path << " T " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::quadratic_bezier_rel(double x, double y)
{
    m_path << " t " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::cubic_bezier(double x1, double y1, double x2, double y2,
                                 double x,  double y)
{
    m_path << " C " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << x << " " << y;
}
//---------------------------------------------------------------------------------------
void SvgDrawer::cubic_bezier_rel(double x1, double y1, double x2, double y2,
                                     double x,  double y)
{
    m_path << " c " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::cubic_bezier(double x2, double y2, double x,  double y)
{
    m_path << " S " << x2 << " " << y2 << " " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::cubic_bezier_rel(double x2, double y2, double x,  double y)
{
    m_path << " s " << x2 << " " << y2 << " " << x << " " << y;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::close_path()
{
    m_attribs << " Z";
}

//---------------------------------------------------------------------------------------
void SvgDrawer::fill(Color color)
{
    m_attribs << " fill='" << to_svg(color) << "'";
}

//---------------------------------------------------------------------------------------
void SvgDrawer::stroke(Color color)
{
    m_attribs << " stroke='" << to_svg(color) << "'";
}

//---------------------------------------------------------------------------------------
void SvgDrawer::stroke_width(double w)
{
    m_attribs << " stroke-width='" << w << "'";
}

//---------------------------------------------------------------------------------------
void SvgDrawer::fill_none()
{
    m_attribs << " fill='none'" ;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::stroke_none()
{
    m_attribs << " stroke='none'";
}

//---------------------------------------------------------------------------------------
void SvgDrawer::add_path(VertexSource& vs,  unsigned path_id,
                            bool UNUSED(solid_path))
{
    double x, y;
    unsigned cmd;
    unsigned cmdPrev;
    vs.rewind(path_id);
    while(!is_stop(cmd = vs.vertex(&x, &y)))
    {
        switch (cmd)
        {
            case agg::path_cmd_move_to:
                m_path << " M " << x << " " << y;
                break;

            case agg::path_cmd_line_to:
                m_path << " L " << x << " " << y;
                break;

            case agg::path_cmd_curve3:
                m_path << " S " << x << " " << y;
                cmd = vs.vertex(&x, &y);
                if (cmd != agg::path_cmd_curve3)
                {
                    LOMSE_LOG_ERROR("curve3 has only one point");
                    return;
                }
                m_path << " " << x << " " << y;
                break;

            case agg::path_cmd_curve4:
            {
                m_path << " C " << x << " " << y;
                for (int i=0; i < 4; ++i)
                {
                    cmd = vs.vertex(&x, &y);
                    if (cmd != agg::path_cmd_curve4)
                    {
                        LOMSE_LOG_ERROR("curve4 has less than five points");
                        return;
                    }
                    m_path << " " << x << " " << y;
                }
                break;
            }

            default:
                if (cmd & agg::path_cmd_end_poly)
                {
                    if (cmdPrev == agg::path_cmd_curve4)    // || cmdPrev == agg::path_cmd_curve3)
                        m_path << " " << x << " " << y << " Z";
                    else
                        m_path << " Z";
                }
        }
        cmdPrev = cmd;
    }
}

//---------------------------------------------------------------------------------------
bool SvgDrawer::select_font(const std::string& language,
                            const std::string& fontFile,
                            const std::string& fontName, double height,
                            bool fBold, bool fItalic)
{
    m_fontSize = height;
    m_fontStyle = fItalic ? "italic" : "normal";
    m_fontWeight = fBold ? "bold" : "normal";
    m_fontFamily = fontName;

    //AWARE: this is needed so that shapes can do measurements at drawing time
    m_pFonts->select_font(language, fontFile, fontName, height, fBold, fItalic);

    return true;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::draw_glyph(double x, double y, unsigned int ch)
{
    const double factor = 35.2778;   //to convert font-size (pt) to LUnits  (25.4*100/72)
    m_svg << "<text x='" << x << "' y='" << y << "' fill='" << to_svg(m_textColor)
         << "' font-family='" << m_fontFamily << "' font-size='" << m_fontSize * factor;

    if (m_fontWeight != "normal")
        m_svg << "' font-weight='" << m_fontWeight;

    if (m_fontStyle != "normal")
        m_svg << "' font-style='" << m_fontStyle;

    m_svg << "'>&#" << ch << ";</text>";
    add_newline();
}

//---------------------------------------------------------------------------------------
void SvgDrawer::draw_glyph_rotated(double x, double y, unsigned int ch, double rotation)
{
    const double factor = 35.2778;   //to convert font-size (pt) to LUnits  (25.4*100/72)
    const double degrees = 180.0 / 3.141592654;   //to convert radians to degrees
    m_svg << "<text x='" << x << "' y='" << y << "' fill='" << to_svg(m_textColor)
         << "' transform='rotate(" << rotation * degrees << "," << x << "," << y << ")"
         << "' font-family='" << m_fontFamily << "' font-size='" << m_fontSize * factor;

    if (m_fontWeight != "normal")
        m_svg << "' font-weight='" << m_fontWeight;

    if (m_fontStyle != "normal")
        m_svg << "' font-style='" << m_fontStyle;

    m_svg << "'>&#" << ch << ";</text>";
    add_newline();
 }

//---------------------------------------------------------------------------------------
int SvgDrawer::draw_text(double x, double y, const std::string& str)
{
    //returns the number of chars drawn

    const double factor = 35.2778;   //to convert font-size (pt) to LUnits  (25.4*100/72)
    m_svg << "<text x='" << x << "' y='" << y << "' fill='" << to_svg(m_textColor)
         << "' font-family='" << m_fontFamily << "' font-size='" << m_fontSize * factor;

    if (m_fontWeight != "normal")
        m_svg << "' font-weight='" << m_fontWeight;

    if (m_fontStyle != "normal")
        m_svg << "' font-style='" << m_fontStyle;

    m_svg << "'>" << str << "</text>";
    add_newline();

    return str.size();
}

//---------------------------------------------------------------------------------------
int SvgDrawer::draw_text(double x, double y, const wstring& str)
{
    //returns the number of chars drawn

    //setup converter wstring -> string
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    string text = converter.to_bytes(str);

    return draw_text(x, y, text);
}

//---------------------------------------------------------------------------------------
void SvgDrawer::device_point_to_model(double* x, double* y) const
{
    //e.g. Pixels to LUnits

    m_transform.inverse_transform(x, y);
}

//---------------------------------------------------------------------------------------
void SvgDrawer::model_point_to_device(double* x, double* y) const
{
    //e.g. LUnits to Pixels

    m_transform.transform(x, y);
}

//---------------------------------------------------------------------------------------
LUnits SvgDrawer::device_units_to_model(double value) const
{
    //e.g. Pixels to LUnits

    return value / m_transform.scale();
}

//---------------------------------------------------------------------------------------
double SvgDrawer::model_to_device_units(LUnits value) const
{
    //e.g. LUnits to Pixels

    return value * m_transform.scale();
}

//---------------------------------------------------------------------------------------
void SvgDrawer::new_viewport_origin(double x, double y)
{
    //coordinates in device units (e.g. Pixel)
    Drawer::new_viewport_origin(x, y);
}

//---------------------------------------------------------------------------------------
void SvgDrawer::new_viewport_size(double x, double y)
{
    //in device units (e.g. Pixel)
    Drawer::new_viewport_size(x, y);
}

//---------------------------------------------------------------------------------------
void SvgDrawer::set_affine_transformation(TransAffine& transform)
{
    m_transform = transform;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::render()
{
}

//---------------------------------------------------------------------------------------
void SvgDrawer::set_shift(LUnits UNUSED(x), LUnits UNUSED(y))
{
    //TODO
    //This method needs re-think. It is mainly used for visual overlays, to shift the
    //position of the shape being drawn. It added this shit to the renderer affine
    //transformation that was applied when the render() method is invoked.
    //But in SvgDrawer the paths are rendered as they are created and, thus, this shift
    //cannot be applied.

    //m_pRenderer->set_shift(x, y);
}

//---------------------------------------------------------------------------------------
void SvgDrawer::remove_shift()
{
    //TODO: see comment in set_shift()

    //m_pRenderer->remove_shift();
}

//---------------------------------------------------------------------------------------
void SvgDrawer::circle(LUnits xCenter, LUnits yCenter, LUnits radius)
{
    m_svg << "<circle cx='" << xCenter << "' cy='" << yCenter
          << "' r='" << radius << "'" << m_attribs.str() << "/>";
    add_newline();
}

//---------------------------------------------------------------------------------------
void SvgDrawer::line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
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
void SvgDrawer::polygon(int n, UPoint points[])
{
    move_to(points[0].x, points[0].y);
    int i;
    for (i=1; i < n; i++)
    {
        line_to(points[i].x, points[i].y);
    }
}

//---------------------------------------------------------------------------------------
void SvgDrawer::rect(UPoint pos, USize size, LUnits radius)
{
    m_svg << "<rect x='" << pos.x << "' y='" << pos.y
          << "' width='" << size.width << "' height='" << size.height;
    if (radius > 0.0)
        m_svg << "' rx='" << radius << "' ry='" << radius;

    m_svg << "'/>";
    add_newline();
 }

//---------------------------------------------------------------------------------------
void SvgDrawer::draw_bitmap(RenderingBuffer& UNUSED(bmap), bool UNUSED(hasAlpha),
                               Pixels UNUSED(srcX1), Pixels UNUSED(srcY1),
                               Pixels UNUSED(srcX2), Pixels UNUSED(srcY2),
                               LUnits UNUSED(dstX1), LUnits UNUSED(dstY1),
                               LUnits UNUSED(dstX2), LUnits UNUSED(dstY2),
                               EResamplingQuality UNUSED(resamplingMode),
                               double UNUSED(alpha))
{
//    double x1 = double(dstX1);
//    double y1 = double(dstY1);
//    double x2 = double(dstX2);
//    double y2 = double(dstY2);
//    model_point_to_device(&x1, &y1);
//    model_point_to_device(&x2, &y2);
//
//    m_pRenderer->render_bitmap(bmap, hasAlpha, double(srcX1), double(srcY1),
//                               double(srcX2), double(srcY2), x1, y1, x2, y2,
//                               resamplingMode, alpha);
}

//---------------------------------------------------------------------------------------
void SvgDrawer::fill_linear_gradient(LUnits UNUSED(x1), LUnits UNUSED(y1),
                                     LUnits UNUSED(x2), LUnits UNUSED(y2))
{
    //TODO
//    PathAttributes& attr = cur_attr();
//    if (!attr.fill_gradient)
//        attr.fill_gradient = LOMSE_NEW GradientAttributes();
//
//    double angle = atan2(double(y2-y1), double(x2-x1));
//    attr.fill_gradient->transform.reset();
//    attr.fill_gradient->transform *= agg::trans_affine_rotation(angle);
//    attr.fill_gradient->transform *= agg::trans_affine_translation(x1, y1);
//
//    attr.fill_gradient->d1 = 0.0;
//    attr.fill_gradient->d2 =
//        sqrt(double(x2-x1) * double(x2-x1) + double(y2-y1) * double(y2-y1));
//    attr.fill_mode = k_fill_gradient_linear;
}

//---------------------------------------------------------------------------------------
void SvgDrawer::gradient_color(Color UNUSED(c1), Color UNUSED(c2),
                               double UNUSED(start), double UNUSED(stop))
{
    //TODO
//    PathAttributes& attr = cur_attr();
//    if (!attr.fill_gradient)
//        attr.fill_gradient = LOMSE_NEW GradientAttributes();
//
//    int iStart = int(255.0 * start);
//    int iStop   = int(255.0 * stop);
//    if (iStop <= iStart)
//        iStop = iStart + 1;
//    double k = 1.0 / double(iStop - iStart);
//
//    GradientColors& colors = attr.fill_gradient->colors;
//    for (int i = iStart; i < iStop; i++)
//    {
//        colors[i] = c1.gradient(c2, double(i - iStart) * k);
//    }
}

//---------------------------------------------------------------------------------------
void SvgDrawer::gradient_color(Color UNUSED(c1),
                               double UNUSED(start), double UNUSED(stop))
{
    //TODO
//    PathAttributes& attr = cur_attr();
//    if (!attr.fill_gradient)
//        attr.fill_gradient = LOMSE_NEW GradientAttributes();
//
//    int iStart = int(255.0 * start);
//    int iStop   = int(255.0 * stop);
//    if (iStop <= iStart)
//        iStop = iStart + 1;
//
//    GradientColors& colors = attr.fill_gradient->colors;
//    for (int i = iStart; i < iStop; i++)
//    {
//        colors[i] = c1;
//    }
}

//---------------------------------------------------------------------------------------
void SvgDrawer::line_with_markers(UPoint start, UPoint end, LUnits width,
                                     ELineCap startCap, ELineCap endCap)
{
    LineVertexSource line(double(start.x), double(start.y),
                          double(end.x), double(end.y) );

    //Converters are VertexSource objects
    LineCapsConverter<LineVertexSource> converter(line, double(width), startCap, endCap);

    add_path(converter, 0);
}

//---------------------------------------------------------------------------------------
bool SvgDrawer::is_ready() const
{
    return true;
}

//---------------------------------------------------------------------------------------
string SvgDrawer::to_svg(Color color)
{
    if (is_equal(color, Color(0,0,0)) )
        return "#000";

    stringstream ss;
    ss << "#";
    int r = color.r;
    int g = color.g;
    int b = color.b;
    int a = color.a;
    ss << std::hex << setfill('0') << setw(2) << r;
    ss << std::hex << setfill('0') << setw(2) << g;
    ss << std::hex << setfill('0') << setw(2) << b;
    ss << std::hex << setfill('0') << setw(2) << a;
    return ss.str();
}


}  //namespace lomse
