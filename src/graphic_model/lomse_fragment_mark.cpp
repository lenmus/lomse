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
// Class RoundedBracketGlyph. Helper class to draw open/close rounded brackets
//=======================================================================================
class RoundedBracketGlyph : public VertexSource
{
protected:
    int                 m_nCurVertex;   //index to current vertex
    int                 m_nContour;     //current countour
    agg::trans_affine   m_trans;        //affine transformation to apply

    double              m_rBracketBarHeight;

    static Vertex m_topHookVertices[];
    static Vertex m_bottomHookVertices[];
    static float m_dxRoundedBracketBar;
    static float m_hooksHeight;

    int m_nNumVerticesRoundedBracket;
    int m_nNumVerticesRoundedBracket2;


public:
    RoundedBracketGlyph();

    void draw(ScreenDrawer* pDrawer, LUnits xLeft, LUnits yTop,
              LUnits xRight, LUnits yBottom, bool fOpen, Color color = Color(0,0,0));

    //URect get_bounds();

protected:
    void set_affine_transform(LUnits xLeft, LUnits yTop, LUnits xRight, LUnits yBottom, bool fOpen);

    //VertexSource
    unsigned vertex(double* px, double* py) override;
    void rewind(int UNUSED(pathId) = 0) override { m_nCurVertex = 0; m_nContour = 0; }
};


//=======================================================================================
// FragmentMark implementation
//=======================================================================================
FragmentMark::FragmentMark(GraphicView* view, LibraryScope& libraryScope)
    : VisualEffect(view, libraryScope)
    , m_type(k_mark_line)
    , m_color(Color(255,0,0,128))   // transparent red
    , m_lineStyle(k_line_solid)
    , m_pBoxSystem(nullptr)
    , m_iPage(0)
    , m_yTop(1000.0)        //arbitrary, to have a valid value
    , m_yBottom(2000.0)     //arbitrary, to have a valid value
    , m_extension(300.0)    //3.0 mm
{
    m_bounds.width = 100.0;     // 1.00 mm
}

//---------------------------------------------------------------------------------------
void FragmentMark::move_to(LUnits xPos, GmoBoxSystem* pBoxSystem)
{
    //This method is protected. It is only invoked one time, when the mark is created.

    LUnits xLeft = xPos;

    if (pBoxSystem)
    {
        m_pBoxSystem = pBoxSystem;
        m_iPage = pBoxSystem->get_page_number();

        top(0,0);       //top point in first instrument, first staff
        bottom(-1,-1);  //bottom point at last instrument, last staff

        //determine extensiton to reach system bounds
        LUnits height = m_yBottom - m_yTop;
        m_extension = (pBoxSystem->get_height() - height) / 2.0;

        //fix xLeft with style margins
        ImoStyle* pStyle = pBoxSystem->get_style();
        if (pStyle)
            xLeft += pStyle->margin_left();
    }

    m_bounds.left(xLeft);
}

//---------------------------------------------------------------------------------------
FragmentMark* const FragmentMark::top(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0;

    //set top point
    URect staffBounds = pStaff->get_bounds();
    m_yTop = staffBounds.y;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yTop -= double(pStyle->margin_top());

    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* const FragmentMark::bottom(int instr, int staff)
{
    if (!m_pBoxSystem)
        return this;

    GmoShapeStaff* pStaff = m_pBoxSystem->get_staff_shape(instr, staff);
    if (!pStaff)
        return this;

    m_extension = pStaff->get_staff_margin() / 2.0;

    //set bottom point
    URect staffBounds = pStaff->get_bounds();
    m_yBottom = staffBounds.y + staffBounds.height;
    ImoStyle* pStyle = m_pBoxSystem->get_style();
    if (pStyle)
        m_yBottom -= double(pStyle->margin_top());

    return this;
}

//---------------------------------------------------------------------------------------
FragmentMark* const FragmentMark::x_shift(LUnits dx)
{
    m_bounds.left( m_bounds.left() + dx );
    return this;
}

//---------------------------------------------------------------------------------------
void FragmentMark::on_draw(ScreenDrawer* pDrawer)
{
    LUnits yTop = m_yTop - m_extension;
    LUnits yBottom = m_yBottom + m_extension;
    LUnits hookLength = 200.0;

    if (m_type == k_mark_open_rounded || m_type == k_mark_close_rounded)
    {
        RoundedBracketGlyph glyph;
        glyph.draw(pDrawer, m_bounds.left(), yTop, m_bounds.left() + m_bounds.width,
                   yBottom, (m_type == k_mark_open_rounded), m_color);
    }
    else
    {
        pDrawer->begin_path();
        pDrawer->fill(Color(0,0,0,0));  //transparent
        pDrawer->stroke(m_color);
        pDrawer->stroke_width(m_bounds.width);

        UPoint org = m_pView->get_page_origin_for(m_iPage);
        pDrawer->set_shift(-org.x, -org.y);

        if (m_type == k_mark_open_bracket)
        {
            //top hook
            pDrawer->move_to(m_bounds.left() + hookLength, yTop);
            pDrawer->hline_to(m_bounds.left());

            //vertical line
            pDrawer->vline_to(yBottom);

            //bottom hook
            pDrawer->hline_to(m_bounds.left() + hookLength);
        }
        else if (m_type == k_mark_close_bracket)
        {
            //top hook
            pDrawer->move_to(m_bounds.left() - hookLength, yTop);
            pDrawer->hline_to(m_bounds.left());

            //vertical line
            pDrawer->vline_to(yBottom);

            //bottom hook
            pDrawer->hline_to(m_bounds.left() - hookLength);
        }
        else
        {
            //vertical line
            pDrawer->move_to(m_bounds.left(), yTop);
            pDrawer->vline_to(yBottom);
        }

        pDrawer->end_path();
    }
    pDrawer->render();
    pDrawer->remove_shift();

    m_bounds.top(yTop);
    m_bounds.bottom(yBottom);
}

//---------------------------------------------------------------------------------------
URect FragmentMark::get_bounds()
{
    return m_bounds;
}


//=======================================================================================
// RoundedBracketGlyph implementation
//=======================================================================================

//top hook
Vertex RoundedBracketGlyph::m_topHookVertices[] = {
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
Vertex RoundedBracketGlyph::m_bottomHookVertices[] = {
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

float RoundedBracketGlyph::m_dxRoundedBracketBar = 81.0f;   //width of the vertical line (105 - 24)
float RoundedBracketGlyph::m_hooksHeight = 330.0;    //2 * height of one hook = 2*(547-712) = 2*165 = 330

//---------------------------------------------------------------------------------------
RoundedBracketGlyph::RoundedBracketGlyph()
{
    m_nNumVerticesRoundedBracket = sizeof(m_topHookVertices)/sizeof(Vertex);
    m_nNumVerticesRoundedBracket2 = sizeof(m_bottomHookVertices)/sizeof(Vertex);
}


//---------------------------------------------------------------------------------------
void RoundedBracketGlyph::set_affine_transform(LUnits xLeft, LUnits yTop,
                                               LUnits xRight, LUnits yBottom, bool fOpen)
{
    m_rBracketBarHeight = (double)(yBottom - yTop);

    double rxScale((xRight - xLeft) / m_dxRoundedBracketBar);
    double ryScale = rxScale;
    if (!fOpen)
        rxScale = -rxScale;

    m_trans = agg::trans_affine(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    m_trans *= agg::trans_affine_scaling(rxScale, ryScale);
    m_trans *= agg::trans_affine_translation(xLeft, yTop);
}

//---------------------------------------------------------------------------------------
unsigned RoundedBracketGlyph::vertex(double* px, double* py)
{
	switch(m_nContour)
	{
		case 0:		//top hook
		{
			*px = m_topHookVertices[m_nCurVertex].ux_coord;
			*py = m_topHookVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

			unsigned cmd = m_topHookVertices[m_nCurVertex++].cmd;

			//change to rectangle contour?
			if(m_nCurVertex == m_nNumVerticesRoundedBracket)
            {
				m_nContour++;
                m_nCurVertex = 0;
            }

            return cmd;
		}
		case 1:		//bottom hook
		{
		    if(m_nCurVertex >= m_nNumVerticesRoundedBracket2)
			    return agg::path_cmd_stop;

			*px = m_bottomHookVertices[m_nCurVertex].ux_coord;
			*py = m_bottomHookVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

            *py += m_rBracketBarHeight;

			return m_bottomHookVertices[m_nCurVertex++].cmd;
		}
		default:
			return agg::path_cmd_stop;
	}
}

//---------------------------------------------------------------------------------------
void RoundedBracketGlyph::draw(ScreenDrawer* pDrawer, LUnits xLeft, LUnits yTop,
                               LUnits xRight, LUnits yBottom, bool fOpen, Color color)
{
    set_affine_transform(xLeft, yTop, xRight, yBottom, fOpen);
    m_nCurVertex = 0;
    m_nContour = 0;

    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();
}


}  //namespace lomse
