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
class SoundEventsTable;
class DynGenerator;
class Document;

class ImoAttachments;
class ImoAuxObj;
class ImoBeamData;
class ImoBeamDto;
class ImoBeam;
class ImoBoxContainer;
class ImoBoxContent;
class ImoBoxInline;
class ImoBoxLevelObj;
class ImoButton;
class ImoChord;
class ImoContent;
class ImoContentObj;
class ImoDocument;
class ImoDynamic;
class ImoFontStyleDto;
class ImoHeading;
class ImoInlineObj;
class ImoInlineWrapper;
class ImoInstrument;
class ImoLineStyle;
class ImoLink;
class ImoMusicData;
class ImoNote;
class ImoNoteRest;
class ImoObj;
class ImoOptionInfo;
class ImoParagraph;
class ImoParamInfo;
class ImoReldataobjs;
class ImoRelDataObj;
class ImoRelObj;
class ImoScoreText;
class ImoSimpleObj;
class ImoSlurDto;
class ImoStaffInfo;
class ImoStaffObj;
class ImoStyle;
class ImoStyles;
class ImoTextBlock;
class ImoTextInfo;
class ImoTextItem;
class ImoTextStyle;
class ImoTieData;
class ImoTieDto;
class ImoTupletData;
class ImoTupletDto;
class ImoTuplet;
class ImoWrapperBox;

class DtoObj;


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

    //-----------------------------------------------------------------------------
    //clefs
    enum EClef
    {
        k_clef_undefined=-1,
        k_clef_G2 = 0,
        k_clef_F4,
        k_clef_F3,
        k_clef_C1,
        k_clef_C2,
        k_clef_C3,
        k_clef_C4,
        k_clef_percussion,
        // other clefs not available for exercises
        k_clef_C5,
        k_clef_F5,
        k_clef_G1,
        k_clef_8_G2,        //8 above
        k_clef_G2_8,        //8 below
        k_clef_8_F4,        //8 above
        k_clef_F4_8,        //8 below
        k_clef_15_G2,       //15 above
        k_clef_G2_15,       //15 below
        k_clef_15_F4,       //15 above
        k_clef_F4_15,       //15 below
    };

    //-----------------------------------------------------------------------------
    //key signatures
	enum EKeySignature
	{
        k_key_undefined=-1,

        k_key_C=0,
        k_min_key = k_key_C,
        k_min_major_key = k_key_C,
        k_key_G,
        k_key_D,
        k_key_A,
        k_key_E,
        k_key_B,
        k_key_Fs,
        k_key_Cs,
        k_key_Cf,
        k_key_Gf,
        k_key_Df,
        k_key_Af,
        k_key_Ef,
        k_key_Bf,
        k_key_F,
        k_max_mayor_key = k_key_F,

        k_key_a,
        k_min_minor_key = k_key_a,
        k_key_e,
        k_key_b,
        k_key_fs,
        k_key_cs,
        k_key_gs,
        k_key_ds,
        k_key_as,
        k_key_af,
        k_key_ef,
        k_key_bf,
        k_key_f,
        k_key_c,
        k_key_g,
        k_key_d,
        k_max_minor_key = k_key_d,
        k_max_key = k_key_d,
    };
    #define k_num_keys k_max_key - k_min_key + 1

    //-----------------------------------------------------------------------------
    //type for ImoObj objects
    enum EImoObjType
    {
        // ImoObj (A)
        k_imo_obj=0,

            //ImoDto (A)
            k_imo_dto, k_imo_font_style_dto,

            // ImoSimpleObj (A)
            k_imo_simpleobj, k_imo_beam_dto, k_imo_bezier_info, k_imo_border_dto,
            k_imo_textblock_info,
            k_imo_color_dto, k_imo_cursor_info, k_imo_figured_bass_info,
            k_imo_instr_group,
            k_imo_line_style, k_imo_midi_info, k_imo_option, k_imo_page_info,
            k_imo_param_info, k_imo_point_dto,
            k_imo_size_dto, k_imo_slur_dto, k_imo_staff_info, k_imo_system_info,
            k_imo_text_info,
            k_imo_text_style, k_imo_style,
            k_imo_tie_dto, k_imo_tuplet_dto,

            // ImoRelDataObj (A)
            k_imo_reldataobj, k_imo_beam_data, k_imo_slur_data,
            k_imo_tie_data, k_imo_tuplet_data,

            //ImoCollection(A)
            k_imo_collection, k_imo_instruments,
            k_imo_instrument_groups, k_imo_music_data, k_imo_options,
            k_imo_reldataobjs, k_imo_styles,

            // Special collections
            k_imo_attachments,

            // ImoContainerObj (A)
            k_imo_containerobj, k_imo_instrument,

            // ImoContentObj (A)
            k_imo_contentobj,

                // ImoScoreObj (A) content only for scores
                k_imo_scoreobj,

                    // ImoStaffObj (A)
                    k_imo_staffobj, k_imo_barline, k_imo_clef, k_imo_key_signature,
                    k_imo_time_signature,
                    k_imo_note, k_imo_rest, k_imo_go_back_fwd,
                    k_imo_metronome_mark, k_imo_control,
                    k_imo_spacer, k_imo_figured_bass,

                    // ImoAuxObj (A)
                    k_imo_auxobj, k_imo_fermata, k_imo_line, k_imo_score_text,
                    k_imo_score_title,
                    k_imo_text_box,

                        // ImoRelObj (A)
                        k_imo_relobj, k_imo_beam, k_imo_chord, k_imo_slur, k_imo_tie,
                        k_imo_tuplet,

                // ImoBoxContainer (A)
                k_imo_box_container,
                    k_imo_content, k_imo_document, k_imo_score,

                // ImoBoxContent (A)
                k_imo_box_content,
                    k_imo_dynamic,
                    k_imo_textblock,
                        k_imo_heading, k_imo_para,

                // ImoInlineObj
                k_imo_inlineobj,
                    k_imo_button, k_imo_text_item,

                    // ImoBoxInline (A)
                    k_imo_box_inline,
                        k_imo_inline_wrapper, k_imo_link,

    };

//---------------------------------------------------------------------------------------
// a struct to contain note/rest figure and dots
struct NoteTypeAndDots
{
    NoteTypeAndDots(int nt, int d) : noteType(nt), dots(d) {}

    int noteType;   //ImoNoteRest enum
    int dots;       //0..n
};


//---------------------------------------------------------------------------------------
// utility function to convert typographical points to LUnits
extern LUnits pt_to_LUnits(float pt);


//---------------------------------------------------------------------------------------
// API for objects that are allowed to create inline-level objects
class InlineLevelCreatorApi
{
protected:
    ImoContentObj* m_pParent;

    inline void set_inline_level_creator_api_parent(ImoContentObj* parent) {
        m_pParent = parent;
    }

public:
    InlineLevelCreatorApi() {}
    virtual ~InlineLevelCreatorApi() {}

    //API
    ImoTextItem* add_text_item(const string& text, ImoStyle* pStyle=NULL);
    ImoButton* add_button(const string& label, const USize& size, ImoStyle* pStyle=NULL);
    ImoInlineWrapper* add_inline_box(LUnits width=0.0f, ImoStyle* pStyle=NULL);
    ImoLink* add_link(const string& url, ImoStyle* pStyle=NULL);

};


//---------------------------------------------------------------------------------------
// API for objects that are allowed to create block-level objects
class BlockLevelCreatorApi
{
protected:
    ImoContentObj* m_pParent;

    inline void set_box_level_creator_api_parent(ImoContentObj* parent) {
        m_pParent = parent;
    }

public:
    BlockLevelCreatorApi() {}
    virtual ~BlockLevelCreatorApi() {}

    //API
    ImoParagraph* add_paragraph(ImoStyle* pStyle=NULL);
    ImoContent* add_content_wrapper(ImoStyle* pStyle=NULL);
    ImoScore* add_score(ImoStyle* pStyle=NULL);

private:
    void add_to_model(ImoBoxLevelObj* pImo, ImoStyle* pStyle);

};



//=======================================================================================
// InternalModel: A container for the objects forming the internal model
//=======================================================================================

class InternalModel
{
protected:
    ImoObj* m_pRoot;

public:
    InternalModel(ImoObj* pRoot) : m_pRoot(pRoot) {}
    ~InternalModel();

    //getters
    inline ImoObj* get_root() { return m_pRoot; }

};


//************************************************************
// Objects that form the content of the internal classes
//************************************************************

//---------------------------------------------------------------------------------------
// Any ImoObj object that wants to behave as a control must derive from this
class ControlObj
{
protected:
    bool m_fEnabled;

public:
    ControlObj() : m_fEnabled(true) {}
    ~ControlObj() {}

    //getters
    inline bool is_enabled() { return m_fEnabled; }

    //setters
    inline void enabled(bool value) { m_fEnabled = value; }

};


//===================================================
// Abstract objects hierachy
//===================================================


//---------------------------------------------------------------------------------------
// the root. Any object must derive from it
class ImoObj : public Visitable, public TreeNode<ImoObj>
{
protected:
    long m_id;
    int m_objtype;

    ImoObj(int objtype, long id=-1L);

public:
    virtual ~ImoObj();

    //getters
    inline long get_id() { return m_id; }

    //setters
    inline void set_id(long id) { m_id = id; }

    //required by Visitable parent class
	virtual void accept_visitor(BaseVisitor& v);
    virtual bool has_visitable_children() { return has_children(); }

    //children
    ImoObj* get_child_of_type(int objtype);

    //Get the name from object type
    static const string& get_name(int type);
    const string& get_name() const;

    //object classification
    inline int get_obj_type() { return m_objtype; }
	inline bool has_children() { return !is_terminal(); }

    //simple objs
        // DTOs
    inline bool is_dto() { return m_objtype >= k_imo_dto
                               && m_objtype < k_imo_simpleobj; }
    inline bool is_font_style_dto() { return m_objtype == k_imo_font_style_dto; }
    inline bool is_color_dto() { return m_objtype == k_imo_color_dto; }

        //SimpleObj
    inline bool is_simpleobj() { return m_objtype >= k_imo_simpleobj
                                     && m_objtype < k_imo_contentobj; }
    inline bool is_beam_dto() { return m_objtype == k_imo_beam_dto; }
    inline bool is_border_dto() { return m_objtype == k_imo_border_dto; }
    inline bool is_point_dto() { return m_objtype == k_imo_point_dto; }
    inline bool is_size_info() { return m_objtype == k_imo_size_dto; }
    inline bool is_slur_dto() { return m_objtype == k_imo_slur_dto; }
    inline bool is_tie_dto() { return m_objtype == k_imo_tie_dto; }
    inline bool is_tuplet_dto() { return m_objtype == k_imo_tuplet_dto; }
        // VOs
    inline bool is_bezier_info() { return m_objtype == k_imo_bezier_info; }
    inline bool is_textblock_info() { return m_objtype == k_imo_textblock_info; }
    inline bool is_cursor_info() { return m_objtype == k_imo_cursor_info; }
    inline bool is_figured_bass_info() { return m_objtype == k_imo_figured_bass_info; }
    inline bool is_instr_group() { return m_objtype == k_imo_instr_group; }
    inline bool is_line_style() { return m_objtype == k_imo_line_style; }
    inline bool is_midi_info() { return m_objtype == k_imo_midi_info; }
    inline bool is_page_info() { return m_objtype == k_imo_page_info; }
    inline bool is_param_info() { return m_objtype == k_imo_param_info; }
    inline bool is_staff_info() { return m_objtype == k_imo_staff_info; }
    inline bool is_system_info() { return m_objtype == k_imo_system_info; }
    inline bool is_text_info() { return m_objtype == k_imo_text_info; }
    inline bool is_text_style() { return m_objtype == k_imo_text_style; }
    inline bool is_style() { return m_objtype == k_imo_style; }
        // relation data objects
    inline bool is_reldataobj() { return m_objtype >= k_imo_reldataobj
                                      && m_objtype < k_imo_collection; }
    inline bool is_beam_data() { return m_objtype == k_imo_beam_data; }
    inline bool is_slur_data() { return m_objtype == k_imo_slur_data; }
    inline bool is_tie_data() { return m_objtype == k_imo_tie_data; }
    inline bool is_tuplet_data() { return m_objtype == k_imo_tuplet_data; }
        //collections
    inline bool is_music_data() { return m_objtype == k_imo_music_data; }
    inline bool is_option() { return m_objtype == k_imo_option; }
    inline bool is_reldataobjs() { return m_objtype == k_imo_reldataobjs; }
    inline bool is_styles() { return m_objtype == k_imo_styles; }
        // special collections
    inline bool is_attachments() { return m_objtype == k_imo_attachments; }
        // container objs
    inline bool is_containerobj() { return m_objtype >= k_imo_containerobj
                                        && m_objtype < k_imo_contentobj; }
    inline bool is_instrument() { return m_objtype == k_imo_instrument; }

    // content objs
    inline bool is_contentobj() { return m_objtype >= k_imo_contentobj; }

        //score objs
    inline bool is_scoreobj() { return m_objtype >= k_imo_scoreobj
                                    && m_objtype < k_imo_box_content; }
	        // staff objs
	inline bool is_staffobj() { return m_objtype >= k_imo_staffobj
                                    && m_objtype < k_imo_auxobj; }
    inline bool is_barline() { return m_objtype == k_imo_barline; }
    inline bool is_clef() { return m_objtype == k_imo_clef; }
    inline bool is_key_signature() { return m_objtype == k_imo_key_signature; }
    inline bool is_time_signature() { return m_objtype == k_imo_time_signature; }
    inline bool is_note_rest() { return m_objtype == k_imo_note
                                     || m_objtype == k_imo_rest; }
    inline bool is_note() { return m_objtype == k_imo_note; }
    inline bool is_rest() { return m_objtype == k_imo_rest; }
    inline bool is_go_back_fwd() { return m_objtype == k_imo_go_back_fwd; }
    inline bool is_metronome_mark() { return m_objtype == k_imo_metronome_mark; }
    inline bool is_control() { return m_objtype == k_imo_control; }
    inline bool is_spacer() { return m_objtype == k_imo_spacer; }
    inline bool is_figured_bass() { return m_objtype == k_imo_figured_bass; }
            // aux objs
	inline bool is_auxobj() { return m_objtype >= k_imo_auxobj
                                  && m_objtype < k_imo_box_content; }
    inline bool is_fermata() { return m_objtype == k_imo_fermata; }
    inline bool is_line() { return m_objtype == k_imo_line; }
    inline bool is_score_text() { return m_objtype == k_imo_score_text; }
    inline bool is_score_title() { return m_objtype == k_imo_score_title; }
                // relation objects
    inline bool is_relobj() { return m_objtype >= k_imo_relobj
                                  && m_objtype < k_imo_box_content; }
    inline bool is_beam() { return m_objtype == k_imo_beam; }
    inline bool is_chord() { return m_objtype == k_imo_chord; }
    inline bool is_slur() { return m_objtype == k_imo_slur; }
    inline bool is_tie() { return m_objtype == k_imo_tie; }
    inline bool is_tuplet() { return m_objtype == k_imo_tuplet; }

        // box container objs
	inline bool is_box_container() { return m_objtype >= k_imo_box_container
	                                   && m_objtype < k_imo_box_content;
    }
    inline bool is_content() { return m_objtype == k_imo_content; }
    inline bool is_document() { return m_objtype == k_imo_document; }
    inline bool is_dynamic() { return m_objtype == k_imo_dynamic; }
	inline bool is_score() { return m_objtype == k_imo_score; }

        // box content objs
	inline bool is_box_content() { return m_objtype >= k_imo_box_content
	                                   && m_objtype < k_imo_inlineobj;
    }
            //textblock
    inline bool is_textblock() { return m_objtype >= k_imo_textblock
                                     && m_objtype < k_imo_inlineobj; }
    inline bool is_heading() { return m_objtype == k_imo_heading; }
    inline bool is_paragraph() { return m_objtype == k_imo_para; }

        //inline objs
	inline bool is_inlineobj() { return m_objtype >= k_imo_inlineobj; }
    inline bool is_button() { return m_objtype == k_imo_button; }
    inline bool is_text_item() { return m_objtype == k_imo_text_item; }

        //box inline objects
    inline bool is_box_inline() { return m_objtype >= k_imo_box_inline; }
    inline bool is_inline_wrapper() { return m_objtype == k_imo_inline_wrapper; }
    inline bool is_link() { return m_objtype == k_imo_link; }

protected:
    void visit_children(BaseVisitor& v);

};

//---------------------------------------------------------------------------------------
// base class for DTOs
class ImoDto : public ImoObj
{
protected:
    ImoDto(int objtype) : ImoObj(objtype) {}

public:
    virtual ~ImoDto() {}
};

//---------------------------------------------------------------------------------------
class ImoFontStyleDto : public ImoDto
{
public:
    string name;
    float size;       // in points
    int style;        // k_normal, k_italic
    int weight;       // k_normal, k_bold

    ImoFontStyleDto()
        : ImoDto(k_imo_font_style_dto)
        , name("Liberation serif")
        , size(12)
        , style(k_normal)
        , weight(k_normal)
    {
    }

    enum { k_normal=0, k_italic, k_bold, };
};

//---------------------------------------------------------------------------------------
class ImoColorDto : public ImoDto
{
protected:
    Color m_color;
    bool m_ok;

public:
    ImoColorDto() : ImoDto(k_imo_color_dto), m_color(0, 0, 0, 255), m_ok(true) {}
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



// just auxiliary objects or data transfer objects
//---------------------------------------------------------------------------------------
class ImoSimpleObj : public ImoObj
{
protected:
    ImoSimpleObj(int objtype, long id) : ImoObj(objtype, id) {}
    ImoSimpleObj(int objtype) : ImoObj(objtype) {}

public:
    virtual ~ImoSimpleObj() {}
};

//---------------------------------------------------------------------------------------
class ImoTextStyle : public ImoSimpleObj
{
public:
    int word_spacing;
    int text_decoration;
    int vertical_align;
    int text_align;
    LUnits text_indent_length;
    LUnits word_spacing_length;     //for case k_length

    ImoTextStyle()
        : ImoSimpleObj(k_imo_text_style)
        , word_spacing(k_normal)
        , text_decoration(k_decoration_none)
        , vertical_align(k_valing_baseline)
        , text_align(k_align_left)
        , text_indent_length(0.0f)
        , word_spacing_length(0.0f) //not applicable
    {
    }

    enum { k_normal=0, k_length, };
    enum { k_decoration_none=0, k_decoration_underline, k_decoration_overline,
           k_decoration_line_through, };
    enum { k_valing_baseline, k_valing_sub, k_valing_super, k_valing_top,
           k_valing_text_top, k_valing_middle, k_valing_bottom,
           k_valing_text_bottom };
    enum { k_align_left, k_align_right, k_align_center, k_align_justify };
};

//---------------------------------------------------------------------------------------
class ImoStyle : public ImoSimpleObj
{
protected:
    string m_name;
    ImoStyle* m_pParent;
    std::map<int, LUnits> m_lunitsProps;
    std::map<int, float> m_floatProps;
    std::map<int, string> m_stringProps;
    std::map<int, int> m_intProps;
    std::map<int, Color> m_colorProps;

    friend class ImFactory;
    ImoStyle() : ImoSimpleObj(k_imo_style), m_name(), m_pParent(NULL) {}

public:
    ~ImoStyle() {}

//    //copy constructor
//    ImoStyle(ImoStyle* pStyle)
//        : ImoSimpleObj(k_imo_style)
//    {
//        m_name = "undefined";
//        m_pParent = pStyle->m_pParent;
//        m_lunitsProps = pStyle->m_lunitsProps;
//        m_floatProps = pStyle->m_floatProps;
//        m_stringProps = pStyle->m_stringProps;
//        m_intProps = pStyle->m_intProps;
//        m_colorProps = pStyle->m_colorProps;
//    }

    //text style
    enum { k_spacing_normal=0, k_length, };
    enum { k_decoration_none=0, k_decoration_underline, k_decoration_overline,
           k_decoration_line_through, };
    enum { k_valing_baseline, k_valing_sub, k_valing_super, k_valing_top,
           k_valing_text_top, k_valing_middle, k_valing_bottom,
           k_valing_text_bottom };
    enum { k_align_left, k_align_right, k_align_center, k_align_justify };

    //font style/weight
    enum { k_font_normal=0, k_italic, k_bold, };

    //style properties
    enum {
        //font
        k_font_name,
        k_font_size,
        k_font_style,
        k_font_weight,
            //text
        k_word_spacing,
        k_text_decoration,
        k_vertical_align,
        k_text_align,
        k_text_indent_length,
        k_word_spacing_length,
            //color and background
        k_color,
        k_background_color,
            //margin
        k_margin_top,
        k_margin_bottom,
        k_margin_left,
        k_margin_right,
            //padding
        k_padding_top,
        k_padding_bottom,
        k_padding_left,
        k_padding_right,

//    ////border
//    k_border_top,
//    k_border_bottom,
//    k_border_left,
//    k_border_right,

            //border width
        k_border_width_top,
        k_border_width_bottom,
        k_border_width_left,
        k_border_width_right,
    };

    //general
    inline const std::string& get_name() { return m_name; }
    inline void set_name(const std::string& value) { m_name = value; }
    inline void set_parent_style(ImoStyle* pStyle) { m_pParent = pStyle; }

    //utility
    LUnits em_to_LUnits(float em) {
        return pt_to_LUnits( get_float_property(ImoStyle::k_font_size) * em );
    }
    inline bool is_bold() { return get_int_property(ImoStyle::k_font_weight) == k_bold; }
    inline bool is_italic() { return get_int_property(ImoStyle::k_font_style) == k_italic; }

    //setters
	void set_string_property(int prop, const string& value)
	{
	    m_stringProps[prop] = value;
	}
	void set_float_property(int prop, float value)
	{
	    m_floatProps[prop] = value;
	}
    void set_int_property(int prop, int value)
	{
	    m_intProps[prop] = value;
	}
    void set_lunits_property(int prop, LUnits value)
	{
	    m_lunitsProps[prop] = value;
	}
    void set_color_property(int prop, Color value)
	{
	    m_colorProps[prop] = value;
	}
	//special setters
	void set_margin_property(LUnits value)
	{
	    set_lunits_property(ImoStyle::k_margin_left, value);
	    set_lunits_property(ImoStyle::k_margin_top, value);
	    set_lunits_property(ImoStyle::k_margin_right, value);
	    set_lunits_property(ImoStyle::k_margin_bottom, value);
	}
	void set_padding_property(LUnits value)
	{
	    set_lunits_property(ImoStyle::k_padding_left, value);
	    set_lunits_property(ImoStyle::k_padding_top, value);
	    set_lunits_property(ImoStyle::k_padding_right, value);
	    set_lunits_property(ImoStyle::k_padding_bottom, value);
	}
	void set_border_width_property(LUnits value)
	{
	    set_lunits_property(ImoStyle::k_border_width_left, value);
	    set_lunits_property(ImoStyle::k_border_width_top, value);
	    set_lunits_property(ImoStyle::k_border_width_right, value);
	    set_lunits_property(ImoStyle::k_border_width_bottom, value);
	}


    //getters
    float get_float_property(int prop)
    {
        map<int, float>::const_iterator it = m_floatProps.find(prop);
        if (it != m_floatProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_float_property(prop);
        else
            throw std::runtime_error( "[ImoStyle::get_float_property]. No parent" );
    }

    LUnits get_lunits_property(int prop)
    {
        map<int, LUnits>::const_iterator it = m_lunitsProps.find(prop);
        if (it != m_lunitsProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_lunits_property(prop);
        else
            throw std::runtime_error( "[ImoStyle::get_float_property]. No parent" );
    }

    const string& get_string_property(int prop)
    {
        map<int, string>::const_iterator it = m_stringProps.find(prop);
        if (it != m_stringProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_string_property(prop);
        else
            throw std::runtime_error( "[ImoStyle::get_string_property]. No parent" );
    }

    int get_int_property(int prop)
    {
        map<int, int>::const_iterator it = m_intProps.find(prop);
        if (it != m_intProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_int_property(prop);
        else
            throw std::runtime_error( "[ImoStyle::get_int_property]. No parent" );
    }

    Color get_color_property(int prop)
    {
        map<int, Color>::const_iterator it = m_colorProps.find(prop);
        if (it != m_colorProps.end())
            return it->second;
        else if (m_pParent)
            return m_pParent->get_color_property(prop);
        else
            throw std::runtime_error( "[ImoStyle::get_color_property]. No parent" );
    }

};

//---------------------------------------------------------------------------------------
// Any object for the renderizable content of a document
class ImoContentObj : public ImoObj
{
protected:
    ImoStyle* m_pStyle;

    Tenths m_txUserLocation;
    Tenths m_tyUserLocation;
    bool m_fVisible;

    ImoContentObj(int objtype);
    ImoContentObj(long id, int objtype);

public:
    virtual ~ImoContentObj();

    //getters
    inline Tenths get_user_location_x() { return m_txUserLocation; }
    inline Tenths get_user_location_y() { return m_tyUserLocation; }
    inline bool is_visible() { return m_fVisible; }

    //setters
    inline void set_user_location_x(Tenths tx) { m_txUserLocation = tx; }
    inline void set_user_location_y(Tenths ty) { m_tyUserLocation = ty; }
    inline void set_visible(bool visible) { m_fVisible = visible; }

    //root
    ImoDocument* get_document();
    Document* get_the_document();

    //attachments (first child)
    ImoAttachments* get_attachments();
    bool has_attachments();
    int get_num_attachments();
    ImoAuxObj* get_attachment(int i);
    void add_attachment(Document* pDoc, ImoAuxObj* pAO);
    void remove_attachment(ImoAuxObj* pAO);
    ImoAuxObj* find_attachment(int type);

    //style
    ImoStyle* get_style();
    ImoStyle* copy_style_as(const std::string& name);
    void set_style(ImoStyle* pStyle);

//    //general
//    inline const std::string& get_name() { return m_name; }
//    inline void set_name(const std::string& value) { m_name = value; }
//
//    //utility
//    LUnits em_to_LUnits(float em) {
//        return pt_to_LUnits( font_size() * em );
//    }

    //font
    inline const std::string font_name() {
        ImoStyle* pStyle = get_style();
        return pStyle ? pStyle->get_string_property(ImoStyle::k_font_name) : "";
    }
//    inline float font_size() { return m_font.size; }
//    inline int font_style() { return m_font.style; }
//    inline int font_weight() { return m_font.weight; }
//    inline bool is_bold() { return font_weight() == ImoStyle::k_bold; }
//    inline bool is_italic() { return font_style() == ImoStyle::k_italic; }
//    inline void font_name(const std::string& value) { m_font.name = value; }
//    inline void font_size(float points) { m_font.size = points; }
//    inline void font_style(int value) { m_font.style = value; }
//    inline void font_weight(int value) { m_font.weight = value; }
//
//    //text
//    inline int word_spacing() { return m_text.word_spacing; }
//    inline int text_decoration() { return m_text.text_decoration; }
//    inline int vertical_align() { return m_text.vertical_align; }
//    inline int text_align() { return m_text.text_align; }
//    inline LUnits text_indent_length() { return m_text.text_indent_length; }
//    inline LUnits word_spacing_length() { return m_text.word_spacing_length; }
//    inline void word_spacing(int value) { m_text.word_spacing = value; }
//    inline void text_decoration(int value) { m_text.text_decoration = value; }
//    inline void vertical_align(int value) { m_text.vertical_align = value; }
//    inline void text_align(int value) { m_text.text_align = value; }
//    inline void text_indent_length(LUnits value) { m_text.text_indent_length = value; }
//    inline void word_spacing_length(LUnits value) { m_text.word_spacing_length = value; }
//
//    //color and background
//    inline Color color() { return m_color; }
//    inline void color(Color value) { m_color = value; }
//    inline Color background_color() { return m_bgcolor; }
//    inline void background_color(Color value) { m_bgcolor = value; }

    //margin
    inline LUnits margin_top() {
        ImoStyle* pStyle = get_style();
        return pStyle ? pStyle->get_lunits_property(ImoStyle::k_margin_top) : 0.0f;
    }
    inline LUnits margin_bottom() {
        ImoStyle* pStyle = get_style();
        return pStyle ? pStyle->get_lunits_property(ImoStyle::k_margin_bottom) : 0.0f;
    }
//    inline LUnits get_lunits_property(ImoStyle::k_margin_left) { return m_box.margin_left; }
//    inline LUnits get_lunits_property(ImoStyle::k_margin_right) { return m_box.margin_right; }
//    inline void margin(LUnits value) { m_box.margin(value); }
//    inline void margin_top(LUnits value) { m_box.margin_top = value; }
//    inline void margin_bottom(LUnits value) { m_box.margin_bottom = value; }
//    inline void margin_left(LUnits value) { m_box.margin_left = value; }
//    inline void margin_right(LUnits value) { m_box.margin_right = value; }
//
//    //padding
//    inline LUnits get_lunits_property(ImoStyle::k_padding_top) { return m_box.padding_top; }
//    inline LUnits get_lunits_property(ImoStyle::k_padding_bottom) { return m_box.padding_bottom; }
//    inline LUnits get_lunits_property(ImoStyle::k_padding_left) { return m_box.padding_left; }
//    inline LUnits get_lunits_property(ImoStyle::k_padding_right) { return m_box.padding_right; }
//    inline void padding(LUnits value) { m_box.padding(value); }
//    inline void padding_top(LUnits value) { m_box.padding_top = value; }
//    inline void padding_bottom(LUnits value) { m_box.padding_bottom = value; }
//    inline void padding_left(LUnits value) { m_box.padding_left = value; }
//    inline void padding_right(LUnits value) { m_box.padding_right = value; }
//
//    ////border
//    //inline LUnits border_top() { return m_box.border_top; }
//    //inline LUnits border_bottom() { return m_box.border_bottom; }
//    //inline LUnits border_left() { return m_box.border_left; }
//    //inline LUnits border_right() { return m_box.border_right; }
//    //inline void border(LUnits value) { m_box.border(value); }
//    //inline void border_top(LUnits value) { m_box.border_top = value; }
//    //inline void border_bottom(LUnits value) { m_box.border_bottom = value; }
//    //inline void border_left(LUnits value) { m_box.border_left = value; }
//    //inline void border_right(LUnits value) { m_box.border_right = value; }
//
//    //border width
//    inline LUnits get_lunits_property(ImoStyle::k_border_width_top) { return m_box.border_width_top; }
//    inline LUnits get_lunits_property(ImoStyle::k_border_width_bottom) { return m_box.border_width_bottom; }
//    inline LUnits get_lunits_property(ImoStyle::k_border_width_left) { return m_box.border_width_left; }
//    inline LUnits get_lunits_property(ImoStyle::k_border_width_right) { return m_box.border_width_right; }
//    inline void border_width(LUnits value) { m_box.border_width(value); }
//    inline void border_width_top(LUnits value) { m_box.border_width_top = value; }
//    inline void border_width_bottom(LUnits value) { m_box.border_width_bottom = value; }
//    inline void border_width_left(LUnits value) { m_box.border_width_left = value; }
//    inline void border_width_right(LUnits value) { m_box.border_width_right = value; }

};

//---------------------------------------------------------------------------------------
class ImoCollection : public ImoSimpleObj
{
public:
    virtual ~ImoCollection() {}

    //contents
    ImoContentObj* get_item(int iItem) {   //iItem = 0..n-1
        return dynamic_cast<ImoContentObj*>( get_child(iItem) );
    }
    inline int get_num_items() { return get_num_children(); }
    inline void remove_item(ImoContentObj* pItem) { remove_child(pItem); }

protected:
    ImoCollection(int objtype) : ImoSimpleObj(objtype) {}
};

//---------------------------------------------------------------------------------------
class ImoAttachments : public ImoSimpleObj
{
protected:
    std::list<ImoAuxObj*> m_attachments;

    friend class ImFactory;
    friend class ImoContentObj;
    ImoAttachments() : ImoSimpleObj(k_imo_attachments) {}

public:
    ~ImoAttachments();

    //overrides, to traverse this special node
	void accept_visitor(BaseVisitor& v);
    bool has_visitable_children() { return get_num_items() > 0; }

    //contents
    ImoAuxObj* get_item(int iItem);   //iItem = 0..n-1
    inline int get_num_items() { return int(m_attachments.size()); }
    inline std::list<ImoAuxObj*>& get_attachments() { return m_attachments; }
    void remove(ImoAuxObj* pAO);
    void add(ImoAuxObj* pAO);
    ImoAuxObj* find_item_of_type(int type);
    void remove_from_all_relations(ImoStaffObj* pSO);
    //void remove_all_attachments();

protected:
    static int get_priority(int type);
};


//---------------------------------------------------------------------------------------
// ContainerObj: A collection of containers and contained objs.
class ImoContainerObj : public ImoObj
{
protected:
    ImoContainerObj(int objtype) : ImoObj(objtype) {}

public:
    virtual ~ImoContainerObj() {}

};

//---------------------------------------------------------------------------------------
// ImoBoxLevelObj: abstract class for all box-level objects
class ImoBoxLevelObj : public ImoContentObj
{
protected:
    ImoBoxLevelObj(long id, int objtype) : ImoContentObj(id, objtype) {}
    ImoBoxLevelObj(int objtype) : ImoContentObj(objtype) {}

public:
    virtual ~ImoBoxLevelObj() {}

};

//---------------------------------------------------------------------------------------
// ImoBoxContainer: A box-level container for box-level objects
class ImoBoxContainer : public ImoBoxLevelObj, public BlockLevelCreatorApi
{
protected:
    ImoBoxContainer(long id, int objtype)
        : ImoBoxLevelObj(id, objtype)
        , BlockLevelCreatorApi()
    {
        set_box_level_creator_api_parent(this);
    }
    ImoBoxContainer(int objtype)
        : ImoBoxLevelObj(objtype)
        , BlockLevelCreatorApi()
    {
        set_box_level_creator_api_parent(this);
    }

public:
    virtual ~ImoBoxContainer() {}


//    //API
//    ImoParagraph* add_paragraph(ImoStyle* pStyle=NULL);
//
//protected:
//    virtual ImoContentObj* get_container_node()=0;

};

//---------------------------------------------------------------------------------------
// ImoBoxContent: A box-level container for ImoInlineObj objs.
class ImoBoxContent : public ImoBoxLevelObj
{
protected:
    ImoBoxContent(long id, int objtype) : ImoBoxLevelObj(id, objtype) {}
    ImoBoxContent(int objtype) : ImoBoxLevelObj(objtype) {}

public:
    virtual ~ImoBoxContent() {}

};

//---------------------------------------------------------------------------------------
// ImoInlineObj: Abstract class from which any ImoBoxContent content object must derive
class ImoInlineObj : public ImoContentObj
{
protected:
    ImoInlineObj(int objtype) : ImoContentObj(objtype) {}

public:
    virtual ~ImoInlineObj() {}

};

//---------------------------------------------------------------------------------------
class ImoBoxInline : public ImoInlineObj, public InlineLevelCreatorApi
{
protected:
    USize m_size;

    ImoBoxInline(int objtype)
        : ImoInlineObj(objtype), InlineLevelCreatorApi(), m_size(0.0f, 0.0f)
    {
        set_inline_level_creator_api_parent(this);
    }
    ImoBoxInline(int objtype, const USize& size)
        : ImoInlineObj(objtype), InlineLevelCreatorApi(), m_size(size)
    {
        set_inline_level_creator_api_parent(this);
    }
    ImoBoxInline(int objtype, LUnits width, LUnits height)
        : ImoInlineObj(objtype), InlineLevelCreatorApi(), m_size(width, height)
    {
        set_inline_level_creator_api_parent(this);
    }

public:
    virtual ~ImoBoxInline() {}

    //content
    inline int get_num_items() { return get_num_children(); }
    inline void add_item(ImoInlineObj* pItem) { append_child(pItem); }
    inline void remove_item(ImoContentObj* pItem) { remove_child(pItem); }
    inline ImoInlineObj* get_first_item() {
        return dynamic_cast<ImoInlineObj*>( get_first_child() );
    }

    //size
    inline USize& get_size() { return m_size; }
    inline LUnits get_width() { return m_size.width; }
    inline LUnits get_height() { return m_size.height; }
    inline void set_size(const USize& size) { m_size = size; }
    inline void set_width(LUnits value) { m_size.width = value; }
    inline void set_height(LUnits value) { m_size.height = value; }

};

//---------------------------------------------------------------------------------------
// An inline-block container for inline objs
class ImoInlineWrapper : public ImoBoxInline
{
protected:
    friend class ImFactory;
    ImoInlineWrapper() : ImoBoxInline(k_imo_inline_wrapper) {}

public:
    ~ImoInlineWrapper() {}
};

//---------------------------------------------------------------------------------------
// An inline-block wrapper to add link properties to wrapped objs
class ImoLink : public ImoBoxInline
{
private:
    string m_url;

    friend class ImFactory;
    ImoLink() : ImoBoxInline(k_imo_link) {}

public:
    ~ImoLink() {}

    //url
    inline string& get_url() { return m_url; }
    inline void set_url(const string& url) { m_url = url; }
};

//---------------------------------------------------------------------------------------
// Any atomic displayable object for a Score
class ImoScoreObj : public ImoContentObj
{
protected:
    Color m_color;

    ImoScoreObj(long id, int objtype) : ImoContentObj(id, objtype) {}
    ImoScoreObj(int objtype) : ImoContentObj(objtype) {}

public:
    virtual ~ImoScoreObj() {}

    //getters
    inline Color& get_color() { return m_color; }

    //setters
    inline void set_color(Color color) { m_color = color; }

};

//---------------------------------------------------------------------------------------
// StaffObj: An object attached to an Staff. Consume time
class ImoStaffObj : public ImoScoreObj
{
protected:
    int m_staff;

    ImoStaffObj(int objtype) : ImoScoreObj(objtype), m_staff(0) {}
    ImoStaffObj(long id, int objtype) : ImoScoreObj(id, objtype), m_staff(0) {}

public:
    virtual ~ImoStaffObj();

    //attachments: relobjs
    void include_in_relation(Document* pDoc, ImoRelObj* pRelObj,
                             ImoRelDataObj* pData=NULL);
    void remove_from_relation(ImoRelObj* pRelObj);
    void remove_but_not_delete_relation(ImoRelObj* pRelObj);

    //reldata objects
    bool has_reldataobjs();
    ImoReldataobjs* get_reldataobjs();
    void add_reldataobj(Document* pDoc, ImoSimpleObj* pSO);
    int get_num_reldataobjs();
    ImoSimpleObj* get_reldataobj(int i);
    void remove_reldataobj(ImoSimpleObj* pSO);
    ImoSimpleObj* find_reldataobj(int type);

    //getters
    virtual float get_duration() { return 0.0f; }
    inline int get_staff() { return m_staff; }

    //setters
    virtual void set_staff(int staff) { m_staff = staff; }


};

//---------------------------------------------------------------------------------------
// AuxObj: a BoxObj that must be attached to other objects but not
//         directly to an staff. Do not consume time
class ImoAuxObj : public ImoScoreObj
{
protected:
    ImoAuxObj(int objtype) : ImoScoreObj(objtype) {}

public:
    virtual ~ImoAuxObj() {}

protected:
    ImoAuxObj(ImoContentObj* pOwner, long id, int objtype) : ImoScoreObj(id, objtype) {}

};

//---------------------------------------------------------------------------------------
//An abstract object containing the specifica data for one node in a relation
class ImoRelDataObj : public ImoSimpleObj
{
public:
    virtual ~ImoRelDataObj() {}

protected:
    ImoRelDataObj(int objtype) : ImoSimpleObj(objtype) {}

};

//---------------------------------------------------------------------------------------
//An abstract object relating two or more StaffObjs
class ImoRelObj : public ImoAuxObj
{
protected:
	std::list< pair<ImoStaffObj*, ImoRelDataObj*> > m_relatedObjects;

public:
    virtual ~ImoRelObj();

    void push_back(ImoStaffObj* pSO, ImoRelDataObj* pData);
    void remove(ImoStaffObj* pSO);
    void remove_all();
    inline int get_num_objects() { return int( m_relatedObjects.size() ); }
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& get_related_objects() {
        return m_relatedObjects;
    }
    ImoStaffObj* get_start_object() { return m_relatedObjects.front().first; }
    ImoStaffObj* get_end_object() { return m_relatedObjects.back().first; }
    ImoRelDataObj* get_start_data() { return m_relatedObjects.front().second; }
    ImoRelDataObj* get_end_data() { return m_relatedObjects.back().second; }
    ImoRelDataObj* get_data_for(ImoStaffObj* pSO);

    virtual int get_min_number_for_autodelete() { return 2; }

protected:
    ImoRelObj(int objtype) : ImoAuxObj(objtype) {}

};


//=======================================================================================
// Simple objects
//=======================================================================================

//---------------------------------------------------------------------------------------
class ImoBeamData : public ImoRelDataObj
{
protected:
    int m_beamNum;
    int m_beamType[6];
    bool m_repeat[6];

    friend class ImFactory;
    ImoBeamData(ImoBeamDto* pDto);

public:
    ~ImoBeamData() {}

    //getters
    inline int get_beam_number() { return m_beamNum; }
    inline int get_beam_type(int level) { return m_beamType[level]; }
    inline bool get_repeat(int level) { return m_repeat[level]; }

    //setters
    inline void set_beam_type(int level, int type) { m_beamType[level] = type; }

    //properties
    bool is_end_of_beam();
    bool is_start_of_beam();
};

//---------------------------------------------------------------------------------------
class ImoBeamDto : public ImoSimpleObj
{
protected:
    int m_beamType[6];
    int m_beamNum;
    bool m_repeat[6];
    LdpElement* m_pBeamElm;
    ImoNoteRest* m_pNR;

public:
    ImoBeamDto();
    ImoBeamDto(LdpElement* pBeamElm);
    ~ImoBeamDto() {}

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
    void set_beam_type(string& segments);
    void set_repeat(int level, bool value);

    //properties
    bool is_end_of_beam();
    bool is_start_of_beam();

    //required by RelationBuilder
    int get_item_number() { return get_beam_number(); }
    bool is_start_of_relation() { return is_start_of_beam(); }
    bool is_end_of_relation() { return is_end_of_beam(); }

};

//---------------------------------------------------------------------------------------
class ImoBezierInfo : public ImoSimpleObj
{
protected:
    TPoint m_tPoints[4];   //start, end, ctrol1, ctrol2

    friend class ImFactory;
    ImoBezierInfo() : ImoSimpleObj(k_imo_bezier_info) {}

public:
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
    Color         m_color;
    Tenths        m_width;
    ELineStyle    m_style;

public:
    ImoBorderDto() : ImoSimpleObj(k_imo_border_dto)
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
class ImoChord : public ImoRelObj
{
protected:
    friend class ImFactory;
    ImoChord() : ImoRelObj(k_imo_chord) {}

public:
    ~ImoChord() {}
};

//---------------------------------------------------------------------------------------
class ImoCursorInfo : public ImoSimpleObj
{
protected:
    int m_instrument;
    int m_staff;
    float m_time;
    long m_id;

    friend class ImFactory;
    ImoCursorInfo() : ImoSimpleObj(k_imo_cursor_info)
                    , m_instrument(0), m_staff(0), m_time(0.0f), m_id(-1L) {}

public:
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
class ImoLineStyle : public ImoSimpleObj
{
protected:
    ELineStyle  m_lineStyle;
    ELineEdge   m_startEdge;
    ELineEdge   m_endEdge;
    ELineCap    m_startStyle;
    ELineCap    m_endStyle;
    Color       m_color;
    Tenths      m_width;
    TPoint      m_startPoint;
    TPoint      m_endPoint;

    friend class ImFactory;
    friend class ImoTextBox;
    friend class ImoLine;
    ImoLineStyle()
        : ImoSimpleObj(k_imo_line_style)
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

    ImoLineStyle(ImoLineStyle& info)
        : ImoSimpleObj(k_imo_line_style)
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

public:
    ~ImoLineStyle() {}

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
class ImoTextBlockInfo : public ImoSimpleObj
{
//<location>[<size>][<color>][<border>]
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
    ImoTextBlockInfo()
        : ImoSimpleObj(k_imo_textblock_info)
        , m_size(TSize(160.0f, 100.0f))
        , m_topLeftPoint(TPoint(0.0f, 0.0f))
        , m_bgColor( Color(255, 255, 255, 255) )
        , m_borderColor( Color(0, 0, 0, 255) )
        , m_borderWidth(1.0f)
        , m_borderStyle(k_line_solid)
    {
    }

    ~ImoTextBlockInfo() {}

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

    friend class ImFactory;
    friend class ImoInstrument;
    ImoMidiInfo();

public:
    ~ImoMidiInfo() {}

    //getters
    inline int get_instrument() { return m_instr; }
    inline int get_channel() { return m_channel; }

    //setters
    inline void set_instrument(int value) { m_instr = value; }
    inline void set_channel(int value) { m_channel = value; }

};

//---------------------------------------------------------------------------------------
class ImoTextInfo : public ImoSimpleObj
{
protected:
    string m_text;
    ImoStyle* m_pStyle;

    friend class ImFactory;
    friend class ImoTextBox;
    friend class ImoButton;
    friend class ImoScoreText;
    friend class ImoTextItem;
    ImoTextInfo() : ImoSimpleObj(k_imo_text_info), m_text(""), m_pStyle(NULL) {}

public:
    ~ImoTextInfo() {}

    //getters
    inline string& get_text() { return m_text; }
    inline ImoStyle* get_style() { return m_pStyle; }
    const std::string& get_font_name();
    float get_font_size();
    int get_font_style();
    int get_font_weight();
    Color get_color();

    //setters
    inline void set_text(const string& text) { m_text = text; }
    inline void set_style(ImoStyle* pStyle) { m_pStyle = pStyle; }
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

    friend class ImFactory;
    friend class ImoDocument;
    friend class ImoScore;
    ImoPageInfo();
    ImoPageInfo(ImoPageInfo& dto);

public:
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
class ImoBarline : public ImoStaffObj
{
protected:
    int m_barlineType;

    friend class ImFactory;
    ImoBarline(): ImoStaffObj(k_imo_barline), m_barlineType(k_simple) {}

public:
    ~ImoBarline() {}

	enum { k_simple=0, k_double, k_start, k_end, k_end_repetition, k_start_repetition,
           k_double_repetition, };

    //barline type
    inline int get_type() { return m_barlineType; }
    inline void set_type(int barlineType) { m_barlineType = barlineType; }

    //overrides: barlines always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};

//---------------------------------------------------------------------------------------
class ImoBeam : public ImoRelObj
{
protected:
    friend class ImFactory;
    ImoBeam() : ImoRelObj(k_imo_beam) {}

public:
    ~ImoBeam() {}

    //type of beam
    enum { k_none = 0, k_begin, k_continue, k_end, k_forward, k_backward, };

};

//---------------------------------------------------------------------------------------
class ImoBlock : public ImoAuxObj
{
protected:
    ImoTextBlockInfo m_box;

    ImoBlock(int objtype) : ImoAuxObj(objtype) {}
    ImoBlock(int objtype, ImoTextBlockInfo& box) : ImoAuxObj(objtype), m_box(box) {}

public:
    ~ImoBlock() {}

};

//---------------------------------------------------------------------------------------
class ImoTextBox : public ImoBlock
{
protected:
    ImoTextInfo m_text;
    ImoLineStyle m_line;
    bool m_fHasAnchorLine;
    //TPoint m_anchorJoinPoint;     //point on the box rectangle

    friend class ImFactory;
    ImoTextBox() : ImoBlock(k_imo_text_box), m_fHasAnchorLine(false) {}
    ImoTextBox(ImoTextBlockInfo& box) : ImoBlock(k_imo_text_box, box)
                                      , m_fHasAnchorLine(false) {}

public:
    ~ImoTextBox() {}

    inline ImoTextBlockInfo* get_box_info() { return &m_box; }
    inline ImoLineStyle* get_anchor_line_info() { return &m_line; }
    inline bool has_anchor_line() { return m_fHasAnchorLine; }

    inline const std::string& get_text() { return m_text.get_text(); }
    inline void set_text(ImoTextInfo* pTI) { m_text = *pTI; }
    inline void set_anchor_line(ImoLineStyle* pLine) {
        m_line = *pLine;
        m_fHasAnchorLine = true;
    }
};

//---------------------------------------------------------------------------------------
class ImoButton : public ImoInlineObj, public ControlObj
{
protected:
    bool m_fOnClick;

    ImoTextInfo m_text;
    USize m_size;
    Color m_bgColor;

    friend class ImFactory;
    ImoButton()
        : ImoInlineObj(k_imo_button)
        , m_fOnClick(true)
        , m_bgColor( Color(255,255,255) )
    {
    }

public:
    ~ImoButton() {}

    //getters
    inline string& get_label() { return m_text.get_text(); }
    inline ImoStyle* get_style() { return m_text.get_style(); }
    inline ImoTextInfo* get_text_info() { return &m_text; }
    inline USize get_size() { return m_size; }
    inline LUnits get_width() { return m_size.width; }
    inline LUnits get_height() { return m_size.height; }
    inline Color get_bg_color() { return m_bgColor; }

    //setters
    inline void set_style(ImoStyle* pStyle) {
        m_text.set_style(pStyle);
        if (pStyle)
            m_bgColor = pStyle->get_color_property(ImoStyle::k_background_color);
    }
    inline void set_bg_color(Color color) { m_bgColor = color; }
    inline void set_label(const string& text) { m_text.set_text(text); }
    inline void set_size(const USize& size) { m_size = size; }
    inline void set_width(LUnits value) { m_size.width = value; }
    inline void set_height(LUnits value) { m_size.height = value; }

    //control
    inline void enable_on_click(bool enabled) { m_fOnClick = enabled; }
    inline bool on_click_enabled() { return m_fOnClick; }
};

//---------------------------------------------------------------------------------------
class ImoClef : public ImoStaffObj
{
protected:
    int m_clefType;
    int m_symbolSize;

    friend class ImFactory;
    ImoClef()
        : ImoStaffObj(k_imo_clef)
        , m_clefType(k_clef_G2)
        , m_symbolSize(k_size_default)
    {
    }

public:
    ~ImoClef() {}

    //getters and setters
    inline int get_clef_type() { return m_clefType; }
    inline void set_clef_type(int type) { m_clefType = type; }
    inline int get_symbol_size() { return m_symbolSize; }
    inline void set_symbol_size(int symbolSize) { m_symbolSize = symbolSize; }

};

//---------------------------------------------------------------------------------------
class ImoContent : public ImoBoxContainer
{
protected:
    Document* m_pOwner;

    friend class ImFactory;
    ImoContent(Document* pOwner) : ImoBoxContainer(k_imo_content), m_pOwner(pOwner) {}
    ImoContent(int objtype) : ImoBoxContainer(objtype) {}   //constructor for ImoDynamic

public:
    virtual ~ImoContent() {}

    inline Document* get_owner() { return m_pOwner; }

    //contents
    ImoContentObj* get_item(int iItem) {   //iItem = 0..n-1
        return dynamic_cast<ImoContentObj*>( get_child(iItem) );
    }
    inline int get_num_items() { return get_num_children(); }
    inline void remove_item(ImoContentObj* pItem) { remove_child(pItem); }

//protected:
//    //mandatory overrides
//    ImoContentObj* get_container_node() { return this; }

};

//---------------------------------------------------------------------------------------
class ImoDynamic : public ImoContent
{
protected:
    string m_classid;
    std::list<ImoParamInfo*> m_params;
    DynGenerator* m_generator;

    friend class ImFactory;
    ImoDynamic() : ImoContent(k_imo_dynamic), m_classid(""), m_generator(NULL) {}

public:
    virtual ~ImoDynamic();

    //construction
    inline void set_classid(const string& value) { m_classid = value; }
    inline void add_param(ImoParamInfo* pParam) { m_params.push_back(pParam); }
    void set_generator(DynGenerator* pGenerator);

    //accessors
    inline string& get_classid() { return m_classid; }
    inline std::list<ImoParamInfo*>& get_params() { return m_params; }
    inline DynGenerator* get_generator() { return m_generator; }

};

//---------------------------------------------------------------------------------------
class ImoControl : public ImoStaffObj
{
protected:
    friend class ImFactory;
    ImoControl() : ImoStaffObj(k_imo_control) {}

public:
    ~ImoControl() {}

    //getters & setters
};

//---------------------------------------------------------------------------------------
class ImoDocument : public ImoBoxContainer  //ImoContainerObj
{
protected:
    Document* m_pOwner;
    string m_version;
    ImoPageInfo m_pageInfo;
    std::list<ImoStyle*> m_privateStyles;

    friend class ImFactory;
    ImoDocument(Document* owner, const std::string& version="");

public:
    ~ImoDocument();

    //info
    inline std::string& get_version() { return m_version; }
    inline void set_version(const string& version) { m_version = version; }
    inline Document* get_owner() { return m_pOwner;; }

    //content
    ImoContentObj* get_content_item(int iItem);
    int get_num_content_items();
    ImoContent* get_content();

    //document intended paper size
    void add_page_info(ImoPageInfo* pPI);
    inline ImoPageInfo* get_page_info() { return &m_pageInfo; }
    inline LUnits get_paper_width() { return m_pageInfo.get_page_width(); }
    inline LUnits get_paper_height() { return m_pageInfo.get_page_height(); }

    //styles
    ImoStyles* get_styles();
    void add_style(ImoStyle* pStyle);
    ImoStyle* find_style(const std::string& name);
    ImoStyle* get_default_style();
    ImoStyle* get_style_or_default(const std::string& name);

    //user API
    ImoStyle* create_style(const string& name, const string& parent="Default style");
    ImoStyle* create_private_style(const string& parent="Default style");

//        //factory methods for ImoObj objects
//    ImoButton* create_button(long id, const string& label, const USize& size,
//                             ImoStyle* pStyle=NULL);
//    ImoInlineWrapper* create_inline_box();
    //ImoTextItem* create_text_item(const string& text, ImoStyle* pStyle=NULL);
    void append_content_item(ImoContentObj* pItem);


    //cursor
    //TODO
    void add_cursor_info(ImoCursorInfo* pCursor) {};

protected:
    void add_private_style(ImoStyle* pStyle);

    ////mandatory overrides
    //ImoContentObj* get_container_node() { return get_content(); }

};

//---------------------------------------------------------------------------------------
class ImoFermata : public ImoAuxObj
{
protected:
    int m_placement;
    int m_symbol;

    friend class ImFactory;
    ImoFermata()
        : ImoAuxObj(k_imo_fermata)
        , m_placement(k_placement_default)
        , m_symbol(k_normal)
    {
    }

public:
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

    friend class ImFactory;
    ImoGoBackFwd()
        : ImoStaffObj(k_imo_go_back_fwd), m_fFwd(true), m_rTimeShift(0.0f)
        , SHIFT_START_END(100000000.0f)
    {}

public:
    ~ImoGoBackFwd() {}

    //getters and setters
    inline bool is_forward() { return m_fFwd; }
    inline void set_forward(bool fFwd) { m_fFwd = fFwd; }
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

    friend class ImFactory;
    friend class ImoInstrument;
    friend class ImoInstrGroup;
    ImoScoreText()
        : ImoAuxObj(k_imo_score_text), m_text(), m_hAlign(k_halign_left) {}
    ImoScoreText(int objtype)
        : ImoAuxObj(objtype), m_text(), m_hAlign(k_halign_left) {}

public:
    virtual ~ImoScoreText() {}

    //getters
    inline int get_h_align() { return m_hAlign; }
    inline string& get_text() { return m_text.get_text(); }
    inline ImoStyle* get_style() { return m_text.get_style(); }
    inline ImoTextInfo* get_text_info() { return &m_text; }

    //setters
    inline void set_text(const std::string& value) { m_text.set_text(value); }
    inline void set_h_align(int value) { m_hAlign = value; }
    inline void set_style(ImoStyle* pStyle) { m_text.set_style(pStyle); }

};

//---------------------------------------------------------------------------------------
class ImoScoreTitle : public ImoScoreText
{
protected:
    int m_hAlign;

    friend class ImFactory;
    ImoScoreTitle() : ImoScoreText(k_imo_score_title), m_hAlign(k_halign_center) {}

public:
    ~ImoScoreTitle() {}

    inline int get_h_align() { return m_hAlign; }
    inline void set_h_align(int value) { m_hAlign = value; }
    inline void set_style(ImoStyle* pStyle) { m_text.set_style(pStyle); }
    inline ImoStyle* get_style() { return m_text.get_style(); }
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

    friend class ImFactory;
    ImoInstrGroup();

public:
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
    Document*       m_pDoc;
    ImoScoreText    m_name;
    ImoScoreText    m_abbrev;
    ImoMidiInfo     m_midi;
    ImoInstrGroup*  m_pGroup;
    std::list<ImoStaffInfo*> m_staves;

    friend class ImFactory;
    ImoInstrument(Document* pDoc);

public:
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

    //API
    ImoClef* add_clef(int type);
    ImoObj* add_object(const string& ldpsource);

protected:

};

//---------------------------------------------------------------------------------------
class ImoInstruments : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoInstruments() : ImoCollection(k_imo_instruments) {}

public:
    ~ImoInstruments() {}

};

//---------------------------------------------------------------------------------------
class ImoInstrGroups : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoInstrGroups() : ImoCollection(k_imo_instrument_groups) {}

public:
    ~ImoInstrGroups() {}

};

//---------------------------------------------------------------------------------------
class ImoKeySignature : public ImoStaffObj
{
protected:
    int m_keyType;

    friend class ImFactory;
    ImoKeySignature() : ImoStaffObj(k_imo_key_signature), m_keyType(k_key_undefined) {}

public:
    ~ImoKeySignature() {}

    //getters and setters
    inline int get_key_type() { return m_keyType; }
    inline void set_key_type(int type) { m_keyType = type; }

    //overrides: key signatures always in staff 0
    void set_staff(int staff) { m_staff = 0; }

};

//---------------------------------------------------------------------------------------
class ImoLine : public ImoAuxObj
{
    ImoLineStyle* m_pStyle;

    friend class ImFactory;
    ImoLine() : ImoAuxObj(k_imo_line), m_pStyle(NULL) {}

public:
    ~ImoLine() { delete m_pStyle; }

    inline ImoLineStyle* get_line_info() { return m_pStyle; }
    inline void set_line_style(ImoLineStyle* pStyle) { m_pStyle = pStyle; }

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

    friend class ImFactory;
    ImoMetronomeMark()
        : ImoStaffObj(k_imo_metronome_mark), m_markType(k_value)
        , m_ticksPerMinute(60), m_leftNoteType(0), m_leftDots(0)
        , m_rightNoteType(0), m_rightDots(0), m_fParenthesis(false)
    {}

public:
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

    //setters
    inline void set_left_note_type(int noteType) { m_leftNoteType = noteType; }
    inline void set_right_note_type(int noteType) { m_rightNoteType = noteType; }
    inline void set_left_dots(int dots) { m_leftDots = dots; }
    inline void set_right_dots(int dots) { m_rightDots = dots; }
    inline void set_ticks_per_minute(int ticks) { m_ticksPerMinute = ticks; }
    inline void set_mark_type(int type) { m_markType = type; }
    inline void set_parenthesis(bool fValue) { m_fParenthesis = fValue; }

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
protected:
    friend class ImFactory;
    ImoMusicData() : ImoCollection(k_imo_music_data) {}

public:
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

    friend class ImFactory;
    ImoOptionInfo()
        : ImoSimpleObj(k_imo_option), m_type(k_boolean), m_name("")
        , m_fValue(false) {}

public:
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
    inline void set_name(const string& name) { m_name = name; }
    inline void set_type(int type) { m_type = type; }
    inline void set_bool_value(bool value) { m_fValue = value; m_type = k_boolean; }
    inline void set_long_value(long value) { m_nValue = value; m_type = k_number_long; }
    inline void set_float_value(float value) { m_rValue = value; m_type = k_number_float; }
    inline void set_string_value(const string& value) { m_sValue = value; }

};

//---------------------------------------------------------------------------------------
class ImoOptions : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoOptions() : ImoCollection(k_imo_options) {}

public:
    ~ImoOptions() {}

};

//---------------------------------------------------------------------------------------
class ImoTextBlock : public ImoBoxContent, public InlineLevelCreatorApi
{
protected:
    ImoTextBlock(int objtype)
        : ImoBoxContent(objtype)
        , InlineLevelCreatorApi()
    {
        set_inline_level_creator_api_parent(this);
    }

public:
    virtual ~ImoTextBlock() {}

    //contents
    inline int get_num_items() { return get_num_children(); }
    inline void remove_item(ImoContentObj* pItem) { remove_child(pItem); }
    inline void add_item(ImoContentObj* pItem) { append_child(pItem); }
    inline ImoContentObj* get_first_item() {
        return dynamic_cast<ImoContentObj*>( get_first_child() );
    }

};

//---------------------------------------------------------------------------------------
class ImoParagraph : public ImoTextBlock
{
protected:
    friend class ImoBoxContainer;
    friend class Document;
    friend class ImFactory;
    ImoParagraph()
        : ImoTextBlock(k_imo_para) {}

public:
    virtual ~ImoParagraph() {}

    //required by Visitable parent class
	virtual void accept_visitor(BaseVisitor& v);
};

//---------------------------------------------------------------------------------------
class ImoParamInfo : public ImoSimpleObj
{
protected:
    string m_name;
    string m_value;

    friend class ImFactory;
    ImoParamInfo() : ImoSimpleObj(k_imo_param_info), m_name(), m_value() {}

public:
    ~ImoParamInfo() {}

    //getters
    inline string& get_name() { return m_name; }
    inline string& get_value() { return m_value; }
    bool get_value_as_int(int* pNumber);

    //setters
    inline void set_name(const string& name) { m_name = name; }
    inline void set_value(const string& value) { m_value = value; }

};

//---------------------------------------------------------------------------------------
class ImoHeading : public ImoTextBlock
{
protected:
    int m_level;

    friend class ImFactory;
    ImoHeading() : ImoTextBlock(k_imo_heading), m_level(1) {}

public:
    virtual ~ImoHeading() {};

    //level
    inline int get_level() { return m_level; }
    inline void set_level(int level) { m_level = level; }

    //required by Visitable parent class
	virtual void accept_visitor(BaseVisitor& v);

};

//---------------------------------------------------------------------------------------
class ImoReldataobjs : public ImoCollection
{
protected:
    friend class ImFactory;
    ImoReldataobjs() : ImoCollection(k_imo_reldataobjs) {}

public:
    ~ImoReldataobjs() {}

};

//---------------------------------------------------------------------------------------
class ImoPointDto : public ImoSimpleObj
{
    TPoint m_point;

public:
    ImoPointDto() : ImoSimpleObj(k_imo_point_dto) {}
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
    ImoSizeDto() : ImoSimpleObj(k_imo_size_dto) {}
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

    friend class ImFactory;
    ImoSpacer() : ImoStaffObj(k_imo_spacer), m_space(0.0f) {}

public:
    ~ImoSpacer() {}

    //getters
    inline Tenths get_width() { return m_space; }

    //setters
    inline void set_width(Tenths space) { m_space = space; }

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

    friend class ImFactory;
    friend class ImoScore;
    ImoSystemInfo();
    ImoSystemInfo(ImoSystemInfo& dto);

public:
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
class ImoScore : public ImoBoxLevelObj      //ImoBoxContainer
{
protected:
    string          m_version;
    Document*       m_pDoc;
    ColStaffObjs*   m_pColStaffObjs;
    SoundEventsTable* m_pMidiTable;
    ImoSystemInfo   m_systemInfoFirst;
    ImoSystemInfo   m_systemInfoOther;
    ImoPageInfo     m_pageInfo;
    std::list<ImoScoreTitle*> m_titles;
	std::map<std::string, ImoStyle*> m_nameToStyle;

	friend class ImFactory;
	ImoScore(Document* pDoc);
    void initialize();


public:
    ~ImoScore();

    //getters and setters
    inline std::string& get_version() { return m_version; }
    inline void set_version(const std::string& version) { m_version = version; }
    inline ColStaffObjs* get_staffobjs_table() { return m_pColStaffObjs; }
    void set_staffobjs_table(ColStaffObjs* pColStaffObjs);
    SoundEventsTable* get_midi_table();

    //instruments
    void add_instrument(ImoInstrument* pInstr);
    ImoInstrument* get_instrument(int iInstr);   //0..n-1
    int get_num_instruments();
    ImoInstruments* get_instruments();

    //instrument groups
    void add_instruments_group(ImoInstrGroup* pGroup);
    ImoInstrGroups* get_instrument_groups();

    //options
    ImoOptions* get_options();
    bool has_options();
    ImoOptionInfo* get_option(const std::string& name);
    void set_float_option(const std::string& name, float value);
    void set_bool_option(const std::string& name, bool value);
    void set_long_option(const std::string& name, long value);
    void add_option(ImoOptionInfo* pOpt);

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
    void add_style(ImoStyle* pStyle);
    void add_required_text_styles();
    ImoStyle* find_style(const std::string& name);
    ImoStyle* get_default_style();
    ImoStyle* get_style_or_default(const std::string& name);

    //API
    ImoInstrument* add_instrument();


protected:
    void delete_text_styles();
    ImoStyle* create_default_style();
    void set_defaults_for_system_info();
    void set_defaults_for_options();

};

//---------------------------------------------------------------------------------------
class ImoSlur : public ImoRelObj
{
protected:
    int m_slurNum;

	friend class ImFactory;
    ImoSlur() : ImoRelObj(k_imo_slur), m_slurNum(0) {}

public:
    ~ImoSlur();

    inline int get_slur_number() { return m_slurNum; }
    inline void set_slur_number(int num) { m_slurNum = num; }
    ImoNote* get_start_note();
    ImoNote* get_end_note();
};

//---------------------------------------------------------------------------------------
// Info about a slur point
class ImoSlurData : public ImoRelDataObj
{
protected:
    int m_slurType;
    int m_slurNum;
    ImoBezierInfo* m_pBezier;
    Color m_color;

	friend class ImFactory;
    ImoSlurData(ImoSlurDto* pDto);

public:
    ~ImoSlurData() {}

    //type of slur
    enum { k_start = 0, k_continue, k_stop };

    //getters
    inline bool is_stop() { return m_slurType == ImoSlurData::k_stop; }
    inline bool is_start() { return m_slurType == ImoSlurData::k_start; }
    inline bool is_continue() { return m_slurType == ImoSlurData::k_continue; }
    inline int get_slur_type() { return m_slurType; }
    inline int get_slur_number() { return m_slurNum; }
    inline ImoBezierInfo* get_bezier() { return m_pBezier; }
    inline Color get_color() { return m_color; }
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

    friend class ImFactory;
    friend class ImoInstrument;
    //Default values for staff. Line spacing: 1.8 mm (staff height = 7.2 mm),
    //line thickness: 0.15 millimeters, top margin: 10 millimeters
    ImoStaffInfo(int numStaff=0, int lines=5, int type=k_staff_regular,
                 LUnits spacing=180.0f, LUnits thickness=15.0f, LUnits margin=1000.0f)
        : ImoSimpleObj(k_imo_staff_info)
        , m_numStaff(numStaff)
        , m_nNumLines(lines)
        , m_staffType(type)
        , m_uSpacing(spacing)
        , m_uLineThickness(thickness)
        , m_uMarging(margin)
    {
    }

public:
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
class ImoStyles : public ImoCollection
{
protected:
	std::map<std::string, ImoStyle*> m_nameToStyle;
	Document* m_pDoc;

	friend class ImFactory;
    ImoStyles(Document* pDoc);

public:
    ~ImoStyles();

    //overrides, to traverse this special node
	void accept_visitor(BaseVisitor& v);
    bool has_visitable_children() { return m_nameToStyle.size() > 0; }

    //styles
    void add_style(ImoStyle* pStyle);
    //void add_required_text_styles();
    ImoStyle* find_style(const std::string& name);
    ImoStyle* get_default_style();
    ImoStyle* get_style_or_default(const std::string& name);

protected:
    void delete_text_styles();
    ImoStyle* create_default_styles();

};

//---------------------------------------------------------------------------------------
class ImoTextItem : public ImoInlineObj
{
private:
    ImoTextInfo m_text;

protected:
    friend class ImFactory;
    friend class TextItemAnalyser;

    ImoTextItem() : ImoInlineObj(k_imo_text_item), m_text() {}

public:
    virtual ~ImoTextItem() {}

    //getters
    inline string& get_text() { return m_text.get_text(); }
    inline ImoStyle* get_style() { return m_text.get_style(); }
    inline ImoTextInfo* get_text_info() { return &m_text; }

    //setters
    inline void set_style(ImoStyle* pStyle) { m_text.set_style(pStyle); }
    inline void set_text(const string& text) { m_text.set_text(text); }

};

//---------------------------------------------------------------------------------------
class ImoTieData : public ImoRelDataObj
{
protected:
    bool m_fStart;
    int m_tieNum;
    ImoBezierInfo* m_pBezier;

	friend class ImFactory;
    ImoTieData(ImoTieDto* pDto);

public:
    ~ImoTieData();

    //getters
    inline bool is_start() { return m_fStart; }
    inline int get_tie_number() { return m_tieNum; }
    inline ImoBezierInfo* get_bezier() { return m_pBezier; }
};

//---------------------------------------------------------------------------------------
class ImoTie : public ImoRelObj
{
protected:
    int     m_tieNum;
    Color   m_color;

	friend class ImFactory;
    ImoTie() : ImoRelObj(k_imo_tie), m_tieNum(0) {}
    ImoTie(int num) : ImoRelObj(k_imo_tie), m_tieNum(num) {}

public:
    ~ImoTie() {}

    //getters
    inline int get_tie_number() { return m_tieNum; }
    ImoNote* get_start_note();
    ImoNote* get_end_note();

    //setters
    inline void set_tie_number(int num) { m_tieNum = num; }
    inline void set_color(Color value) { m_color = value; }

    //access to data objects
    ImoBezierInfo* get_start_bezier();
    ImoBezierInfo* get_stop_bezier() ;

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
    ImoTieDto() : ImoSimpleObj(k_imo_tie_dto), m_fStart(true), m_tieNum(0), m_pNote(NULL)
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

    //required by RelationBuilder
    int get_item_number() { return get_tie_number(); }
    bool is_start_of_relation() { return is_start(); }
    bool is_end_of_relation() { return !is_start(); }
};

//---------------------------------------------------------------------------------------
class ImoTimeSignature : public ImoStaffObj
{
protected:
    int     m_beats;
    int     m_beatType;

    friend class ImFactory;
    ImoTimeSignature() : ImoStaffObj(k_imo_time_signature), m_beats(2), m_beatType(4) {}

public:
    ~ImoTimeSignature() {}

    //getters and setters
    inline int get_beats() { return m_beats; }
    inline void set_beats(int beats) { m_beats = beats; }
    inline int get_beat_type() { return m_beatType; }
    inline void set_beat_type(int beatType) { m_beatType = beatType; }

    //overrides: time signatures always in staff 0
    void set_staff(int staff) { m_staff = 0; }

    //other
    inline bool is_compound_meter() { return (m_beats==6 || m_beats==9 || m_beats==12); }
    int get_num_pulses();
    float get_beat_duration();
    float get_measure_duration();

};

//---------------------------------------------------------------------------------------
// raw info about a tuplet
class ImoTupletDto : public ImoSimpleObj
{
protected:
    int m_tupletType;
    int m_nActualNum;
    int m_nNormalNum;
    int m_nShowBracket;
    int m_nPlacement;
    int m_nShowNumber;
    LdpElement* m_pTupletElm;
    ImoNoteRest* m_pNR;

public:
    ImoTupletDto();
    ImoTupletDto(LdpElement* pBeamElm);
    ~ImoTupletDto() {}

    enum { k_unknown = 0, k_start, k_continue, k_stop, };

    //getters
    inline LdpElement* get_tuplet_element() { return m_pTupletElm; }
    inline ImoNoteRest* get_note_rest() { return m_pNR; }
    inline bool is_start_of_tuplet() { return m_tupletType == ImoTupletDto::k_start; }
    inline bool is_end_of_tuplet() { return m_tupletType == ImoTupletDto::k_stop;; }
    inline int get_actual_number() { return m_nActualNum; }
    inline int get_normal_number() { return m_nNormalNum; }
    inline int get_show_bracket() { return m_nShowBracket; }
    inline int get_show_number() { return m_nShowNumber; }
    inline int get_placement() { return m_nPlacement; }
    int get_line_number();

    //setters
    inline void set_note_rest(ImoNoteRest* pNR) { m_pNR = pNR; }
    inline void set_tuplet_element(LdpElement* pElm) { m_pTupletElm = pElm; }
    inline void set_tuplet_type(int value) { m_tupletType = value; }
    inline void set_actual_number(int value) { m_nActualNum = value; }
    inline void set_normal_number(int value) { m_nNormalNum = value; }
    inline void set_show_bracket(int value) { m_nShowBracket = value; }
    inline void set_show_number(int value) { m_nShowNumber = value; }
    inline void set_placement(int value) { m_nPlacement = value; }

    //required by RelationBuilder
    int get_item_number() { return 0; }
    bool is_start_of_relation() { return is_start_of_tuplet(); }
    bool is_end_of_relation() { return is_end_of_tuplet(); }
};

//---------------------------------------------------------------------------------------
// Tuplet info for a note/rest
class ImoTupletData : public ImoRelDataObj
{
protected:
	friend class ImFactory;
    ImoTupletData(ImoTupletDto* pDto);

public:
    ~ImoTupletData() {}
};

//---------------------------------------------------------------------------------------
class ImoTuplet : public ImoRelObj
{
protected:
    int m_nActualNum;
    int m_nNormalNum;
    int m_nShowBracket;
    int m_nShowNumber;
    int m_nPlacement;

	friend class ImFactory;
    ImoTuplet() : ImoRelObj(k_imo_tuplet) {}
    ImoTuplet(ImoTupletDto* dto);

public:
    ~ImoTuplet() {}

    enum { k_straight = 0, k_curved, k_slurred, };
    enum { k_number_actual=0, k_number_both, k_number_none, };

    //getters
    inline int get_actual_number() { return m_nActualNum; }
    inline int get_normal_number() { return m_nNormalNum; }
    inline int get_show_bracket() { return m_nShowBracket; }
    inline int get_show_number() { return m_nShowNumber; }
    inline int get_placement() { return m_nPlacement; }
};


        //************************************************************
        // DTO classes. Used only during model construction
        //************************************************************


//---------------------------------------------------------------------------------------
// Info about a slur point
class ImoSlurDto : public ImoSimpleObj
{
protected:
    int m_slurType;
    int m_slurNum;
    ImoNote* m_pNote;
    ImoBezierInfo* m_pBezier;
    LdpElement* m_pSlurElm;
    Color m_color;


public:
    ImoSlurDto()
        : ImoSimpleObj(k_imo_slur_dto)
        , m_slurType(ImoSlurData::k_start)
        , m_slurNum(0)
        , m_pNote(NULL)
        , m_pBezier(NULL)
        , m_pSlurElm(NULL)
    {
    }
    ~ImoSlurDto();

    //getters
    inline bool is_stop() { return m_slurType == ImoSlurData::k_stop; }
    inline bool is_start() { return m_slurType == ImoSlurData::k_start; }
    inline bool is_continue() { return m_slurType == ImoSlurData::k_continue; }
    inline int get_slur_type() { return m_slurType; }
    inline int get_slur_number() { return m_slurNum; }
    inline ImoNote* get_note() { return m_pNote; }
    inline ImoBezierInfo* get_bezier() { return m_pBezier; }
    inline LdpElement* get_slur_element() { return m_pSlurElm; }
    int get_line_number();
    inline Color get_color() { return m_color; }

    //setters
    inline void set_slur_type(int value) { m_slurType = value; }
    inline void set_slur_number(int num) { m_slurNum = num; }
    inline void set_note(ImoNote* pNote) { m_pNote = pNote; }
    inline void set_bezier(ImoBezierInfo* pBezier) { m_pBezier = pBezier; }
    inline void set_slur_element(LdpElement* pElm) { m_pSlurElm = pElm; }
    inline void set_color(Color value) { m_color = value; }

    //required by RelationBuilder
    int get_item_number() { return get_slur_number(); }
    bool is_start_of_relation() { return is_start(); }
    bool is_end_of_relation() { return is_stop(); }

};


//---------------------------------------------------------------------------------------
// A tree of ImoObj objects
typedef Tree<ImoObj>          ImoTree;
typedef TreeNode<ImoObj>      ImoNode;

//---------------------------------------------------------------------------------------
// Visitors for ImoObj objects
typedef Tree<ImoObj>          ImoTree;
//typedef Visitor<ImoAttachments> ImVisitor;
//typedef Visitor<ImoAuxObj> ImVisitor;
//typedef Visitor<ImoBeamData> ImVisitor;
//typedef Visitor<ImoBeamDto> ImVisitor;
//typedef Visitor<ImoBeam> ImVisitor;
//typedef Visitor<ImoBoxContainer> ImVisitor;
//typedef Visitor<ImoBoxContent> ImVisitor;
//typedef Visitor<ImoBoxInline> ImVisitor;
//typedef Visitor<ImoButton> ImVisitor;
//typedef Visitor<ImoChord> ImVisitor;
//typedef Visitor<ImoContentObj> ImVisitor;
//typedef Visitor<ImoDocument> ImVisitor;
//typedef Visitor<ImoDynamic> ImVisitor;
typedef Visitor<ImoHeading> ImHeadingVisitor;
//typedef Visitor<ImoInlineObj> ImVisitor;
//typedef Visitor<ImoInstrument> ImVisitor;
//typedef Visitor<ImoLineStyle> ImVisitor;
//typedef Visitor<ImoMusicData> ImVisitor;
//typedef Visitor<ImoNote> ImVisitor;
//typedef Visitor<ImoNoteRest> ImVisitor;
typedef Visitor<ImoObj> ImObjVisitor;
//typedef Visitor<ImoOptionInfo> ImVisitor;
typedef Visitor<ImoParagraph> ImParagraphVisitor;
//typedef Visitor<ImoParamInfo> ImVisitor;
//typedef Visitor<ImoReldataobjs> ImVisitor;
//typedef Visitor<ImoRelDataObj> ImVisitor;
//typedef Visitor<ImoRelObj> ImVisitor;
//typedef Visitor<ImoScoreText> ImVisitor;
//typedef Visitor<ImoSimpleObj> ImVisitor;
//typedef Visitor<ImoSlurDto> ImVisitor;
//typedef Visitor<ImoStaffInfo> ImVisitor;
//typedef Visitor<ImoStaffObj> ImVisitor;
//typedef Visitor<ImoStyles> ImVisitor;
//typedef Visitor<ImoTextBlock> ImVisitor;
//typedef Visitor<ImoTextInfo> ImVisitor;
//typedef Visitor<ImoTextItem> ImVisitor;
//typedef Visitor<ImoTextStyle> ImVisitor;
//typedef Visitor<ImoStyle> ImVisitor;
//typedef Visitor<ImoTieData> ImVisitor;
//typedef Visitor<ImoTieDto> ImVisitor;
//typedef Visitor<ImoTupletData> ImVisitor;
//typedef Visitor<ImoTupletDto> ImVisitor;
//typedef Visitor<ImoTuplet> ImVisitor;
//typedef Visitor<ImoWrapperBox> ImVisitor;


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

