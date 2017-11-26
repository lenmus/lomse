//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2017. All rights reserved.
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

#include "lomse_mnx_analyser.h"

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
// MnxPartList implementation: helper class to save part-list info
//=======================================================================================
MnxPartList::MnxPartList()
    : m_numInstrs(0)
    , m_fInstrumentsAdded(false)
{
}

//---------------------------------------------------------------------------------------
MnxPartList::~MnxPartList()
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
void MnxPartList::add_score_part(const string& id, ImoInstrument* pInstrument)
{
    m_locators[id] = m_numInstrs++;
    m_instruments.push_back(pInstrument);
    m_partAdded.push_back(false);
}

//---------------------------------------------------------------------------------------
bool MnxPartList::mark_part_as_added(const string& id)
{
    int i = find_index_for(id);
    if (m_partAdded[i])
        return true;    //if instrument is already marked!
    m_partAdded[i] = true;
    return false;
}

//---------------------------------------------------------------------------------------
ImoInstrument* MnxPartList::get_instrument(const string& id)
{
	int i = find_index_for(id);
	return (i != -1 ? m_instruments[i] : nullptr);
}

//---------------------------------------------------------------------------------------
int MnxPartList::find_index_for(const string& id)
{
	map<string, int>::const_iterator it = m_locators.find(id);
	return (it != m_locators.end() ? it->second : -1);
}

//---------------------------------------------------------------------------------------
void MnxPartList::add_all_instruments(ImoScore* pScore)
{
    m_fInstrumentsAdded = true;
    for (int i=0; i < m_numInstrs; ++i)
        pScore->add_instrument(m_instruments[i]);
}

//---------------------------------------------------------------------------------------
void MnxPartList::check_if_missing_parts(ostream& reporter)
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
// MnxPartGroups implementation: helper class to manage open <part-group> tags
//=======================================================================================
MnxPartGroups::MnxPartGroups()
{
}

//---------------------------------------------------------------------------------------
MnxPartGroups::~MnxPartGroups()
{
    map<int, ImoInstrGroup*>::iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
        delete it->second;

    m_groups.clear();
}

//---------------------------------------------------------------------------------------
void MnxPartGroups::add_instrument_to_groups(ImoInstrument* pInstr)
{
    map<int, ImoInstrGroup*>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        ImoInstrGroup* pGrp = it->second;
        pGrp->add_instrument(pInstr);
    }
}

//---------------------------------------------------------------------------------------
void MnxPartGroups::start_group(int number, ImoInstrGroup* pGrp)
{
    m_groups[number] = pGrp;
}

//---------------------------------------------------------------------------------------
void MnxPartGroups::terminate_group(int number)
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
bool MnxPartGroups::group_exists(int number)
{
    map<int, ImoInstrGroup*>::const_iterator it = m_groups.find(number);
	return (it != m_groups.end());
}

//---------------------------------------------------------------------------------------
ImoInstrGroup* MnxPartGroups::get_group(int number)
{
    map<int, ImoInstrGroup*>::iterator it = m_groups.find(number);
	if (it != m_groups.end())
        return it->second;
    else
        return nullptr;

}

//---------------------------------------------------------------------------------------
void MnxPartGroups::check_if_all_groups_are_closed(ostream& reporter)
{
    map<int, ImoInstrGroup*>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        reporter << "Error: missing <part-group type='stop'> for <part-group> number='"
                 << it->first << "'." << endl;
    }
}

//---------------------------------------------------------------------------------------
void MnxPartGroups::set_barline_layout_in_instruments(ImoInstrGroup* pGrp)
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
enum EMnxTag
{
    k_mnx_tag_undefined = -1,

//    k_mnx_tag_accordion_registration,
//    k_mnx_tag_articulations,
    k_mnx_tag_attributes,
//    k_mnx_tag_backup,
//    k_mnx_tag_barline,
//    k_mnx_tag_bracket,
//    k_mnx_tag_clef,
//    k_mnx_tag_coda,
//    k_mnx_tag_damp,
//    k_mnx_tag_damp_all,
//    k_mnx_tag_dashes,
//    k_mnx_tag_direction,
//    k_mnx_tag_direction_type,
//    k_mnx_tag_dynamics,
//    k_mnx_tag_ending,
    k_mnx_tag_event,
//    k_mnx_tag_eyeglasses,
//    k_mnx_tag_fermata,
//    k_mnx_tag_forward,
//    k_mnx_tag_harp_pedals,
    k_mnx_tag_head,
//    k_mnx_tag_image,
//    k_mnx_tag_key,
//    k_mnx_tag_lyric,
    k_mnx_tag_measure,
//    k_mnx_tag_metronome,
//    k_mnx_tag_midi_device,
//    k_mnx_tag_midi_instrument,
    k_mnx_tag_mnx,
//    k_mnx_tag_notations,
//    k_mnx_tag_octave_shift,
//    k_mnx_tag_ornaments,
    k_mnx_tag_part,
//    k_mnx_tag_part_group,
//    k_mnx_tag_part_list,
    k_mnx_tag_part_name,
//    k_mnx_tag_pedal,
//    k_mnx_tag_percussion,
//    k_mnx_tag_pitch,
//    k_mnx_tag_principal_voice,
//    k_mnx_tag_print,
//    k_mnx_tag_rehearsal,
//    k_mnx_tag_scordatura,
    k_mnx_tag_score,
//    k_mnx_tag_score_instrument,
//    k_mnx_tag_score_part,
//    k_mnx_tag_score_partwise,
//    k_mnx_tag_segno,
    k_mnx_tag_sequence,
//    k_mnx_tag_slur,
//    k_mnx_tag_sound,
    k_mnx_tag_staff,
//    k_mnx_tag_string_mute,
//    k_mnx_tag_technical,
//    k_mnx_tag_text,
//    k_mnx_tag_tied,
//    k_mnx_tag_time,
//    k_mnx_tag_time_modification,
//    k_mnx_tag_tuplet,
//    k_mnx_tag_tuplet_actual,
//    k_mnx_tag_tuplet_normal,
//    k_mnx_tag_virtual_instr,
//    k_mnx_tag_wedge,
//    k_mnx_tag_words,
};


//=======================================================================================
// Helper class MnxElementAnalyser.
// Abstract class: any element analyser must derive from it

class MnxElementAnalyser
{
protected:
    ostream& m_reporter;
    MnxAnalyser* m_pAnalyser;
    LibraryScope& m_libraryScope;
    LdpFactory* m_pLdpFactory;
    ImoObj* m_pAnchor;

public:
    MnxElementAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor=nullptr)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_libraryScope(libraryScope)
        , m_pLdpFactory(libraryScope.ldp_factory())
        , m_pAnchor(pAnchor) {}
    virtual ~MnxElementAnalyser() {}
    ImoObj* analyse_node(XmlNode* pNode);

protected:

    //analysis
    virtual ImoObj* do_analysis() = 0;

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

    //analysers for common elements
    int analyse_optional_staff(int nDefault);

    //methods to get attributes of current element
    bool has_attribute(const string& name);
    string get_attribute(const string& name);
    int get_attribute_as_integer(const string& name, int nNumber);
    string get_mandatory_string_attribute(const string& name, const string& sDefault,
                                          const string& element);
    string get_optional_string_attribute(const string& name, const string& sDefault);
    int get_mandatory_integer_attribute(const string& name, int nDefault,
                                        const string& element);
    int get_optional_int_attribute(const string& name, int nDefault);
    bool get_optional_yes_no_attribute(const string& name, bool fDefault);

    //methods to analyse attributes of current element
    bool get_attribute_note_value(int* noteType, int* dots);
    Tenths get_attribute_as_tenths(const string& name, Tenths rDefault);
    int get_attribute_placement();
    void get_attributes_for_text_formatting(ImoObj* pImo);
    void get_attributes_for_print_style_align(ImoObj* pImo);
    void get_attributes_for_print_style(ImoObj* pImo);
    void get_attributes_for_position(ImoObj* pObj);
    void get_attribute_color(ImoObj* pImo);

    //methods to get value of current node
    int get_cur_node_value_as_integer(int nDefault);

    //methods for analysing children
    string get_child_value_string() { return m_childToAnalyse.value(); }
    long get_child_value_long(long nDefault=0L);
    int get_child_value_integer(int nDefault);
    float get_child_value_float(float rDefault=0.0f);
    bool get_child_value_bool(bool fDefault=false);
    int get_child_value_yes_no(int nDefault);
    bool is_child_value_long();
    bool is_child_value_float();
    bool is_child_value_bool();

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

};



//=======================================================================================
// MnxElementAnalyser implementation
//=======================================================================================
ImoObj* MnxElementAnalyser::analyse_node(XmlNode* pNode)
{
    m_analysedNode = *pNode;
    move_to_first_child();
    return do_analysis();
}

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::error_missing_element(const string& tag)
{
    string parentName = m_analysedNode.name();
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
               "<" + parentName + ">: missing mandatory element <" + tag + ">.");
    return false;
}

//---------------------------------------------------------------------------------------
void MnxElementAnalyser::report_msg(int numLine, const std::stringstream& msg)
{
    report_msg(numLine, msg.str());
}

//---------------------------------------------------------------------------------------
void MnxElementAnalyser::report_msg(int numLine, const std::string& msg)
{
    m_reporter << "Line " << numLine << ". " << msg << endl;
}

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::has_attribute(const string& name)
{
    return has_attribute(&m_analysedNode, name);
}

//---------------------------------------------------------------------------------------
string MnxElementAnalyser::get_attribute(const string& name)
{
    return m_analysedNode.attribute_value(name);
}

//---------------------------------------------------------------------------------------
string MnxElementAnalyser::get_mandatory_string_attribute(const string& name,
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
string MnxElementAnalyser::get_optional_string_attribute(const string& name,
                                                         const string& sDefault)
{
    if (has_attribute(&m_analysedNode, name))
        return m_analysedNode.attribute_value(name);
    else
        return sDefault;
}

//---------------------------------------------------------------------------------------
int MnxElementAnalyser::get_attribute_as_integer(const string& name, int nDefault)
{
    string number = m_analysedNode.attribute_value(name);
    long nNumber;
    std::istringstream iss(number);
    if ((iss >> std::dec >> nNumber).fail())
        return nDefault;
    else
        return nNumber;
}

//---------------------------------------------------------------------------------------
int MnxElementAnalyser::get_optional_int_attribute(const string& name,
                                                       int nDefault)
{
    if (has_attribute(&m_analysedNode, name))
        return get_attribute_as_integer(name, nDefault);
    else
        return nDefault;
}

//---------------------------------------------------------------------------------------
int MnxElementAnalyser::get_mandatory_integer_attribute(const string& name, int nDefault,
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
bool MnxElementAnalyser::get_optional_yes_no_attribute(const string& name, bool fDefault)
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
bool MnxElementAnalyser::get_mandatory(const string& tag)
{
    if (!more_children_to_analyse())
    {
        error_missing_element(tag);
        return nullptr;
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
bool MnxElementAnalyser::analyse_mandatory(const string& tag, ImoObj* pAnchor)
{
    if (get_mandatory(tag))
        return (m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor) != nullptr);
    else
        return false;
}

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::get_optional(const string& name)
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
bool MnxElementAnalyser::analyse_optional(const string& name, ImoObj* pAnchor)
{
    if (get_optional(name))
    {
        m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
string MnxElementAnalyser::analyze_mandatory_child_pcdata(const string& name)
{
    if (get_mandatory(name))
        return m_childToAnalyse.value();

	return "";
}

//---------------------------------------------------------------------------------------
string MnxElementAnalyser::analyze_optional_child_pcdata(const string& name,
                                                         const string& sDefault)
{
    if (get_optional(name))
        return m_childToAnalyse.value();

	return sDefault;
}

//---------------------------------------------------------------------------------------
int MnxElementAnalyser::analyze_optional_child_pcdata_int(const string& name,
                                                          int nMin, int nMax,
                                                          int nDefault)
{
    if (get_optional(name))
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

	return nDefault;
}

//---------------------------------------------------------------------------------------
float MnxElementAnalyser::analyze_optional_child_pcdata_float(const string& name,
                                                              float rMin, float rMax,
                                                              float rDefault)
{
    if (get_optional(name))
    {
        bool fError = false;
        string number = m_childToAnalyse.value();
        long rNumber;
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

	return rDefault;
}

//---------------------------------------------------------------------------------------
int MnxElementAnalyser::get_cur_node_value_as_integer(int nDefault)
{
    string number = m_analysedNode.value();
    long nNumber;
    std::istringstream iss(number);
    if ((iss >> std::dec >> nNumber).fail())
        return nDefault;
    else
        return nNumber;
}

////---------------------------------------------------------------------------------------
//void MnxElementAnalyser::analyse_one_or_more(ELdpElement* pValid, int nValid)
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
//bool MnxElementAnalyser::contains(ELdpElement type, ELdpElement* pValid, int nValid)
//{
//    for (int i=0; i < nValid; i++, pValid++)
//        if (*pValid == type) return true;
//    return false;
//}

//---------------------------------------------------------------------------------------
void MnxElementAnalyser::error_invalid_child()
{
    string name = m_childToAnalyse.name();
    if (name == "label")
        name += ":" + m_childToAnalyse.value();
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
        "Element '" + name + "' unknown or not possible here. Ignored.");
}

//---------------------------------------------------------------------------------------
void MnxElementAnalyser::error_msg(const string& msg)
{
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode), msg);
}

//---------------------------------------------------------------------------------------
void MnxElementAnalyser::error_msg2(const string& msg)
{
    error_msg(m_pAnalyser->get_element_info() + msg);
}

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::error_if_more_elements()
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
void MnxElementAnalyser::add_to_model(ImoObj* pImo, int type)
{
    Linker linker( m_pAnalyser->get_document_being_analysed() );
    linker.add_child_to_model(m_pAnchor, pImo, type == -1 ? pImo->get_obj_type() : type);
}


//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::is_child_value_long()
{
    string number = m_childToAnalyse.value();
    long nNumber;
    std::istringstream iss(number);
    return !((iss >> std::dec >> nNumber).fail());
}

//---------------------------------------------------------------------------------------
long MnxElementAnalyser::get_child_value_long(long nDefault)
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

//---------------------------------------------------------------------------------------
int MnxElementAnalyser::get_child_value_integer(int nDefault)
{
    return static_cast<int>( get_child_value_long(static_cast<int>(nDefault)) );
}

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::is_child_value_float()
{
    string number = m_childToAnalyse.value();
    float rNumber;
    std::istringstream iss(number);
    return !((iss >> std::dec >> rNumber).fail());
}

//---------------------------------------------------------------------------------------
float MnxElementAnalyser::get_child_value_float(float rDefault)
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

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::is_child_value_bool()
{
    string value = string(m_childToAnalyse.value());
    return  value == "true" || value == "yes"
         || value == "false" || value == "no" ;
}

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::get_child_value_bool(bool fDefault)
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

//---------------------------------------------------------------------------------------
int MnxElementAnalyser::get_child_value_yes_no(int nDefault)
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
// Analysers for common attributes
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//Note value
bool MnxElementAnalyser::get_attribute_note_value(int* noteType, int* dots)
{
    //return FALSE is error

    if (has_attribute(&m_analysedNode, "value"))
    {
        string value = m_analysedNode.attribute_value("value");

//            stringstream s;
//            s << "get_attribute_note_value. value=" << value;

        //count and remove trailing '*'
        *dots = 0;
        while(value.back() == '*')
        {
            *dots += 1;
            value.pop_back();
        }

        //analyse remaning string
        if (value == "breve")
            *noteType = k_longa;
        else if (value == "1" || value == "whole")
            *noteType = k_whole;
        else if (value == "2" || value == "half")
            *noteType = k_half;
        else if (value == "4" || value == "quarter")
            *noteType = k_quarter;
        else if (value == "8" || value == "eighth")
            *noteType = k_eighth;
        else if (value == "16")
            *noteType = k_16th;
        else if (value == "32")
            *noteType = k_32nd;
        else if (value == "64")
            *noteType = k_64th;
        else if (value == "128")
            *noteType = k_128th;
        else
            *noteType = k_256th;

//            s << ". Value=" << value << ", noteType=" << *noteType
//              << ", dots=" << *dots;
//            LOMSE_LOG_DEBUG(Logger::k_all, s.str());

        return true;
    }
    return false;
}


//-----------------------------------------------------------------------------------
//@ % tenths
//@ The tenths entity is a number representing tenths. Both integer and decimal
//@ values are allowed, such as 5 for a half space and -2.5
//@<!ENTITY % tenths "CDATA">
Tenths MnxElementAnalyser::get_attribute_as_tenths(const string& name, Tenths rDefault)
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
int MnxElementAnalyser::get_attribute_placement()
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
void MnxElementAnalyser::get_attributes_for_text_formatting(ImoObj* pImo)
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
void MnxElementAnalyser::get_attributes_for_print_style_align(ImoObj* pImo)
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
void MnxElementAnalyser::get_attributes_for_print_style(ImoObj* pImo)
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
void MnxElementAnalyser::get_attributes_for_position(ImoObj* pObj)
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
void MnxElementAnalyser::get_attribute_color(ImoObj* pImo)
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
int MnxElementAnalyser::analyse_optional_staff(int nDefault)
{
    if (get_optional("staff"))
        return get_child_value_integer(nDefault);
    else
        return nDefault;
}





//---------------------------------------------------------------------------------------
// default analyser to use when there is no defined analyser for an LDP element

class NullMnxAnalyser : public MnxElementAnalyser
{
protected:
    const string m_tag;

public:
    NullMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    const string& tag)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope)
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
//@ <attributes>
//@ <!ELEMENT attributes (staff | instrument-sound | tempo | time)* )>
//
class AttributesMnxAnalyser : public MnxElementAnalyser
{
public:
    AttributesMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                          LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        {}


    ImoObj* do_analysis()
    {
        //In MNX Clefs, time signatures and key signatures are
        //treated as attributes of a measure, not as objects and, therefore, ordering
        //is not important for MNX and this information is
        //coded bad order (first key signatures, then time signatures, then clefs).
        //As Lomse expects that these objects are defined in correct order,
        //objects creation will be delayed until all attributes are parsed.
        vector<ImoObj*> times;
        vector<ImoObj*> keys;
        vector<ImoObj*> clefs;

        // (staff | instrument-sound | tempo | time)*    alternatives: zero or more
        int staves = 0;
        while (more_children_to_analyse())
        {
            if (analyse_optional("staff"))
            {
                ++staves;
            }
            else if (analyse_optional("time"))
            {
            }
            else if (analyse_optional("instrument-sound"))
            {
            }
            else if (analyse_optional("tempo"))
            {
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

//        // staff*
//        while ();
//
//        // key*
//        while (get_optional("key"))
//            keys.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr) );
//
//        // time*
//        while (get_optional("time"))
//            times.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr) );
//
        // tempo

        //set staves
        if (staves > 1)
        {
            ImoInstrument* pInstr = m_pAnalyser->get_instrument_being_analysed();
            for(; staves > 1; --staves)
                pInstr->add_staff();
        }

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

//---------------------------------------------------------------------------------------
//@ <event>
//@<!ELEMENT event ( stem?,
//@    (note | rest | tuplet | lyric | beam)* )>
//@<!ATTLIST event
//@    value            ==> event duration, expressed in terms of note value and dot count
//@    grace="true"     ==> grace notes
//@    type="measure"   ==> For full measure rests
//@>
//
class EventMnxAnalyser : public MnxElementAnalyser
{
protected:
    struct NoteInfo
    {
        int step;
        int octave;
        EAccidentals accidentals;
        float alterations;

        NoteInfo(int s, int o, EAccidentals acc, float alter)
        {
            step = s;
            octave = o;
            accidentals = acc;
            alterations = alter;
        }
    };

    struct RestInfo
    {
        int displayStep;
        int displayOctave;

        RestInfo(int s, int o)
        {
            displayStep = s;
            displayOctave = o;
        }
    };

    vector<NoteInfo*> m_notes;
    vector<RestInfo*> m_rests;

public:
    EventMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ~EventMnxAnalyser()
    {
        delete_info_objects();
    }

    ImoObj* do_analysis()
    {
        //First step: extract information -----------------------------------------------

        // attrib: value
        int noteType;
        int dots;
        if (!get_attribute_note_value(&noteType, &dots))
        {
            error_msg("Missing or invalid 'value' attribute in <event>.");
            return nullptr;
        }

        // attrib: grace
            //TODO
        //bool fIsGrace = false;

        // attrib: type
            //TODO

        // (xxxx | yyyy | zzzz)*    alternatives: zero or more
        while (more_children_to_analyse())
        {
            if (get_optional("note"))
                analyse_note();
            else if (get_optional("rest"))
                analyse_rest();
            else if (get_optional("stem"))
            {
            }
            else if (get_optional("tuplet"))
            {
            }
            else if (get_optional("lyric"))
            {
            }
            else if (get_optional("beam"))
            {
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }


        //second step: create notes -----------------------------------------------------

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        int staff = m_pAnalyser->get_current_staff();

        //add notes
        bool fIsChord = m_notes.size() > 1;
        vector<NoteInfo*>::iterator itN;
        ImoNote* pPrevNote = nullptr;
        for (itN = m_notes.begin(); itN != m_notes.end(); ++itN)
        {
            ImoNote* pNR = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, pDoc));
            pNR->set_note_type_and_dots(noteType, dots);
            pNR->set_notated_pitch((*itN)->step, (*itN)->octave, (*itN)->accidentals);
            pNR->set_staff(staff);
            pNR->set_voice( m_pAnalyser->get_current_voice() );
            add_to_model(pNR);

            //deal with notes in chord
            if (fIsChord)
            {
                if (!pPrevNote)
                {
                    //this note is the base note. Create the chord
                    ImoChord* pChord = static_cast<ImoChord*>(ImFactory::inject(k_imo_chord, pDoc));
                    pNR->include_in_relation(pDoc, pChord);
                }
                else
                {
                    //chord already created. just add this note to itN
                    ImoChord* pChord = pPrevNote->get_chord();
                    pNR->include_in_relation(pDoc, pChord);
                }
            }
            pPrevNote = pNR;
            delete *itN;
        }

        //add rests
        vector<RestInfo*>::iterator itR;
        for (itR = m_rests.begin(); itR != m_rests.end(); ++itR)
        {
            ImoRest* pNR = static_cast<ImoRest*>(ImFactory::inject(k_imo_rest, pDoc));
            pNR->set_note_type_and_dots(noteType, dots);
            add_to_model(pNR);
            pNR->set_staff(staff);
            pNR->set_voice( m_pAnalyser->get_current_voice() );
            delete *itR;
        }

//        m_pAnalyser->shift_time( pNR->get_duration() );

        return nullptr;
    }

protected:

    void analyse_note()
    {
        //@ <note>
        //@<!ELEMENT note EMPTY )>
        //@<!ATTLIST note
        //@    pitch
        //@>

        if (!m_childToAnalyse.has_attribute("pitch"))
        {
            error_msg("Missing attribute 'pitch' in note. note ignored.");
            return;
        }

        string pitch = m_childToAnalyse.attribute_value("pitch");
        int step = k_step_C;
        int octave = 4;
        EAccidentals accidentals = k_no_accidentals;
        float alter = 0.0f;
        NoteInfo* pInfo;
        if (MnxAnalyser::pitch_to_components(pitch, &step, &octave,&accidentals, &alter))
        {
            error_msg("Unknown note pitch '" + pitch + "'. Replaced by 'C4'.");
            pInfo = LOMSE_NEW NoteInfo(k_step_C, 4, k_no_accidentals, 0.0f);
        }
        else
            pInfo = LOMSE_NEW NoteInfo(step, octave, accidentals, alter);

        m_notes.push_back(pInfo);
    }

    void analyse_rest()
    {
        //@ <rest>
        //@<!ELEMENT rest EMPTY )>
        //@<!ATTLIST rest
        //@>

        int step = k_step_C;
        int octave = 4;
        RestInfo* pInfo = LOMSE_NEW RestInfo(step, octave);
        m_rests.push_back(pInfo);
    }

    void delete_info_objects()
    {
        m_notes.clear();
        m_rests.clear();
    }

};

//@--------------------------------------------------------------------------------------
//@ <head>
//@ <!ELEMENT head (identification?, style?
//@     (sequence)* ) >
//@ <!ATTLIST head
//@>
//
class HeadMnxAnalyser : public MnxElementAnalyser
{
protected:
    const string m_tag;

public:
    HeadMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
		//TODO
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT measure (attributes?,
//@     (sequence)* ) >
////@ <!ATTLIST measure
////@     number CDATA #REQUIRED
////@     implicit %yes-no; #IMPLIED
////@     non-controlling %yes-no; #IMPLIED
////@     width %tenths; #IMPLIED
////@ >
//
class MeasureMnxAnalyser : public MnxElementAnalyser
{
public:
    MeasureMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        ImoMusicData* pMD = dynamic_cast<ImoMusicData*>(m_pAnchor);
        bool fSomethingAdded = false;

//        //attrb: number #REQUIRED
//        string num = get_optional_string_attribute("number", "");
//        if (num.empty())
//        {
//            error_msg("<measure>: missing mandatory 'number' attribute. <measure> content will be ignored");
//            return nullptr;
//        }
//        m_pAnalyser->save_current_measure_num(num);

        if (analyse_optional("attributes", pMD))
            fSomethingAdded = true;

        // [{<xxxx>|<yyyy>|<zzzz>}*]    alternatives: zero or more
        while (more_children_to_analyse())
        {
            if (analyse_optional("sequence", pMD))
                fSomethingAdded = true;
//            else if (analyse_optional("barline", pMD))
//                fSomethingAdded = true;
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        error_if_more_elements();

        if (fSomethingAdded)
        {
            ImoObj* pSO = static_cast<ImoStaffObj*>(pMD->get_last_child());
            if (pSO == nullptr || !pSO->is_barline())
                add_barline(pMD);
        }

        return pMD;
    }

protected:

    void add_barline(ImoMusicData* UNUSED(pMD))
    {
        advance_timepos_if_required();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBarline* pBarline = static_cast<ImoBarline*>(
                                    ImFactory::inject(k_imo_barline, pDoc) );
        pBarline->set_type(k_barline_simple);
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
//@ <mnx>
//@<!ELEMENT mnx (head?,
//@    (collection | score*)) >
//
class MnxMnxAnalyser : public MnxElementAnalyser
{
public:
    MnxMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                   LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

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
        pDoc->set_imo_doc(pImoDoc);
        m_pAnchor = pImoDoc;

        // add default styles
        add_default(pImoDoc);

		// head?
        analyse_optional("head", pImoDoc);

        // (collection | score*)
        analyse_optional("score", pImoDoc);

        //TODO: collection

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
};

//@--------------------------------------------------------------------------------------
//@<!ELEMENT part (identification?,
//@    part-name, part-name-display?,
//@    part-abbreviation?, part-abbreviation-display?,
//@    group*, score-instrument*,
//@    (midi-device?, midi-instrument?)*,
//@    measure* )>
//@<!ATTLIST part
//@    id ID #REQUIRED
//@>
//
class PartMnxAnalyser : public MnxElementAnalyser
{
public:
    PartMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoInstrument* pInstrument = create_instrument();

        // part-name
        analyse_optional("part-name", pInstrument);

//        // part-name-display?
//        analyse_optional("part-name-display", pInstrument);
//
//        // part-abbreviation?
//        pInstrument->set_abbrev(
//            analyze_optional_child_pcdata("part-abbreviation", "") );
//        //TODO: full analysis. class PartAbbrevMnxAnalyser
//
//        // part-abbreviation-display?
//        analyse_optional("part-abbreviation-display", pInstrument);
//
//        // group*
//        while (analyse_optional("group", pInstrument));
//
//        // score-instrument*
//        while (get_optional("score-instrument"))
//            m_pAnalyser->analyse_node(&m_childToAnalyse, pInstrument);
//
//        // (midi-device?, midi-instrument?)*
//        while (more_children_to_analyse())
//        {
//            m_childToAnalyse = get_child_to_analyse();
//            if (m_childToAnalyse.name() == "midi-device"
//                || m_childToAnalyse.name() == "midi-instrument")
//            {
//                move_to_next_child();
//                m_pAnalyser->analyse_node(&m_childToAnalyse, pInstrument);
//            }
//            else
//                break;
//        }
////        while (get_optional("midi-device") || get_optional("midi-instrument") )
////        {
////             m_pAnalyser->analyse_node(&m_childToAnalyse, pInstrument);
////        }

        m_pAnalyser->prepare_for_new_instrument_content();
        ImoMusicData* pMD = pInstrument->get_musicdata();

        // measure*
        while (analyse_optional("measure", pMD));

//        error_if_more_elements();

        add_to_model(pMD);

        return pInstrument;
    }

protected:

    ImoInstrument* create_instrument()
    {
        m_pAnalyser->clear_pending_relations();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrument* pInstrument = static_cast<ImoInstrument*>(
                                        ImFactory::inject(k_imo_instrument, pDoc) );
        ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, pDoc) );


        string id = generate_new_id();
        pInstrument->set_instr_id(id);

        Linker linker(pDoc);
        linker.add_child_to_model(pInstrument, pMD, pMD->get_obj_type());
        m_pAnalyser->add_score_part(id, pInstrument);
        m_pAnalyser->instrument_analysis_begin(pInstrument);
        return pInstrument;
    }

    string generate_new_id()
    {
        static int num=1;
        stringstream s;
        s << "P" << num;
        ++num;
        return s.str();
    }
};

//@--------------------------------------------------------------------------------------
//@ <part-name> = string
//@ attrb:   print-object="no"
//
class PartNameMnxAnalyser : public MnxElementAnalyser
{
public:
    PartNameMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
//        //attrb: print-object
//        string print = get_optional_string_attribute("print-object", "yes");
//        bool fVisible = (print == "yes" ? true : false);
//
//        if (fVisible)
//        {
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
//        }

        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <score>
//@<!ELEMENT score (system?,
//@    part* )>
//@<!ATTLIST score
//@    content="cwmn"
//@    profile="standard"
//@>
//
class ScoreMnxAnalyser : public MnxElementAnalyser
{
public:
    ScoreMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoContent* pContent = static_cast<ImoContent*>(
                        ImFactory::inject(k_imo_content, pDoc) );
        add_to_model(pContent);

        // attrb: content
        if (get_attribute("content") != "cwmn")
        {
            error_msg("Invalid or unsupported score type '" + get_attribute("content")
                      + "'.");
            return nullptr;
        }

        ImoScore* pScore = create_score();

		//TODO
        // system?
        get_optional("system");

        // part*
        while (analyse_mandatory("part", pScore));

        m_pAnalyser->add_all_instruments(pScore);
        return pScore;
    }

protected:

    ImoScore* create_score()
    {
        //add an empty score
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, pDoc));
        m_pAnalyser->score_analysis_begin(pScore);
        add_to_model(pScore);
        m_pAnchor = pScore;

        pScore->set_version(200);   //2.0
        set_options(pScore);
        pScore->add_required_text_styles();

        return pScore;
    }

    void set_options(ImoScore* pScore)
    {
        ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
        pOpt->set_float_value(0.35f);

        pOpt = pScore->get_option("Score.JustifyLastSystem");
        pOpt->set_long_value(3);    //justify it in any case

        pOpt = pScore->get_option("Render.SpacingOptions");
        pOpt->set_long_value(k_render_opt_breaker_optimal
                             | k_render_opt_dmin_global);
    }

};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT sequence
//@     (event | direction | tuplet)* )>
//@ <!ATTLIST sequence
//@     orientation
//@     staff
//@     name
//@ >
class SequenceMnxAnalyser : public MnxElementAnalyser
{
public:
    SequenceMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        // attrib: orientation
            //TODO

        // attrib: staff
        m_pAnalyser->set_current_staff( get_attribute_as_integer("staff", 1) - 1 );

        // attrib: name
            //TODO
        m_pAnalyser->set_current_voice( m_pAnalyser->get_current_voice() + 1 );


        // (event | direction | tuplet)*    alternatives: zero or more
        while (more_children_to_analyse())
        {
            if (analyse_optional("event", m_pAnchor)
                || analyse_optional("direction", m_pAnchor)
                || analyse_optional("tuplet", m_pAnchor)
               )
            {
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }
        return nullptr;
    }

protected:

};

//@--------------------------------------------------------------------------------------
//@ <staff>
//@ <!ELEMENT staff
//@     (clef | bbb | ccc)* )>
//@ <!ATTLIST staff
//@     mmm
//@     nnn
//@ >
//
class StaffMnxAnalyser : public MnxElementAnalyser
{
public:
    StaffMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <template_for_analyser>
//@ <!ELEMENT xxxx
//@     (aaa | bbb | ccc)* )>
//@ <!ATTLIST xxxx
//@     mmm
//@     nnn
//@ >
//
class TemplateMnxAnalyser : public MnxElementAnalyser
{
public:
    TemplateMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        return nullptr;
    }
};






//=======================================================================================
// MnxAnalyser implementation
//=======================================================================================
MnxAnalyser::MnxAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc,
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
    , m_musicxmlVersion(0)
    , m_pNodeImo(nullptr)
    , m_tieNum(0)
    , m_slurNum(0)
    , m_voltaNum(0)
    , m_pTree()
    , m_fileLocator("")
//    , m_nShowTupletBracket(k_yesno_default)
//    , m_nShowTupletNumber(k_yesno_default)
    , m_pCurScore(nullptr)
    , m_pLastNote(nullptr)
    , m_pImoDoc(nullptr)
    , m_time(0.0)
    , m_maxTime(0.0)
    , m_divisions(1.0f)
{
    //populate the name to enum conversion map
//    m_NameToEnum["accordion-registration"] = k_mnx_tag_accordion_registration;
//    m_NameToEnum["articulations"] = k_mnx_tag_articulations;
    m_NameToEnum["attributes"] = k_mnx_tag_attributes;
//    m_NameToEnum["backup"] = k_mnx_tag_backup;
//    m_NameToEnum["barline"] = k_mnx_tag_barline;
//    m_NameToEnum["bracket"] = k_mnx_tag_bracket;
//    m_NameToEnum["clef"] = k_mnx_tag_clef;
//    m_NameToEnum["coda"] = k_mnx_tag_coda;
//    m_NameToEnum["damp"] = k_mnx_tag_damp;
//    m_NameToEnum["damp-all"] = k_mnx_tag_damp_all;
//    m_NameToEnum["dashes"] = k_mnx_tag_dashes;
//    m_NameToEnum["direction"] = k_mnx_tag_direction;
//    m_NameToEnum["direction-type"] = k_mnx_tag_direction_type;
//    m_NameToEnum["dynamics"] = k_mnx_tag_dynamics;
//    m_NameToEnum["ending"] = k_mnx_tag_ending;
    m_NameToEnum["event"] = k_mnx_tag_event;
//    m_NameToEnum["eyeglasses"] = k_mnx_tag_eyeglasses;
//    m_NameToEnum["fermata"] = k_mnx_tag_fermata;
//    m_NameToEnum["forward"] = k_mnx_tag_forward;
//    m_NameToEnum["harp-pedals"] = k_mnx_tag_harp_pedals;
    m_NameToEnum["head"] = k_mnx_tag_head;
//    m_NameToEnum["image"] = k_mnx_tag_image;
//    m_NameToEnum["key"] = k_mnx_tag_key;
//    m_NameToEnum["lyric"] = k_mnx_tag_lyric;
    m_NameToEnum["measure"] = k_mnx_tag_measure;
//    m_NameToEnum["metronome"] = k_mnx_tag_metronome;
//    m_NameToEnum["midi-device"] = k_mnx_tag_midi_device;
//    m_NameToEnum["midi-instrument"] = k_mnx_tag_midi_instrument;
    m_NameToEnum["mnx"] = k_mnx_tag_mnx;
//    m_NameToEnum["notations"] = k_mnx_tag_notations;
//    m_NameToEnum["note"] = k_mnx_tag_note;
//    m_NameToEnum["octave-shift"] = k_mnx_tag_octave_shift;
//    m_NameToEnum["ornaments"] = k_mnx_tag_ornaments;
    m_NameToEnum["part"] = k_mnx_tag_part;
//    m_NameToEnum["part-group"] = k_mnx_tag_part_group;
//    m_NameToEnum["part-list"] = k_mnx_tag_part_list;
    m_NameToEnum["part-name"] = k_mnx_tag_part_name;
//    m_NameToEnum["pedal"] = k_mnx_tag_pedal;
//    m_NameToEnum["percussion"] = k_mnx_tag_percussion;
//    m_NameToEnum["pitch"] = k_mnx_tag_pitch;
//    m_NameToEnum["principal-voice"] = k_mnx_tag_principal_voice;
//    m_NameToEnum["print"] = k_mnx_tag_print;
//    m_NameToEnum["rehearsal"] = k_mnx_tag_rehearsal;
//    m_NameToEnum["scordatura"] = k_mnx_tag_scordatura;
    m_NameToEnum["score"] = k_mnx_tag_score;
//    m_NameToEnum["score-instrument"] = k_mnx_tag_score_instrument;
//    m_NameToEnum["score-part"] = k_mnx_tag_score_part;
//    m_NameToEnum["score-partwise"] = k_mnx_tag_score_partwise;
//    m_NameToEnum["segno"] = k_mnx_tag_segno;
    m_NameToEnum["sequence"] = k_mnx_tag_sequence;
//    m_NameToEnum["slur"] = k_mnx_tag_slur;
//    m_NameToEnum["sound"] = k_mnx_tag_sound;
    m_NameToEnum["staff"] = k_mnx_tag_staff;
//    m_NameToEnum["string-mute"] = k_mnx_tag_string_mute;
//    m_NameToEnum["technical"] = k_mnx_tag_technical;
//    m_NameToEnum["text"] = k_mnx_tag_text;
//    m_NameToEnum["tied"] = k_mnx_tag_tied;
//    m_NameToEnum["time"] = k_mnx_tag_time;
//    m_NameToEnum["time-modification"] = k_mnx_tag_time_modification;
//    m_NameToEnum["tuplet"] = k_mnx_tag_tuplet;
//    m_NameToEnum["tuplet-actual"] = k_mnx_tag_tuplet_actual;
//    m_NameToEnum["tuplet-normal"] = k_mnx_tag_tuplet_normal;
//    m_NameToEnum["virtual-instrument"] = k_mnx_tag_virtual_instr;
//    m_NameToEnum["wedge"] = k_mnx_tag_wedge;
//    m_NameToEnum["words"] = k_mnx_tag_words;
}

//---------------------------------------------------------------------------------------
MnxAnalyser::~MnxAnalyser()
{
    delete_relation_builders();
    m_NameToEnum.clear();
    m_lyrics.clear();
    m_lyricIndex.clear();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::delete_relation_builders()
{
    delete m_pTiesBuilder;
    delete m_pBeamsBuilder;
    delete m_pTupletsBuilder;
    delete m_pSlursBuilder;
    delete m_pVoltasBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* MnxAnalyser::analyse_tree_and_get_object(XmlNode* root)
{
    delete_relation_builders();
    m_pTiesBuilder = LOMSE_NEW MnxTiesBuilder(m_reporter, this);
    m_pBeamsBuilder = LOMSE_NEW MnxBeamsBuilder(m_reporter, this);
    m_pTupletsBuilder = LOMSE_NEW MnxTupletsBuilder(m_reporter, this);
    m_pSlursBuilder = LOMSE_NEW MnxSlursBuilder(m_reporter, this);
    m_pVoltasBuilder = LOMSE_NEW MnxVoltasBuilder(m_reporter, this);

    m_pTree = root;
    m_curStaff = 0;
    m_curVoice = 1;
    return analyse_node(root);
}

//---------------------------------------------------------------------------------------
InternalModel* MnxAnalyser::analyse_tree(XmlNode* tree, const string& locator)
{
    m_fileLocator = locator;
    ImoObj* pRoot = analyse_tree_and_get_object(tree);
    return LOMSE_NEW InternalModel( pRoot );
}

//---------------------------------------------------------------------------------------
ImoObj* MnxAnalyser::analyse_node(XmlNode* pNode, ImoObj* pAnchor)
{
    //m_reporter << "DBG. Analysing node: " << pNode->name() << endl;
    MnxElementAnalyser* a = new_analyser( pNode->name(), pAnchor );
    ImoObj* pImo = a->analyse_node(pNode);
    delete a;
    return pImo;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_line_number(XmlNode* node)
{
    return m_pParser->get_line_number(node);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::prepare_for_new_instrument_content()
{
    clear_pending_relations();
    m_time = 0.0;
    m_maxTime = 0.0;
    save_last_barline(nullptr);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::add_relation_info(ImoObj* pDto)
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
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::clear_pending_relations()
{
    m_pTiesBuilder->clear_pending_items();
    m_pSlursBuilder->clear_pending_items();
    m_pBeamsBuilder->clear_pending_items();
    m_pTupletsBuilder->clear_pending_items();
    m_pVoltasBuilder->clear_pending_items();

    m_lyrics.clear();
    m_lyricIndex.clear();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::add_lyrics_data(ImoNote* pNote, ImoLyric* pLyric)
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
void MnxAnalyser::add_marging_space_for_lyrics(ImoNote* pNote, ImoLyric* pLyric)
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
ImoInstrGroup* MnxAnalyser::start_part_group(int number)
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
void MnxAnalyser::terminate_part_group(int number)
{
    ImoInstrGroup* pGrp = m_partGroups.get_group(number);
    if (pGrp)
        m_partGroups.terminate_group(number);
}

//---------------------------------------------------------------------------------------
ImoInstrGroup* MnxAnalyser::get_part_group(int number)
{
    return m_partGroups.get_group(number);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::check_if_all_groups_are_closed()
{
    m_partGroups.check_if_all_groups_are_closed(m_reporter);
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::new_tie_id(int UNUSED(numTie), FPitch fp)
{
    m_tieIds[int(fp)] = ++m_tieNum;
    return m_tieNum;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_tie_id(int UNUSED(numTie), FPitch fp)
{
    return m_tieIds[int(fp)];
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_tie_id_and_close(int UNUSED(numTie), FPitch fp)
{
    return m_tieIds[int(fp)];
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::new_slur_id(int numSlur)
{
    m_slurIds[numSlur] = ++m_slurNum;
    return m_slurNum;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_slur_id(int numSlur)
{
    return m_slurIds[numSlur];
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_slur_id_and_close(int numSlur)
{
    return m_slurIds[numSlur];
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::new_volta_id()
{
    return ++m_voltaNum;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_volta_id()
{
    return m_voltaNum;
}

//---------------------------------------------------------------------------------------
TimeUnits MnxAnalyser::duration_to_timepos(int duration)
{
    //AWARE: 'divisions' indicates how many divisions per quarter note
    //       and 'duration' is expressed in 'divisions'
    float LdpTimeUnitsPerDivision = k_duration_quarter / m_divisions;
    return TimeUnits( float(duration) * LdpTimeUnitsPerDivision);
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::set_musicxml_version(const string& version)
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
string MnxAnalyser::get_element_info()
{
    stringstream ss;
    ss << "Part '" << m_curPartId << "', measure '" << m_curMeasureNum << "'. ";
    return ss.str();
}

//---------------------------------------------------------------------------------------
bool MnxAnalyser::to_integer(const string& text, int* pResult)
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
MnxElementAnalyser* MnxAnalyser::new_analyser(const string& name, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch ( name_to_enum(name) )
    {
        case k_mnx_tag_attributes:          return LOMSE_NEW AttributesMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_event:               return LOMSE_NEW EventMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_head:                return LOMSE_NEW HeadMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_measure:             return LOMSE_NEW MeasureMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_mnx:                 return LOMSE_NEW MnxMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_part:                return LOMSE_NEW PartMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_part_name:           return LOMSE_NEW PartNameMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_score:               return LOMSE_NEW ScoreMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_sequence:            return LOMSE_NEW SequenceMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_staff:               return LOMSE_NEW StaffMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        default:
            return LOMSE_NEW NullMnxAnalyser(this, m_reporter, m_libraryScope, name);
    }
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::name_to_enum(const string& name) const
{
	map<string, int>::const_iterator it = m_NameToEnum.find(name);
	if (it != m_NameToEnum.end())
		return it->second;
    else
        return k_mnx_tag_undefined;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_voice_for_name(const string& name) const
{
	map<string, int>::const_iterator it = m_nameToVoice.find(name);
	if (it != m_nameToVoice.end())
		return it->second;
    else
        return -1;
}


//---------------------------------------------------------------------------------------
// static public methods
//---------------------------------------------------------------------------------------

bool MnxAnalyser::pitch_to_components(const string& pitch, int *step, int* octave,
                                      EAccidentals* accidentals, float* alterations)
{
    // Analyzes string pitch (MNX format), extracts its parts (step, octave, accidentals
    // and alterations) and stores them in the corresponding parameters.
    // Returns true if error (pitch is not a valid pitch string)
    //
    // In MNX pitch is represented as a combination of the step of the diatonic
    // scale, followed optionally by 0..2 occurrences of '#' or 'b' representing
    // an integer alteration, followed by the octave number. An additional,
    // non-integer alteration may be added to the preceding integral amount by
    // including the suffix '+' or '-', followed by a real-valued number of semitones.
    //
    // Examples:
    //     C4       - Middle C
    //     C#4      - The pitch one semitone above middle C
    //     Db4      - The pitch one semitone above middle C (identical to the above)
    //     C4+0.5   - The pitch one quarter-tone above middle C
    //     B3+1.5   - The pitch one quarter-tone above middle C (identical to the above)
    //     C#4-0.5  - The pitch one quarter-tone above middle C (identical to the above)

    size_t iMax = pitch.length();
    if (iMax < 2)
        return true;   //error

    // step
    *step = to_step(pitch[0]);
    if (*step == -1)
        return true;   //error

    //0..2 accidentals
    unsigned int i = 1;
    while(pitch[i] == '#' || pitch[i] == 'b')
        ++i;
    if (i == 1)
        *accidentals = k_no_accidentals;
    else
    {
        *accidentals = to_accidentals(pitch.substr(1, i-1));
        if (*accidentals == k_invalid_accidentals)
            return true;   //error
    }

    //octave
    *octave = to_octave(pitch[i]);
    if (*octave == -1)
        return true;   //error

    //optional: non-integer alteration
    if (++i == iMax)
        *alterations = 0.0f;
    else
    {
        if (pitch[i] == '+' || pitch[i] == '-')
        {
            try
            {
                size_t sz;
                *alterations = std::stof(pitch.substr(i), &sz);
                if (i+sz != iMax)
                    return true;   //error
            }
            catch (const std::invalid_argument& ia)
            {
                return true;   //error
            }
        }
        else
            return true;   //error
    }

    return false;  //no error
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::to_step(const char& letter)
{
	switch (letter)
    {
		case 'A':	return k_step_A;
		case 'B':	return k_step_B;
		case 'C':	return k_step_C;
		case 'D':	return k_step_D;
		case 'E':	return k_step_E;
		case 'F':	return k_step_F;
		case 'G':	return k_step_G;
	}
	return -1;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::to_octave(const char& letter)
{
	switch (letter)
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
	}
	return -1;
}

//---------------------------------------------------------------------------------------
EAccidentals MnxAnalyser::to_accidentals(const std::string& accidentals)
{
    switch (accidentals.length())
    {
        case 0:
            return k_no_accidentals;
            break;

        case 1:
            if (accidentals[0] == '#')
                return k_sharp;
            else if (accidentals[0] == 'b')
                return k_flat;
            else
                return k_invalid_accidentals;
            break;

        case 2:
            if (accidentals.compare(0, 2, "##") == 0)
                return k_sharp_sharp;
            else if (accidentals.compare(0, 2, "bb") == 0)
                return k_flat_flat;
            else
                return k_invalid_accidentals;
            break;

        default:
            return k_invalid_accidentals;
    }
}


//=======================================================================================
// MnxTiesBuilder implementation
//=======================================================================================
void MnxTiesBuilder::add_relation_to_staffobjs(ImoTieDto* pEndDto)
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
bool MnxTiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
{
    return (pStartNote->get_voice() == pEndNote->get_voice())
            && (pStartNote->get_staff() == pEndNote->get_staff())
            && (pStartNote->get_actual_accidentals() == pEndNote->get_actual_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

//---------------------------------------------------------------------------------------
void MnxTiesBuilder::tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto)
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
void MnxTiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". Requesting to tie notes of different voice or pitch. Tie number "
               << pEndInfo->get_tie_number()
               << " will be ignored." << endl;
}


//=======================================================================================
// MnxSlursBuilder implementation
//=======================================================================================
void MnxSlursBuilder::add_relation_to_staffobjs(ImoSlurDto* pEndInfo)
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
// MnxBeamsBuilder implementation
//=======================================================================================
void MnxBeamsBuilder::add_relation_to_staffobjs(ImoBeamDto* pEndInfo)
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
// MnxTupletsBuilder implementation
//=======================================================================================
void MnxTupletsBuilder::add_relation_to_staffobjs(ImoTupletDto* UNUSED(pEndDto))
{
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
void MnxTupletsBuilder::add_to_open_tuplets(ImoNoteRest* pNR)
{
    if (m_pendingItems.size() > 0)
    {
        ListIterator it;
        for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
        {
            if ((*it)->is_start_of_relation() )
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
void MnxTupletsBuilder::get_factors_from_nested_tuplets(int* pTop, int* pBottom)
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
// MnxVoltasBuilder implementation
//=======================================================================================
void MnxVoltasBuilder::add_relation_to_staffobjs(ImoVoltaBracketDto* pEndDto)
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

    std::list<ImoVoltaBracketDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoBarline* pBarline = (*it)->get_barline();
        pBarline->include_in_relation(pDoc, pVB, nullptr);
    }
}


}   //namespace lomse
