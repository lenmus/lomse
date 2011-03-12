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
class GmoShapeArch : public GmoSimpleShape
{
protected:
//    bool m_fArchUnder;                   //arch under notes (like a 'U')
#define LOMSE_BEZIER_POINTS   4             //num points defining a bezier arch
    UPoint m_uPoint[LOMSE_BEZIER_POINTS];   //start, end, ctrol1, ctrol2
//    UPoint m_uSavePoint[LOMSE_BEZIER_POINTS];     //to save start, end and control points when dragging/moving
//    lmHandlerSquare* m_pHandler[LOMSE_BEZIER_POINTS];       //handlers

public:
    GmoShapeArch(ImoObj* pCreatorImo, int idx, UPoint uStart, UPoint uEnd,
                 UPoint uCtrol1, UPoint uCtrol2, Color color);
//    GmoShapeArch(ImoObj* pCreatorImo, int idx, UPoint uStart, UPoint uEnd,
//                 bool fArchUnder, Color color);
    GmoShapeArch(ImoObj* pCreatorImo, int idx, bool fArchUnder,
                 Color color);

    virtual ~GmoShapeArch();

    //implementation of virtual methods from base class
    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);
//    virtual void RenderWithHandlers(lmPaper* pPaper);
//    virtual void Shift(LUnits xIncr, LUnits yIncr);
//
//    //creation
//    void SetStartPoint(LUnits xPos, LUnits yPos);
//    void SetEndPoint(LUnits xPos, LUnits yPos);
//    void SetCtrolPoint1(LUnits xPos, LUnits yPos);
//    void SetCtrolPoint2(LUnits xPos, LUnits yPos);
//
//    //shape dragging
//    wxBitmap* OnBeginDrag(double rScale, wxDC* pDC);
//	UPoint OnDrag(lmPaper* pPaper, const UPoint& uPos);
//	void OnEndDrag(lmPaper* pPaper, lmController* pCanvas, const UPoint& uPos);
//
//    //handlers dragging
//    UPoint OnHandlerDrag(lmPaper* pPaper, const UPoint& uPos, long nHandlerID);
//    void OnHandlerEndDrag(lmController* pCanvas, const UPoint& uPos, long nHandlerID);
//
//    //access to information
//    inline LUnits GetStartPosX() const { return m_uPoint[lmBEZIER_START].x; }
//    inline LUnits GetStartPosY() const { return m_uPoint[lmBEZIER_START].y; }
//    inline LUnits GetEndPosX() const { return m_uPoint[lmBEZIER_END].x; }
//    inline LUnits GetEndPosY() const { return m_uPoint[lmBEZIER_END].y; }
//    inline LUnits GetCtrol1PosX() const { return m_uPoint[lmBEZIER_CTROL1].x; }
//    inline LUnits GetCtrol1PosY() const { return m_uPoint[lmBEZIER_CTROL1].y; }
//    inline LUnits GetCtrol2PosX() const { return m_uPoint[lmBEZIER_CTROL2].x; }
//    inline LUnits GetCtrol2PosY() const { return m_uPoint[lmBEZIER_CTROL2].y; }
//
//    //call backs
//    void MovePoints(int nNumPoints, int nShapeIdx, UPoint* pShifts, bool fAddShifts);

protected:
    void initialize();
    void draw(Drawer* pDrawer, Color color);
//    void SetDefaultControlPoints();

};


//---------------------------------------------------------------------------------------
//class GmoShapeSlurTie : public GmoSimpleShape, public VertexSource
//{
//protected:
//    int m_nCurVertex;               //index to current vertex
//    int m_nContour;                 //current countour
//    agg::trans_affine   m_trans;    //affine transformation to apply
//
//public:
//    GmoShapeTie(ImoObj* pCreatorImo, int type, int idx, Color color);
//	virtual ~GmoShapeTie();
//
//    void on_draw(Drawer* pDrawer, RenderOptions& opt);
//
//    //VertexSource
//    void rewind(int pathId = 0) { m_nCurVertex = 0; }
//
//
//protected:
//    virtual void set_affine_transform() = 0;
//
//};

class GmoShapeTie : public GmoSimpleShape, public VertexSource      //public GmoShapeArch
{
protected:
    GmoShapeArch* m_pFirstArch;
    GmoShapeArch* m_pSecondArch;
    LUnits m_thickness;
    UPoint m_points[4];

    UPoint m_vertices[7];
    int m_nCurVertex;               //index to current vertex
    int m_nContour;                 //current countour
    agg::trans_affine   m_trans;    //affine transformation to apply

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
    void set_affine_transform();
};
//class GmoShapeTie : public GmoShapeArch
//{
//public:
//    GmoShapeTie(ImoTie* pOwner, int nShapeIdx, lmNote* pEndNote, UPoint* pPoints,
//               GmoShapeNote* pShapeStart, GmoShapeNote* pShapeEnd,
//               bool fTieUnderNote, Color color = *wxBLACK, bool fVisible = true);
//	~GmoShapeTie();
//
//	//implementation of virtual methods in base class
//    void Render(lmPaper* pPaper, Color color);
//    void DrawControlPoints(lmPaper* pPaper);
//
//    //overrides
//    void Shift(LUnits xIncr, LUnits yIncr) {}       //any shift is taken into account in method OnAttachmentPointMoved()
//
//	//layout changes
//	void OnAttachmentPointMoved(GmoShape* pShape, lmEAttachType nTag,
//								LUnits ux, LUnits uy, lmEParentEvent nEvent);
//    void ApplyUserShifts();
//
//	//splitting
//	void SetBrotherTie(GmoShapeTie* pBrotherTie) { m_pBrotherTie = pBrotherTie; }
//
//    //access to information
//    lmNote* GetStartNote();
//    lmNote* GetEndNote();
//
//
//private:
//    bool			m_fTieUnderNote;
//    bool            m_fUserShiftsApplied;
//	GmoShapeTie*		m_pBrotherTie;		    //when tie is splitted
//    lmNote*         m_pEndNote;
//    UPoint        m_uUserShifts[4];
//
//};



}   //namespace lomse

#endif    // __LOMSE_SHAPE_TIE_H__

