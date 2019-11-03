//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#include "lomse_mxl_analyser.h"

#include "lomse_xml_parser.h"
#include "lomse_ldp_exporter.h"
#include "lomse_ldp_factory.h"
#include "lomse_tree.h"
#include "lomse_xml_parser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_im_figured_bass.h"
#include "lomse_ldp_elements.h"
#include "lomse_linker.h"
#include "lomse_injectors.h"
#include "lomse_events.h"
#include "lomse_im_factory.h"
#include "lomse_document.h"
#include "lomse_image_reader.h"
#include "lomse_score_player_ctrl.h"
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_time.h"
#include "lomse_autobeamer.h"
#include "lomse_im_attributes.h"


#include <iostream>
#include <sstream>
//BUG: In my Ubuntu box next line causes problems since approx. 20/march/2011
#if (LOMSE_PLATFORM_WIN32 == 1)
    #include <locale>
#endif
#include <vector>
#include <algorithm>   // for find
#include <regex>
using namespace std;


namespace lomse
{

//=======================================================================================
// PartList implementation: helper class to save part-list info
//=======================================================================================
PartList::PartList()
    : m_numInstrs(0)
    , m_fInstrumentsAdded(false)
{
}

//---------------------------------------------------------------------------------------
PartList::~PartList()
{
    if (!m_fInstrumentsAdded)
    {
        for (int i=0; i < m_numInstrs; ++i)
            delete m_instruments[i];
    }
    m_instruments.clear();
    m_locators.clear();
    m_partAdded.clear();
}

//---------------------------------------------------------------------------------------
void PartList::add_score_part(const string& id, ImoInstrument* pInstrument)
{
    m_locators[id] = m_numInstrs++;
    m_instruments.push_back(pInstrument);
    m_partAdded.push_back(false);
}

//---------------------------------------------------------------------------------------
bool PartList::mark_part_as_added(const string& id)
{
    int i = find_index_for(id);
    if (i == -1)
    {
        LOMSE_LOG_ERROR("Logic error. Part %s does not exist", id.c_str());
        return true;    //error: instrument does not exist
    }
    if (m_partAdded[i])
    {
        LOMSE_LOG_ERROR("Logic error. Part %s is already marked!", id.c_str());
        return true;    //error: instrument is already marked!
    }

    m_partAdded[i] = true;
    return false;
}

//---------------------------------------------------------------------------------------
ImoInstrument* PartList::get_instrument(const string& id)
{
	int i = find_index_for(id);
	return (i != -1 ? m_instruments[i] : nullptr);
}

//---------------------------------------------------------------------------------------
int PartList::find_index_for(const string& id)
{
	map<string, int>::const_iterator it = m_locators.find(id);
	return (it != m_locators.end() ? it->second : -1);
}

//---------------------------------------------------------------------------------------
void PartList::add_all_instruments(ImoScore* pScore)
{
    m_fInstrumentsAdded = true;
    for (int i=0; i < m_numInstrs; ++i)
        pScore->add_instrument(m_instruments[i]);
}

//---------------------------------------------------------------------------------------
void PartList::check_if_missing_parts(ostream& reporter)
{
    map<string, int>::const_iterator it;
    for (it = m_locators.begin(); it != m_locators.end(); ++it)
    {
        if (!m_partAdded[it->second])
        {
            reporter << "Error: missing <part> for <score-part id='"
                     << it->first << "'>." << endl;
        }
    }
}


//=======================================================================================
// PartGroups implementation: helper class to manage open <part-group> tags
//=======================================================================================
PartGroups::PartGroups()
{
}

//---------------------------------------------------------------------------------------
PartGroups::~PartGroups()
{
    map<int, ImoInstrGroup*>::iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
        delete it->second;

    m_groups.clear();
}

//---------------------------------------------------------------------------------------
void PartGroups::add_instrument_to_groups(ImoInstrument* pInstr)
{
    map<int, ImoInstrGroup*>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        ImoInstrGroup* pGrp = it->second;
        pGrp->add_instrument(pInstr);
    }
}

//---------------------------------------------------------------------------------------
void PartGroups::start_group(int number, ImoInstrGroup* pGrp)
{
    m_groups[number] = pGrp;
}

//---------------------------------------------------------------------------------------
void PartGroups::terminate_group(int number)
{
    map<int, ImoInstrGroup*>::iterator it = m_groups.find(number);
	if (it == m_groups.end())
        return;

    ImoInstrGroup* pGrp = it->second;
    if (pGrp->join_barlines() != ImoInstrGroup::k_no)
        set_barline_layout_in_instruments(pGrp);

    m_groups.erase(it);
}

//---------------------------------------------------------------------------------------
bool PartGroups::group_exists(int number)
{
    map<int, ImoInstrGroup*>::const_iterator it = m_groups.find(number);
	return (it != m_groups.end());
}

//---------------------------------------------------------------------------------------
ImoInstrGroup* PartGroups::get_group(int number)
{
    map<int, ImoInstrGroup*>::iterator it = m_groups.find(number);
	if (it != m_groups.end())
        return it->second;
    else
        return nullptr;

}

//---------------------------------------------------------------------------------------
void PartGroups::check_if_all_groups_are_closed(ostream& reporter)
{
    map<int, ImoInstrGroup*>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        reporter << "Error: missing <part-group type='stop'> for <part-group> number='"
                 << it->first << "'." << endl;
    }
}

//---------------------------------------------------------------------------------------
void PartGroups::set_barline_layout_in_instruments(ImoInstrGroup* pGrp)
{
    int layout = (pGrp->join_barlines() == ImoInstrGroup::k_standard
                    ? ImoInstrument::k_joined
                    : ImoInstrument::k_mensurstrich);

    ImoInstrument* pLastInstr = pGrp->get_last_instrument();
    list<ImoInstrument*>& instrs = pGrp->get_instruments();
    list<ImoInstrument*>::iterator it;
    for (it = instrs.begin(); it != instrs.end(); ++it)
    {
        if (*it != pLastInstr)
            (*it)->set_barline_layout(layout);
        else if (layout == ImoInstrument::k_mensurstrich)
            (*it)->set_barline_layout(ImoInstrument::k_nothing);
    }
}


//=======================================================================================
// Enum to assign a int to each valid MusicXML element
enum EMxlTag
{
    k_mxl_tag_undefined = -1,

    k_mxl_tag_accordion_registration,
    k_mxl_tag_articulations,
    k_mxl_tag_attributes,
    k_mxl_tag_backup,
    k_mxl_tag_barline,
    k_mxl_tag_bracket,
    k_mxl_tag_clef,
    k_mxl_tag_coda,
    k_mxl_tag_damp,
    k_mxl_tag_damp_all,
    k_mxl_tag_dashes,
    k_mxl_tag_direction,
    k_mxl_tag_direction_type,
    k_mxl_tag_dynamics,
    k_mxl_tag_ending,
    k_mxl_tag_eyeglasses,
    k_mxl_tag_fermata,
    k_mxl_tag_forward,
    k_mxl_tag_harp_pedals,
    k_mxl_tag_image,
    k_mxl_tag_key,
    k_mxl_tag_lyric,
    k_mxl_tag_measure,
    k_mxl_tag_metronome,
    k_mxl_tag_midi_device,
    k_mxl_tag_midi_instrument,
    k_mxl_tag_notations,
    k_mxl_tag_note,
    k_mxl_tag_octave_shift,
    k_mxl_tag_ornaments,
    k_mxl_tag_part,
    k_mxl_tag_part_group,
    k_mxl_tag_part_list,
    k_mxl_tag_part_name,
    k_mxl_tag_pedal,
    k_mxl_tag_percussion,
    k_mxl_tag_pitch,
    k_mxl_tag_principal_voice,
    k_mxl_tag_print,
    k_mxl_tag_rehearsal,
    k_mxl_tag_rest,
    k_mxl_tag_scordatura,
    k_mxl_tag_score_instrument,
    k_mxl_tag_score_part,
    k_mxl_tag_score_partwise,
    k_mxl_tag_segno,
    k_mxl_tag_slur,
    k_mxl_tag_sound,
    k_mxl_tag_string_mute,
    k_mxl_tag_technical,
    k_mxl_tag_text,
    k_mxl_tag_tied,
    k_mxl_tag_time,
    k_mxl_tag_time_modification,
    k_mxl_tag_tuplet,
    k_mxl_tag_tuplet_actual,
    k_mxl_tag_tuplet_normal,
    k_mxl_tag_virtual_instr,
    k_mxl_tag_wedge,
    k_mxl_tag_words,
};


//=======================================================================================
// Helper class MxlElementAnalyser.
// Abstract class: any element analyser must derive from it

class MxlElementAnalyser
{
protected:
    ostream& m_reporter;
    MxlAnalyser* m_pAnalyser;
    LibraryScope& m_libraryScope;
    LdpFactory* m_pLdpFactory;
    ImoObj* m_pAnchor;

public:
    MxlElementAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor=nullptr)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_libraryScope(libraryScope)
        , m_pLdpFactory(libraryScope.ldp_factory())
        , m_pAnchor(pAnchor) {}
    virtual ~MxlElementAnalyser() {}
    ImoObj* analyse_node(XmlNode* pNode);
    bool analyse_node_bool(XmlNode* pNode);

protected:

    //analysis
    virtual ImoObj* do_analysis() = 0;
    virtual bool do_analysis_bool() { return false; }

    //error reporting
    bool error_missing_element(const string& tag);
    void report_msg(int numLine, const std::string& msg);
    void report_msg(int numLine, const std::stringstream& msg);
    bool error_if_more_elements();
    void error_invalid_child();
    void error_msg(const string& msg);
    void error_msg2(const string& msg);

    //helpers, to simplify writing grammar rules
    XmlNode m_analysedNode;
    XmlNode m_childToAnalyse;
    XmlNode m_nextParam;
    XmlNode m_nextNextParam;

    // the main method to perform the analysis of a node
    inline ImoObj* analyse_child() { return m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr); }

    // 'get' methods just update m_childToAnalyse to point to the next node to analyse
    bool get_mandatory(const string& tag);
    bool get_optional(const string& type);

    // 'analyse' methods do a 'get' and, if found, analyse the found element
    bool analyse_mandatory(const string& tag, ImoObj* pAnchor=nullptr);
    bool analyse_optional(const string& name, ImoObj* pAnchor=nullptr);
    string analyze_mandatory_child_pcdata(const string& name);
    string analyze_optional_child_pcdata(const string& name, const string& sDefault);
    int analyze_optional_child_pcdata_int(const string& name,
                                          int nMin, int nMax, int nDefault);
    float analyze_optional_child_pcdata_float(const string& name,
                                              float rMin, float rMax, float rDefault);

    //methods to analyse attributes of current node
    bool has_attribute(const string& name);
    string get_attribute(const string& name);
    int get_attribute_as_integer(const string& name, int nNumber);
    float get_attribute_as_float(const string& name, float rDefault);
    string get_mandatory_string_attribute(const string& name, const string& sDefault,
                                          const string& element);
    string get_optional_string_attribute(const string& name, const string& sDefault);
    int get_mandatory_integer_attribute(const string& name, int nDefault,
                                        const string& element);
    int get_optional_int_attribute(const string& name, int nDefault);
    bool get_optional_yes_no_attribute(const string& name, bool fDefault);
    float get_optional_float_attribute(const string& name, float rDefault);

    //methods to get value of current node
    int get_cur_node_value_as_integer(int nDefault);

    //methods to get value of current child
    int get_child_pcdata_int(const string& name, int nMin, int nMax, int nDefault);
    float get_child_pcdata_float(const string& name, float rMin, float rMax, float rDefault);

    //building the model
    void add_to_model(ImoObj* pImo, int type=-1);

    //auxiliary
    inline const string& get_document_locator() {
        return m_pAnalyser->get_document_locator();
    }

    int get_line_number()
    {
        return m_pAnalyser->get_line_number(&m_analysedNode);
    }



    //-----------------------------------------------------------------------------------
    //XmlNode helper methods
    inline bool has_attribute(XmlNode* node, const string& name)
    {
        return node->attribute(name.c_str()) != nullptr;
    }
    inline string get_attribute(XmlNode* node, const string& name)
    {
        XmlAttribute attr = node->attribute(name.c_str());
        return string( attr.value() );
    }

    //-----------------------------------------------------------------------------------
    inline bool more_children_to_analyse() {
        return !m_nextParam.is_null();
    }

    //-----------------------------------------------------------------------------------
    inline XmlNode get_child_to_analyse() {
        return m_nextParam;
    }

    //-----------------------------------------------------------------------------------
    inline void move_to_next_child() {
        m_nextParam = m_nextNextParam;
        prepare_next_one();
    }

    //-----------------------------------------------------------------------------------
    inline void prepare_next_one() {
        if (!m_nextParam.is_null())
            m_nextNextParam = m_nextParam.next_sibling();
        else
            m_nextNextParam = XmlNode();
    }

    //-----------------------------------------------------------------------------------
    inline void move_to_first_child() {
        m_nextParam = m_analysedNode.first_child();
        prepare_next_one();
    }

    //-----------------------------------------------------------------------------------
    bool is_long_value()
    {
        string number = m_childToAnalyse.value();
        long nNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> nNumber).fail());
    }

    //-----------------------------------------------------------------------------------
    long get_child_value_long(long nDefault=0L)
    {
        string number = m_childToAnalyse.value();
        long nNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> nNumber).fail())
        {
            stringstream replacement;
            replacement << nDefault;
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid integer number '" + number + "'. Replaced by '"
                + replacement.str() + "'.");
            return nDefault;
        }
        else
            return nNumber;
    }

    //-----------------------------------------------------------------------------------
    int get_child_value_integer(int nDefault)
    {
        return static_cast<int>( get_child_value_long(static_cast<int>(nDefault)) );
    }

    //-----------------------------------------------------------------------------------
    bool is_float_value()
    {
        string number = m_childToAnalyse.value();
        float rNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> rNumber).fail());
    }

    //-----------------------------------------------------------------------------------
    float get_child_value_float(float rDefault=0.0f)
    {
        string number = m_childToAnalyse.value();
        float rNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> rNumber).fail())
        {
            stringstream replacement;
            replacement << rDefault;
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid real number '" + number + "'. Replaced by '"
                + replacement.str() + "'.");
            return rDefault;
        }
        else
            return rNumber;
    }

    //-----------------------------------------------------------------------------------
    bool is_bool_value()
    {
        string value = string(m_childToAnalyse.value());
        return  value == "true" || value == "yes"
             || value == "false" || value == "no" ;
    }

    //-----------------------------------------------------------------------------------
    bool get_child_value_bool(bool fDefault=false)
    {
        string value = string(m_childToAnalyse.value());
        if (value == "true" || value == "yes")
            return true;
        else if (value == "false" || value == "no")
            return false;
        else
        {
            stringstream replacement;
            replacement << fDefault;
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid boolean value '" + value + "'. Replaced by '"
                + replacement.str() + "'.");
            return fDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    int get_child_value_yes_no(int nDefault)
    {
        string value = m_childToAnalyse.value();
        if (value == "yes")
            return k_yesno_yes;
        else if (value == "no")
            return k_yesno_no;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid yes/no value '" + value + "'. Replaced by default.");
            return nDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    string get_child_value_string()
    {
        return m_childToAnalyse.value();
    }


    //-----------------------------------------------------------------------------------
    // Analysers for common attributes
    //-----------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------
    //@ % tenths
    //@ The tenths entity is a number representing tenths. Both integer and decimal
    //@ values are allowed, such as 5 for a half space and -2.5
    //@<!ENTITY % tenths "CDATA">
    Tenths get_attribute_as_tenths(const string& name, Tenths rDefault)
    {
        if (has_attribute(&m_analysedNode, name))
        {
            string number = m_analysedNode.attribute_value(name);
            float rNumber;
            std::istringstream iss(number);
            if ((iss >> std::dec >> rNumber).fail())
            {
                stringstream replacement;
                replacement << rDefault;
                report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                    "Invalid real number '" + number + "'. Replaced by '"
                    + replacement.str() + "'.");
                return rDefault;
            }
            else
                return rNumber;
        }
        else
            return rDefault;
    }

    //-----------------------------------------------------------------------------------
    //@ % placement
    //@ The placement attribute indicates whether something is
    //@ above or below another element, such as a note or anotation.
    //@<!ENTITY % placement
    //@    "placement %above-below; #IMPLIED">
    int get_attribute_placement()
    {
        if (has_attribute(&m_analysedNode, "placement"))
        {
            string value = m_analysedNode.attribute_value("placement");
            if (value == "above")
                return k_placement_above;
            else if (value == "below")
                return k_placement_below;
            else
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Unknown placement attrib. '" + value + "'. Ignored.");
                return k_placement_default;
            }
        }
        else
            return k_placement_default;
    }

    //-----------------------------------------------------------------------------------
    //@ % text-formatting
    //@ The text-formatting entity contains the common formatting attributes for text
    //@ elements. Default values may differ across the elements that use this entity.
    //@
    //@<!ENTITY % text-formatting
    //@    "%justify;
    //@     %print-style-align;   <------------
    //@     %text-decoration;
    //@     %text-rotation;
    //@     %letter-spacing;
    //@     %line-height;
    //@     xml:lang NMTOKEN #IMPLIED
    //@     xml:space (default | preserve) #IMPLIED
    //@     %text-direction;
    //@     %enclosure;">
    //
    void get_attributes_for_text_formatting(ImoObj* pImo)
    {
        //TODO
        //get_attributes_for_justify(pImo);
        get_attributes_for_print_style_align(pImo);
        //get_attributes_for_text_decoration(pImo);
        //get_attributes_for_text_rotation(pImo);
        //get_attributes_for_letter_spacing(pImo);
        //get_attributes_for_line_height(pImo);
        //get_attributes_for_text_direction(pImo);
        //get_attributes_for_enclosure(pImo);
        //get_attributes_for_xml_lang(pImo);
        //get_attributes_for_xml_space(pImo);
    }

    //-----------------------------------------------------------------------------------
    //@ % print-style-align
    //@ The print-style-align entity adds the halign and valign attributes to the
    //@ position, font, and color attributes.
    //@
    //@<!ENTITY % print-style-align
    //@    "%print-style;
    //@     %halign;
    //@     %valign;">
    //
    void get_attributes_for_print_style_align(ImoObj* pImo)
    {
        get_attributes_for_print_style(pImo);
        //TODO
        //get_attributes_for_halign(pImo);
        //get_attributes_for_valign(pImo);
    }

    //-----------------------------------------------------------------------------------
    //@ % print-style
    //@ The print-style entity groups together the most popular combination of
    //@ printing attributes: position, font, and color.
    //@
    //@<!ENTITY % print-style
    //@    "%position;
    //@     %font;
    //@     %color;">
    //
    void get_attributes_for_print_style(ImoObj* pImo)
    {
        get_attributes_for_position(pImo);
        //TODO
        //get_attributes_for_font(pImo);
        get_attribute_color(pImo);
    }

    //-----------------------------------------------------------------------------------
    //@ % position
    //@<!ENTITY % position
    //@    "default-x     %tenths;    #IMPLIED
    //@     default-y     %tenths;    #IMPLIED
    //@     relative-x    %tenths;    #IMPLIED
    //@     relative-y    %tenths;    #IMPLIED">
    //@
    void get_attributes_for_position(ImoObj* pObj)
    {
        if (!pObj || !pObj->is_contentobj())
            return;

        ImoContentObj* pImo = static_cast<ImoContentObj*>(pObj);

        if (has_attribute(&m_analysedNode, "default-x"))
        {
            Tenths pos = get_attribute_as_tenths("default-x", 0.0f);
            if (pos != 0.0f)
                pImo->set_user_ref_point_x(pos);
        }

        if (has_attribute(&m_analysedNode, "default-y"))
        {
            Tenths pos = get_attribute_as_tenths("default-y", 0.0f);
            if (pos != 0.0f)
                //AWARE: positive y is up, negative y is down
                pImo->set_user_ref_point_y(-pos);
        }

        if (has_attribute(&m_analysedNode, "relative-x"))
        {
            Tenths pos = get_attribute_as_tenths("relative-x", 0.0f);
            if (pos != 0.0f)
                pImo->set_user_location_x(pos);
        }

        if (has_attribute(&m_analysedNode, "relative-y"))
        {
            Tenths pos = get_attribute_as_tenths("relative-y", 0.0f);
            if (pos != 0.0f)
                //AWARE: positive y is up, negative y is down
                pImo->set_user_location_y(-pos);
        }
    }

    //-----------------------------------------------------------------------------------
    //@ % font
    //@ - The font-family is a comma-separated list of font names.
    //@   These can be specific font styles such as Maestro or Opus, or one of several
    //@   generic font styles: music, engraved, handwritten, text, serif, sans-serif,
    //@   handwritten, cursive, fantasy, and monospace. The music, engraved, and
    //@   handwritten values refer to music fonts; the rest refer to text fonts.
    //@ - The font-style can be normal or italic.
    //@ - The font-size can be one of the CSS sizes (xx-small, x-small, small, medium,
    //@   large, x-large, xx-large) or a numeric point size.
    //@ - The font-weight can be normal or bold.
    //@
    //@<!ENTITY % font
    //@    "font-family  CDATA  #IMPLIED
    //@     font-style   CDATA  #IMPLIED     can be normal or italic
    //@     font-size    CDATA  #IMPLIED
    //@     font-weight  CDATA  #IMPLIED">
//    ImoStyle* get_attribute_font()
//    {
//        ImoStyle* pStyle = nullptr;pScore->new_unnamed_style();   //derived from default
//        if (has_attribute(&m_childToAnalyse, "font-style"))
//        {
//            string value = get_attribute(&m_childToAnalyse, "font-style");
//            if (!pStyle)
//                pStyle = pScore->new_unnamed_style();   //derived from default
//        }
//        if (has_attribute(&m_childToAnalyse, "font-size"))
//        {
//            string value = get_attribute(&m_childToAnalyse, "font-size");
//            if (!pStyle)
//                pStyle = pScore->new_unnamed_style();   //derived from default
//        }
//        if (has_attribute(&m_childToAnalyse, "font-weight"))
//        {
//            string value = get_attribute(&m_childToAnalyse, "font-weight");
//            if (!pStyle)
//                pStyle = pScore->new_unnamed_style();   //derived from default
//        }
//        if (has_attribute(&m_childToAnalyse, "font-family"))
//        {
//            string value = get_attribute(&m_childToAnalyse, "font-family");
//            if (!pStyle)
//                pStyle = pScore->new_unnamed_style();   //derived from default
//        }
//        if (!pStyle)
//            pStyle = pScore->default();
//          return pStyle;
//    }

    //-----------------------------------------------------------------------------------
    //@ % color
    //@ The color entity indicates the color of an element. Color may be represented:
    //@ - as hexadecimal RGB triples, as in HTML (i.e. "#800080" purple), or
    //@ - as hexadecimal ARGB tuples (i.e. "#40800080" transparent purple).
    //@   Alpha 00 means 'totally transparent'; FF = 'totally opaque'
    //@ If RGB is used, the A value is assumed to be FF
    //@
    //@<!ENTITY % color
    //@    "color CDATA #IMPLIED">
    //
    void get_attribute_color(ImoObj* pImo)
    {
        if (!pImo || !pImo->is_scoreobj())
            return;

        ImoScoreObj* pObj = static_cast<ImoScoreObj*>(pImo);

        if (has_attribute(&m_analysedNode, "color"))
        {
            string value = m_analysedNode.attribute_value("color");
            bool fError = false;
            ImoColorDto color;
            if (value.length() == 7)
                color.set_from_rgb_string(value);
            else if (value.length() == 9)
                color.set_from_argb_string(value);
            else
                fError = true;

            if (fError || !color.is_ok())
                error_msg("Invalid color value. Default color assigned.");
            else
                pObj->set_color( color.get_color() );
        }
    }

//    //-----------------------------------------------------------------------------------
//    float get_font_size_value()
//    {
//        const string value = m_childToAnalyse.value();
//        int size = static_cast<int>(value.size()) - 2;
//        string points = value.substr(0, size);
//        float rNumber;
//        std::istringstream iss(points);
//        if ((iss >> std::dec >> rNumber).fail())
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                "Invalid size '" + value + "'. Replaced by '12'.");
//            return 12.0f;
//        }
//        else
//            return rNumber;
//    }
//<!--
//    In cases where text extends over more than one line,
//    horizontal alignment and justify values can be different.
//    The most typical case is for credits, such as:
//
//        Words and music by
//          Pat Songwriter
//
//    Typically this type of credit is aligned to the right,
//    so that the position information refers to the right-
//    most part of the text. But in this example, the text
//    is center-justified, not right-justified.
//
//    The halign attribute is used in these situations. If it
//    is not present, its value is the same as for the justify
//    attribute.
//-->
//<!ENTITY % halign
//    "halign (left | center | right) #IMPLIED">
//
//<!--
//    The valign entity is used to indicate vertical
//    alignment to the top, middle, bottom, or baseline
//    of the text. Defaults are implementation-dependent.
//-->
//<!ENTITY % valign
//    "valign (top | middle | bottom | baseline) #IMPLIED">
//


    //-----------------------------------------------------------------------------------
    // Analysers for common elements
    //-----------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------
    //@ <staff>
    //@ Staff assignment is only needed for music notated on
    //@ multiple staves. Used by both notes and directions. Staff
    //@ values are numbers, with 1 referring to the top-most staff
    //@ in a part.
    //@
    //@ <!ELEMENT staff (#PCDATA)>
    //
    int analyse_optional_staff(int nDefault)
    {
        if (get_optional("staff"))
            return get_child_value_integer(nDefault);
        else
            return nDefault;
    }

//<!--
//    The text-decoration entity is based on the similar
//    feature in XHTML and CSS. It allows for text to
//    be underlined, overlined, or struck-through. It
//    extends the CSS version by allow double or
//    triple lines instead of just being on or off.
//-->
//<!ENTITY % text-decoration
//    "underline  %number-of-lines;  #IMPLIED
//     overline  %number-of-lines;   #IMPLIED
//     line-through  %number-of-lines;   #IMPLIED">
//
//<!--
//    The justify entity is used to indicate left, center, or
//    right justification. The default value varies for different
//    elements. For elements where the justify attribute is present
//    but the halign attribute is not, the justify attribute
//    indicates horizontal alignment as well as justification.
//-->
//<!ENTITY % justify
//    "justify (left | center | right) #IMPLIED">
//
//<!--
//    The valign-image entity is used to indicate vertical
//    alignment for images and graphics, so it removes the
//    baseline value. Defaults are implementation-dependent.
//-->
//<!ENTITY % valign-image
//    "valign (top | middle | bottom) #IMPLIED">
//
//<!--
//    The letter-spacing entity specifies text tracking.
//    Values are either "normal" or a number representing
//    the number of ems to add between each letter. The
//    number may be negative in order to subtract space.
//    The default is normal, which allows flexibility of
//    letter-spacing for purposes of text justification.
//-->
//<!ENTITY % letter-spacing
//    "letter-spacing CDATA #IMPLIED">
//
//<!--
//    The line-height entity specified text leading. Values
//    are either "normal" or a number representing the
//    percentage of the current font height  to use for
//    leading. The default is "normal". The exact normal
//    value is implementation-dependent, but values
//    between 100 and 120 are recommended.
//-->
//<!ENTITY % line-height
//    "line-height CDATA #IMPLIED">
//
//<!--
//    The text-direction entity is used to adjust and override
//    the Unicode bidirectional text algorithm, similar to the
//    W3C Internationalization Tag Set recommendation. Values
//    are ltr (left-to-right embed), rtl (right-to-left embed),
//    lro (left-to-right bidi-override), and rlo (right-to-left
//    bidi-override). The default value is ltr. This entity
//    is typically used by applications that store text in
//    left-to-right visual order rather than logical order.
//    Such applications can use the lro value to better
//    communicate with other applications that more fully
//    support bidirectional text.
//-->
//<!ENTITY % text-direction
//    "dir (ltr | rtl | lro | rlo) #IMPLIED">
//
//<!--
//    The text-rotation entity is used to rotate text
//    around the alignment point specified by the
//    halign and valign entities. The value is a number
//    ranging from -180 to 180. Positive values are
//    clockwise rotations, while negative values are
//    counter-clockwise rotations.
//-->
//<!ENTITY % text-rotation
//    "rotation CDATA #IMPLIED">
//
//<!--
//    The enclosure entity is used to specify the
//    formatting of an enclosure around text or symbols.
//-->
//<!ENTITY % enclosure
//    "enclosure %enclosure-shape; #IMPLIED">
//


//    //-----------------------------------------------------------------------------------
//    EHAlign get_alignment_value(EHAlign defaultValue)
//    {
//        const std::string& value = m_childToAnalyse.value();
//        if (value == "left")
//            return k_halign_left;
//        else if (value == "right")
//            return k_halign_right;
//        else if (value == "center")
//            return k_halign_center;
//        else
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                    "Invalid alignment value '" + value + "'. Assumed 'center'.");
//            return defaultValue;
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    ImoStyle* get_text_style_child(const string& defaulName="Default style")
//    {
//        m_childToAnalyse = get_child(m_childToAnalyse, 1);
//        string styleName = get_child_value_string();
//        ImoStyle* pStyle = nullptr;
//
//        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
//        if (pScore)
//        {
//            pStyle = pScore->find_style(styleName);
//            if (!pStyle)
//            {
//                //try to find it in document global styles
//                Document* pDoc = m_pAnalyser->get_document_being_analysed();
//                ImoDocument* pImoDoc = pDoc->get_im_root();
//                if (pImoDoc)
//                    pStyle = pImoDoc->find_style(styleName);
//            }
//            if (!pStyle)
//            {
//                report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                        "Style '" + styleName + "' is not defined. Default style will be used.");
//                pStyle = pScore->get_style_or_default(defaulName);
//            }
//        }
//
//        return pStyle;
//    }
//
//    //-----------------------------------------------------------------------------------
//    TPoint get_point_child()
//    {
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
//        TPoint point;
//        if (pImo->is_point_dto())
//        {
//            ImoPointDto* pPoint = static_cast<ImoPointDto*>( pImo );
//            point = pPoint->get_point();
//        }
//        delete pImo;
//        return point;
//    }
//
//    //-----------------------------------------------------------------------------------
//    TSize get_size_child()
//    {
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
//        TSize size;
//        if (pImo->is_size_info())
//        {
//            ImoSizeDto* pSize = static_cast<ImoSizeDto*>( pImo );
//            size = pSize->get_size();
//        }
//        delete pImo;
//        return size;
//    }

    //-----------------------------------------------------------------------------------
    // Auxiliary methods
    //-----------------------------------------------------------------------------------

    int to_note_type(const string& type)
    {
        int noteType = k_unknown_notetype;

        if (type == "quarter")
            noteType = k_quarter;
        else if (type == "eighth")
            noteType = k_eighth;
        else if (type == "16th")
            noteType = k_16th;
        else if (type == "half")
            noteType = k_half;
        else if (type == "32nd")
            noteType = k_32nd;
        else if (type == "64th")
            noteType = k_64th;
        else if (type == "whole")
            noteType = k_whole;
        else if (type == "long")
            noteType = k_longa;
        else if (type == "128th")
            noteType = k_128th;
        else if (type == "256th")
            noteType = k_256th;
        else if (type == "breve")
            noteType = k_breve;
//        else if (type == "512th")
//            noteType = k_512th;
//        else if (type == "1024th")
//            noteType = k_1024th;
//        else if (type == "maxima")
//            noteType = k_maxima;
        else
        {
            error_msg2(
                "Invalid or not supported <type> value '" + type + "'. Replaced by 'eighth'.");
            noteType = k_eighth;
        }
        return noteType;
    }

};



//=======================================================================================
// MxlElementAnalyser implementation
//=======================================================================================
ImoObj* MxlElementAnalyser::analyse_node(XmlNode* pNode)
{
    m_analysedNode = *pNode;
    move_to_first_child();
    return do_analysis();
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::analyse_node_bool(XmlNode* pNode)
{
    m_analysedNode = *pNode;
    move_to_first_child();
    return do_analysis_bool();
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::error_missing_element(const string& tag)
{
    string parentName = m_analysedNode.name();
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
               "<" + parentName + ">: missing mandatory element <" + tag + ">.");
    return false;
}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::report_msg(int numLine, const std::stringstream& msg)
{
    report_msg(numLine, msg.str());
}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::report_msg(int numLine, const std::string& msg)
{
    m_reporter << "Line " << numLine << ". " << msg << endl;
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::has_attribute(const string& name)
{
    return has_attribute(&m_analysedNode, name);
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::get_attribute(const string& name)
{
    return m_analysedNode.attribute_value(name);
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::get_mandatory_string_attribute(const string& name,
                                  const string& sDefault, const string& element)
{
    string attrb = sDefault;
    if (has_attribute(&m_analysedNode, name))
        attrb = m_analysedNode.attribute_value(name);
    else if (sDefault.empty())
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            element + ": missing mandatory attribute '" + name + "'." );
    else
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            element + ": missing mandatory attribute '" + name + "'. Value '"
            + sDefault + "' assumed.");

    return attrb;
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::get_optional_string_attribute(const string& name,
                                                         const string& sDefault)
{
    if (has_attribute(&m_analysedNode, name))
        return m_analysedNode.attribute_value(name);
    else
        return sDefault;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_attribute_as_integer(const string& name, int nDefault)
{
    string number = m_analysedNode.attribute_value(name);
    long nNumber;
    std::istringstream iss(number);
    if ((iss >> std::dec >> nNumber).fail())
        return nDefault;
    else
        return int(nNumber);
}

//---------------------------------------------------------------------------------------
float MxlElementAnalyser::get_attribute_as_float(const string& name, float rDefault)
{
    string number = m_analysedNode.attribute_value(name);
    float rNumber;
    bool fError = false;
    try
    {
        size_t sz;
        rNumber = std::stof(number, &sz);
        fError = (number.size() != sz);
    }
    catch (...)
    {
        fError = true;
    }

    if (fError)
    {
        stringstream replacement;
        replacement << rDefault;
        report_msg(get_line_number(),
            "Invalid real number '" + number + "'. Replaced by '"
            + replacement.str() + "'.");
        return rDefault;
    }
    else
        return rNumber;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_optional_int_attribute(const string& name,
                                                   int nDefault)
{
    if (has_attribute(&m_analysedNode, name))
        return get_attribute_as_integer(name, nDefault);
    else
        return nDefault;
}

//---------------------------------------------------------------------------------------
float MxlElementAnalyser::get_optional_float_attribute(const string& name,
                                                       float rDefault)
{
    if (has_attribute(&m_analysedNode, name))
        return get_attribute_as_float(name, rDefault);
    else
        return rDefault;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_mandatory_integer_attribute(const string& name, int nDefault,
                                                        const string& element)
{
    int attrb = nDefault;
    if (has_attribute(&m_analysedNode, name))
        attrb = get_attribute_as_integer(name, nDefault);
    else
    {
        stringstream replacement;
        replacement << nDefault;
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            element + ": missing mandatory attribute '" + name + "'. Value '"
            + replacement.str() + "' assumed.");
    }

    return attrb;
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::get_optional_yes_no_attribute(const string& name, bool fDefault)
{
    if (has_attribute(&m_analysedNode, name))
    {
        string value = m_analysedNode.attribute_value(name);
        if (value == "yes")
            return true;
        else if (value == "no")
            return false;
        else
        {

            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                m_analysedNode.name() + ": invalid value for yes-no attribute '"
                + name + "'. Value '" + (fDefault ? "yes" : "no") + "' assumed.");
            return fDefault;
        }
    }
    else
        return fDefault;
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::get_mandatory(const string& tag)
{
    if (!more_children_to_analyse())
    {
        error_missing_element(tag);
        return false;
    }

    m_childToAnalyse = get_child_to_analyse();
    if (m_childToAnalyse.name() != tag)
    {
        error_missing_element(tag);
        return false;
    }

    move_to_next_child();
    return true;
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::analyse_mandatory(const string& tag, ImoObj* pAnchor)
{
    if (get_mandatory(tag))
        return (m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor) != nullptr);
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::get_optional(const string& name)
{
    if (more_children_to_analyse())
    {
        m_childToAnalyse = get_child_to_analyse();
        if (m_childToAnalyse.name() == name)
        {
            move_to_next_child();
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::analyse_optional(const string& name, ImoObj* pAnchor)
{
    if (get_optional(name))
    {
        m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::analyze_mandatory_child_pcdata(const string& name)
{
    if (get_mandatory(name))
    {
        return m_childToAnalyse.value();
    }

	return "";
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::analyze_optional_child_pcdata(const string& name,
                                                         const string& sDefault)
{
    if (get_optional(name))
    {
        return m_childToAnalyse.value();
    }

	return sDefault;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::analyze_optional_child_pcdata_int(const string& name,
                                                          int nMin, int nMax,
                                                          int nDefault)
{
    if (get_optional(name))
        return get_child_pcdata_int(name, nMin, nMax, nDefault);
    else
        return nDefault;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_child_pcdata_int(const string& name,
                                             int nMin, int nMax, int nDefault)
{
    bool fError = false;
    string number = m_childToAnalyse.value();
    long nNumber;
    std::istringstream iss(number);
    if ((iss >> std::dec >> nNumber).fail())
        fError = true;
    else
    {
        if (nNumber < nMin || nNumber > nMax)
            fError = true;
    }

    if (fError)
    {
        stringstream range;
        range << nMin << " to " << nMax;
        stringstream sDefault;
        sDefault << nDefault;
        report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
            name + ": invalid value " + number + ". Must be integer in range "
            + range.str() + ". Value " + sDefault.str() + " assumed.");
        return nDefault;

    }
    else
        return nNumber;
}

//---------------------------------------------------------------------------------------
float MxlElementAnalyser::analyze_optional_child_pcdata_float(const string& name,
                                                              float rMin, float rMax,
                                                              float rDefault)
{
    if (get_optional(name))
        return get_child_pcdata_float(name, rMin, rMax, rDefault);
    else
        return rDefault;
}

//---------------------------------------------------------------------------------------
float MxlElementAnalyser::get_child_pcdata_float(const string& name,
                                                 float rMin, float rMax, float rDefault)
{
    bool fError = false;
    string number = m_childToAnalyse.value();
    float rNumber;
    std::istringstream iss(number);
    if ((iss >> rNumber).fail())
        fError = true;
    else
    {
        if (rNumber < rMin || rNumber > rMax)
            fError = true;
    }

    if (fError)
    {
        stringstream range;
        range << rMin << " to " << rMax;
        stringstream sDefault;
        sDefault << rDefault;
        report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
            name + ": invalid value " + number + ". Must be decimal in range "
            + range.str() + ". Value " + sDefault.str() + " assumed.");
        return rDefault;

    }
    else
        return rNumber;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_cur_node_value_as_integer(int nDefault)
{
    string number = m_analysedNode.value();
    long nNumber;
    std::istringstream iss(number);
    if ((iss >> std::dec >> nNumber).fail())
        return nDefault;
    else
        return int(nNumber);
}

////---------------------------------------------------------------------------------------
//void MxlElementAnalyser::analyse_one_or_more(ELdpElement* pValid, int nValid)
//{
//    while(more_children_to_analyse())
//    {
//        m_childToAnalyse = get_child_to_analyse();
//
//        ELdpElement type = get_type(m_childToAnalyse);
//        if (contains(type, pValid, nValid))
//        {
//            move_to_next_child();
//            m_pAnalyser->analyse_node(&m_childToAnalyse);
//        }
//        else
//        {
//            string name = m_childToAnalyse.name();
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                "Element '" + name + "' unknown or not possible here. Ignored.");
//        }
//        move_to_next_child();
//    }
//}
//
////---------------------------------------------------------------------------------------
//bool MxlElementAnalyser::contains(ELdpElement type, ELdpElement* pValid, int nValid)
//{
//    for (int i=0; i < nValid; i++, pValid++)
//        if (*pValid == type) return true;
//    return false;
//}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::error_invalid_child()
{
    string name = m_childToAnalyse.name();
    if (name == "label")
        name += ":" + m_childToAnalyse.value();
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
        "Element '" + name + "' unknown or not possible here. Ignored.");
}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::error_msg(const string& msg)
{
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode), msg);
}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::error_msg2(const string& msg)
{
    error_msg(m_pAnalyser->get_element_info() + msg);
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::error_if_more_elements()
{
    if (more_children_to_analyse())
    {
        string next = m_nextParam.next_sibling().name();
        string name = m_childToAnalyse.name();
        if (name == "label")
            name += ":" + m_childToAnalyse.value();
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Element <" + m_analysedNode.name()
                + ">: too many children. Elements after <"
                + name + "> have been ignored. First ignored: <"
                + next + ">.");
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::add_to_model(ImoObj* pImo, int type)
{
    Linker linker( m_pAnalyser->get_document_being_analysed() );
    linker.add_child_to_model(m_pAnchor, pImo, type == -1 ? pImo->get_obj_type() : type);
}





//---------------------------------------------------------------------------------------
// default analyser to use when there is no defined analyser for an LDP element

class NullMxlAnalyser : public MxlElementAnalyser
{
protected:
    const string m_tag;

public:
    NullMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    const string& tag)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope)
        , m_tag(tag)
        {
        }

    ImoObj* do_analysis()
    {
        error_msg("Missing analyser for element '" + m_tag + "'. Node ignored.");
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <accordion-registration>
class AccordionRegistrationMxlAnalyser : public MxlElementAnalyser
{
public:
    AccordionRegistrationMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <articulations> = (articulations <articulation>+)
//@ <articulation> = [accent | strong-accent | staccato | tenuto |
//@                   detached-legato | staccatissimo | spiccato |
//@                   scoop | plop | doit | falloff | breath-mark |
//@                   caesura | stress | unstress | other-articulation ]
//
// Examples:
//    <articulations>
//        <accent placement="below"/>
//        <tenuto placement="below"/>
//        <staccato placement="above"/>
//    </articulations>
//
//    <articulations><accent/></articulations>

class ArticulationsMxlAnalyser : public MxlElementAnalyser
{
public:
    ArticulationsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoNoteRest* pNR = nullptr;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoNoteRest");
            return nullptr;
        }

        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            if (m_childToAnalyse.name() == "accent")
            {
                get_articulation_symbol(pNR, k_articulation_accent);
            }
            else if (m_childToAnalyse.name() == "staccato")
            {
                get_articulation_symbol(pNR, k_articulation_staccato);
            }
            else if (m_childToAnalyse.name() == "tenuto")
            {
                get_articulation_symbol(pNR, k_articulation_tenuto);
            }
            else if (m_childToAnalyse.name() == "detached-legato")
            {
                get_articulation_symbol(pNR, k_articulation_mezzo_staccato);
            }
            else if (m_childToAnalyse.name() == "staccatissimo")
            {
                get_articulation_symbol(pNR, k_articulation_staccatissimo);
            }
            else if (m_childToAnalyse.name() == "spiccato")
            {
                get_articulation_symbol(pNR, k_articulation_spiccato);
            }
            else if (m_childToAnalyse.name() == "breath-mark")
            {
                get_articulation_breath_mark(pNR);
            }
            else if (m_childToAnalyse.name() == "caesura")
            {
                get_articulation_symbol(pNR, k_articulation_caesura);
            }
            else if (m_childToAnalyse.name() == "stress")
            {
                get_articulation_symbol(pNR, k_articulation_stress);
            }
            else if (m_childToAnalyse.name() == "unstress")
            {
                get_articulation_symbol(pNR, k_articulation_unstress);
            }
            else if (m_childToAnalyse.name() == "strong-accent")
            {
                get_articulation_strong_accent(pNR);
            }
                // articulation line
            else if (m_childToAnalyse.name() == "scoop")
            {
                get_articulation_line(pNR, k_articulation_scoop);
            }
            else if (m_childToAnalyse.name() == "plop")
            {
                get_articulation_line(pNR, k_articulation_plop);
            }
            else if (m_childToAnalyse.name() == "doit")
            {
                get_articulation_line(pNR, k_articulation_doit);
            }
            else if (m_childToAnalyse.name() == "falloff")
            {
                get_articulation_line(pNR, k_articulation_falloff);
            }
            else        //other-articulation
            {
                error_invalid_child();
            }
            move_to_next_child();
        }

        error_if_more_elements();

        return nullptr;
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoArticulationSymbol* get_articulation_symbol(ImoNoteRest* pNR, int type)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoArticulationSymbol* pImo = static_cast<ImoArticulationSymbol*>(
                                ImFactory::inject(k_imo_articulation_symbol, pDoc) );
        pImo->set_articulation_type(type);

        // [attrib]: placement (above | below)
        if (has_attribute(&m_childToAnalyse, "placement"))
            set_placement(pImo);

        pNR->add_attachment(pDoc, pImo);
        return pImo;
    }

    //-----------------------------------------------------------------------------------
    void get_articulation_strong_accent(ImoNoteRest* pNR)
    {
        ImoArticulationSymbol* pImo =
            get_articulation_symbol(pNR, k_articulation_marccato);

        // [attrib]: type (up | down)
        if (has_attribute(&m_childToAnalyse, "type"))
            set_type(pImo);
    }

    //-----------------------------------------------------------------------------------
    void get_articulation_breath_mark(ImoNoteRest* pNR)
    {
        ImoArticulationSymbol* pImo =
            get_articulation_symbol(pNR, k_articulation_breath_mark);

        // [attrib]: type (up | down)
        if (has_attribute(&m_childToAnalyse, "type"))
            set_breath_mark_type(pImo);
    }

    //-----------------------------------------------------------------------------------
    void get_articulation_line(ImoNoteRest* pNR, int type)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoArticulationLine* pImo = static_cast<ImoArticulationLine*>(
                                ImFactory::inject(k_imo_articulation_line, pDoc) );
        pImo->set_articulation_type(type);

        // [attrib]: placement (above | below)
        if (has_attribute(&m_childToAnalyse, "placement"))
            set_placement(pImo);

        //TODO
        //%line-shape;
        //%line-type;
        //%dashed-formatting;


        pNR->add_attachment(pDoc, pImo);
    }

    //-----------------------------------------------------------------------------------
    void set_placement(ImoArticulation* pImo)
    {
        string value = get_attribute(&m_childToAnalyse, "placement");
        if (value == "above")
            pImo->set_placement(k_placement_above);
        else if (value == "below")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown placement attrib. '" + value + "'. Ignored.");
        }
    }

    //-----------------------------------------------------------------------------------
    void set_type(ImoArticulationSymbol* pImo)
    {
        string value = get_attribute(&m_childToAnalyse, "type");
        if (value == "up")
            pImo->set_up(true);
        else if (value == "below")
            pImo->set_up(false);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown type attrib. '" + value + "'. Ignored.");
        }
    }

    //-----------------------------------------------------------------------------------
    void set_breath_mark_type(ImoArticulationSymbol* pImo)
    {
        //The breath-mark element may have a text value to
        //indicate the symbol used for the mark. Valid values are
        //comma, tick, and an empty string.

        string value = m_analysedNode.value();
        if (value == "comma")
            pImo->set_symbol(ImoArticulationSymbol::k_breath_comma);
        else if (value == "tick")
            pImo->set_symbol(ImoArticulationSymbol::k_breath_tick);
        else
            pImo->set_symbol(ImoArticulationSymbol::k_default);
    }
};

//@--------------------------------------------------------------------------------------
//@ <attributes>
//@
//@ The attributes element contains musical information that typically changes
//@ on measure boundaries. This includes key and time signatures, clefs,
//@ transpositions, and staving.
//@
//@ <!ELEMENT attributes (%editorial;, divisions?, key*, time*,
//@     staves?, part-symbol?, instruments?, clef*, staff-details*,
//@     transpose*, directive*, measure-style*)>
//@
//
class AtribbutesMxlAnalyser : public MxlElementAnalyser
{
public:
    AtribbutesMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        {}


    ImoObj* do_analysis()
    {
        //In MusicXML. Clefs, time signatures and key signatures are
        //treated as attributes of a measure, not as objects and, therefore, ordering
        //is not important for MusicXML and this information is
        //coded bad order (first key signatures, then time signatures, then clefs).
        //As Lomse expects that these objects are defined in correct order,
        //objects creation will be delayed until all attributes are parsed.
        vector<ImoObj*> times;
        vector<ImoObj*> keys;
        vector<ImoObj*> clefs;

        //TODO
        // %editorial;

        // divisions?
        if (get_optional("divisions"))
            set_divisions();

        // key*
        while (get_optional("key"))
            keys.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr) );

        // time*
        while (get_optional("time"))
            times.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr) );

        // staves?
        if (get_optional("staves"))
        {
            int staves = get_child_value_integer(1);
            ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>(m_pAnchor->get_parent_imo());
            // coverity[tainted_data]
            for(; staves > 1; --staves)
                pInstr->add_staff();
        }

        // part-symbol?
        if (get_optional("part-symbol"))
        {
            //TODO <part-symbol>
        }

        // instruments?
        if (get_optional("instruments"))
        {
            //TODO <instruments>
        }

        // clef*
        while (get_optional("clef"))
            clefs.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr) );

        // staff-details*
        while (get_optional("staff-details"))
            ; //TODO <staff-details>

        // transpose*
        while (get_optional("transpose"))
            ; //TODO <transpose>

        // directive*
        while (get_optional("directive"))
            ; //TODO <directive>

        // measure-style*
        while (get_optional("measure-style"))
            ; //TODO <measure-style>
//        <measure-style>
//          <multiple-rest>1</multiple-rest>
//        </measure-style>

        error_if_more_elements();

        //add elements to model, in right order
        vector<ImoObj*>::const_iterator it;
        for (it = clefs.begin(); it != clefs.end(); ++it)
        {
            if (*it)
                add_to_model(*it);
        }
        for (it = keys.begin(); it != keys.end(); ++it)
        {
            if (*it)
                add_to_model(*it);
        }
        for (it = times.begin(); it != times.end(); ++it)
        {
            if (*it)
                add_to_model(*it);
        }
        return m_pAnchor;
    }

protected:
    void set_divisions()
    {
        // Musical notation duration is commonly represented as fractions. The divisions
        // element indicates how many divisions per quarter note are used to indicate a
        // note's duration. For example, if duration = 1 and divisions = 2, this is an
        // eighth note duration. Duration and divisions are used directly for generating
        // sound output, so they must be chosen to take tuplets into account. Using a
        // divisions element lets us use just one number to represent a duration for
        // each note in the score, while retaining the full power of a fractional
        // representation. If maximum compatibility with Standard MIDI 1.0 files is
        // important, do not have the divisions value exceed 16383.

        int divisions = get_child_value_integer(4);
        m_pAnalyser->set_current_divisions( float(divisions) );
    }

};

//@--------------------------------------------------------------------------------------
//@ http://www.musicxml.com/for-developers/musicxml-dtd/barline-elements/
//@ <!ELEMENT barline (bar-style?, %editorial;, wavy-line?,
//@     segno?, coda?, (fermata, fermata?)?, ending?, repeat?)>
//@ <!ATTLIST barline
//@     location (right | left | middle) "right"
//@     segno CDATA #IMPLIED
//@     coda CDATA #IMPLIED
//@     divisions CDATA #IMPLIED
//@ >
//@
//@ <barline location="right">
//@     <bar-style>light-heavy</bar-style>
//@     <ending number="1" type="stop"/>
//@     <repeat direction="backward" winged="none"/>
//@ </barline>

class BarlineMxlAnalyser : public MxlElementAnalyser
{
protected:
    bool        m_fNewBarline;
    ImoBarline* m_pBarline;

public:
    BarlineMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_fNewBarline(false)
        , m_pBarline(nullptr)
    {
    }


    ImoObj* do_analysis()
    {
        //ImoMusicData* pMD = dynamic_cast<ImoMusicData*>(m_pAnchor);

            //attributes:

        // attrib: location (right | left | middle) "right"
        string location = get_optional_string_attribute("location", "right");

        // attrib: segno CDATA #IMPLIED
			//TODO

        // attrib: coda CDATA #IMPLIED
			//TODO

        // attrib: divisions CDATA #IMPLIED
			//TODO


            //content:

        //@ bar-style?
        //@ Valid values are regular, dotted, dashed, heavy, light-light,
        //@ light-heavy, heavy-light, heavy-heavy, tick (a
        //@ short stroke through the top line), short (a partial
        //@ barline between the 2nd and 4th lines), and none.
        //@ <!ELEMENT bar-style (#PCDATA)>
        //@ <!ATTLIST bar-style
        //@     %color;
        //@>
        //
        //TODO: proper analysis and validation of <bar-style>
        string barStyle = "";
        if (get_optional("bar-style"))
            barStyle = m_childToAnalyse.value();
        if (barStyle.empty())
            barStyle = "none";

        create_barline(location);

        //TODO
        // %editorial;
        // wavy-line?
        // segno?
        // coda?
        // (fermata, fermata?)?

        // ending?
        analyse_optional("ending", m_pBarline);

        // repeat?
        string repeat = "";
        if (get_optional("repeat"))
            repeat = get_repeat();

        error_if_more_elements();

        EBarline type = find_barline_type(barStyle, repeat);
        combine_barlines(m_pBarline, type);

        if (m_fNewBarline)
        {
            advance_timepos_if_required();
            add_to_model(m_pBarline);
            m_pAnalyser->save_last_barline(m_pBarline);
        }

        return m_pBarline;
    }

protected:

    void create_barline(const string& location)
    {
        m_pBarline = nullptr;
        if (location == "left")
        {
            //must be combined with previous barline
            m_pBarline = m_pAnalyser->get_last_barline();
        }

        m_fNewBarline = false;
        if (m_pBarline == nullptr)
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            m_pBarline = static_cast<ImoBarline*>(
                                ImFactory::inject(k_imo_barline, pDoc) );
            m_pBarline->set_type(k_barline_simple);
            m_fNewBarline = true;
        }
    }

    EBarline find_barline_type(const string& barType, const string& repeat)
    {
        bool fError = false;
        EBarline type = k_barline_simple;

        if (barType == "none")
            type = k_barline_none;
        else if (barType == "regular")
            type = k_barline_simple;
//        else if (barType == "dotted")
//            type = ?
//        else if (barType == "dashed")
//            type = ?
//        else if (barType == "heavy")
//            type = ?
        else if (barType == "light-light")
            type = k_barline_double;
        else if (barType == "light-heavy")
        {
            if (repeat == "backward")
                type = k_barline_end_repetition;
            else if (repeat.empty())
                type = k_barline_end;
            else
                fError = true;
        }
        else if (barType == "heavy-light")
        {
            if (repeat == "forward")
                type = k_barline_start_repetition;
            else if (repeat.empty())
                type = k_barline_start;
            else
                fError = true;
        }
        else if (barType == "heavy-heavy")
        {
            if (repeat == "backward")
                type = k_barline_double_repetition_alt;
            else if (repeat.empty())
                type = k_barline_double;
            else
                fError = true;
        }
//        else if (barType == "tick")   //a short stroke through the top line
//            type = ?
//        else if (barType == "short")  //a partial barline between the 2nd and 4th lines
//            type = ?
//        else if (barType == "none")
//            type =

        else
            fError = true;

        if (fError)
        {
            error_msg2(
                "Invalid or not supported <bar-style> ('" + barType
                + "') and/or <repeat> ('" + repeat
                + "') values. Replaced by 'regular' barline.");
        }

        return type;
    }

    //@ <repeat>
    //@ <!ELEMENT repeat EMPTY>
    //@ <!ATTLIST repeat
    //@     direction (backward | forward) #REQUIRED
    //@     times CDATA #IMPLIED
    //@     winged (none | straight | curved |
    //@         double-straight | double-curved) #IMPLIED
    //@ >
    //
    //TODO: proper analysis of <repeat>
    string get_repeat()
    {
        // attrib: direction (backward | forward) #REQUIRED
        // 		The start of the repeat has a forward direction
        // 		while the end of the repeat has a backward direction.
        string direction = "";
        if (has_attribute(&m_childToAnalyse, "direction"))
            direction = m_childToAnalyse.attribute_value("direction");
        return direction;

        // attrib: times CDATA #IMPLIED
        // 		Backward repeats **that are not part of an ending** can use the times
        // 		attribute to indicate the number of times the repeated section
        // 		is played.
            //TODO

        // attrib: winged (none | straight | curved | double-straight | double-curved) #IMPLIED
        // 		The winged attribute indicates whether the repeat
        //		has winged extensions that appear above and below the barline.
        // 		The straight and curved values represent single wings, while
        // 		the double-straight and double-curved values represent double
        // 		wings. The none value indicates no wings and is the default.
            //TODO
    }

    void combine_barlines(ImoBarline* pBarline, EBarline rightType)
    {
        EBarline type;
        EBarline leftType = EBarline(pBarline->get_type());

        if (leftType == k_barline_simple && rightType == k_barline_simple)
            type = k_barline_double;
        else if (rightType == k_barline_simple || rightType == k_barline_none)
            type = leftType;
        else if (leftType == k_barline_simple)
            type = rightType;
        else if (leftType == k_barline_end && rightType == k_barline_start_repetition)
            type = rightType;
        else if (leftType == k_barline_end_repetition &&
                 rightType == k_barline_start_repetition)
            type = k_barline_double_repetition;
        else
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Barlines combination not supported: left = "
                + LdpExporter::barline_type_to_ldp(leftType)
                + ", right = "
                + LdpExporter::barline_type_to_ldp(rightType)
                + ". Replaced by 'double' barline.");
            type = k_barline_double;
        }
#if 0
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Combining barlines: left = "
                + LdpExporter::barline_type_to_ldp(leftType)
                + ", right = "
                + LdpExporter::barline_type_to_ldp(rightType)
                + ", result = "
                + LdpExporter::barline_type_to_ldp(type)
                );
#endif

        pBarline->set_type(type);
        if (pBarline->get_num_repeats() == 0
            && (type == k_barline_double_repetition || type == k_barline_end_repetition))
        {
            pBarline->set_num_repeats(1);           //TODO: extract from <repeat>
        }
    }

    void advance_timepos_if_required()
    {
        TimeUnits curTime = m_pAnalyser->get_current_time();
        TimeUnits maxTime = m_pAnalyser->get_max_time();
        if (maxTime <= curTime)
            return;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoGoBackFwd* pImo = static_cast<ImoGoBackFwd*>(
                                ImFactory::inject(k_imo_go_back_fwd, pDoc) );
        pImo->set_forward(true);
        pImo->set_time_shift(maxTime - curTime);

        m_pAnalyser->set_current_time(maxTime);
        add_to_model(pImo);
    }

};

//@--------------------------------------------------------------------------------------
//@ <bracket>
class BracketMxlAnalyser : public MxlElementAnalyser
{
public:
    BracketMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                       LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <clef>
//@<!ELEMENT clef (sign, line?, clef-octave-change?)>
//@<!ATTLIST clef
//@    number CDATA #IMPLIED
//@    additional %yes-no; #IMPLIED
//@    size %symbol-size; #IMPLIED
//@    after-barline %yes-no; #IMPLIED
//@    %print-style;
//@    %print-object;
//@>
//
class ClefMxlAnalyser : public MxlElementAnalyser
{
protected:
    string m_sign;
    int m_line;
    int m_octaveChange;

public:
    ClefMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_line(0)
        , m_octaveChange(0)
    {
    }


    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoClef* pClef = static_cast<ImoClef*>( ImFactory::inject(k_imo_clef, pDoc) );

        // attrib: number CDATA #IMPLIED
        int nStaffNum = get_optional_int_attribute("number", 1);
        pClef->set_staff(nStaffNum - 1);

        // attrib: additional %yes-no; #IMPLIED
        //TODO

        // attrib: size %symbol-size; #IMPLIED
        //TODO

        // attrib: after-barline %yes-no; #IMPLIED
        //TODO

        // attrib: %print-style;
        get_attributes_for_print_style(pClef);

        // attrib: %print-object;
        //TODO

            //content

        // sign         <!ELEMENT sign (#PCDATA)>
        //TODO sign is mandatory (? check)
        if (get_optional("sign"))
            m_sign = get_child_value_string();

        // line?        <!ELEMENT line (#PCDATA)>
        if (get_optional("line"))
            m_line = get_child_value_integer(0);

        // clef-octave-change?      <!ELEMENT clef-octave-change (#PCDATA)>
        if (get_optional("clef-octave-change"))
            m_octaveChange = get_child_value_integer(0);

        int type = determine_clef_type();
        if (type == k_clef_undefined)
        {
            error_msg2(
                    "Unknown clef '" + m_sign + "'. Assumed 'G' in line 2.");
            type = k_clef_G2;
        }
        pClef->set_clef_type(type);

        error_if_more_elements();

        add_to_model(pClef);
        return pClef;
    }

protected:

    int determine_clef_type()
    {
        if (m_octaveChange==1 && !(m_sign == "F" || m_sign == "G"))
        {
            error_msg("Warning: <clef-octave-change> only implemented for F and G keys. Ignored.");
            m_octaveChange=0;
        }

        if (m_line < 1 || m_line > 5)
        {
            //TODO
            //error_msg("Warning: F clef only supported in lines 3, 4 or 5. Clef F in m_line " + m_line + "changed to F in m_line 4.");
            m_line = 1;
        }

        if (m_sign == "G")
        {
            if (m_line==2)
                return k_clef_G2;
            else if (m_line==1)
                return k_clef_G1;
            else
            {
                //TODO
                //error_msg("Warning: G clef only supported in lines 1 or 2. Clef G in line " + m_line + "changed to G in line 2.");
                return k_clef_G2;
            }
        }
        else if (m_sign == "F")
        {
            if (m_line==4)
                return k_clef_F4;
            else if (m_line==3)
                return k_clef_F3;
            else if (m_line==5)
                return k_clef_F5;
            else
            {
                //TODO
                //error_msg("Warning: F clef only supported in lines 3, 4 or 5. Clef F in line " + m_line + "changed to F in line 4.");
                return k_clef_F4;
            }
        }
        else if (m_sign == "C")
        {
            if (m_line==1)
                return k_clef_C1;
            else if (m_line==2)
                return k_clef_C2;
            else if (m_line==3)
                return k_clef_C3;
            else if (m_line==4)
                return k_clef_C4;
            else
                return k_clef_C5;
        }

        //TODO
        else if (m_sign == "percussion")
            return k_clef_percussion;
        else if (m_sign == "8_G")
            return k_clef_8_G2;
        else if (m_sign == "G_8")
            return k_clef_G2_8;
        else if (m_sign == "8_F4")
            return k_clef_8_F4;
        else if (m_sign == "F4_8")
            return k_clef_F4_8;
        else if (m_sign == "15_G")
            return k_clef_15_G2;
        else if (m_sign == "G_15")
            return k_clef_G2_15;
        else if (m_sign == "15_F4")
            return k_clef_15_F4;
        else if (m_sign == "F4_15")
            return k_clef_F4_15;
        else
            return k_clef_undefined;
    }

//    void set_symbol_size(ImoClef* pClef)
//    {
//        const std::string& value = m_childToAnalyse.first_child().value();
//        if (value == "cue")
//            pClef->set_symbol_size(k_size_cue);
//        else if (value == "full")
//            pClef->set_symbol_size(k_size_full);
//        else if (value == "large")
//            pClef->set_symbol_size(k_size_large);
//        else
//        {
//            pClef->set_symbol_size(k_size_full);
//            error_msg("Invalid symbol size '" + value + "'. 'full' size assumed.");
//        }
//    }

};

//@--------------------------------------------------------------------------------------
//@ <damp>
class DampMxlAnalyser : public MxlElementAnalyser
{
public:
    DampMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <coda>
//@ Coda signs can be associated with a measure or a musical direction.
//@ It is a visual indicator only; a sound element is needed for reliably playback.
//@
//@<!ELEMENT coda EMPTY>
//@<!ATTLIST coda
//@    %print-style-align;
//@>
//
class CodaMxlAnalyser : public MxlElementAnalyser
{
public:
    CodaMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoDirection* pDirection = nullptr;
        if (m_pAnchor && m_pAnchor->is_direction())
            pDirection = static_cast<ImoDirection*>(m_pAnchor);
        else
        {
            //TODO: deal with <coda> when child of <measure>
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoDirection");
            error_msg("<direction-type> <coda> is not child of <direction>. Ignored.");
            return nullptr;
        }
        pDirection->set_display_repeat(k_repeat_coda);

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSymbolRepetitionMark* pImo = static_cast<ImoSymbolRepetitionMark*>(
            ImFactory::inject(k_imo_symbol_repetition_mark, pDoc) );
        pImo->set_symbol(ImoSymbolRepetitionMark::k_coda);

        // attrib: %print-style-align;
        get_attributes_for_print_style_align(pImo);

        pDirection->add_attachment(pDoc, pImo);
        return pImo;
    }
};

//@--------------------------------------------------------------------------------------
//@ <damp-all>
class DampAllMxlAnalyser : public MxlElementAnalyser
{
public:
    DampAllMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <dashes>
class DashesMxlAnalyser : public MxlElementAnalyser
{
public:
    DashesMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ direction
//<!ELEMENT direction (direction-type+, offset?,
//    %editorial-voice;, staff?, sound?)>
//<!ATTLIST direction
//    %placement;
//    %directive;
//>
//
class DirectionMxlAnalyser : public MxlElementAnalyser
{
public:
    DirectionMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                         LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDirection* pDirection = static_cast<ImoDirection*>(
                                        ImFactory::inject(k_imo_direction, pDoc) );

        // attrib: %placement;
        pDirection->set_placement(get_attribute_placement());

        // attrib: %directive;
        //TODO

        // direction-type+
        bool fSpanner = false;;
        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            if (m_childToAnalyse.name() == "direction-type")
            {
                fSpanner |= m_pAnalyser->analyse_node_bool(&m_childToAnalyse, pDirection);
                move_to_next_child();
            }
            else
                break;
        }

        // offset?
        if (get_optional("offset"))
        {
            //TODO
        }

        // %editorial-voice; = (footnote?, level?, voice?)
        if (get_optional("footnote"))
        {
            //TODO
        }
        if (get_optional("level"))
        {
            //TODO
        }
        if (get_optional("voice"))
        {
            //TODO
        }

        // staff?
        pDirection->set_staff(analyse_optional_staff(1) - 1);

        // sound?
        analyse_optional("sound", pDirection);

        error_if_more_elements();

        if (fSpanner || pDirection->get_num_attachments() > 0)
            add_to_model(pDirection);
        else
        {
            delete pDirection;
            pDirection = nullptr;
        }

        return pDirection;
    }

};

//@--------------------------------------------------------------------------------------
//@ <direction-type>
//<!ELEMENT direction-type (rehearsal+ | segno+ | words+ |
//    coda+ | wedge | dynamics+ | dashes | bracket | pedal |
//    metronome | octave-shift | harp-pedals | damp | damp-all |
//    eyeglasses | string-mute | scordatura | image |
//    principal-voice | accordion-registration | percussion+ |
//    other-direction)>
//
class DirectionTypeMxlAnalyser : public MxlElementAnalyser
{
public:
    DirectionTypeMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                             LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis() { return nullptr; }

    bool do_analysis_bool()
    {
        bool fSpanner = false;
        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            if (m_childToAnalyse.name() == "rehearsal"
                || m_childToAnalyse.name() == "segno"
                || m_childToAnalyse.name() == "words"
                || m_childToAnalyse.name() == "coda"
                || m_childToAnalyse.name() == "dynamics"
                || m_childToAnalyse.name() == "metronome"
                || m_childToAnalyse.name() == "harp-pedals"
                || m_childToAnalyse.name() == "damp"
                || m_childToAnalyse.name() == "damp-all"
                || m_childToAnalyse.name() == "eyeglasses"
                || m_childToAnalyse.name() == "string-mute"
                || m_childToAnalyse.name() == "scordatura"
                || m_childToAnalyse.name() == "image"
                || m_childToAnalyse.name() == "accordion-registration"
                || m_childToAnalyse.name() == "percussion"
                || m_childToAnalyse.name() == "other-direction"
                //
                // spanners attached to notes instead of attaching them to ImoDirection
                || m_childToAnalyse.name() == "octave-shift"
               )
            {
                m_pAnalyser->analyse_node(&m_childToAnalyse, m_pAnchor);
            }
            else if (m_childToAnalyse.name() == "wedge"
                     || m_childToAnalyse.name() == "dashes"
                     || m_childToAnalyse.name() == "bracket"
                     || m_childToAnalyse.name() == "pedal"
                     || m_childToAnalyse.name() == "principal-voice"
                    )
            {
                m_pAnalyser->analyse_node(&m_childToAnalyse, m_pAnchor);
                fSpanner = true;
            }
            else
            {
                error_msg("Invalid direction-type <" + m_childToAnalyse.name()
                    + ">. Ignored.");
            }
            move_to_next_child();
        }

        return fSpanner;
    }
};

//@--------------------------------------------------------------------------------------
//@ <dynamics> = (fermata [<type>* | <other-dynamics>])
//@ <placement> = { above | below }
//<!--
//  Dynamics can be associated either with a note or a general
//  musical direction. To avoid inconsistencies between and
//  amongst the letter abbreviations for dynamics (what is sf
//  vs. sfz, standing alone or with a trailing dynamic that is
//  not always piano), we use the actual letters as the names
//  of these dynamic elements. The other-dynamics element
//  allows other dynamic marks that are not covered here, but
//  many of those should perhaps be included in a more general
//  musical direction element. Dynamics may also be combined as
//  in <sf/><mp/>.
//-->
//<!ELEMENT dynamics ((p | pp | ppp | pppp | ppppp | pppppp |
//    f | ff | fff | ffff | fffff | ffffff | mp | mf | sf |
//    sfp | sfpp | fp | rf | rfz | sfz | sffz | fz |
//    other-dynamics)*)>
//<!ATTLIST dynamics
//    %print-style-align;
//    %placement;
//    %text-decoration;
//    %enclosure;
//>

class DynamicsMxlAnalyser : public MxlElementAnalyser
{
public:
    DynamicsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoStaffObj* pSO = nullptr;
        if (m_pAnchor && (m_pAnchor->is_note_rest() || m_pAnchor->is_direction()))
            pSO = static_cast<ImoStaffObj*>(m_pAnchor);
        else
        {
            error_msg("pAnchor is nullptr or it is neither ImoNoteRest nor ImoDirection.");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>(
                                ImFactory::inject(k_imo_dynamics_mark, pDoc) );

        // attrib: placement
        if (has_attribute("placement"))
            set_placement(pImo);

        //content
        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            string type = m_childToAnalyse.name();
            if (type == "other-dynamics")
            {
                pImo->set_mark_type( m_childToAnalyse.value() );
            }
            else
            {
                //TODO: can have many marks. Need to append then
                pImo->set_mark_type(type);
            }
            move_to_next_child();
        }

        error_if_more_elements();

        pSO->add_attachment(pDoc, pImo);
        return pImo;
    }

protected:

    //-----------------------------------------------------------------------------------
    void set_placement(ImoDynamicsMark* pImo)
    {
        string value = get_attribute(&m_childToAnalyse, "placement");
        if (value == "above")
            pImo->set_placement(k_placement_above);
        else if (value == "below")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown placement attrib. '" + value + "'. Ignored.");
        }
    }

};

//@--------------------------------------------------------------------------------------
//<!ELEMENT ending (#PCDATA)>
//<!ATTLIST ending
//    number CDATA #REQUIRED
//        The number attribute reflects the numeric values of what
//        is under the ending line. Single endings such as "1" or
//        comma-separated multiple endings such as "1, 2" may be
//        used.
//    type (start | stop | discontinue) #REQUIRED
//    %print-object;
//    %print-style;
//    end-length %tenths; #IMPLIED
//    text-x %tenths; #IMPLIED
//    text-y %tenths; #IMPLIED
//>
//
//Examples:
//
//    <ending number="1, 2, 3" type="start">1-3.</ending>
//    <ending number="1" type="start">1.</ending>
//    <ending number="1" type="start">First time</ending>
//    <ending number="1" type="start"/>

class EndingMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoVoltaBracketDto* m_pVolta;

public:
    EndingMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                      LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pVolta(nullptr)
    {
    }

    ImoObj* do_analysis()
    {
        ImoBarline* pBarline = nullptr;
        if (m_pAnchor && m_pAnchor->is_barline())
            pBarline = static_cast<ImoBarline*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("nullptr pAnchor or it is not ImoBarline");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pVolta = static_cast<ImoVoltaBracketDto*>(
                        ImFactory::inject(k_imo_volta_bracket_dto, pDoc));
        m_pVolta->set_line_number( get_line_number() );

        // attrib: number CDATA #REQUIRED
        if (!set_volta_number())
        {
            delete m_pVolta;
            return nullptr;
        }

        // attrib: type (start | stop | discontinue) #REQUIRED
        if (!set_volta_type())
        {
            delete m_pVolta;
            return nullptr;
        }

        //TODO
        // attrib: %print-object;
        // attrib: %print-style;
        // attrib: end-length %tenths; #IMPLIED
        // attrib: text-x %tenths; #IMPLIED
        // attrib: text-y %tenths; #IMPLIED

        // ending (#PCDATA)
        m_pVolta->set_volta_text( m_analysedNode.value() );

        m_pVolta->set_barline(pBarline);
        m_pAnalyser->add_relation_info(m_pVolta);

        return nullptr;     //m_pVolta has been deleted in add_relation_info()
    }

protected:

    //-----------------------------------------------------------------------------------
    bool set_volta_number()
    {
        //returns false if error

        if (!has_attribute(&m_analysedNode, "number"))
            return false;   //error

        string num = m_analysedNode.attribute_value("number");
        if (num.empty())
            return false;   //error

        //validate ending number
        if (!mxl_is_valid_ending_number(num))
        {
            error_msg("Invalid ending number '" + num + "'. <ending> ignored.");
            return false;   //error
        }

        //extract numbers
        vector<int> repetitions;
        mxl_extract_numbers_from_ending(num, &repetitions);

        m_pVolta->set_repetitions(repetitions);
        m_pVolta->set_volta_number(num);

        return true;    //success
    }

    //-----------------------------------------------------------------------------------
    bool set_volta_type()
    {
        //returns false if error

        if (!has_attribute(&m_analysedNode, "type"))
            return false;   //error

        string value = m_analysedNode.attribute_value("type");
        if (value == "start")
        {
            m_pVolta->set_volta_type(ImoVoltaBracketDto::k_start);
            m_pVolta->set_volta_id( m_pAnalyser->new_volta_id() );
        }
        else if (value == "stop")
        {
            m_pVolta->set_volta_type(ImoVoltaBracketDto::k_stop);
            m_pVolta->set_final_jog(true);
            m_pVolta->set_volta_id( m_pAnalyser->get_volta_id() );
        }
        else if (value == "discontinue")
        {
            m_pVolta->set_volta_type(ImoVoltaBracketDto::k_stop);
            m_pVolta->set_final_jog(false);
            m_pVolta->set_volta_id( m_pAnalyser->get_volta_id() );
        }
        else
        {
            error_msg("Missing or invalid type. <ending> ignored.");
            return false;   //error
        }

        return true;    //success
    }

};

//public function to simplify unit testing of the regex
bool mxl_is_valid_ending_number(const string& num)
{
    //return TRUE if no error

    //XSD regex is  "([ ]*)|([1-9][0-9]*(, ?[1-9][0-9]*)*)"
    //but Lomse will be permissive with blank space errors such as :  "1,2", "1, 2 "
    std::regex regexValid("^([ ]*)$|^([1-9][0-9]*(, *[1-9][0-9]*)* *)$");
    return std::regex_match(num, regexValid);
}

//public function to simplify unit testing of the regex
void mxl_extract_numbers_from_ending(const string& num, vector<int>* repetitions)
{
    std::regex regexExtract(R"(\d+)");
    std::sregex_iterator it(num.begin(), num.end(), regexExtract);
    std::sregex_iterator end;

    while(it != end)
    {
        for(unsigned i = 0; i < it->size(); ++i)
        {
            repetitions->push_back( std::stoi( (*it)[i] ) );
        }
        ++it;
    }
}


//@--------------------------------------------------------------------------------------
//@ <eyeglasses>
class EyeglassesMxlAnalyser : public MxlElementAnalyser
{
public:
    EyeglassesMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <fermata> = (fermata <placement>[<componentOptions>*])
//@ <placement> = { above | below }
//<!--
//    Fermata and wavy-line elements can be applied both to
//    notes and to measures. Wavy
//    lines are one way to indicate trills; when used with a
//    measure element, they should always have type="continue"
//    set. The fermata text content represents the shape of the
//    fermata sign and may be normal, angled, or square.
//    An empty fermata element represents a normal fermata.
//    The fermata type is upright if not specified.
//-->
//<!ELEMENT fermata  (#PCDATA)>
//<!ATTLIST fermata
//    type (upright | inverted) #IMPLIED
//    %print-style;
//>

class FermataMxlAnalyser : public MxlElementAnalyser
{
public:
    FermataMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoNoteRest* pNR = nullptr;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoNoteRest");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoFermata* pImo = static_cast<ImoFermata*>(
                                ImFactory::inject(k_imo_fermata, pDoc) );

        // attrib: type (upright | inverted) #IMPLIED
        if (has_attribute("type"))
            set_type(pImo);

        set_shape_type(pImo);

//        error_if_more_elements();

        pNR->add_attachment(pDoc, pImo);
        return pImo;
    }

protected:

    //-----------------------------------------------------------------------------------
    void set_type(ImoFermata* pImo)
    {
        string type = get_attribute("type");
        if (type == "upright")
            pImo->set_placement(k_placement_above);
        else if (type == "inverted")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Unknown fermata type '" + type + "'. Ignored.");
        }
    }

    //-----------------------------------------------------------------------------------
    void set_shape_type(ImoFermata* pImo)
    {
        //text content (optional) indicates the shape of the
        //fermata sign and may be normal, angled, or square.
        //If not present, normal is implied.

        string shape = m_analysedNode.value();
        if (shape == "angled")
            pImo->set_symbol(ImoFermata::k_short);
        else if (shape == "square")
            pImo->set_symbol(ImoFermata::k_long);
        else
            pImo->set_symbol(ImoFermata::k_normal);
    }

};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT backup (duration, %editorial;)>
//@ <!ELEMENT forward
//@     (duration, %editorial-voice;, staff?)>
//@
//@ attrb: none
//@ Doc:
//    The backup and forward elements are required to coordinate
//    multiple voices in one part, including music on multiple
//    staves. The forward element is generally used within voices
//    and staves, while the backup element is generally used to
//    move between voices and staves. Thus the backup element
//    does not include voice or staff elements. Duration values
//    should always be positive, and should not cross measure
//    boundaries or mid-measure changes in the divisions value.

class FwdBackMxlAnalyser : public MxlElementAnalyser
{
public:
    FwdBackMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        bool fFwd = (m_analysedNode.name() == "forward");
        ImoStaffObj* pSO = nullptr;

        // <duration>
        if (!get_mandatory("duration"))
            return nullptr;
        int duration = get_child_value_integer(0);
        TimeUnits shift = m_pAnalyser->duration_to_timepos(duration);

        //<voice>
        if (fFwd && get_optional("voice"))
        {
            int voice = get_child_value_integer( m_pAnalyser->get_current_voice() );

            // staff?
            int staff = 1;
            if (get_optional("staff"))
                staff = get_child_value_integer(1) - 1;

            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoRest* pImo = static_cast<ImoRest*>(
                                    ImFactory::inject(k_imo_rest, pDoc) );
            pImo->mark_as_go_fwd();
            pImo->set_visible(false);
            pImo->set_type_dots_duration(k_quarter, 0, shift);
            pImo->set_staff(staff);
            pImo->set_voice(voice);
            pSO = pImo;
        }
        else
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoGoBackFwd* pImo = static_cast<ImoGoBackFwd*>(
                                    ImFactory::inject(k_imo_go_back_fwd, pDoc) );
            pImo->set_forward(fFwd);
            pImo->set_time_shift(shift);
            pSO = pImo;
        }

        m_pAnalyser->shift_time( fFwd ? shift : -shift);
        add_to_model(pSO);
        return pSO;

    }

protected:

};

//@--------------------------------------------------------------------------------------
//@ <harp-pedals>
class HarpPedalsMxlAnalyser : public MxlElementAnalyser
{
public:
    HarpPedalsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <image>
class ImageMxlAnalyser : public MxlElementAnalyser
{
public:
    ImageMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <key> = <fifths><mode> ????
//@ attrb:   ??
//            /*
//            Traditional key signatures are represented by the number
//            of flats and sharps, plus an optional mode for major/minor mode
//            distinctions. Negative numbers are used for
//            flats and positive numbers for sharps, reflecting the
//            key's placement within the circle of fifths (hence the
//            element name). A cancel element indicates that the old
//            key signature should be cancelled before the new one
//            appears. This will always happen when changing to C major
//            or A minor and need not be specified then. The cancel
//            value matches the fifths value of the cancelled key
//            signature (e.g., a cancel of -2 will provide an explicit
//            cancellation for changing from B flat major to F major).
//
//            Non-traditional key signatures can be represented using
//            the Humdrum/Scot concept of a list of altered tones.
//            The key-step and key-alter elements are represented the
//            same way as the step and alter elements are in the pitch
//            element in note.dtd. The different element names indicate
//            the different meaning of altering notes in a scale versus
//            altering a sounding pitch.
//
//            Valid mode values include major, minor, dorian, phrygian,
//            lydian, mixolydian, aeolian, ionian, and locrian.
//
//            <!ELEMENT key ((cancel?, fifths, mode?) |
//                ((key-step, key-alter)*))>
//            <!ELEMENT cancel (#PCDATA)>
//            <!ELEMENT fifths (#PCDATA)>
//            <!ELEMENT mode (#PCDATA)>
//            <!ELEMENT key-step (#PCDATA)>
//            <!ELEMENT key-alter (#PCDATA)>

class KeyMxlAnalyser : public MxlElementAnalyser
{
public:
    KeyMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoKeySignature* pKey = static_cast<ImoKeySignature*>(
                                    ImFactory::inject(k_imo_key_signature, pDoc) );

        //TODO: Here we are dealing only with "traditional" key signatures:
        //      chromatic scale in major and minor modes).

        int fifths = 0;
        bool fMajor = true;

        // <fifths> (num)
        if (get_mandatory("fifths"))
            fifths = get_child_value_integer(0);

        // <mode>
        if (get_optional("mode"))
            fMajor = (get_child_value_string() == "major");


        error_if_more_elements();

        //set key
        pKey->set_key_type( fifths_to_key_signature(fifths, fMajor) );

        add_to_model(pKey);
        return pKey;
    }

protected:

    int fifths_to_key_signature(int fifths, bool fMajor)
    {
        // Returns the key signature for the given number of fifths and mode

        if (fMajor)
        {
            switch(fifths)
            {
                case 0:
                    return k_key_C;

                //Sharps ---------------------------------------
                case 1:
                    return k_key_G;
                case 2:
                    return k_key_D;
                case 3:
                    return k_key_A;
                case 4:
                    return k_key_E;
                case 5:
                    return k_key_B;
                case 6:
                    return k_key_Fs;
                case 7:
                    return k_key_Cs;

                //Flats -------------------------------------------
                case -1:
                    return k_key_F;
                case -2:
                    return k_key_Bf;
                case -3:
                    return k_key_Ef;
                case -4:
                    return k_key_Af;
                case -5:
                    return k_key_Df;
                case -6:
                    return k_key_Gf;
                case -7:
                    return k_key_Cf;

                default:
                {
                    stringstream msg;
                    msg << "Invalid number of fifths " <<
                           fifths ;
                    error_msg(msg.str());
    //                LOMSE_LOG_ERROR(msg.str());
    //                throw runtime_error(msg.str());
                    return k_key_C;
                }
            }
        }
        else
        {
            switch(fifths)
            {
                case 0:
                    return k_key_a;

                //Sharps ---------------------------------------
                case 1:
                    return k_key_e;
                case 2:
                    return k_key_b;
                case 3:
                    return k_key_fs;
                case 4:
                    return k_key_cs;
                case 5:
                    return k_key_gs;
                case 6:
                    return k_key_ds;
                case 7:
                    return k_key_as;

                //Flats -------------------------------------------
                case -1:
                    return k_key_d;
                case -2:
                    return k_key_g;
                case -3:
                    return k_key_c;
                case -4:
                    return k_key_f;
                case -5:
                    return k_key_bf;
                case -6:
                    return k_key_ef;
                case -7:
                    return k_key_af;

                default:
                {
                    stringstream msg;
                    msg << "Invalid number of fifths " <<
                           fifths ;
                    error_msg(msg.str());
    //                LOMSE_LOG_ERROR(msg.str());
    //                throw runtime_error(msg.str());
                    return k_key_a;
                }
            }
        }
    }


};

//@--------------------------------------------------------------------------------------
//@ lyric = ([syllabic] text [ ([elision] [syllabic] text)* [extend] |
//@                            extend | laughing | humming ] )
//@         [end-line] [end-paragraph] [%editorial]

//<!ELEMENT lyric
//    ((((syllabic?, text),
//       (elision?, syllabic?, text)*, extend?) |
//       extend | laughing | humming),
//      end-line?, end-paragraph?, %editorial;)>

class LyricMxlAnalyser : public MxlElementAnalyser
{
public:
    LyricMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        ImoNote* pNote = nullptr;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoNote");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLyric* pData = static_cast<ImoLyric*>(
                                    ImFactory::inject(k_imo_lyric, pDoc) );

        // attrib: number
        int num = 1;
        if (has_attribute("number"))
            num = get_attribute_as_integer("number", 1);
        pData->set_number(num);

        // attrib: type (upright | inverted) #IMPLIED
        if (has_attribute("placement"))
            set_placement(pData);

        ImoLyricsTextInfo* pText = static_cast<ImoLyricsTextInfo*>(
                ImFactory::inject(k_imo_lyrics_text_info, pDoc) );
        pData->add_text_item(pText);

        // [syllabic]
        if (get_optional("syllabic"))
            set_syllabic(pText, pData);

        // text
        if (!analyse_mandatory("text", pText))
        {
            delete pData;
            return nullptr;
        }

        // [extend]
        if (get_optional("extend"))
            pData->set_melisma(true);

        m_pAnalyser->add_lyrics_data(pNote, pData);
        add_to_model(pData);

        return pData;
    }

protected:

    //-----------------------------------------------------------------------------------
    void set_placement(ImoLyric* pImo)
    {
        string type = get_attribute("placement");
        if (type == "above")
            pImo->set_placement(k_placement_above);
        else if (type == "below")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Unknown placement value '" + type + "'. Ignored.");
        }
    }

    //-----------------------------------------------------------------------------------
    void set_syllabic(ImoLyricsTextInfo* pImo, ImoLyric* pLyric)
    {
        string value = m_childToAnalyse.value();
        if (value == "single")
            pImo->set_syllable_type(ImoLyricsTextInfo::k_single);
        else if (value == "begin")
        {
            pImo->set_syllable_type(ImoLyricsTextInfo::k_begin);
            pLyric->set_hyphenation(true);
        }
        else if (value == "end")
            pImo->set_syllable_type(ImoLyricsTextInfo::k_end);
        else if (value == "middle")
        {
            pImo->set_syllable_type(ImoLyricsTextInfo::k_middle);
            pLyric->set_hyphenation(true);
        }
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Unknown syllabic value '" + value + "'. Ignored.");
        }
    }

};


//@--------------------------------------------------------------------------------------
//@ <!ELEMENT measure (%music-data;)>
//@ <!ENTITY % music-data
//@     "(note | backup | forward | direction | attributes |
//@       harmony | figured-bass | print | sound | barline |
//@       grouping | link | bookmark)*">
//@ <!ATTLIST measure
//@     number CDATA #REQUIRED
//@     implicit %yes-no; #IMPLIED
//@     non-controlling %yes-no; #IMPLIED
//@     width %tenths; #IMPLIED
//@ >

class MeasureMxlAnalyser : public MxlElementAnalyser
{
public:
    MeasureMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        ImoMusicData* pMD = nullptr;
        if (m_pAnchor && m_pAnchor->is_music_data())
            pMD = static_cast<ImoMusicData*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not musicData");
            return nullptr;
        }

        //attrb: number CDATA #REQUIRED
        string num = get_optional_string_attribute("number", "");
        if (num.empty())
        {
            error_msg("<measure>: missing mandatory 'number' attribute. <measure> content will be ignored");
            return nullptr;
        }
        TypeMeasureInfo* pInfo = create_measure_info(num);

        //attrb: implicit %yes-no; #IMPLIED
        //AWARE: implicit="yes" means 'do not display measure number'
        pInfo->fHideNumber = get_optional_yes_no_attribute("implicit", false);

        //attrb: non-controlling %yes-no; #IMPLIED
        //'non-controlling': a barline not suitable for line breaks or page breaks.
        //MusicXML uses this concept for dealing with multi-metrics
        //TODO

        //attrb: width %tenths; #IMPLIED
        //TODO

        // [{<xxxx>|<yyyy>|<zzzz>}*]    alternatives: zero or more
        while (more_children_to_analyse())
        {
            if (!(analyse_optional("attributes", pMD)
                  || analyse_optional("barline", pMD)
                  || analyse_optional("direction", pMD)
                  || analyse_optional("note", pMD)
                  || analyse_optional("forward", pMD)
                  || analyse_optional("backup", pMD)
                  || analyse_optional("print")
                  || analyse_optional("sound", pMD)
                 )
               )
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        error_if_more_elements();

        ImoObj* pSO = static_cast<ImoStaffObj*>(pMD->get_last_child());
        if (pSO == nullptr || !pSO->is_barline())
            add_barline(pInfo);
        else if (pSO->is_barline())
            static_cast<ImoBarline*>(pSO)->set_measure_info(pInfo);

        return pMD;
    }

protected:

    TypeMeasureInfo* create_measure_info(const string& num)
    {
        TypeMeasureInfo* pInfo = LOMSE_NEW TypeMeasureInfo();
        pInfo->count = m_pAnalyser->increment_measures_counter();
        pInfo->number = num;
        m_pAnalyser->save_current_measure_num(num);
        return pInfo;
    }

    void add_barline(TypeMeasureInfo* pInfo)
    {
        advance_timepos_if_required();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBarline* pBarline = static_cast<ImoBarline*>(
                                    ImFactory::inject(k_imo_barline, pDoc) );
        pBarline->set_type(k_barline_simple);
        pBarline->set_measure_info(pInfo);
        add_to_model(pBarline);
        m_pAnalyser->save_last_barline(pBarline);
    }

    void advance_timepos_if_required()
    {
        TimeUnits curTime = m_pAnalyser->get_current_time();
        TimeUnits maxTime = m_pAnalyser->get_max_time();
        if (maxTime <= curTime)
            return;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoGoBackFwd* pImo = static_cast<ImoGoBackFwd*>(
                                ImFactory::inject(k_imo_go_back_fwd, pDoc) );
        pImo->set_forward(true);
        pImo->set_time_shift(maxTime - curTime);

        m_pAnalyser->set_current_time(maxTime);
        add_to_model(pImo);
    }

};

//@--------------------------------------------------------------------------------------
//@ <metronome>
//@ <!ELEMENT metronome
//@ 	((beat-unit, beat-unit-dot*,
//@      (per-minute | (beat-unit, beat-unit-dot*))) |
//@ 	(metronome-note+, (metronome-relation, metronome-note+)?))>
//@ <!ATTLIST metronome
//@     %print-style;
//@     parentheses %yes-no; #IMPLIED
//@ >
//@ <!ELEMENT beat-unit (#PCDATA)>
//@ <!ELEMENT beat-unit-dot EMPTY>
//@ <!ELEMENT per-minute (#PCDATA)>
//@ <!ATTLIST per-minute
//@     %font;
//@ >
class MetronomeMxlAnalyser : public MxlElementAnalyser
{
public:
    MetronomeMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        if (m_pAnchor == nullptr || !m_pAnchor->is_direction())
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoDirection");
            error_msg("<direction-type> <measure> is not child of <direction>. Ignored.");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMetronomeMark* pMtr = static_cast<ImoMetronomeMark*>(
                    ImFactory::inject(k_imo_metronome_mark, pDoc) );

        //attrb: %print-style;
            //TODO

        //attrb: parentheses %yes-no; #IMPLIED
            //TODO


        //elements

        if (get_optional("beat-unit"))
        {
            // (beat-unit, beat-unit-dot*, (per-minute | (beat-unit, beat-unit-dot*))

            int noteType = get_beat_unit();
            pMtr->set_left_note_type(noteType);

            int dots = 0;
            while (get_optional("beat-unit-dot"))
                ++dots;
            pMtr->set_left_dots(dots);

            if (get_optional("per-minute"))
            {
                // case 1: (beat-unit, beat-unit-dot*) = per-minute
                pMtr->set_ticks_per_minute( get_child_value_integer(60) );
                pMtr->set_mark_type(ImoMetronomeMark::k_note_value);
            }
            else if (get_optional("beat-unit"))
            {
                // case 2: (beat-unit, beat-unit-dot*) = (beat-unit, beat-unit-dot*)
                int noteType = get_beat_unit();
                pMtr->set_right_note_type(noteType);

                int dots = 0;
                while (get_optional("beat-unit-dot"))
                    ++dots;
                pMtr->set_right_dots(dots);
                pMtr->set_mark_type(ImoMetronomeMark::k_note_note);
            }
            else
            {
                error_msg2(
                        "Error in metronome parameters. Replaced by '(metronome 60)'.");
                pMtr->set_ticks_per_minute(60);
                pMtr->set_mark_type(ImoMetronomeMark::k_value);
                add_to_model(pMtr);
                return pMtr;
            }
        }
        else if (get_optional("metronome-note"))
        {
            // (metronome-note+, (metronome-relation, metronome-note+)?)

            //TODO: examples needed, for understanding and unit tests
        }

        add_to_model(pMtr);
        return pMtr;
    }

protected:

    int get_beat_unit()
    {
        return to_note_type( m_childToAnalyse.value() );
    }


};

//@--------------------------------------------------------------------------------------
//@<!ELEMENT midi-device (#PCDATA)>
//@<!ATTLIST midi-device
//@    port CDATA #IMPLIED
//@    id IDREF #IMPLIED
//@>
//
class MidiDeviceMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoSounds* m_pSounds;
    ImoSoundChange* m_pSC;

public:
    MidiDeviceMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                          LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pSounds(nullptr)
        , m_pSC(nullptr)
    {
    }


    ImoObj* do_analysis()
    {
        //anchor parent is ImoSounds when analysing <score-instrument> or
        //ImoSoundChange when analysing <sound>
        m_pSounds = dynamic_cast<ImoSounds*>(m_pAnchor);
        m_pSC = dynamic_cast<ImoSoundChange*>(m_pAnchor);
        if (m_pSounds == nullptr && m_pSC == nullptr)
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is neither ImoSounds nor ImoSoundChange.");
            return nullptr;
        }

        ImoInstrument* pInstr = m_pAnalyser->get_current_instrument();

        //attrb: id IDREF #IMPLIED
        string id = get_optional_string_attribute("id", "");
        if (!id.empty())
        {
            //validate id
            ImoSoundInfo* pInfo = nullptr;
            if (m_pSC && !pInstr)
            {
                error_msg("Unit test error? Can not analyse <midi-instrument> when not "
                          "inside a <part> element. Ignored.");
                return nullptr;
            }
            if (pInstr)
                pInfo = pInstr->get_sound_info(id);
            else if (m_pSounds)
                pInfo = m_pSounds->get_sound_info(id);
            if (!pInfo)
            {
                error_msg("id '" + id + "' doesn't match any <score-instrument>"
                           + ". <midi-instrument> ignored.");
                return nullptr;
            }
        }


        // attrb: port CDATA #IMPLIED
        int port = get_optional_int_attribute("port", 1);

        // midi-device name
        string name = m_analysedNode.value();


        //If 'id' attribute is missing, the device assignment affects
        //all ImoSoundInfo elements in this Instrument.
        if (!id.empty())
        {
            ImoMidiInfo* pMidi = get_midi_info(id);
            pMidi->set_midi_port(port - 1);
            pMidi->set_midi_device_name(name);
        }
        else
        {
            if (m_pSounds)
            {
                int  nSounds = m_pSounds->get_num_sounds();
                for (int i=0; i < nSounds; ++i)
                {
                    ImoSoundInfo* pInfo = m_pSounds->get_sound_info(i);
                    ImoMidiInfo* pMidi = pInfo->get_midi_info();
                    pMidi->set_midi_port(port - 1);
                    pMidi->set_midi_device_name(name);
                }
            }
            else    //m_pSC != nullptr, analysing <sound>
            {
                ImoSounds* pSounds = pInstr->get_sounds();
                int  nSounds = pSounds->get_num_sounds();
                for (int i=0; i < nSounds; ++i)
                {
                    ImoSoundInfo* pInfo = pSounds->get_sound_info(i);
                    ImoMidiInfo* pMidi = get_midi_info(pInfo->get_score_instr_id());
                    pMidi->set_midi_port(port - 1);
                    pMidi->set_midi_device_name(name);
                }
            }
        }

        return nullptr;
    }

protected:

    ImoMidiInfo* get_midi_info(const string& id)
    {
        //get midiInfo or create a new one
        ImoMidiInfo* pMidi = nullptr;
        if (m_pSC)
        {
            //analysing <sound>
            //ImoMidiInfo could have been created analysing <midi-device>.
            //But if not created, create it here
            pMidi = m_pSC->get_midi_info(id);
            if (!pMidi)
            {
                Document* pDoc = m_pAnalyser->get_document_being_analysed();
                pMidi = static_cast<ImoMidiInfo*>(
                                    ImFactory::inject(k_imo_midi_info, pDoc) );
                pMidi->set_score_instr_id(id);
                m_pSC->append_child_imo(pMidi);

                //copy data from latest MidiInfo for this score-instrument
                ImoMidiInfo* pMidiOld = m_pAnalyser->get_latest_midi_info_for(id);
                if (pMidiOld)
                    *pMidi = *pMidiOld;
                m_pAnalyser->set_latest_midi_info_for(id, pMidi);
            }
        }
        else
        {
            ImoSoundInfo* pInfo = m_pSounds->get_sound_info(id);
            pMidi = pInfo->get_midi_info();
        }
        return pMidi;
    }

};


//@--------------------------------------------------------------------------------------
//@<!ELEMENT midi-instrument
//@    (midi-channel?, midi-name?, midi-bank?, midi-program?,
//@     midi-unpitched?, volume?, pan?, elevation?)>
//@<!ATTLIST midi-instrument
//@    id IDREF #REQUIRED
//@>
//@<!ELEMENT midi-channel (#PCDATA)>	1 to 16
//@<!ELEMENT midi-name (#PCDATA)>
//@<!ELEMENT midi-bank (#PCDATA)>		1 to 16,384
//@<!ELEMENT midi-program (#PCDATA)>	1 to 128
//@<!ELEMENT midi-unpitched (#PCDATA)>	1 to 128
//@<!ELEMENT volume (#PCDATA)>			0 to 100, with decimal values
//@<!ELEMENT pan (#PCDATA)>			    -180 and 180, with decimal values
//@<!ELEMENT elevation (#PCDATA)>		-90 and 90, with decimal values
//
class MidiInstrumentMxlAnalyser : public MxlElementAnalyser
{
public:
    MidiInstrumentMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                              LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //anchor parent is ImoSounds when analysing <score-instrument> or
        //ImoSoundChange when analysing <sound>
        ImoSounds* pSounds = dynamic_cast<ImoSounds*>(m_pAnchor);
        ImoSoundChange* pSC = dynamic_cast<ImoSoundChange*>(m_pAnchor);
        if (pSounds == nullptr && pSC == nullptr)
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is neither ImoSounds nor ImoSoundChange.");
            return nullptr;
        }

        //attrb: id
        string id = get_mandatory_string_attribute("id", "", "midi-instrument");
        if (id.empty())
            return nullptr;

        ImoSoundInfo* pInfo = nullptr;
        ImoInstrument* pInstr = m_pAnalyser->get_current_instrument();
        if (pSC && !pInstr)
        {
            error_msg("Unit test error? Can not analyse <midi-instrument> when not "
                      "inside a <part> element. Ignored.");
            return nullptr;
        }
        if (pInstr)
            pInfo = pInstr->get_sound_info(id);
        else if (pSounds)
            pInfo = pSounds->get_sound_info(id);
        if (!pInfo)
        {
            error_msg("id '" + id + "' doesn't match any <score-instrument>"
                       + ". <midi-instrument> ignored.");
            return nullptr;
        }

        //get midiInfo or create a new one
        ImoMidiInfo* pMidi = nullptr;
        if (pSC)
        {
            //analysing <sound>
            //ImoMidiInfo could have been created analysing <midi-device>.
            //But if not created, create it here
            pMidi = pSC->get_midi_info(id);
            if (!pMidi)
            {
                Document* pDoc = m_pAnalyser->get_document_being_analysed();
                pMidi = static_cast<ImoMidiInfo*>(
                                    ImFactory::inject(k_imo_midi_info, pDoc) );
                pMidi->set_score_instr_id(id);
                pSC->append_child_imo(pMidi);

                //copy data from latest MidiInfo for this score-instrument
                ImoMidiInfo* pMidiOld = m_pAnalyser->get_latest_midi_info_for(id);
                if (pMidiOld)
                    *pMidi = *pMidiOld;
                m_pAnalyser->set_latest_midi_info_for(id, pMidi);
            }
        }
        else
        {
            pInfo = pSounds->get_sound_info(id);
            pMidi = pInfo->get_midi_info();
        }

        // midi-channel?    1 to 16
        if (get_optional("midi-channel"))
            pMidi->set_midi_channel(get_child_pcdata_int("midi-channel", 1, 16, 1) - 1);

        // midi-name?
        if (get_optional("midi-name"))
            pMidi->set_midi_name(m_childToAnalyse.value());

        // midi-bank?   1 to 16,384
        if (get_optional("midi-bank"))
            pMidi->set_midi_bank(get_child_pcdata_int("midi-bank", 1, 16384, 1) - 1);

        // midi-program?    1 to 128
        if (get_optional("midi-program"))
            pMidi->set_midi_program(get_child_pcdata_int("midi-program", 1, 128, 1) - 1);

        // midi-unpitched?  1 to 128
        if (get_optional("midi-unpitched"))
            pMidi->set_midi_unpitched(get_child_pcdata_int("midi-unpitched", 1, 128, 1) - 1);

        // volume?  0 to 100, with decimal values
        if (get_optional("volume"))
            pMidi->set_midi_volume(get_child_pcdata_float("volume", 0.0f, 100.0f, 100.0f) / 100.0f);

        // pan?     -180 and 180, with decimal values
        if (get_optional("pan"))
            pMidi->set_midi_pan(
                        int(get_child_pcdata_float("pan", -180.0f, 180.0f, 0.0f)) );

        // elevation?   -90 and 90, with decimal values
        if (get_optional("elevation"))
            pMidi->set_midi_elevation(
                        int(get_child_pcdata_float("elevation", -90.0f, 90.0f, 0.0f)) );

        error_if_more_elements();


        return nullptr;
    }

};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT notations
//@     (%editorial;,
//@      (tied | slur | tuplet | glissando | slide |
//@       ornaments | technical | articulations | dynamics |
//@       fermata | arpeggiate | non-arpeggiate |
//@       accidental-mark | other-notation)*)>
//@ <!ATTLIST notations
//@     %print-object;
//@ >

class NotationsMxlAnalyser : public MxlElementAnalyser
{
public:
    NotationsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        // [{<xxxx>|<yyyy>|<zzzz>}*]    alternatives: zero or more
        while (more_children_to_analyse())
        {
            analyse_optional("tied", m_pAnchor)
            || analyse_optional("slur", m_pAnchor)
            || analyse_optional("tuplet", m_pAnchor)
            || analyse_optional("glissando", m_pAnchor)
            || analyse_optional("slide", m_pAnchor)
            || analyse_optional("ornaments", m_pAnchor)
            || analyse_optional("technical", m_pAnchor)
            || analyse_optional("articulations", m_pAnchor)
            || analyse_optional("dynamics", m_pAnchor)
            || analyse_optional("fermata", m_pAnchor)
            || analyse_optional("arpeggiate", m_pAnchor)
            || analyse_optional("non-arpeggiate", m_pAnchor)
            || analyse_optional("accidental-mark", m_pAnchor)
            || analyse_optional("other-notation", m_pAnchor);
        }

        return nullptr;
    }

protected:

};


//@--------------------------------------------------------------------------------------
//@ <!ELEMENT note
//@     (((grace, %full-note;, (tie, tie?)?) |
//@      (cue, %full-note;, duration) |
//@      (%full-note;, duration, (tie, tie?)?)),
//@      instrument?, %editorial-voice;, type?, dot*,
//@      accidental?, time-modification?, stem?, notehead?,
//@      notehead-text?, staff?, beam*, notations*, lyric*, play?)>
//@ <!ELEMENT cue EMPTY>
//@ <!ELEMENT grace EMPTY>
//@ <!ENTITY % full-note "(chord?, (pitch | unpitched | rest))">
//@
//@ - Grace notes do not have a duration element.
//@ - Cue notes have a duration element, as do forward elements, but no tie elements.
//@

class NoteRestMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoBeamDto* m_pBeamInfo;
//    ImoSlurDto* m_pSlurDto;
//    std::string m_srcOldTuplet;

public:
    NoteRestMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pBeamInfo(nullptr)
//        , m_pSlurDto(nullptr)
//        , m_srcOldTuplet("")
    {
    }

    ImoObj* do_analysis()
    {
        bool fIsCue = get_optional("cue");
        bool fIsGrace = get_optional("grace");
        bool fInChord = false;
        bool fIsRest = false;

        //for now, ignore cue & grace notes
        if (fIsCue || fIsGrace)
            return nullptr;

        // [<chord>]
        if (get_optional("chord"))
        {
            //The chord element indicates that this note is an additional chord tone
            //with the preceding note. The duration of this note can be no longer
            //than the preceding note.
            fInChord = true;
        }

        // <pitch> | <unpitched> | <rest>
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoNoteRest* pNR = nullptr;
        ImoNote* pNote = nullptr;
        ImoRest* pRest = nullptr;

        if (get_optional("rest"))
        {
            fIsRest = true;
            pRest = static_cast<ImoRest*>(ImFactory::inject(k_imo_rest, pDoc));
            pNR = pRest;
            pRest->mark_as_full_measure( analyse_rest() );
        }
        else
        {
            pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, pDoc));
            pNR = pNote;
            if (get_optional("unpitched"))
                pNote->set_notated_pitch(k_no_pitch, 4, k_no_accidentals);
            else
                analyse_mandatory("pitch", pNote);
        }

        // <duration>, except for grace notes
        int duration = 0;
        if (!fIsGrace && get_optional("duration"))
            duration = get_child_value_integer(0);

        //tie, except for cue notes
        //AWARE: <tie> is for sound
        if (!fIsCue && get_optional("tie"))
        {
        }
        if (!fIsCue && get_optional("tie"))
        {
        }

        // [<instrument>]
        if (get_optional("instrument"))
        {
        }

        // [<voice>]
        int voice = m_pAnalyser->get_current_voice();
        if (get_optional("voice"))
            voice = get_child_value_integer( voice );
        set_voice(pNR, voice);

        // [<type>]
        string type;
        if (get_optional("type"))
            type = m_childToAnalyse.value();

        // <dot>*
        int dots = 0;
        while (get_optional("dot"))
            dots++;

        set_type_duration(pNR, type, dots, duration);

        // [<accidental>]
        if (!fIsRest && get_optional("accidental"))
            set_notated_accidentals(pNote);

        // [<time-modification>]
        analyse_optional("time-modification", pNR);

        // [<stem>]
        if (!fIsRest && get_optional("stem"))
            set_stem(pNote);

        // [<notehead>]
        if (get_optional("notehead"))
        {
        }

        // [<notehead-text>]
        if (get_optional("notehead-text"))
        {
        }

        // [<staff>]
        if (get_optional("staff"))
            pNR->set_staff(get_child_value_integer(1) - 1);

        // <beam>*
        while (get_optional("beam"))
            analyse_beam();
        add_beam_info(pNR);

        // <notations>*
        while (analyse_optional("notations", pNR));
        m_pAnalyser->add_to_open_tuplets(pNR);

        // <lyric>*
        while (analyse_optional("lyric", pNR));

        // [<play>]
        if (get_optional("play"))
        {
        }

        error_if_more_elements();

        add_to_model(pNR);
        add_to_spanners(pNote);

        //deal with notes in chord
        if (!fIsRest && fInChord)
        {
            ImoNote* pPrevNote = m_pAnalyser->get_last_note();
            ImoChord* pChord;
            if (pPrevNote->is_in_chord())
            {
                //chord already created. just add note to it
                pChord = pPrevNote->get_chord();
            }
            else
            {
                //previous note is the base note. Create the chord
                pChord = static_cast<ImoChord*>(ImFactory::inject(k_imo_chord, pDoc));
                pPrevNote->include_in_relation(pDoc, pChord);
            }

            //add current note to chord
            pNote->include_in_relation(pDoc, pChord);

//        //TODO: check if note in chord has the same duration than base note
//      //  if (fInChord && m_pLastNote
//      //      && !IsEqualTime(m_pLastNote->GetDuration(), rDuration) )
//      //  {
//      //      report_msg("Error: note in chord has different duration than base note. Duration changed.");
//		    //rDuration = m_pLastNote->GetDuration();
//      //      nNoteType = m_pLastNote->GetNoteType();
//      //      nDots = m_pLastNote->GetNumDots();
//      //  }
        }

        //save this note as last note
        if (!fIsRest)
            m_pAnalyser->save_last_note(pNote);

        m_pAnalyser->shift_time( pNR->get_duration() );
        return pNR;
    }

protected:

    //----------------------------------------------------------------------------------
    bool analyse_rest()
    {
        //@ <!ELEMENT rest ((display-step, display-octave)?)>
        //@ <!ATTLIST rest
        //@      measure %yes-no; #IMPLIED
        //@ >

        //returns value of measure attrib. (true or false)
        if (has_attribute(&m_childToAnalyse, "measure"))
        {
            const string& measure = m_childToAnalyse.attribute_value("measure");
            return measure == "yes";
        }
        else
            return false;
    }

    //----------------------------------------------------------------------------------
    void set_type_duration(ImoNoteRest* pNR, const string& type, int dots,
                           int duration)
    {
        int noteType = k_unknown_notetype;
        TimeUnits units = m_pAnalyser->duration_to_timepos(duration);
        if (!type.empty())
            noteType = to_note_type(type);
        else if (pNR->is_rest())
        {
            //<type> is not required for full-measure rests
            dots = 0;
            noteType = k_whole;
            static_cast<ImoRest*>(pNR)->mark_as_full_measure(true);
        }
        else
        {
            //<type> is not required in full-measure rests. And, in any
            //case it is not mandatory. If not present, <type>
            //must be derived from <duration>.
            if (is_equal_time(units, k_duration_longa))
                noteType = k_longa;
            else if (is_equal_time(units, k_duration_whole))
                noteType = k_whole;
            else if (is_equal_time(units, k_duration_half))
                noteType = k_half;
            else if (is_equal_time(units, k_duration_quarter))
                noteType = k_quarter;
            else if (is_equal_time(units, k_duration_eighth))
                noteType = k_eighth;
            else if (is_equal_time(units, k_duration_16th))
                noteType = k_16th;
            else if (is_equal_time(units, k_duration_32nd))
                noteType = k_32nd;
            else if (is_equal_time(units, k_duration_64th))
                noteType = k_64th;
            else if (is_equal_time(units, k_duration_128th))
                noteType = k_128th;
            else if (is_equal_time(units, k_duration_256th))
                noteType = k_256th;
            else
            {
                stringstream msg;
                msg << "Invalid <duration> value " << duration << " ("
                    << units << " TimeUnits).";
                error_msg2(msg.str());
                noteType = k_256th;
            }
        }

        pNR->set_type_dots_duration(noteType, dots, units);
    }

    //----------------------------------------------------------------------------------
    void set_notated_accidentals(ImoNote* pNote)
    {
        //@ <!ELEMENT accidental (#PCDATA)>
        //@ <!ATTLIST accidental
        //@           cautionary %yes-no; #IMPLIED
        //@           editorial %yes-no; #IMPLIED
        //@           %level-display;
        //@           %print-style;
        //@>
        EAccidentals accidentals = k_no_accidentals;
        string acc = m_childToAnalyse.value();  //get_child_value_string();
        if (acc == "sharp")
            accidentals = k_sharp;
        else if (acc == "natural")
            accidentals = k_natural;
        else if (acc == "flat")
            accidentals = k_flat;
        else if (acc == "double-sharp")
            accidentals = k_double_sharp;
        else if (acc == "sharp-sharp")
            accidentals = k_sharp_sharp;
        else if (acc == "flat-flat")
            accidentals = k_flat_flat;
        //else if (acc == "double-flat")
            //AWARE: double-flat is not in the specification. Lilypond test suite
            //       uses it and MuseScore imports it correctly. But Michael Good
            //       is clear about this. See:
            //http://forums.makemusic.com/viewtopic.php?f=12&t=2253&p=5965#p5964
            //http://forums.makemusic.com/viewtopic.php?f=12&t=2408&p=6558#p6556
            //accidentals = k_flat_flat;
        else if (acc == "natural-sharp")
            accidentals = k_natural_sharp;
        else if (acc == "natural-flat")
            accidentals = k_natural_flat;

//        //Tartini-style quarter-tone accidentals
//        else if (acc == "quarter-flat")
//        else if (acc == "quarter-sharp")
//        else if (acc == "three-quarters-flat")
//        else if (acc == "three-quarters-sharp")
//        //quarter-tone accidentals that include arrows pointing down or up
//        else if (acc == "sharp-down")
//        else if (acc == "sharp-up")
//        else if (acc == "natural-down")
//        else if (acc == "natural-up")
//        else if (acc == "flat-down")
//        else if (acc == "flat-up")
//        else if (acc == "triple-sharp")
//        else if (acc == "triple-flat")
//        //used in Turkish classical music
//        else if (acc == "slash-quarter-sharp")
//        else if (acc == "slash-sharp")
//        else if (acc == "slash-flat")
//        else if (acc == "double-slash-flat")
//        //superscripted versions of the accidental signs, used in Turkish folk music
//        else if (acc == "sharp-1")
//        else if (acc == "sharp-2")
//        else if (acc == "sharp-3")
//        else if (acc == "sharp-5")
//        else if (acc == "flat-1")
//        else if (acc == "flat-2")
//        else if (acc == "flat-3")
//        else if (acc == "flat-4")
//        //microtonal sharp and flat accidentals used in Iranian and Persian music
//        else if (acc == "sori")
//        else if (acc == "koron")

        else
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Invalid or not supported <accidentals> value '" + acc + "'. Ignored.");
        }
        pNote->set_notated_accidentals(accidentals);
        if (accidentals != k_no_accidentals)
            pNote->force_to_display_accidentals();
    }

    //----------------------------------------------------------------------------------
    void set_stem(ImoNote* pNote)
    {
        string type = m_childToAnalyse.value();
        ENoteStem value = k_stem_default;

        if (type == "none")
            value = k_stem_none;
        else if (type == "up")
            value = k_stem_up;
        else if (type == "down")
            value = k_stem_down;
        else if (type == "double")
            value = k_stem_double;
        else
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Invalid or not supported <stem> value '" + type + "'. Replaced by 'default'.");
        }
        pNote->set_stem_direction(value);
    }

    //----------------------------------------------------------------------------------
    void set_duration(ImoNoteRest* pNR)
    {
        pNR->set_note_type_and_dots(k_whole, 0);
    }

    //----------------------------------------------------------------------------------
    void analyse_beam()
    {
        //@ <!ELEMENT beam (#PCDATA)>
        //@ <!ATTLIST beam number %beam-level; "1" repeater %yes-no; #IMPLIED >

        // attrib: number
        const string& level = m_childToAnalyse.attribute_value("number");
        int iLevel;
        if (m_pAnalyser->to_integer(level, &iLevel))
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Missing or invalid beam number '" + level + "'. Beam ignored.");
            return;
        }

        if (iLevel <= 0 || iLevel > 6)
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Invalid beam number '" + level +"'. Beam ignored.");
            return;
        }

        // value: beam type
        const string& type = m_childToAnalyse.value();
        int iType = ImoBeam::k_none;
        if (type == "begin")
            iType = ImoBeam::k_begin;
        else if (type == "continue")
            iType = ImoBeam::k_continue;
        else if (type == "end")
            iType = ImoBeam::k_end;
        else if (type == "forward hook")
            iType = ImoBeam::k_forward;
        else if (type == "backward hook")
            iType = ImoBeam::k_backward;
        else
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Invalid or not supported <beam> value '" + type + "'. Beam ignored");
            return;
        }

        if (m_pBeamInfo == nullptr)
            m_pBeamInfo = LOMSE_NEW ImoBeamDto();

        m_pBeamInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
        m_pBeamInfo->set_beam_type(--iLevel, iType);
    }

    //----------------------------------------------------------------------------------
    void add_beam_info(ImoNoteRest* pNR)
    {
        if (m_pBeamInfo)
        {
            m_pBeamInfo->set_note_rest(pNR);
            m_pAnalyser->add_relation_info(m_pBeamInfo);
        }
    }

    //----------------------------------------------------------------------------------
    void add_to_spanners(ImoNote* pNote)
    {
        m_pAnalyser->add_to_open_octave_shifts(pNote);
    }

    //----------------------------------------------------------------------------------
    void set_voice(ImoNoteRest* pNR, int voice)
    {
        m_pAnalyser->set_current_voice(voice);
        pNR->set_voice(voice);
    }


};

//@--------------------------------------------------------------------------------------
//@ <part> = <measure>*
//@ attrb:   attributeGroup ref="part-attributes"/>

class PartMxlAnalyser : public MxlElementAnalyser
{
public:
    PartMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        //attrb: id
        string id = get_optional_string_attribute("id", "");
        if (id.empty())
        {
            error_msg("<part>: missing mandatory 'id' attribute. <part> content will be ignored");
            return nullptr;
        }
        ImoInstrument* pInstr = m_pAnalyser->get_instrument(id);
        if (pInstr==nullptr)
        {
            error_msg("No <score-part> found for part id='" + id + "'. <part> content will be ignored.");
            return nullptr;
        }
        if (m_pAnalyser->mark_part_as_added(id))
        {
            error_msg("Duplicated <part> for part id='" + id + "'. <part> content will be ignored.");
            return nullptr;
        }

        m_pAnalyser->save_current_part_id(id);
        m_pAnalyser->prepare_for_new_instrument_content();
        m_pAnalyser->save_current_instrument(pInstr);
        ImoMusicData* pMD = pInstr->get_musicdata();

        // <measure>*
        while (analyse_optional("measure", pMD));

        error_if_more_elements();

        add_to_model(pMD);
        return pMD;
    }

};

//@--------------------------------------------------------------------------------------
//@ <part-group>
//@
//@ Doc: The part-group element indicates groupings of parts in the score, usually indicated
//@      by braces and brackets. Braces that are used for multi-staff parts should be defined
//@      in the attributes element for that part. The part-group start element appears before
//@      the first score-part in the group. The part-group stop element appears after the last
//@      score-part in the group.
//
//The number attribute is used to distinguish overlapping and nested part-groups, not the sequence of groups. As with parts, groups can have a name and abbreviation. Values for the child elements are ignored at the stop of a group.
//
//A part-group element is not needed for a single multi-staff part. By default, multi-staff parts include a brace symbol and (if appropriate given the bar-style) common barlines. The symbol formatting for a multi-staff part can be more fully specified using the part-symbol element.</xs:documentation>
//        </xs:annotation>
//        <xs:sequence>
//            <xs:element name="group-name" type="group-name" minOccurs="0"/>
//            <xs:element name="group-name-display" type="name-display" minOccurs="0">
//                <xs:annotation>
//                    <xs:documentation>Formatting specified in the group-name-display element overrides formatting specified in the group-name element.</xs:documentation>
//                </xs:annotation>
//            </xs:element>
//            <xs:element name="group-abbreviation" type="group-name" minOccurs="0"/>
//            <xs:element name="group-abbreviation-display" type="name-display" minOccurs="0">
//                <xs:annotation>
//                    <xs:documentation>Formatting specified in the group-abbreviation-display element overrides formatting specified in the group-abbreviation element.</xs:documentation>
//                </xs:annotation>
//            </xs:element>
//            <xs:element name="group-symbol" type="group-symbol" minOccurs="0"/>
//            <xs:element name="group-barline" type="group-barline" minOccurs="0"/>
//            <xs:element name="group-time" type="empty" minOccurs="0">
//                <xs:annotation>
//                    <xs:documentation>The group-time element indicates that the displayed time signatures should stretch across all parts and staves in the group.</xs:documentation>
//                </xs:annotation>
//            </xs:element>
//            <xs:group ref="editorial"/>
//        </xs:sequence>
//        <xs:attribute name="type" type="start-stop" use="required"/>
//        <xs:attribute name="number" type="xs:token" default="1"/>
//    </xs:complexType>




//@--------------------------------------------------------------------------------------
//@ <octave-shift>
//@
//@ <!ELEMENT octave-shift EMPTY>
//@ <!ATTLIST octave-shift
//@     type (up | down | stop | continue) #REQUIRED
//@     number %number-level; #IMPLIED
//@     size CDATA "8"
//@     %dashed-formatting;
//@     %print-style;
//@     %optional-unique-id;
//@ >
//@
class OctaveShiftMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoOctaveShiftDto* m_pInfo1;
    ImoOctaveShiftDto* m_pInfo2;

public:
    OctaveShiftMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pInfo1(nullptr)
        , m_pInfo2(nullptr)
    {
    }

    ImoObj* do_analysis()
    {
        ImoDirection* pDirection = nullptr;
        if (m_pAnchor && m_pAnchor->is_direction())
            pDirection = static_cast<ImoDirection*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoDirection");
            error_msg("<direction-type> <octave-shift> is not child of <direction>. Ignored.");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pInfo1 = static_cast<ImoOctaveShiftDto*>(
                                ImFactory::inject(k_imo_octave_shift_dto, pDoc));
        m_pInfo1->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        // attrib: type (up | down | stop | continue) #REQUIRED
        const string& type = get_mandatory_string_attribute("type", "", "octave-shift");

        // attrib: number %number-level; #IMPLIED
        int num = get_optional_int_attribute("number", 1);

        // attrib: size CDATA "8"
        int size = get_optional_int_attribute("size", 8);
        if (!(size == 8 || size == 15))
        {
            const string& value = get_optional_string_attribute("size", "");
            error_msg("Invalid octave-shift size '" + value + "'. Changed to 8.");
            size = 8;
        }

        set_mandatory_data(type, num, size);

        //TODO
        // attrib: %dashed-formatting;
        // attrib: %print-style;
        // attrib: %optional-unique-id;

        if (m_pInfo1)
        {
            int iStaff = pDirection->get_staff();

            m_pInfo1->set_staffobj(nullptr);
            m_pInfo1->set_staff(iStaff);
            m_pAnalyser->add_relation_info(m_pInfo1);

            if (m_pInfo2)
            {
                m_pInfo2->set_staffobj(nullptr);
                m_pInfo2->set_staff(iStaff);
                m_pAnalyser->add_relation_info(m_pInfo2);
            }
        }

        return nullptr;     //m_pInfo1 has been deleted in add_relation_info()
    }

protected:

    void set_mandatory_data(const string& value, int num, int size)
    {
        if (value == "up" || value == "down")
        {
            m_pInfo1->set_start(true);
            int id =  m_pAnalyser->new_octave_shift_id(num);
            m_pInfo1->set_octave_shift_number(id);
            --size;
            if (value == "down")
                size = -size;
            m_pInfo1->set_shift_steps(size);
        }
        else if (value == "stop")
        {
            m_pInfo1->set_start(false);
            int id =  m_pAnalyser->get_octave_shift_id_and_close(num);
            m_pInfo1->set_octave_shift_number(id);
        }
//        else if (value == "continue")
//        {
//            m_pInfo1->set_start(false);
//            int id =  m_pAnalyser->get_octave_shift_id_and_close(num);
//            m_pInfo1->set_octave_shift_number(id);
//
//            Document* pDoc = m_pAnalyser->get_document_being_analysed();
//            m_pInfo2 = static_cast<ImoWedgeDto*>(
//                                ImFactory::inject(k_imo_octave_shift_dto, pDoc));
//            m_pInfo2->set_start(true);
//            m_pInfo2->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
//            id =  m_pAnalyser->new_octave_shift_id(num);
//            m_pInfo2->set_octave_shift_number(wedgeId);
//        }
        else
        {
            error_msg("Missing or invalid octave-shift type '" + value
                      + "'. Octave-shift ignored.");
            delete m_pInfo1;
            m_pInfo1 = nullptr;
        }
    }
};

//@--------------------------------------------------------------------------------------
//@ <ornaments> = (ornaments [<ornament> | <accidental-mark>+ ]+ )
//@ <ornament> = [trill-mark | turn | delayed-turn | inverted-turn |
//@               delayed-inverted-turn | vertical-turn | shake |
//@               wavy-line | mordent | inverted-mordent | schleifer |
//@               tremolo | other-ornament]
// Examples:
//      <ornaments><tremolo>3</tremolo></ornaments>
//      <ornaments>
//          <turn/>
//          <accidental-mark>natural</accidental-mark>
//      </ornaments>
//      <ornaments>
//          <wavy-line placement="below" type="stop"/>
//      </ornaments>
//      <ornaments><mordent/></ornaments>
//      <ornaments>
//          <inverted-mordent long="yes" placement="above"/>
//      </ornaments>

class OrnamentsMxlAnalyser : public MxlElementAnalyser
{
public:
    OrnamentsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoNoteRest* pNR = nullptr;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoNoteRest");
            return nullptr;
        }

        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            if (m_childToAnalyse.name() == "trill-mark")
            {
                get_ornament_symbol(pNR, k_ornament_trill_mark);
            }
            else if (m_childToAnalyse.name() == "delayed-inverted-turn")
            {
                get_ornament_symbol(pNR, k_ornament_delayed_inverted_turn);
            }
            else if (m_childToAnalyse.name() == "vertical-turn")
            {
                get_ornament_symbol(pNR, k_ornament_vertical_turn);
            }
            else if (m_childToAnalyse.name() == "shake")
            {
                get_ornament_symbol(pNR, k_ornament_shake);
            }
            else if (m_childToAnalyse.name() == "wavy-line")
            {
                get_ornament_wavy_line(pNR);
            }
            else if (m_childToAnalyse.name() == "turn")
            {
                get_ornament_symbol(pNR, k_ornament_turn);
            }
            else if (m_childToAnalyse.name() == "delayed-turn")
            {
                get_ornament_symbol(pNR, k_ornament_delayed_turn);
            }
            else if (m_childToAnalyse.name() == "inverted-turn")
            {
                get_ornament_symbol(pNR, k_ornament_inverted_turn);
            }
            else if (m_childToAnalyse.name() == "mordent")
            {
                get_ornament_symbol(pNR, k_ornament_mordent);
            }
            else if (m_childToAnalyse.name() == "inverted-mordent")
            {
                get_ornament_symbol(pNR, k_ornament_inverted_mordent);
            }
            else if (m_childToAnalyse.name() == "schleifer")
            {
                get_ornament_symbol(pNR, k_ornament_schleifer);
            }
            else if (m_childToAnalyse.name() == "tremolo")
            {
                get_ornament_symbol(pNR, k_ornament_tremolo);
            }
            else if (m_childToAnalyse.name() == "other-ornament")
            {
                get_ornament_symbol(pNR, k_ornament_other);
            }
            else if (m_childToAnalyse.name() == "accidental-mark")
            {
                get_accidental_mark(pNR);
            }
            else
            {
                error_invalid_child();
            }
            move_to_next_child();
        }

        error_if_more_elements();

        return nullptr;
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoOrnament* get_ornament_symbol(ImoNoteRest* pNR, int type)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoOrnament* pImo = static_cast<ImoOrnament*>(
                                ImFactory::inject(k_imo_ornament, pDoc) );
        pImo->set_ornament_type(type);

        // [attrib]: placement (above | below)
        if (has_attribute(&m_childToAnalyse, "placement"))
            set_placement(pImo);

        pNR->add_attachment(pDoc, pImo);
        return pImo;
    }

    //-----------------------------------------------------------------------------------
    void get_ornament_wavy_line(ImoNoteRest* pNR)
    {
        //ImoOrnament* pImo =
            get_ornament_symbol(pNR, k_ornament_wavy_line);

//        // [attrib]: type (up | down)
//        if (has_attribute(&m_childToAnalyse, "type"))
//            set_type(pImo);
    }

    //-----------------------------------------------------------------------------------
    void get_accidental_mark(ImoNoteRest* UNUSED(pNR))
    {
//        ImoOrnament* pImo =
//            get_ornament_symbol(pNR, k_ornament_breath_mark);
//
//        // [attrib]: type (up | down)
//        if (has_attribute(&m_childToAnalyse, "type"))
//            set_breath_mark_type(pImo);
    }

    //-----------------------------------------------------------------------------------
    void set_placement(ImoOrnament* pImo)
    {
        string value = get_attribute(&m_childToAnalyse, "placement");
        if (value == "above")
            pImo->set_placement(k_placement_above);
        else if (value == "below")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown placement attrib. '" + value + "'. Ignored.");
        }
    }

    //-----------------------------------------------------------------------------------
    void set_type(ImoOrnament* UNUSED(pImo))
    {
//        string value = get_attribute(&m_childToAnalyse, "type");
//        if (value == "up")
//            pImo->set_up(true);
//        else if (value == "below")
//            pImo->set_up(false);
//        else
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
//                "Unknown type attrib. '" + value + "'. Ignored.");
//        }
    }

};


//@--------------------------------------------------------------------------------------
//@ <!ELEMENT part-group (group-name?, group-name-display?,
//@           group-abbreviation?, group-abbreviation-display?,
//@           group-symbol?, group-barline?, group-time?, %editorial;)>
//@
//@ attrb:  number="4" type="start"
//
class PartGroupMxlAnalyser : public MxlElementAnalyser
{
public:
    PartGroupMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                         LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //attrb: number
        int number = get_attribute_as_integer("number", -1);
        if (number == -1)
        {
            error_msg("<part-group>: invalid or missing mandatory 'number' attribute."
                      " Tag ignored.");
            return nullptr;
        }

        //attrb: type = "start | stop"
        string type = get_optional_string_attribute("type", "");
        if (type.empty())
        {
            error_msg("<part-group>: missing mandatory 'type' attribute. Tag ignored.");
            return nullptr;
        }

        if (type == "stop")
        {
            ImoInstrGroup* pGrp = m_pAnalyser->get_part_group(number);
            if (pGrp)
            {
                ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
                pScore->add_instruments_group(pGrp);
                m_pAnalyser->terminate_part_group(number);
                return pGrp;
            }
            else
            {
                error_msg("<part-group> type='stop': missing <part-group> with the same number and type='start'.");
                return nullptr;
            }
        }

        if (type != "start")
        {
            error_msg("<part-group>: invalid mandatory 'type' attribute. Must be "
                      "'start' or 'stop'.");
            return nullptr;
        }

        ImoInstrGroup* pGrp = m_pAnalyser->start_part_group(number);
        if (pGrp == nullptr)
        {
            error_msg("<part-group> type=start for number already started and not stopped");
            return nullptr;
        }

        // group-name?
        if (get_optional("group-name"))
        {
            ImoScoreText* pText = get_name_abbrev();
            if (pText)
                pGrp->set_name(pText);
        }

        // group-name-display?
        if (get_optional("group-name-display"))
        {
            ;   //TODO
        }

        // group-abbreviation?
        if (get_optional("group-abbreviation"))
        {
            ImoScoreText* pText = get_name_abbrev();
            if (pText)
                pGrp->set_abbrev(pText);
        }

        // group-abbreviation-display?
        if (get_optional("group-abbreviation-display"))
        {
            ;   //TODO
        }

        // group-symbol?
        if (get_optional("group-symbol"))
        {
            set_symbol(pGrp);
        }

        // group-barline?
        if (get_optional("group-barline"))
        {
            set_join_barlines(pGrp);
        }

        // group-time?
        if (get_optional("group-time"))
        {
            ;   //TODO
        }

        error_if_more_elements();

        return nullptr;
    }

protected:

    void set_symbol(ImoInstrGroup* pGrp)
    {
        string symbol = m_childToAnalyse.first_child().value();
        if (symbol == "brace")
            pGrp->set_symbol(ImoInstrGroup::k_brace);
        else if (symbol == "bracket")
            pGrp->set_symbol(ImoInstrGroup::k_bracket);
        else if (symbol == "line")
            pGrp->set_symbol(ImoInstrGroup::k_line);
        else if (symbol == "none")
            pGrp->set_symbol(ImoInstrGroup::k_none);
        else
            error_msg("Invalid value for <group-symbol>. Must be "
                      "'none', 'brace', 'line' or 'bracket'. 'none' assumed.");
    }

    void set_join_barlines(ImoInstrGroup* pGrp)
    {
        string value = m_childToAnalyse.value();
        if (value == "yes")
            pGrp->set_join_barlines(ImoInstrGroup::k_standard);
        else if (value == "no")
            pGrp->set_join_barlines(ImoInstrGroup::k_no);
        else if (value == "Mensurstrich")
            pGrp->set_join_barlines(ImoInstrGroup::k_mensurstrich);
        else
        {
            pGrp->set_join_barlines(ImoInstrGroup::k_standard);
            error_msg("Invalid value for <group-barline>. Must be "
                      "'yes', 'no' or 'Mensurstrich'. 'yes' assumed.");
        }
    }

    ImoScoreText* get_name_abbrev()
    {
        string name = m_childToAnalyse.value();
        if (!name.empty())
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoScoreText* pText = static_cast<ImoScoreText*>(
                                        ImFactory::inject(k_imo_score_text, pDoc));
            pText->set_text(name);

            ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
            ImoStyle* pStyle = nullptr;
            if (pScore)     //in unit tests the score might not exist
                pStyle = pScore->get_default_style();
            pText->set_style(pStyle);
            return pText;
        }
        return nullptr;
    }
};


//@--------------------------------------------------------------------------------------
//@ <!ELEMENT part-list (part-group*, score-part, (part-group | score-part)*)>
//@ attrb:
//@ Doc:  the <part-list> element lists all the parts or instruments in a musical score
//
// http://www.musicxml.com/tutorial/file-structure/score-header-entity/

class PartListMxlAnalyser : public MxlElementAnalyser
{
public:
    PartListMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope) {}

    ImoObj* do_analysis()
    {
        // part-group*
        while (analyse_optional("part-group"));

        // score-part
        analyse_mandatory("score-part");

        // { part-group | score-part }*
        while (more_children_to_analyse())
        {
            if (analyse_optional("score-part"))
                ;
            else if (analyse_optional("part-group"))
                ;
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        error_if_more_elements();

        m_pAnalyser->check_if_all_groups_are_closed();

        return nullptr;
    }

protected:
};

//@--------------------------------------------------------------------------------------
//@ <part-name> = string
//@ attrb:   print-object="no"
//@ Doc:  Introduced in 1.1, but deprecated in 2.0 in favor of the new part-name-display

class PartNameMxlAnalyser : public MxlElementAnalyser
{
public:
    PartNameMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        //attrb: print-object
        string print = get_optional_string_attribute("print-object", "yes");
        bool fVisible = (print == "yes" ? true : false);

        if (fVisible)
        {
            //get value
            string name = m_analysedNode.value();
            if (!name.empty())
            {
                Document* pDoc = m_pAnalyser->get_document_being_analysed();
                ImoScoreText* pText = static_cast<ImoScoreText*>(
                            ImFactory::inject(k_imo_score_text, pDoc));
                pText->set_text(name);


                // [<style>]
                ImoStyle* pStyle = nullptr;
    //            if (get_optional(k_style))
    //                pStyle = get_text_style_param(m_styleName);
    //            else
                {
                    ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
                    if (pScore)     //in unit tests the score might not exist
                        pStyle = pScore->get_default_style();
                }
                pText->set_style(pStyle);

                add_to_model(pText, k_name);
            }
        }

        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <pedal>
class PedalMxlAnalyser : public MxlElementAnalyser
{
public:
    PedalMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <percussion>
class PercussionMxlAnalyser : public MxlElementAnalyser
{
public:
    PercussionMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                          LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <pitch> = <step>[<alter>]<octave>
//@ attrb:   none

class PitchMxlAnalyser : public MxlElementAnalyser
{
public:
    PitchMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        //anchor object is ImoNote
        ImoNote* pNote = nullptr;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not note");
            return nullptr;
        }

        // <step>
        string step = (get_mandatory("step") ? m_childToAnalyse.value() : "C");

        // [<alter>]
        string accidentals = (get_optional("alter") ? m_childToAnalyse.value() : "0");

        // <octave>
        string octave = (get_mandatory("octave") ? m_childToAnalyse.value() : "4");

        error_if_more_elements();

        int nStep = mxl_step_to_step(step);
        float acc = mxl_alter_to_accidentals(accidentals);
        int nOctave = mxl_octave_to_octave(octave);
        pNote->set_pitch(nStep, nOctave, acc);
        return pNote;
    }

protected:

    int mxl_step_to_step(const string& step)
    {
        switch (step[0])
        {
            case 'A':	return k_step_A;
            case 'B':	return k_step_B;
            case 'C':	return k_step_C;
            case 'D':	return k_step_D;
            case 'E':	return k_step_E;
            case 'F':	return k_step_F;
            case 'G':	return k_step_G;
            default:
            {
                //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                error_msg2(
                    "Unknown note step '" + step + "'. Replaced by 'C'.");
                return k_step_C;
            }
        }
    }

    int mxl_octave_to_octave(const string& octave)
    {
        //@ MusicXML octaves are represented by the numbers 0 to 9, where 4
        //@ indicates the octave started by middle C.

        switch (octave[0])
        {
            case '0':	return 0;
            case '1':	return 1;
            case '2':	return 2;
            case '3':	return 3;
            case '4':	return 4;
            case '5':	return 5;
            case '6':	return 6;
            case '7':	return 7;
            case '8':	return 8;
            case '9':	return 9;
            default:
            {
                //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                error_msg2(
                    "Unknown octave '" + octave + "'. Replaced by '4'.");
                return 4;
            }
        }
    }

    float mxl_alter_to_accidentals(const string& accidentals)
    {
        //@ The <alter> element is needed for the sounding pitch, whether the
        //@ accidental is in the key signature or not. If you want to see an
        //@ accidental, you need to use the <accidental> element. The <alter> is
        //@ for what you hear; the <accidental> is for what you see.
        //@
        //@ The alter element represents chromatic alteration in number of
        //@ semitones (e.g., -1 for flat, 1 for sharp). Decimal values like 0.5
        //@ (quarter tone sharp) are used for microtones.
        //@ AWARE: <alter> is for pitch, not for displayed accidental. The displayed
        //@ accidentals is encoded in an <accidental> element

        float number;
        std::istringstream iss(accidentals);
        if ((iss >> number).fail())
        {
            error_msg2(
                "Invalid or not supported <alter> value '" + accidentals + "'. Ignored.");
            return 0.0f;
        }
        return number;
    }

};

//@--------------------------------------------------------------------------------------
//@ <principal-voice>
class PrincipalVoiceMxlAnalyser : public MxlElementAnalyser
{
public:
    PrincipalVoiceMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                            LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ print
//@ <!ELEMENT print (page-layout?, system-layout?, staff-layout*,
//@     measure-layout?, measure-numbering?, part-name-display?,
//@     part-abbreviation-display?)>
//@ <!ATTLIST print
//@     staff-spacing %tenths; #IMPLIED
//@     new-system %yes-no; #IMPLIED
//@     new-page %yes-no; #IMPLIED
//@     blank-page NMTOKEN #IMPLIED
//@     page-number CDATA #IMPLIED
//@     %optional-unique-id;
//@ >
//@
class PrintMxlAnalyser : public MxlElementAnalyser
{
public:
    PrintMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //TODO: Finish this

            //attribs

        // staff-spacing %tenths; #IMPLIED
        // new-system %yes-no; #IMPLIED
        // new-page %yes-no; #IMPLIED
        // blank-page NMTOKEN #IMPLIED
        // page-number CDATA #IMPLIED
        // %optional-unique-id;

            // elements


        // page-layout?
        if (get_optional("page-layout"))
        {

        }

        // system-layout?
        if (get_optional("system-layout"))
        {

        }

        // staff-layout*
        while (get_optional("staff-layout"))
        {

        }

        // measure-layout?
        if (get_optional("measure-layout"))
        {

        }

        // measure-numbering?
        if (get_optional("measure-numbering"))
            set_measures_numbering();

        // part-name-display?
        if (get_optional("part-name-display"))
        {

        }

        // part-abbreviation-display?
        if (get_optional("part-abbreviation-display"))
        {

        }

        return nullptr;
    }

protected:

    void set_measures_numbering()
    {
        string numbering = m_childToAnalyse.value();
        ImoInstrument* pInstr = m_pAnalyser->get_current_instrument();
        if (numbering == "system")
            pInstr->set_measures_numbering(ImoInstrument::k_system);
        else if (numbering == "measure")
            pInstr->set_measures_numbering(ImoInstrument::k_all);
        else if (numbering == "none")
            pInstr->set_measures_numbering(ImoInstrument::k_none);
        else
        {
            error_msg2(
                "Invalid value '" + numbering + "'. Value 'none' assumed.");
            pInstr->set_measures_numbering(ImoInstrument::k_none);
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <rehearsal>
class RehearsalMxlAnalyser : public MxlElementAnalyser
{
public:
    RehearsalMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                         LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};


//@--------------------------------------------------------------------------------------
//@ score-instrument
//<!ELEMENT score-instrument
//    (instrument-name, instrument-abbreviation?,
//     instrument-sound?, (solo | ensemble)?,
//     virtual-instrument?)>
//<!ATTLIST score-instrument
//    id ID #REQUIRED
//>
//<!ELEMENT instrument-name (#PCDATA)>
//<!ELEMENT instrument-abbreviation (#PCDATA)>
//<!ELEMENT instrument-sound (#PCDATA)>
//<!ELEMENT solo EMPTY>
//<!ELEMENT ensemble (#PCDATA)>
//<!ELEMENT virtual-instrument
//    (virtual-library?, virtual-name?)>
//<!ELEMENT virtual-library (#PCDATA)>
//<!ELEMENT virtual-name (#PCDATA)>

class ScoreInstrumentMxlAnalyser : public MxlElementAnalyser
{
public:
    ScoreInstrumentMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                               LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>(m_pAnchor);
        if (!pInstr)
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoInstrument");
            return nullptr;
        }

        //attrb: id
        string id = get_mandatory_string_attribute("id", "", "score-instrument");
        if (id.empty())
            return nullptr;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(
                    ImFactory::inject(k_imo_sound_info, pDoc) );

        pInfo->set_score_instr_id(id);
        m_pAnalyser->create_index_for_sound(id);
        m_pAnalyser->set_latest_midi_info_for(id, pInfo->get_midi_info());

        // instrument-name
        pInfo->set_score_instr_name(
                    analyze_mandatory_child_pcdata("instrument-name") );

        // instrument-abbreviation?
        pInfo->set_score_instr_abbrev(
                    analyze_optional_child_pcdata("instrument-abbreviation", "") );

        // instrument-sound?
        pInfo->set_score_instr_sound(
                    analyze_optional_child_pcdata("instrument-sound", "") );

        // (solo | ensemble)?
        bool fSolo = get_optional("solo");
        pInfo->set_score_instr_solo(true);

        if (get_optional("ensemble"))
        {
            if (fSolo)
                error_msg("'ensemble' element ignored. Element 'solo' is also specified.");
            else
            {
                pInfo->set_score_instr_ensemble(true);
                pInfo->set_score_instr_ensemble_size(
                    analyze_optional_child_pcdata_int("ensemble", 1, 100000, 0) );
            }
        }

        // virtual-instrument?
        analyse_optional("virtual-instrument", pInfo);

        error_if_more_elements();

        pInstr->add_sound_info(pInfo);
        return nullptr;
    }
};


//@--------------------------------------------------------------------------------------
//<!ELEMENT score-part (identification?,
//    part-name, part-name-display?,
//    part-abbreviation?, part-abbreviation-display?,
//    group*, score-instrument*,
//    (midi-device?, midi-instrument?)*)>
//<!ATTLIST score-part
//    id ID #REQUIRED
//>
//
class ScorePartMxlAnalyser : public MxlElementAnalyser
{
public:
    ScorePartMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope) {}

    ImoObj* do_analysis()
    {
        //attrb: id
        string id = get_mandatory_string_attribute("id", "", "score-part");
        if (id.empty())
            return nullptr;

        ImoInstrument* pInstrument = create_instrument(id);

        // identification?
        analyse_optional("identification", pInstrument);

        // part-name
        analyse_optional("part-name", pInstrument);

        // part-name-display?
        analyse_optional("part-name-display", pInstrument);

        // part-abbreviation?
        pInstrument->set_abbrev(
            analyze_optional_child_pcdata("part-abbreviation", "") );
        //TODO: full analysis. class PartAbbrevMxlAnalyser

        // part-abbreviation-display?
        analyse_optional("part-abbreviation-display", pInstrument);

        // group*
        while (analyse_optional("group", pInstrument));

        // score-instrument*
        bool fScoreInstr = false;
        while (analyse_optional("score-instrument", pInstrument))
            fScoreInstr = true;

        // (midi-device?, midi-instrument?)*
        // score-instrument is mandatory if midi-device or midi-instrument defined
        if (fScoreInstr)
        {
            ImoSounds* pSounds = pInstrument->get_sounds();
            while (more_children_to_analyse())
            {
                if (!(analyse_optional("midi-device", pSounds)
                      || analyse_optional("midi-instrument", pSounds)
                    ))
                {
                    break;
                }
            }
        }

        error_if_more_elements();

        return pInstrument;
    }

protected:

    ImoInstrument* create_instrument(const string& id)
    {
        m_pAnalyser->clear_pending_relations();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrument* pInstrument = static_cast<ImoInstrument*>(
                                        ImFactory::inject(k_imo_instrument, pDoc) );
        ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, pDoc) );
        pInstrument->set_instr_id(id);

        Linker linker(pDoc);
        linker.add_child_to_model(pInstrument, pMD, pMD->get_obj_type());

        m_pAnalyser->add_score_part(id, pInstrument);
        m_pAnalyser->save_current_instrument(pInstrument);
        return pInstrument;
    }

};

//@--------------------------------------------------------------------------------------
//@ <score-partwise> = [<work>][<movement-number>][<movement-title>][<identification>]
//@                    [<defaults>][<credit>*]<part-list><part>+
//@ attrb: name="version" type="xs:token" default="1.0"
//@ Doc: added in version 1.1. If not present, assume 1.0

class ScorePartwiseMxlAnalyser : public MxlElementAnalyser
{
public:
    ScorePartwiseMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope) {}

    ImoObj* do_analysis()
    {
        ImoDocument* pImoDoc = nullptr;

        //create the document
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        pImoDoc = static_cast<ImoDocument*>(
                    ImFactory::inject(k_imo_document, pDoc));
        pImoDoc->set_version("0.0");    //AWARE: This is lenmusdoc version!
        pImoDoc->set_language("en");    //TODO: analyse language
        m_pAnalyser->save_root_imo_document(pImoDoc);
        m_pAnchor = pImoDoc;

        // attrb: version
        string version = get_optional_string_attribute("version", "1.0");
        m_pAnalyser->set_musicxml_version(version);
        //AWARE:
        //version value should be used for selecting an specific derived class to use
        //for the remaining of the analysis. Something as:
        //
        //  MxlElementAnalyser* a;
        //  if (version == "1.0")
        //      a = LOMSE_NEW MxlAnalyser10();
        //  else if (version == "1.1")
        //      a = LOMSE_NEW MxlAnalyser11();
        //  else if (version == "2.0")
        //      a = LOMSE_NEW MxlAnalyser20();
        //  else if (version == "3.0")
        //      a = LOMSE_NEW MxlAnalyser30();
        //  else
        //      return error;
        //  a->analyse_partwise();
        //  delete a;
        //
        //But, as MusicXML changes between versions are not huge and are backwards
        //compatible, I prefer to be practical and deal with version differences
        //in each specific MxlElementAnalyser.

        ImoScore* pScore = create_score();

        //TODO: deal with ignored elements
        // [<work>]
        // coverity[check_return]
        get_optional("work");
        // [<movement-number>]
        // coverity[check_return]
        get_optional("movement-number");
        // [<movement-title>]
        // coverity[check_return]
        get_optional("movement-title");
        // [<identification>]
        // coverity[check_return]
        get_optional("identification");
        // [<defaults>]
        // coverity[check_return]
        get_optional("defaults");
        // [<credit>*]
        while (get_optional("credit"));

        // add default styles
        add_default(pImoDoc);

        // <part-list>
        if (!analyse_optional("part-list"))
        {
            error_missing_element("part-list");
            remove_score(pImoDoc, pScore);
            return pImoDoc;
        }
        if (!m_pAnalyser->part_list_is_valid())
        {
            error_msg("errors in <part-list>. Analysis stopped.");
            remove_score(pImoDoc, pScore);
            return pImoDoc;
        }
        add_all_instruments(pScore);

        // <part>*
        while (more_children_to_analyse())
        {
            analyse_mandatory("part", pScore);
        }
        error_if_more_elements();

        check_if_missing_parts();

        //m_pAnalyser->score_analysis_end();
        set_options(pScore);
        return pImoDoc;
    }

protected:

    void add_default(ImoDocument* pImoDoc)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        Linker linker(pDoc);
        ImoStyles* pStyles = static_cast<ImoStyles*>(
                                    ImFactory::inject(k_imo_styles, pDoc));
        linker.add_child_to_model(pImoDoc, pStyles, k_styles);
        ImoStyle* pDefStyle = pImoDoc->get_default_style();
        pImoDoc->set_style(pDefStyle);
    }

    ImoScore* create_score()
    {
        //add an empty score
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoContent* pContent = static_cast<ImoContent*>(
                        ImFactory::inject(k_imo_content, pDoc) );
        add_to_model(pContent);
        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, pDoc));
        m_pAnalyser->score_analysis_begin(pScore);
        add_to_model(pScore);
        m_pAnchor = pScore;

        pScore->set_version(160);   //use version 1.6 to allow using ImoFwdBack
        pScore->add_required_text_styles();

        return pScore;
    }

    void set_options(ImoScore* pScore)
    {
        //justify last system except for very short scores (less than 5 measures)
        ImoOptionInfo* pOpt = pScore->get_option("Score.JustifyLastSystem");
        if (m_pAnalyser->get_measures_counter() < 5)
        {
            pOpt->set_long_value(k_justify_never);
            pOpt = pScore->get_option("StaffLines.Truncate");
            pOpt->set_long_value(k_truncate_always);
        }
        else
            pOpt->set_long_value(k_justify_always);

        pOpt = pScore->get_option("Render.SpacingOptions");
        pOpt->set_long_value(k_render_opt_breaker_optimal | k_render_opt_dmin_global);
    }

    void remove_score(ImoDocument* pImoDoc, ImoScore* pScore)
    {
        pImoDoc->delete_block_level_obj(pScore);
    }

    void add_all_instruments(ImoScore* pScore)
    {
        m_pAnalyser->add_all_instruments(pScore);
    }

    void check_if_missing_parts()
    {
        m_pAnalyser->check_if_missing_parts();
    }

};

//@--------------------------------------------------------------------------------------
//@ <scordatura>
class ScordaturaMxlAnalyser : public MxlElementAnalyser
{
public:
    ScordaturaMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                          LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <segno>
//@ Segno signs can be associated with a measure or a musical direction.
//@ It is a visual indicator only; a sound element is needed for reliably playback.
//@
//@<!ELEMENT segno EMPTY>
//@<!ATTLIST segno
//@    %print-style-align;
//@>
//
class SegnoMxlAnalyser : public MxlElementAnalyser
{
public:
    SegnoMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoDirection* pDirection = nullptr;
        if (m_pAnchor && m_pAnchor->is_direction())
            pDirection = static_cast<ImoDirection*>(m_pAnchor);
        else
        {
            //TODO: deal with <segno> when child of <measure>
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoDirection");
            error_msg("<direction-type> <segno> is not child of <direction>. Ignored.");
            return nullptr;
        }
        pDirection->set_display_repeat(k_repeat_segno);

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSymbolRepetitionMark* pImo = static_cast<ImoSymbolRepetitionMark*>(
            ImFactory::inject(k_imo_symbol_repetition_mark, pDoc) );
        pImo->set_symbol(ImoSymbolRepetitionMark::k_segno);

        // attrib: %print-style-align;
        get_attributes_for_print_style_align(pImo);

        pDirection->add_attachment(pDoc, pImo);
        return pImo;
    }
};

//@--------------------------------------------------------------------------------------
//@ <string-mute>
class StringMmuteMxlAnalyser : public MxlElementAnalyser
{
public:
    StringMmuteMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                           LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <technical> = (technical <tech-mark>+)
//@ <tech-mark> = [ up-bow | down-bow | harmonic | open-string |
//@                 thumb-position | fingering | pluck | double-tongue |
//@                 triple-tongue | stopped | snap-pizzicato | fret |
//@                 string | hammer-on | pull-off | bend | tap | heel |
//@                 toe | fingernails | hole | arrow | handbell |
//@                 other-technical ]
//@

class TecnicalMxlAnalyser : public MxlElementAnalyser
{
public:
    TecnicalMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoNoteRest* pNR = nullptr;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoNoteRest");
            return nullptr;
        }

        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            if (m_childToAnalyse.name() == "up-bow")
            {
                get_technical_symbol(pNR, k_technical_up_bow);
            }
            else if (m_childToAnalyse.name() == "down-bow")
            {
                get_technical_symbol(pNR, k_technical_down_bow);
            }
            else if (m_childToAnalyse.name() == "double-tongue")
            {
                get_technical_symbol(pNR, k_technical_double_tongue);
            }
            else if (m_childToAnalyse.name() == "triple-tongue")
            {
                get_technical_symbol(pNR, k_technical_triple_tongue);
            }

        //not properly supported:
            else if (m_childToAnalyse.name() == "harmonic")
            {
                get_technical_symbol(pNR, k_technical_harmonic);
            }
//            else if (m_childToAnalyse.name() == "fingering")
//            {
//                get_technical_symbol(pNR, k_technical_fingering);
//            }
            else if (m_childToAnalyse.name() == "hole")
            {
                get_technical_symbol(pNR, k_technical_hole);
            }
            else if (m_childToAnalyse.name() == "handbell")
            {
                get_technical_symbol(pNR, k_technical_handbell);
            }
            else        //other-technical
            {
                error_invalid_child();
            }
            move_to_next_child();
        }

        error_if_more_elements();

        return nullptr;
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoTechnical* get_technical_symbol(ImoNoteRest* pNR, int type)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTechnical* pImo = static_cast<ImoTechnical*>(
                                ImFactory::inject(k_imo_technical, pDoc) );
        pImo->set_technical_type(type);

        // [attrib]: placement (above | below)
        if (has_attribute(&m_childToAnalyse, "placement"))
            set_placement(pImo);

        pNR->add_attachment(pDoc, pImo);
        return pImo;
    }

    //-----------------------------------------------------------------------------------
    void set_placement(ImoTechnical* pImo)
    {
        string value = get_attribute(&m_childToAnalyse, "placement");
        if (value == "above")
            pImo->set_placement(k_placement_above);
        else if (value == "below")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown placement attrib. '" + value + "'. Ignored.");
        }
    }

};

//@--------------------------------------------------------------------------------------
//@
//    Slur elements are empty. Most slurs are represented with
//    two elements: one with a start type, and one with a stop
//    type. Slurs can add more elements using a continue type.
//    This is typically used to specify the formatting of cross-
//    system slurs, or to specify the shape of very complex slurs.
//
//<!ELEMENT slur EMPTY>
//<!ATTLIST slur
//    type %start-stop-continue; #REQUIRED
//    number %number-level; "1"
//    %line-type;
//    %dashed-formatting;
//    %position;
//    %placement;
//    %orientation;
//    %bezier;
//    %color;
//>

class SlurMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoSlurDto* m_pInfo1;
    ImoSlurDto* m_pInfo2;

public:
    SlurMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pInfo1(nullptr)
        , m_pInfo2(nullptr)
    {
    }

    ImoObj* do_analysis()
    {
        ImoNote* pNote = nullptr;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("nullptr pAnchor or it is not ImoNote");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pInfo1 = static_cast<ImoSlurDto*>(
                                ImFactory::inject(k_imo_slur_dto, pDoc));
        m_pInfo1->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        // attrib: type %start-stop-continue; #REQUIRED
        const string& type = get_mandatory_string_attribute("type", "", "slur");

        // attrib: number %number-level; #IMPLIED
        int num = get_optional_int_attribute("number", 0);

//        // attrib: %line-type;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_child_value_integer(0) );

//        // attrib: %dashed-formatting;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_child_value_integer(0) );

//        // attrib: %position;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_child_value_integer(0) );

//        // attrib: %placement;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_child_value_integer(0) );

        // attrib: %orientation;
        if (has_attribute("orientation"))
        {
            string orientation = get_attribute("orientation");

            //AWARE: must be type == "start"
            if (orientation == "over")
                m_pInfo1->set_orientation(k_orientation_over);
            else
                m_pInfo1->set_orientation(k_orientation_under);
        }

//        // attrib: %bezier;
//        analyse_optional(k_bezier, pInfo);
//
//        // attrib: %color;
//        if (get_optional(k_color))
//            pInfo->set_color( get_color_child() );

        set_slur_type_and_id(type, num);

        if (m_pInfo1)
        {
            m_pInfo1->set_note(pNote);
            m_pAnalyser->add_relation_info(m_pInfo1);

            if (m_pInfo2)
            {
                m_pInfo2->set_note(pNote);
                m_pAnalyser->add_relation_info(m_pInfo2);
            }
        }

        return nullptr;     //m_pInfo1 has been deleted in add_relation_info()
    }

protected:

    void set_slur_type_and_id(const string& value, int num)
    {
        if (value == "start")
        {
            m_pInfo1->set_start(true);
            int slurId =  m_pAnalyser->new_slur_id(num);
            m_pInfo1->set_slur_number(slurId);
        }
        else if (value == "stop")
        {
            m_pInfo1->set_start(false);
            int slurId =  m_pAnalyser->get_slur_id_and_close(num);
            m_pInfo1->set_slur_number(slurId);
        }
        else if (value == "continue")
        {
            m_pInfo1->set_start(false);
            int slurId =  m_pAnalyser->get_slur_id_and_close(num);
            m_pInfo1->set_slur_number(slurId);

            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            m_pInfo2 = static_cast<ImoSlurDto*>(
                                ImFactory::inject(k_imo_slur_dto, pDoc));
            m_pInfo2->set_start(true);
            m_pInfo2->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
            slurId =  m_pAnalyser->new_slur_id(num);
            m_pInfo2->set_slur_number(slurId);
        }
        else
        {
            error_msg("Missing or invalid slur type. Slur ignored.");
            delete m_pInfo1;
            m_pInfo1 = nullptr;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <sound>
//@ A sound element represents a change in playback parameters.
//@ It can stand alone within a part/measure, or be a
//@ component element within a direction.
//@
//@<!ELEMENT sound ((midi-device?, midi-instrument?, play?)*, offset?)>
//@<!ATTLIST sound
//@    tempo CDATA #IMPLIED
//@    dynamics CDATA #IMPLIED
//@    dacapo %yes-no; #IMPLIED
//@    segno CDATA #IMPLIED
//@    dalsegno CDATA #IMPLIED
//@    coda CDATA #IMPLIED
//@    tocoda CDATA #IMPLIED
//@    divisions CDATA #IMPLIED
//@    forward-repeat %yes-no; #IMPLIED
//@    fine CDATA #IMPLIED
//@    %time-only;
//@    pizzicato %yes-no; #IMPLIED
//@    pan CDATA #IMPLIED                <-- deprecated MusicXML 2.0
//@    elevation CDATA #IMPLIED          <-- deprecated MusicXML 2.0
//@    damper-pedal %yes-no-number; #IMPLIED
//@    soft-pedal %yes-no-number; #IMPLIED
//@    sostenuto-pedal %yes-no-number; #IMPLIED
//@>
//
class SoundMxlAnalyser : public MxlElementAnalyser
{
public:
    SoundMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSoundChange* pSC = static_cast<ImoSoundChange*>(
                                    ImFactory::inject(k_imo_sound_change, pDoc));

        // attrib: tempo CDATA #IMPLIED - non-negative decimal
        //         quarter notes per minute. 0 = no change
        if (has_attribute("tempo"))
        {
            float value = get_optional_float_attribute("tempo", 70.0f);
            pSC->set_float_attribute(k_attr_tempo, value);
        }

        // attrib: dynamics CDATA #IMPLIED - non-negative decimal
        if (has_attribute("dynamics"))
        {
            float value = get_optional_float_attribute("dynamics", 70.0f);
            pSC->set_float_attribute(k_attr_dynamics, value);
        }

        // attrib: dacapo %yes-no; #IMPLIED
        if (has_attribute("dacapo"))
        {
            bool value = get_optional_yes_no_attribute("dacapo", false);
            pSC->set_bool_attribute(k_attr_dacapo, value);
        }

        // attrib: segno CDATA #IMPLIED - label to reference it
        if (has_attribute("segno"))
        {
            string value = get_optional_string_attribute("segno", "");
            pSC->set_string_attribute(k_attr_segno, value);
        }

        // attrib: dalsegno CDATA #IMPLIED - label to reference it
        if (has_attribute("dalsegno"))
        {
            string value = get_optional_string_attribute("dalsegno", "");
            pSC->set_string_attribute(k_attr_dalsegno, value);
        }

        // attrib: coda CDATA #IMPLIED - label to reference it
        if (has_attribute("coda"))
        {
            string value = get_optional_string_attribute("coda", "");
            pSC->set_string_attribute(k_attr_coda, value);
        }

        // attrib: tocoda CDATA #IMPLIED - label to reference it
        if (has_attribute("tocoda"))
        {
            string value = get_optional_string_attribute("tocoda", "");
            pSC->set_string_attribute(k_attr_tocoda, value);
        }

        // attrib: divisions CDATA #IMPLIED
            //TODO

        // attrib: forward-repeat %yes-no; #IMPLIED. When used, value must be "yes"
        if (has_attribute("forward-repeat"))
        {
            if (get_attribute("forward-repeat") != "yes")
            {
                error_msg2("Invalid value for 'forward-repeat' attribute. "
                           "When used, value must be 'yes'. Ignored.");
            }
            else
                pSC->set_bool_attribute(k_attr_forward_repeat, true);
        }

        // attrib: fine CDATA #IMPLIED - number or "yes"
        if (has_attribute("fine"))
        {
            //TODO: treatment of value (number or "yes")
            pSC->set_bool_attribute(k_attr_fine, true);
        }

        // attrib: %time-only; = time-only CDATA #IMPLIED
        if (has_attribute("time-only"))
        {
            string value = validate_time_only( get_attribute("time-only") );
            pSC->set_string_attribute(k_attr_time_only, value);
        }

        // attrib: pizzicato %yes-no; #IMPLIED
        if (has_attribute("pizzicato"))
        {
            bool value = get_optional_yes_no_attribute("pizzicato", false);
            pSC->set_bool_attribute(k_attr_pizzicato, value);
        }

        //TODO: yes-no-number
//        // attrib: damper-pedal %yes-no-number; #IMPLIED
//        if (has_attribute("damper-pedal"))
//        {
//            bool value = get_optional_yes_no_attribute("damper-pedal", false);
//            pSC->set_bool_attribute(k_attr_damper_pedal, value);
//        }
//
//        // attrib: soft-pedal %yes-no-number; #IMPLIED
//        if (has_attribute("soft-pedal"))
//        {
//            bool value = get_optional_yes_no_attribute("soft-pedal", false);
//            pSC->set_bool_attribute(k_attr_soft_pedal, value);
//        }
//
//        // attrib: sostenuto-pedal %yes-no-number; #IMPLIED
//        if (has_attribute("sostenuto-pedal"))
//        {
//            bool value = get_optional_yes_no_attribute("sostenuto-pedal", false);
//            pSC->set_bool_attribute(k_attr_sostenuto_pedal, value);
//        }

        bool fHasContent = (pSC->get_num_attributes() > 0);


            // content

        if (more_children_to_analyse())
        {
            // (midi-device?, midi-instrument?, play?)*, offset?
            while (more_children_to_analyse())
            {
                if (analyse_optional("midi-device", pSC)
                    || analyse_optional("midi-instrument", pSC)
                    || analyse_optional("play", pSC))
                {
                }
                else if (analyse_optional("offset", pSC))
                {
                    break;
                }
                else
                {
                    error_invalid_child();
                    move_to_next_child();
                }
            }

            fHasContent |= (pSC->get_num_children() > 0);
        }


        if (fHasContent)
        {
            add_to_model(pSC);
            return pSC;
        }
        else
        {
            error_msg("Empty <sound> element. Ignored.");
            return nullptr;
        }

    }

protected:

    string validate_time_only(const string& UNUSED(value))
    {
        //The value must be a comma-separated list of positive integers arranged
        //in ascending order.
        //If error, string "1" is returned
        //Otherwise any spaces are removed, i.e. "1, 2, 4" --> "1,2,4"
            //TODO
        return string("1");
    }

};

//@--------------------------------------------------------------------------------------
//<!ELEMENT text (#PCDATA)>
//<!ATTLIST text
//    %font;
//    %color;
//    %text-decoration;
//    %text-rotation;
//    %letter-spacing;
//    xml:lang NMTOKEN #IMPLIED
//    %text-direction;
//>

class TextMxlAnalyser : public MxlElementAnalyser
{
public:
    TextMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        ImoLyricsTextInfo* pParent = nullptr;
        if (m_pAnchor && m_pAnchor->is_lyrics_text_info())
            pParent = static_cast<ImoLyricsTextInfo*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("nullptr pAnchor or it is not ImoLyricsTextInfo");
            return nullptr;
        }

        //ATTLIST
        //    %font;
        //    %color;
        //    %text-decoration;
        //    %text-rotation;
        //    %letter-spacing;
        //    xml:lang NMTOKEN #IMPLIED
        //    %text-direction;

        // <string>
        string value = m_analysedNode.value();
        if (value.empty())
        {
            error_msg("text: missing mandatory string in element <text>.");
            return nullptr;
        }

        pParent->set_syllable_text(value);

        return pParent;
    }
};

//@--------------------------------------------------------------------------------------
//@    The tied element represents the notated tie. The tie element
//@    represents the tie sound.
//
//    The number attribute is rarely needed to disambiguate ties,
//    since note pitches will usually suffice. The attribute is
//    implied rather than defaulting to 1 as with most elements.
//    It is available for use in more complex tied notation
//    situations.
//@
//@ <!ELEMENT tied EMPTY>
//@ <!ATTLIST tied
//@     type %start-stop-continue; #REQUIRED
//@     number %number-level; #IMPLIED
//@     %line-type;
//@     %dashed-formatting;
//@     %position;
//@     %placement;
//@     %orientation;
//@     %bezier;
//@     %color;
//@ >
class TiedMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoTieDto* m_pInfo1;
    ImoTieDto* m_pInfo2;

public:
    TiedMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pInfo1(nullptr)
        , m_pInfo2(nullptr)
    {
    }

    ImoObj* do_analysis()
    {
        ImoNote* pNote = nullptr;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("nullptr pAnchor or it is not ImoNote");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pInfo1 = static_cast<ImoTieDto*>(
                                ImFactory::inject(k_imo_tie_dto, pDoc));
        m_pInfo1->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        // attrib: type %start-stop-continue; #REQUIRED
        const string& type = get_mandatory_string_attribute("type", "", "tied");

        // attrib: number %number-level; #IMPLIED
        int num = get_optional_int_attribute("number", 0);

        //TODO
//        // attrib: %line-type;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_child_value_integer(0) );

//        // attrib: %dashed-formatting;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_child_value_integer(0) );

//        // attrib: %position;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_child_value_integer(0) );

//        // attrib: %placement;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_child_value_integer(0) );

        // attrib: %orientation;
        if (has_attribute("orientation"))
        {
            string orientation = get_attribute("orientation");

            //AWARE: must be type == "start"
            if (orientation == "over")
                m_pInfo1->set_orientation(k_orientation_over);
            else
                m_pInfo1->set_orientation(k_orientation_under);
        }

        //TODO
//        // attrib: %position;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_child_value_integer(0) );

//        // attrib: %bezier;
//        analyse_optional(k_bezier, pInfo);
//
//        // attrib: %color;
//        if (get_optional(k_color))
//            pInfo->set_color( get_color_child() );

        set_tie_type_and_id(type, num, pNote);

        m_pInfo1->set_note(pNote);
        m_pAnalyser->add_relation_info(m_pInfo1);

        if (m_pInfo2)
        {
            m_pInfo2->set_note(pNote);
            m_pAnalyser->add_relation_info(m_pInfo2);
        }

        return nullptr;     //m_pInfo1 has been deleted in add_relation_info()
    }

protected:

    void set_tie_type_and_id(const string& value, int num, ImoNote* pNote)
    {
        if (value == "start")
        {
            m_pInfo1->set_start(true);
            int tieId =  m_pAnalyser->new_tie_id(num, pNote->get_fpitch());
            m_pInfo1->set_tie_number(tieId);
        }
        else if (value == "stop")
        {
            m_pInfo1->set_start(false);
            int tieId =  m_pAnalyser->get_tie_id_and_close(num, pNote->get_fpitch());
            m_pInfo1->set_tie_number(tieId);
        }
        else if (value == "continue")
        {
            m_pInfo1->set_start(false);
            int tieId =  m_pAnalyser->get_tie_id_and_close(num, pNote->get_fpitch());
            m_pInfo1->set_tie_number(tieId);

            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            m_pInfo2 = static_cast<ImoTieDto*>(
                                ImFactory::inject(k_imo_tie_dto, pDoc));
            m_pInfo2->set_start(true);
            tieId =  m_pAnalyser->new_tie_id(num, pNote->get_fpitch());
            m_pInfo2->set_tie_number(tieId);
            m_pInfo2->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
        }
        else
        {
            error_msg("Missing or invalid tie type. Tie ignored.");
            delete m_pInfo1;
            m_pInfo1 = nullptr;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT time ((beats, beat-type)+ | senza-misura)>
//@ <!ATTLIST time
//@         symbol (common | cut | single-number | normal) #IMPLIED
//@ >
//@ <!ELEMENT beats (#PCDATA)>
//@ <!ELEMENT beat-type (#PCDATA)>
//@ <!ELEMENT senza-misura EMPTY>

class TimeMxlAnalyser : public MxlElementAnalyser
{
public:
    TimeMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTimeSignature* pTime = static_cast<ImoTimeSignature*>(
                                    ImFactory::inject(k_imo_time_signature, pDoc) );

        // attrib: symbol (common | cut | single-number | normal)
        if (has_attribute("symbol"))
            set_symbol(pTime);

        // <beats> (num)
        if (get_mandatory("beats"))
            pTime->set_top_number( get_child_value_integer(2) );

        // <beat-type> (num)
        if (pTime->get_type() != ImoTimeSignature::k_single_number
             && get_mandatory("beat-type"))
            pTime->set_bottom_number( get_child_value_integer(4) );

        add_to_model(pTime);
        return pTime;
    }

protected:

    //-----------------------------------------------------------------------------------
    void set_symbol(ImoTimeSignature* pImo)
    {
        // attrib: symbol (common | cut | single-number | normal)

        string value = get_attribute("symbol");
        if (value == "common")
            pImo->set_type(ImoTimeSignature::k_common);
        else if (value == "cut")
            pImo->set_type(ImoTimeSignature::k_cut);
        else if (value == "single-number")
            pImo->set_type(ImoTimeSignature::k_single_number);
        else if (value == "normal")
            pImo->set_type(ImoTimeSignature::k_normal);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Unknown time signature type '" + value + "'. Ignored.");
        }
    }
};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT time-modification
//@    (actual-notes, normal-notes, (normal-type, normal-dot*)?)>
//@
class TimeModificationXmlAnalyser : public MxlElementAnalyser
{
protected:
    ImoNoteRest* m_pNR;
    int m_actual;
    int m_normal;

public:
    TimeModificationXmlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                                LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pNR(nullptr)
        , m_actual(0)
        , m_normal(0)
        {
        }

    ImoObj* do_analysis()
    {
        if (m_pAnchor && m_pAnchor->is_note_rest())
            m_pNR = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("nullptr pAnchor or it is not note/rest");
            return nullptr;
        }

        bool fError = false;

        // actual-notes
        if (get_mandatory("actual-notes"))
            fError |= set_actual_notes();
        else
            fError = true;

        // normal-notes
        if (get_mandatory("normal-notes"))
            fError |= set_normal_notes();
        else
            fError = true;

        //TODO: They are useless in IM. Confirm this.
        // (normal-type, normal-dot*)?
        // coverity[check_return]
        get_optional("normal-type");
        while (get_optional("normal-dots"));

        fError |= error_if_more_elements();

        if (!fError)
            m_pNR->set_time_modification(m_normal, m_actual);

        return nullptr;
    }

protected:

    bool set_actual_notes()
    {
        //returns true is error

        string actual = m_childToAnalyse.value();
        if (m_pAnalyser->to_integer(actual, &m_actual))
        {
            error_msg2(
                "Invalid actual-notes number '" + actual +
                "'. time-modification ignored.");
            return true;
        }
        return false;
    }

    bool set_normal_notes()
    {
        //returns true is error

        string normal = m_childToAnalyse.value();
        if (m_pAnalyser->to_integer(normal, &m_normal))
        {
            error_msg2(
                "Invalid normal-notes number '" + normal +
                "'. time-modification ignored.");
            return true;
        }
        return false;
    }
};


//---------------------------------------------------------------------------------------
//@ <!ELEMENT tuplet (tuplet-actual?, tuplet-normal?)>
//@ <!ATTLIST tuplet
//@     type %start-stop; #REQUIRED
//@     number %number-level; #IMPLIED
//@     bracket %yes-no; #IMPLIED
//@     show-number (actual | both | none) #IMPLIED
//@     show-type (actual | both | none) #IMPLIED
//@     %line-shape;
//@     %position;
//@     %placement;
//@ >
//@
class TupletMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoTupletDto* m_pInfo;

public:
    TupletMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                      LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pInfo(nullptr)
        {
        }

    ImoObj* do_analysis()
    {
        ImoNoteRest* pNR = nullptr;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("nullptr pAnchor or it is not note/rest");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pInfo = static_cast<ImoTupletDto*>(
                                ImFactory::inject(k_imo_tuplet_dto, pDoc));
        set_default_values(m_pInfo);
        m_pInfo->set_note_rest(pNR);
        m_pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        // atrib: type %start-stop; #REQUIRED
        const string& type = get_mandatory_string_attribute("type", "", "tuplet");
        if (type.empty() || !set_tuplet_type(type))
        {
            error_msg("Missing or invalid tuplet type. Tuplet ignored.");
            delete m_pInfo;
            return nullptr;
        }

        // attrib: number %number-level; #IMPLIED
        string snum = get_optional_string_attribute("number", "");
        set_tuplet_id(snum);

        // attrib: bracket %yes-no; #IMPLIED
        string value = get_optional_string_attribute("bracket", "");
        set_bracket(value);

        // attrib: show-number (actual | both | none) #IMPLIED
        value = get_optional_string_attribute("show-number", "");
        set_show_number(value);

        // attrib: show-type (actual | both | none) #IMPLIED
        value = get_optional_string_attribute("show-type", "");
        set_show_type(value);

        //TODO
        //%line-shape;
        //%position;
        //%placement;

        //compute default values for actual/normal numbers
        if (m_pInfo->is_start_of_tuplet())
        {
            int top, bottom;
            m_pAnalyser->get_factors_from_nested_tuplets(&top, &bottom);
            m_pInfo->set_actual_number( pNR->get_time_modifier_bottom() / bottom );
            m_pInfo->set_normal_number( pNR->get_time_modifier_top() / top );
        }

        //(tuplet-actual?, tuplet-normal?)
        analyse_optional("tuplet-actual", m_pInfo);
        analyse_optional("tuplet-normal", m_pInfo);

        //add to model
        m_pAnalyser->add_relation_info(m_pInfo);

        return nullptr;     //m_pInfo has been deleted in add_relation_info()
    }

protected:

    void set_default_values(ImoTupletDto* pInfo)
    {
        pInfo->set_show_bracket(k_yesno_default);
        pInfo->set_placement(k_placement_default);
        pInfo->set_only_graphical(true);
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
    }

    void set_tuplet_id(const string& snum)
    {
        if (snum.empty())
        {
            error_msg("Invalid tuplet number. Number ignored.");
            return;
        }

        ////c++11
        //long num = std::stol(snum);
        //c++98
        char* pEnd;
        long num = std::strtol(snum.c_str(), &pEnd, 10);
        m_pInfo->set_id(num);
        m_pInfo->set_tuplet_number(num);
    }

    bool set_tuplet_type(const string& value)
    {
        if (value == "start")
            m_pInfo->set_tuplet_type(ImoTupletDto::k_start);
        else if (value == "stop")
            m_pInfo->set_tuplet_type(ImoTupletDto::k_stop);
        else
            return false;   //error
        return true;    //ok
    }

    void set_bracket(const string& value)
    {
        if (value.empty())
            m_pInfo->set_show_bracket(k_yesno_default);
        else if (value == "yes")
            m_pInfo->set_show_bracket(k_yesno_yes);
        else if (value == "no")
            m_pInfo->set_show_bracket(k_yesno_no);
        else
        {
            error_msg("Invalid value '" + value +
                      "' for yes-no bracket attribute. 'no' assumed.");
            m_pInfo->set_show_bracket(k_yesno_no);
        }
    }

    void set_show_number(const string& value)
    {
        //The show-number attribute is used to display either the
        //number of actual notes, the number of both actual and
        //normal notes, or neither. It is actual by default.

        if (value.empty())
            m_pInfo->set_show_number(ImoTuplet::k_number_actual);
        else if (value == "none")
            m_pInfo->set_show_number(ImoTuplet::k_number_none);
        else if (value == "actual")
            m_pInfo->set_show_number(ImoTuplet::k_number_actual);
        else if (value == "both")
            m_pInfo->set_show_number(ImoTuplet::k_number_both);
        else
        {
            error_msg("Invalid value '" + value +
                      "' for show-number attribute. 'actual' assumed.");
            m_pInfo->set_show_number(ImoTuplet::k_number_actual);
        }
        //m_pAnalyser->set_current_show_tuplet_number(nShowNumber);
    }

    void set_show_type(const string& UNUSED(value))
    {
        //The show-type attribute is used to display either the actual note
        //type, both the actual and normal types, or neither. It is
        //none by default.

        //TODO This is for drawing small notes with dots near the numbers
        //For now not implemented. I never saw this in a real score, so I prefer
        //to devote my time to more common notation.

    }

};



//---------------------------------------------------------------------------------------
//<!ELEMENT tuplet-actual (tuplet-number?,
//    tuplet-type?, tuplet-dot*)>
//<!ELEMENT tuplet-normal (tuplet-number?,
//    tuplet-type?, tuplet-dot*)>
//<!ELEMENT tuplet-number (#PCDATA)>
//<!ATTLIST tuplet-number
//    %font;
//    %color;
//>
//<!ELEMENT tuplet-type (#PCDATA)>
//<!ATTLIST tuplet-type
//    %font;
//    %color;
//>
//<!ELEMENT tuplet-dot EMPTY>
//<!ATTLIST tuplet-dot
//    %font;
//    %color;
//>
class TupletNumbersMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoTupletDto* m_pInfo;

public:
    TupletNumbersMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                             LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pInfo(nullptr)
    {
    }

    ImoObj* do_analysis()
    {
        if (m_pAnchor && m_pAnchor->is_tuplet_dto())
            m_pInfo = static_cast<ImoTupletDto*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("nullptr pAnchor or it is not tuplet dto");
            return nullptr;
        }

        bool fIsActual = m_analysedNode.name() == "tuplet-actual";

        // tuplet-number?
        if (get_optional("tuplet-number"))
        {
            string value = m_childToAnalyse.value();
            int num;
            if (m_pAnalyser->to_integer(value, &num))
                error_msg2("Invalid value for 'tuplet-number' element. Ignored.");
            else
            {
                if (fIsActual)
                    m_pInfo->set_actual_number(num);
                else
                    m_pInfo->set_normal_number(num);
            }
        }

        //TODO: tuplet-type?, tuplet-dot*

        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <virtual-instrument>
//<!ELEMENT virtual-instrument
//    (virtual-library?, virtual-name?)>
//<!ELEMENT virtual-library (#PCDATA)>
//<!ELEMENT virtual-name (#PCDATA)>
//
class VirtualInstrumentMxlAnalyser : public MxlElementAnalyser
{
public:
    VirtualInstrumentMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                                 LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoSoundInfo* pInfo = dynamic_cast<ImoSoundInfo*>(m_pAnchor);
        //ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>(m_pAnchor);
        if (!pInfo)
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoInstrument");
            return nullptr;
        }

        // virtual-library?
        pInfo->set_score_instr_virtual_library(
                    analyze_optional_child_pcdata("virtual-library", "") );

        // virtual-name?
        pInfo->set_score_instr_virtual_name(
                    analyze_optional_child_pcdata("virtual-name", "") );

        error_if_more_elements();

        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <wedge>
//@<!ELEMENT wedge EMPTY>
//@<!ATTLIST wedge
//@    type (crescendo | diminuendo | stop | continue) #REQUIRED
//@    number %number-level; #IMPLIED
//@    spread %tenths; #IMPLIED
//@    niente %yes-no; #IMPLIED
//@    %line-type;
//@    %dashed-formatting;
//@    %position;
//@    %color;
//@    %optional-unique-id;
//@>
//
class WedgeMxlAnalyser : public MxlElementAnalyser
{
protected:
    ImoWedgeDto* m_pInfo1;
    ImoWedgeDto* m_pInfo2;

public:
    WedgeMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pInfo1(nullptr)
        , m_pInfo2(nullptr)
    {
    }

    ImoObj* do_analysis()
    {
        ImoDirection* pDirection = nullptr;
        if (m_pAnchor && m_pAnchor->is_direction())
            pDirection = static_cast<ImoDirection*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoDirection");
            error_msg("<direction-type> <wedge> is not child of <direction>. Ignored.");
            return nullptr;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pInfo1 = static_cast<ImoWedgeDto*>(
                                ImFactory::inject(k_imo_wedge_dto, pDoc));
        m_pInfo1->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        // attrib: type (crescendo | diminuendo | stop | continue) #REQUIRED
        const string& type = get_mandatory_string_attribute("type", "", "wedge");

        // attrib: number %number-level; #IMPLIED
        int num = get_optional_int_attribute("number", 1);

        // attrib: spread %tenths; #IMPLIED
        if (has_attribute("spread"))
        {
            Tenths spread = get_attribute_as_float("spread", 0.0f);
            m_pInfo1->set_spread(spread);
        }

        // attrib: niente %yes-no; #IMPLIED
        m_pInfo1->set_niente( get_optional_yes_no_attribute("niente", false) );

        // attrib: %line-type;
        // attrib: %dashed-formatting;
        // attrib: %position;
        // attrib: %color;
        // attrib: %optional-unique-id;
        //TODO

        set_wedge_type_and_id(type, num);

        if (m_pInfo1)
        {
            m_pInfo1->set_staffobj(pDirection);
            m_pAnalyser->add_relation_info(m_pInfo1);

            if (m_pInfo2)
            {
                m_pInfo2->set_staffobj(pDirection);
                m_pAnalyser->add_relation_info(m_pInfo2);
            }
        }

        return nullptr;     //m_pInfo1 has been deleted in add_relation_info()
    }

protected:

    void set_wedge_type_and_id(const string& value, int num)
    {
        //AWARE: The values of start, stop, and continue refer to how an element appears
        //in musical score order, not in MusicXML document order. An element with a stop
        //attribute may precede the corresponding element with a start attribute within
        //a MusicXML document. Therefore, the following combinations are valid:
        //  (stop, crescendo)           (crescendo, stop)
        //  (continue, crescendo)       (continue, continue)
        //  (continue, stop)            (stop, continue)
        //  (diminuendo, stop)          (diminuendo, continue)

        if (m_pAnalyser->wedge_id_exists(num) && value != "continue")
        {
            m_pInfo1->set_start(false);
            int wedgeId =  m_pAnalyser->get_wedge_id_and_close(num);
            m_pInfo1->set_wedge_number(wedgeId);
            if (value == "crescendo")
                m_pInfo1->set_crescendo(true);
        }
        else if (value == "crescendo" || value == "diminuendo" || value == "stop")
        {
            m_pInfo1->set_start(true);
            int wedgeId =  m_pAnalyser->new_wedge_id(num);
            m_pInfo1->set_wedge_number(wedgeId);
            m_pInfo1->set_crescendo(value == "crescendo");
        }
        else if (value == "continue")
        {
            m_pInfo1->set_start(false);
            int wedgeId =  m_pAnalyser->get_wedge_id_and_close(num);
            m_pInfo1->set_wedge_number(wedgeId);

            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            m_pInfo2 = static_cast<ImoWedgeDto*>(
                                ImFactory::inject(k_imo_wedge_dto, pDoc));
            m_pInfo2->set_start(true);
            m_pInfo2->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
            wedgeId =  m_pAnalyser->new_wedge_id(num);
            m_pInfo2->set_wedge_number(wedgeId);
        }
        else
        {
            error_msg("Missing or invalid wedge type '" + value
                      + "'. Wedge ignored.");
            delete m_pInfo1;
            m_pInfo1 = nullptr;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <words>
//@ The words element specifies a standard text direction.
//@ Left justification is assumed if not specified.
//@ Language is Italian ("it") by default.
//@ Enclosure is none by default.
//@
//@<!ELEMENT words (#PCDATA)>
//@<!ATTLIST words
//@    %text-formatting;
//@>
//
class WordsMxlAnalyser : public MxlElementAnalyser
{
public:
    WordsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoDirection* pDirection = nullptr;
        if (m_pAnchor && m_pAnchor->is_direction())
            pDirection = static_cast<ImoDirection*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is nullptr or it is not ImoDirection");
            error_msg("<direction-type> <words> is not child of <direction>. Ignored.");
            return nullptr;
        }

        string text = m_analysedNode.value();
        int repeat = is_repetion_mark(text);
        pDirection->set_display_repeat(repeat);

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScoreText* pImo;
        if (repeat != k_repeat_none)
        {
            ImoTextRepetitionMark* pRM = static_cast<ImoTextRepetitionMark*>(
                    ImFactory::inject(k_imo_text_repetition_mark, pDoc) );
            pRM->set_repeat_mark(repeat);
            pImo = pRM;
        }
        else
        {
            pImo = static_cast<ImoScoreText*>(
                        ImFactory::inject(k_imo_score_text, pDoc) );
        }

        //set default values
        pImo->set_language("it");
            //TODO:
            //Left justification is assumed if not specified.
            //Enclosure is none by default.

        // attrib: %text-formatting;
        get_attributes_for_text_formatting(pImo);

        // words (#PCDATA)
        pImo->set_text(text);

        pDirection->add_attachment(pDoc, pImo);
        return pImo;
    }

protected:

    int is_repetion_mark(const string& value)
    {
        return mxl_type_of_repetion_mark(value);
    }

};

//defined out of WordsMxlAnalyser to simplify unit testing of the regex
int mxl_type_of_repetion_mark(const string& value)
{
    //get text and use it for deducing if it is a repetition mark

    string text = value;
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);

    //by default, regex uses modified ECMAScript syntax
    //See:  http://www.cplusplus.com/reference/regex/ECMAScript/
    //See:  http://en.cppreference.com/w/cpp/regex/ecmascript
    std::regex regexDaCapo("^ *(d|d\\.) *(c|c\\.) *$|^ *da *capo *$");    //d\\.? *c\\.? Fails!
    std::regex regexDaCapoAlFine("^ *(d|d\\.) *(c|c\\.) *al *fine *$|^ *da *capo *al *fine *$");
    std::regex regexDaCapoAlCoda("^ *(d|d\\.) *(c|c\\.) *al *coda *$|^ *da *capo *al *coda *$");
    std::regex regexDalSegno("^ *(d|d\\.) *(s|s\\.) *$|^ *d(a|e)l *segno *$");
    std::regex regexDalSegnoAlFine("^ *(d|d\\.) *(s|s\\.) *al *fine *$|^ *d(a|e)l *segno *al *fine *$");
    std::regex regexDalSegnoAlCoda("^ *(d|d\\.) *(s|s\\.) *al *coda *$|^ *d(a|e)l *segno *al *coda *$");
    std::regex regexFine("^ *fine *$");
    std::regex regexToCoda("^ *to *coda *$");

    if (std::regex_match(text, regexDaCapo))
        return k_repeat_da_capo;
    else if (std::regex_match(text, regexDaCapoAlFine))
        return k_repeat_da_capo_al_fine;
    else if (std::regex_match(text, regexDaCapoAlCoda))
        return k_repeat_da_capo_al_coda;
    else if (std::regex_match(text, regexDalSegno))
        return k_repeat_dal_segno;
    else if (std::regex_match(text, regexDalSegnoAlFine))
        return k_repeat_dal_segno_al_fine;
    else if (std::regex_match(text, regexDalSegnoAlCoda))
        return k_repeat_dal_segno_al_coda;
    else if (std::regex_match(text, regexFine))
        return k_repeat_fine;
    else if (std::regex_match(text, regexToCoda))
        return k_repeat_to_coda;
    else
        return k_repeat_none;
}


//=======================================================================================
// MxlAnalyser implementation
//=======================================================================================
MxlAnalyser::MxlAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc,
                         XmlParser* parser)
    : Analyser()
    , m_reporter(reporter)
    , m_libraryScope(libraryScope)
    , m_pDoc(pDoc)
    , m_pParser(parser)
    , m_pLdpFactory(libraryScope.ldp_factory())
    , m_pTiesBuilder(nullptr)
    , m_pBeamsBuilder(nullptr)
    , m_pTupletsBuilder(nullptr)
    , m_pSlursBuilder(nullptr)
    , m_pVoltasBuilder(nullptr)
    , m_pWedgesBuilder(nullptr)
    , m_pOctaveShiftBuilder(nullptr)
    , m_musicxmlVersion(0)
    , m_pNodeImo(nullptr)
    , m_tieNum(0)
    , m_slurNum(0)
    , m_voltaNum(0)
    , m_wedgeNum(0)
    , m_octaveShiftNum(0)
    , m_pTree()
    , m_fileLocator("")
//    , m_nShowTupletBracket(k_yesno_default)
//    , m_nShowTupletNumber(k_yesno_default)
    , m_pCurScore(nullptr)
    , m_pCurInstrument(nullptr)
    , m_pLastNote(nullptr)
    , m_pLastBarline(nullptr)
    , m_pImoDoc(nullptr)
    , m_time(0.0)
    , m_maxTime(0.0)
    , m_divisions(1.0f)
    , m_curMeasureNum("")
    , m_measuresCounter(0)
    , m_curVoice(0)
{
    //populate the name to enum conversion map
    m_NameToEnum["accordion-registration"] = k_mxl_tag_accordion_registration;
    m_NameToEnum["articulations"] = k_mxl_tag_articulations;
    m_NameToEnum["attributes"] = k_mxl_tag_attributes;
    m_NameToEnum["backup"] = k_mxl_tag_backup;
    m_NameToEnum["barline"] = k_mxl_tag_barline;
    m_NameToEnum["bracket"] = k_mxl_tag_bracket;
    m_NameToEnum["clef"] = k_mxl_tag_clef;
    m_NameToEnum["coda"] = k_mxl_tag_coda;
    m_NameToEnum["damp"] = k_mxl_tag_damp;
    m_NameToEnum["damp-all"] = k_mxl_tag_damp_all;
    m_NameToEnum["dashes"] = k_mxl_tag_dashes;
    m_NameToEnum["direction"] = k_mxl_tag_direction;
    m_NameToEnum["direction-type"] = k_mxl_tag_direction_type;
    m_NameToEnum["dynamics"] = k_mxl_tag_dynamics;
    m_NameToEnum["ending"] = k_mxl_tag_ending;
    m_NameToEnum["eyeglasses"] = k_mxl_tag_eyeglasses;
    m_NameToEnum["fermata"] = k_mxl_tag_fermata;
    m_NameToEnum["forward"] = k_mxl_tag_forward;
    m_NameToEnum["harp-pedals"] = k_mxl_tag_harp_pedals;
    m_NameToEnum["image"] = k_mxl_tag_image;
    m_NameToEnum["key"] = k_mxl_tag_key;
    m_NameToEnum["lyric"] = k_mxl_tag_lyric;
    m_NameToEnum["measure"] = k_mxl_tag_measure;
    m_NameToEnum["metronome"] = k_mxl_tag_metronome;
    m_NameToEnum["midi-device"] = k_mxl_tag_midi_device;
    m_NameToEnum["midi-instrument"] = k_mxl_tag_midi_instrument;
    m_NameToEnum["notations"] = k_mxl_tag_notations;
    m_NameToEnum["note"] = k_mxl_tag_note;
    m_NameToEnum["octave-shift"] = k_mxl_tag_octave_shift;
    m_NameToEnum["ornaments"] = k_mxl_tag_ornaments;
    m_NameToEnum["part"] = k_mxl_tag_part;
    m_NameToEnum["part-group"] = k_mxl_tag_part_group;
    m_NameToEnum["part-list"] = k_mxl_tag_part_list;
    m_NameToEnum["part-name"] = k_mxl_tag_part_name;
    m_NameToEnum["pedal"] = k_mxl_tag_pedal;
    m_NameToEnum["percussion"] = k_mxl_tag_percussion;
    m_NameToEnum["pitch"] = k_mxl_tag_pitch;
    m_NameToEnum["principal-voice"] = k_mxl_tag_principal_voice;
    m_NameToEnum["print"] = k_mxl_tag_print;
    m_NameToEnum["rehearsal"] = k_mxl_tag_rehearsal;
    m_NameToEnum["rest"] = k_mxl_tag_rest;
    m_NameToEnum["scordatura"] = k_mxl_tag_scordatura;
    m_NameToEnum["score-instrument"] = k_mxl_tag_score_instrument;
    m_NameToEnum["score-part"] = k_mxl_tag_score_part;
    m_NameToEnum["score-partwise"] = k_mxl_tag_score_partwise;
    m_NameToEnum["segno"] = k_mxl_tag_segno;
    m_NameToEnum["slur"] = k_mxl_tag_slur;
    m_NameToEnum["sound"] = k_mxl_tag_sound;
    m_NameToEnum["string-mute"] = k_mxl_tag_string_mute;
    m_NameToEnum["technical"] = k_mxl_tag_technical;
    m_NameToEnum["text"] = k_mxl_tag_text;
    m_NameToEnum["tied"] = k_mxl_tag_tied;
    m_NameToEnum["time"] = k_mxl_tag_time;
    m_NameToEnum["time-modification"] = k_mxl_tag_time_modification;
    m_NameToEnum["tuplet"] = k_mxl_tag_tuplet;
    m_NameToEnum["tuplet-actual"] = k_mxl_tag_tuplet_actual;
    m_NameToEnum["tuplet-normal"] = k_mxl_tag_tuplet_normal;
    m_NameToEnum["virtual-instrument"] = k_mxl_tag_virtual_instr;
    m_NameToEnum["wedge"] = k_mxl_tag_wedge;
    m_NameToEnum["words"] = k_mxl_tag_words;

    m_notes.assign(50, nullptr);
}

//---------------------------------------------------------------------------------------
MxlAnalyser::~MxlAnalyser()
{
    delete_relation_builders();
    m_NameToEnum.clear();
    m_lyrics.clear();
    m_lyricIndex.clear();
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::delete_relation_builders()
{
    delete m_pTiesBuilder;
    delete m_pBeamsBuilder;
    delete m_pTupletsBuilder;
    delete m_pSlursBuilder;
    delete m_pVoltasBuilder;
    delete m_pWedgesBuilder;
    delete m_pOctaveShiftBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* MxlAnalyser::analyse_tree_and_get_object(XmlNode* root)
{
    delete_relation_builders();
    m_pTiesBuilder = LOMSE_NEW MxlTiesBuilder(m_reporter, this);
    m_pBeamsBuilder = LOMSE_NEW MxlBeamsBuilder(m_reporter, this);
    m_pTupletsBuilder = LOMSE_NEW MxlTupletsBuilder(m_reporter, this);
    m_pSlursBuilder = LOMSE_NEW MxlSlursBuilder(m_reporter, this);
    m_pVoltasBuilder = LOMSE_NEW MxlVoltasBuilder(m_reporter, this);
    m_pWedgesBuilder = LOMSE_NEW MxlWedgesBuilder(m_reporter, this);
    m_pOctaveShiftBuilder = LOMSE_NEW MxlOctaveShiftBuilder(m_reporter, this);

    m_pTree = root;
//    m_curStaff = 0;
    m_curVoice = 1;
    return analyse_node(root);
}

//---------------------------------------------------------------------------------------
ImoObj* MxlAnalyser::analyse_tree(XmlNode* tree, const string& locator)
{
    m_fileLocator = locator;
    return analyse_tree_and_get_object(tree);
}

//---------------------------------------------------------------------------------------
ImoObj* MxlAnalyser::analyse_node(XmlNode* pNode, ImoObj* pAnchor)
{
    //m_reporter << "DBG. Analysing node: " << pNode->name() << endl;
    MxlElementAnalyser* a = new_analyser( pNode->name(), pAnchor );
    ImoObj* pImo = a->analyse_node(pNode);
    delete a;
    return pImo;
}

//---------------------------------------------------------------------------------------
bool MxlAnalyser::analyse_node_bool(XmlNode* pNode, ImoObj* pAnchor)
{
    MxlElementAnalyser* a = new_analyser( pNode->name(), pAnchor );
    return a->analyse_node_bool(pNode);
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_line_number(XmlNode* node)
{
    return m_pParser->get_line_number(node);
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::prepare_for_new_instrument_content()
{
    clear_pending_relations();
    m_time = 0.0;
    m_maxTime = 0.0;
    save_last_barline(nullptr);
    m_measuresCounter = 0;
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::save_last_note(ImoNote* pNote)
{
    m_pLastNote = pNote;

    int iStaff = pNote->get_staff();
    if (m_notes.size() > size_t(iStaff))
        m_notes[iStaff] = pNote;
}

//---------------------------------------------------------------------------------------
ImoNote* MxlAnalyser::get_last_note_for(int iStaff)
{
    return m_notes[iStaff];
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::save_current_instrument(ImoInstrument* pInstr)
{
    m_pCurInstrument = pInstr;

    int numStaves = pInstr->get_num_staves();
    m_notes.assign( max(numStaves, 10), nullptr);
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_index_for_sound(const string& id)
{
	map<string, int>::const_iterator it = m_soundIdToIdx.find(id);
	if (it != m_soundIdToIdx.end())
		return it->second;
    else
        return -1;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::create_index_for_sound(const string& id)
{
    int idx = int(m_latestMidiInfo.size());
    m_soundIdToIdx[id] = idx;
    m_latestMidiInfo.push_back(nullptr);
    return idx;
}

//---------------------------------------------------------------------------------------
ImoMidiInfo* MxlAnalyser::get_latest_midi_info_for(const string& id)
{
    int idx = get_index_for_sound(id);
    if (idx < 0)
    {
        LOMSE_LOG_ERROR("Logic error: instrument %s not found!", id.c_str());
        return nullptr;
    }
    return m_latestMidiInfo[idx];
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::set_latest_midi_info_for(const string& id, ImoMidiInfo* pMidi)
{
    int idx = get_index_for_sound(id);
    if (idx == -1)
        idx = create_index_for_sound(id);
    m_latestMidiInfo[idx] = pMidi;
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::add_relation_info(ImoObj* pDto)
{
    // factory method to deal with all relations

    if (pDto->is_beam_dto())
        m_pBeamsBuilder->add_item_info(static_cast<ImoBeamDto*>(pDto));
    else if (pDto->is_tie_dto())
        m_pTiesBuilder->add_item_info(static_cast<ImoTieDto*>(pDto));
    else if (pDto->is_slur_dto())
        m_pSlursBuilder->add_item_info(static_cast<ImoSlurDto*>(pDto));
    else if (pDto->is_tuplet_dto())
        m_pTupletsBuilder->add_item_info(static_cast<ImoTupletDto*>(pDto));
    else if (pDto->is_volta_bracket_dto())
        m_pVoltasBuilder->add_item_info(static_cast<ImoVoltaBracketDto*>(pDto));
    else if (pDto->is_wedge_dto())
        m_pWedgesBuilder->add_item_info(static_cast<ImoWedgeDto*>(pDto));
    else if (pDto->is_octave_shift_dto())
        m_pOctaveShiftBuilder->add_item_info(static_cast<ImoOctaveShiftDto*>(pDto));
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::clear_pending_relations()
{
    m_pTiesBuilder->clear_pending_items();
    m_pSlursBuilder->clear_pending_items();
    m_pBeamsBuilder->clear_pending_items();
    m_pTupletsBuilder->clear_pending_items();
    m_pVoltasBuilder->clear_pending_items();
    m_pWedgesBuilder->clear_pending_items();
    m_pOctaveShiftBuilder->clear_pending_items();

    m_lyrics.clear();
    m_lyricIndex.clear();
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::add_lyrics_data(ImoNote* pNote, ImoLyric* pLyric)
{
    //build hash code from number & voice. Instrument is not needed as
    //the lyrics map is cleared when a new instrument is analysed.
    stringstream tag;
    int num = pLyric->get_number();
    tag << num << "-" << pNote->get_voice();
    string id = tag.str();


    //get index for this number-voice. If none, create index
    int i = 0;
    map<string, int>::iterator it = m_lyricIndex.find(id);
    if (it == m_lyricIndex.end())
    {
        m_lyrics.push_back(nullptr);
        i = int(m_lyrics.size()) - 1;
        m_lyricIndex[id] = i;

        //inform Instrument about the new lyrics line
        add_marging_space_for_lyrics(pNote, pLyric);
    }
    else
        i = it->second;

    //link new lyric with previous one
    ImoLyric* pPrev = m_lyrics[i];
    if (pPrev)
        pPrev->link_to_next_lyric(pLyric);

    //save current as new previous
    m_lyrics[i] = pLyric;
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::add_marging_space_for_lyrics(ImoNote* pNote, ImoLyric* pLyric)
{
    //inform Instrument about the new lyrics line for reserving space

    int iStaff = pNote->get_staff();
    bool fAbove = pLyric->get_placement() == k_placement_above;
    LUnits space = 400.0f;  //4mm per lyrics line
    ImoInstrument* pInstr = get_instrument(m_curPartId);

    if (fAbove)
    {
        pInstr->reserve_space_for_lyrics(iStaff, space);
        //TODO: Doesnt work for first staff in first instrument
    }
    else
    {
        //add space to top margin of next staff
        int staves = pInstr->get_num_staves();
        if (++iStaff == staves)
        {
            //add space to top margin of first staff in next instrument
            //AWARE: All instruments are already created
            int iInstr = m_pCurScore->get_instr_number_for(pInstr) + 1;
            if (iInstr < m_pCurScore->get_num_instruments())
            {
                pInstr = m_pCurScore->get_instrument(iInstr);
                pInstr->reserve_space_for_lyrics(0, space);
            }
            else
            {
                ;   //TODO: Space for last staff in last instrument
            }
        }
        else
        {
            //add space to top margin of next staff in this instrument
            pInstr->reserve_space_for_lyrics(iStaff, space);
        }
    }
}

//---------------------------------------------------------------------------------------
ImoInstrGroup* MxlAnalyser::start_part_group(int number)
{
    if (m_partGroups.group_exists(number))
        return nullptr;

    Document* pDoc = get_document_being_analysed();
    ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(
                                    ImFactory::inject(k_imo_instr_group, pDoc));

    m_partGroups.start_group(number, pGrp);
    return pGrp;
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::terminate_part_group(int number)
{
    ImoInstrGroup* pGrp = m_partGroups.get_group(number);
    if (pGrp)
        m_partGroups.terminate_group(number);
}

//---------------------------------------------------------------------------------------
ImoInstrGroup* MxlAnalyser::get_part_group(int number)
{
    return m_partGroups.get_group(number);
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::check_if_all_groups_are_closed()
{
    m_partGroups.check_if_all_groups_are_closed(m_reporter);
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::new_tie_id(int UNUSED(numTie), FPitch fp)
{
    m_tieIds[int(fp)] = ++m_tieNum;
    return m_tieNum;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_tie_id(int UNUSED(numTie), FPitch fp)
{
    return m_tieIds[int(fp)];
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_tie_id_and_close(int UNUSED(numTie), FPitch fp)
{
    return m_tieIds[int(fp)];
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::new_slur_id(int numSlur)
{
    m_slurIds[numSlur] = ++m_slurNum;
    return m_slurNum;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_slur_id(int numSlur)
{
    return m_slurIds[numSlur];
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_slur_id_and_close(int numSlur)
{
    return m_slurIds[numSlur];
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::new_volta_id()
{
    return ++m_voltaNum;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_volta_id()
{
    return m_voltaNum;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::new_wedge_id(int numWedge)
{
    m_wedgeIds[numWedge] = ++m_wedgeNum;
    return m_wedgeNum;
}

//---------------------------------------------------------------------------------------
bool MxlAnalyser::wedge_id_exists(int numWedge)
{
    return numWedge <= m_wedgeNum && m_wedgeIds[numWedge] != -1;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_wedge_id(int numWedge)
{
    return m_wedgeIds[numWedge];
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_wedge_id_and_close(int numWedge)
{
    int id = m_wedgeIds[numWedge];
    m_wedgeIds[numWedge] = -1;
    return id;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::new_octave_shift_id(int num)
{
    m_octaveShiftIds[num] = ++m_octaveShiftNum;
    return m_octaveShiftNum;
}

//---------------------------------------------------------------------------------------
bool MxlAnalyser::octave_shift_id_exists(int num)
{
    return num <= m_octaveShiftNum && m_octaveShiftIds[num] != -1;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_octave_shift_id(int num)
{
    return m_octaveShiftIds[num];
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_octave_shift_id_and_close(int num)
{
    int id = m_octaveShiftIds[num];
    m_octaveShiftIds[num] = -1;
    return id;
}

//---------------------------------------------------------------------------------------
TimeUnits MxlAnalyser::duration_to_timepos(int duration)
{
    //AWARE: 'divisions' indicates how many divisions per quarter note
    //       and 'duration' is expressed in 'divisions'
    float LdpTimeUnitsPerDivision = k_duration_quarter / m_divisions;
    return TimeUnits( float(duration) * LdpTimeUnitsPerDivision);
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::set_musicxml_version(const string& version)
{
    //version is a string "major.minor". Extract major and minor and compose
    //and integer 100*major+minor

    m_musicxmlVersion = 0;
    size_t i = version.find('.');
    if (i != string::npos)
    {
        string major = version.substr(0, i);
        if ( to_integer(major, &m_musicxmlVersion) )
        {
            m_musicxmlVersion = 100;
            return m_musicxmlVersion;
        }

        m_musicxmlVersion *= 100;
        string minor = version.substr(i+1);
        int nMinor;
        if ( to_integer(minor, &nMinor) )
        {
            m_musicxmlVersion = 100;
            return m_musicxmlVersion;
        }

        m_musicxmlVersion += nMinor;
    }
    return m_musicxmlVersion;
}

//---------------------------------------------------------------------------------------
string MxlAnalyser::get_element_info()
{
    stringstream ss;
    ss << "Part '" << m_curPartId << "', measure '" << m_curMeasureNum << "'. ";
    return ss.str();
}

//---------------------------------------------------------------------------------------
bool MxlAnalyser::to_integer(const string& text, int* pResult)
{
    //return true if error

    long number;
    std::istringstream iss(text);
    if ((iss >> std::dec >> number).fail())
    {
        *pResult = 0;
        return true;    //error
    }
    else
    {
        *pResult = number;
        return false;   //ok
    }
}

//---------------------------------------------------------------------------------------
MxlElementAnalyser* MxlAnalyser::new_analyser(const string& name, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch ( name_to_enum(name) )
    {
//        case k_mxl_tag_accordion_registration: return LOMSE_NEW AccordionRegistrationMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_articulations:        return LOMSE_NEW ArticulationsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_attributes:           return LOMSE_NEW AtribbutesMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_backup:               return LOMSE_NEW FwdBackMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_barline:              return LOMSE_NEW BarlineMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_bracket:              return LOMSE_NEW BracketMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_clef:                 return LOMSE_NEW ClefMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_coda:                 return LOMSE_NEW CodaMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_damp:                 return LOMSE_NEW DampMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_damp_all:             return LOMSE_NEW DampAllMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_dashes:               return LOMSE_NEW DashesMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_direction:            return LOMSE_NEW DirectionMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_direction_type:       return LOMSE_NEW DirectionTypeMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_dynamics:             return LOMSE_NEW DynamicsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_ending:               return LOMSE_NEW EndingMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_eyeglasses:           return LOMSE_NEW EyeglassesMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_fermata:              return LOMSE_NEW FermataMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_forward:              return LOMSE_NEW FwdBackMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_harp_pedals:          return LOMSE_NEW HarpPedalsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_image:                return LOMSE_NEW ImageMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_key:                  return LOMSE_NEW KeyMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_lyric:                return LOMSE_NEW LyricMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_measure:              return LOMSE_NEW MeasureMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_metronome:            return LOMSE_NEW MetronomeMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_midi_device:          return LOMSE_NEW MidiDeviceMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_midi_instrument:      return LOMSE_NEW MidiInstrumentMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_notations:            return LOMSE_NEW NotationsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_note:                 return LOMSE_NEW NoteRestMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_octave_shift:         return LOMSE_NEW OctaveShiftMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_ornaments:            return LOMSE_NEW OrnamentsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part:                 return LOMSE_NEW PartMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part_group:           return LOMSE_NEW PartGroupMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part_list:            return LOMSE_NEW PartListMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_part_name:            return LOMSE_NEW PartNameMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_pedal:                return LOMSE_NEW PedalMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_percussion:           return LOMSE_NEW PercussionMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_pitch:                return LOMSE_NEW PitchMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_principal_voice:      return LOMSE_NEW PrincipalVoiceMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_print:                return LOMSE_NEW PrintMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_rehearsal:            return LOMSE_NEW RehearsalMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_scordatura:           return LOMSE_NEW ScordaturaMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_score_instrument:     return LOMSE_NEW ScoreInstrumentMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_score_part:           return LOMSE_NEW ScorePartMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_score_partwise:       return LOMSE_NEW ScorePartwiseMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_segno:                return LOMSE_NEW SegnoMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_slur:                 return LOMSE_NEW SlurMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_sound:                return LOMSE_NEW SoundMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_mxl_tag_string_mute:          return LOMSE_NEW StringMmuteMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_technical:            return LOMSE_NEW TecnicalMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_text:                 return LOMSE_NEW TextMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_tied:                 return LOMSE_NEW TiedMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_time:                 return LOMSE_NEW TimeMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_time_modification:    return LOMSE_NEW TimeModificationXmlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_tuplet:               return LOMSE_NEW TupletMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_tuplet_actual:        return LOMSE_NEW TupletNumbersMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_tuplet_normal:        return LOMSE_NEW TupletNumbersMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_virtual_instr:        return LOMSE_NEW VirtualInstrumentMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_wedge:                return LOMSE_NEW WedgeMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_words:                return LOMSE_NEW WordsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        default:
            return LOMSE_NEW NullMxlAnalyser(this, m_reporter, m_libraryScope, name);
    }
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::name_to_enum(const string& name) const
{
	map<string, int>::const_iterator it = m_NameToEnum.find(name);
	if (it != m_NameToEnum.end())
		return it->second;
    else
        return k_mxl_tag_undefined;
}



//=======================================================================================
// MxlTiesBuilder implementation
//=======================================================================================
void MxlTiesBuilder::add_relation_to_staffobjs(ImoTieDto* pEndDto)
{
    ImoTieDto* pStartDto = m_matches.front();
    ImoNote* pStartNote = pStartDto->get_note();
    ImoNote* pEndNote = pEndDto->get_note();
    if (notes_can_be_tied(pStartNote, pEndNote))
        tie_notes(pStartDto, pEndDto);
    else
        error_notes_can_not_be_tied(pEndDto);
}

//---------------------------------------------------------------------------------------
bool MxlTiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
{
    return (pStartNote->get_voice() == pEndNote->get_voice())
            && (pStartNote->get_staff() == pEndNote->get_staff())
            && (pStartNote->get_actual_accidentals() == pEndNote->get_actual_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

//---------------------------------------------------------------------------------------
void MxlTiesBuilder::tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto)
{
    ImoNote* pStartNote = pStartDto->get_note();
    ImoNote* pEndNote = pEndDto->get_note();
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoTie* pTie = static_cast<ImoTie*>(
                    ImFactory::inject(k_imo_tie, pDoc));
    pTie->set_tie_number( pStartDto->get_tie_number() );
    pTie->set_color( pStartDto->get_color() );
    pTie->set_orientation( pStartDto->get_orientation() );

    ImoTieData* pStartData = ImFactory::inject_tie_data(pDoc, pStartDto);
    pStartNote->include_in_relation(pDoc, pTie, pStartData);

    ImoTieData* pEndData = ImFactory::inject_tie_data(pDoc, pEndDto);
    pEndNote->include_in_relation(pDoc, pTie, pEndData);

    pStartNote->set_tie_next(pTie);
    pEndNote->set_tie_prev(pTie);
}

//---------------------------------------------------------------------------------------
void MxlTiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". Requesting to tie notes of different voice or pitch. Tie number "
               << pEndInfo->get_tie_number()
               << " will be ignored." << endl;
}


//=======================================================================================
// MxlSlursBuilder implementation
//=======================================================================================
void MxlSlursBuilder::add_relation_to_staffobjs(ImoSlurDto* pEndInfo)
{
    m_matches.push_back(pEndInfo);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoSlur* pSlur = static_cast<ImoSlur*>(ImFactory::inject(k_imo_slur, pDoc));
    pSlur->set_slur_number( pEndInfo->get_slur_number() );

    std::list<ImoSlurDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNote* pNote = (*it)->get_note();
        ImoSlurData* pData = ImFactory::inject_slur_data(pDoc, *it);
        pNote->include_in_relation(pDoc, pSlur, pData);
    }
}


//=======================================================================================
// MxlBeamsBuilder implementation
//=======================================================================================
void MxlBeamsBuilder::add_relation_to_staffobjs(ImoBeamDto* pEndInfo)
{
    m_matches.push_back(pEndInfo);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoBeam* pBeam = static_cast<ImoBeam*>(ImFactory::inject(k_imo_beam, pDoc));

    bool fErrors = false;
    std::list<ImoBeamDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        ImoBeamData* pData = ImFactory::inject_beam_data(pDoc, *it);
        pNR->include_in_relation(pDoc, pBeam, pData);

        //check if beam is congruent with note type
        int level = 0;
        for (int i=0; i < 6; ++i)
        {
            if ((*it)->get_beam_type(i) == ImoBeam::k_none)
                break;
            ++level;
        }
        int type = pNR->get_note_type();
        switch(level)
        {
            case 0: fErrors = true;                 break;
            case 1: fErrors |= (type != k_eighth);  break;
            case 2: fErrors |= (type != k_16th);    break;
            case 3: fErrors |= (type != k_32nd);    break;
            case 4: fErrors |= (type != k_64th);    break;
            case 5: fErrors |= (type != k_128th);   break;
            case 6: fErrors |= (type != k_256th);   break;
        }
    }

    //AWARE: MusicXML requires full item description. Autobeamer is only needed
    //       when the file is malformed and the option 'fix_beams' is enabled
    if (fErrors && m_pAnalyser->fix_beams())
    {
        AutoBeamer autobeamer(pBeam);
        autobeamer.do_autobeam();
    }
}



//=======================================================================================
// MxlTupletsBuilder implementation
//=======================================================================================
void MxlTupletsBuilder::add_relation_to_staffobjs(ImoTupletDto* pEndDto)
{
    m_matches.push_back(pEndDto);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoTupletDto* pStartDto = m_matches.front();
    ImoTuplet* pTuplet = ImFactory::inject_tuplet(pDoc, pStartDto);

    std::list<ImoTupletDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        pNR->include_in_relation(pDoc, pTuplet, nullptr);
    }
}

//---------------------------------------------------------------------------------------
void MxlTupletsBuilder::add_to_open_tuplets(ImoNoteRest* pNR)
{
    if (m_pendingItems.size() > 0)
    {
        list<int> excludes;
        ListIterator it;
        for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
        {
            if ((*it)->get_staffobj() == pNR)
                excludes.push_back((*it)->get_item_number());
        }

        for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
        {
            if ((*it)->is_start_of_relation() && (*it)->get_item_number()
                && (find(excludes.begin(), excludes.end(), (*it)->get_item_number())
                     == excludes.end()) )
            {
                ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
                pInfo->set_tuplet_number( (*it)->get_item_number() );
                pInfo->set_tuplet_type(ImoTupletDto::k_continue);
                pInfo->set_note_rest(pNR);
                save_item_info(pInfo);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void MxlTupletsBuilder::get_factors_from_nested_tuplets(int* pTop, int* pBottom)
{
    *pTop = 1;
    *pBottom = 1;
    ListIterator it;
    for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
    {
        if ((*it)->is_start_of_relation() )
        {
            ImoTupletDto* pInfo = static_cast<ImoTupletDto*>(*it);
            *pTop *= pInfo->get_normal_number();
            *pBottom *= pInfo->get_actual_number();
        }
    }
}


//=======================================================================================
// MxlVoltasBuilder implementation
//=======================================================================================
void MxlVoltasBuilder::add_relation_to_staffobjs(ImoVoltaBracketDto* pEndDto)
{
    ImoVoltaBracketDto* pStartDto = m_matches.front();
    m_matches.push_back(pEndDto);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoVoltaBracket* pVB = static_cast<ImoVoltaBracket*>(
                                ImFactory::inject(k_imo_volta_bracket, pDoc));

    //set data taken from end dto
    pVB->set_volta_number( pEndDto->get_volta_number() );
    pVB->set_final_jog( pEndDto->get_final_jog() );

    //set data taken from start dto
    pVB->set_volta_text( pStartDto->get_volta_text() );
    pVB->set_repetitions( pStartDto->get_repetitions() );

    std::list<ImoVoltaBracketDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoBarline* pBarline = (*it)->get_barline();
        pBarline->include_in_relation(pDoc, pVB, nullptr);
    }

    //count number of voltas in the set
    if (pVB->is_first_repeat())
        m_pFirstVB = pVB;
    else if (m_pFirstVB)
        m_pFirstVB->increment_total_voltas();

    //set number of repetitions in barline
    ImoBarline* pBarline = static_cast<ImoBarline*>( pVB->get_end_object() );
    pBarline->set_num_repeats( pVB->get_number_of_repetitions() );
}


//=======================================================================================
// MxlWedgesBuilder implementation
//=======================================================================================
void MxlWedgesBuilder::add_relation_to_staffobjs(ImoWedgeDto* pEndDto)
{
    ImoWedgeDto* pStartDto = m_matches.front();
    m_matches.push_back(pEndDto);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoWedge* pWedge = static_cast<ImoWedge*>(
                                ImFactory::inject(k_imo_wedge, pDoc));

    //set data taken from start dto
    pWedge->set_start_spread( pStartDto->get_spread() );
    pWedge->set_wedge_number( pStartDto->get_wedge_number() );
    pWedge->set_color( pStartDto->get_color() );

    //set data taken from end dto
    pWedge->set_end_spread( pEndDto->get_spread() );

    //set data that can be on any of them
    pWedge->set_niente( pStartDto->is_niente() || pEndDto->is_niente() );
    pWedge->set_crescendo( pStartDto->is_crescendo() || pEndDto->is_crescendo());


    //set default spread when no spread is specified
    if (pEndDto->get_spread() == 0.0f && pStartDto->get_spread() == 0.0f)
    {
        if (pWedge->is_crescendo())
            pWedge->set_end_spread(15.0f);
        else
            pWedge->set_start_spread(15.0f);
    }

    std::list<ImoWedgeDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoDirection* pDirection = (*it)->get_staffobj();
        pDirection->include_in_relation(pDoc, pWedge, nullptr);
    }
}


//=======================================================================================
// MxlOctaveShiftBuilder implementation
//=======================================================================================
void MxlOctaveShiftBuilder::add_relation_to_staffobjs(ImoOctaveShiftDto* pEndDto)
{
    ImoOctaveShiftDto* pStartDto = m_matches.front();
    ImoNote* pStartNote = pStartDto->get_staffobj();
    m_matches.push_back(pEndDto);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoOctaveShift* pOctave = static_cast<ImoOctaveShift*>(
                                        ImFactory::inject(k_imo_octave_shift, pDoc));

    //set data taken from start dto
    pOctave->set_octave_shift_number( pStartDto->get_octave_shift_number() );
    pOctave->set_shift_steps( pStartDto->get_shift_steps() );
    pOctave->set_color( pStartDto->get_color() );

    //set data taken from end dto
        //none

    //set data that can be on any of them
        //none

    std::list<ImoOctaveShiftDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNote* pNote = (*it)->get_staffobj();
        if ((*it)->is_end_of_relation() && pNote==nullptr)
        {
            int iStaff = (*it)->get_staff();
            pNote = m_pAnalyser->get_last_note_for(iStaff);
            (*it)->set_staffobj(pNote);
            if (pStartNote != pNote)
                pNote->include_in_relation(pDoc, pOctave, nullptr);
        }
        else
            pNote->include_in_relation(pDoc, pOctave, nullptr);
    }
}

//---------------------------------------------------------------------------------------
void MxlOctaveShiftBuilder::add_to_open_octave_shifts(ImoNote* pNote)
{
    if (m_pendingItems.size() > 0)
    {
        ListIterator it;
        for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
        {
            ImoOctaveShiftDto* pInfo = *it;
            if (pInfo->is_start_of_relation()
                && pInfo->get_staffobj() == nullptr
                && pInfo->get_staff() == pNote->get_staff()
               )
            {
                pInfo->set_staffobj(pNote);
            }
        }
    }
}


}   //namespace lomse
