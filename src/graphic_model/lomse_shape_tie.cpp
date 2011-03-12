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

#include "lomse_shape_tie.h"

#include "lomse_score_meter.h"
#include "lomse_engraving_options.h"
#include "lomse_gm_basic.h"
#include "lomse_drawer.h"
#include "lomse_shape_note.h"


namespace lomse
{

//some macros to improve code reading
#define START       ImoBezierInfo::k_start
#define END         ImoBezierInfo::k_end
#define CTROL1      ImoBezierInfo::k_ctrol1
#define CTROL2      ImoBezierInfo::k_ctrol2


//=======================================================================================
// GmoShapeArch implementation
//=======================================================================================
//GmoShapeArch::GmoShapeArch(ImoObj* pCreatorImo, int idx, UPoint uStart, UPoint uEnd,
//                         bool fArchUnder, Color nColor)
//    : GmoSimpleShape(GmoObj::k_shape_arch, pOwner, idx, sName, fDraggable, lmSELECTABLE, nColor,
//                    fVisible)
//      , m_fArchUnder(fArchUnder)
//{
//    m_uPoint[ImoBezierInfo::k_start] = uStart;
//    m_uPoint[ImoBezierInfo::k_end] = uEnd;
//    m_uPoint[ImoBezierInfo::k_ctrol1] = UPoint(0.0, 0.0);
//    m_uPoint[ImoBezierInfo::k_ctrol2] = UPoint(0.0, 0.0);
//
//    m_color = nColor;
//    SetDefaultControlPoints();
//    initialize();
//}

//---------------------------------------------------------------------------------------
GmoShapeArch::GmoShapeArch(ImoObj* pCreatorImo, int idx, UPoint uStart, UPoint uEnd,
                           UPoint uCtrol1, UPoint uCtrol2, Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_arch, idx, color)
      //, m_fArchUnder(uStart.y < uCtrol1.y)
{
    m_uPoint[ImoBezierInfo::k_start] = uStart;
    m_uPoint[ImoBezierInfo::k_end] = uEnd;
    m_uPoint[ImoBezierInfo::k_ctrol1] = uCtrol1;
    m_uPoint[ImoBezierInfo::k_ctrol2] = uCtrol2;

    initialize();
}

//---------------------------------------------------------------------------------------
GmoShapeArch::GmoShapeArch(ImoObj* pCreatorImo, int idx, bool fArchUnder,
                           Color color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_arch, idx, color)
    //, m_fArchUnder(fArchUnder)
{
    initialize();
}

//---------------------------------------------------------------------------------------
void GmoShapeArch::initialize()
{
//    //Create handlers
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//        m_pHandler[i] = new lmHandlerSquare(m_pOwner, this, i);
}

//---------------------------------------------------------------------------------------
GmoShapeArch::~GmoShapeArch()
{
//    //delete handlers
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//        if (m_pHandler[i]) delete m_pHandler[i];
}

////---------------------------------------------------------------------------------------
//void GmoShapeArch::Shift(LUnits xIncr, LUnits yIncr)
//{
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//    {
//        m_uPoint[i].x += xIncr;
//        m_uPoint[i].y += yIncr;
//    }
//
//    ShiftBoundsAndSelRec(xIncr, yIncr);
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((lmCompositeShape*)GetParentShape())->RecomputeBounds();
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::SetStartPoint(LUnits xPos, LUnits yPos)
//{
//    m_uPoint[ImoBezierInfo::k_start].x = xPos;
//    m_uPoint[ImoBezierInfo::k_start].y = yPos;
//    SetDefaultControlPoints();
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::SetEndPoint(LUnits xPos, LUnits yPos)
//{
//    m_uPoint[ImoBezierInfo::k_end].x = xPos;
//    m_uPoint[ImoBezierInfo::k_end].y = yPos;
//    SetDefaultControlPoints();
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::SetCtrolPoint1(LUnits xPos, LUnits yPos)
//{
//    m_uPoint[ImoBezierInfo::k_ctrol1].x = xPos;
//    m_uPoint[ImoBezierInfo::k_ctrol1].y = yPos;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::SetCtrolPoint2(LUnits xPos, LUnits yPos)
//{
//    m_uPoint[ImoBezierInfo::k_ctrol2].x = xPos;
//    m_uPoint[ImoBezierInfo::k_ctrol2].y = yPos;
//}

//---------------------------------------------------------------------------------------
void GmoShapeArch::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
//	if (!m_fVisible) return;
//
//    //if selected, book to be rendered with handlers when posible
//    if (IsSelected())
//    {
//        //book to be rendered with handlers
//        GetOwnerBoxPage()->OnNeedToDrawHandlers(this);
//
//         for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//         {
//            //save points and update handlers position
//            m_uSavePoint[i] = m_uPoint[i];
//            m_pHandler[i]->SetHandlerCenterPoint(m_uPoint[i].x, m_uPoint[i].y);
//         }
//    }
//    else
//    {
        draw(pDrawer, m_color);
        GmoSimpleShape::on_draw(pDrawer, opt);
//    }
}

//---------------------------------------------------------------------------------------
void GmoShapeArch::draw(Drawer* pDrawer, Color color)
{
//    LUnits uWidth = lmToLogicalUnits(0.2, lmMILLIMETERS);         // width = 0.2 mm
//
//    //GmoShapeArch is rendered as a cubic bezier curve. The number of points to draw is
//    // variable, to suit a minimun resolution of 5 points / mm.
//
//    // determine number of interpolation points to use
//    int nNumPoints = abs((int)((m_uPoint[ImoBezierInfo::k_end].x - m_uPoint[ImoBezierInfo::k_start].x) / lmToLogicalUnits(0.2, lmMILLIMETERS) ));
//    if (nNumPoints < 5) nNumPoints = 5;
//
//    // compute increment for mu variable
//    double incr = 1.0 / (double)(nNumPoints-1);
//
//    // start point
//    double x1 = m_uPoint[ImoBezierInfo::k_start].x;
//    double y1 = m_uPoint[ImoBezierInfo::k_start].y;
//
//    //take the opportunity to compute bounds limits
//    double xMin = x1 , yMin = y1;
//    double xMax = x1 , yMax = y1;
//
//    // loop to compute bezier curve points and draw segment lines
//    int i;
//    double mu, mum1, a, b, c, d;
//    double x2, y2;
//    for (i=1, mu = incr; i < nNumPoints-1; i++, mu += incr) {
//        mum1 = 1 - mu;
//        a = mum1 * mum1 * mum1;
//        b = 3 * mu * mum1 * mum1;
//        c = 3 * mu * mu * mum1;
//        d = mu * mu * mu;
//
//        // compute next point
//        x2 = a * m_uPoint[ImoBezierInfo::k_start].x + b * m_uPoint[ImoBezierInfo::k_ctrol1].x
//             + c * m_uPoint[ImoBezierInfo::k_ctrol2].x + d * m_uPoint[ImoBezierInfo::k_end].x;
//        y2 = a * m_uPoint[ImoBezierInfo::k_start].y + b * m_uPoint[ImoBezierInfo::k_ctrol1].y
//             + c * m_uPoint[ImoBezierInfo::k_ctrol2].y + d * m_uPoint[ImoBezierInfo::k_end].y;
//
//        // draw segment line
//        if (fSketch)
//            pPaper->SketchLine(x1, y1, x2, y2, colorC);
//        else
//            pPaper->SolidLine(x1, y1, x2, y2, uWidth, lm_eEdgeNormal, colorC);
//
//        //update bounds
//        xMin = wxMin(xMin, x2);
//        yMin = wxMin(yMin, y2);
//        xMax = wxMax(xMax, x2);
//        yMax = wxMax(yMax, y2);
//
//        // prepare for next point
//        x1 = x2;
//        y1 = y2;
//    }
//
//    //Update bounds rectangle
//    SetXLeft(xMin);
//    SetYTop(yMin);
//    SetXRight(xMax);
//    SetYBottom(yMax);
//
//    //update selection rectangle
//    m_uSelRect = GetBounds();
}

////---------------------------------------------------------------------------------------
//void GmoShapeArch::RenderWithHandlers(lmPaper* pPaper)
//{
//    //render the arch and its handlers
//
//    //as painting uses XOR we need the complementary color
//    Color color = *wxBLUE;      //TODO User options
//    Color colorC = Color(255 - (int)color.Red(),
//                               255 - (int)color.Green(),
//                               255 - (int)color.Blue() );
//
//    //prepare to render
//    pPaper->SetLogicalFunction(wxXOR);
//
//    //draw the handlers
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//    {
//        m_pHandler[i]->Render(pPaper, colorC);
//        GetOwnerBoxPage()->AddActiveHandler(m_pHandler[i]);
//    }
//
//    //draw the arch
//    Draw(pPaper, colorC, true);        //true: sketch
//
//    //terminate renderization
//    pPaper->SetLogicalFunction(wxCOPY);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::SetDefaultControlPoints()
//{
//    // compute the default control points for the arc
//    m_uPoint[ImoBezierInfo::k_ctrol1].x = m_uPoint[ImoBezierInfo::k_start].x + (m_uPoint[ImoBezierInfo::k_end].x - m_uPoint[ImoBezierInfo::k_start].x) / 3;
//    LUnits yDsplz = lmToLogicalUnits(2, lmMILLIMETERS);
//    m_uPoint[ImoBezierInfo::k_ctrol1].y = m_uPoint[ImoBezierInfo::k_start].y + (m_fArchUnder ? yDsplz : -yDsplz);
//
//    m_uPoint[ImoBezierInfo::k_ctrol2].x = m_uPoint[ImoBezierInfo::k_ctrol1].x + (m_uPoint[ImoBezierInfo::k_end].x - m_uPoint[ImoBezierInfo::k_start].x) / 3;
//    m_uPoint[ImoBezierInfo::k_ctrol2].y = m_uPoint[ImoBezierInfo::k_end].y + (m_fArchUnder ? yDsplz : -yDsplz);
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeArch::OnHandlerDrag(lmPaper* pPaper, const UPoint& uPos, long nHandlerID)
//{
//    //erase previous draw
//    RenderWithHandlers(pPaper);
//
//    //store new handler coordinates and update all
//    wxASSERT(nHandlerID >= 0 && nHandlerID < LOMSE_BEZIER_POINTS);
//    m_pHandler[nHandlerID]->SetHandlerTopLeftPoint(uPos);
//    m_uPoint[nHandlerID] = m_pHandler[nHandlerID]->GetHandlerCenterPoint();
//
//    //draw at new position
//    RenderWithHandlers(pPaper);
//
//    return uPos;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::OnHandlerEndDrag(lmController* pCanvas, const UPoint& uPos, long nHandlerID)
//{
//	// End drag. Receives the command processor associated to the view and the
//	// final position of the object (logical units referred to page origin).
//	// This method must validate/adjust final position and, if ok, it must
//	// send a move object command to the controller.
//
//    //Compute shifts from start of drag points
//    UPoint uShifts[LOMSE_BEZIER_POINTS];
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//        uShifts[i] = UPoint(0.0, 0.0);
//    uShifts[nHandlerID] = uPos + m_pHandler[nHandlerID]->GetTopCenterDistance()
//                         - m_uSavePoint[nHandlerID];
//
//    //MoveObjectPoints() apply shifts computed from drag start points. As handlers and
//    //shape points are already displaced, it is necesary to restore the original positions to
//    //avoid double displacements.
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//        m_uPoint[i] = m_uSavePoint[i];
//
//    pCanvas->MoveObjectPoints(this, uShifts, LOMSE_BEZIER_POINTS, false);  //false-> do not update views
//}
//
////---------------------------------------------------------------------------------------
//wxBitmap* GmoShapeArch::OnBeginDrag(double rScale, wxDC* pDC)
//{
//    //save all points position
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//        m_uSavePoint[i] = m_uPoint[i];
//
//    //No bitmap needed as we are going to re-draw the line as it is moved.
//    return (wxBitmap*)NULL;
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoShapeArch::OnDrag(lmPaper* pPaper, const UPoint& uPos)
//{
//    //erase previous draw
//    RenderWithHandlers(pPaper);
//
//    //update all handler points and object points
//    UPoint uShift(uPos - this->GetBounds().GetTopLeft());
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//    {
//        m_pHandler[i]->SetHandlerTopLeftPoint( uShift + m_pHandler[i]->GetBounds().GetLeftTop() );
//        m_uPoint[i] = m_pHandler[i]->GetHandlerCenterPoint();
//    }
//
//    //draw at new position
//    RenderWithHandlers(pPaper);
//
//    return uPos;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::OnEndDrag(lmPaper* pPaper, lmController* pCanvas, const UPoint& uPos)
//{
//    //erase previous draw
//    RenderWithHandlers(pPaper);
//
//    //compute shift from start of drag point
//    UPoint uShift = uPos - m_uSavePoint[0];
//
//    //restore shape position to that of start of drag start so that MoveObject() or
//    //MoveObjectPoints() commands can apply shifts from original points.
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//        m_uPoint[i] = m_uSavePoint[i];
//
//    //as this is an object defined by points, instead of MoveObject() command we have to issue
//    //a MoveObjectPoints() command.
//    UPoint uShifts[LOMSE_BEZIER_POINTS];
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//        uShifts[i] = uShift;
//    pCanvas->MoveObjectPoints(this, uShifts, LOMSE_BEZIER_POINTS, false);  //false-> do not update views
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeArch::MovePoints(int nNumPoints, int idx, UPoint* pShifts,
//                             bool fAddShifts)
//{
//    //Each time a commnad is issued to change the object, we will receive a call
//    //back to update the shape
//
//    for (int i=0; i < LOMSE_BEZIER_POINTS; i++)
//    {
//        if (fAddShifts)
//        {
//            m_uPoint[i].x += (*(pShifts+i)).x;
//            m_uPoint[i].y += (*(pShifts+i)).y;
//        }
//        else
//        {
//            m_uPoint[i].x -= (*(pShifts+i)).x;
//            m_uPoint[i].y -= (*(pShifts+i)).y;
//        }
//
//        m_pHandler[i]->SetHandlerCenterPoint(m_uPoint[i]);
//    }
//}



//=======================================================================================
// GmoShapeTie implementation
//=======================================================================================

Vertex m_cmd[] = {
    {   0.0,    0.0, agg::path_cmd_move_to },
    { 110.0,  114.0, agg::path_cmd_curve4 },        //ctrol1
    { 530.0,  114.0, agg::path_cmd_curve4 },        //ctrol2
    { 640.0,    0.0, agg::path_cmd_curve4 },        //on-curve
    { 530.0,   88.0, agg::path_cmd_curve4 },        //ctrol1
    { 110.0,   88.0, agg::path_cmd_curve4 },        //ctrol2
    {   0.0,    0.0, agg::path_cmd_end_poly 
                     | agg::path_flags_close 
                     | agg::path_flags_ccw },       //close polygon
    {   0.0,    0.0, agg::path_cmd_stop }
};

const int m_nNumVertices = sizeof(m_cmd)/sizeof(Vertex);

//---------------------------------------------------------------------------------------
GmoShapeTie::GmoShapeTie(ImoObj* pCreatorImo, int idx, UPoint* points, LUnits tickness,
                         Color color)
    //: GmoShapeArch(pCreatorImo, GmoObj::k_shape_tie, idx, color)
    : GmoSimpleShape(pCreatorImo, GmoObj::k_shape_tie, idx, color)
    , m_thickness(tickness)
{
    save_points(points);
    compute_vertices();
    set_affine_transform();

    //m_pFirstArch = new GmoShapeArch(pCreatorImo, 0, false, color);
    //m_pSecondArch = new GmoShapeArch(pCreatorImo, 0, false, color);
//    //compute the default arch
//    OnAttachmentPointMoved(pShapeStart, lm_eGMA_StartObj, 0.0, 0.0, lmSHIFT_EVENT);
//    OnAttachmentPointMoved(pShapeEnd, lm_eGMA_EndObj, 0.0, 0.0, lmSHIFT_EVENT);
//    m_fUserShiftsApplied = false;
}

//---------------------------------------------------------------------------------------
GmoShapeTie::~GmoShapeTie()
{
    //delete m_pFirstArch;
    //delete m_pSecondArch;
}

//---------------------------------------------------------------------------------------
void GmoShapeTie::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    Color color = determine_color_to_use(opt);
    pDrawer->begin_path();
    pDrawer->fill(color);
    pDrawer->add_path(*this);
    pDrawer->end_path();
    pDrawer->render(true);

    GmoSimpleShape::on_draw(pDrawer, opt);
////    GmoShapeArch::Render(pPaper, color);
//
//    //Test code, just to see where the points are located
//    Color color = determine_color_to_use(opt);
//    pDrawer->begin_path();
//    pDrawer->fill(color);
//    pDrawer->stroke(color);
//    LUnits uWidth = 50.0f;
//    pDrawer->stroke_width(uWidth);
//    pDrawer->move_to(m_points[START].x + uWidth / 2.0f, m_points[START].y);
//    pDrawer->line_to(m_points[CTROL1].x + uWidth / 2.0f, m_points[CTROL1].y);
//    pDrawer->line_to(m_points[CTROL2].x + uWidth / 2.0f, m_points[CTROL2].y);
//    pDrawer->line_to(m_points[END].x + uWidth / 2.0f, m_points[END].y);
//    pDrawer->end_path();
//    pDrawer->render(true);
}

//---------------------------------------------------------------------------------------
void GmoShapeTie::save_points(UPoint* points)
{
    for (int i=0; i < 4; i++)
        m_points[i] = *(points+i);
}

//---------------------------------------------------------------------------------------
void GmoShapeTie::compute_vertices()
{
    LUnits t = m_thickness / 2.0f;
    m_vertices[0] = m_points[START];
    m_vertices[1].x = m_points[CTROL1].x;
    m_vertices[1].y = m_points[CTROL1].y - t;
    m_vertices[2].x = m_points[CTROL2].x;
    m_vertices[2].y = m_points[CTROL2].y - t;
    m_vertices[3] = m_points[END];
    m_vertices[4].x = m_points[CTROL2].x - t;
    m_vertices[4].y = m_points[CTROL2].y + t;
    m_vertices[5].x = m_points[CTROL1].x + t;
    m_vertices[5].y = m_points[CTROL1].y + t;
    m_vertices[6] = m_points[START];
}

//---------------------------------------------------------------------------------------
void GmoShapeTie::set_affine_transform()
{
    //double rxScale = (m_points[END].x - m_points[START].x) / 640.0;
    //double ryScale = 1.9 * ((m_points[CTROL1].y - m_points[START].y) / 114.0);
    m_trans = agg::trans_affine(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    //m_trans *= agg::trans_affine_scaling(rxScale, ryScale);
    //m_trans *= agg::trans_affine_translation(m_points[START].x, m_points[START].y);
}

//---------------------------------------------------------------------------------------
unsigned GmoShapeTie::vertex(double* px, double* py)
{
	if(m_nCurVertex >= m_nNumVertices)
		return agg::path_cmd_stop;

    if (m_nCurVertex < 7)
    {
	    *px = m_vertices[m_nCurVertex].x;   //.ux_coord;
	    *py = m_vertices[m_nCurVertex].y;   //.uy_coord;
	    //m_trans.transform(px, py);
    }
    else
    {
	    *px = 0.0;
	    *py = 0.0;
    }

	return m_cmd[m_nCurVertex++].cmd;
}

////---------------------------------------------------------------------------------------
//void GmoShapeTie::DrawControlPoints(lmPaper* pPaper)
//{
//    //DBG
//    DrawBounds(pPaper, *wxGREEN);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTie::OnAttachmentPointMoved(GmoShape* pShape, lmEAttachType nTag,
//								LUnits uxShift, LUnits uyShift, lmEParentEvent nEvent)
//{
//    //start or end note moved. Recompute start/end of tie and, if necessary, split
//    //the tie.
//
//	//WXUNUSED(uxShift);
//	//WXUNUSED(uyShift);
//	//WXUNUSED(nEvent);
//
//	//get notehead shape
//	GmoShape* pSNH = ((GmoShapeNote*)pShape)->GetNoteHead();
//	wxASSERT(pSNH);
//
//	//Compute new attachment point
//	LUnits uHalfNH = (pSNH->GetXRight() - pSNH->GetXLeft()) / 2.0;
//    LUnits uHeightNH = pSNH->GetYBottom() - pSNH->GetYTop();
//    LUnits uxPos = pSNH->GetXLeft() + uHalfNH;
//    LUnits uyPos = ((lmStaffObj*)m_pOwner)->TenthsToLogical(5.0);
//    uyPos = (m_fTieUnderNote ?
//            pSNH->GetYTop() + uHeightNH + uyPos : pSNH->GetYTop() - uyPos );
//
//    //update arch start/end points
//	if (nTag == lm_eGMA_StartObj)
//        SetStartPoint(uxPos, uyPos);
//	else if (nTag == lm_eGMA_EndObj)
//        SetEndPoint(uxPos, uyPos);
//
//    // check if the tie have to be splitted
//	if (!m_pBrotherTie) return;		//creating the tie. No information yet
//
//    UPoint paperPosEnd = GetEndNote()->GetReferencePaperPos();
//    UPoint paperPosStart = m_pBrotherTie->GetEndNote()->GetReferencePaperPos();
//    if (paperPosEnd.y != paperPosStart.y)
//	{
//        //if start note paperPos Y is not the same than end note paperPos Y the
//		//notes are in different systems. Therefore, the tie must be splitted.
//		//To do it:
//		//	- detach the two intermediate points.
//		//	- make both shapes visible.
//		//
//		// As there is no controller object to perform these actions, the first tie
//		// detecting the need must co-ordinate the necessary actions.
//
//		//determine which tie is the first one
//		GmoShapeTie* pFirstTie = this;		//assume this is the first one
//		GmoShapeTie* pSecondTie = m_pBrotherTie;
//		if (paperPosStart.y > paperPosEnd.y)
//		{
//			//wrong assumption. Reverse asignment
//			pFirstTie = m_pBrotherTie;
//			pSecondTie = this;
//		}
//
//        //first tie end point is right paper margin
//		lmBoxSystem* pSystem = this->GetOwnerSystem();
//		UPoint uEnd;
//		uEnd.x = pSystem->GetSystemFinalX();
//		uEnd.y = pFirstTie->GetStartPosY();
//		pFirstTie->SetEndPoint(uEnd.x, uEnd.y);
//		pFirstTie->SetVisible(true);
//
//		//second tie start point is begining of system
//		UPoint uStart;
//		uStart.x = pSystem->GetPositionX();
//		uStart.y = pSecondTie->GetEndPosY();
//		pSecondTie->SetStartPoint(uStart.x, uStart.y);
//		pSecondTie->SetVisible(true);
//	}
//}
//
////---------------------------------------------------------------------------------------
//lmNote* GmoShapeTie::GetStartNote()
//{
//    //the owner of a tie is always the end note
//    return m_pEndNote->GetTiedNotePrev();
//}
//
////---------------------------------------------------------------------------------------
//lmNote* GmoShapeTie::GetEndNote()
//{
//    //the owner of a tie is always the end note
//    return m_pEndNote;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShapeTie::ApplyUserShifts()
//{
//    //Start and end notes are now at their final positions. Therefore, default bezier curve is computed.
//    //This method is then invoked to apply user shifts to bezier arch.
//
//    if (!m_fUserShiftsApplied)
//    {
//        //transfer bezier data
//        for (int i=0; i < 4; i++)
//        {
//            m_uPoint[i].x += m_uUserShifts[i].x;
//            m_uPoint[i].y += m_uUserShifts[i].y;
//        }
//        m_fUserShiftsApplied = true;
//    }
//}


}  //namespace lomse
