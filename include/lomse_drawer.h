//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2022. All rights reserved.
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

#ifndef __LOMSE_DRAWER_H__        //to avoid nested includes
#define __LOMSE_DRAWER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_vertex_source.h"
#include "agg_color_rgba.h"     //rgba & rgba8
#include "agg_math_stroke.h"    //line_cap_e & line_join_e
#include "agg_trans_affine.h"   //trans_affine
#include "agg_ellipse.h"
#include "agg_renderer_markers.h"       //for rendering markers
#include "agg_conv_curve.h"
#include "agg_conv_stroke.h"
#include "agg_conv_marker.h"
#include "agg_conv_concat.h"
#include "agg_vcgen_markers_term.h"
#include "lomse_shape_base.h"   //enums ELineEdge, ...
#include "lomse_agg_types.h"    //RenderingBuffer
#include "lomse_gm_basic.h"     //for GmoObj::k_max
#include "lomse_path_attributes.h"

#include <string>
#include <bitset>
using namespace std;

using namespace agg;

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class FontStorage;

//---------------------------------------------------------------------------------------
//rendering options
/** @ingroup enumerations

	This enum describes valid values for method GraphicView::set_rendering_option(),
	mainly used for debug purposes.

	@#include <lomse_drawer.h>
*/
enum ERenderOptions
{
    //for debugging
    k_option_draw_box_doc_page_content=0,   ///< Draw borders around all DocPage boxes
    k_option_draw_box_container,            ///< Draw borders around all DocPageContent boxes
    k_option_draw_box_system,               ///< Draw borders around all System boxes
    k_option_draw_box_slice,                ///< Draw borders around all SystemSlice boxes
    k_option_draw_box_slice_instr,          ///< Draw borders around all InstrSlice boxes
    k_option_draw_box_inline_flag,          ///< Draw borders around all Inline boxes

    //for user application needs
    k_option_display_voices_in_colours,     ///< Display each music voice in a different color
};

///@cond INTERNALS

//---------------------------------------------------------------------------------------
// RenderOptions: struct holding the rendering options for a document
//---------------------------------------------------------------------------------------
struct RenderOptions
{
    //for debug
    bitset<GmoObj::k_max> boxes;    //draw a box around boxes of selected types
    bool draw_anchor_objects;   //draw anchor objs. (e.g., invisible shapes)
    bool draw_anchor_lines;     //draw anchor lines. (spacing algorithm)
    bool draw_shape_bounds;     //draw bounds around selected shapes
    bool draw_slur_points;      //draw control and reference points in slurs and ties
    bool draw_vertical_profile;     //draw vertical profile
    bool draw_chords_coloured;  //dbg: draw flag, link and start chord notes in colors
    //bool g_fShowMargins;    //draw margins in scores, so user can change them
    //bool g_fFreeMove;		//the shapes can be dragged without restrictions


    //-------------------------------------------------------------------------
    // scaling factor
    // To be transferred to the View
    double  scale;

    //colors
    Color background_color;
    Color highlighted_color;
    Color dragged_color;
    Color selected_color;
    Color focussed_box_color;
    Color unfocussed_box_color;
    Color not_highlighted_voice_color;
    Color voiceColor[9];

    //document page appearance
    bool page_border_flag;
    bool cast_shadow_flag;

    //other options
    bool draw_focus_lines_on_boxes_flag;
    bool draw_shapes_highlighted;
    bool draw_shapes_dragged;
    bool draw_shapes_selected;
    bool draw_voices_coloured;

    bool read_only_mode;
    int highlighted_voice;          //0 for none


    RenderOptions()
        : draw_anchor_objects(false)
        , draw_anchor_lines(false)
        , draw_shape_bounds(false)
        , draw_slur_points(false)
        , draw_vertical_profile(false)
        , draw_chords_coloured(false)
        , scale(1.0)
        , background_color(127, 127, 127)       //grey
        , highlighted_color(255,0,0)            //red
        , dragged_color(0,0,255,128)            //blue, semitransparent
        , selected_color(0,0,255)               //blue
        , focussed_box_color(0,0,255)           //blue
        , unfocussed_box_color(200,200,200)     //light grey
        , not_highlighted_voice_color(160, 160, 160)    //medium grey
        , page_border_flag(true)
        , cast_shadow_flag(true)
        , draw_focus_lines_on_boxes_flag(false)
        , draw_shapes_highlighted(false)
        , draw_shapes_dragged(false)
        , draw_shapes_selected(false)
        , draw_voices_coloured(false)
        , read_only_mode(true)
        , highlighted_voice(0)                  //0=none, 1..n= voice 1..n
    {
        boxes.reset();

        //default colors for voices
        voiceColor[0] = Color(  0,   0,   0);   //black
        voiceColor[1] = Color(190,  40,  40);   //dark red
        voiceColor[2] = Color( 40,  52, 190);   //dark blue
        voiceColor[3] = Color( 40, 190, 178);   //turquose
        voiceColor[4] = Color(190,  40, 165);   //dark magenta
        voiceColor[5] = Color( 87,  40, 190);   //violet
        voiceColor[6] = Color( 40, 128, 190);   //blue
        voiceColor[7] = Color( 50, 153,  60);   //dark green
        voiceColor[8] = Color(126, 153,  50);   //olive green
    }

    void reset_boxes_to_draw()
    {
        boxes.reset();
    }

    void draw_box_for(int type)
    {
        boxes[type] = true;
    }

    bool must_draw_box_for(int type)
    {
        return boxes[type];
    }


};

//---------------------------------------------------------------------------------------
// SvgOptions: struct holding the rendering options for generating SVG
//---------------------------------------------------------------------------------------
struct SvgOptions
{
    int indent = 0;                 //num.spaces for each indentation step
    bool add_id = true;             //include id='..' in elements
    bool add_class = true;          //include class="...." in elements
    bool add_newlines = false;      //add a new lines after each element

    SvgOptions() {}
};


// Values for different attributes
//---------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    enum ELineJoin
    {
        k_miter_join = agg::miter_join,
        k_round_join = agg::round_join,
        k_bevel_join = agg::bevel_join
    };

    //-------------------------------------------------------------------------
    enum ETextAlignment
    {
        k_align_left = 0,
        k_align_right,
        k_align_center,
        k_align_top = k_align_right,
        k_align_bottom = k_align_left
    };

    //-------------------------------------------------------------------------
    enum EBlendMode     //agg constants in agg_pixfmt_rgba.h
    {
        k_blend_alpha       = agg::end_of_comp_op_e,
        k_blend_clear       = agg::comp_op_clear,
        k_blend_src         = agg::comp_op_src,
        k_blend_dst         = agg::comp_op_dst,
        k_blend_src_over    = agg::comp_op_src_over,
        k_blend_dst_over    = agg::comp_op_dst_over,
        k_blend_src_in      = agg::comp_op_src_in,
        k_blend_dst_in      = agg::comp_op_dst_in,
        k_blend_src_out     = agg::comp_op_src_out,
        k_blend_dst_out     = agg::comp_op_dst_out,
        k_blend_src_atop    = agg::comp_op_src_atop,
        k_blend_dst_atop    = agg::comp_op_dst_atop,
        k_blend_xor         = agg::comp_op_xor,
        k_blend_plus        = agg::comp_op_plus,
        //k_blend_minus       = agg::comp_op_minus,
        k_blend_multiply    = agg::comp_op_multiply,
        k_blend_screen      = agg::comp_op_screen,
        k_blend_overlay     = agg::comp_op_overlay,
        k_blend_darken      = agg::comp_op_darken,
        k_blend_lighten     = agg::comp_op_lighten,
        k_blend_color_dodge = agg::comp_op_color_dodge,
        k_blend_color_burn  = agg::comp_op_color_burn,
        k_blend_hard_light  = agg::comp_op_hard_light,
        k_blend_soft_light  = agg::comp_op_soft_light,
        k_blend_difference  = agg::comp_op_difference,
        k_blend_exclusion   = agg::comp_op_exclusion,
        //k_blend_contrast    = agg::comp_op_contrast,
    };

    //-------------------------------------------------------------------------
    enum EResamplingQuality
    {
        k_quality_low = 0,  ///uses a nearest-neighbour algorithm
        k_quality_medium,   //bilinear, for upsampling and area-averaging for downsampling
        k_quality_high,     //bicubic, for upsampling and area-averaging for downsampling
    };

///@endcond

//---------------------------------------------------------------------------------------
/** %Drawer is an abstract base class for any drawer object.
    A drawer is responsible for transforming drawing commands into something
    understandable by the underlying rendering engine (e.g.: a bitmap, svg file, paths,
    etc.), to display the sheet music.

    The %Drawer class is equivalent to the drawing interfaces present the operating
    systems' graphics libraries, for example macOS' “Quartz”, Linux X Window System's
    “Xlib/XCB” or the Microsoft Windows API “Device Context” (DC) that is part
    of the “Graphics Device Interface” (GDI).

    As Lomse aims to be platform independent, it does not use any platform specific
    graphics interface. Instead it was decided to create an abstract interface
    class: %Drawer. This solution allows any user application to implement its own
    drawer classes and do all drawing natively without having to use the
    BitmapDrawer class implemented by Lomse.

    The interface of %Drawer class mimics SVG commands.
*/
class Drawer
{
protected:
    LibraryScope& m_libraryScope;
    FontStorage* m_pFonts;
    Color m_textColor;

    //current viewport origin and size, in device units (e.g. Pixel)
    Point<double> m_viewportOrg;
    Size<double>  m_viewportSize;

public:
    /** Constructor.
        @param libraryScope  The library Scope object associated to this instance of the
            Lomse library. Can be obtained with method LomseDoorway::get_library_scope().

        @attention
        Once created and injected in a View, the ownership of the %Drawer is passed to
        the View.
    */
    Drawer(LibraryScope& libraryScope);

    /** Destructor */
    virtual ~Drawer() {}


    // http://www.w3.org/TR/SVG/paths.html#PathData
    /** @name SVG path commands
        Methods to define a path and all its commands.
    */
    //@{

    /** Start a path. Equivalent to SVG start tag "<path ".  */
    virtual void begin_path() = 0;                                  //SVG: <path>

    /** Terminate a path. Equivalent to closing the SVG path element " />".  */
    virtual void end_path() = 0;                                    //SVG: </path>

    /** Close a sub-path. Equivalent to the "closepath" (Z and z) SVG command.  */
    virtual void close_path() = 0;                               //SVG: Z, z

    /** Creates a new path with its commands and points provided by the "VertexSource"
        parameter. It is similar to SVG "path" element.
    */
    virtual void add_path(VertexSource& vs, unsigned path_id = 0, bool solid_path = true) = 0;

    /** Move to (absolute coordinates) command. The effect is as if the "pen" were
        lifted and moved to a new location. It is equivalent to the abs. "moveto"
        (M) SVG path instruction.
    */
    virtual void move_to(double x, double y) = 0;                   //SVG: M

    /** Move to (relative coordinates) command. The effect is as if the "pen" were
        lifted and moved to a new location. It is equivalent to the rel. "moveto"
        (m) SVG path instruction.
    */
    virtual void move_to_rel(double x, double y) = 0;               //SVG: m

    /** Line to (absolute coordinates) command. It is equivalent to the abs. "lineto"
        (L) SVG path instruction.
    */
    virtual void line_to(double x,  double y) = 0;                  //SVG: L

    /** Line to (relative coordinates) command. It is equivalent to the rel. "lineto"
        (l) SVG path instruction.
    */
    virtual void line_to_rel(double x,  double y) = 0;              //SVG: l

    /** Horizontal line to (absolute coordinates) command. It is equivalent to the
        "H" SVG path instruction.
    */
    virtual void hline_to(double x) = 0;                            //SVG: H

    /** Horizontal line to (relative coordinates) command. It is equivalent to the
        "h" SVG path instruction.
    */
    virtual void hline_to_rel(double x) = 0;                        //SVG: h

    /** Vertical line to (absolute coordinates) command. It is equivalent to the
        "V" SVG path instruction.
    */
    virtual void vline_to(double y) = 0;                            //SVG: V

    /** Vertical line to (relative coordinates) command. It is equivalent to the
        "v" SVG path instruction.
    */
    virtual void vline_to_rel(double y) = 0;                        //SVG: v

    /** Draws a quadratic Bézier curve from the current point to (x,y) using (x1,y1)
        as the control point.
        All points in absolute coordinates.
        It is equivalent to the "Q" SVG path instruction.
    */
    virtual void quadratic_bezier(double x1, double y1,             //SVG: Q  quadratic Bézier, abs (x1 y1 x y)
                                  double x, double y) = 0;

    /** Draws a quadratic Bézier curve from the current point to (x,y) using (x1,y1)
        as the control point.
        All points in relative coordinates.
        It is equivalent to the "q" SVG path instruction.
    */
    virtual void quadratic_bezier_rel(double x1, double y1,         //SVG: q  quadratic Bézier, rel (x1 y1 x y)
                                      double x, double y) = 0;

    /** Draws a quadratic Bézier curve from the current point to (x,y). The control
        point is assumed to be the reflection of the control point on the previous
        command relative to the current point. (If there is no previous command or if
        the previous command was not a Q, q, T or t, assume the control point is
        coincident with the current point.)
        All points in absolute coordinates.
        It is equivalent to the "T" SVG path instruction.
    */
    virtual void quadratic_bezier(double x, double y) = 0;          //SVG: T  smooth quadratic Bézier, abs (x y)

    /** Draws a quadratic Bézier curve from the current point to (x,y). The control
        point is assumed to be the reflection of the control point on the previous
        command relative to the current point. (If there is no previous command or if
        the previous command was not a Q, q, T or t, assume the control point is
        coincident with the current point.)
        All points in relative coordinates.
        It is equivalent to the "t" SVG path instruction.
    */
    virtual void quadratic_bezier_rel(double x, double y) = 0;      //SVG: t  smooth quadratic Bézier, rel (x y)

    /** Draws a cubic Bézier curve from the current point to (x,y) using (x1,y1) as
        the control point at the beginning of the curve and (x2,y2) as the control
        point at the end of the curve. All points in absolute coordinates.
        It is equivalent to the "C" SVG path instruction.
    */
    virtual void cubic_bezier(double x1, double y1,                 //SVG: C  cubic Bézier, abs (x1 y1 x2 y2 x y)
                              double x2, double y2,
                              double x, double y) = 0;

    /** Draws a cubic Bézier curve from the current point to (x,y) using (x1,y1) as
        the control point at the beginning of the curve and (x2,y2) as the control
        point at the end of the curve. All points in absolute coordinates.
        It is equivalent to the "c" SVG path instruction.
    */
    virtual void cubic_bezier_rel(double x1, double y1,             //SVG: c  cubic Bézier, rel (x1 y1 x2 y2 x y)
                                  double x2, double y2,
                                  double x, double y) = 0;

    /** Draws a cubic Bézier curve from the current point to (x,y). The first control
        point is assumed to be the reflection of the second control point on the previous
        command relative to the current point. (If there is no previous command or if
        the previous command was not an C, c, S or s, assume the first control point
        is coincident with the current point.) (x2,y2) is the second control point
        (i.e., the control point at the end of the curve).
        All points in absolute coordinates.
        It is equivalent to the "S" SVG path instruction.
    */
    virtual void cubic_bezier(double x2, double y2,                //SVG: S  smooth cubic Bézier, abs (x2 y2 x y)
                              double x, double y) = 0;

    /** Draws a cubic Bézier curve from the current point to (x,y). The first control
        point is assumed to be the reflection of the second control point on the previous
        command relative to the current point. (If there is no previous command or if
        the previous command was not an C, c, S or s, assume the first control point
        is coincident with the current point.) (x2,y2) is the second control point
        (i.e., the control point at the end of the curve).
        All points in relative coordinates.
        It is equivalent to the "s" SVG path instruction.
    */
    virtual void cubic_bezier_rel(double x2, double y2,            //SVG: s  smooth cubic Bézier, rel (x2 y2 x y)
                                  double x, double y) = 0;

    //curve?: elliptical_arc (A and a)

    //@}    //SVG path commands


    /** @name SVG basic shapes commands
        Methods to draw elements defined by some combination of straight lines and
        curves. Specifically: ‘circle’, ‘line’, ‘path’, ‘polygon’, and ‘rect’.

        Using these methods requires first to invoque begin_path(), as well as
        to define the desired attributes by invoking attribute setting methods
        (e.g. fill(), stroke(), ...). And finally closing the path close_path().
    */
    //@{

    /** Draw a rectangle which is axis-aligned with the current user coordinate system.
        Rounded rectangles can be achieved by setting a non-zero value for the radius
        parameter.
        It is equivalent to the "rect" SVG element.
    */
    virtual void rect(UPoint pos, USize size, LUnits radius) = 0;           //SVG: <rect>

    /** Draw a circle based on a center point and a radius.
        It is equivalent to the "circle" SVG element.
    */
    virtual void circle(LUnits xCenter, LUnits yCenter, LUnits radius) = 0; //SVG: <circle>

    //virtual void ellipse() = 0;                                           //SVG: <ellipse>

    /** Draw a line segment that starts at one point and ends at another.
        It is equivalent to the "circle" SVG element.
    */
    virtual void line(LUnits x1, LUnits y1, LUnits x2, LUnits y2,
                      LUnits width, ELineEdge nEdge=k_edge_normal) = 0;     //SVG: <line>

    //virtual void polyline() = 0;                                          //SVG: <polyline>

    /** Draw a closed shape consisting of a set of connected straight line segments.
        It is equivalent to the "polygon" SVG element.
    */
   virtual void polygon(int n, UPoint points[]) = 0;                       //SVG: <polygon>

    /** SVG line with start/end markers. Absolute coordinates. */
    virtual void line_with_markers(UPoint start, UPoint end, LUnits width,
                                   ELineCap startCap, ELineCap endCap) = 0;
    //@}    //SVG basic shapes commands



    /** @name Attribute setting methods
        Define the attributes for current open path.
    */
    //@{
    /** Set the color to fill current open path. Color can have alpha value,
        if transparency is desired. */
    virtual void fill(Color color) = 0;

    /** Do not fill current open path. */
    virtual void fill_none() = 0;

    /** Set the color to stroke current open path. Color can have alpha value,
        if transparency is desired. */
    virtual void stroke(Color color) = 0;

    /** Do not stroke current open path */
    virtual void stroke_none() = 0;

    /** Define the width (in model units, a tenth of one millimeter) of the
        line to stroke current open path. */
    virtual void stroke_width(double w) = 0;

    /** Define a linear gradient that can be used in current open path.  */
    virtual void gradient_color(Color c1, Color c2, double start, double stop) = 0;

    /** Define a linear gradient that can be used in current open path.  */
    virtual void gradient_color(Color c1, double start, double stop) = 0;

    /** Set the currently defined linear gradient to be used for filling current open
        path. These methods are used by Lomse as follows:

        @code
        //draw a rounded rectangle filled with a gradient color and a border
        pDrawer->begin_path();
        pDrawer->fill( normalColor );
        pDrawer->stroke( strokeColor );
        pDrawer->stroke_width(15.0);
        Color white(255, 255, 255);
        Color dark(40, 30, 20);
        dark.a = 45;        //set alpha channel. It could have been included in color creation
        Color light(dark);
        light = light.gradient(white, 0.2);
        pDrawer->gradient_color(white, 0.0, 0.1);
        pDrawer->gradient_color(white, dark, 0.1, 0.7);
        pDrawer->gradient_color(dark, light, 0.7, 1.0);
        pDrawer->fill_linear_gradient(m_pos.x, m_pos.y,
                                      m_pos.x, m_pos.y + m_height);
        pDrawer->rect(pos, USize(width, height), 100.0f);
        pDrawer->end_path();
        @endcode
    */
    virtual void fill_linear_gradient(LUnits x1, LUnits y1, LUnits x2, LUnits y2) = 0;

    //Not currently used by Lomse. They were initially defined for completness but
    //probably should be removed
///@cond INTERNALS
    #if (0)
    /** Define the value for the SVG fill-rule property for current open path.
        @param flag   If @TRUE the 'evenodd' rule will be used. If @FALSE the
            'nonzero' rule will be used.
    */
    virtual void even_odd(bool flag) = 0;

    /** Define the alpha value for the color to use for filling the current path. */
    virtual void fill_opacity(unsigned op) = 0;

    /** Define the alpha value for the color to use for stroking the current path. */
    virtual void stroke_opacity(unsigned op) = 0;

    /** Define the shape to be used at the corners of paths or basic shapes when they
        are stroked. It corresponds to the 'stroke-linejoin' SVG property.
    */
    //Need to change line_join_e by ELineJoin
    virtual void line_join(line_join_e join) = 0;

    /** Define the 'stroke-linecap' property to use for stroking the current path. */
    //Need to change line_cap_e by ELineCap
    virtual void line_cap(line_cap_e cap) = 0;

    /** Impose a limit on the extent of the line join. This corresponds to the
        'stroke-miterlimit' SVG property.
    */
    virtual void miter_limit(double ml) = 0;
    #endif
///@endcond

    //@}    //Attribute setting methods


    /** @name Text redering methods
        Are somehow equivalent to the "<text>" SVG element.
    */
    //@{
    /** Select the font to be used form now on in all text rendering methods. The
        parameters are equivalent to the attibutes in the "text" SVG element.
        @param language  A language code drawn from ISO 639, optionally extended with
            a country code drawn from ISO 3166, as 'en-US'. It represents the default
            language for the texts to be rendered. If not specified "en" is assumed.
        @param fontFile Filename (without path) for the font file to use. This parameter
            is deprecated and should not be used; it will normally be empty.
        @param fontName  The name of a font, as it is usually specified in the
            'font-name' CCS property. But it must contain only one font name,
            without quotes, e.g.: "Times New Roman". It can be generic, e.g.: "sans".
        @param height   Desired font height, in points.
        @param fBold    @TRUE if the desired font weight is 'bold'.
        @param fItalic  @TRUE if the desired font style is 'italic'.

        The font file must be selected based on the values for all these parameters.
        If exact match is not possible it is expected than a substitution font be
        selected.
    */
    virtual bool select_font(const std::string& language,
                             const std::string& fontFile,
                             const std::string& fontName, double height,
                             bool fBold=false, bool fItalic=false) = 0;

    /** Define the font-color attribute to use from now on. */
    virtual void set_text_color(Color color);

    /** Render text. This method is equivalent to providing the x,y attributes
        for the "<text>" SVG element and the string content for that element.
        @param x,y  Destination point, in model coordinates (LUnits).
        @param str  The text to render.
    */
    virtual int draw_text(double x, double y, const std::string& str) = 0;

    /** Render text. This method is equivalent to providing the x,y attributes
        for the "<text>" SVG element and the string content for that element.
        @param x,y  Destination point, in model coordinates (LUnits).
        @param str  The text to render.
    */
    virtual int draw_text(double x, double y, const wstring& str) = 0;

    /** Render a character. This method is equivalent to providing the x,y attributes
        for the "<text>" SVG element and the string content for that element.
        @param x,y  Destination point, in model coordinates (LUnits).
        @param ch  The character to render.
    */
    virtual void draw_glyph(double x, double y, unsigned int ch) = 0;
    virtual void draw_glyph_rotated(double x, double y, unsigned int ch, double rotation) = 0;
    //@}    //Text redering methods


    /** @name Render a bitmap
    */
    //@{
    //Not currently used by Lomse. Probably should be removed
///@cond INTERNALS
    #if (0)
    /** Render a full bitmap.
        @param img  The struct RenderingBuffer containing the pointer to the bitmap
            and its dimensions in pixels.
        @param pos  The destination top-left corner of the bitmap, in model units.

        This method is not currently used by Lomse so perhaps you coud delay its
        implementation until it is really needed future (not probable).
    */
    virtual void copy_bitmap(RenderingBuffer& img, UPoint pos) = 0;

    /** Render part of a bitmap.
        @param bmap  The struct RenderingBuffer containing the pointer to the bitmap
            and its dimensions in pixels.
        @param srcX1, srcY1, srcX2, srcY2 top-left and bottom-right points of the
            source bitmap, in device units.
        @param dest  The destination top-left corner of the bitmap, in model units.

        This method is not currently used by Lomse so perhaps you coud delay its
        implementation until it is really needed future (not probable).
    */
    virtual void copy_bitmap(RenderingBuffer& bmap,
                             Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                             UPoint dest) = 0;
    #endif
///@endcond

    /** Render part of a bitmap. It is somehow equivalent to the "image" SVG
        element.
        @param bmap  The struct RenderingBuffer containing the pointer to the bitmap
            and its dimensions in pixels.
        @param hasAlpha   @TRUE when the bitmap has alpha channel information.
        @param srcX1, srcY1, srcX2, srcY2   Top-left and bottom-right points of the
            source bitmap, in device units.
        @param dstX1, dstY1, dstX2, dstY2   The destination top-left corner and
            bottom-right corner, in model units.
        @param resamplingMode   Value from enum EResamplingQuality that defines
            how resampling (if required) must be done.
        @param alpha    Transparency (alpha value: 1.0 opaque).

        This method is currently used by Lomse when the Document includes a JPG
        or PNG image.
    */
    virtual void draw_bitmap(RenderingBuffer& bmap, bool hasAlpha,
                             Pixels srcX1, Pixels srcY1, Pixels srcX2, Pixels srcY2,
                             LUnits dstX1, LUnits dstY1, LUnits dstX2, LUnits dstY2,
                             EResamplingQuality resamplingMode,
                             double alpha=1.0) = 0;
    //@}    //Render a bitmap



    /** @name Drawer settings
        Methods to render a bitmap. They are somehow equivalent to the "image" SVG
        element.
        The only one currently used by Lomse  is draw_bitmap() and is used when the
        Document includes some JPG or PNG image.
    */
    //@{

    /** Add a temporay shift to subsequent elements and paths created after
        invoking this method. It is a facility to avoid that classes using the drawer
        having to directly manipulate the affine matrix for some position adjustment.
    */
    virtual void set_shift(LUnits x, LUnits y) = 0;

    /** Remove the temporary shift defined in the set_shift() method. */
    virtual void remove_shift() = 0;

    /** Do render currently defined paths. */
    virtual void render() = 0;

    /** Set the affine transformation matrix to use. */
    virtual void set_affine_transformation(TransAffine& transform) = 0;

    /** Set the background color and prepare to render a new image. */
    virtual void reset(Color bgcolor) = 0;

    //@}    //Drawer settings



    /** @name Device <--> model units conversion
        In Lomse, there are two types of coordinates: those defined in the device space,
        that is, the %Drawer space (e.g. pixels) and those defined in the model space, that
        is, real world units (tenths of one millimeter).
    */
    //@{
    /** Convert a point in device coordinates (e.g. pixels) to model coordinates
        (real world units, tenths of one millimeter).
    */
    virtual void device_point_to_model(double* x, double* y) const = 0;

    /** Convert a point in model coordinates (real world units, tenths of one
        millimeter) to device coordinates (e.g. pixels).
    */
    virtual void model_point_to_device(double* x, double* y) const = 0;

    /** Convert device units (e.g. pixels) to model units
        (real world units, tenths of one millimeter).
    */
    virtual LUnits device_units_to_model(double value) const = 0;

    /** Convert model units (real world units, tenths of one millimeter) to device
        units (e.g. pixels).
    */
    virtual double model_to_device_units(LUnits value) const = 0;
    //@}    //Device <--> model units conversion



    /** @name Viewport info
        Viewport is a concept related to the View, not to the %Drawer. But for some %Drawer
        implementations, this information can be required. For instance, the %Drawer can
        speed up rendition if it knows the area that is currently visible, so that it
        can ignore non-visible areas.
        Therefore, the View always informs the %Drawer when any change in the viewport.
        Received coordinates are in device units (e.g. Pixels).
    */
    //@{
    /** Set a new origin for the viewport. Its size does not change.
        Received coordinates are in device units (e.g. Pixels).
    */
    virtual void new_viewport_origin(double x, double y);

    /** Chage the size of the viewport. Its origin is not affected.
        Received values are in device units (e.g. Pixels).
    */
    virtual void new_viewport_size(double x, double y);
    //@}    //Viewport info


    /** @name Other methods */
    //@{
    /** Returns @TRUE if the %Drawer is initialized and can be used. */
    virtual bool is_ready() const = 0;
    //@}    //Other methods

};


//=======================================================================================
// Helper class LineVertexSource: a vertex source for a line
//=======================================================================================
class LineVertexSource : public VertexSource
{
protected:
    double x1, y1, x2, y2;
    int f;

public:
    LineVertexSource(double x1_, double y1_, double x2_, double y2_)
        : VertexSource()
        , x1(x1_)
        , y1(y1_)
        , x2(x2_)
        , y2(y2_)
        , f(0)
    {
    }

    void rewind(unsigned) override { f = 0; }
    unsigned vertex(double* x, double* y) override
    {
        if(f == 0) { ++f; *x = x1; *y = y1; return agg::path_cmd_move_to; }
        if(f == 1) { ++f; *x = x2; *y = y2; return agg::path_cmd_line_to; }
        return agg::path_cmd_stop;
    }
};


//=======================================================================================
// Helper class MarkerVertexSource:
//    It is a vertex source for different line caps markers
//=======================================================================================
class MarkerVertexSource : public VertexSource
{
public:
    MarkerVertexSource();

    void head_arrowhead(double d1, double d2, double d3, double d4)
    {
        m_head_d1 = d1;
        m_head_d2 = d2;
        m_head_d3 = d3;
        m_head_d4 = d4;
        m_head_type = k_arrowhead;
    }

    void head_arrowtail(double d1, double d2, double d3, double d4)
    {
        m_head_d1 = d1;
        m_head_d2 = d2;
        m_head_d3 = d3;
        m_head_d4 = d4;
        m_head_type = k_arrowtail;
    }

    void head_circle(double r1)
    {
        m_head_d1 = r1;
        m_head_type = k_circle;
    }

    void head_square(double d1, double d2)
    {
        m_head_d1 = d1;
        m_head_d2 = d2;
        m_head_d3 = d1 / 2.0;
        m_head_type = k_square;
    }

    void head_diamond(double d1)
    {
        m_head_d1 = d1;
        m_head_d2 = d1;
        m_head_d3 = d1;
        m_head_d4 = -d1;
        m_head_type = k_arrowhead;
    }

    void no_head() { m_head_type = k_none; }

    //-----------------------------------------------------------------------------------

    void tail_diamond(double d1)
    {
        m_tail_d1 = d1;
        m_tail_d2 = d1;
        m_tail_d3 = d1;
        m_tail_d4 = -d1;
        m_tail_type = k_arrowhead;
    }

    void tail_arrowhead(double d1, double d2, double d3, double d4)
    {
        m_tail_d1 = d1;
        m_tail_d2 = d2;
        m_tail_d3 = d3;
        m_tail_d4 = d4;
        m_tail_type = k_arrowhead;
    }

    void tail_arrowtail(double d1, double d2, double d3, double d4)
    {
        m_tail_d1 = d1;
        m_tail_d2 = d2;
        m_tail_d3 = d3;
        m_tail_d4 = d4;
        m_tail_type = k_arrowtail;
    }

    void tail_square(double d1, double d2)
    {
        m_tail_d1 = d1;
        m_tail_d2 = d2;
        m_tail_d3 = d1 / 2.0;
        m_tail_type = k_square;
    }

    void tail_circle(double r1)
    {
        m_tail_d1 = r1;
        m_tail_type = k_circle;
    }

    void no_tail() { m_tail_type = k_none; }

    void rewind(unsigned path_id) override;
    unsigned vertex(double* x, double* y) override;

private:
    double   m_head_d1;
    double   m_head_d2;
    double   m_head_d3;
    double   m_head_d4;
    double   m_tail_d1;
    double   m_tail_d2;
    double   m_tail_d3;
    double   m_tail_d4;

    enum type_e { k_none=0, k_arrowhead, k_arrowtail, k_circle, k_square, };
    unsigned    m_head_type;
    unsigned    m_tail_type;

    double      m_coord[16];
    unsigned    m_cmd[8];
    unsigned    m_curr_id;
    unsigned    m_curr_coord;

    enum status_e
    {
        stop,
        circle_start,
        circle_point,
        points,
    };

    unsigned        m_status;
    agg::ellipse    m_circle;
    double			m_radius;
};


//=======================================================================================
// Helper class LineCapsConverter
// It is a conversion pipeline to add stroke and line head/tail markers.
// Internally it has three converters:
//  * stroke_type [of type agg::conv_stroke] converts the path to the required stroke
//  * marker_type [of type agg::conv_marker, MarkerVertexSource>] adds an arrow head
//    to the line.
//  * concat_type [of type agg::conv_concat] concats both converters
//=======================================================================================
template<class Source> class LineCapsConverter : public VertexSource
{
protected:
    typedef agg::conv_stroke<Source, agg::vcgen_markers_term> stroke_type;
    typedef agg::conv_marker<typename stroke_type::marker_type, MarkerVertexSource>
            marker_type;
    typedef agg::conv_concat<stroke_type, marker_type> concat_type;

    stroke_type    s;
    MarkerVertexSource   vs;
    marker_type    m;
    concat_type    c;

public:
    LineCapsConverter(Source& src, double w, ELineCap nStartCap, ELineCap nEndCap)
        : s(src)
        , vs()
        , m(s.markers(), vs)
        , c(s, m)
    {
        s.width(w);

        switch(nStartCap)
        {
            case k_cap_none:
                break;

            case k_cap_arrowhead:
                vs.head_arrowhead(3.0*w, 3.0*w, 2.25*w, 1.5*w);
                break;

            case k_cap_arrowtail:
                vs.head_arrowtail(5.0*w, 2.0*w, 2.0*w, 5.0*w);
                break;

            case k_cap_diamond:
                vs.head_diamond(3.0*w);
                break;

            case k_cap_square:
                vs.head_square(4.0*w, 2.0*w);
                break;

            case k_cap_circle:
                vs.head_circle(2.8*w);
                break;
        }

        switch(nEndCap)
        {
            case k_cap_none:
                break;

            case k_cap_arrowhead:
                vs.tail_arrowhead(3.0*w, 3.0*w, 2.25*w, 1.5*w);
                break;

            case k_cap_arrowtail:
                vs.tail_arrowtail(5.0*w, 2.0*w, 2.0*w, 5.0*w);
                break;

            case k_cap_diamond:
                vs.tail_diamond(3.0*w);
                break;

            case k_cap_square:
                vs.tail_square(4.0*w, 2.0*w);
                break;

            case k_cap_circle:
                vs.tail_circle(2.8*w);
                break;
        }
        //s.shorten(w * 2.0);       //reduce el tamaño de la linea, recortando el final
    }

    void rewind(unsigned path_id) override { c.rewind(path_id); }
    unsigned vertex(double* x, double* y) override { return c.vertex(x, y); }
};


}   //namespace lomse

#endif    // __LOMSE_DRAWER_H__

