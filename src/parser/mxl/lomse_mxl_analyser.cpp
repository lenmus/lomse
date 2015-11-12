//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2014-2015 Cecilio Salmeron. All rights reserved.
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
// Enum to assign a int to each valid MusicXML element
enum EMxlTag
{
    k_mxl_tag_undefined = -1,

    k_mxl_tag_attributes,
    k_mxl_tag_backup,
    k_mxl_tag_barline,
    k_mxl_tag_clef,
    k_mxl_tag_direction,
    k_mxl_tag_fermata,
    k_mxl_tag_forward,
    k_mxl_tag_key,
    k_mxl_tag_measure,
    k_mxl_tag_notations,
    k_mxl_tag_note,
    k_mxl_tag_part,
    k_mxl_tag_part_list,
    k_mxl_tag_part_name,
    k_mxl_tag_pitch,
    k_mxl_tag_print,
    k_mxl_tag_rest,
    k_mxl_tag_score_part,
    k_mxl_tag_score_partwise,
    k_mxl_tag_sound,
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
    XmlNode m_pAnalysedNode;
    XmlNode m_pChildToAnalyse;
    XmlNode m_pNextParam;
    XmlNode m_pNextNextParam;

    // the main method to perform the analysis of a node
    inline ImoObj* analyse_child() { return m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL); }

    // 'get' methods just update m_pChildToAnalyse to point to the next node to analyse
    bool get_mandatory(const string& tag);
//    bool get_optional(EMxlTag type);
    bool get_optional(const string& type);

    // 'analyse' methods do a 'get' and, if found, analyse the found element
    void analyse_mandatory(const string& tag, ImoObj* pAnchor=NULL);
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
//    inline ImoId get_node_id() { return get_node_id(m_pAnalysedNode); }
//    bool contains(ELdpElement type, ELdpElement* pValid, int nValid);
    inline const string& get_document_locator() {
        return m_pAnalyser->get_document_locator();
    }



    //-----------------------------------------------------------------------------------
    //XmlNode helper methods
//    inline ImoId get_node_id(XmlNode* node) { return m_pAnalyser->get_node_id(node); }
    inline int get_line_number(XmlNode* node) { return m_pAnalyser->get_line_number(node); }
    inline bool has_attribute(XmlNode* node, const string& name)
    {
        return node->attribute(name.c_str()) != NULL;
    }
    inline string get_attribute(XmlNode* node, const string& name)
    {
        XmlAttribute attr = node->attribute(name.c_str());
        return string( attr.value() );
    }
//    inline ELdpElement get_type(XmlNode* node) { return m_pAnalyser->get_type(node); }
    //inline ImoObj* get_imo(XmlNode* node) { return m_pAnalyser->get_imo(node); }
    //inline void set_imo(XmlNode* node, ImoObj* pImo) { return m_pAnalyser->set_imo(node, pImo); }

//    inline bool is_type(XmlNode* node, ELdpElement type) { return get_type(node) == type; }


//    //-----------------------------------------------------------------------------------
//    inline void post_event(SpEventInfo event)
//    {
//        m_libraryScope.post_event(event);
//    }

    //-----------------------------------------------------------------------------------
    inline bool more_children_to_analyse() {
        return !m_pNextParam.is_null();
    }

    //-----------------------------------------------------------------------------------
    inline XmlNode get_child_to_analyse() {
        return m_pNextParam;
    }

    //-----------------------------------------------------------------------------------
    inline void move_to_next_child() {
        m_pNextParam = m_pNextNextParam;
        prepare_next_one();
    }

    //-----------------------------------------------------------------------------------
    inline void prepare_next_one() {
        if (!m_pNextParam.is_null())
            m_pNextNextParam = m_pNextParam.next_sibling();
        else
            m_pNextNextParam = XmlNode();
    }

    //-----------------------------------------------------------------------------------
    inline void move_to_first_child() {
        m_pNextParam = m_pAnalysedNode.first_child();
        prepare_next_one();
    }

//    //-----------------------------------------------------------------------------------
//    void get_num_staff()
//    {
//        string staff = m_pChildToAnalyse.value();
//        int nStaff;
//        //http://www.codeguru.com/forum/showthread.php?t=231054
//        std::istringstream iss(staff);
//        if ((iss >> std::dec >> nStaff).fail())
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Invalid staff '" + staff + "'. Replaced by '1'.");
//            m_pAnalyser->set_current_staff(0);
//        }
//        else
//            m_pAnalyser->set_current_staff(--nStaff);
//    }

    //-----------------------------------------------------------------------------------
    bool is_long_value()
    {
        string number = m_pChildToAnalyse.value();
        long nNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> nNumber).fail());
    }

    //-----------------------------------------------------------------------------------
    long get_long_value(long nDefault=0L)
    {
        string number = m_pChildToAnalyse.value();
        long nNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> nNumber).fail())
        {
            stringstream replacement;
            replacement << nDefault;
            report_msg(get_line_number(&m_pChildToAnalyse),
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
        string number = m_pChildToAnalyse.value();
        float rNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> rNumber).fail());
    }

    //-----------------------------------------------------------------------------------
    float get_float_value(float rDefault=0.0f)
    {
        string number = m_pChildToAnalyse.value();
        float rNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> rNumber).fail())
        {
            stringstream replacement;
            replacement << rDefault;
            report_msg(get_line_number(&m_pChildToAnalyse),
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
        string value = string(m_pChildToAnalyse.value());
        return  value == "true" || value == "yes"
             || value == "false" || value == "no" ;
    }

    //-----------------------------------------------------------------------------------
    bool get_bool_value(bool fDefault=false)
    {
        string value = string(m_pChildToAnalyse.value());
        if (value == "true" || value == "yes")
            return true;
        else if (value == "false" || value == "no")
            return false;
        else
        {
            stringstream replacement;
            replacement << fDefault;
            report_msg(get_line_number(&m_pChildToAnalyse),
                "Invalid boolean value '" + value + "'. Replaced by '"
                + replacement.str() + "'.");
            return fDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    int get_yes_no_value(int nDefault)
    {
        string value = m_pChildToAnalyse.value();
        if (value == "yes")
            return k_yesno_yes;
        else if (value == "no")
            return k_yesno_no;
        else
        {
            report_msg(get_line_number(&m_pChildToAnalyse),
                "Invalid yes/no value '" + value + "'. Replaced by default.");
            return nDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    string get_string_value()
    {
        return m_pChildToAnalyse.value();
    }

//    //-----------------------------------------------------------------------------------
//    EHAlign get_alignment_value(EHAlign defaultValue)
//    {
//        const std::string& value = m_pChildToAnalyse.value();
//        if (value == "left")
//            return k_halign_left;
//        else if (value == "right")
//            return k_halign_right;
//        else if (value == "center")
//            return k_halign_center;
//        else
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                    "Invalid alignment value '" + value + "'. Assumed 'center'.");
//            return defaultValue;
//        }
//    }

//    //-----------------------------------------------------------------------------------
//    Color get_color_child()
//    {
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL);
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
//        string value = m_pChildToAnalyse.value();
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
//        const string value = m_pChildToAnalyse.value();
//        int size = static_cast<int>(value.size()) - 2;
//        string points = value.substr(0, size);
//        string number = m_pChildToAnalyse.value();
//        float rNumber;
//        std::istringstream iss(number);
//        if ((iss >> std::dec >> rNumber).fail())
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
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
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
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
//                report_msg(get_line_number(&m_pChildToAnalyse),
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
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL);
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
//        ImoObj* pImo = m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL);
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
//                report_msg(get_line_number(&m_pChildToAnalyse),
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
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//        const std::string& value = m_pChildToAnalyse.value();
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
//            report_msg(get_line_number(&m_pAnalysedNode),
//                "Element 'lineStyle': Invalid value '" + value
//                + "'. Replaced by 'solid'." );
//            return k_line_solid;
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    ELineCap get_line_cap_child()
//    {
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//        const std::string& value = m_pChildToAnalyse.value();
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
//            report_msg(get_line_number(&m_pAnalysedNode),
//                "Element 'lineCap': Invalid value '" + value
//                + "'. Replaced by 'none'." );
//            return k_cap_none;
//        }
//    }
//
//    //-----------------------------------------------------------------------------------
//    void check_visible(ImoInlinesContainer* pCO)
//    {
//        string value = m_pChildToAnalyse.value();
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
//        string duration = m_pChildToAnalyse.value();
//        NoteTypeAndDots figdots = ldp_duration_to_components(duration);
//        if (figdots.noteType == k_unknown_notetype)
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
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
//            m_pChildToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_pChildToAnalyse);
//            if (is_auxobj(type))
//                m_pAnalyser->analyse_node(&m_pChildToAnalyse, pAnchor);
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
//            m_pChildToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_pChildToAnalyse);
//            if (   /*type == k_inlineWrapper
//                ||*/ type == k_txt
//                || type == k_image
//                || type == k_link
//               )
//            {
//                return static_cast<ImoInlineLevelObj*>(
//                    m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL) );
//            }
//            else if (type == k_string)
//            {
//                //string: implicit <txt>
//                Document* pDoc = m_pAnalyser->get_document_being_analysed();
//                ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                            ImFactory::inject(k_imo_text_item, pDoc) );
//                pText->set_text( string(m_pChildToAnalyse.value()) );
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
//            m_pChildToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_pChildToAnalyse);
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
//                m_pAnalyser->analyse_node(&m_pChildToAnalyse, pParent);
//            }
//            else if (type == k_string)
//            {
//                //string: implicit <txt>
//                Document* pDoc = m_pAnalyser->get_document_being_analysed();
//                ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                            ImFactory::inject(k_imo_text_item, pDoc) );
//                pText->set_text( string(m_pChildToAnalyse.value()) );
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
    m_pAnalysedNode = *pNode;
    move_to_first_child();
    return do_analysis();
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::error_missing_element(const string& tag)
{
    string parentName = m_pAnalysedNode.name();
    report_msg(get_line_number(&m_pAnalysedNode),
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
    return has_attribute(&m_pAnalysedNode, name);
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::get_attribute(const string& name)
{
    return m_pAnalysedNode.attribute_value(name);
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::get_mandatory_string_attribute(const string& name,
                                  const string& sDefault, const string& element)
{
    string attrb = sDefault;
    if (has_attribute(&m_pAnalysedNode, name))
        attrb = m_pAnalysedNode.attribute_value(name);
    else if (sDefault.empty())
        report_msg(get_line_number(&m_pAnalysedNode),
            element + ": missing mandatory attribute '" + name + "'." );
    else
        report_msg(get_line_number(&m_pAnalysedNode),
            element + ": missing mandatory attribute '" + name + "'. Value '"
            + sDefault + "' assumed.");

    return attrb;
}

//---------------------------------------------------------------------------------------
string MxlElementAnalyser::get_optional_string_attribute(const string& name,
                                                         const string& sDefault)
{
    if (has_attribute(&m_pAnalysedNode, name))
        return m_pAnalysedNode.attribute_value(name);
    else
        return sDefault;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_attribute_as_integer(const string& name, int nDefault)
{
    string number = m_pAnalysedNode.attribute_value(name);
    long nNumber;
    std::istringstream iss(number);
    if ((iss >> std::dec >> nNumber).fail())
    {
        stringstream replacement;
        replacement << nDefault;
        report_msg(get_line_number(&m_pChildToAnalyse),
            "Invalid integer number '" + number + "'. Replaced by '"
            + replacement.str() + "'.");
        return nDefault;
    }
    else
        return nNumber;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_optional_integer_attribute(const string& name,
                                                       int nDefault)
{
    if (has_attribute(&m_pAnalysedNode, name))
        return get_attribute_as_integer(name, nDefault);
    else
        return nDefault;
}

//---------------------------------------------------------------------------------------
int MxlElementAnalyser::get_mandatory_integer_attribute(const string& name, int nDefault,
                                                        const string& element)
{
    int attrb = nDefault;
    if (has_attribute(&m_pAnalysedNode, name))
        attrb = get_attribute_as_integer(name, nDefault);
    else
    {
        stringstream replacement;
        replacement << nDefault;
        report_msg(get_line_number(&m_pAnalysedNode),
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

    m_pChildToAnalyse = get_child_to_analyse();
    if (m_pChildToAnalyse.name() != tag)
    {
        error_missing_element(tag);
        return false;
    }

    move_to_next_child();
    return true;
}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::analyse_mandatory(const string& tag, ImoObj* pAnchor)
{
    if (get_mandatory(tag))
        m_pAnalyser->analyse_node(&m_pChildToAnalyse, pAnchor);
}

//---------------------------------------------------------------------------------------
bool MxlElementAnalyser::get_optional(const string& name)
{
    if (more_children_to_analyse())
    {
        m_pChildToAnalyse = get_child_to_analyse();
        if (m_pChildToAnalyse.name() == name)
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
        m_pAnalyser->analyse_node(&m_pChildToAnalyse, pAnchor);
        return true;
    }
    return false;
}

////---------------------------------------------------------------------------------------
//void MxlElementAnalyser::analyse_one_or_more(ELdpElement* pValid, int nValid)
//{
//    while(more_children_to_analyse())
//    {
//        m_pChildToAnalyse = get_child_to_analyse();
//
//        ELdpElement type = get_type(m_pChildToAnalyse);
//        if (contains(type, pValid, nValid))
//        {
//            move_to_next_child();
//            m_pAnalyser->analyse_node(&m_pChildToAnalyse);
//        }
//        else
//        {
//            string name = m_pChildToAnalyse.name();
//            report_msg(get_line_number(&m_pChildToAnalyse),
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
    string name = m_pChildToAnalyse.name();
    if (name == "label")
        name += ":" + m_pChildToAnalyse.value();
    report_msg(get_line_number(&m_pChildToAnalyse),
        "Element '" + name + "' unknown or not possible here. Ignored.");
}

//---------------------------------------------------------------------------------------
void MxlElementAnalyser::error_msg(const string& msg)
{
    report_msg(get_line_number(&m_pAnalysedNode), msg);
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
        string next = m_pNextParam.next_sibling().name();
        string name = m_pChildToAnalyse.name();
        if (name == "label")
            name += ":" + m_pChildToAnalyse.value();
        report_msg(get_line_number(&m_pAnalysedNode),
                "Element <" + m_pAnalysedNode.name()
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
//        m_pChildToAnalyse = get_child_to_analyse();
//        if (m_pChildToAnalyse.name() == "staff")
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
//        m_pChildToAnalyse = get_child_to_analyse();
//        ELdpElement type = get_type(m_pChildToAnalyse);
//        switch (type)
//        {
//            case k_visible:
//            {
//                m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
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
        cout << "Missing analyser for element '" << m_tag << "'. Node ignored." << endl;
        return NULL;
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
            keys.push_back( m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL) );

        // <time>*
        while (get_optional("time"))
            times.push_back( m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL) );

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
            ; //TODO <part-symbol>

        // [<instruments>]
        if (get_optional("instruments"))
            ; //TODO <instruments>

        // <clef>*
        while (get_optional("clef"))
            clefs.push_back( m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL) );

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
            barStyle = m_pChildToAnalyse.value();

        //TODO
        // %editorial;
        // wavy-line?
        // segno?
        // coda?
        // (fermata, fermata?)?

        //TODO
        // ending?
        if (get_optional("ending"))
            ;

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
            //report_msg(get_line_number(&m_pChildToAnalyse),
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
        if (has_attribute(&m_pChildToAnalyse, "direction"))
            direction = m_pChildToAnalyse.attribute_value("direction");
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
            //report_msg(get_line_number(&m_pChildToAnalyse),
            error_msg2(
                "Barlines combination not supported: left = "
                + LdpExporter::barline_type_to_ldp(leftType)
                + ", right = "
                + LdpExporter::barline_type_to_ldp(rightType)
                + ". Replaced by 'double' barline.");
            type = k_barline_double;
        }
#if 0
        report_msg(get_line_number(&m_pChildToAnalyse),
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
            m_sign = get_string_value();    //m_pChildToAnalyse.value();

        if (get_optional("line"))
            m_line = get_integer_value(0);   //(m_pChildToAnalyse);

        if (get_optional("clef-octave-change"))
            m_octaveChange = get_integer_value(0);   //(m_pChildToAnalyse);

        int type = determine_clef_type();
        if (type == k_clef_undefined)
        {
            //report_msg(get_line_number(&m_pChildToAnalyse),
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
//        const std::string& value = m_pChildToAnalyse.first_child().value();
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
        bool fFwd = (m_pAnalysedNode.name() == "forward");
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

    void add_barline(ImoMusicData* pMD)
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
//        m_pChildToAnalyse = get_child_to_analyse();
//        const string& name = m_pChildToAnalyse.name();
//        if (name == "tuplet")
//        {
//            ImoObj* pImo = m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL);
//            m_pTupletInfo = static_cast<ImoTupletDto*>( pImo );
//        }
//            if (name == "tuplet")
//            {
//                ImoObj* pImo = m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL);
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
            ;
        if (!fIsCue && get_optional("tie"))
            ;

        // [<instrument>]
        if (get_optional("instrument"))
            ;

        // [<voice>]
        int voice = m_pAnalyser->get_current_voice();
        if (get_optional("voice"))
            voice = get_integer_value( voice );
        set_voice(pNR, voice);

        // [<type>]
        string type;
        if (get_optional("type"))
            type = m_pChildToAnalyse.value();

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
            ;

        // [<stem>]
        if (!fIsRest && get_optional("stem"))
            set_stem(pNote);

        // [<notehead>]
        if (get_optional("notehead"))
            ;

        // [<notehead-text>]
        if (get_optional("notehead-text"))
            ;

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
            ;

        error_if_more_elements();

        //for now, ignore cue & grace notes
        if (!(fIsCue || fIsGrace))
            add_to_model(pNR);
        else
        {
            delete pNote;
            return NULL;
        }

//        //tuplet
//        if (m_pTupletInfo==NULL && m_pAnalyser->is_tuplet_open())
//            add_to_current_tuplet(pNR);
//
//        add_tuplet_info(pNR);

//        //slur
//        add_slur_info(pNote);

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
        if (has_attribute(&m_pChildToAnalyse, "measure"))
        {
            const string& measure = m_pChildToAnalyse.attribute_value("measure");
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
            //case it is not mandatory. If not present, <type> and <dots>
            //must be derived from <duration>.
            dots = 0;
            noteType = k_whole;
            //TODO
//            if (units >= k_duration_whole_dotted)
//            {
//                dots = 1;
//                noteType = k_whole;
//            }
//            if (units >= k_duration_whole)
//            {
//                dots = 0;
//                noteType = k_whole;
//            }
//            else if (units >= k_duration_half_dotted)
//            {
//                dots = 1;
//                noteType = k_duration_half_dotted;
//            }
//            else if (units >= k_duration_half)
//            {
//                dots = 0;
//                noteType = k_half;
//            }
//            else if (units >= k_duration_quarter_dotted)
//            {
//                dots = 1;
//                noteType = k_duration_quarter_dotted;
//            }
//            else if (units >= k_duration_eighth_dotted)
//            {
//                dots = 1;
//                noteType = k_duration_eighth_dotted;
//            }
//            else if (units >= k_duration_16th_dotted)
//            {
//                dots = 1;
//                noteType = k_duration_16th_dotted;
//            }
//            else if (units >= k_duration_32th_dotted)
//            {
//                dots = 1;
//                noteType = k_duration_32th_dotted;
//            }
//            else if (units >= k_duration_64th_dotted)
//            {
//                dots = 1;
//                noteType = k_duration_64th_dotted;
//            }
//            else if (units >= k_duration_128th_dotted)
//            {
//                dots = 1;
//                noteType = k_duration_128th_dotted;
//            }
//            else
//            {
//                dots = 0;
//                noteType = k_duration_256th;
//            }
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
            noteType = k_32th;
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
            //report_msg(get_line_number(&m_pChildToAnalyse),
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
        string acc = m_pChildToAnalyse.value();  //get_string_value();
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
            //report_msg(get_line_number(&m_pChildToAnalyse),
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
        string type = m_pChildToAnalyse.value();
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
            //report_msg(get_line_number(&m_pChildToAnalyse),
            error_msg2(
                "Invalid or not supported <stem> value '" + type + "'. Replaced by 'default'.");
        }
        pNote->set_stem_direction(value);
    }

    //----------------------------------------------------------------------------------
//    void analyse_note_rest_options(ImoNoteRest* pNR)
//    {
//        // { <beam> | <tuplet> | | <fermata> }
//
//        while( more_children_to_analyse() )
//        {
//            m_pChildToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_pChildToAnalyse);
//            if (type == k_tuplet)
//            {
//                ImoObj* pImo = m_pAnalyser->analyse_node(&m_pChildToAnalyse, NULL);
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
        const string& level = m_pChildToAnalyse.attribute_value("number");
        int iLevel;
        if (m_pAnalyser->to_integer(level, &iLevel))
        {
            //report_msg(get_line_number(&m_pChildToAnalyse),
            error_msg2(
                "Missing or invalid beam number '" + level + "'. Beam ignored.");
            return;
        }

        if (iLevel <= 0 || iLevel > 6)
        {
            //report_msg(get_line_number(&m_pChildToAnalyse),
            error_msg2(
                "Invalid beam number '" + level +"'. Beam ignored.");
            return;
        }

        // value: beam type
        const string& type = m_pChildToAnalyse.value();
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
            //report_msg(get_line_number(&m_pChildToAnalyse),
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

    //----------------------------------------------------------------------------------
//    void add_slur_info(ImoNote* pNR)
//    {
//        if (m_pSlurDto)
//        {
//            m_pSlurDto->set_note(pNR);
//            m_pAnalyser->add_relation_info(m_pSlurDto);
//        }
//    }
//
    //----------------------------------------------------------------------------------
//    void add_attachment(ImoNoteRest* pNR, ImoFermata* pFermata)
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        pNR->add_attachment(pDoc, pFermata);
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
//@ <part-list> = <part-group>* <score-part> { <part-group> | <score-part> }*
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
        // <part-group>*
        while (get_optional("part-group"))
        {
            ;   //TODO: add part-group to table
        }

        // <score-part>
        analyse_mandatory("score-part");

        // { <part-group> | <score-part> }*
        while (more_children_to_analyse())
        {
            if (analyse_optional("score-part"))
                ;
            else if (analyse_optional("part-group"))
                ;   //TODO: add part-group to table
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        error_if_more_elements();

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
            string name = m_pAnalysedNode.value();
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
//class TextStringMxlAnalyser : public MxlElementAnalyser
//{
//protected:
//    string m_styleName;
//
//public:
//    TextStringMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                       ImoObj* pAnchor, const string& styleName="Default style")
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
//        , m_styleName(styleName)
//    {
//    }
//
//    ImoObj* do_analysis()
//    {
//        // <string>
//        if (get_mandatory(k_string))
//        {
//            Document* pDoc = m_pAnalyser->get_document_being_analysed();
//            ImoScoreText* pText = static_cast<ImoScoreText*>(
//                                        ImFactory::inject(k_imo_score_text, pDoc));
//            pText->set_text(get_string_value());
//
//            // [<style>]
//            ImoStyle* pStyle = NULL;
//            if (get_optional(k_style))
//                pStyle = get_text_style_child(m_styleName);
//            else
//            {
//                ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
//                if (pScore)     //in unit tests the score might not exist
//                    pStyle = pScore->get_default_style();
//            }
//            pText->set_style(pStyle);
//
//            // [<location>]
//            while (more_children_to_analyse())
//            {
//                if (get_optional(k_dx))
//                    pText->set_user_location_x( get_location_child() );
//                else if (get_optional(k_dy))
//                    pText->set_user_location_y( get_location_child() );
//                else
//                {
//                    error_invalid_child();
//                    move_to_next_child();
//                }
//            }
//            error_if_more_elements();
//
//            add_to_model(pText);
//            return pText;
//        }
//        return NULL;
//    }
//
//};

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
        string step = (get_mandatory("step") ? m_pChildToAnalyse.value() : "C");

        // [<alter>]
        string accidentals = (get_optional("alter") ? m_pChildToAnalyse.value() : "0");

        // <octave>
        string octave = (get_mandatory("octave") ? m_pChildToAnalyse.value() : "4");

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
                //report_msg(get_line_number(&m_pChildToAnalyse),
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
                //report_msg(get_line_number(&m_pChildToAnalyse),
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
            //report_msg(get_line_number(&m_pChildToAnalyse),
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
        Linker linker(pDoc);
        linker.add_child_to_model(pInstrument, pMD, pMD->get_obj_type());

        m_pAnalyser->add_score_part(id, pInstrument);
        return pInstrument;
    }

};

//
////@--------------------------------------------------------------------------------------
////@ <instrument> = (instrument [<instrName>][<instrAbbrev>][<staves>][<staff>]*
////@                            [<infoMIDI>] <musicData> )
////@ <instrName> = <textString>
////@ <instrAbbrev> = <textString>
//
//class InstrumentMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    InstrumentMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                       ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        m_pAnalyser->clear_pending_relations();
//
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoInstrument* pInstrument = static_cast<ImoInstrument*>(
//                                        ImFactory::inject(k_imo_instrument, pDoc) );
//
//        // [<name>]
//        analyse_optional(k_name, pInstrument);
//
//        // [<abbrev>]
//        analyse_optional(k_abbrev, pInstrument);
//
//        // [<staves>]
//        if (get_optional(k_staves))
//            set_staves(pInstrument);
//
//        // [<staff>]*
//        while (analyse_optional(k_staff, pInstrument));
//
//        // [<infoMIDI>]
//        analyse_optional(k_infoMIDI, pInstrument);
//
//        // <musicData>
//        analyse_mandatory(k_musicData, pInstrument);
//
//        error_if_more_elements();
//
//        add_to_model(pInstrument);
//        return pInstrument;
//    }
//
//protected:
//
//    void set_staves(ImoInstrument* pInstrument)
//    {
//        // <staves> = (staves <num>)
//
//        XmlNode* pValue = get_first_child(m_pChildToAnalyse);
//        string staves = get_value(pValue);
//        int nStaves;
//        bool fError = !is_type(pValue, k_number);
//        if (!fError)
//        {
//            std::istringstream iss(staves);
//            fError = (iss >> std::dec >> nStaves).fail();
//        }
//        if (fError)
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Invalid value '" + staves + "' for staves. Replaced by 1.");
//        }
//        else
//        {
//            for(; nStaves > 1; --nStaves)
//                pInstrument->add_staff();
//        }
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <infoMIDI> = (infoMIDI num_instr [num_channel])
////@ num_instr = integer: 0..255
////@ num_channel = integer: 0..15
//
//class InfoMidiMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    InfoMidiMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                       ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoMidiInfo* pInfo = static_cast<ImoMidiInfo*>(
//                                    ImFactory::inject(k_imo_midi_info, pDoc) );
//
//        // num_instr
//        if (!get_optional(k_number) || !set_instrument(pInfo))
//        {
//            error_msg("Missing or invalid MIDI instrument (0..255). MIDI info ignored.");
//            delete pInfo;
//            return NULL;
//        }
//
//        // [num_channel]
//        if (get_optional(k_number) && !set_channel(pInfo))
//        {
//            report_msg(get_line_number(&m_pAnalysedNode),
//                        "Invalid MIDI channel (0..15). Channel info ignored.");
//        }
//
//        error_if_more_elements();
//
//        add_to_model(pInfo);
//        return pInfo;
//    }
//
//protected:
//
//    bool set_instrument(ImoMidiInfo* pInfo)
//    {
//        int value = get_integer_value(0);
//        if (value < 0 || value > 255)
//            return false;   //error
//
//        pInfo->set_instrument(value);
//        return true;
//    }
//
//    bool set_channel(ImoMidiInfo* pInfo)
//    {
//        int value = get_integer_value(0);
//        if (value < 0 || value > 15)
//            return false;   //error
//
//        pInfo->set_channel(value);
//        return true;
//    }
//
//};

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
        //TODO: review if this is necessary
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
        pScore->set_version(160);   //use version 1.6 to allow using ImoFwdBack
        return pScore;
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

        // <beats> (num)
        if (get_mandatory("beats"))
            pTime->set_top_number( get_integer_value(2) );

        // <beat-type> (num)
        if (get_mandatory("beat-type"))
            pTime->set_bottom_number( get_integer_value(4) );

        add_to_model(pTime);
        return pTime;
    }

};




////@--------------------------------------------------------------------------------------
////@ <anchorLine> = (anchorLine <destination-point>[<lineStyle>][<color>][<width>]
////@                            [<lineCapEnd>])
////@ <destination-point> = <location>    in Tenths
////@     i.e.: (anchorLine (dx value)(dy value)(lineStyle value)(color value)(width value))
////@
//
//class AnchorLineMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    AnchorLineMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                       ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoLineStyle* pLine = static_cast<ImoLineStyle*>(
//                                    ImFactory::inject(k_imo_line_style, pDoc) );
//        pLine->set_start_point( TPoint(0.0f, 0.0f) );
//        pLine->set_start_edge(k_edge_normal);
//        pLine->set_start_cap(k_cap_none);
//        pLine->set_end_edge(k_edge_normal);
//
//        // <destination-point> = <dx><dy>
//        TPoint point;
//        if (get_mandatory(k_dx))
//            point.x = get_location_child();
//        if (get_mandatory(k_dy))
//            point.y = get_location_child();
//        pLine->set_end_point( point );
//
//        // [<lineStyle>]
//        if (get_optional(k_lineStyle))
//            pLine->set_line_style( get_line_style_child() );
//
//        // [<color>])
//        if (get_optional(k_color))
//            pLine->set_color( get_color_child() );
//
//        // [<width>]
//        if (get_optional(k_width))
//            pLine->set_width( get_width_child(1.0f) );
//
//        // [<lineCapEnd>])
//        if (get_optional(k_lineCapEnd))
//            pLine->set_end_cap( get_line_cap_child() );
//
//        add_to_model( pLine );
//        return pLine;
//    }
//
//};
//
//
////@--------------------------------------------------------------------------------------
////@ <bezier> = (bezier <bezier-location>* )
////@ <bezier-location> = { (start-x num) | (start-y num) | (end-x num) | (end-y num) |
////@                       (ctrol1-x num) | (ctrol1-y num) | (ctrol2-x num) | (ctrol2-y num) }
////@ <num> = real number, in tenths
//
//class BezierMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    BezierMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                   ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoBezierInfo* pBezier = static_cast<ImoBezierInfo*>(
//                                    ImFactory::inject(k_imo_bezier_info, pDoc));
//
//        while (more_children_to_analyse())
//        {
//            if (get_optional(k_start_x))
//                set_x_in_point(ImoBezierInfo::k_start, pBezier);
//            else if (get_optional(k_end_x))
//                set_x_in_point(ImoBezierInfo::k_end, pBezier);
//            else if (get_optional(k_ctrol1_x))
//                set_x_in_point(ImoBezierInfo::k_ctrol1, pBezier);
//            else if (get_optional(k_ctrol2_x))
//                set_x_in_point(ImoBezierInfo::k_ctrol2, pBezier);
//            else if (get_optional(k_start_y))
//                set_y_in_point(ImoBezierInfo::k_start, pBezier);
//            else if (get_optional(k_end_y))
//                set_y_in_point(ImoBezierInfo::k_end, pBezier);
//            else if (get_optional(k_ctrol1_y))
//                set_y_in_point(ImoBezierInfo::k_ctrol1, pBezier);
//            else if (get_optional(k_ctrol2_y))
//                set_y_in_point(ImoBezierInfo::k_ctrol2, pBezier);
//            else
//            {
//                error_invalid_child();
//                move_to_next_child();
//            }
//        }
//
//        add_to_model(pBezier);
//        return pBezier;
//    }
//
//    void set_x_in_point(int i, ImoBezierInfo* pBezier)
//    {
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//        float value = get_float_value(0.0f);
//        TPoint& point = pBezier->get_point(i);
//        point.x = value;
//    }
//
//    void set_y_in_point(int i, ImoBezierInfo* pBezier)
//    {
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//        float value = get_float_value(0.0f);
//        TPoint& point = pBezier->get_point(i);
//        point.y = value;
//    }
//};
//
////@--------------------------------------------------------------------------------------
////@ <border> = (border <width><lineStyle><color>)
////@     i.e.: (border (width 2.5)(lineStyle solid)(color #ff0000))
//
//class BorderMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    BorderMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                   ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        ImoBorderDto* border = LOMSE_NEW ImoBorderDto();
//
//        // <width>
//        if (get_mandatory(k_width))
//            border->set_width( get_width_child(1.0f) );
//
//        // <lineStyle>
//        if (get_mandatory(k_lineStyle))
//            border->set_style( get_line_style_child() );
//
//        // <color>
//        if (get_mandatory(k_color))
//            border->set_color( get_color_child() );
//
//        add_to_model(border);
//        return border;
//    }
//
//protected:
//
//        void set_box_border(ImoTextBlockInfo& box)
//        {
//            ImoObj* pImo = analyse_child();
//            if (pImo)
//            {
//                if (pImo->is_border_dto())
//                    box.set_border( static_cast<ImoBorderDto*>(pImo) );
//                delete pImo;
//            }
//        }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <chord> = (chord <note>+ )
//
//class ChordMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    ChordMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                  ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoChord* pChord = static_cast<ImoChord*>( ImFactory::inject(k_imo_chord, pDoc) );
//
//        // <note>*
//        while (more_children_to_analyse())
//        {
//            if (!analyse_optional(k_note, pChord))
//            {
//                error_invalid_child();
//                move_to_next_child();
//            }
//        }
//
//        add_notes_to_music_data(pChord);
//        return pChord;
//    }
//
//protected:
//
//    void add_notes_to_music_data(ImoChord* pChord)
//    {
//        //get anchor musicData
//        if (m_pAnchor && m_pAnchor->is_music_data())
//        {
//            ImoMusicData* pMD = static_cast<ImoMusicData*>(m_pAnchor);
//
//            //add notes to musicData
//            std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes
//                = pChord->get_related_objects();
//            std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
//            for (it = notes.begin(); it != notes.end(); ++it)
//            {
//                ImoNote* pNote = static_cast<ImoNote*>( (*it).first );
//                pMD->append_child_imo(pNote);
//            }
//
//            add_to_model(pChord);
//        }
//    }
//
//};
//
//
////@--------------------------------------------------------------------------------------
////@ <color> = (color <rgba>}
////@ <rgba> = label: { #rrggbb | #rrggbbaa }
//
//class ColorMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    ColorMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope) {}
//
//    ImoObj* do_analysis()
//    {
//        string value = get_value(m_pAnalysedNode);
//        ImoColorDto* pColor = LOMSE_NEW ImoColorDto();
//        pColor->set_from_string(value);
//        if (!pColor->is_ok())
//        {
//            error_msg("Missing or invalid color value. Must be #rrggbbaa. Color ignored.");
//            delete pColor;
//            return NULL;
//        }
//        return pColor;
//    }
//};
//
////@--------------------------------------------------------------------------------------
////@ <newSystem> = (newSystem}
//
//class ControlMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    ControlMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                    ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoSystemBreak* pCtrl = static_cast<ImoSystemBreak*>(
//                                    ImFactory::inject(k_imo_system_break, pDoc) );
//        add_to_model(pCtrl);
//        return pCtrl;
//    }
//};
//
//
////---------------------------------------------------------------------------------------
//class DefineStyleMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    DefineStyleMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                        ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        ImoStyle* pStyle;
//        string name;
//
//        //<name>
//        if (get_mandatory(k_name))
//            name = get_string_value();
//        else
//            return NULL;
//        string parent = (name == "Default style" ? "" : "Default style");
//
//        pStyle = create_style(name, parent);
//
//        bool fHasFontFile = false;
//        while (more_children_to_analyse())
//        {
//            // color and background
//            if (get_optional(k_color))
//                pStyle->color( get_color_value() );
//            else if (get_optional(k_background_color))
//                pStyle->background_color( get_color_value() );
//
//            // font
//            else if (get_optional("font-file"))
//            {
//                pStyle->font_file( get_string_value() );
//                fHasFontFile = true;
//            }
//            else if (get_optional(k_font_name))
//            {
//                pStyle->font_name( get_string_value() );
//                if (!fHasFontFile)
//                    pStyle->font_file("");
//            }
//            else if (get_optional(k_font_size))
//                pStyle->font_size( get_font_size_value() );
//            else if (get_optional(k_font_style))
//                pStyle->font_style( get_font_style() );
//            else if (get_optional(k_font_weight))
//                pStyle->font_weight( get_font_weight() );
//
////                // border
////                else if (get_optional(k_border))
////                    pStyle->border( get_lenght_child() );
////                else if (get_optional(k_border_top))
////                    pStyle->border_top( get_lenght_child() );
////                else if (get_optional(k_border_right))
////                    pStyle->border_right( get_lenght_child() );
////                else if (get_optional(k_border_bottom))
////                    pStyle->border_bottom( get_lenght_child() );
////                else if (get_optional(k_border_left))
////                    pStyle->border_left( get_lenght_child() );
//
//            // border width
//            else if (get_optional(k_border_width))
//                pStyle->border_width( get_lenght_child() );
//            else if (get_optional(k_border_width_top))
//                pStyle->border_width_top( get_lenght_child() );
//            else if (get_optional(k_border_width_right))
//                pStyle->border_width_right( get_lenght_child() );
//            else if (get_optional(k_border_width_bottom))
//                pStyle->border_width_bottom( get_lenght_child() );
//            else if (get_optional(k_border_width_left))
//                pStyle->border_width_left( get_lenght_child() );
//
//            // margin
//            else if (get_optional(k_margin))
//                pStyle->margin( get_lenght_child() );
//            else if (get_optional(k_margin_top))
//                pStyle->margin_top( get_lenght_child() );
//            else if (get_optional(k_margin_right))
//                pStyle->margin_right( get_lenght_child() );
//            else if (get_optional(k_margin_bottom))
//                pStyle->margin_bottom( get_lenght_child() );
//            else if (get_optional(k_margin_left))
//                pStyle->margin_left( get_lenght_child() );
//
//            // padding
//            else if (get_optional(k_padding))
//                pStyle->padding( get_lenght_child() );
//            else if (get_optional(k_padding_top))
//                pStyle->padding_top( get_lenght_child() );
//            else if (get_optional(k_padding_right))
//                pStyle->padding_right( get_lenght_child() );
//            else if (get_optional(k_padding_bottom))
//                pStyle->padding_bottom( get_lenght_child() );
//            else if (get_optional(k_padding_left))
//                pStyle->padding_left( get_lenght_child() );
//
//            //text
//            else if (get_optional(k_text_decoration))
//                pStyle->text_decoration( get_text_decoration() );
//            else if (get_optional(k_vertical_align))
//                pStyle->vertical_align( get_valign() );
//            else if (get_optional(k_text_align))
//                pStyle->text_align( get_text_align() );
//            else if (get_optional(k_line_height))
//                pStyle->line_height( get_float_child(1.5f) );
//
//            //size
//            else if (get_optional(k_min_height))
//                pStyle->min_height( get_lenght_child() );
//            else if (get_optional(k_min_width))
//                pStyle->min_width( get_lenght_child() );
//            else if (get_optional(k_max_height))
//                pStyle->max_height( get_lenght_child() );
//            else if (get_optional(k_max_width))
//                pStyle->max_width( get_lenght_child() );
//            else if (get_optional(k_height))
//                pStyle->height( get_lenght_child() );
//            else if (get_optional(k_width))
//                pStyle->width( get_lenght_child() );
//
//            //table
//            else if (get_optional(k_table_col_width))
//                pStyle->table_col_width( get_lenght_child() );
//
//            else
//            {
//                error_invalid_child();
//                move_to_next_child();
//            }
//        }
//
//        if (pStyle->get_name() != "Default style")
//            add_to_model(pStyle);
//
//        return pStyle;
//    }
//
//protected:
//
//    int get_font_style()
//    {
//        const string value = get_string_value();
//        if (value == "normal")
//            return ImoStyle::k_font_normal;
//        else if (value == "italic")
//            return ImoStyle::k_font_style_italic;
//        else
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Unknown font-style '" + value + "'. Replaced by 'normal'.");
//            return ImoStyle::k_font_normal;
//        }
//    }
//
//    int get_font_weight()
//    {
//        const string value = get_string_value();
//        if (value == "normal")
//            return ImoStyle::k_font_normal;
//        else if (value == "bold")
//            return ImoStyle::k_font_weight_bold;
//        else
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Unknown font-weight '" + value + "'. Replaced by 'normal'.");
//            return ImoStyle::k_font_normal;
//        }
//    }
//
//    int get_text_decoration()
//    {
//        const string value = get_string_value();
//        if (value == "none")
//            return ImoStyle::k_decoration_none;
//        else if (value == "underline")
//            return ImoStyle::k_decoration_underline;
//        else if (value == "overline")
//            return ImoStyle::k_decoration_overline;
//        else if (value == "line-through")
//            return ImoStyle::k_decoration_line_through;
//        else
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Unknown text decoration value '" + value + "'. Replaced by 'none'.");
//            return ImoStyle::k_decoration_none;
//        }
//    }
//
//    int get_valign()
//    {
//        const string value = get_string_value();
//        if (value == "baseline")
//            return ImoStyle::k_valign_baseline;
//        else if (value == "sub")
//            return ImoStyle::k_valign_sub;
//        else if (value == "super")
//            return ImoStyle::k_valign_super;
//        else if (value == "top")
//            return ImoStyle::k_valign_top;
//        else if (value == "text-top")
//            return ImoStyle::k_valign_text_top;
//        else if (value == "middle")
//            return ImoStyle::k_valign_middle;
//        else if (value == "bottom")
//            return ImoStyle::k_valign_bottom;
//        else if (value == "text-bottom")
//            return ImoStyle::k_valign_text_bottom;
//        else
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Unknown vertical align '" + value + "'. Replaced by 'baseline'.");
//            return ImoStyle::k_valign_baseline;
//        }
//    }
//
//    int get_text_align()
//    {
//        const string value = get_string_value();
//        if (value == "left")
//            return ImoStyle::k_align_left;
//        else if (value == "right")
//            return ImoStyle::k_align_right;
//        else if (value == "center")
//            return ImoStyle::k_align_center;
//        else if (value == "justify")
//            return ImoStyle::k_align_justify;
//        else
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Unknown text align '" + value + "'. Replaced by 'left'.");
//            return ImoStyle::k_align_left;
//        }
//    }
//
//    ImoStyle* create_style(const string& name, const string& parent)
//    {
//        ImoStyle* pDefault = NULL;
//        ImoStyles* pStyles = NULL;
//        if (m_pAnchor && m_pAnchor->is_styles())
//        {
//            pStyles = static_cast<ImoStyles*>(m_pAnchor);
//            pDefault = pStyles->get_default_style();
//        }
//
//        if (name == "Default style")
//        {
//            return pDefault;
//        }
//        else
//        {
//            ImoStyle* pParent = pDefault;
//            if (pStyles)
//                pParent = pStyles->get_style_or_default(parent);
//
//            Document* pDoc = m_pAnalyser->get_document_being_analysed();
//            ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, pDoc));
//            pStyle->set_name(name);
//            pStyle->set_parent_style(pParent);
//            return pStyle;
//        }
//    }
//
//};

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
            LOMSE_LOG_ERROR("NULL pAchor or it is not ImoNoteRest");
            return NULL;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoFermata* pImo = static_cast<ImoFermata*>(
                                ImFactory::inject(k_imo_fermata, pDoc) );

        // atrrib: type (upright | inverted) #IMPLIED
        if (has_attribute("type"))
            set_type(pImo);

//        error_if_more_elements();

        pNR->add_attachment(pDoc, pImo);
        return pImo;
    }

    void set_type(ImoFermata* pImo)
    {
        string type = get_attribute("type");
        if (type == "upright")
            pImo->set_placement(k_placement_above);
        else if (type == "inverted")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(get_line_number(&m_pChildToAnalyse),
                "Unknown fermata type '" + type + "'. Ignored.");
        }
    }

};

////@--------------------------------------------------------------------------------------
////@ <figuredBass> = (figuredBass <figuredBassSymbols>[<parenthesis>][<fbline>]
////@                              [<componentOptions>*] )
////@ <parenthesis> = (parenthesis { yes | no })  default: no
////@
////@ <fbline> = (fbline <numLine> start <startPoint><endPoint><width><color>)
////@ <fbline> = (fbline num {start | stop} [<startPoint>][<endPoint>][<width>][<color>])
//
////@ <figuredBassSymbols> = an string.
////@        It is formed by concatenation of individual strings for each interval.
////@        Each interval string is separated by a blank space from the previous one.
////@        And it can be enclosed in parenthesis.
////@        Each interval string is a combination of prefix, number and suffix,
////@        such as  "#3", "5/", "(3)", "2+" or "#".
////@        Valid values for prefix and suffix are:
////@            prefix = { + | - | # | b | = | x | bb | ## }
////@            suffix = { + | - | # | b | = | x | bb | ## | / | \ }
////@
////@ examples:
////@
////@        b6              (figuredBass "b6 b")
////@        b
////@
////@        7 ________      (figuredBass "7 5 2" (fbline 15 start))
////@        5
////@        2
////@
////@        6               (figuredBass "6 (3)")
////@        (3)
////@
//
//class FiguredBassMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    FiguredBassMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                        ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//
//        // <figuredBassSymbols> (string)
//        if (get_mandatory(k_string))
//        {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//            ImoFiguredBassInfo info( get_string_value() );
//
//            // [<parenthesis>]
//            if (get_optional(k_parenthesis))
//            {
//                //TODO: Not yet necessary. Not implemented in LenMus
//            }
//
//            // [<fbline>]
//            if (get_optional(k_fbline))
//            {}
//
//            // [<componentOptions>*]
//            //analyse_scoreobj_options(dto);
//
//            error_if_more_elements();
//
//            ImoFiguredBass* pImo = LOMSE_NEW ImoFiguredBass(info);
//            add_to_model(pImo);
//            return pImo;
//        }
//        return NULL;
//    }
//};
//
//////    //get figured bass string and split it into components
//////    std::string sData = GetNodeName(pNode->GetParameter(1));
//////    ImoFiguredBassInfo oFBData(sData);
//////    if (oFBData.get_error_msg() != "")
//////    {
//////        AnalysisError(pNode, oFBData.get_error_msg());
//////        return (ImoFiguredBass*)NULL;    //error
//////    }
//////
//////    //initialize options with default values
//////    //AWARE: There can be two fblines, one starting in this FB and another
//////    //one ending in it.
//////    int nFBL=0;     //index to next fbline
//////    lmFBLineInfo* pFBLineInfo[2];
//////    pFBLineInfo[0] = (lmFBLineInfo*)NULL;
//////    pFBLineInfo[1] = (lmFBLineInfo*)NULL;
//////
//////    //get options: <parenthesis> & <fbline>
//////    int iP;
//////    for(iP=2; iP <= nNumParms; ++iP)
//////    {
//////        lmLDPNode* pX = pNode->GetParameter(iP);
//////        std::string sName = GetNodeName(pX);
//////        if (sName == "parenthesis")
//////            ;   //TODO
//////        else if (sName == "fbline")     //start/end of figured bass line
//////        {
//////            if (nFBL > 1)
//////                AnalysisError(pX, "[Element '%s'. More than two 'fbline'. Ignored.",
//////                            sElmName.wx_str() );
//////            else
//////                pFBLineInfo[nFBL++] = AnalyzeFBLine(pX, pVStaff);
//////        }
//////        else
//////            AnalysisError(pX, "[Element '%s'. Invalid parameter '%s'. Ignored.",
//////                          sElmName.c_str(), sName.wx_str() );
//////    }
//////
//////    //analyze remaining optional parameters: <location>, <cursorPoint>
//////	lmLDPOptionalTags oOptTags(this);
//////	oOptTags.SetValid(lm_eTag_Location_x, lm_eTag_Location_y, -1);		//finish list with -1
//////	lmLocation tPos = g_tDefaultPos;
//////	oOptTags.AnalyzeCommonOptions(pNode, iP, pVStaff, NULL, NULL, &tPos);
//////
//////	//create the Figured Bass object
//////    ImoFiguredBass* pFB = pVStaff->AddFiguredBass(&oFBData, nId);
//////	pFB->SetUserLocation(tPos);
//////
//////    //save cursor data
//////    if (m_fCursorData && m_nCursorObjID == nId)
//////        m_pCursorSO = pFB;
//////
//////    //add FB line, if exists
//////    for(int i=0; i < 2; i++)
//////    {
//////        if (pFBLineInfo[i])
//////        {
//////            if (pFBLineInfo[i]->fStart)
//////            {
//////                //start of FB line. Save the information
//////                pFBLineInfo[i]->pFB = pFB;
//////                m_PendingFBLines.push_back(pFBLineInfo[i]);
//////            }
//////            else
//////            {
//////                //end of FB line. Add it to the internal model
//////                AddFBLine(pFB, pFBLineInfo[i]);
//////            }
//////        }
//////    }
//////
//////    return pFB;       //no error
//////}
//
////@--------------------------------------------------------------------------------------
////@ <font> = (font <font_name> <font_size> <font_style>)
////@ <font_name> = string   i.e. "Times New Roman", "Trebuchet"
////@ <font_size> = num      in points
////@ <font_style> = { "bold" | "normal" | "italic" | "bold-italic" }
////@
////@ Compatibility 1.5
////@     size is a number followed by 'pt'. i.e.: 12pt
//
//
//class FontMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    FontMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                 ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        ImoFontStyleDto* pFont = LOMSE_NEW ImoFontStyleDto();
//
//        //<font_name> = string
//        if (get_mandatory(k_string))
//            pFont->name = get_string_value();
//
//        //<font_size> = num
//        if (get_optional(k_number))
//            pFont->size = get_float_value(8.0f);
//        //<font_size>. Compatibility 1.5
//        else if (get_mandatory(k_label))
//            pFont->size = get_font_size_value();
//
//        //<font_style>
//        if (get_mandatory(k_label))
//            set_font_style_weight(pFont);
//
//        //add font info to parent
//        add_to_model(pFont);
//        return pFont;
//    }
//
//    void set_font_style_weight(ImoFontStyleDto* pFont)
//    {
//        const string& value = m_pChildToAnalyse.value();
//        if (value == "bold")
//        {
//            pFont->weight = ImoStyle::k_font_weight_bold;
//            pFont->style = ImoStyle::k_font_style_normal;
//        }
//        else if (value == "normal")
//        {
//            pFont->weight = ImoStyle::k_font_weight_normal;
//            pFont->style = ImoStyle::k_font_style_normal;
//        }
//        else if (value == "italic")
//        {
//            pFont->weight = ImoStyle::k_font_weight_normal;
//            pFont->style = ImoStyle::k_font_style_italic;
//        }
//        else if (value == "bold-italic")
//        {
//            pFont->weight = ImoStyle::k_font_weight_bold;
//            pFont->style = ImoStyle::k_font_style_italic;
//        }
//        else
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                "Unknown font style '" + value + "'. Replaced by 'normal'.");
//            pFont->weight = ImoStyle::k_font_weight_normal;
//            pFont->style = ImoStyle::k_font_style_normal;
//        }
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <group> = (group [<grpName>][<grpAbbrev>]<grpSymbol>[<joinBarlines>]
////@                  <instrument>+ )
////@
////@ <grpName> = <textString>
////@ <grpAbbrev> = <textString>
////@ <grpSymbol> = (symbol {none | brace | bracket} )
////@ <joinBarlines> = (joinBarlines {yes | no} )
//
//class GroupMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    GroupMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                  ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(
//                                    ImFactory::inject(k_imo_instr_group, pDoc));
//
//
//        // [<grpName>]
//        analyse_optional(k_name, pGrp);
//
//        // [<grpAbbrev>]
//        analyse_optional(k_abbrev, pGrp);
//
//        // <grpSymbol>
//        if (!get_optional(k_symbol) || !set_symbol(pGrp))
//        {
//            error_msg("Missing or invalid group symbol. Must be 'none', 'brace' or 'bracket'. Group ignored.");
//            delete pGrp;
//            return NULL;
//        }
//
//        // [<joinBarlines>]
//        if (get_optional(k_joinBarlines))
//            set_join_barlines(pGrp);
//
//        // <instrument>+
//        if (!more_children_to_analyse())
//        {
//            error_msg("Missing instruments in group!. Group ignored.");
//            delete pGrp;
//            return NULL;
//        }
//        else
//        {
//            while (more_children_to_analyse())
//            {
//                if (!analyse_optional(k_instrument, pGrp))
//                {
//                    error_invalid_child();
//                    move_to_next_child();
//                }
//            }
//        }
//
//        add_to_model(pGrp);
//        return pGrp;
//    }
//
//protected:
//
//    bool set_symbol(ImoInstrGroup* pGrp)
//    {
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//        string symbol = get_string_value();
//        if (symbol == "brace")
//            pGrp->set_symbol(ImoInstrGroup::k_brace);
//        else if (symbol == "bracket")
//            pGrp->set_symbol(ImoInstrGroup::k_bracket);
//        else if (symbol == "none")
//            pGrp->set_symbol(ImoInstrGroup::k_none);
//        else
//            return false;   //error
//
//        return true;    //ok
//    }
//
//    void set_join_barlines(ImoInstrGroup* pGrp)
//    {
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//        pGrp->set_join_barlines( get_bool_value(true) );
//    }
//
//};
//
//

////@--------------------------------------------------------------------------------------
////@ <line> = (line <startPoint><endPoint>[<width>][<color>]
////@                [<lineStyle>][<startCap>][<endCap>])
////@ <startPoint> = (startPoint <location>)      (coordinates in tenths)
////@ <endPoint> = (endPoint <location>)      (coordinates in tenths)
////@ <lineStyle> = (lineStyle { none | solid | longDash | shortDash | dot | dotDash } )
////@ <startCap> = (lineCapStart <capType>)
////@ <endCap> = (lineCapEnd <capType>)
////@ <capType> = label: { none | arrowhead | arrowtail | circle | square | diamond }
////@
////@ Example:
////@     (line (startPoint (dx 5.0)(dy 5.0)) (endPoint (dx 80.0)(dy -10.0))
////@           (width 1.0)(color #000000)(lineStyle solid)
////@           (lineCapStart arrowhead)(lineCapEnd none) )
////@
//
//class LineMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    LineMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                 ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoScoreLine* pLine = static_cast<ImoScoreLine*>(
//                                    ImFactory::inject(k_imo_score_line, pDoc) );
//
//        // <startPoint>
//        if (get_mandatory(k_startPoint))
//            pLine->set_start_point( get_point_child() );
//
//        // <endPoint>
//        if (get_mandatory(k_endPoint))
//            pLine->set_end_point( get_point_child() );
//
//        // [<width>]
//        if (get_optional(k_width))
//            pLine->set_width( get_width_child(1.0f) );
//
//        // [<color>])
//        if (get_optional(k_color))
//            pLine->set_color( get_color_child() );
//
//        // [<lineStyle>]
//        if (get_optional(k_lineStyle))
//            pLine->set_line_style( get_line_style_child() );
//
//        // [<startCap>]
//        if (get_optional(k_lineCapStart))
//            pLine->set_start_cap( get_line_cap_child() );
//
//        // [<endCap>]
//        if (get_optional(k_lineCapEnd))
//            pLine->set_end_cap( get_line_cap_child() );
//
//        add_to_model(pLine);
//        return pLine;
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <metronome> = (metronome { <NoteType><TicksPerMinute> | <NoteType><NoteType> |
////@                            <TicksPerMinute> }
////@                          [parenthesis][<componentOptions>*] )
////@
////@ examples:
////@    (metronome q 80)                -->  quarter_note_sign = 80
////@    (metronome q q.)                -->  quarter_note_sign = dotted_quarter_note_sign
////@    (metronome 80)                  -->  m.m. = 80
////@    (metronome q 80 parenthesis)    -->  (quarter_note_sign = 80)
////@    (metronome 120 noVisible)       -->  nothing displayed
//
//class MetronomeMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    MetronomeMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                      ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoMetronomeMark* pMtr = static_cast<ImoMetronomeMark*>(
//                                    ImFactory::inject(k_imo_metronome_mark, pDoc) );
//
//        // { <NoteType><TicksPerMinute> | <NoteType><NoteType> | <TicksPerMinute> }
//        if (get_optional(k_label))
//        {
//            NoteTypeAndDots figdots = get_note_type_and_dots();
//            pMtr->set_left_note_type( figdots.noteType );
//            pMtr->set_left_dots( figdots.dots );
//
//            if (get_optional(k_number))
//            {
//                // case 1: <NoteType><TicksPerMinute>
//                pMtr->set_ticks_per_minute( get_integer_value(60) );
//                pMtr->set_mark_type(ImoMetronomeMark::k_note_value);
//            }
//            else if (get_optional(k_label))
//            {
//                // case 2: <NoteType><NoteType>
//                NoteTypeAndDots figdots = get_note_type_and_dots();
//                pMtr->set_right_note_type( figdots.noteType );
//                pMtr->set_right_dots( figdots.dots );
//                pMtr->set_mark_type(ImoMetronomeMark::k_note_note);
//            }
//            else
//            {
//                report_msg(get_line_number(&m_pAnalysedNode),
//                        "Error in metronome parameters. Replaced by '(metronome 60)'.");
//                pMtr->set_ticks_per_minute(60);
//                pMtr->set_mark_type(ImoMetronomeMark::k_value);
//                add_to_model(pMtr);
//                return NULL;
//            }
//        }
//        else if (get_optional(k_number))
//        {
//            // case 3: <TicksPerMinute>
//            pMtr->set_ticks_per_minute( get_integer_value(60) );
//            pMtr->set_mark_type(ImoMetronomeMark::k_value);
//        }
//        else
//        {
//            report_msg(get_line_number(&m_pAnalysedNode),
//                    "Missing metronome parameters. Replaced by '(metronome 60)'.");
//            pMtr->set_ticks_per_minute(60);
//            pMtr->set_mark_type(ImoMetronomeMark::k_value);
//            add_to_model(pMtr);
//            return NULL;
//        }
//
//        // [parenthesis]
//        if (get_optional(k_label))
//        {
//            if (m_pChildToAnalyse.value() == "parenthesis")
//                pMtr->set_parenthesis(true);
//            else
//                error_invalid_child();
//        }
//
//        // [<componentOptions>*]
//        analyse_scoreobj_options(pMtr);
//
//        error_if_more_elements();
//
//        add_to_model(pMtr);
//        return pMtr;
//    }
//};
//
////@--------------------------------------------------------------------------------------
////@ <option> = (opt <name><value>)
////@ <name> = label
////@ <value> = { number | label | string }
//
//class OptMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    OptMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        ImoOptionInfo* pOpt = NULL;
//
//        // <name> (label)
//        string name;
//        if (get_mandatory(k_label))
//            name = m_pChildToAnalyse.value();
//
//        // <value> { number | label | string }
//        if (get_optional(k_label) || get_optional(k_number) || get_optional(k_string))
//        {
//            pOpt =  set_option(name);
//            if (pOpt)
//                error_if_more_elements();
//            else
//            {
//                if ( is_bool_option(name) || is_number_long_option(name)
//                     || is_number_float_option(name) || is_string_option(name) )
//                    report_msg(get_line_number(&m_pChildToAnalyse),
//                        "Invalid value for option '" + name + "'. Option ignored.");
//                else
//                    report_msg(get_line_number(&m_pChildToAnalyse),
//                        "Invalid option '" + name + "'. Option ignored.");
//            }
//        }
//        else
//            error_msg("Missing value for option '" + name + "'. Option ignored.");
//
//        return pOpt;
//    }
//
//
//    ImoOptionInfo* set_option(string& name)
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
//                                        ImFactory::inject(k_imo_option, pDoc) );
//        pOpt->set_name(name);
//
//        bool fOk = false;
//        if (is_bool_option(name))
//            fOk = set_bool_value(pOpt);
//        else if (is_number_long_option(name))
//            fOk= set_long_value(pOpt);
//        else if (is_number_float_option(name))
//            fOk = set_float_value(pOpt);
//        else if (is_string_option(name))
//            fOk = set_string_value(pOpt);
//
//        if (fOk)
//        {
//            add_to_model(pOpt);
//            return pOpt;
//        }
//        else
//            delete pOpt;
//
//        return NULL;
//    }
//
//    bool is_bool_option(const string& name)
//    {
//        return (name == "StaffLines.StopAtFinalBarline")
//            || (name == "StaffLines.Hide")
//            || (name == "Staff.DrawLeftBarline")
//            || (name == "Score.FillPageWithEmptyStaves")
//            || (name == "Score.JustifyFinalBarline");
//    }
//
//    bool is_number_long_option(const string& name)
//    {
//        return (name == "Staff.UpperLegerLines.Displacement")
//                 || (name == "Render.SpacingMethod")
//                 || (name == "Render.SpacingValue");
//    }
//
//    bool is_number_float_option(const string& name)
//    {
//        return (name == "Render.SpacingFactor");
//    }
//
//    bool is_string_option(const string& name)
//    {
//        return false;       //no options for now
//    }
//
//    bool set_bool_value(ImoOptionInfo* pOpt)
//    {
//        if (is_bool_value())
//        {
//            pOpt->set_bool_value( get_bool_value() );
//            pOpt->set_type(ImoOptionInfo::k_boolean);
//            return true;    //ok
//        }
//        return false;   //error
//    }
//
//    bool set_long_value(ImoOptionInfo* pOpt)
//    {
//        if (is_long_value())
//        {
//            pOpt->set_long_value( get_long_value() );
//            pOpt->set_type(ImoOptionInfo::k_number_long);
//            return true;    //ok
//        }
//        return false;   //error
//    }
//
//    bool set_float_value(ImoOptionInfo* pOpt)
//    {
//        if (is_float_value())
//        {
//            pOpt->set_float_value( get_float_value() );
//            pOpt->set_type(ImoOptionInfo::k_number_float);
//            return true;    //ok
//        }
//        return false;   //error
//    }
//
//    bool set_string_value(ImoOptionInfo* pOpt)
//    {
//        string value = m_pChildToAnalyse.value();
//        pOpt->set_string_value( value );
//        pOpt->set_type(ImoOptionInfo::k_string);
//        return true;   //no error
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <pageLayout> = (pageLayout <pageSize><pageMargins><pageOrientation>)
////@ <pageSize> = (pageSize width height)
////@ <pageMargins> = (pageMargins left top right bottom binding)
////@ <pageOrientation> = [ "portrait" | "landscape" ]
////@ width, height, left, top right, bottom, binding = <num> in LUnits
//
//class PageLayoutMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    PageLayoutMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                       ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoPageInfo* pInfo = static_cast<ImoPageInfo*>(
//                                        ImFactory::inject(k_imo_page_info, pDoc) );
//
//        // <pageSize>
//        analyse_mandatory(k_pageSize, pInfo);
//
//        // <pageMargins>
//        analyse_mandatory(k_pageMargins, pInfo);
//
//        // [ "portrait" | "landscape" ]
//        if (!get_mandatory(k_label) || !set_orientation(pInfo))
//        {
//            report_msg(get_line_number(&m_pChildToAnalyse),
//                    "Invalid orientation. Expected 'portrait' or 'landscape'."
//                    " 'portrait' assumed.");
//            pInfo->set_portrait(true);
//        }
//
//        error_if_more_elements();
//
//        add_to_model(pInfo);
//        return pInfo;
//    }
//
//protected:
//
//    bool set_orientation(ImoPageInfo* pInfo)
//    {
//        // return true if ok
//        string type = m_pChildToAnalyse.value();
//        if (type == "portrait")
//        {
//            pInfo->set_portrait(true);
//            return true;
//        }
//        else if (type == "landscape")
//        {
//            pInfo->set_portrait(false);
//            return true;
//        }
//        return false;
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <pageMargins> = (pageMargins left top right bottom binding)     LUnits
//
//class PageMarginsMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    PageMarginsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                        ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        //ImoPageInfo dto;
//        ImoPageInfo* pDto;
//        if (m_pAnchor && m_pAnchor->is_page_info())
//            pDto = static_cast<ImoPageInfo*>(m_pAnchor);
//        else
//            return NULL;         //what is this for?
//            //pDto = &dto;
//
//        //left
//        if (get_mandatory(k_number))
//            pDto->set_left_margin( get_float_value(2000.0f) );
//
//        //top
//        if (get_mandatory(k_number))
//            pDto->set_top_margin( get_float_value(2000.0f) );
//
//        //right
//        if (get_mandatory(k_number))
//            pDto->set_right_margin( get_float_value(1500.0f) );
//
//        //bottom
//        if (get_mandatory(k_number))
//            pDto->set_bottom_margin( get_float_value(2000.0f) );
//
//        //binding
//        if (get_mandatory(k_number))
//            pDto->set_binding_margin( get_float_value(0.0f) );
//
//        return pDto;
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <pageSize> = (pageSize width height)        LUnits
//
//class PageSizeMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    PageSizeMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                     ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        //ImoPageInfo dto;
//        ImoPageInfo* pDto;
//        if (m_pAnchor && m_pAnchor->is_page_info())
//            pDto = static_cast<ImoPageInfo*>(m_pAnchor);
//        else
//            return NULL;     //what is this for?
//            //pDto = &dto;
//
//        //width
//        if (get_mandatory(k_number))
//            pDto->set_page_width( get_float_value(21000.0f) );
//
//        //height
//        if (get_mandatory(k_number))
//            pDto->set_page_height( get_float_value(29700.0f) );
//
//        return pDto;
//    }
//};
//
//
////@--------------------------------------------------------------------------------------
////@ <point> = (tag (dx value)(dy value))
//
//class PointMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    PointMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                  ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoPointDto point;
//
//        // <dx>
//        if (get_mandatory(k_dx))
//            point.set_x( get_location_child() );
//
//        // <dy>
//        if (get_mandatory(k_dy))
//            point.set_y( get_location_child() );
//
//        error_if_more_elements();
//
//        ImoPointDto* pImo = LOMSE_NEW ImoPointDto(point);
//        add_to_model(pImo);
//        return pImo;
//    }
//};
//
//
////@--------------------------------------------------------------------------------------
////@ <size> = (size <width><height>)
////@ <width> = (width number)        value in LUnits
////@ <height> = (height number)      value in LUnits
////@     i.e.; (size (width 160)(height 100.7))
//
//class SizeMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    SizeMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                 ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoSizeDto size;
//
//        // <width>
//        if (get_mandatory(k_width))
//            size.set_width( get_width_child() );
//
//        // <height>
//        if (get_mandatory(k_height))
//            size.set_height( get_height_child() );
//
//        error_if_more_elements();
//
//        ImoSizeDto* pImo = LOMSE_NEW ImoSizeDto(size);
//        add_to_model(pImo);
//        return pImo;
//    }
//};
//
//
////@--------------------------------------------------------------------------------------
////@ <slur> = (slur num <slurType>[<bezier>][color] )   ;num = slur number. integer
////@ <slurType> = { start | continue | stop }
////@
////@ Example:
////@     (slur 27 start (bezier (ctrol2-x -25)(start-y 36.765)) )
////@
//
//class SlurMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    SlurMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoSlurDto* pInfo = LOMSE_NEW ImoSlurDto();
//
//        // num
//        if (get_mandatory(k_number))
//            pInfo->set_slur_number( get_integer_value(0) );
//
//        // <slurType> (label)
//        if (!get_mandatory(k_label) || !set_slur_type(pInfo))
//        {
//            error_msg("Missing or invalid slur type. Slur ignored.");
//            delete pInfo;
//            return NULL;
//        }
//
//        // [<bezier>]
//        analyse_optional(k_bezier, pInfo);
//
//        // [<color>]
//        if (get_optional(k_color))
//            pInfo->set_color( get_color_child() );
//
//        return pInfo;   //set_imo(m_pAnalysedNode, pInfo);
//    }
//
//protected:
//
//    bool set_slur_type(ImoSlurDto* pInfo)
//    {
//        const std::string& value = m_pChildToAnalyse.value();
//        if (value == "start")
//            pInfo->set_slur_type(ImoSlurData::k_start);
//        else if (value == "stop")
//            pInfo->set_slur_type(ImoSlurData::k_stop);
//        else if (value == "continue")
//            pInfo->set_slur_type(ImoSlurData::k_continue);
//        else
//            return false;   //error
//        return true;    //ok
//    }
//};
//
////@--------------------------------------------------------------------------------------
////@ ImoSpacer StaffObj
////@ <spacer> = (spacer <width>[<staffobjOptions>*][<attachments>*])     width in Tenths
//
//class SpacerMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    SpacerMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                   ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoSpacer* pSpacer = static_cast<ImoSpacer*>(
//                                    ImFactory::inject(k_imo_spacer, pDoc) );
//
//        // <width>
//        if (get_optional(k_number))
//        {
//            pSpacer->set_width( get_float_value() );
//        }
//        else
//        {
//            error_msg("Missing width for spacer. Spacer ignored.");
//            delete pSpacer;
//            return NULL;
//        }
//
//        // [<staffobjOptions>*]
//        analyse_staffobjs_options(pSpacer);
//
//        add_to_model(pSpacer);
//
//       // [<attachments>*]
//        analyse_attachments(pSpacer);
//        return pSpacer;
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ ImoStaffInfo SimpleObj
////@ <staff> = (staff <num> [<staffType>][<staffLines>][<staffSpacing>]
////@                        [<staffDistance>][<lineThickness>] )
////@
////@ <staffType> = (staffType { ossia | cue | editorial | regular | alternate } )
////@ <staffLines> = (staffLines <integer_num>)
////@ <staffSpacing> = (staffSpacing <real_num>)      LUnits (cents of millimeter)
////@ <staffDistance> = (staffDistance <real_num>)    LUnits (cents of millimeter)
////@ <lineThickness> = (lineThickness <real_num>)    LUnits (cents of millimeter)
////@
////@ Example:
////@     (staff 1 (staffType regular)(staffLines 5)(staffSpacing 180.00)
////@              (staffDistance 2000.00)(lineThickness 15.00))
////@
//
//class StaffMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    StaffMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                  ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
//                                    ImFactory::inject(k_imo_staff_info, pDoc) );
//
//        // num_instr
//        if (!get_optional(k_number) || !set_staff_number(pInfo))
//        {
//            error_msg("Missing or invalid staff number. Staff info ignored.");
//            delete pInfo;
//            return NULL;
//        }
//
//        // [<staffType>]
//        if (get_optional(k_staffType))
//            set_staff_type(pInfo);
//
//        // [<staffLines>]
//        if (get_optional(k_staffLines))
//            set_staff_lines(pInfo);
//
//        // [<staffSpacing>]
//        if (get_optional(k_staffSpacing))
//        {
//            m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//            pInfo->set_line_spacing( get_float_value(180.0f));
//        }
//
//        //[<staffDistance>]
//        if (get_optional(k_staffDistance))
//        {
//            m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//            pInfo->set_staff_margin( get_float_value(1000.0f));
//        }
//
//        //[<lineThickness>]
//        if (get_optional(k_lineThickness))
//        {
//            m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//            pInfo->set_line_thickness( get_float_value(15.0f));
//        }
//
//        error_if_more_elements();
//
//        add_to_model(pInfo);
//        return pInfo;
//    }
//
//protected:
//
//    bool set_staff_number(ImoStaffInfo* pInfo)
//    {
//        int value = get_integer_value(0);
//        if (value < 1)
//            return false;   //error
//        pInfo->set_staff_number(value-1);
//        return true;
//    }
//
//    void set_staff_type(ImoStaffInfo* pInfo)
//    {
//        const std::string& value = get_value( get_child(m_pChildToAnalyse, 1) );
//        if (value == "ossia")
//            pInfo->set_staff_type(ImoStaffInfo::k_staff_ossia);
//        else if (value == "cue")
//            pInfo->set_staff_type(ImoStaffInfo::k_staff_cue);
//        else if (value == "editorial")
//            pInfo->set_staff_type(ImoStaffInfo::k_staff_editorial);
//        else if (value == "regular")
//            pInfo->set_staff_type(ImoStaffInfo::k_staff_regular);
//        else if (value == "alternate")
//            pInfo->set_staff_type(ImoStaffInfo::k_staff_alternate);
//        else
//            error_msg("Invalid staff type '" + value + "'. 'regular' staff assumed.");
//    }
//
//    void set_staff_lines(ImoStaffInfo* pInfo)
//    {
//        m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//
//        int value = get_integer_value(0);
//        if (value < 1)
//            error_msg("Invalid staff. Num lines must be greater than zero. Five assumed.");
//        else
//            pInfo->set_num_lines(value);
//    }
//
//
//};
//
////---------------------------------------------------------------------------------------
//class StylesMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    StylesMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                   ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoStyles* pStyles = static_cast<ImoStyles*>(ImFactory::inject(k_imo_styles, pDoc));
//
//        // [<defineStyle>*]
//        while (analyse_optional(k_defineStyle, pStyles));
//
//        error_if_more_elements();
//
//        add_to_model(pStyles);
//        return pStyles;
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <systemLayout> = (systemLayout {first | other} <systemMargins>)
//
//class SystemLayoutMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    SystemLayoutMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                         ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoSystemInfo* pInfo = static_cast<ImoSystemInfo*>(
//                                        ImFactory::inject(k_imo_system_info, pDoc));
//
//        // {first | other} <label>
//        if (get_mandatory(k_label))
//        {
//            string type = m_pChildToAnalyse.value();
//            if (type == "first")
//                pInfo->set_first(true);
//            else if (type == "other")
//                pInfo->set_first(false);
//            else
//            {
//                report_msg(get_line_number(&m_pChildToAnalyse),
//                        "Expected 'first' or 'other' value but found '" + type
//                        + "'. 'first' assumed.");
//                pInfo->set_first(true);
//            }
//        }
//
//        // <systemMargins>
//        analyse_mandatory(k_systemMargins, pInfo);
//
//        error_if_more_elements();
//
//        add_to_model(pInfo);
//        return pInfo;
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ <systemMargins> = (systemMargins <leftMargin><rightMargin><systemDistance>
////@                                  <topSystemDistance>)
////@ <leftMargin>, <rightMargin>, <systemDistance>, <topSystemDistance> = number (Tenths)
//
//class SystemMarginsMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    SystemMarginsMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                          ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        //ImoSystemInfo dto;
//        ImoSystemInfo* pDto;
//        if (m_pAnchor && m_pAnchor->is_system_info())
//            pDto = static_cast<ImoSystemInfo*>(m_pAnchor);
//        else
//            return NULL;     //what is this for?
//            //pDto = &dto;
//
//        if (get_mandatory(k_number))
//            pDto->set_left_margin(get_float_value());
//
//        if (get_mandatory(k_number))
//            pDto->set_right_margin(get_float_value());
//
//        if (get_mandatory(k_number))
//            pDto->set_system_distance(get_float_value());
//
//        if (get_mandatory(k_number))
//            pDto->set_top_system_distance(get_float_value());
//
//        error_if_more_elements();
//        return pDto;
//    }
//
//};
//
//
////@--------------------------------------------------------------------------------------
////@ <textItem> = (txt [<style>] string)
////@ <style> = (style <name>)
////@     if no style is specified it will inherit from parent style
////@
////$ <!ELEMENT txt %String;>
////$ <!ATTLIST txt %styleName;>
////$
////$ Example:
////$     <txt style='bold'>This is a text.</txt>
////$
//class TextItemMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    TextItemMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                       ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
//    {
//    }
//
//    ImoObj* do_analysis()
//    {
//        // [<style>]
//        ImoStyle* pStyle = NULL;
//        if (has_attribute("style"))
//            pStyle = get_doc_text_style( get_attribute("style") );
//
//        // <string>
//        string value = get_value(m_pAnalysedNode);
//        if (value.empty())
//        {
//            error_msg("txt: missing mandatory value 'string'. Element <txt> ignored.");
//            return NULL;
//        }
//        else
//        {
//            Document* pDoc = m_pAnalyser->get_document_being_analysed();
//            ImoTextItem* pText = static_cast<ImoTextItem*>(
//                            ImFactory::inject(k_imo_text_item, pDoc, get_node_id()) );
//            pText->set_text(value);
//            pText->set_style(pStyle);
//
//            add_to_model(pText);
//            return pText;
//        }
//    }
//};
//
////@--------------------------------------------------------------------------------------
////@ <textString> = (<textTag> string [<style>][<location>])
////@ <textTag> = { name | abbrev | text }
////@ <style> = (style <name>)
////@
////@ Compatibility 1.5:
////@ <style> is now mandatory
////@     For compatibility with 1.5, if no style is specified default style is
////@     assigned.
////@
//class TextStringMxlAnalyser : public MxlElementAnalyser
//{
//protected:
//    string m_styleName;
//
//public:
//    TextStringMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                       ImoObj* pAnchor, const string& styleName="Default style")
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
//        , m_styleName(styleName)
//    {
//    }
//
//    ImoObj* do_analysis()
//    {
//        // <string>
//        if (get_mandatory(k_string))
//        {
//            Document* pDoc = m_pAnalyser->get_document_being_analysed();
//            ImoScoreText* pText = static_cast<ImoScoreText*>(
//                                        ImFactory::inject(k_imo_score_text, pDoc));
//            pText->set_text(get_string_value());
//
//            // [<style>]
//            ImoStyle* pStyle = NULL;
//            if (get_optional(k_style))
//                pStyle = get_text_style_child(m_styleName);
//            else
//            {
//                ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
//                if (pScore)     //in unit tests the score might not exist
//                    pStyle = pScore->get_default_style();
//            }
//            pText->set_style(pStyle);
//
//            // [<location>]
//            while (more_children_to_analyse())
//            {
//                if (get_optional(k_dx))
//                    pText->set_user_location_x( get_location_child() );
//                else if (get_optional(k_dy))
//                    pText->set_user_location_y( get_location_child() );
//                else
//                {
//                    error_invalid_child();
//                    move_to_next_child();
//                }
//            }
//            error_if_more_elements();
//
//            add_to_model(pText);
//            return pText;
//        }
//        return NULL;
//    }
//
//};
//
//class InstrNameMxlAnalyser : public TextStringMxlAnalyser
//{
//public:
//    InstrNameMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                      ImoObj* pAnchor)
//        : TextStringMxlAnalyser(pAnalyser, reporter, libraryScope, pAnchor,
//                             "Instrument names") {}
//};


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
                m_pInfo1->set_orientation(ImoTie::k_orientation_over);
            else
                m_pInfo1->set_orientation(ImoTie::k_orientation_under);
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

////@--------------------------------------------------------------------------------------
////@ <title> = (title <h-alignment> string [<style>][<location>])
////@ <h-alignment> = label: {left | center | right }
////@ <style> = (style name)
////@         name = string.  Must be a style name defined with defineStyle
////@
////@ Note:
////@     <h-alignment> overrides <style> (but doesn't modify it)
////@
////@ Examples:
////@     (title center "Prelude" (style "Title")
////@     (title center "Op. 28, No. 20" (style "Subtitle")
////@     (title right "F. Chopin" (style "Composer"(dy 30))
//
//class TitleMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    TitleMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                  ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        // [<h-alignment>]
//        int nAlignment = k_halign_left;
//        if (get_mandatory(k_label))
//            nAlignment = get_alignment_value(k_halign_center);
//
//        // <string>
//        if (get_mandatory(k_string))
//        {
//            Document* pDoc = m_pAnalyser->get_document_being_analysed();
//            ImoScoreTitle* pTitle = static_cast<ImoScoreTitle*>(
//                ImFactory::inject(k_imo_score_title, pDoc) );
//            pTitle->set_text( get_string_value() );
//            pTitle->set_h_align(nAlignment);
//
//            // [<style>]
//            if (get_optional(k_style))
//                pTitle->set_style( get_text_style_child() );
//
//            // [<location>]
//            while (more_children_to_analyse())
//            {
//                if (get_optional(k_dx))
//                    pTitle->set_user_location_x( get_location_child() );
//                else if (get_optional(k_dy))
//                    pTitle->set_user_location_y( get_location_child() );
//                else
//                {
//                    error_invalid_child();
//                    move_to_next_child();
//                }
//            }
//            error_if_more_elements();
//
//            add_to_model(pTitle);
//            return pTitle;
//        }
//        return NULL;
//    }
//
//};
//
////@--------------------------------------------------------------------------------------
////@ Old syntax: v1.5
////@ <tuplet> = (t { - | + <actualNotes>[<normalNotes>][<tupletOptions>] } )
////@ <actualNotes> = num
////@ <normalNotes> = num
////@ <tupletOptions> = noBracket
////@    future options: squaredBracket | curvedBracket |
////@                    numNone | numActual | numBoth
////@
////@ Abbreviations (old syntax. Deprecated since 1.6):
////@      (t -)     --> t-
////@      (t + n)   --> tn
////@      (t + n m) --> tn/m
////@
////@ New syntax: v1.6
////@ <tuplet> = (t <tupletID> { - | + <actualNotes>[<normalNotes>][<tupletOptions>] } )
////@ <tupletID> = integer number
////@ <actualNotes> = integer number
////@ <normalNotes> = integer number
////@ <tupletOptions> =  [<bracketType>] [<displayBracket>] [<displayNumber>]
////@ <bracketType> = (bracketType { squaredBracket | curvedBracket })
////@ <displayBracket> = (displayBracket { yes | no })
////@ <displayNumber> = (displayNumber { none | actual | both })
//
//class TupletMxlAnalyser : public MxlElementAnalyser
//{
//public:
//    TupletMxlAnalyser(MxlAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
//                   ImoObj* pAnchor)
//        : MxlElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}
//
//    ImoObj* do_analysis()
//    {
//        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
//        set_default_values(pInfo);
//
//        // [<tupletID>]     //optional for 1.5 compatibility. TO BE REMOVED
//        if (get_optional(k_number))
//            set_tuplet_id(pInfo);
//
//        // { + | - }
//        if (!get_mandatory(k_label) || !set_tuplet_type(pInfo))
//        {
//            error_msg("Missing or invalid tuplet type. Tuplet ignored.");
//            delete pInfo;
//            return NULL;
//        }
//
//        if (pInfo->is_start_of_tuplet())
//        {
//            // <actualNotes>
//            if (!get_mandatory(k_number) || !set_actual_notes(pInfo))
//            {
//                error_msg("Tuplet: missing or invalid actual notes number. Tuplet ignored.");
//                delete pInfo;
//                return NULL;
//            }
//
//            // [<normalNotes>]
//            if (get_optional(k_number))
//                set_normal_notes(pInfo);
//            if (pInfo->get_normal_number() == 0)
//            {
//                error_msg("Tuplet: Missing or invalid normal notes number. Tuplet ignored.");
//                delete pInfo;
//                return NULL;
//            }
//
//            // [<tupletOptions>]
//            analyse_tuplet_options(pInfo);
//        }
//
//        return pInfo;   //set_imo(m_pAnalysedNode, pInfo);
//    }
//
//protected:
//
//    void set_default_values(ImoTupletDto* pInfo)
//    {
//        pInfo->set_show_bracket( m_pAnalyser->get_current_show_tuplet_bracket() );
//        pInfo->set_placement(k_placement_default);
//    }
//
//    void set_tuplet_id(ImoTupletDto* pInfo)
//    {
//        //TODO. For now tuplet id is not needed. Perhaps when implementing nested
//        //      tuplets it will have any use.
//        get_integer_value(0);
//    }
//
//    bool set_tuplet_type(ImoTupletDto* pInfo)
//    {
//        const std::string& value = m_pChildToAnalyse.value();
//        if (value == "+")
//            pInfo->set_tuplet_type(ImoTupletDto::k_start);
//        else if (value == "-")
//            pInfo->set_tuplet_type(ImoTupletDto::k_stop);
//        else
//            return false;   //error
//        return true;    //ok
//    }
//
//    bool set_actual_notes(ImoTupletDto* pInfo)
//    {
//        int actual = get_integer_value(0);
//        pInfo->set_actual_number(actual);
//        if (actual == 2)
//            pInfo->set_normal_number(3);   //duplet
//        else if (actual == 3)
//            pInfo->set_normal_number(2);   //triplet
//        else if (actual == 4)
//            pInfo->set_normal_number(6);
//        else if (actual == 5)
//            pInfo->set_normal_number(6);
//        else
//            pInfo->set_normal_number(0);  //required
//        return true;    //ok
//    }
//
//    void set_normal_notes(ImoTupletDto* pInfo)
//    {
//        int normal = get_integer_value(0);
//        pInfo->set_normal_number(normal);
//    }
//
//    void analyse_tuplet_options(ImoTupletDto* pInfo)
//    {
//        //@ <tupletOptions> =  [<bracketType>] [<displayBracket>] [<displayNumber>]
//        //@ <bracketType> = (bracketType { squaredBracket | curvedBracket })
//        //@ <displayBracket> = (displayBracket { yes | no })
//        //@ <displayNumber> = (displayNumber { none | actual | both })
//
//        int nShowBracket = m_pAnalyser->get_current_show_tuplet_bracket();
//        int nShowNumber = m_pAnalyser->get_current_show_tuplet_number();
//        while( more_children_to_analyse() )
//        {
//            m_pChildToAnalyse = get_child_to_analyse();
//            ELdpElement type = get_type(m_pChildToAnalyse);
//            switch (type)
//            {
//                case k_label:
//                {
//                    const std::string& value = m_pChildToAnalyse.value();
//                    if (value == "noBracket")
//                        nShowBracket = k_yesno_no;
//                    else
//                        error_invalid_child();
//                    break;
//                }
//
//                case k_displayBracket:
//                {
//                    m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//                    nShowBracket = get_yes_no_value(k_yesno_default);
//                    break;
//                }
//
//                case k_displayNumber:
//                {
//                    m_pChildToAnalyse = get_child(m_pChildToAnalyse, 1);
//                    const std::string& value = m_pChildToAnalyse.value();
//                    if (value == "none")
//                        nShowNumber = ImoTuplet::k_number_none;
//                    else if (value == "actual")
//                        nShowNumber = ImoTuplet::k_number_actual;
//                    else if (value == "both")
//                        nShowNumber = ImoTuplet::k_number_both;
//                    else
//                    {
//                        error_invalid_child();
//                        nShowNumber = ImoTuplet::k_number_actual;
//                    }
//                    break;
//                }
//
//                default:
//                    error_invalid_child();
//            }
//
//            move_to_next_child();
//        }
//
//        pInfo->set_show_bracket(nShowBracket);
//        pInfo->set_show_number(nShowNumber);
//        m_pAnalyser->set_current_show_tuplet_bracket(nShowBracket);
//        m_pAnalyser->set_current_show_tuplet_number(nShowNumber);
//    }
//
//};


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
//    , m_pSlursBuilder(NULL)
    , m_musicxmlVersion(0)
    , m_pNodeImo(NULL)
    , m_tieNum(0)
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
    m_NameToEnum["attributes"] = k_mxl_tag_attributes;
    m_NameToEnum["backup"] = k_mxl_tag_backup;
    m_NameToEnum["barline"] = k_mxl_tag_barline;
    m_NameToEnum["clef"] = k_mxl_tag_clef;
    m_NameToEnum["direction"] = k_mxl_tag_direction;
    m_NameToEnum["fermata"] = k_mxl_tag_fermata;
    m_NameToEnum["forward"] = k_mxl_tag_forward;
    m_NameToEnum["key"] = k_mxl_tag_key;
    m_NameToEnum["measure"] = k_mxl_tag_measure;
    m_NameToEnum["notations"] = k_mxl_tag_notations;
    m_NameToEnum["note"] = k_mxl_tag_note;
    m_NameToEnum["part"] = k_mxl_tag_part;
    m_NameToEnum["part-list"] = k_mxl_tag_part_list;
    m_NameToEnum["part-name"] = k_mxl_tag_part_name;
    m_NameToEnum["pitch"] = k_mxl_tag_pitch;
    m_NameToEnum["print"] = k_mxl_tag_print;
    m_NameToEnum["rest"] = k_mxl_tag_rest;
    m_NameToEnum["score-part"] = k_mxl_tag_score_part;
    m_NameToEnum["score-partwise"] = k_mxl_tag_score_partwise;
    m_NameToEnum["sound"] = k_mxl_tag_sound;
    m_NameToEnum["tied"] = k_mxl_tag_tied;
    m_NameToEnum["time"] = k_mxl_tag_time;
}

//---------------------------------------------------------------------------------------
MxlAnalyser::~MxlAnalyser()
{
    delete_relation_builders();
    m_NameToEnum.clear();
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::delete_relation_builders()
{
    delete m_pTiesBuilder;
    delete m_pBeamsBuilder;
//    delete m_pTupletsBuilder;
//    delete m_pSlursBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* MxlAnalyser::analyse_tree_and_get_object(XmlNode* root)
{
    delete_relation_builders();
    m_pTiesBuilder = LOMSE_NEW MxlTiesBuilder(m_reporter, this);
    m_pBeamsBuilder = LOMSE_NEW MxlBeamsBuilder(m_reporter, this);
//    m_pTupletsBuilder = LOMSE_NEW MxlTupletsBuilder(m_reporter, this);
//    m_pSlursBuilder = LOMSE_NEW MxlSlursBuilder(m_reporter, this);

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
//    else if (pDto->is_slur_dto())
//        m_pSlursBuilder->add_item_info(static_cast<ImoSlurDto*>(pDto));
//    else if (pDto->is_tuplet_dto())
//        m_pTupletsBuilder->add_item_info(static_cast<ImoTupletDto*>(pDto));
}

//---------------------------------------------------------------------------------------
void MxlAnalyser::clear_pending_relations()
{
    m_pTiesBuilder->clear_pending_items();
//    m_pSlursBuilder->clear_pending_items();
    m_pBeamsBuilder->clear_pending_items();
//    m_pTupletsBuilder->clear_pending_items();
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::new_tie_id(int numTie, FPitch fp)
{
    m_tieIds[int(fp)] = ++m_tieNum;
    return m_tieNum;
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_tie_id(int numTie, FPitch fp)
{
    return m_tieIds[int(fp)];
}

//---------------------------------------------------------------------------------------
int MxlAnalyser::get_tie_id_and_close(int numTie, FPitch fp)
{
    return m_tieIds[int(fp)];
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
int MxlAnalyser::xml_data_to_clef_type(const string& m_sign, int line, int m_octaveChange)
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
        case k_mxl_tag_attributes:           return LOMSE_NEW AtribbutesMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_backup:               return LOMSE_NEW FwdBackMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_barline:              return LOMSE_NEW BarlineMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_clef:                 return LOMSE_NEW ClefMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_direction:            return LOMSE_NEW DirectionMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_fermata:              return LOMSE_NEW FermataMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_forward:              return LOMSE_NEW FwdBackMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_key:                  return LOMSE_NEW KeyMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_measure:              return LOMSE_NEW MeasureMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_notations:            return LOMSE_NEW NotationsMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_note:                 return LOMSE_NEW NoteRestMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part:                 return LOMSE_NEW PartMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_part_list:            return LOMSE_NEW PartListMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_part_name:            return LOMSE_NEW PartNameMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_pitch:                return LOMSE_NEW PitchMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_print:                return LOMSE_NEW PrintMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_mxl_tag_score_part:           return LOMSE_NEW ScorePartMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_score_partwise:       return LOMSE_NEW ScorePartwiseMxlAnalyser(this, m_reporter, m_libraryScope);
        case k_mxl_tag_sound:                return LOMSE_NEW SoundMxlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
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


////=======================================================================================
//// MxlSlursBuilder implementation
////=======================================================================================
//void MxlSlursBuilder::add_relation_to_notes_rests(ImoSlurDto* pEndInfo)
//{
//    m_matches.push_back(pEndInfo);
//    Document* pDoc = m_pAnalyser->get_document_being_analysed();
//    ImoSlur* pSlur = static_cast<ImoSlur*>(ImFactory::inject(k_imo_slur, pDoc));
//    pSlur->set_slur_number( pEndInfo->get_slur_number() );
//    std::list<ImoSlurDto*>::iterator it;
//    for (it = m_matches.begin(); it != m_matches.end(); ++it)
//    {
//        ImoNote* pNote = (*it)->get_note();
//        ImoSlurData* pData = ImFactory::inject_slur_data(pDoc, *it);
//        pNote->include_in_relation(pDoc, pSlur, pData);
//    }
//}



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
//        case k_32th:
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
