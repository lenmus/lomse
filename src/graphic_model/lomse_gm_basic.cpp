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

#include "lomse_gm_basic.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_drawer.h"
#include "lomse_selections.h"
#include "lomse_time.h"

#include <cstdlib>      //abs
#include <iomanip>


namespace lomse
{


//=======================================================================================
// Graphic model implementation
//=======================================================================================
GraphicModel::GraphicModel()
    : m_fCanBeDrawn(false)
{
    m_root = new GmoBoxDocument(this, NULL);    //TODO: replace NULL by ImoDocument
}

//---------------------------------------------------------------------------------------
GraphicModel::~GraphicModel()
{
    delete m_root;
    delete_stubs();
    m_noterestToShape.clear();
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
GmoBoxDocPage* GraphicModel::get_page(int i)
{
    return m_root->get_page(i);
}

//---------------------------------------------------------------------------------------
GmoStubScore* GraphicModel::get_score_stub(int i)
{
    //TODO-LOG fix this: we are assuming all stubs are score stubs
    return dynamic_cast<GmoStubScore*>( m_stubs[i] );
}

//---------------------------------------------------------------------------------------
void GraphicModel::draw_page(int iPage, UPoint& origin, Drawer* pDrawer,
                             RenderOptions& opt)
{
    if (!m_fCanBeDrawn) return;

    pDrawer->set_shift(origin.x, origin.y);
    get_page(iPage)->on_draw(pDrawer, opt);
    pDrawer->render(true);
    pDrawer->remove_shift();
}

//---------------------------------------------------------------------------------------
void GraphicModel::dump_page(int iPage, ostream& outStream)
{
    outStream << "                    org.x        org.y     size.x      size.y" << endl;
    outStream << "-------------------------------------------------------------" << endl;
    get_page(iPage)->dump_boxes_shapes(outStream);
}

//---------------------------------------------------------------------------------------
GmoObj* GraphicModel::hit_test(int iPage, LUnits x, LUnits y)
{
    return get_page(iPage)->hit_test(x, y);
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::find_shape_at(int iPage, LUnits x, LUnits y)
{
    return get_page(iPage)->find_shape_at(x, y);
}

//---------------------------------------------------------------------------------------
GmoBox* GraphicModel::find_inner_box_at(int iPage, LUnits x, LUnits y)
{
    return get_page(iPage)->find_inner_box_at(x, y);
}

//---------------------------------------------------------------------------------------
void GraphicModel::select_objects_in_rectangle(int iPage, SelectionSet& selection,
                                               const URect& selRect, unsigned flags)
{
    selection.clear();
    get_page(iPage)->select_objects_in_rectangle(selection, selRect, flags);
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::find_shape_for_object(ImoStaffObj* pSO)
{
    int numPages = get_num_pages();
    for (int i = 0; i < numPages; ++i)
    {
        GmoShape* pShape = get_page(i)->find_shape_for_object(pSO);
        if (pShape)
            return pShape;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
GmoShape* GraphicModel::get_shape_for_noterest(ImoNoteRest* pNR)
{
    map<ImoNoteRest*, GmoShape*>::const_iterator it =  m_noterestToShape.find( pNR );
	if (it != m_noterestToShape.end())
		return it->second;
    else
        return NULL;
}

//---------------------------------------------------------------------------------------
void GraphicModel::store_in_map_imo_shape(ImoNoteRest* pNR, GmoShape* pShape)
{
    if (pNR)
        m_noterestToShape[pNR] = pShape;
}

//---------------------------------------------------------------------------------------
void GraphicModel::highlight_object(ImoStaffObj* pSO, bool value)
{
    ImoNoteRest* pNR = dynamic_cast<ImoNoteRest*>( pSO );
    if (pNR)
    {
        GmoShape* pShape = get_shape_for_noterest(pNR);
        pShape->set_highlighted(value);
    }
}


//=======================================================================================
// GmoObj implementation
//=======================================================================================

//association object-type <-> object-name
static std::map<int, std::string> m_typeToName;
static bool m_fNamesLoaded = false;

//---------------------------------------------------------------------------------------
GmoObj::GmoObj(int objtype, ImoObj* pCreatorImo)
    : m_objtype(objtype)
    , m_origin(0.0f, 0.0f)
    , m_size(0.0f, 0.0f)
    , m_fSelected(false)
    , m_pCreatorImo(pCreatorImo)
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
    if (xLeft == 0.0f && yTop == 0.0f) return;

    USize shift(xLeft - m_origin.x, yTop - m_origin.y);
    shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void GmoObj::set_left(LUnits xLeft)
{
    if (xLeft == 0.0f) return;

    USize shift(xLeft - m_origin.x, 0.0f);
    shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void GmoObj::set_top(LUnits yTop)
{
    if (yTop == 0.0f) return;

    USize shift(0.0f, yTop - m_origin.y);
    shift_origin(shift);
}

//---------------------------------------------------------------------------------------
URect GmoObj::get_bounds()
{
    return URect(m_origin.x, m_origin.y, m_size.width, m_size.height);
}

//---------------------------------------------------------------------------------------
bool GmoObj::bounds_contains_point(UPoint& p)
{
    return get_bounds().contains(p);
}

//---------------------------------------------------------------------------------------
void GmoObj::shift_origin(const USize& shift)
{
    m_origin.x += shift.width;
    m_origin.y += shift.height;
}

//---------------------------------------------------------------------------------------
void GmoObj::dump(ostream& outStream)
{
    outStream << get_name(m_objtype)
              << fixed << setprecision(2) << setfill(' ')
              << setw(10) << round_half_up(m_origin.x) << ", "
              << setw(10) << round_half_up(m_origin.y) << ", "
              << setw(10) << round_half_up(m_size.width) << ", "
              << setw(10) << round_half_up(m_size.height) << endl;
}

//---------------------------------------------------------------------------------------
string& GmoObj::get_name(int objtype)
{
    if (!m_fNamesLoaded)
    {
        m_typeToName[k_box_document]            = "box-document   ";
        m_typeToName[k_box_doc_page]            = "box-doc-page   ";
        m_typeToName[k_box_doc_page_content]    = "box-docpg-cont.";
        m_typeToName[k_box_inline]              = "box-inline     ";
        m_typeToName[k_box_paragraph]           = "box-paragraph  ";
        m_typeToName[k_box_score_page]          = "box-score-page ";
        m_typeToName[k_box_slice]               = "box-slice      ";
        m_typeToName[k_box_slice_instr]         = "box-slice-intr ";
        m_typeToName[k_box_system]              = "box-system     ";

        // shapes
        m_typeToName[k_shape_accidentals]       = "accidentals    ";
        m_typeToName[k_shape_accidental_sign]   = "accidental-sign";
        m_typeToName[k_shape_arch]              = "arch           ";
        m_typeToName[k_shape_barline]           = "barline        ";
        m_typeToName[k_shape_beam]              = "beam           ";
        m_typeToName[k_shape_brace]             = "brace          ";
        m_typeToName[k_shape_bracket]           = "bracket        ";
        m_typeToName[k_shape_button]            = "button         ";
        m_typeToName[k_shape_clef]              = "clef           ";
        m_typeToName[k_shape_dot]               = "dot            ";
        m_typeToName[k_shape_fermata]           = "fermata        ";
        m_typeToName[k_shape_flag]              = "flag           ";
        m_typeToName[k_shape_invisible]         = "invisible      ";
        m_typeToName[k_shape_key_signature]     = "key            ";
        m_typeToName[k_shape_note]              = "note           ";
        m_typeToName[k_shape_notehead]          = "notehead       ";
        m_typeToName[k_shape_rectangle]         = "rectangle      ";
        m_typeToName[k_shape_rest]              = "rest           ";
        m_typeToName[k_shape_rest_glyph]        = "rest-glyph     ";
        m_typeToName[k_shape_stem]              = "stem           ";
        m_typeToName[k_shape_staff]             = "staff          ";
        m_typeToName[k_shape_text]              = "text           ";
        m_typeToName[k_shape_time_signature]    = "time           ";
        m_typeToName[k_shape_tie]               = "tie            ";
        m_typeToName[k_shape_time_signature_digit]
                                                = "time-digit     ";
        m_typeToName[k_shape_tuplet]            = "tuplet         ";
        m_typeToName[k_shape_word]              = "word           ";

        // stub
        m_typeToName[k_stub_score]              = "stub-score     ";

        m_fNamesLoaded = true;
    }

    return m_typeToName[objtype];
}



//=======================================================================================
// GmoBox
//=======================================================================================
GmoBox::GmoBox(int objtype, ImoObj* pCreatorImo)
    : GmoObj(objtype, pCreatorImo)
    , m_pParentBox(NULL)
    , m_uTopMargin(0.0f)
    , m_uBottomMargin(0.0f)
    , m_uLeftMargin(0.0f)
    , m_uRightMargin(0.0f)
{
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
void GmoBox::add_child_box(GmoBox* child)
{
    m_childBoxes.push_back(child);
    child->set_owner_box(this);
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
void GmoBox::add_child_box_and_its_shapes(GmoBox* child)
{
    add_child_box(child);
    child->store_shapes_in(this);
}

//---------------------------------------------------------------------------------------
void GmoBox::add_shape(GmoShape* shape, int layer)
{
    shape->set_layer(layer);
    //shape->set_owner_box(this);
    m_shapes.push_back(shape);

//    GmoBoxDocPage* pPage = get_parent_box_page();
//    pPage->store_shape(shape, layer);
}
//---------------------------------------------------------------------------------------
void GmoBox::store_shapes_in(GmoBox* pBox)
{
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
            pBox->add_shape(*itS, (*itS)->get_layer());

    std::vector<GmoBox*>::iterator itB;
    for (itB=m_childBoxes.begin(); itB != m_childBoxes.end(); ++itB)
        (*itB)->store_shapes_in(pBox);
}

//---------------------------------------------------------------------------------------
void GmoBox::store_shapes_in_page(GmoBoxDocPage* pPage)
{
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        pPage->store_shape(*itS);

    std::vector<GmoBox*>::iterator itB;
    for (itB=m_childBoxes.begin(); itB != m_childBoxes.end(); ++itB)
        (*itB)->store_shapes_in_page(pPage);
}

//---------------------------------------------------------------------------------------
void GmoBox::store_shapes_in_doc_page()
{
    GmoBoxDocPage* pPage = get_parent_box_page();
    GmoBox* pBox = this;        //gcc complains if next method is invoked directly
    pBox->store_shapes_in_page(pPage);
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBox::get_parent_box_page()
{
    if (this->is_box_doc_page())
        return dynamic_cast<GmoBoxDocPage*>(this);
    else
        return get_parent_box()->get_parent_box_page();
}

//---------------------------------------------------------------------------------------
GraphicModel* GmoBox::get_graphic_model()
{
    return get_parent_box()->get_graphic_model();
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
void GmoBox::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    draw_border(pDrawer, opt);
    draw_shapes(pDrawer, opt);

    //draw contained boxes
    std::vector<GmoBox*>::iterator it;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
        (*it)->on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoBox::draw_shapes(Drawer* pDrawer, RenderOptions& opt)
{
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        (*itS)->on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoBox::draw_border(Drawer* pDrawer, RenderOptions& opt)
{
    ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( m_pCreatorImo );
    if (pImo)
    {
        ImoStyle* pStyle = pImo->get_style();
        if (pStyle && (pStyle->get_lunits_property(ImoStyle::k_border_width_top) > 0.0f
                       || pStyle->get_lunits_property(ImoStyle::k_border_width_bottom) > 0.0f
                       || pStyle->get_lunits_property(ImoStyle::k_border_width_left) > 0.0f
                       || pStyle->get_lunits_property(ImoStyle::k_border_width_right) > 0.0f) )
        {
            double xLeft = double(m_origin.x
                           + pStyle->get_lunits_property(ImoStyle::k_margin_left));
            double yTop = double(m_origin.y
                          + pStyle->get_lunits_property(ImoStyle::k_margin_top));
            double xRight = double(get_right()
                            - pStyle->get_lunits_property(ImoStyle::k_margin_right));
            double yBottom = double(get_bottom()
                             - pStyle->get_lunits_property(ImoStyle::k_margin_bottom));

            pDrawer->begin_path();
            pDrawer->fill( Color(255, 255, 255, 0) );     //background white transparent
            pDrawer->stroke( Color(0,0,0) );            //TODO: border color
            pDrawer->move_to(xLeft, yTop);

            //top border
            if (pStyle->get_lunits_property(ImoStyle::k_border_width_top) > 0.0f)
            {
                pDrawer->stroke_width( pStyle->get_lunits_property(ImoStyle::k_border_width_top) );
                pDrawer->hline_to(xRight);
            }
            else
                pDrawer->move_to(xRight, yTop);

            //right border
            if (pStyle->get_lunits_property(ImoStyle::k_border_width_right) > 0.0f)
            {
                pDrawer->stroke_width( pStyle->get_lunits_property(ImoStyle::k_border_width_right) );
                pDrawer->vline_to(yBottom);
            }
            else
                pDrawer->move_to(xRight, yBottom);

            //bottom border
            if (pStyle->get_lunits_property(ImoStyle::k_border_width_bottom) > 0.0f)
            {
                pDrawer->stroke_width( pStyle->get_lunits_property(ImoStyle::k_border_width_bottom) );
                pDrawer->hline_to(xLeft);
            }
            else
                pDrawer->move_to(xLeft, yBottom);

            //left border
            if (pStyle->get_lunits_property(ImoStyle::k_border_width_left) > 0.0f)
            {
                pDrawer->stroke_width( pStyle->get_lunits_property(ImoStyle::k_border_width_left) );
                pDrawer->vline_to(yTop);
            }

            pDrawer->end_path();
        }
    }

    if (must_draw_bounds(opt))
    {
        double xorg = m_origin.x;
        double yorg = m_origin.y;
        Color color = get_box_color();
        draw_box_bounds(pDrawer, xorg, yorg, color);
    }
}

//---------------------------------------------------------------------------------------
bool GmoBox::must_draw_bounds(RenderOptions& opt)
{
    return (this->is_box_doc_page_content() && opt.draw_box_doc_page_content_flag)
        || (this->is_item_main_box() && opt.draw_box_container_flag)
        || (this->is_box_system() && opt.draw_box_system_flag)
        || (this->is_box_slice() && opt.draw_box_slice_flag)
        || (this->is_box_slice_instr() && opt.draw_box_slice_instr_flag)
        || (this->is_box_inline() && opt.draw_box_inline_flag);
}

//---------------------------------------------------------------------------------------
Color GmoBox::get_box_color()
{
    if (this->is_box_doc_page_content())
        return Color(255,255,0);    //yellow
    if (this->is_item_main_box())
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
    pDrawer->begin_path();
    pDrawer->fill( Color(255, 255, 255, 0) );     //background white transparent
    pDrawer->stroke( color );
    pDrawer->stroke_width(50.0);    //0.5 mm
    pDrawer->move_to(xorg, yorg);
    pDrawer->hline_to(xorg + get_width());
    pDrawer->vline_to(yorg + get_height());
    pDrawer->hline_to(xorg);
    pDrawer->vline_to(yorg);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
GmoBox* GmoBox::find_inner_box_at(LUnits x, LUnits y)
{
    URect bbox = get_bounds();
    if (bbox.contains(x, y))
    {
        std::vector<GmoBox*>::iterator it;
        for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
        {
            GmoBox* pBox = (*it)->find_inner_box_at(x, y);
            if (pBox)
			    return pBox;
        }
        return this;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
void GmoBox::shift_origin(const USize& shift)
{
    if (shift.width == 0.0f && shift.height == 0.0f) return;

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
void GmoBox::dump_boxes_shapes(ostream& outStream)
{
    GmoObj::dump(outStream);

    dump_shapes(outStream);

    //dump contained boxes
    std::vector<GmoBox*>::iterator it;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
        (*it)->dump_boxes_shapes(outStream);
}

//---------------------------------------------------------------------------------------
void GmoBox::dump_shapes(ostream& outStream)
{
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        (*itS)->dump(outStream);
}
//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_top()
{
    if (m_pCreatorImo && m_pCreatorImo->is_contentobj())
    {
        ImoContentObj* pImo = dynamic_cast<ImoContentObj*>(m_pCreatorImo);
        ImoStyle* pStyle = pImo->get_style();
        if (pStyle)
            return get_top() + pStyle->get_lunits_property(ImoStyle::k_margin_top)
                   + pStyle->get_lunits_property(ImoStyle::k_border_width_top)
                   + pStyle->get_lunits_property(ImoStyle::k_padding_top);
    }
    return get_top();
}

//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_left()
{
    if (m_pCreatorImo && m_pCreatorImo->is_contentobj())
    {
        ImoContentObj* pImo = dynamic_cast<ImoContentObj*>(m_pCreatorImo);
        ImoStyle* pStyle = pImo->get_style();
        if (pStyle)
            return get_left() + pStyle->get_lunits_property(ImoStyle::k_margin_left)
                   + pStyle->get_lunits_property(ImoStyle::k_border_width_left)
                   + pStyle->get_lunits_property(ImoStyle::k_padding_left);
    }
    return get_left();
}

//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_width()
{
    if (m_pCreatorImo && m_pCreatorImo->is_contentobj())
    {
        ImoContentObj* pImo = dynamic_cast<ImoContentObj*>(m_pCreatorImo);
        ImoStyle* pStyle = pImo->get_style();
        if (pStyle)
            return get_width() - pStyle->get_lunits_property(ImoStyle::k_margin_left)
                   - pStyle->get_lunits_property(ImoStyle::k_border_width_left)
                   - pStyle->get_lunits_property(ImoStyle::k_padding_left)
                   - pStyle->get_lunits_property(ImoStyle::k_margin_right)
                   - pStyle->get_lunits_property(ImoStyle::k_border_width_right)
                   - pStyle->get_lunits_property(ImoStyle::k_padding_right);
    }
    return get_width();
}

//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_height()
{
    if (m_pCreatorImo && m_pCreatorImo->is_contentobj())
    {
        ImoContentObj* pImo = dynamic_cast<ImoContentObj*>(m_pCreatorImo);
        ImoStyle* pStyle = pImo->get_style();
        if (pStyle)
            return get_height() - pStyle->get_lunits_property(ImoStyle::k_margin_top)
                   - pStyle->get_lunits_property(ImoStyle::k_border_width_top)
                   - pStyle->get_lunits_property(ImoStyle::k_padding_top)
                   - pStyle->get_lunits_property(ImoStyle::k_margin_bottom)
                   - pStyle->get_lunits_property(ImoStyle::k_border_width_bottom)
                   - pStyle->get_lunits_property(ImoStyle::k_padding_bottom);
    }
    return get_height();
}


//=======================================================================================
// GmoLayer: helper class. A collection of GmoShape objects
//=======================================================================================

////to assign ID to user layers
//static long m_nLayerCounter = GmoShape::k_layer_user;

class GmoLayer
{
public:
    GmoLayer(int layer) : m_nLayerID(layer) {}
    ~GmoLayer() {}

    //void InsertShapeInLayer(GmoShape* pShape);
    //void RenderLayer(lmPaper* pPaper);
    //GmoObj* FindShapeAtPos(lmUPoint& uPoint, bool fSelectable);

    long m_nLayerID;
    std::list<GmoShape*> m_shapes;		//contained shapes, ordered by creation order
};

////---------------------------------------------------------------------------------------
//void GmoLayer::InsertShapeInLayer(GmoShape* pShape)
//{
//    //Algorithm: the sorted list is traversed in order to find the element which is
//    //greater than or equal to the object to be inserted.
//    //In the worst case, when the object to be inserted is larger than all of the
//    //objects already present in the list, the entire list needs to be traversed
//    //before doing the insertion. Therefore, the total running time for the insert
//    //operation is O(n).
//
//    if (m_shapes.empty())
//    {
//        //this is going to be the first element.
//        m_shapes.push_back(pShape);
//        return;
//    }
//
//    //there are elements. Find where to insert new item
//    std::list<GmoShape*>::iterator itCur;
//    for (itCur = m_shapes.begin(); itCur != m_shapes.end(); ++itCur)
//    {
//        if ((*itCur)->GetOrder() < pShape->GetOrder())
//            break;      //insertion point found
//    }
//
//    //position found: insert before itCur
//    if (itCur != m_shapes.end())
//        m_shapes.insert(itCur, pShape);
//    else
//        m_shapes.push_back(pShape);
//
//    return;
//
//    //std::list<GmoShape*>::iterator it = m_shapes.begin();
//    //for (; it != m_shapes.end() && (*it)->get_layer() <= layer; ++it);
//
//    //if (it == m_shapes.end())
//    //    m_shapes.push_back(shape);
//    //else
//    //    m_shapes.insert(it, shape);
//}
//
////---------------------------------------------------------------------------------------
//void GmoLayer::RenderLayer(lmPaper* pPaper)
//{
//    std::list<GmoShape*>::iterator it;
//    for (it = m_shapes.begin(); it != m_shapes.end(); ++it)
//    {
//        (*it)->Render(pPaper);
//    }
//}
//
////---------------------------------------------------------------------------------------
//GmoObj* GmoLayer::FindShapeAtPos(lmUPoint& uPoint, bool fSelectable)
//{
//    std::list<GmoShape*>::reverse_iterator it;
//    for (it = m_shapes.rbegin(); it != m_shapes.rend(); ++it)
//    {
//        bool fFound = (*it)->HitTest(uPoint);
//        if ( (fSelectable && (*it)->IsSelectable() && fFound) ||
//            (!fSelectable && fFound) )
//            return *it;
//    }
//    return NULL;
//}




//=======================================================================================
// GmoBoxDocPage
//=======================================================================================
GmoBoxDocPage::GmoBoxDocPage(ImoObj* pCreatorImo)
    : GmoBox(GmoObj::k_box_doc_page, pCreatorImo)
{
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    ////clear lists with renderization information
    //m_ActiveHandlers.clear();
    //m_GMObjsWithHandlers.clear();

    draw_page_background(pDrawer, opt);
    GmoBox::on_draw(pDrawer, opt);

    ////if requested, book to render page margins
    //if (g_fShowMargins)
    //    this->OnNeedToDrawHandlers(this);
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::draw_page_background(Drawer* pDrawer, RenderOptions& opt)
{
    pDrawer->begin_path();
    pDrawer->fill( Color(255, 255, 255) );     //background white
    pDrawer->stroke( Color(255, 255, 255) );
    pDrawer->move_to(0.0, 0.0);
    pDrawer->hline_to(get_width());
    pDrawer->vline_to(get_height());
    pDrawer->hline_to(0.0);
    pDrawer->vline_to(0.0);
    pDrawer->end_path();
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::store_shape(GmoShape* pShape)
{
    int layer = pShape->get_layer();
    std::list<GmoShape*>::iterator it;
    for (it = m_allShapes.begin(); it != m_allShapes.end(); ++it)
    {
        if ((*it)->get_layer() > layer)
            break;
    }

    if (it == m_allShapes.end())
        m_allShapes.push_back(pShape);
    else
        m_allShapes.insert(it, pShape);

    store_in_map_imo_shape(pShape);
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::store_in_map_imo_shape(GmoShape* pShape)
{
    if (pShape->is_shape_note() || pShape->is_shape_rest())
    {
        ImoNoteRest* pNR = dynamic_cast<ImoNoteRest*>( pShape->get_creator_imo() );
        if (pNR)
        {
            GraphicModel* pModel = get_graphic_model();
            if (pModel)
                pModel->store_in_map_imo_shape(pNR, pShape);
        }
    }
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBoxDocPage::get_first_shape_for_layer(int layer)
{
    std::list<GmoShape*>::reverse_iterator it;
    for (it = m_allShapes.rbegin(); it != m_allShapes.rend(); ++it)
    {
        if ((*it)->get_layer() == layer)
            return *it;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBoxDocPage::find_shape_at(LUnits x, LUnits y)
{
    std::list<GmoShape*>::reverse_iterator it;
    for (it = m_allShapes.rbegin(); it != m_allShapes.rend(); ++it)
    {
        URect bbox = (*it)->get_bounds();
        if (bbox.contains(x, y))
            return *it;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBoxDocPage::find_shape_for_object(ImoStaffObj* pSO)
{
    std::list<GmoShape*>::iterator it;
    for (it = m_allShapes.begin(); it != m_allShapes.end(); ++it)
    {
        if ((*it)->was_created_by(pSO))
            return *it;
    }
    return NULL;
}

//---------------------------------------------------------------------------------------
GmoObj* GmoBoxDocPage::hit_test(LUnits x, LUnits y)
{
    GmoObj* pShape = find_shape_at(x, y);
    if (pShape)
        return pShape;

    GmoBox* pBox = find_inner_box_at(x, y);
    if (pBox)
	    return pBox;

    return NULL;
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::select_objects_in_rectangle(SelectionSet& selection,
                                                const URect& selRect,
                                                unsigned flags)
{
    bool fSomethingSelected = false;
    std::list<GmoShape*>::reverse_iterator it;
    for (it = m_allShapes.rbegin(); it != m_allShapes.rend(); ++it)
    {
        URect bbox = (*it)->get_bounds();
        if (selRect.contains(bbox))
        {
            selection.add(*it);
            fSomethingSelected = true;
        }
    }

    //if no objects in rectangle try to select clicked object
    if (!fSomethingSelected)
    {
        GmoShape* pShape = find_shape_at(selRect.get_x(), selRect.get_y());
        if (pShape)
            selection.add(pShape);
    }

}



//=======================================================================================
// GmoBoxDocument
//=======================================================================================
GmoBoxDocument::GmoBoxDocument(GraphicModel* pGModel, ImoObj* pCreatorImo)
    : GmoBox(GmoObj::k_box_document, pCreatorImo)
    , m_pLastPage(NULL)
    , m_pGModel(pGModel)
{
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBoxDocument::add_new_page()
{
    m_pLastPage = new GmoBoxDocPage(NULL);      //TODO creator imo?
    add_child_box(m_pLastPage);
    m_pLastPage->set_number(get_num_pages());
    return m_pLastPage;
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBoxDocument::get_page(int i)
{
    return dynamic_cast<GmoBoxDocPage*>(get_child_box(i));
}


//=======================================================================================
// GmoBoxDocPageContent
//=======================================================================================
GmoBoxDocPageContent::GmoBoxDocPageContent(ImoObj* pCreatorImo)
    : GmoBox(GmoObj::k_box_doc_page_content, pCreatorImo)
{
}


//=======================================================================================
// GmoStubScore
//=======================================================================================
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
//void GmoStubScore::AddToSelection(GmoObj* pGMO)
//{
//    if (pGMO->IsSelectable())
//        m_Selection.AddToSelection(pGMO);
//}
//
//---------------------------------------------------------------------------------------
//void GmoStubScore::RemoveFromSelection(GmoObj* pGMO)
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
