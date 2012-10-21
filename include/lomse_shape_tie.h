//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

