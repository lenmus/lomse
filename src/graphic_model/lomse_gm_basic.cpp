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

#include "lomse_gm_basic.h"

#include "lomse_internal_model.h"
#include "lomse_drawer.h"

//#include <sstream>
//
//using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// Graphic model implementation
//---------------------------------------------------------------------------------------

GraphicModel::GraphicModel()
    : m_fCanBeDrawn(false)
{
    m_root = new GmoBoxDocument();
}

//---------------------------------------------------------------------------------------
GraphicModel::~GraphicModel()
{
    if (m_root)
        delete m_root;
    delete_stubs();
}

//---------------------------------------------------------------------------------------
void GraphicModel::delete_stubs()
{
    std::vector<GmoStub*>::iterator it;
    for (it = m_stubs.begin(); it != m_stubs.end(); ++it)
        delete *it;
    m_stubs.clear();
}

//---------------------------------------------------------------------------------------
int GraphicModel::get_num_pages()
{
    return m_root->get_num_pages();
}

//---------------------------------------------------------------------------------------
//////GmoBox* GraphicModel::create_main_box_for(ImoDocObj* pItem)
//////{
//////    //factory method
//////
//////    GmoBoxDocPage* pCurPage = m_root->get_last_page();
//////
//////    switch(pItem->get_obj_type())
//////    {
//////        case ImoObj::k_score:
//////        {
//////            GmoBoxScore* pBox = new GmoBoxScore(pCurPage);
//////            pCurPage->add_child_box(pBox);
//////            return pBox;
//////        }
//////        default:
//////            return NULL;
//////    }
//////}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GraphicModel::get_page(int i)
{
    return m_root->get_page(i);
}

//---------------------------------------------------------------------------------------
GmoStubScore* GraphicModel::get_score_stub(int i)
{
    //TODO fix this: we are assuming all stubs are score stubs
    return dynamic_cast<GmoStubScore*>( m_stubs[i] );
}

//---------------------------------------------------------------------------------------
void GraphicModel::draw_page(int iPage, UPoint& origin, Drawer* pDrawer, RenderOptions& opt)
{
    if (!m_fCanBeDrawn) return;
    get_page(iPage)->on_draw(pDrawer, opt, origin);
}


//---------------------------------------------------------------------------------------
// Graphic model objects (shapes and boxes) implementation
//---------------------------------------------------------------------------------------

GmoObj::GmoObj(GmoObj* owner, int objtype)
    : m_pOwnerGmo(owner)
    , m_objtype(objtype)
    , m_size(0.0f, 0.0f)
    , m_origin(0.0f, 0.0f)
{
}

//---------------------------------------------------------------------------------------
GmoObj::~GmoObj()
{
}

//---------------------------------------------------------------------------------------
void GmoObj::set_origin(UPoint& pos)
{
    USize shift(pos.x - m_origin.x, pos.y - m_origin.y);
    shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void GmoObj::set_origin(LUnits xLeft, LUnits yTop)
{
    USize shift(xLeft - m_origin.x, yTop - m_origin.y);
    shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void GmoObj::set_left(LUnits xLeft)
{
    USize shift(xLeft - m_origin.x, 0.0f);
    shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void GmoObj::set_top(LUnits yTop)
{
    USize shift(0.0f, yTop - m_origin.y);
    shift_origin(shift);
}


//---------------------------------------------------------------------------------------
// GmoBox
//---------------------------------------------------------------------------------------

GmoBox::GmoBox(GmoObj* owner, int objtype)
    : GmoObj(owner, objtype)
    , m_uTopMargin(0.0f)
    , m_uBottomMargin(0.0f)
    , m_uLeftMargin(0.0f)
    , m_uRightMargin(0.0f)
{
    for (int i=0; i < GmoShape::k_layer_max; i++)
        m_pStartOfLayer[i] = NULL;
}

//---------------------------------------------------------------------------------------
GmoBox::~GmoBox()
{
    delete_boxes();
    delete_shapes();
}

//---------------------------------------------------------------------------------------
void GmoBox::delete_boxes()
{
    std::vector<GmoBox*>::iterator it;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
        delete *it;
    m_childBoxes.clear();
}

//---------------------------------------------------------------------------------------
void GmoBox::delete_shapes()
{
    std::list<GmoShape*>::iterator it;
    for (it=m_shapes.begin(); it != m_shapes.end(); ++it)
        delete *it;
    m_shapes.clear();
}

//---------------------------------------------------------------------------------------
GmoBox* GmoBox::get_child_box(int i)  //i = 0..n-1
{
    if (i < get_num_boxes())
        return m_childBoxes[i];
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
void GmoBox::add_shape(GmoShape* shape, int layer)
{
    m_shapes.push_back(shape);
    shape->set_layer(layer);
	//shape->set_owner_box(this);
 //   shape->SetOrder(++m_nShapeOrder);
    if (m_pStartOfLayer[layer] == NULL)
        m_pStartOfLayer[layer] = shape;
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBox::get_shape(int i)  //i = 0..n-1
{
    std::list<GmoShape*>::iterator it = m_shapes.begin();
    for (; it != m_shapes.end() && i > 0; ++it, --i);
    if (it != m_shapes.end())
        return *it;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
void GmoBox::on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin)
{
    double xorg = m_origin.x + origin.x;
    double yorg = m_origin.y + origin.y;

    if (draw_box_requested(opt))
    {
        Color color = get_box_color();
        draw_box_bounds(pDrawer, xorg, yorg, color);
    }

    //draw shapes in this box
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        (*itS)->on_draw(pDrawer, opt, origin);

    //draw contained boxes
    std::vector<GmoBox*>::iterator it;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
        (*it)->on_draw(pDrawer, opt, origin);
}

//---------------------------------------------------------------------------------------
bool GmoBox::draw_box_requested(RenderOptions& opt)
{
    return (this->is_box_doc_page_content() && opt.draw_box_doc_page_content_flag)
        || (this->is_box_score_page() && opt.draw_box_score_page_flag)
        || (this->is_box_system() && opt.draw_box_system_flag)
        || (this->is_box_slice() && opt.draw_box_slice_flag)
        || (this->is_box_slice_instr() && opt.draw_box_slice_instr_flag);
}

//---------------------------------------------------------------------------------------
Color GmoBox::get_box_color()
{
    if (this->is_box_doc_page_content())
        return Color(255,255,0);    //yellow
    if (this->is_box_score_page())
        return Color(255,128,0);    //orange
    if (this->is_box_system())
        return Color(255,0,0);      //red
    if (this->is_box_slice())
        return Color(255,0,255);    //magenta
    if (this->is_box_slice_instr())
        return Color(0,0,255);      //blue
    return Color(0,255,0);          //green
}

//---------------------------------------------------------------------------------------
void GmoBox::draw_box_bounds(Drawer* pDrawer, double xorg, double yorg, Color& color)
{
    double red = double(color.r)/255.0;
    double green = double(color.g)/255.0;
    double blue = double(color.b)/255.0;
    double opacity = double(color.a)/255.0;

    pDrawer->begin_path();
    pDrawer->fill( rgba(1.0, 1.0, 1.0, 0.0) );     //background white transparent
    pDrawer->stroke( rgba(red,green,blue,opacity) );
    pDrawer->stroke_width(50.0);    //0.5 mm
    pDrawer->move_to(xorg, yorg);
    pDrawer->hline_to(xorg + get_width());
    pDrawer->vline_to(yorg + get_height());
    pDrawer->hline_to(xorg);
    pDrawer->vline_to(yorg);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoBox::shift_origin(USize& shift)
{
    m_origin.x += shift.width;
    m_origin.y += shift.height;

    //shift contained boxes
    std::vector<GmoBox*>::iterator itB;
    for (itB=m_childBoxes.begin(); itB != m_childBoxes.end(); ++itB)
        (*itB)->shift_origin(shift);

    //shift contained shapes
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        (*itS)->shift_origin(shift);
}


//---------------------------------------------------------------------------------------
// GmoBoxDocPage
//---------------------------------------------------------------------------------------

GmoBoxDocPage::GmoBoxDocPage(GmoObj* owner)
    : GmoBox(owner, GmoObj::k_box_doc_page)
{
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::on_draw(Drawer* pDrawer, RenderOptions& opt, UPoint& origin)
{
    ////clear lists with renderization information
    //m_ActiveHandlers.clear();
    //m_GMObjsWithHandlers.clear();

    draw_page_background(pDrawer, opt, origin);
    GmoBox::on_draw(pDrawer, opt, origin);

    ////if requested, book to render page margins
    //if (g_fShowMargins)
    //    this->OnNeedToDrawHandlers(this);
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::draw_page_background(Drawer* pDrawer, RenderOptions& opt, UPoint& origin)
{
    pDrawer->begin_path();
    pDrawer->fill( rgba(1.0, 1.0, 1.0) );     //background white
    pDrawer->stroke( rgba(1.0, 1.0, 1.0) );
    pDrawer->move_to(origin.x, origin.y);
    pDrawer->hline_to(origin.x + get_width());
    pDrawer->vline_to(origin.y + get_height());
    pDrawer->hline_to(origin.x);
    pDrawer->vline_to(origin.y);
    pDrawer->end_path();
}


//---------------------------------------------------------------------------------------
// GmoBoxDocument
//---------------------------------------------------------------------------------------

GmoBoxDocument::GmoBoxDocument()
    : GmoBox(NULL, GmoObj::k_box_document)
    , m_pLastPage(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBoxDocument::add_new_page()
{
    m_pLastPage = new GmoBoxDocPage(this);
    m_pLastPage->set_number(get_num_pages()+1);
    add_child_box(m_pLastPage);
    return m_pLastPage;
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBoxDocument::get_page(int i)
{
    return dynamic_cast<GmoBoxDocPage*>(get_child_box(i));
}


//---------------------------------------------------------------------------------------
// GmoBoxDocPageContent
//---------------------------------------------------------------------------------------

GmoBoxDocPageContent::GmoBoxDocPageContent(GmoObj* owner)
    : GmoBox(owner, GmoObj::k_box_doc_page_content)
{
}


//---------------------------------------------------------------------------------------
// GmoStubScore
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
GmoStubScore::GmoStubScore(ImoScore* pScore)
    : GmoStub(GmoObj::k_stub_score, pScore)
{
}

//---------------------------------------------------------------------------------------
GmoStubScore::~GmoStubScore()
{
}

//---------------------------------------------------------------------------------------
//void GmoStubScore::RenderPage(int nPage, lmPaper* pPaper, wxWindow* pRenderWindow,
//                            wxPoint& vOffset)
//{
//    // Render page nPage (1..n)
//	// This method is invoked from lmGraphicManager::Render()
//
//    wxASSERT(nPage > 0 && nPage <= (int)m_pages.size());
//
//    ////if firts page render score titles
//    //if (nPage == 1)
//	   // RenderShapes(pPaper);
//
//    //render the requested page
//    m_pages[nPage-1]->SetRenderWindow(pRenderWindow);
//    m_pages[nPage-1]->SetRenderWindowOffset(vOffset);
//    m_pages[nPage-1]->Render(m_pScore, pPaper);
//}

//---------------------------------------------------------------------------------------
void GmoStubScore::add_page(GmoBoxScorePage* pPage)
{
    m_pages.push_back(pPage);
}

//---------------------------------------------------------------------------------------
GmoBoxScorePage* GmoStubScore::get_page(int iPage)
{
    if (iPage < get_num_pages())
        return m_pages[iPage];
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
int GmoStubScore::get_num_pages()
{
    return (int)m_pages.size();
}

//---------------------------------------------------------------------------------------
//wxString GmoStubScore::Dump(int nIndent)
//{
//	wxString sDump = _T("");
//	sDump.append(nIndent * lmINDENT_STEP, _T(' '));
//	sDump += wxString::Format(_T("GmoStubScore. ID %d\n"), GetID());
//
//    //loop to dump the pages in this score
//	nIndent++;
//    for (int i=0; i < (int)m_pages.size(); i++)
//    {
//        sDump += m_pages[i]->Dump(nIndent);
//    }
//
//	return sDump;
//}

//---------------------------------------------------------------------------------------
int GmoStubScore::get_num_systems()
{
    return m_pages.back()->get_num_last_system() + 1;
}

//---------------------------------------------------------------------------------------
//GmoBoxSystem* GmoStubScore::get_system(int nSystem)
//{
//	//return pointer to GmoBoxSystem for system nSystem (1..n)
//
//	//locate page
//	bool fFound = false;
//    int i;
//    for (i=0; i < (int)m_pages.size(); i++)
//    {
//        if (m_pages[i]->get_num_first_system() <= nSystem &&
//			m_pages[i]->get_num_last_system() >= nSystem)
//		{
//			fFound = true;
//			break;
//		}
//    }
//
//	wxASSERT(fFound);
//	return m_pages[i]->get_system(nSystem);
//}
//
//---------------------------------------------------------------------------------------
//void GmoStubScore::PopulateLayers()
//{
//    std::vector<GmoBoxScorePage*>::iterator it;
//    for (it = m_pages.begin(); it != m_pages.end(); ++it)
//        (*it)->PopulateLayers();
//}
//
//---------------------------------------------------------------------------------------
//void GmoStubScore::AddToSelection(lmGMObject* pGMO)
//{
//    if (pGMO->IsSelectable())
//        m_Selection.AddToSelection(pGMO);
//}
//
//---------------------------------------------------------------------------------------
//void GmoStubScore::RemoveFromSelection(lmGMObject* pGMO)
//{
//    m_Selection.RemoveFromSelection(pGMO);
//}
//
//---------------------------------------------------------------------------------------
//void GmoStubScore::ClearSelection()
//{
//    m_Selection.ClearSelection();
//}
//
//---------------------------------------------------------------------------------------
//void GmoStubScore::AddToSelection(int nNumPage, LUnits uXMin, LUnits uXMax,
//                                LUnits uYMin, LUnits uYMax)
//{
//    wxASSERT(nNumPage <= get_num_pages());
//
//    GmoBoxScorePage* pBPage = get_page(nNumPage);
//    pBPage->SelectGMObjects(true, uXMin, uXMax, uYMin, uYMax);
//}

//---------------------------------------------------------------------------------------


}  //namespace lomse
