//-------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------

#ifndef __LOMSE_INTERNAL_MODEL_H__        //to avoid nested includes
#define __LOMSE_INTERNAL_MODEL_H__

#include <string>
#include <list>
#include <vector>
#include <map>
#include "lomse_visitor.h"
#include "lomse_tree.h"
#include "lomse_basic.h"
#include "lomse_score_enums.h"
#include "lomse_shape_base.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
//forward declarations
class ColStaffObjs;
class LdpElement;

class ImoAttachments;
class ImoAuxObj;
class ImoDocObj;
class ImoInstrument;
class ImoMusicData;
class ImoNote;
class ImoNoteRest;
class ImoObjVisitor;
class ImoOptionInfo;
class ImoScoreText;
class ImoStaffInfo;
class ImoTextStyleInfo;

class DtoAuxObj;
class DtoBarline;
class DtoClef;
class DtoComponentObj;
class DtoDocObj;
class DtoFermata;
class DtoGoBackFwd;
class DtoKeySignature;
class DtoMetronomeMark;
class DtoObj;
class DtoSpacer;
class DtoStaffObj;
class DtoTimeSignature;


//---------------------------------------------------------------------------------------
// enums for common values

    //-----------------------------------------------------------------------------
	//The placement attribute indicates whether something is above or below another
    //element, such as a note or a notation.
    enum EPlacement
    {
        k_placement_default = 0,
	    k_placement_above,
        k_placement_below,
    };

    //-----------------------------------------------------------------------------
	//The orientation attribute indicates whether slurs and ties are overhand (tips
    //down) or underhand (tips up). This is distinct from the placement attribute:
    //placement is relative, one object with respect to another object. But
    //orientation is referred to the object itself: turned up or down.
	enum EOrientation
    {
        k_orientation_default = 0,
        k_orientation_over,
        k_orientation_under,
    };


//---------------------------------------------------------------------------------------
// a struct to contain note/rest figure and dots
struct NoteTypeAndDots
{
    NoteTypeAndDots(int nt, int d) : noteType(nt), dots(d) {}

    int noteType;   //ImoNoteRest enum
    int dots;       //0..n
};




// Model classes
//================================


//===================================================
// Abstarct hierachy
//===================================================

//---------------------------------------------------------------------------------------
// the root. Any object must derive from it
class ImoObj : public Visitable, public NodeInTree<ImoObj>
{
protected:
    long m_id;
    int m_objtype;

    ImoObj(int objtype) : m_id(-1L), m_objtype(objtype) {}
    ImoObj(long id, int objtype) : m_id(id), m_objtype(objtype) {}
    ImoObj(int objtype, DtoObj& dto);

public:
    virtual ~ImoObj();

    enum {
        // ImoObj (A)
        k_obj=0,
            // ImoSimpleObj (A)
            k_simpleobj, k_beam_info, k_bezier_info,
            k_border_info, k_box_info, k_color_info, k_cursor_info, k_figured_bass_info,
            k_font_info, k_instr_group,
            k_line_info, k_midi_info, k_option, k_page_info, k_point_info,
            k_size_info, k_staff_info, k_system_info, k_text_info, k_text_style_info,
            k_tie_info, k_tuplet_info,
                //ImoCollection(A)
                k_collection, k_attachments, k_content, k_instruments,
                k_instrument_groups, k_music_data, k_options,

            // ImoDocObj (A)
            k_docobj,

                // ImoContainerObj
                k_containerobj, k_document, k_instrument, k_score,

                // ImoComponentObj (A)
                k_componentobj,

                    // ImoStaffObj (A)
                    k_staffobj, k_barline, k_clef, k_key_signature, k_time_signature,
                    k_note, k_rest, k_go_back_fwd, k_metronome_mark, k_control,
                    k_spacer, k_figured_bass,

                    // ImoAuxObj (A)
                    k_auxobj, k_fermata, k_line, k_score_text, k_score_title,
                    k_text_box,
                        //ImoRelObj (A)
                        k_relobj,
                            // BinaryRelObj (A)
                            k_binary_relobj, k_tie,
                            // MultiRelObj (A)
                            k_multi_relobj, k_beam, k_chord, k_tuplet,
    };


    //getters
    inline long get_id() { return m_id; }

    //setters
    inline void set_id(long id) { m_id = id; }

    //overrides to Visitable class members
	virtual void accept_in(BaseVisitor& v);
	virtual void accept_out(BaseVisitor& v);

    //children
    ImoObj* get_child_of_type(int objtype);


    //object classification
    inline int get_obj_type() { return m_objtype; }
	inline bool has_children() { return !is_terminal(); }

    //simple objs
        // properties
    inline bool is_simpleobj() { return m_objtype >= k_simpleobj && m_objtype < k_docobj; }
    inline bool is_beam_info() { return m_objtype == k_beam_info; }
    inline bool is_bezier_info() { return m_objtype == k_bezier_info; }
    inline bool is_border_info() { return m_objtype == k_border_info; }
    inline bool is_box_info() { return m_objtype == k_box_info; }
    inline bool is_color_info() { return m_objtype == k_color_info; }
    inline bool is_cursor_info() { return m_objtype == k_cursor_info; }
    inline bool is_figured_bass_info() { return m_objtype == k_figured_bass_info; }
    inline bool is_font_info() { return m_objtype == k_font_info; }
    inline bool is_instr_group() { return m_objtype == k_instr_group; }
    inline bool is_line_info() { return m_objtype == k_line_info; }
    inline bool is_midi_info() { return m_objtype == k_midi_info; }
    inline bool is_page_info() { return m_objtype == k_page_info; }
    inline bool is_point_info() { return m_objtype == k_point_info; }
    inline bool is_size_info() { return m_objtype == k_size_info; }
    inline bool is_staff_info() { return m_objtype == k_staff_info; }
    inline bool is_system_info() { return m_objtype == k_system_info; }
    inline bool is_text_info() { return m_objtype == k_text_info; }
    inline bool is_text_style_info() { return m_objtype == k_text_style_info; }
    inline bool is_tie_info() { return m_objtype == k_tie_info; }
    inline bool is_tuplet_info() { return m_objtype == k_tuplet_info; }
        // containers
    inline bool is_content() { return m_objtype == k_content; }
    inline bool is_music_data() { return m_objtype == k_music_data; }
    inline bool is_option() { return m_objtype == k_option; }

    // doc objs
    inline bool is_docobj() { return m_objtype >= k_docobj; }

    // container objs
    inline bool is_containerobj() { return m_objtype >= k_containerobj && m_objtype < k_componentobj; }
    inline bool is_document() { return m_objtype == k_document; }
    inline bool is_instrument() { return m_objtype == k_instrument; }
	inline bool is_score() { return m_objtype == k_score; }

    // component objs
	inline bool is_componentobj() { return m_objtype >= k_componentobj; }

	// staff objs
	inline bool is_staffobj() { return m_objtype >= k_staffobj && m_objtype < k_auxobj; }
    inline bool is_barline() { return m_objtype == k_barline; }
    inline bool is_clef() { return m_objtype == k_clef; }
    inline bool is_key_signature() { return m_objtype == k_key_signature; }
    inline bool is_time_signature() { return m_objtype == k_time_signature; }
    inline bool is_note_rest() { return m_objtype == k_note || m_objtype == k_rest; }
    inline bool is_note() { return m_objtype == k_note; }
    inline bool is_rest() { return m_objtype == k_rest; }
    inline bool is_go_back_fwd() { return m_objtype == k_go_back_fwd; }
    inline bool is_metronome_mark() { return m_objtype == k_metronome_mark; }
    inline bool is_control() { return m_objtype == k_control; }
    inline bool is_spacer() { return m_objtype == k_spacer; }
    inline bool is_figured_bass() { return m_objtype == k_figured_bass; }

    // aux objs
	inline bool is_auxobj() { return m_objtype >= k_auxobj; }
    inline bool is_fermata() { return m_objtype == k_fermata; }
    inline bool is_line() { return m_objtype == k_line; }
    inline bool is_score_text() { return m_objtype == k_score_text; }
    inline bool is_score_title() { return m_objtype == k_score_title; }

    // rel objs
    inline bool is_relobj() { return m_objtype >= k_relobj; }

    // binary rel objs
    inline bool is_binary_relobj() { return m_objtype >= k_binary_relobj
                                          && m_objtype < k_multi_relobj; }
    inline bool is_tie() { return m_objtype == k_tie; }

    // multi rel objs
    inline bool is_multi_relobj() { return m_objtype >= k_multi_relobj; }
    inline bool is_beam() { return m_objtype == k_beam; }
    inline bool is_chord() { return m_objtype == k_chord; }
    inline bool is_tuplet() { return m_objtype == k_tuplet; }

};

// just auxiliary objects or data transfer objects
//---------------------------------------------------------------------------------------
class ImoSimpleObj : public ImoObj
{
protected:
    ImoSimpleObj(long id, int objtype) : ImoObj(id, objtype) {}
    ImoSimpleObj(int objtype) : ImoObj(objtype) {}
    ImoSimpleObj(int objtype, DtoObj& dto) : ImoObj(objtype, dto) {}

public:
    virtual ~ImoSimpleObj() {}
};

// Any object for a music score, including the score itself
//---------------------------------------------------------------------------------------
class ImoDocObj : public ImoObj
{
protected:
    Tenths m_txUserLocation;
    Tenths m_tyUserLocation;

    ImoDocObj(int objtype);
    ImoDocObj(long id, int objtype);
    ImoDocObj(int objtype, DtoDocObj& dto);

public:
    virtual ~ImoDocObj();

    //getters
    inline Tenths get_user_location_x() { return m_txUserLocation; }
    inline Tenths get_user_location_y() { return m_tyUserLocation; }

    //setters
    inline void set_user_location_x(Tenths tx) { m_txUserLocation = tx; }
    inline void set_user_location_y(Tenths ty) { m_tyUserLocation = ty; }

    //attachments (first child)
    ImoAttachments* get_attachments();
    bool has_attachments();
    int get_num_attachments();
    ImoAuxObj* get_attachment(int i);
    void attach(ImoAuxObj* pAO);
    void remove_attachment(ImoAuxObj* pAO);

};

//---------------------------------------------------------------------------------------
class ImoCollection : public ImoSimpleObj
{
protected:
    ImoCollection(int objtype) : ImoSimpleObj(objtype) {}

public:
    virtual ~ImoCollection() {}

    //contents
    ImoDocObj* get_item(int iItem) {   //iItem = 0..n-1
        return dynamic_cast<ImoDocObj*>( get_child(iItem) );
    }
    inline int get_num_items() { return get_num_children(); }
    inline void remove_item(ImoDocObj* pItem) { remove_child(pItem); }
};

// ContainerObj: A collection of containers and contained objs.
//---------------------------------------------------------------------------------------
class ImoContainerObj : public ImoDocObj
{
protected:
    ImoContainerObj(int objtype) : ImoDocObj(objtype) {}

public:
    virtual ~ImoContainerObj() {}

};

// ComponentObj: Any atomic displayable object. Must be attached to
//               containers (ContainerObj) or to other ComponentObj
//---------------------------------------------------------------------------------------
class ImoComponentObj : public ImoDocObj
{
protected:
    bool m_fVisible;
    Color m_color;

    ImoComponentObj(long id, int objtype) : ImoDocObj(id, objtype), m_fVisible(true) {}
    ImoComponentObj(int objtype) : ImoDocObj(objtype), m_fVisible(true) {}
    ImoComponentObj(int objtype, DtoComponentObj& dto);

public:
    virtual ~ImoComponentObj() {}

    //getters
    inline bool is_visible() { return m_fVisible; }
    inline Color& get_color() { return m_color; }

    //setters
    inline void set_visible(bool visible) { m_fVisible = visible; }
    void set_color(Color color);

};

// StaffObj: A ComponentObj that is attached to an Staff. Consume time
//---------------------------------------------------------------------------------------
class ImoStaffObj : public ImoComponentObj
{
protected:
    int m_staff;

    ImoStaffObj(int objtype) : ImoComponentObj(objtype), m_staff(0) {}
    ImoStaffObj(long id, int objtype) : ImoComponentObj(id, objtype), m_staff(0) {}
    ImoStaffObj(int objtype, DtoStaffObj& dto);

public:
    virtual ~ImoStaffObj() {}

    //getters
    virtual float get_duration() { return 0.0f; }
    inline int get_staff() { return m_staff; }

    //setters
    virtual void set_staff(int staff) { m_staff = staff; }


};

// AuxObj: a ComponentObj that must be attached to other objects but not
//         directly to an staff. Do not consume time
//---------------------------------------------------------------------------------------
class ImoAuxObj : public ImoComponentObj
{
protected:
    ImoAuxObj(int objtype) : ImoComponentObj(objtype) {}
    ImoAuxObj(int objtype, DtoAuxObj& dto);

public:
    virtual ~ImoAuxObj() {}

protected:
    ImoAuxObj(ImoDocObj* pOwner, long id, int objtype) : ImoComponentObj(id, objtype) {}

};

//An abstract AuxObj relating at least two StaffObjs
//---------------------------------------------------------------------------------------
class ImoRelObj : public ImoAuxObj
{
protected:
    ImoRelObj(int objtype) : ImoAuxObj(objtype) {}
    //ImoRelObj(ImoDocObj* pOwner, long id, int objtype)
    //    : ImoAuxObj(pOwner, id, objtype) {}

public:
	virtual ~ImoRelObj() {}

    ////building/destroying the relationship
    //virtual void include(ImoStaffObj* pSO)=0;
    //virtual void remove(ImoStaffObj* pSO)=0;
	//virtual void on_relationship_modified()=0;

    //information
    virtual ImoStaffObj* get_start_object()=0;
    virtual ImoStaffObj* get_end_object()=0;

};


//An abstract AuxObj relating two and only two StaffObjs
//---------------------------------------------------------------------------------------
class ImoBinaryRelObj : public ImoRelObj
{
protected:
    ImoStaffObj* m_pStartSO;     //StaffObjs related by this ImoRelObj
    ImoStaffObj* m_pEndSO;

    ImoBinaryRelObj(int objtype) 
        : ImoRelObj(objtype), m_pStartSO(NULL), m_pEndSO(NULL) {}
    ImoBinaryRelObj(int objtype, ImoStaffObj* pStartSO, ImoStaffObj* pEndSO)
        : ImoRelObj(objtype), m_pStartSO(pStartSO), m_pEndSO(pEndSO) {}

public:
    virtual ~ImoBinaryRelObj();

    //implementation of ImoRelObj pure virtual methods
    //virtual void include(ImoStaffObj* pSO) {};
    //virtual void remove(ImoStaffObj* pSO);
    //virtual void on_relationship_modified() {};
    virtual ImoStaffObj* get_start_object() { return m_pStartSO; }
    virtual ImoStaffObj* get_end_object() { return m_pEndSO; }

};

//An abstract AuxObj relating two or more StaffObjs
//---------------------------------------------------------------------------------------
class ImoMultiRelObj : public ImoRelObj
{
protected:
    std::list<ImoStaffObj*> m_relatedObjects;

    ImoMultiRelObj(int objtype) : ImoRelObj(objtype) {}
    //ImoMultiRelObj(ImoDocObj* pOwner, long id, int objtype);

public:
    virtual ~ImoMultiRelObj();

    //implementation of ImoRelObj pure virtual methods
    //virtual void include(ImoStaffObj* pSO, int index = -1);
    //virtual void remove(ImoStaffObj* pSO);
    //virtual void on_relationship_modified() {};
    ImoStaffObj* get_start_object() { return m_relatedObjects.front(); }
    ImoStaffObj* get_end_object() { return m_relatedObjects.back(); }


    //specific methods
    void push_back(ImoStaffObj* pSO);
    inline int get_num_objects() { return static_cast<int>( m_relatedObjects.size() ); }
    //int get_object_index(ImoStaffObj* pSO);
    std::list<ImoStaffObj*>& get_related_objects() { return m_relatedObjects; }

};



//=======================================================================================
// Simple objects: Info and Dto
//=======================================================================================

//---------------------------------------------------------------------------------------
class ImoBeamInfo : public ImoSimpleObj
{
protected:
    int m_beamType[6];
    int m_beamNum;
    bool m_repeat[6];
    LdpElement* m_pBeamElm;
    ImoNoteRest* m_pNR;

public:
    ImoBeamInfo();
    ImoBeamInfo(LdpElement* pBeamElm);
    ~ImoBeamInfo() {}

    //getters
    inline int get_beam_number() { return m_beamNum; }
    inline LdpElement* get_beam_element() { return m_pBeamElm; }
    inline ImoNoteRest* get_note_rest() { return m_pNR; }
    int get_line_number();
    int get_beam_type(int level);
    bool get_repeat(int level);

    //setters
    inline void set_beam_number(int num) { m_beamNum = num; }
    inline void set_note_rest(ImoNoteRest* pNR) { m_pNR = pNR; }
    inline void set_beam_element(LdpElement* pElm) { m_pBeamElm = pElm; }
    void set_beam_type(int level, int type);
    void set_repeat(int level, bool value);

    //properties
    bool is_end_of_beam();

};

//---------------------------------------------------------------------------------------
class ImoBezierInfo : public ImoSimpleObj
{
protected:
    TPoint m_tPoints[4];   //start, end, ctrol1, ctrol2

public:
    ImoBezierInfo() : ImoSimpleObj(ImoObj::k_bezier_info) {}
    ImoBezierInfo(ImoBezierInfo* pBezier);

    ~ImoBezierInfo() {}

	enum { k_start=0, k_end, k_ctrol1, k_ctrol2, k_max};     // point number

    //points
    inline void set_point(int i, TPoint& value) { m_tPoints[i] = value; }
    inline TPoint& get_point(int i) { return m_tPoints[i]; }

};

//---------------------------------------------------------------------------------------
class ImoBorderDto : public ImoSimpleObj
{
    Color        m_color;
    Tenths        m_width;
    ELineStyle    m_style;

public:
    ImoBorderDto() : ImoSimpleObj(ImoObj::k_border_info)
        , m_color(Color(0,0,0,255)), m_width(1.0f), m_style(k_line_solid) {}
    ~ImoBorderDto() {}

    //getters
    inline Color get_color() { return m_color; }
    inline Tenths get_width() { return m_width; }
    inline ELineStyle get_style() { return m_style; }

    //setters
    inline void set_color(Color value) { m_color = value; }
    inline void set_width(Tenths value) { m_width = value; }
    inline void set_style(ELineStyle value) { m_style = value; }

};

//---------------------------------------------------------------------------------------
class ImoChord : public ImoMultiRelObj
{
public:
    ImoChord() : ImoMultiRelObj(ImoObj::k_chord) {}
    ~ImoChord() {}
};

//---------------------------------------------------------------------------------------
class ImoColorDto : public ImoSimpleObj
{
protected:
    Color m_color;
    bool m_ok;

public:
    ImoColorDto() : ImoSimpleObj(ImoObj::k_color_info), m_color(0, 0, 0, 255), m_ok(true) {}
    ImoColorDto(Int8u r, Int8u g, Int8u b, Int8u a = 255);
    ~ImoColorDto() {}

    Color& get_from_rgb_string(const std::string& rgb);
    Color& get_from_rgba_string(const std::string& rgba);
    Color& get_from_string(const std::string& hex);
    inline bool is_ok() { return m_ok; }

    inline Int8u red() { return m_color.r; }
    inline Int8u blue() { return m_color.b; }
    inline Int8u green() { return m_color.g; }
    inline Int8u alpha() { return m_color.a; }
    inline Color& get_color() { return m_color; }


protected:
    Int8u convert_from_hex(const std::string& hex);

};

//---------------------------------------------------------------------------------------
class ImoCursorInfo : public ImoSimpleObj
{
protected:
    int m_instrument;
    int m_staff;
    float m_time;
    long m_id;

public:
    ImoCursorInfo() : ImoSimpleObj(ImoObj::k_cursor_info)
                    , m_instrument(0), m_staff(0), m_time(0.0f), m_id(-1L) {}
    ~ImoCursorInfo() {}

    //getters
    inline int get_instrument() { return m_instrument; }
    inline int get_staff() { return m_staff; }
    inline float get_time() { return m_time; }
    inline long get_id() { return m_id; }

    //setters
    inline void set_instrument(int value) { m_instrument = value; }
    inline void set_staff(int value) { m_staff = value; }
    inline void set_time(float value) { m_time = value; }
    inline void set_id(long value) { m_id = value; }
};

//---------------------------------------------------------------------------------------
class ImoLineInfo : public ImoSimpleObj
{
protected:
    ELineStyle  m_lineStyle;
    ELineEdge   m_startEdge;
    ELineEdge   m_endEdge;
    ELineCap    m_startStyle;
    ELineCap    m_endStyle;
    Color      m_color;
    Tenths      m_width;
    TPoint      m_startPoint;
    TPoint      m_endPoint;

public:

    ImoLineInfo()
        : ImoSimpleObj(ImoObj::k_line_info)
        , m_lineStyle(k_line_solid)
        , m_startEdge(k_edge_normal)
        , m_endEdge(k_edge_normal)
        , m_startStyle(k_cap_none)
        , m_endStyle(k_cap_none)
        , m_color(Color(0,0,0,255))
        , m_width(1.0f)
        , m_startPoint(0.0f, 0.0f)
        , m_endPoint(0.0f, 0.0f)
    {
    }

    ImoLineInfo(ImoLineInfo& info)
        : ImoSimpleObj(ImoObj::k_line_info)
        , m_lineStyle( info.get_line_style() )
        , m_startEdge( info.get_start_edge() )
        , m_endEdge( info.get_end_edge() )
        , m_startStyle( info.get_start_style() )
        , m_endStyle( info.get_end_style() )
        , m_color( info.get_color() )
        , m_width( info.get_width() )
        , m_startPoint( info.get_start_point() )
        , m_endPoint( info.get_end_point() )
    {
    }

    ~ImoLineInfo() {}

    //getters
    inline ELineStyle get_line_style() { return m_lineStyle; }
    inline ELineEdge get_start_edge() { return m_startEdge; }
    inline ELineEdge get_end_edge() { return m_endEdge; }
    inline ELineCap get_start_style() { return m_startStyle; }
    inline ELineCap get_end_style() { return m_endStyle; }
    inline Color get_color() { return m_color; }
    inline Tenths get_width() { return m_width; }
    inline TPoint get_start_point() { return m_startPoint; }
    inline TPoint get_end_point() { return m_endPoint; }

    //setters
    inline void set_line_style(ELineStyle value) { m_lineStyle = value; }
    inline void set_start_edge(ELineEdge value) { m_startEdge = value; }
    inline void set_end_edge(ELineEdge value) { m_endEdge = value; }
    inline void set_start_style(ELineCap value) { m_startStyle = value; }
    inline void set_end_style(ELineCap value) { m_endStyle = value; }
    inline void set_color(Color value) { m_color = value; }
    inline void set_width(Tenths value) { m_width = value; }
    inline void set_start_point(TPoint point) { m_startPoint = point; }
    inline void set_end_point(TPoint point) { m_endPoint = point; }

};

//---------------------------------------------------------------------------------------
class ImoBoxInfo : public ImoSimpleObj
{
//<location>[<size>][<color>][<border>]<text>[<anchorLine>]
protected:
    //block position and size
    TSize     m_size;
    TPoint    m_topLeftPoint;

    ////block position within a possible parent block
    //EHAlign       m_hAlign;
    //EVAlign       m_vAlign;

    //block looking
    Color        m_bgColor;
    Color        m_borderColor;
    Tenths        m_borderWidth;
    ELineStyle    m_borderStyle;

public:
    ImoBoxInfo()
        : ImoSimpleObj(k_box_info)
        , m_size(TSize(160.0f, 100.0f))
        , m_topLeftPoint(TPoint(0.0f, 0.0f))
        , m_bgColor( Color(255, 255, 255, 255) )
        , m_borderColor( Color(0, 0, 0, 255) )
        , m_borderWidth(1.0f)
        , m_borderStyle(k_line_solid)
    {
    }

    ~ImoBoxInfo() {}

    //getters
    inline Tenths get_height() { return m_size.height; }
    inline Tenths get_width() { return m_size.width; }
    inline TPoint get_position() { return m_topLeftPoint; }
    inline Color get_bg_color() { return m_bgColor; }
    inline Color get_border_color() { return m_borderColor; }
    inline Tenths get_border_width() { return m_borderWidth; }
    inline ELineStyle get_border_style() { return m_borderStyle; }

    //setters
    inline void set_position_x(Tenths value) { m_topLeftPoint.x = value; }
    inline void set_position_y(Tenths value) { m_topLeftPoint.y = value; }
    inline void set_size(TSize size) { m_size = size; }
    inline void set_bg_color(Color color) { m_bgColor = color; }
    inline void set_border(ImoBorderDto* pBorder) {
        m_borderColor = pBorder->get_color();
        m_borderWidth = pBorder->get_width();
        m_borderStyle = pBorder->get_style();
    }
};

//---------------------------------------------------------------------------------------
class ImoMidiInfo : public ImoSimpleObj
{
protected:
    int m_instr;
    int m_channel;

public:
    ImoMidiInfo();
    ImoMidiInfo(ImoMidiInfo& dto);
    ~ImoMidiInfo() {}

    //getters
    inline int get_instrument() { return m_instr; }
    inline int get_channel() { return m_channel; }

    //setters
    inline void set_instrument(int value) { m_instr = value; }
    inline void set_channel(int value) { m_channel = value; }

};

//---------------------------------------------------------------------------------------
class ImoFontInfo : public ImoSimpleObj
{
public:
    string name;
    float size;       // in points
    int style;        // k_normal, k_italic
    int weight;       // k_normal, k_bold

    ImoFontInfo() : ImoSimpleObj(ImoObj::k_font_info)
                  , size(8), style(k_normal), weight(k_normal) {}

    enum { k_normal=0, k_italic, k_bold, };
};

//---------------------------------------------------------------------------------------
class ImoTextInfo : public ImoSimpleObj
{
protected:
    string m_text;
    ImoTextStyleInfo* m_pStyle;

public:
    ImoTextInfo(const std::string& value="")
        : ImoSimpleObj(ImoObj::k_text_info), m_text(value), m_pStyle(NULL) {}
    ~ImoTextInfo() {}

    //getters
    inline string& get_text() { return m_text; }
    inline ImoTextStyleInfo* get_style() { return m_pStyle; }
    const std::string& get_font_name();
    float get_font_size();
    int get_font_style();
    int get_font_weight();
    Color get_color();

    //setters
    inline void set_text(const string& text) { m_text = text; }
    inline void set_style(ImoTextStyleInfo* pStyle) { m_pStyle = pStyle; }
};

//---------------------------------------------------------------------------------------
class ImoPageInfo : public ImoSimpleObj
{
protected:
    LUnits  m_uLeftMargin;
    LUnits  m_uRightMargin;
    LUnits  m_uTopMargin;
    LUnits  m_uBottomMargin;
    LUnits  m_uBindingMargin;
    USize   m_uPageSize;
    bool    m_fPortrait;

public:
    ImoPageInfo();
    ImoPageInfo(ImoPageInfo& dto);
    ~ImoPageInfo() {}

    //getters
    inline LUnits get_left_margin() { return m_uLeftMargin; }
    inline LUnits get_right_margin() { return m_uRightMargin; }
    inline LUnits get_top_margin() { return m_uTopMargin; }
    inline LUnits get_bottom_margin() { return m_uBottomMargin; }
    inline LUnits get_binding_margin() { return m_uBindingMargin; }
    inline bool is_portrait() { return m_fPortrait; }
    inline USize get_page_size() { return m_uPageSize; }
    inline LUnits get_page_width() { return m_uPageSize.width; }
    inline LUnits get_page_height() { return m_uPageSize.height; }

    //setters
    inline void set_left_margin(LUnits value) { m_uLeftMargin = value; }
    inline void set_right_margin(LUnits value) { m_uRightMargin = value; }
    inline void set_top_margin(LUnits value) { m_uTopMargin = value; }
    inline void set_bottom_margin(LUnits value) { m_uBottomMargin = value; }
    inline void set_binding_margin(LUnits value) { m_uBindingMargin = value; }
    inline void set_portrait(bool value) { m_fPortrait = value; }
    inline void set_page_size(USize uPageSize) { m_uPageSize = uPageSize; }
    inline void set_page_width(LUnits value) { m_uPageSize.width = value; }
    inline void set_page_height(LUnits value) { m_uPageSize.height = value; }
};


//===================================================
// Real objects
//===================================================

//---------------------------------------------------------------------------------------
class ImoAttachments : public ImoSimpleObj
{
protected:
    std::list<ImoAuxObj*> m_attachments;

public:
    ImoAttachments() : ImoSimpleObj(ImoObj::k_attachments) {}
    ~ImoAttachments();

    //contents
    ImoAuxObj* get_item(int iItem);   //iItem = 0..n-1
    inline int get_num_items() { return int(m_attachments.size()); }
    void remove(ImoAuxObj* pAO);
    inline void add(ImoAuxObj* pAO) { m_attachments.push_back(pAO); }
};

//---------------------------------------------------------------------------------------
class ImoBarline : public ImoStaffObj
{
protected:
    long m_type;

public:
    ImoBarline(DtoBarline& dto, long id);
    ~ImoBarline() {}

	enum { k_simple=0, k_double, k_start, k_end, k_end_repetition, k_start_repetition,
           k_double_repetition, };

    //getters
    inline int get_type() { return m_type; }

    //overrides: barlines always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};

//---------------------------------------------------------------------------------------
class ImoBeam : public ImoMultiRelObj
{
public:
    ImoBeam() : ImoMultiRelObj(ImoObj::k_beam) {}
    ~ImoBeam() {}

    //type of beam
    enum { k_none = 0, k_begin, k_continue, k_end, k_forward, k_backward, };

};

//---------------------------------------------------------------------------------------
class ImoBlock : public ImoAuxObj
{
protected:
    ImoBoxInfo m_box;

    ImoBlock(int objtype) : ImoAuxObj(objtype) {}
    ImoBlock(int objtype, ImoBoxInfo& box) : ImoAuxObj(objtype), m_box(box) {}

public:
    ~ImoBlock() {}

};

//---------------------------------------------------------------------------------------
class ImoTextBox : public ImoBlock
{
protected:
    ImoTextInfo m_text;
    ImoLineInfo m_line;
    bool m_fHasAnchorLine;
    //TPoint m_anchorJoinPoint;     //point on the box rectangle

public:
    ImoTextBox() : ImoBlock(k_text_box), m_fHasAnchorLine(false) {}
    ImoTextBox(ImoBoxInfo& box) : ImoBlock(k_text_box, box), m_fHasAnchorLine(false) {}
    ~ImoTextBox() {}

    inline ImoBoxInfo* get_box_info() { return &m_box; }
    inline ImoLineInfo* get_anchor_line_info() { return &m_line; }
    inline bool has_anchor_line() { return m_fHasAnchorLine; }

    inline const std::string& get_text() { return m_text.get_text(); }
    inline void set_text(ImoTextInfo* pTI) { m_text = *pTI; }
    inline void set_anchor_line(ImoLineInfo* pLine) {
        m_line = *pLine;
        m_fHasAnchorLine = true;
    }
};

//---------------------------------------------------------------------------------------
class ImoClef : public ImoStaffObj
{
protected:
    int m_clefType;
    int m_symbolSize;

public:
    ImoClef(DtoClef& dto);
    ImoClef(int clefType);
    ~ImoClef() {}

    enum {
        k_undefined=-1,
        k_G2 = 0,
        k_F4,
        k_F3,
        k_C1,
        k_C2,
        k_C3,
        k_C4,
        k_percussion,
        // other clefs not available for exercises
        k_C5,
        k_F5,
        k_G1,
        k_8_G2,        //8 above
        k_G2_8,        //8 below
        k_8_F4,        //8 above
        k_F4_8,        //8 below
        k_15_G2,       //15 above
        k_G2_15,       //15 below
        k_15_F4,       //15 above
        k_F4_15,       //15 below
    };

    //getters and setters
    inline int get_clef_type() { return m_clefType; }
    inline void set_clef_type(int type) { m_clefType = type; }
    inline int get_symbol_size() { return m_symbolSize; }

};

//---------------------------------------------------------------------------------------
class ImoContent : public ImoCollection
{
public:
    ImoContent() : ImoCollection(ImoObj::k_content) {}
    ~ImoContent() {}

};

//---------------------------------------------------------------------------------------
class ImoControl : public ImoStaffObj
{
protected:

public:
    ImoControl() : ImoStaffObj(ImoObj::k_control) {}
    ~ImoControl() {}

    //getters & setters
};

//---------------------------------------------------------------------------------------
class ImoDocument : public ImoContainerObj
{
protected:
    string m_version;
    ImoPageInfo m_pageInfo;

public:
    ImoDocument(const std::string& version="");
    ~ImoDocument();

    //getters and setters
    inline std::string& get_version() { return m_version; }
//    inline void set_version(const std::string& version) { m_version = version; }

    //content
    ImoDocObj* get_content_item(int iItem);
    int get_num_content_items();
    ImoContent* get_content();

    //document intended paper size
    void add_page_info(ImoPageInfo* pPI);
    inline ImoPageInfo* get_page_info() { return &m_pageInfo; }
    inline LUnits get_paper_width() { return m_pageInfo.get_page_width(); }
    inline LUnits get_paper_height() { return m_pageInfo.get_page_height(); }

    //cursor
    //TODO
    void add_cursor_info(ImoCursorInfo* pCursor) {};

};

//---------------------------------------------------------------------------------------
class ImoFermata : public ImoAuxObj
{
protected:
    int m_placement;
    int m_symbol;

public:
    ImoFermata(DtoFermata& dto);
    ~ImoFermata() {}

    enum { k_normal, k_angled, k_square, };     //symbol

    //getters
    inline int get_placement() { return m_placement; }
    inline int get_symbol() { return m_symbol; }

    //setters
    inline void set_placement(int placement) { m_placement = placement; }
    inline void set_symbol(int symbol) { m_symbol = symbol; }

};

//---------------------------------------------------------------------------------------
class ImoGoBackFwd : public ImoStaffObj
{
protected:
    bool    m_fFwd;
    float   m_rTimeShift;

    const float SHIFT_START_END;     //any too big value

public:
//    ImoGoBackFwd(bool fFwd) : ImoStaffObj(ImoObj::k_go_back_fwd), m_fFwd(fFwd), m_rTimeShift(0.0f),
//                             SHIFT_START_END(100000000.0f) {}
    ImoGoBackFwd(DtoGoBackFwd& dto);
    ~ImoGoBackFwd() {}

    //getters and setters
    inline bool is_forward() { return m_fFwd; }
    inline bool is_to_start() { return !m_fFwd && (m_rTimeShift == -SHIFT_START_END); }
    inline bool is_to_end() { return m_fFwd && (m_rTimeShift == SHIFT_START_END); }
    inline float get_time_shift() { return m_rTimeShift; }
    inline void set_to_start() { set_time_shift(SHIFT_START_END); }
    inline void set_to_end() { set_time_shift(SHIFT_START_END); }
    inline void set_time_shift(float rTime) { m_rTimeShift = (m_fFwd ? rTime : -rTime); }
};

//---------------------------------------------------------------------------------------
class ImoScoreText : public ImoAuxObj
{
protected:
    ImoTextInfo m_text;
    int m_hAlign;

    ImoScoreText(int objtype, const std::string& value)
        : ImoAuxObj(objtype), m_text(value), m_hAlign(k_halign_left) {}

public:
    ImoScoreText(const std::string& value="")
        : ImoAuxObj(ImoObj::k_score_text), m_text(value), m_hAlign(k_halign_left) {}
    virtual ~ImoScoreText() {}

    //getters
    inline int get_h_align() { return m_hAlign; }
    inline string& get_text() { return m_text.get_text(); }
    inline ImoTextStyleInfo* get_style() { return m_text.get_style(); }
    inline ImoTextInfo* get_text_info() { return &m_text; }

    //setters
    inline void set_h_align(int value) { m_hAlign = value; }
    inline void set_style(ImoTextStyleInfo* pStyle) { m_text.set_style(pStyle); }

};

//---------------------------------------------------------------------------------------
class ImoScoreTitle : public ImoScoreText
{
protected:
    int m_hAlign;

public:
    ImoScoreTitle(const std::string& value="", int hAlign=k_halign_center)
        : ImoScoreText(ImoObj::k_score_title, value), m_hAlign(hAlign) {}
    ~ImoScoreTitle() {}

    inline int get_h_align() { return m_hAlign; }
    inline void set_style(ImoTextStyleInfo* pStyle) { m_text.set_style(pStyle); }
    inline ImoTextStyleInfo* get_style() { return m_text.get_style(); }
};

//---------------------------------------------------------------------------------------
class ImoInstrGroup : public ImoSimpleObj
{
protected:
    bool m_fJoinBarlines;
    int m_symbol;           // enum k_none, k_default, k_brace, k_bracket, ...
    ImoScoreText m_name;
    ImoScoreText m_abbrev;
    std::list<ImoInstrument*> m_instruments;

public:
    ImoInstrGroup();
    ~ImoInstrGroup();

    enum { k_none=0, k_default, k_brace, k_bracket, };

    //getters
    inline bool join_barlines() { return m_fJoinBarlines; }
    inline int get_symbol() { return m_symbol; }
    inline const std::string& get_name() { return m_name.get_text(); }
    inline const std::string& get_abbrev() { return m_abbrev.get_text(); }

    //setters
    void set_name(ImoScoreText* pText);
    void set_abbrev(ImoScoreText* pText);
    inline void set_symbol(int symbol) { m_symbol = symbol; }
    inline void set_join_barlines(bool value) { m_fJoinBarlines = value; }

    //instruments
    //ImoInstruments* get_instruments();
    void add_instrument(ImoInstrument* pInstr);
    ImoInstrument* get_instrument(int iInstr);   //0..n-1
    int get_num_instruments();

};

//---------------------------------------------------------------------------------------
class ImoInstrument : public ImoContainerObj
{
protected:
    ImoScoreText m_name;
    ImoScoreText m_abbrev;
    ImoMidiInfo m_midi;
    ImoInstrGroup* m_pGroup;
    std::list<ImoStaffInfo*> m_staves;

public:
    ImoInstrument();
    ~ImoInstrument();

    //getters
    inline int get_num_staves() { return static_cast<int>(m_staves.size()); }
    inline ImoScoreText& get_name() { return m_name; }
    inline ImoScoreText& get_abbrev() { return m_abbrev; }
    inline int get_instrument() { return m_midi.get_instrument(); }
    inline int get_channel() { return m_midi.get_channel(); }
    ImoMusicData* get_musicdata();
    inline bool is_in_group() { return m_pGroup != NULL; }
    inline ImoInstrGroup* get_group() { return m_pGroup; }
    ImoStaffInfo* get_staff(int iStaff);
    LUnits get_line_spacing_for_staff(int iStaff);

    //setters
    void add_staff();
    void set_name(ImoScoreText* pText);
    void set_abbrev(ImoScoreText* pText);
    void set_midi_info(ImoMidiInfo* pInfo);
    inline void set_in_group(ImoInstrGroup* pGroup) { m_pGroup = pGroup; }
    void replace_staff_info(ImoStaffInfo* pInfo);

    //info
    inline bool has_name() { return m_name.get_text() != ""; }
    inline bool has_abbrev() { return m_abbrev.get_text() != ""; }


protected:

};

//---------------------------------------------------------------------------------------
class ImoInstruments : public ImoCollection
{
public:
    ImoInstruments() : ImoCollection(ImoObj::k_instruments) {}
    ~ImoInstruments() {}

};

//---------------------------------------------------------------------------------------
class ImoInstrGroups : public ImoCollection
{
public:
    ImoInstrGroups() : ImoCollection(ImoObj::k_instrument_groups) {}
    ~ImoInstrGroups() {}

};

//---------------------------------------------------------------------------------------
class ImoKeySignature : public ImoStaffObj
{
protected:
    int m_keyType;

public:
//    ImoKeySignature() : ImoStaffObj(ImoObj::k_key_signature) , m_keyType(ImoKeySignature::k_undefined) {}
    ImoKeySignature(DtoKeySignature& dto);
    ~ImoKeySignature() {}

	enum { k_undefined=-1, k_C=0, k_G, k_D, k_A, k_E, k_B, k_Fs, k_Cs, k_Cf, k_Gf,
           k_Df, k_Af, k_Ef, k_Bf, k_F, k_a, k_e, k_b, k_fs, k_cs, k_gs, k_ds, k_as,
           k_af, k_ef, k_bf, k_f, k_c, k_g, k_d };

    //getters and setters
    inline int get_key_type() { return m_keyType; }
//    inline void set_key_type(int type) { m_keyType = type; }

    //overrides: key signatures always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};

//---------------------------------------------------------------------------------------
class ImoLine : public ImoAuxObj
{
    ImoLineInfo m_lineInfo;

public:
    //ImoLine() : ImoStaffObj(ImoObj::k_line) {}
    ImoLine(ImoLineInfo& info);
    ~ImoLine() {}

    inline ImoLineInfo* get_line_info() { return &m_lineInfo; }

};

//---------------------------------------------------------------------------------------
class ImoMetronomeMark : public ImoStaffObj
{
protected:
    int     m_markType;
    int     m_ticksPerMinute;
    int     m_leftNoteType;
    int     m_leftDots;
    int     m_rightNoteType;
    int     m_rightDots;
    bool    m_fParenthesis;

public:
    ImoMetronomeMark(DtoMetronomeMark& dto);
    //ImoMetronomeMark() : ImoStaffObj(ImoObj::k_metronome_mark), m_markType(k_value),
    //    m_ticksPerMinute(60),
    //    m_leftNoteType(0), m_leftDots(0),
    //    m_rightNoteType(0), m_rightDots(0),
    //    m_fParenthesis(false) {}
    ~ImoMetronomeMark() {}

    enum { k_note_value=0, k_note_note, k_value, };

    //getters
    inline int get_left_note_type() { return m_leftNoteType; }
    inline int get_right_note_type() { return m_rightNoteType; }
    inline int get_left_dots() { return m_leftDots; }
    inline int get_right_dots() { return m_rightDots; }
    inline int get_ticks_per_minute() { return m_ticksPerMinute; }
    inline int get_mark_type() { return m_markType; }
    inline bool has_parenthesis() { return m_fParenthesis; }

    ////setters
    //inline void set_left_note_type(int noteType) { m_leftNoteType = noteType; }
    //inline void set_right_note_type(int noteType) { m_rightNoteType = noteType; }
    //inline void set_left_dots(int dots) { m_leftDots = dots; }
    //inline void set_right_dots(int dots) { m_rightDots = dots; }
    //inline void set_ticks_per_minute(int ticks) { m_ticksPerMinute = ticks; }
    //inline void set_mark_type(int type) { m_markType = type; }
    //inline void set_parenthesis(bool fValue) { m_fParenthesis = fValue; }

    //inline void set_right_note_dots(const NoteTypeAndDots& figdots) {
    //    m_rightNoteType = figdots.noteType;
    //    m_rightDots = figdots.dots;
    //}
    //inline void set_left_note_dots(const NoteTypeAndDots& figdots) {
    //    m_leftNoteType = figdots.noteType;
    //    m_leftDots = figdots.dots;
    //}

};

//---------------------------------------------------------------------------------------
class ImoMusicData : public ImoCollection
{
public:
    ImoMusicData() : ImoCollection(ImoObj::k_music_data) {}
    ~ImoMusicData() {}

};

//---------------------------------------------------------------------------------------
class ImoOptionInfo : public ImoSimpleObj
{
protected:
    int         m_type;
    string      m_name;
    string      m_sValue;
    bool        m_fValue;
    long        m_nValue;
    float       m_rValue;

public:
    ImoOptionInfo(const string& name)
        : ImoSimpleObj(ImoObj::k_option), m_type(k_boolean), m_name(name)
        , m_fValue(false) {}
    ~ImoOptionInfo() {}

    enum { k_boolean=0, k_number_long, k_number_float, k_string };

    //getters
    inline string get_name() { return m_name; }
    inline int get_type() { return m_type; }
    inline bool get_bool_value() { return m_fValue; }
    inline long get_long_value() { return m_nValue; }
    inline float get_float_value() { return m_rValue; }
    inline string& get_string_value() { return m_sValue; }
    inline bool is_bool_option() { return m_type == k_boolean; }
    inline bool is_long_option() { return m_type == k_number_long; }
    inline bool is_float_option() { return m_type == k_number_float; }

    //setters
    inline void set_type(int type) { m_type = type; }
    inline void set_bool_value(bool value) { m_fValue = value; m_type = k_boolean; }
    inline void set_long_value(long value) { m_nValue = value; m_type = k_number_long; }
    inline void set_float_value(float value) { m_rValue = value; m_type = k_number_float; }
    inline void set_string_value(const string& value) { m_sValue = value; }

};

//class ImoOptionBool : public ImoOptionInfo
//class ImoOptionLong : public ImoOptionInfo
//class ImoOptionFloat : public ImoOptionInfo

//---------------------------------------------------------------------------------------
class ImoOptions : public ImoCollection
{
public:
    ImoOptions() : ImoCollection(ImoObj::k_options) {}
    ~ImoOptions() {}

};

//---------------------------------------------------------------------------------------
class ImoPointDto : public ImoSimpleObj
{
    TPoint m_point;

public:
    ImoPointDto() : ImoSimpleObj(ImoObj::k_point_info) {}
    ~ImoPointDto() {}

    inline TPoint get_point() { return m_point; }

    inline void set_x(Tenths x) { m_point.x = x; }
    inline void set_y(Tenths y) { m_point.y = y; }

};

//---------------------------------------------------------------------------------------
class ImoSizeDto : public ImoSimpleObj
{
    TSize m_size;

public:
    ImoSizeDto() : ImoSimpleObj(ImoObj::k_size_info) {}
    ~ImoSizeDto() {}

    inline TSize get_size() { return m_size; }

    inline void set_width(Tenths w) { m_size.width = w; }
    inline void set_height(Tenths h) { m_size.height = h; }

};

//---------------------------------------------------------------------------------------
class ImoSpacer : public ImoStaffObj
{
protected:
    Tenths  m_space;

public:
    ImoSpacer(DtoSpacer& dto);
    ImoSpacer(Tenths space=0.0f) : ImoStaffObj(ImoObj::k_spacer), m_space(space) {}
    ~ImoSpacer() {}

    //getters
    inline Tenths get_width() { return m_space; }

    //setters
    inline void set_width(Tenths space) { m_space = space; }

};

//---------------------------------------------------------------------------------------
class ImoTextStyleInfo : public ImoSimpleObj
{
protected:
    string m_name;
    ImoFontInfo m_fontInfo;
    Color m_color;

public:
    ImoTextStyleInfo() : ImoSimpleObj(ImoObj::k_text_style_info) {}
    ~ImoTextStyleInfo() {}

    //getters
    inline const std::string& get_name() { return m_name; }
    inline const std::string& get_font_name() { return m_fontInfo.name; }
    inline float get_font_size() { return m_fontInfo.size; }
    inline int get_font_style() { return m_fontInfo.style; }
    inline int get_font_weight() { return m_fontInfo.weight; }
    inline Color get_color() { return m_color; }
    inline bool is_bold() { return get_font_weight() == ImoFontInfo::k_bold; }
    inline bool is_italic() { return get_font_style() == ImoFontInfo::k_italic; }

    //setters
    inline void set_name (const std::string& value) { m_name = value; }
    inline void set_font_name(const std::string& value) { m_fontInfo.name = value; }
    inline void set_font_size(float points) { m_fontInfo.size = points; }
    inline void set_font_style(int value) { m_fontInfo.style = value; }
    inline void set_font_weight(int value) { m_fontInfo.weight = value; }
    inline void set_color(Color value) { m_color = value; }
};

//---------------------------------------------------------------------------------------
class ImoSystemInfo : public ImoSimpleObj
{
protected:
    bool    m_fFirst;   //true=first, false=other
    LUnits   m_leftMargin;
    LUnits   m_rightMargin;
    LUnits   m_systemDistance;
    LUnits   m_topSystemDistance;

public:
    ImoSystemInfo();
    ImoSystemInfo(ImoSystemInfo& dto);
    ~ImoSystemInfo() {}

    //getters
    inline bool is_first() { return m_fFirst; }
    inline LUnits get_left_margin() { return m_leftMargin; }
    inline LUnits get_right_margin() { return m_rightMargin; }
    inline LUnits get_system_distance() { return m_systemDistance; }
    inline LUnits get_top_system_distance() { return m_topSystemDistance; }

    //setters
    inline void set_first(bool fValue) { m_fFirst = fValue; }
    inline void set_left_margin(LUnits rValue) { m_leftMargin = rValue; }
    inline void set_right_margin(LUnits rValue) { m_rightMargin = rValue; }
    inline void set_system_distance(LUnits rValue) { m_systemDistance = rValue; }
    inline void set_top_system_distance(LUnits rValue) { m_topSystemDistance = rValue; }
};

//---------------------------------------------------------------------------------------
class ImoScore : public ImoContainerObj
{
protected:
    string          m_version;
    ColStaffObjs*   m_pColStaffObjs;
    ImoSystemInfo   m_systemInfoFirst;
    ImoSystemInfo   m_systemInfoOther;
    ImoPageInfo     m_pageInfo;
    std::list<ImoScoreTitle*> m_titles;
	std::map<std::string, ImoTextStyleInfo*> m_nameToStyle;

public:
    ImoScore();
    ~ImoScore();

    //getters and setters
    inline std::string& get_version() { return m_version; }
    inline void set_version(const std::string& version) { m_version = version; }

    inline ColStaffObjs* get_staffobjs_table() { return m_pColStaffObjs; }
    inline void set_staffobjs_table(ColStaffObjs* pColStaffObjs) { m_pColStaffObjs = pColStaffObjs; }

    //instruments
    void add_instrument(ImoInstrument* pInstr);
    ImoInstrument* get_instrument(int iInstr);   //0..n-1
    int get_num_instruments();
    ImoInstruments* get_instruments();

    //instrumen groups
    void add_instruments_group(ImoInstrGroup* pGroup);
    ImoInstrGroups* get_instrument_groups();

    //options
    ImoOptions* get_options();
    void set_option(ImoOptionInfo* pOpt);
    bool has_options();
    ImoOptionInfo* get_option(const std::string& name);
    void set_float_option(const std::string& name, float value);
    void set_bool_option(const std::string& name, bool value);
    void set_long_option(const std::string& name, long value);

    //score layout
    void add_sytem_info(ImoSystemInfo* pSL);
    inline ImoSystemInfo* get_first_system_info() { return &m_systemInfoFirst; }
    inline ImoSystemInfo* get_other_system_info() { return &m_systemInfoOther; }
    void add_page_info(ImoPageInfo* pPI);
    inline ImoPageInfo* get_page_info() { return &m_pageInfo; }

    //titles
    void add_title(ImoScoreTitle* pTitle);
    inline std::list<ImoScoreTitle*>& get_titles() { return m_titles; }

    //styles
    void add_style_info(ImoTextStyleInfo* pStyle);
    void add_required_text_styles();
    ImoTextStyleInfo* get_style_info(const std::string& name);
    ImoTextStyleInfo* get_default_style_info();
    ImoTextStyleInfo* get_style_info_or_defaults(const std::string& name);

protected:
    void delete_staffobjs_collection();
    void delete_text_styles();
    ImoTextStyleInfo* create_default_style();
    void set_defaults_for_system_info();
    void set_defaults_for_options();
    void add_option(ImoOptionInfo* pOpt);

};

//---------------------------------------------------------------------------------------
class ImoStaffInfo : public ImoSimpleObj
{
protected:
    int m_numStaff;
    int m_nNumLines;
    int m_staffType;
    LUnits m_uSpacing;      //between line centers
    LUnits m_uLineThickness;
    LUnits m_uMarging;      //distance from the bottom line of the previous staff

public:
    //Default values for staff. Line spacing: 1.8 mm (staff height = 7.2 mm),
    //line thickness: 0.15 millimeters, top margin: 10 millimeters
    ImoStaffInfo(int numStaff=0, int lines=5, int type=k_staff_regular,
                 LUnits spacing=180.0f, LUnits thickness=15.0f, LUnits margin=1000.0f)
        : ImoSimpleObj(ImoObj::k_staff_info)
        , m_numStaff(numStaff)
        , m_nNumLines(lines)
        , m_staffType(type)
        , m_uSpacing(spacing)
        , m_uLineThickness(thickness)
        , m_uMarging(margin)
    {
    }
    ~ImoStaffInfo() {}

    enum { k_staff_ossia=0, k_staff_cue, k_staff_editorial, k_staff_regular,
        k_staff_alternate, };

    //staff number
    inline int get_staff_number() { return m_numStaff; }
    inline void set_staff_number(int num) { m_numStaff = num; }

    //staff type
    inline int get_staff_type() { return m_staffType; }
    inline void set_staff_type(int type) { m_staffType = type; }

	//margins
    inline LUnits get_staff_margin() { return m_uMarging; }
    inline void set_staff_margin(LUnits uSpace) { m_uMarging = uSpace; }

    //spacing and size
    inline LUnits get_line_spacing() { return m_uSpacing; }
    inline void set_line_spacing(LUnits uSpacing) { m_uSpacing = uSpacing; }
    LUnits get_height() {
        if (m_nNumLines > 1)
            return (m_nNumLines - 1) * m_uSpacing + m_uLineThickness;
        else
            return m_uLineThickness;
    }

    //lines
    inline LUnits get_line_thickness() { return m_uLineThickness; }
    inline void set_line_thickness(LUnits uTickness) { m_uLineThickness = uTickness; }
    inline int get_num_lines() { return m_nNumLines; }
    inline void set_num_lines(int nLines) { m_nNumLines = nLines; }

};

//---------------------------------------------------------------------------------------
class ImoTie : public ImoBinaryRelObj
{
protected:
    bool        m_fStart;
    int         m_tieNum;
    ImoBezierInfo*   m_pStartBezier;
    ImoBezierInfo*   m_pEndBezier;

public:
    ImoTie() : ImoBinaryRelObj(ImoObj::k_tie), m_fStart(true), m_tieNum(0)
             , m_pStartBezier(NULL), m_pEndBezier(NULL) {}
    ~ImoTie();

    //getters
    inline bool is_start() { return m_fStart; }
    inline int get_tie_number() { return m_tieNum; }
    ImoNote* get_start_note();
    ImoNote* get_end_note();
    inline ImoBezierInfo* get_start_bezier() { return m_pStartBezier; }
    inline ImoBezierInfo* get_stop_bezier() { return m_pEndBezier; }

    //setters
    inline void set_start(bool value) { m_fStart = value; }
    inline void set_tie_number(int num) { m_tieNum = num; }
    void set_start_note(ImoNote* pNote);
    void set_end_note(ImoNote* pNote);
    inline void set_start_bezier(ImoBezierInfo* pBezier) { m_pStartBezier = pBezier; }
    inline void set_stop_bezier(ImoBezierInfo* pBezier) { m_pEndBezier = pBezier; }

};

// raw info about a pending tie
//---------------------------------------------------------------------------------------
class ImoTieDto : public ImoSimpleObj
{
protected:
    bool m_fStart;
    int m_tieNum;
    ImoNote* m_pNote;
    ImoBezierInfo* m_pBezier;
    LdpElement* m_pTieElm;
    Color m_color;

public:
    ImoTieDto() : ImoSimpleObj(ImoObj::k_tie_info), m_fStart(true), m_tieNum(0), m_pNote(NULL)
                 , m_pBezier(NULL), m_pTieElm(NULL) {}
    ~ImoTieDto();

    //getters
    inline bool is_start() { return m_fStart; }
    inline int get_tie_number() { return m_tieNum; }
    inline ImoNote* get_note() { return m_pNote; }
    inline ImoBezierInfo* get_bezier() { return m_pBezier; }
    inline LdpElement* get_tie_element() { return m_pTieElm; }
    int get_line_number();
    inline Color get_color() { return m_color; }

    //setters
    inline void set_start(bool value) { m_fStart = value; }
    inline void set_tie_number(int num) { m_tieNum = num; }
    inline void set_note(ImoNote* pNote) { m_pNote = pNote; }
    inline void set_bezier(ImoBezierInfo* pBezier) { m_pBezier = pBezier; }
    inline void set_tie_element(LdpElement* pElm) { m_pTieElm = pElm; }
    inline void set_color(Color value) { m_color = value; }

};

//---------------------------------------------------------------------------------------
class ImoTimeSignature : public ImoStaffObj
{
protected:
    int     m_beats;
    int     m_beatType;

public:
    ImoTimeSignature(DtoTimeSignature& dto);
    ~ImoTimeSignature() {}

    //getters and setters
    inline int get_beats() { return m_beats; }
    inline void set_beats(int beats) { m_beats = beats; }
    inline int get_beat_type() { return m_beatType; }
    inline void set_beat_type(int beatType) { m_beatType = beatType; }

    //overrides: time signatures always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};

//---------------------------------------------------------------------------------------
// raw info about a tuplet
class ImoTupletDto : public ImoSimpleObj
{
protected:
    bool m_fStartOfTuplet;
    int m_nActualNum;
    int m_nNormalNum;
    int m_nShowBracket;
    int m_nPlacement;
    int m_nShowNormalNum;
    LdpElement* m_pTupletElm;
    ImoNoteRest* m_pNR;

public:
    ImoTupletDto();
    ImoTupletDto(LdpElement* pBeamElm);
    ~ImoTupletDto() {}

    //getters
    inline LdpElement* get_tuplet_element() { return m_pTupletElm; }
    inline ImoNoteRest* get_note_rest() { return m_pNR; }
    inline bool is_start_of_tuplet() { return m_fStartOfTuplet; }
    inline bool is_end_of_tuplet() { return !m_fStartOfTuplet; }
    inline int get_actual_number() { return m_nActualNum; }
    inline int get_normal_number() { return m_nNormalNum; }
    inline int get_show_bracket() { return m_nShowBracket; }
    inline int get_show_normal_num() { return m_nShowNormalNum; }
    inline int get_placement() { return m_nPlacement; }
    int get_line_number();

    //setters
    inline void set_note_rest(ImoNoteRest* pNR) { m_pNR = pNR; }
    inline void set_tuplet_element(LdpElement* pElm) { m_pTupletElm = pElm; }
    inline void set_start_of_tuplet(bool value) { m_fStartOfTuplet = value; }
    inline void set_actual_number(int value) { m_nActualNum = value; }
    inline void set_normal_number(int value) { m_nNormalNum = value; }
    inline void set_show_bracket(int value) { m_nShowBracket = value; }
    inline void set_show_normal_num(int value) { m_nShowNormalNum = value; }
    inline void set_placement(int value) { m_nPlacement = value; }
};

//---------------------------------------------------------------------------------------
class ImoTuplet : public ImoMultiRelObj
{
protected:
    int m_nActualNum;
    int m_nNormalNum;
    int m_nShowBracket;
    int m_nShowNormalNum;
    int m_nPlacement;

public:
    ImoTuplet() : ImoMultiRelObj(ImoObj::k_tuplet) {}
    ImoTuplet(ImoTupletDto* dto);
    ~ImoTuplet() {}

    enum { k_straight = 0, k_curved, k_slurred, };

    //getters
    inline int get_actual_number() { return m_nActualNum; }
    inline int get_normal_number() { return m_nNormalNum; }
    inline int get_show_bracket() { return m_nShowBracket; }
    inline int get_show_normal_num() { return m_nShowNormalNum; }
    inline int get_placement() { return m_nPlacement; }
};



// A tree of ImoObj objects
typedef Tree<ImoObj>            ImoTree;
typedef NodeInTree<ImoObj>      ImoNode;



//---------------------------------------------------------------------------------------
// global functions

extern int to_step(const char& letter);
extern int to_octave(const char& letter);
extern int to_accidentals(const std::string& accidentals);
extern int to_note_type(const char& letter);
extern bool ldp_pitch_to_components(const string& pitch, int *step, int* octave, int* accidentals);
extern NoteTypeAndDots ldp_duration_to_components(const string& duration);
extern float to_duration(int nNoteType, int nDots);


}   //namespace lomse

#endif    // __LOMSE_INTERNAL_MODEL_H__

