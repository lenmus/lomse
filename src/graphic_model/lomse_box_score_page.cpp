//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include "lomse_gm_basic.h"

#include "lomse_internal_model.h"
#include "lomse_box_system.h"

//#include <sstream>
//
//using namespace std;

namespace lomse
{


//#include "BoxScore.h"
//#include "BoxPage.h"
//#include "GmoBoxSlice.h"
//#include "GmoBoxSystem.h"
//#include "Handlers.h"
//
//
////access to colors
//#include "../globals/Colors.h"
//extern lmColors* g_pColors;
//
////-----------------------------------------------------------------------------------------
//// lmLayer: helper class. A collection of lmShapes
////-----------------------------------------------------------------------------------------
//
////to assign ID to user layers
//static long m_nLayerCounter = lm_eLayerUser;
//
//class lmLayer
//{
//public:
//    lmLayer(long nLayerID) : m_nLayerID(nLayerID) {}
//    ~lmLayer() {}
//
//    void InsertShapeInLayer(lmShape* pShape);
//    void RenderLayer(lmPaper* pPaper);
//    lmGMObject* FindShapeAtPos(lmUPoint& uPoint, bool fSelectable);
//
//    long                    m_nLayerID;
//    std::list<lmShape*>	    m_Shapes;		//contained shapes, ordered by creation order
//
//
//};
//
//void lmLayer::InsertShapeInLayer(lmShape* pShape)
//{
//    //Algorithm: the sorted list is traversed in order to find the element which is
//    //greater than or equal to the object to be inserted.
//    //In the worst case, when the object to be inserted is larger than all of the
//    //objects already present in the list, the entire list needs to be traversed
//    //before doing the insertion. Therefore, the total running time for the insert
//    //operation is O(n).
//
//    //wxLogMessage(_T("[lmLayer::InsertShapeInLayer] Shape %d inserted in layer %d"),
//    //    pShape->GetID(), m_nLayerID);
//
//    if (m_Shapes.empty())
//    {
//        //this is going to be the first element.
//        m_Shapes.push_back(pShape);
//        return;
//    }
//
//    //there are elements. Find where to insert new item
//    std::list<lmShape*>::iterator itCur;
//    for (itCur = m_Shapes.begin(); itCur != m_Shapes.end(); ++itCur)
//    {
//        if ((*itCur)->GetOrder() < pShape->GetOrder())
//            break;      //insertion point found
//    }
//
//    //position found: insert before itCur
//    if (itCur != m_Shapes.end())
//        m_Shapes.insert(itCur, pShape);
//    else
//        m_Shapes.push_back(pShape);
//
//    return;
//}
//
//void lmLayer::RenderLayer(lmPaper* pPaper)
//{
//    std::list<lmShape*>::iterator it;
//    for (it = m_Shapes.begin(); it != m_Shapes.end(); ++it)
//    {
//        (*it)->Render(pPaper);
//    }
//}
//
//lmGMObject* lmLayer::FindShapeAtPos(lmUPoint& uPoint, bool fSelectable)
//{
//    std::list<lmShape*>::reverse_iterator it;
//    for (it = m_Shapes.rbegin(); it != m_Shapes.rend(); ++it)
//    {
//        bool fFound = (*it)->HitTest(uPoint);
//        if ( (fSelectable && (*it)->IsSelectable() && fFound) ||
//            (!fSelectable && fFound) )
//            return *it;
//    }
//    return (lmGMObject*)NULL;
//}
//
//
//
//


//-------------------------------------------------------------------------------------
// GmoBoxScorePage implementation
//-------------------------------------------------------------------------------------

GmoBoxScorePage::GmoBoxScorePage(ImoScore* pScore)
    : GmoBox(GmoObj::k_box_score_page, pScore)
//    , m_nNumPage(nNumPage)
    , m_nFirstSystem(-1)
    , m_nLastSystem(-1)
//    , m_pRenderWindow(NULL)
{
//    CreateLayers();     //create standard layers
//
//    //create margin shapes
//	ImoScore* pScore = (ImoScore*)m_pOwner;
//	LUnits uxLeftMargin = pScore->GetLeftMarginXPos();
//    LUnits uxRightMargin = pScore->GetRightMarginXPos();
//	LUnits uyTopMargin = pScore->GetPageTopMargin();
//    LUnits uyBottomMargin = pScore->GetMaximumY();
//    LUnits uPageWidth = pScore->GetPaperSize().GetWidth();
//    LUnits uPageHeight = pScore->GetPaperSize().GetHeight();
//
//    m_pMarginShapes[0] =
//        LOMSE_NEW lmShapeMargin(pScore, this, lmMARGIN_TOP, m_nNumPage, lmHORIZONTAL,
//                          uyTopMargin, uPageWidth);
//
//    m_pMarginShapes[1] =
//        LOMSE_NEW lmShapeMargin(pScore, this, lmMARGIN_BOTTOM, m_nNumPage, lmHORIZONTAL,
//                          uyBottomMargin, uPageWidth);
//
//    m_pMarginShapes[2] =
//        LOMSE_NEW lmShapeMargin(pScore, this, lmMARGIN_LEFT, m_nNumPage, lmVERTICAL,
//                          uxLeftMargin, uPageHeight);
//
//    m_pMarginShapes[3] =
//        LOMSE_NEW lmShapeMargin(pScore, this, lmMARGIN_RIGHT, m_nNumPage, lmVERTICAL,
//                          uxRightMargin, uPageHeight);
}

GmoBoxScorePage::~GmoBoxScorePage()
{
//    m_GMObjsWithHandlers.clear();
//
//    //delete handlers
//    for (int i=0; i<4; i++)
//        delete m_pMarginShapes[i];
//
//    //delete layers
//    std::list<lmLayer*>::iterator itL;
//    for (itL = m_Layers.begin(); itL != m_Layers.end(); ++itL)
//        delete *itL;
}

//void GmoBoxScorePage::CreateLayers()
//{
//    //create standard layers and layer indexes
//    m_Layers.push_back( LOMSE_NEW lmLayer(lm_eLayerBackground) );
//    m_Layers.push_back( LOMSE_NEW lmLayer(lm_eLayerStaff) );
//    m_Layers.push_back( LOMSE_NEW lmLayer(lm_eLayerBarlines) );
//    m_Layers.push_back( LOMSE_NEW lmLayer(lm_eLayerNotes) );
//    m_Layers.push_back( LOMSE_NEW lmLayer(lm_eLayerAuxObjs) );
//    m_Layers.push_back( LOMSE_NEW lmLayer(lm_eLayerTop) );
//}
//
//void GmoBoxScorePage::AddShapeToLayer(lmShape* pShape, long nLayerID)
//{
//    wxASSERT(nLayerID >= lm_eLayerBackground && nLayerID < m_nLayerCounter);
//
//    //find requested layer
//    std::list<lmLayer*>::iterator it;
//    for (it = m_Layers.begin(); it != m_Layers.end(); ++it)
//    {
//        if ((*it)->m_nLayerID == nLayerID)
//            break;  //found
//    }
//    wxASSERT(it != m_Layers.end());
//
//    //add shape to layer
//    (*it)->InsertShapeInLayer(pShape);
//}
//
//void GmoBoxScorePage::PopulateLayers()
//{
//    //reorganize shapes into layers
//
//	AddShapesToLayers(this);
//}

void GmoBoxScorePage::add_system(GmoBoxSystem* pSystem, int iSystem)
{
    //Update references
    if (m_nFirstSystem == -1)
        m_nFirstSystem = iSystem;
    m_nLastSystem = iSystem;

    add_child_box(pSystem);
}

GmoBoxSystem* GmoBoxScorePage::get_system(int iSystem)
{
	//returns pointer to GmoBoxSystem for system nSystem (0..n-1)

	int i = iSystem - m_nFirstSystem;
	if (i < 0)
		return (GmoBoxSystem*)NULL;		//the system is not in this page
	else
		return (GmoBoxSystem*)m_childBoxes[i];
}

//lmGMObject* GmoBoxScorePage::FindShapeAtPos(lmUPoint& uPoint, bool fSelectable)
//{
//    //look up in active handlers
//	std::list<lmHandler*>::reverse_iterator it;
//	for (it = m_ActiveHandlers.rbegin(); it != m_ActiveHandlers.rend(); ++it)
//    {
//		if ((*it)->BoundsContainsPoint(uPoint))
//            return *it;
//    }
//
//    //find in layers
//    std::list<lmLayer*>::reverse_iterator itL;
//    for (itL = m_Layers.rbegin(); itL != m_Layers.rend(); ++itL)
//    {
//        lmGMObject* pGMO = (*itL)->FindShapeAtPos(uPoint, fSelectable);
//        if (pGMO)
//			return pGMO;    //found
//    }
//    return (lmGMObject*)NULL;
//}

//void GmoBoxScorePage::on_draw(Drawer* pDrawer, RenderOptions& opt)
//{
//    //clear lists with renderization information
//    m_ActiveHandlers.clear();
//    m_GMObjsWithHandlers.clear();
//
//    //render shapes in layers
//    std::list<lmLayer*>::iterator itL;
//    for (itL = m_Layers.begin(); itL != m_Layers.end(); ++itL)
//        (*itL)->RenderLayer(pPaper);
//
//    //if debug render box bounds
//    if (g_fDrawBounds)
//        lmBox::Render(pPaper, lmUPoint(0,0));
//
//    //if requested, book to render page margins
//    if (g_fShowMargins)
//        this->OnNeedToDrawHandlers(this);
//}

//void GmoBoxScorePage::RenderWithHandlers(lmPaper* pPaper)
//{
//    //render page with margins
//
//    wxColour color = *wxGREEN;      //TODO User options
//    for (int i=0; i < 4; i++)
//    {
//        m_pMarginShapes[i]->Render(pPaper, color);
//        AddActiveHandler( m_pMarginShapes[i] );
//    }
//}
//
//void GmoBoxScorePage::DrawAllHandlers(lmPaper* pPaper)
//{
//    //Inform all GMObjects that booked to draw handlers (during renderization) to do
//    //it now
//
//	std::vector<lmGMObject*>::iterator it;
//	for (it = m_GMObjsWithHandlers.begin(); it != m_GMObjsWithHandlers.end(); ++it)
//        (*it)->RenderWithHandlers(pPaper);
//}
//
//void GmoBoxScorePage::AddActiveHandler(lmHandler* pHandler)
//{
//    m_ActiveHandlers.push_back(pHandler);
//}
//
//void GmoBoxScorePage::OnNeedToDrawHandlers(lmGMObject* pGMO)
//{
//    //This method is invoked by objects contained in this BoxPage.
//    //Handlers are not rendered during shapes renderization. Instead, if
//    //during renderization and object has the need to draw handlers (for instance,
//    //if the object is selected) if MUST inform its parent BoxPage by invoking this
//    //method. BoxPage mantains a list of objects having requested to draw handlers
//    //and, when appropriate, it will invoke method DrawHandlers for those objects,
//    //so that they can do it.
//
//    m_GMObjsWithHandlers.push_back(pGMO);
//}
//
//int GmoBoxScorePage::GetSystemNumber(GmoBoxSystem* pSystem)
//{
//	//returns the number of the system (1..n) received as parameter
//	//precondition: the system must be in this page
//
//    //loop to look up in the systems collection
//	for(int i=0; i < (int)m_childBoxes.size(); i++)
//    {
//        if (((GmoBoxSystem*)m_childBoxes[i]) == pSystem)
//			return i + m_nFirstSystem;		//system found
//    }
//	wxASSERT(false);	//system not in this page
//	return 0;			//keep compiler happy
//}



}  //namespace lomse


