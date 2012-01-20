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

#ifndef __LOMSE_GM_BASIC_H__
#define __LOMSE_GM_BASIC_H__

#include "lomse_basic.h"
#include "lomse_observable.h"
#include "lomse_events.h"

#include <vector>
#include <list>
#include <ostream>
#include <map>
using namespace std;

namespace lomse
{

//forward declarations
class GmoObj;
class GmoBox;
class GmoBoxDocument;
class GmoBoxDocPage;
class GmoBoxScorePage;
class GmoBoxSlice;
class GmoBoxSystem;
class GmoShape;
class GmoShapeStaff;
class ImoContentObj;
class ImoNoteRest;
class ImoObj;
class ImoScore;
class ImoStaffObj;
class ImoStyle;
class Drawer;
struct RenderOptions;
class GmoLayer;
class SelectionSet;
class RefToGmo;
class MultiRefToGmo;
class Control;

//---------------------------------------------------------------------------------------
// GraphicModel: storage for the graphic objects
//
class GraphicModel
{
protected:
    GmoBoxDocument* m_root;
    bool m_fCanBeDrawn;
    bool m_modified;
    std::map<ImoNoteRest*, GmoShape*> m_noterestToShape;
    std::map<ImoObj*, RefToGmo*> m_imoToGmo;

public:
    GraphicModel();
    virtual ~GraphicModel();

    //accessors
    inline GmoBoxDocument* get_root() { return m_root; }
    int get_num_pages();
    GmoBoxDocPage* get_page(int i);
    inline void set_modified(bool value) { m_modified = value; }
    inline bool is_modified() { return m_modified; }

    //drawing
    inline void set_ready(bool value) { m_fCanBeDrawn = value; }
    void draw_page(int iPage, UPoint& origin, Drawer* pDrawer, RenderOptions& opt);
    void highlight_object(ImoStaffObj* pSO, bool value);

    //hit testing
    GmoObj* hit_test(int iPage, LUnits x, LUnits y);
    GmoShape* find_shape_at(int iPage, LUnits x, LUnits y);
    GmoBox* find_inner_box_at(int iPage, LUnits x, LUnits y);

    //selection
    void select_objects_in_rectangle(int iPage, SelectionSet& selection,
                                     const URect& selRect, unsigned flags=0);
    GmoShape* find_shape_for_object(ImoStaffObj* pSO);
    GmoShape* get_shape_for_noterest(ImoNoteRest* pNR);

    //creation
    void store_in_map_imo_shape(ImoNoteRest* pNR, GmoShape* pShape);
    void remove_from_map_imo_gmo(GmoBox* child);
    void add_to_map_imo_gmo(GmoBox* child);
    GmoShape* get_shape_for(ImoObj* pImo, int id=0);
    GmoBox* get_box_for(ImoObj* pImo);

    //tests
    void dump_page(int iPage, ostream& outStream);

protected:
    GmoObj* get_gmo_for(ImoObj* pImo, int id=0);

};

//---------------------------------------------------------------------------------------
//Abstract class to define the interface for objects to be stored in the map to track
//the shape/box for an ImoObj
class RefToGmo
{
public:
    RefToGmo() {}

    virtual GmoObj* get_gmo(int id) = 0;
    virtual bool is_simple_ref() = 0;
};

//---------------------------------------------------------------------------------------
//Abstract class from which all graphic objects must derive
class GmoObj
{
protected:
    int m_objtype;
    UPoint m_origin;        //Relative to DocPage, for boxes. Relative to owner box, for shapes
    USize m_size;
    unsigned int m_flags;
    ImoObj* m_pCreatorImo;
    GmoBox* m_pParentBox;

public:
    virtual ~GmoObj();

    //flag values
    enum {
        k_selected          = 0x0001,   //selected
        k_dirty             = 0x0002,   //dirty: modified since last "clear_dirty()": need to render it again
        k_children_dirty    = 0x0004,   //this is not dirty but some children are dirty
        k_highlighted       = 0x0008,   //highlighted
        k_hover             = 0x0010,   //mouse over
        k_in_link           = 0x0020,   //is part of a link
    };

    //selection
    inline bool is_selected() { return (m_flags & k_selected) != 0; }
    virtual void set_selected(bool value) { value ? m_flags |= k_selected
                                                  : m_flags &= ~k_selected; }

    //hover (mouse over)
    inline bool is_hover() { return (m_flags & k_hover) != 0; }
    virtual void set_hover(bool value) = 0;

    //in link
    inline bool is_in_link() { return (m_flags & k_in_link) != 0; }
    virtual void set_in_link(bool value) = 0;

    //dirty
    inline bool is_dirty() { return (m_flags & k_dirty) != 0; }
    void set_dirty(bool value);
    inline bool are_children_dirty() { return (m_flags & k_children_dirty) != 0; }
    void set_children_dirty(bool value);


    //clasification
    enum { k_box = 0,
                k_box_document=0, k_box_doc_page, k_box_doc_page_content,
                k_box_inline, k_box_link, k_box_paragraph,
                k_box_score_page, k_box_slice, k_box_slice_instr, k_box_system,
                k_box_control,
           k_shape,
                k_shape_accidentals, k_shape_accidental_sign,
                k_shape_barline,
                k_shape_beam, k_shape_brace,
                k_shape_bracket, k_shape_button,
                k_shape_clef, k_shape_dot, k_shape_fermata, k_shape_flag, k_shape_image,
                k_shape_invisible, k_shape_key_signature, k_shape_note, k_shape_notehead,
                k_shape_rectangle, k_shape_rest, k_shape_rest_glyph,
                k_shape_slur, k_shape_stem, k_shape_staff,
                k_shape_text, k_shape_time_signature, k_shape_tie,
                k_shape_time_signature_digit, k_shape_tuplet, k_shape_word,
            k_max
         };

    inline int get_gmobj_type() { return m_objtype; }
    inline bool is_item_main_box() { return m_objtype == k_box_score_page
                                         || m_objtype == k_box_paragraph
                                         || m_objtype == k_box_control
                                         ;
    }
    inline bool is_box() { return m_objtype >= k_box && m_objtype < k_shape; }
    inline bool is_shape() { return m_objtype >= k_shape; }

    //item main boxes
    inline bool is_box_score_page() { return m_objtype == k_box_score_page; }
    inline bool is_box_paragraph() { return m_objtype == k_box_paragraph; }
    inline bool is_box_control() { return m_objtype == k_box_control; }

    //other boxes
    inline bool is_box_document() { return m_objtype == k_box_document; }
    inline bool is_box_doc_page() { return m_objtype == k_box_doc_page; }
    inline bool is_box_doc_page_content() { return m_objtype == k_box_doc_page_content; }
    inline bool is_box_inline() { return m_objtype == k_box_inline; }
    inline bool is_box_link() { return m_objtype == k_box_link; }
    inline bool is_box_slice() { return m_objtype == k_box_slice; }
    inline bool is_box_slice_instr() { return m_objtype == k_box_slice_instr; }
    inline bool is_box_system() { return m_objtype == k_box_system; }

    inline bool is_shape_accidentals() { return m_objtype == k_shape_accidentals; }
    inline bool is_shape_accidental_sign() { return m_objtype == k_shape_accidental_sign; }
    inline bool is_shape_barline() { return m_objtype == k_shape_barline; }
    inline bool is_shape_beam() { return m_objtype == k_shape_beam; }
    inline bool is_shape_brace() { return m_objtype == k_shape_brace; }
    inline bool is_shape_bracket() { return m_objtype == k_shape_bracket; }
    inline bool is_shape_button() { return m_objtype == k_shape_button; }
    inline bool is_shape_clef() { return m_objtype == k_shape_clef; }
    inline bool is_shape_dot() { return m_objtype == k_shape_dot; }
    inline bool is_shape_fermata() { return m_objtype == k_shape_fermata; }
    inline bool is_shape_flag() { return m_objtype == k_shape_flag; }
    inline bool is_shape_image() { return m_objtype == k_shape_image; }
    inline bool is_shape_invisible() { return m_objtype == k_shape_invisible; }
    inline bool is_shape_key_signature() { return m_objtype == k_shape_key_signature; }
    inline bool is_shape_note() { return m_objtype == k_shape_note; }
    inline bool is_shape_notehead() { return m_objtype == k_shape_notehead; }
    inline bool is_shape_rectangle() { return m_objtype == k_shape_rectangle; }
    inline bool is_shape_rest() { return m_objtype == k_shape_rest; }
    inline bool is_shape_rest_glyph() { return m_objtype == k_shape_rest_glyph; }
    inline bool is_shape_slur() { return m_objtype == k_shape_slur; }
    inline bool is_shape_stem() { return m_objtype == k_shape_stem; }
    inline bool is_shape_staff() { return m_objtype == k_shape_staff; }
    inline bool is_shape_text() { return m_objtype == k_shape_text; }
    inline bool is_shape_tie() { return m_objtype == k_shape_tie; }
    inline bool is_shape_time_signature() { return m_objtype == k_shape_time_signature; }
    inline bool is_shape_time_signature_digit() { return m_objtype == k_shape_time_signature_digit; }
    inline bool is_shape_tuplet() { return m_objtype == k_shape_tuplet; }
    inline bool is_shape_word() { return m_objtype == k_shape_word; }

    //size
    inline LUnits get_width() { return m_size.width; }
    inline LUnits get_height() { return m_size.height; }
    inline void set_width(LUnits width) { m_size.width = width; }
    inline void set_height(LUnits height) { m_size.height = height; }

    //position
    inline LUnits get_left() { return m_origin.x; }
    inline LUnits get_top() { return m_origin.y; }
    inline LUnits get_right() { return m_origin.x + m_size.width; }
    inline LUnits get_bottom() { return m_origin.y + m_size.height; }
    void set_origin(UPoint& pos);
    void set_origin(const UPoint& pos);
    void set_origin(LUnits xLeft, LUnits yTop);
    void set_left(LUnits xLeft);
    void set_top(LUnits yTop);
    virtual void shift_origin(const USize& shift);
    void shift_origin(LUnits x, LUnits y);

    //bounds
    bool bounds_contains_point(UPoint& p);
    URect get_bounds();
    inline UPoint get_origin() { return m_origin; }
    inline USize get_size() { return m_size; }

    //creator
    inline bool was_created_by(ImoObj* pImo) { return m_pCreatorImo == pImo; }
    inline ImoObj* get_creator_imo() { return m_pCreatorImo; }

    //parent
    inline void set_owner_box(GmoBox* pBox) { m_pParentBox = pBox; };
    inline GmoBox* get_owner_box() { return m_pParentBox; };

    //tests & debug
    void dump(ostream& outStream, int level);
    static string& get_name(int objtype);

protected:
    GmoObj(int objtype, ImoObj* pCreatorImo);
    void propagate_dirty();

};

//---------------------------------------------------------------------------------------
class GmoShape : public GmoObj, public Linkable<USize>, public RefToGmo
{
protected:
    int m_idx;
    int m_layer;
	Color m_color;
    std::list<GmoShape*>* m_pRelatedShapes;

public:
    virtual ~GmoShape();

    //RefToGmo virtual methods
    GmoObj* get_gmo(int id) { return this; }
    bool is_simple_ref() { return true; }

    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);

    // layer identifiers. Shapes are placed in layers. The first layer to
    // render is layer 0 (background). Then, layer 1 (staves), and so on.
    enum { k_layer_background = 0, k_layer_staff, k_layer_barlines, k_layer_notes,
           k_layer_aux_objs, k_layer_top, k_layer_user,
           k_layer_max };

    //layer
    inline int get_layer() { return m_layer; }
    inline void set_layer(int layer) { m_layer = layer; }

    //overrides required by Linkable
	virtual void handle_link_event(Linkable<USize>* pShape, int type, USize shift);
    virtual void on_linked_to(Linkable<USize>* pShape, int type);

    virtual LUnits get_anchor_offset() { return 0.0f; }

    //methods related to position
    void set_origin_and_notify_observers(LUnits xLeft, LUnits yTop);

    //related shapes
    inline std::list<GmoShape*>* get_related_shapes() { return m_pRelatedShapes; }
    void add_related_shape(GmoShape* pShape);
    GmoShape* find_related_shape(int type);

    //highlight
    void highlight_off() { set_highlighted(false); };
    void highlight_on() { set_highlighted(true); };
    virtual	void set_highlighted(bool value) { value ? m_flags |= k_highlighted
                                                     : m_flags &= ~k_highlighted; }
    inline bool is_highlighted() { return (m_flags & k_highlighted) != 0; }

    //other flags
    void set_hover(bool value) { value ? m_flags |= k_hover : m_flags &= ~k_hover; }
    void set_in_link(bool value) { value ? m_flags |= k_in_link : m_flags &= ~k_in_link; }
    void set_flag_value(bool value, unsigned int flag) {
        value ? m_flags |= flag : m_flags &= ~flag;
    }

protected:
    GmoShape(ImoObj* pCreatorImo, int objtype, int idx, Color color);
    Color determine_color_to_use(RenderOptions& opt);
    virtual Color get_normal_color() { return m_color; }

};

//---------------------------------------------------------------------------------------
class GmoBox : public GmoObj, public RefToGmo
{
protected:
    std::vector<GmoBox*> m_childBoxes;
	std::list<GmoShape*> m_shapes;		    //contained shapes

    // All boxes have four margins (top, bottom, left and right) around the
    // box area (bounds rectangle). The margins define a smaller rectangle
    // (content rectangle) contained into the bounds rectangle. You can consider
    // the box as a content rectangle sorrounded blank space, defined by the margins.
    LUnits m_uTopMargin;
    LUnits m_uBottomMargin;
    LUnits m_uLeftMargin;
    LUnits m_uRightMargin;

public:
    virtual ~GmoBox();

    //RefToGmo virtual methods
    GmoObj* get_gmo(int id) { return this; }
    bool is_simple_ref() { return true; }

    //child boxes
    inline int get_num_boxes() { return static_cast<int>( m_childBoxes.size() ); }
    void add_child_box(GmoBox* child);
    GmoBox* get_child_box(int i);  //i = 0..n-1

    //contained shapes
    inline int get_num_shapes() { return static_cast<int>( m_shapes.size() ); }
    void add_shape(GmoShape* shape, int layer);
    GmoShape* get_shape(int i);  //i = 0..n-1

    //flags
    void set_hover(bool value) { set_flag_value(value, k_hover); }
    void set_in_link(bool value) { set_flag_value(value, k_in_link); }
    void set_flag_value(bool value, unsigned int flag);

    //maintaining references
    void add_shapes_to_tables();

    //margins
    inline LUnits get_top_margin() { return m_uTopMargin; }
    inline LUnits get_bottom_margin() { return m_uBottomMargin; }
    inline LUnits get_left_margin() { return m_uLeftMargin; }
    inline LUnits get_right_margin() { return m_uRightMargin; }
    inline void set_top_margin(LUnits space) { m_uTopMargin = space; }
    inline void set_bottom_margin(LUnits space) { m_uBottomMargin = space; }
    inline void set_left_margin(LUnits space) { m_uLeftMargin = space; }
    inline void set_right_margin(LUnits space) { m_uRightMargin = space; }

    //content size
    //old semantic, based on score margins
    inline LUnits get_content_width_old() { return get_width() - m_uLeftMargin - m_uRightMargin; }
    //new semantic, based on style
    LUnits get_content_top();
    LUnits get_content_left();
    LUnits get_content_width();
    LUnits get_content_height();

    //position
    void shift_origin(const USize& shift);
    inline void new_left(LUnits xLeft) { m_origin.x = xLeft; }
    inline void new_top(LUnits yTop) { m_origin.y = yTop; }
    inline void new_origin(UPoint& pos) { m_origin = pos; }

    //drawing
    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //hit testing
    GmoBox* find_inner_box_at(LUnits x, LUnits y);

    //model
    virtual GraphicModel* get_graphic_model();

    //tests
    void dump_boxes_shapes(ostream& outStream, int level);
    void dump_shapes(ostream& outStream, int level);

protected:
    GmoBox(int objtype, ImoObj* pCreatorImo);
    void delete_boxes();
    void delete_shapes();
    GmoBoxDocPage* get_parent_box_page();
    GmoBox* get_parent_box() { return m_pParentBox; }
    void draw_border(Drawer* pDrawer, RenderOptions& opt);
    bool must_draw_bounds(RenderOptions& opt);
    Color get_box_color();
    void draw_box_bounds(Drawer* pDrawer, double xorg, double yorg, Color& color);
    void draw_shapes(Drawer* pDrawer, RenderOptions& opt);
    void remove_from_map_imo_gmo(GmoBox* child);
    void add_to_map_imo_gmo(GmoBox* child);
    void add_shapes_to_tables_in(GmoBoxDocPage* pPage);

    virtual ImoStyle* get_style();

};

//---------------------------------------------------------------------------------------
class GmoBoxDocument : public GmoBox
{
protected:
    GmoBoxDocPage* m_pLastPage;
    GraphicModel* m_pGModel;

public:
    GmoBoxDocument(GraphicModel* pGModel, ImoObj* pCreatorImo);
    virtual ~GmoBoxDocument() {}

    //doc pages
    GmoBoxDocPage* add_new_page();
    GmoBoxDocPage* get_page(int i);     //i = 0..n-1
    inline int get_num_pages() { return get_num_boxes(); }
    inline GmoBoxDocPage* get_last_page() { return m_pLastPage; }

    //overrides
    GraphicModel* get_graphic_model() { return m_pGModel; }

};

//---------------------------------------------------------------------------------------
class GmoBoxDocPage : public GmoBox
{
protected:
    int m_numPage;
	std::list<GmoLayer*> m_Layers;     //contained shapes, ordered by layer
    std::list<GmoShape*> m_allShapes;		//contained shapes, ordered by creation order

public:
    GmoBoxDocPage(ImoObj* pCreatorImo);
    virtual ~GmoBoxDocPage() {}

    //page number
    inline void set_number(int num) { m_numPage = num; }
    inline int get_number() { return m_numPage; }

    //renderization
    void on_draw(Drawer* pDrawer, RenderOptions& opt);

    //shapes
    void add_to_tables(GmoShape* pShape);
    GmoShape* get_first_shape_for_layer(int order);
    GmoShape* find_shape_for_object(ImoStaffObj* pSO);
    void store_in_map_imo_shape(GmoShape* pShape);

    //hit testing
    GmoObj* hit_test(LUnits x, LUnits y);
    GmoShape* find_shape_at(LUnits x, LUnits y);

    //selection
    void select_objects_in_rectangle(SelectionSet& selection, const URect& selRect,
                                     unsigned flags=0);

protected:
    void draw_page_background(Drawer* pDrawer, RenderOptions& opt);
};

//---------------------------------------------------------------------------------------
class GmoBoxDocPageContent : public GmoBox
{
protected:

public:
    GmoBoxDocPageContent(ImoObj* pCreatorImo);
    virtual ~GmoBoxDocPageContent() {}

};

//---------------------------------------------------------------------------------------
class GmoBoxScorePage : public GmoBox
{
protected:
    int             m_nFirstSystem;     //0..n-1
    int             m_nLastSystem;      //0..n-1

public:
    GmoBoxScorePage(ImoScore* pScore);
    virtual ~GmoBoxScorePage();

	//systems
    void add_system(GmoBoxSystem* pSystem, int iSystem);
    inline int get_num_last_system() const { return m_nLastSystem; }
    inline int get_num_systems() {
        return (m_nFirstSystem == -1 ? 0 : m_nLastSystem - m_nFirstSystem + 1);
    }
	GmoBoxSystem* get_system(int iSystem);		//nSystem = 0..n-1
};

//---------------------------------------------------------------------------------------
class GmoBoxParagraph : public GmoBox
{
protected:

public:
    GmoBoxParagraph(ImoObj* pCreatorImo)
        : GmoBox(GmoObj::k_box_paragraph, pCreatorImo) {}
    virtual ~GmoBoxParagraph() {}
};

//---------------------------------------------------------------------------------------
class GmoBoxInline : public GmoBox
{
protected:

public:
    GmoBoxInline(ImoObj* pCreatorImo)
        : GmoBox(GmoObj::k_box_inline, pCreatorImo) {}
    virtual ~GmoBoxInline() {}
};

//---------------------------------------------------------------------------------------
class GmoBoxLink : public GmoBox
{
protected:

public:
    GmoBoxLink(ImoObj* pCreatorImo)
        : GmoBox(GmoObj::k_box_link, pCreatorImo) {}
    virtual ~GmoBoxLink() {}

    void notify_event(SpEventInfo pEvent);
};

//---------------------------------------------------------------------------------------
// A box for GUI controls
class GmoBoxControl : public GmoBox
{
protected:
    ImoStyle*   m_pStyle;
    Control*    m_pControl;

public:
    GmoBoxControl(Control* ctrl, const UPoint& origin, LUnits width, LUnits height,
                 ImoStyle* style=NULL, ImoObj* pCreatorImo=NULL);

    virtual ~GmoBoxControl() {}

    inline void set_style(ImoStyle* pStyle) { m_pStyle = pStyle; }
    void notify_event(SpEventInfo pEvent);

    //GmoBox override
    void on_draw(Drawer* pDrawer, RenderOptions& opt);

protected:
    //overrides
    ImoStyle* get_style() { return m_pStyle; }
};


//---------------------------------------------------------------------------------------
//Derived class, to contain a list of shapes
class MultiRefToGmo : public RefToGmo
{
protected:
    list<GmoShape*> m_shapes;

public:
    MultiRefToGmo(GmoShape* pShape);
    ~MultiRefToGmo();

    GmoObj* get_gmo(int id);
    bool is_simple_ref() { return false; }
};


}   //namespace lomse

#endif      //__LOMSE_GM_BASIC_H__
