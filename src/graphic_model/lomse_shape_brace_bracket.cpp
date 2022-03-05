//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_shape_brace_bracket.h"

#include "lomse_drawer.h"
using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
Vertex m_BraceVertices[] = {
    {   0.0, 2051.5, agg::path_cmd_move_to },
    {   0.0, 2036.5, agg::path_cmd_line_to },
    {  85.0, 1975.7, agg::path_cmd_curve3 },      //ctrol
    { 131.5, 1897.4, agg::path_cmd_curve3 },      //on-curve
    { 183.5, 1809.7, agg::path_cmd_curve3 },      //ctrol
    { 183.5, 1704.0, agg::path_cmd_curve3 },      //on-curve
    { 183.5, 1624.2, agg::path_cmd_curve3 },      //ctrol
    { 143.5, 1456.2, agg::path_cmd_curve3 },      //on-curve
    {  72.5, 1157.1, agg::path_cmd_curve3 },      //ctrol
    {  69.0, 1140.7, agg::path_cmd_curve3 },      //on-curve
    {  28.5,  943.7, agg::path_cmd_curve3 },      //ctrol
    {  28.5,  800.2, agg::path_cmd_curve3 },      //on-curve
    {  28.5,  683.0, agg::path_cmd_curve3 },      //ctrol
    {  54.5,  560.4, agg::path_cmd_curve3 },      //on-curve
    {  83.5,  424.8, agg::path_cmd_curve3 },      //ctrol
    { 142.5,  289.6, agg::path_cmd_curve3 },      //on-curve
    { 207.0,  142.1, agg::path_cmd_curve3 },      //ctrol
    { 307.0,    0.0, agg::path_cmd_curve3 },      //on-curve
    { 307.0,   15.0, agg::path_cmd_line_to },
    { 247.0,  126.1, agg::path_cmd_curve3 },      //ctrol
    { 213.0,  206.4, agg::path_cmd_curve3 },      //on-curve
    { 171.0,  305.6, agg::path_cmd_curve3 },      //ctrol
    { 151.0,  394.8, agg::path_cmd_curve3 },      //on-curve
    { 129.0,  493.6, agg::path_cmd_curve3 },      //ctrol
    { 129.0,  601.2, agg::path_cmd_curve3 },      //on-curve
    { 129.0,  729.4, agg::path_cmd_curve3 },      //ctrol
    { 175.0,  914.8, agg::path_cmd_curve3 },      //on-curve
    { 218.0, 1069.4, agg::path_cmd_curve3 },      //ctrol
    { 261.0, 1224.4, agg::path_cmd_curve3 },      //on-curve
    { 307.0, 1395.9, agg::path_cmd_curve3 },      //ctrol
    { 307.0, 1490.6, agg::path_cmd_curve3 },      //on-curve
    { 307.0, 1633.7, agg::path_cmd_curve3 },      //ctrol
    { 272.0, 1729.4, agg::path_cmd_curve3 },      //on-curve
    { 247.0, 1825.1, agg::path_cmd_curve3 },      //ctrol
    { 178.5, 1902.4, agg::path_cmd_curve3 },      //on-curve
    { 117.0, 1971.7, agg::path_cmd_curve3 },      //ctrol
    {  11.0, 2043.0, agg::path_cmd_curve3 },      //on-curve
//
    {   0.0, 2051.5, agg::path_cmd_move_to },
    {  85.0, 2112.3, agg::path_cmd_curve3 },      //ctrol
    { 131.5, 2190.6, agg::path_cmd_curve3 },      //on-curve
    { 183.5, 2278.3, agg::path_cmd_curve3 },      //ctrol
    { 183.5, 2383.0, agg::path_cmd_curve3 },      //on-curve
    { 183.5, 2463.8, agg::path_cmd_curve3 },      //ctrol
    { 143.5, 2631.8, agg::path_cmd_curve3 },      //on-curve
    {  72.5, 2930.9, agg::path_cmd_curve3 },      //ctrol
    {  69.0, 2947.3, agg::path_cmd_curve3 },      //on-curve
    {  28.5, 3144.3, agg::path_cmd_curve3 },      //ctrol
    {  28.5, 3287.8, agg::path_cmd_curve3 },      //on-curve
    {  28.5, 3405.0, agg::path_cmd_curve3 },      //ctrol
    {  54.5, 3527.6, agg::path_cmd_curve3 },      //on-curve
    {  83.5, 3663.2, agg::path_cmd_curve3 },      //ctrol
    { 142.5, 3798.4, agg::path_cmd_curve3 },      //on-curve
    { 207.0, 3945.9, agg::path_cmd_curve3 },      //ctrol
    { 307.0, 4088.0, agg::path_cmd_curve3 },      //on-curve
    { 307.0, 4073.0, agg::path_cmd_line_to },
    { 247.0, 3961.9, agg::path_cmd_curve3 },      //ctrol
    { 213.0, 3881.6, agg::path_cmd_curve3 },      //on-curve
    { 171.0, 3782.4, agg::path_cmd_curve3 },      //ctrol
    { 151.0, 3693.2, agg::path_cmd_curve3 },      //on-curve
    { 129.0, 3594.4, agg::path_cmd_curve3 },      //ctrol
    { 129.0, 3486.8, agg::path_cmd_curve3 },      //on-curve
    { 129.0, 3358.6, agg::path_cmd_curve3 },      //ctrol
    { 175.0, 3173.2, agg::path_cmd_curve3 },      //on-curve
    { 218.0, 3018.6, agg::path_cmd_curve3 },      //ctrol
    { 261.0, 2863.6, agg::path_cmd_curve3 },      //on-curve
    { 307.0, 2692.1, agg::path_cmd_curve3 },      //ctrol
    { 307.0, 2597.4, agg::path_cmd_curve3 },      //on-curve
    { 307.0, 2454.3, agg::path_cmd_curve3 },      //ctrol
    { 272.0, 2358.6, agg::path_cmd_curve3 },      //on-curve
    { 247.0, 2262.9, agg::path_cmd_curve3 },      //ctrol
    { 178.5, 2185.6, agg::path_cmd_curve3 },      //on-curve
    { 117.0, 2116.3, agg::path_cmd_curve3 },      //ctrol
    {  11.0, 2043.0, agg::path_cmd_curve3 },      //on_curve
    {   0.0,    0.0, agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw }, //close polygon
    {   0.0,    0.0, agg::path_cmd_stop }
};

float m_rxMaxBrace = 307.0f;
float m_ryMaxBrace = 4088.0f;
const int m_nNumVerticesBrace = sizeof(m_BraceVertices)/sizeof(Vertex);


Vertex m_BracketVertices[] = {
    {   0.0, 327.0, agg::path_cmd_move_to },
    { 273.0, 307.0, agg::path_cmd_curve3 },
    { 561.0, 196.0, agg::path_cmd_curve3 }, //on
    { 641.0, 151.0, agg::path_cmd_curve3 },
    { 696.0, 102.0, agg::path_cmd_curve3 },    //on-curve
    { 739.0,  64.0, agg::path_cmd_curve3 },
    { 791.0,   0.0, agg::path_cmd_curve3 },    //on-curve
    { 836.0,  16.0, agg::path_cmd_line_to },
    { 804.0,  83.0, agg::path_cmd_curve3 },
    { 760.0, 139.0, agg::path_cmd_curve3 },    //on-curve
    { 724.0, 185.0, agg::path_cmd_curve3 },
    { 655.0, 254.0, agg::path_cmd_curve3 },    //on-curve
    { 601.0, 306.0, agg::path_cmd_curve3 },
    { 551.0, 342.0, agg::path_cmd_curve3 },    //on-curve
    { 510.0, 372.0, agg::path_cmd_curve3 },
    { 443.0, 408.0, agg::path_cmd_curve3 },    //on-curve
    { 404.0, 430.0, agg::path_cmd_curve3 },
    { 307.0, 479.0, agg::path_cmd_curve3 },    //on-curve
};

Vertex m_BracketVertices2[] = {
    { 307.0,  685.0, agg::path_cmd_line_to },
    { 404.0,  734.0, agg::path_cmd_curve3 },
    { 443.0,  756.0, agg::path_cmd_curve3 },    //on-curve
    { 510.0,  792.0, agg::path_cmd_curve3 },
    { 551.0,  822.0, agg::path_cmd_curve3 },    //on-curve
    { 601.0,  858.0, agg::path_cmd_curve3 },
    { 655.0,  910.0, agg::path_cmd_curve3 },    //on-curve
    { 724.0,  979.0, agg::path_cmd_curve3 },
    { 760.0, 1025.0, agg::path_cmd_curve3 },    //on-curve
    { 804.0, 1081.0, agg::path_cmd_curve3 },
    { 836.0, 1148.0, agg::path_cmd_curve3 },    //on-curve
    { 791.0, 1164.0, agg::path_cmd_line_to },
    { 739.0, 1100.0, agg::path_cmd_curve3 },
    { 696.0, 1062.0, agg::path_cmd_curve3 },    //on-curve
    { 641.0, 1102.0, agg::path_cmd_curve3 },
    { 561.0,  968.0, agg::path_cmd_curve3 },    //on-curve
    { 273.0,  857.0, agg::path_cmd_curve3 },
    {   0.0,  885.0, agg::path_cmd_curve3 },    //on-curve
    {   0.0,    0.0, agg::path_cmd_end_poly | agg::path_flags_close | agg::path_flags_ccw }, //close polygon
    {   0.0,    0.0, agg::path_cmd_stop }
};


float m_dyBracket = 606.0f;
float m_dxBracketBar = 307.0f;
const int m_nNumVerticesBracket = sizeof(m_BracketVertices)/sizeof(Vertex);
const int m_nNumVerticesBracket2 = sizeof(m_BracketVertices2)/sizeof(Vertex);


//---------------------------------------------------------------------------------------
// Implementation of GmoShapeBracketBrace
//---------------------------------------------------------------------------------------
GmoShapeBracketBrace::GmoShapeBracketBrace(ImoObj* pCreatorImo, int type, ShapeId idx,
                                           Color color)
    : GmoSimpleShape(pCreatorImo, type, idx, color)
    , m_nCurVertex(0)
    , m_nContour(0)
{
}

//---------------------------------------------------------------------------------------
GmoShapeBracketBrace::~GmoShapeBracketBrace()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeBracketBrace::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    set_affine_transform();

    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();

    GmoSimpleShape::on_draw(pDrawer, opt);
}


//---------------------------------------------------------------------------------------
// Implementation of GmoShapeBrace
//---------------------------------------------------------------------------------------
GmoShapeBrace::GmoShapeBrace(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop,
                             LUnits xRight, LUnits yBottom, Color color)
    : GmoShapeBracketBrace(pCreatorImo, GmoObj::k_shape_brace, idx, color)
{
    set_origin(xLeft, yTop);
	set_width(xRight - xLeft);
	set_height(yBottom - yTop);
}

//---------------------------------------------------------------------------------------
GmoShapeBrace::~GmoShapeBrace()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeBrace::set_affine_transform()
{
    double rxScale((get_right() - get_left()) / m_rxMaxBrace);
    double ryScale((get_bottom() - get_top()) / m_ryMaxBrace);
    m_trans = agg::trans_affine(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    m_trans *= agg::trans_affine_scaling(rxScale, ryScale);
    m_trans *= agg::trans_affine_translation(get_left(), get_top());
}

//---------------------------------------------------------------------------------------
unsigned GmoShapeBrace::vertex(double* px, double* py)
{
	if(m_nCurVertex >= m_nNumVerticesBrace)
		return agg::path_cmd_stop;

	*px = m_BraceVertices[m_nCurVertex].ux_coord;
	*py = m_BraceVertices[m_nCurVertex].uy_coord;
	m_trans.transform(px, py);

	return m_BraceVertices[m_nCurVertex++].cmd;
}



//---------------------------------------------------------------------------------------
// Implementation of GmoShapeBracket
//---------------------------------------------------------------------------------------
GmoShapeBracket::GmoShapeBracket(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft,
                                 LUnits yTop, LUnits xRight, LUnits yBottom,
                                 LUnits dyHook, Color color)
    : GmoShapeBracketBrace(pCreatorImo, GmoObj::k_shape_bracket, idx, color)
{
    set_origin(xLeft, yTop - dyHook);
	set_width(xRight - xLeft);
	set_height(yBottom - yTop + dyHook + dyHook);

    m_udyHook = dyHook;
    m_rBracketBarHeight = (double)(yBottom - yTop);
}

//---------------------------------------------------------------------------------------
GmoShapeBracket::~GmoShapeBracket()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeBracket::set_affine_transform()
{
    double rxScale((get_right() - get_left()) / m_dxBracketBar);
    double ryScale(m_udyHook / m_dyBracket);
    m_trans = agg::trans_affine(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    m_trans *= agg::trans_affine_scaling(rxScale, ryScale);
    m_trans *= agg::trans_affine_translation(get_left(), get_top());
}

//---------------------------------------------------------------------------------------
unsigned GmoShapeBracket::vertex(double* px, double* py)
{
	switch(m_nContour)
	{
		case 0:		//top hook
		{
			*px = m_BracketVertices[m_nCurVertex].ux_coord;
			*py = m_BracketVertices[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

			unsigned cmd = m_BracketVertices[m_nCurVertex++].cmd;

			//change to rectangle contour?
			if(m_nCurVertex == m_nNumVerticesBracket)
            {
				m_nContour++;
                m_nCurVertex = 0;
            }

            return cmd;
		}
		case 1:		//bottom hook
		{
		    if(m_nCurVertex >= m_nNumVerticesBracket2)
			    return agg::path_cmd_stop;

			*px = m_BracketVertices2[m_nCurVertex].ux_coord;
			*py = m_BracketVertices2[m_nCurVertex].uy_coord;
			m_trans.transform(px, py);

            *py += m_rBracketBarHeight;

			return m_BracketVertices2[m_nCurVertex++].cmd;
		}
		default:
			return agg::path_cmd_stop;
	}
}



//---------------------------------------------------------------------------------------
// Implementation of GmoShapeSquaredBracket
//---------------------------------------------------------------------------------------
GmoShapeSquaredBracket::GmoShapeSquaredBracket(ImoObj* pCreatorImo, ShapeId idx,
                                               LUnits xLeft, LUnits yTop,
                                               LUnits xRight, LUnits yBottom,
                                               LUnits lineThickness, Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_squared_bracket, idx, color)
    , m_lineThickness(lineThickness)
{
    set_origin(xLeft, yTop);
	set_width(xRight - xLeft);
	set_height(yBottom - yTop);

}

//---------------------------------------------------------------------------------------
GmoShapeSquaredBracket::~GmoShapeSquaredBracket()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeSquaredBracket::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->stroke(color);
    pDrawer->fill(Color(0,0,0,0));  //transparent
    pDrawer->stroke_width(m_lineThickness);

    //top hook
    double yPos = m_origin.y;
    pDrawer->move_to(m_origin.x + m_size.width, yPos);
    pDrawer->hline_to(m_origin.x);

    //vertical line
    yPos += m_size.height;
    pDrawer->vline_to(yPos);

    //bottom hook
    pDrawer->hline_to(m_origin.x + m_size.width);

    pDrawer->end_path();
    pDrawer->render();

    GmoSimpleShape::on_draw(pDrawer, opt);
}


}  //namespace lomse
