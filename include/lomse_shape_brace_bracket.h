//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
class InstrumentEngraver;

//---------------------------------------------------------------------------------------
class GmoShapeBracketBrace : public GmoSimpleShape, public VertexSource
{
protected:
    int m_nCurVertex;               //index to current vertex
    int m_nContour;                 //current countour
    agg::trans_affine   m_trans;    //affine transformation to apply

public:
	virtual ~GmoShapeBracketBrace();

    void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //VertexSource
    void rewind(int UNUSED(pathId) = 0) { m_nCurVertex = 0; m_nContour = 0; }


protected:
    GmoShapeBracketBrace(ImoObj* pCreatorImo, int type, ShapeId idx, Color color);
    virtual void set_affine_transform() = 0;

};

//---------------------------------------------------------------------------------------
class GmoShapeBrace : public GmoShapeBracketBrace
{
    friend class InstrumentEngraver;
    friend class GroupEngraver;
    friend class FragmentMark;
    GmoShapeBrace(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop,
                    LUnits xRight, LUnits yBottom, Color color);

public:
	~GmoShapeBrace();

    //VertexSource
    unsigned vertex(double* px, double* py);

protected:
    void set_affine_transform();

};


//---------------------------------------------------------------------------------------
class GmoShapeBracket : public GmoShapeBracketBrace
{
protected:
    double m_rBracketBarHeight;
    LUnits m_udyHook;

    friend class InstrumentEngraver;
    friend class GroupEngraver;
    GmoShapeBracket(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop,
                  LUnits xRight, LUnits yBottom, LUnits dyHook, Color color);

public:
    ~GmoShapeBracket();

    //VertexSource
    unsigned vertex(double* px, double* py);

protected:
    void set_affine_transform();

};


//---------------------------------------------------------------------------------------
class GmoShapeSquaredBracket : public GmoSimpleShape
{
protected:
    LUnits m_lineThickness;

    friend class InstrumentEngraver;
    friend class GroupEngraver;
    GmoShapeSquaredBracket(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop,
                           LUnits xRight, LUnits yBottom, LUnits lineThickness,
                           Color color);

public:
    ~GmoShapeSquaredBracket();

	//implementation of pure virtual methods in base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt);

protected:

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_BRACE_BRACKET_H__

