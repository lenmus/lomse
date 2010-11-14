//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------

#include "lomse_shape_base.h"

//#include "ShapeNote.h"          //function lmDrawLegerLines()
//#include "../score/Score.h"
//#include "../score/VStaff.h"
//#include "../score/Staff.h"
//
//#include "ShapeStaff.h"
//
//
//#include "lomse_internal_model.h"
//#include <sstream>
//
//using namespace std;

namespace lomse
{

////=======================================================================================
////helper class to represent an attached shape
////=======================================================================================
//class lmAttachPoint
//{
//public:
//    lmAttachPoint(GmoShape* shape, lmEAttachType tag) : pShape(shape), nType(tag) {}
//    ~lmAttachPoint() {}
//
//	GmoShape*		pShape;
//	lmEAttachType	nType;
//};


//---------------------------------------------------------------------------------------
// Implementation of class GmoShape: any renderizable object, such as a line,
// a glyph, a note head, an arch, etc.
//---------------------------------------------------------------------------------------
GmoShape::GmoShape(GmoObj* owner, int objtype)
    : GmoObj(owner, objtype)
    , m_layer(GmoShape::k_layer_background)
{
}
//GmoShape::GmoShape(lmEGMOType nType, lmScoreObj* pOwner, int nOwnerIdx, wxString sName,
//                 bool fDraggable, bool fSelectable, wxColour color, bool fVisible)
//	: GmoObj(pOwner, nType, fDraggable, fSelectable, sName, nOwnerIdx)
//    , m_pOwnerBox((lmBox*)NULL)
//	, m_color(color)
//	, m_fVisible(fVisible)
//	, m_pParentShape((GmoShape*)NULL)
//    , m_pMouseCursorWindow((wxWindow*)NULL)
//    , m_nOrder(0)
//{
//}

//---------------------------------------------------------------------------------------
GmoShape::~GmoShape()
{
//	//delete attachment data
//	std::list<lmAttachPoint*>::iterator pItem;
//	for (pItem = m_cAttachments.begin(); pItem != m_cAttachments.end(); ++pItem)
//	{
//		(*pItem)->pShape->OnDetached(this);     //inform: this attachment removed
//		delete *pItem;
//    }
//    m_cAttachments.clear();
//
//    //if this shape is attached to others, ask for removal
//	std::list<GmoShape*>::iterator it;
//	for (it = m_cAttachedTo.begin(); it != m_cAttachedTo.end(); ++it)
//	{
//		(*it)->Detach(this, false);     //false: do not inform back this shape
//    }
//
//    //restore mouse cursor if necessary
//    if (m_pMouseCursorWindow)
//    {
//        UPoint anyPoint(0.0f, 0.0f);
//        OnMouseOut(m_pMouseCursorWindow, anyPoint);
//    }
}

//---------------------------------------------------------------------------------------
void GmoShape::shift_origin(USize& shift) 
{ 
    m_origin.x += shift.width;
    m_origin.y += shift.height;
}

////---------------------------------------------------------------------------------------
//bool GmoShape::Collision(GmoShape* pShape)
//{
//    lmURect rect1 = GetBounds();
//    return rect1.Intersects( pShape->GetBounds() );
//}
//
////---------------------------------------------------------------------------------------
//bool GmoShape::IsInRectangle(lmURect& rect)
//{
//    lmURect rect1 = GetBounds();
//    //wxLogMessage(_T("[GmoShape::IsInRectangle] Bounds(x=%.2f, y=%.2f, w=%.2f, h=%.2f), sel=(x=%.2f, y=%.2f, w=%.2f, h=%.2f)"),
//    //    rect1.x, rect1.y, rect1.width, rect1.height,
//    //    rect.x, rect.y, rect.width, rect.height );
//    return rect.Contains(rect1);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::Render(lmPaper* pPaper)
//{
//    if (IsVisible())
//        Render(pPaper, (IsSelected() ? g_pColors->ScoreSelected() : m_color) );
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::OnMouseIn(wxWindow* pWindow, UPoint& uPoint)
//{
//    if (IsSelected() && IsLeftDraggable())
//    {
//	    pWindow->SetCursor( wxCursor(wxCURSOR_SIZING));
//        m_pMouseCursorWindow = pWindow;
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::OnMouseOut(wxWindow* pWindow, UPoint& uPoint)
//{
//    if (m_pMouseCursorWindow)
//    {
//	    pWindow->SetCursor(*wxSTANDARD_CURSOR);
//        m_pMouseCursorWindow = (wxWindow*)NULL;
//    }
//}
//
////---------------------------------------------------------------------------------------
//wxString GmoShape::DumpSelRect()
//{
//    return wxString::Format(_T("SelRect=(%.2f, %.2f, %.2f, %.2f)"),
//        	m_uSelRect.x, m_uSelRect.y, m_uSelRect.width, m_uSelRect.height);
//
//}
//
////---------------------------------------------------------------------------------------
//wxString GmoShape::Dump(int nIndent)
//{
//    //Dump info about owner ScoreObj
//
//    return GetScoreOwner()->Dump();
//}
//
////---------------------------------------------------------------------------------------
//int GmoShape::Attach(GmoShape* pShape, lmEAttachType nTag)
//{
//	lmAttachPoint* pData = new lmAttachPoint(pShape, nTag);
//    m_cAttachments.push_back(pData);
//
//    //inform pShape that has been attached to this shape
//    pShape->OnAttached(this);
//
//	//return index to attached shape
//	return (int)m_cAttachments.size() - 1;
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::Detach(GmoShape* pShape, bool fInform)
//{
//    //detach shape pShape. If flag fInform is true, inform pShape of the
//    //detachment
//
//	std::list<lmAttachPoint*>::iterator it;
//	for (it = m_cAttachments.begin(); it != m_cAttachments.end(); ++it)
//	{
//		if ( (*it)->pShape->GetID() == pShape->GetID() ) break;
//    }
//	if (it != m_cAttachments.end())
//    {
//        if (fInform)
//            (*it)->pShape->OnDetached(this);
//        lmAttachPoint* pData = *it;
//		m_cAttachments.erase(it);
//        delete pData;
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::OnAttached(GmoShape* pShape)
//{
//    //this shape has been attached to shape pShape. Log it
//    m_cAttachedTo.push_back(pShape);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::OnDetached(GmoShape* pShape)
//{
//    //this shape has been detached from shape pShape. Remove from log
//
//	std::list<GmoShape*>::iterator it;
//	for (it = m_cAttachedTo.begin(); it != m_cAttachedTo.end(); ++it)
//	{
//		if ( (*it)->GetID() == pShape->GetID() ) break;
//    }
//	if (it != m_cAttachedTo.end())
//		m_cAttachedTo.erase(it);
//}
//
////---------------------------------------------------------------------------------------
//void GmoShape::InformAttachedShapes(LUnits uxShift, LUnits uyShift, lmEParentEvent nEvent)
//{
//    //Parent shape has been shifted by (uxShift, uyShift). Inform attached shapes
//
//	std::list<lmAttachPoint*>::iterator pItem;
//	for (pItem = m_cAttachments.begin(); pItem != m_cAttachments.end(); ++pItem)
//	{
//		lmAttachPoint* pData = *pItem;
//        pData->pShape->OnAttachmentPointMoved(this, pData->nType, uxShift, uyShift, nEvent);
//    }
//}
//
////---------------------------------------------------------------------------------------
//int GmoShape::GetPageNumber() const
//{
//	if (!m_pOwnerBox || !m_pOwnerBox->IsBox() ) return 0;
//	return m_pOwnerBox->GetPageNumber();
//}
//
////---------------------------------------------------------------------------------------
//unsigned GmoShape::GetVertex(LUnits* pux, LUnits* puy)
//{
//    return agg::path_cmd_stop;
//}
//
////---------------------------------------------------------------------------------------
//lmBoxScore* GmoShape::GetOwnerBoxScore()
//{
//    if (m_pOwnerBox)
//        return m_pOwnerBox->GetOwnerBoxScore();
//    else
//        return (lmBoxScore*)NULL;
//}
//
////---------------------------------------------------------------------------------------
//lmBoxPage* GmoShape::GetOwnerBoxPage()
//{
//    if (m_pOwnerBox)
//        return m_pOwnerBox->GetOwnerBoxPage();
//    else if (IsChildShape())
//        return GetParentShape()->GetOwnerBoxPage();
//    else
//        return (lmBoxPage*)NULL;
//}



//=======================================================================================
// Implementation of class GmoSimpleShape
//=======================================================================================

GmoSimpleShape::GmoSimpleShape(GmoObj* owner, int objtype)
    : GmoShape(owner, objtype) 
{
}
//GmoSimpleShape::GmoSimpleShape(lmEGMOType nType, lmScoreObj* pOwner, int nOwnerIdx,
//                             wxString sName, bool fDraggable, bool fSelectable,
//                             wxColour color, bool fVisible)
//	: GmoShape(nType, pOwner, nOwnerIdx, sName, fDraggable, fSelectable, color, fVisible)
//{
//}

//---------------------------------------------------------------------------------------
GmoSimpleShape::~GmoSimpleShape()
{
}

////---------------------------------------------------------------------------------------
//void GmoSimpleShape::Shift(LUnits xIncr, LUnits yIncr)
//{
//	//Default behaviour is to shift bounding and selection rectangles
//
//    m_uSelRect.x += xIncr;		//AWARE: As it is a rectangle, changing its origin does not
//    m_uSelRect.y += yIncr;		//       change its width/height
//
//	m_uBoundsTop.x += xIncr;
//	m_uBoundsBottom.x += xIncr;
//	m_uBoundsTop.y += yIncr;
//	m_uBoundsBottom.y += yIncr;
//
//	//if included in a composite shape update parent bounding and selection rectangles
//	if (this->IsChildShape())
//		((GmoCompositeShape*)GetParentShape())->RecomputeBounds();
//}



//=======================================================================================
// Implementation of class GmoCompositeShape
//=======================================================================================

GmoCompositeShape::GmoCompositeShape(GmoObj* owner, int objtype) 
    : GmoShape(owner, objtype) 
{
}
//GmoCompositeShape::GmoCompositeShape(lmScoreObj* pOwner, int nOwnerIdx, wxColour color,
//                                   wxString sName, bool fDraggable, lmEGMOType nType, bool fVisible)
//	: GmoShape(nType, pOwner, nOwnerIdx, sName, fDraggable, lmSELECTABLE, color, fVisible)
//{
//    m_fGrouped = true;	//by default all constituent shapes are grouped
//	m_fDoingShift = false;
//}

//---------------------------------------------------------------------------------------
GmoCompositeShape::~GmoCompositeShape()
{
//    //delete Components collection
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        delete m_Components[i];
//    }
//    m_Components.clear();
}

////---------------------------------------------------------------------------------------
//int GmoCompositeShape::Add(GmoShape* pShape)
//{
//    m_Components.push_back(pShape);
//
//	if (m_Components.size() == 1)
//	{
//		//compute new selection rectangle
//		m_uSelRect = pShape->GetSelRectangle();
//
//		// compute outer rectangle for bounds
//		m_uBoundsTop.x = pShape->GetXLeft();
//		m_uBoundsTop.y = pShape->GetYTop();
//		m_uBoundsBottom.x = pShape->GetXRight();
//		m_uBoundsBottom.y = pShape->GetYBottom();
//	}
//	else
//	{
//		//compute new selection rectangle by union of individual selection rectangles
//		m_uSelRect.Union(pShape->GetSelRectangle());
//
//		// compute outer rectangle for bounds
//		m_uBoundsTop.x = wxMin(m_uBoundsTop.x, pShape->GetXLeft());
//		m_uBoundsTop.y = wxMin(m_uBoundsTop.y, pShape->GetYTop());
//		m_uBoundsBottom.x = wxMax(m_uBoundsBottom.x, pShape->GetXRight());
//		m_uBoundsBottom.y = wxMax(m_uBoundsBottom.y, pShape->GetYBottom());
//	}
//
//	//link to parent
//	pShape->SetParentShape(this);
//
//	//return index to added shape
//	return (int)m_Components.size() - 1;
//
//}
//
////---------------------------------------------------------------------------------------
//void GmoCompositeShape::Shift(LUnits uxIncr, LUnits uyIncr)
//{
//	//Default behaviour is to shift all components
//	m_fDoingShift = true;		//semaphore to avoid recomputing constantly the bounds
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        m_Components[i]->Shift(uxIncr, uyIncr);
//    }
//	m_fDoingShift = false;
//
//	ShiftBoundsAndSelRec(uxIncr, uyIncr);
//
//    //Do this in derived classes because it causes problems in Beams (I didn't
//    //investigate but probably the attached shapes are informed twice and, therefore,
//    //get shifted too much!
//	//InformAttachedShapes(uxIncr, uyIncr, lmSHIFT_EVENT);
//}
//
////---------------------------------------------------------------------------------------
//wxString GmoCompositeShape::Dump(int nIndent)
//{
//	//TODO
//	wxString sDump = _T("");
//	sDump.append(nIndent * lmINDENT_STEP, _T(' '));
//	sDump += wxString::Format(_T("Idx: %d %s: grouped=%s, "), m_nOwnerIdx, m_sGMOName.c_str(),
//        (m_fGrouped ? _T("yes") : _T("no")) );
//    sDump += DumpBounds();
//    sDump += _T("\n");
//
//    //dump all its components
//    nIndent++;
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        sDump += m_Components[i]->Dump(nIndent);
//    }
//	return sDump;
//}
//
////---------------------------------------------------------------------------------------
//void GmoCompositeShape::Render(lmPaper* pPaper,  wxColour color)
//{
//    GmoShape::Render(pPaper, color);
//
//	//Default behaviour: render all components
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        m_Components[i]->Render(pPaper, color);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoCompositeShape::RenderHighlighted(wxDC* pDC, wxColour color)
//{
//    GmoShape::RenderHighlighted(pDC, color);
//
//	//Default behaviour: render all components
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        m_Components[i]->RenderHighlighted(pDC, color);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoCompositeShape::RenderWithHandlers(lmPaper* pPaper)
//{
//    GmoShape::RenderWithHandlers(pPaper);
//
//	//Default behaviour: render all components
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        m_Components[i]->RenderWithHandlers(pPaper);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void GmoCompositeShape::SetSelected(bool fValue)
//{
//    if (m_fSelected == fValue) return;      //nothing to do
//
//    //change selection status
//    m_fSelected = fValue;
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        m_Components[i]->Restricted_SetSelected(fValue);
//    }
//
//    //add/remove object from global list
//    lmBoxScore* pBS = this->GetOwnerBoxScore();
//    wxASSERT(pBS);
//    if (fValue)
//        pBS->AddToSelection(this);
//    else
//        pBS->RemoveFromSelection(this);
//}
//
////---------------------------------------------------------------------------------------
//bool GmoCompositeShape::BoundsContainsPoint(UPoint& uPoint)
//{
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        if (m_Components[i]->BoundsContainsPoint(uPoint))
//			return true;
//    }
//	return false;
//
//}
//
////---------------------------------------------------------------------------------------
//bool GmoCompositeShape::Collision(GmoShape* pShape)
//{
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        if (m_Components[i]->Collision(pShape))
//			return true;
//    }
//	return false;
//}
//
////---------------------------------------------------------------------------------------
//GmoShape* GmoCompositeShape::GetShape(int nShape)
//{
//	wxASSERT(nShape < (int)m_Components.size());
//	return m_Components[nShape];
//}
//
////---------------------------------------------------------------------------------------
//void GmoCompositeShape::RecomputeBounds()
//{
//	if (m_fDoingShift) return;
//
//	if (m_Components.size() > 0)
//	{
//		GmoShape* pShape = m_Components[0];
//
//		//initilaize selection rectangle
//		m_uSelRect = pShape->GetSelRectangle();
//
//		// initialize bounds
//		m_uBoundsTop.x = pShape->GetXLeft();
//		m_uBoundsTop.y = pShape->GetYTop();
//		m_uBoundsBottom.x = pShape->GetXRight();
//		m_uBoundsBottom.y = pShape->GetYBottom();
//	}
//
//    for (int i=1; i < (int)m_Components.size(); i++)
//    {
//		GmoShape* pShape = m_Components[i];
//
//		//compute new selection rectangle by union of individual selection rectangles
//		m_uSelRect.Union(pShape->GetSelRectangle());
//
//		// compute outer rectangle for bounds
//		m_uBoundsTop.x = wxMin(m_uBoundsTop.x, pShape->GetXLeft());
//		m_uBoundsTop.y = wxMin(m_uBoundsTop.y, pShape->GetYTop());
//		m_uBoundsBottom.x = wxMax(m_uBoundsBottom.x, pShape->GetXRight());
//		m_uBoundsBottom.y = wxMax(m_uBoundsBottom.y, pShape->GetYBottom());
//	}
//
//}
//
////---------------------------------------------------------------------------------------
//wxBitmap* GmoCompositeShape::OnBeginDrag(double rScale, wxDC* pDC)
//{
//	// A dragging operation is started. The view invokes this method to request the
//	// bitmap to be used as drag image. No other action is required.
//	// If no bitmap is returned drag is cancelled.
//    // The received DC is used for logical units to pixels conversions
//	//
//	// So this method returns the bitmap to use with the drag image.
//
//
//    // allocate a memory DC for drawing onto a bitmap
//    wxMemoryDC dc2;
//    dc2.SetMapMode(wxMM_TEXT);			// each logical unit is 1 pixel
//
//    // allocate the bitmap
//    // convert size to pixels
//    int wD = (int)pDC->LogicalToDeviceXRel( (wxCoord)GetWidth() );
//    int hD = (int)pDC->LogicalToDeviceYRel( (wxCoord)GetHeight() );
//    wxBitmap bitmap(wD, hD);
//
//	//clear the bitmap
//    dc2.SelectObject(bitmap);
//    dc2.SetBackground(*wxWHITE_BRUSH);
//    dc2.Clear();
//    dc2.SetBackgroundMode(wxTRANSPARENT);
//
//    //loop to get each shape bitmap and to merge it
//    for (int i=0; i < (int)m_Components.size(); i++)
//    {
//        //get shape bitmap
//        GmoShape* pShape = m_Components[i];
//		wxBitmap* pBMS = pShape->OnBeginDrag(rScale, pDC);
//
//        //merge it
//        if (pBMS)
//		{
//            lmPixels vxPos = pDC->LogicalToDeviceXRel( (wxCoord)(pShape->GetXLeft() - GetXLeft()) );
//            lmPixels vyPos = pDC->LogicalToDeviceXRel( (wxCoord)(pShape->GetYTop() - GetYTop()) );
//            dc2.DrawBitmap(*pBMS, vxPos, vyPos, true);       //true = transparent
//
//            delete pBMS;    //bitmap no longer needed
//        }
//    }
//    dc2.SelectObject(wxNullBitmap);
//
//    // the common bitmap is prepared. Make it masked
//    wxImage image = bitmap.ConvertToImage();
//    image.SetMaskColour(255, 255, 255);
//    wxBitmap* pBitmap = new wxBitmap(image);
//    ////DBG -----------
//    //wxString sFileName = _T("CompositeShape.bmp");
//    //image.SaveFile(sFileName, wxBITMAP_TYPE_BMP);
//    ////END DBG -------
//
//    return pBitmap;
//}
//
////---------------------------------------------------------------------------------------
//UPoint GmoCompositeShape::OnDrag(lmPaper* pPaper, const UPoint& uPos)
//{
//	// The view informs that the user continues dragging. We receive the new desired
//	// shape position and we must return the new allowed shape position.
//	//
//	// The default behaviour is to return the received position, so the view redraws
//	// the drag image at that position. No action must be performed by the shape on
//	// the score and score objects.
//	//
//	// The received new desired shape position is in logical units and referred to page
//	// origin. The returned new allowed shape position must also be in in logical units
//	// and referred to page origin.
//
//	return uPos;
//
//}
//
//
//

}  //namespace lomse
