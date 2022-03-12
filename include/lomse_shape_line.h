//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SHAPE_LINE_H__        //to avoid nested includes
#define __LOMSE_SHAPE_LINE_H__

#include "lomse_shape_base.h"
#include "lomse_injectors.h"
#include "lomse_shapes.h"

namespace lomse
{

//forward declarations
class ImoObj;


//---------------------------------------------------------------------------------------
class ArrowHead
{
private:
    LUnits m_uHeight;
    LUnits m_uWidth;

public:
    ArrowHead(LUnits uHeight = 0.0f, LUnits uWidth = 0.0f);
    ~ArrowHead();

};

//---------------------------------------------------------------------------------------
class CircleHead
{
private:
    LUnits m_uRadius;
    LUnits m_uLineWidth;

public:
    CircleHead(LUnits uRadius = 0.0f, LUnits uLineWidth = 0.0f);
    ~CircleHead();

};

//---------------------------------------------------------------------------------------
class GmoShapeLine : public GmoSimpleShape
{
protected:
    LUnits      m_uWidth;
	LUnits      m_uBoundsExtraWidth;
    ELineStyle  m_nStyle;
	ELineEdge   m_nEdge;
    ELineCap    m_nStartCap;
    ELineCap    m_nEndCap;

    enum { k_start=0, k_end };
    UPoint m_uPoint[2];

    friend class LineEngraver;
    friend class LyricEngraver;
    friend class OctaveShiftEngraver;
    GmoShapeLine(ImoObj* pCreatorImo, ShapeId idx,
                 LUnits xStart, LUnits yStart, LUnits xEnd, LUnits yEnd,
                 LUnits uWidth, LUnits uBoundsExtraWidth, ELineStyle nStyle,
                 Color color, ELineEdge nEdge, ELineCap nStartCap, ELineCap nEndCap);

public:
    virtual ~GmoShapeLine();

    //properties and options
    inline void set_head_type(ELineCap nHeadType) { m_nStartCap = nHeadType; }
    inline void set_tail_type(ELineCap nHeadType) { m_nEndCap = nHeadType; }

    //implementation of virtual methods from base class
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //overrides
    bool HitTest(UPoint& uPoint);


protected:
    LUnits GetDistanceToLine(UPoint uPoint);


    //some vector algebra

    typedef struct UVectorStruct
    {
        LUnits x;
        LUnits y;
    }
    UVector;

    LUnits VectorDotProduct(UVector& v0, UVector& v1);
    void SubtractVectors(UVector& v0, UVector& v1, UVector& v);
    LUnits VectorMagnitude(UVector& v);

};

//---------------------------------------------------------------------------------------
class GmoShapeGraceStroke : public GmoShapeLine, public VoiceRelatedShape
{
protected:
    friend class NoteEngraver;
    friend class StemFlagEngraver;
    friend class BeamEngraver;
    GmoShapeGraceStroke(ImoObj* pCreatorImo, LUnits xStart, LUnits yStart, LUnits xEnd,
                        LUnits yEnd, LUnits uWidth, Color color)
        : GmoShapeLine(pCreatorImo, 0, xStart, yStart, xEnd, yEnd, uWidth, 0.0f,
                       k_line_solid, color, k_edge_normal, k_cap_none, k_cap_none)
        , VoiceRelatedShape()
    {
    }

public:
};

////---------------------------------------------------------------------------------------
//
//class GmoShapeFBLine : public GmoShapeLine
//{
//public:
//    GmoShapeFBLine(lmScoreObj* pOwner, ShapeId idx, lmFiguredBass* pEndFB,
//                  LUnits uxStart, LUnits uyStart, LUnits uxEnd, LUnits uyEnd,
//                  lmTenths tWidth, lmShapeFiguredBass* pShapeStartFB,
//                  lmShapeFiguredBass* pShapeEndFB, Color nColor, bool fVisible);
//    ~GmoShapeFBLine();
//
//	//layout changes
//	void OnAttachmentPointMoved(lmShape* pShape, lmEAttachType nTag,
//								LUnits ux, LUnits uy, lmEParentEvent nEvent);
//	//splitting
//	inline void SetBrotherLine(GmoShapeFBLine* pBrotherLine)
//                    { m_pBrotherLine = pBrotherLine; }
//
//    //access to information
//    //lmFiguredBass* GetStartFB();
//    lmFiguredBass* GetEndFB();
//
//    //changing values
//    inline void SetStartPoint(UPoint uPos) { m_uPoint[lmID_START] = uPos; }
//    inline void SetEndPoint(UPoint uPos) { m_uPoint[lmID_END] = uPos; }
//
//    //access to information
//    inline LUnits GetStartPosX() const { return m_uPoint[lmID_START].x; }
//    inline LUnits GetStartPosY() const { return m_uPoint[lmID_START].y; }
//    inline LUnits GetEndPosX() const { return m_uPoint[lmID_END].x; }
//    inline LUnits GetEndPosY() const { return m_uPoint[lmID_END].y; }
//
//private:
//	GmoShapeFBLine*  m_pBrotherLine;		    //when line is splitted
//    lmFiguredBass*  m_pEndFB;
//    bool            m_fUserShiftsApplied;
//    UPoint        m_uUserShifts[4];
//
//};


}   //namespace lomse

#endif    // __LOMSE_SHAPE_LINE_H__
