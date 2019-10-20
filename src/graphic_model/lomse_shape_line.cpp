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

#include "lomse_shape_line.h"

//#include "lomse_internal_model.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"

namespace lomse
{


//=======================================================================================
// GmoShapeLine object implementation
//=======================================================================================
GmoShapeLine::GmoShapeLine(ImoObj* pCreatorImo, ShapeId idx,
                           LUnits xStart, LUnits yStart,
                           LUnits xEnd, LUnits yEnd, LUnits uWidth,
                           LUnits uBoundsExtraWidth, ELineStyle nStyle,
                           Color color, ELineEdge nEdge,
                           ELineCap nStartCap, ELineCap nEndCap)
	: GmoSimpleShape(pCreatorImo, GmoObj::k_shape_line, idx, color)
    , m_uWidth(uWidth)
	, m_uBoundsExtraWidth(uBoundsExtraWidth)
    , m_nStyle(nStyle)
	, m_nEdge(nEdge)
    , m_nStartCap(nStartCap)
    , m_nEndCap(nEndCap)
{
    //origin
    m_origin.x = xStart;
    m_origin.y = yStart;

    //points (relative to origin)
    m_uPoint[k_start].x = 0.0;
    m_uPoint[k_start].y = 0.0;
    m_uPoint[k_end].x = xEnd - xStart;
    m_uPoint[k_end].y = yEnd - yStart;

    // bounding rectangle size
	//TODO: For now it is assumed that the line is either vertical or horizontal
	if (xStart == xEnd)
	{
		//vertical line
        m_size.width = uWidth + uBoundsExtraWidth;
        m_size.height = yEnd - yStart + uBoundsExtraWidth;
	}
	else
	{
		//Horizontal line
		m_size.width = xEnd - xStart + uBoundsExtraWidth;
        m_size.height = uWidth + uBoundsExtraWidth;
	}
}

//---------------------------------------------------------------------------------------
GmoShapeLine::~GmoShapeLine()
{
}

//---------------------------------------------------------------------------------------
void GmoShapeLine::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    UPoint start = m_uPoint[k_start] + m_origin;
    UPoint end = m_uPoint[k_end] + m_origin;

    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->stroke(color);
    pDrawer->stroke_width(m_uWidth);
    pDrawer->line_with_markers(start, end, m_uWidth, m_nStartCap, m_nEndCap);
    pDrawer->end_path();

    GmoSimpleShape::on_draw(pDrawer, opt);
}

////---------------------------------------------------------------------------------------
//bool GmoShapeLine::HitTest(UPoint& uPoint)
//{
//    //point is not within the limits of this object selection rectangle
//    if (!GetSelRectangle().Contains(uPoint))
//        return false;
//
//    //Find the distance from point to line
//    LUnits uDistance = GetDistanceToLine(uPoint);
//
//    LUnits uTolerance = m_uBoundsExtraWidth;
//
//    //return true if click point is within tolerance margin
//    return (uDistance >= -uTolerance && uDistance <= uTolerance);
//}
//
////---------------------------------------------------------------------------------------
////Some vector computations
////---------------------------------------------------------------------------------------
//
//LUnits GmoShapeLine::GetDistanceToLine(UPoint uPoint)
//{
//    //define line vector
//    UVector uLineVector;
//    uLineVector.x = m_uPoint[k_end].x - m_uPoint[k_start].x;
//    uLineVector.y = m_uPoint[k_end].y - m_uPoint[k_start].y;
//
//    //vector for line through the point
//    UVector uPointVector;
//    uPointVector.x = uPoint.x - m_uPoint[k_start].x;
//    uPointVector.y = uPoint.y - m_uPoint[k_start].y;
//
//    //get its projection on the line.
//    //  uProjectionVector = uLineVector * factor;
//    //  factor = (uPointVector * uLineVector) / (|uLineVector|^2);
//    UVector uProjectionVector;
//    LUnits uFactor = VectorDotProduct(uPointVector, uLineVector) / VectorDotProduct(uLineVector, uLineVector);
//    uProjectionVector.x = uLineVector.x * uFactor;
//    uProjectionVector.y = uLineVector.y * uFactor;
//
//    //get normal vector: uNormalVector = uPointVector - uProjectionVector
//    UVector uNormalVector;
//    SubtractVectors(uPointVector, uProjectionVector, uNormalVector);
//
//    //return its magnitude
//    return VectorMagnitude(uNormalVector);
//}
//
////---------------------------------------------------------------------------------------
//LUnits GmoShapeLine::VectorDotProduct(UVector& v0, UVector& v1)
//{
//    return v0.x * v1.x + v0.y * v1.y;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeLine::SubtractVectors(UVector& v0, UVector& v1, UVector& v)
//{
//    v.x = v0.x - v1.x;
//    v.y = v0.y - v1.y;
//}
//
////---------------------------------------------------------------------------------------
//LUnits GmoShapeLine::VectorMagnitude(UVector& v)
//{
//    return sqrt(v.x * v.x + v.y * v.y);
//}



////=======================================================================================
////class GmoShapeFBLine implementation
////=======================================================================================
//
//GmoShapeFBLine::GmoShapeFBLine(lmScoreObj* pOwner, ShapeId idx,
//                             lmFiguredBass* pEndFB,
//                             LUnits uxStart, LUnits uyStart,    //user shift
//                             LUnits uxEnd, LUnits uyEnd,        //user shift
//                             lmTenths tWidth,
//                             lmShapeFiguredBass* pShapeStartFB,
//                             lmShapeFiguredBass* pShapeEndFB,
//                             Color nColor, bool fVisible)
//    : GmoShapeLine(pOwner, idx, uxStart, uyStart, uxEnd, uyEnd,
//                  pOwner->TenthsToLogical(tWidth),
//                  pOwner->TenthsToLogical(tWidth + 1.0f),       //BoundsExtraWidth
//                  lm_eLine_Solid, nColor, lm_eEdgeNormal, lmDRAGGABLE,
//                  lmSELECTABLE, fVisible, _T("FB Line"))
//    , m_pEndFB(pEndFB)
//    , m_pBrotherLine((GmoShapeFBLine*)nullptr)
//{
//    m_nType = eGMO_ShapeFBLine;
//
//    //save user shifts
//    m_uUserShifts[0] = UPoint(uxStart, uyStart);
//    m_uUserShifts[1] = UPoint(uxEnd, uyEnd);
//
//    //compute the default line
//    OnAttachmentPointMoved(pShapeStartFB, lm_eGMA_StartObj, 0.0, 0.0, lmSHIFT_EVENT);
//    OnAttachmentPointMoved(pShapeEndFB, lm_eGMA_EndObj, 0.0, 0.0, lmSHIFT_EVENT);
//    m_fUserShiftsApplied = false;
//}
//
//GmoShapeFBLine::~GmoShapeFBLine()
//{
//}
//
//void GmoShapeFBLine::OnAttachmentPointMoved(lmShape* pSFB, lmEAttachType nTag,
//								           LUnits uxShift, LUnits uyShift,
//                                           lmEParentEvent nEvent)
//{
//    //start or end figured bass object moved. Recompute start/end of line and,
//    //if necessary, split the line
//
//	WXUNUSED(uxShift);
//	WXUNUSED(uyShift);
//	WXUNUSED(nEvent);
//
//	//Compute new attachment point and update line start/end point. FB line is
//    //placed 20 tenths appart from the FB number, and 10 tenths down
//    UPoint uPos;
//    uPos.y = pSFB->GetYTop() + ((lmStaffObj*)m_pOwner)->TenthsToLogical(10.0);
//	if (nTag == lm_eGMA_StartObj)
//    {
//        uPos.x = pSFB->GetXRight() + ((lmStaffObj*)m_pOwner)->TenthsToLogical(20.0);
//        SetStartPoint(uPos);
//    }
//	else if (nTag == lm_eGMA_EndObj)
//    {
//        uPos.x = pSFB->GetXLeft() - ((lmStaffObj*)m_pOwner)->TenthsToLogical(20.0);
//        SetEndPoint(uPos);
//    }
//
//    // check if the line have to be splitted
//	if (!m_pBrotherLine) return;		//creating the line. No information yet
//
//    UPoint paperPosEnd = GetEndFB()->GetReferencePaperPos();
//    UPoint paperPosStart = m_pBrotherLine->GetEndFB()->GetReferencePaperPos();
//    if (paperPosEnd.y != paperPosStart.y)
//	{
//        //if start FB paperPos Y is not the same than end FB paperPos Y the
//		//FBs are in different systems. Therefore, the line must be splitted.
//		//To do it:
//		//	- detach the two intermediate points.
//		//	- make both shapes visible.
//		//
//		// As there is no controller object to perform these actions, the first line
//		// detecting the need must co-ordinate the necessary actions.
//
//		//determine which line is the first one
//		GmoShapeFBLine* pFirstLine = this;		//assume this is the first one
//		GmoShapeFBLine* pSecondLine = m_pBrotherLine;
//		if (paperPosStart.y > paperPosEnd.y)
//		{
//			//wrong assumption. Reverse asignment
//			pFirstLine = m_pBrotherLine;
//			pSecondLine = this;
//		}
//
//        //first line end point is right paper margin
//		lmBoxSystem* pSystem = this->GetOwnerSystem();
//		UPoint uEnd;
//		uEnd.x = pSystem->GetSystemFinalX();
//		uEnd.y = pFirstLine->GetStartPosY();
//		pFirstLine->SetEndPoint(uEnd);
//		pFirstLine->SetVisible(true);
//
//		//second line start point is begining of system
//		UPoint uStart;
//		uStart.x = pSystem->GetPositionX();
//		uStart.y = pSecondLine->GetEndPosY();
//		pSecondLine->SetStartPoint(uStart);
//		pSecondLine->SetVisible(true);
//	}
//}
//
////lmFiguredBass* GmoShapeFBLine::GetStartFB()
////{
////    //the owner of a FB line is always the end note. Therefore, to get the start
////    //FB let's access the end FB
////    return m_pEndFB->GetTiedNotePrev();
////}
//
//lmFiguredBass* GmoShapeFBLine::GetEndFB()
//{
//    //the owner of a FB line is always the end note
//    return m_pEndFB;
//}


}  //namespace lomse
