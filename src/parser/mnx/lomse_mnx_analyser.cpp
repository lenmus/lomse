//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
#include "private/lomse_document_p.h"
#include "lomse_image_reader.h"
#include "lomse_score_player_ctrl.h"
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_time.h"
#include "lomse_autobeamer.h"
#include "lomse_im_measures_table.h"
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
// Helper classes to store values returned by analysers
//=======================================================================================
//---------------------------------------------------------------------------------------
//base class
class AnalysisData
{
protected:
    int m_type = 0;

    AnalysisData(int type) : m_type(type) {}

public:

    int get_type() { return m_type; }

    enum {
        k_data_undefined = 0,
        k_data_imo,
        k_data_event,
        k_data_jump,
    };

};

//---------------------------------------------------------------------------------------
//for ImoObj analysis
class ImoData : public AnalysisData
{
public:
    ImoObj* pImo;

    ImoData(ImoObj* pObj) : AnalysisData(AnalysisData::k_data_imo), pImo(pObj) {}
};

//---------------------------------------------------------------------------------------
//for <events> analysis
class EventData : public AnalysisData
{
public:
    bool fMeasure;      //it is a full measure event
    ImoNoteRest* pNR;   //single, note, rest or chord base note

    EventData(bool value, ImoNoteRest* nr)
        : AnalysisData(AnalysisData::k_data_event)
        , fMeasure(value)
        , pNR(nr)
    {
    }
};

//---------------------------------------------------------------------------------------
//for <jump> analysis
class JumpData : public AnalysisData
{
public:
    ImoDirection* pDirection;
    ImoSoundChange* pSC;

    JumpData(ImoDirection* dir, ImoSoundChange* sc)
        : AnalysisData(AnalysisData::k_data_jump)
        , pDirection(dir)
        , pSC(sc)
    {
    }
};

//=======================================================================================
// Helper class to manage analysers's results.
class AnalysisResult
{
protected:
    AnalysisData* m_pResult = nullptr;

public:
    AnalysisResult() { set_result(nullptr); }

    //-----------------------------------------------------------------------------------
    void set_result(AnalysisData* pResult)
    {
        delete m_pResult;
        m_pResult = pResult;
    }

    //-----------------------------------------------------------------------------------
    void delete_result()
    {
        delete m_pResult;
        m_pResult = nullptr;
    }

    //-----------------------------------------------------------------------------------
    std::unique_ptr<AnalysisData> get_result()
    {
        AnalysisData* data = m_pResult;
        m_pResult = nullptr;
        return std::unique_ptr<AnalysisData>(data);
    }

    //-----------------------------------------------------------------------------------
    std::unique_ptr<ImoData> get_imo_result()
    {
        AnalysisData* data = m_pResult;
        m_pResult = nullptr;
        if (data && data->get_type() == AnalysisData::k_data_imo)
            return std::unique_ptr<ImoData>( static_cast<ImoData*>(data) );

        return std::unique_ptr<ImoData>(nullptr);
    }

    //-----------------------------------------------------------------------------------
    std::unique_ptr<JumpData> get_jump_result()
    {
        AnalysisData* data = m_pResult;
        m_pResult = nullptr;
        if (data && data->get_type() == AnalysisData::k_data_jump)
            return std::unique_ptr<JumpData>( static_cast<JumpData*>(data) );

        return std::unique_ptr<JumpData>(nullptr);
    }

    //-----------------------------------------------------------------------------------
    std::unique_ptr<EventData> get_event_result()
    {
        AnalysisData* data = m_pResult;
        m_pResult = nullptr;
        if (data && data->get_type() == AnalysisData::k_data_event)
            return std::unique_ptr<EventData>( static_cast<EventData*>(data) );

        return std::unique_ptr<EventData>(nullptr);
    }
};


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
}

//---------------------------------------------------------------------------------------
int MnxPartList::add_score_part(const string& id, ImoInstrument* pInstrument)
{
    int iInstr = m_numInstrs;
    m_locators[id] = m_numInstrs++;
    m_instruments.push_back(pInstrument);
    m_partAdded.push_back(false);
    return iInstr;
}

//---------------------------------------------------------------------------------------
bool MnxPartList::mark_part_as_added(const string& id)
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

    if (m_numInstrs < 1)        //sanity check, when invalid MNX format
        pScore->add_instrument();   //add empty instrument
    else
    {
        for (int i=0; i < m_numInstrs; ++i)
            pScore->add_instrument(m_instruments[i]);
    }
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
void MnxPartGroups::add_instrument_to_groups(int iInstr)
{
    map<int, ImoInstrGroup*>::const_iterator it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        ImoInstrGroup* pGrp = it->second;
        pGrp->add_instrument(iInstr);
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


//=======================================================================================
// Enum to assign a int to each valid MNX element
enum EMnxTag
{
    k_mnx_tag_undefined = -1,

//    k_mnx_tag_accordion_registration,
//    k_mnx_tag_articulations,
//    k_mnx_tag_backup,
//    k_mnx_tag_barline,
    k_mnx_tag_beam,
    k_mnx_tag_beam_hook,
    k_mnx_tag_beams,
//    k_mnx_tag_bracket,
    k_mnx_tag_clef,
//    k_mnx_tag_coda,
//    k_mnx_tag_damp,
//    k_mnx_tag_damp_all,
//    k_mnx_tag_dashes,
//    k_mnx_tag_direction,
    k_mnx_tag_directions,
//    k_mnx_tag_direction_type,
    k_mnx_tag_dynamics,
//    k_mnx_tag_ending,
    k_mnx_tag_event,
    k_mnx_tag_expression,
//    k_mnx_tag_eyeglasses,
//    k_mnx_tag_fermata,
//    k_mnx_tag_forward,
    k_mnx_tag_fine,
    k_mnx_tag_global,
    k_mnx_tag_grace,
//    k_mnx_tag_harp_pedals,
    k_mnx_tag_head,
//    k_mnx_tag_image,
    k_mnx_tag_instrument_sound,
    k_mnx_tag_jump,
    k_mnx_tag_key,
//    k_mnx_tag_lyric,
    k_mnx_tag_measure,
//    k_mnx_tag_metronome,
//    k_mnx_tag_midi_device,
//    k_mnx_tag_midi_instrument,
    k_mnx_tag_mnx,
//    k_mnx_tag_notations,
    k_mnx_tag_note,
    k_mnx_tag_octave_shift,
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
    k_mnx_tag_repeat,
    k_mnx_tag_rest,
//    k_mnx_tag_scordatura,
    k_mnx_tag_score,
//    k_mnx_tag_score_instrument,
//    k_mnx_tag_score_part,
//    k_mnx_tag_score_partwise,
    k_mnx_tag_segno,
    k_mnx_tag_sequence,
    k_mnx_tag_sequence_content,
//    k_mnx_tag_slur,
//    k_mnx_tag_sound,
    k_mnx_tag_staff,
//    k_mnx_tag_string_mute,
//    k_mnx_tag_technical,
//    k_mnx_tag_text,
    k_mnx_tag_tied,
    k_mnx_tag_time,
//    k_mnx_tag_time_modification,
    k_mnx_tag_tuplet,
//    k_mnx_tag_tuplet_actual,
//    k_mnx_tag_tuplet_normal,
//    k_mnx_tag_virtual_instr,
    k_mnx_tag_wedge,
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
    ImoObj* m_pAnchor;

public:
    MnxElementAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                       LibraryScope& libraryScope, ImoObj* pAnchor=nullptr)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_libraryScope(libraryScope)
        , m_pAnchor(pAnchor) {}
    virtual ~MnxElementAnalyser() {}
    bool analyse_node(XmlNode* pNode);

protected:

    //analysis
    virtual bool do_analysis() = 0;

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
    inline bool analyse_child(ImoObj* pAnchor=nullptr) {
        return m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
    }

    // 'get' methods just update m_childToAnalyse to point to the next node to analyse
    bool get_mandatory(const string& tag);
    bool get_optional(const string& type);

    // 'analyse' methods do a 'get' and, if found, analyse the found element
    bool analyse_mandatory(const string& tag, ImoObj* pAnchor=nullptr);
    bool analyse_optional(const string& name, ImoObj* pAnchor=nullptr);
    bool analyse_content(const string& tag, ImoObj* pAnchor=nullptr);
    string analyze_mandatory_child_pcdata(const string& name);
    string analyze_optional_child_pcdata(const string& name, const string& sDefault);
    int analyze_optional_child_pcdata_int(const string& name,
                                          int nMin, int nMax, int nDefault);
    float analyze_optional_child_pcdata_float(const string& name,
                                              float rMin, float rMax, float rDefault);

    //results
    void set_result(AnalysisData* pData) { m_pAnalyser->set_result(pData); }
    void delete_result() { m_pAnalyser->delete_result(); }

    std::unique_ptr<AnalysisData> get_result() { return m_pAnalyser->get_result(); }
    std::unique_ptr<ImoData> get_imo_result() { return m_pAnalyser->get_imo_result(); }
    std::unique_ptr<JumpData> get_jump_result() { return m_pAnalyser->get_jump_result(); }
    std::unique_ptr<EventData> get_event_result() { return m_pAnalyser->get_event_result(); }

    //very common operations
    void analyse_and_set_xml_id(ImoObj* pObj);

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
    inline const string& get_document_locator() { return m_pAnalyser->get_document_locator(); }
    int get_line_number() { return m_pAnalyser->get_line_number(&m_analysedNode); }
    ImoNoteRest* get_noterest(const std::string& xmlId, const std::string& element);




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
    // Helper, to check and cast anchor object
    //-----------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------
    ImoDirection* get_anchor_as_direction()
    {
        if (m_pAnchor && m_pAnchor->is_direction())
            return static_cast<ImoDirection*>(m_pAnchor);

        const string msg("pAnchor is nullptr or it is not direction");
        LOMSE_LOG_ERROR(msg);
        error_msg(msg);
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    ImoMusicData* get_anchor_as_music_data()
    {
        if (m_pAnchor && m_pAnchor->is_music_data())
            return static_cast<ImoMusicData*>(m_pAnchor);

        const string msg("pAnchor is nullptr or it is not musicData");
        LOMSE_LOG_ERROR(msg);
        error_msg(msg);
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    ImoNote* get_anchor_as_note()
    {
        if (m_pAnchor && m_pAnchor->is_note())
            return static_cast<ImoNote*>(m_pAnchor);

        const string msg("pAnchor is nullptr or it is not note");
        LOMSE_LOG_ERROR(msg);
        error_msg(msg);
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    ImoScore* get_anchor_as_score()
    {
        if (m_pAnchor && m_pAnchor->is_score())
            return static_cast<ImoScore*>(m_pAnchor);

        const string msg("pAnchor is nullptr or it is not ImoScore");
        LOMSE_LOG_ERROR(msg);
        error_msg(msg);
        return nullptr;
    }
};



//=======================================================================================
// MnxElementAnalyser implementation
//=======================================================================================
bool MnxElementAnalyser::analyse_node(XmlNode* pNode)
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
        return int(nNumber);
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
bool MnxElementAnalyser::analyse_mandatory(const string& tag, ImoObj* pAnchor)
{
    if (get_mandatory(tag))
        return m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
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
    //TODO: analyse_optional() has a problem: if the optional element exists
    //      but its analysis fails due to errors, analyse_optional() returns
    //      false as if the element didn't exist.
    if (get_optional(name))
    {
        return m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool MnxElementAnalyser::analyse_content(const string& tag, ImoObj* pAnchor)
{
    MnxElementAnalyser* a = m_pAnalyser->new_analyser(tag, pAnchor);
    bool ret = a->analyse_node(&m_analysedNode);
    delete a;
    return ret;
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
        return int(nNumber);
}

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
ImoNoteRest* MnxElementAnalyser::get_noterest(const string& xmlId, const string& element)
{
    Document* pDoc = m_pAnalyser->get_document_being_analysed();
    ImoObj* pImo = pDoc->get_pointer_to_imo(xmlId);
    if (pImo)
    {
        if (pImo->is_note_rest())
            return static_cast<ImoNoteRest*>(pImo);
        else
            error_msg("<" + element + ">: @xml:id '" + xmlId
                + "' is not a note/rest but a " + pImo->get_name() );
    }
    return nullptr;
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
void MnxElementAnalyser::analyse_and_set_xml_id(ImoObj* pImo)
{
    string id = get_optional_string_attribute("id", "");
    if (id.empty() || pImo == nullptr)
        return;

    pImo->set_xml_id(id);
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
        return MnxAnalyser::get_note_value(value, noteType, dots);
    }
    else
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
//@ - as hexadecimal RGB triples, as in HTML (e.g., "#800080" purple), or
//@ - as hexadecimal ARGB tuples (e.g., "#40800080" transparent purple).
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

    bool do_analysis() override
    {
        error_msg("Missing analyser for element '" + m_tag + "'. Node ignored.");
        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <beam>
//@ Parent elements: <beam>, <beams>
//@ Context:
//@    In any order: <beam>*, <beam-hook>*
//@ Attributes:
//@    events - mandatory. A space-separated list of event IDs for events that comprise
//@             this beam — in order by their position in the beam.
//@
class BeamMnxAnalyser : public MnxElementAnalyser
{
public:
    BeamMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: events
        string events = get_mandatory_string_attribute("events", "", "beam");
        process_beam(events);

        while (more_children_to_analyse())
        {
            if (get_optional("beam"))
            {
                m_pAnalyser->increment_beam_level();
                analyse_child();
                m_pAnalyser->decrement_beam_level();
            }
            else if (get_optional("beam-hook"))
                analyse_child();
            else
                break;
        }

        set_result(nullptr);
        return true;    //success
    }

protected:

    //-----------------------------------------------------------------------------------
    void process_beam(const string& events)
    {
        //extract ids
        vector<string> xmlId;
        istringstream iss(events);
        string id;
        while (getline(iss, id, ' '))
        {
            if (!id.empty())
                xmlId.push_back(id);
        }

        //get referenced note/rests
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        vector<ImoNoteRest*> notes;
        for (size_t i=0; i < xmlId.size(); ++i)
        {
            ImoObj* pImo = pDoc->get_pointer_to_imo(xmlId[i]);
            if (pImo)
            {
                if (pImo->is_note_rest())
                    notes.push_back( static_cast<ImoNoteRest*>(pImo) );
                else
                    error_msg("<beam>: @xml:id '" + xmlId[i]
                        + "' is not a note/rest but a " + pImo->get_name() );
            }
            else
                error_msg("<beam>: No object found with @xml:id = '" + xmlId[i] + "'");
        }

        //create the beam
        if (notes.size() > 1)
        {
            if (m_pAnalyser->get_beam_level() == 0)
                create_beam(notes);
            else
                add_secondary_beam(notes);
        }
        else
        {
            stringstream ss;
            ss << "<beam>: Less than two note/rests in <beam> (found " << notes.size()
               << "). <beam> ignored.";
            error_msg(ss.str());
        }
    }

    //-----------------------------------------------------------------------------------
    void create_beam(vector<ImoNoteRest*>& notes)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBeam* pBeam = static_cast<ImoBeam*>(ImFactory::inject(k_imo_beam, pDoc));

//        bool fErrors = false;
        size_t lastNR = notes.size() - 1;
        for (size_t i=0; i <= lastNR; ++i)
        {
            //determine beam type
            int type = ImoBeam::k_continue;
            if (i == 0)
                type = ImoBeam::k_begin;
            else if (i == lastNR)
                type = ImoBeam::k_end;

            ImoNoteRest* pNR = notes[i];
            ImoBeamData* pData = static_cast<ImoBeamData*>(
                                        ImFactory::inject(k_imo_beam_data, pDoc) );
            pData->set_beam_type(0, type);
            pNR->include_in_relation(pDoc, pBeam, pData);

//            //check if beam is congruent with note type
//            int level = 0;
//            for (int i=0; i < 6; ++i)
//            {
//                if ((*it)->get_beam_type(i) == ImoBeam::k_none)
//                    break;
//                ++level;
//            }
//            int type = pNR->get_note_type();
//            switch(level)
//            {
//                case 0: fErrors = true;                 break;
//                case 1: fErrors |= (type != k_eighth);  break;
//                case 2: fErrors |= (type != k_16th);    break;
//                case 3: fErrors |= (type != k_32nd);    break;
//                case 4: fErrors |= (type != k_64th);    break;
//                case 5: fErrors |= (type != k_128th);   break;
//                case 6: fErrors |= (type != k_256th);   break;
//            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_secondary_beam(vector<ImoNoteRest*>& notes)
    {
//        bool fErrors = false;
        int level = m_pAnalyser->get_beam_level();
        size_t lastNR = notes.size() - 1;
        for (size_t i=0; i <= lastNR; ++i)
        {
            //determine beam type
            int type = ImoBeam::k_continue;
            if (i == 0)
                type = ImoBeam::k_begin;
            else if (i == lastNR)
                type = ImoBeam::k_end;

            ImoNoteRest* pNR = notes[i];
            ImoBeam* pBeam = pNR->get_beam();
            ImoBeamData* pData = static_cast<ImoBeamData*>( pBeam->get_data_for(pNR) );
            pData->set_beam_type(level, type);
        }
        //TODO: validation
    }

};

//@--------------------------------------------------------------------------------------
//@ <beam-hook>
//@ Parent element: <beam>
//@ Context
//@     Always empty.
//@ Attributes
//@     event - mandatory. event ID
//@     direction - optional. Beam hook direction
//@
class BeamHookMnxAnalyser : public MnxElementAnalyser
{
public:
    BeamHookMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        string xmlId = get_mandatory_string_attribute("event", "", "beam-hook");
        string direction = get_mandatory_string_attribute("direction", "", "beam-hook");
//        cout << "process_beam_hook(). id='" << xmlId << "', dir=" << direction << endl;

        int level = m_pAnalyser->get_beam_level() + 1;

        //determine beam type
        int type = ImoBeam::k_continue;
        if (direction == "right")
            type = ImoBeam::k_forward;
        else if (direction == "left")
            type = ImoBeam::k_backward;
        else
        {
            //TODO
            set_result(nullptr);
            return false;    //failure
        }

        ImoNoteRest* pNR = get_noterest(xmlId, "beam-hook");
        ImoBeam* pBeam = pNR->get_beam();
        ImoBeamData* pData = static_cast<ImoBeamData*>( pBeam->get_data_for(pNR) );
        pData->set_beam_type(level, type);

        set_result(nullptr);
        return true;    //success
    }

};

//@--------------------------------------------------------------------------------------
//@ <beams>
//@ Contexts:
//@    <sequence>
//@ Context Model:
//@    One or more <beam> elements
//@ Attributes:
//@    None.
//@
class BeamsMnxAnalyser : public MnxElementAnalyser
{
public:
    BeamsMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //beam*
        m_pAnalyser->reset_beam_level();
        while (analyse_optional("beam", nullptr))
        {
            m_pAnalyser->reset_beam_level();
        }

        set_result(nullptr);
        return true;    //success
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
//Attributes:
//@     line, sign, staff

class ClefMnxAnalyser : public MnxElementAnalyser
{
protected:
    string m_sign;
    int m_line;
    int m_octaveChange;

public:
    ClefMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_line(0), m_octaveChange(0)
        {}


    bool do_analysis() override
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoClef* pClef = static_cast<ImoClef*>( ImFactory::inject(k_imo_clef, pDoc) );

        //attrib: staff
        int staff = get_optional_int_attribute("staff", m_pAnalyser->get_current_staff()+1);
        pClef->set_staff(staff - 1);

        //attrib: sign
        m_sign = get_optional_string_attribute("sign", "G");

        //attrib: line
        m_line = get_optional_int_attribute("line", 0);

        //attrib: %print-style;
        //get_attributes_for_print_style(pClef);

        //attrib: %print-object;
        //TODO

//        //TODO: Remove this debug code
//        stringstream s;
//        s << "Debug: processing clef " << m_sign << " in line " << m_line
//          << " staff " << staff;
//        error_msg(s.str());


            //content


//        // clef-octave-change?      <!ELEMENT clef-octave-change (#PCDATA)>
//        if (get_optional("clef-octave-change"))
//            m_octaveChange = get_child_value_integer(0);

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

        set_result( LOMSE_NEW ImoData(pClef) );
        return true;    //success
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
            stringstream s;
            s << "Warning: F clef only supported in lines 3, 4 or 5. Clef F in line "
              << m_line << "changed to F in line 4.";
            error_msg(s.str());
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
                stringstream s;
                s << "Warning: G clef only supported in lines 1 or 2. Clef G in line "
                  << m_line << "changed to G in line 2.";
                error_msg(s.str());
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
                stringstream s;
                s << "Warning: F clef only supported in lines 3, 4 or 5. Clef F in line "
                  << m_line << "changed to F in line 4.";
                error_msg(s.str());
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
//@ <collection>
//@ Contexts:
//@    <mnx>,<collection>
//@ Context Model:
//@    Any combination of <collection> and <score> elements.
//@ Attributes:
//@    type - The type of the collection
//
class CollectionMnxAnalyser : public MnxElementAnalyser
{
protected:
    const string m_tag;

public:
    CollectionMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                          LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
		//TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <directions>
//@
//@ Attributes
//@    None.
//@
//Scope for contained directions:
//    Directions in a measure within the global element apply to:
//      - all sequences of the measure in all parts.
//      - Directions with an orientation of up appear above the first displayed part;
//      - those with an orientation of down below the last displayed part.
//
//    Directions in a measure within a part element apply to:
//      - all sequences of the measure in a given part.
//
//    Directions in a sequence apply to the sequence in which it occurs.

class DirectionsMnxAnalyser : public MnxElementAnalyser
{
public:
    DirectionsMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                          LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        {}


    bool do_analysis() override
    {
        if (m_pAnalyser->get_parent_for_directions() == "measure-global")
            return do_analysis_for_measure_global();

        else if (m_pAnalyser->get_parent_for_directions() == "measure-part")
            return do_analysis_for_measure_part();

        else if (m_pAnalyser->get_parent_for_directions() == "sequence")
            return do_analysis_for_sequence();

        else
        {
            cout << "Directions parent is '" << m_pAnalyser->get_parent_for_directions()
                << "'" << endl;
            cout << "cur.node. Name=" << m_childToAnalyse.name() << ", value="
                << m_childToAnalyse.value() << endl;
        }

        set_result(nullptr);
        return false;    //failure
    }

protected:

    //-----------------------------------------------------------------------------------
    bool do_analysis_for_measure_global()
    {
        //@ Parent element: <measure> (global)
        //@     Content: In any order:  (time | repeat | ending | segno | jump | fine | key)
        //@         <time> (Optional)
        //@         <repeat> (0 to 2 times)
        //@         <ending> (0 to 2 times)
        //@         <segno> (Optional)
        //@         <jump> (Optional)
        //@         <fine> (Optional)
        //@         <key> (Optional)

        //In MNX Clefs, time signatures and key signatures are
        //treated as attributes of a measure, not as objects and, therefore, ordering
        //is not important for MNX and this information is
        //coded bad order (first key signatures, then time signatures, then clefs).
        //As Lomse expects that these objects are defined in correct order,
        //objects creation will be delayed until all attributes are parsed.
        vector<ImoObj*> times;
        vector<ImoObj*> keys;
        vector<ImoObj*> clefs;
        vector<ImoObj*> other;

        while (more_children_to_analyse())
        {
            if (analyse_optional("clef"))
            {
                clefs.push_back( static_cast<ImoClef*>(get_imo_result()->pImo) );
            }
            else if (analyse_optional("key"))
            {
                keys.push_back( static_cast<ImoKeySignature*>(get_imo_result()->pImo) );
            }
            else if (analyse_optional("time"))
            {
                times.push_back( static_cast<ImoTimeSignature*>(get_imo_result()->pImo) );
            }
            else if (analyse_optional("repeat"))
            {
                //TODO:
                // Defines the two dots in barlines for repetions
                // repeat type='start' creates the two dots in left barline, and
                // type='end' creates the two dots in right barline
                // Thus, <repeat> (0 to 2 times)
            }
            else if (get_optional("ending"))
            {
                //TODO
//                if (m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr))
//                    other.push_back( static_cast<ImoObj*>(get_result()) );
            }
            else if (analyse_optional("segno"))
            {
                //Defines a 'segno' symbol and a point for playback jumps
                other.push_back( static_cast<ImoObj*>(get_imo_result()->pImo) );
            }
            else if (analyse_optional("jump"))
            {
                std::unique_ptr<JumpData> result = get_jump_result();
                if (result->pDirection)
                    other.push_back(result->pDirection);
                if (result->pSC)
                    other.push_back(result->pSC);
            }
            else if (analyse_optional("fine"))
            {
                //Defines a 'Fine' symbol and a point for playback jumps
                other.push_back( static_cast<ImoObj*>(get_imo_result()->pImo) );
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
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
        for (it = other.begin(); it != other.end(); ++it)
        {
            if (*it)
                add_to_model(*it);
        }

        set_result(nullptr);
        return true;    //success
    }

    //-----------------------------------------------------------------------------------
    bool do_analysis_for_measure_part()
    {
        //@ Parent element: <measure> (part)
        //@     Content: In any order
        //@         <clef> (Optional)
        //@         <key> (Optional)
        //@         <staves> (Optional)  <-- //TODO: No está claro dónde va

        int staves = 0;
        while (more_children_to_analyse())
        {
            if (get_optional("staves"))
            {
                if (has_attribute(&m_childToAnalyse, "number"))
                {
                    string number = m_childToAnalyse.attribute_value("number");
                    long nNumber;
                    std::istringstream iss(number);
                    if ((iss >> std::dec >> nNumber).fail())
                    {
                        //TODO: report error
                    }
                    else
                        staves = nNumber;
                }
                else
                {
                    //TODO: report error
                }
            }
            else if (analyse_optional("clef"))
            {
                std::unique_ptr<ImoData> result = get_imo_result();
                add_to_model( static_cast<ImoClef*>(result->pImo) );
            }
            else if (analyse_optional("key"))
            {
                std::unique_ptr<ImoData> result = get_imo_result();
                add_to_model( static_cast<ImoKeySignature*>(result->pImo) );
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        //set staves
        if (staves > 1)
        {
            ImoInstrument* pInstr = m_pAnalyser->get_instrument_being_analysed();
            // coverity[tainted_data]
            for(; staves > 1; --staves)
                pInstr->add_staff();
        }

        set_result(nullptr);
        return true;    //success
    }

    //-----------------------------------------------------------------------------------
    bool do_analysis_for_sequence()
    {
        //@ Parent element: <sequence>
        //@     Content: In any order
        //@         <octave-shift> (Zero or more times)
        //@         <dynamics> (Zero or more times)
        //@         <instruction> (Zero or more times)
        //@         <expression> (Zero or more times)
        //@         <wedge> (Zero or more times)
        //@         <cresc> (Zero or more times)
        //@         <dim> (Zero or more times)

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDirection* pDirection = static_cast<ImoDirection*>(
                                        ImFactory::inject(k_imo_direction, pDoc) );

        while (more_children_to_analyse())
        {
            if (analyse_optional("octave-shift", pDirection))
            {
                //TODO
            }
            else if (analyse_optional("dynamics"))
            {
                //TODO
            }
            else if (analyse_optional("instruction"))
            {
                //TODO
            }
            else if (analyse_optional("expression"))
            {
                //TODO
            }
            else if (analyse_optional("wedge"))
            {
                //TODO
            }
            else if (analyse_optional("cresc"))
            {
                //TODO
            }
            else if (analyse_optional("dim"))
            {
                //TODO
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        set_result(nullptr);
        return true;    //success
    }

    //-----------------------------------------------------------------------------------
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
//@ <dynamics>
//@ Contexts:
//@    x
//@ Context Model:
//@    y
//@ Attributes:
//@    z
//
class DynamicsMnxAnalyser : public MnxElementAnalyser
{
public:
    DynamicsMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
    }
};

//---------------------------------------------------------------------------------------
//@ <event>
//@ Parent elements: <grace>, <sequence>, <tuplet>
//@ Context Model:
//@    Metadata content
//@    content, in any order: (note* | rest? | slur*)
//@    Event content: (articulations | lyric | ornaments | technical)
//@    Interpretation content
//@ Attributes:
//@    color - optional
//@    id - optional @xml:id
//@    smufl-font - optional glyph to render this event
//@    value - the metrical duration. Required unless this event has measure="yes"
//@    measure - optional flag indicating that the event occupies the entire measure.
//@    orient - optional orientation of this event
//@    staff - optional staff index of this event
//@    duration - optional performed metrical duration, if different from value
//@    stem-direction - optional: up | down
//
class EventMnxAnalyser : public MnxElementAnalyser
{
protected:
    vector<ImoNote*> m_notes;
    vector<ImoRest*> m_rests;

public:
    EventMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //First step: extract information -----------------------------------------------

        //attrib: measure
        bool fMeasure = false;
        if (has_attribute("measure"))
        {
            fMeasure = true;
            //TODO: validate attribute value
            //string value = get_optional_string_attribute("measure");
        }

        //attrib: value
        int noteType = k_quarter;
        int dots = 0;
        if (!fMeasure && !get_attribute_note_value(&noteType, &dots))
        {
            error_msg("Missing or invalid 'value' attribute in <event>.");
            set_result(nullptr);
            return false;
        }

        //attrib: orient
        //TODO

        //attrib: staff
        //TODO

        //attrib: duration
        //TODO

        //attrib: color
        //TODO

        //attrib: smufl-font
        //TODO

        //attrib: stem-direction
        //TODO


            // Content

        // Metadata content
        //TODO

        //Either zero or more note elements, or one rest element = (note* | rest)
        // (xxxx | yyyy | zzzz)*    alternatives: zero or more
        ImoNoteRest* pFirstNR = nullptr;
        while (more_children_to_analyse())
        {
            if (get_optional("note"))
            {
                if (analyse_child())
                {
                    std::unique_ptr<ImoData> result = get_imo_result();
                    ImoNote* pNote = static_cast<ImoNote*>(result->pImo);
                    m_notes.push_back(pNote);
                    if (pFirstNR == nullptr)
                        pFirstNR = pNote;
                }
            }
            else if (get_optional("rest"))
            {
                if (analyse_child())
                {
                    std::unique_ptr<ImoData> result = get_imo_result();
                    ImoRest* pRest = static_cast<ImoRest*>(result->pImo);
                    m_rests.push_back(pRest);
                    if (pFirstNR == nullptr)
                        pFirstNR = pRest;
                }
            }
            else
                break;
        }

        analyse_and_set_xml_id(pFirstNR);

        //Event content = (articulations | lyric | ornaments | technical)
        // (xxxx | yyyy | zzzz)*    alternatives: zero or more
        while (more_children_to_analyse())
        {
            if (get_optional("articulations"))
            {
                //TODO
            }
            else if (get_optional("lyric"))
            {
                //TODO
            }
            else if (get_optional("ornaments"))
            {
                //TODO
            }
            else if (get_optional("technical"))
            {
                //TODO
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        // Interpretation content
        //TODO


        //second step: create notes -----------------------------------------------------

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        int staff = m_pAnalyser->get_current_staff();

        //add notes
        bool fIsChord = m_notes.size() > 1;
        vector<ImoNote*>::iterator itN;
        ImoNote* pPrevNote = nullptr;
        for (itN = m_notes.begin(); itN != m_notes.end(); ++itN)
        {
            ImoNote* pNR = *itN;
            pNR->set_note_type_and_dots(noteType, dots);
            add_to_model(pNR);
            m_pAnalyser->save_last_note(pNR);

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
        }

        //add rests
        vector<ImoRest*>::iterator itR;
        for (itR = m_rests.begin(); itR != m_rests.end(); ++itR)
        {
            ImoRest* pNR = *itR;
            pNR->mark_as_full_measure(fMeasure);
            //TODO: if 'measure="yes" set rest duration = measure duration as implied
            //      by current time signature
            pNR->set_note_type_and_dots(noteType, dots);
            pNR->set_staff(staff);
            add_to_model(pNR);
        }

//        m_pAnalyser->shift_time( pNR->get_duration() );

        set_result( LOMSE_NEW EventData(fMeasure, pFirstNR) );
        return true;    //success
    }

};

//@--------------------------------------------------------------------------------------
//@ <expression>
//@ Contexts:
//@    x
//@ Context Model:
//@    y
//@ Attributes:
//@    z
//
class ExpressionMnxAnalyser : public MnxElementAnalyser
{
public:
    ExpressionMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                          LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <fine>
//@ Parent element: <directions> (global)
//@ Context: Always empty.
//@ Attributes: None
//@
class FineMnxAnalyser : public MnxElementAnalyser
{
public:
    FineMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDirection* pDirection = static_cast<ImoDirection*>(
                                        ImFactory::inject(k_imo_direction, pDoc) );

        pDirection->set_display_repeat(k_repeat_fine);

        ImoTextRepetitionMark* pRM = static_cast<ImoTextRepetitionMark*>(
                ImFactory::inject(k_imo_text_repetition_mark, pDoc) );

        pRM->set_repeat_mark(k_repeat_fine);
        pRM->set_language("it");
        pRM->set_text("Fine");

        pDirection->add_attachment(pDoc, pRM);

        set_result( LOMSE_NEW ImoData(pDirection) );
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <global>
//@ Contexts:
//@    <mnx-common>
//@ Context Model:
//@    Measure content, which must not include any sequence content
//@ Attributes:
//@    parts - an optional set of IDs of parts to which this global applies
//
class GlobalMnxAnalyser : public MnxElementAnalyser
{
protected:
    const string m_tag;

public:
    GlobalMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                      LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: parts
        string parts = get_optional_string_attribute("parts", "*$SINGLE-PART$*");

        // attrib 'parts' is an optional set of IDs of part elements to which this
        // global content applies. The parts attribute is a list of part element IDs
        // as an unordered set of space-separated tokens.

        MeasuresVector* pMeasures = nullptr;
        m_pAnalyser->set_parent_for_directions("measure-global");

        //fill the table
        int numMeasures = 0;
        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            if (m_childToAnalyse.name() == "measure")
            {
                ++numMeasures;
                if (!pMeasures)
                {
                    //create the measures table for this global
                    pMeasures = m_pAnalyser->new_global(parts);
                }

                pMeasures->push_back(m_childToAnalyse);
                move_to_next_child();
            }
            else
                break;
        }

        set_result(nullptr);
        if (numMeasures == 0)
        {
            error_msg("<global>: missing mandatory element <measure>.");
            return false;       //error
        }

        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <grace>
//@ Parent elements:
//@    <sequence>, <tuplet>
//@ Context:
//@    <event>+
//@ Attributes:
//@    slash -	Yes or no. Optional, default: "yes"
//@    type - Optional, default: "steal-previous"
//@
class GraceMnxAnalyser : public MnxElementAnalyser
{
protected:
    int m_graceType = ImoGraceRelObj::k_grace_steal_previous;
    bool m_fSlash = true;
    float m_percentage = LOMSE_STEAL_TIME_SHORT;
    TimeUnits m_makeTime = 0.0;

public:
    GraceMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        ImoMusicData* pMD = get_anchor_as_music_data();
        if (!pMD)
            return false;    //failure

        //attrib: slash
        m_fSlash = get_optional_yes_no_attribute("slash", true);

        //attrib: type
        string type = get_optional_string_attribute("type", "steal-previous");
        m_graceType = ImoGraceRelObj::k_grace_steal_previous;
        m_percentage = (m_fSlash ? LOMSE_STEAL_TIME_SHORT : LOMSE_STEAL_TIME_LONG);
        if (type == "steal-previous")
        {
            m_graceType = ImoGraceRelObj::k_grace_steal_previous;
//            m_percentage = get_child_attribute_as_float("steal-time-previous", m_percentage);
        }
        else if (type == "steal-following")
        {
            m_graceType = ImoGraceRelObj::k_grace_steal_following;
//            m_percentage = get_child_attribute_as_float("steal-time-following", m_percentage);
        }
        else
        {
            error_msg("<grace>. Invalid type '" + type + "'. steal-previous assumed.");
        }
        m_percentage /= 100.0f;


        //event+
        m_pAnalyser->set_note_class(k_imo_note_grace);
        if (analyse_mandatory("event", pMD))
        {
            delete_result();
            ImoGraceRelObj* pGraceRO = create_grace_notes_relationship();

            while (analyse_optional("event", pMD))
            {
                delete_result();
                add_to_grace_notes_relationship(pGraceRO);
            }
        }
        m_pAnalyser->set_note_class(k_imo_note_regular);

        set_result(nullptr);
        return true;    //success
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoNote* get_grace_note()
    {
        ImoNote* pPrevNote = m_pAnalyser->get_last_note();
        return pPrevNote;
    }

    //-----------------------------------------------------------------------------------
    ImoGraceRelObj* create_grace_notes_relationship()
    {
        ImoNote* pNote = get_grace_note();
        if (pNote)
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();

            ImoGraceRelObj* pGraceRO = static_cast<ImoGraceRelObj*>(
                                        ImFactory::inject(k_imo_grace_relobj, pDoc));

            pNote->include_in_relation(pDoc, pGraceRO);
            pGraceRO->set_grace_type(m_graceType);
            pGraceRO->set_slash(m_fSlash);
            pGraceRO->set_percentage(m_percentage);
            pGraceRO->set_time_to_make(m_makeTime);

            return pGraceRO;
        }
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    void add_to_grace_notes_relationship(ImoGraceRelObj* pGraceRO)
    {
        ImoNote* pNote = get_grace_note();
        if (pNote)
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            pNote->include_in_relation(pDoc, pGraceRO);
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <head>
//@ Contexts:
//@    Any.
//@ Context Model:
//@    Metadata content:
//@     <title> <subtitle> <creator>+ <rights>
//@    Stylesheet definitions:
//@     (style-class | style-selector)+
//@ Attributes:
//@    None.
//
class HeadMnxAnalyser : public MnxElementAnalyser
{
protected:
    const string m_tag;

public:
    HeadMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
		//TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <instrument-sound>
//@ Contexts:
//@    x
//@ Context Model:
//@    y
//@ Attributes:
//@    z
//
class InstrumentSoundMnxAnalyser : public MnxElementAnalyser
{
public:
    InstrumentSoundMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                               LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <jump>
//@ Parent element: <directions> (global)
//@ Context: Always empty.
//@ Attributes:
//@    type - mandatory, jump type: segno, dsalfine,
//@             Other probably: dacapo, dalsegno, tocoda,
//@
class JumpMnxAnalyser : public MnxElementAnalyser
{
public:
    JumpMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    bool do_analysis() override
    {
        //attrib: type
        const string& type = get_mandatory_string_attribute("type", "", "jump");
        if (type.empty())
            return false;   //failure

        //AWARE: <jump> elements can just define playback instruction (e.g. type='????')
        //or also define a symbol (e.g. type='????')
        int symbol = 0;
        int instruction = k_repeat_none;
        bool fAtStart = true;
        if (type == "segno")    //D.S.
        {
            symbol = k_attr_segno;
            instruction = k_repeat_dal_segno;
            fAtStart = false;
        }
        else if (type == "dsalfine")    //D.S. al Fine
        {
            symbol = k_attr_segno;
            instruction = k_repeat_dal_segno_al_fine;
            fAtStart = false;
        }
        else if (type == "dacapo")
        {
            //TODO
            symbol = k_attr_dacapo;
//            instruction = k_repeat_da_capo_al_fine;
//            instruction = k_repeat_da_capo_al_coda
            instruction = k_repeat_da_capo;
//            fAtStart = false;
        }
        else if (type == "dalsegno")
        {
            //TODO
            symbol = k_attr_dalsegno;
//            instruction = k_repeat_dal_segno_al_coda;
            instruction = k_repeat_dal_segno;
//            fAtStart = false;
        }
        else if (type == "tocoda")
        {
            //TODO
            symbol = k_attr_tocoda;
            instruction = k_repeat_to_coda;
//            fAtStart = false;
        }
        else
        {
            error_msg("<jump>: invalid type '" + type + "'. Jump ignored.");
            set_result(nullptr);
            return false;    //failure
        }

        //create objects
        ImoDirection* pDirection = create_symbol(type, instruction, fAtStart);
        ImoSoundChange* pSC = create_instruction(symbol, fAtStart);

        set_result( LOMSE_NEW JumpData(pDirection, pSC) );
        return true;    //success
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoSoundChange* create_instruction(int symbol, bool fAtStart)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSoundChange* pSC = static_cast<ImoSoundChange*>(
                                    ImFactory::inject(k_imo_sound_change, pDoc));

        pSC->set_bool_attribute(symbol, true);
        if (!fAtStart)
            pSC->set_bool_attribute(k_attr_right_located, true);

        return pSC;
    }

    //-----------------------------------------------------------------------------------
    ImoDirection* create_symbol(const string& type, int instruction, bool fAtStart)
    {
        if (instruction == k_repeat_none)
            return nullptr;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDirection* pDirection = static_cast<ImoDirection*>(
                                        ImFactory::inject(k_imo_direction, pDoc) );

        pDirection->set_display_repeat(instruction);
        if (!fAtStart)
            pDirection->set_bool_attribute(k_attr_right_located, true);

        ImoTextRepetitionMark* pRM = static_cast<ImoTextRepetitionMark*>(
                ImFactory::inject(k_imo_text_repetition_mark, pDoc) );

        pRM->set_repeat_mark(instruction);
        pRM->set_language("it");
        pRM->set_text( determine_text(type) );
//        if (!fAtStart)
//            pRM->set_?????              //TODO: left justification


        pDirection->add_attachment(pDoc, pRM);
        return pDirection;
    }

    //-----------------------------------------------------------------------------------
    string determine_text(const string& type)
    {
        if (type == "segno")       return("D.S.");
        else if (type == "dsalfine")    return("D.S. al Fine");
//        else if (type == "dacapo")           return("D.C."); //TODO
//        else if (type == "dalsegno")    return("D.C."); //TODO
//        else if (type == "tocoda")      return("D.C."); //TODO
        else
        {
            stringstream ss;
            ss << "Missing case: jump label '" << type << "'.";
            LOMSE_LOG_ERROR(ss.str());
            error_msg(ss.str());
            return("??????");
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <key>
//@ Contexts:
//@    Direction content
//@ Context Model:
//@    None
//@ Attributes:
//@    fifths - the transposition from concert pitch in fifths
//@    mode - { major | minor }
//
class KeyMnxAnalyser : public MnxElementAnalyser
{
public:
    KeyMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    bool do_analysis() override
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoKeySignature* pKey = static_cast<ImoKeySignature*>(
                                    ImFactory::inject(k_imo_key_signature, pDoc) );

        //TODO: Here we are dealing only with "traditional" key signatures:
        //      chromatic scale in major and minor modes).

        bool fMajor = true;

        //attrib: fifths
        int fifths = get_mandatory_integer_attribute("fifths", 0, "key");

        //attrib: mode
        string mode = get_optional_string_attribute("mode", "major");
        fMajor = (mode == "major");


        error_if_more_elements();

        //set key
        pKey->set_key_type( fifths_to_key_signature(fifths, fMajor) );

        set_result( LOMSE_NEW ImoData(pKey) );
        return true;    //success
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
//@ <measure>
//@ Parent element: <part>
//@ Content, in any order
//@     <directions> (One, optional)
//@     <sequence> (One or more times)
//@
//@ Attributes
//@     barline - optional. Default: "light-heavy" for last measure, otherwise "regular".
//@     index - optional, one-based index, Default 1 for the first <measure> in the <part>
//@     number - optional, measure number to display. Deafult: same as index.
//@
//@ measure = directions?, sequence+
//@
class MeasureMnxAnalyser : public MnxElementAnalyser
{
protected:
    bool m_fGlobalMeasure;      //parsing a <measure> included in <global> element

public:
    MeasureMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor, bool fGlobalMeasure=false)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_fGlobalMeasure(fGlobalMeasure)
    {
    }

    bool do_analysis() override
    {
        if (m_fGlobalMeasure)
            return do_measure_global_analysis();
        else
            return do_measure_part_analysis();
    }

protected:

    //-----------------------------------------------------------------------------------
    bool do_measure_global_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, pDoc) );

        // directions?
        analyse_optional("directions", pMD);

        //TODO
        //error_if_more_elements();

        set_result( LOMSE_NEW ImoData(pMD) );
        return true;    //success
    }

    //-----------------------------------------------------------------------------------
    bool do_measure_part_analysis()
    {
        ImoMusicData* pMD = get_anchor_as_music_data();
        if (!pMD)
            return false;    //error

        //attrb: number
        string num = get_optional_string_attribute("number", "");
        m_pAnalyser->save_current_measure_num(num);
        TypeMeasureInfo* pInfo = create_measure_info(num);

        //attrb: index
        //TODO

        //attrb: barline
        //pMeasures = m_pAnalyser->new_global(parts);
        string barType = get_optional_string_attribute("barline", "regular");

            //content

        //before adding measure content, add global content
        m_pAnalyser->add_left_data_from_global_measure(pMD);

        // directions?
        analyse_optional("directions", pMD);

        // sequence+
        if (analyse_mandatory("sequence", pMD))
        {
            while (analyse_optional("sequence", pMD))
                move_to_next_child();
        }

        //TODO
        //error_if_more_elements();

        //add global content at end of measure
        m_pAnalyser->add_right_data_from_global_measure(pMD);

        //if there is a <repeat type='start'> element, fix previous barline to
        //add the repeat dots
        fix_prev_barline_if_start_repeat();

        ImoObj* pSO = static_cast<ImoStaffObj*>(pMD->get_last_child());
        if (pSO == nullptr || !pSO->is_barline())
            add_barline(pInfo, barType);
        else
            delete pInfo;


        set_result(nullptr);
        return true;    //success
    }

    //-----------------------------------------------------------------------------------
    TypeMeasureInfo* create_measure_info(const string& num)
    {
        TypeMeasureInfo* pInfo = LOMSE_NEW TypeMeasureInfo();
        pInfo->count = m_pAnalyser->increment_measures_counter();
        pInfo->number = num;
        m_pAnalyser->save_current_measure_num(num);
        return pInfo;
    }

    //-----------------------------------------------------------------------------------
    void add_barline(TypeMeasureInfo* pInfo, const string& barType)
    {
        advance_timepos_if_required();

        int endTimes = m_pAnalyser->get_end_repeat_info();
        string repeat = (endTimes == 0 ? "" : "backward");

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBarline* pBarline = static_cast<ImoBarline*>(
                                    ImFactory::inject(k_imo_barline, pDoc) );

        int barline = find_barline_type(barType, repeat);
        pBarline->set_type(barline);
        pBarline->set_measure_info(pInfo);
        pBarline->set_num_repeats(endTimes);
        add_to_model(pBarline);
        m_pAnalyser->save_last_barline(pBarline);
    }

    //-----------------------------------------------------------------------------------
    void fix_prev_barline_if_start_repeat()
    {
        int startTimes = m_pAnalyser->get_start_repeat_info();
        if (startTimes == 0)
            return;

        ImoBarline* pBarline = m_pAnalyser->get_last_barline();
        if (pBarline)
        {
            int type = pBarline->get_type();

            if (type == k_barline_simple)
                pBarline->set_type( k_barline_start_repetition );
            else if (type == k_barline_end)
                pBarline->set_type( k_barline_start_repetition );
            else if (type == k_barline_end_repetition)
                pBarline->set_type( k_barline_double_repetition );
            else
            {
                error_msg2(
                    "Not possible to add repetion dots to barline '"
                    + LdpExporter::barline_type_to_ldp(type)
                    + "'. <repeat> ignored.");
                return;
            }
        }
    }

    //-----------------------------------------------------------------------------------
    EBarline find_barline_type(const string& barType, const string& repeat)
    {
        bool fError = false;
        EBarline type = k_barline_simple;

        if (barType == "none")
            type = k_barline_none;
        else if (barType == "regular")      //TODO: regular should require repeat=""
            if (repeat == "backward")
                type = k_barline_end_repetition;
            else if (repeat.empty())
                type = k_barline_simple;
            else
                fError = true;
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
                type = k_barline_double_repetition_alt;     //heavy-heavy. See E.Gould, p.234
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
                "Invalid or not supported 'barline' attribute '" + barType
                + "' and/or <repeat> type '" + repeat
                + "' values. Replaced by 'regular' barline.");
        }

        return type;
    }

    //-----------------------------------------------------------------------------------
    void advance_timepos_if_required()
    {
//        TimeUnits curTime = m_pAnalyser->get_current_time();
//        TimeUnits maxTime = m_pAnalyser->get_max_time();
//        if (maxTime <= curTime)
//            return;
//
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoGoBackFwd* pImo = static_cast<ImoGoBackFwd*>(
//                                ImFactory::inject(k_imo_go_back_fwd, pDoc) );
//        pImo->set_forward(true);
//        pImo->set_time_shift(maxTime - curTime);
//
//        m_pAnalyser->set_current_time(maxTime);
//        add_to_model(pImo);
    }

};

//@--------------------------------------------------------------------------------------
//@ <mnx>
//@ Contexts:
//@    None: this is the top-level element.
//@ Context Model:
//@    Metadata content.
//@    <stylesheet>.
//@    One or more <global> elements - measure content shared by sets of parts within the score.
//@    One or more <part> elements - description and measure content of each part in the score.
//@    Zero or more <score-audio> elements - recordings of the score and their associated synchronization date.
//@ Attributes:
//@    None.
//@
//@ mnx = stylesheet?, global+, part+, score-audio*
//@
class MnxMnxAnalyser : public MnxElementAnalyser
{
public:
    MnxMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                   LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //create the document
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDocument* pImoDoc = static_cast<ImoDocument*>(
                                    ImFactory::inject(k_imo_document, pDoc));
        pImoDoc->set_version("0.0");    //AWARE: This is lenmusdoc version!
        pImoDoc->set_language("en");    //TODO: analyse language
        m_pAnchor = pImoDoc;
        m_pAnalyser->save_root_imo_document(pImoDoc);

        ImoScore* pScore = create_score();

        // add default styles
        add_default(pImoDoc);

        //stylesheet?
        //TODO

        //global+
        analyse_mandatory("global");
        while (analyse_optional("global"))
            mark_score_as_polymetric_polytonal();

        //head?
        //TODO: Confirm it goes here

        //part+
        analyse_mandatory("part", m_pAnchor);
        while (analyse_optional("part", m_pAnchor));

        //score-audio*
        //TODO

        m_pAnalyser->add_all_instruments(pScore);

        set_options(pScore);
        set_result( LOMSE_NEW ImoData(pImoDoc) );
        return true;    //success
    }

protected:

    ImoScore* create_score()
    {
        //add an empty score
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoContent* pContent = static_cast<ImoContent*>(
                        ImFactory::inject(k_imo_content, pDoc) );
        add_to_model(pContent);
        m_pAnchor = pContent;

        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, pDoc));
        pScore->set_accidentals_model( ImoScore::k_pitch_and_notation_provided );
        m_pAnalyser->score_analysis_begin(pScore);
        add_to_model(pScore);
        m_pAnchor = pScore;

        pScore->set_version(200);   //2.0
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

        //spacing options
        pOpt = pScore->get_option("Render.SpacingOptions");
        pOpt->set_long_value(k_render_opt_breaker_optimal);
    }

    void mark_score_as_polymetric_polytonal()
    {
        //TODO: mark the score
    }

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
//@ <note>
//@ Parent elements: <event>
//@ Context
//@    <tied>?
//@ Attributes:
//@    accidental - The accidental to display. Must match the alteration of the "pitch" attribute
//@    color - Optional
//@    display - Optional
//@    id - Optional
//@    perform - Optional. Controls whether this note is performed by consuming software.
//@    pitch - Optional. The note's pitch.
//@    smufl-font - Optional. The SMuFL-compliant font to be used when rendering this note.
//@    staff - Optional. If not provided, the value is inherited from any ancestor element that specified it.
//@    visibility - Optional
//@
class NoteMnxAnalyser : public MnxElementAnalyser
{
public:
    NoteMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: accidental
        string acc = get_optional_string_attribute("accidental", "");
        EAccidentals accidentals = get_notated_accidentals(acc);

        //attrib: color - Optional
        //TODO

        //attrib: display - Optional
        //TODO

        //attrib: perform - Optional. Controls whether this note is performed by consuming software.
        //TODO

        //attrib: pitch  (the written pitch of the note as a chromatic pitch: step, alter, octave)
        string pitch = get_mandatory_string_attribute("pitch", "", "note");
        if (pitch == "")
            return false;

        //attrib: smufl-font - Optional. The SMuFL-compliant font to be used when rendering this note.
        //TODO

        //attrib: staff
        int defStaff = m_pAnalyser->get_current_staff() + 1;
        int staff = get_optional_int_attribute("staff", defStaff) - 1;
        //TODO: check that staff < maxStaff set in part

        //attrib: visibility - Optional
        //TODO


            //create the note

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        int step = k_step_C;
        int octave = 4;
        float alter = 0.0f;
        int type = m_pAnalyser->get_note_class();
        ImoNote* pNote = static_cast<ImoNote*>(ImFactory::inject(type, pDoc));

        //set pitch and accidentals
        if (MnxAnalyser::pitch_to_components(pitch, &step, &octave, &alter))
        {
            error_msg("Unknown note pitch '" + pitch + "'. Replaced by 'C4'.");
            pNote->set_pitch(k_step_C, 4, k_no_accidentals);
        }
        else
            pNote->set_pitch(step, octave, accidentals);

        pNote->set_notated_accidentals(accidentals);
        pNote->force_to_display_accidentals();
        pNote->set_staff(staff);
        pNote->set_voice( m_pAnalyser->get_current_voice() );

        analyse_and_set_xml_id(pNote);


            // optional content

        //tied?
        if (get_optional("tied"))
            m_pAnalyser->save_tied_element(&m_childToAnalyse, pNote);

        set_result( LOMSE_NEW ImoData(pNote) );
        return true;    //success
    }

protected:

    //----------------------------------------------------------------------------------
    EAccidentals get_notated_accidentals(const string& acc)
    {
        //no accidentals specified
        if (acc.empty())    return k_no_accidentals;

        //standard accidentals
        if (acc == "sharp")                     return k_sharp;
        else if (acc == "natural")              return k_natural;
        else if (acc == "flat")                 return k_flat;
        else if (acc == "double-sharp")         return k_double_sharp;
        else if (acc == "sharp-sharp")          return k_sharp_sharp;
        else if (acc == "flat-flat")            return k_flat_flat;
        //else if (acc == "double-flat")
            //AWARE: double-flat is not in the specification. Lilypond test suite
            //       uses it and MuseScore imports it correctly. But Michael Good
            //       is clear about this. See:
            //http://forums.makemusic.com/viewtopic.php?f=12&t=2253&p=5965#p5964
            //http://forums.makemusic.com/viewtopic.php?f=12&t=2408&p=6558#p6556

        else if (acc == "natural-sharp")        return k_natural_sharp;
        else if (acc == "natural-flat")         return k_natural_flat;
        else if (acc == "triple-sharp")         return k_acc_triple_sharp;
        else if (acc == "triple-flat")          return k_acc_triple_flat;

        //microtonal: Tartini-style quarter-tone accidentals
        else if (acc == "quarter-flat")         return k_acc_quarter_flat;
        else if (acc == "quarter-sharp")        return k_acc_quarter_sharp;
        else if (acc == "three-quarters-flat")  return k_acc_three_quarters_flat;
        else if (acc == "three-quarters-sharp") return k_acc_three_quarters_sharp;

        //microtonal: quarter-tone accidentals that include arrows pointing down or up
        else if (acc == "sharp-down")           return k_acc_sharp_down;
        else if (acc == "sharp-up")             return k_acc_sharp_up;
        else if (acc == "natural-down")         return k_acc_natural_down;
        else if (acc == "natural-up")           return k_acc_natural_up;
        else if (acc == "flat-down")            return k_acc_flat_down;
        else if (acc == "flat-up")              return k_acc_flat_up;
        else if (acc == "double-sharp-down")    return k_acc_double_sharp_down;
        else if (acc == "double-sharp-up")      return k_acc_double_sharp_up;
        else if (acc == "flat-flat-down")       return k_acc_flat_flat_down;
        else if (acc == "flat-flat-up")         return k_acc_flat_flat_up;
        else if (acc == "arrow-down")           return k_acc_arrow_down;
        else if (acc == "arrow-up")             return k_acc_arrow_up;

    	//accidentals used in Turkish classical music
        else if (acc == "slash-quarter-sharp")  return k_acc_slash_quarter_sharp;
        else if (acc == "slash-sharp")          return k_acc_slash_sharp;
        else if (acc == "slash-flat")           return k_acc_slash_flat;
        else if (acc == "double-slash-flat")    return k_acc_double_slash_flat;

        //superscripted versions of the accidental signs, used in Turkish folk music
        else if (acc == "sharp-1")              return k_acc_sharp_1;
        else if (acc == "sharp-2")              return k_acc_sharp_2;
        else if (acc == "sharp-3")              return k_acc_sharp_3;
        else if (acc == "sharp-5")              return k_acc_sharp_5;
        else if (acc == "flat-1")               return k_acc_flat_1;
        else if (acc == "flat-2")               return k_acc_flat_2;
        else if (acc == "flat-3")               return k_acc_flat_3;
        else if (acc == "flat-4")               return k_acc_flat_4;

        //microtonal sharp and flat accidentals used in Iranian and Persian music
        else if (acc == "sori")                 return k_acc_sori;
        else if (acc == "koron")                return k_acc_koron;

        //other; unspecified. MusicXML file should specify SMuFl glyph to use
        else if (acc == "other")                return k_acc_other;

        else
        {
            error_msg2(
                "Invalid or not supported 'accidentals' value: '" + acc + "'.");
            return k_no_accidentals;
        }
    }

};
//@--------------------------------------------------------------------------------------
//@ <octave-shift>
//@ Parent element: <directions> (sequence)
//@ Context: Always empty.
//@ Attributes:
//@     type - mandatory, the type of octave shift: 8, -8, 15, -15, 22, -22.
//@     end - mandatory, the measure location of the last note that is affected by this octave shift.
//@     orient - optional, inherited from any sequence ancestor that specified it. If no ancestor, app dependent.
//@     staff - optional, inherited from any sequence ancestor that specified it. If no ancestor, app dependent.
//@
//@
class OctaveShiftMnxAnalyser : public MnxElementAnalyser
{
protected:
    ImoOctaveShiftDto* m_pInfo = nullptr;

public:
    OctaveShiftMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                           LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
    {
    }

    bool do_analysis() override
    {
        ImoDirection* pDirection = get_anchor_as_direction();
        if (!pDirection)
            return false;

//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        m_pInfo = static_cast<ImoOctaveShiftDto*>(
//                                ImFactory::inject(k_imo_octave_shift_dto, pDoc));
//        m_pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        //attrib: end
        const string& endLoc = get_optional_string_attribute("end", "");
        if (endLoc.empty())
        {
            error_msg("<octave-shift>: Missing 'end' attribute. <octave-shift> ignored.");
            return false;   //failure
        }
        //TODO: search for get_measure_location_value and decide how to convert

        //attrib: type
        int type = get_mandatory_integer_attribute("type", 8, "octave-shift");
        if (!(type == 8 || type == -8 || type == 15 || type == -15 || type == 22 || type == -22))
        {
            const string& value = get_optional_string_attribute("type", "");
            error_msg("Invalid octave-shift type '" + value + "'. Changed to 8.");
            type = 8;
        }

        //attrib: orient
        const string& orient = get_optional_string_attribute("orient", "");

        //attrib: staff
        //TODO

        static int num = 0;

        set_mandatory_data(orient, ++num, type);

//        int iStaff = pDirection->get_staff();

//        m_pInfo1->set_staffobj(nullptr);
//        m_pInfo1->set_staff(iStaff);
//        m_pAnalyser->add_relation_info(m_pInfo);    //AWARE: this deletes m_pInfo

        set_result(nullptr);
        return true;    //success
    }

protected:

    void set_mandatory_data(const string& UNUSED(value), int UNUSED(num), int UNUSED(size))
    {
//        if (value == "up" || value == "down")
//        {
//            m_pInfo1->set_start(true);
//            int id =  m_pAnalyser->new_octave_shift_id(num);
//            m_pInfo1->set_octave_shift_number(id);
//            --size;
//            if (value == "down")
//                size = -size;
//            m_pInfo1->set_shift_steps(size);
//        }
//        else if (value == "stop")
//        {
//            m_pInfo1->set_start(false);
//            int id =  m_pAnalyser->get_octave_shift_id_and_close(num);
//            m_pInfo1->set_octave_shift_number(id);
//        }
//        else
//        {
//            error_msg("Missing or invalid octave-shift type '" + value
//                      + "'. Octave-shift ignored.");
//            delete m_pInfo1;
//            m_pInfo1 = nullptr;
//        }
    }
};

//@--------------------------------------------------------------------------------------
//@ Contexts:
//@    mnx-common
//@ Context Model:
//@    Part description content =
//@         part-name?, part-abbreviation?, instrument-sound?
//@    Measure content =
//@         measure*
//
class PartMnxAnalyser : public MnxElementAnalyser
{
public:
    PartMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        m_pAnalyser->set_parent_for_directions("measure-part");

        ImoInstrument* pInstrument = create_instrument();

        //attrib: id  e.g.: id="p1"
        string id = get_optional_string_attribute("id", "*$SINGLE-PART$*");
        m_pAnalyser->save_current_part_id(id);


            // content

        // part-name
        analyse_optional("part-name", pInstrument);
//        // part-name-display?
//        analyse_optional("part-name-display", pInstrument);

        // part-abbreviation
        analyse_optional("part-abbreviation", pInstrument);
//        pInstrument->set_abbrev(
//            analyze_optional_child_pcdata("part-abbreviation", "") );
//        //TODO: full analysis. class PartAbbrevMnxAnalyser
//
//        // part-abbreviation-display?
//        analyse_optional("part-abbreviation-display", pInstrument);


        // instrument-sound
        analyse_optional("instrument-sound", pInstrument);

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

        m_pAnalyser->process_beams();
        m_pAnalyser->process_ties();

        set_result( LOMSE_NEW ImoData(pInstrument) );
        return true;    //success
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


    bool do_analysis() override
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

        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <repeat>
//@ Parent element: <directions> (global)
//@ Content: Always empty
//@ Attributes:
//@     type 	- mandatory, repeat type
//@     times 	- optional, repeat times
//@
class RepeatMnxAnalyser : public MnxElementAnalyser
{
public:
    RepeatMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                      LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: type
        string type = get_optional_string_attribute("type", "");
        if (!is_valid_type(type))
            return false;    //failure

        //attrib: times
        int times = get_optional_int_attribute("times", 1);

        m_pAnalyser->save_repeat_info(type, times);

        set_result(nullptr);
        return true;    //success
    }

protected:

    //-----------------------------------------------------------------------------------
    bool is_valid_type(const string& type)
    {
        if (type.empty() || !(type=="start" || type=="end"))
        {
            error_msg2("Missing or invalid attribute. type='" + type
                       + "'. <repeat> ignored.");
            return false;
        }
        return true;
    }

};

//@--------------------------------------------------------------------------------------
//@ <rest>
//@ Contexts:
//@    event
//@ Context Model:
//@    Metadata content
//@ Attributes:
//@    pitch - the musical pitch to which this rest should be visually registered
//
class RestMnxAnalyser : public MnxElementAnalyser
{
public:
    RestMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: pitch
        string pitch = get_optional_string_attribute("pitch", "");
        if (!pitch.empty())
        {
            int step = k_step_C;
            int octave = 4;
            float alter = 0.0f;
            if (MnxAnalyser::pitch_to_components(pitch, &step, &octave, &alter))
            {
                error_msg("Unknown rest pitch '" + pitch + "'. Replaced by 'C4'.");
                step = k_step_C;
                octave = 4;
            }
        }
        //TODO: What to do with pitch?

            //content

        // Metadata content
        //TODO: analyse Metadata content


        //create the rest
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoRest* pRest = static_cast<ImoRest*>(ImFactory::inject(k_imo_rest, pDoc));
        pRest->set_staff( m_pAnalyser->get_current_staff() );
        pRest->set_voice( m_pAnalyser->get_current_voice() );

        set_result( LOMSE_NEW ImoData(pRest) );
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <score>
//
class ScoreMnxAnalyser : public MnxElementAnalyser
{
public:
    ScoreMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        return false;   //failure
    }

protected:
};

//@--------------------------------------------------------------------------------------
//@ <segno>
//@ Parent element: <directions> (global)
//@ Context: Always empty.
//@ Attributes:
//@    glyph - optional, SMuFL glyph name
//@
class SegnoMnxAnalyser : public MnxElementAnalyser
{
public:
    SegnoMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                     LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: glyph
		//TODO

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDirection* pDirection = static_cast<ImoDirection*>(
                                        ImFactory::inject(k_imo_direction, pDoc) );

        pDirection->set_display_repeat(k_repeat_segno);

        ImoSymbolRepetitionMark* pImo = static_cast<ImoSymbolRepetitionMark*>(
            ImFactory::inject(k_imo_symbol_repetition_mark, pDoc) );
        pImo->set_symbol(ImoSymbolRepetitionMark::k_segno);

        pDirection->add_attachment(pDoc, pImo);
        add_to_model(pDirection);

        set_result( LOMSE_NEW ImoData(pDirection) );
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT sequence
//@     (beamed | dynamics | event | forward | grace | directions | tuplet)* )>
//@ <!ATTLIST sequence
//@     orientation
//@     staff
//@     name
//@ >
//
//@ Contexts:
//@    measure
//@ Context Model:
//@    Metadata content
//@    Zero or one directions elements
//@    Sequence content
//@    Interpretation content
//@ Attributes:
//@    orient - default orientation of direction and sequence content
//@    staff - default staff index of direction or sequence content
//@    voice - optional cross-measure voice identifier
//
class SequenceMnxAnalyser : public MnxElementAnalyser
{
public:
    SequenceMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: orient
        //TODO

        //attrib: staff
        int staff = get_optional_int_attribute("staff", 1) - 1;
        m_pAnalyser->set_current_staff(staff);

        //attrib: voice
            //TODO
        m_pAnalyser->set_current_voice( m_pAnalyser->get_current_voice() + 1 );


            // content

        m_pAnalyser->set_parent_for_directions("sequence");

        // Metadata content
        //TODO: analyse_metadata_content(m_pAnchor);

        // Zero or one directions elements
        analyse_optional("directions");

        // Sequence content
        analyse_content("sequence_content", m_pAnchor);

        // Interpretation content
        //TODO: analyse_interpretation_content(m_pAnchor);

        m_pAnalyser->set_parent_for_directions("measure-part");

        set_result(nullptr);
        return true;    //success
    }

protected:

};

//---------------------------------------------------------------------------------------
//@ Context Model:
//@    Metadata content
//@    Zero or one <directions> elements
//@    Zero or one <beams> elements
//@    Sequence content = (event? | forward? | tuplet? | grace?)*
//@    Interpretation content
//@ Attributes:
//@    orient - default orientation of direction and sequence content
//@    staff - default staff index of direction or sequence content
//@    voice - optional cross-measure voice identifier
//@//
// Not in specification but they exist in example:
//      expression | wedge
//
class SequenceContentMnxAnalyser : public MnxElementAnalyser
{
protected:
    TimeUnits m_cursor;

    list<ImoObj*> m_content;

public:
    SequenceContentMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                               LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_cursor(0.0)
    {
    }

    bool do_analysis() override
    {
        //attrib: orient
        //TODO

        //attrib: staff
        int staff = get_optional_int_attribute("staff", 1) - 1;
        m_pAnalyser->set_current_staff(staff);

        //attrib: voice
        //TODO

            // content

        m_cursor = 0.0f;    //for sequence_content() method

        // directions*
        //TODO

        // beams?
        if (get_optional("beams"))
            m_pAnalyser->save_beams_element(&m_childToAnalyse);

        // sequence_content
        while (more_children_to_analyse())
        {
            if (analyse_optional("dynamics", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("event", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("forward", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("grace", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("directions", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("tuplet", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("expression", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("wedge", m_pAnchor))
            {
                sequence_content();
            }
            else if (analyse_optional("clef", m_pAnchor))  //in example hot-cross-buns
            {
                sequence_content();
                //TODO: clef is added at the end of the measure (hot-cross-buns)
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }


        set_result(nullptr);
        return true;    //success
    }

protected:
    void sequence_content()
    {
        //TODO: finish algorithm for "Sequence content" (is this needed?)

        //2. If next is a beamed element:
        if (m_childToAnalyse.name() == "beamed")
        {
            //If beamed group has a value of list type, beamed groups have been illegally nested. Throw an error.
            //Set beamed group to an empty list.
            //Sequence the content of next, retaining the value of beamed group.
            //Record beamed group as a group of beamed events within the sequence.
            //Set beamed group to an undefined value.
        }
        //If next is an event element:
        else if (m_childToAnalyse.name() == "event")
        {
            std::unique_ptr<EventData> data = get_event_result();
            //If next has a measure value of yes,
//            if (data->fMeasure)
//            {
//                //If sequence cursor is greater than zero, throw a processing error.
//                //TODO:if (m_cursor > 0.0f)
//                //Set sequence cursor to the end of the measure as defined by its time signature.
//                ImoTimeSignature* pTime = m_pAnalyser->get_current_time_signature();
//                m_cursor = pTime->get_measure_duration();
//            }
//            else    //Else,
//            {
//                //Set the metrical position of next to sequence cursor.
//                //Add the duration of next, multiplied by the time modification ratio, to sequence cursor.
//            }
            //If beamed group is a list, append next to beamed group.
        }
        //If next is a forward element:
        else if (m_childToAnalyse.name() == "forward")
        {
            //Set the metrical position of next to sequence cursor.
            //Add the duration of next, multiplied by the time modification ratio, to sequence cursor.
        }
        //Else, if next is a tuplet element:
        else if (m_childToAnalyse.name() == "tuplet")
        {
            //Sequence the content of next, using sequence cursor as the starting position, retaining the current value of beamed group, and multiplying the time modification ratio by the tuplet's outer / inner ratio for the processing of the tuplet.
            //Add the total duration of next as given by outer, multiplied by the time modification ratio, to sequence cursor.
        }
        //Else, if next is a grace element:
        else if (m_childToAnalyse.name() == "grace")
        {
            //Process the contents of next, assigning them a non-metrical ordering relative to preceding or following elements as appropriate.
        }
        //Else, if next is direction content:
        else if (m_childToAnalyse.name() == "directions")
        {
            //Take the current value of sequence cursor as the measure location of next.
        }
        //If sequence cursor exceeds the specified duration for the enclosing element
        //(time signature for a measure, inner attribute for a tuplet), throw a processing error.
    }
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

    bool do_analysis() override
    {
		//TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <tied>
//@ Parent element: <note>
//@ Context: Always empty.
//@ Attributes:
//@    location
//@    target
//@
class TiedMnxAnalyser : public MnxElementAnalyser
{
public:
    TiedMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        ImoNote* pNote = get_anchor_as_note();
        if (!pNote)
            return false;

        //attrib: location
        //TODO

        //attrib: target
        string target = get_optional_string_attribute("target", "");

        if (!target.empty())
        {
            ImoNoteRest* pNR = get_noterest(target, "tied");
            if (pNR && pNR->is_note())
            {
                set_result(  LOMSE_NEW ImoData( create_tie(pNote, static_cast<ImoNote*>(pNR)) ));
                return true;    //success
            }
            else
            {
                //TODO: error no target note
            }
        }

        set_result(nullptr);
        return false;    //failure
    }

protected:
    ImoTie* create_tie(ImoNote* pStartNote, ImoNote* pEndNote)
    {
        //TODO: Finish this
        static int tieNumber = 0;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();

        ImoTie* pTie = static_cast<ImoTie*>(
                            ImFactory::inject(k_imo_tie, pDoc));
        pTie->set_tie_number( ++tieNumber );
        //pTie->set_color( pStartDto->get_color() );
        //pTie->set_orientation( pStartDto->get_orientation() );

        ImoTieData* pStartData = static_cast<ImoTieData*>(
                                    ImFactory::inject(k_imo_tie_data, pDoc) );
        pStartNote->include_in_relation(pDoc, pTie, pStartData);

        ImoTieData* pEndData = static_cast<ImoTieData*>(
                                    ImFactory::inject(k_imo_tie_data, pDoc) );
        pEndNote->include_in_relation(pDoc, pTie, pEndData);

        pStartNote->set_tie_next(pTie);
        pEndNote->set_tie_prev(pTie);

        return pTie;
    }

};

//@--------------------------------------------------------------------------------------
//@ <time>
//@ Contexts:
//@    Direction content
//@ Context Model:
//@    None
//@ Attributes:
//@    signature - the displayed time signature
//@    measure - a time signature which describes the content of the current measure
//@              only, and which is not displayed
//
class TimeMnxAnalyser : public MnxElementAnalyser
{
public:
    TimeMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                    LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //attrib: signature
        string signature = get_mandatory_string_attribute("signature", "4/4", "time");
        ImoTimeSignature* pTime = parse_time_signature(signature);

        //attrib: measure
        //TODO

        set_result( LOMSE_NEW ImoData(pTime) );
        return true;    //success
    }

protected:

    ImoTimeSignature* parse_time_signature(const string& input)
    {
        //TODO: This is test code. Replace for code for parsing time signature when
        //      MNX standard is more advanced
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTimeSignature* pTime = static_cast<ImoTimeSignature*>(
                                    ImFactory::inject(k_imo_time_signature, pDoc) );

        int beats= 4;
        int btype = 4;

        if (input == "3/4")
        {
            beats = 3;
            btype = 4;
        }
        else if (input == "2/4")
        {
            beats = 2;
            btype = 4;
        }
        else if (input == "6/8")
        {
            beats = 6;
            btype = 8;
        }

        pTime->set_top_number(beats);
        pTime->set_bottom_number(btype);

        return pTime;




//        //Examples:
//        //
//        //3/4     - Three-quarters time
//        //2+3+2/8 - A compound time signature of 2/8, 3/8 and 2/8, with 2+3+2 over the
//        //          shared denominator 8.
//        //2/8 + 3/4 + 2/8 - A compound time signature of 2/8, 3/4 and 2/8 as separate
//        //                  fractions (note that the spaces are ignored)
//
//
//        //1.Let input be the string being parsed.
//            //done
//
//        //2.Let tokens be the result of strictly splitting the string input using U+002B PLUS as a delimiter.
//        stringstream ss(input);
//        string token;
//        vector<string> tokens;
//        while (std::getline(ss, token, '+'))
//        {
//            tokens.push_back(token);
//        }
//
//        //3. If tokens is empty, return an error.
//        if (tokens.size() == 0)
//        {
//            error_msge();
//            return k_error;
//        }
//
//        //4. Let shared denominator be true.
//        bool fSharedDenominator = true;
//
//        //5. Let fractions be an empty list.
//        ?
//
//        //6. While tokens is not empty,
//        while(?)
//        {
//            //6.1. Remove the first element of tokens and assign it to t after stripping leading and trailing white space.
//
//            //6.2. If t contains the characters U+002F SLASH or U+002A ASTERISK,
//            if (?)
//            {
//                //6.2.1. Let nv be the result of parsing t as a note value quantity.
//
//                //6.2.2. If nv has a number of dots greater than zero, return an error.
//
//                //6.2.3. If shared denominator is true,
//                if (fSharedDenominator)
//                {
//
//                    //6.2.3.1. Replace the denominator in each element of fractions with the denominator of nv.
//
//                    //6.2.3.2. If more elements remain in tokens,
//                    if (?)
//                        //Set shared denominator to false.
//                        fSharedDenominator = false;
//                }
//
//                //6.2.4. Append nv to fractions.
//
//            }
//            else    //6.3. Else,
//            {
//                //6.3.1. If tokens is empty, return an error.
//
//                //6.3.2. If shared denominator is false, return an error.
//
//                //6.3.3. Let numerator be the result of parsing t as a valid integer.
//
//                //6.3.4. Append the fraction composed of numerator and the denominator 1 to fractions.
//
//            }
//        }
//        //7. Return fractions and shared denominator as the result.

    }

};

//@--------------------------------------------------------------------------------------
//@ <tuplet>
//@ Parent elements: <sequence>, <tuplet>
//@ Context, in any order:
//@    <event> (Zero or more times)
//@    <tuplet> (Zero or more times)
//@    <grace> (Zero or more times)
//@    <forward> (Zero or more times)
//@
//@ Attributes
//@    inner - mandatory, the duration of the enclosed content, as notated.
//@    outer - mandatory, the duration of this tuplet with respect to its containing element.
//@    bracket - optional, controls the display of a bracket
//@    orient - optional
//@    show-number - optional
//@    show-value - optional
//@    staff - optional
//@
class TupletMnxAnalyser : public MnxElementAnalyser
{
protected:
    ImoTuplet* m_pTuplet = nullptr;

public:
    TupletMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                      LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pTuplet = static_cast<ImoTuplet*>( ImFactory::inject(k_imo_tuplet, pDoc) );

        //attrib: inner & outer - mandatory
        const string& inner = get_mandatory_string_attribute("inner", "", "tuplet");
        const string& outer = get_mandatory_string_attribute("outer", "", "tuplet");
        if (inner.empty() || outer.empty())
            return false;

        set_multipliers(inner, outer);


        //attrib: bracket - optional
        string value = get_optional_string_attribute("bracket", "");
        set_bracket(value);

        //attrib: orient - optional
        //attrib: show-number - optional
//        // attrib: show-number (actual | both | none) #IMPLIED
//        value = get_optional_string_attribute("show-number", "");
//        set_show_number(value);
//
        //attrib: show-value - optional
        //attrib: staff - optional


            //content

        while (more_children_to_analyse())
        {
            if (get_optional("event"))
            {
                if (analyse_child(m_pAnchor))
                    add_event_to_tuplet();
            }
            else if (get_optional("tuplet"))
            {
//                if (analyse_child(m_pAnchor))
//                    add_tuplet_to_tuplet();
            }
            else if (get_optional("grace"))
            {
//                if (analyse_child(m_pAnchor))
//                    add_grace_to_tuplet();
            }
            else if (get_optional("forward"))
            {
//                if (analyse_child(m_pAnchor))
                    //?????????????????
            }
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        create_tuplet();

        set_result(nullptr);
        return true;    //success
    }

protected:

    //-----------------------------------------------------------------------------------
    void set_multipliers(const string& inner, const string& outer)
    {
        int innerType;
        int innerDots;
        int innerMultiplier;

        if (!MnxAnalyser::get_note_value_quantity(inner, &innerType, &innerDots, &innerMultiplier))
            return;

        int outerType;
        int outerDots;
        int outerMultiplier;

        if (!MnxAnalyser::get_note_value_quantity(outer, &outerType, &outerDots, &outerMultiplier))
            return;

        if (innerType == outerType && innerDots == outerDots)
        {
            m_pTuplet->set_actual_number(innerMultiplier);
            m_pTuplet->set_normal_number(outerMultiplier);
        }
    }

    //-----------------------------------------------------------------------------------
    void set_bracket(const string& value)
    {
        if (value.empty())
            m_pTuplet->set_show_bracket(k_yesno_default);
        else if (value == "yes")
            m_pTuplet->set_show_bracket(k_yesno_yes);
        else if (value == "no")
            m_pTuplet->set_show_bracket(k_yesno_no);
        else
        {
            error_msg("Invalid value '" + value +
                      "' for yes-no bracket attribute. 'no' assumed.");
            m_pTuplet->set_show_bracket(k_yesno_no);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_event_to_tuplet()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        std::unique_ptr<EventData> data = get_event_result();
        ImoNoteRest* pNR = data->pNR;
        pNR->include_in_relation(pDoc, m_pTuplet, nullptr);
    }

    //-----------------------------------------------------------------------------------
    void create_tuplet()
    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//
//        ImoTupletDto* pStartDto = m_matches.front();
//        ImoTuplet* pTuplet = ImFactory::inject_tuplet(pDoc, pStartDto);
//
//        std::list<ImoTupletDto*>::iterator it;
//        for (it = m_matches.begin(); it != m_matches.end(); ++it)
//        {
//            ImoNoteRest* pNR = (*it)->get_note_rest();
//            pNR->include_in_relation(pDoc, pTuplet, nullptr);
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    void MnxTupletsBuilder::add_to_open_tuplets(ImoNoteRest* pNR)
//    {
//        if (m_pendingItems.size() > 0)
//        {
//            ListIterator it;
//            for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
//            {
//                if ((*it)->is_start_of_relation() )
//                {
//                    ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
//                    pInfo->set_tuplet_number( (*it)->get_item_number() );
//                    pInfo->set_tuplet_type(ImoTupletDto::k_continue);
//                    pInfo->set_note_rest(pNR);
//                    save_item_info(pInfo);
//                }
//            }
//        }
    }
//
//    //-----------------------------------------------------------------------------------
//    void MnxTupletsBuilder::get_factors_from_nested_tuplets(int* pTop, int* pBottom)
//    {
//        *pTop = 1;
//        *pBottom = 1;
//        ListIterator it;
//        for(it=m_pendingItems.begin(); it != m_pendingItems.end(); ++it)
//        {
//            if ((*it)->is_start_of_relation() )
//            {
//                ImoTupletDto* pInfo = static_cast<ImoTupletDto*>(*it);
//                *pTop *= pInfo->get_normal_number();
//                *pBottom *= pInfo->get_actual_number();
//            }
//        }
//    }

};

//@--------------------------------------------------------------------------------------
//@ <wedge>
//@ Contexts:
//@    x
//@ Context Model:
//@    y
//@ Attributes:
//@    z
//
class WedgeMnxAnalyser : public MnxElementAnalyser
{
public:
    WedgeMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
    }
};

//@--------------------------------------------------------------------------------------
//@ <template_for_analyser>
//@ Contexts:
//@    x
//@ Context Model:
//@    y
//@ Attributes:
//@    z
//
class TemplateMnxAnalyser : public MnxElementAnalyser
{
public:
    TemplateMnxAnalyser(MnxAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : MnxElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    bool do_analysis() override
    {
        //TODO: implement Analyser
        set_result(nullptr);
        return true;    //success
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
//    , m_pTiesBuilder(nullptr)
//    , m_pBeamsBuilder(nullptr)
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
    , m_result( LOMSE_NEW AnalysisResult() )
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
    , m_curStaff(0)
    , m_curVoice(0)
    , m_beamLevel(0)
    , m_noteClass(k_imo_note_regular)
{
    //populate the name to enum conversion map
//    m_NameToEnum["accordion-registration"] = k_mnx_tag_accordion_registration;
//    m_NameToEnum["articulations"] = k_mnx_tag_articulations;
//    m_NameToEnum["backup"] = k_mnx_tag_backup;
//    m_NameToEnum["barline"] = k_mnx_tag_barline;
    m_NameToEnum["beam"] = k_mnx_tag_beam;
    m_NameToEnum["beam-hook"] = k_mnx_tag_beam_hook;
    m_NameToEnum["beams"] = k_mnx_tag_beams;
//    m_NameToEnum["bracket"] = k_mnx_tag_bracket;
    m_NameToEnum["clef"] = k_mnx_tag_clef;
//    m_NameToEnum["coda"] = k_mnx_tag_coda;
//    m_NameToEnum["damp"] = k_mnx_tag_damp;
//    m_NameToEnum["damp-all"] = k_mnx_tag_damp_all;
//    m_NameToEnum["dashes"] = k_mnx_tag_dashes;
//    m_NameToEnum["direction"] = k_mnx_tag_direction;
    m_NameToEnum["directions"] = k_mnx_tag_directions;
//    m_NameToEnum["direction-type"] = k_mnx_tag_direction_type;
    m_NameToEnum["dynamics"] = k_mnx_tag_dynamics;
//    m_NameToEnum["ending"] = k_mnx_tag_ending;
    m_NameToEnum["event"] = k_mnx_tag_event;
    m_NameToEnum["expression"] = k_mnx_tag_expression;
//    m_NameToEnum["eyeglasses"] = k_mnx_tag_eyeglasses;
//    m_NameToEnum["fermata"] = k_mnx_tag_fermata;
//    m_NameToEnum["forward"] = k_mnx_tag_forward;
    m_NameToEnum["fine"] = k_mnx_tag_fine;
    m_NameToEnum["global"] = k_mnx_tag_global;
    m_NameToEnum["grace"] = k_mnx_tag_grace;
//    m_NameToEnum["harp-pedals"] = k_mnx_tag_harp_pedals;
    m_NameToEnum["head"] = k_mnx_tag_head;
//    m_NameToEnum["image"] = k_mnx_tag_image;
    m_NameToEnum["instrument-sound"] = k_mnx_tag_instrument_sound;
    m_NameToEnum["jump"] = k_mnx_tag_jump;
    m_NameToEnum["key"] = k_mnx_tag_key;
//    m_NameToEnum["lyric"] = k_mnx_tag_lyric;
    m_NameToEnum["measure"] = k_mnx_tag_measure;
//    m_NameToEnum["metronome"] = k_mnx_tag_metronome;
//    m_NameToEnum["midi-device"] = k_mnx_tag_midi_device;
//    m_NameToEnum["midi-instrument"] = k_mnx_tag_midi_instrument;
    m_NameToEnum["mnx"] = k_mnx_tag_mnx;
//    m_NameToEnum["notations"] = k_mnx_tag_notations;
    m_NameToEnum["note"] = k_mnx_tag_note;
    m_NameToEnum["octave-shift"] = k_mnx_tag_octave_shift;
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
    m_NameToEnum["repeat"] = k_mnx_tag_repeat;
    m_NameToEnum["rest"] = k_mnx_tag_rest;
//    m_NameToEnum["scordatura"] = k_mnx_tag_scordatura;
    m_NameToEnum["score"] = k_mnx_tag_score;
//    m_NameToEnum["score-instrument"] = k_mnx_tag_score_instrument;
//    m_NameToEnum["score-part"] = k_mnx_tag_score_part;
//    m_NameToEnum["score-partwise"] = k_mnx_tag_score_partwise;
    m_NameToEnum["segno"] = k_mnx_tag_segno;
    m_NameToEnum["sequence"] = k_mnx_tag_sequence;
    m_NameToEnum["sequence_content"] = k_mnx_tag_sequence_content;
//    m_NameToEnum["slur"] = k_mnx_tag_slur;
//    m_NameToEnum["sound"] = k_mnx_tag_sound;
    m_NameToEnum["staff"] = k_mnx_tag_staff;
//    m_NameToEnum["string-mute"] = k_mnx_tag_string_mute;
//    m_NameToEnum["technical"] = k_mnx_tag_technical;
//    m_NameToEnum["text"] = k_mnx_tag_text;
    m_NameToEnum["tied"] = k_mnx_tag_tied;
    m_NameToEnum["time"] = k_mnx_tag_time;
//    m_NameToEnum["time-modification"] = k_mnx_tag_time_modification;
    m_NameToEnum["tuplet"] = k_mnx_tag_tuplet;
//    m_NameToEnum["tuplet-actual"] = k_mnx_tag_tuplet_actual;
//    m_NameToEnum["tuplet-normal"] = k_mnx_tag_tuplet_normal;
//    m_NameToEnum["virtual-instrument"] = k_mnx_tag_virtual_instr;
    m_NameToEnum["wedge"] = k_mnx_tag_wedge;
//    m_NameToEnum["words"] = k_mnx_tag_words;
}

//---------------------------------------------------------------------------------------
MnxAnalyser::~MnxAnalyser()
{
    delete_relation_builders();
    delete_globals();
    m_NameToEnum.clear();
    m_lyrics.clear();
    m_lyricIndex.clear();
    set_result(nullptr);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::delete_globals()
{
    if (m_globals.size() > 0)
    {
        map<string, MeasuresVector*>::iterator it = m_globals.begin();
        MeasuresVector* measures = it->second;
        measures->clear();
        delete measures;
        m_globals.clear();
    }
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::delete_relation_builders()
{
//    delete m_pTiesBuilder;
//    delete m_pBeamsBuilder;
    delete m_pTupletsBuilder;
    delete m_pSlursBuilder;
    delete m_pVoltasBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* MnxAnalyser::analyse_tree_and_get_object(XmlNode* root)
{
    delete_relation_builders();
    m_pTupletsBuilder = LOMSE_NEW MnxTupletsBuilder(m_reporter, this);
    m_pSlursBuilder = LOMSE_NEW MnxSlursBuilder(m_reporter, this);
    m_pVoltasBuilder = LOMSE_NEW MnxVoltasBuilder(m_reporter, this);

    m_pTree = root;
    m_curStaff = 0;
    m_curVoice = 1;
    m_beamLevel = 0;
    analyse_node(root);
    return get_imo_result()->pImo;
}

//---------------------------------------------------------------------------------------
ImoObj* MnxAnalyser::analyse_tree(XmlNode* tree, const string& locator)
{
    m_fileLocator = locator;
    return analyse_tree_and_get_object(tree);
}

//---------------------------------------------------------------------------------------
bool MnxAnalyser::analyse_node(XmlNode* pNode, ImoObj* pAnchor)
{
    //m_reporter << "DBG. Analysing node: " << pNode->name() << endl;
    MnxElementAnalyser* a = new_analyser( pNode->name(), pAnchor );
    set_result(nullptr);
    bool res = a->analyse_node(pNode);
    delete a;
    return res;
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
    m_measuresCounter = 0;
    m_noteClass = k_imo_note_regular;
    set_result(nullptr);
}

//---------------------------------------------------------------------------------------
std::unique_ptr<AnalysisData> MnxAnalyser::get_result()
{
    return m_result->get_result();
}

//---------------------------------------------------------------------------------------
std::unique_ptr<ImoData> MnxAnalyser::get_imo_result()
{
    return m_result->get_imo_result();
}

//---------------------------------------------------------------------------------------
std::unique_ptr<JumpData> MnxAnalyser::get_jump_result()
{
    return m_result->get_jump_result();
}

//---------------------------------------------------------------------------------------
std::unique_ptr<EventData> MnxAnalyser::get_event_result()
{
    return m_result->get_event_result();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::set_result(AnalysisData* pData)
{
    m_result->set_result(pData);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::delete_result()
{
    m_result->delete_result();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::add_relation_info(ImoObj* pDto)
{
    // factory method to deal with all relations

//    if (pDto->is_beam_dto())
//        m_pBeamsBuilder->add_item_info(static_cast<ImoBeamDto*>(pDto));
//    else if (pDto->is_tie_dto())
//        m_pTiesBuilder->add_item_info(static_cast<ImoTieDto*>(pDto));
    if (pDto->is_slur_dto())
        m_pSlursBuilder->add_item_info(static_cast<ImoSlurDto*>(pDto));
    else if (pDto->is_tuplet_dto())
        m_pTupletsBuilder->add_item_info(static_cast<ImoTupletDto*>(pDto));
    else if (pDto->is_volta_bracket_dto())
        m_pVoltasBuilder->add_item_info(static_cast<ImoVoltaBracketDto*>(pDto));
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::clear_pending_relations()
{
    m_beams.clear();
    m_ties.clear();
//    m_pTiesBuilder->clear_pending_items();
    m_pSlursBuilder->clear_pending_items();
//    m_pBeamsBuilder->clear_pending_items();
    m_pTupletsBuilder->clear_pending_items();
    m_pVoltasBuilder->clear_pending_items();

    m_lyrics.clear();
    m_lyricIndex.clear();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::process_beams()
{
    vector<XmlNode>::const_iterator it;
    for (it=m_beams.begin(); it != m_beams.end(); ++it)
    {
        XmlNode node = *it;
        m_beamLevel = 0;
        analyse_node(&node, nullptr);
    }
    m_beams.clear();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::save_beams_element(XmlNode node)
{
    m_beams.push_back(node);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::process_ties()
{
    vector< pair<XmlNode, ImoNote*> >::const_iterator it;
    for (it=m_ties.begin(); it != m_ties.end(); ++it)
    {
        XmlNode node = it->first;
        analyse_node(&node, it->second);
    }
    m_ties.clear();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::save_tied_element(XmlNode node, ImoNote* pNote)
{
    m_ties.push_back( std::make_pair(node, pNote) );
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
    pGrp->set_owner_score(get_score_being_analysed());

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
        case k_mnx_tag_beam:                return LOMSE_NEW BeamMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_beam_hook:           return LOMSE_NEW BeamHookMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_beams:               return LOMSE_NEW BeamsMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_clef:                return LOMSE_NEW ClefMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_directions:          return LOMSE_NEW DirectionsMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_dynamics:            return LOMSE_NEW DynamicsMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_event:               return LOMSE_NEW EventMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_expression:          return LOMSE_NEW ExpressionMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_fine:                return LOMSE_NEW FineMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_global:              return LOMSE_NEW GlobalMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_grace:               return LOMSE_NEW GraceMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_head:                return LOMSE_NEW HeadMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_instrument_sound:    return LOMSE_NEW InstrumentSoundMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_jump:                return LOMSE_NEW JumpMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_key:                 return LOMSE_NEW KeyMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_measure:             return LOMSE_NEW MeasureMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_mnx:                 return LOMSE_NEW MnxMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_note:                return LOMSE_NEW NoteMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_octave_shift:        return LOMSE_NEW OctaveShiftMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_part:                return LOMSE_NEW PartMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_part_name:           return LOMSE_NEW PartNameMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_repeat:              return LOMSE_NEW RepeatMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_rest:                return LOMSE_NEW RestMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_score:               return LOMSE_NEW ScoreMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_segno:               return LOMSE_NEW SegnoMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_sequence:            return LOMSE_NEW SequenceMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_sequence_content:    return LOMSE_NEW SequenceContentMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_staff:               return LOMSE_NEW StaffMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_tied:                return LOMSE_NEW TiedMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_time:                return LOMSE_NEW TimeMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_tuplet:              return LOMSE_NEW TupletMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mnx_tag_wedge:               return LOMSE_NEW WedgeMnxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
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

bool MnxAnalyser::pitch_to_components(const string& pitch, int *step, int* octave,
                                      float* alter)
{
    // Analyzes string pitch (MNX format), extracts its parts (step, octave and
    // and alteration) and stores them in the corresponding parameters.
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
        *alter = 0.0f;
    else
    {
        *alter = 0.0f;
        *alter = to_alteration(pitch.substr(1, i-1));
        if (*alter == -999.0f)
            return true;   //error
    }

    //octave
    *octave = to_octave(pitch[i]);
    if (*octave == -1)
        return true;   //error

    //optional: non-integer alteration
    if (++i != iMax)
    {
        if (pitch[i] == '+' || pitch[i] == '-')
        {
            try
            {
                size_t sz;
                *alter += std::stof(pitch.substr(i), &sz);
                if (i+sz != iMax)
                    return true;   //error
            }
            catch (...)
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
float MnxAnalyser::to_alteration(const std::string& accidentals)
{
    switch (accidentals.length())
    {
        case 0:
            return 0.0f;
            break;

        case 1:
            if (accidentals[0] == '#')
                return 1.0f;
            else if (accidentals[0] == 'b')
                return -1.0f;
            else
            {
                //error_msg();    //k_invalid_accidentals
                return -999.0f;     //error: invalid alteration
            }
            break;

        case 2:
            if (accidentals.compare(0, 2, "##") == 0)
                return 2.0f;
            else if (accidentals.compare(0, 2, "bb") == 0)
                return -2.0;
            else
            {
                //error_msg();    //k_invalid_accidentals
                return -999.0f;     //error: invalid alteration
            }
            break;

        default:
            {
                //error_msg();    //k_invalid_accidentals
                return -999.0f;     //error: invalid alteration
            }
    }
}

//---------------------------------------------------------------------------------------
MeasuresVector* MnxAnalyser::new_global(const string& partList)
{
    //'parts' is a list of part element IDs, an unordered set of space-separated tokens.

    MeasuresVector* pVector = LOMSE_NEW MeasuresVector;

    vector<string> result = MnxAnalyser::tokenize_spaces(partList);
    vector<string>::const_iterator it;
    for (it=result.begin(); it != result.end(); ++it)
        m_globals[*it] = pVector;

    return pVector;
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::add_left_data_from_global_measure(ImoMusicData* pMD)
{
    map<string, MeasuresVector*>::iterator it = m_globals.find(m_curPartId);
    if (it == m_globals.end())
        return;

    MeasuresVector* pMeasures = it->second;

    string curParent = get_parent_for_directions();
    int idx = m_measuresCounter - 1;
    ImoMusicData* pGlobal = nullptr;
    if (idx < int(pMeasures->size()))
    {
        XmlNode node = pMeasures->at(idx);
        XmlParser parser;
        MnxAnalyser a(m_reporter, m_libraryScope, m_pDoc, &parser);
        a.set_parent_for_directions("measure-global");
        pGlobal = a.analyse_global_measure(&node);
        split_global_content(pGlobal, pMD);
        transfer_data_from_temporal_analyser(a);
    }
    set_parent_for_directions(curParent);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::add_right_data_from_global_measure(ImoMusicData* pMD)
{
    for (auto pSO : m_rightGlobals)
        pMD->append_child_imo(pSO);

    m_rightGlobals.clear();
}

//---------------------------------------------------------------------------------------
ImoMusicData* MnxAnalyser::analyse_global_measure(XmlNode* pNode)
{
    //m_reporter << "DBG. Analysing node: " << pNode->name() << endl;
    set_result(nullptr);
    MeasureMnxAnalyser a(this, m_reporter, m_libraryScope, nullptr, true);
    a.analyse_node(pNode);
    return static_cast<ImoMusicData*>(get_imo_result()->pImo);
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::split_global_content(ImoMusicData* pGlobal, ImoMusicData* pMD)
{
    //content in pGlobal ImoMusicData is re-assigned: left located objects are
    //added to pMD and right located objects are saved in m_leftGlobals

    ImoObj::children_iterator it = pGlobal->begin();
    while (it != pGlobal->end())
    {
        ImoStaffObj* pSO = static_cast<ImoStaffObj*>(*it);
        pGlobal->remove_child(pSO);
        if (goes_at_right(pSO))
            m_rightGlobals.push_back(pSO);
        else
            pMD->append_child_imo(pSO);

        it = pGlobal->begin();
    }
    delete pGlobal;
}

//---------------------------------------------------------------------------------------
bool MnxAnalyser::goes_at_right(ImoStaffObj* pSO)
{
    if (pSO->is_direction() || pSO->is_sound_change())
    {
        AttrObj* pA = pSO->get_attribute(k_attr_right_located);
        if (pA)
        {
            pSO->remove_attribute(k_attr_right_located);
            return true;
        }
        return false;
    }
    else if (pSO->is_time_signature())
    {
        return false;
    }
    else
        return false;
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::transfer_data_from_temporal_analyser(MnxAnalyser& a)
{
    m_startRepeat = a.get_start_repeat_info();
    m_endRepeat = a.get_end_repeat_info();
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::save_repeat_info(const std::string& type, int times)
{
    if (type == "start")
        m_startRepeat = 1;
    else
        m_endRepeat = times;
}

//---------------------------------------------------------------------------------------
void MnxAnalyser::clear_repeat_info()
{
    m_startRepeat = 0;
    m_endRepeat = 0;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_start_repeat_info()
{
    return m_startRepeat;
}

//---------------------------------------------------------------------------------------
int MnxAnalyser::get_end_repeat_info()
{
    return m_endRepeat;
}

//---------------------------------------------------------------------------------------
vector<string> MnxAnalyser::tokenize_spaces(const string& input)
{
    char sep = ' ';
    string::size_type b = 0;
    vector<string> result;

    while ((b = input.find_first_not_of(sep, b)) != string::npos)
    {
        auto e = input.find_first_of(sep, b);
        result.push_back(input.substr(b, e-b));
        b = e;
    }
    return result;
}


//-----------------------------------------------------------------------------------
// Analysers for notational syntaxes
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//Note value
bool MnxAnalyser::get_note_value(const string& value, int* noteType, int* dots)
{
    *dots = 0;
    bool fFractional = false;
    string::const_iterator it = value.begin();

    //get initial '*' or '/'
    if (it != value.end())
    {
        if (*it == '*')
            fFractional = false;
        else if (*it == '/')
            fFractional = true;
        else
            return false;   //error_invalid_start();
        ++it;
    }

    if (it == value.end())
        return false;   //error_missing_number();

    //get number
    string::const_iterator start = it;
    ++it;
    while(it != value.end() && isdigit(*it))
    {
        ++it;
    }
    string number(start, it);

    //get dots: count and remove trailing 'd'
    while(it != value.end() && *it == 'd')
    {
        *dots += 1;
        ++it;
    }

    if (it != value.end())
        return false;   //error_invalid_chars_at_end();

    //determine note type
    if (number == "2" && !fFractional)
        *noteType = k_longa;
    else if (number == "1")
        *noteType = k_whole;
    else if (number == "2" && fFractional)
        *noteType = k_half;
    else if (number == "4")
        *noteType = k_quarter;
    else if (number == "8")
        *noteType = k_eighth;
    else if (number == "16")
        *noteType = k_16th;
    else if (number == "32")
        *noteType = k_32nd;
    else if (number == "64")
        *noteType = k_64th;
    else if (number == "128")
        *noteType = k_128th;
    else
        *noteType = k_256th;

//            s << ". Value=" << value << ", noteType=" << *noteType
//              << ", dots=" << *dots;
//            LOMSE_LOG_DEBUG(Logger::k_all, s.str());

    return true;
}

//-----------------------------------------------------------------------------------
//Note value quantity
bool MnxAnalyser::get_note_value_quantity(const string& value, int* noteType,
                                          int* dots, int* multiplier)
{
    *dots = 0;
    *multiplier = 1;
    string::const_iterator it = value.begin();

    //get initial digits until '/' found
    string::const_iterator start = it;
    while(it != value.end() && isdigit(*it))
    {
        ++it;
    }
    if (it == value.end() || (*it != '/'))
        return false;   //error_missing_note_value();

    string number(start, it);
    if (!number.empty())
    {
        long mult;
        std::istringstream iss(number);
        if ((iss >> std::dec >> mult).fail())
            return false;   //error_invalid_multiplier();

        *multiplier = int(mult);
    }


    //get note value
    string noteValue(it, value.end());

    return MnxAnalyser::get_note_value(noteValue, noteType, dots);
}

//-----------------------------------------------------------------------------------
//Note value quantity
bool MnxAnalyser::note_value_quantity_to_duration(const string& value, TimeUnits* duration)
{
    *duration = 0.0;

    int noteType;
    int dots;
    int multiplier;

    if (MnxAnalyser::get_note_value_quantity(value, &noteType, &dots, &multiplier))
    {
        *duration = to_duration(noteType, dots) * TimeUnits(multiplier);
        return true;
    }
    return false;   //error_invalid_value();
}

////-----------------------------------------------------------------------------------
////Time signature value
//bool MnxAnalyser::get_time_signature_value(const string& value, )
//{
//
//}
//
////-----------------------------------------------------------------------------------
////Chromatic pitch
//bool MnxAnalyser::get_chromatic_pitch_value(const string& value, )
//{
//
//}
//
////-----------------------------------------------------------------------------------
////Measure location
//bool MnxAnalyser::get_measure_location_value(const string& value, )
//{
//
//}
//
////-----------------------------------------------------------------------------------
////SMuFL glyph name
//bool MnxAnalyser::get_smufl_glyph_name(const string& value, )
//{
//
//}


////=======================================================================================
//// MnxTiesBuilder implementation
////=======================================================================================
//void MnxTiesBuilder::add_relation_to_staffobjs(ImoTieDto* pEndDto)
//{
//    ImoTieDto* pStartDto = m_matches.front();
//    ImoNote* pStartNote = pStartDto->get_note();
//    ImoNote* pEndNote = pEndDto->get_note();
//    if (notes_can_be_tied(pStartNote, pEndNote))
//        tie_notes(pStartDto, pEndDto);
//    else
//        error_notes_can_not_be_tied(pEndDto);
//}
//
////---------------------------------------------------------------------------------------
//bool MnxTiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
//{
//    return (pStartNote->get_voice() == pEndNote->get_voice())
//            && (pStartNote->get_staff() == pEndNote->get_staff())
//            && (pStartNote->get_actual_accidentals() == pEndNote->get_actual_accidentals())
//            && (pStartNote->get_step() == pEndNote->get_step())
//            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
//}
//
////---------------------------------------------------------------------------------------
//void MnxTiesBuilder::tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto)
//{
//    ImoNote* pStartNote = pStartDto->get_note();
//    ImoNote* pEndNote = pEndDto->get_note();
//    Document* pDoc = m_pAnalyser->get_document_being_analysed();
//
//    ImoTie* pTie = static_cast<ImoTie*>(
//                    ImFactory::inject(k_imo_tie, pDoc));
//    pTie->set_tie_number( pStartDto->get_tie_number() );
//    pTie->set_color( pStartDto->get_color() );
//    pTie->set_orientation( pStartDto->get_orientation() );
//
//    ImoTieData* pStartData = ImFactory::inject_tie_data(pDoc, pStartDto);
//    pStartNote->include_in_relation(pDoc, pTie, pStartData);
//
//    ImoTieData* pEndData = ImFactory::inject_tie_data(pDoc, pEndDto);
//    pEndNote->include_in_relation(pDoc, pTie, pEndData);
//
//    pStartNote->set_tie_next(pTie);
//    pEndNote->set_tie_prev(pTie);
//}
//
////---------------------------------------------------------------------------------------
//void MnxTiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
//{
//    m_reporter << "Line " << pEndInfo->get_line_number()
//               << ". Requesting to tie notes of different voice or pitch. Tie number "
//               << pEndInfo->get_tie_number()
//               << " will be ignored." << endl;
//}


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
