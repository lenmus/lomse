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

#include "lomse_mxl_analyser.h"

#include "lomse_xml_parser.h"
#include "lomse_ldp_exporter.h"

#include <iostream>
#include <sstream>
//BUG: In my Ubuntu box next line causes problems since approx. 20/march/2011
#if (LOMSE_PLATFORM_WIN32 == 1)
    #include <locale>
#endif
#include <vector>
#include <algorithm>   // for find
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
    if (m_partAdded[i])
        return true;    //if instrument is already marked!
    m_partAdded[i] = true;
    return false;
}

//---------------------------------------------------------------------------------------
ImoInstrument* PartList::get_instrument(const string& id)
{
	int i = find_index_for(id);
	return (i != -1 ? m_instruments[i] : NULL);
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
        return NULL;

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

    k_mxl_tag_articulations,
    k_mxl_tag_attributes,
    k_mxl_tag_backup,
    k_mxl_tag_barline,
    k_mxl_tag_clef,
    k_mxl_tag_direction,
    k_mxl_tag_dynamics,
    k_mxl_tag_fermata,
    k_mxl_tag_forward,
    k_mxl_tag_key,
    k_mxl_tag_lyric,
    k_mxl_tag_measure,
    k_mxl_tag_notations,
    k_mxl_tag_note,
    k_mxl_tag_ornaments,
    k_mxl_tag_part,
    k_mxl_tag_part_group,
    k_mxl_tag_part_list,
    k_mxl_tag_part_name,
    k_mxl_tag_pitch,
    k_mxl_tag_print,
    k_mxl_tag_rest,
    k_mxl_tag_score_part,
    k_mxl_tag_score_partwise,
    k_mxl_tag_slur,
    k_mxl_tag_sound,
    k_mxl_tag_technical,
    k_mxl_tag_text,
    k_mxl_tag_tied,
    k_mxl_tag_time,
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
                    ImoObj* pAnchor=NULL)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_libraryScope(libraryScope)
        , m_pLdpFactory(libraryScope.ldp_factory())
        , m_pAnchor(pAnchor) {}
    virtual ~MxlElementAnalyser() {}
    ImoObj* analyse_node(XmlNode* pNode);

protected:

    //analysis
    virtual ImoObj* do_analysis() = 0;

    //error reporting
    bool error_missing_element(const string& tag);
    void report_msg(int numLine, const std::string& msg);
    void report_msg(int numLine, const std::stringstream& msg);
    void error_if_more_elements();
    void error_invalid_child();
    void error_msg(const string& msg);
    void error_msg2(const string& msg);

    //helpers, to simplify writing grammar rules
    XmlNode m_analysedNode;
    XmlNode m_childToAnalyse;
    XmlNode m_nextParam;
    XmlNode m_nextNextParam;

    // the main method to perform the analysis of a node
    inline ImoObj* analyse_child() { return m_pAnalyser->analyse_node(&m_childToAnalyse, NULL); }

    // 'get' methods just update m_childToAnalyse to point to the next node to analyse
    bool get_mandatory(const string& tag);
//    bool get_optional(EMxlTag type);
    bool get_optional(const string& type);

    // 'analyse' methods do a 'get' and, if found, analyse the found element
    bool analyse_mandatory(const string& tag, ImoObj* pAnchor=NULL);
//    bool analyse_optional(EMxlTag type, ImoObj* pAnchor=NULL);
    bool analyse_optional(const string& name, ImoObj* pAnchor=NULL);
//    void analyse_one_or_more(EMxlTag* pValid, int nValid);
//    void analyse_staffobjs_options(ImoStaffObj* pSO);
//    void analyse_scoreobj_options(ImoScoreObj* pSO);

    //methods to analyse attributes of current node
    bool has_attribute(const string& name);
    string get_attribute(const string& name);
    int get_attribute_as_integer(const string& name, int nNumber);
    string get_mandatory_string_attribute(const string& name, const string& sDefault,
                                          const string& element);
    string get_optional_string_attribute(const string& name, const string& sDefault);
    int get_mandatory_integer_attribute(const string& name, int nDefault,
                                        const string& element);
    int get_optional_integer_attribute(const string& name, int nDefault);

    //building the model
    void add_to_model(ImoObj* pImo, int type=-1);

    //auxiliary
    inline const string& get_document_locator() {
        return m_pAnalyser->get_document_locator();
    }



    //-----------------------------------------------------------------------------------
    //XmlNode helper methods
    inline bool has_attribute(XmlNode* node, const string& name)
    {
        return node->attribute(name.c_str()) != NULL;
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

//    //-----------------------------------------------------------------------------------
//    void get_num_staff()
//    {
//        string staff = m_childToAnalyse.value();
//        int nStaff;
//        //http://www.codeguru.com/forum/showthread.php?t=231054
//        std::istringstream iss(staff);
//        if ((iss >> std::dec >> nStaff).fail())
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                "Invalid staff '" + staff + "'. Replaced by '1'.");
//            m_pAnalyser->set_current_staff(0);
//        }
//        else
//            m_pAnalyser->set_current_staff(--nStaff);
//    }

    //-----------------------------------------------------------------------------------
    bool is_long_value()
    {
        string number = m_childToAnalyse.value();
        long nNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> nNumber).fail());
    }

    //-----------------------------------------------------------------------------------
    long get_long_value(long nDefault=0L)
    {
        string number = m_childToAnalyse.value();
        long nNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> nNumber).fail())
        {
            stringstream replacement;
            replacement << nDefault;
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Invalid integer number '" + number + "'. Replaced by '"
                + replacement.str() + "'.");
            return nDefault;
        }
        else
            return nNumber;
    }

    //-----------------------------------------------------------------------------------
    int get_integer_value(int nDefault)
    {
        return static_cast<int>( get_long_value(static_cast<int>(nDefault)) );
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
    float get_float_value(float rDefault=0.0f)
    {
        string number = m_childToAnalyse.value();
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

    //-----------------------------------------------------------------------------------
    bool is_bool_value()
    {
        string value = string(m_childToAnalyse.value());
        return  value == "true" || value == "yes"
             || value == "false" || value == "no" ;
    }

    //-----------------------------------------------------------------------------------
    bool get_bool_value(bool fDefault=false)
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
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Invalid boolean value '" + value + "'. Replaced by '"
                + replacement.str() + "'.");
            return fDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    int get_yes_no_value(int nDefault)
    {
        string value = m_childToAnalyse.value();
        if (value == "yes")
            return k_yesno_yes;
        else if (value == "no")
            return k_yesno_no;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Invalid yes/no value '" + value + "'. Replaced by default.");
            return nDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    string get_string_value()
    {
        return m_childToAnalyse.value();
    }

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

//    //-----------------------------------------------------------------------------------
//    Color get_color_child()
//    {
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, NULL);
//        Color color;
//        if (pImo->is_color_dto())
//        {
//            ImoColorDto* pColor = static_cast<ImoColorDto*>( pImo );
//            color = pColor->get_color();
//        }
//        delete pImo;
//        return color;
//    }
//
//    //-----------------------------------------------------------------------------------
//    Color get_color_value()
//    {
//        string value = m_childToAnalyse.value();
//        ImoColorDto* pColor = LOMSE_NEW ImoColorDto();
//        pColor->set_from_string(value);
//        if (!pColor->is_ok())
//        {
//            error_msg("Missing or invalid color value. Must be #rrggbbaa. Color ignored.");
//            delete pColor;
//            return Color(0,0,0);
//        }
//        Color color = pColor->get_color();
//        delete pColor;
//        return color;
//    }
//
//    //-----------------------------------------------------------------------------------
//    float get_font_size_value()
//    {
//        const string value = m_childToAnalyse.value();
//        int size = static_cast<int>(value.size()) - 2;
//        string points = value.substr(0, size);
//        string number = m_childToAnalyse.value();
//        float rNumber;
//        std::istringstream iss(number);
//        if ((iss >> std::dec >> rNumber).fail())
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                "Invalid size '" + number + "'. Replaced by '12'.");
//            return 12.0f;
//        }
//        else
//            return rNumber;
//    }
//
//    //-----------------------------------------------------------------------------------
//    ImoStyle* get_text_style_child(const string& defaulName="Default style")
//    {
//        m_childToAnalyse = get_child(m_childToAnalyse, 1);
//        string styleName = get_string_value();
//        ImoStyle* pStyle = NULL;
//
//        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
//        if (pScore)
//        {
//            pStyle = pScore->find_style(styleName);
//            if (!pStyle)
//            {
//                //try to find it in document global styles
//                Document* pDoc = m_pAnalyser->get_document_being_analysed();
//                ImoDocument* pImoDoc = pDoc->get_imodoc();
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
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, NULL);
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
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, NULL);
//        TSize size;
//        if (pImo->is_size_info())
//        {
//            ImoSizeDto* pSize = static_cast<ImoSizeDto*>( pImo );
//            size = pSize->get_size();
//        }
//        delete pImo;
//        return size;
//    }
//
//    //-----------------------------------------------------------------------------------
//    float get_location_child()
//    {
//        return get_float_value(0.0f);
//    }
//
//    //-----------------------------------------------------------------------------------
//    float get_width_child(float rDefault=1.0f)
//    {
//        return get_float_value(rDefault);
//    }
//
//    //-----------------------------------------------------------------------------------
//    float get_height_child(float rDefault=1.0f)
//    {
//        return get_float_value(rDefault);
//    }

    //-----------------------------------------------------------------------------------
    float get_float_child(float rDefault=1.0f)
    {
        return get_float_value(rDefault);
    }

//    //-----------------------------------------------------------------------------------
//    float get_lenght_child(float rDefault=0.0f)
//    {
//        return get_float_value(rDefault);
//    }
//
//    //-----------------------------------------------------------------------------------
//    ImoStyle* get_doc_text_style(const string& styleName)
//    {
//        ImoStyle* pStyle = NULL;
//
//        ImoDocument* pDoc = m_pAnalyser->get_root_imo_document();
//        if (pDoc)
//        {
//            pStyle = pDoc->find_style(styleName);
//            if (!pStyle)
//            {
//                report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                        "Style '" + styleName + "' is not defined. Default style will be used.");
//                pStyle = pDoc->get_style_or_default(styleName);
//            }
//        }
//
//        return pStyle;
//    }
//
//    //-----------------------------------------------------------------------------------
//    ELineStyle get_line_style_child()
//    {
//        m_childToAnalyse = get_child(m_childToAnalyse, 1);
//        const std::string& value = m_childToAnalyse.value();
//        if (value == "none")
//            return k_line_none;
//        else if (value == "dot")
//            return k_line_dot;
//        else if (value == "solid")
//            return k_line_solid;
//        else if (value == "longDash")
//            return k_line_long_dash;
//        else if (value == "shortDash")
//            return k_line_short_dash;
//        else if (value == "dotDash")
//            return k_line_dot_dash;
//        else
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                "Element 'lineStyle': Invalid value '" + value
//                + "'. Replaced by 'solid'." );
//            return k_line_solid;
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    ELineCap get_line_cap_child()
//    {
//        m_childToAnalyse = get_child(m_childToAnalyse, 1);
//        const std::string& value = m_childToAnalyse.value();
//        if (value == "none")
//            return k_cap_none;
//        else if (value == "arrowhead")
//            return k_cap_arrowhead;
//        else if (value == "arrowtail")
//            return k_cap_arrowtail;
//        else if (value == "circle")
//            return k_cap_circle;
//        else if (value == "square")
//            return k_cap_square;
//        else if (value == "diamond")
//            return k_cap_diamond;
//        else
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                "Element 'lineCap': Invalid value '" + value
//                + "'. Replaced by 'none'." );
//            return k_cap_none;
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    void check_visible(ImoInlinesContainer* pCO)
//    {
//        string value = m_childToAnalyse.value();
//        if (value == "visible")
//            pCO->set_visible(true);
//        else if (value == "noVisible")
//            pCO->set_visible(false);
//        else
//        {
//            error_invalid_child();
//            pCO->set_visible(true);
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    NoteTypeAndDots get_note_type_and_dots()
//    {
//        string duration = m_childToAnalyse.value();
//        NoteTypeAndDots figdots = ldp_duration_to_components(duration);
//        if (figdots.noteType == k_unknown_notetype)
//        {
//            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
//                "Unknown note/rest duration '" + duration + "'. Replaced by 'q'.");
//            figdots.noteType = k_quarter;
//        }
//        return figdots;
//    }
//
//    //-----------------------------------------------------------------------------------
//    void analyse_attachments(ImoStaffObj* pAnchor)
//    {
//        while( more_children_to_analyse() )
//        {
//            m_childToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_childToAnalyse);
//            if (is_auxobj(type))
//                m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
//            else
//                error_invalid_child();
//
//            move_to_next_child();
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    bool is_auxobj(int type)
//    {
//        return     type == k_beam
//                || type == k_text
//                || type == k_textbox
//                || type == k_line
//                || type == k_fermata
//                || type == k_tie
//                || type == k_tuplet
//                ;
//    }
//
//    //-----------------------------------------------------------------------------------
//    ImoInlineLevelObj* analyse_inline_object()
//    {
//        // { <inlineWrapper> | <link> | <textItem> | <image> | <button> | <string> }
//
//        if(more_children_to_analyse())
//        {
//            m_childToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_childToAnalyse);
//            if (   /*type == k_inlineWrapper
//                ||*/ type == k_txt
//                || type == k_image
//                || type == k_link
//               )
//            {
//                return static_cast<ImoInlineLevelObj*>(
//                    m_pAnalyser->analyse_node(&m_childToAnalyse, NULL) );
//            }
//            else if (type == k_string)
//            {
//                //string: implicit <txt>
//                Document* pDoc = m_pAnalyser->get_document_being_analysed();
//                ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                            ImFactory::inject(k_imo_text_item, pDoc) );
//                pText->set_text( string(m_childToAnalyse.value()) );
//                return pText;
//            }
//            else
//                error_invalid_child();
//
//            move_to_next_child();
//        }
//        return NULL;
//    }

//    //-----------------------------------------------------------------------------------
//    void analyse_optional_style(ImoContentObj* pParent)
//    {
//        // [<style>]
//        ImoStyle* pStyle = NULL;
//        if (has_attribute("style"))
//            pStyle = get_doc_text_style( get_attribute("style") );
//        pParent->set_style(pStyle);
//    }

//    //-----------------------------------------------------------------------------------
//    void analyse_inline_objects(ImoInlinesContainer* pParent)
//    {
//        // <inlineObject>*
//        while( more_children_to_analyse() )
//        {
//            ImoInlineLevelObj* pItem = analyse_inline_object();
//            if (pItem)
//                pParent->add_item(pItem);
//
//            move_to_next_child();
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    void analyse_inline_or_block_objects(ImoBlocksContainer* pParent)
//    {
//        // {<inlineObject> | <blockObject>}*
//        while (more_children_to_analyse())
//        {
//            m_childToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_childToAnalyse);
//
//            if (
//               // inline: { <inlineWrapper> | <link> | <textItem> | <image> | <button> }
//                /*type == k_inlineWrapper
//                ||*/ type == k_txt
//                || type == k_image
//                || type == k_link
//               // block:  { <list> | <para> | <score> | <table> }
//                || type == k_itemizedlist
//                || type == k_orderedlist
//                || type == k_para
//                || type == k_table
//                || type == k_score
//               )
//            {
//                m_pAnalyser->analyse_node(&m_childToAnalyse, pParent);
//            }
//            else if (type == k_string)
//            {
//                //string: implicit <txt>
//                Document* pDoc = m_pAnalyser->get_document_being_analysed();
//                ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                            ImFactory::inject(k_imo_text_item, pDoc) );
//                pText->set_text( string(m_childToAnalyse.value()) );
//                ImoObj* pSave = m_pAnchor;
//                m_pAnchor = pParent;
//                add_to_model(pText);
//                m_pAnchor = pSave;
//            }
//            else
//                error_invalid_child();
//
//            move_to_next_child();
//        }
//    }

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
        return nNumber;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_optional_integer_attribute(const string& name,
                                                       int nDefault)
{
    if (has_attribute(&m_analysedNode, name))
        return get_attribute_as_integer(name, nDefault);
    else
        return nDefault;
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
bool MxlElementAnalyser::get_mandatory(const string& tag)
{
    if (!more_children_to_analyse())
    {
        error_missing_element(tag);
        return NULL;
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
        return (m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor) != NULL);
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
void MxlElementAnalyser::error_if_more_elements()
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
    }
}

////---------------------------------------------------------------------------------------
//void MxlElementAnalyser::analyse_staffobjs_options(ImoStaffObj* pSO)
//{
//    //@----------------------------------------------------------------------------
//    //@ <staffobjOptions> = { <staffNum> | <componentOptions> }
//    //@ <numStaff> = pn
//
//    // [<numStaff>]
//    if (more_children_to_analyse())
//    {
//        m_childToAnalyse = get_child_to_analyse();
//        if (m_childToAnalyse.name() == "staff")
//        {
//            get_num_staff();
//            move_to_next_child();
//        }
//    }
//
//    //set staff: either found value or inherited one
//    pSO->set_staff( m_pAnalyser->get_current_staff() );
//
//    analyse_scoreobj_options(pSO);
//}
//
////---------------------------------------------------------------------------------------
//void MxlElementAnalyser::analyse_scoreobj_options(ImoScoreObj* pSO)
//{
//    //@----------------------------------------------------------------------------
//    //@ <componentOptions> = { <visible> | <location> | <color> }
//    //@ <visible> = (visible {yes | no})
//    //@ <location> = { (dx num) | (dy num) }    ;num in Tenths
//    //@ <color> = value                         ;value in #rrggbb hex format
//
//    // [ { <visible> | <location> | <color> }* ]
//    while( more_children_to_analyse() )
//    {
//        m_childToAnalyse = get_child_to_analyse();
//        ELdpElement type = get_type(m_childToAnalyse);
//        switch (type)
//        {
//            case k_visible:
//            {
//                m_childToAnalyse = get_child(m_childToAnalyse, 1);
//                pSO->set_visible( get_bool_value(true) );
//                break;
//            }
//            case k_color:
//            {
//                pSO->set_color( get_color_child() );
//                break;
//            }
//            case k_dx:
//            {
//                pSO->set_user_location_x( get_location_child() );
//                break;
//            }
//            case k_dy:
//            {
//                pSO->set_user_location_y( get_location_child() );
//                break;
//            }
//            default:
//                return;
//        }
//
//        move_to_next_child();
//    }
//}

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
        cout << "Line " << m_pAnalyser->get_line_number(&m_analysedNode)
             << ". Missing analyser for element '" << m_tag
             << "'. Node ignored." << endl;
        return NULL;
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
        ImoNoteRest* pNR = NULL;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is NULL or it is not ImoNoteRest");
            return NULL;
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

        return NULL;
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoArticulationSymbol* get_articulation_symbol(ImoNoteRest* pNR, int type)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoArticulationSymbol* pImo = static_cast<ImoArticulationSymbol*>(
                                ImFactory::inject(k_imo_articulation_symbol, pDoc) );
        pImo->set_articulation_type(type);

        // [atrrib]: placement (above | below)
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

        // [atrrib]: type (up | down)
        if (has_attribute(&m_childToAnalyse, "type"))
            set_type(pImo);
    }

    //-----------------------------------------------------------------------------------
    void get_articulation_breath_mark(ImoNoteRest* pNR)
    {
        ImoArticulationSymbol* pImo =
            get_articulation_symbol(pNR, k_articulation_breath_mark);

        // [atrrib]: type (up | down)
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

        // [atrrib]: placement (above | below)
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
//@ <!ELEMENT attributes (%editorial;, divisions?, key*, time*,
//@     staves?, part-symbol?, instruments?, clef*, staff-details*,
//@     transpose*, directive*, measure-style*)>
//@
//@ Doc:    The attributes element contains musical information that typically changes
//@         on measure boundaries. This includes key and time signatures, clefs,
//@         transpositions, and staving.

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

        // [<divisions>]
        if (get_optional("divisions"))
            set_divisions();

        // <key>*
        while (get_optional("key"))
            keys.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, NULL) );

        // <time>*
        while (get_optional("time"))
            times.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, NULL) );

        // [<staves>]
        if (get_optional("staves"))
        {
            int staves = get_integer_value(1);
            ImoInstrument* pInstr = dynamic_cast<ImoInstrument*>(m_pAnchor->get_parent_imo());
            for(; staves > 1; --staves)
                pInstr->add_staff();
        }

        // [<part-symbol>]
        if (get_optional("part-symbol"))
        {
            //TODO <part-symbol>
        }

        // [<instruments>]
        if (get_optional("instruments"))
        {
            //TODO <instruments>
        }

        // <clef>*
        while (get_optional("clef"))
            clefs.push_back( m_pAnalyser->analyse_node(&m_childToAnalyse, NULL) );

        // <staff-details>*
        while (get_optional("staff-details"))
            ; //TODO <staff-details>

        // <transpose>*
        while (get_optional("transpose"))
            ; //TODO <transpose>

        // <directive>*
        while (get_optional("directive"))
            ; //TODO <directive>

        // <measure-style>*
        while (get_optional("measure-style"))
            ; //TODO <measure-style>

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

        int divisions = get_integer_value(4);
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
public:
    BarlineMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        //ImoMusicData* pMD = dynamic_cast<ImoMusicData*>(m_pAnchor);

        //attributes:

        // location (right | left | middle) "right"
        string location = get_optional_string_attribute("location", "right");

        //children elements:

        // bar-style?
        string barStyle = "";
        if (get_optional("bar-style"))
            barStyle = m_childToAnalyse.value();

        //TODO
        // %editorial;
        // wavy-line?
        // segno?
        // coda?
        // (fermata, fermata?)?

        //TODO
        // ending?
        if (get_optional("ending"))
        {
        }

        // repeat?
        string repeat = "";
        if (get_optional("repeat"))
            repeat = get_repeat();

        error_if_more_elements();

        //if no bar-style, do not create a barline.
        if (barStyle.empty())
            return NULL;

        ImoBarline* pBarline = NULL;
        if (location == "left")
        {
            //must be combined with previous barline
            pBarline = m_pAnalyser->get_last_barline();
        }

        bool fNewBarline = false;
        if (pBarline == NULL)
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            pBarline = static_cast<ImoBarline*>(
                                ImFactory::inject(k_imo_barline, pDoc) );
            pBarline->set_type(k_barline_simple);
            fNewBarline = true;
        }

        EBarline type = find_barline_type(barStyle, repeat);
        combine_barlines(pBarline, type);

        if (fNewBarline)
        {
            advance_timepos_if_required();
            add_to_model(pBarline);
            m_pAnalyser->save_last_barline(pBarline);
        }

        return pBarline;
    }

protected:

    EBarline find_barline_type(const string& barType, const string& repeat)
    {
        bool fError = false;
        EBarline type = k_barline_simple;

        if (barType == "regular")
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
//        else if (barType == "heavy-heavy")
//            type = ?
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
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Invalid or not supported <bar-style> ('" + barType
                + "') and/or <repeat> ('" + repeat
                + "') values. Replaced by 'regular' barline.");
        }

        return type;
    }

    // <!ELEMENT repeat EMPTY>
    // <!ATTLIST repeat
    //     direction (backward | forward) #REQUIRED
    //     times CDATA #IMPLIED
    //     winged (none | straight | curved |
    //         double-straight | double-curved) #IMPLIED
    // >
    string get_repeat()
    {
        //attrb: direction
        string direction = "";
        if (has_attribute(&m_childToAnalyse, "direction"))
            direction = m_childToAnalyse.attribute_value("direction");
        return direction;
    }

    void combine_barlines(ImoBarline* pBarline, EBarline rightType)
    {
        EBarline type;
        EBarline leftType = EBarline(pBarline->get_type());

        if (leftType == k_barline_simple && rightType == k_barline_simple)
            type = k_barline_double;
        else if (leftType == k_barline_simple)
            type = rightType;
        else if (rightType == k_barline_simple)
            type = leftType;
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
//@ <clef> = <sign>[<line>][<clef-octave-change>]
//@ attrb: none is mandatory:
//    number  	    staff-number  	The optional number attribute refers to staff numbers
//                                  within the part. A value of 1 is assumed if not present.
//    additional  	yes-no  	    Sometimes clefs are added to the staff in non-standard
//                                  line positions, either to indicate cue passages, or
//                                  when there are multiple clefs present simultaneously
//                                  on one staff. In this situation, the additional
//                                  attribute is set to "yes" and the line value is ignored.
//    size  	    symbol-size  	The size attribute is used for clefs where the additional
//                                  attribute is "yes". It is typically used to indicate
//                                  cue clefs. The after-barline attribute is set to "yes"
//                                  in this situation. The attribute is ignored for
//                                  mid-measure clefs.
//    after-barline yes-no  	    Sometimes clefs at the start of a measure need to
//                                  appear after the barline rather than before, as for
//                                  cues or for use after a repeated section.
//    default-x  	tenths
//    default-y  	tenths
//    relative-x  	tenths
//    relative-y  	tenths
//    font-family  	comma-separated-text
//    font-style  	font-style
//    font-size  	font-size
//    font-weight  	font-weight
//    color  	    color
//    print-object 	yes-no

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
        , m_line(0), m_octaveChange(0)
        {}


    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoClef* pClef = static_cast<ImoClef*>( ImFactory::inject(k_imo_clef, pDoc) );

        //attributes:

        // staff number
        int nStaffNum = get_optional_integer_attribute("number", 1);
        pClef->set_staff(nStaffNum - 1);

        //content:

        // <sign>
        if (get_optional("sign"))
            m_sign = get_string_value();    //m_childToAnalyse.value();

        if (get_optional("line"))
            m_line = get_integer_value(0);   //(m_childToAnalyse);

        if (get_optional("clef-octave-change"))
            m_octaveChange = get_integer_value(0);   //(m_childToAnalyse);

        int type = determine_clef_type();
        if (type == k_clef_undefined)
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                    "Unknown clef '" + m_sign + "'. Assumed 'G' in line 2.");
            type = k_clef_G2;
        }
        pClef->set_clef_type(type);

//        // [<symbolSize>]
//        if (get_optional(k_symbolSize))
//            set_symbol_size(pClef);
//
//        // [<staff>][visible][<location>]
//        analyse_staffobjs_options(pClef);

        error_if_more_elements();

        add_to_model(pClef);
        return pClef;
    }

protected:

    int determine_clef_type()
    {
        //int clef = MxlAnalyser::xml_data_to_clef_type(m_sign, m_line, m_octaveChange);

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
//@ direction

class DirectionMxlAnalyser : public MxlElementAnalyser
{
public:
    DirectionMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        //TODO
        return NULL;
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
    DynamicsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoNoteRest* pNR = NULL;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is NULL or it is not ImoNoteRest");
            return NULL;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>(
                                ImFactory::inject(k_imo_dynamics_mark, pDoc) );

        // atrrib: placement
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

        pNR->add_attachment(pDoc, pImo);
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
        ImoNoteRest* pNR = NULL;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is NULL or it is not ImoNoteRest");
            return NULL;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoFermata* pImo = static_cast<ImoFermata*>(
                                ImFactory::inject(k_imo_fermata, pDoc) );

        // atrrib: type (upright | inverted) #IMPLIED
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
        ImoStaffObj* pSO = NULL;

        // <duration>
        if (!get_mandatory("duration"))
            return NULL;
        int duration = get_integer_value(0);
        TimeUnits shift = m_pAnalyser->duration_to_timepos(duration);

        //<voice>
        if (fFwd && get_optional("voice"))
        {
            int voice = get_integer_value( m_pAnalyser->get_current_voice() );

            // staff?
            int staff = 1;
            if (get_optional("staff"))
                staff = get_integer_value(1) - 1;

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
            fifths = get_integer_value(0);

        // <mode>
        if (get_optional("mode"))
            fMajor = (get_string_value() == "major");


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
                    string msg = str( boost::format(
                                        "Invalid number of fifths %d")
                                        % fifths );
                    error_msg(msg);
    //                LOMSE_LOG_ERROR(msg);
    //                throw runtime_error(msg);
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
                    string msg = str( boost::format(
                                        "Invalid number of fifths %d")
                                        % fifths );
                    error_msg(msg);
    //                LOMSE_LOG_ERROR(msg);
    //                throw runtime_error(msg);
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
        ImoNote* pNote = NULL;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is NULL or it is not ImoNote");
            return NULL;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLyric* pData = static_cast<ImoLyric*>(
                                    ImFactory::inject(k_imo_lyric, pDoc) );

        // atrrib: number
        int num = 1;
        if (has_attribute("number"))
            num = get_attribute_as_integer("number", 1);
        pData->set_number(num);

        // atrrib: type (upright | inverted) #IMPLIED
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
            return NULL;
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
        ImoMusicData* pMD = dynamic_cast<ImoMusicData*>(m_pAnchor);
        bool fSomethingAdded = false;

        //attrb: number #REQUIRED
        string num = get_optional_string_attribute("number", "");
        if (num.empty())
        {
            error_msg("<measure>: missing mandatory 'number' attribute. <measure> content will be ignored");
            return NULL;
        }
        m_pAnalyser->save_current_measure_num(num);

        // [{<xxxx>|<yyyy>|<zzzz>}*]    alternatives: zero or more
        while (more_children_to_analyse())
        {
            if (analyse_optional("attributes", pMD))
                fSomethingAdded = true;
            else if (analyse_optional("barline", pMD))
                fSomethingAdded = true;
            else if (analyse_optional("direction", pMD))
                //TODO: add fSomethingAdded = true;  when direction analyser coded
                ;
            else if (analyse_optional("note", pMD))
                fSomethingAdded = true;
            else if (analyse_optional("forward", pMD))
                fSomethingAdded = true;
            else if (analyse_optional("backup", pMD))
                fSomethingAdded = true;
            else if (analyse_optional("print"))
                //TODO: add fSomethingAdded = true;  when print analyser coded
                ;
            else if (analyse_optional("sound", pMD))
                //TODO: add fSomethingAdded = true;  when sound analyser coded
                ;
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
            if (pSO == NULL || !pSO->is_barline())
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
//        m_childToAnalyse = get_child_to_analyse();
//        const string& name = m_childToAnalyse.name();
//        if (name == "tuplet")
//        {
//            ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, NULL);
//            m_pTupletInfo = static_cast<ImoTupletDto*>( pImo );
//        }
//            if (name == "tuplet")
//            {
//                ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, NULL);
//                m_pTupletInfo = static_cast<ImoTupletDto*>( pImo );
//            }
//            else
//                break;

        return NULL;
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
//    ImoTupletDto* m_pTupletInfo;
    ImoBeamDto* m_pBeamInfo;
//    ImoSlurDto* m_pSlurDto;
//    std::string m_srcOldTuplet;

public:
    NoteRestMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
//        , m_pTupletInfo(NULL)
        , m_pBeamInfo(NULL)
//        , m_pSlurDto(NULL)
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
            return NULL;

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
        ImoNoteRest* pNR = NULL;
        ImoNote* pNote = NULL;
        ImoRest* pRest = NULL;

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
            duration = get_integer_value(0);

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
            voice = get_integer_value( voice );
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
        if (get_optional("time-modification"))
        {
        }

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
            set_staff(pNR);

        // <beam>*
        while (get_optional("beam"))
            analyse_beam();
        add_beam_info(pNR);

        // <notations>*
        while (analyse_optional("notations", pNR));

        // <lyric>*
        while (analyse_optional("lyric", pNR));

        // [<play>]
        if (get_optional("play"))
        {
        }

        error_if_more_elements();

        add_to_model(pNR);

//        //tuplet
//        if (m_pTupletInfo==NULL && m_pAnalyser->is_tuplet_open())
//            add_to_current_tuplet(pNR);
//
//        add_tuplet_info(pNR);

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
            noteType = get_note_type(type);
        else if (pNR->is_rest() && static_cast<ImoRest*>(pNR)->is_full_measure())
        {
            dots = 0;
            noteType = k_whole;
        }
        else
        {
            //<type> is not required in multi-metric rests. And, in any
            //case it is not mandatory. If not present, <type>
            //must be derived from <duration>.
            if (is_equal_time(units, k_duration_longa))
                noteType = k_longa;
            else if (is_equal_time(units, k_duration_whole))
                noteType = k_whole;
            else if (is_equal_time(units, k_duration_half))
                noteType = k_half;
            else if (is_equal_time(units, k_duration_quarter))
                noteType = k_duration_quarter;
            else if (is_equal_time(units, k_duration_eighth))
                noteType = k_duration_eighth;
            else if (is_equal_time(units, k_duration_16th))
                noteType = k_duration_16th;
            else if (is_equal_time(units, k_duration_32nd))
                noteType = k_duration_32nd;
            else if (is_equal_time(units, k_duration_64th))
                noteType = k_duration_64th;
            else if (is_equal_time(units, k_duration_128th))
                noteType = k_duration_128th;
            else
                noteType = k_duration_256th;
        }

        pNR->set_type_dots_duration(noteType, dots, units);
    }

    //----------------------------------------------------------------------------------
    int get_note_type(const string& type)
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
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Invalid or not supported <type> value '" + type + "'. Replaced by 'eighth'.");
            noteType = k_eighth;
        }
        return noteType;
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
        string acc = m_childToAnalyse.value();  //get_string_value();
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
    }

    //----------------------------------------------------------------------------------
    void set_staff(ImoNoteRest* pNR)
    {
        int i = get_integer_value(1) - 1;
        pNR->set_staff(i);
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
//    void analyse_note_rest_options(ImoNoteRest* pNR)
//    {
//        // { <beam> | <tuplet> | }
//
//        while( more_children_to_analyse() )
//        {
//            m_childToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_childToAnalyse);
//            if (type == k_tuplet)
//            {
//                ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, NULL);
//                m_pTupletInfo = static_cast<ImoTupletDto*>( pImo );
//            }
//            else
//                break;
//
//            move_to_next_child();
//        }
//    }

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

        if (m_pBeamInfo == NULL)
            m_pBeamInfo = LOMSE_NEW ImoBeamDto();

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
//    void add_to_current_tuplet(ImoNoteRest* pNR)
//    {
//        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
//        pInfo->set_note_rest(pNR);
//        pInfo->set_tuplet_type(ImoTupletDto::k_continue);
//        m_pAnalyser->add_relation_info(pInfo);
//    }
//

    //----------------------------------------------------------------------------------
    void set_voice(ImoNoteRest* pNR, int voice)
    {
        m_pAnalyser->set_current_voice(voice);
        pNR->set_voice(voice);
    }

    //----------------------------------------------------------------------------------
//    void add_tuplet_info(ImoNoteRest* pNR)
//    {
//        if (m_pTupletInfo)
//        {
//            m_pTupletInfo->set_note_rest(pNR);
//            m_pAnalyser->add_relation_info(m_pTupletInfo);
//        }
//    }


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
            return NULL;
        }
        ImoInstrument* pInstr = m_pAnalyser->get_instrument(id);
        if (pInstr==NULL)
        {
            error_msg("No <score-part> found for part id='" + id + "'. <part> content will be ignored.");
            return NULL;
        }
        if (m_pAnalyser->mark_part_as_added(id))
        {
            error_msg("Duplicated <part> for part id='" + id + "'. <part> content will be ignored.");
            return NULL;
        }

        m_pAnalyser->save_current_part_id(id);
        m_pAnalyser->prepare_for_new_instrument_content();
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
        ImoNoteRest* pNR = NULL;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is NULL or it is not ImoNoteRest");
            return NULL;
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

        return NULL;
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoOrnament* get_ornament_symbol(ImoNoteRest* pNR, int type)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoOrnament* pImo = static_cast<ImoOrnament*>(
                                ImFactory::inject(k_imo_ornament, pDoc) );
        pImo->set_ornament_type(type);

        // [atrrib]: placement (above | below)
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

//        // [atrrib]: type (up | down)
//        if (has_attribute(&m_childToAnalyse, "type"))
//            set_type(pImo);
    }

    //-----------------------------------------------------------------------------------
    void get_accidental_mark(ImoNoteRest* UNUSED(pNR))
    {
//        ImoOrnament* pImo =
//            get_ornament_symbol(pNR, k_ornament_breath_mark);
//
//        // [atrrib]: type (up | down)
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
            return NULL;
        }

        //attrb: type = "start | stop"
        string type = get_optional_string_attribute("type", "");
        if (type.empty())
        {
            error_msg("<part-group>: missing mandatory 'type' attribute. Tag ignored.");
            return NULL;
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
                return NULL;
            }
        }

        if (type != "start")
        {
            error_msg("<part-group>: invalid mandatory 'type' attribute. Must be "
                      "'start' or 'stop'.");
            return NULL;
        }

        ImoInstrGroup* pGrp = m_pAnalyser->start_part_group(number);
        if (pGrp == NULL)
        {
            error_msg("<part-group> type=start for number already started and not stopped");
            return NULL;
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

        return NULL;
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
            ImoStyle* pStyle = NULL;
            if (pScore)     //in unit tests the score might not exist
                pStyle = pScore->get_default_style();
            pText->set_style(pStyle);
            return pText;
        }
        return NULL;
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

        return NULL;
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
                ImoStyle* pStyle = NULL;
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

        return NULL;
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

        // <step>
        string step = (get_mandatory("step") ? m_childToAnalyse.value() : "C");

        // [<alter>]
        string accidentals = (get_optional("alter") ? m_childToAnalyse.value() : "0");

        // <octave>
        string octave = (get_mandatory("octave") ? m_childToAnalyse.value() : "4");

        error_if_more_elements();

        ImoNote* pNote = dynamic_cast<ImoNote*>(m_pAnchor);
        int nStep = mxl_step_to_step(step);
        EAccidentals nAcc = mxl_alter_to_accidentals(accidentals);
        int nOctave = mxl_octave_to_octave(octave);
        pNote->set_step(nStep);
        pNote->set_octave(nOctave);
        pNote->set_actual_accidentals(nAcc);
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

    EAccidentals mxl_alter_to_accidentals(const string& accidentals)
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

        //TODO: only integer accidentals -2..+2 supported. Modify for more.

        long nNumber;
        std::istringstream iss(accidentals);
        if ((iss >> std::dec >> nNumber).fail() || nNumber > 2 || nNumber < -2)
        {
            //report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            error_msg2(
                "Invalid or not supported <alter> value '" + accidentals + "'. Ignored.");
            return k_no_accidentals;
        }

        switch(nNumber)
        {
            case 0:
                return k_no_accidentals;
            case 1:
                return k_sharp;
            case 2:
                return k_double_sharp;
            case -1:
                return k_flat;
            case -2:
                return k_flat_flat;
        }
        return k_no_accidentals;        //should not reach this
    }

};

//@--------------------------------------------------------------------------------------
//@ print

class PrintMxlAnalyser : public MxlElementAnalyser
{
public:
    PrintMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        //TODO
        return NULL;
    }
};


//@--------------------------------------------------------------------------------------
//@ <score-part> = [<identification>]<part-name>[<part-name-display>][<part-abbreviation>]
//@                [<part-abbreviation-display>]<group>*
//@                <score-instrument>* { [<midi-device>][<midi-instrument>] }*
//@ Attrb:  name="id" type="xs:ID" use="required"
//@ Doc: Each <score-part> defines a MIDI track (one instrument)
//@      The score-instrument elements are used when there are multiple instruments per track.
//@      The midi-device element is used to make a MIDI device or port assignment for the
//@      given track. Initial midi-instrument assignments may be made here as well.

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
            return NULL;

        ImoInstrument* pInstrument = create_instrument(id);

        // [<identification>]
        analyse_optional("identification", pInstrument);

        // [<part-name>]
        analyse_optional("part-name", pInstrument);

        // [<part-name-display>]
        analyse_optional("part-name-display", pInstrument);

        // [<part-abbreviation>]
        analyse_optional("part-abbreviation", pInstrument);

        // [<part-abbreviation-display>]
        analyse_optional("part-abbreviation-display", pInstrument);

        // <group>*
        while (analyse_optional("group", pInstrument));

        // <score-instrument>*
        while (get_optional("score-instrument"))
            ;   //TODO <score-instrument>

        // { [<midi-device>][<midi-instrument>] }*
        while (get_optional("midi-instrument"))
        {
            //TODO: Are these the children? See Saltarello.xml

//            // [<midi-device>]
//            analyse_optional("midi-device", pInstrument);
//
//            // [<midi-instrument>]
//            analyse_optional("midi-instrument", pInstrument);
//
//            // [<midi-channel>]
//            get_optional("midi-channel");   //TODO <midi-channel>
//
//            // [<midi-program>]
//            get_optional("midi-program");   //TODO <midi-program>
//
//            // [<volume>]
//            get_optional("volume");   //TODO <volume>
//
//            // [<pan>]
//            get_optional("pan");   //TODO <pan>
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
        ImoDocument* pImoDoc = NULL;

        //create the document
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        pImoDoc = static_cast<ImoDocument*>(
                    ImFactory::inject(k_imo_document, pDoc));
        pImoDoc->set_version("0.0");    //AWARE: This is lenmusdoc version!
        pImoDoc->set_language("en");    //TODO: analyse language
        m_pAnalyser->save_root_imo_document(pImoDoc);
        pDoc->set_imo_doc(pImoDoc);
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
        get_optional("work");
        // [<movement-number>]
        get_optional("movement-number");
        // [<movement-title>]
        get_optional("movement-title");
        // [<identification>]
        get_optional("identification");
        // [<defaults>]
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
        set_options(pScore);

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
        ImoNoteRest* pNR = NULL;
        if (m_pAnchor && m_pAnchor->is_note_rest())
            pNR = static_cast<ImoNoteRest*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("pAnchor is NULL or it is not ImoNoteRest");
            return NULL;
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

        return NULL;
    }

protected:

    //-----------------------------------------------------------------------------------
    ImoTechnical* get_technical_symbol(ImoNoteRest* pNR, int type)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTechnical* pImo = static_cast<ImoTechnical*>(
                                ImFactory::inject(k_imo_technical, pDoc) );
        pImo->set_technical_type(type);

        // [atrrib]: placement (above | below)
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
        , m_pInfo1(NULL)
        , m_pInfo2(NULL)
    {
    }

    ImoObj* do_analysis()
    {
        ImoNote* pNote = NULL;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("NULL pAnchor or it is not ImoNote");
            return NULL;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pInfo1 = static_cast<ImoSlurDto*>(
                                ImFactory::inject(k_imo_slur_dto, pDoc));

        // arrib: type %start-stop-continue; #REQUIRED
        const string& type = get_mandatory_string_attribute("type", "", "slur");

        // attrib: number %number-level; #IMPLIED
        int num = get_optional_integer_attribute("number", 0);

//        // attrib: %line-type;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_integer_value(0) );

//        // attrib: %dashed-formatting;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_integer_value(0) );

//        // attrib: %position;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_integer_value(0) );

//        // attrib: %placement;
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_integer_value(0) );

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

        m_pInfo1->set_note(pNote);
        m_pAnalyser->add_relation_info(m_pInfo1);

        if (m_pInfo2)
        {
            m_pInfo2->set_note(pNote);
            m_pAnalyser->add_relation_info(m_pInfo2);
        }

        return m_pInfo1;
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
            slurId =  m_pAnalyser->new_slur_id(num);
            m_pInfo2->set_slur_number(slurId);
        }
        else
        {
            error_msg("Missing or invalid slur type. Slur ignored.");
            delete m_pInfo1;
            m_pInfo1 = NULL;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ sound

class SoundMxlAnalyser : public MxlElementAnalyser
{
public:
    SoundMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}


    ImoObj* do_analysis()
    {
        //TODO
        return NULL;
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

        // atrrib: symbol (common | cut | single-number | normal)
        if (has_attribute("symbol"))
            set_symbol(pTime);

        // <beats> (num)
        if (get_mandatory("beats"))
            pTime->set_top_number( get_integer_value(2) );

        // <beat-type> (num)
        if (pTime->get_type() != ImoTimeSignature::k_single_number
             && get_mandatory("beat-type"))
            pTime->set_bottom_number( get_integer_value(4) );

        add_to_model(pTime);
        return pTime;
    }

protected:

    //-----------------------------------------------------------------------------------
    void set_symbol(ImoTimeSignature* pImo)
    {
        // atrrib: symbol (common | cut | single-number | normal)

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
        ImoLyricsTextInfo* pParent = NULL;
        if (m_pAnchor && m_pAnchor->is_lyrics_text_info())
            pParent = static_cast<ImoLyricsTextInfo*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("NULL pAnchor or it is not ImoLyricsTextInfo");
            return NULL;
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
            return NULL;
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
        , m_pInfo1(NULL)
        , m_pInfo2(NULL)
    {
    }

    ImoObj* do_analysis()
    {
        ImoNote* pNote = NULL;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);
        else
        {
            LOMSE_LOG_ERROR("NULL pAnchor or it is not ImoNote");
            return NULL;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        m_pInfo1 = static_cast<ImoTieDto*>(
                                ImFactory::inject(k_imo_tie_dto, pDoc));

        // arrib: type %start-stop-continue; #REQUIRED
        const string& type = get_mandatory_string_attribute("type", "", "tied");

        // attrib: number %number-level; #IMPLIED
        int num = get_optional_integer_attribute("number", 0);

//        // attrib: %line-type;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_integer_value(0) );

//        // attrib: %dashed-formatting;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_integer_value(0) );

//        // attrib: %position;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_integer_value(0) );

//        // attrib: %placement;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_integer_value(0) );

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

//        // attrib: %position;
//        if (get_mandatory(k_number))
//            pInfo->set_tie_number( get_integer_value(0) );

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

        return m_pInfo1;
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
        }
        else
        {
            error_msg("Missing or invalid tie type. Tie ignored.");
            delete m_pInfo1;
            m_pInfo1 = NULL;
        }
    }

};



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
    , m_pTiesBuilder(NULL)
    , m_pBeamsBuilder(NULL)
//    , m_pTupletsBuilder(NULL)
    , m_pSlursBuilder(NULL)
    , m_musicxmlVersion(0)
    , m_pNodeImo(NULL)
    , m_tieNum(0)
    , m_slurNum(0)
    , m_pTree()
    , m_fileLocator("")
//    , m_nShowTupletBracket(k_yesno_default)
//    , m_nShowTupletNumber(k_yesno_default)
    , m_pCurScore(NULL)
    , m_pLastNote(NULL)
    , m_pImoDoc(NULL)
    , m_time(0.0)
    , m_maxTime(0.0)
    , m_divisions(1.0f)
{
    //populate the name to enum conversion map
    m_NameToEnum["articulations"] = k_mxl_tag_articulations;
    m_NameToEnum["attributes"] = k_mxl_tag_attributes;
    m_NameToEnum["backup"] = k_mxl_tag_backup;
    m_NameToEnum["barline"] = k_mxl_tag_barline;
    m_NameToEnum["clef"] = k_mxl_tag_clef;
    m_NameToEnum["direction"] = k_mxl_tag_direction;
    m_NameToEnum["dynamics"] = k_mxl_tag_dynamics;
    m_NameToEnum["fermata"] = k_mxl_tag_fermata;
    m_NameToEnum["forward"] = k_mxl_tag_forward;
    m_NameToEnum["key"] = k_mxl_tag_key;
    m_NameToEnum["lyric"] = k_mxl_tag_lyric;
    m_NameToEnum["measure"] = k_mxl_tag_measure;
    m_NameToEnum["notations"] = k_mxl_tag_notations;
    m_NameToEnum["note"] = k_mxl_tag_note;
    m_NameToEnum["ornaments"] = k_mxl_tag_ornaments;
    m_NameToEnum["part"] = k_mxl_tag_part;
    m_NameToEnum["part-group"] = k_mxl_tag_part_group;
    m_NameToEnum["part-list"] = k_mxl_tag_part_list;
    m_NameToEnum["part-name"] = k_mxl_tag_part_name;
    m_NameToEnum["pitch"] = k_mxl_tag_pitch;
    m_NameToEnum["print"] = k_mxl_tag_print;
    m_NameToEnum["rest"] = k_mxl_tag_rest;
    m_NameToEnum["score-part"] = k_mxl_tag_score_part;
    m_NameToEnum["score-partwise"] = k_mxl_tag_score_partwise;
    m_NameToEnum["slur"] = k_mxl_tag_slur;
    m_NameToEnum["sound"] = k_mxl_tag_sound;
    m_NameToEnum["technical"] = k_mxl_tag_technical;
    m_NameToEnum["text"] = k_mxl_tag_text;
    m_NameToEnum["tied"] = k_mxl_tag_tied;
    m_NameToEnum["time"] = k_mxl_tag_time;
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
//    delete m_pTupletsBuilder;
    delete m_pSlursBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* MxlAnalyser::analyse_tree_and_get_object(XmlNode* root)
{
    delete_relation_builders();
    m_pTiesBuilder = LOMSE_NEW MxlTiesBuilder(m_reporter, this);
    m_pBeamsBuilder = LOMSE_NEW MxlBeamsBuilder(m_reporter, this);
//    m_pTupletsBuilder = LOMSE_NEW MxlTupletsBuilder(m_reporter, this);
    m_pSlursBuilder = LOMSE_NEW MxlSlursBuilder(m_reporter, this);

    m_pTree = root;
//    m_curStaff = 0;
    m_curVoice = 1;
    return analyse_node(root);
}

//---------------------------------------------------------------------------------------
InternalModel* MxlAnalyser::analyse_tree(XmlNode* tree, const string& locator)
{
    m_fileLocator = locator;
    ImoObj* pRoot = analyse_tree_and_get_object(tree);
    return LOMSE_NEW InternalModel( pRoot );
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
    save_last_barline(NULL);
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
//    else if (pDto->is_tuplet_dto())
//        m_pTupletsBuilder->add_item_info(static_cast<ImoTupletDto*>(pDto));
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::clear_pending_relations()
{
    m_pTiesBuilder->clear_pending_items();
    m_pSlursBuilder->clear_pending_items();
    m_pBeamsBuilder->clear_pending_items();
//    m_pTupletsBuilder->clear_pending_items();

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
        m_lyrics.push_back(NULL);
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
            int iInstr = pInstr->get_instrument() + 1;
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
        return NULL;

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

////---------------------------------------------------------------------------------------
//int MxlAnalyser::ldp_name_to_key_type(const string& value)
//{
//    if (value == "C")
//        return k_key_C;
//    else if (value == "G")
//        return k_key_G;
//    else if (value == "D")
//        return k_key_D;
//    else if (value == "A")
//        return k_key_A;
//    else if (value == "E")
//        return k_key_E;
//    else if (value == "B")
//        return k_key_B;
//    else if (value == "F+")
//        return k_key_Fs;
//    else if (value == "C+")
//        return k_key_Cs;
//    else if (value == "C-")
//        return k_key_Cf;
//    else if (value == "G-")
//        return k_key_Gf;
//    else if (value == "D-")
//        return k_key_Df;
//    else if (value == "A-")
//        return k_key_Af;
//    else if (value == "E-")
//        return k_key_Ef;
//    else if (value == "B-")
//        return k_key_Bf;
//    else if (value == "F")
//        return k_key_F;
//    else if (value == "a")
//        return k_key_a;
//    else if (value == "e")
//        return k_key_e;
//    else if (value == "b")
//        return k_key_b;
//    else if (value == "f+")
//        return k_key_fs;
//    else if (value == "c+")
//        return k_key_cs;
//    else if (value == "g+")
//        return k_key_gs;
//    else if (value == "d+")
//        return k_key_ds;
//    else if (value == "a+")
//        return k_key_as;
//    else if (value == "a-")
//        return k_key_af;
//    else if (value == "e-")
//        return k_key_ef;
//    else if (value == "b-")
//        return k_key_bf;
//    else if (value == "f")
//        return k_key_f;
//    else if (value == "c")
//        return k_key_c;
//    else if (value == "g")
//        return k_key_g;
//    else if (value == "d")
//        return k_key_d;
//    else
//        return k_key_undefined;
//}

//---------------------------------------------------------------------------------------
int MxlAnalyser::xml_data_to_clef_type(const string& UNUSED(m_sign), int UNUSED(line),
                                       int UNUSED(m_octaveChange))
{
//    if (m_octaveChange==1 && !(m_sign == "F" || m_sign == "G"))
//    {
//        error_msg("Warning: <clef-octave-change> only implemented for F and G keys. Ignored.");
//        m_octaveChange=0;
//    }
//
//    if (line < 1 || line > 5)
//    {
//        error_msg("Warning: F clef only supported in lines 3, 4 or 5. Clef F in line " + line + "changed to F in line 4.");
//        line = 1;
//    }
//
//    if (m_sign == "G")
//    {
//        if (line==2)
//            return k_clef_G2;
//        else if (line==1)
//            return k_clef_G1;
//        else
//        {
//            error_msg("Warning: G clef only supported in lines 1 or 2. Clef G in line " + line + "changed to G in line 2.");
//            return k_clef_G2;
//        }
//    }
//    else if (m_sign == "F")
//    {
//        if (line==4)
//            return k_clef_F4;
//        else if (line==3)
//            return k_clef_F3;
//        else if (line==5)
//            return k_clef_F5;
//        else
//        {
//            error_msg("Warning: F clef only supported in lines 3, 4 or 5. Clef F in line " + line + "changed to F in line 4.");
//            return k_clef_F4;
//        }
//    }
//    else if (m_sign == "C")
//    {
//        if (line==1)
//            return k_clef_C1;
//        else if (line==2)
//            return k_clef_C2;
//        else if (line==3)
//            return k_clef_C3;
//        else if (line==4)
//            return k_clef_C4;
//        else
//            return k_clef_C5;
//    }
//
//    //TODO
//    else if (m_sign == "percussion")
//        return k_clef_percussion;
//    else if (m_sign == "8_G")
//        return k_clef_8_G2;
//    else if (m_sign == "G_8")
//        return k_clef_G2_8;
//    else if (m_sign == "8_F4")
//        return k_clef_8_F4;
//    else if (m_sign == "F4_8")
//        return k_clef_F4_8;
//    else if (m_sign == "15_G")
//        return k_clef_15_G2;
//    else if (m_sign == "G_15")
//        return k_clef_G2_15;
//    else if (m_sign == "15_F4")
//        return k_clef_15_F4;
//    else if (m_sign == "F4_15")
//        return k_clef_F4_15;
//    else
        return k_clef_undefined;
}

////---------------------------------------------------------------------------------------
//bool MxlAnalyser::ldp_pitch_to_components(const string& pitch, int *step, int* octave,
//                                       EAccidentals* accidentals)
//{
//    // Analyzes string pitch (LDP format), extracts its parts (step, octave and
//    // accidentals) and stores them in the corresponding parameters.
//    // Returns true if error (pitch is not a valid pitch name)
//    //
//    // In LDP pitch is represented as a combination of the step of the diatonic
//    // scale, the chromatic alteration, and the octave.
//    //    - The accidentals parameter represents chromatic alteration (does not
//    //      include key alterations)
//    //    - The octave element is represented by the numbers 0 to 9, where 4
//    //      is the octave started by middle C.
//    //
//    // pitch must be trimed (no spaces before or after real data) and lower case
//
//    size_t i = pitch.length() - 1;
//    if (i < 1)
//        return true;   //error
//
//    *octave = to_octave(pitch[i--]);
//    if (*octave == -1)
//        return true;   //error
//
//    *step = to_step(pitch[i--]);
//    if (*step == -1)
//        return true;   //error
//
//    if (++i == 0)
//    {
//        *accidentals = k_no_accidentals;
//        return false;   //no error
//    }
//    else
//        *accidentals = to_accidentals(pitch.substr(0, i));
//    if (*accidentals == k_invalid_accidentals)
//        return true;   //error
//
//    return false;  //no error
//}
//
////---------------------------------------------------------------------------------------
//int MxlAnalyser::to_step(const char& letter)
//{
//	switch (letter)
//    {
//		case 'a':	return k_step_A;
//		case 'b':	return k_step_B;
//		case 'c':	return k_step_C;
//		case 'd':	return k_step_D;
//		case 'e':	return k_step_E;
//		case 'f':	return k_step_F;
//		case 'g':	return k_step_G;
//	}
//	return -1;
//}
//
////---------------------------------------------------------------------------------------
//int MxlAnalyser::to_octave(const char& letter)
//{
//	switch (letter)
//    {
//		case '0':	return 0;
//		case '1':	return 1;
//		case '2':	return 2;
//		case '3':	return 3;
//		case '4':	return 4;
//		case '5':	return 5;
//		case '6':	return 6;
//		case '7':	return 7;
//		case '8':	return 8;
//		case '9':	return 9;
//	}
//	return -1;
//}

////---------------------------------------------------------------------------------------
//EAccidentals MxlAnalyser::to_accidentals(const std::string& accidentals)
//{
//    switch (accidentals.length())
//    {
//        case 0:
//            return k_no_accidentals;
//            break;
//
//        case 1:
//            if (accidentals[0] == '+')
//                return k_sharp;
//            else if (accidentals[0] == '-')
//                return k_flat;
//            else if (accidentals[0] == '=')
//                return k_natural;
//            else if (accidentals[0] == 'x')
//                return k_double_sharp;
//            else
//                return k_invalid_accidentals;
//            break;
//
//        case 2:
//            if (accidentals.compare(0, 2, "++") == 0)
//                return k_sharp_sharp;
//            else if (accidentals.compare(0, 2, "--") == 0)
//                return k_flat_flat;
//            else if (accidentals.compare(0, 2, "=-") == 0)
//                return k_natural_flat;
//            else
//                return k_invalid_accidentals;
//            break;
//
//        default:
//            return k_invalid_accidentals;
//    }
//}

//---------------------------------------------------------------------------------------
MxlElementAnalyser* MxlAnalyser::new_analyser(const string& name, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch ( name_to_enum(name) )
    {
        case k_mxl_tag_articulations:        return LOMSE_NEW ArticulationsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_attributes:           return LOMSE_NEW AtribbutesMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_backup:               return LOMSE_NEW FwdBackMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_barline:              return LOMSE_NEW BarlineMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_clef:                 return LOMSE_NEW ClefMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_direction:            return LOMSE_NEW DirectionMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_dynamics:             return LOMSE_NEW DynamicsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_fermata:              return LOMSE_NEW FermataMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_forward:              return LOMSE_NEW FwdBackMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_key:                  return LOMSE_NEW KeyMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_lyric:                return LOMSE_NEW LyricMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_measure:              return LOMSE_NEW MeasureMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_notations:            return LOMSE_NEW NotationsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_note:                 return LOMSE_NEW NoteRestMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_ornaments:            return LOMSE_NEW OrnamentsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part:                 return LOMSE_NEW PartMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part_group:           return LOMSE_NEW PartGroupMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part_list:            return LOMSE_NEW PartListMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_part_name:            return LOMSE_NEW PartNameMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_pitch:                return LOMSE_NEW PitchMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_print:                return LOMSE_NEW PrintMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_score_part:           return LOMSE_NEW ScorePartMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_score_partwise:       return LOMSE_NEW ScorePartwiseMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_slur:                 return LOMSE_NEW SlurMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_sound:                return LOMSE_NEW SoundMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_technical:            return LOMSE_NEW TecnicalMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_text:                 return LOMSE_NEW TextMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_tied:                 return LOMSE_NEW TiedMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_time:                 return LOMSE_NEW TimeMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);

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
void MxlTiesBuilder::add_relation_to_notes_rests(ImoTieDto* pEndDto)
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
                    ImFactory::inject(k_imo_tie, pDoc, pStartDto->get_tie_number()));
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

//---------------------------------------------------------------------------------------
void MxlTiesBuilder::error_duplicated_tie(ImoTieDto* pExistingInfo, ImoTieDto* pNewInfo)
{
    m_reporter << "Line " << pNewInfo->get_line_number()
               << ". This tie has the same number than that defined in line "
               << pExistingInfo->get_line_number()
               << ". This tie will be ignored." << endl;
}


//=======================================================================================
// MxlSlursBuilder implementation
//=======================================================================================
void MxlSlursBuilder::add_relation_to_notes_rests(ImoSlurDto* pEndInfo)
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
void MxlBeamsBuilder::add_relation_to_notes_rests(ImoBeamDto* pEndInfo)
{
    m_matches.push_back(pEndInfo);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoBeam* pBeam = static_cast<ImoBeam*>(ImFactory::inject(k_imo_beam, pDoc));

    std::list<ImoBeamDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        ImoBeamData* pData = ImFactory::inject_beam_data(pDoc, *it);
        pNR->include_in_relation(pDoc, pBeam, pData);
    }

    //AWARE: MusicXML requires full item description, Autobeamer is not needed
    //MxlAutoBeamer autobeamer(pBeam);
    //autobeamer.do_autobeam();
}


////=======================================================================================
//// OldMxlBeamsBuilder implementation
////=======================================================================================
//OldMxlBeamsBuilder::OldMxlBeamsBuilder(ostream& reporter, MxlAnalyser* pAnalyser)
//    : m_reporter(reporter)
//    , m_pAnalyser(pAnalyser)
//{
//}
//
////---------------------------------------------------------------------------------------
//OldMxlBeamsBuilder::~OldMxlBeamsBuilder()
//{
//    clear_pending_old_beams();
//}
//
////---------------------------------------------------------------------------------------
//void OldMxlBeamsBuilder::add_old_beam(ImoBeamDto* pInfo)
//{
//    m_pendingOldBeams.push_back(pInfo);
//}
//
////---------------------------------------------------------------------------------------
//void OldMxlBeamsBuilder::clear_pending_old_beams()
//{
//    std::list<ImoBeamDto*>::iterator it;
//    for (it = m_pendingOldBeams.begin(); it != m_pendingOldBeams.end(); ++it)
//    {
//        error_no_end_old_beam(*it);
//        delete *it;
//    }
//    m_pendingOldBeams.clear();
//}
//
////---------------------------------------------------------------------------------------
//bool OldMxlBeamsBuilder::is_old_beam_open()
//{
//    return m_pendingOldBeams.size() > 0;
//}
//
////---------------------------------------------------------------------------------------
//void OldMxlBeamsBuilder::error_no_end_old_beam(ImoBeamDto* pInfo)
//{
//    m_reporter << "Line " << pInfo->get_line_number()
//               << ". No matching 'g-' element for 'g+'. Beam ignored." << endl;
//}
//
////---------------------------------------------------------------------------------------
//void OldMxlBeamsBuilder::close_old_beam(ImoBeamDto* pInfo)
//{
//    add_old_beam(pInfo);
//    do_create_old_beam();
//}
//
////---------------------------------------------------------------------------------------
//void OldMxlBeamsBuilder::do_create_old_beam()
//{
//    Document* pDoc = m_pAnalyser->get_document_being_analysed();
//    ImoBeam* pBeam = static_cast<ImoBeam*>(ImFactory::inject(k_imo_beam, pDoc));
//    std::list<ImoBeamDto*>::iterator it;
//    for (it = m_pendingOldBeams.begin(); it != m_pendingOldBeams.end(); ++it)
//    {
//        ImoNoteRest* pNR = (*it)->get_note_rest();
//        ImoBeamData* pData = ImFactory::inject_beam_data(pDoc, *it);
//        pNR->include_in_relation(pDoc, pBeam, pData);
//        delete *it;
//    }
//    m_pendingOldBeams.clear();
//
//    MxlAutoBeamer autobeamer(pBeam);
//    autobeamer.do_autobeam();
//}
//
//
//
////=======================================================================================
//// MxlTupletsBuilder implementation
////=======================================================================================
//void MxlTupletsBuilder::add_relation_to_notes_rests(ImoTupletDto* pEndDto)
//{
//    m_matches.push_back(pEndDto);
//    Document* pDoc = m_pAnalyser->get_document_being_analysed();
//
//    ImoTupletDto* pStartDto = m_matches.front();
//    ImoTuplet* pTuplet = ImFactory::inject_tuplet(pDoc, pStartDto);
//
//    std::list<ImoTupletDto*>::iterator it;
//    for (it = m_matches.begin(); it != m_matches.end(); ++it)
//    {
//        ImoNoteRest* pNR = (*it)->get_note_rest();
//        ImoTupletData* pData = ImFactory::inject_tuplet_data(pDoc, *it);
//        pNR->include_in_relation(pDoc, pTuplet, pData);
//    }
//}

//=======================================================================================
//// MxlAutoBeamer implementation
////=======================================================================================
//void MxlAutoBeamer::do_autobeam()
//{
//    extract_notes();
//    process_notes();
//}
//
////---------------------------------------------------------------------------------------
//void MxlAutoBeamer::extract_notes()
//{
//    m_notes.clear();
//    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& noteRests
//        = m_pBeam->get_related_objects();
//    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
//    for (it = noteRests.begin(); it != noteRests.end(); ++it)
//    {
//        if ((*it).first->is_note())
//            m_notes.push_back( static_cast<ImoNote*>( (*it).first ) );
//    }
//    //cout << "Num. note/rests in beam: " << noteRests.size() << endl;
//    //cout << "NUm. notes in beam: " << m_notes.size() << endl;
//}
//
////---------------------------------------------------------------------------------------
//void MxlAutoBeamer::get_triad(int iNote)
//{
//    if (iNote == 0)
//    {
//        m_curNotePos = k_first_note;
//        m_pPrevNote = NULL;
//        m_pCurNote = m_notes[0];
//        m_pNextNote = m_notes[1];
//    }
//    else if (iNote == (int)m_notes.size() - 1)
//    {
//        m_curNotePos = k_last_note;
//        m_pPrevNote = m_pCurNote;
//        m_pCurNote = m_notes[iNote];
//        m_pNextNote = NULL;
//    }
//    else
//    {
//        m_curNotePos = k_middle_note;
//        m_pPrevNote = m_pCurNote;
//        m_pCurNote = m_notes[iNote];
//        m_pNextNote = m_notes[iNote+1];
//    }
//}
//
////---------------------------------------------------------------------------------------
//void MxlAutoBeamer::determine_maximum_beam_level_for_current_triad()
//{
//    m_nLevelPrev = (m_curNotePos == k_first_note ? -1 : m_nLevelCur);
//    m_nLevelCur = get_beaming_level(m_pCurNote);
//    m_nLevelNext = (m_pNextNote ? get_beaming_level(m_pNextNote) : -1);
//}
//
////---------------------------------------------------------------------------------------
//void MxlAutoBeamer::process_notes()
//{
//    for (int iNote=0; iNote < (int)m_notes.size(); iNote++)
//    {
//        get_triad(iNote);
//        determine_maximum_beam_level_for_current_triad();
//        compute_beam_types_for_current_note();
//    }
//}
//
////---------------------------------------------------------------------------------------
//void MxlAutoBeamer::compute_beam_types_for_current_note()
//{
//    for (int level=0; level < 6; level++)
//    {
//        compute_beam_type_for_current_note_at_level(level);
//    }
//}
//
////---------------------------------------------------------------------------------------
//void MxlAutoBeamer::compute_beam_type_for_current_note_at_level(int level)
//{
//    if (level > m_nLevelCur)
//        m_pCurNote->set_beam_type(level, ImoBeam::k_none);
//
//    else if (m_curNotePos == k_first_note)
//    {
//        //a) Case First note:
//	    // 2.1) CurLevel > Level(i+1)   -->		Forward hook
//	    // 2.2) other cases             -->		Begin
//
//        if (level > m_nLevelNext)
//            m_pCurNote->set_beam_type(level, ImoBeam::k_forward);    //2.1
//        else
//            m_pCurNote->set_beam_type(level, ImoBeam::k_begin);      //2.2
//    }
//
//    else if (m_curNotePos == k_middle_note)
//    {
//        //b) Case Intermediate note:
//	    //   2.1) CurLevel < Level(i)
//	    //     2.1a) CurLevel > Level(i+1)		-->		End
//	    //     2.1b) else						-->		Continue
//        //
//	    //   2.2) CurLevel > Level(i-1)
//		//     2.2a) CurLevel > Level(i+1)		-->		Hook (fwd or bwd, depending on beat)
//		//     2.2b) else						-->		Begin
//        //
//	    //   2.3) else [CurLevel <= Level(i-1)]
//		//     2.3a) CurLevel > Level(i+1)		-->		End
//		//     2.3b) else						-->		Continue
//
//        if (level > m_nLevelCur)     //2.1) CurLevel < Level(i)
//        {
//            if (level < m_nLevelNext)
//                m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.1a
//            else
//                m_pCurNote->set_beam_type(level, ImoBeam::k_continue);   //2.1b
//        }
//        else if (level > m_nLevelPrev)       //2.2) CurLevel > Level(i-1)
//        {
//            if (level > m_nLevelNext)        //2.2a
//            {
//                //hook. Backward/Forward, depends on position in beat or on values
//                //of previous beams
//                int i;
//                for (i=0; i < level; i++)
//                {
//                    if (m_pCurNote->get_beam_type(i) == ImoBeam::k_begin ||
//                        m_pCurNote->get_beam_type(i) == ImoBeam::k_forward)
//                    {
//                        m_pCurNote->set_beam_type(level, ImoBeam::k_forward);
//                        break;
//                    }
//                    else if (m_pCurNote->get_beam_type(i) == ImoBeam::k_end ||
//                                m_pCurNote->get_beam_type(i) == ImoBeam::k_backward)
//                    {
//                        m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
//                        break;
//                    }
//                }
//                if (i == level)
//                {
//                    //no possible to take decision based on higher level beam values
//                    //Determine it based on position in beat
//
//                    //int nPos = m_pCurNote->GetPositionInBeat();
//                    //if (nPos == lmUNKNOWN_BEAT)
//                        //Unknownn time signature. Cannot determine type of hook. Use backward
//                        m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
//                    //else if (nPos >= 0)
//                    //    //on-beat note
//                    //    m_pCurNote->set_beam_type(level, ImoBeam::k_forward);
//                    //else
//                    //    //off-beat note
//                    //    m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
//                }
//            }
//            else
//                m_pCurNote->set_beam_type(level, ImoBeam::k_begin);      //2.2b
//        }
//
//        else   //   2.3) else [CurLevel <= Level(i-1)]
//        {
//            if (level > m_nLevelNext)
//                m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.3a
//            else
//                m_pCurNote->set_beam_type(level, ImoBeam::k_continue);   //2.3b
//        }
//    }
//
//    else
//    {
//        //c) Case Final note:
//	    //   2.1) CurLevel <= Level(i-1)    -->		End
//	    //   2.2) else						-->		Backward hook
//        if (level <= m_nLevelPrev)
//            m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.1
//        else
//            m_pCurNote->set_beam_type(level, ImoBeam::k_backward);   //2.2
//    }
//}
//
////---------------------------------------------------------------------------------------
//int MxlAutoBeamer::get_beaming_level(ImoNote* pNote)
//{
//    switch(pNote->get_note_type())
//    {
//        case k_eighth:
//            return 0;
//        case k_16th:
//            return 1;
//        case k_32nd:
//            return 2;
//        case k_64th:
//            return 3;
//        case k_128th:
//            return 4;
//        case k_256th:
//            return 5;
//        default:
//            return -1; //Error: Requesting beaming a note longer than eight
//    }
//}


}   //namespace lomse
