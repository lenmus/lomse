//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class GmoObj;
class GmoBox;
class GmoBoxDocument;
class GmoBoxDocPage;
class GmoBoxScorePage;
class GmoBoxSlice;
class GmoBoxSliceInstr;
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
class Control;
class ScoreStub;
class GraphicModel;
class GmMeasuresTable;


///@cond INTERNALS
//excluded from public API. Only for internal use.

//---------------------------------------------------------------------------------------
/** %ScoreStub is a helper class containing information related to graphical model for
    one score.

    As the graphic model is a general model, it has no specific information about the
    details for each score graphic model. Therefore, to store and manage tables and
    other information related to each score graphic model, this helper class
    is used.
*/
class ScoreStub
{
protected:
    ImoId m_scoreId;
    std::vector<GmoBoxScorePage*> m_pages;
    GmMeasuresTable* m_measures;

public:
    ScoreStub(ImoScore* pScore);
    ~ScoreStub();

    inline void add_page(GmoBoxScorePage* pPage) { m_pages.push_back(pPage); }
    inline std::vector<GmoBoxScorePage*>& get_pages() { return m_pages; }

    /** Returns the GmoBoxScorePage containing timepos @c time. If @c time is not in
        the score, returns @nullptr. This method gives preference to find pages for
        events instead of non-timed staff objects. For example, the last
        barline in one page has the same timepos than the first event in next page.
        Therefore, as there exist two pages containing the same timepos, this method
        will return the second page.
        @param time The time position (absolute time units) for the requested page.
    */
    GmoBoxScorePage* get_page_for(TimeUnits time);

    /** Returns the table of measures for this score */
    inline GmMeasuresTable* get_measures_table() { return m_measures; }

};
///@endcond


//---------------------------------------------------------------------------------------
/** Abstract class from which all graphical objects derive. All graphical objects
    have a bounding box and a position, and know how to draw themselves.
*/
class GmoObj
{
protected:
    int m_objtype;
    UPoint m_origin;        //Relative to DocPage, for boxes. Relative to owner box, for shapes
    USize m_size;
    unsigned int m_flags;
    ImoObj* m_pCreatorImo;
    GmoBox* m_pParentBox;

    GmoObj(int objtype, ImoObj* pCreatorImo);

public:
    ///@cond INTERNALS
    //excluded from public API. Only for internal use.
    virtual ~GmoObj();

    ///@endcond


    //size and position
    /// @name Size and position
    //@{

    /** Returns the width of the bounding box, in logical units. */
    inline LUnits get_width() { return m_size.width; }

    /** Returns the height of the bounding box, in logical units. */
    inline LUnits get_height() { return m_size.height; }

    /** Returns the x coordinate for the left border of the bounding box. The returned
        value is in logical units, relative to top left corner of GmoDocPage containing
        this object.
    */
    LUnits get_left() const { return m_origin.x; }

    /** Returns the y coordinate for the top border of the bounding box. The returned
        value is in logical units, relative to top left corner of GmoDocPage containing
        this object.
    */
    LUnits get_top() const { return m_origin.y; }

    /** Returns the x coordinate for the right border of the bounding box. The returned
        value is in logical units, relative to top left corner of GmoDocPage containing
        this object.
    */
    LUnits get_right() const { return m_origin.x + m_size.width; }

    /** Returns the y coordinate for the bottom border of the bounding box. The returned
        value is in logical units, relative to top left corner of GmoDocPage containing
        this object.
    */
    LUnits get_bottom() const { return m_origin.y + m_size.height; }

    /** Returns @true if the point is inside this object bounding box rectangle.
        @param p The point to be tested, in logical units.
    */
    bool bounds_contains_point(UPoint& p);

    /** Returns the bounding box rectangle, in logical units. Origin (top left corner)
        is relative to top left corner of GmoDocPage containing this object.
    */
    URect get_bounds();

    /** Returns the top left corner of the bounding box rectangle. The point is in
        logical units relative to top left corner of GmoDocPage containing this object.
    */
    inline UPoint get_origin() { return m_origin; }

    /** Returns the width and heigh of the bounding box rectangle (in logical units).
    */
    inline USize get_size() { return m_size; }

    //@}    //size and position


    ///@cond INTERNALS
    //excluded from public API. Only for internal use.

    //flag values
    enum {
        //temporary flags
        k_dirty             = 0x0002,   //dirty: modified since last "clear_dirty()": need to render it again
        k_children_dirty    = 0x0004,   //this is not dirty but some children are dirty
        k_hover             = 0x0008,   //mouse over
        k_has_edit_focus    = 0x0010,   //this box has the focus for edition

        //structural flags
        k_in_link           = 0x0100,   //is part of a link
        k_add_to_vprofile   = 0x0200,   //add to VProfile
    };


    //info
    GmoRef get_ref();

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

    //do not add to VProfile
    inline bool add_to_vprofile() { return (m_flags & k_add_to_vprofile) != 0; }
    void set_add_to_vprofile(bool value) {
        value ? m_flags |= k_add_to_vprofile : m_flags &= ~k_add_to_vprofile;
    }


    //clasification
    enum { k_box = 0,
                k_box_document=0, k_box_doc_page, k_box_doc_page_content,
                k_box_inline, k_box_link, k_box_paragraph,
                k_box_score_page, k_box_slice, k_box_slice_instr,
                k_box_slice_staff, k_box_system,
                k_box_control, k_box_table, k_box_table_rows,
           k_shape,
                k_shape_accidentals, k_shape_accidental_sign,
                k_shape_arpeggio,
                k_shape_articulation,
                k_shape_barline, k_shape_beam, k_shape_brace,
                k_shape_bracket, k_shape_clef, k_shape_coda_segno,
                k_shape_debug, k_shape_dot, k_shape_dynamics_mark,
                k_shape_fermata, k_shape_fingering_box, k_shape_fingering,
                k_shape_flag, k_shape_grace_stroke, k_shape_image,
                k_shape_invisible, k_shape_key_signature, k_shape_line, k_shape_lyrics,
                k_shape_metronome_glyph, k_shape_metronome_mark,
                k_shape_note, k_shape_chord_base_note, k_shape_notehead,
                k_shape_octave_shift, k_shape_octave_glyph, k_shape_ornament,
                k_shape_pedal_glyph, k_shape_pedal_line,
                k_shape_rectangle, k_shape_rest, k_shape_rest_glyph,
                k_shape_slur, k_shape_squared_bracket,
                k_shape_staff, k_shape_stem,
                k_shape_technical,
                k_shape_text,
                k_shape_text_box,
                k_shape_tie, k_shape_time_signature_glyph,
                k_shape_time_signature, k_shape_tuplet,
                k_shape_volta_bracket, k_shape_wedge, k_shape_word,
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
    inline bool is_box_control() { return m_objtype == k_box_control; }
    inline bool is_box_score_page() { return m_objtype == k_box_score_page; }
    inline bool is_box_paragraph() { return m_objtype == k_box_paragraph; }
    inline bool is_box_table() { return m_objtype == k_box_table; }

    //other boxes
    inline bool is_box_document() { return m_objtype == k_box_document; }
    inline bool is_box_doc_page() { return m_objtype == k_box_doc_page; }
    inline bool is_box_doc_page_content() { return m_objtype == k_box_doc_page_content; }
    inline bool is_box_inline() { return m_objtype == k_box_inline; }
    inline bool is_box_link() { return m_objtype == k_box_link; }
    inline bool is_box_slice() { return m_objtype == k_box_slice; }
    inline bool is_box_slice_instr() { return m_objtype == k_box_slice_instr; }
    inline bool is_box_slice_staff() { return m_objtype == k_box_slice_staff; }
    inline bool is_box_system() { return m_objtype == k_box_system; }
    inline bool is_box_table_rows() { return m_objtype == k_box_table_rows; }

    bool is_shape_arpeggio() const { return m_objtype == k_shape_arpeggio; }
    inline bool is_shape_articulation() { return m_objtype == k_shape_articulation; }
    inline bool is_shape_accidentals() { return m_objtype == k_shape_accidentals; }
    inline bool is_shape_accidental_sign() { return m_objtype == k_shape_accidental_sign; }
    inline bool is_shape_barline() { return m_objtype == k_shape_barline; }
    inline bool is_shape_beam() { return m_objtype == k_shape_beam; }
    inline bool is_shape_brace() { return m_objtype == k_shape_brace; }
    inline bool is_shape_bracket() { return m_objtype == k_shape_bracket; }
    inline bool is_shape_clef() { return m_objtype == k_shape_clef; }
    inline bool is_shape_coda_segno() { return m_objtype == k_shape_coda_segno; }
    inline bool is_shape_debug() { return m_objtype == k_shape_debug; }
    inline bool is_shape_dot() { return m_objtype == k_shape_dot; }
    inline bool is_shape_dynamics_mark() { return m_objtype == k_shape_dynamics_mark; }
    inline bool is_shape_fermata() { return m_objtype == k_shape_fermata; }
    inline bool is_shape_fingering_box() { return m_objtype == k_shape_fingering_box; }
    inline bool is_shape_fingering() { return m_objtype == k_shape_fingering; }
    inline bool is_shape_flag() { return m_objtype == k_shape_flag; }
    inline bool is_shape_grace_stroke() { return m_objtype == k_shape_grace_stroke; }
    inline bool is_shape_image() { return m_objtype == k_shape_image; }
    inline bool is_shape_invisible() { return m_objtype == k_shape_invisible; }
    inline bool is_shape_key_signature() { return m_objtype == k_shape_key_signature; }
    inline bool is_shape_line() { return m_objtype == k_shape_line; }
    inline bool is_shape_lyrics() { return m_objtype == k_shape_lyrics; }
    inline bool is_shape_metronome_glyph() { return m_objtype == k_shape_metronome_glyph; }
    inline bool is_shape_metronome_mark() { return m_objtype == k_shape_metronome_mark; }
    inline bool is_shape_note() { return m_objtype == k_shape_note
                                      || m_objtype == k_shape_chord_base_note; }
    inline bool is_shape_chord_base_note() { return m_objtype == k_shape_chord_base_note; }
    inline bool is_shape_notehead() { return m_objtype == k_shape_notehead; }
    inline bool is_shape_octave_shift() { return m_objtype == k_shape_octave_shift; }
    inline bool is_shape_octave_num() { return m_objtype == k_shape_octave_glyph; }
    inline bool is_shape_ornament() { return m_objtype == k_shape_ornament; }
    inline bool is_shape_pedal_glyph() const { return m_objtype == k_shape_pedal_glyph; }
    inline bool is_shape_pedal_line() const { return m_objtype == k_shape_pedal_line; }
    inline bool is_shape_rectangle() { return m_objtype == k_shape_rectangle; }
    inline bool is_shape_rest() { return m_objtype == k_shape_rest; }
    inline bool is_shape_rest_glyph() { return m_objtype == k_shape_rest_glyph; }
    inline bool is_shape_slur() { return m_objtype == k_shape_slur; }
    inline bool is_shape_squared_bracket() { return m_objtype == k_shape_squared_bracket; }
    inline bool is_shape_stem() { return m_objtype == k_shape_stem; }
    inline bool is_shape_staff() { return m_objtype == k_shape_staff; }
    inline bool is_shape_technical() { return m_objtype == k_shape_technical; }
    inline bool is_shape_text() { return m_objtype == k_shape_text; }
    inline bool is_shape_text_box() { return m_objtype == k_shape_text_box; }
    inline bool is_shape_tie() { return m_objtype == k_shape_tie; }
    inline bool is_shape_time_signature() { return m_objtype == k_shape_time_signature; }
    inline bool is_shape_time_signature_glyph() { return m_objtype == k_shape_time_signature_glyph; }
    inline bool is_shape_tuplet() { return m_objtype == k_shape_tuplet; }
    inline bool is_shape_volta_bracket() { return m_objtype == k_shape_volta_bracket; }
    inline bool is_shape_wedge() { return m_objtype == k_shape_wedge; }
    inline bool is_shape_word() { return m_objtype == k_shape_word; }

    //size
    inline void set_width(LUnits width) { m_size.width = width; }
    inline void set_height(LUnits height) { m_size.height = height; }

    //position
    void set_origin(UPoint& pos);
    void set_origin(const UPoint& pos);
    void set_origin(LUnits xLeft, LUnits yTop);
    void set_left(LUnits xLeft);
    void set_top(LUnits yTop);
    virtual void shift_origin(const USize& shift);

    //creator
    inline bool was_created_by(ImoObj* pImo) { return m_pCreatorImo == pImo; }
    inline ImoObj* get_creator_imo() { return m_pCreatorImo; }

    //parent
    inline void set_owner_box(GmoBox* pBox) { m_pParentBox = pBox; }
    inline GmoBox* get_owner_box() { return m_pParentBox; }
    GmoBoxDocPage* get_page_box();

    //support for handlers
    inline bool has_handlers() { return get_num_handlers() > 0; }
    virtual int get_num_handlers() { return 0; }
    virtual UPoint get_handler_point(int UNUSED(i)) { return UPoint(0.0, 0.0); }
    virtual void on_handler_dragged(int UNUSED(iHandler), UPoint UNUSED(newPos)) {}
    virtual void on_end_of_handler_drag(int UNUSED(iHandler), UPoint UNUSED(newPos)) {}

    //info
    const std::string get_notation_id(const std::string& prefix="");
    const std::string get_notation_class();

    //tests & debug
    virtual void dump(ostream& outStream, int level);
    static const std::string& get_name(int objtype);
    inline const std::string& get_name() { return get_name(m_objtype); }

    ///@endcond

protected:
    void propagate_dirty();

};

//---------------------------------------------------------------------------------------
/** All visible objects derive from abstract class %GmoShape. It represents a visible
    object, such as a line, a glyph, an arch, a note head, etc.
    As %GmoShape objects derive from GmoObj they have a bounds rectangle that defines
    the space occupied by the shape. This rectangle defines reference bounds for
    laying out other shapes and, also, it is used to detect visual collisions with
    other shapes during the layout process.

    In general, the only responsibility of a %GmoShape object is to draw itself when
    requested to do it by a Drawer object. Therefore, %GmoShape objects usually do not
    have public methods. As an exception, some shapes can provide information about
    sub-shapes or special reference points.
*/
class GmoShape : public GmoObj      //, public Linkable<USize>
{
protected:
    ShapeId m_idx;
    int m_layer;
	Color m_color;
    std::list<GmoShape*>* m_pRelatedShapes;

public:
    ///@cond INTERNALS
    //excluded from public API. Only for internal use.
    ~GmoShape() override;

    ///@endcond


    ///@cond INTERNALS
    //excluded from public API. Only for internal use.

    virtual void on_draw(Drawer* pDrawer, RenderOptions& opt);

    // layer identifiers. Shapes are placed in layers. The first layer to
    // render is layer 0 (background). Then, layer 1 (staves), and so on.
    enum { k_layer_background = 0, k_layer_staff, k_layer_barlines, k_layer_notes,
           k_layer_aux_objs, k_layer_top, k_layer_user,
           k_layer_max };

    //layer
    inline int get_layer() { return m_layer; }
    inline void set_layer(int layer) { m_layer = layer; }

//    //overrides required by Linkable
//	virtual void handle_link_event(Linkable<USize>* pShape, int type, USize shift);
//    virtual void on_linked_to(Linkable<USize>* pShape, int type);

//    //overrides required by score shapes related to a voice
//    virtual bool is_voice_related_shape() { return false; }

    virtual LUnits get_anchor_offset() { return 0.0f; }

    //methods related to position
    void set_origin_and_notify_observers(LUnits xLeft, LUnits yTop);
    virtual bool hit_test(LUnits x, LUnits y);
    virtual void reposition_shape(LUnits yShift);
    virtual LUnits get_baseline_y() const { return get_bottom(); }

    //related shapes
    inline std::list<GmoShape*>* get_related_shapes() { return m_pRelatedShapes; }
    void add_related_shape(GmoShape* pShape);
    GmoShape* find_related_shape(int type);

    //other flags
    void set_hover(bool value) override { value ? m_flags |= k_hover : m_flags &= ~k_hover; }
    void set_in_link(bool value) override { value ? m_flags |= k_in_link : m_flags &= ~k_in_link; }
    void set_flag_value(bool value, unsigned int flag) {
        value ? m_flags |= flag : m_flags &= ~flag;
    }

    //shape id
    inline ShapeId get_shape_id() { return m_idx; }
    inline void assign_id_as_main_shape() { m_idx = 0; }
    inline void assign_id_as_main_or_implicit_shape(int iStaff) { m_idx = iStaff; }
    inline void assign_id_as_courtesy_shape(int iStaff, int numStaves)
    {
        m_idx = numStaves + iStaff;
    }
    inline void assign_id_as_prolog_shape(int iSystem, int iStaff, int numStaves)
    {
        m_idx = iSystem * numStaves + iStaff;
    }
    static ShapeId generate_main_or_implicity_shape_id(int iStaff) { return iStaff; }

    //test and debug
    void dump(ostream& outStream, int level) override;
    virtual void set_color(Color color) { m_color = color; }
    virtual Color get_normal_color() { return m_color; }

    ///@endcond

protected:
    GmoShape(ImoObj* pCreatorImo, int objtype, ShapeId idx, Color color);
    virtual Color determine_color_to_use(RenderOptions& opt);

};

//---------------------------------------------------------------------------------------
/** %GmoBox is an abstract class from which all box objects derive.
    As %GmoBox derive from GmoObj they have a bounds rectangle that defines
    the box area. This rectangle defines reference bounds for laying out other objects
    (the box content).

    %GmoBox objects are
    used to organize the layout space. They define areas, in the final image, in which
    shapes will be positioned. For instance, GmoBoxParagraph is a box delimiting
    the space occupied by the text in a paragraph.

    Apart of defining the content area, all %GmoBox objects are containers for other
    boxes and for shapes.

    Boxes can be nested but can not partially overlap other boxes. That is, a box is
    either independent (covers a region of paper) or is fully contained in another box,
    subdividing it. For instance, %GmoBoxSystem, that represents the space ocuppied by
    a system, is subdivided in %GmoBoxSlice boxes, representing vertical slices of the
    system (e.g. measures).
*/
class GmoBox : public GmoObj
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

    GmoBox(int objtype, ImoObj* pCreatorImo);
    ~GmoBox() override;

public:

    ///@cond INTERNALS
    //excluded from public API. Only for internal use.

    //child boxes
    inline int get_num_boxes() { return static_cast<int>( m_childBoxes.size() ); }
    void add_child_box(GmoBox* child);
    GmoBox* get_child_box(int i);  //i = 0..n-1
    inline std::vector<GmoBox*>& get_child_boxes() { return m_childBoxes; }

    //parent
    GmoBox* get_parent_box() { return m_pParentBox; }
    GmoBoxDocPage* get_parent_doc_page();

    //contained shapes
    inline int get_num_shapes() { return static_cast<int>( m_shapes.size() ); }
    void add_shape(GmoShape* shape, int layer);
    GmoShape* get_shape(int i);  //i = 0..n-1

    //flags
    void set_hover(bool value) override { set_flag_value(value, k_hover); }
    void set_in_link(bool value) override { set_flag_value(value, k_in_link); }
    void set_has_edit_focus(bool value) { set_flag_value(value, k_has_edit_focus); }
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
    virtual ImoStyle* get_style();

    //content size
    //old semantic, based on score margins
    inline LUnits get_usable_width() { return get_width() - m_uLeftMargin - m_uRightMargin; }
    //new semantic, based on style
    LUnits get_content_top();
    LUnits get_content_left();
    LUnits get_content_width();
    LUnits get_content_height();

    //position
    void shift_origin_and_content(const USize& shift);
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

    ///@endcond

protected:
    void delete_boxes();
    void delete_shapes();
    void draw_border(Drawer* pDrawer, RenderOptions& opt);
    bool must_draw_bounds(RenderOptions& opt);
    Color get_box_color();
    virtual void draw_box_bounds(Drawer* pDrawer, double xorg, double yorg, Color& color);
    void draw_shapes(Drawer* pDrawer, RenderOptions& opt);
    void add_shapes_to_tables_in(GmoBoxDocPage* pPage);

    friend class StaffObjShapeCursor;
    friend class GraphicModel;
    void add_boxes_to_map_imo_to_box(GraphicModel* pGM);
    void add_boxes_to_controls_map(GraphicModel* pGM);

};

///@cond INTERNALS
//excluded from public API. Only for internal use.

//---------------------------------------------------------------------------------------
class StaffObjShapeCursor {
public:
    explicit StaffObjShapeCursor(GmoBox* pBox);
    explicit StaffObjShapeCursor(GmoShape* pShape);

    GmoShape* get_shape() const { return (*m_it); }
    TimeUnits get_time() const;

    bool next();
    bool next(TimeUnits maxTime);
    bool nextAfter(TimeUnits time);

    bool prev();
    bool prev(TimeUnits minTime);
    bool prevBefore(TimeUnits time);


protected:
    GmoBox* m_pCurrentBox;
    std::list<GmoShape*>::const_iterator m_it;
};
///@endcond

//---------------------------------------------------------------------------------------
/** %GmoBoxDocument is a container for enclosing the whole document, and contains one
    or more instances of GmoDocPage objects, representing each page of the document.

    %GmoBoxDocument is the root of the GraphicModel and only one instance exist for
    a document. It is simply a container, with no further responsibilities, and its only
    purpose is to store and manage its GmoBoxDocPage children.

    Its bounding box delimits the visual space occupied by the complete document but
    margins or padding values in its base GmoBox are useless and are ignored.
*/
class GmoBoxDocument : public GmoBox
{
protected:
    GmoBoxDocPage* m_pLastPage;
    GraphicModel* m_pGModel;

public:

    ///@cond INTERNALS
    //excluded from public API. Only for internal use.

    GmoBoxDocument(GraphicModel* pGModel, ImoObj* pCreatorImo);
    ~GmoBoxDocument() override {}

    //doc pages
    GmoBoxDocPage* add_new_page();
    GmoBoxDocPage* get_page(int i);     //i = 0..n-1
    inline int get_num_pages() { return get_num_boxes(); }
    inline GmoBoxDocPage* get_last_page() { return m_pLastPage; }
    int get_page_number(GmoBoxDocPage* pBoxPage);

    //overrides
    GraphicModel* get_graphic_model() override { return m_pGModel; }

    ///@endcond

};

//---------------------------------------------------------------------------------------
/** %GmoBoxDocPage encloses the shapes and boxes that make up a page of the document.

    It is mainly a container for GmoBoxDocPageContent objects (e.g. GmoBoxParagraph,
    GmoBoxScore, GmoBoxTable, etc.).
*/
class GmoBoxDocPage : public GmoBox
{
protected:
    int m_numPage;      //1..n
    std::list<GmoShape*> m_allShapes;		//contained shapes, ordered by layer and creation order

public:
    ///@cond INTERNALS
    //excluded from public API. Only for internal use.
    GmoBoxDocPage(ImoObj* pCreatorImo);
    ~GmoBoxDocPage() override {}


    //page number
    inline void set_number(int num) { m_numPage = num; }
    inline int get_number() { return m_numPage; }

    //renderization
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //shapes
    void add_to_tables(GmoShape* pShape);
    GmoShape* get_first_shape_for_layer(int order);
    GmoShape* find_shape_for_object(ImoStaffObj* pSO);
    void store_in_map_imo_shape(GmoShape* pShape);

    //hit testing
    GmoObj* hit_test(LUnits x, LUnits y);
    GmoShape* find_shape_at(LUnits x, LUnits y);

    //selection
    void select_objects_in_rectangle(SelectionSet* selection, const URect& selRect,
                                     unsigned flags=0);

    ///@endcond

protected:
    void draw_page_background(Drawer* pDrawer, RenderOptions& opt);
};

//---------------------------------------------------------------------------------------
/** %GmoBoxDocPageContent is just a container for all boxes and shapes that define
    the visual content generated by any ImoContentBlock objects, such as a music score,
    a paragraph or a table.

    It is simply a container, with no further responsibilities or functionality, and
    therefore has no specific methods.
*/
class GmoBoxDocPageContent : public GmoBox
{
protected:

public:
    ///@cond INTERNALS
    //excluded from public API. Only for internal use.
    GmoBoxDocPageContent(ImoObj* pCreatorImo);
    virtual ~GmoBoxDocPageContent() {}
    ///@endcond

};

//---------------------------------------------------------------------------------------
/** %GmoBoxScorePage is a container for the boxes and for shapes that make up a page
    of the score. It contains one or more GmoBoxSystem objects as well as the shapes
    all for page level notations, such as headers and footers.
*/
class GmoBoxScorePage : public GmoBox
{
protected:
    int m_iFirstSystem;         //0..n-1
    int m_iLastSystem;          //0..n-1
    int m_iPage;                //0..n-1        number of this score-page
    LUnits m_maxSystemHeight;   //height of highest system in this page

public:

    ///@cond INTERNALS
    //excluded from public API. Only for internal use.

    GmoBoxScorePage(ImoScore* pScore);
    virtual ~GmoBoxScorePage();

    inline void set_page_number(int iPage) { m_iPage = iPage; }

	//systems
    void add_system(GmoBoxSystem* pSystem, int iSystem);
    inline int get_num_first_system() const { return m_iFirstSystem; }
    inline int get_num_last_system() const { return m_iLastSystem; }
    inline int get_num_systems() {
        return (m_iFirstSystem == -1 ? 0 : m_iLastSystem - m_iFirstSystem + 1);
    }
	GmoBoxSystem* get_system(int iSystem);		//nSystem = 0..n-1
	inline int get_page_number() { return m_iPage; }
    LUnits get_max_system_height() { return m_maxSystemHeight; }

	//timepos information
	TimeUnits end_time();
	TimeUnits start_time();

    //hit tests related
    int nearest_system_to_point(LUnits y);

    ///@endcond
};


///@cond INTERNALS
//excluded from public API. Only for internal use.

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
                  ImoStyle* style=nullptr);

    ~GmoBoxControl() override {}

    ///@cond INTERNALS
    //excluded from public API. Only for internal use.


    inline void set_style(ImoStyle* pStyle) { m_pStyle = pStyle; }
    void notify_event(SpEventInfo pEvent);

    //GmoBox override
    void on_draw(Drawer* pDrawer, RenderOptions& opt) override;

    //accessors
    inline Control* get_creator_control() { return m_pControl; }

    ///@endcond

protected:
    //overrides
    ImoStyle* get_style() override { return m_pStyle; }
};

//---------------------------------------------------------------------------------------
class GmoBoxTable : public GmoBox
{
protected:

public:
    GmoBoxTable(ImoObj* pCreatorImo)
        : GmoBox(GmoObj::k_box_table, pCreatorImo) {}
    virtual ~GmoBoxTable() {}
};

//---------------------------------------------------------------------------------------
class GmoBoxTableRows : public GmoBox
{
protected:

public:
    GmoBoxTableRows(ImoObj* pCreatorImo)
        : GmoBox(GmoObj::k_box_table_rows, pCreatorImo) {}
    virtual ~GmoBoxTableRows() {}
};


//=======================================================================================
// Utility global functions
//=======================================================================================
inline LUnits compute_distance(LUnits x1, LUnits y1, LUnits x2, LUnits y2)
{
    LUnits dx = x2-x1;
    LUnits dy = y2-y1;
    return sqrt(dx * dx + dy * dy);
}

///@endcond


}   //namespace lomse

#endif      //__LOMSE_GM_BASIC_H__
