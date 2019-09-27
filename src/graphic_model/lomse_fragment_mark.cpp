//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#include "lomse_fragment_mark.h"

#include "lomse_screen_drawer.h"
#include "lomse_gm_basic.h"
#include "lomse_logger.h"
#include "lomse_graphic_view.h"
#include "lomse_box_system.h"
#include "lomse_shape_staff.h"
#include "lomse_vertex_source.h"
#include "agg_trans_affine.h"


namespace lomse
{

//=======================================================================================
// Class RawShape. Base class for all auxiliary objects for drawing shapes.
//=======================================================================================
/** %RawShape is the base class for all auxiliary objects for drawing shapes.
    %RawShape objects are not part of the Graphic Model and are intended to be used in
    GmoShapes for drawing the shape and in VisualEffect objects for drawing the
    visual effect.

    This base class defines the mandatory methods that must be implement in any derived
    class. The methods for positioning the shape are specific for each derived class.
*/
class RawShape
{
protected:
    URect m_bounds;

public:
    RawShape() {};
    virtual ~RawShape() {};

//    ///Draw the shape
//    void on_draw(ScreenDrawer* pDrawer, Color color=Color(0,0,0)) = 0;
    ///Return the shape bounding box
    virtual URect get_bounds() { return m_bounds; }

protected:
    //debug
    void draw_bounding_box(ScreenDrawer* pDrawer);

};


//=======================================================================================
// Class RawShapeRoundedBracket. RawShape class to draw open/close rounded brackets
//=======================================================================================
class RawShapeRoundedBracket : public RawShape, public VertexSource
{
protected:
    int                 m_nCurVertex;   //index to current vertex
    int                 m_nContour;     //current countour
    agg::trans_affine   m_trans;        //affine transformation to apply

    double              m_rBarlineHeight;

    static Vertex m_topHookVertices[];
    static Vertex m_bottomHookVertices[];
    static float m_dxBarline;
    static float m_hooksHeight;
    static double m_xMin;
    static double m_yMin;
    static double m_xMax;
    static double m_yMax;

    int m_numVerticesTop;
    int m_numVerticesBottom;


public:
    RawShapeRoundedBracket();

    void draw(ScreenDrawer* pDrawer, LUnits xLeft, LUnits yTop,
              LUnits xRight, LUnits yBottom, bool fOpen, Color color = Color(0,0,0));

//    //RawShape
//    URect get_bounds() override;

protected:
    void set_position_bounds(LUnits xLeft, LUnits yTop, LUnits xRight, LUnits yBottom, bool fOpen);

    //VertexSource
    unsigned vertex(double* px, double* py) override;
    void rewind(int UNUSED(pathId) = 0) override { m_nCurVertex = 0; m_nContour = 0; }
};

//=======================================================================================
// Class RawShapeCurlyBracket. RawShape class to draw open/close curly brackets
//=======================================================================================
class RawShapeCurlyBracket : public RawShape, public VertexSource
{
protected:
    int                 m_nCurVertex;   //index to current vertex
    int                 m_nContour;     //current countour
    agg::trans_affine   m_trans;        //affine transformation to apply

    double  m_rBarlineHeight;
    double  m_rBarlineHalf;

    static Vertex m_topVertices[];
    static Vertex m_centerDownVertices[];
    static Vertex m_bottomVertices[];
    static Vertex m_centerUpVertices[];
    static float m_dxBarline;
    static float m_hooksHeight;
    static float m_centerHeight;
    static double m_xMin;
    static double m_yMin;
    static double m_xMax;
    static double m_yMax;

    int m_numVerticesTop;
    int m_numVerticesCenterDown;
    int m_numVerticesBottom;
    int m_numVerticesCenterUp;


public:
    RawShapeCurlyBracket();

    void draw(ScreenDrawer* pDrawer, LUnits xLeft, LUnits yTop,
              LUnits xRight, LUnits yBottom, bool fOpen, Color color = Color(0,0,0));

    //URect get_bounds();

protected:
    void set_position_bounds(LUnits xLeft, LUnits yTop, LUnits xRight, LUnits yBottom, bool fOpen);

    //VertexSource
    unsigned vertex(double* px, double* py) override;
    void rewind(int UNUSED(pathId) = 0) override { m_nCurVertex = 0; m_nContour = 0; }
};


//=======================================================================================
// FragmentMark implementation
//=======================================================================================
FragmentMark::FragmentMark(GraphicView* view, LibraryScope& libraryScope)
    : ApplicationMark(view, libraryScope)
    , m_type(k_mark_line)
    , m_color(Color(255,0,0,128))   // transparent red
    , m_lineStyle(k_line_solid)
    , m_pBoxSystem(nullptr)
    , m_iPage(0)
    , m_xLeft(1000.0)       //arbitrary, to have a valid value
    , m_yTop(1000.0)        //arbitrary, to have a valid value
    , m_yBottom(2000.0)     //arbitrary, to have a valid value
    , m_extension(300.0)    //3.0 mm
    , m_thickness(100.0)    // 1.00 mm. Arbitrary, to have a valid value)
{
}

//---------------------------------------------------------------------------------------
void FragmentMark::initialize(LUnits xPos, GmoBoxSystem* pBoxSystem, bool fBarline)
{
    //This method is protected. It is only invoked one time, when the mark is created.

    m_xLeft = xPos;
    m_fCentered = fBarline;

    if (pBoxSystem)
    {
        m_pBoxSystem = pBoxSystem;
        m_iPage = pBoxSystem->get_page_number();

        top(0,0);       //top point in first instrument, first staff
        bottom(-1,-1);  //bottom point at last instrument, last staff

        //determine extensiton to reach system bounds
        LUnits height = m_yBottom - m_yTop;
        m_extension = (pBoxSystem->get_height() - height) / 2.0f;

        //fix xLeft with style margins
        ImoStyle* pStyle = pBoxSystem->get_style();
        if (pStyle)
            m_xLeft += pStyle->margin_left();

        //line thickness
        thickness(6.0f);    //tenths
    }
}

//---------------------------------------------------------------------------------------
FragmentMark* FragmentMark::top(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0f;

    //set top point
    URect staffBounds = pStaff->get_bounds();
    m_yTop = staffBounds.y;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yTop -= pStyle->margin_top();

    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* FragmentMark::bottom(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0f;

    //set bottom point
    URect staffBounds = pStaff->get_bounds();
    m_yBottom = staffBounds.y + staffBounds.height;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yBottom -= pStyle->margin_top();

    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* FragmentMark::x_shift(Tenths dx)
{
    if (!m_pBoxSystem)
        return this;

    m_xLeft += m_pBoxSystem->tenths_to_logical(dx);
    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* FragmentMark::extra_height(Tenths value)
{
    if (!m_pBoxSystem)
        return this;

    m_extension = m_pBoxSystem->tenths_to_logical(value);
    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* FragmentMark::thickness(Tenths value)
{
    if (!m_pBoxSystem)
        return this;

    m_thickness = m_pBoxSystem->tenths_to_logical(value);
    return this;
}

//---------------------------------------------------------------------------------------
void FragmentMark::on_draw(ScreenDrawer* pDrawer)
{
    if (!m_pBoxSystem)
        return;

    LUnits yTop = m_yTop - m_extension;
    LUnits yBottom = m_yBottom + m_extension;
    LUnits hookLength = 200.0;

    UPoint org = m_pView->get_page_origin_for(m_iPage);
    pDrawer->set_shift(-org.x, -org.y);

    if (m_type == k_mark_open_rounded)
    {
        LUnits xShift = (m_fCentered ? m_thickness/2.0f : 0.0f);
        LUnits xLeft = m_xLeft - m_thickness + xShift;
        LUnits xRight = m_xLeft + xShift;

        RawShapeRoundedBracket glyph;
        glyph.draw(pDrawer, xLeft, yTop, xRight, yBottom, true, m_color);
        m_bounds = glyph.get_bounds();
    }
    else if (m_type == k_mark_close_rounded)
    {
        LUnits xShift = (m_fCentered ? m_thickness/2.0f : 0.0f);
        LUnits xLeft = m_xLeft + xShift;
        LUnits xRight = m_xLeft + m_thickness + xShift;

        RawShapeRoundedBracket glyph;
        glyph.draw(pDrawer, xLeft, yTop, xRight, yBottom, false, m_color);
        m_bounds = glyph.get_bounds();
    }
    else if (m_type == k_mark_open_curly)
    {
        LUnits xShift = (m_fCentered ? m_thickness/2.0f : 0.0f);
        LUnits xLeft = m_xLeft - m_thickness + xShift;
        LUnits xRight = m_xLeft + xShift;

        RawShapeCurlyBracket glyph;
        glyph.draw(pDrawer, xLeft, yTop, xRight, yBottom, true, m_color);
        m_bounds = glyph.get_bounds();
    }
    else if (m_type == k_mark_close_curly)
    {
        LUnits xShift = (m_fCentered ? m_thickness/2.0f : 0.0f);
        LUnits xLeft = m_xLeft + xShift;
        LUnits xRight = m_xLeft + m_thickness + xShift;

        RawShapeCurlyBracket glyph;
        glyph.draw(pDrawer, xLeft, yTop, xRight, yBottom, false, m_color);
        m_bounds = glyph.get_bounds();
    }
    else
    {
        pDrawer->begin_path();
        pDrawer->fill(Color(0,0,0,0));  //transparent
        pDrawer->stroke(m_color);
        pDrawer->stroke_width(m_thickness);

        if (m_type == k_mark_open_squared)
        {
            LUnits xShift = (m_fCentered ? m_thickness/2.0f : 0.0f);
            LUnits xLeft = m_xLeft - m_thickness / 2.0f + xShift;
            m_bounds.left(xLeft - m_thickness/2.0f);
            m_bounds.right(xLeft + hookLength);

            m_bounds.top(yTop);
            m_bounds.bottom(yBottom);
            yTop += m_thickness / 2.0f;
            yBottom -= m_thickness / 2.0f;

            //top hook
            pDrawer->move_to(xLeft + hookLength, yTop);
            pDrawer->hline_to(xLeft);

            //vertical line
            pDrawer->vline_to(yBottom);

            //bottom hook
            pDrawer->hline_to(xLeft + hookLength);
        }
        else if (m_type == k_mark_close_squared)
        {
            LUnits xShift = (m_fCentered ? m_thickness/2.0f : 0.0f);
            LUnits xLeft = m_xLeft - m_thickness / 2.0f + xShift;
            m_bounds.left(xLeft - hookLength);
            m_bounds.right(xLeft + m_thickness/2.0f);

            m_bounds.top(yTop);
            m_bounds.bottom(yBottom);
            yTop += m_thickness / 2.0f;
            yBottom -= m_thickness / 2.0f;

            //top hook
            pDrawer->move_to(xLeft - hookLength, yTop);
            pDrawer->hline_to(xLeft);

            //vertical line
            pDrawer->vline_to(yBottom);

            //bottom hook
            pDrawer->hline_to(xLeft - hookLength);
        }
        else
        {
            //vertical line
            pDrawer->move_to(m_xLeft, yTop);
            pDrawer->vline_to(yBottom);

            m_bounds.left(m_xLeft - m_thickness / 2.0f);
            m_bounds.top(yTop);
            m_bounds.bottom(yBottom);
            m_bounds.right(m_xLeft + m_thickness / 2.0f);
       }

        pDrawer->end_path();
    }
    //draw_bounding_box(pDrawer);       //debug

    pDrawer->render();
    pDrawer->remove_shift();

}

//---------------------------------------------------------------------------------------
URect FragmentMark::get_bounds()
{
    return m_bounds;
}

//---------------------------------------------------------------------------------------
void FragmentMark::draw_bounding_box(ScreenDrawer* pDrawer)
{
    pDrawer->begin_path();
    pDrawer->fill(Color(0, 0, 0, 0));
    pDrawer->stroke(Color(255, 0, 255));
    pDrawer->stroke_width(15.0);
    pDrawer->move_to(m_bounds.left(), m_bounds.top());
    pDrawer->hline_to(m_bounds.left() + m_bounds.width);
    pDrawer->vline_to(m_bounds.top() + m_bounds.height);
    pDrawer->hline_to(m_bounds.left());
    pDrawer->vline_to(m_bounds.top());
    pDrawer->end_path();
}



//=======================================================================================
// RawShape implementation
//=======================================================================================
void RawShape::draw_bounding_box(ScreenDrawer* pDrawer)
{
    pDrawer->begin_path();
    pDrawer->fill(Color(0, 0, 0, 0));
    pDrawer->stroke(Color(0, 0, 255));
    pDrawer->stroke_width(15.0);
    pDrawer->move_to(m_bounds.left(), m_bounds.top());
    pDrawer->hline_to(m_bounds.left() + m_bounds.width);
    pDrawer->vline_to(m_bounds.top() + m_bounds.height);
    pDrawer->hline_to(m_bounds.left());
    pDrawer->vline_to(m_bounds.top());
    pDrawer->end_path();
}



//=======================================================================================
// RawShapeRoundedBracket implementation
//=======================================================================================

//top hook
Vertex RawShapeRoundedBracket::m_topHookVertices[] = {
    {   0.0,     0.0, agg::path_cmd_move_to },
    {   0.0,   -39.0, agg::path_cmd_curve3 },      //ctrol
    {  14.0,   -70.0, agg::path_cmd_curve3 },      //on-curve
    {  28.0,  -101.0, agg::path_cmd_curve3 },      //ctrol
    {  52.0,  -122.0, agg::path_cmd_curve3 },      //on-curve
    {  76.0,  -143.0, agg::path_cmd_curve3 },      //ctrol
    { 107.0,  -154.0, agg::path_cmd_curve3 },      //on-curve
    { 138.0,  -165.0, agg::path_cmd_curve3 },      //ctrol
    { 171.0,  -165.0, agg::path_cmd_curve3 },      //on-curve
    { 171.0,  -128.0, agg::path_cmd_line_to },
    { 159.0,  -128.0, agg::path_cmd_curve3 },      //ctrol       <
    { 143.0,  -122.0, agg::path_cmd_curve3 },      //on-curve    <
    { 127.0,  -116.0, agg::path_cmd_curve3 },      //ctrol
    { 113.5,  -106.0, agg::path_cmd_curve3 },      //on-curve
    { 100.0,   -96.0, agg::path_cmd_curve3 },      //ctrol
    {  90.5,   -82.0, agg::path_cmd_curve3 },      //on-curve
    {  81.0,   -69.0, agg::path_cmd_curve3 },      //ctrol
    {  81.0,   -55.0, agg::path_cmd_curve3 },      //on-curve
    {  81.0,     0.0, agg::path_cmd_line_to },
};

//bottom hook
Vertex RawShapeRoundedBracket::m_bottomHookVertices[] = {
    {  81.0,    62.0, agg::path_cmd_line_to },
    {  81.0,    75.0, agg::path_cmd_curve3 },      //ctrol
    {  89.5,    87.0, agg::path_cmd_curve3 },      //on-curve
    {  98.0,    99.0, agg::path_cmd_curve3 },      //ctrol
    { 111.0,   108.0, agg::path_cmd_curve3 },      //on-curve
    { 124.0,   117.0, agg::path_cmd_curve3 },      //ctrol
    { 140.0,   122.5, agg::path_cmd_curve3 },      //on-curve
    { 156.0,   128.0, agg::path_cmd_curve3 },      //ctrol
    { 171.0,   128.0, agg::path_cmd_curve3 },      //on-curve
    { 171.0,   165.0, agg::path_cmd_line_to },
    { 137.0,   165.0, agg::path_cmd_curve3 },      //ctrol
    { 106.0,   151.5, agg::path_cmd_curve3 },      //on-curve
    {  75.0,   138.0, agg::path_cmd_curve3 },      //ctrol
    {  51.5,   115.5, agg::path_cmd_curve3 },      //on-curve
    {  28.0,    93.0, agg::path_cmd_curve3 },      //ctrol
    {  14.0,    63.0, agg::path_cmd_curve3 },      //on-curve
    {   0.0,    33.0, agg::path_cmd_curve3 },      //ctrol
    {   0.0,     0.0, agg::path_cmd_curve3 },      //on-curve
    {   0.0,     0.0, agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw }, //close polygon
    {   0.0,     0.0, agg::path_cmd_stop }
};

float RawShapeRoundedBracket::m_dxBarline = 81.0f;   //width of the vertical line (105 - 24)
float RawShapeRoundedBracket::m_hooksHeight = 330.0;    //2 * height of one hook = 2*165 = 330

double RawShapeRoundedBracket::m_xMin = 0.0;
double RawShapeRoundedBracket::m_yMin = -165.0;
double RawShapeRoundedBracket::m_xMax = 171.0;
double RawShapeRoundedBracket::m_yMax = 165.0;

//---------------------------------------------------------------------------------------
RawShapeRoundedBracket::RawShapeRoundedBracket()
    : RawShape()
{
    m_numVerticesTop = sizeof(m_topHookVertices)/sizeof(Vertex);
    m_numVerticesBottom = sizeof(m_bottomHookVertices)/sizeof(Vertex);
}


//---------------------------------------------------------------------------------------
void RawShapeRoundedBracket::set_position_bounds(LUnits xLeft, LUnits yTop,
                                                 LUnits xRight, LUnits yBottom,
                                                 bool fOpen)
{
    double rxScale((xRight - xLeft) / m_dxBarline);
    double ryScale = rxScale;
    if (!fOpen)
        rxScale = -rxScale;

    double yShift = -m_yMin * ryScale;

    m_trans = agg::trans_affine(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    m_trans *= agg::trans_affine_scaling(rxScale, ryScale);
    m_trans *= agg::trans_affine_translation(xLeft, yTop+yShift);

    //length of the vertical segment
    m_rBarlineHeight = double(yBottom - yTop) - m_hooksHeight * ryScale;

    //bounding box
    double xMin = m_xMin;
    double yMin = m_yMin;
    m_trans.transform(&xMin, &yMin);
    double xMax = m_xMax;
    double yMax = m_yMax;
    m_trans.transform(&xMax, &yMax);

    m_bounds.left( float(fOpen ? xMin : xMax) );
    m_bounds.top(float(yMin));
    m_bounds.set_height( float(abs(yMax - yMin) + m_rBarlineHeight) );
    m_bounds.set_width( float(abs(xMax - xMin)) );
}

//---------------------------------------------------------------------------------------
unsigned RawShapeRoundedBracket::vertex(double* px, double* py)
{
	switch(m_nContour)
	{
		case 0:		//top hook
		{
			*px = m_topHookVertices[m_nCurVertex].ux_coord;
			*py = m_topHookVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

			unsigned cmd = m_topHookVertices[m_nCurVertex++].cmd;

			//change to next contour?
			if(m_nCurVertex == m_numVerticesTop)
            {
				m_nContour++;
                m_nCurVertex = 0;
            }

            return cmd;
		}
		case 1:		//bottom hook
		{
		    if(m_nCurVertex >= m_numVerticesBottom)
			    return agg::path_cmd_stop;

			*px = m_bottomHookVertices[m_nCurVertex].ux_coord;
			*py = m_bottomHookVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

            *py += m_rBarlineHeight;

			return m_bottomHookVertices[m_nCurVertex++].cmd;
		}
		default:
			return agg::path_cmd_stop;
	}
}

//---------------------------------------------------------------------------------------
void RawShapeRoundedBracket::draw(ScreenDrawer* pDrawer, LUnits xLeft, LUnits yTop,
                               LUnits xRight, LUnits yBottom, bool fOpen, Color color)
{
    set_position_bounds(xLeft, yTop, xRight, yBottom, fOpen);
    m_nCurVertex = 0;
    m_nContour = 0;

    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();
}



//=======================================================================================
// RawShapeCurlyBracket implementation
//=======================================================================================

Vertex RawShapeCurlyBracket::m_topVertices[] = {
	{  73.0,    0.0, agg::path_cmd_move_to },
	{  82.0,  -30.0, agg::path_cmd_curve3 },	//ctrol
	{ 103.5,  -51.0, agg::path_cmd_curve3 },	//on-curve
	{ 125.0,  -72.0, agg::path_cmd_curve3 },	//ctrol
	{ 155.0,  -82.0, agg::path_cmd_curve3 },	//on-curve
	{ 155.0, -110.0, agg::path_cmd_line_to },
	{ 112.0, -106.0, agg::path_cmd_curve3 },	//ctrol
	{  73.0,  -89.5, agg::path_cmd_curve3 },	//on-curve
	{  34.0,  -73.0, agg::path_cmd_curve3 },	//ctrol
	{   0.0,  -45.0, agg::path_cmd_curve3 },	//on-curve
};

Vertex RawShapeCurlyBracket::m_centerDownVertices[] = {
	{   0.0,    0.0, agg::path_cmd_line_to },
	{ -66.0,   71.0, agg::path_cmd_line_to },
	{   0.0,  142.0, agg::path_cmd_line_to },
};

Vertex RawShapeCurlyBracket::m_bottomVertices[] = {

	{   0.0,  187.0, agg::path_cmd_line_to },
	{  34.0,  215.0, agg::path_cmd_curve3 },	//ctrol
	{  73.0,  231.5, agg::path_cmd_curve3 },	//on-curve
	{ 112.0,  248.0, agg::path_cmd_curve3 },	//ctrol
	{ 155.0,  252.0, agg::path_cmd_curve3 },	//on-curve
	{ 155.0,  224.0, agg::path_cmd_line_to },
	{ 125.0,  214.0, agg::path_cmd_curve3 },	//ctrol
	{ 103.5,  193.0, agg::path_cmd_curve3 },	//on-curve
	{  82.0,  172.0, agg::path_cmd_curve3 },	//ctrol
	{  73.0,  142.0, agg::path_cmd_curve3 },	//on-curve
};

Vertex RawShapeCurlyBracket::m_centerUpVertices[] = {

	{  73.0,  117.0, agg::path_cmd_line_to },
	{  29.0,   71.0, agg::path_cmd_line_to },
	{  73.0,   25.0, agg::path_cmd_line_to },
    {   0.0,    0.0, agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw }, //close polygon
    {   0.0,    0.0, agg::path_cmd_stop }
};

float RawShapeCurlyBracket::m_dxBarline = 73.0f;     //width of the vertical line (73 - 0)
float RawShapeCurlyBracket::m_hooksHeight = 220.0f;  //2 * height of one hook = 2*(110 - 0) = 220
float RawShapeCurlyBracket::m_centerHeight = 142.0f;  //142 - 0 = 142

double RawShapeCurlyBracket::m_xMin = -66.0;
double RawShapeCurlyBracket::m_yMin = -110.0;
double RawShapeCurlyBracket::m_xMax = 155.0;
double RawShapeCurlyBracket::m_yMax = 252.0;

//---------------------------------------------------------------------------------------
RawShapeCurlyBracket::RawShapeCurlyBracket()
    : RawShape()
{
    m_numVerticesTop = sizeof(m_topVertices)/sizeof(Vertex);
    m_numVerticesCenterDown = sizeof(m_centerDownVertices)/sizeof(Vertex);
    m_numVerticesBottom = sizeof(m_bottomVertices)/sizeof(Vertex);
    m_numVerticesCenterUp = sizeof(m_centerUpVertices)/sizeof(Vertex);
}


//---------------------------------------------------------------------------------------
void RawShapeCurlyBracket::set_position_bounds(LUnits xLeft, LUnits yTop,
                                               LUnits xRight, LUnits yBottom, bool fOpen)
{
    double rxScale((xRight - xLeft) / m_dxBarline);
    double ryScale = rxScale;
    if (!fOpen)
        rxScale = -rxScale;

    double yShift = -m_yMin * ryScale;

    m_trans = agg::trans_affine(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    m_trans *= agg::trans_affine_scaling(rxScale, ryScale);
    m_trans *= agg::trans_affine_translation(xLeft, yTop+yShift);

    m_rBarlineHeight = double(yBottom - yTop) - (m_centerHeight + m_hooksHeight) * ryScale;
    m_rBarlineHalf = m_rBarlineHeight / 2.0;

    //bounding box
    double xMin = m_xMin;
    double yMin = m_yMin;
    m_trans.transform(&xMin, &yMin);
    double xMax = m_xMax;
    double yMax = m_yMax;
    m_trans.transform(&xMax, &yMax);

    m_bounds.left( float(fOpen ? xMin : xMax) );
    m_bounds.top(float(yMin));
    m_bounds.set_height( float(abs(yMax - yMin) + m_rBarlineHeight) );
    m_bounds.set_width( float(abs(xMax - xMin)) );
}

//---------------------------------------------------------------------------------------
unsigned RawShapeCurlyBracket::vertex(double* px, double* py)
{
	switch(m_nContour)
	{
		case 0:		//top hook
		{
			*px = m_topVertices[m_nCurVertex].ux_coord;
			*py = m_topVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

			unsigned cmd = m_topVertices[m_nCurVertex++].cmd;

			//change to next contour?
			if(m_nCurVertex == m_numVerticesTop)
            {
				m_nContour++;
                m_nCurVertex = 0;
            }

            return cmd;
		}
		case 1:		//center down hook
		{
			*px = m_centerDownVertices[m_nCurVertex].ux_coord;
			*py = m_centerDownVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);
            *py += m_rBarlineHalf;

			unsigned cmd = m_centerDownVertices[m_nCurVertex++].cmd;

			//change to next contour?
			if(m_nCurVertex == m_numVerticesCenterDown)
            {
				m_nContour++;
                m_nCurVertex = 0;
            }

            return cmd;
		}
		case 2:		//bottom hook
		{
			*px = m_bottomVertices[m_nCurVertex].ux_coord;
			*py = m_bottomVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);
            *py += m_rBarlineHeight;

			unsigned cmd = m_bottomVertices[m_nCurVertex++].cmd;

			//change to next contour?
			if(m_nCurVertex == m_numVerticesBottom)
            {
				m_nContour++;
                m_nCurVertex = 0;
            }

            return cmd;
		}
		case 3:		//center up hook
		{
		    if(m_nCurVertex >= m_numVerticesCenterUp)
			    return agg::path_cmd_stop;

			*px = m_centerUpVertices[m_nCurVertex].ux_coord;
			*py = m_centerUpVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

            *py += m_rBarlineHalf;

			return m_centerUpVertices[m_nCurVertex++].cmd;
		}
		default:
			return agg::path_cmd_stop;
	}
}

//---------------------------------------------------------------------------------------
void RawShapeCurlyBracket::draw(ScreenDrawer* pDrawer, LUnits xLeft, LUnits yTop,
                               LUnits xRight, LUnits yBottom, bool fOpen, Color color)
{
    set_position_bounds(xLeft, yTop, xRight, yBottom, fOpen);
    m_nCurVertex = 0;
    m_nContour = 0;

    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();
}


}  //namespace lomse
