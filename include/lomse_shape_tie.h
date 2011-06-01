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
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_TIE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_TIE_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_engraver.h"
#include "lomse_shape_base.h"
#include "lomse_shape_text.h"
#include "lomse_internal_model.h"

#include "lomse_shape_base.h"
#include "lomse_vertex_source.h"
#include "agg_trans_affine.h"

namespace lomse
{

//forward declarations
class ImoObj;


//---------------------------------------------------------------------------------------
class GmoShapeTie : public GmoSimpleShape, public VertexSource
{
protected:
    LUnits m_thickness;
    UPoint m_points[4];

    UPoint m_vertices[7];
    int m_nCurVertex;               //index to current vertex
    int m_nContour;                 //current countour

public:
    GmoShapeTie(ImoObj* pCreatorImo, int idx, UPoint points[], LUnits tickness,
                Color color = Color(0,0,0));
    ~GmoShapeTie();

    void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //VertexSource
    void rewind(int pathId = 0) { m_nCurVertex = 0; }
    unsigned vertex(double* px, double* py);

protected:
    void save_points(UPoint* points);
    void compute_vertices();
};

//---------------------------------------------------------------------------------------
class GmoShapeSlur : public GmoSimpleShape, public VertexSource
{
protected:
    LUnits m_thickness;
    UPoint m_points[4];

    UPoint m_vertices[7];
    int m_nCurVertex;               //index to current vertex
    int m_nContour;                 //current countour

public:
    GmoShapeSlur(ImoObj* pCreatorImo, int idx, UPoint points[], LUnits tickness,
                Color color = Color(0,0,0));
    ~GmoShapeSlur();

    void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //VertexSource
    void rewind(int pathId = 0) { m_nCurVertex = 0; }
    unsigned vertex(double* px, double* py);

protected:
    void save_points(UPoint* points);
    void compute_vertices();
};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_TIE_H__

