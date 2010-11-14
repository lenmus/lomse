//-------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_BRACE_BRACKET_H__        //to avoid nested includes
#define __LOMSE_SHAPE_BRACE_BRACKET_H__

#include "lomse_shape_base.h"
#include "lomse_vertex_source.h"
#include "agg_trans_affine.h"
//#include <string>
//#include <list>
//#include <vector>
//using namespace std;

namespace lomse
{


//forward declarations
class ImoInstrument;
//class lmPaper;

//---------------------------------------------------------------------------------------
class GmoShapeBracketBrace : public GmoSimpleShape, public VertexSource
{
protected:
    //int m_nSymbol;
    int m_nCurVertex;               //index to current vertex
    int m_nContour;                 //current countour
    agg::trans_affine   m_trans;    //affine transformation to apply

public:
	virtual ~GmoShapeBracketBrace();

    void on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin);

    //VertexSource
    void rewind(int pathId = 0) { m_nCurVertex = 0; }


protected:
    GmoShapeBracketBrace(GmoBox* owner, int type);
    virtual void set_affine_transform() = 0;

};

//---------------------------------------------------------------------------------------
class GmoShapeBracket : public GmoShapeBracketBrace
{
public:
    GmoShapeBracket(GmoBox* owner, LUnits xLeft, LUnits yTop,
                 LUnits xRight, LUnits yBottom);   //, wxColour color = *wxBLACK);
	~GmoShapeBracket();

    //VertexSource
    unsigned vertex(double* px, double* py);

protected:
    void set_affine_transform();

};


//---------------------------------------------------------------------------------------
class GmoShapeBrace : public GmoShapeBracketBrace
{
protected:
    double m_rBraceBarHeight;
    LUnits m_udyHook;

public:
    GmoShapeBrace(GmoBox* owner, LUnits xLeft, LUnits yTop, LUnits xRight,
               LUnits yBottom, LUnits dyHook);    //, wxColour color = *wxBLACK);
	~GmoShapeBrace();

    //VertexSource
    virtual unsigned vertex(double* px, double* py) = 0;

protected:
    void set_affine_transform();

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_BRACE_BRACKET_H__

