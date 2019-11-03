//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_gm_basic.h"

#include "lomse_graphical_model.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_drawer.h"
#include "lomse_selections.h"
#include "lomse_time.h"
#include "lomse_control.h"
#include "lomse_box_system.h"
#include "lomse_logger.h"
#include "lomse_gm_measures_table.h"

#include <cstdlib>      //abs
#include <iomanip>


namespace lomse
{


//=======================================================================================
// GmoObj implementation
//=======================================================================================

//association object-type <-> object-name
static std::map<int, std::string> m_typeToName;
static bool m_fNamesLoaded = false;
static string m_unknown = "unknown";

//---------------------------------------------------------------------------------------
GmoObj::GmoObj(int objtype, ImoObj* pCreatorImo)
    : m_objtype(objtype)
    , m_origin(0.0f, 0.0f)
    , m_size(0.0f, 0.0f)
    , m_flags(k_dirty | k_add_to_vprofile)
    , m_pCreatorImo(pCreatorImo)
    , m_pParentBox(nullptr)
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
void GmoObj::set_origin(const UPoint& pos)
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
void GmoObj::shift_origin(LUnits x, LUnits y)
{
    m_origin.x += x;
    m_origin.y += y;
}

//---------------------------------------------------------------------------------------
void GmoObj::dump(ostream& outStream, int level)
{
    std::ios_base::fmtflags f( outStream.flags() );  //save formating options

    outStream << setw(level*3) << level << " [" << setw(3) << m_objtype << "] "
              << get_name(m_objtype)
              << fixed << setprecision(2) << setfill(' ')
              << setw(10) << round_half_up(m_origin.x) << ", "
              << setw(10) << round_half_up(m_origin.y) << ", "
              << setw(10) << round_half_up(m_size.width) << ", "
              << setw(10) << round_half_up(m_size.height) << endl;

    outStream.flags( f );  //restore formating options
}

//---------------------------------------------------------------------------------------
const string& GmoObj::get_name(int objtype)
{
    if (!m_fNamesLoaded)
    {
        m_typeToName[k_box]                     = "box (A)        ";
        m_typeToName[k_box_control]             = "box-control    ";
        m_typeToName[k_box_document]            = "box-document   ";
        m_typeToName[k_box_doc_page]            = "box-doc-page   ";
        m_typeToName[k_box_doc_page_content]    = "box-docpg-cont.";
        m_typeToName[k_box_inline]              = "box-inline     ";
        m_typeToName[k_box_link]                = "box-link       ";
        m_typeToName[k_box_paragraph]           = "box-paragraph  ";
        m_typeToName[k_box_score_page]          = "box-score-page ";
        m_typeToName[k_box_slice]               = "box-slice      ";
        m_typeToName[k_box_slice_instr]         = "box-slice-intr ";
        m_typeToName[k_box_slice_staff]         = "box-slice-staff";
        m_typeToName[k_box_system]              = "box-system     ";
        m_typeToName[k_box_table]               = "box-table      ";
        m_typeToName[k_box_table_rows]          = "box-table-rows ";

        // shapes
        m_typeToName[k_shape]                   = "shape (A)      ";
        m_typeToName[k_shape_accidentals]       = "accidentals    ";
        m_typeToName[k_shape_accidental_sign]   = "accidental-sign";
        m_typeToName[k_shape_articulation]      = "articulation   ";
        m_typeToName[k_shape_barline]           = "barline        ";
        m_typeToName[k_shape_beam]              = "beam           ";
        m_typeToName[k_shape_brace]             = "brace          ";
        m_typeToName[k_shape_bracket]           = "bracket        ";
        m_typeToName[k_shape_button]            = "button         ";
        m_typeToName[k_shape_clef]              = "clef           ";
        m_typeToName[k_shape_coda_segno]        = "coda-segno     ";
        m_typeToName[k_shape_debug]             = "shape-debug    ";
        m_typeToName[k_shape_dot]               = "dot            ";
        m_typeToName[k_shape_dynamics_mark]     = "dynamics-mark  ";
        m_typeToName[k_shape_fermata]           = "fermata        ";
        m_typeToName[k_shape_flag]              = "flag           ";
        m_typeToName[k_shape_image]             = "image          ";
        m_typeToName[k_shape_invisible]         = "invisible      ";
        m_typeToName[k_shape_key_signature]     = "key            ";
        m_typeToName[k_shape_line]              = "line           ";
        m_typeToName[k_shape_lyrics]            = "lyrics         ";
        m_typeToName[k_shape_metronome_glyph]   = "metronome-glyph";
        m_typeToName[k_shape_metronome_mark]    = "metronome-mark ";
        m_typeToName[k_shape_note]              = "note           ";
        m_typeToName[k_shape_chord_base_note]   = "base-note      ";
        m_typeToName[k_shape_notehead]          = "notehead       ";
        m_typeToName[k_shape_octave_shift]      = "octave-shift   ";
        m_typeToName[k_shape_octave_glyph]      = "octave-glyph   ";
        m_typeToName[k_shape_ornament]          = "ornament       ";
        m_typeToName[k_shape_rectangle]         = "rectangle      ";
        m_typeToName[k_shape_rest]              = "rest           ";
        m_typeToName[k_shape_rest_glyph]        = "rest-glyph     ";
        m_typeToName[k_shape_slur]              = "slur           ";
        m_typeToName[k_shape_stem]              = "stem           ";
        m_typeToName[k_shape_squared_bracket]   = "squared-bracket";
        m_typeToName[k_shape_staff]             = "staff          ";
        m_typeToName[k_shape_technical]         = "technical      ";
        m_typeToName[k_shape_text]              = "text           ";
        m_typeToName[k_shape_text_box]          = "text-box       ";
        m_typeToName[k_shape_time_signature]    = "time           ";
        m_typeToName[k_shape_tie]               = "tie            ";
        m_typeToName[k_shape_time_signature_glyph]
                                                = "time-glyph     ";
        m_typeToName[k_shape_tuplet]            = "tuplet         ";
        m_typeToName[k_shape_volta_bracket]     = "volta-bracket  ";
        m_typeToName[k_shape_word]              = "word           ";
        m_typeToName[k_shape_wedge]             = "wedge          ";

        m_fNamesLoaded = true;
    }

	map<int, std::string>::const_iterator it = m_typeToName.find( objtype );
	if (it != m_typeToName.end())
		return it->second;
    else
        return m_unknown;
}

//---------------------------------------------------------------------------------------
void GmoObj::set_dirty(bool dirty)
{
    if (dirty)
    {
        m_flags |= k_dirty;
        propagate_dirty();
    }
    else
        m_flags &= ~k_dirty;
}

//---------------------------------------------------------------------------------------
void GmoObj::set_children_dirty(bool value)
{
    value ? m_flags |= k_children_dirty : m_flags &= ~k_children_dirty;
}

//---------------------------------------------------------------------------------------
void GmoObj::propagate_dirty()
{
    if (m_pParentBox)
    {
        m_pParentBox->set_children_dirty(true);
        m_pParentBox->propagate_dirty();
    }

    if (this->is_box_document())
    {
        GmoBoxDocument* pBox = static_cast<GmoBoxDocument*>(this);
        pBox->get_graphic_model()->set_modified(true);
    }
}

//---------------------------------------------------------------------------------------
GmoRef GmoObj::get_ref()
{
    if (this->is_box())
    {
        ImoObj* pImo = get_creator_imo();
        if (pImo)
        {
            if (this->is_box_control() || pImo->is_mouse_over_generator())
            {
                ImoId id = pImo->get_id();
                ImoId idg = 0;
                if (is_box_control())
                {
                    Control* pControl =
                        static_cast<GmoBoxControl*>(this)->get_creator_control();
                    idg = pControl->get_control_id();
                }

                return make_pair(id, idg);
            }
        }
    }
    return k_no_gmo_ref;
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoObj::get_page_box()
{
    if (is_box_doc_page())
        return static_cast<GmoBoxDocPage*>(this);
    else
    {
        GmoBox* pParent = get_owner_box();
        return pParent->get_page_box();
    }
}



//=======================================================================================
// GmoBox
//=======================================================================================
GmoBox::GmoBox(int objtype, ImoObj* pCreatorImo)
    : GmoObj(objtype, pCreatorImo)
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
        return nullptr;
}

//---------------------------------------------------------------------------------------
void GmoBox::add_shape(GmoShape* shape, int layer)
{
    shape->set_layer(layer);
    shape->set_owner_box(this);
    m_shapes.push_back(shape);
}

//---------------------------------------------------------------------------------------
void GmoBox::add_shapes_to_tables_in(GmoBoxDocPage* pPage)
{
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        pPage->add_to_tables(*itS);

    std::vector<GmoBox*>::iterator itB;
    for (itB=m_childBoxes.begin(); itB != m_childBoxes.end(); ++itB)
        (*itB)->add_shapes_to_tables_in(pPage);
}

//---------------------------------------------------------------------------------------
void GmoBox::add_shapes_to_tables()
{
    GmoBoxDocPage* pPage = get_parent_doc_page();
    GmoBox* pBox = this;        //gcc complains if next method is invoked directly
    pBox->add_shapes_to_tables_in(pPage);
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBox::get_parent_doc_page()
{
    if (this->is_box_doc_page())
        return dynamic_cast<GmoBoxDocPage*>(this);
    else
        return get_parent_box()->get_parent_doc_page();
}

//---------------------------------------------------------------------------------------
GraphicModel* GmoBox::get_graphic_model()
{
    GmoBox* pParent = get_parent_box();
    return (pParent == nullptr ? nullptr : pParent->get_graphic_model());
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBox::get_shape(int i)  //i = 0..n-1
{
    std::list<GmoShape*>::iterator it = m_shapes.begin();
    for (; it != m_shapes.end() && i > 0; ++it, --i);
    if (it != m_shapes.end())
        return *it;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void GmoBox::set_flag_value(bool value, unsigned int flag)
{
    //set flag
    if (value)
        m_flags |= flag;
    else
        m_flags &= ~flag;

    //propagate to contained shapes
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        (*itS)->set_flag_value(value, flag);

    //propagate to contained boxes
    std::vector<GmoBox*>::iterator itB;
    for (itB=m_childBoxes.begin(); itB != m_childBoxes.end(); ++itB)
        (*itB)->set_flag_value(value, flag);
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
    ImoStyle* pStyle = get_style();
    if (pStyle && (pStyle->border_width_top() > 0.0f
                    || pStyle->border_width_bottom() > 0.0f
                    || pStyle->border_width_left() > 0.0f
                    || pStyle->border_width_right() > 0.0f) )
    {
        double xLeft = double(m_origin.x
                        + pStyle->margin_left());
        double yTop = double(m_origin.y
                        + pStyle->margin_top());
        double xRight = double(get_right()
                        - pStyle->margin_right());
        double yBottom = double(get_bottom()
                            - pStyle->margin_bottom());

        pDrawer->begin_path();
        pDrawer->fill( pStyle->background_color() );
        pDrawer->stroke( Color(0,0,0) );            //TODO: border color
        pDrawer->move_to(xLeft, yTop);

        //top border
        if (pStyle->border_width_top() > 0.0f)
        {
            pDrawer->stroke_width( pStyle->border_width_top() );
            pDrawer->hline_to(xRight);
        }
        else
            pDrawer->move_to(xRight, yTop);

        //right border
        if (pStyle->border_width_right() > 0.0f)
        {
            pDrawer->stroke_width( pStyle->border_width_right() );
            pDrawer->vline_to(yBottom);
        }
        else
            pDrawer->move_to(xRight, yBottom);

        //bottom border
        if (pStyle->border_width_bottom() > 0.0f)
        {
            pDrawer->stroke_width( pStyle->border_width_bottom() );
            pDrawer->hline_to(xLeft);
        }
        else
            pDrawer->move_to(xLeft, yBottom);

        //left border
        if (pStyle->border_width_left() > 0.0f)
        {
            pDrawer->stroke_width( pStyle->border_width_left() );
            pDrawer->vline_to(yTop);
        }

        pDrawer->end_path();
    }

    if (this->is_item_main_box() && opt.draw_focus_lines_on_boxes_flag == true)
    {
        double xorg = m_origin.x;
        double yorg = m_origin.y;
        Color color = opt.unfocussed_box_color;
        draw_box_bounds(pDrawer, xorg, yorg, color);
    }

    else if (must_draw_bounds(opt))
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
    return opt.must_draw_box_for( get_gmobj_type() );
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
    return nullptr;
}

//---------------------------------------------------------------------------------------
void GmoBox::shift_origin_and_content(const USize& shift)
{
    if (shift.width == 0.0f && shift.height == 0.0f) return;

    m_origin.x += shift.width;
    m_origin.y += shift.height;

    //shift contained boxes
    std::vector<GmoBox*>::iterator itB;
    for (itB=m_childBoxes.begin(); itB != m_childBoxes.end(); ++itB)
        (*itB)->shift_origin_and_content(shift);

    //shift contained shapes
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        (*itS)->shift_origin(shift);
}

//---------------------------------------------------------------------------------------
void GmoBox::dump_boxes_shapes(ostream& outStream, int level)
{
    dump(outStream, level);

    dump_shapes(outStream, ++level);

    //dump contained boxes
    std::vector<GmoBox*>::iterator it;
    for (it=m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
        (*it)->dump_boxes_shapes(outStream, level);
}

//---------------------------------------------------------------------------------------
void GmoBox::dump_shapes(ostream& outStream, int level)
{
    std::list<GmoShape*>::iterator itS;
    for (itS=m_shapes.begin(); itS != m_shapes.end(); ++itS)
        (*itS)->dump(outStream, level);
}

//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_top()
{
    ImoStyle* pStyle = get_style();
    if (pStyle)
        return get_top() + pStyle->margin_top()
                + pStyle->border_width_top()
                + pStyle->padding_top();
    else
        return get_top();
}

//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_left()
{
    ImoStyle* pStyle = get_style();
    if (pStyle)
        return get_left() + pStyle->margin_left()
                + pStyle->border_width_left()
                + pStyle->padding_left();
    else
        return get_left();
}

//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_width()
{
    ImoStyle* pStyle = get_style();
    if (pStyle)
        return get_width() - pStyle->margin_left()
                - pStyle->border_width_left()
                - pStyle->padding_left()
                - pStyle->margin_right()
                - pStyle->border_width_right()
                - pStyle->padding_right();
    else
        return get_width();
}

//---------------------------------------------------------------------------------------
LUnits GmoBox::get_content_height()
{
    ImoStyle* pStyle = get_style();
    if (pStyle)
        return get_height() - pStyle->margin_top()
                - pStyle->border_width_top()
                - pStyle->padding_top()
                - pStyle->margin_bottom()
                - pStyle->border_width_bottom()
                - pStyle->padding_bottom();
    else
        return get_height();
}

//---------------------------------------------------------------------------------------
ImoStyle* GmoBox::get_style()
{
    if (m_pCreatorImo && m_pCreatorImo->is_contentobj())
    {
        ImoContentObj* pImo = dynamic_cast<ImoContentObj*>(m_pCreatorImo);
        if (pImo)
            return pImo->get_style();
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
void GmoBox::add_boxes_to_controls_map(GraphicModel* pGM)
{
    pGM->add_to_map_ref_to_box(this);

    vector<GmoBox*>::iterator it;
    for (it = m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
    {
        (*it)->add_boxes_to_controls_map(pGM);
    }
}

//---------------------------------------------------------------------------------------
void GmoBox::add_boxes_to_map_imo_to_box(GraphicModel* pGM)
{
    //filter out boxes for non-terminal elements
    if (this->is_box_control()
        || this->is_box_document()
        || this->is_box_doc_page()
        || this->is_box_doc_page_content()
        || this->is_box_paragraph()
        || this->is_box_score_page()
        || this->is_box_table()
       )
    {
        pGM->add_to_map_imo_to_box(this);

        vector<GmoBox*>::iterator it;
        for (it = m_childBoxes.begin(); it != m_childBoxes.end(); ++it)
        {
            (*it)->add_boxes_to_map_imo_to_box(pGM);
        }
    }
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
//    return nullptr;
//}




//=======================================================================================
// GmoBoxDocPage
//=======================================================================================
GmoBoxDocPage::GmoBoxDocPage(ImoObj* pCreatorImo)
    : GmoBox(GmoObj::k_box_doc_page, pCreatorImo)
    , m_numPage(1)
{
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    draw_page_background(pDrawer, opt);
    GmoBox::on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::draw_page_background(Drawer* pDrawer, RenderOptions& UNUSED(opt))
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
void GmoBoxDocPage::add_to_tables(GmoShape* pShape)
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
    ImoObj* pImo = pShape->get_creator_imo();
    if (pImo)
    {
        GraphicModel* pModel = get_graphic_model();
        if (pModel)
            pModel->store_in_map_imo_shape(pImo, pShape);
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
    return nullptr;
}

//---------------------------------------------------------------------------------------
GmoShape* GmoBoxDocPage::find_shape_at(LUnits x, LUnits y)
{
    std::list<GmoShape*>::reverse_iterator it;
    for (it = m_allShapes.rbegin(); it != m_allShapes.rend(); ++it)
    {
        if ((*it)->hit_test(x, y))
            return *it;
    }
    return nullptr;
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
    return nullptr;
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

    return nullptr;
}

//---------------------------------------------------------------------------------------
void GmoBoxDocPage::select_objects_in_rectangle(SelectionSet* selection,
                                                const URect& selRect,
                                                unsigned UNUSED(flags))
{
    bool fSomethingSelected = false;
    std::list<GmoShape*>::reverse_iterator it;
    for (it = m_allShapes.rbegin(); it != m_allShapes.rend(); ++it)
    {
        URect bbox = (*it)->get_bounds();
        if (selRect.contains(bbox))
        {
            selection->add(*it);
            fSomethingSelected = true;
        }
    }

    //if no objects in rectangle try to select clicked object
    if (!fSomethingSelected)
    {
        GmoShape* pShape = find_shape_at(selRect.get_x(), selRect.get_y());
        if (pShape)
            selection->add(pShape);
    }

}



//=======================================================================================
// GmoBoxDocument
//=======================================================================================
GmoBoxDocument::GmoBoxDocument(GraphicModel* pGModel, ImoObj* pCreatorImo)
    : GmoBox(GmoObj::k_box_document, pCreatorImo)
    , m_pLastPage(nullptr)
    , m_pGModel(pGModel)
{
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBoxDocument::add_new_page()
{
    m_pLastPage = LOMSE_NEW GmoBoxDocPage(nullptr);      //TODO creator imo?
    add_child_box(m_pLastPage);
    m_pLastPage->set_number(get_num_pages());
    return m_pLastPage;
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* GmoBoxDocument::get_page(int i)
{
    return dynamic_cast<GmoBoxDocPage*>(get_child_box(i));
}

//---------------------------------------------------------------------------------------
int GmoBoxDocument::get_page_number(GmoBoxDocPage* pBoxPage)
{
    vector<GmoBox*>& pages = get_child_boxes();
    int iMax = int(pages.size());
    for (int i=0; i < iMax; ++i)
    {
        if (pages[i] == pBoxPage)
            return i;
    }
    return -1;
}


//=======================================================================================
// GmoBoxDocPageContent
//=======================================================================================
GmoBoxDocPageContent::GmoBoxDocPageContent(ImoObj* pCreatorImo)
    : GmoBox(GmoObj::k_box_doc_page_content, pCreatorImo)
{
}


//=======================================================================================
// GmoBoxControl
//=======================================================================================
GmoBoxControl::GmoBoxControl(Control* ctrl, const UPoint& origin,
                             LUnits width, LUnits height, ImoStyle* style)
    : GmoBox(GmoObj::k_box_control, ctrl->get_owner_imo() )
    , m_pStyle(style)
    , m_pControl(ctrl)
{
    set_origin(origin);
    set_width(width);
    set_height(height);
}

//---------------------------------------------------------------------------------------
void GmoBoxControl::on_draw(Drawer* pDrawer, RenderOptions& opt)
{
    GmoBox::on_draw(pDrawer, opt);

    if (m_pControl)
        m_pControl->on_draw(pDrawer, opt);
}

//---------------------------------------------------------------------------------------
void GmoBoxControl::notify_event(SpEventInfo pEvent)
{
    if (m_pControl)
        m_pControl->handle_event(pEvent);
}


//=======================================================================================
// GmoBoxLink
//=======================================================================================
void GmoBoxLink::notify_event(SpEventInfo pEvent)
{
    if (pEvent->is_mouse_in_event())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "set hover true");
        set_hover(true);
        set_dirty(true);
    }
    else if (pEvent->is_mouse_out_event())
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "set hover false");
        set_hover(false);
        set_dirty(true);
    }
    else if (pEvent->is_on_click_event())
    {
        LOMSE_LOG_ERROR("is_on_click_event: TODO");
        //AWARE: should not reach this point as click events on links
        //will be dispatched to the global handler.
        //TODO: GmoBoxLink::notify_event, on_click_event
//        m_visited = true;
//        m_prevColor = m_visitedColor;
    }
    else
    {
        LOMSE_LOG_DEBUG(Logger::k_events, "event ignored");
    }
}


//=======================================================================================
// ScoreStub implementation
//=======================================================================================
ScoreStub::ScoreStub(ImoScore* pScore)
    : m_scoreId(pScore->get_id())
{
    m_measures = LOMSE_NEW GmMeasuresTable(pScore);
}

//---------------------------------------------------------------------------------------
ScoreStub::~ScoreStub()
{
    delete m_measures;
}

//---------------------------------------------------------------------------------------
GmoBoxScorePage* ScoreStub::get_page_for(TimeUnits timepos)
{
    //find page with end time greater or equal than requested time

    int maxPage = int(m_pages.size());
    int i=0;
    for (; i < maxPage; ++i)
    {
//        LOMSE_LOG_DEBUG(Logger::k_events, "page %d. End time = %f",
//                        i, m_pages[i]->end_time());

        //BUG-BYPASS: first page could be empty when the score is embedded in a text and
        // space in current page is small. In these cases, an score page is allocated
        // at the end of current document page, but as there is not enough space for
        // the score, a new document page is created and a new score page is allocated.
        // This causes the previously allocated score page to remain without content.
        // This is a bug bypass. The real solution would be not to allocate the unused
        // page or to remove it from ScoreStub.
        if (m_pages[i]->get_num_systems() == 0)
            continue;

        if (is_lower_time(timepos, m_pages[i]->end_time()))
            break;
        else if(is_equal_time(timepos, m_pages[i]->end_time()))
        {
            //look in next page
            int iNext = i + 1;
            if (iNext < maxPage)
            {
                if (is_equal_time(timepos, m_pages[iNext]->start_time()))
                    ++i;
            }
            break;
        }
    }
    if (i == maxPage)
        return nullptr;
    else
        return m_pages[i];
}


}  //namespace lomse
