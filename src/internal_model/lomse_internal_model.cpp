//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

///@cond INTERNALS

#include "lomse_internal_model.h"

#include <algorithm>
#include <math.h>                   //pow
#include "lomse_staffobjs_table.h"
#include "lomse_im_note.h"
#include "lomse_midi_table.h"
#include "lomse_ldp_elements.h"
#include "lomse_im_factory.h"
#include "lomse_model_builder.h"
#include "lomse_control.h"
#include "lomse_score_player_ctrl.h"
#include "lomse_button_ctrl.h"
#include "lomse_logger.h"
#include "lomse_ldp_exporter.h"
#include "lomse_autobeamer.h"
#include "lomse_im_attributes.h"
#include "lomse_im_measures_table.h"
#include "lomse_score_utilities.h"
#include "lomse_relobj_cloner.h"


using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// utility function to convert typographical points to LUnits
LUnits pt_to_LUnits(float pt)
{
    // 1pt = 1/72" = 25.4/72 mm = 2540/72 LU = 35.2777777777 LU
    // Wikipedia (http://en.wikipedia.org/wiki/Typographic_unit)
    return pt * 35.277777777f;
}

//---------------------------------------------------------------------------------------
// static variables to convert from ImoObj type to name
static map<int, string> m_TypeToName;
static bool m_fNamesRegistered;
static string m_unknown = "unknown";

//---------------------------------------------------------------------------------------
//values for default style
#define k_default_font_size         12.0f
#define k_default_font_style        ImoStyle::k_font_style_normal
#define k_default_font_weight       ImoStyle::k_font_weight_normal
    //text
#define k_default_word_spacing      ImoStyle::k_spacing_normal
#define k_default_text_decoration   ImoStyle::k_decoration_none
#define k_default_vertical_align    ImoStyle::k_valign_baseline
#define k_default_text_align        ImoStyle::k_align_left
#define k_default_text_indent_length    0.0f
#define k_default_word_spacing_length   0.0f   //not applicable
#define k_default_line_height           1.5f
    //color and background
#define k_default_color             Color(0,0,0)
#define k_default_background_color  Color(255,255,255)
    //margin
#define k_default_margin_top        0.0f
#define k_default_margin_bottom     0.0f
#define k_default_margin_left       0.0f
#define k_default_margin_right      0.0f
    //padding
#define k_default_padding_top       0.0f
#define k_default_padding_bottom    0.0f
#define k_default_padding_left      0.0f
#define k_default_padding_right     0.0f
    //border
//    //&& set_lunits_property(ImoStyle::k_border_top, ImoStyle::k_default_border_top0.0f);
//    //&& set_lunits_property(ImoStyle::k_border_bottom, ImoStyle::k_default_border_bottom0.0f);
//    //&& set_lunits_property(ImoStyle::k_border_left, ImoStyle::k_default_border_left0.0f);
//    //&& set_lunits_property(ImoStyle::k_border_right, ImoStyle::k_default_border_right0.0f);
    //border width
#define k_default_border_width_top      0.0f
#define k_default_border_width_bottom   0.0f
#define k_default_border_width_left     0.0f
#define k_default_border_width_right    0.0f
    //size
#define k_default_min_height    0.0f
#define k_default_max_height    0.0f
#define k_default_height        0.0f
#define k_default_min_width     0.0f
#define k_default_max_width     0.0f
#define k_default_width         0.0f

//=======================================================================================
// AttrObj implementation
//=======================================================================================
const string AttrObj::get_name() const
{
    return get_name(m_attrbIdx);
}

//---------------------------------------------------------------------------------------
const string AttrObj::get_name(int idx)
{
    AttributesData data = AttributesTable::get_data_for(idx);
    return data.label;
}

//---------------------------------------------------------------------------------------
AttrObj* AttrObj::replace_by(AttrObj* newAttr)
{
    newAttr->set_next_attrib(m_next);
    newAttr->set_prev_attrib(m_prev);
    if (m_prev)
        m_prev->set_next_attrib(newAttr);

    m_next = nullptr;
    delete this;

    return newAttr;
}

//---------------------------------------------------------------------------------------
int AttrObj::get_int_value() const
{
    const Attr<int>* p1 = static_cast< Attr<int> const * >(this);
    Attr<int>* p2 = const_cast< Attr<int>* >(p1);
    return p2->get_value();
//    return static_cast< Attr<int>* >(this)->get_value();
}

//---------------------------------------------------------------------------------------
double AttrObj::get_double_value() const
{
    const Attr<double>* p1 = static_cast< Attr<double> const * >(this);
    Attr<double>* p2 = const_cast< Attr<double>* >(p1);
    return p2->get_value();
//    return static_cast< Attr<double>* >(this)->get_value();
}

//---------------------------------------------------------------------------------------
std::string AttrObj::get_string_value() const
{
    const Attr<string>* p1 = static_cast< Attr<string> const * >(this);
    Attr<string>* p2 = const_cast< Attr<string>* >(p1);
    return p2->get_value();
//    return static_cast< Attr<string>* >(this)->get_value();
}

//---------------------------------------------------------------------------------------
bool AttrObj::get_bool_value() const
{
    const Attr<bool>* p1 = static_cast< Attr<bool> const * >(this);
    Attr<bool>* p2 = const_cast< Attr<bool>* >(p1);
    return p2->get_value();
//    return static_cast< Attr<bool>* >(this)->get_value();
}

//---------------------------------------------------------------------------------------
float AttrObj::get_float_value() const
{
    const Attr<float>* p1 = static_cast< Attr<float> const * >(this);
    Attr<float>* p2 = const_cast< Attr<float>* >(p1);
    return p2->get_value();
//    return static_cast< Attr<float>* >(this)->get_value();
}

//---------------------------------------------------------------------------------------
Color AttrObj::get_color_value() const
{
    const Attr< Color >* p1 = static_cast< Attr< Color > const * >(this);
    Attr< Color >* p2 = const_cast< Attr< Color >* >(p1);
    return p2->get_value();
//    return static_cast< const Attr< Color >* >(this)->get_value();
}


//=======================================================================================
// AttrVariant implementation
//=======================================================================================
AttrVariant::AttrVariant(int idx, const string& value)
    : AttrObj(idx)
{
    set_string_value(value);
}

//---------------------------------------------------------------------------------------
AttrVariant::AttrVariant(int idx, int value)
    : AttrObj(idx)
{
    set_int_value(value);
}

//---------------------------------------------------------------------------------------
AttrVariant::AttrVariant(int idx, double value)
    : AttrObj(idx)
{
    set_double_value(value);
}

//---------------------------------------------------------------------------------------
AttrVariant::AttrVariant(int idx, bool value)
    : AttrObj(idx)
{
    set_bool_value(value);
}

//---------------------------------------------------------------------------------------
AttrVariant::AttrVariant(int idx, float value)
    : AttrObj(idx)
{
    set_float_value(value);
}

//---------------------------------------------------------------------------------------
AttrVariant::AttrVariant(int idx, Color value)
    : AttrObj(idx)
{
    set_color_value(value);
}


//=======================================================================================
// AttrList implementation
//=======================================================================================
AttrList& AttrList::clone(const AttrList& a)
{
    if (a.m_first)
        m_first = a.m_first->clone();
    else
        m_first = nullptr;

    return *this;
}

//---------------------------------------------------------------------------------------
size_t AttrList::size()
{
    AttrObj* pAttr = m_first;
    size_t i = 0;
    while (pAttr)
    {
        ++i;
        pAttr = pAttr->get_next_attrib();
    }

    return i;
}

//---------------------------------------------------------------------------------------
void AttrList::clear()
{
    delete m_first;
    m_first = nullptr;
}

//---------------------------------------------------------------------------------------
AttrObj* AttrList::back()
{
    AttrObj* pAttr = m_first;
    AttrObj* pLast = pAttr;
    while (pAttr)
    {
        pLast = pAttr;
        pAttr = pAttr->get_next_attrib();
    }

    return pLast;
}

//---------------------------------------------------------------------------------------
AttrObj* AttrList::push_back(AttrObj* newAttr)
{
    newAttr->set_next_attrib(nullptr);
    AttrObj* pLast = back();
    if (pLast)
    {
        pLast->set_next_attrib(newAttr);
        newAttr->set_prev_attrib(pLast);
    }
    else
        m_first = newAttr;

    return newAttr;
}

//---------------------------------------------------------------------------------------
void AttrList::remove(TIntAttribute idx)
{
    AttrObj* pAttr = find(idx);
    if (pAttr)
    {
        AttrObj* pPrev = pAttr->get_prev_attrib();
        if (pPrev)
        {
            AttrObj* pNext = pAttr->get_next_attrib();
            pPrev->set_next_attrib(pNext);
            if (pNext)
                pNext->set_prev_attrib(pPrev);
        }

        if (pAttr == m_first)
            m_first = nullptr;

        pAttr->set_next_attrib(nullptr);
        delete pAttr;
    }
}

//---------------------------------------------------------------------------------------
AttrObj* AttrList::find(TIntAttribute idx)
{
    AttrObj* pAttr = m_first;
    while (pAttr && pAttr->get_attrib_idx() != idx)
        pAttr = pAttr->get_next_attrib();

    return pAttr;
}


//=======================================================================================
// AttribValue implementation
//=======================================================================================
AttribValue::operator int() const
{
    check_type(vt_int);
    return intValue;
}

//---------------------------------------------------------------------------------------
AttribValue::operator float() const
{
    check_type(vt_float);
    return floatValue;
}

//---------------------------------------------------------------------------------------
AttribValue::operator double() const
{
    check_type(vt_double);
    return doubleValue;
}

//---------------------------------------------------------------------------------------
AttribValue::operator bool() const
{
    check_type(vt_bool);
    return boolValue;
}
//---------------------------------------------------------------------------------------
AttribValue::operator string() const
{
    check_type(vt_string);
    return stringValue;
}

//---------------------------------------------------------------------------------------
AttribValue::operator Color() const
{
    check_type(vt_color);
    return colorValue;
}

//---------------------------------------------------------------------------------------
void AttribValue::operator=(int value)
{
    cleanup();
    intValue = value;
    m_type = vt_int;
}

//---------------------------------------------------------------------------------------
void AttribValue::operator=(float value)
{
    cleanup();
    floatValue = value;
    m_type = vt_float;
}

//---------------------------------------------------------------------------------------
void AttribValue::operator=(double value)
{
    cleanup();
    doubleValue = value;
    m_type = vt_double;
}

//---------------------------------------------------------------------------------------
void AttribValue::operator=(bool value)
{
    cleanup();
    boolValue = value;
    m_type = vt_bool;
}

//---------------------------------------------------------------------------------------
void AttribValue::operator=(const std::string& value)
{
    cleanup();
    // placement new (http://en.cppreference.com/w/cpp/language/union)
    new (&stringValue) string(value);
    m_type = vt_string;
}

//---------------------------------------------------------------------------------------
void AttribValue::operator=(const Color& value)
{
    cleanup();
    // placement new (http://en.cppreference.com/w/cpp/language/union)
    new (&colorValue) Color(value);
    m_type = vt_color;
}

//---------------------------------------------------------------------------------------
void AttribValue::cleanup()
{
    if (m_type == vt_string)
    {
        // explicit destructor call (http://en.cppreference.com/w/cpp/language/union)
        stringValue.~string();
    }
    else if (m_type == vt_color)
    {
        // explicit destructor call (http://en.cppreference.com/w/cpp/language/union)
        colorValue.~Color();
    }
    m_type = vt_empty;
}

//---------------------------------------------------------------------------------------
void AttribValue::check_type(AttribType type) const
{
    if (type != m_type)
    {
        std::stringstream ss;
        ss << "[AttribValue::operator(<type>)]. Type mismatch. Expected type=" <<
            type << ", real type=" << m_type;
        LOMSE_LOG_ERROR(ss.str());
        throw std::runtime_error(ss.str());
    }
}

//---------------------------------------------------------------------------------------
AttribValue& AttribValue::clone(const AttribValue& a)
{
    switch (a.m_type)
    {
        case vt_int:        intValue = a.intValue;          break;
        case vt_string:     stringValue = a.stringValue;    break;
        case vt_bool:       boolValue = a.boolValue;        break;
        case vt_float:      floatValue = a.floatValue;      break;
        case vt_double:     doubleValue = a.doubleValue;    break;
        case vt_color:      colorValue = a.colorValue;      break;
        default:
        {
            string msg("[AttribValue::clone]. Invalid type in source object");
            LOMSE_LOG_ERROR(msg);
            throw std::runtime_error(msg);
        }
    }
    return *this;
}

//=======================================================================================
// implementation of standard interface for ImoObj objects containing an ImoSounds child
//=======================================================================================
#define LOMSE_IMPLEMENT_IMOSOUNDS_INTERFACE(xxxxx)                                       \
ImoSounds* xxxxx::get_sounds()                                                           \
{                                                                                        \
    return dynamic_cast<ImoSounds*>( get_child_of_type(k_imo_sounds) );                  \
}                                                                                        \
                                                                                         \
void xxxxx::add_sound_info(ImoSoundInfo* pInfo)                                          \
{                                                                                        \
    ImoSounds* pColSounds = get_sounds();                                                \
    if (!pColSounds)                                                                     \
    {                                                                                    \
        pColSounds = static_cast<ImoSounds*>( ImFactory::inject(k_imo_sounds, m_pDocModel) );   \
        append_child_imo(pColSounds);                                                    \
    }                                                                                    \
    pColSounds->add_sound_info(pInfo);                                                   \
}                                                                                        \
                                                                                         \
int xxxxx::get_num_sounds()                                                              \
{                                                                                        \
    ImoSounds* pColSounds = get_sounds();                                                \
    if (pColSounds)                                                                      \
        return pColSounds->get_num_sounds();                                             \
    else                                                                                 \
        return 0;                                                                        \
}                                                                                        \
                                                                                         \
ImoSoundInfo* xxxxx::get_sound_info(const string& soundId)                               \
{                                                                                        \
    ImoSounds* pColSounds = get_sounds();                                                \
    if (pColSounds)                                                                      \
        return pColSounds->get_sound_info(soundId);                                      \
    else                                                                                 \
        return nullptr;                                                                  \
}                                                                                        \
                                                                                         \
ImoSoundInfo* xxxxx::get_sound_info(int iSound)                                          \
{                                                                                        \
    ImoSounds* pColSounds = get_sounds();                                                \
    if (pColSounds)                                                                      \
        return pColSounds->get_sound_info(iSound);                                       \
    else                                                                                 \
        return nullptr;                                                                  \
}


//=======================================================================================
// implementation of standard interface for ImoObj objects having inline-level content
//=======================================================================================
#define LOMSE_IMPLEMENT_INLINE_LEVEL_INTERFACE(xxxxx)                                    \
                                                                                         \
ImoTextItem* xxxxx::add_text_item(const string& text, ImoStyle* pStyle)                  \
{                                                                                        \
    ImoTextItem* pImo = static_cast<ImoTextItem*>(                                       \
                            ImFactory::inject(k_imo_text_item, m_pDocModel) );                \
    pImo->set_text(text);                                                                \
    pImo->set_style(pStyle);                                                             \
    pImo->set_language( m_pDocModel->get_language() );                                        \
    append_child_imo(pImo);                                                              \
    return pImo;                                                                         \
}                                                                                        \
                                                                                         \
ButtonCtrl* xxxxx::add_button(LibraryScope& libScope, const string& label,               \
                                             const USize& size, ImoStyle* pStyle)        \
{                                                                                        \
    ImoButton* pImo = static_cast<ImoButton*>(ImFactory::inject(k_imo_button, m_pDocModel));  \
    ImoDocument* pImoDoc = m_pDocModel->get_im_root();                                        \
    pImo->set_label(label);                                                              \
    pImo->set_language( pImoDoc->get_language() );                                       \
    pImo->set_size(size);                                                                \
    pImo->set_style(pStyle);                                                             \
                                                                                         \
    ButtonCtrl* pCtrol = LOMSE_NEW ButtonCtrl(libScope, nullptr, m_pDocModel->get_owner_document(), label,          \
                                              size.width, size.height, pStyle);          \
                                                                                         \
    pImo->attach_control(pCtrol);                                                        \
                                                                                         \
    append_child_imo(pImo);                                                              \
                                                                                         \
    return pCtrol;                                                                       \
}                                                                                        \
                                                                                         \
ImoInlineWrapper* xxxxx::add_inline_box(LUnits width, ImoStyle* pStyle)                  \
{                                                                                        \
    ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(                             \
                                ImFactory::inject(k_imo_inline_wrapper, m_pDocModel) );       \
    pImo->set_width(width);                                                              \
    pImo->set_style(pStyle);                                                             \
    append_child_imo(pImo);                                                              \
    return pImo;                                                                         \
}                                                                                        \
                                                                                         \
ImoLink* xxxxx::add_link(const string& url, ImoStyle* pStyle)                            \
{                                                                                        \
    ImoLink* pImo = static_cast<ImoLink*>( ImFactory::inject(k_imo_link, m_pDocModel) );      \
    pImo->set_url(url);                                                                  \
    pImo->set_style(pStyle);                                                             \
    append_child_imo(pImo);                                                              \
    return pImo;                                                                         \
}                                                                                        \
                                                                                         \
ImoImage* xxxxx::add_image(unsigned char* imgbuf, VSize bmpSize,                         \
                                           EPixelFormat format, USize imgSize,           \
                                           ImoStyle* pStyle)                             \
{                                                                                        \
    ImoImage* pImo = ImFactory::inject_image(m_pDocModel, imgbuf, bmpSize, format, imgSize);  \
    pImo->set_style(pStyle);                                                             \
    append_child_imo(pImo);                                                              \
    return pImo;                                                                         \
}                                                                                        \
                                                                                         \
ImoControl* xxxxx::add_control(Control* pCtrol)                                          \
{                                                                                        \
    ImoControl* pImo = ImFactory::inject_control(m_pDocModel);                           \
    pImo->attach_control(pCtrol);                                                        \
    append_child_imo(pImo);                                                              \
    return pImo;                                                                         \
}



//=======================================================================================
// ImoObj implementation
//=======================================================================================
ImoObj::ImoObj(int objtype, ImoId id)
    : m_pDocModel(nullptr)
    , m_id(id)
    , m_objtype(objtype)
    , m_flags(k_dirty)
{
}

//---------------------------------------------------------------------------------------
ImoObj::~ImoObj()
{
    TreeNode<ImoObj>::children_iterator it(this);
    it = begin();
    while (it != end())
    {
        ImoObj* child = *it;
        ++it;
	    delete child;
    }

    remove_id();
}

//---------------------------------------------------------------------------------------
ImoObj& ImoObj::clone(const ImoObj& a)
{
    m_pDocModel = a.m_pDocModel;
    m_id = a.m_id;
    m_objtype = a.m_objtype;
    m_flags = a.m_flags;
    m_attribs = a.m_attribs;

    //clone children
    ImoObj* node = a.m_firstChild;
    while (node)
    {
        ImoObj* newImo = ImFactory::clone(node);
        if (newImo)
            append_child(newImo);

        node = node->get_next_sibling();
    }

    return *this;
}

//---------------------------------------------------------------------------------------
void ImoObj::set_owner_model(DocModel* pDocModel)
{
    m_pDocModel = pDocModel;
}

//---------------------------------------------------------------------------------------
void ImoObj::remove_id()
{
    if (get_id() != k_no_imoid && m_pDocModel)
        m_pDocModel->on_removed_from_model(this);
}

//---------------------------------------------------------------------------------------
std::string ImoObj::get_xml_id()
{
    return (m_pDocModel ? m_pDocModel->get_xml_id_for(m_id) : "");
}

//---------------------------------------------------------------------------------------
void ImoObj::set_xml_id(const std::string& value)
{
    if (m_pDocModel)
        m_pDocModel->set_xml_id_for(m_id, value);
}

//---------------------------------------------------------------------------------------
const string& ImoObj::get_name(int type)
{
    //Register all IM objects
    if (!m_fNamesRegistered)
    {
        // ImoStaffObj (A)
        m_TypeToName[k_imo_barline] = "barline";
        m_TypeToName[k_imo_clef] = "clef";
        m_TypeToName[k_imo_direction] = "direction";
        m_TypeToName[k_imo_figured_bass] = "figured-bass";
        m_TypeToName[k_imo_go_back_fwd] = "go-back-fwd";
        m_TypeToName[k_imo_key_signature] = "key-signature";
        m_TypeToName[k_imo_note_regular] = "note";
        m_TypeToName[k_imo_note_grace] = "grace-note";
        m_TypeToName[k_imo_note_cue] = "cue-note";
        m_TypeToName[k_imo_rest] = "rest";
        m_TypeToName[k_imo_system_break] = "system-break";
        m_TypeToName[k_imo_time_signature] = "time-signature";

        // ImoBlocksContainer (A)
        m_TypeToName[k_imo_content] = "content";
        m_TypeToName[k_imo_dynamic] = "dynamic";
        m_TypeToName[k_imo_document] = "lenmusdoc";
        m_TypeToName[k_imo_list] = "list";
        m_TypeToName[k_imo_listitem] = "listitem";
        m_TypeToName[k_imo_multicolumn] = "multicolumn";
        m_TypeToName[k_imo_table] = "table";
        m_TypeToName[k_imo_table_cell] = "table-cell";
        m_TypeToName[k_imo_table_row] = "table-row";
        m_TypeToName[k_imo_score] = "score";

        // ImoInlinesContainer (A)
        m_TypeToName[k_imo_anonymous_block] = "anonymous-block";
        m_TypeToName[k_imo_heading] = "heading";
        m_TypeToName[k_imo_para] = "paragraph";

        // ImoInlineLevelObj
        m_TypeToName[k_imo_button] = "buttom";
        m_TypeToName[k_imo_control] = "control";
        m_TypeToName[k_imo_image] = "image";
        m_TypeToName[k_imo_score_player] = "score-player";
        m_TypeToName[k_imo_text_item] = "text";

        // ImoBoxInline (A)
        m_TypeToName[k_imo_link] = "link";
        m_TypeToName[k_imo_inline_wrapper] = "wrapper";

        // ImoDto, ImoSimpleObj (A)
        m_TypeToName[k_imo_arpeggio_dto] = "arpeggio";
        m_TypeToName[k_imo_beam_dto] = "beam";
        m_TypeToName[k_imo_bezier_info] = "bezier";
        m_TypeToName[k_imo_border_dto] = "border";
        m_TypeToName[k_imo_color_dto] = "color";
        m_TypeToName[k_imo_cursor_info] = "cursor";
        m_TypeToName[k_imo_figured_bass_info] = "figured-bass";
        m_TypeToName[k_imo_font_style_dto] = "font-style";
        m_TypeToName[k_imo_instr_group] = "instr-group";
        m_TypeToName[k_imo_line_style_dto] = "line-style-dto";
        m_TypeToName[k_imo_lyrics_text_info] = "lyric-text";
        m_TypeToName[k_imo_midi_info] = "midi-info";
        m_TypeToName[k_imo_octave_shift] = "octave-shift";
        m_TypeToName[k_imo_octave_shift_dto] = "octave-shift-dto";
        m_TypeToName[k_imo_pedal_line] = "pedal-line";
        m_TypeToName[k_imo_pedal_line_dto] = "pedal-line-dto";
        m_TypeToName[k_imo_option] = "opt";
        m_TypeToName[k_imo_page_info] = "page-info";
        m_TypeToName[k_imo_param_info] = "param";
        m_TypeToName[k_imo_point_dto] = "point";
        m_TypeToName[k_imo_size_dto] = "size";
        m_TypeToName[k_imo_slur_dto] = "slur-dto";
        m_TypeToName[k_imo_sound_change] = "sound-change";
        m_TypeToName[k_imo_sound_info] = "sound-info";
        m_TypeToName[k_imo_staff_info] = "staff-info";
        m_TypeToName[k_imo_style] = "style";
        m_TypeToName[k_imo_system_info] = "system-info";
        m_TypeToName[k_imo_textblock_info] = "textblock";
        m_TypeToName[k_imo_text_style] = "text-style";
        m_TypeToName[k_imo_tie_dto] = "tie-dto";
        m_TypeToName[k_imo_time_modification_dto] = "time-modificator-dto";
        m_TypeToName[k_imo_transpose] = "transpose";
        m_TypeToName[k_imo_tuplet_dto] = "tuplet-dto";
        m_TypeToName[k_imo_volta_bracket_dto] = "volta_bracket_dto";
        m_TypeToName[k_imo_wedge_dto] = "wedge_dto";

        // ImoRelDataObj (A)
        m_TypeToName[k_imo_beam_data] = "beam-data";
        m_TypeToName[k_imo_slur_data] = "slur-data";
        m_TypeToName[k_imo_tie_data] = "tie-data";
//
        //ImoCollection(A)
        m_TypeToName[k_imo_instruments] = "instruments";
        m_TypeToName[k_imo_instrument_groups] = "instr-groups";
        m_TypeToName[k_imo_music_data] = "musicData";
        m_TypeToName[k_imo_options] = "options";
        m_TypeToName[k_imo_styles] = "styles";
        m_TypeToName[k_imo_score_titles] = "score-titles";
        m_TypeToName[k_imo_sounds] = "sounds";
        m_TypeToName[k_imo_parameters] = "parameters";
        m_TypeToName[k_imo_table_head] = "table-head";
        m_TypeToName[k_imo_table_body] = "table-body";

        // Special collections
        m_TypeToName[k_imo_attachments] = "attachments";
        m_TypeToName[k_imo_relations] = "relations";

        // ImoContainerObj (A)
        m_TypeToName[k_imo_instrument] = "instrument";

        // ImoAuxObj (A)
        m_TypeToName[k_imo_articulation_line] = "articulation-line";
        m_TypeToName[k_imo_articulation_symbol] = "articulation-symbol";
        m_TypeToName[k_imo_dynamics_mark] = "dynamics-mark";
        m_TypeToName[k_imo_fermata] = "fermata";
        m_TypeToName[k_imo_fingering] = "fingering";
        m_TypeToName[k_imo_fret_string] = "fret-string";
        m_TypeToName[k_imo_line] = "line";
        m_TypeToName[k_imo_metronome_mark] = "metronome-mark";
        m_TypeToName[k_imo_ornament] = "ornament";
        m_TypeToName[k_imo_pedal_mark] = "pedal-mark";
        m_TypeToName[k_imo_score_text] = "score-text";
        m_TypeToName[k_imo_score_line] = "score-line";
        m_TypeToName[k_imo_score_title] = "title";
        m_TypeToName[k_imo_symbol_repetition_mark] = "symbol-repetition-mark";
        m_TypeToName[k_imo_technical] = "technical";
        m_TypeToName[k_imo_text_box] = "text-box";
        m_TypeToName[k_imo_text_repetition_mark] = "text-repetition-mark";

        // ImoAuxRelObj (A)
        m_TypeToName[k_imo_lyric] = "lyric";

        // ImoRelObj (A)
        m_TypeToName[k_imo_arpeggio] = "arpeggio";
        m_TypeToName[k_imo_beam] = "beam";
        m_TypeToName[k_imo_chord] = "chord";
        m_TypeToName[k_imo_grace_relobj] = "grace-relobj";
        m_TypeToName[k_imo_octave_shift] = "octave-shift";
        m_TypeToName[k_imo_slur] = "slur";
        m_TypeToName[k_imo_tie] = "tie";
        m_TypeToName[k_imo_tuplet] = "tuplet";
        m_TypeToName[k_imo_volta_bracket] = "volta-bracket";
        m_TypeToName[k_imo_wedge] = "wedge";

        //abstract and non-valid objects
        m_TypeToName[k_imo_obj] = "non-valid";
        m_TypeToName[k_imo_dto] = "non-valid";
        m_TypeToName[k_imo_dto_last] = "non-valid";
        m_TypeToName[k_imo_simpleobj] = "non-valid";
        m_TypeToName[k_imo_simpleobj_last] = "non-valid";
        m_TypeToName[k_imo_reldataobj] = "non-valid";
        m_TypeToName[k_imo_reldataobj_last] = "non-valid";
        m_TypeToName[k_imo_collection] = "non-valid";
        m_TypeToName[k_imo_collection_last] = "non-valid";
        m_TypeToName[k_imo_containerobj] = "non-valid";
        m_TypeToName[k_imo_containerobj_last] = "non-valid";
        m_TypeToName[k_imo_contentobj] = "non-valid";
        m_TypeToName[k_imo_scoreobj] = "non-valid";
        m_TypeToName[k_imo_staffobj] = "non-valid";
        m_TypeToName[k_imo_staffobj_last] = "non-valid";
        m_TypeToName[k_imo_auxobj] = "non-valid";
        m_TypeToName[k_imo_auxrelobj] = "non-valid";
        m_TypeToName[k_imo_auxobj_last] = "non-valid";
        m_TypeToName[k_imo_relobj] = "non-valid";
        m_TypeToName[k_imo_relobj_last] = "non-valid";
        m_TypeToName[k_imo_scoreobj_last] = "non-valid";
        m_TypeToName[k_imo_block_level_obj] = "non-valid";
        m_TypeToName[k_imo_blocks_container] = "non-valid";
        m_TypeToName[k_imo_blocks_container_last] = "non-valid";
        m_TypeToName[k_imo_inlines_container] = "non-valid";
        m_TypeToName[k_imo_inlines_container_last] = "non-valid";
        m_TypeToName[k_imo_block_level_obj_last] = "non-valid";
        m_TypeToName[k_imo_inline_level_obj] = "non-valid";
        m_TypeToName[k_imo_control_end] = "non-valid";
        m_TypeToName[k_imo_box_inline] = "non-valid";
        m_TypeToName[k_imo_box_inline_last] = "non-valid";
        m_TypeToName[k_imo_inline_level_obj_last] = "non-valid";
        m_TypeToName[k_imo_contentobj_last] = "non-valid";
        m_TypeToName[k_imo_articulation] = "non-valid";
        m_TypeToName[k_imo_articulation_last] = "non-valid";
        m_TypeToName[k_imo_last] = "non-valid";

        m_fNamesRegistered = true;
    }

	map<int, std::string>::const_iterator it = m_TypeToName.find( type );
	if (it != m_TypeToName.end())
		return it->second;
    else
    {
        LOMSE_LOG_ERROR("Invalid type.");
        return m_unknown;
        //throw runtime_error( "[ImoObj::get_name]. Invalid type" );
    }
}

//---------------------------------------------------------------------------------------
bool ImoObj::is_gap()
{
    return m_objtype == k_imo_rest ? (static_cast<ImoRest*>(this))->is_go_fwd()
                                   : false;
}

//---------------------------------------------------------------------------------------
const string& ImoObj::get_name() const
{
	return ImoObj::get_name( m_objtype );
}

//---------------------------------------------------------------------------------------
Document* ImoObj::get_the_document()
{
    return (m_pDocModel ? m_pDocModel->get_owner_document() : nullptr);
}

//---------------------------------------------------------------------------------------
ImoDocument* ImoObj::get_document()
{
    return (m_pDocModel ? m_pDocModel->get_im_root() : nullptr);
}

//---------------------------------------------------------------------------------------
DocModel* ImoObj::get_doc_model()
{
    return m_pDocModel;
}

//---------------------------------------------------------------------------------------
Observable* ImoObj::get_observable_parent()
{
    if (this->is_document())
    {
        Document* pDoc = static_cast<ImoDocument*>(this)->get_the_document();
        return static_cast<Observable*>( pDoc );
    }
    else
        return static_cast<Observable*>( get_block_level_parent() );
}

//---------------------------------------------------------------------------------------
ImoContentObj* ImoObj::get_block_level_parent()
{
    //Returns the ImoBlockLevelObj parent (the editable block in a Document) of this
    //ImoObj. Should be:
    //- ImoScore,
    //- ImoBlocksContainer (ImoList, ImoTable, ImoMultiColumn or ImoDocument),
    //- or ImoInlinesContainer (ImoAnonymousBlock, ImoHeading or ImoParagraph).

    //TODO: Review this method code. It does not match method name and this
    //method is only used for supporting other methods related to Document edition.
    //I will postpone this review until having time to continue developping the editor
    //or until an issue forces to do it.
    //Currently this method does not return the ImoBlockLevelObj parent of all
    //ImoObj (e.g. not for ImoStyle) but only of some objects (the renderizable ones?)
    //and it is not clear what are the real needs and use cases.

    if (this->is_staffobj())            //parent is ImoMusicData (ImoSimpleObj)
        return static_cast<ImoStaffObj*>(this)->get_score();

//    else if (this->is_music_data())     //parent is ImoInstrument (ImoSimpleObj)
//        return static_cast<ImoInstrument*>(this)->get_score();

    else if (this->is_instrument())     //ImoInstrument is ImoSimpleObj
        return static_cast<ImoInstrument*>(this)->get_score();

//    else if (this->is_table_section())     //ImoTableSection is ImoSimpleObj
//        return static_cast<ImoTableSection*>(this)->get_table();

    else if (this->is_relobj())     //no single parent! use first of relation
        return static_cast<ImoRelObj*>(this)->get_start_object();
        //TODO: Should be as follows?
        //return static_cast<ImoRelObj*>(this)->get_start_object()->get_block_level_parent();

    else
    {
        //get its parent and see
        ImoObj* pImo = get_parent();
        if (pImo)
        {
            if (pImo->is_simpleobj()) //ImoCollection, ImoAttachments, ImoRelations, ...
            {
                pImo = pImo->get_parent();
            }
            else if (pImo->is_auxobj())  //normally child of ImoAttachments
            {
                pImo = pImo->get_parent();
                while(pImo && pImo->is_auxobj())
                    pImo = pImo->get_parent();
                if (pImo && pImo->is_attachments())
                    pImo = pImo->get_parent();
            }

            if (pImo && pImo->is_contentobj())
                return static_cast<ImoContentObj*>(pImo);
        }
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoBlockLevelObj* ImoObj::find_block_level_parent()
{
    ImoObj* pParent = this;

    while (pParent && !pParent->is_block_level_obj())
    {
        pParent = pParent->get_block_level_parent();
    }

    return static_cast<ImoBlockLevelObj*>( pParent );
}

//---------------------------------------------------------------------------------------
void ImoObj::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoObj>* p = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (p)
        p->start_visit(this);

    visit_children(v);

    if (p)
        p->end_visit(this);
}

//---------------------------------------------------------------------------------------
void ImoObj::visit_children(BaseVisitor& v)
{
    TreeNode<ImoObj>::children_iterator it;
    for (it = this->begin(); it != this->end(); ++it)
    {
        (*it)->accept_visitor(v);
    }
}

//---------------------------------------------------------------------------------------
ImoObj* ImoObj::get_child_of_type(int objtype)
{
    children_iterator it(this);
    for (it=this->begin(); it != this->end(); ++it)
    {
        if ((*it)->get_obj_type() == objtype)
            return *it;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoObj* ImoObj::get_ancestor_of_type(int objtype)
{
    ImoObj* pImo = get_parent_imo();
    while (pImo && pImo->get_obj_type() != objtype)
        pImo = pImo->get_parent_imo();

    return pImo;
}

//---------------------------------------------------------------------------------------
void ImoObj::set_dirty(bool dirty)
{
    //change status and propagate
    if (dirty)
    {
        m_flags |= k_dirty;
        propagate_dirty();
    }
    else
        m_flags &= ~k_dirty;
}

//---------------------------------------------------------------------------------------
void ImoObj::set_children_dirty(bool value)
{
    value ? m_flags |= k_children_dirty : m_flags &= ~k_children_dirty;
}

//---------------------------------------------------------------------------------------
void ImoObj::propagate_dirty()
{
    ImoObj* pParent = get_parent_imo();
    if (pParent)
    {
        pParent->set_children_dirty(true);
        pParent->propagate_dirty();
    }

    if (this->is_document())
    {
        ImoDocument* pImoDoc = static_cast<ImoDocument*>(this);
        Document* pDoc = pImoDoc->get_the_document();
        pDoc->set_dirty();
    }
}

//---------------------------------------------------------------------------------------
void ImoObj::append_child_imo(ImoObj* pImo)
{
    append_child(pImo);
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ImoObj::remove_child_imo(ImoObj* pImo)
{
    //AWARE: removes child but does not delete it
    remove_child(pImo);
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
string ImoObj::to_string(bool fWithIds)
{
    LdpExporter exporter;
    exporter.set_remove_newlines(true);
    exporter.set_add_id(fWithIds);
    return exporter.get_source(this);
}

//---------------------------------------------------------------------------------------
bool ImoObj::has_attributte(TIntAttribute idx)
{
    return get_attribute(idx) != nullptr;
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoObj::get_supported_attributes()
{
    list<TIntAttribute> supported;
    return supported;
}

//---------------------------------------------------------------------------------------
void ImoObj::set_int_attribute(TIntAttribute attrib, int value)
{
    set_attribute<int>(attrib, value);
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
int ImoObj::get_int_attribute(TIntAttribute attrib)
{
    return get_attribute_value<int>(attrib);
}

//---------------------------------------------------------------------------------------
void ImoObj::set_bool_attribute(TIntAttribute attrib, bool value)
{
    set_attribute<bool>(attrib, value);
}

//---------------------------------------------------------------------------------------
bool ImoObj::get_bool_attribute(TIntAttribute attrib)
{
    return get_attribute_value<bool>(attrib);
}

//---------------------------------------------------------------------------------------
void ImoObj::set_double_attribute(TIntAttribute attrib, double value)
{
    set_attribute<double>(attrib, value);
}

//---------------------------------------------------------------------------------------
double ImoObj::get_double_attribute(TIntAttribute attrib)
{
    return get_attribute_value<double>(attrib);
}

//---------------------------------------------------------------------------------------
void ImoObj::set_float_attribute(TIntAttribute attrib, float value)
{
    set_attribute<float>(attrib, value);
}

//---------------------------------------------------------------------------------------
float ImoObj::get_float_attribute(TIntAttribute attrib)
{
    return get_attribute_value<float>(attrib);
}

//---------------------------------------------------------------------------------------
void ImoObj::set_string_attribute(TIntAttribute attrib, const string& value)
{
    set_attribute<string>(attrib, value);
}

//---------------------------------------------------------------------------------------
string ImoObj::get_string_attribute(TIntAttribute attrib)
{
    return get_attribute_value<string>(attrib);
}

//---------------------------------------------------------------------------------------
void ImoObj::set_color_attribute(TIntAttribute idx, Color value)
{
    AttrObj* pAttr = get_attribute(idx);
    if (pAttr)
    {
        Attr< Color >* pT = dynamic_cast< Attr< Color >* >(pAttr);
        if (pT)
        {
            pT->set_value(value);
            return;
        }

        else
        {
            pAttr->replace_by( LOMSE_NEW Attr< Color >(idx, value) );
            return;
        }
    }

    add_attribute( LOMSE_NEW Attr< Color >(idx, value) );
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
Color ImoObj::get_color_attribute(TIntAttribute idx)
{
    return get_attribute_value< Color >(idx);
}


//=======================================================================================
// ImoAuxObj implementation
//=======================================================================================
ImoStaffObj* ImoAuxObj::get_parent_staffobj()
{
    ImoObj* pImo = get_parent_imo();
    while (pImo && !pImo->is_staffobj())
        pImo = pImo->get_parent_imo();

    return (pImo && pImo->is_staffobj() ? static_cast<ImoStaffObj*>(pImo) : nullptr);
}


//=======================================================================================
// ImoAuxRelObj implementation
//=======================================================================================
ImoAuxRelObj::~ImoAuxRelObj()
{
    if (m_nextId != k_no_imoid)
        get_ARO_imo(m_nextId)->set_prev_ARO(m_prevId);

    if (m_prevId != k_no_imoid)
        get_ARO_imo(m_prevId)->link_to_next_ARO(m_nextId);
}

//---------------------------------------------------------------------------------------
void ImoAuxRelObj::link_to_next_ARO(ImoId idNext)
{
    m_nextId = idNext;
    if (idNext != k_no_imoid)
        get_ARO_imo(idNext)->set_prev_ARO(this->get_id());
}

//---------------------------------------------------------------------------------------
void ImoAuxRelObj::set_prev_ARO(ImoId idPrev)
{
    m_prevId = idPrev;
}

//---------------------------------------------------------------------------------------
ImoAuxRelObj* ImoAuxRelObj::get_ARO_imo(ImoId id)
{
    if (m_pDocModel)
        return static_cast<ImoAuxRelObj*>( m_pDocModel->get_pointer_to_imo(id) );

    return nullptr;
}


//=======================================================================================
// ImoRelObj implementation
//=======================================================================================
ImoRelObj::~ImoRelObj()
{
#if (LOMSE_RELOBJ_USES_ID == 1)
    delete m_pointersList;
#endif
}

//---------------------------------------------------------------------------------------
ImoRelObj& ImoRelObj::clone(const ImoRelObj& a)
{
    std::list< pair<ImoId, ImoRelDataObj*> >::const_iterator it;
    for (it = a.m_relatedObjects.begin(); it != a.m_relatedObjects.end(); ++it)
    {
        if ((*it).second)
            m_relatedObjects.emplace_back(
                (*it).first, static_cast<ImoRelDataObj*>( ImFactory::clone((*it).second) ));
        else
            m_relatedObjects.emplace_back((*it).first, nullptr);
    }
    return *this;
}

//---------------------------------------------------------------------------------------
void ImoRelObj::accept_visitor_for_data(BaseVisitor& v, ImoStaffObj* pSO)
{
    ImoRelDataObj* pImo = get_data_for(pSO);
    if (pImo)
        pImo->accept_visitor(v);
}

//---------------------------------------------------------------------------------------
void ImoRelObj::push_back(ImoStaffObj* pSO, ImoRelDataObj* pData)
{
#if (LOMSE_RELOBJ_USES_ID == 1)
    m_relatedObjects.emplace_back(pSO->get_id(), pData);
    delete m_pointersList;
    m_pointersList = nullptr;
#else
    m_relatedObjects.push_back( make_pair(pSO, pData));
#endif
}

//---------------------------------------------------------------------------------------
void ImoRelObj::remove(ImoStaffObj* pSO)
{
#if (LOMSE_RELOBJ_USES_ID == 1)
    ImoId id = pSO->get_id();
    std::list< pair<ImoId, ImoRelDataObj*> >::iterator it;
    for (it = m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it)
    {
        if ((*it).first == id)
        {
            delete (*it).second;
            m_relatedObjects.erase(it);
            break;
        }
    }
    if (m_pointersList)
    {
        std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it2;
        for (it2 = m_pointersList->begin(); it2 != m_pointersList->end(); ++it2)
        {
            if ((*it2).first == pSO)
            {
                m_pointersList->erase(it2);
                return;
            }
        }
    }
#else
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for (it = m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it)
    {
        if ((*it).first == pSO)
        {
            delete (*it).second;
            m_relatedObjects.erase(it);
            return;
        }
    }
#endif
}

//---------------------------------------------------------------------------------------
void ImoRelObj::remove_all()
{
    //This is recursive. If there are objects, the first one is deleted by
    //invoking "pSO->remove_but_not_delete_relation(this);" . And it, in turn,
    //invokes this method, until all items get deleted!
    while (m_relatedObjects.size() > 0)
    {
        ImoStaffObj* pSO = get_start_object();
        pSO->remove_but_not_delete_relation(this);
    }

    delete m_pointersList;
    m_pointersList = nullptr;
}

//---------------------------------------------------------------------------------------
ImoRelDataObj* ImoRelObj::get_data_for(ImoStaffObj* pSO)
{
#if (LOMSE_RELOBJ_USES_ID == 1)
    ImoId id = pSO->get_id();
    std::list< pair<ImoId, ImoRelDataObj*> >::iterator it;
    for (it = m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it)
    {
        if ((*it).first == id)
            return (*it).second;
    }
    return nullptr;
#else
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for (it = m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it)
    {
        if ((*it).first == pSO)
            return (*it).second;
    }
    return nullptr;
#endif
}

#if (LOMSE_RELOBJ_USES_ID == 1)
//---------------------------------------------------------------------------------------
ImoStaffObj* ImoRelObj::get_start_object()
{
    ImoId id = m_relatedObjects.front().first;
    return static_cast<ImoStaffObj*>( m_pDocModel->get_pointer_to_imo(id) );
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoRelObj::get_end_object()
{
    ImoId id = m_relatedObjects.back().first;
    return static_cast<ImoStaffObj*>( m_pDocModel->get_pointer_to_imo(id) );
}

//---------------------------------------------------------------------------------------
std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& ImoRelObj::get_related_objects()
{
    if (m_pointersList)
        return *m_pointersList;

    m_pointersList = LOMSE_NEW std::list< pair<ImoStaffObj*, ImoRelDataObj*> >;
    std::list< pair<ImoId, ImoRelDataObj*> >::iterator it;
    for (it = m_relatedObjects.begin(); it != m_relatedObjects.end(); ++it)
    {
        ImoId id = (*it).first;
        m_pointersList->emplace_back(
                static_cast<ImoStaffObj*>( m_pDocModel->get_pointer_to_imo(id) ),
                (*it).second );
    }
    return *m_pointersList;
}
#endif

//=======================================================================================
// ImoScoreObj implementation
//=======================================================================================
void ImoScoreObj::set_int_attribute(TIntAttribute attrib, int value)
{
    ImoContentObj::set_int_attribute(attrib, value);
}

//---------------------------------------------------------------------------------------
int ImoScoreObj::get_int_attribute(TIntAttribute attrib)
{
    return ImoContentObj::get_int_attribute(attrib);
}

//---------------------------------------------------------------------------------------
void ImoScoreObj::set_color_attribute(TIntAttribute attrib, Color value)
{
    if (k_attr_color)
    {
        set_color(value);
        set_dirty(true);
    }
    else
        ImoContentObj::set_color_attribute(attrib, value);
}

//---------------------------------------------------------------------------------------
Color ImoScoreObj::get_color_attribute(TIntAttribute attrib)
{
    if (k_attr_color)
        return m_color;
    else
        return ImoContentObj::get_color_attribute(attrib);
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoScoreObj::get_supported_attributes()
{
    list<TIntAttribute> supported = ImoContentObj::get_supported_attributes();
    supported.push_back(k_attr_color);
    return supported;
}


//=======================================================================================
// ImoStaffObj implementation
//=======================================================================================
ImoStaffObj::~ImoStaffObj()
{
    ImoRelations* pRelObjs = get_relations();
    if (pRelObjs)
    {
        pRelObjs->remove_from_all_relations(this);
        this->remove_child_imo(pRelObjs);
        delete pRelObjs;
    }
}

//---------------------------------------------------------------------------------------
ImoStaffObj& ImoStaffObj::clone(const ImoStaffObj& a)
{
    m_staff = a.m_staff;
    m_nVoice = a.m_nVoice;
    m_time = a.m_time;
    m_pEntry = nullptr;  //will be instantiated when the model is built

    return *this;
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::add_relation(ImoRelObj* pRO)
{
    ImoRelations* pRelObjs = get_relations();
    if (!pRelObjs)
    {
        pRelObjs = static_cast<ImoRelations*>(
                        ImFactory::inject(k_imo_relations, m_pDocModel) );
        append_child_imo(pRelObjs);
    }
    pRelObjs->add_relation(pRO);
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::include_in_relation(ImoRelObj* pRelObj, ImoRelDataObj* pData)
{
    add_relation(pRelObj);
    pRelObj->push_back(this, pData);
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::remove_from_relation(ImoRelObj* pRelObj)
{
    remove_but_not_delete_relation(pRelObj);

    if (pRelObj->get_num_objects() < pRelObj->get_min_number_for_autodelete())
        pRelObj->remove_all();

    if (pRelObj->get_num_objects() == 0)
        delete pRelObj;
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::remove_but_not_delete_relation(ImoRelObj* pRelObj)
{
    //Remove this ImoStaffObj from the ImoRelObj
    pRelObj->remove(this);

    //detach this ImoRelObj from the relations in this ImoStaffObj
    ImoRelations* pRelObjs = get_relations();
    pRelObjs->remove_relation(pRelObj);
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoStaffObj::get_relation(int i)
{
    ImoRelations* pRelObjs = get_relations();
    return pRelObjs->get_item(i);
}

//---------------------------------------------------------------------------------------
bool ImoStaffObj::has_relations()
{
    ImoRelations* pRelObjs = get_relations();
    if (pRelObjs)
        return pRelObjs->get_num_items() > 0;
    else
        return false;
}

//---------------------------------------------------------------------------------------
int ImoStaffObj::get_num_relations()
{
    ImoRelations* pRelObjs = get_relations();
    if (pRelObjs)
        return pRelObjs->get_num_items();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoRelations* ImoStaffObj::get_relations()
{
    return dynamic_cast<ImoRelations*>( get_child_of_type(k_imo_relations) );
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoStaffObj::find_relation(int type)
{
    ImoRelations* pRelObjs = get_relations();
    if (!pRelObjs)
        return nullptr;
    else
        return pRelObjs->find_item_of_type(type);
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoStaffObj::get_next_noterest_or_barline()
{
    ImoObj* pNext = get_next_sibling();
    while (pNext && !(pNext->is_note_rest() || pNext->is_barline()))
        pNext = pNext->get_next_sibling();

    if (pNext && pNext->is_staffobj())
        return static_cast<ImoStaffObj*>(pNext);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoStaffObj::get_instrument()
{
    ImoObj* pParent = this->get_parent_imo();   //musicData
    return static_cast<ImoInstrument*>( pParent->get_parent_imo() );
}

//---------------------------------------------------------------------------------------
ImoScore* ImoStaffObj::get_score()
{
    ImoInstrument* pInstr = get_instrument();
    return pInstr->get_score();
}

//---------------------------------------------------------------------------------------
void ImoStaffObj::set_int_attribute(TIntAttribute attrib, int value)
{
    switch(attrib)
    {
        case k_attr_staff_num:
            m_staff = value;
            set_dirty(true);
            break;

        default:
            ImoScoreObj::set_int_attribute(attrib, value);
    }
}

//---------------------------------------------------------------------------------------
int ImoStaffObj::get_int_attribute(TIntAttribute attrib)
{
    switch(attrib)
    {
        case k_attr_staff_num:      return m_staff;
        default:
            return ImoScoreObj::get_int_attribute(attrib);
    }
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoStaffObj::get_supported_attributes()
{
    list<TIntAttribute> supported = ImoScoreObj::get_supported_attributes();
    supported.push_back(k_attr_staff_num);
    return supported;
}


//=======================================================================================
// ImoBeamData implementation
//=======================================================================================
ImoBeamData::ImoBeamData(ImoBeamDto* pDto)
    : ImoRelDataObj(k_imo_beam_data)
{
    for (int i=0; i < 6; ++i)
    {
        m_beamType[i] = pDto->get_beam_type(i);
        m_repeat[i] = pDto->get_repeat(i);
    }
}

//---------------------------------------------------------------------------------------
ImoBeamData::ImoBeamData()
    : ImoRelDataObj(k_imo_beam_data)
{
    for (int level=0; level < 6; level++)
    {
        m_beamType[level] = ImoBeam::k_none;
        m_repeat[level] = false;
    }
}

//---------------------------------------------------------------------------------------
bool ImoBeamData::is_start_of_beam()
{
    //at least one is begin, forward or backward.

    bool fStart = false;
    for (int level=0; level < 6; level++)
    {
        if (m_beamType[level] == ImoBeam::k_end
            || m_beamType[level] == ImoBeam::k_continue
            )
            return false;
        if (m_beamType[level] != ImoBeam::k_none)
            fStart = true;
    }
    return fStart;
}

//---------------------------------------------------------------------------------------
bool ImoBeamData::is_end_of_beam()
{
    for (int level=0; level < 6; level++)
    {
        if (m_beamType[level] == ImoBeam::k_begin
            || m_beamType[level] == ImoBeam::k_forward
            || m_beamType[level] == ImoBeam::k_continue
            )
            return false;
    }
    return true;
}


//=======================================================================================
// ImoBeamDto implementation
//=======================================================================================
ImoBeamDto::ImoBeamDto()
    : ImoSimpleObj(k_imo_beam_dto)
    , m_beamNum(0)
    , m_pBeamElm(nullptr)
    , m_pNR(nullptr)
    , m_lineNum(0)
{
    for (int level=0; level < 6; level++)
    {
        m_beamType[level] = ImoBeam::k_none;
        m_repeat[level] = false;
    }
}

//---------------------------------------------------------------------------------------
void ImoBeamDto::set_beam_type(int level, int type)
{
    m_beamType[level] = type;
}

//---------------------------------------------------------------------------------------
void ImoBeamDto::set_beam_type(string& segments)
{
    if (segments.size() < 7)
    {
        for (int i=0; i < int(segments.size()); ++i)
        {
            if (segments[i] == '+')
                set_beam_type(i, ImoBeam::k_begin);
            else if (segments[i] == '=')
                set_beam_type(i, ImoBeam::k_continue);
            else if (segments[i] == '-')
                set_beam_type(i, ImoBeam::k_end);
            else if (segments[i] == 'f')
                set_beam_type(i, ImoBeam::k_forward);
            else if (segments[i] == 'b')
                set_beam_type(i, ImoBeam::k_backward);
            else
                return;   //error
        }
    }
}

//---------------------------------------------------------------------------------------
int ImoBeamDto::get_beam_type(int level)
{
    return m_beamType[level];
}

//---------------------------------------------------------------------------------------
bool ImoBeamDto::is_start_of_beam()
{
    //at least one is begin, forward or backward.

    bool fStart = false;
    for (int level=0; level < 6; level++)
    {
        if (m_beamType[level] == ImoBeam::k_end
            || m_beamType[level] == ImoBeam::k_continue
            )
            return false;
        if (m_beamType[level] != ImoBeam::k_none)
            fStart = true;
    }
    return fStart;
}

//---------------------------------------------------------------------------------------
bool ImoBeamDto::is_end_of_beam()
{
    for (int level=0; level < 6; level++)
    {
        if (m_beamType[level] == ImoBeam::k_begin
            || m_beamType[level] == ImoBeam::k_forward
            || m_beamType[level] == ImoBeam::k_continue
            )
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------
void ImoBeamDto::set_repeat(int level, bool value)
{
    m_repeat[level] = value;
}

//---------------------------------------------------------------------------------------
bool ImoBeamDto::get_repeat(int level)
{
    return m_repeat[level];
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoBeamDto::get_staffobj()
{
    return m_pNR;
}


//=======================================================================================
// ImoBeam implementation
//=======================================================================================
void ImoBeam::reorganize_after_object_deletion()
{
    AutoBeamer autobeamer(this);
    autobeamer.do_autobeam();
}

//---------------------------------------------------------------------------------------
int ImoBeam::get_max_staff()
{
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = get_related_objects();
    ImoStaffObj* pSO = notes.front().first;
    if (pSO->get_instrument()->get_num_staves() == 1)
        return pSO->get_staff();

    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    int iStaff = 0;
    for (it=notes.begin(); it != notes.end(); ++it)
    {
        if ((*it).first->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>((*it).first);
            if (pNote->is_in_chord())
            {
                ImoChord* pChord = pNote->get_chord();
                list< pair<ImoStaffObj*, ImoRelDataObj*> >& chordNotes = pChord->get_related_objects();
                list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itC;
                for (itC=chordNotes.begin(); itC != chordNotes.end(); ++itC)
                {
                    iStaff = max(iStaff, ((*itC).first)->get_staff());
                }
            }
            else
                iStaff = max(iStaff, ((*it).first)->get_staff());
        }
    }
    return iStaff;
}

//---------------------------------------------------------------------------------------
int ImoBeam::get_min_staff()
{
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = get_related_objects();
    ImoStaffObj* pSO = notes.front().first;
    if (pSO->get_instrument()->get_num_staves() == 1)
        return pSO->get_staff();

    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    int iStaff = 10000;
    for (it=notes.begin(); it != notes.end() && iStaff != 0; ++it)
    {
        if ((*it).first->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>((*it).first);
            if (pNote->is_in_chord())
            {
                ImoChord* pChord = pNote->get_chord();
                list< pair<ImoStaffObj*, ImoRelDataObj*> >& chordNotes = pChord->get_related_objects();
                list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator itC;
                for (itC=chordNotes.begin(); itC != chordNotes.end(); ++itC)
                {
                    iStaff = min(iStaff, ((*itC).first)->get_staff());
                }
            }
            else
                iStaff = min(iStaff, ((*it).first)->get_staff());
        }
    }
    return iStaff;
}

//---------------------------------------------------------------------------------------
bool ImoBeam::contains_chords()
{
    list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = get_related_objects();
    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for (it=notes.begin(); it != notes.end(); ++it)
    {
        if ((*it).first->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>((*it).first);
            if (pNote->is_in_chord())
                return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------
void ImoBeam::set_stems_direction(vector<int>* pStemsDir)
{
    delete m_pStemsDir;
    m_pStemsDir = pStemsDir;
}


//=======================================================================================
// ImoBezierInfo implementation
//=======================================================================================
ImoBezierInfo::ImoBezierInfo(ImoBezierInfo* pBezier)
    : ImoSimpleObj(k_imo_bezier_info)
{
    if (pBezier)
    {
        for (int i=0; i < 4; ++i)
            m_tPoints[i] = pBezier->get_point(i);
    }
    else
    {
        for (int i=0; i < 4; ++i)
            m_tPoints[i] = TPoint(0.0f, 0.0f);
    }
}


//=======================================================================================
// ImoBlocksContainer implementation
//=======================================================================================
int ImoBlocksContainer::get_num_content_items()
{
    ImoContent* pContent = get_content();
    if (pContent)
        return pContent->get_num_children();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoContentObj* ImoBlocksContainer::get_content_item(int iItem)
{
    ImoContent* pContent = get_content();
    if (pContent)
    {
        int numChild = 0;
        children_iterator it = pContent->begin();
        for (; it != pContent->end() && numChild < iItem; ++it, ++numChild);
        if (it != pContent->end() && iItem == numChild)
            return dynamic_cast<ImoContentObj*>(*it);
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoContentObj* ImoBlocksContainer::get_first_content_item()
{
    return get_content_item(0);
}

//---------------------------------------------------------------------------------------
ImoContentObj* ImoBlocksContainer::get_last_content_item()
{
    int last = get_num_content_items() - 1;
    if (last >= 0)
        return get_content_item(last);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoContent* ImoBlocksContainer::get_content()
{
    //AWARE: ImoContent doesn't have an inner ImoContent container
    if (this->is_content())
        return static_cast<ImoContent*>(this);
    else
        return static_cast<ImoContent*>( get_child_of_type(k_imo_content) );
}

//---------------------------------------------------------------------------------------
void ImoBlocksContainer::create_content_container()
{
    append_child_imo( ImFactory::inject(k_imo_content, m_pDocModel) );
}

//---------------------------------------------------------------------------------------
void ImoBlocksContainer::append_content_item(ImoContentObj* pItem)
{
    ImoContent* pContent = get_content();
    if (!pContent)
        pContent = add_content_wrapper();
    pContent->append_child_imo(pItem);
}

//---------------------------------------------------------------------------------------
ImoParagraph* ImoBlocksContainer::add_paragraph(ImoStyle* pStyle)
{
    ImoParagraph* pImo = static_cast<ImoParagraph*>(
                                    ImFactory::inject(k_imo_para, m_pDocModel) );
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoList* ImoBlocksContainer::add_list(int type, ImoStyle* pStyle)
{
    ImoList* pImo = static_cast<ImoList*>(ImFactory::inject(k_imo_list, m_pDocModel) );
    pImo->set_list_type(type);
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoContent* ImoBlocksContainer::add_content_wrapper(ImoStyle* pStyle)
{
    ImoContent* pImo = static_cast<ImoContent*>(
                                    ImFactory::inject(k_imo_content, m_pDocModel) );
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoMultiColumn* ImoBlocksContainer::add_multicolumn_wrapper(int numCols, ImoStyle* pStyle)
{
    ImoMultiColumn* pImo =
        static_cast<ImoMultiColumn*>( ImFactory::inject(k_imo_multicolumn, m_pDocModel) );

    add_to_model(pImo, pStyle);
    pImo->create_columns(numCols);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoScore* ImoBlocksContainer::add_score(ImoStyle* pStyle)
{
    ImoScore* pImo = static_cast<ImoScore*>( ImFactory::inject(k_imo_score, m_pDocModel) );
    pImo->set_version(200);   // version 2.0
    add_to_model(pImo, pStyle);
    return pImo;
}

//---------------------------------------------------------------------------------------
void ImoBlocksContainer::add_to_model(ImoBlockLevelObj* pImo, ImoStyle* pStyle)
{
    pImo->set_style(pStyle);
    ImoObj* pNode = this->is_document() ?
                    static_cast<ImoDocument*>(this)->get_content()
                    : this;
    pNode->append_child_imo(pImo);
}



//=======================================================================================
// ImoBarline implementation
//=======================================================================================
ImoBarline::~ImoBarline()
{
    delete m_pMeasureInfo;
}

//---------------------------------------------------------------------------------------
ImoBarline& ImoBarline::clone(const ImoBarline& a)
{
    m_barlineType = a.m_barlineType;
    m_fMiddle = a.m_fMiddle;
    m_fTKChange = a.m_fTKChange;
    m_times = a.m_times;
    m_winged = a.m_winged;

    if (a.m_pMeasureInfo)
        m_pMeasureInfo = LOMSE_NEW TypeMeasureInfo(*a.m_pMeasureInfo);
    else
        m_pMeasureInfo = nullptr;

    return *this;
}

//---------------------------------------------------------------------------------------
void ImoBarline::set_int_attribute(TIntAttribute attrib, int value)
{
    //AWARE: override of staff_num (in ImoStaffObj)

    switch(attrib)
    {
        case k_attr_staff_num:
            m_staff = 0;    //barlines always in staff 0
            break;

        case k_attr_barline:
            m_barlineType = value;
            set_dirty(true);
            break;

        default:
            ImoStaffObj::set_int_attribute(attrib, value);
    }
}

//---------------------------------------------------------------------------------------
int ImoBarline::get_int_attribute(TIntAttribute attrib)
{
    switch(attrib)
    {
        case k_attr_barline:        return m_barlineType;
        default:
            return ImoStaffObj::get_int_attribute(attrib);
    }
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoBarline::get_supported_attributes()
{
    list<TIntAttribute> supported = ImoStaffObj::get_supported_attributes();
    supported.push_back(k_attr_barline);

    supported.remove(k_attr_staff_num);     //always 0
    supported.remove(k_attr_ypos);          //vertical displacement not allowed

    return supported;
}


//=======================================================================================
// ImoButton implementation
//=======================================================================================
ImoButton::ImoButton()
    : ImoControl(k_imo_button)
    , m_bgColor( Color(255,255,255) )
    , m_fEnabled(true)
{
}


//=======================================================================================
// ImoColorDto implementation
//=======================================================================================
ImoColorDto::ImoColorDto(Int8u r, Int8u g, Int8u b, Int8u a)
    : ImoDto(k_imo_color_dto)
    , m_color(r, g, b, a)
    , m_ok(true)
{
}

//---------------------------------------------------------------------------------------
Int8u ImoColorDto::convert_from_hex(const std::string& hex)
{
    int value = 0;

    int a = 0;
    int b = static_cast<int>(hex.length()) - 1;
    for (; b >= 0; a++, b--)
    {
        if (hex[b] >= '0' && hex[b] <= '9')
        {
            value += (hex[b] - '0') * (1 << (a * 4));
        }
        else
        {
            switch (hex[b])
            {
                case 'A':
                case 'a':
                    value += 10 * (1 << (a * 4));
                    break;

                case 'B':
                case 'b':
                    value += 11 * (1 << (a * 4));
                    break;

                case 'C':
                case 'c':
                    value += 12 * (1 << (a * 4));
                    break;

                case 'D':
                case 'd':
                    value += 13 * (1 << (a * 4));
                    break;

                case 'E':
                case 'e':
                    value += 14 * (1 << (a * 4));
                    break;

                case 'F':
                case 'f':
                    value += 15 * (1 << (a * 4));
                    break;

                default:
                    m_ok = false;
                    //cout << "Error: invalid character '" << hex[b] << "' in hex number" << endl;
                    break;
            }
        }
    }

    return static_cast<Int8u>(value);
}

//---------------------------------------------------------------------------------------
Color& ImoColorDto::set_from_rgb_string(const std::string& rgb)
{
    m_ok = true;

    if (rgb[0] == '#')
    {
        m_color.r = convert_from_hex( rgb.substr(1, 2) );
        m_color.g = convert_from_hex( rgb.substr(3, 2) );
        m_color.b = convert_from_hex( rgb.substr(5, 2) );
        m_color.a = 255;
    }

    if (!m_ok)
        m_color = Color(0,0,0,255);

    return m_color;
}

//---------------------------------------------------------------------------------------
Color& ImoColorDto::set_from_rgba_string(const std::string& rgba)
{
    m_ok = true;

    if (rgba[0] == '#')
    {
        m_color.r = convert_from_hex( rgba.substr(1, 2) );
        m_color.g = convert_from_hex( rgba.substr(3, 2) );
        m_color.b = convert_from_hex( rgba.substr(5, 2) );
        m_color.a = convert_from_hex( rgba.substr(7, 2) );
    }

    if (!m_ok)
        m_color = Color(0,0,0,255);

    return m_color;
}

//---------------------------------------------------------------------------------------
Color& ImoColorDto::set_from_argb_string(const std::string& argb)
{
    m_ok = true;

    if (argb[0] == '#')
    {
        m_color.a = convert_from_hex( argb.substr(1, 2) );
        m_color.r = convert_from_hex( argb.substr(3, 2) );
        m_color.g = convert_from_hex( argb.substr(5, 2) );
        m_color.b = convert_from_hex( argb.substr(7, 2) );
    }

    if (!m_ok)
        m_color = Color(0,0,0,255);

    return m_color;
}

//---------------------------------------------------------------------------------------
Color& ImoColorDto::set_from_string(const std::string& hex)
{
    if (hex.length() == 7)
        return set_from_rgb_string(hex);
    else if (hex.length() == 9)
        return set_from_rgba_string(hex);
    else
    {
        m_ok = false;
        m_color = Color(0,0,0,255);
        return m_color;
    }
}


//=======================================================================================
// ImoChord implementation
//=======================================================================================
void ImoChord::update_cross_staff_data()
{
    m_fCrossStaff = false;

    list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes = get_related_objects();
    list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it = notes.begin();
    ImoNote* pNote = static_cast<ImoNote*>((*it).first);
    int staff = pNote->get_staff();

    for (++it; it != notes.end(); ++it)
    {
        ImoNote* pN = static_cast<ImoNote*>((*it).first);
        if ((m_fCrossStaff = (pN->get_staff() != staff) ))
            break;
    }

//    list< pair<ImoId, ImoRelDataObj*> >::iterator it = m_relatedObjects.begin();
//    ImoNote* pNote = static_cast<ImoNote*>(m_pDocModel->get_pointer_to_imo((*it).first));
//    int staff = pNote->get_staff();
//
//    for (++it; it != m_relatedObjects.end(); ++it)
//    {
//        ImoNote* pN = static_cast<ImoNote*>(m_pDocModel->get_pointer_to_imo((*it).first));
//        if ((m_fCrossStaff = (pN->get_staff() != staff) ))
//            break;
//    }
}

//---------------------------------------------------------------------------------------
void ImoChord::reorganize_after_object_deletion()
{
    update_cross_staff_data();
}

//---------------------------------------------------------------------------------------
void ImoChord::push_back(ImoStaffObj* pSO, ImoRelDataObj* pData)
{
    ImoRelObj::push_back(pSO, pData);
    update_cross_staff_data();
}

//---------------------------------------------------------------------------------------
ImoNote* ImoChord::get_start_note()
{
    return static_cast<ImoNote*>(get_start_object());
}

//---------------------------------------------------------------------------------------
ImoNote* ImoChord::get_end_note()
{
    return static_cast<ImoNote*>(get_end_object());
}


//=======================================================================================
// ImoClef implementation
//=======================================================================================
void ImoClef::set_clef(int sign, int line, int octaveChange)
{
    m_sign = sign;
    m_line = line;
    m_octaveChange = octaveChange;
}

//---------------------------------------------------------------------------------------
int ImoClef::get_clef_type() const
{
    if (m_sign == k_clef_sign_G)
    {
        if (m_line==1)
            return k_clef_G1;
        else if (m_line==2)
        {
            if (m_octaveChange==0)
                return k_clef_G2;
            else if (m_octaveChange==1)
                return k_clef_8_G2;     //G2, 8ve. above
            else if (m_octaveChange==2)
                return k_clef_15_G2;    //G2, 15 above
            else if (m_octaveChange==-1)
                return k_clef_G2_8;     //G2, 8ve. below
            else // must be m_octaveChange==-2
                return k_clef_G2_15;    //G2, 15 below
        }
    }
    else if (m_sign == k_clef_sign_F)
    {
        if (m_line==4)
        {
            if (m_octaveChange==0)
                return k_clef_F4;
            else if (m_octaveChange==1)
                return k_clef_8_F4;     //F4 clef, 8ve. above
            else if (m_octaveChange==2)
                return k_clef_15_F4;    //F4 clef, 15 above
            else if (m_octaveChange==-1)
                return k_clef_F4_8;     //F4 clef, 8ve. below
            else    //must be m_octaveChange==-2
                return k_clef_F4_15;    //F4 clef, 15 below
        }
        else if (m_line==3)
            return k_clef_F3;
        else if (m_line==5)
            return k_clef_F5;
    }
    else if (m_sign == k_clef_sign_C)
    {
        if (m_line==1)
            return k_clef_C1;
        else if (m_line==2)
            return k_clef_C2;
        else if (m_line==3)
            return k_clef_C3;
        else if (m_line==4)
            return k_clef_C4;
        else if (m_line==5)
            return k_clef_C5;
    }

    else if (m_sign == k_clef_sign_percussion)
        return k_clef_percussion;
    else if (m_sign == k_clef_sign_TAB)
        return k_clef_TAB;
    else if (m_sign == k_clef_sign_none)
        return k_clef_none;
    //TODO: Other values: jianpu

    return k_clef_undefined;
}

//---------------------------------------------------------------------------------------
void ImoClef::set_clef_type(int type)
{
    switch(type)
    {
        case k_clef_15_G2:  set_clef(k_clef_sign_G, 2, 2);      break;
        case k_clef_8_G2:   set_clef(k_clef_sign_G, 2, 1);      break;
        case k_clef_G2:     set_clef(k_clef_sign_G, 2, 0);      break;
        case k_clef_G2_8:   set_clef(k_clef_sign_G, 2, -1);     break;
        case k_clef_G2_15:  set_clef(k_clef_sign_G, 2, -2);     break;
        case k_clef_15_F4:  set_clef(k_clef_sign_F, 4, 2);      break;
        case k_clef_8_F4:   set_clef(k_clef_sign_F, 4, 1);      break;
        case k_clef_F4:     set_clef(k_clef_sign_F, 4, 0);      break;
        case k_clef_F4_8:   set_clef(k_clef_sign_F, 4, -1);     break;
        case k_clef_F4_15:  set_clef(k_clef_sign_F, 4, -2);     break;
        case k_clef_F3:     set_clef(k_clef_sign_F, 3, 0);      break;
        case k_clef_C1:     set_clef(k_clef_sign_C, 1, 0);      break;
        case k_clef_C2:     set_clef(k_clef_sign_C, 2, 0);      break;
        case k_clef_C3:     set_clef(k_clef_sign_C, 3, 0);      break;
        case k_clef_C4:     set_clef(k_clef_sign_C, 4, 0);      break;
        case k_clef_C5:     set_clef(k_clef_sign_C, 5, 0);      break;
        case k_clef_F5:     set_clef(k_clef_sign_F, 5, 0);      break;
        case k_clef_G1:     set_clef(k_clef_sign_G, 1, 0);      break;
        case k_clef_percussion: set_clef(k_clef_sign_percussion, 3, 0);     break;
        case k_clef_TAB:        set_clef(k_clef_sign_TAB, 3, 0);    break;
        case k_clef_none:       set_clef(k_clef_sign_none, 3, 0);   break;
        case k_clef_undefined:  set_clef(k_clef_sign_none, 3, 0);   break;
        default:
        {
            LOMSE_LOG_ERROR("Program maintenance error: missing clef type %d", type);
            set_clef(k_clef_sign_none, 3, 0);
        }
    }
}

//---------------------------------------------------------------------------------------
int ImoClef::get_default_octave()
{
    switch(m_sign)
    {
        case k_clef_sign_G:     return 4;
        case k_clef_sign_F:     return 3;
        case k_clef_sign_C:     return 4;
        default:
            return 0;
    }
}

//---------------------------------------------------------------------------------------
int ImoClef::get_octave()
{
    return m_octaveChange + get_default_octave();
}

//---------------------------------------------------------------------------------------
bool ImoClef::supports_accidentals()
{
    switch(m_sign)
    {
        case k_clef_sign_G:     return true;
        case k_clef_sign_F:     return true;
        case k_clef_sign_C:     return true;
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------
DiatonicPitch ImoClef::get_first_ledger_line_pitch()
{
    //return diatonic pitch for first line

    int step = 0;
    if (m_sign == k_clef_sign_G)
        step = k_step_G;
    else if (m_sign == k_clef_sign_F)
        step = k_step_F;
    else if (m_sign == k_clef_sign_C)
        step = k_step_C;
    else
        return DiatonicPitch(-1);

    step -= m_line * 2;
    int octave = get_octave();
    while (step < 0)
    {
        step += 7;
        --octave;
    }

    return DiatonicPitch(step, octave);
}


//=======================================================================================
// ImoControl implementation
//=======================================================================================
ImoControl::ImoControl(Control* ctrol)
    : ImoInlineLevelObj(k_imo_control)
    , m_ctrol( std::shared_ptr<Control>(ctrol) )
{
}

////---------------------------------------------------------------------------------------
//ImoControl::~ImoControl()
//{
//    delete m_ctrol;
//}

//---------------------------------------------------------------------------------------
void ImoControl::attach_control(Control* ctrol)
{
    m_ctrol = std::shared_ptr<Control>(ctrol);
    ctrol->set_owner_imo(this);
}

//---------------------------------------------------------------------------------------
GmoBoxControl* ImoControl::layout(LibraryScope& libraryScope, UPoint pos)
{
    return m_ctrol->layout(libraryScope, pos);
}

//---------------------------------------------------------------------------------------
USize ImoControl::measure()
{
    return m_ctrol->measure();
}

//---------------------------------------------------------------------------------------
Control* ImoControl::get_control()
{
    return m_ctrol.get();
}


//=======================================================================================
// ImoDirection implementation
//=======================================================================================
ImoNoteRest* ImoDirection::get_noterest_for_transferred_dynamics()
{
    if (m_pDocModel->get_owner_document() && m_idNR != k_no_imoid)
        return static_cast<ImoNoteRest*>( m_pDocModel->get_pointer_to_imo(m_idNR) );
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoDirection::mark_as_dynamics_removed(ImoNoteRest* pNR)
{
    if (pNR)
        m_idNR = pNR->get_id();
    else
        m_idNR = k_no_imoid;
}


//=======================================================================================
// ImoRelations implementation
//=======================================================================================
ImoRelations::~ImoRelations()
{
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end(); ++it)
        delete *it;

    m_relations.clear();
}

//---------------------------------------------------------------------------------------
ImoRelations& ImoRelations::clone(const ImoRelations& a)
{
    RelObjCloner* pCloner = m_pDocModel->get_relobj_cloner();
    std::list<ImoRelObj*>::const_iterator it;
    for(it = a.m_relations.begin(); it != a.m_relations.end(); ++it)
    {
        m_relations.emplace_back( pCloner->clone_relobj(*it) );
    }
    return *this;
}

//---------------------------------------------------------------------------------------
void ImoRelations::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoObj>* vObj = nullptr;

    vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (vObj)
        vObj->start_visit(this);

    //visit_children
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end(); ++it)
    {
        (*it)->accept_visitor(v);
        (*it)->accept_visitor_for_data(v, static_cast<ImoStaffObj*>(get_parent()));
    }

    if (vObj)
        vObj->end_visit(this);
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoRelations::get_item(int iItem)
{
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end() && iItem > 0; ++it, --iItem);
    if (it != m_relations.end())
        return *it;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoRelations::remove_relation(ImoRelObj* pRO)
{
    m_relations.remove(pRO);
}

//---------------------------------------------------------------------------------------
void ImoRelations::add_relation(ImoRelObj* pRO)
{
    m_relations.push_back(pRO);
}

//---------------------------------------------------------------------------------------
ImoRelObj* ImoRelations::find_item_of_type(int type)
{
    std::list<ImoRelObj*>::iterator it;
    for(it = m_relations.begin(); it != m_relations.end(); ++it)
    {
        if ((*it)->get_obj_type() == type)
            return *it;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoRelations::remove_from_all_relations(ImoStaffObj* pSO)
{
    std::list<ImoRelObj*>::iterator it = m_relations.begin();
    while (it != m_relations.end())
    {
//        if ((*it)->is_relobj())
//        {
            ImoRelObj* pRO = static_cast<ImoRelObj*>( *it );
            ++it;
            pSO->remove_from_relation(pRO);
//        }
//        else    // ????? Must be ImoRelObj. Moreover, code in elso is non-sense
//        {
//            ImoRelObj* pRO = *it;
//            ++it;
//            delete pRO;
//        }
    }
    m_relations.clear();
}


//=======================================================================================
// ImoContentObj implementation
//=======================================================================================
ImoContentObj::ImoContentObj(int objtype)
    : ImoObj(objtype)
    , Observable()
    , m_styleId(k_no_imoid)
    , m_txUserLocation(0.0f)
    , m_tyUserLocation(0.0f)
    , m_txUserRefPoint(0.0f)
    , m_tyUserRefPoint(0.0f)
    , m_fVisible(true)
{
}

//---------------------------------------------------------------------------------------
ImoContentObj::ImoContentObj(ImoId id, int objtype)
    : ImoObj(id, objtype)
    , Observable()
    , m_styleId(k_no_imoid)
    , m_txUserLocation(0.0f)
    , m_tyUserLocation(0.0f)
    , m_txUserRefPoint(0.0f)
    , m_tyUserRefPoint(0.0f)
    , m_fVisible(true)
{
}

//---------------------------------------------------------------------------------------
void ImoContentObj::add_attachment(ImoAuxObj* pAO)
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (!pAuxObjs)
    {
        pAuxObjs = static_cast<ImoAttachments*>(
                        ImFactory::inject(k_imo_attachments, m_pDocModel) );
        append_child_imo(pAuxObjs);
    }
    pAuxObjs->add(pAO);
}

//---------------------------------------------------------------------------------------
ImoAuxObj* ImoContentObj::get_attachment(int i)
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (pAuxObjs)
        return static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
    return nullptr;
}

//---------------------------------------------------------------------------------------
bool ImoContentObj::has_attachments()
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (pAuxObjs)
        return pAuxObjs->get_num_items() > 0;
    else
        return false;
}

//---------------------------------------------------------------------------------------
int ImoContentObj::get_num_attachments()
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (pAuxObjs)
        return pAuxObjs->get_num_items();
    else
        return 0;
}

//---------------------------------------------------------------------------------------
ImoAttachments* ImoContentObj::get_attachments()
{
    return dynamic_cast<ImoAttachments*>( get_child_of_type(k_imo_attachments) );
}

//---------------------------------------------------------------------------------------
void ImoContentObj::remove_attachment(ImoAuxObj* pAO)
{
    ImoAttachments* pAuxObjs = get_attachments();
    pAuxObjs->remove(pAO);
    delete pAO;
}

//---------------------------------------------------------------------------------------
void ImoContentObj::remove_but_not_delete_attachment(ImoAuxObj* pAO)
{
    ImoAttachments* pAuxObjs = get_attachments();
    pAuxObjs->remove(pAO);
}

//---------------------------------------------------------------------------------------
ImoAuxObj* ImoContentObj::find_attachment(int type)
{
    ImoAttachments* pAuxObjs = get_attachments();
    if (!pAuxObjs)
        return nullptr;
    else
        return pAuxObjs->find_item_of_type(type);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoContentObj::get_style(bool fInherit)
{
    if (m_styleId != k_no_imoid)
        return get_style_imo();

    if (fInherit)
        return get_inherited_style();

    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoContentObj::get_style_imo()
{
    if (m_styleId != k_no_imoid)
        return dynamic_cast<ImoStyle*>(get_the_document()->get_pointer_to_imo(m_styleId));
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoContentObj::get_inherited_style()
{
    string name;
    switch(get_obj_type())
    {
        case k_imo_heading:
        {
            stringstream style;
            style << "Heading-" << static_cast<ImoHeading*>(this)->get_level();
            name = style.str();
            break;
        }
        case k_imo_para:
            name = "Paragraph";
            break;
        case k_imo_table:
            name = "Table";
            break;

        default:
            ImoContentObj* pParent = dynamic_cast<ImoContentObj*>( get_block_level_parent() );
            if (pParent)
                return pParent->get_style();
            else if (this->is_document())
                return (static_cast<ImoDocument*>(this))->get_default_style();
            else
                return nullptr;
    }

    ImoDocument* pImoDoc = this->get_document();
    if (pImoDoc)
        return pImoDoc->find_style(name);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoContentObj::copy_style_as(const std::string& name)
{
    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name(name);
    pStyle->set_parent_style( get_style_imo() );
    return pStyle;
}

//---------------------------------------------------------------------------------------
void ImoContentObj::set_style(ImoStyle* pStyle)
{
    if (pStyle)
        m_styleId = pStyle->get_id();
    else
        m_styleId = k_no_imoid;
}

//---------------------------------------------------------------------------------------
void ImoContentObj::add_event_handler(int eventType, EventHandler* pHandler)
{
    Document* pDoc = get_the_document();
    pDoc->add_event_handler(Observable::k_imo, get_id(), eventType, pHandler);
}

//---------------------------------------------------------------------------------------
void ImoContentObj::add_event_handler(int eventType, void* pThis,
                                      void (*pt2Func)(void* pObj, SpEventInfo event) )
{
    Document* pDoc = get_the_document();
    pDoc->add_event_handler(Observable::k_imo, get_id(), eventType, pThis, pt2Func);
}

//---------------------------------------------------------------------------------------
void ImoContentObj::add_event_handler(int eventType,
                                      void (*pt2Func)(SpEventInfo event) )
{
    Document* pDoc = get_the_document();
    pDoc->add_event_handler(Observable::k_imo, get_id(), eventType, pt2Func);
}

//---------------------------------------------------------------------------------------
EventNotifier* ImoContentObj::get_event_notifier()
{
    return get_the_document();
}

//---------------------------------------------------------------------------------------
list<TIntAttribute> ImoContentObj::get_supported_attributes()
{
    list<TIntAttribute> supported;
    supported.push_back(k_attr_style);
    supported.push_back(k_attr_xpos);
    supported.push_back(k_attr_ypos);
    supported.push_back(k_attr_visible);
    return supported;
}

//---------------------------------------------------------------------------------------
void ImoContentObj::set_bool_attribute(TIntAttribute attrib, bool value)
{
    switch(attrib)
    {
        case k_attr_visible:
            m_fVisible = value;
            break;

        default:
            set_attribute<bool>(attrib, value);
    }
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
bool ImoContentObj::get_bool_attribute(TIntAttribute attrib)
{
    switch(attrib)
    {
        case k_attr_visible:
            return m_fVisible;
        default:
            return get_attribute_value<bool>(attrib);
    }
}

//---------------------------------------------------------------------------------------
void ImoContentObj::set_double_attribute(TIntAttribute attrib, double value)
{
    switch(attrib)
    {
        case k_attr_xpos:
            m_txUserLocation = Tenths(value);
            break;

        case k_attr_ypos:
            m_tyUserLocation = Tenths(value);
            break;

        default:
            set_attribute<double>(attrib, value);
    }
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
double ImoContentObj::get_double_attribute(TIntAttribute attrib)
{
    switch(attrib)
    {
        case k_attr_xpos:
            return m_txUserLocation;
        case k_attr_ypos:
            return m_tyUserLocation;
        default:
            return get_attribute_value<double>(attrib);
    }
}

//---------------------------------------------------------------------------------------
void ImoContentObj::set_string_attribute(TIntAttribute attrib, const string& value)
{
    switch(attrib)
    {
        case k_attr_style:
            //TODO
            break;

        default:
            set_attribute<string>(attrib, value);
    }
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
string ImoContentObj::get_string_attribute(TIntAttribute attrib)
{
    switch(attrib)
    {
        case k_attr_style:
        {
            ImoStyle* pStyle = get_style_imo();
            return (pStyle ? pStyle->get_name() : "");
        }
        default:
            return get_attribute_value<string>(attrib);
    }
}


//=======================================================================================
// ImoAnonymousBlock implementation
//=======================================================================================
void ImoAnonymousBlock::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoAnonymousBlock>* vAb = nullptr;
    Visitor<ImoObj>* vObj = nullptr;

    vAb = dynamic_cast<Visitor<ImoAnonymousBlock>*>(&v);
    if (vAb)
        vAb->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    visit_children(v);

    if (vAb)
        vAb->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}


//=======================================================================================
// ImoDocument implementation
//=======================================================================================
ImoDocument::ImoDocument(const std::string& version)
    : ImoBlocksContainer(k_imo_document)
    , m_scale(1.0f)
    , m_version(version)
    , m_language("en")
{
}

//---------------------------------------------------------------------------------------
ImoDocument::~ImoDocument()
{
    std::list<ImoStyle*>::iterator it;
    for (it = m_privateStyles.begin(); it != m_privateStyles.end(); ++it)
        delete *it;
    m_privateStyles.clear();
}

//---------------------------------------------------------------------------------------
ImoDocument& ImoDocument::clone(const ImoDocument& a)
{
    m_scale = a.m_scale;
    m_version = a.m_version;
    m_language = a.m_language;

    std::list<ImoStyle*>::const_iterator it;
    for (it = a.m_privateStyles.begin(); it != a.m_privateStyles.end(); ++it)
        m_privateStyles.push_back( static_cast<ImoStyle*>( ImFactory::clone(*it) ));

    return *this;
}

//---------------------------------------------------------------------------------------
void ImoDocument::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoDocument>* vThis = nullptr;
    Visitor<ImoObj>* vObj = nullptr;

    vThis = dynamic_cast<Visitor<ImoDocument>*>(&v);
    if (vThis)
        vThis->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    //visit_children
    std::list<ImoStyle*>::const_iterator it;
    for (it = m_privateStyles.begin(); it != m_privateStyles.end(); ++it)
        (*it)->accept_visitor(v);
    visit_children(v);

    if (vThis)
        vThis->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}

//---------------------------------------------------------------------------------------
bool ImoDocument::has_visitable_children()
{
    return has_children() || m_privateStyles.size() > 0;
}

//---------------------------------------------------------------------------------------
void ImoDocument::initialize_object()
{
    add_default_styles();
    append_child( ImFactory::inject(k_imo_page_info, m_pDocModel) );
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_default_styles()
{
    //AWARE: default styles are automatically added in ImoStyles constructor
    append_child( ImFactory::inject(k_imo_styles, m_pDocModel) );
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_page_info(ImoPageInfo* pPI)
{
    ImoPageInfo* pInfo = get_page_info();
    if (pInfo)
    {
        remove_child_imo(pInfo);
        delete pInfo;
    }

    append_child_imo(pPI);
}

//---------------------------------------------------------------------------------------
ImoPageInfo* ImoDocument::get_page_info()
{
    return static_cast<ImoPageInfo*>( get_child_of_type(k_imo_page_info) );
}

//---------------------------------------------------------------------------------------
ImoStyles* ImoDocument::get_styles()
{
    return static_cast<ImoStyles*>( get_child_of_type(k_imo_styles) );
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_style(ImoStyle* pStyle)
{
    get_styles()->add_style(pStyle);
}

//---------------------------------------------------------------------------------------
void ImoDocument::add_private_style(ImoStyle* pStyle)
{
    m_privateStyles.push_back(pStyle);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::find_style(const std::string& name)
{
    return get_styles()->find_style(name);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::get_default_style()
{
    return get_styles()->get_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::get_style_or_default(const std::string& name)
{
    return get_styles()->get_style_or_default(name);
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::create_style(const string& name, const string& parent)
{
    ImoStyle* pParent = find_style(parent);
    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name(name);
    pStyle->set_parent_style(pParent);
    add_style(pStyle);
    return pStyle;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoDocument::create_private_style(const string& parent)
{
    ImoStyle* pParent = get_style_or_default(parent);
    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("");
    pStyle->set_parent_style(pParent);
    add_private_style(pStyle);
    return pStyle;
}

//---------------------------------------------------------------------------------------
void ImoDocument::insert_block_level_obj(ImoBlockLevelObj* pAt,
                                         ImoBlockLevelObj* pImoNew)
{
    if (pAt)
        insert(pAt, pImoNew);
    else
    {
        ImoObj* pNode = get_child_of_type(k_imo_content);
        pNode->append_child(pImoNew);
    }

    pImoNew->set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ImoDocument::delete_block_level_obj(ImoBlockLevelObj* pAt)
{

    erase(pAt);
    delete pAt;

    set_dirty(true);
}


//=======================================================================================
// ImoArpeggio implementation
//=======================================================================================
void ImoArpeggio::reorganize_after_object_deletion()
{
    //Nothing to do
}


//=======================================================================================
// ImoDynamic implementation
//=======================================================================================
void ImoDynamic::add_param(ImoParamInfo* pParam)
{
    ImoParameters* params = get_parameters();
    if (params == nullptr)
    {
        params = static_cast<ImoParameters* >(ImFactory::inject(k_imo_parameters, m_pDocModel));
        append_child_imo(params);
    }
    params->append_child_imo(pParam);
}

//---------------------------------------------------------------------------------------
std::list<ImoParamInfo*> ImoDynamic::get_params()
{
    std::list<ImoParamInfo*> parmlist;
    ImoParameters* params = get_parameters();
    ImoObj::children_iterator it;
    for (it = params->begin(); it != params->end(); ++it)
    {
        parmlist.push_back( static_cast<ImoParamInfo*>(*it) );
    }

    return parmlist;
}

//---------------------------------------------------------------------------------------
ImoParameters* ImoDynamic::get_parameters()
{
    return static_cast<ImoParameters*>( get_child_of_type(k_imo_parameters) );
}


//=======================================================================================
// ImoFingering implementation
//=======================================================================================
FingerData& ImoFingering::add_fingering(const string& value)
{
    m_fingerings.emplace_back(value);
    return m_fingerings.back();
}


//=======================================================================================
// ImoGraceRelObj implementation
//=======================================================================================
void ImoGraceRelObj::reorganize_after_object_deletion()
{
    //A grace notes relobj involves the grace notes and their principal note.
    //Thehe grace relationship must be deleted when the principal note
    //is deleted (it should also delete the grace notes) or when the last grace note
    //is deleted. In both cases, the grace relationship is automatically deleted when
    //only one note remains in the relationship. So nothing to do here.
}


//=======================================================================================
// ImoHeading implementation
//=======================================================================================
void ImoHeading::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoHeading>* vHeading = nullptr;
    Visitor<ImoObj>* vObj = nullptr;

    vHeading = dynamic_cast<Visitor<ImoHeading>*>(&v);
    if (vHeading)
        vHeading->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    visit_children(v);

    if (vHeading)
        vHeading->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}



//=======================================================================================
// ImoInstrument implementation
//=======================================================================================
ImoInstrument::ImoInstrument()
    : ImoContainerObj(k_imo_instrument)
    , m_name()
    , m_abbrev()
    , m_partId("")
    , m_barlineLayout(EBarlineLayout::k_isolated)
    , m_measuresNumbering(EMeasuresNumbering::k_undefined)
    , m_pMeasures(nullptr)
    , m_pLastMeasureInfo(nullptr)
{
}

//---------------------------------------------------------------------------------------
ImoInstrument::~ImoInstrument()
{
    std::list<ImoStaffInfo*>::iterator it;
    for (it = m_staves.begin(); it != m_staves.end(); ++it)
        delete *it;
    m_staves.clear();

    delete m_pMeasures;
    delete m_pLastMeasureInfo;
}

//---------------------------------------------------------------------------------------
ImoInstrument& ImoInstrument::clone(const ImoInstrument& a)
{
    m_name = a.m_name;
    m_abbrev = a.m_abbrev;
    m_nameStyle = a.m_nameStyle;
    m_abbrevStyle = a.m_abbrevStyle;
    m_partId = a.m_partId;

    std::list<ImoStaffInfo*>::const_iterator it;
    for (it = a.m_staves.begin(); it != a.m_staves.end(); ++it)
        m_staves.push_back( static_cast<ImoStaffInfo*>( ImFactory::clone(*it) ));

    m_barlineLayout = a.m_barlineLayout;
    m_measuresNumbering = a.m_measuresNumbering;

    m_pMeasures = nullptr;      //will be instantiated when the table is built, in ModelBuilder
    if (a.m_pLastMeasureInfo)
        m_pLastMeasureInfo = LOMSE_NEW TypeMeasureInfo(*a.m_pLastMeasureInfo);
    else
        m_pLastMeasureInfo = nullptr;

    return *this;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::initialize_object()
{
    add_staff();
}

//---------------------------------------------------------------------------------------
void ImoInstrument::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoInstrument>* vThis = nullptr;
    Visitor<ImoObj>* vObj = nullptr;

    vThis = dynamic_cast<Visitor<ImoInstrument>*>(&v);
    if (vThis)
        vThis->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    //visit_children
    std::list<ImoStaffInfo*>::iterator it;
    for (it = m_staves.begin(); it != m_staves.end(); ++it)
        (*it)->accept_visitor(v);
    visit_children(v);

    if (vThis)
        vThis->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}

//---------------------------------------------------------------------------------------
bool ImoInstrument::has_visitable_children()
{
    return has_children() || m_staves.size() > 0;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_measures_table(ImMeasuresTable* pTable)
{
    delete m_pMeasures;
    m_pMeasures = pTable;
}

//---------------------------------------------------------------------------------------
ImoStaffInfo* ImoInstrument::add_staff()
{
    ImoStaffInfo* pStaff = static_cast<ImoStaffInfo*>(
                                ImFactory::inject(k_imo_staff_info, m_pDocModel) );
    m_staves.push_back(pStaff);
    return pStaff;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::replace_staff_info(ImoStaffInfo* pInfo)
{
    int iStaff = pInfo->get_staff_number();
    std::list<ImoStaffInfo*>::iterator it = m_staves.begin();
    for (; it != m_staves.end() && iStaff > 0; ++it, --iStaff);

    if (it != m_staves.end())
    {
        ImoStaffInfo* pOld = *it;
        it = m_staves.erase(it);
        delete pOld;
        m_staves.insert(it, static_cast<ImoStaffInfo*>(
                                ImFactory::clone(pInfo)) );

        //pInfo is going to be deleted. Remove id to avoid removing it from IdAssigner
        //as this id is reused by cloned ImoStaffInfo
        pInfo->set_id(k_no_imoid);
    }
    delete pInfo;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_staff_margin(int iStaff, LUnits distance)
{
    std::list<ImoStaffInfo*>::iterator it = m_staves.begin();
    for (; it != m_staves.end() && iStaff > 0; ++it, --iStaff);

    if (it != m_staves.end())
    {
        ImoStaffInfo* pInfo = *it;
        pInfo->set_staff_margin(distance);
    }
}

//---------------------------------------------------------------------------------------
void ImoInstrument::mark_staff_margin_as_imported(int iStaff)
{
    ImoStaffInfo* pInfo = get_staff(iStaff);
    pInfo->mark_staff_margin_as_imported();
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_name(TypeTextInfo& text)
{
    m_name = text;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_abbrev(TypeTextInfo& text)
{
    m_abbrev = text;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_name(const string& value)
{
    m_name.text = value;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_abbrev(const string& value)
{
    m_abbrev.text = value;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_name_style(ImoStyle* style)
{
    if (style)
        m_nameStyle = style->get_id();
    else
        m_nameStyle = k_no_imoid;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::set_abbrev_style(ImoStyle* style)
{
    if (style)
        m_abbrevStyle = style->get_id();
    else
        m_abbrevStyle = k_no_imoid;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoInstrument::get_style_imo(ImoId id)
{
    if (id != k_no_imoid)
        return dynamic_cast<ImoStyle*>(get_the_document()->get_pointer_to_imo(id));
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoMusicData* ImoInstrument::get_musicdata()
{
    return static_cast<ImoMusicData*>( get_child_of_type(k_imo_music_data) );
}

//---------------------------------------------------------------------------------------
ImoStaffInfo* ImoInstrument::get_staff(int iStaff)
{
    std::list<ImoStaffInfo*>::iterator it = m_staves.begin();
    for (; it != m_staves.end() && iStaff > 0; ++it, --iStaff);
    return (it != m_staves.end() ? *it : nullptr);
}

//---------------------------------------------------------------------------------------
LUnits ImoInstrument::get_line_spacing_for_staff(int iStaff)
{
    return get_staff(iStaff)->get_line_spacing();
}

//---------------------------------------------------------------------------------------
LUnits ImoInstrument::get_line_thickness_for_staff(int iStaff)
{
    return get_staff(iStaff)->get_line_thickness();
}

//---------------------------------------------------------------------------------------
double ImoInstrument::get_notation_scaling_for_staff(int iStaff)
{
    return get_staff(iStaff)->get_notation_scaling();
}

//---------------------------------------------------------------------------------------
LUnits ImoInstrument::tenths_to_logical(Tenths value, int iStaff)
{
	return (value * get_line_spacing_for_staff(iStaff)) / 10.0f;
}

//---------------------------------------------------------------------------------------
ImoScore* ImoInstrument::get_score()
{
    return static_cast<ImoScore*>(get_ancestor_of_type(k_imo_score));
}

//---------------------------------------------------------------------------------------
void ImoInstrument::reserve_space_for_lyrics(int iStaff, LUnits space)
{
    ImoStaffInfo* pInfo = get_staff(iStaff);
    pInfo->add_space_for_lyrics(space);
}


// ImoSounds interface
LOMSE_IMPLEMENT_IMOSOUNDS_INTERFACE(ImoInstrument);


//---------------------------------------------------------------------------------------
// Instrument API
//---------------------------------------------------------------------------------------
ImoBarline* ImoInstrument::add_barline(int type, bool fVisible)
{
    ImoMusicData* pMD = get_musicdata();
    ImoBarline* pImo = static_cast<ImoBarline*>(
                                ImFactory::inject(k_imo_barline, m_pDocModel) );
    pImo->set_type(type);
    pImo->set_visible(fVisible);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoClef* ImoInstrument::add_clef(int type, int nStaff, bool fVisible)
{
    ImoMusicData* pMD = get_musicdata();
    ImoClef* pImo = static_cast<ImoClef*>( ImFactory::inject(k_imo_clef, m_pDocModel) );
    pImo->set_clef_type(type);
    pImo->set_staff(nStaff - 1);
    pImo->set_visible(fVisible);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoKeySignature* ImoInstrument::add_key_signature(int type, bool fVisible)
{
    ImoMusicData* pMD = get_musicdata();
    ImoKeySignature* pImo = static_cast<ImoKeySignature*>(
                                ImFactory::inject(k_imo_key_signature, m_pDocModel) );
    pImo->set_key_type(type);
    pImo->set_visible(fVisible);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoDirection* ImoInstrument::add_spacer(Tenths space)
{
    ImoMusicData* pMD = get_musicdata();
    ImoDirection* pImo =
            static_cast<ImoDirection*>( ImFactory::inject(k_imo_direction, m_pDocModel) );
    pImo->set_width(space);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoTimeSignature* ImoInstrument::add_time_signature(int top, int bottom,
                                                    bool fVisible)
{
    ImoMusicData* pMD = get_musicdata();
    ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
                                ImFactory::inject(k_imo_time_signature, m_pDocModel) );
    pImo->set_top_number(top);
    pImo->set_bottom_number(bottom);
    pImo->set_visible(fVisible);
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoObj* ImoInstrument::add_object(const string& ldpsource)
{
    ImoObj* pImo = m_pDocModel->get_owner_document()->create_object_from_ldp(ldpsource);
    ImoMusicData* pMD = get_musicdata();
    pMD->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::add_staff_objects(const string& ldpsource)
{
    ImoMusicData* pMD = get_musicdata();
    m_pDocModel->get_owner_document()->add_staff_objects(ldpsource, pMD);
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoInstrument::insert_staffobj_at(ImoStaffObj* pAt, ImoStaffObj* pImo)
{
    if (pAt)
        insert_staffobj(pAt, pImo);
    else
    {
        ImoMusicData* pMD = get_musicdata();
        pMD->append_child_imo(pImo);
    }
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoInstrument::insert_staffobj_at(ImoStaffObj* pAt, const string& ldpsource,
                                               ostream& reporter)
{
    ImoStaffObj* pImo =
        static_cast<ImoStaffObj*>( m_pDocModel->get_owner_document()->create_object_from_ldp(ldpsource, reporter) );

    if (pImo)
        return insert_staffobj_at(pAt, pImo);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoInstrument::delete_staffobj(ImoStaffObj* pSO)
{
    //high level method. Can not be used if ColStaffObjs not build
    ColStaffObjs* pColStaffObjs = get_score()->get_staffobjs_table();
    if (pColStaffObjs)
    {
        //General houskeeping:

            //recursively delete all attachments
            //DONE: this is automatic: deleting ImoObj recursively deletes its attachments

            //remove from all relations. Could imply deleting the relation.
            //TODO

            //if duration > 0 && ! is-noterest-in-chord shift back all staffobjs in that
            //instr/staff. When a barline is found determine time implied by each staff
            //and choose greater one. If barline time doesn't change stop shifting times.
            //OPTIMIZATION: barlines should store time implied by each staff.
            if (is_greater_time(pSO->get_duration(), 0.0))
            {
    //            if (pSO->is_noterest() && )
            }

        //remove from ColStaffObjs
        pColStaffObjs->delete_entry_for(pSO);
    }

    //remove from ImoTree
    ImoMusicData* pMusicData = get_musicdata();
    pMusicData->remove_child(pSO);
    delete pSO;

    set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ImoInstrument::insert_staffobj(ImoStaffObj* pPos, ImoStaffObj* pImo)
{
    //insert pImo before pPos

    TreeNode<ImoObj>::iterator it(pPos);
    ImoMusicData* pMD = get_musicdata();
    pMD->insert(it, pImo);
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
void ImoInstrument::insert_staffobj_after(ImoStaffObj* pPos, ImoStaffObj* pImo)
{
    //insert pImo after pPos

    TreeNode<ImoObj>::iterator it( pPos->get_next_sibling() );
    ImoMusicData* pMD = get_musicdata();
    if (*it)
        pMD->insert(it, pImo);
    else
        pMD->append_child(pImo);
    set_dirty(true);
}

//---------------------------------------------------------------------------------------
list<ImoStaffObj*> ImoInstrument::insert_staff_objects_at(ImoStaffObj* pAt,
                                            const string& ldpsource, ostream& reporter)
{
    string data = "(musicData " + ldpsource + ")";
    ImoMusicData* pObjects =
        static_cast<ImoMusicData*>( m_pDocModel->get_owner_document()->create_object_from_ldp(data, reporter) );

    if (pObjects)
        return insert_staff_objects_at(pAt, pObjects);

    //error. return empty list
    list<ImoStaffObj*> objects;
    return objects;
}

//---------------------------------------------------------------------------------------
list<ImoStaffObj*> ImoInstrument::insert_staff_objects_at(ImoStaffObj* pAt,
                                                          ImoMusicData* pObjects)
{
    //move all objects in pObjects to this instrument MusicData, before object pAt

    ImoMusicData* pMD = get_musicdata();
    TreeNode<ImoObj>::iterator itPos(pAt);

    list<ImoStaffObj*> objects;
    ImoObj::children_iterator it = pObjects->begin();
    while (it != pObjects->end())
    {
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>(*it);
        pObjects->remove_child(pImo);
        if (pAt)
            pMD->insert(itPos, pImo);
        else
            pMD->append_child(pImo);
        objects.push_back(pImo);
        it = pObjects->begin();
    }
    delete pObjects;
    set_dirty(true);
    return objects;
}


//=======================================================================================
// ImoInstrGroup implementation
//=======================================================================================
ImoInstrGroup::ImoInstrGroup()
    : ImoSimpleObj(k_imo_instr_group)
    , m_joinBarlines(EJoinBarlines::k_joined_barlines)
    , m_symbol(k_group_symbol_none)
{
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_name(TypeTextInfo& text)
{
    m_name = text;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_abbrev(TypeTextInfo& text)
{
    m_abbrev = text;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_name(const string& value)
{
    m_name.text = value;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_abbrev(const string& value)
{
    m_abbrev.text = value;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_name_style(ImoStyle* style)
{
    if (style)
        m_nameStyle = style->get_id();
    else
        m_nameStyle = k_no_imoid;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_abbrev_style(ImoStyle* style)
{
    if (style)
        m_abbrevStyle = style->get_id();
    else
        m_abbrevStyle = k_no_imoid;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoInstrGroup::get_style_imo(ImoId id)
{
    if (id != k_no_imoid)
        return dynamic_cast<ImoStyle*>(get_the_document()->get_pointer_to_imo(id));
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoScore* ImoInstrGroup::get_score()
{
    return static_cast<ImoScore*>(get_ancestor_of_type(k_imo_score));
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoInstrGroup::get_instrument(int iInstr)    //iInstr = 0..n-1
{
    if (iInstr < m_numInstrs && m_iFirstInstr >= 0)
        return get_score()->get_instrument(iInstr + m_iFirstInstr);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoInstrGroup::get_first_instrument()
{
    return get_score()->get_instrument(m_iFirstInstr);
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoInstrGroup::get_last_instrument()
{
    return get_score()->get_instrument(m_iFirstInstr + m_numInstrs - 1);
}

//---------------------------------------------------------------------------------------
bool ImoInstrGroup::contains_instrument(ImoInstrument* pInstr)
{
    int index = get_score()->get_instr_number_for(pInstr);
    return (index >= m_iFirstInstr) && (index < m_iFirstInstr + m_numInstrs);
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::set_range(int iFirstInstr, int iLastInstr)
{
    m_iFirstInstr = iFirstInstr;
    m_numInstrs = iLastInstr - iFirstInstr + 1;
}

//---------------------------------------------------------------------------------------
void ImoInstrGroup::add_instrument(int iInstr)
{
    if (m_iFirstInstr == -1)
    {
        m_iFirstInstr = iInstr;
        m_numInstrs = 1;
    }
    else
        ++m_numInstrs;
}


//=======================================================================================
// ImoKeySignature implementation
//=======================================================================================
int ImoKeySignature::get_key_type() const
{
    if (m_fStandard)
        return int( KeyUtilities::key_components_to_key_type(m_fifths, EKeyMode(m_keyMode)) );
    else
        return int(k_key_non_standard);
}

//---------------------------------------------------------------------------------------
void ImoKeySignature::set_key_type(int type)
{
    m_fStandard = true;
    m_keyMode = KeyUtilities::get_key_mode(EKeySignature(type));
    m_fifths = KeyUtilities::key_signature_to_num_fifths(EKeySignature(type));
}

//---------------------------------------------------------------------------------------
void ImoKeySignature::set_standard_key(int fifths, bool fMajor)
{
    m_fStandard = true;
    m_keyMode = (fMajor ? k_key_mode_major : k_key_mode_minor);
    m_fifths = fifths;
}

//---------------------------------------------------------------------------------------
void ImoKeySignature::set_non_standard_key(const KeyAccidental (&acc)[7])
{
    m_fStandard = false;
    m_keyMode = k_key_mode_none;
    m_fifths = 0;

    for (size_t i=0; i < 7; ++i)
    	m_accidentals[i] = acc[i];
}

//---------------------------------------------------------------------------------------
bool ImoKeySignature::has_accidentals() const
{
    if (m_fStandard)
        return m_fifths != 0;
    else
    {
        for (size_t i=0; i < 7; ++i)
        {
            if (m_accidentals[i].accidental != k_no_accidentals)
                return true;
        }
        return false;
    }
}

//---------------------------------------------------------------------------------------
void ImoKeySignature::set_staff(int staff)
{
    m_fForAllStaves = false;
    ImoStaffObj::set_staff(staff);
}

//---------------------------------------------------------------------------------------
void ImoKeySignature::set_staff_common_for_all(int staff)
{
    ImoStaffObj::set_staff(staff);
}

//---------------------------------------------------------------------------------------
void ImoKeySignature::transpose(const int semitones)
{
                       //fifths  -7 -6 -5 -4  -3 -2 -1  0  1  2  3   4  5  6  7
                       //Key:     C- G- D- A-  E- B- F  C  G  D  A   E  B  F+ C+
    static int fifthsToIndex[] = {1, 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6, 1, 8, 3};

    if (!m_fStandard)
        return;         //TODO? transpose is currently only supported for standard keys

    int index = fifthsToIndex[m_fifths+7];
    int newIndex = index + semitones;
    //normalize 0..11
    while (newIndex < 0)
        newIndex += 11;
    while (newIndex > 11)
        newIndex -= 11;

                       //index    0   1   2   3   4   5   6   7   8   9  10  11
                       //newKey:  b-  b   c   c+  d   e-  e   f   f+  g  a-  a
                       //             c-      d-                  g-
    static int indexToFifths[] = {-2, 5,  0,  7,  2, -3,  4, -1,  6,  1, -4, 3};

    m_fifths = indexToFifths[newIndex];
    if (semitones < 0)
    {
        if (newIndex == 1)      //key can be B or C flat
            m_fifths = -7;          //use C flat
        else if (newIndex == 3) //key can be C sharp or D flat
            m_fifths = -5;          //key D flat
        else if (newIndex == 8) //key can be F sharp or G flat
            m_fifths = -6;          //key G flat
    }
}

//=======================================================================================
// ImoLink implementation
//=======================================================================================
string ImoLink::get_language()
{
    if (!m_language.empty())
        return m_language;
    else if (m_pDocModel)
        return m_pDocModel->get_language();
    else
        return "en";
}


//=======================================================================================
// ImoList implementation
//=======================================================================================
ImoList::ImoList()
    : ImoBlocksContainer(k_imo_list)
    , m_listType(k_itemized)
{
}

//---------------------------------------------------------------------------------------
void ImoList::initialize_object()
{
    create_content_container();
}

//---------------------------------------------------------------------------------------
ImoListItem* ImoList::add_listitem(ImoStyle* pStyle)
{
    ImoListItem* pImo = static_cast<ImoListItem*>(
                            ImFactory::inject(k_imo_listitem, m_pDocModel) );
    pImo->set_style(pStyle);
    append_content_item(pImo);
    return pImo;
}


//=======================================================================================
// ImoListItem implementation
//=======================================================================================
ImoListItem::ImoListItem()
    : ImoBlocksContainer(k_imo_listitem)
{
}

//---------------------------------------------------------------------------------------
void ImoListItem::initialize_object()
{
    create_content_container();
}


//=======================================================================================
// ImoLyric implementation
//=======================================================================================
ImoLyricsTextInfo* ImoLyric::get_text_item(int iText)
{
    if (iText >= m_numTextItems)
        return nullptr;

    for (int i=0; i < m_numTextItems; ++i)
    {
        ImoObj* pChild = get_child(i);
        if (pChild->is_lyrics_text_info() && i==iText)
            return static_cast<ImoLyricsTextInfo*>(pChild);
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoLyric::add_text_item(ImoLyricsTextInfo* pText)
{
    append_child_imo(pText);
    m_numTextItems++;
}


//=======================================================================================
// ImoLyricsTextInfo implementation
//=======================================================================================
void ImoLyricsTextInfo::set_syllable_style(ImoStyle* pStyle)
{
    if (pStyle)
        m_styleId = pStyle->get_id();
    else
        m_styleId = k_no_imoid;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoLyricsTextInfo::get_syllable_style()
{
    if (m_styleId != k_no_imoid)
        return dynamic_cast<ImoStyle*>(get_the_document()->get_pointer_to_imo(m_styleId));
    else
        return nullptr;
}


//=======================================================================================
// ImoMusicData implementation
//=======================================================================================
ImoInstrument* ImoMusicData::get_instrument()
{
    return dynamic_cast<ImoInstrument*>( get_parent() );
}


//=======================================================================================
// ImoMultiColumn implementation
//=======================================================================================
ImoMultiColumn::ImoMultiColumn()
    : ImoBlocksContainer(k_imo_multicolumn)
{
}

//---------------------------------------------------------------------------------------
void ImoMultiColumn::initialize_object()
{
    create_content_container();
}

//---------------------------------------------------------------------------------------
void ImoMultiColumn::create_columns(int numCols)
{
    m_widths.reserve(numCols);
    float width = 100.0f / float(numCols);
    for (int i=numCols; i > 0; i--)
    {
        append_content_item(
            static_cast<ImoContentObj*>( ImFactory::inject(k_imo_content, m_pDocModel) ));
        m_widths.push_back(width);
    }
}

//---------------------------------------------------------------------------------------
void ImoMultiColumn::set_column_width(int iCol, float percentage)
{
    m_widths[iCol] = percentage;
}

//---------------------------------------------------------------------------------------
float ImoMultiColumn::get_column_width(int iCol)
{
    return m_widths[iCol];
}


//=======================================================================================
// ImoParagraph implementation
//=======================================================================================
void ImoParagraph::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoParagraph>* vPara = nullptr;
    Visitor<ImoObj>* vObj = nullptr;

    vPara = dynamic_cast<Visitor<ImoParagraph>*>(&v);
    if (vPara)
        vPara->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    visit_children(v);

    if (vPara)
        vPara->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}


//=======================================================================================
// ImoParamInfo implementation
//=======================================================================================
bool ImoParamInfo::get_value_as_int(int* pNumber)
{
    //http://www.codeguru.com/forum/showthread.php?t=231054
    std::istringstream iss(m_value);
    if ((iss >> std::dec >> *pNumber).fail())
        return false;   //error
    else
        return true;    //ok
}

//=======================================================================================
// ImoScore implementation
//=======================================================================================

//tables with default values for options
typedef struct
{
    string  sOptName;
    bool    fBoolValue;
}
BoolOption;

typedef struct
{
    string  sOptName;
    string  sStringValue;
}
StringOption;

typedef struct
{
    string  sOptName;
    float   rFloatValue;
}
FloatOption;

typedef struct
{
    string  sOptName;
    long    nLongValue;
}
LongOption;

//---------------------------------------------------------------------------------------
static const BoolOption m_BoolOptions[] =
{
    {"Score.FillPageWithEmptyStaves", false },
    {"Staff.DrawLeftBarline", true },
    {"StaffLines.Hide", false },
};

//static StringOption m_StringOptions[] = {};

//---------------------------------------------------------------------------------------
static const FloatOption m_FloatOptions[] =
{
    // Spacing is proportional to note/rest duration.
    // As the duration of a quarter note is 64 (time units), this will be mapped
    // to 35 tenths. This gives a conversion factor of 35/64 = 0.547
    {"Render.SpacingFactor", 0.547f },

    {"Render.SpacingFopt", 1.4f },

    //All notes with a duration lower than Dmin will be spaced by a fixed small amount.
    //Value 0.0 implies that Dmin will be determined by the spacing algorithm
    {"Render.SpacingDmin", 0.0f }, //16.0f },
};

//---------------------------------------------------------------------------------------
static const LongOption m_LongOptions[] =
{
    {"Render.SpacingMethod", long(k_spacing_proportional) },
    {"Render.SpacingOptions", k_render_opt_breaker_simple },
    {"Render.SpacingValue", 35L },      //15 tenths (1.5 lines) [add 20 to desired value]
    {"Score.JustifyLastSystem", k_justify_never },
    {"Staff.UpperLegerLines.Displacement", 0L },
    {"StaffLines.Truncate", k_truncate_barline_final },
};

//---------------------------------------------------------------------------------------
ImoScore::ImoScore()
    : ImoBlockLevelObj(k_imo_score)
    , m_scaling(LOMSE_STAFF_LINE_SPACING / 10.0f)     //default staff: 7.2mm = 40 tenths
    , m_systemInfoFirst()
    , m_systemInfoOther()
{
}

//---------------------------------------------------------------------------------------
ImoScore::~ImoScore()
{
    delete m_pColStaffObjs;
    delete_text_styles();
    delete m_pMidiTable;
}

//---------------------------------------------------------------------------------------
ImoScore& ImoScore::clone(const ImoScore& a)
{
    m_version = a.m_version;
    m_sourceFormat = a.m_sourceFormat;
    m_accidentalsModel = a.m_accidentalsModel;
    m_pColStaffObjs = nullptr;
    m_pMidiTable = nullptr;
    m_scaling = a.m_scaling;
    m_systemInfoFirst = a.m_systemInfoFirst;
    m_systemInfoOther = a.m_systemInfoOther;

    std::map<std::string, ImoStyle*>::const_iterator it;
    for (it = a.m_nameToStyle.begin(); it != a.m_nameToStyle.end(); ++it)
        m_nameToStyle[it->first] = static_cast<ImoStyle*>(
                                        ImFactory::clone(it->second) );

    m_numLyricFonts = a.m_numLyricFonts;

    std::map<int, std::string>::const_iterator it2;
    for (it2 = a.m_lyricLanguages.begin(); it2 != a.m_lyricLanguages.end(); ++it2)
        m_lyricLanguages[it2->first] = string(it2->second);

    m_staffDistance = a.m_staffDistance;
    m_modified = a.m_modified;

    return *this;
}

//---------------------------------------------------------------------------------------
void ImoScore::initialize_object()
{
    set_edit_terminal(true);
    append_child_imo( ImFactory::inject(k_imo_options, m_pDocModel) );
    append_child_imo( ImFactory::inject(k_imo_score_titles, m_pDocModel) );
    append_child_imo( ImFactory::inject(k_imo_instruments, m_pDocModel) );
    set_defaults_for_system_info();
    set_defaults_for_options();
}

//---------------------------------------------------------------------------------------
void ImoScore::delete_text_styles()
{
    map<std::string, ImoStyle*>::const_iterator it;
    for (it = m_nameToStyle.begin(); it != m_nameToStyle.end(); ++it)
        delete it->second;

    m_nameToStyle.clear();
}

//---------------------------------------------------------------------------------------
string ImoScore::get_version_string()
{
    stringstream v;
    v << get_version_major() << "." << get_version_minor();
    return v.str();
}

//---------------------------------------------------------------------------------------
void ImoScore::set_defaults_for_system_info()
{
    m_systemInfoFirst.set_first(true);
    m_systemInfoFirst.set_default_top_system_distance(1000.0f);     //half system distance
    m_systemInfoFirst.set_default_system_distance(2000.0f);         //2 cm

    m_systemInfoOther.set_first(false);
    m_systemInfoOther.set_default_top_system_distance(1500.0f);     //1.5 cm
    m_systemInfoOther.set_default_system_distance(2000.0f);         //2 cm
}

//---------------------------------------------------------------------------------------
bool ImoScore::has_default_values(ImoSystemInfo* pInfo)
{
    if (pInfo->is_first())
    {
        return pInfo->get_left_margin() == 0.0f
            && pInfo->get_right_margin() == 0.0f
            && pInfo->get_top_system_distance() == 1000.0f
            && pInfo->get_system_distance() == 2000.0f;
    }
    else
    {
        return pInfo->get_left_margin() == 0.0f
            && pInfo->get_right_margin() == 0.0f
            && pInfo->get_top_system_distance() == 1500.0f
            && pInfo->get_system_distance() == 2000.0f;
    }
}

//---------------------------------------------------------------------------------------
bool ImoScore::has_default_value(ImoOptionInfo* pOpt)
{
    string name = pOpt->get_name();
    if (pOpt->is_bool_option())
    {
        for (unsigned i=0; i < sizeof(m_BoolOptions)/sizeof(BoolOption); i++)
        {
            if (m_BoolOptions[i].sOptName == name)
                return pOpt->get_bool_value() == m_BoolOptions[i].fBoolValue;
        }
        return false;
    }

    if (pOpt->is_long_option())
    {
        for (unsigned i=0; i < sizeof(m_LongOptions)/sizeof(LongOption); i++)
        {
            if (m_LongOptions[i].sOptName == name)
                return pOpt->get_long_value() == m_LongOptions[i].nLongValue;
        }
        return false;
    }

    if (pOpt->is_float_option())
    {
        for (unsigned i=0; i < sizeof(m_FloatOptions)/sizeof(FloatOption); i++)
        {
            if (m_FloatOptions[i].sOptName == name)
                return pOpt->get_float_value() == m_FloatOptions[i].rFloatValue;
        }
        return false;
    }

    return false;
}

//---------------------------------------------------------------------------------------
void ImoScore::set_defaults_for_options()
{
    //bool
    for (unsigned i=0; i < sizeof(m_BoolOptions)/sizeof(BoolOption); i++)
    {
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDocModel) );
        pOpt->set_name(m_BoolOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_boolean);
        pOpt->set_bool_value( m_BoolOptions[i].fBoolValue );
        add_option(pOpt);
    }

    //long
    for (unsigned i=0; i < sizeof(m_LongOptions)/sizeof(LongOption); i++)
    {
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDocModel) );
        pOpt->set_name(m_LongOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_number_long);
        pOpt->set_long_value( m_LongOptions[i].nLongValue );
        add_option(pOpt);
    }

    //double
    for (unsigned i=0; i < sizeof(m_FloatOptions)/sizeof(FloatOption); i++)
    {
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                    ImFactory::inject(k_imo_option, m_pDocModel) );
        pOpt->set_name(m_FloatOptions[i].sOptName);
        pOpt->set_type(ImoOptionInfo::k_number_float);
        pOpt->set_float_value( m_FloatOptions[i].rFloatValue );
        add_option(pOpt);
    }
}

//---------------------------------------------------------------------------------------
void ImoScore::set_staffobjs_table(ColStaffObjs* pColStaffObjs)
{
    delete m_pColStaffObjs;
    m_pColStaffObjs = pColStaffObjs;
}

//---------------------------------------------------------------------------------------
void ImoScore::set_global_scaling(float millimeters, float tenths)
{
    if (millimeters > 0.0f && tenths > 0.0f)
    {
        m_scaling = (millimeters * 100.0f) / tenths;
        m_modified |= k_modified_scaling;
    }
}

//---------------------------------------------------------------------------------------
void ImoScore::set_float_option(const std::string& name, float value)
{
     ImoOptionInfo* pOpt = get_option(name);
     if (pOpt)
     {
         pOpt->set_float_value(value);
     }
     else
     {
        pOpt = static_cast<ImoOptionInfo*>(ImFactory::inject(k_imo_option, m_pDocModel) );
        pOpt->set_name(name);
        pOpt->set_type(ImoOptionInfo::k_number_float);
        pOpt->set_float_value(value);
        add_option(pOpt);
     }
}

//---------------------------------------------------------------------------------------
void ImoScore::set_bool_option(const std::string& name, bool value)
{
     ImoOptionInfo* pOpt = get_option(name);
     if (pOpt)
     {
         pOpt->set_bool_value(value);
     }
     else
     {
        pOpt = static_cast<ImoOptionInfo*>(ImFactory::inject(k_imo_option, m_pDocModel) );
        pOpt->set_name(name);
        pOpt->set_type(ImoOptionInfo::k_boolean);
        pOpt->set_bool_value(value);
        add_option(pOpt);
     }
}

//---------------------------------------------------------------------------------------
void ImoScore::set_long_option(const std::string& name, long value)
{
     ImoOptionInfo* pOpt = get_option(name);
     if (pOpt)
     {
         pOpt->set_long_value(value);
     }
     else
     {
        pOpt = static_cast<ImoOptionInfo*>(ImFactory::inject(k_imo_option, m_pDocModel) );
        pOpt->set_name(name);
        pOpt->set_type(ImoOptionInfo::k_number_long);
        pOpt->set_long_value(value);
        add_option(pOpt);
     }
}

//---------------------------------------------------------------------------------------
void ImoScore::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoScore>* vThis = nullptr;
    Visitor<ImoObj>* vObj = nullptr;

    vThis = dynamic_cast<Visitor<ImoScore>*>(&v);
    if (vThis)
        vThis->start_visit(this);
    else
    {
        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
        if (vObj)
            vObj->start_visit(this);
    }

    //visit_children
    std::map<std::string, ImoStyle*>::const_iterator it;
    for (it = m_nameToStyle.begin(); it != m_nameToStyle.end(); ++it)
        (it->second)->accept_visitor(v);
    visit_children(v);

    if (vThis)
        vThis->end_visit(this);
    else if (vObj)
        vObj->end_visit(this);
}

//---------------------------------------------------------------------------------------
bool ImoScore::has_visitable_children()
{
    return has_children() || m_nameToStyle.size() > 0;
}

//---------------------------------------------------------------------------------------
ImoInstruments* ImoScore::get_instruments()
{
    return dynamic_cast<ImoInstruments*>( get_child_of_type(k_imo_instruments) );
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoScore::get_instrument(int iInstr)    //iInstr = 0..n-1
{
    ImoInstruments* pColInstr = get_instruments();
    if (iInstr >= 0 && iInstr < pColInstr->get_num_items())
        return dynamic_cast<ImoInstrument*>( pColInstr->get_child(iInstr) );
    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoInstrument* ImoScore::get_instrument(const string& partId)
{
    ImoInstruments* pColInstr = get_instruments();
    ImoObj::children_iterator it;
    for (it= pColInstr->begin(); it != pColInstr->end(); ++it)
    {
        ImoInstrument* pInstr = static_cast<ImoInstrument*>(*it);
        if (pInstr->get_instr_id() == partId)
            return pInstr;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
int ImoScore::get_instr_number_for(ImoInstrument* pInstr)
{
    ImoInstruments* pColInstr = get_instruments();
    int i=0;
    ImoObj::children_iterator it;
    for (it= pColInstr->begin(); it != pColInstr->end(); ++it, ++i)
    {
        if (*it == pInstr)
            return i;
    }
    LOMSE_LOG_ERROR("[ImoScore::get_instr_number_for] pInstr not found!");
    throw runtime_error("[ImoScore::get_instr_number_for] pInstr not found!");
}

//---------------------------------------------------------------------------------------
void ImoScore::add_instrument(ImoInstrument* pInstr, const string& partId)
{
    if (pInstr->get_instr_id() == "")
        pInstr->set_instr_id(partId);
    ImoInstruments* pColInstr = get_instruments();
    pColInstr->append_child_imo(pInstr);
}

//---------------------------------------------------------------------------------------
int ImoScore::get_num_instruments()
{
    ImoInstruments* pColInstr = get_instruments();
    return pColInstr->get_num_children();
}

//---------------------------------------------------------------------------------------
ImoOptionInfo* ImoScore::get_option(const std::string& name)
{
    ImoOptions* pColOpts = get_options();
    ImoObj::children_iterator it;
    for (it= pColOpts->begin(); it != pColOpts->end(); ++it)
    {
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(*it);
        if (pOpt->get_name() == name)
            return pOpt;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoOptions* ImoScore::get_options()
{
    return dynamic_cast<ImoOptions*>( get_child_of_type(k_imo_options) );
}

//---------------------------------------------------------------------------------------
void ImoScore::add_option(ImoOptionInfo* pOpt)
{
    ImoOptions* pColOpts = get_options();
    return pColOpts->append_child_imo(pOpt);
}

//---------------------------------------------------------------------------------------
void ImoScore::add_or_replace_option(ImoOptionInfo* pOpt)
{
    ImoOptionInfo* pOldOpt = get_option( pOpt->get_name() );
    if (pOldOpt)
    {
        ImoOptions* pColOpts = get_options();
        m_pDocModel->on_removed_from_model(pOldOpt);
        pColOpts->remove_child_imo(pOldOpt);
        delete pOldOpt;
    }
    add_option(pOpt);
}

//---------------------------------------------------------------------------------------
bool ImoScore::has_options()
{
    ImoOptions* pColOpts = get_options();
    return pColOpts->get_num_children() > 0;
}

//---------------------------------------------------------------------------------------
void ImoScore::add_sytem_info(ImoSystemInfo* pSL)
{
    if (pSL->is_first())
        m_systemInfoFirst = *pSL;
    else
        m_systemInfoOther = *pSL;
}

//---------------------------------------------------------------------------------------
ImoInstrGroups* ImoScore::get_instrument_groups()
{
    return static_cast<ImoInstrGroups*>( get_child_of_type(k_imo_instrument_groups) );
}

//---------------------------------------------------------------------------------------
list<ImoInstrGroup*> ImoScore::find_groups_containing_instrument(ImoInstrument* pInstr)
{
    list<ImoInstrGroup*> groups;
    ImoInstrGroups* pGroups = get_instrument_groups();
    if (pGroups)
    {
        ImoObj::children_iterator itG;
        for (itG= pGroups->begin(); itG != pGroups->end(); ++itG)
        {
            ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(*itG);
            if (pGroup->contains_instrument(pInstr))
                groups.push_back(pGroup);
        }
    }
    return groups;
}

//---------------------------------------------------------------------------------------
void ImoScore::add_instruments_group(ImoInstrGroup* pGroup)
{
    ImoInstrGroups* pGroups = get_instrument_groups();
    if (!pGroups)
    {
        pGroups = static_cast<ImoInstrGroups*>(
                        ImFactory::inject(k_imo_instrument_groups, m_pDocModel) );
        append_child_imo(pGroups);
    }
    pGroups->append_child_imo(pGroup);
}

//---------------------------------------------------------------------------------------
ImoScoreTitles* ImoScore::get_titles()
{
    return static_cast<ImoScoreTitles*>( get_child_of_type(k_imo_score_titles) );
}

//---------------------------------------------------------------------------------------
void ImoScore::add_title(ImoScoreTitle* pTitle)
{
    ImoScoreTitles* pTitles = get_titles();
    pTitles->append_child_imo(pTitle);
}

//---------------------------------------------------------------------------------------
void ImoScore::add_style(ImoStyle* pStyle)
{
    const string& name = pStyle->get_name();
    m_nameToStyle[name] = pStyle;
    if (name.compare(0, 6, "Lyric-") == 0)
        ++m_numLyricFonts;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::find_style(const std::string& name)
{
	map<std::string, ImoStyle*>::const_iterator it
        = m_nameToStyle.find(name);
	if (it != m_nameToStyle.end())
		return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::get_style_or_default(const std::string& name)
{
    ImoStyle* pStyle = find_style(name);
	if (pStyle)
		return pStyle;
    else
        return get_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::get_default_style()
{
    ImoStyle* pStyle = find_style("Default style");
    if (pStyle)
		return pStyle;
    else
        return create_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoScore::create_default_style()
{
    //TODO: all code related to creating/storing/findig styles should be
    //removed and use Document Styles instead
	ImoStyle* pDefStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pDefStyle->set_name("Default style");

            //font properties
    pDefStyle->font_file("");
    pDefStyle->font_name("Liberation serif");

    //BUG_BYPASS: Default style should use the right font file / font name for the current
    // document language. This block is a fix just for Chinese.language
    //TODO: Fix for other languages
    //TODO: How to get default values for comparisons (k_default_language) ?
    {
        //get document language
        ImoDocument* pImoDoc = m_pDocModel->get_im_root();
        if (pImoDoc)    //AWARE: in unit tests there could be no ImoDoc
        {
            string& language = pImoDoc->get_language();
            if (language == "zh_CN")
            {
                pDefStyle->font_file("");
                pDefStyle->font_name("WenQuanYi Zen Hei");
            }
        }
    }

    pDefStyle->font_size( k_default_font_size );
    pDefStyle->font_style( k_default_font_style );
    pDefStyle->font_weight( k_default_font_weight );
        //text
    pDefStyle->word_spacing( k_default_word_spacing );
    pDefStyle->text_decoration( k_default_text_decoration );
    pDefStyle->vertical_align( k_default_vertical_align );
    pDefStyle->text_align( k_default_text_align );
    pDefStyle->text_indent_length( k_default_text_indent_length );
    pDefStyle->word_spacing_length( k_default_word_spacing_length );   //not applicable
    pDefStyle->line_height( k_default_line_height );
        //color and background
    pDefStyle->color( k_default_color );
    pDefStyle->background_color( k_default_background_color );
        //margin
    pDefStyle->margin_top( k_default_margin_top );
    pDefStyle->margin_bottom( k_default_margin_bottom );
    pDefStyle->margin_left( k_default_margin_left );
    pDefStyle->margin_right( k_default_margin_right );
        //padding
    pDefStyle->padding_top( k_default_padding_top );
    pDefStyle->padding_bottom( k_default_padding_bottom );
    pDefStyle->padding_left( k_default_padding_left );
    pDefStyle->padding_right( k_default_padding_right );
        ////border
    //pDefStyle->set_lunits_property(ImoStyle::k_border_top, k_default_border_top);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_left, k_default_border_left);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_right, k_default_border_right);
        //border width
    pDefStyle->border_width_top( k_default_border_width_top );
    pDefStyle->border_width_bottom( k_default_border_width_bottom );
    pDefStyle->border_width_left( k_default_border_width_left );
    pDefStyle->border_width_right( k_default_border_width_right );
        //size
    pDefStyle->min_height( k_default_min_height );
    pDefStyle->max_height( k_default_max_height );
    pDefStyle->height( k_default_height );
    pDefStyle->min_width( k_default_min_width );
    pDefStyle->max_width( k_default_max_width );
    pDefStyle->width( k_default_width );

//    m_nameToStyle[pDefStyle->get_name()] = pDefStyle;
    add_style(pDefStyle);
    pDefStyle->reset_style_modified();
    return pDefStyle;
}

//---------------------------------------------------------------------------------------
void ImoScore::add_required_text_styles()
{
    //AWARE: When modifying this method check also method
    //       ImoStyle::is_default_style_with_default_values()

    ImoStyle* pDefStyle = get_default_style();

    //For tuplets numbers
    if (find_style("Tuplet numbers") == nullptr)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
        pStyle->set_name("Tuplet numbers");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->font_size( 10.0f);
        pStyle->font_style( ImoStyle::k_font_style_italic);
        pStyle->font_weight( ImoStyle::k_font_weight_normal);
        add_style(pStyle);
        pStyle->reset_style_modified();
    }

    //For instrument and group names and abbreviations
    if (find_style("Instrument names") == nullptr)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
        pStyle->set_name("Instrument names");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->font_size( 14.0f);
        add_style(pStyle);
        pStyle->reset_style_modified();
    }

    //For metronome marks
    if (find_style("Metronome marks") == nullptr)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
        pStyle->set_name("Metronome marks");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->font_size( 7.0f);
        add_style(pStyle);
        pStyle->reset_style_modified();
    }

    //for lyrics
    if (find_style("Lyrics") == nullptr)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
        pStyle->set_name("Lyrics");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->font_size(10.0f);
        add_style(pStyle);
        pStyle->reset_style_modified();
    }
    //<lyric-font font-family="Times New Roman" font-size="10"/>

    //For volta brackets
    if (find_style("Volta brackets") == nullptr)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
        pStyle->set_name("Volta brackets");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->font_size( 10.0f);
        pStyle->font_style( ImoStyle::k_font_style_normal);
        pStyle->font_weight( ImoStyle::k_font_weight_bold);
        add_style(pStyle);
        pStyle->reset_style_modified();
    }
    //font-family="Liberation Serif" font-size="10" bold

    //For measure numbers
    if (find_style("Measure numbers") == nullptr)
    {
	    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
        pStyle->set_name("Measure numbers");
        pStyle->set_parent_style(pDefStyle);
	    pStyle->font_size( 10.0f);
        pStyle->font_style( ImoStyle::k_font_style_italic);
        pStyle->font_weight( ImoStyle::k_font_weight_normal);
        add_style(pStyle);
        pStyle->reset_style_modified();
    }

}

//---------------------------------------------------------------------------------------
void ImoScore::add_lyric_language(int number, const string& lang)
{
    m_lyricLanguages[number] = lang;
}

//---------------------------------------------------------------------------------------
string ImoScore::get_lyric_language(int number)
{
	map<int, string>::const_iterator it = m_lyricLanguages.find(number);
	if (it != m_lyricLanguages.end())
		return it->second;
    else
        return "";
}


//---------------------------------------------------------------------------------------
SoundEventsTable* ImoScore::get_midi_table()
{
    if (!m_pMidiTable)
    {
        m_pMidiTable = LOMSE_NEW SoundEventsTable(this);
        m_pMidiTable->create_table();
    }
    return m_pMidiTable;
}

//---------------------------------------------------------------------------------------
// Score API
//---------------------------------------------------------------------------------------
ImoInstrument* ImoScore::add_instrument()
{
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                ImFactory::inject(k_imo_instrument, m_pDocModel) );

    ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(
                                ImFactory::inject(k_imo_sound_info, m_pDocModel) );
    pInstr->add_sound_info(pInfo);

    ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, m_pDocModel));
    pInstr->append_child_imo(pMD);

    add_instrument(pInstr);

    return pInstr;
}

//---------------------------------------------------------------------------------------
void ImoScore::end_of_changes()
{
    ModelBuilder builder;
    builder.structurize(this);
}


//=======================================================================================
// ImoScoreLine implementation
//=======================================================================================

//ImoScoreLine::ImoScoreLine(lmScoreObj* pOwner, ImoId nID, lmTenths xStart, lmTenths yStart,
//                         lmTenths xEnd, lmTenths yEnd, lmTenths nWidth,
//                         lmELineCap nStartCap, lmELineCap nEndCap, lmELineStyle nStyle,
//                         wxColour nColor)
//    : lmAuxObj(pOwner, nID, lm_eSO_Line, lmDRAGGABLE)
//    , m_txStart(xStart)
//    , m_tyStart(yStart)
//    , m_txEnd(xEnd)
//    , m_tyEnd(yEnd)
//    , m_tWidth(nWidth)
//	, m_nColor(nColor)
//	, m_nEdge(lm_eEdgeNormal)
//    , m_nStyle(nStyle)
//    , m_nStartCap(nStartCap)
//    , m_nEndCap(nEndCap)
//{
//}


#if (LOMSE_ENABLE_THREADS == 1)
//=======================================================================================
// ImoScorePlayer implementation
//=======================================================================================
ImoScorePlayer::ImoScorePlayer()
    : ImoControl(k_imo_score_player)
    , m_idScore(k_no_imoid)
    , m_playLabel("Play")
    , m_stopLabel("Stop playing")
{
}

//---------------------------------------------------------------------------------------
void ImoScorePlayer::attach_player(ScorePlayerCtrl* pPlayer)
{
    ImoControl::attach_control(pPlayer);
}

//---------------------------------------------------------------------------------------
ImoScore* ImoScorePlayer::get_score()
{
    if (m_pDocModel && m_idScore != k_no_imoid)
        return static_cast<ImoScore*>( m_pDocModel->get_pointer_to_imo(m_idScore) );
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoScorePlayer::attach_score(ImoScore* pScore)
{
    if (pScore)
        m_idScore = pScore->get_id();
    else
        m_idScore = k_no_imoid;
}

//---------------------------------------------------------------------------------------
ScorePlayerCtrl* ImoScorePlayer::get_player()
{
    return static_cast<ScorePlayerCtrl*>(m_ctrol.get());
}

//---------------------------------------------------------------------------------------
void ImoScorePlayer::set_metronome_mm(int value)
{
    ScorePlayerCtrl* pPlayer = get_player();
    if (pPlayer)
        pPlayer->set_metronome_mm(value);
}

//---------------------------------------------------------------------------------------
void ImoScorePlayer::set_play_label(const string& value)
{
    m_playLabel = value;
    ScorePlayerCtrl* pPlayer = get_player();
    if (pPlayer)
        pPlayer->set_text(value);
}

//---------------------------------------------------------------------------------------
int ImoScorePlayer::get_metronome_mm()
{
    ScorePlayerCtrl* pPlayer = get_player();
    if (pPlayer)
        return pPlayer->get_metronome_mm();
    else
        return 60;
}
#endif  //LOMSE_ENABLE_THREADS == 1


//=======================================================================================
// ImoSounds implementation
//=======================================================================================
void ImoSounds::add_sound_info(ImoSoundInfo* pInfo)
{
    append_child_imo(pInfo);
}

//---------------------------------------------------------------------------------------
int ImoSounds::get_num_sounds()
{
    return get_num_children();
}

//---------------------------------------------------------------------------------------
ImoSoundInfo* ImoSounds::get_sound_info(const string& soundId)
{
    ImoObj::children_iterator it;
    for (it= this->begin(); it != this->end(); ++it)
    {
        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(*it);
        if (pInfo->get_score_instr_id() == soundId)
            return pInfo;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
ImoSoundInfo* ImoSounds::get_sound_info(int iSound)
{
    return dynamic_cast<ImoSoundInfo*>( get_child(iSound) );
}


//=======================================================================================
// ImoSoundInfo implementation
//=======================================================================================
ImoSoundInfo::ImoSoundInfo()
    : ImoSimpleObj(k_imo_sound_info)
    , m_instrName("")
    , m_instrAbbrev("")
    , m_instrSound("")
    , m_fSolo(false)
    , m_fEnsemble(false)
    , m_ensembleSize(0)
    , m_virtualLibrary("")
    , m_virtualName("")
    , m_playTechnique(0)
{

}

//---------------------------------------------------------------------------------------
void ImoSoundInfo::initialize_object()
{
    ImoMidiInfo* pMidi = static_cast<ImoMidiInfo*>(
                                ImFactory::inject(k_imo_midi_info, m_pDocModel) );
    append_child_imo(pMidi);
}

//---------------------------------------------------------------------------------------
void ImoSoundInfo::set_score_instr_id(const string& value)
{
    m_soundId = value;
    ImoMidiInfo* pMidi = static_cast<ImoMidiInfo*>(get_child_of_type(k_imo_midi_info));
    pMidi->set_score_instr_id(value);
}

//---------------------------------------------------------------------------------------
ImoMidiInfo* ImoSoundInfo::get_midi_info()
{
    return static_cast<ImoMidiInfo*>(get_child_of_type(k_imo_midi_info));
}


//=======================================================================================
// ImoSoundChange implementation
//=======================================================================================
ImoMidiInfo* ImoSoundChange::get_midi_info(const string& soundId)
{
    ImoObj::children_iterator it = this->begin();
    while (it != this->end())
    {
        if ((*it)->is_midi_info())
        {
            ImoMidiInfo* pMidi = static_cast<ImoMidiInfo*>(*it);
            if (pMidi->get_score_instr_id() == soundId)
                return pMidi;
        }
    }
    return nullptr;
}


//=======================================================================================
// ImoStyle implementation
//=======================================================================================
void ImoStyle::set_parent_style(ImoStyle* pStyle)
{
    if (pStyle)
        m_idParent = pStyle->get_id();
    else
        m_idParent = k_no_imoid;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyle::get_parent_style()
{
    if (m_pDocModel && m_idParent != k_no_imoid)
        return static_cast<ImoStyle*>( m_pDocModel->get_pointer_to_imo(m_idParent) );
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_string_property(int prop, const std::string& value)
{
    m_stringProps[prop] = value;
    switch(prop)
    {
        case ImoStyle::k_font_name:    m_modified |= k_modified_font_name;   break;
        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_float_property(int prop, float value)
{
    m_floatProps[prop] = value;
    switch(prop)
    {
        case ImoStyle::k_font_size:    m_modified |= k_modified_font_size;   break;
        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_int_property(int prop, int value)
{
    m_intProps[prop] = value;
    switch(prop)
    {
        case ImoStyle::k_font_style:    m_modified |= k_modified_font_style;   break;
        case ImoStyle::k_font_weight:   m_modified |= k_modified_font_weight;  break;
        default:
            ;
    }
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_lunits_property(int prop, LUnits value)
{
    m_lunitsProps[prop] = value;
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_color_property(int prop, Color value)
{
    m_colorProps[prop] = value;
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_margin_property(LUnits value)
{
    set_lunits_property(ImoStyle::k_margin_left, value);
    set_lunits_property(ImoStyle::k_margin_top, value);
    set_lunits_property(ImoStyle::k_margin_right, value);
    set_lunits_property(ImoStyle::k_margin_bottom, value);
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_padding_property(LUnits value)
{
    set_lunits_property(ImoStyle::k_padding_left, value);
    set_lunits_property(ImoStyle::k_padding_top, value);
    set_lunits_property(ImoStyle::k_padding_right, value);
    set_lunits_property(ImoStyle::k_padding_bottom, value);
}

//---------------------------------------------------------------------------------------
void ImoStyle::set_border_width_property(LUnits value)
{
    set_lunits_property(ImoStyle::k_border_width_left, value);
    set_lunits_property(ImoStyle::k_border_width_top, value);
    set_lunits_property(ImoStyle::k_border_width_right, value);
    set_lunits_property(ImoStyle::k_border_width_bottom, value);
}

//---------------------------------------------------------------------------------------
bool ImoStyle::get_float_property(int prop, float* value)
{
    map<int, float>::const_iterator it = m_floatProps.find(prop);
    if (it != m_floatProps.end())
    {
        *value = it->second;
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool ImoStyle::get_lunits_property(int prop, LUnits* value)
{
    map<int, LUnits>::const_iterator it = m_lunitsProps.find(prop);
    if (it != m_lunitsProps.end())
    {
        *value = it->second;
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool ImoStyle::get_string_property(int prop, string* value)
{
    map<int, string>::const_iterator it = m_stringProps.find(prop);
    if (it != m_stringProps.end())
    {
        *value = it->second;
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool ImoStyle::get_int_property(int prop, int* value)
{
    map<int, int>::const_iterator it = m_intProps.find(prop);
    if (it != m_intProps.end())
    {
        *value = it->second;
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool ImoStyle::get_color_property(int prop, Color* value)
{
    map<int, Color>::const_iterator it = m_colorProps.find(prop);
    if (it != m_colorProps.end())
    {
        *value = it->second;
        return true;
    }
    else
        return false;
}

//---------------------------------------------------------------------------------------
float ImoStyle::get_float_property(int prop)
{
    map<int, float>::const_iterator it = m_floatProps.find(prop);
    if (it != m_floatProps.end())
        return it->second;
    else if (m_idParent != k_no_imoid)
        return get_parent_style()->get_float_property(prop);
    else
    {
        LOMSE_LOG_ERROR("Aborting. Style has no parent.");
        throw std::runtime_error( "[ImoStyle::get_float_property]. No parent" );
    }
}

//---------------------------------------------------------------------------------------
LUnits ImoStyle::get_lunits_property(int prop)
{
    map<int, LUnits>::const_iterator it = m_lunitsProps.find(prop);
    if (it != m_lunitsProps.end())
        return it->second;
    else if (m_idParent != k_no_imoid)
        return get_parent_style()->get_lunits_property(prop);
    else
    {
        LOMSE_LOG_ERROR("Aborting. Style has no parent.");
        throw std::runtime_error( "[ImoStyle::get_lunits_property]. No parent" );
    }
}

//---------------------------------------------------------------------------------------
const std::string& ImoStyle::get_string_property(int prop)
{
    map<int, string>::const_iterator it = m_stringProps.find(prop);
    if (it != m_stringProps.end())
        return it->second;
    else if (m_idParent != k_no_imoid)
        return get_parent_style()->get_string_property(prop);
    else
    {
        LOMSE_LOG_ERROR("Aborting. Style has no parent.");
        throw std::runtime_error( "[ImoStyle::get_string_property]. No parent" );
    }
}

//---------------------------------------------------------------------------------------
int ImoStyle::get_int_property(int prop)
{
    map<int, int>::const_iterator it = m_intProps.find(prop);
    if (it != m_intProps.end())
        return it->second;
    else if (m_idParent != k_no_imoid)
        return get_parent_style()->get_int_property(prop);
    else
    {
        LOMSE_LOG_ERROR("Aborting. Style has no parent.");
        throw std::runtime_error( "[ImoStyle::get_int_property]. No parent" );
    }
}

//---------------------------------------------------------------------------------------
Color ImoStyle::get_color_property(int prop)
{
    map<int, Color>::const_iterator it = m_colorProps.find(prop);
    if (it != m_colorProps.end())
        return it->second;
    else if (m_idParent != k_no_imoid)
        return get_parent_style()->get_color_property(prop);
    else
        throw std::runtime_error( "[ImoStyle::get_color_property]. No parent" );
}

//---------------------------------------------------------------------------------------
bool ImoStyle::is_default_style_with_default_values()
{
    //returns true if it is a default style and contains default values

    //TODO: It should also be checked that the other defaults still maintain the value.

    //Default style
    if (m_name == "Default style")
        return true;

    //Headers 1-6
    if (m_name == "Heading-1")
        return font_size() == 21.0f
            && font_weight() == k_font_weight_bold
            && is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            //&& font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            && is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    if (m_name == "Heading-2")
        return font_size() == 17.0f
            && font_weight() == k_font_weight_bold
            && is_equal_pos( margin_top(), 400.0f )
            && is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            //&& font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            //&& is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    if (m_name == "Heading-3")
        return font_size() == 14.0f
            && font_weight() == k_font_weight_bold
            && is_equal_pos( margin_top(), 400.0f )
            && is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            //&& font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            //&& is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    if (m_name == "Heading-4")
        return font_size() == 12.0f
            && font_weight() == k_font_weight_bold
            && is_equal_pos( margin_top(), 400.0f )
            && is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            //&& font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            //&& is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    if (m_name == "Heading-5")
        return font_style() == k_font_style_italic
            && font_weight() == k_font_weight_bold
            && is_equal_pos( margin_top(), 400.0f )
            && is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            && font_size() == 12.0f
            //&& font_style() == k_font_style_normal
            //&& font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            //&& is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    if (m_name == "Heading-6")
        return font_style() == k_font_style_italic
            && is_equal_pos( margin_top(), 400.0f )
            && is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            && font_size() == 12.0f
            //&& font_style() == k_font_style_normal
            //&& font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            //&& is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    //table
    if (m_name == "Table")
        return is_equal_pos( border_width_top(), 20.0f )
            && is_equal_pos( border_width_bottom(), 20.0f )
            && is_equal_pos( border_width_left(), 20.0f )
            && is_equal_pos( border_width_right(), 20.0f )
            && is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            && font_size() == 12.0f
            && font_style() == k_font_style_normal
            && font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            //&& is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            //&& is_equal_pos( border_width_top(), k_default_border_width_top )
            //&& is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            //&& is_equal_pos( border_width_left(), k_default_border_width_left )
            //&& is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    //paragraph
    if (m_name == "Paragraph")
        return is_equal_pos( margin_bottom(), 300.0f )
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            && font_size() == 12.0f
            && font_style() == k_font_style_normal
            && font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            //&& is_equal_pos( margin_top(), k_default_margin_top )
            //&& is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    //Tuplets numbers
    if (m_name == "Tuplet numbers")
        return font_size() == 10.0f
            && font_style() == k_font_style_italic
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            //&& font_style() == k_font_style_normal
            && font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            && is_equal_pos( margin_top(), k_default_margin_top )
            && is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
               ;

    //Instrument names
    if (m_name == "Instrument names")
	    return font_size() == 14.0f
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            && font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            && is_equal_pos( margin_top(), k_default_margin_top )
            && is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    //Metronome marks
    if (m_name == "Metronome marks")
	    return font_size() == 7.0f
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            && font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            && is_equal_pos( margin_top(), k_default_margin_top )
            && is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    //Lyrics
    if (m_name == "Lyrics")
	    return font_size() == 10.0f
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            && font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            && is_equal_pos( margin_top(), k_default_margin_top )
            && is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    //Lyrics
    if (m_name == "Volta brackets")
	    return font_size() == 10.0f
            && font_weight() == k_font_weight_bold
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            && font_style() == k_font_style_normal
            //&& font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            && is_equal_pos( margin_top(), k_default_margin_top )
            && is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
            ;

    //Measure numbers
    if (m_name == "Measure numbers")
        return font_size() == 10.0f
            && font_style() == k_font_style_italic
            //inherited defaults:
               //font
            && font_file() == ""
            && font_name() == "Liberation serif"
            //&& font_size() == 12.0f
            //&& font_style() == k_font_style_normal
            && font_weight() == k_font_weight_normal
               //text
            && word_spacing() == k_default_word_spacing
            && text_decoration() == k_default_text_decoration
            && vertical_align() == k_default_vertical_align
            && text_align() == k_default_text_align
            && text_indent_length() == k_default_text_indent_length
            && word_spacing_length() == k_default_word_spacing_length   //not applicable
            && line_height() == k_default_line_height
               //color and background
            && is_equal(color(), Color(0,0,0))
            && is_equal(background_color(), Color(255,255,255))
               //margin
            && is_equal_pos( margin_top(), k_default_margin_top )
            && is_equal_pos( margin_bottom(), k_default_margin_bottom )
            && is_equal_pos( margin_left(), k_default_margin_left )
            && is_equal_pos( margin_right(), k_default_margin_right )
               //padding
            && is_equal_pos( padding_top(), k_default_padding_top )
            && is_equal_pos( padding_bottom(), k_default_padding_bottom )
            && is_equal_pos( padding_left(), k_default_padding_left )
            && is_equal_pos( padding_right(), k_default_padding_right )
               ////border
            //&& set_lunits_property(ImoStyle::k_border_top, k_default_border_top
            //&& set_lunits_property(ImoStyle::k_border_bottom, k_default_border_bottom
            //&& set_lunits_property(ImoStyle::k_border_left, k_default_border_left
            //&& set_lunits_property(ImoStyle::k_border_right, k_default_border_right
               //border width
            && is_equal_pos( border_width_top(), k_default_border_width_top )
            && is_equal_pos( border_width_bottom(), k_default_border_width_bottom )
            && is_equal_pos( border_width_left(), k_default_border_width_left )
            && is_equal_pos( border_width_right(), k_default_border_width_right )
               //size
            && is_equal_pos( min_height(), k_default_min_height )
            && is_equal_pos( max_height(), k_default_max_height )
            && is_equal_pos( height(), k_default_height )
            && is_equal_pos( min_width(), k_default_min_width )
            && is_equal_pos( max_width(), k_default_max_width )
            && is_equal_pos( width(), k_default_width )
               ;

    return false;
}


//=======================================================================================
// ImoStaffInfo implementation
//=======================================================================================
LUnits ImoStaffInfo::get_height() const
{
    //Stafflines are shown in the order of middle-line of a five-line staff and add
    //lines alternating with the next line above this line and the next line below.
    //
    //When 1 line or 5 lines, height is 5 lines (4 spaces)
    //Otherwise, the height is the number of visible lines + 2 (one space above and below)

    int numVisible = (m_nNumLines < 5 ? 5 : m_nNumLines);
    if (!m_fTablature && (m_nNumLines != 1 && m_nNumLines != 5))
        numVisible = m_nNumLines + 2;

    return (numVisible - 1) * m_uSpacing + m_uLineThickness;
}

//---------------------------------------------------------------------------------------
bool ImoStaffInfo::is_line_visible(int iLine) const
{
    //Stafflines are shown in the order of middle-line of a five-line staff and add
    //lines alternating with the next line above this line and the next line below.

    switch(m_nNumLines)
    {
        case 1:     return iLine == 2;
        case 2:     return iLine == 1 || iLine == 2;
        case 3:     return iLine == 1 || iLine == 2 || iLine == 3;
        case 4:     return iLine <= 3;
        default:    return true;
    }
}

//---------------------------------------------------------------------------------------
double ImoStaffInfo::get_applied_spacing_factor() const
{
    return double(m_uSpacing) / double(LOMSE_STAFF_LINE_SPACING);
}


//=======================================================================================
// ImoStyles implementation
//=======================================================================================
ImoStyles::ImoStyles()
    : ImoSimpleObj(k_imo_styles)
{
}

//---------------------------------------------------------------------------------------
ImoStyles::~ImoStyles()
{
    delete_styles();
}

//---------------------------------------------------------------------------------------
ImoStyles& ImoStyles::clone(const ImoStyles& a)
{
    map<std::string, ImoStyle*>::const_iterator it;
    for (it = a.m_nameToStyle.begin(); it != a.m_nameToStyle.end(); ++it)
    {
        m_nameToStyle[it->first] = static_cast<ImoStyle*>(
                                        ImFactory::clone(it->second) );
    }
    return *this;
}

//---------------------------------------------------------------------------------------
void ImoStyles::initialize_object()
{
    create_default_styles();
}

//---------------------------------------------------------------------------------------
void ImoStyles::delete_styles()
{
    map<std::string, ImoStyle*>::const_iterator it;
    for (it = m_nameToStyle.begin(); it != m_nameToStyle.end(); ++it)
        delete it->second;

    m_nameToStyle.clear();
}

//---------------------------------------------------------------------------------------
void ImoStyles::accept_visitor(BaseVisitor& v)
{
    Visitor<ImoObj>* vObj = nullptr;

    vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
    if (vObj)
    {
        vObj->start_visit(this);
    }

    //visit_children
    map<std::string, ImoStyle*>::iterator it;
    for(it = m_nameToStyle.begin(); it != m_nameToStyle.end(); ++it)
        (it->second)->accept_visitor(v);

    if (vObj)
        vObj->end_visit(this);
}

//---------------------------------------------------------------------------------------
void ImoStyles::add_style(ImoStyle* pStyle)
{
    string name = pStyle->get_name();
    ImoStyle* oldStyle = find_style(name);
    delete oldStyle;
    m_nameToStyle[name] = pStyle;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyles::find_style(const std::string& name)
{
	map<std::string, ImoStyle*>::const_iterator it
        = m_nameToStyle.find(name);
	if (it != m_nameToStyle.end())
		return it->second;
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyles::get_style_or_default(const std::string& name)
{
    ImoStyle* pStyle = find_style(name);
	if (pStyle)
		return pStyle;
    else
        return get_default_style();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoStyles::get_default_style()
{
    return find_style("Default style");
}

//---------------------------------------------------------------------------------------
void ImoStyles::create_default_styles()
{
    //AWARE: When modifying this method you will have to modify also method
    //       ImoStyle::is_default_style_with_default_values()

    // Default style
	ImoStyle* pDefStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pDefStyle->set_name("Default style");

        //font properties
    pDefStyle->font_file("");
    pDefStyle->font_name("Liberation serif");
    pDefStyle->font_size(12.0f);
    pDefStyle->font_style( ImoStyle::k_font_style_normal );
    pDefStyle->font_weight( ImoStyle::k_font_weight_normal );
        //text
    pDefStyle->word_spacing(ImoStyle::k_spacing_normal);
    pDefStyle->text_decoration(ImoStyle::k_decoration_none);
    pDefStyle->vertical_align(ImoStyle::k_valign_baseline);
    pDefStyle->text_align(ImoStyle::k_align_left);
    pDefStyle->text_indent_length(0.0f);
    pDefStyle->word_spacing_length(0.0f);   //not applicable
    pDefStyle->line_height(1.5f);
        //color and background
    pDefStyle->color( Color(0,0,0) );
    pDefStyle->background_color( Color(255,255,255));
        //margin
    pDefStyle->margin_top(0.0f);
    pDefStyle->margin_bottom(0.0f);
    pDefStyle->margin_left(0.0f);
    pDefStyle->margin_right(0.0f);
        //padding
    pDefStyle->padding_top(0.0f);
    pDefStyle->padding_bottom(0.0f);
    pDefStyle->padding_left(0.0f);
    pDefStyle->padding_right(0.0f);
        ////border
    //pDefStyle->set_lunits_property(ImoStyle::k_border_top, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_bottom, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_left, 0.0f);
    //pDefStyle->set_lunits_property(ImoStyle::k_border_right, 0.0f);
        //border width
    pDefStyle->border_width_top(0.0f);
    pDefStyle->border_width_bottom(0.0f);
    pDefStyle->border_width_left(0.0f);
    pDefStyle->border_width_right(0.0f);
        //size
    pDefStyle->min_height(0.0f);
    pDefStyle->max_height(0.0f);
    pDefStyle->height(0.0f);
    pDefStyle->min_width(0.0f);
    pDefStyle->max_width(0.0f);
    pDefStyle->width(0.0f);

    add_style(pDefStyle);
    pDefStyle->reset_style_modified();


    //Headers 1-6
    ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Heading-1");
    pStyle->set_parent_style(pDefStyle);
    pStyle->font_size( 21.0f);
    pStyle->font_weight( ImoStyle::k_font_weight_bold);
    pStyle->margin_bottom(300);
    add_style(pStyle);
    pStyle->reset_style_modified();

    pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Heading-2");
    pStyle->set_parent_style(pDefStyle);
    pStyle->font_size( 17.0f);
    pStyle->font_weight( ImoStyle::k_font_weight_bold);
    pStyle->margin_top(400);
    pStyle->margin_bottom(300);
    add_style(pStyle);
    pStyle->reset_style_modified();

    pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Heading-3");
    pStyle->set_parent_style(pDefStyle);
    pStyle->font_size( 14.0f);
    pStyle->font_weight( ImoStyle::k_font_weight_bold);
    pStyle->margin_top(400);
    pStyle->margin_bottom(300);
    add_style(pStyle);
    pStyle->reset_style_modified();

    pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Heading-4");
    pStyle->set_parent_style(pDefStyle);
    pStyle->font_size( 12.0f);
    pStyle->font_weight( ImoStyle::k_font_weight_bold);
    pStyle->margin_top(400);
    pStyle->margin_bottom(300);
    add_style(pStyle);
    pStyle->reset_style_modified();

    pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Heading-5");
    pStyle->set_parent_style(pDefStyle);
    pStyle->font_size( 12.0f);
    pStyle->font_weight( ImoStyle::k_font_weight_bold);
    pStyle->font_style(ImoStyle::k_font_style_italic );
    pStyle->margin_top(400);
    pStyle->margin_bottom(300);
    add_style(pStyle);
    pStyle->reset_style_modified();

    pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Heading-6");
    pStyle->set_parent_style(pDefStyle);
    pStyle->font_size( 12.0f);
    pStyle->font_weight( ImoStyle::k_font_weight_normal);
    pStyle->font_style(ImoStyle::k_font_style_italic );
    pStyle->margin_top(400);
    pStyle->margin_bottom(300);
    add_style(pStyle);
    pStyle->reset_style_modified();

    //table
    pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Table");
    pStyle->set_parent_style(pDefStyle);
    pStyle->margin_bottom(300);
    pStyle->border_width(20.0);
    add_style(pStyle);
    pStyle->reset_style_modified();

    //paragraph
    pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, m_pDocModel));
    pStyle->set_name("Paragraph");
    pStyle->set_parent_style(pDefStyle);
    pStyle->margin_bottom(300);
    add_style(pStyle);
    pStyle->reset_style_modified();
}


//=======================================================================================
// ImoBoxInline implementation
//=======================================================================================
//methods for adding inline-level content
LOMSE_IMPLEMENT_INLINE_LEVEL_INTERFACE(ImoBoxInline)


//=======================================================================================
// ImoInlinesContainer implementation
//=======================================================================================
//methods for adding inline-level content
LOMSE_IMPLEMENT_INLINE_LEVEL_INTERFACE(ImoInlinesContainer)


//=======================================================================================
// ImoPageInfo implementation
//=======================================================================================
ImoPageInfo::ImoPageInfo()
    : ImoSimpleObj(k_imo_page_info)
    //odd pages
    , m_uLeftMarginOdd(1500.0f)
    , m_uRightMarginOdd(1500.0f)
    , m_uTopMarginOdd(2000.0f)
    , m_uBottomMarginOdd(2000.0f)
    //even pages
    , m_uLeftMarginEven(1500.0f)
    , m_uRightMarginEven(1500.0f)
    , m_uTopMarginEven(2000.0f)
    , m_uBottomMarginEven(2000.0f)
    //size and orientation
    , m_uPageSize(21000.0f, 29700.0f)
    , m_fPortrait(true)
{
    //defaults: DIN A4 (210.0 x 297.0 mm), portrait
}


//=======================================================================================
// ImoScoreText implementation
//=======================================================================================
string& ImoScoreText::get_language()
{
    string& language = m_text.language;
    if (!language.empty())
        return language;
    else
    {
        ImoDocument* pDoc = get_document();
        if (pDoc)
            return pDoc->get_language();
        else
        {
            LOMSE_LOG_ERROR("[ImoScoreText::get_language] No owner Document.");
            throw runtime_error("[ImoScoreText::get_language] No owner Document.");
        }
    }
}


//=======================================================================================
// ImoSlur implementation
//=======================================================================================
ImoNote* ImoSlur::get_start_note()
{
    return static_cast<ImoNote*>( get_start_object() );
}

//---------------------------------------------------------------------------------------
ImoNote* ImoSlur::get_end_note()
{
    return static_cast<ImoNote*>( get_end_object() );
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoSlur::get_start_bezier()
{
    ImoSlurData* pData = static_cast<ImoSlurData*>( get_start_data() );
    return pData->get_bezier();
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoSlur::get_start_bezier_or_create()
{
    ImoSlurData* pData = static_cast<ImoSlurData*>( get_start_data() );
    ImoBezierInfo* pInfo = pData->get_bezier();
    if (!pInfo)
        pInfo = pData->add_bezier();
    return pInfo;
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoSlur::get_stop_bezier()
{
    ImoSlurData* pData = static_cast<ImoSlurData*>( get_end_data() );
    return pData->get_bezier();
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoSlur::get_stop_bezier_or_create()
{
    ImoSlurData* pData = static_cast<ImoSlurData*>( get_end_data() );
    ImoBezierInfo* pInfo = pData->get_bezier();
    if (!pInfo)
        pInfo = pData->add_bezier();
    return pInfo;
}

//---------------------------------------------------------------------------------------
void ImoSlur::reorganize_after_object_deletion()
{
    //Nothing to do. As a slur involves only two objects, the slur is removed when
    //one of the notes is deleted. Also, in note destructor, the other note is informed.
}



//=======================================================================================
// ImoSlurData implementation
//=======================================================================================
ImoSlurData::ImoSlurData(ImoSlurDto* pDto)
    : ImoRelDataObj(k_imo_slur_data)
    , m_fStart( pDto->is_start() )
    , m_slurNum( pDto->get_slur_number() )
    , m_orientation(k_orientation_default)
{
    if (pDto->get_bezier())
        append_child_imo( ImFactory::clone(pDto->get_bezier()) );

}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoSlurData::add_bezier()
{
    ImoBezierInfo* pBezier = get_bezier();
    if (!pBezier)
    {
        pBezier = static_cast<ImoBezierInfo*>(
                        ImFactory::inject(k_imo_bezier_info, m_pDocModel) );
        append_child_imo(pBezier);
    }
    return pBezier;
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoSlurData::get_bezier()
{
    return dynamic_cast<ImoBezierInfo*>(get_first_child());
}


//=======================================================================================
// ImoSlurDto implementation
//=======================================================================================
ImoSlurDto::~ImoSlurDto()
{
    delete m_pBezier;
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoSlurDto::get_staffobj()
{
    return m_pNote;
}


//=======================================================================================
// ImoTable implementation
//=======================================================================================
ImoTableHead* ImoTable::get_head()
{
    return static_cast<ImoTableHead*>( get_child_of_type(k_imo_table_head) );
}

//---------------------------------------------------------------------------------------
ImoTableBody* ImoTable::get_body()
{
    return static_cast<ImoTableBody*>( get_child_of_type(k_imo_table_body) );
}

//---------------------------------------------------------------------------------------
void ImoTable::add_column_style(ImoStyle* pStyle)
{
    m_colStyles.push_back(pStyle ? pStyle->get_id() : k_no_imoid);
}

//---------------------------------------------------------------------------------------
std::list<ImoStyle*> ImoTable::get_column_styles()
{
    std::list<ImoStyle*> styles;
    for (auto id : m_colStyles)
        styles.push_back( get_style_imo(id) );

    return styles;
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoTable::get_style_imo(ImoId id)
{
    if (id != k_no_imoid)
        return dynamic_cast<ImoStyle*>(get_the_document()->get_pointer_to_imo(id));
    else
        return nullptr;
}


//=======================================================================================
// ImoTableCell implementation
//=======================================================================================
ImoTableCell::ImoTableCell()
    : ImoBlocksContainer(k_imo_table_cell)
    , m_rowspan(1)
    , m_colspan(1)
{
}

//---------------------------------------------------------------------------------------
void ImoTableCell::initialize_object()
{
    create_content_container();
}

////---------------------------------------------------------------------------------------
//void ImoTableCell::accept_visitor(BaseVisitor& v)
//{
//    Visitor<ImoTableCell>* vCell = nullptr;
//    Visitor<ImoObj>* vObj = nullptr;
//
//    vCell = dynamic_cast<Visitor<ImoTableCell>*>(&v);
//    if (vCell)
//        vCell->start_visit(this);
//    else
//    {
//        vObj = dynamic_cast<Visitor<ImoObj>*>(&v);
//        if (vObj)
//            vObj->start_visit(this);
//    }
//
//    visit_children(v);
//
//    if (vCell)
//        vCell->end_visit(this);
//    else if (vObj)
//        vObj->end_visit(this);
//}


//=======================================================================================
// ImoTableRow implementation
//=======================================================================================
ImoTableRow::ImoTableRow()
    : ImoBlocksContainer(k_imo_table_row)
{
}

//---------------------------------------------------------------------------------------
void ImoTableRow::initialize_object()
{
    create_content_container();
}

//---------------------------------------------------------------------------------------
ImoStyle* ImoTableRow::get_style(bool fInherit)
{
    ImoStyle* pStyle = get_style_imo();
    if (pStyle)
        return pStyle;
    else
    {
        ImoObj* pParent = get_parent();
        if (pParent && pParent->is_collection())
        {
            pParent = pParent->get_parent();
            if (pParent && pParent->is_table())
                return (static_cast<ImoContentObj*>(pParent))->get_style(fInherit);
            else
            {
                LOMSE_LOG_ERROR("[ImoTableRow::get_style] No parent or table row not in table!");
                throw runtime_error("[ImoTableRow::get_style] No parent or table row not in table!");
            }
        }
        else
        {
            LOMSE_LOG_ERROR("[ImoTableRow::get_style] No parent or table row not in table!");
            throw runtime_error("[ImoTableRow::get_style] No parent or table row not in table!");
        }
    }
}


//=======================================================================================
// ImoTextItem implementation
//=======================================================================================
string& ImoTextItem::get_language()
{
    if (!m_language.empty())
        return m_language;
    else
    {
        ImoDocument* pDoc = get_document();
        if (pDoc)
            return pDoc->get_language();
        else
        {
            LOMSE_LOG_ERROR("[ImoTextItem::get_language] No owner Document.");
            throw runtime_error("[ImoTextItem::get_language] No owner Document.");
        }
    }
}


//=======================================================================================
// ImoTie implementation
//=======================================================================================
ImoNote* ImoTie::get_start_note()
{
    return static_cast<ImoNote*>( get_start_object() );
}

//---------------------------------------------------------------------------------------
ImoNote* ImoTie::get_end_note()
{
    return static_cast<ImoNote*>( get_end_object() );
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTie::get_start_bezier()
{
    ImoTieData* pData = static_cast<ImoTieData*>( get_start_data() );
    return pData->get_bezier();
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTie::get_start_bezier_or_create()
{
    ImoTieData* pData = static_cast<ImoTieData*>( get_start_data() );
    ImoBezierInfo* pInfo = pData->get_bezier();
    if (!pInfo)
        pInfo = pData->add_bezier();
    return pInfo;
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTie::get_stop_bezier()
{
    ImoTieData* pData = static_cast<ImoTieData*>( get_end_data() );
    return pData->get_bezier();
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTie::get_stop_bezier_or_create()
{
    ImoTieData* pData = static_cast<ImoTieData*>( get_end_data() );
    ImoBezierInfo* pInfo = pData->get_bezier();
    if (!pInfo)
        pInfo = pData->add_bezier();
    return pInfo;
}

//---------------------------------------------------------------------------------------
void ImoTie::reorganize_after_object_deletion()
{
    //Nothing to do. As a tie involves only two objects, the tie is removed when
    //one of the notes is deleted. Also, in note destructor, the other note is informed.
}



//=======================================================================================
// ImoTieData implementation
//=======================================================================================
ImoTieData::ImoTieData(ImoTieDto* pDto)
    : ImoRelDataObj(k_imo_tie_data)
    , m_fStart( pDto->is_start() )
    , m_tieNum( pDto->get_tie_number() )
    , m_orientation(k_orientation_default)
{
    if (pDto->get_bezier())
        append_child_imo( ImFactory::clone(pDto->get_bezier()) );
}

//---------------------------------------------------------------------------------------
ImoTieData::ImoTieData()
    : ImoRelDataObj(k_imo_tie_data)
    , m_fStart(false)
    , m_tieNum(0)
    , m_orientation(k_orientation_default)
{
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTieData::add_bezier()
{
    ImoBezierInfo* pBezier = get_bezier();
    if (!pBezier)
    {
        pBezier = static_cast<ImoBezierInfo*>(
                        ImFactory::inject(k_imo_bezier_info, m_pDocModel) );
        append_child_imo(pBezier);
    }
    return pBezier;
}

//---------------------------------------------------------------------------------------
ImoBezierInfo* ImoTieData::get_bezier()
{
    return dynamic_cast<ImoBezierInfo*>(get_first_child());
}


//=======================================================================================
// ImoTieDto implementation
//=======================================================================================
ImoTieDto::~ImoTieDto()
{
    delete m_pBezier;
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoTieDto::get_staffobj()
{
    return m_pNote;
}


//=======================================================================================
// ImoTimeSignature implementation
//=======================================================================================
bool ImoTimeSignature::is_compound_meter()
{
    //In compound time signatures, the beat is broken down into three equal parts,
    //so that a dotted note (half again longer than a regular note) becomes the
    //beat unit. The top number is always divisible by 3, with the exception
    //of time signatures where the top number is 3. Common examples of compound
    //time signatures are 6/8, 12/8, and 9/4.

    return m_top==6 || m_top==9 || m_top==12;
}

//---------------------------------------------------------------------------------------
int ImoTimeSignature::get_num_pulses()
{
    //returns the number of pulses (beats or metronome pulses) implied by this TS

    //In simple time signatures, such as  4/4, 3/4, 2/4, 3/8, and 2/2, the number
    //of beats is given by the top number, with the exception of 3/8 which is
    //conducted in one beat.
    //TODO: Any other exception?
    //In compound time signatures (6/x, 12/x, and 9/x) the number of beats is given
    //by dividing the top number by three.
    //TODO: Any exception?

    if (is_compound_meter()
        || (get_top_number() == 3 && get_bottom_number() == 8))
        return m_top / 3;
    else
        return m_top;
}

//---------------------------------------------------------------------------------------
TimeUnits ImoTimeSignature::get_ref_note_duration()
{
    // returns duration (in LDP time units) implied by Time Signature bottom number

    switch(m_bottom)
    {
        case 1:
            return pow(2.0, (10 - k_whole));
        case 2:
            return pow(2.0, (10 - k_half));
        case 4:
            return pow(2.0, (10 - k_quarter));
        case 8:
            return pow(2.0, (10 - k_eighth));
        case 16:
            return pow(2.0, (10 - k_16th));
        default:
            return 64.0;     //compiler happy
    }
}

//---------------------------------------------------------------------------------------
TimeUnits ImoTimeSignature::get_measure_duration()
{
    //TODO Deal with non-standard time signatures

    return m_top * get_ref_note_duration();
}

//---------------------------------------------------------------------------------------
TimeUnits ImoTimeSignature::get_beat_duration()
{
    return get_measure_duration() / get_num_pulses();
}


//=======================================================================================
// ImoTupletDto implementation
//=======================================================================================
ImoTupletDto::ImoTupletDto()
    : ImoSimpleObj(k_imo_tuplet_dto)
    , m_tupletType(ImoTupletDto::k_unknown)
    , m_tupletNum(0)
    , m_nActualNum(0)
    , m_nNormalNum(0)
    , m_nShowBracket(k_yesno_default)
    , m_nPlacement(k_placement_default)
    , m_nShowNumber(ImoTuplet::k_number_actual)
    , m_lineNum(0)
    , m_fOnlyGraphical(false)
    , m_pNR(nullptr)
{
}

//---------------------------------------------------------------------------------------
ImoStaffObj* ImoTupletDto::get_staffobj()
{
    return m_pNR;
}


//=======================================================================================
// ImoTuplet implementation
//=======================================================================================
ImoTuplet::ImoTuplet(ImoTupletDto* dto)
    : ImoRelObj(k_imo_tuplet)
    , m_nActualNum(dto->get_actual_number())
    , m_nNormalNum(dto->get_normal_number())
    , m_nShowBracket(dto->get_show_bracket())
    , m_nShowNumber(dto->get_show_number())
    , m_nPlacement(dto->get_placement())
{
}

//---------------------------------------------------------------------------------------
void ImoTuplet::reorganize_after_object_deletion()
{
    //TODO
}


//=======================================================================================
// ImoVoltaBracket implementation
//=======================================================================================
ImoBarline* ImoVoltaBracket::get_start_barline()
{
    return static_cast<ImoBarline*>( get_start_object() );
}

//---------------------------------------------------------------------------------------
ImoBarline* ImoVoltaBracket::get_stop_barline()
{
    return static_cast<ImoBarline*>( get_end_object() );
}

//---------------------------------------------------------------------------------------
void ImoVoltaBracket::reorganize_after_object_deletion()
{
    //Nothing to do. As a volta bracket involves only two objects, the volta bracket
    //is automatically removed when one of the barlines is deleted.
    //Also, in barline destructor, the other barline is informed.
}


//=======================================================================================
// ImoWedge implementation
//=======================================================================================
void ImoWedge::reorganize_after_object_deletion()
{
    //Nothing to do. As a wedge involves only two objects, the wedge is removed when
    //one of the ImoDirections is deleted.
}

//=======================================================================================
// ImoWedgeDto implementation
//=======================================================================================
ImoWedgeDto::~ImoWedgeDto()
{
}


//=======================================================================================
// ImoOctaveShift implementation
//=======================================================================================
void ImoOctaveShift::reorganize_after_object_deletion()
{
    //Nothing to do. As an octave-shift involves only two objects, the wedge is removed when
    //one of the ImoDirections is deleted.
}


//=======================================================================================
// ImoPedalMark implementation
//=======================================================================================
EPlacement ImoPedalMark::get_placement()
{
    ImoObj* pParent = get_parent_imo();

    if (pParent && pParent->is_direction())
        return static_cast<ImoDirection*>(pParent)->get_placement();

    return k_placement_default;
}

//=======================================================================================
// ImoPedalLine implementation
//=======================================================================================
void ImoPedalLine::reorganize_after_object_deletion()
{
}





//=======================================================================================
// global functions related to notes
//=======================================================================================
int to_note_type(const char& letter)
{
    //  USA           UK                      ESP               LDP     NoteType
    //  -----------   --------------------    -------------     ---     ---------
    //  long          longa                   longa             l       k_longa = 0
    //  double whole  breve                   cuadrada, breve   b       k_breve = 1
    //  whole         semibreve               redonda           w       k_whole = 2
    //  half          minim                   blanca            h       k_half = 3
    //  quarter       crochet                 negra             q       k_quarter = 4
    //  eighth        quaver                  corchea           e       k_eighth = 5
    //  sixteenth     semiquaver              semicorchea       s       k_16th = 6
    //  32nd          demisemiquaver          fusa              t       k_32nd = 7
    //  64th          hemidemisemiquaver      semifusa          i       k_64th = 8
    //  128th         semihemidemisemiquaver  garrapatea        o       k_128th = 9
    //  256th         ???                     semigarrapatea    f       k_256th = 10

    switch (letter)
    {
        case 'l':     return k_longa;
        case 'b':     return k_breve;
        case 'w':     return k_whole;
        case 'h':     return k_half;
        case 'q':     return k_quarter;
        case 'e':     return k_eighth;
        case 's':     return k_16th;
        case 't':     return k_32nd;
        case 'i':     return k_64th;
        case 'o':     return k_128th;
        case 'f':     return k_256th;
        default:
            return -1;
    }
}

//---------------------------------------------------------------------------------------
NoteTypeAndDots ldp_duration_to_components(const string& duration)
{
    // Return struct with noteType and dots.
    // If error, noteType is set to unknown and dots to zero

    size_t size = duration.length();
    if (size == 0)
        return NoteTypeAndDots(k_unknown_notetype, 0);   //error

    //duration
    int noteType = to_note_type(duration[0]);
    if (noteType == -1)
        return NoteTypeAndDots(k_unknown_notetype, 0);   //error

    //dots
    int dots = 0;
    for (size_t i=1; i < size; i++)
    {
        if (duration[i] == '.')
            dots++;
        else
            return NoteTypeAndDots(k_unknown_notetype, 0);   //error
    }

    return NoteTypeAndDots(noteType, dots);   //no error
}

//---------------------------------------------------------------------------------------
TimeUnits to_duration(int nNoteType, int nDots)
{
    //compute duration without modifiers
    //TimeUnits rDuration = pow(2.0, (10 - nNoteType));
    //Removed: pow not safe
    //      Valgrind: Conditional jump or move depends on uninitialised value(s)
    //                ==8126==    at 0x4140BBF: __ieee754_pow (e_pow.S:118)
    TimeUnits rDuration = 1.0;
    switch (nNoteType)
    {
        case k_longa:   rDuration=1024.0; break;    //  0
        case k_breve:   rDuration=512.0;  break;    //  1
        case k_whole:   rDuration=256.0;  break;    //  2
        case k_half:    rDuration=128.0;  break;    //  3
        case k_quarter: rDuration=64.0;   break;    //  4
        case k_eighth:  rDuration=32.0;   break;    //  5
        case k_16th:    rDuration=16.0;   break;    //  6
        case k_32nd:    rDuration=8.0;    break;    //  7
        case k_64th:    rDuration=4.0;    break;    //  8
        case k_128th:   rDuration=2.0;    break;    //  9
        case k_256th:   rDuration=1.0;    break;    //  10
        default:
            rDuration=64.0;
    }

    //take dots into account
    switch (nDots)
    {
        case 0:                            break;
        case 1: rDuration *= 1.5;          break;
        case 2: rDuration *= 1.75;         break;
        case 3: rDuration *= 1.875;        break;
        case 4: rDuration *= 1.9375;       break;
        case 5: rDuration *= 1.96875;      break;
        case 6: rDuration *= 1.984375;     break;
        case 7: rDuration *= 1.9921875;    break;
        case 8: rDuration *= 1.99609375;   break;
        case 9: rDuration *= 1.998046875;  break;
        default:
            ;
            //wxLogMessage(_T("[to_duration] Program limit: do you really need more than nine dots?"));
    }

    return rDuration;
}
///@endcond

}  //namespace lomse
