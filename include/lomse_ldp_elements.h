//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#ifndef __LOMSE_LDP_ELEMENTS_H__        //to avoid nested includes
#define __LOMSE_LDP_ELEMENTS_H__

#include <vector>
#include <cassert>

#include "lomse_build_options.h"
#include "lomse_tree.h"
#include "lomse_visitor.h"
#include "lomse_basic.h"


namespace lomse
{

enum ELdpElement
{
    eElmFirst = 0,
    k_undefined = eElmFirst,

    //simple, generic elements
    k_label,
    k_number,
    k_string,

    //composite elements

    //container objs
    k_instrument,
    k_lenmusdoc,
    k_meta,
    k_parts,
    k_score,
    k_settings,

    //staffobjs
    k_barline,
    k_clef,
    k_direction,
    k_figuredBass,
    k_goBack,
    k_goFwd,
    k_key_signature,
    k_metronome,
    k_newSystem,
    k_na,           //note in chord
    k_note,         //n - note
    k_rest,         //r - rest
    k_spacer,
    k_time_signature,

    //auxobjs
    k_beam,
    k_dynamics_mark,
    k_text,
    k_textbox,
    k_line,
    k_lyric,
    k_tie,
    k_tuplet,
    //pauses and breath marks
    k_fermata,
    k_breath_mark,
    k_caesura,
    //accents
    k_accent,
    k_legato_duro,
    k_marccato,
    k_marccato_legato,
    k_marccato_staccato,
    k_marccato_staccatissimo,
    k_mezzo_staccato,
    k_mezzo_staccatissimo,
    k_staccato,
    k_staccato_duro,
    k_staccatissimo_duro,
    k_staccatissimo,
    k_tenuto,
    //stress articulations
    k_stress,
    k_unstress,
    //jazz pitch articulations
    k_scoop,
    k_plop,
    k_doit,
    k_falloff,

    //doc_only objs
    k_dynamic,
    k_heading,
    k_itemizedlist,
    k_link,
    k_listitem,
    k_orderedlist,
    k_para,
    k_score_player,
    k_styles,
    k_table,
    k_table_cell,
    k_table_column,
    k_table_body,
    k_table_head,
    k_table_row,
    k_txt,

    //properties
    k_abbrev,
    k_anchorLine,
    k_background_color,
    k_bezier,
    k_border,
    k_border_width,
    k_border_width_top,
    k_border_width_right,
    k_border_width_bottom,
    k_border_width_left,
    k_brace,
    k_bracket,
    k_bracketType,
    k_chord,
    k_classid,
    k_color,
    k_colspan,
    k_content,
    k_ctrol1_x,
    k_ctrol1_y,
    k_ctrol2_x,
    k_ctrol2_y,
    k_creationMode,
    k_cursor,
    k_defineStyle,
    k_displayBracket,
    k_displayNumber,
    k_duration,
    k_dx,
    k_dy,
    k_end,
    k_end_x,
    k_end_y,
    k_endPoint,
    k_fbline,
    k_file,
    k_font,
    k_font_file,
    k_font_name,
    k_font_size,
    k_font_style,
    k_font_weight,
    k_graphic,
    k_group,
    k_hasWidth,
    k_height,
    k_image,
    k_infoMIDI,
    k_instrIds,
    k_joinBarlines,
    k_landscape,
    k_language,
    k_left,
    k_lineCapEnd,
    k_lineCapStart,
    k_lineStyle,
    k_lineThickness,
    k_line_height,      //css
    k_margin,
    k_margin_top,
    k_margin_right,
    k_margin_bottom,
    k_margin_left,
    k_max_height,
    k_max_width,
    k_melisma,
    k_min_height,
    k_min_width,
    k_mm,
    k_musicData,
    k_name,
    k_opt,
    k_padding,
    k_padding_top,
    k_padding_right,
    k_padding_bottom,
    k_padding_left,
    k_pageLayout,
    k_pageMargins,
    k_pageSize,
    k_parameter,
    k_parenthesis,
    k_pitch,
    k_playLabel,
    k_rowspan,
    k_size,
    k_slur,
    k_split,
    k_staff,
    k_staffDistance,
    k_staffLines,
    k_staffNum,
    k_staffSpacing,
    k_staffType,
    k_start,
    k_start_x,
    k_start_y,
    k_startPoint,
    k_staves,
    k_stem,
    k_stopLabel,
    k_style,
    k_syllable,
    k_symbol,
    k_symbolSize,
    k_systemLayout,
    k_systemMargins,
    k_table_col_width,
    k_text_align,
    k_text_decoration,
    k_time_modification,
    k_title,
    k_url,
    k_value,
    k_vers,
    k_vertical_align,
    k_visible,
    k_voice,
    k_width,
    k_undoData,

    // values

    k_above,
    k_below,
    k_bold,
    k_bold_italic,
    k_center,
    k_down,
    k_font_style_italic,
    k_portrait,
    k_right,
    k_no,
    k_normal,
    k_up,
    k_yes,


    eElmLast,
};

//forward declarations
class LdpElement;
class ImoObj;
typedef std::shared_ptr<LdpElement>    SpLdpElement;

//---------------------------------------------------------------------------------------
// A generic LDP element representation.
//
// An LdpElement is the content of a node in the Ldp source tree. I combines
// links to other nodes as well as the actual element data.
// There are two types of elements:
//   - simple: it is just a type (label, string or number) and its value. They
//     are similar to LISP atoms.
//   - composite: they have a name and any number of parameters (zero
//     is allowed). They are like LISP lists.
//---------------------------------------------------------------------------------------

class LdpElement : public Visitable,
                                public TreeNode<LdpElement>
{
protected:
	ELdpElement m_type;     // the element type
	std::string	m_name;     // for composite: element name
	std::string m_value;    // for simple: the element value
    bool m_fSimple;         // true for simple elements
    int m_numLine;          // file line in whicht the elemnt starts or 0
    ImoId m_id;              // for composite: element ID (0..n)
    ImoObj* m_pImo;

    LdpElement();

public:
    virtual ~LdpElement();

    //overrides to Visitable class members
	virtual void accept_visitor(BaseVisitor& v);

    //getters and setters
	inline void set_value(const std::string& value) { m_value = value; }
    inline const std::string& get_value() { return m_value; }
    float get_value_as_float();
    inline void set_name(const std::string& name) { m_name = name; }
	inline const std::string& get_name() { return m_name; }
	inline ELdpElement get_type() { return m_type; }
    inline void set_num_line(int numLine) { m_numLine = numLine; }
    inline int get_line_number() { return m_numLine; }
    void set_imo(ImoObj* pImo);
    inline ImoObj* get_imo() { return m_pImo; }
    inline ImoId get_id() { return m_id; }
    inline void set_id(ImoId id) { m_id = id; }

	//! returns the element value as it is represented in source LDP
	std::string get_ldp_value();
	//! elements comparison
	bool operator ==(LdpElement& element);
	inline bool operator !=(LdpElement& element) { return !(*this == element); }

    std::string to_string();
    std::string to_string_with_ids();

    inline bool is_simple() { return m_fSimple; }
    inline void set_simple() { m_fSimple = true; }
	inline bool has_children() { return !is_terminal(); }
    int get_num_parameters();

    //helper methods
    inline bool is_type(ELdpElement type) { return get_type() == type; }

    //! random access to parameter i (1..n)
    LdpElement* get_parameter(int i);
};

//---------------------------------------------------------------------------------------
// A specific LDP element representation.
// For each ldp element we define a specific class. It is equivalent to defining
// specific lmLDPNodes for each ldp tag. In this way we have specific nodes
// LdpObject<type>.
//
template <ELdpElement type>
class LdpObject : public LdpElement
{
    protected:
        LdpObject() : LdpElement() { m_type = type; }

	public:
        //! static constructor to be used by Factory
		static LdpElement* new_ldp_object()
			{ LdpObject<type>* o = LOMSE_NEW LdpObject<type>; assert(o!=0); return o; }

        //! implementation of Visitable interface
        virtual void accept_visitor(BaseVisitor& v) {
			if (Visitor<LdpObject<type> >* p = dynamic_cast<Visitor<LdpObject<type> >*>(&v))
            {
				p->start_visit(this);
			}
			else LdpElement::accept_visitor(v);
		}

};

// A tree of LdpElements
typedef Tree<LdpElement>  LdpTree;
typedef std::shared_ptr<LdpTree> SpLdpTree;
typedef TreeNode<LdpElement> LdpNode;
typedef std::shared_ptr<LdpNode> SpLdpNode;


}   //namespace lomse

#endif    // __LOMSE_LDP_ELEMENTS_H__

