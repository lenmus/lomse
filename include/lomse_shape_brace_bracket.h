//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //VertexSource
    void rewind(unsigned UNUSED(pathId) = 0) override { m_nCurVertex = 0; m_nContour = 0; }


protected:
    GmoShapeBracketBrace(ImoObj* pCreatorImo, int type, ShapeId idx, Color color);
    virtual void set_affine_transform() = 0;

};

//---------------------------------------------------------------------------------------
class GmoShapeBrace : public GmoShapeBracketBrace
{
public:
    GmoShapeBrace(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop,
                    LUnits xRight, LUnits yBottom, Color color);

	~GmoShapeBrace();

    //VertexSource
    unsigned vertex(double* px, double* py) override;

protected:
    void set_affine_transform() override;

};


//---------------------------------------------------------------------------------------
class GmoShapeBracket : public GmoShapeBracketBrace
{
protected:
    double m_rBracketBarHeight;
    LUnits m_udyHook;

public:
    GmoShapeBracket(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop,
                  LUnits xRight, LUnits yBottom, LUnits dyHook, Color color);

    ~GmoShapeBracket();

    //VertexSource
    unsigned vertex(double* px, double* py) override;

protected:
    void set_affine_transform() override;

};


//---------------------------------------------------------------------------------------
class GmoShapeSquaredBracket : public GmoSimpleShape
{
protected:
    LUnits m_lineThickness;

public:
    GmoShapeSquaredBracket(ImoObj* pCreatorImo, ShapeId idx, LUnits xLeft, LUnits yTop,
                           LUnits xRight, LUnits yBottom, LUnits lineThickness,
                           Color color);

    ~GmoShapeSquaredBracket();

	//implementation of pure virtual methods in base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

protected:

};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_BRACE_BRACKET_H__

